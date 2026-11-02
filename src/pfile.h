/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  PFILE.H                                                         *
*  Usage: Plain-text character records                                    *
\*********************************************************************** */

#ifndef __PFILE_H__
#define __PFILE_H__

#include <stdio.h>

/*
 * A character record, as stored inside its account's file (account.h):
 *
 *     Character 1          <- record format version
 *     Name Kalinda
 *     Id 2
 *     Level 3
 *     ...
 *     End
 *
 * one "Key value" pair per line.  Strings are written on one line with
 * "\n" and "\\" escapes, numbers in decimal and arrays as space-separated
 * lists.  Readers skip keys they don't know and leave missing fields zero,
 * so fields can be added without converting existing files.  The format
 * doesn't depend on the compiler, CPU or the layout of struct
 * char_file_u, which is only an in-memory staging area between the files
 * and struct char_data.
 *
 * Only the standard C library is used, so the utilities in util/ can link
 * pfile.o as well as the game.
 */

#define PFILE_VERSION   1

struct char_file_u;

/* Write one record, from its "Character" line to its "End" line.
   Returns 0, or -1 on a write error. */
int pfile_write_record(FILE *fp, const struct char_file_u *st);

/* Read one record into *st (zeroed first).  Call it just after reading a
   record's "Character" line; it reads up to and including "End".  Keys it
   doesn't know are counted in *unknown (may be NULL).  Returns 0, or -1 if
   the file ends before "End". */
int pfile_read_record(FILE *fp, struct char_file_u *st, int *unknown);

#endif /* __PFILE_H__ */
