/* ************************************************************************
*  file: purgeplay.c                                    Part of CircleMUD *
*  Usage: purge useless characters from lib/players                       *
*  All Rights Reserved                                                    *
*  Copyright (C) 1992, 1993 The Trustees of The Johns Hopkins University  *
************************************************************************* */

/*
 * Lists the characters that would be purged (invalid, never played, idle
 * too long for their level, or flagged deleted), and with -d removes their
 * player files.  Run it with the server stopped; their rent files in
 * lib/plrobjs are cleaned up by the game at its next boot.
 */

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"
#include "../pfile.h"

static int do_delete = 0, num = 0;


static int check(const char *path, struct char_file_u *player, void *arg)
{
  int okay = 1;
  long timeout;
  char *ptr, reason[80];

  *reason = '\0';

  if (!player) {
    printf("%4d. %-20s can't be read; left alone\n", ++num, path);
    return 0;
  }
  for (ptr = player->name; *ptr; ptr++)
    if (!isalpha((int)*ptr)) {
      okay = 0;
      strcpy(reason, "Invalid name");
    }
  if (player->level == 0) {
    okay = 0;
    strcpy(reason, "Never entered game");
  }
  if (player->level < 0 || player->level > LVL_CREATOR) {
    okay = 0;
    strcpy(reason, "Invalid level");
  }
  /* now, check for timeouts.  They only apply if the char is not
     cryo-rented.   Immortals don't time out.  */
  if (okay && player->level <= LVL_IMMORT) {
    if (!(player->char_specials_saved.act & PLR_CRYO)) {
      if (player->level == 1)		timeout = 4;	/* Lev   1 : 4 days */
      else if (player->level <= 4)	timeout = 7;	/* Lev 2-4 : 7 days */
      else if (player->level <= 10)	timeout = 30;	/* Lev 5-10: 30 days */
      else if (player->level <= LVL_IMMORT - 1)
	timeout = 60;					/* Lev 11-30: 60 days */
      else
	timeout = 90;					/* immortal: 90 days */
    } else
      timeout = 120;

    timeout *= SECS_PER_REAL_DAY;

    if ((time(0) - player->last_logon) > timeout) {
      okay = 0;
      sprintf(reason, "Level %2d idle for %3ld days", player->level,
	      (long) ((time(0) - player->last_logon) / SECS_PER_REAL_DAY));
    }
  }
  if (player->char_specials_saved.act & PLR_DELETED) {
    okay = 0;
    strcpy(reason, "Deleted flag set");
  }

  /* Don't delete for *any* of the above reasons if they have NODELETE */
  if (!okay && (player->char_specials_saved.act & PLR_NODELETE)) {
    okay = 2;
    strcat(reason, "; NOT deleted.");
  }
  if (player->level >= LVL_IMPL)
    okay = 1;

  if (okay != 1) {
    printf("%4d. %-20s %s\n", ++num, player->name, reason);
    if (!okay && do_delete && remove(path) != 0)
      perror(path);
  }
  return 0;
}


int main(int argc, char *argv[])
{
  if (argc == 3 && !strcmp(argv[1], "-d"))
    do_delete = 1;
  else if (argc != 2) {
    printf("Usage: %s [-d] players-dir\n"
	   "  Lists the characters to purge; with -d, deletes their files.\n",
	   argv[0]);
    return 1;
  }
  printf(do_delete ? "Deleting:\n" : "Would delete (run with -d to do it):\n");
  pfile_foreach(argv[argc - 1], check, NULL);
  puts("Done.");
  return 0;
}
