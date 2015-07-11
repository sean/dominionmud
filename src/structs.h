/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  STRUCTS.H                                    Based on CircleMUD *
*  Usage: Header: Central Structures and Constants (Backbone of TD)       *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All Modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#ifndef __STRUCTS_H__       /* To prevent multiple inclusions */
#define __STRUCTS_H__

#include <sys/types.h>

/* preamble *************************************************************/

#define NOWHERE    -1    /* nil reference for room-database     */
#define NOTHING    -1    /* nil reference for objects           */
#define NOBODY     -1    /* nil reference for mobiles           */

#define SPECIAL(name) \
   int (name)(struct char_data *ch, void *me, int cmd, char *argument)


/* room-related defines *************************************************/


/* The cardinal directions: used as index to room_data.dir_option[] */
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5
#define NORTHEAST      6
#define NORTHWEST      7
#define SOUTHEAST      8
#define SOUTHWEST      9

#define MAX_DIR       10

/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define ROOM_DARK               (1 << 0)   /* Dark                      */
#define ROOM_DEATH              (1 << 1)   /* Death trap                */
#define ROOM_NOMOB              (1 << 2)   /* MOBs not allowed          */
#define ROOM_INDOORS            (1 << 3)   /* Indoors                   */
#define ROOM_PEACEFUL           (1 << 4)   /* Violence not allowed      */
#define ROOM_SOUNDPROOF         (1 << 5)   /* Shouts, gossip blocked    */
#define ROOM_NOTRACK            (1 << 6)   /* Track won't go through    */
#define ROOM_NOMAGIC            (1 << 7)   /* Magic not allowed         */
#define ROOM_TUNNEL             (1 << 8)   /* room for only 1 pers      */
#define ROOM_PRIVATE            (1 << 9)   /* Can't teleport in         */
#define ROOM_GODROOM            (1 << 10)  /* LVL_GOD+ only allowed     */
#define ROOM_HOUSE              (1 << 11)  /* (R) Room is a house       */
#define ROOM_HOUSE_CRASH        (1 << 12)  /* (R) House needs saving    */
#define ROOM_ATRIUM             (1 << 13)  /* (R) The door to a house   */
#define ROOM_OLC                (1 << 14)  /* (R) Modifyable/!compress  */
#define ROOM_BFS_MARK           (1 << 15)  /* (R) breath-first srch mrk */
#define ROOM_UNUSED             (1 << 16)  /* Room is Trapped          q */
#define ROOM_REGEN              (1 << 17)  /* Regeneration 3x HMV/tick r */
#define ROOM_NOSUMMON           (1 << 18)  /* Nosummon in this room    s */
#define ROOM_NOHEAL             (1 << 19)  /* No Reg/tick in this room t */
#define ROOM_BLADEBARRIER       (1 << 20)  /* No one can leave the room  */

#define NUM_ROOM_FLAGS          20

/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR               (1 << 0)   /* Exit is a door            */
#define EX_CLOSED               (1 << 1)   /* The door is closed        */
#define EX_LOCKED               (1 << 2)   /* The door is locked        */
#define EX_PICKPROOF            (1 << 3)   /* Lock can't be picked      */
#define EX_SECRET               (1 << 4)   /* Exit is secret         16  */
#define EX_CLIMB                (1 << 5)   /* Must climb up exit     32  */
#define EX_ARROWSLIT            (1 << 6)   /* Exit is an arrowslit   64  */

#define NUM_EXIT_TYPES          7

/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0             /* Indoors                   */
#define SECT_CITY            1             /* In a city                 */
#define SECT_FIELD           2             /* In a field                */
#define SECT_FOREST          3             /* In a forest               */
#define SECT_HILLS           4             /* In the hills              */
#define SECT_MOUNTAIN        5             /* On a mountain             */
#define SECT_WATER_SWIM      6             /* Swimmable water           */
#define SECT_WATER_NOSWIM    7             /* Water - need a boat       */
#define SECT_UNDERWATER      8             /* Underwater                */
#define SECT_FLYING          9             /* Wheee!                    */
#define SECT_GLACIER         10            /* Ice cold                  */
#define SECT_DESERT          11            /* Very hot                  */
#define SECT_JUNGLE          12            /* Difficult to travel thru  */

#define NUM_ROOM_SECTORS     10

#define TELE_LOOK            1
#define TELE_COUNT           2
#define TELE_RANDOM          4
#define TELE_SPIN            8

#define LARGE_NONE           0
#define LARGE_WATER          1
#define LARGE_AIR            2
#define LARGE_IMPASS         4
/* char and mob-related defines *****************************************/


/* These are mostly left to role-playing . . .                            */
#define DISORD_AGORAPHOBIA      (1 << 0)   /* Char is afraid of open spcs */
#define DISORD_ALCOHOLISM       (1 << 1)   /* Dependant on Alcohol        */
#define DISORD_ANIMOSITY        (1 << 2)   /* Unreasoning hatred of choice*/
#define DISORD_CATATONIA        (1 << 3)   /* Complete withdrawl          */
#define DISORD_DEMENTIA         (1 << 4)   /* Mumbling, paranoia, fear    */
#define DISORD_DEPRESSION       (1 << 5)   /* Deep pessimism              */
#define DISORD_SCOTOPHOBIA      (1 << 6)   /* Fear of the dark            */
#define DISORD_MANIC_DEPRESSIVE (1 << 7)   /* Tense/overact or Depression */
#define DISORD_MEGALOMANIA      (1 << 8)   /* Inflated Ego                */
#define DISORD_PATHO_LIAR       (1 << 9)   /* Inability to tell the truth */
#define DISORD_ABSENT_MINDED    (1 << 10)  /* Lose Concentration          */
#define DISORD_NARCISSISM       (1 << 11)  /* Self-infatuation            */
#define DISORD_SHAKING          (1 << 12)  /* Uncontrollable shaking      */
#define DISORD_STAMMER          (1 << 13)  /* Char stutters               */
#define DISORD_TALKING_2SELF    (1 << 14)  /* Char talks to self out-loud */

#define ABIL_REGENERATION       (1 << 0)   /* Char can regenerate         */
#define ABIL_AMBIDEXTROUS       (1 << 1)   /* Char can use either hand    */
#define ABIL_CONTORTIONIST      (1 << 2)   /* Char can manipulate body    */
#define ABIL_IMMUNE_DISEASE     (1 << 3)   /* Char is immune to disease   */
#define ABIL_IMMUNE_POISON      (1 << 4)   /* Char is immune to poison    */
#define ABIL_IMMUNE_FIRE        (1 << 5)   /* Char is immune to fire      */
#define ABIL_IMMUNE_COLD        (1 << 6)   /* Char is immune to cold      */
#define ABIL_IMMUNE_SHOCK       (1 << 7)   /* Char is immune to electrici */
#define ABIL_LIGHTNING_REFLEXES (1 << 8)   /* Char has very quick reflexes*/


#define CLASS_WIZARD       0
#define CLASS_PRIEST       1
#define CLASS_ROGUE        2
#define CLASS_WARRIOR      3
#define NUM_BASIC_CLASSES  4

/* PC and NPC races */
#define RACE_UNDEFINED   -1
#define RACE_HUMAN        0
#define RACE_ATHASIANAE   1      /* Half-Elf   */
#define RACE_THURGAR      2      /* High Dwarf */
#define RACE_GNOME        3
#define RACE_DARGONAE     4      /* Drow        */
#define RACE_KINTHALAS    5      /* Minotaur    */
#define RACE_CENTAUR      6
#define RACE_ARMACHNAE    7      /* High Elves  */
#define RACE_TARMIRNAE    8      /* Grey Elves  */
#define RACE_RADINAE      9      /* Wild Elves  */
#define RACE_KENDER       10     /* Kender      */
#define RACE_DAERWAR      11     /* Smart Dwarf */
#define RACE_KAERGAR      12     /* Holy Dwarf  */
#define RACE_ZAKHAR       13     /* Dark Dwarf  */
#define RACE_HALFLING     14     /* Halfling    */
#define RACE_HALFGIANT    15     /* Half-Giant  */
#define RACE_SESSANATHI   16     /* Lizard Men  */
#define RACE_HALFOGRE     17     /* Half-Ogre   */
#define RACE_BYTERIAN     18     /* Byterian Min*/
#define RACE_GULLYDWARF   19
#define RACE_VAMPIRE      20
#define RACE_WEREWOLF     21

#define NUM_PLR_RACES     20

#define RACE_ANIMAL       22
#define RACE_UNDEAD       23
#define RACE_GIANT        24
#define RACE_HUMANOID     25
#define RACE_VEGGIE       26
#define RACE_DEMON        27
#define RACE_MAGIC        28
#define RACE_WATER        29
#define RACE_ILLUSIONARY  30
/* Evil Dragons */
#define RACE_RED_DRAGON   31
#define RACE_BLUE_DRAGON  32
#define RACE_GREEN_DRAGON 33
#define RACE_BLACK_DRAGON 34
#define RACE_WHITE_DRAGON 35
/* Good Dragons */
#define RACE_BRONZE_DRAGON   36
#define RACE_COPPER_DRAGON   37
#define RACE_SILVER_DRAGON   38
#define RACE_GOLD_DRAGON     39
#define RACE_PLATINUM_DRAGON 40
/* more races added below */
#define RACE_PIXIE           41
#define RACE_CHROMATIC       42
#define RACE_ARACHNID        43  /* Both of these are poisonous */
#define RACE_SERPENT         44
#define TOT_RACES            45  /* This must be the number of races!! */

/* PC & NPC religions - NPCs have them incase I want to have a jyhad! */
#define REL_UNDEFINED    -1
#define REL_NONE          0
#define REL_GOOD          1
#define REL_NEUT          2
#define REL_EVIL          3
#define REL_LAWFL         4
#define REL_CHAOS         5

#define NUM_RELS          6  /* This must be the number of religions */

#define MAX_GUILDS	    20
#define GUILD_UNDEFINED    -1
#define GUILD_NONE          0
#define GUILD_ATHAR         1
#define GUILD_BELIEVERS     2
#define GUILD_CABAL         3
#define GUILD_DOOMGUARD     4
#define GUILD_DUSTMEN       5
#define GUILD_FATED         6
#define GUILD_ORDER         7
#define GUILD_FREE_LEAGUE   8
#define GUILD_HARMONIUM     9
#define GUILD_MERCYKILLERS  10
#define GUILD_REVOLUTIONARY 11
#define GUILD_SIGN_ONE      12
#define GUILD_SENSATION     13
#define GUILD_TRANSCENDANT  14
#define GUILD_XAOSITECTS    15

#define NUM_GUILDS          16
#define MAX_GUILD_LVL	    5

/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* Languages */
#define LANG_COMMON       0
#define LANG_ELVEN        1
#define LANG_DWARVEN      2
#define LANG_GNOMISH      3
#define LANG_MINOTAUR     4      /* Is that a word?  */
#define LANG_CENTAUR      5      /* Is this one too? */
#define LANG_HALFLING     6      /* Halfling    */
#define LANG_HALFGIANT    7      /* Half-Giant  */
#define LANG_SESSANATHI   8      /* Lizard Men  */
#define LANG_HALFOGRE     9      /* Half-Ogre   */
#define LANG_DRAGON       10     /* Dragon      */
#define LANG_ANIMAL       11     /* Animals     */

#define MAX_LANGUAGES     30      /* Used in char_file_u DO NOT CHANGE!!!! */

/* Dialects for languages */
#define DIAL_NORTHERN   (1 << 0)  /* Northern dialect   */
#define DIAL_SOUTHERN   (1 << 1)  /* Southern dialect   */
#define DIAL_CENTRAL    (1 << 2)  /* Central dialect    */
#define DIAL_RED        (1 << 3)  /* Red Dragons        */
#define DIAL_BLUE       (1 << 4)  /* Blue Dragons       */
#define DIAL_GREEN      (1 << 5)  /* Green Dragons      */
#define DIAL_BLACK      (1 << 6)  /* Black Dragons      */
#define DIAL_WHITE      (1 << 7)  /* White Dragons      */
#define DIAL_COPPER     (1 << 8)  /* Copper Dragons     */
#define DIAL_BRONZE     (1 << 9)  /* Bronze Dragons     */
#define DIAL_SILVER     (1 << 10) /* Silver Dragons     */
#define DIAL_GOLD       (1 << 11) /* Gold Dragons       */
#define DIAL_PLATINUM   (1 << 12) /* Platinum Dragons   */

/* Markers for Align permanency */
#define ALIGN_NEUT     0
#define ALIGN_EVIL     1
#define ALIGN_GOOD     2

/* Positions */
#define POS_DEAD       0        /* dead                 */
#define POS_MORTALLYW  1        /* mortally wounded     */
#define POS_INCAP      2        /* incapacitated        */
#define POS_STUNNED    3        /* stunned              */
#define POS_SLEEPING   4        /* sleeping             */
#define POS_RESTING    5        /* resting              */
#define POS_SITTING    6        /* sitting              */
#define POS_FIGHTING   7        /* fighting             */
#define POS_STANDING   8        /* standing             */
#define POS_MOUNTED    9        /* mounted              */
#define POS_FLYING     10       /* flying               */
#define POS_MEDITATING 11       /* meditating           */

#define NUM_POSITIONS  12

/* Defines for Hates */
#define HATE_SEX   (1 << 0)
#define HATE_RACE  (1 << 1)
#define HATE_CHAR  (1 << 2)
#define HATE_EVIL  (1 << 3)
#define HATE_GOOD  (1 << 4)
#define HATE_VNUM  (1 << 5)

/* Defines for Fears */
#define FEAR_SEX   (1 << 0)
#define FEAR_RACE  (1 << 1)
#define FEAR_CHAR  (1 << 2)
#define FEAR_EVIL  (1 << 3)
#define FEAR_GOOD  (1 << 4)
#define FEAR_VNUM  (1 << 5)

/* Defines for Opinions */
#define OP_SEX     (1 << 0)
#define OP_RACE    (1 << 1)
#define OP_CHAR    (1 << 3)
#define OP_EVIL    (1 << 4)
#define OP_GOOD    (1 << 5)
#define OP_VNUM    (1 << 6)


/* Player flags: used by char_data.char_specials.act */
#define PLR_KILLER      (1 << 0)   /* Player is a player-killer         */
#define PLR_THIEF       (1 << 1)   /* Player is a player-thief          */
#define PLR_FROZEN      (1 << 2)   /* Player is frozen                  */
#define PLR_DONTSET     (1 << 3)   /* Don't EVER set (ISNPC bit)        */
#define PLR_WRITING     (1 << 4)   /* Player writing (board/mail/olc)   */
#define PLR_MAILING     (1 << 5)   /* Player is writing mail            */
#define PLR_CRASH       (1 << 6)   /* Player needs to be crash-saved    */
#define PLR_SITEOK      (1 << 7)   /* Player has been site-cleared      */
#define PLR_NOSHOUT     (1 << 8)   /* Player not allowed to shout/goss  */
#define PLR_NOTITLE     (1 << 9)   /* Player not allowed to set title   */
#define PLR_DELETED     (1 << 10)  /* Player deleted - space reusable   */
#define PLR_LOADROOM    (1 << 11)  /* Player uses nonstandard loadroom  */
#define PLR_NOWIZLIST   (1 << 12)  /* Player shouldn't be on wizlist    */
#define PLR_NODELETE    (1 << 13)  /* Player shouldn't be deleted       */
#define PLR_INVSTART    (1 << 14)  /* Player should enter game wizinvis */
#define PLR_CRYO        (1 << 15)  /* Player is cryo-saved (purge prog) */
#define PLR_EDITING     (1 << 16)  /* Player is zone editing            */
#define PLR_NEWBIE      (1 << 17)  /* Player is a newbie, and cannot be PK */
#define PLR_ASSHOLE     (1 << 18)  /* God has abused commands           */
#define PLR_PSIONIC     (1 << 19)  /* Player has psionic abilities      */


#define TEMPLE_SQUARE_VNUM  3021
/* Important Monster vnums */
#define ARENAGUY_VNUM     XXXX
#define CHRONO_VNUM       3095
#define CHRONO_VNUM2      5491

/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC         (1 << 0)  /* Mob has a callable spec-proc      */
#define MOB_SENTINEL     (1 << 1)  /* Mob should not move               */
#define MOB_SCAVENGER    (1 << 2)  /* Mob picks up stuff on the ground  */
#define MOB_ISNPC        (1 << 3)  /* (R) Automatically set on all Mobs */
#define MOB_AWARE        (1 << 4)  /* Mob can't be backstabbed          */
#define MOB_AGGRESSIVE   (1 << 5)  /* Mob hits players in the room      */
#define MOB_STAY_ZONE    (1 << 6)  /* Mob shouldn't wander out of zone  */
#define MOB_WIMPY        (1 << 7)  /* Mob flees if severely injured     */
#define MOB_AGGR_EVIL    (1 << 8)  /* auto attack evil PC's             */
#define MOB_AGGR_GOOD    (1 << 9)  /* auto attack good PC's             */
#define MOB_AGGR_NEUTRAL (1 << 10) /* auto attack neutral PC's          */
#define MOB_MEMORY       (1 << 11) /* remember attackers if attacked    */
#define MOB_HELPER       (1 << 12) /* attack PCs fighting other NPCs    */
#define MOB_NOCHARM      (1 << 13) /* Mob can't be charmed              */
#define MOB_NOSUMMON     (1 << 14) /* Mob can't be summoned             */
#define MOB_NOSLEEP      (1 << 15) /* Mob can't be slept                */
#define MOB_NOBASH       (1 << 16) /* Mob can't be bashed (e.g. trees)  */
#define MOB_NOBLIND      (1 << 17) /* Mob can't be blinded              */
#define MOB_GUARDIAN     (1 << 18) /* Mob guards its master             */
#define MOB_HATEFUL      (1 << 19) /* Mob hates someone or something    */
#define MOB_AFRAID       (1 << 20) /* Mob is afraid of someone or something */
#define MOB_REPLICANT    (1 << 21) /* Mob replicates itself             */

#define NUM_MOB_FLAGS    22

/* Preference flags: used by char_data.player_specials.pref */
#define PRF_BRIEF       (1 << 0)  /* Room descs won't normally be shown */
#define PRF_COMPACT     (1 << 1)  /* No extra CRLF pair before prompts  */
#define PRF_DEAF        (1 << 2)  /* Can't hear shouts                  */
#define PRF_NOTELL      (1 << 3)  /* Can't receive tells                */
#define PRF_DISPHP      (1 << 4)  /* Display hit points in prompt       */
#define PRF_DISPMANA    (1 << 5)  /* Display mana points in prompt      */
#define PRF_DISPMOVE    (1 << 6)  /* Display move points in prompt      */
#define PRF_AUTOEXIT    (1 << 7)  /* Display exits in a room            */
#define PRF_NOHASSLE    (1 << 8)  /* Aggr mobs won't attack             */
#define PRF_QUEST       (1 << 9)  /* On quest                           */
#define PRF_DISPTANK    (1 << 10) /* Display Tank's HPS                 */
#define PRF_NOREPEAT    (1 << 11) /* No repetition of comm commands     */
#define PRF_HOLYLIGHT   (1 << 12) /* Can see in dark                    */
#define PRF_COLOR_1     (1 << 13) /* Color (low bit)                    */
#define PRF_COLOR_2     (1 << 14) /* Color (high bit)                   */
#define PRF_NOWIZ       (1 << 15) /* Can't hear wizline                 */
#define PRF_LOG1        (1 << 16) /* On-line System Log (low bit)       */
#define PRF_LOG2        (1 << 17) /* On-line System Log (high bit)      */
#define PRF_DISPENEMY   (1 << 18) /* Display Enemy health in prompt     */
#define PRF_NOGOSS      (1 << 19) /* Can't hear gossip channel          */
#define PRF_NOOOC       (1 << 20) /* Can't hear OOC channel             */
#define PRF_ROOMFLAGS   (1 << 21) /* Can see room flags (ROOM_x)        */
#define PRF_DISEXP2LEV  (1 << 22) /* Display % of exp needed for nxt lvl*/
#define PRF_INFOBAR     (1 << 23) /* ANSI Infobar setup                 */
#define PRF_AUTOASS     (1 << 24) /* Auto Assists leader                */
#define PRF_AUTOLOOT    (1 << 25) /* auto loot corpses                  */
#define PRF_AUTOGOLD    (1 << 26) /* auto get coins corpse              */
#define PRF_NOSPAM      (1 << 27) /* no spammage                        */
#define PRF_SCOREBAR    (1 << 28) /* ScoreBar part of InfoBar           */
#define PRF_METER       (1 << 29) /* Health Meter                       */
#define PRF_AFK         (1 << 30) /* AFK Flag                           */

/* Player object specials affects stored in char_special_data.item_aff_by */
#define AFFECTED_SANCTUARY             (1 << 0)
#define AFFECTED_DETECT_INVIS          (1 << 1)
#define AFFECTED_DETECT_ALIGN          (1 << 2)
#define AFFECTED_DETECT_MAGIC          (1 << 3)
#define AFFECTED_INFRA                 (1 << 4)
#define AFFECTED_SENSE_LIFE            (1 << 5)
#define AFFECTED_WATERWALK             (1 << 6)
#define AFFECTED_WATERBREATH           (1 << 7)
#define AFFECTED_HASTE                 (1 << 8)
#define AFFECTED_INVIS                 (1 << 9)
#define AFFECTED_FLYING                (1 << 10)
#define AFFECTED_PEEKING               (1 << 11)
#define AFFECTED_UNUSED03              (1 << 12)
#define AFFECTED_UNUSED04              (1 << 13)

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_REGENERATION      (1 << 0)     /* Char can regenerate        a */
#define AFF_INVISIBLE         (1 << 1)     /* Char is invisible          b */
#define AFF_DETECT_ALIGN      (1 << 2)     /* Char is sensitive to align c */
#define AFF_DETECT_INVIS      (1 << 3)     /* Char can see invis chars   d */
#define AFF_DETECT_MAGIC      (1 << 4)     /* Char is sensitive to magic e */
#define AFF_SENSE_LIFE        (1 << 5)     /* Char can sense hidden life f */
#define AFF_WATERWALK         (1 << 6)     /* Char can walk on water     g */
#define AFF_SANCTUARY         (1 << 7)     /* Char protected by sanct.   h */
#define AFF_UNUSED02          (1 << 8)     /* Char is ?                  i */
#define AFF_UNUSED03          (1 << 9)     /* Char is ?                  j */
#define AFF_INFRAVISION       (1 << 10)    /* Char can see in dark       k */
#define AFF_UNUSED04          (1 << 11)    /* Char is ?                  l */
#define AFF_PROTECT_EVIL      (1 << 12)    /* Char protected from evil   m */
#define AFF_PROTECT_GOOD      (1 << 13)    /* Char protected from good   n */
#define AFF_UNUSED05          (1 << 14)    /* Char is ?                  o */
#define AFF_NOTRACK           (1 << 15)    /* Char can't be tracked      p */
#define AFF_WATERBREATH       (1 << 16)    /* Char can breath underwater q */
#define AFF_HASTE             (1 << 17)    /* Room for future expansion  r */
#define AFF_SNEAK             (1 << 18)    /* Char can move quietly      s */
#define AFF_HIDE              (1 << 19)    /* Char is hidden             t */
#define AFF_UNUSED06          (1 << 20)    /* Char is ?                  u */
#define AFF_CHARM             (1 << 21)    /* Char is charmed            v */
#define AFF_UNUSED07          (1 << 22)    /* Char is ?                  w */
#define AFF_UNUSED08          (1 << 23)    /* Char is ?                  x */
#define AFF_MIN_GLOBE         (1 << 24)    /* Char protected by Minor G  y */
#define AFF_MAJ_GLOBE         (1 << 25)    /* Char protected by Major G  z */

#define NUM_AFF_FLAGS         26

/* Affect2 bits: used in char_data.char_specials.saved.affected_by2 */
/* These can only be set by the game ...                            */
#define AFF_BLIND             (1 << 0)     /* (R) Char is blind           a */
#define AFF_GROUP             (1 << 1)     /* (R) Char is grouped         b */
#define AFF_POISON_I          (1 << 2)     /* (R) Char is poisoned Typ I  c */
#define AFF_POISON_II         (1 << 3)     /* (R) Char is poisoned Typ II d */
#define AFF_POISON_III        (1 << 4)     /* (R) Char is poisoned Typ IIIe */
#define AFF_SLEEP             (1 << 5)     /* (R) Char magically asleep   f */
#define AFF_DISEASE           (1 << 6)     /* Char is affected by disease g */
#define AFF_RELIGED           (1 << 7)     /* Char has been blessed       h */
#define AFF_FLYING            (1 << 8)     /* Char is flying (see POS)    i */
#define AFF_PETRIFIED         (1 << 9)     /* Char is petrified           j */
#define AFF_CURSE             (1 << 10)    /* Char is cursed              k */
#define AFF_SILENCE           (1 << 11)    /* Char is silenced no spells  l */
#define AFF_PARALYSIS         (1 << 12)    /* Char is paralized (sp?)     m */
#define AFF_BOUND             (1 << 13)    /* Char is tied up and immoble n */
#define AFF_BLACKMANTLE       (1 << 14)    /* Char is mantled ;-)         o */
#define AFF_FIREWALL          (1 << 15)    /* Char is firewalled          p */

/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING      0              /* Playing - Nominal state      */
#define CON_CLOSE        1              /* Disconnecting                */
#define CON_NEW_CONN     2              /* Color?                       */
#define CON_GET_NAME     3              /* By what name ..?             */
#define CON_NAME_CNFRM   4              /* Did I get that right, x?     */
#define CON_PASSWORD     5              /* Password:                    */
#define CON_NEWPASSWD    6              /* Give me a password for x     */
#define CON_CNFPASSWD    7              /* Please retype password:      */
#define CON_QSEX         8              /* Sex?                         */
#define CON_QRACE        9              /* Race?                        */
#define CON_QATTRIBS     10              /* Player sets-up their attribs */
#define CON_QALIGN       11             /* Permanent Alignment?         */
#define CON_QHANDEDNESS  12             /* Righty or Lefty?             */
/* These are unused at this time */
#define CON_QSHORT_DESC  13             /* Your short description?      */
#define CON_SHORT_CNFRM  14             /* Does this look right?        */
#define CON_QKEYWORDS    15             /* Your keywords?               */
#define CON_KEYWDS_CNFRM 16             /* Do these look right?         */
/* End of unused CON_'s */
#define CON_QEMAIL       17             /* Get the email address        */
#define CON_QEMAIL_CNFRM 18             /* confirm their email address  */
#define CON_NEWM         19             /* PRESS RETURN after newbie Mes*/
#define CON_RMOTD        20             /* PRESS RETURN after MOTD      */
#define CON_MENU         21             /* Your choice: (main menu)     */
#define CON_EXDESC       22             /* Enter a new description:     */
#define CON_CHPWD_GETOLD 23             /* Changing passwd: get old     */
#define CON_CHPWD_GETNEW 24             /* Changing passwd: get new     */
#define CON_CHPWD_VRFY   25             /* Verify new password          */
#define CON_DELCNF1      26             /* Delete confirmation 1        */
#define CON_DELCNF2      27             /* Delete confirmation 2        */
#define CON_OEDIT        28             /*. OLC mode - object edit     .*/
#define CON_REDIT        29             /*. OLC mode - room edit       .*/
#define CON_ZEDIT        30             /*. OLC mode - zone info edit  .*/
#define CON_MEDIT        31             /*. OLC mode - mobile edit     .*/
#define CON_SEDIT        32             /*. OLC mode - shop edit       .*/
#define CON_GEDIT        33             /*. OLC mode - guild edit      .*/
#define CON_PICO         34             /*. Using the Editor .*/

/* Locations for armor values */
#define ARMOR_HEAD      0
#define ARMOR_SHIELD    1
#define ARMOR_BODY      2
#define ARMOR_LEG_L     3
#define ARMOR_LEG_R     4
#define ARMOR_ARM_L     5
#define ARMOR_ARM_R     6
#define ARMOR_LIMIT     7

/* Character equipment positions: used as index for char_data.equipment[] */
/* NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment */
#define WEAR_FINGER_R   0
#define WEAR_FINGER_L   1
#define WEAR_NECK_1     2
#define WEAR_NECK_2     3
#define WEAR_BODY       4
#define WEAR_HEAD       5
#define WEAR_LEGS       6
#define WEAR_FOOT_R     7
#define WEAR_FOOT_L     8
#define WEAR_HAND_R     9
#define WEAR_HAND_L    10
#define WEAR_ARM_R     11
#define WEAR_ARM_L     12
#define WEAR_BACK      13
#define WEAR_WAIST     14
#define WEAR_WRIST_R   15
#define WEAR_WRIST_L   16
#define WEAR_WIELD     17
#define WEAR_HOLD      18

#define NUM_WEARS      19       /* This must be the # of eq positions!! */

/* object-related defines ********************************************/

/* Item types: used by obj_data.obj_flags.type_flag */
#define ITEM_LIGHT      1               /* Item is a light source       */
#define ITEM_SCROLL     2               /* Item is a scroll             */
#define ITEM_WAND       3               /* Item is a wand               */
#define ITEM_STAFF      4               /* Item is a staff              */
#define ITEM_WEAPON     5               /* Item is a weapon             */
#define ITEM_FIREWEAPON 6               /* Unimplemented                */
#define ITEM_MISSILE    7               /* Unimplemented                */
#define ITEM_TREASURE   8               /* Item is a treasure, not gold */
#define ITEM_ARMOR      9               /* Item is armor                */
#define ITEM_POTION    10               /* Item is a potion             */
#define ITEM_WORN      11               /* Unimplemented                */
#define ITEM_OTHER     12               /* Misc object                  */
#define ITEM_TRASH     13               /* Trash - shopkeeps won't buy  */
#define ITEM_TRAP      14               /* Unimplemented                */
#define ITEM_CONTAINER 15               /* Item is a container          */
#define ITEM_NOTE      16               /* Item is note                 */
#define ITEM_DRINKCON  17               /* Item is a drink container    */
#define ITEM_KEY       18               /* Item is a key                */
#define ITEM_FOOD      19               /* Item is food                 */
#define ITEM_MONEY     20               /* Item is money (gold)         */
#define ITEM_PEN       21               /* Item is a pen                */
#define ITEM_BOAT      22               /* Item is a boat               */
#define ITEM_FOUNTAIN  23               /* Item is a fountain           */
#define ITEM_VEHICLE    24              /* Players can enter & ride it  */
#define ITEM_SPECIAL    25              /* Special item...              */
#define ITEM_INSTRUMENT 26              /* Requiem for Bards            */
#define ITEM_BELT       27              /* Item is a belt               */
#define ITEM_SPELLBOOK  28              /* Soon to be a Requiem for spl */
#define ITEM_FURNITURE  29              /* Place to sit, rest, sleep, etc */

#define NUM_ITEM_TYPES  29

/* Take/Wear flags: used by obj_data.obj_flags.wear_flags */
#define ITEM_WEAR_TAKE          (1 << 0)  /* Item can be takes        a */
#define ITEM_WEAR_FINGER        (1 << 1)  /* Can be worn on finger    b */
#define ITEM_WEAR_NECK          (1 << 2)  /* Can be worn around neck  c */
#define ITEM_WEAR_BODY          (1 << 3)  /* Can be worn on body      d */
#define ITEM_WEAR_HEAD          (1 << 4)  /* Can be worn on head      e */
#define ITEM_WEAR_LEGS          (1 << 5)  /* Can be worn on legs      f */
#define ITEM_WEAR_FEET          (1 << 6)  /* Can be worn on feet      g */
#define ITEM_WEAR_HANDS         (1 << 7)  /* Can be worn on hands     h */
#define ITEM_WEAR_ARMS          (1 << 8)  /* Can be worn on arms      i */
#define ITEM_WEAR_BACK          (1 << 9)  /* Can be worn on back      j */
#define ITEM_WEAR_WAIST         (1 << 10) /* Can be worn around waist k */
#define ITEM_WEAR_WRIST         (1 << 11) /* Can be worn on wrist     l */
#define ITEM_WEAR_HOLD          (1 << 12) /* Can be held in hand      m */

#define NUM_WEAR_BITS           13

/* size flags used to see who can wear the item */
#define ITEM_SIZE_TINY     (1 << 0)     /* Item can be used by tiny ppls */
#define ITEM_SIZE_SMALL    (1 << 1)     /* Item can be used by small ppl */
#define ITEM_SIZE_MEDIUM   (1 << 2)     /* Item can be used by humanoids */
#define ITEM_SIZE_LARGE    (1 << 3)     /* Item can be used by large ppl */
#define ITEM_SIZE_HUGE     (1 << 4)     /* Item can be used by dragons & */
#define NUM_SIZES          5

/* Extra object flags: used by obj_data.obj_flags.extra_flags */
#define ITEM_GLOW          (1 << 0)     /* Item is glowing              */
#define ITEM_HUM           (1 << 1)     /* Item is humming              */
#define ITEM_NORENT        (1 << 2)     /* Item cannot be rented        */
#define ITEM_NODONATE      (1 << 3)     /* Item cannot be donated       */
#define ITEM_NOINVIS       (1 << 4)     /* Item cannot be made invis    */
#define ITEM_INVISIBLE     (1 << 5)     /* Item is invisible            */
#define ITEM_MAGIC         (1 << 6)     /* Item is magical              */
#define ITEM_NODROP        (1 << 7)     /* Item is cursed: can't drop   */
#define ITEM_BLESS         (1 << 8)     /* Item is blessed              */
#define ITEM_ANTI_GOOD     (1 << 9)     /* Not usable by good people    */
#define ITEM_ANTI_EVIL     (1 << 10)    /* Not usable by evil people    */
#define ITEM_ANTI_NEUTRAL  (1 << 11)    /* Not usable by neutral people */
#define ITEM_ANTI_HUMAN    (1 << 12)    /* Not usable by mages          */
#define ITEM_ANTI_HALFELF  (1 << 13)    /* Not usable by clerics        */
#define ITEM_ANTI_DWARF    (1 << 14)    /* Not usable by thieves        */
#define ITEM_ANTI_GNOME    (1 << 15)    /* Not usable by warriors       */
#define ITEM_NOSELL        (1 << 16)    /* Shopkeepers won't touch it   */
#define ITEM_FROSTY        (1 << 17)    /* Frost breath                 */
#define ITEM_SPARKING      (1 << 18)    /* Lightning breath             */
#define ITEM_FLAMING       (1 << 19)    /* Fire breath                  */
#define ITEM_ACIDIC        (1 << 20)    /* Acid breath                  */
#define ITEM_NOXIOUS       (1 << 21)    /* Gas breath                   */
#define ITEM_BROKEN        (1 << 22)    /* Item is broken - not usable  */
#define ITEM_LIMITED       (1 << 23)    /* Item is limited, cannot be loaded */
#define ITEM_ANTI_DROW     (1 << 24)
#define ITEM_ANTI_MINOTAUR (1 << 25)
#define ITEM_ANTI_CENTAUR  (1 << 26)
#define ITEM_ANTI_ELF      (1 << 27)
#define ITEM_NOSHOW        (1 << 28)
#define ITEM_NOSUN         (1 << 29)
#define ITEM_THROWABLE     (1 << 30)

#define NUM_EXTRA_BITS     31

/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0       /* No effect                    */
#define APPLY_STR               1       /* Apply to strength            */
#define APPLY_DEX               2       /* Apply to dexterity           */
#define APPLY_INT               3       /* Apply to constitution        */
#define APPLY_WIS               4       /* Apply to wisdom              */
#define APPLY_CON               5       /* Apply to constitution        */
#define APPLY_CHA               6       /* Apply to charisma            */
#define APPLY_WILL              7       /* Apply to will                */
#define APPLY_LEVEL             8       /* Reserved                     */
#define APPLY_AGE               9       /* Apply to age                 */
#define APPLY_CHAR_WEIGHT      10       /* Apply to weight              */
#define APPLY_CHAR_HEIGHT      11       /* Apply to height              */
#define APPLY_MANA             12       /* Apply to max mana            */
#define APPLY_HIT              13       /* Apply to max hit points      */
#define APPLY_MOVE             14       /* Apply to max move points     */
#define APPLY_GOLD             15       /* Reserved                     */
#define APPLY_EXP              16       /* Reserved                     */
#define APPLY_AC               17       /* Apply to Armor Class         */
#define APPLY_HITROLL          18       /* Apply to hitroll             */
#define APPLY_DAMROLL          19       /* Apply to damage roll         */
#define APPLY_SAVING_PARA      20       /* Apply to save throw: paralz  */
#define APPLY_SAVING_ROD       21       /* Apply to save throw: rods    */
#define APPLY_SAVING_PETRI     22       /* Apply to save throw: petrif  */
#define APPLY_SAVING_BREATH    23       /* Apply to save throw: breath  */
#define APPLY_SAVING_SPELL     24       /* Apply to save throw: spells  */
#define APPLY_SAVING_COLD      25       /* Apply to save throw: cold    */
#define APPLY_SAVING_FIRE      26       /* Apply to save throw: fire    */
#define APPLY_SAVING_ACID      27       /* Apply to save throw: acid    */
#define APPLY_SAVING_ELEC      28       /* Apply to save throw: elec    */
#define APPLY_AFFECTS          29       /* Apply special object affects */
#define APPLY_HIT_REGEN        30
#define APPLY_MANA_REGEN       31
#define APPLY_MOVE_REGEN       32

#define NUM_APPLIES            33

/* ammo flags */
#define AMMO_UNKNOWN            0
#define AMMO_ARROW              1
#define AMMO_BOLT               2
#define AMMO_STONE              3

#define NUM_AMMO_TYPES          4
/* trap flags */
#define TRAP_DAM_UNDEFINED      0
#define TRAP_DAM_PIERCE         1
#define TRAP_DAM_SLASH          2
#define TRAP_DAM_BLUNT          3
#define TRAP_DAM_FIRE           4
#define TRAP_DAM_COLD           5
#define TRAP_DAM_ACID           6
#define TRAP_DAM_ELEC           7
#define TRAP_DAM_SLEEP          8
#define TRAP_DAM_TELEPORT       9
#define TRAP_DAM_POISON        10
#define TRAP_DAM_DISEASE       11
#define TRAP_DAM_SPELL         12

#define NUM_TRAP_TYPES         13
/* trap triggers */
#define TRIG_UNDEFINED          0
#define TRIG_ENTER_ROOM         1
#define TRIG_LEAVE_ROOM         2
#define TRIG_GET_OBJ            3
#define TRIG_DROP_OBJ           4
#define TRIG_CLOSE_DOOR         5
#define TRIG_OPEN_DOOR          6
#define NUM_TRIGGERS            7

/* Container flags - value[1] */
#define CONT_CLOSEABLE      (1 << 0)    /* Container can be closed      */
#define CONT_PICKPROOF      (1 << 1)    /* Container is pickproof       */
#define CONT_CLOSED         (1 << 2)    /* Container is closed          */
#define CONT_LOCKED         (1 << 3)    /* Container is locked          */


/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15
#define LIQ_URINE      16
#define LIQ_FECES      17

#define NUM_LIQ_TYPES  18

/* Default vnums for newbie items */
#define OBJ_VNUM_DEFAULT_LIGHT    1
#define OBJ_VNUM_DEFAULT_DAGGER   2
#define OBJ_VNUM_DEFAULT_VEST     3
#define OBJ_VNUM_DEFAULT_BELT     4
#define OBJ_VNUM_DEFAULT_LEGGINGS 5
#define OBJ_VNUM_DEFAULT_PACK     6
#define OBJ_VNUM_DEFAULT_FOOD     7
#define OBJ_VNUM_DEFAULT_DRINK    8
#define OBJ_VNUM_DEFAULT_INSTR    9
#define OBJ_VNUM_DEFAULT_MAP      3062
#define OBJ_VNUM_DEFAULT_MACE     90
#define OBJ_VNUM_RECALL           3052
#define OBJ_VNUM_POOLOBLOOD       98

/* other miscellaneous defines *******************************************/


/* Player conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2
#define TIRED        3


/* Sun state for weather_data */
#define SUN_DARK        0
#define SUN_RISE        1
#define SUN_LIGHT       2
#define SUN_SET         3
#define MOON_RISE       4
#define MOON_SET        5
#define MOON_CRESCENT   6
#define MOON_HALF       7
#define MOON_3QUARTERS  8
#define MOON_FULL       9
#define NEW_MOON        10


/* Sky conditions for weather_data */
#define SKY_HEATWAVE     0
#define SKY_SIZZLING     1
#define SKY_SUNNY        2
#define SKY_CLEAR        3
#define SKY_PARTCLOUD    4
#define SKY_CLOUDY       5
#define SKY_RAINING      6
#define SKY_POURING      7
#define SKY_THUNDERSTORM 8
#define SKY_TORNADO      9
#define SKY_SNOWING      10
#define SKY_SNOWSTORM    11
#define SKY_BLIZZARD     12


/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5

/* PC body parts */
#define BPT_BODY        0        /* Player's Body       */
#define BPT_LEG_L       1        /* Player's Left Leg   */
#define BPT_LEG_R       2        /* Player's Right Leg  */
#define BPT_ARM_L       3        /* Player's Left Arm   */
#define BPT_ARM_R       4        /* Player's Right Arm  */
#define BPT_FOOT_L      5        /* Player's Left Foot  */
#define BPT_FOOT_R      6        /* Player's Right Foot */
#define BPT_HAND_L      7        /* Player's Left Hand  */
#define BPT_HAND_R      8        /* Player's Right Hand */
#define BPT_HEAD        9        /* Player's Head       */



/* other #defined constants **********************************************/

#define MAX_DAMAGE      150

/*
 * **DO**NOT** blindly change the number of levels in your MUD merely by
 * changing these numbers and without changing the rest of the code to match.
 * Other changes throughout the code are required.  See coding.doc for
 * details.
 */
#define LVL_CREATOR     50
#define LVL_IMPL        49
#define LVL_SUP         48
#define LVL_GRGOD       47
#define LVL_GOD         46
#define LVL_DEITY       45
#define LVL_BUILDER     44
#define LVL_IMMORT      43
#define MAX_MORT_LEVEL  42

#define IS_IMMORT(ch)   (GET_LEVEL(ch) > MAX_MORT_LEVEL)
#define LVL_FREEZE      LVL_GRGOD

#define NUM_OF_DIRS     10      /* number of directions in a room (nsewud) */
#define NUM_LIMITEDS    5       /* number of limited objs the char can save with */

#define OPT_USEC        100000  /* 10 passes per second */
#define PASSES_PER_SEC  (1000000 / OPT_USEC)
#define RL_SEC          * PASSES_PER_SEC

#define PULSE_ZONE       (10 RL_SEC)
#define PULSE_MOBILE     (10 RL_SEC)
#define PULSE_VIOLENCE   (2 RL_SEC)
#define PULSE_EVENTS     (1 RL_SEC)
#define PULSE_SPC_UPDATE (15 RL_SEC)
#define PULSE_RIVER      (1 RL_SEC)
#define PULSE_TELEPORT   (1 RL_SEC)

#define SMALL_BUFSIZE           1024
#define LARGE_BUFSIZE           (12 * 1024)
#define GARBAGE_SPACE           32

#define MAX_STRING_LENGTH       65536
#define MAX_INPUT_LENGTH        512     /* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH    1024    /* Max size of *raw* input */
#define MAX_MESSAGES            40
#define MAX_NAME_LENGTH         20  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_PWD_LENGTH          12  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH        80  /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH             30  /* Used in char_file_u *DO*NOT*CHANGE* */
#define EXDSCR_LENGTH           1024/* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TONGUE              3   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS              1700/* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT              32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT          6 /* Used in obj_file_elem *DO*NOT*CHANGE* */
#define MAX_COLOR_LIST         19

#define MAX_MATERIAL_LIST      23
#define MAX_MATERIALS          23
#define MAT_MAGICAL            20

/**********************************************************************
* Structures                                                          *
**********************************************************************/


typedef signed char             sbyte;
typedef unsigned char           ubyte;
typedef signed short int        sh_int;
typedef unsigned short int      ush_int;
typedef signed long int         ln_int;      /* TD 5/25/95 SPM */
typedef unsigned long int       uln_int;     /* TD 5/25/95 SPM */
typedef long long int           suprlong;    /* 64 bit storage */
#ifdef __cplusplus
#ifndef bool
enum bool {FALSE, TRUE};
#endif
#else
typedef char			bool;
#endif
typedef char                    byte;

typedef ln_int  room_num;
typedef ln_int  rnum;
typedef ln_int  obj_num;
typedef ln_int  mob_num;

/* Event structure for delayed events */
struct event_data {
   struct char_data *ch;            /* char this event happens to */
   int    cmd;                      /* command number             */
   char   *args;                    /* typed in args              */
   int    count;                    /* pulses to event completion */
   struct event_data *next;         /* next command event in list */
};

struct guild_type {
   int  number;            /* Guild's UNIQUE ID Number      */
   long restrictions;      /* Bitvector of restrictions     */
   char *name;             /* Name of Guild (string)        */
   char *petitioners[20];  /* Pointer to strings            */
   char *leadersname;      /* Leader's (Player's) Name      */
   char *xlvlname;         /* Name of Leader    - Prefect   */
   char *hlvlname;         /* Name of High Lvls - Champions */
   char *mlvlname;         /* Name of Med Lvls  - Disciples */
   char *blvlname;         /* Name of Low Lvls  - Initiates */
   char *member_look_str;  /* What you see when you look at a member */
   long guild_entr_room;   /* VNUM of Guild Entrance Room   */
   long guardian_mob_vn;   /* VNUM of Guardian MOB          */
   int  direction;         /* The direction to block of ^   */
   struct guild_type *next;
};

/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
   char *keyword;                 /* Keyword in look/examine          */
   char *description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};

/* Used in Opinions */
struct char_list {
  struct char_data *op_ch;
  char   name[50];
  struct char_list *next;
};

typedef struct {
       struct char_list  *clist;
       int    sex;   /*number 1=male,2=female,3=both,4=neut,5=m&n,6=f&n,7=all*/
       int    race;  /*number */
       int    vnum;  /* # */
       int    evil;  /* align < evil = attack */
       int    good;  /* align > good = attack */
}  Opinion;

/* object-related structures ******************************************/

struct QuestItem {
  obj_num item;
  char    *where;
};


/* object flags; used in obj_data */
struct obj_flag_data {
   long value[10];      /* Values of the item (see list)    */
   byte type_flag;      /* Type of item                     */
   int  wear_flags;     /* Where you can wear it            */
   long extra_flags;    /* If it hums, glows, etc.          */
   int  weight;         /* Weigt what else                  */
   int  cost;           /* Value when sold (gp.)            */
   int  cost_per_day;   /* Cost to keep pr. real day        */
   int  timer;          /* Timer for object                 */
   long bitvector;      /* To set chars bits                */
};


/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte  location;     /* Which ability to change (APPLY_XXX) */
   sbyte modifier;     /* How much it changes by              */
};

/* used in random material process */
struct material_type {
   char *name;
   int  sturdiness;
   int  value_mod;
   int  bonus_type;
   int  bonus_mod;
};

/* ================== Memory Structure for Objects ================== */
struct obj_data {
   obj_num  item_number;        /* Where in data-base                   */
   room_num in_room;            /* In what room -1 when conta/carr      */

   struct obj_flag_data obj_flags;/* Object information               */
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects */

   sh_int sector;                 /* For LARGE rooms - TD 9/16/95     */
   char *name;                    /* Title of object :get etc.        */
   char *description;             /* When in room                     */
   char *short_description;       /* when worn/carry/in cont.         */
   char *action_description;      /* What to write when used          */
   struct extra_descr_data *ex_description; /* extra descriptions     */
   struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
   struct char_data *worn_by;     /* Worn by?                         */
   sh_int worn_on;                /* Worn where?                      */

   struct obj_data *in_obj;       /* In what object NULL when none    */
   struct obj_data *contains;     /* Contains objects                 */

   struct obj_data *next_content; /* For 'contains' lists             */
   struct obj_data *next;         /* For the object list              */
    char *corpsename;
};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files                */
struct obj_file_elem {
   obj_num item_number;
   sh_int  locate;                  /* Item's location */
   long    value[10];
   long    extra_flags;
   int     weight;
   int     timer;
   long    bitvector;
   struct  obj_affected_type affected[MAX_OBJ_AFFECT];
};


/* header block for rent files.  BEWARE: Changing it will ruin rent files  */
struct rent_info {
   int    time;
   int    rentcode;
   int    net_cost_per_diem;
   ln_int gold;
   ln_int account;
   int    nitems;
   int    coins[5];
   int  spare0;
   int  spare1;
   int  spare2;
   int  spare3;
   int  spare4;
   int  spare5;
   int  spare6;
   int  spare7;
};
/* ======================================================================= */


/* room-related structures ************************************************/

struct large_room_data {
  unsigned int flags[10];
};

struct room_direction_data {
   char *general_description;       /* When look DIR.                   */

   char *keyword;               /* for open/close                       */

   sh_int   exit_info;          /* Exit info                            */
   obj_num  key;                /* Key's number (-1 for no key)         */
   room_num to_room;            /* Where direction leads (NOWHERE)      */
   room_num to_room_vnum;       /* the vnum of the room. Used for OLC   */
};


/* ================== Memory Structure for room ======================= */
struct room_data {
   room_num number;             /* Rooms number (vnum)                */
   sh_int   zone;               /* Room zone (for resetting)          */
   sh_int   continent;          /* Which continent - TD 9/16/95       */
   int      sector_type;        /* sector type (move/hide)            */

   int    river_dir;            /* dir of flow on river               */
   int    river_speed;          /* speed of flow on river             */

   int  tele_time;              /* time to a teleport                 */
   long tele_targ;              /* target room of a teleport          */
   long tele_mask;              /* flags for use with teleport        */
   int  tele_cnt;               /* countdown teleports                */

   unsigned char moblim;        /* # of mobs allowed in room.         */

   char *name;                  /* Rooms name 'You are ...'           */
   char *description;           /* Shown when entered                 */
   struct extra_descr_data *ex_description; /* for examine/look       */
   struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
   long   room_flags;           /* DEATH,DARK ... etc                 */

   byte light;                  /* Number of lightsources in room     */
   SPECIAL(*func);

   struct obj_data *contents;   /* List of items in room              */
   struct char_data *people;    /* List of NPC / PC in room           */
   struct large_room_data *large;  /* Special for large rooms - TD 9/16  */
};
/* ====================================================================== */


/* char-related structures ************************************************/

struct language {
   sh_int basic_known;
   long   dialects_known;
};

/*
 * infobar1 (new structure to store infobar stats)
 * notes: this is the new method for updating the infobar,
 *        resulting in less code and fewer bugs.
 */
struct char_infobar_data {
  long          hit, end, mana, maxhit, maxend, maxmana, inroom, opphit;
  long          gold, exp, pra, align;
  long          level;
  int           ac;
  int           hitbon, dambon;
  int           hunger, thirst, drunk, tired;
  int           race;
  int           bank, weight;
  int           exits;  /* will require additional coding */
};

/* memory structure for characters */
struct memory_rec_struct {
   long id;
   struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;

/* MOB Scripts structs */
/*
 * Trigger types for repetitive loops, command etc.
 */
#define TRIG_COMMAND          0   /* When a player types x command        */
#define TRIG_EACH_PULSE       1   /* Increments every pulse               */
#define TRIG_COMBAT           2   /* Every combat pulse                   */
#define TRIG_TICK_PULSE       3   /* Every tick (one mud-hour)            */
#define TRIG_BORN             4   /* Happens upon creation                */
#define TRIG_GETS             5   /* Happens when a mob gets/is given     */
#define TRIG_SAY              6   /* Happens when a mob sees a string     */
#define TRIG_TELL             7   /* Happens when a mob sees a string     */
#define TRIG_KILLS            8   /* Happens when a mob kills another     */
#define TRIG_DIES             9   /* Happens when the mob is killed       */
#define TRIG_ENTER           10   /* Happens on arrival (for each person) */
#define TRIG_MOVES           11   /* Happens whenever a mobile moves      */

/*
 * Internally used bits for triggers.
 */
#define SCRIPT_ADVANCE       0x00000001 /* Its OK to go to next command set  */
#define SCRIPT_HALT          0x00000002 /* current=NULL & disallow trig      */

struct script_data
{
   struct script_data * next;
   char *               command;
};

struct trigger_data
{
   struct trigger_data *next;
   struct script_data  *script;
   struct script_data  *current;
   sh_int               tracer;
   sh_int               waiting;
   sh_int               trigger_type;
   int                  bits;
   char *               keywords;
   char *               name;
};

struct variable_data
{
   struct variable_data *next;
   char *                name;
   char *                value;
};


/* MOBProgram foo */
struct mob_prog_act_list {
  struct mob_prog_act_list *next;
  char   *buf;
  struct char_data *ch;
  struct obj_data *obj;
  void   *vo;
};

typedef struct mob_prog_act_list MPROG_ACT_LIST;

struct mob_prog_data {
  struct mob_prog_data *next;
  int    type;
  char   *arglist;
  char   *comlist;
};

typedef struct mob_prog_data MPROG_DATA;

extern bool MOBTrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024

/* end of MOBProg foo */

/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
   byte   seconds, minutes,  hours, day, month;
   sh_int year;
};


/* These data contain information about a players time data */
struct time_data {
   time_t birth;    /* This represents the characters age                */
   time_t logon;    /* Time of the last logon (used to calculate played) */
   ln_int played;   /* This is the total accumulated time played in secs */
};


/* general player-related info, usually PC's and NPC's */
struct char_player_data {
   char passwd[MAX_PWD_LENGTH+1]; /* character's password      */
   char *name;         /* PC / NPC s name (kill ...  )         */
   char *short_descr;  /* for NPC 'actions'                    */
   char *long_descr;   /* for 'look'                           */
   char *description;  /* Extra descriptions                   */
   char *title;        /* PC / NPC's title                     */
   char *poly_descr;   /* Polymorphed description              */
   char *xtra_descr;   /* Xtra-description for later use       */
   char *room_descr;   /* Room description                     */
   char *keywords;     /* Keyword for character                */
   byte sex;           /* PC / NPC's sex                       */
   byte unused1[3];    /* PC / NPC's Invitations               */
   byte race;          /* PC / NPC's race                      */
   byte assocs[5];     /* PC / NPC Rel, Guild, GuildLev, Unused*/
   byte regens[5];     /* PC / NPC Regen Rate (HP/MANA/MV)     */
   byte level;         /* PC / NPC's level                     */
   byte trust;         /* PC's Trust Level                     */
   int  hometown;      /* PC s Hometown (zone) - index into array */
   sh_int race_mod;    /* race mod for age                     */
   struct time_data time;  /* PC's AGE in days                 */
   ubyte weight;       /* PC / NPC's weight                    */
   ubyte height;       /* PC / NPC's height                    */

   struct language speaks[MAX_LANGUAGES]; /* PC/NPC languages  */
};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data {
   sbyte str;
   sbyte str_add;      /* 000 - 100 if strength 18             */
   sbyte intel;
   sbyte wis;
   sbyte dex;
   sbyte con;
   sbyte cha;
   sbyte will;
};


/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
   int mana;
   int max_mana;     /* Max move for PC/NPC                        */
   int hit;
   int max_hit;      /* Max hit for PC/NPC                      */
   int move;
   int max_move;     /* Max move for PC/NPC                     */

   sh_int armor[8];     /* Internal -100..100, external -10..10 AC */
   ln_int gold;         /* Money carried                           */
   ln_int bank_gold;    /* Gold the char has in a bank account     */
   ln_int coins[5];     /* Plat, Gold, Elec, Silv, and Copper Coins*/
   ln_int exp;          /* The experience of the player            */

   sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
   sbyte damroll;       /* Any bonus or penalty to the damage roll */
};


/*
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct char_special_data_saved {
   sbyte handedness;            /* Character's handedness               */
   sbyte battle_stance;         /* player's stance when fighting        */
   sbyte perm_align;            /* Permanent Alignment                  */
   int   alignment;             /* +-1000 for alignments                */
   long  idnum;                 /* player's idnum; -1 for mobiles       */
   suprlong act;                /* act flag for NPC's; player flag for PC's */
   suprlong act2;               /* act2 flag for NPC's; player flag for PC's */

   suprlong affected_by;        /* Bitvector for spells/skills affected by */
   suprlong affected_by2;       /* Bitvector for spells/skills affected by2 */
   long disabilities;           /* PC's disabilities                    */
   long abilities;              /* PC's abilities                       */
   sh_int apply_saving_throw[9]; /* Saving throw (Bonuses)              */
};


/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
   struct char_data *fighting;  /* Opponent                             */
   struct char_data *hunting;   /* Char hunted by this char             */
   struct char_data *mounted_on;  /* Char is mounted on what?           */
   struct char_data *ridden_by;   /* Char is ridden by what?            */
   struct char_data *shadowing;   /* Char is shadowing who?             */
   struct char_data *shadowed_by; /* Char is shadowed by who?           */
   struct obj_data  *using;       /* Sitting, sleeping, resting on      */
   struct obj_data  *lodged[12];  /* 12 pointers to logded items        */

   byte position;               /* Standing, fighting, sleeping, etc.   */

   int  carry_weight;           /* Carried weight                       */
   byte carry_items;            /* Number of items carried              */
   int  timer;                  /* Timer for update                     */

   long   item_aff_by;            /* Bitvector for item spells affected */
   sbyte  num_fols;               /* Numer of followers                 */

   char   *email;                 /* email address */
   struct char_special_data_saved saved; /* constants saved in plrfile  */
};


/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
struct player_special_data_saved {
   byte     skills[MAX_SKILLS+1];  /* array of skills plus skill 0          */
   byte     curr_lang;             /* used to be spells_to_learn            */
   int      wimp_level;            /* Below this # of hit points, flee!     */
   int      spells_to_learn;       /* How many spells you can learn this lvl*/
   byte     freeze_level;          /* Level of god who froze char, if any   */
   sh_int   invis_level;           /* level of invisibility                 */
   room_num load_room;             /* Which room to place char in           */
   suprlong pref;                  /* preference flags for PC's.            */
   suprlong pref2;                 /* preference flags for PC's.            */
   ubyte    bad_pws;               /* number of bad password attemps        */
   sbyte    conditions[5];         /* Drunk, full, thirsty, tired           */
   ubyte    fate_pts;              /* # of times the PC died this level.    */
   ubyte    wounds;                /* PC's current wound level.             */
   ubyte    addictions[5];         /* PC's addicted level to diff drugs     */
   long     body_parts[12];        /* Condition of PC's Body Parts.         */

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */

   ubyte spare0;
   ubyte spare1;
   ubyte spare2;
   ubyte spare3;
   ubyte spare4;
   ubyte spare5;
   int   olc_zone;                /* Saved Zone # Editing */
   int spare7;
   int spare8;
   int spare9;
   int spare10;
   int spare11;
   int spare12;
   int spare13;
   int spare14;
   int spare15;
   int spare16;
   long spare17;
   long spare18;
   long spare19;
   long spare20;
   long spare21;
   suprlong spare22;
   suprlong spare23;
   suprlong spare24;
   suprlong spare25;
};

/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs and the portion
 * of it labelled 'saved' is saved in the playerfile.  This structure can
 * be changed freely; beware, though, that changing the contents of
 * player_special_data_saved will corrupt the playerfile.
 */
struct player_special_data {
   struct player_special_data_saved saved;

   char   *poofin;              /* Description on arrival of a god.     */
   char   *poofout;             /* Description upon a god's exit.       */
   struct alias *aliases;       /* Character's aliases                  */
   long   last_tell;            /* idnum of last tell from              */
   void   *last_olc_targ;       /* olc control                          */
   int    last_olc_mode;        /* olc control                          */
   int    killed_mobs[2];       /* mobs killed alone and in group       */
   int    exp_gained;           /* amount of exp gained in this session */
   float  rel_contrib;          /* contrib to gods potency              */
   int    screensize;           /* PC's screensize                      */
};


/* Specials used by NPCs, not PCs */
struct mob_special_data {
   byte last_direction;     /* The last direction the monster went     */
   int  attack_type;        /* The Attack Type Bitvector for NPC's     */
   byte default_pos;        /* Default position for NPC                */
   memory_rec *memory;      /* List of attackers to remember           */
   byte damnodice;          /* The number of damage dice's             */
   byte damsizedice;        /* The size of the damage dice's           */
   int  wait_state;         /* Wait state for bashed mobs              */
   char mob_type;           /* Mob Type, used with OLC (S, E, A)       */
   struct char_data *mesg_for;    /* Char has a mesg for this PC/NPC   */
   struct char_data *mesg_about;  /* Char has a mesg about this PC/NPC */
   char *sounds;            /* Sound that the monster makes (in room)  */
   char *distant_snds;      /* Sound that the monster makes (other)    */
};


/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   sbyte  modifier;      /* This is added to apropriate ability     */
   byte   location;      /* Tells which ability to change(APPLY_XXX)*/
   suprlong bitvector;   /* Tells which bits to set (AFF_XXX)       */

   struct affected_type *next;
};


/* Structure used for chars following other chars */
struct follow_type {
   struct char_data   *follower;
   struct follow_type *next;
};


/* ================== Structure for player/non-player ===================== */
struct char_data {
   int      pfilepos;                    /* playerfile pos                */
   mob_num  nr;                          /* Mob's rnum                    */
   room_num in_room;                     /* Location (real room number)   */
   room_num was_in_room;                 /* location for linkdead people  */

   int term;                             /* Terminal Type                 */
   int size_w, size_h;                   /* Size of Terminal              */
   /* basic crap I ripped from SillyMUD but haven't used yet */
   unsigned immune;                      /* Immunities                         */
   unsigned M_immune;                    /* Meta Immunities                    */
   unsigned susc;                        /* susceptibilities                   */
   byte     sector;                      /* which part of a large room am i in?*/
   int      generic;                     /* generic int */
   int      commandp;                    /* command poitner for scripts */
   int      waitp;                       /* waitp for scripts           */
   int      commandp2;                   /* place-holder for gosubs, etc. */
   int      script;
   unsigned short hatefield;
   unsigned short fearfield;
   Opinion hates;
   Opinion fears;
   sh_int  persist;
   int     old_room;
   void   *act_ptr;            /* numeric argument for the mobile actions */

   struct char_player_data  player;        /* Normal data                   */
   struct char_infobar_data infobar;       /* Infobar stats                 */
   struct char_ability_data real_abils;    /* Abilities without modifiers   */
   struct char_ability_data aff_abils;     /* Abils with spells/stones/etc  */
   struct char_point_data   points;        /* Points                        */
   struct char_special_data char_specials;      /* PC/NPC specials        */
   struct player_special_data *player_specials; /* PC specials            */
   struct mob_special_data  mob_specials;       /* NPC specials           */

   struct affected_type *affected;       /* affected by what spells       */
   struct affected_type *affected2;      /* affected by what bad spells   */
   struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

   struct obj_data *carrying;            /* Head of list                  */
   struct descriptor_data *desc;         /* NULL for mobiles              */

   struct char_data *next_in_room;     /* For room->people - list         */
   struct char_data *next;             /* For either monster or ppl-list  */
   struct char_data *next_fighting;    /* For fighting list               */

   struct follow_type *followers;      /* List of chars followers       */
   struct char_data   *master;         /* Who is char following?        */

   /* mob script stuff */
   struct trigger_data  *triggers;     /* for scripts                   */
   struct variable_data *variables;    /* for scripts                   */

   MPROG_ACT_LIST *mpact;
   int mpactnum;
};
/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile               */
struct char_file_u {
   /* char_player_data */
   char name[MAX_NAME_LENGTH+1];
   char description[EXDSCR_LENGTH];
   char title[MAX_TITLE_LENGTH+1];
   /* Extra space for descriptions */
   char poly_descr[MAX_TITLE_LENGTH+1];
   char xtra_descr[MAX_TITLE_LENGTH+1];
   /* For gods poofin/outs */
   char spoofin[MAX_TITLE_LENGTH+1];
   char spoofout[MAX_TITLE_LENGTH+1];
   /* For PC's room names */
   char keywords[70];
   char room_descr[MAX_TITLE_LENGTH+1];
   /* the normal stuff */
   byte sex;
   byte race;
   byte invitations[3];     /* Invitations to 3 Factions    */
   byte assocs[5];          /* Rel, Guild, GuildLev, unused */
   byte assocs2[5];         /* Unused at this time          */
   byte level;
   sh_int race_mod;     /* race modifier for age */
   sh_int hometown;
   time_t birth;        /* Time of birth of character     */
   long   played;       /* Number of secs played in total */
   ubyte  weight;
   ubyte  height;

   struct language speaks[MAX_LANGUAGES];   /* Languages the char knows */
   char   pwd[MAX_PWD_LENGTH+1];            /* character's password */

   struct char_special_data_saved    char_specials_saved;
   struct player_special_data_saved  player_specials_saved;
   struct char_ability_data          abilities;
   struct char_point_data            points;
   struct affected_type              affected[MAX_AFFECT];
   struct affected_type              affected2[MAX_AFFECT];

   time_t last_logon;                /* Time (in secs) of last logon */
   char email[MAX_TITLE_LENGTH+1];   /* PC's email address */
   char host[HOST_LENGTH+1];         /* host of last logon */
   char username[MAX_NAME_LENGTH+1]; /* username of player if known */
};
/* ====================================================================== */


/* descriptor-related structures ******************************************/


struct txt_block {
   char *text;
   int aliased;
   struct txt_block *next;
};


struct txt_q {
   struct txt_block *head;
   struct txt_block *tail;
};


struct descriptor_data {
   int  descriptor;             /* file descriptor for socket           */
   char host[HOST_LENGTH+1];    /* hostname                             */
   byte bad_pws;                /* number of bad pw attemps this login  */
   byte idle_tics;              /* tics idle at password prompt         */
   int  connected;              /* mode of 'connectedness'              */
   int  wait;                   /* wait for how many loops              */
   int  desc_num;               /* unique num assigned to desc          */
   time_t login_time;           /* when the person connected            */
   char *showstr_head;          /* for keeping track of an internal str */
   char **showstr_vector;       /* for paging through texts             */
   int  showstr_count;          /* number of pages to page through      */
   int  showstr_page;           /* which page are we currently showing? */
   char **str;                  /* for the modify-str system            */
   int  max_str;                /*              -                       */
   long mail_to;                /* name for mail system                 */
   int  prompt_mode;            /* control of prompt-printing           */
   char inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input           */
   char last_input[MAX_INPUT_LENGTH]; /* the last input                 */
   char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer         */
   char *output;                /* ptr to the current output buffer     */
   int  bufptr;                 /* ptr to end of current output         */
   int  bufspace;               /* space left in the output buffer      */
   struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
   struct txt_q input;          /* q of unprocessed input               */
   struct char_data *character; /* linked to char                       */
   struct char_data *original;  /* original char if switched            */
   struct descriptor_data *snooping; /* Who is this char snooping       */
   struct descriptor_data *snoop_by; /* And who is snooping this char   */
   struct descriptor_data *next; /* link to next descriptor             */
   bool   ansi;                  /* whether or not plr has ANSI term    */
   /* OLC stuff below */
   struct olc_data *olc;         /* OLC info - defined in olc.h         */
};


/* other miscellaneous structures ***************************************/


struct msg_type {
   char *attacker_msg;  /* message to attacker */
   char *victim_msg;    /* message to victim   */
   char *room_msg;      /* message to room     */
};


struct message_type {
   struct msg_type die_msg;     /* messages when death                  */
   struct msg_type miss_msg;    /* messages when miss                   */
   struct msg_type hit_msg;     /* messages when hit                    */
   struct msg_type god_msg;     /* messages when hit on god             */
   struct message_type *next;   /* to next messages of this kind.       */
};


struct message_list {
   int  a_type;                 /* Attack type                          */
   int  number_of_attacks;      /* How many attack messages to chose from. */
   struct message_type *msg;    /* List of messages.                    */
};


struct dex_skill_type {
   sh_int p_pocket;
   sh_int p_locks;
   sh_int traps;
   sh_int sneak;
   sh_int hide;
};


struct dex_app_type {
   sh_int reaction;
   sh_int miss_att;
   sh_int defensive;
};


struct str_app_type {
   sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
   sh_int todam;    /* Damage Bonus/Penalty                */
   sh_int carry_w;  /* Maximum weight that can be carrried */
   sh_int wield_w;  /* Maximum weight that can be wielded  */
};


struct wis_app_type {
   byte bonus;       /* how many practices player gains per lev */
};


struct int_app_type {
   byte learn;       /* how many % a player learns a spell/skill */
};


struct cha_app_type {
   byte num_fols;      /* how many followers a player can have, this  */
};

struct con_app_type {
   sh_int hitp;
   sh_int shock;
};


struct weather_data {
   int  humidity;       /* How humid is it? In percentage.       */
   int  change;         /* How fast and what way does it change. */
   int  sky;            /* How is the sky.                       */
   int  sunlight;       /* And how much sun.                     */
};


/* element in monster and object index-tables   */
struct index_data {
   ln_int  vnumber;    /* virtual number of this mob/obj                */
   ln_int  number;     /* number of existing units of this mob/obj      */
   char    *script_fn; /* Script filename                               */
   struct  trigger_data *triggers;   /* script triggers                 */
   struct  variable_data *variables; /* script variables                */
   int     progtypes;  /* program types for MOBProg                     */
   MPROG_DATA *mobprogs;         /* programs for MOBProg                */
   SPECIAL(*func);
};

#endif
