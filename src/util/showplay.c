/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SHOWPLAY.C                                   Based on CircleMUD *
*  Usage: Listing the characters in lib/accounts outside of the game     *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#include "../conf.h"
#include "../sysdep.h"
#include "../structs.h"
#include "../utils.h"
#include "../pfile.h"
#include "../account.h"

static int show(const char *path, struct account_file *f, void *arg)
{
  struct char_file_u *st;
  char when[32];
  int i;

  if (!f) {
    printf("%-60s (can't be read)\n", path);
    return 0;
  }
  if (!f->a.num_chars)
    printf("%5s  %-20s %3s  %-16s  %-30s\n", "-", "(no characters)", "", "",
	   f->a.email);
  for (i = 0; i < f->a.num_chars; i++) {
    st = &f->ch[i];
    strftime(when, sizeof(when), "%Y-%m-%d %H:%M", localtime(&st->last_logon));
    printf("%5ld  %-20s %3d  %-16s  %-30s %s\n", st->char_specials_saved.idnum,
	   st->name, st->level, when, f->a.email, st->host);
  }
  return 0;
}


int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "Usage: %s accounts-dir\n", argv[0]);
    return 1;
  }
  printf("%5s  %-20s %3s  %-16s  %-30s %s\n", "Id", "Name", "Lvl",
	 "Last logon", "Account", "Host");
  account_foreach(argv[1], show, NULL);
  return 0;
}
