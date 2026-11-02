/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  PFILE.H                                                         *
*  Usage: Plain-text player files, one per character                      *
\*********************************************************************** */

#ifndef __PFILE_H__
#define __PFILE_H__

#include <stddef.h>

/*
 * Each character is saved in its own text file,
 *
 *     lib/players/<first letter>/<name in lower case>
 *
 * one "Key value" pair per line, starting with "Version <n>".  Strings are
 * written on one line with "\n" and "\\" escapes, numbers in decimal and
 * arrays as space-separated lists.  Readers skip keys they don't know and
 * leave missing fields zero, so fields can be added without converting
 * existing files.  The format doesn't depend on the compiler, CPU or the
 * layout of struct char_file_u, which is now only an in-memory staging
 * area between the files and struct char_data.
 *
 * Only the standard C library is used, so the utilities in util/ can link
 * pfile.o as well as the game.
 */

#define PFILE_DIR       "players"   /* relative to the lib directory    */
#define PFILE_VERSION   1

struct char_file_u;

/* Build "<dir>/<letter>/<name>" for a name.  Returns 0, or -1 if the name
   isn't 1-MAX_NAME_LENGTH letters (so it can't name any other path). */
int pfile_path(char *buf, size_t size, const char *dir, const char *name);

/* Read a player file into *st (zeroed first).  Returns 0, or -1 if the
   file can't be opened or doesn't start with a Version line.  Unknown keys
   are reported through *unknown (may be NULL), counting them. */
int pfile_read(const char *path, struct char_file_u *st, int *unknown);

/* Write *st to path, through "<path>.new" and a rename, so a crash leaves
   the old file whole.  Returns 0 or -1. */
int pfile_write(const char *path, const struct char_file_u *st);

/* Create dir and its 26 letter subdirectories if they're missing.
   Returns 0 or -1. */
int pfile_make_dirs(const char *dir);

/* Call fn for every player file under dir, in no particular order, with
   the file's path and its contents (or st == NULL if it can't be read).
   Skips "<name>.new" leftovers.  Stops early if fn returns nonzero, and
   returns that value (0 when every file was visited). */
int pfile_foreach(const char *dir,
		  int (*fn)(const char *path, struct char_file_u *st, void *arg),
		  void *arg);

#endif /* __PFILE_H__ */
