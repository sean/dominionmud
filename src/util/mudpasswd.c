/* ************************************************************************
*  file:  mudpasswd.c                                 Part of CircleMud   *
*  Usage: changing an account's password (lib/accounts)                  *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
************************************************************************* */


#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"
#include "../password.h"
#include "../pfile.h"
#include "../account.h"


void pword(char *dir, char *email, char *password)
{
  static struct account_file acct;
  char path[512];

  if (account_path(path, sizeof(path), dir, email) < 0) {
    fprintf(stderr, "'%s' isn't a valid email address.\n", email);
    exit(1);
  }
  if (account_read(path, &acct, NULL) < 0) {
    fprintf(stderr, "%s: can't read %s (no such account?)\n", email, path);
    exit(1);
  }
  if (password_hash(password, acct.a.pwd, sizeof(acct.a.pwd))) {
    fprintf(stderr, "Unable to hash the new password.\n");
    exit(1);
  }
  acct.a.bad_pws = 0;
  if (account_write(dir, &acct) < 0) {
    perror(path);
    exit(1);
  }
  printf("The password for %s has been changed.\n", acct.a.email);
}


int main(int argc, char **argv)
{
  if (argc != 4)
    fprintf(stderr, "Usage: %s accounts-dir email new-password\n"
	    "  e.g. %s lib/accounts sean@example.com 'new secret'\n", argv[0], argv[0]);
  else
    pword(argv[1], argv[2], argv[3]);

  return (0);
}
