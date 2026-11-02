/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SAVE.H                                       Based on CircleMUD *
*  Usage: Loading and saving accounts and their characters               *
*                                                                         *
*  Dominion Mud copyright (c) 1994, 1995, 1996, 1997, 2000, 2001 by       *
*  Sean Mountcastle, John Olvey and Kevin Huff                            *
\*********************************************************************** */

#ifndef __SAVE_H__
#define __SAVE_H__

struct char_file_u;
struct account_data;
struct account_file;

void build_player_index(void);
int  create_entry(char *name, char *email);
long get_id_by_name(char *name);
char *get_name_by_id(long id);
int  load_char(char *name, struct char_file_u *char_element);
int  save_char_file(struct char_file_u *st);            /* 0 or -1 */
int  find_name(char *name);

struct account_data *account_load(const char *email);   /* free() it */
int  account_reload(struct account_data *a);            /* 0 or -1 */
int  account_save(struct account_data *a);              /* 0 or -1 */
int  account_delete_char(const char *email, const char *name);
const struct account_file *account_last_read(void);

#endif /* __SAVE_H__ */
