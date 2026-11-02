/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ACCOUNT.C                                                       *
*  Usage: Account files with their characters (account.h); C library only*
\*********************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "structs.h"
#include "pfile.h"
#include "account.h"


int account_email(char *dst, size_t size, const char *src)
{
  size_t n = 0;
  int at = -1, i;

  while (*src && isspace((unsigned char) *src))
    src++;
  for (; *src && !isspace((unsigned char) *src); src++) {
    if (n + 1 >= size || n >= MAX_EMAIL_LENGTH)
      return -1;
    dst[n++] = tolower((unsigned char) *src);
  }
  dst[n] = '\0';
  while (*src && isspace((unsigned char) *src))
    src++;
  if (*src || n < 3)
    return -1;                          /* blanks inside, or too short   */

  for (i = 0; i < (int) n; i++) {
    char c = dst[i];

    if (c == '@') {
      if (at >= 0)
	return -1;                      /* two @s                        */
      at = i;
    } else if (!(isalnum((unsigned char) c) || c == '.' || c == '_' ||
		 c == '-' || c == '+'))
      return -1;                        /* keeps it a safe file name     */
    if (c == '.' && i > 0 && dst[i - 1] == '.')
      return -1;
  }
  if (at < 1 || at == (int) n - 1 || dst[0] == '.' ||
      !strchr(dst + at + 1, '.'))
    return -1;                          /* need user@domain.tld          */
  return 0;
}


int account_path(char *buf, size_t size, const char *dir, const char *email)
{
  char clean[MAX_EMAIL_LENGTH + 1];

  if (account_email(clean, sizeof(clean), email) < 0)
    return -1;
  if ((size_t) snprintf(buf, size, "%s/%c/%s", dir, clean[0], clean) >= size)
    return -1;
  return 0;
}


/* one-line strings: no escapes needed, but drop anything unprintable */
static void put_str(FILE *fp, const char *key, const char *s)
{
  fprintf(fp, "%s ", key);
  for (; *s; s++)
    if (isprint((unsigned char) *s))
      fputc(*s, fp);
  fputc('\n', fp);
}


int account_write(const char *dir, const struct account_file *f)
{
  const struct account_data *a = &f->a;
  char path[512], tmp[520], sub[300];
  FILE *fp;
  int i, ok;

  if (account_path(path, sizeof(path), dir, a->email) < 0)
    return -1;
  snprintf(sub, sizeof(sub), "%s/%c", dir, *a->email);
  if ((mkdir(dir, 0755) < 0 && errno != EEXIST) ||
      (mkdir(sub, 0755) < 0 && errno != EEXIST))
    return -1;
  snprintf(tmp, sizeof(tmp), "%s.new", path);
  if (!(fp = fopen(tmp, "w")))
    return -1;

  fprintf(fp, "Account %d\n", ACCOUNT_VERSION);
  put_str(fp, "Email", a->email);
  put_str(fp, "Password", a->pwd);
  fprintf(fp, "Created %lld\n", (long long) a->created);
  fprintf(fp, "LastLogon %lld\n", (long long) a->last_logon);
  put_str(fp, "LastHost", a->last_host);
  fprintf(fp, "BadPasswords %d\n", a->bad_pws);
  fputs("End\n", fp);
  for (i = 0; i < a->num_chars; i++)
    pfile_write_record(fp, &f->ch[i]);

  ok = !ferror(fp);
  if (fclose(fp) != 0)
    ok = 0;
  if (!ok || rename(tmp, path) != 0) {
    remove(tmp);
    return -1;
  }
  return 0;
}


static void get_str(char *dst, size_t size, const char *s)
{
  strncpy(dst, s, size - 1);
  dst[size - 1] = '\0';
}


int account_read(const char *path, struct account_file *f, int *unknown)
{
  static struct char_file_u extra;
  struct account_data *a = &f->a;
  char line[512], *key, *val, *p;
  int in_header = 0, got_header = 0, unk = 0, n, i;
  FILE *fp;

  memset(a, 0, sizeof(*a));
  if (unknown)
    *unknown = 0;
  if (!(fp = fopen(path, "r")))
    return -1;

  while (fgets(line, sizeof(line), fp)) {
    if ((p = strchr(line, '\n')))
      *p = '\0';
    key = line;
    if (!*key || *key == '#')
      continue;
    if ((val = strchr(key, ' ')))
      *val++ = '\0';
    else
      val = key + strlen(key);

    if (!strcmp(key, "Account"))
      in_header = got_header = 1;
    else if (!strcmp(key, "Character")) {
      in_header = 0;
      if (a->num_chars < MAX_ACCOUNT_CHARS) {
	if (pfile_read_record(fp, &f->ch[a->num_chars], &n) < 0)
	  break;
	unk += n;
	a->num_chars++;
      } else {                          /* one too many: drop it */
	if (pfile_read_record(fp, &extra, &n) < 0)
	  break;
	unk++;
      }
    } else if (!in_header)
      unk++;                            /* a line outside any section */
    else if (!strcmp(key, "End"))		in_header = 0;
    else if (!strcmp(key, "Email"))		get_str(a->email, sizeof(a->email), val);
    else if (!strcmp(key, "Password"))	get_str(a->pwd, sizeof(a->pwd), val);
    else if (!strcmp(key, "Created"))		a->created = (time_t) strtoll(val, NULL, 10);
    else if (!strcmp(key, "LastLogon"))	a->last_logon = (time_t) strtoll(val, NULL, 10);
    else if (!strcmp(key, "LastHost"))	get_str(a->last_host, sizeof(a->last_host), val);
    else if (!strcmp(key, "BadPasswords"))	a->bad_pws = atoi(val);
    else
      unk++;                            /* a newer key this build doesn't know */
  }
  fclose(fp);

  /* the account's address is every character's address */
  for (i = 0; i < a->num_chars; i++) {
    get_str(f->ch[i].email, sizeof(f->ch[i].email), a->email);
    get_str(a->chars[i], sizeof(a->chars[i]), f->ch[i].name);
  }
  if (unknown)
    *unknown = unk;
  return (got_header && *a->email) ? 0 : -1;
}


int account_foreach(const char *dir,
		    int (*fn)(const char *path, struct account_file *f, void *arg),
		    void *arg)
{
  static struct account_file f;
  struct dirent *top, *de;
  char sub[512], path[1024];
  DIR *tp, *dp;
  int rc = 0;

  if (!(tp = opendir(dir)))
    return 0;
  while (!rc && (top = readdir(tp))) {
    if (*top->d_name == '.')
      continue;
    snprintf(sub, sizeof(sub), "%s/%s", dir, top->d_name);
    if (!(dp = opendir(sub)))
      continue;                         /* not a letter folder */
    while (!rc && (de = readdir(dp))) {
      if (*de->d_name == '.' ||
	  (strlen(de->d_name) > 4 &&
	   !strcmp(de->d_name + strlen(de->d_name) - 4, ".new")))
	continue;
      snprintf(path, sizeof(path), "%s/%s", sub, de->d_name);
      rc = fn(path, account_read(path, &f, NULL) < 0 ? NULL : &f, arg);
    }
    closedir(dp);
  }
  closedir(tp);
  return rc;
}


int account_find_char(const struct account_file *f, const char *name)
{
  int i;

  for (i = 0; i < f->a.num_chars; i++)
    if (!strcasecmp(f->ch[i].name, name))
      return i;
  return -1;
}


int account_put_char(struct account_file *f, const struct char_file_u *st)
{
  struct account_data *a = &f->a;
  int i = account_find_char(f, st->name);

  if (i < 0) {
    if (a->num_chars >= MAX_ACCOUNT_CHARS)
      return -1;
    i = a->num_chars++;
  }
  f->ch[i] = *st;
  get_str(f->ch[i].email, sizeof(f->ch[i].email), a->email);
  get_str(a->chars[i], sizeof(a->chars[i]), st->name);
  return 0;
}


int account_drop_char(struct account_file *f, const char *name)
{
  struct account_data *a = &f->a;
  int i = account_find_char(f, name);

  if (i < 0)
    return -1;
  for (; i < a->num_chars - 1; i++) {
    f->ch[i] = f->ch[i + 1];
    strcpy(a->chars[i], a->chars[i + 1]);
  }
  a->num_chars--;
  return 0;
}
