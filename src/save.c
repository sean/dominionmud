/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SAVE.C                                       Based on CircleMUD *
*  Usage: Loading and saving accounts and their characters (account.c,   *
*         pfile.c), and the in-memory index of players                    *
*                                                                         *
*  Dominion Mud copyright (c) 1994, 1995, 1996, 1997, 2000, 2001 by       *
*  Sean Mountcastle, John Olvey and Kevin Huff                            *
\*********************************************************************** */

#define __SAVE_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"
#include "pfile.h"
#include "account.h"

extern struct room_data *world;
extern struct player_index_element *player_table;
extern int    top_of_p_table;
extern long   top_idnum;

/* One account file at a time is read, changed and written here; nothing
   below calls back into another function that uses it while it's busy. */
static struct account_file acct_buf;


/* add a name, its account and id to the in-memory index; returns its
   position (a deleted character's emptied slot is reused) */
int create_entry(char *name, char *email)
{
  int i, pos;

  for (pos = 0; pos <= top_of_p_table; pos++)
    if (!*player_table[pos].name)
      break;
  if (pos > top_of_p_table) {
    if (top_of_p_table == -1) {
      CREATE(player_table, struct player_index_element, 1);
      top_of_p_table = 0;
    } else if (!(player_table = (struct player_index_element *)
		 realloc(player_table, sizeof(struct player_index_element) *
			 (++top_of_p_table + 1)))) {
      perror("create entry");
      exit(1);
    }
    pos = top_of_p_table;
  } else {
    free(player_table[pos].name);
    free(player_table[pos].account);
  }
  CREATE(player_table[pos].name, char, strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; (*(player_table[pos].name + i) = LOWER(*(name + i))); i++);
  player_table[pos].account = str_dup(email);
  player_table[pos].id = -1;

  return (pos);
}


/* forget a character that no longer exists (its slot can be reused) */
static void remove_entry(const char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if (!str_cmp(player_table[i].name, (char *) name)) {
      *player_table[i].name = '\0';
      player_table[i].id = -1;
    }
}


static int index_account(const char *path, struct account_file *f, void *arg)
{
  int i, nr;

  if (!f) {
    sprintf(buf, "SYSERR: account file %s can't be read; skipped.", path);
    log(buf);
    return 0;
  }
  for (i = 0; i < f->a.num_chars; i++) {
    if (find_name(f->ch[i].name) >= 0) {
      sprintf(buf, "SYSERR: %s is in more than one account; the copy in %s "
	      "is ignored.", f->ch[i].name, path);
      log(buf);
      continue;
    }
    nr = create_entry(f->ch[i].name, f->a.email);
    player_table[nr].id = f->ch[i].char_specials_saved.idnum;
    top_idnum = MAX(top_idnum, f->ch[i].char_specials_saved.idnum);
  }
  return 0;
}


/* Build the index of characters from every account file at boot. */
void build_player_index(void)
{
  player_table = NULL;
  top_of_p_table = -1;
  account_foreach(ACCOUNT_DIR, index_account, NULL);
  sprintf(buf, "   %d players in database.", top_of_p_table + 1);
  log(buf);
}


long get_id_by_name(char *name)
{
  int i;

  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if (*player_table[i].name && !str_cmp(player_table[i].name, arg))
      return (player_table[i].id);

  return -1;
}


char *get_name_by_id(long id)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if (*player_table[i].name && player_table[i].id == id)
      return (player_table[i].name);

  return NULL;
}


/* Read an account file into acct_buf; returns 0 or -1 (and logs why). */
static int read_account_file(const char *email)
{
  char path[512];
  int unknown;

  if (account_path(path, sizeof(path), ACCOUNT_DIR, email) < 0 ||
      account_read(path, &acct_buf, &unknown) < 0)
    return -1;
  if (unknown) {
    sprintf(buf, "Account file %s has %d unknown key(s); they'll be dropped at "
	    "its next save.", path, unknown);
    log(buf);
  }
  return 0;
}


static int write_account_file(void)
{
  if (account_write(ACCOUNT_DIR, &acct_buf) < 0) {
    sprintf(buf, "SYSERR: unable to write the account file for %s: %s",
	    acct_buf.a.email, strerror(errno));
    log(buf);
    return -1;
  }
  return 0;
}


/* Load a character's record; returns its index position, or -1. */
int load_char(char *name, struct char_file_u *char_element)
{
  int player_i, i;

  if (!*name || (player_i = find_name(name)) < 0)
    return (-1);

  if (read_account_file(player_table[player_i].account) < 0 ||
      (i = account_find_char(&acct_buf, name)) < 0) {
    sprintf(buf, "SYSERR: load_char: %s isn't in the account file for %s.",
	    name, player_table[player_i].account);
    log(buf);
    return (-1);
  }
  *char_element = acct_buf.ch[i];
  return (player_i);
}


/* Store a character's saved data in its account's file (st->email names
   the account).  Returns 0 or -1. */
int save_char_file(struct char_file_u *st)
{
  if (read_account_file(st->email) < 0) {
    sprintf(buf, "SYSERR: can't save %s: no account file for '%s'.",
	    st->name, st->email);
    log(buf);
    return -1;
  }
  if (account_put_char(&acct_buf, st) < 0) {
    sprintf(buf, "SYSERR: can't save %s: account %s already has %d characters.",
	    st->name, st->email, MAX_ACCOUNT_CHARS);
    log(buf);
    return -1;
  }
  return write_account_file();
}


/*
 * Save a playing character.
 *
 * NOTE: load_room should be an *RNUM* now.  It is converted to a vnum here.
 */
void save_char(struct char_data * ch, ln_int load_room)
{
  struct char_file_u st;

  if (IS_NPC(ch) || !ch->desc || GET_PFILEPOS(ch) < 0)
    return;

  memset(&st, 0, sizeof(st));
  char_to_store(ch, &st);

  strncpy(st.host, ch->desc->host, HOST_LENGTH);
  st.host[HOST_LENGTH] = '\0';
  if (ch->char_specials.email) {
    strncpy(st.email, ch->char_specials.email, MAX_TITLE_LENGTH);
    st.email[MAX_TITLE_LENGTH] = '\0';
  }

  if (!PLR_FLAGGED(ch, PLR_LOADROOM)) {
    if (load_room == NOWHERE)
      st.player_specials_saved.load_room = NOWHERE;
    else
      st.player_specials_saved.load_room = world[load_room].number;
  }

  strcpy(st.pwd, GET_PASSWD(ch));

  save_char_file(&st);
}


/* ------------------------------------------------------------------ */
/* accounts                                                            */

/* Load an account by email; returns a new copy (free() it), or NULL. */
struct account_data *account_load(const char *email)
{
  struct account_data *a;

  if (read_account_file(email) < 0)
    return NULL;
  CREATE(a, struct account_data, 1);
  *a = acct_buf.a;
  return a;
}


/* Refresh *a (its character list in particular) from its file.
   Returns 0 or -1. */
int account_reload(struct account_data *a)
{
  if (read_account_file(a->email) < 0)
    return -1;
  *a = acct_buf.a;
  return 0;
}


/* Save the account's own fields (password, last login ...), keeping the
   characters already in its file; makes the file for a new account.
   Returns 0 or -1. */
int account_save(struct account_data *a)
{
  int i;

  if (read_account_file(a->email) < 0)
    memset(&acct_buf, 0, sizeof(acct_buf));     /* a new account */
  i = acct_buf.a.num_chars;
  acct_buf.a = *a;
  acct_buf.a.num_chars = i;                     /* the file's characters */
  for (i = 0; i < acct_buf.a.num_chars; i++)
    strcpy(acct_buf.a.chars[i], acct_buf.ch[i].name);
  if (write_account_file() < 0)
    return -1;
  *a = acct_buf.a;
  return 0;
}


/* Delete a character from its account for good.  Returns 0 or -1. */
int account_delete_char(const char *email, const char *name)
{
  if (read_account_file(email) < 0 || account_drop_char(&acct_buf, name) < 0)
    return -1;
  if (write_account_file() < 0)
    return -1;
  remove_entry(name);
  return 0;
}


/* The account file as last read (by account_load, account_reload ...):
   for listing an account's characters without reading the file again. */
const struct account_file *account_last_read(void)
{
  return &acct_buf;
}
