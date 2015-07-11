/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  DB.H                                         Based on CircleMUD *
*  Usage: Header: Database Handling                                       *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define OLD_HELP 1               /* use old style helps */

#ifndef __DB_H__
#define __DB_H__

/* arbitrary constants used by index_boot() (must be unique) */
#define DB_BOOT_WLD     0
#define DB_BOOT_MOB     1
#define DB_BOOT_OBJ     2
#define DB_BOOT_ZON     3
#define DB_BOOT_SHP     4
#define DB_BOOT_HLP     5
#define DB_BOOT_WIZHLP  6

/* names of various files and directories */
#define INDEX_FILE      "index"         /* index of world files         */
#define MINDEX_FILE     "index.mini"    /* ... and for mini-mud-mode    */
#define WLD_PREFIX      "world/wld"     /* room definitions             */
#define MOB_PREFIX      "world/mob"     /* monster prototypes           */
#define OBJ_PREFIX      "world/obj"     /* object prototypes            */
#define ZON_PREFIX      "world/zon"     /* zon defs & command tables    */
#define SHP_PREFIX      "world/shp"     /* shop definitions             */
#define HLP_PREFIX      "text/help"     /* for HELP <keyword>           */
#define MOB_DIR         "prog"          /* mobprogs                     */

#define CREDITS_FILE    "text/credits"  /* for the 'credits' command    */
#define NEWS_FILE       "text/news"     /* for the 'news' command       */
#define MOTD_FILE       "text/motd"     /* messages of the day / mortal */
#define IMOTD_FILE      "text/imotd"    /* messages of the day / immort */
#define NEWBM_FILE      "text/newbinfo" /* message for new players      */
#define HELP_PAGE_FILE  "text/help/screen"/* for HELP <CR>              */
#define HELP_KWRD_FILE  "text/help/help.hlp" /* for old HELP style      */
#define WIZHELP_KWRD_FILE "text/help/wizhelp.hlp"
#define INFO_FILE       "text/info"     /* for INFO                     */
#define WIZLIST_FILE    "text/wizlist"  /* for WIZLIST                  */
#define IMMLIST_FILE    "text/immlist"  /* for IMMLIST                  */
#define BACKGROUND_FILE "text/background" /* for the background story   */
#define POLICIES_FILE   "text/policies" /* player policies/rules        */
#define HANDBOOK_FILE   "text/handbook" /* handbook for new immorts     */
#define WIZHELP_FILE    "text/wizhelp"  /* for WizHelp <keyword>        */

#define SYSLOG_FILE     "../syslog"     /* For vsyslog cmd */
#define IDEA_FILE       "misc/ideas"    /* for the 'idea'-command       */
#define TYPO_FILE       "misc/typos"    /*         'typo'               */
#define BUG_FILE        "misc/bugs"     /*         'bug'                */
#define TODO_FILE       "misc/todo"     /*         'todo'               */
#define MESS_FILE       "misc/messages" /* damage messages              */
#define SOCMESS_FILE    "misc/socials"  /* messgs for social acts       */
#define XNAME_FILE      "misc/xnames"   /* invalid name substrings      */

#define PLAYER_FILE     "etc/players"   /* the player database          */
#define MAIL_FILE       "etc/plrmail"   /* for the mudmail system       */
#define BAN_FILE        "etc/badsites"  /* for the siteban system       */
#define HCONTROL_FILE   "etc/hcontrol"  /* for the house system         */
#define GUILD_FILE      "etc/guilds"    /* for the guild database       */

/* public procedures in db.c */
void    boot_db(void);
int     create_entry(char *name);
void    zone_update(void);
long    real_room(long virt);
char    *fread_string(FILE *fl, const char *error);
long    get_id_by_name(char *name);
char    *get_name_by_id(long id);

void    char_to_store(struct char_data *ch, struct char_file_u *st);
void    store_to_char(struct char_file_u *st, struct char_data *ch);
int     load_char(char *name, struct char_file_u *char_element);
void    save_char(struct char_data *ch, long load_room);
void    init_char(struct char_data *ch);
struct  char_data* create_char(void);
struct  char_data *read_mobile(long nr, int type);
long    real_mobile(long virt);
long    vnum_mobile(char *searchname, struct char_data *ch);
void    clear_char(struct char_data *ch);
void    reset_char(struct char_data *ch);
void    free_char(struct char_data *ch);

struct  obj_data *create_obj(void);
void    clear_object(struct obj_data *obj);
void    free_obj(struct obj_data *obj);
long    real_object(long virt);
struct  obj_data *read_object(long nr, int type);
long    vnum_object(char *searchname, struct char_data *ch);

#define REAL    0
#define VNUMBER 1

/* structure for the reset commands */
struct reset_com {
   char command;   /* current command                      */

   bool if_flag;        /* if TRUE: exe only if preceding exe'd */
   long arg1;           /*                                      */
   long arg2;           /* Arguments to the command             */
   long arg3;           /*                                      */
   int  chance;         /* percentage of repoping 100% default  */
   int  line;           /* line number this command appears on  */

   /*
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
   */
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data {
   char *name;              /* name of this zone                  */
   char *builder;           /* name of builder - TD 3/24/95       */
   char *levelrec;          /* levels recommended - TD 3/24/95    */
   long recall_loc;         /* Where recall takes you             */
   int  pkill;              /* Pkill? 1 = YES, 0 = NO             */
   int  lifespan;           /* how long between resets (minutes)  */
   int  age;                /* current age of this zone (minutes) */
   long top;                /* upper limit for rooms in this zone */
   int  min_hum,  max_hum;  /* Ranges for humidity */
   int  min_temp, max_temp; /* Ranges for temperature */
   struct weather_data weather_info; /*
				      * The infomation about the weather
				      * for this zone.
				      */
   sh_int olc_save[5];      /* 0 rooms, 1 mobs, 2 objs, 3 zone, 4 shops */
   int    reset_mode;       /* conditions for reset (see below)   */
   int    number;           /* virtual number of this zone        */
   struct reset_com *cmd;   /* command table for reset            */

   /*
	*  Reset mode:                              *
	*  0: Don't reset, and don't update age.    *
	*  1: Reset if no PC's are located in zone. *
	*  2: Just reset.                           *
   */
};



/* for queueing zones for update   */
struct reset_q_element {
   int  zone_to_reset;            /* ref to zone_data */
   struct reset_q_element *next;
};



/* structure for the update queue     */
struct reset_q_type {
   struct reset_q_element *head;
   struct reset_q_element *tail;
};



struct player_index_element {
   char *name;
   long id;
};


struct help_index_element {
   char *keyword;
#ifdef OLD_HELP
   long pos;
#elif !defined OLD_HELP
   char *entry;
   int duplicate;
#endif
};

/* don't change these */
#define BAN_NOT         0
#define BAN_NEW         1
#define BAN_SELECT      2
#define BAN_ALL         3

#define BANNED_SITE_LENGTH    50
struct ban_list_element {
   char site[BANNED_SITE_LENGTH+1];
   int  type;
   time_t date;
   char name[MAX_NAME_LENGTH+1];
   struct ban_list_element *next;
};


/* global buffering system */

#ifdef __DB_C__
char    buf[MAX_STRING_LENGTH];
char    buf1[MAX_STRING_LENGTH];
char    buf2[MAX_STRING_LENGTH];
char    arg[MAX_STRING_LENGTH];
char    bigbuf[(MAX_STRING_LENGTH>>1)];
char    specbuf[(MAX_STRING_LENGTH>>1)];
#else
extern struct zone_data *zone_table;
extern char     buf[MAX_STRING_LENGTH];
extern char     buf1[MAX_STRING_LENGTH];
extern char     buf2[MAX_STRING_LENGTH];
extern char     arg[MAX_STRING_LENGTH];
extern char     bigbuf[(MAX_STRING_LENGTH>>1)];
extern char     specbuf[(MAX_STRING_LENGTH>>1)];
#endif

#ifndef __CONFIG_C__
extern char     *OK;
extern char     *NOPERSON;
extern char     *NOEFFECT;
#endif

#endif
