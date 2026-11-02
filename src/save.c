/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SAVE.C                                       Based on CircleMUD *
*  Usage: Loading and saving characters, one text file each (pfile.c),    *
*         and the in-memory index of players                              *
*                                                                         *
*  Dominion Mud copyright (c) 1994, 1995, 1996, 1997, 2000, 2001 by       *
*  Sean Mountcastle, John Olvey and Kevin Huff                            *
\*********************************************************************** */

#define __SAVE_C__

#include "conf.h"
#include "sysdep.h"

#include <dirent.h>

#include "protos.h"
#include "pfile.h"

extern struct room_data *world;
extern struct player_index_element *player_table;
extern int    top_of_p_table;
extern long   top_idnum;


/* add a name (and id) to the in-memory index; returns its position */
int create_entry(char *name)
{
  int i;

  if (top_of_p_table == -1) {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  } else if (!(player_table = (struct player_index_element *)
	       realloc(player_table, sizeof(struct player_index_element) *
		       (++top_of_p_table + 1)))) {
    perror("create entry");
    exit(1);
  }
  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
       i++);
  player_table[top_of_p_table].id = -1;

  return (top_of_p_table);
}


/*
 * Build the index from the files in lib/players, creating the folders if
 * they're missing.  A file that can't be read, or whose Name doesn't match
 * its file name, is reported and left out.
 */
void build_player_index(void)
{
  struct char_file_u st;
  struct dirent *de;
  char dir[64], path[256], expect[256];
  DIR *dp;
  int c, nr, unknown;

  player_table = NULL;
  top_of_p_table = -1;

  if (pfile_make_dirs(PFILE_DIR) < 0) {
    sprintf(buf, "SYSERR: can't create the player folders under %s", PFILE_DIR);
    log(buf);
    exit(1);
  }

  for (c = 'a'; c <= 'z'; c++) {
    sprintf(dir, "%s/%c", PFILE_DIR, c);
    if (!(dp = opendir(dir)))
      continue;
    while ((de = readdir(dp))) {
      if (*de->d_name == '.' || strchr(de->d_name, '.'))
	continue;                       /* ".", "..", and "<name>.new" */
      if ((size_t) snprintf(path, sizeof(path), "%s/%s", dir, de->d_name)
	  >= sizeof(path))
	continue;
      if (pfile_read(path, &st, &unknown) < 0) {
	sprintf(buf, "SYSERR: player file %s can't be read; skipped.", path);
	log(buf);
	continue;
      }
      if (pfile_path(expect, sizeof(expect), PFILE_DIR, st.name) < 0 ||
	  strcmp(expect, path)) {
	sprintf(buf, "SYSERR: player file %s holds '%s'; skipped.", path, st.name);
	log(buf);
	continue;
      }
      if (unknown) {
	sprintf(buf, "Player file %s has %d unknown key(s); they'll be dropped "
		"at its next save.", path, unknown);
	log(buf);
      }
      nr = create_entry(st.name);
      player_table[nr].id = st.char_specials_saved.idnum;
      top_idnum = MAX(top_idnum, st.char_specials_saved.idnum);
    }
    closedir(dp);
  }

  sprintf(buf, "   %d players in database.", top_of_p_table + 1);
  log(buf);
}


long get_id_by_name(char *name)
{
  int i;

  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if (!str_cmp(player_table[i].name, arg))
      return (player_table[i].id);

  return -1;
}


char *get_name_by_id(long id)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if (player_table[i].id == id)
      return (player_table[i].name);

  return NULL;
}


/* Load a character's file; returns its index position, or -1. */
int load_char(char *name, struct char_file_u *char_element)
{
  int player_i, unknown;
  char path[256];

  int find_name(char *name);

  if ((player_i = find_name(name)) < 0)
    return (-1);

  if (pfile_path(path, sizeof(path), PFILE_DIR, player_table[player_i].name) < 0 ||
      pfile_read(path, char_element, &unknown) < 0) {
    sprintf(buf, "SYSERR: load_char: unable to read the player file for %s.", name);
    log(buf);
    return (-1);
  }
  return (player_i);
}


/* Write a character's saved data to its file.  Returns 0 or -1. */
int save_char_file(struct char_file_u *st)
{
  char path[256];

  if (pfile_path(path, sizeof(path), PFILE_DIR, st->name) < 0 ||
      pfile_write(path, st) < 0) {
    sprintf(buf, "SYSERR: unable to write the player file for %s: %s",
	    st->name, strerror(errno));
    log(buf);
    return -1;
  }
  return 0;
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
