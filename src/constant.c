/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  CONSTANT.C                                   Based on CircleMUD *
*  Usage: Numeric and string constants used by The Dominion               *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __CONSTANT_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"

const char circlemud_version[] = {
"The Dominion, v1.02 based on CircleMUD, version 3.00 beta patchlevel 10\r\n"};


/* strings corresponding to ordinals/bitvectors in structs.h ***********/


/* (Note: strings for class definitions in class.c instead of here) */


/* cardinal directions */
const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "northeast",
  "northwest",
  "southeast",
  "southwest",
  "\n"
};


/* ROOM_x */
const char *room_bits[] = {
  "DARK",
  "DEATH",
  "!MOB",
  "INDOORS",
  "PEACEFUL",
  "SOUNDPROOF",
  "!TRACK",
  "!MAGIC",
  "TUNNEL",
  "PRIVATE",
  "GODROOM",
  "HOUSE",
  "HCRSH",
  "ATRIUM",
  "OLC",
  "*",                          /* BFS MARK */
  "UNUSED",
  "REGEN",
  "NOSUMMON",
  "NOHEAL",
  "BLADE_BARRIER",
  "\n"
};


/* EX_x */
const char *exit_bits[] = {
  "DOOR",
  "CLOSED",
  "LOCKED",
  "PICKPROOF",
  "SECRET",
  "CLIMB",
  "ARROWSLIT",
  "\n"
};


/* SECT_ */
const char *sector_types[] = {
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water (Swim)",
  "Water (No Swim)",
  "Underwater",
  "Air",
  "Glacier",
  "Desert",
  "Jungle",
  "\n"
};


/* SEX_x */
const char *genders[] =
{
  "Neutral",
  "Male",
  "Female"
};

const char *disorders[] = {
  "AGORAPHOBIA",
  "ALCOHOLISM",
  "ANIMOSITY",
  "CATATONIA",
  "DEMENTIA",
  "DEPRESSION",
  "SCOTOPHOBIA",
  "MANIC_DEPRESSIVE",
  "MEGALOMANIA",
  "PATHO_LIAR",
  "ABSENT_MINDED",
  "NARCISSISTIC",
  "SHAKES",
  "STAMMERS",
  "TALKS2SELF",
  "\n"
};

const char *abilities[] = {
  "REGENERATION",
  "AMBIDEXTROUS",
  "CONTORTIONIST",
  "IMM_DISEASE",
  "IMM_POISON",
  "IMM_FIRE",
  "IMM_COLD",
  "IMM_SHOCK",
  "LIGHTNING_REFLEX",
  "\n"
};


const char *restr_bits[] = {
  "UNUSED",
  "\n"
};


/* POS_x */
const char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "Mounted",
  "Meditating",
  "\n"
};


/* PLR_x */
const char *player_bits[] = {
  "KILLER",               /* 0 */
  "THIEF",
  "FROZEN",
  "!!DONTSET!!",
  "WRITING",
  "MAILING",              /* 5 */
  "CSH",
  "SITEOK",
  "NOSHOUT",
  "NOTITLE",
  "DELETED",              /* 10 */
  "LOADRM",
  "!WIZL",
  "!DEL",
  "INVST",
  "CRYO",                 /* 15 */
  "EDITING",
  "NEWBIE",
  "<PERSONA NON GRATA>",
  "PSIONIC",
  "\n"                    /* 20 */
};


/* MOB_x */
const char *action_bits[] = {
  "SPEC",                    /* 0 */
  "SENTINEL",
  "SCAVENGER",
  "ISNPC",
  "AWARE",
  "AGGR",                    /* 5 */
  "STAY-ZONE",
  "WIMPY",
  "AGGR_EVIL",
  "AGGR_GOOD",
  "AGGR_NEUTRAL",            /* 10 */
  "MEMORY",
  "HELPER",
  "!CHARM",
  "!SUMMN",
  "!SLEEP",                  /* 15 */
  "!BASH",
  "!BLIND",
  "GUARDIAN",
  "HATEFUL",
  "AFRAID",                  /* 20 */
  "REPLICATE",               
  "\n"
};


/* PRF_x */
const char *preference_bits[] = {
  "BRIEF",                 /* 0 */
  "COMPACT",
  "DEAF",
  "!TELL",
  "D_HP",
  "D_MANA",                /* 5 */
  "D_MOVE",
  "AUTOEX",
  "!HASS",
  "QUEST",
  "D_TANK",                /* 10 */
  "!REP",
  "LIGHT",
  "C1",
  "C2",
  "!WIZ",                  /* 15 */
  "L1",
  "L2",
  "D_ENEMY",
  "!GOS",
  "!OOC",                  /* 20 */
  "RMFLG",
  "D_EXP",
  "INFOBAR",
  "AUTOASS",
  "AUTOLOOT",              /* 25  */
  "AUTOGOLD",
  "NOSPAM",
  "SCOREBAR",
  "METER",
  "AFK",                   /* 30 */
  "\n"
};


/* AFF_x */
const char *affected_bits[] =
{
  "REGEN",               /* 0 */
  "INVIS",
  "DET-ALIGN",
  "DET-INVIS",
  "DET-MAGIC",
  "SENSE-LIFE",          /* 5 */
  "WATWALK",
  "SANCT",
  "NONE2",
  "NONE3",
  "INFRA",               /* 10 */
  "NONE4",
  "PROT-EVIL",
  "PROT-GOOD",
  "NONE5",
  "!TRACK",              /* 15 */
  "WATBREATH",
  "HASTE",
  "SNEAK",
  "HIDE",
  "NONE6",               /* 20 */
  "CHARM",
  "NONE7",
  "NONE8",
  "MINGLOB",
  "MAJGLOB",             /* 25 */
  "\n"
};

/* AFF2_x */
const char *affected2_bits[] =
{
  "BLIND",                /* 0 */
  "GROUP",
  "POISON_I",
  "POISON_II",
  "POISON_III",
  "SLEEP",                /* 5 */
  "DISEASE",
  "RELIGED",
  "FLYING",
  "PETRFIED",
  "CURSE",                /* 10 */
  "SILENCE",
  "PARALYSIS",
  "BOUND",
  "BLACK_MANTLE",
  "FLAMEWALL",            /* 15 */
  "!UNUSED!",
  "\n"
};

/* CON_x */
const char *connected_types[] = {
  "Playing",               /* 0 */
  "Disconnecting",
  "New Connection",
  "Get name",
  "Confirm name",
  "Get password",          /* 5 */
  "Get new PW",
  "Confirm new PW",
  "Select Sex",
  "Select Race",
  "Select Attribs",        /* 10 */
  "Select Align",
  "Select Handed",
  "Enter Short D",
  "Cnfrm Short D",
  "Enter Keywords",        /* 15 */
  "Cnfrm Keywords",
  "Enter Email",
  "Confirm Email",
  "Reading NewbieInfo",
  "Reading MOTD",          /* 20 */
  "Main Menu",
  "Get descript.",
  "Changing PW 1",
  "Changing PW 2",
  "Changing PW 3",         /* 25 */
  "Self-Delete 1",
  "Self-Delete 2",
  "Object Edit",
  "Room Edit",
  "Zone Edit",             /* 30 */
  "Mobile Edit",
  "Shop Edit",
  "Guild Edit",
  "\n"
};


const char *body_parts[] = {
  "HEAD",
  "EYE_R",
  "EYE_L",
  "EAR_R",
  "EAR_L",
  "NOSE",
  "ARM_R",
  "ARM_L",
  "HAND_R",
  "HAND_L",
  "LEG_R",
  "LEG_L",
  "FOOT_R",
  "FOOT_L",
  "BODY",
};

const char *where[] = {
  "<Right ring finger>      ",  /* 0 */
  "<Left  ring finger>      ",
  "<Worn around neck>       ",
  "<Worn around neck>       ",
  "<Worn on body>           ",
  "<Worn on head>           ",
  "<Worn on legs>           ",
  "<Worn on right foot>     ",
  "<Worn on left  foot>     ",
  "<Worn on right hand>     ",
  "<Worn on left  hand>     ",  /* 10 */
  "<Worn on right arm>      ",
  "<Worn on left  arm>      ",
  "<Worn on back>           ",
  "<Worn around waist>      ",
  "<Worn on right wrist>    ",
  "<Worn on left  wrist>    ",
  "<Wielded in %-5.5s hand>  ", /* 17 */
  "<Held in %-5.5s hand>     ", /* 18 */
};


/* WEAR_x - for stat */
const char *equipment_types[] = {
  "Worn on right finger",
  "Worn on left finger",
  "First worn around Neck",
  "Second worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on legs",
  "Worn on right foot",
  "Worn on left foot",
  "Worn on right hand",
  "Worn on left hand",
  "Worn on right arm",
  "Worn on left arm",
  "Worn on back",
  "Worn around waist",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Held",
  "\n"
};


/* ITEM_x (ordinal object types) */
const char *item_types[] = {
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FIRE WEAPON",
  "MISSILE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "TRAP",
  "CONTAINER",
  "NOTE",
  "LIQ CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "FOUNTAIN",
  "VEHICLE",
  "SPECIAL",
  "INSTRUMENT",
  "BELT",
  "SPELLBOOK",
  "FURNITURE",
  "\n"
};


/* ITEM_WEAR_ (wear bitvector) */
const char *wear_bits[] = {
  "TAKE",
  "FINGER",
  "NECK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "BACK",
  "WAIST",
  "WRIST",
  "HOLD",
  "\n"
};


/* item size bits */
const char *item_size_bits[] = {
  "TINY",
  "SMALL",
  "MEDIUM",
  "LARGE",
  "HUGE",
  "\n"
};

/* ITEM_x (extra bits) */
const char *extra_bits[] = {
  "GLOW",
  "HUM",
  "!RENT",
  "!DONATE",
  "!INVIS",
  "INVISIBLE",
  "MAGIC",
  "!DROP",
  "BLESS",
  "!GOOD",
  "!EVIL",
  "!NEUTRAL",
  "!HUMAN",
  "!HALFELF",
  "!DWARF",
  "!GNOME",
  "!SELL",
  "FROSTY",
  "SPARKING",
  "FLAMING",
  "ACIDIC",
  "NOXIOUS",
  "BROKEN",
  "*LIMITED*",
  "!DROW",
  "!MINOTAUR",
  "!CENTAUR",
  "!ELF",
  "!SHOW",
  "!SUN",
  "THROWABLE",
  "\n"
};


/* APPLY_x */
const char *apply_types[] = {
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "CHA",
  "WILL",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MAXMANA",
  "MAXHIT",
  "MAXMOVE",
  "GOLD",
  "EXP",
  "AC",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "SAVING_COLD",
  "SAVING_FIRE",
  "SAVING_ACID",
  "SAVING_ELEC",
  "OBJ_AFFECTS",
  "HIT REGEN",
  "MANA REGEN",
  "MOVE REGEN",
  "\n"
};

const char *ammo_types[] = {
   "UNKNOWN",
   "ARROW",
   "XBOW-BOLT",
   "STONE",
   "\n"
};
/* TRAP_DAM_x */
const char *trap_dam_type[] = {
  "UNDEFINED",
  "PIERCE",
  "SLASH",
  "BLUNT",
  "FIRE",
  "COLD",
  "ACID",
  "ELEC",
  "SLEEP",
  "TELEPORT",
  "POISON",
  "DISEASE",
  "SPELL",
  "\n"
};

/* TRIG_x */
const char *trap_triggers[] = {
  "UNDEFINED",
  "ENTER_ROOM",
  "LEAVE_ROOM",
  "GET_OBJ",
  "DROP_OBJ",
  "CLOSE_DOOR",
  "OPEN_DOOR",
  "\n"
};

/* CONT_x */
const char *container_bits[] = {
  "CLOSEABLE",
  "PICKPROOF",
  "CLOSED",
  "LOCKED",
  "\n",
};


/* LIQ_x */
const char *drinks[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "dark ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local speciality",
  "slime mold juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt water",
  "clear water",
  "\n"
};


/* other constants for liquids ******************************************/


/* one-word alias for each drink */
const char *drinknames[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local",
  "juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt",
  "water",
  "urine",
  "feces",
  "\n"
};


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
const int drink_aff[][3] = {
  {0, 1, 10},
  {3, 2, 5},
  {5, 2, 5},
  {2, 2, 5},
  {1, 2, 5},
  {6, 1, 4},
  {0, 1, 8},
  {10, 0, 0},
  {3, 3, 3},
  {0, 4, -8},
  {0, 3, 6},
  {0, 1, 6},
  {0, 1, 6},
  {0, 2, -1},
  {0, 1, -2},
  {0, 0, 13},
  {-10, -20, -5},
  {-10, -20, -5},
};


/* color of the various drinks */
const char *color_liquid[] =
{
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "crystal clear",
  "ochre",
  "brown",
  "\n",
};


/* level of fullness for drink containers */
const char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};


/* str, int, wis, dex, con applies **************************************/


/* [ch] strength apply (all) */
const struct str_app_type str_app[] = {
  {-5, -4, 0, 0},         /* str 0 */
  {-5, -4, 3, 1},         /* str 1 */
  {-3, -2, 3, 2},
  {-3, -1, 10, 3},
  {-2, -1, 25, 4},
  {-2, -1, 55, 5},        /* 5 */
  {-1, 0, 80, 6},
  {-1, 0, 90, 7},
  {0, 0, 100, 8},
  {0, 0, 100, 9},
  {0, 0, 115, 10},        /* 10 */
  {0, 0, 115, 11},
  {0, 0, 140, 12},
  {0, 0, 140, 13},
  {0, 0, 170, 14},
  {0, 0, 170, 15},        /* 15 */
  {0, 1, 195, 16},
  {1, 1, 220, 18},
  {1, 2, 255, 20},        /* 18 */
  {3, 7, 640, 40},
  {3, 8, 700, 40},        /* 20 */
  {4, 9, 810, 40},
  {4, 10, 970, 40},
  {5, 11, 1130, 40},
  {6, 12, 1440, 40},
  {7, 14, 1750, 40},
  {1, 3, 280, 22},
  {2, 3, 305, 24},
  {2, 4, 330, 26},
  {7, 14, 1750, 40},            /* str = 25 */
  {3, 6, 480, 30},              /* 18/0  - 18/50 */
  {3, 6, 480, 30},              /* 18/51 - 18/75 */
  {3, 6, 480, 30},              /* 18/76 - 18/90 */
  {3, 6, 580, 30},              /* 18/91 - 18/99 */
  {3, 7, 680, 32}               /* 18/100 */
};



/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[] = {
   {-99, -99, -90, -99, -60},   /* dex = 0 */
   {-90, -90, -60, -90, -50},   /* dex = 1 */
   {-80, -80, -40, -80, -45},
   {-70, -70, -30, -70, -40},
   {-60, -60, -30, -60, -35},
   {-50, -50, -20, -50, -30},   /* dex = 5 */
   {-40, -40, -20, -40, -25},
   {-30, -30, -15, -30, -20},
   {-20, -20, -15, -20, -15},
   {-15, -10, -10, -20, -10},
   {-10, -5, -10, -15, -5},     /* dex = 10 */
   {-5, 0, -5, -10, 0},
   {0, 0, 0, -5, 0},
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0},             /* dex = 15 */
   {0, 5, 0, 0, 0},
   {5, 10, 0, 5, 5},
   {10, 15, 5, 10, 10},         /* dex = 18 */
   {15, 20, 10, 15, 15},
   {15, 20, 10, 15, 15},                /* dex = 20 */
   {20, 25, 10, 15, 20},
   {20, 25, 15, 20, 20},
   {25, 25, 15, 20, 20},
   {25, 30, 15, 25, 25},
   {25, 30, 15, 25, 25}         /* dex = 25 */
};


/* [dex] apply (all) */
struct dex_app_type dex_app[] = {
   {-7, -7, 0},         /* dex = 0 */
   {-6, -6, 2},         /* dex = 1 */
   {-4, -4, 4},
   {-3, -3, 6},
   {-2, -2, 8},
   {-1, -1, 10},        /* dex = 5 */
   {0, 0, 15},
   {0, 0, 20},
   {0, 0, 25},
   {0, 0, 30},
   {0, 0, 35},           /* dex = 10 */
   {0, 0, 40},
   {0, 0, 45},
   {0, 0, 50},
   {0, 0, 55},
   {0, 0, 60},          /* dex = 15 */
   {1, 1, 65},
   {2, 2, 70},
   {2, 2, 80},          /* dex = 18 */
   {3, 3, 85},
   {3, 3, 90},          /* dex = 20 */
   {4, 4, 95},
   {4, 4, 100},
   {4, 4, 105},
   {5, 5, 110},
   {5, 5, 115}          /* dex = 25 */
};


/* [con] apply (all) */
struct con_app_type con_app[] = {
   {-4, 20},            /* con = 0 */
   {-3, 25},            /* con = 1 */
   {-2, 30},
   {-2, 35},
   {-1, 40},
   {-1, 45},            /* con = 5 */
   {-1, 50},
   {0, 55},
   {0, 60},
   {0, 65},
   {0, 70},             /* con = 10 */
   {0, 75},
   {0, 80},
   {0, 85},
   {0, 88},
   {1, 90},             /* con = 15 */
   {2, 95},
   {2, 97},
   {3, 99},             /* con = 18 */
   {3, 99},
   {4, 99},             /* con = 20 */
   {5, 99},
   {5, 99},
   {5, 99},
   {6, 99},
   {6, 100}             /* con = 25 */
};


/* [int] apply (all) */
struct int_app_type int_app[] = {
   {3},         /* int = 0 */
   {5},         /* int = 1 */
   {7},
   {8},
   {9},
   {10},                /* int = 5 */
   {11},
   {12},
   {13},
   {15},
   {17},                /* int = 10 */
   {19},
   {22},
   {25},
   {30},
   {35},                /* int = 15 */
   {40},
   {45},
   {50},                /* int = 18 */
   {53},
   {55},                /* int = 20 */
   {56},
   {57},
   {58},
   {59},
   {60}         /* int = 25 */
};


/* [wis] apply (all) */
struct wis_app_type wis_app[] = {
   {0}, /* wis = 0 */
   {0},  /* wis = 1 */
   {0},
   {0},
   {0},
   {0},  /* wis = 5 */
   {0},
   {0},
   {0},
   {0},
   {0},  /* wis = 10 */
   {0},
   {2},
   {2},
   {3},
   {3},  /* wis = 15 */
   {3},
   {4},
   {5}, /* wis = 18 */
   {6},
   {6},  /* wis = 20 */
   {6},
   {6},
   {7},
   {7},
   {7}  /* wis = 25 */
};

/* [cha] apply (all) */
struct cha_app_type cha_app[] = {
  {0},
  {0},                          /* 1 */
  {0},
  {0},
  {1},
  {1},                          /* 5 */
  {2},
  {2},
  {3},
  {3},
  {4},                          /* 10 */
  {4},
  {5},
  {5},
  {5},
  {6},                          /* 15 */
  {6},
  {6},
  {6},
  {6},
  {6},                          /* 20 */
  {7},
  {8},
  {9},
  {10},
  {15}                          /* 25 */
};


const char *spell_wear_off_msg[] = {
  "RESERVED DB.C",              /* 0 */
  "RESERVED DB.C",              /* 1 */
  "RESERVED DB.C",              /* 2 */
  "RESERVED DB.C",              /* 3 */
  "RESERVED DB.C",              /* 4 */
  "RESERVED DB.C",              /* 5 */
  "RESERVED DB.C",              /* 6 */
  "RESERVED DB.C",              /* 7 */
  "RESERVED DB.C",              /* 8 */
  "RESERVED DB.C",              /* 9 */
  "RESERVED DB.C",              /* 10 */
  "RESERVED DB.C",              /* 11 */
  "RESERVED DB.C",              /* 12 */
  "RESERVED DB.C",              /* 13 */
  "RESERVED DB.C",              /* 14 */
  "RESERVED DB.C",              /* 15 */
  "RESERVED DB.C",              /* 16 */
  "RESERVED DB.C",              /* 17 */
  "RESERVED DB.C",              /* 18 */
  "RESERVED DB.C",              /* 19 */
  "RESERVED DB.C",              /* 20 */
  "RESERVED DB.C",              /* 21 */
  "RESERVED DB.C",              /* 22 */
  "RESERVED DB.C",              /* 23 */
  "RESERVED DB.C",              /* 24 */
  "RESERVED DB.C",              /* 25 */
  "RESERVED DB.C",              /* 26 */
  "RESERVED DB.C",              /* 27 */
  "RESERVED DB.C",              /* 28 */
  "RESERVED DB.C",              /* 29 */
  "RESERVED DB.C",              /* 30 */
  "RESERVED DB.C",              /* 31 */
  "RESERVED DB.C",              /* 32 */
  "RESERVED DB.C",              /* 33 */
  "RESERVED DB.C",              /* 34 */
  "RESERVED DB.C",              /* 35 */
  "RESERVED DB.C",              /* 36 */
  "RESERVED DB.C",              /* 37 */
  "RESERVED DB.C",              /* 38 */
  "RESERVED DB.C",              /* 39 */
  "RESERVED DB.C",              /* 40 */
  "RESERVED DB.C",              /* 41 */
  "RESERVED DB.C",              /* 42 */
  "RESERVED DB.C",              /* 43 */
  "RESERVED DB.C",              /* 44 */
  "RESERVED DB.C",              /* 45 */
  "RESERVED DB.C",              /* 46 */
  "RESERVED DB.C",              /* 47 */
  "RESERVED DB.C",              /* 48 */
  "RESERVED DB.C",              /* 49 */
  "RESERVED DB.C",              /* 50 */
  "You feel less protected.",   /* 51 */
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",            /* 55 */
  "!Call Lightning",
  "You feel more self-confident.",
  "You feel your strength return.",
  "!Clone!",
  "!Color Spray!",              /* 60 */
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",              /* 65 */
  "!Cure Light!",
  "You feel more optimistic.",
  "You feel less aware.",
  "Your eyes stop tingling.",
  "The detect magic wears off.", /* 70 */
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",             /* 75 */
  "!Fireball!",
  "!Harm!",
  "!Heal!",
  "You feel yourself exposed.",
  "!Lightning Bolt!",           /* 80 */
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected.",
  "!Remove Curse!",             /* 85 */
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",                   /* 90 */
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your suroundings.",
  "!Animate Dead!",             /* 95 */
  "!Dispel Good!",
  "!Group Armor!",
  "!Group Heal!",
  "!Group Recall!",
  "Your night vision seems to fade.",   /* 100 */
  "Your feet seem less boyant.",
  "!Ice Shower!",
  "!Fire Storm!",
  "!Meteor Swarm!",
  "!Power Word Kill!",                  /* 105 */
  "Your gills recede back into your neck.",
  "!Refresh!",
  "!Astral Walk!",
  "You begin to feel more sluggish.",
  "The yellow sphere around you fades.",/* 110 */
  "The magical field around you fades.",
  "You begin to float down to the ground.",
  "!DISPEL MAGIC!",
  "!PORTAL!",
  "!ENCHANT ARMOR!",                    /* 115 */
  "!Cure Serious!",
  "!Disease!",
  "!MONSUM I!",
  "!MONSUN II!",
  "!MONSUM III!",                       /* 120 */
  "!MONSUM IV!",
  "!GATE I!",
  "!GATE II!",
  "!GATE III!",
  "!GATE IV!",                          /* 125 */
  "!SUM FIRE ELE!",
  "!SUM WATER ELE!",
  "!SUM EARTH ELE!",
  "!SUM AIR ELE!",
  "!SUM AERIAL SERV!",                  /* 130 */
  "!DISP ILLUSION!",
  "!ILLUS PRES I!",
  "!ILLUS PRES II!",
  "!ILLUS PRES III!",
  "!ILLUS PRES IV!",                    /* 135 */
  "!GROUP_PORTAL!",
  "!PASS_DOOR!",
  "!KNOCK!",
  "!CURE_PARALYSIS!",
  "Your throat loosens.",               /* 140 */
  "The burn in your blood subsides.",
  "!LIFESTEAL!",
  "Your body reverts back to its normal form.",
  "!FORCE_BOLT!",
  "!PURIFY!",                           /* 145 */
  "!GROUP_PASS_DOOR!",
  "!CURE_DISEASE!",
  "!WIZARD_EYE!",
  "!SOUL_SHOCK!",
  "Your sickness fades.",               /* 150 */
  "Your vision returns to normal.",
  "You feel less protected.",
  "!SPELL_SOUL_SUM_I!",
  "!SPELL_SOUL_SUM_II!",
  "!SPELL_SOUL_SUM_III!",               /* 155 */
  "!SPELL_SOUL_SUM_IV!",
  "!DECAY!",
  "You feel less protected.",
  "Your limbs feel more supple.",
  "!CURSE_WEAPON!",                     /* 160 */
  "!CURSE_ARMOR!",
  "!DEATHKNELL!",
  "The black aura of death around your body fades.",
  "The sores and scabs on your body dry up and fall off.",
  "Your illusion fades.",               /* 165 */
  "Your vision returns.",
  "The illusionary glow around your body fades.",
  "!LIGHTSTRIKE!",
  "Your smoke shield dissipates.",
  "!CALM!",                             /* 170 */
  "You regain your composure.",
  "The room stops spinning about you.",
  "Your vision returns to normal",
  "You reapper!",
  "!FLAME_BOLT!",                       /* 175 */
  "You feel more like yourself.",
  "Your enhanced vision fades.",
  "You feel less aware of deception.",
  "!RESURRECTION!",
  "!HARM_LIGHT!",                       /* 180 */
  "!HARM_SERIOUS!",
  "!HARM_CRITICAL!",
  "You are able to move your limbs again.",
  "!CURE_POISON!",
  "!KNIT_BONE!",                        /* 185 */
  "!RESTORE_LIMB!",
  "!FLAMESTRIKE!",
  "You feel less protected.",
  "The column of flames around you recedes.",
  "!BLADE BARRIER!",
  "!UNUSED!"                            /* 190 */
};



const char *npc_class_types[] = {
  "Normal",
  "Undead",
  "\n"
};



const int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4,
  9,
  8,
  7,
  6
};


const int movement_loss[] =
{
  1,    /* Inside     */
  1,    /* City       */
  2,    /* Field      */
  3,    /* Forest     */
  4,    /* Hills      */
  6,    /* Mountains  */
  4,    /* Swimming   */
  1,    /* Unswimable */
  5,    /* Underwater */
  1,                            /* Flying     */
  5,                            /* Glacier    */
  7,                            /* Desert     */
  5                             /* Jungle     */
};


const char *weekdays[] = {
  "the Day of the Moon",
  "the Day of the Bull",
  "the Day of the Deception",
  "the Day of Thunder",
  "the Day of Freedom",
  "the day of the Great Gods",
  "the Day of the Sun"
};


const char *month_name[] = {
  "Month of Winter",            /* 0 */
  "Month of the Winter Wolf",
  "Month of the Frost Giant",
  "Month of the Old Forces",
  "Month of the Grand Struggle",
  "Month of the Spring",
  "Month of Nature",
  "Month of Futility",
  "Month of the Dragon",
  "Month of the Sun",
  "Month of the Heat",          /* 10 */
  "Month of the Battle",
  "Month of the Dark Shades",
  "Month of the Shadows",
  "Month of the Long Shadows",
  "Month of the Ancient Darkness",
  "Month of the Great Evil"
};


const int sharp[] = {
  0,
  0,
  0,
  1,                            /* Slashing */
  0,
  0,
  0,
  0,                            /* Bludgeon */
  0,
  0,
  0,
  0                             /* Pierce   */
};


char *   const  percent_hit [] = {
    "massacred",            /* 0 */
    "slaughtered",
    "bloodied",
    "beaten",
    "wounded",
    "hurt",                 /* 5 */
    "bruised",
    "scratched",
    "fine",
    "fine",
    "excellent"             /* 10 */
};

char *   const  percent_tired [] = {
    "exhausted",            /* 0 */
    "beat",
    "tired",
    "weary",
    "haggard",
    "fatigued",             /* 5 */
    "worked",
    "winded",
    "rested",
    "well rested",
    "well rested"           /* 10 */
};


char *   const  color_list[] = {
    "red",
    "blue",
    "black",
    "turquoise",
    "yellow",             /* 5 */
    "faded indigo",
    "purple",
    "magenta",
    "lavender",
    "green",              /* 10 */
    "white",
    "faded orange",
    "violet",
    "maroon",
    "mauve",              /* 15 */
    "brown",
    "tan",
    "grey"
};

struct material_type const material_list[] = {
    {"unknown",   50,    0, 0, 0, },    /* 0 */
    {"satin",     32,    6, 0, 0, },
    {"linen",     65,    2, 0, 0, },
    {"silk",      28,    8, 0, 0, },
    {"cloth",     50,    0, 0, 0, },
    {"wool",      85,    1, 0, 0, },    /* 5 */
    {"cotton",    75,    1, 0, 0, },    
    {"reed",      65,    0, 0, 0, },
    {"leather",   100,   3, 0, 0, },
    {"stone",     250,   2, 0, 0, },
    {"wood",      100,   1, 0, 0, },    /* 10 */
    {"brass",     200,   6, 0, 0, },
    {"tin",       225,   5, 0, 0, },
    {"copper",    300,  10, 0, 0, },
    {"bronze",    350,  15, 0, 0, },    
    {"silver",    500,  25, 0, 0, },    /* 15 */
    {"gold",      285,  45, 0, 0, },
    {"platinum",  550,  75, 0, 0, },
    {"steel",    1500,  50, 0, 0, },
    {"admantium",5000, 100, 0, 0, },  
    {"magical",    -1,1000, 0, 0, },    /* 20 */
    {"glass",       5,   5, 0, 0, },
    {"paper",       2,   0, 0, 0, },    /* 22 */
};


#if 0
/*
 * Coin information
 */
const struct coin_type coin_table[MAX_COIN] =
{
   {  10,   10000,   3, "obsidian",   "op"  },
   {  10,    1000,   2, "platinum",   "pp"  },
   {  10,     100,   1, "gold",       "gp"  },
   {  10,      10,   1, "silver",     "sp"  },
   {  10,       1,   1, "copper",     "cp"  }
};

#endif

char *locations[] = {
     "head",             /* 0 */
     "right arm",
     "left arm",
     "right leg",
     "left leg",
     "chest",            /* 5 */
     "right foot",
     "left foot",
     "right hand",
     "left hand",
     "neck",             /* 10 */
     "body",             /* 11 */
     "\n"
};


