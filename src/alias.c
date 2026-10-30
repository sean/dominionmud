/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ALIAS.C                                      Based on CircleMUD *
*  Usage: Writing/Reading Player's Aliases from disk                      *
*  Programmer(s): Original work by Jeremy Hess and Chad Thompson          *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#define __ALIAS_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

void free_alias(struct alias *a);   /* In interpre.c */

void write_aliases(struct char_data *ch)
{
  FILE *file;
  char fn[127],*buf;
  struct alias *temp;
  int length;

  get_filename(GET_NAME(ch),fn,ALIAS_FILE);
  unlink(fn);
  if( !GET_ALIASES(ch) )
    return;

  file = fopen(fn,"wt");

  temp = GET_ALIASES(ch);
  
  while( temp )
  {
    length = strlen(temp->alias);
    fprintf(file,"%d\n",length); 
    fprintf(file,"%s\n",temp->alias);
    buf = temp->replacement; 
    while( *++buf == ' ' );
    length = strlen(buf);
    fprintf(file,"%d\n",length); 
    fprintf(file,"%s\n",buf);
    fprintf(file,"%d\n",temp->type);
    temp = temp->next;
  }
  
  fclose(file);
}

void read_aliases(struct char_data *ch)
{
  FILE *file;
  char fn[127];
  struct alias *t2, *a;
  int length, type;
  char temp_buf[127], buf[127];

  get_filename(GET_NAME(ch), fn, ALIAS_FILE);

  file = fopen(fn, "r");

  if (!file)
    return;

  CREATE(GET_ALIASES(ch), struct alias, 1);
  t2 = GET_ALIASES(ch);
  do
  {
    /* alias text, replacement text, then type; lengths come from the file
       so they must be checked against our buffers */
    if (fscanf(file, "%d\n", &length) != 1 ||
        length < 0 || length >= (int) sizeof(buf) ||
        !fgets(buf, length + 1, file))
      break;
    t2->alias = str_dup(buf);
    if (fscanf(file, "%d\n", &length) != 1 ||
        length < 0 || length >= (int) sizeof(temp_buf) - 1 ||
        !fgets(buf, length + 1, file))
      break;
    strcpy(temp_buf, " ");
    strcat(temp_buf, buf);
    t2->replacement = str_dup(temp_buf);
    if (fscanf(file, "%d\n", &type) != 1)
      break;
    t2->type = type;
    if (!feof(file)) {
      CREATE(t2->next, struct alias, 1);
      t2 = t2->next;
    }
  } while (!feof(file));

  if (!t2->alias || !t2->replacement || ferror(file)) {
    /* unreadable or truncated alias file: drop them all rather than keep
       a half-built list */
    plog("SYSERR: alias file for %s is corrupt; aliases not loaded.",
         GET_NAME(ch));
    while ((a = GET_ALIASES(ch)) != NULL) {
      GET_ALIASES(ch) = a->next;
      free_alias(a);
    }
  }

  fclose(file);
}
 

