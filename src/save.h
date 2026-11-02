/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SAVE.H                                       Based on CircleMUD *
*  Usage: Loading and saving characters, one text file each               *
*                                                                         *
*  Dominion Mud copyright (c) 1994, 1995, 1996, 1997, 2000, 2001 by       *
*  Sean Mountcastle, John Olvey and Kevin Huff                            *
\*********************************************************************** */

#ifndef __SAVE_H__
#define __SAVE_H__

struct char_file_u;

void build_player_index(void);
int  create_entry(char *name);
long get_id_by_name(char *name);
char *get_name_by_id(long id);
int  load_char(char *name, struct char_file_u *char_element);
int  save_char_file(struct char_file_u *st);   /* 0 or -1 */

#endif /* __SAVE_H__ */
