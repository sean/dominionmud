/* ************************************************************************
*  file:  mudpasswd.c                                 Part of CircleMud   *
*  Usage: changing the password in a player file (lib/players)           *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
************************************************************************* */


#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"
#include "../password.h"
#include "../pfile.h"


void pword(char *dir, char *name, char *password)
{
  struct char_file_u st;
  char path[512];

  if (pfile_path(path, sizeof(path), dir, name) < 0) {
    fprintf(stderr, "'%s' isn't a valid character name.\n", name);
    exit(1);
  }
  if (pfile_read(path, &st, NULL) < 0) {
    fprintf(stderr, "%s: can't read %s (no such character?)\n", name, path);
    exit(1);
  }
  if (password_hash(password, st.pwd, sizeof(st.pwd))) {
    fprintf(stderr, "Unable to hash the new password.\n");
    exit(1);
  }
  if (pfile_write(path, &st) < 0) {
    perror(path);
    exit(1);
  }
  printf("%s's password has been changed.\n", st.name);
}


int main(int argc, char **argv)
{
  if (argc != 4)
    fprintf(stderr, "Usage: %s players-dir character-name new-password\n"
	    "  e.g. %s lib/players Glasgian 'new secret'\n", argv[0], argv[0]);
  else
    pword(argv[1], argv[2], argv[3]);

  return (0);
}
