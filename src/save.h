/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SAVE.C                                       Based on CircleMUD *
*  Usage: Loading/saving player objects for rent and crash-save           *
*                                                                         *
*  Dominion Mud copyright (c) 1994, 1995, 1996, 1997, 2000, 2001 by       *
*  Sean Mountcastle, John Olvey and Kevin Huff                            *
\*********************************************************************** */

#ifndef __SAVE_H__
#define __SAVE_H__

#define GET_DESC(ch) ((ch)->desc)

/*
 * Player character key data struct
 * Stuff for new error trapping of corrupt pfiles.
 */
struct  key_data
{
    char        key[20];	/* Increase if you make a key > 20 chars */
    int         string;		/* TRUE for string, FALSE for int        */
    int         deflt;		/* Default value or pointer              */
    void *      ptrs[12];	/* Increase if you have > 12 parms/line   */
};

#define MAND		3344556	/* Magic # for manditory field           */
#define SPECIFIED	3344557 /* Key was used already.                 */
#define DEFLT		3344558 /* Use default from fread_char_obj       */

bool save_char_obj( struct char_data * ch );

void tail_chain( void );

#endif /* __SAVE_H__ */
