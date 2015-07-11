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
  struct alias *t2;
  int length;
  char temp_buf[127],buf[127];

  get_filename(GET_NAME(ch),fn,ALIAS_FILE);

  file = fopen(fn,"r");

  if( !file )
    return;
 
  CREATE(GET_ALIASES(ch),struct alias,1);
  t2 = GET_ALIASES(ch); 
  do 
  { 
    fscanf(file,"%d\n",&length);
    fgets(buf,length+1,file);
    t2->alias = str_dup(buf);
    fscanf(file,"%d\n",&length);
    fgets(buf,length+1,file);
    strcpy(temp_buf," ");
    strcat(temp_buf,buf); 
    t2->replacement = str_dup(temp_buf); 
    fscanf(file,"%d\n",&length);
    t2->type = length; 
    if( !feof(file) ){
      CREATE(t2->next,struct alias,1);
      t2 = t2->next;
    } 
  } while( !feof(file) ); 
  
  fclose(file);
} 

