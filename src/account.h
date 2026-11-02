/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ACCOUNT.H                                                       *
*  Usage: Player accounts: one login (email + password) and one file      *
*         holding up to five characters                                   *
\*********************************************************************** */

#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

/* needs structs.h first (struct char_file_u and the name/host sizes) */

/*
 * A person logs in with their email address and password, then picks one
 * of their characters or makes a new one.  Each account, with all of its
 * characters, is one text file,
 *
 *     lib/accounts/<first character>/<email in lower case>
 *
 * in "Key value" lines: the account itself, then one record per character
 * in the format pfile.h describes:
 *
 *     Account 1                 <- file format version
 *     Email sean@example.com
 *     Password $pbkdf2-sha256$...
 *     Created 1793621381
 *     LastLogon 1793621401
 *     LastHost localhost
 *     BadPasswords 0
 *     End
 *     Character 1
 *     Name Kalinda
 *     ...
 *     End
 *     Character 1
 *     Name Thorne
 *     ...
 *     End
 *
 * A character's email (its "Email" key, and char_file_u.email) is always
 * the account's.  Characters have no passwords of their own.
 *
 * Only the standard C library is used, so the utilities can link
 * account.o (with pfile.o) too.
 */

#define ACCOUNT_DIR         "accounts"  /* relative to the lib directory */
#define ACCOUNT_VERSION     1
#define MAX_ACCOUNT_CHARS   5           /* characters per account        */
#define MAX_EMAIL_LENGTH    MAX_TITLE_LENGTH   /* = char_file_u.email    */

/* The account itself, plus the names of its characters (in file order). */
struct account_data {
  char   email[MAX_EMAIL_LENGTH + 1];
  char   pwd[PWD_HASH_LENGTH + 1];         /* password hash (password.h) */
  time_t created;
  time_t last_logon;
  char   last_host[HOST_LENGTH + 1];
  int    bad_pws;                          /* failures since last login  */
  int    num_chars;
  char   chars[MAX_ACCOUNT_CHARS][MAX_NAME_LENGTH + 1];
};

/* A whole account file: the account and its character records
   (a.num_chars of them).  About 31 kB: allocate it, or keep one static. */
struct account_file {
  struct account_data a;
  struct char_file_u  ch[MAX_ACCOUNT_CHARS];
};

/* Copy an email address into dst in canonical form (lower case, no
   surrounding blanks).  Returns 0 if it looks like an address that is
   also safe as a file name, -1 if not. */
int account_email(char *dst, size_t size, const char *src);

/* Build "<dir>/<first character>/<email>".  Returns 0 or -1. */
int account_path(char *buf, size_t size, const char *dir, const char *email);

/* Read an account file into *f.  Returns 0, or -1 if it can't be opened or
   isn't an account file.  Unknown keys are counted in *unknown (may be
   NULL); characters past MAX_ACCOUNT_CHARS are dropped and counted too. */
int account_read(const char *path, struct account_file *f, int *unknown);

/* Write *f through "<path>.new" and a rename, creating the letter folder
   (and dir) if needed.  Returns 0 or -1. */
int account_write(const char *dir, const struct account_file *f);

/* Call fn for every account file under dir, with its path and contents (or
   f == NULL if it can't be read).  Skips "<email>.new" leftovers.  Stops
   early if fn returns nonzero and returns that (0 when all were visited). */
int account_foreach(const char *dir,
		    int (*fn)(const char *path, struct account_file *f, void *arg),
		    void *arg);

/* The index of a character in f by name (case-insensitive), or -1. */
int account_find_char(const struct account_file *f, const char *name);

/* Store a character's record in f, replacing one of the same name or
   adding it.  Returns 0, or -1 if f already holds MAX_ACCOUNT_CHARS. */
int account_put_char(struct account_file *f, const struct char_file_u *st);

/* Remove a character's record by name.  Returns 0, or -1 if absent. */
int account_drop_char(struct account_file *f, const char *name);

#endif /* __ACCOUNT_H__ */
