/* ***********************************************************************\
*    _____ _            ____                  _       _                   *
*   |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __        *
*     | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \       *
*     | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |      *
*     |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|      *
*                                                                         *
*  File:  CREATION.H                                                      *
*  Usage: Header file for On-Line-Creation Utilities                      *
*  Programmer(s): Ideas gleaned from the Creators of SillyMUD             *
*                 and the creator of redit and iedit for CircleMUD        *
*                 All code by Sean Mountcastle (Glasgian)                 *
*                 and Harvey Gilpin (Copyright 1996)                      *
\*********************************************************************** */

#ifndef __OLC_H__
#define __OLC_H__

#include "shop.h"

/*. Macros, defines, structs and globals for the OLC suite .*/

/*. CONFIG DEFINES - these should be in structs.h, but here is easyer.*/

#define NUM_ATTACK_TYPES        17

#define NUM_GENDERS             3
#define NUM_SHOP_FLAGS          2
#define NUM_TRADERS             7

/*. Utils exported from olc.c .*/
void strip_string(char *);
void cleanup_olc(struct descriptor_data *d, byte cleanup_type);
void get_char_cols(struct char_data *ch);
void olc_add_to_save_list(int zone, char type);
void olc_remove_from_save_list(int zone, char type);


/*. OLC structs .*/

struct olc_data {
  int    mode;
  int    zone_num;
  int    value;
  long   number;
  struct char_data  *mob;
  struct room_data  *room;
  struct obj_data   *obj;
  struct zone_data  *zone;
  struct shop_data  *shop;
  struct guild_type *gptr;
  struct extra_descr_data *desc;
};

struct olc_save_info {
  int zone;
  char type;
  struct olc_save_info *next;
};


/*. Exported globals .*/
#ifdef __OLC_C__
char *nrm, *grn, *cyn, *yel;
struct olc_save_info *olc_save_list = NULL;
#endif
#if 0
#else
extern char *nrm, *grn, *cyn, *yel;
extern struct olc_save_info *olc_save_list;
#endif


/*. Descriptor access macros .*/
#define OLC_MODE(d)     ((d)->olc->mode)        /*. Parse input mode    .*/
#define OLC_NUM(d)      ((d)->olc->number)      /*. Room/Obj VNUM       .*/
#define OLC_VAL(d)      ((d)->olc->value)       /*. Scratch variable    .*/
#define OLC_ZNUM(d)     ((d)->olc->zone_num)    /*. Real zone number    .*/
#define OLC_ROOM(d)     ((d)->olc->room)        /*. Room structure      .*/
#define OLC_OBJ(d)      ((d)->olc->obj)         /*. Object structure    .*/
#define OLC_ZONE(d)     ((d)->olc->zone)        /*. Zone structure      .*/
#define OLC_MOB(d)      ((d)->olc->mob)         /*. Mob structure       .*/
#define OLC_SHOP(d)     ((d)->olc->shop)        /*. Shop structure      .*/
#define OLC_DESC(d)     ((d)->olc->desc)        /*. Extra description   .*/
#define OLC_GUILD(d)    ((d)->olc->gptr)        /*. Guild Struct        .*/

/*. Other macros .*/

#define OLC_EXIT(d)     (OLC_ROOM(d)->dir_option[OLC_VAL(d)])
#define GET_OLC_ZONE(c) ((c)->player_specials->saved.olc_zone)

/*. Cleanup types .*/
#define CLEANUP_ALL                     1       /*. Free the whole lot  .*/
#define CLEANUP_STRUCTS                 2       /*. Don't free strings  .*/

/*. Add/Remove save list types  .*/
#define OLC_SAVE_ROOM                   0
#define OLC_SAVE_OBJ                    1
#define OLC_SAVE_ZONE                   2
#define OLC_SAVE_MOB                    3
#define OLC_SAVE_SHOP                   4

/* Submodes of OEDIT connectedness */
#define OEDIT_MAIN_MENU                 1
#define OEDIT_EDIT_NAMELIST             2
#define OEDIT_SHORTDESC                 3
#define OEDIT_LONGDESC                  4
#define OEDIT_ACTDESC                   5
#define OEDIT_TYPE                      6
#define OEDIT_EXTRAS                    7
#define OEDIT_WEAR                      8
#define OEDIT_WEIGHT                    9
#define OEDIT_COST                      10
#define OEDIT_COSTPERDAY                11
#define OEDIT_TIMER                     12
#define OEDIT_VALUE_1                   13
#define OEDIT_VALUE_2                   14
#define OEDIT_VALUE_3                   15
#define OEDIT_VALUE_4                   16
#define OEDIT_APPLY                     17
#define OEDIT_APPLYMOD                  18
#define OEDIT_EXTRADESC_KEY             19
#define OEDIT_CONFIRM_SAVEDB            20
#define OEDIT_CONFIRM_SAVESTRING        21
#define OEDIT_PROMPT_APPLY              22
#define OEDIT_EXTRADESC_DESCRIPTION     23
#define OEDIT_EXTRADESC_MENU            24
#define OEDIT_MATERIAL                  25
#define OEDIT_SIZE                      26


/* Submodes of REDIT connectedness */
#define REDIT_MAIN_MENU                 1
#define REDIT_NAME                      2
#define REDIT_DESC                      3
#define REDIT_FLAGS                     4
#define REDIT_SECTOR                    5
#define REDIT_EXIT_MENU                 6
#define REDIT_CONFIRM_SAVEDB            7
#define REDIT_CONFIRM_SAVESTRING        8
#define REDIT_EXIT_NUMBER               9
#define REDIT_EXIT_DESCRIPTION          10
#define REDIT_EXIT_KEYWORD              11
#define REDIT_EXIT_KEY                  12
#define REDIT_EXIT_DOORFLAGS            13
#define REDIT_EXTRADESC_MENU            14
#define REDIT_EXTRADESC_KEY             15
#define REDIT_EXTRADESC_DESCRIPTION     16
#define REDIT_TELE_STUFF		17
#define REDIT_TELE_DEST			18
#define REDIT_TELE_TIME			19
#define REDIT_TELE_MASK			20

/*. Submodes of ZEDIT connectedness     .*/
#define ZEDIT_MAIN_MENU                 0
#define ZEDIT_DELETE_ENTRY              1
#define ZEDIT_NEW_ENTRY                 2
#define ZEDIT_CHANGE_ENTRY              3
#define ZEDIT_COMMAND_TYPE              4
#define ZEDIT_IF_FLAG                   5
#define ZEDIT_ARG1                      6
#define ZEDIT_ARG2                      7
#define ZEDIT_ARG3                      8
#define ZEDIT_ARG4			9
#define ZEDIT_ZONE_NAME                 10
#define ZEDIT_ZONE_LIFE                 11
#define ZEDIT_ZONE_TOP                  12
#define ZEDIT_ZONE_RESET                13
#define ZEDIT_CONFIRM_SAVESTRING        14
#define ZEDIT_ZONE_BUILDER              15
#define ZEDIT_ZONE_LEVELREC             16
#define ZEDIT_ZONE_PKILL                17
#define ZEDIT_ZONE_RECALL               18


/*. Submodes of MEDIT connectedness     .*/
#define MEDIT_MAIN_MENU                 0
#define MEDIT_ALIAS                     1
#define MEDIT_S_DESC                    2
#define MEDIT_L_DESC                    3
#define MEDIT_D_DESC                    4
#define MEDIT_NPC_FLAGS                 5
#define MEDIT_AFF_FLAGS                 6
#define MEDIT_CONFIRM_SAVESTRING        7
/*. Numerical responses .*/
#define MEDIT_NUMERICAL_RESPONSE        10
#define MEDIT_SEX                       11
#define MEDIT_HITROLL                   12
#define MEDIT_DAMROLL                   13
#define MEDIT_NDD                       14
#define MEDIT_SDD                       15
#define MEDIT_NUM_HP_DICE               16
#define MEDIT_SIZE_HP_DICE              17
#define MEDIT_ADD_HP                    18
#define MEDIT_AC                        19
#define MEDIT_EXP                       20
#define MEDIT_GOLD                      21
#define MEDIT_POS                       22
#define MEDIT_DEFAULT_POS               23
#define MEDIT_ATTACK                    24
#define MEDIT_LEVEL                     25
#define MEDIT_ALIGNMENT                 26
#define MEDIT_TYPE                      27
#define MEDIT_RACE                      28
#define MEDIT_ATTRIBUTES                29
#define MEDIT_STR                       30
#define MEDIT_INT                       31
#define MEDIT_WIS                       32
#define MEDIT_DEX                       33
#define MEDIT_CON                       34
#define MEDIT_CHA                       35
#define MEDIT_WIL                       36
#define MEDIT_HEIGHT                    37
#define MEDIT_WEIGHT                    38

/*. Submodes of SEDIT connectedness     .*/
#define SEDIT_MAIN_MENU                 0
#define SEDIT_CONFIRM_SAVESTRING        1
#define SEDIT_NOITEM1                   2
#define SEDIT_NOITEM2                   3
#define SEDIT_NOCASH1                   4
#define SEDIT_NOCASH2                   5
#define SEDIT_NOBUY                     6
#define SEDIT_BUY                       7
#define SEDIT_SELL                      8
#define SEDIT_PRODUCTS_MENU             11
#define SEDIT_ROOMS_MENU                12
#define SEDIT_NAMELIST_MENU             13
#define SEDIT_NAMELIST                  14
/*. Numerical responses .*/
#define SEDIT_NUMERICAL_RESPONSE        20
#define SEDIT_OPEN1                     21
#define SEDIT_OPEN2                     22
#define SEDIT_CLOSE1                    23
#define SEDIT_CLOSE2                    24
#define SEDIT_KEEPER                    25
#define SEDIT_BUY_PROFIT                26
#define SEDIT_SELL_PROFIT               27
#define SEDIT_TYPE_MENU                 29
#define SEDIT_DELETE_TYPE               30
#define SEDIT_DELETE_PRODUCT            31
#define SEDIT_NEW_PRODUCT               32
#define SEDIT_DELETE_ROOM               33
#define SEDIT_NEW_ROOM                  34
#define SEDIT_SHOP_FLAGS                35
#define SEDIT_NOTRADE                   36

/*. Guild EditModes .*/
#define GEDIT_MAIN_MENU                 1
#define GEDIT_CONFIRM_SAVE              2
#define GEDIT_NAME                      3
#define GEDIT_LEADERSNAME               4
#define GEDIT_XLVLNAME                  5
#define GEDIT_HLVLNAME                  6
#define GEDIT_MLVLNAME                  7
#define GEDIT_BLVLNAME                  8
#define GEDIT_MBR_LOOK_STR              9
#define GEDIT_ENTR_ROOM                 10
#define GEDIT_GUARD                     11
#define GEDIT_DIRECTION                 12
#define GEDIT_RESTRICTS                 13
#define GEDIT_DELETE_GUILD              14

/*. Limit info .*/
#define MAX_ROOM_NAME   75
#define MAX_MOB_NAME    75
#define MAX_OBJ_NAME    75
#define MAX_ROOM_DESC   1024
#define MAX_EXIT_DESC   256
#define MAX_EXTRA_DESC  4096
#define MAX_MOB_DESC    1024
#define MAX_OBJ_DESC    1024

extern char *trap_triggers[];
extern char *trap_dam_type[];

#endif
