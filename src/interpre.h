/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  INTERPRE.H                                   Based on CircleMUD *
*  Usage: Header: Public procs, macro defs, subcommand defines            *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#ifndef __INTERPRE_H__
#define __INTERPRE_H__


#define ACMD(name)  \
   void (name)(struct char_data *ch, char *argument, int cmd, int subcmd)

#define CMD_NAME (cmd_info[cmd].command)
#define CMD_IS(cmd_name) (!strcmp(cmd_name, cmd_info[cmd].command))
#define IS_MOVE(cmdnum) (cmdnum >= 1 && cmdnum <= 6)

void    command_interpreter(struct char_data *ch, char *argument);
int     search_block(char *arg, char **list, bool exact);
char    lower( char c );
char    *one_argument(char *argument, char *first_arg);
char    *one_word(char *argument, char *first_arg);
char    *any_one_arg(char *argument, char *first_arg);
char    *any_two_args(char *argument, char *first, char *second);
char    *two_arguments(char *argument, char *first_arg, char *second_arg);
char    *three_arguments(char *argument, char *first_arg, char *second_arg, char *third_arg);
int     fill_word(char *argument);
void    half_chop(char *string, char *arg1, char *arg2);
void    nanny(struct descriptor_data *d, char *arg);
int     is_abbrev(char *arg1, char *arg2);
int     is_number(char *str);
int     find_command(char *command);
void    skip_spaces(char **string);
char    *delete_doubledollar(char *string);
/* for the event handler  */
int add_event(struct char_data *ch, char *args, int cmd, int count);
int remove_event(struct char_data *ch);

/* for compatibility with 2.20: */
#define argument_interpreter(a, b, c) two_arguments(a, b, c)

struct command_info {
   char *command;
   byte minimum_position;
   void (*command_pointer)
   (struct char_data *ch, char * argument, int cmd, int subcmd);
   sh_int minimum_level;
   int  subcmd;
};

/* necessary for CMD_IS macro */
#ifndef __INTERPRE_C__
extern const struct command_info cmd_info[];
#endif

struct alias {
  char *alias;
  char *replacement;
  int type;
  struct alias *next;
};

#define ALIAS_SIMPLE    0
#define ALIAS_COMPLEX   1

#define ALIAS_SEP_CHAR  ';'
#define ALIAS_VAR_CHAR  '$'
#define ALIAS_GLOB_CHAR '*'

/*
 * SUBCOMMANDS
 *   You can define these however you want to, and the definitions of the
 *   subcommands are independent from function to function.
 */

/* directions */
#define SCMD_NORTH      1
#define SCMD_EAST       2
#define SCMD_SOUTH      3
#define SCMD_WEST       4
#define SCMD_UP         5
#define SCMD_DOWN       6
#define SCMD_NE         7
#define SCMD_NW         8
#define SCMD_SE         9
#define SCMD_SW        10

/* do_gen_ps */
#define SCMD_INFO       0
#define SCMD_HANDBOOK   1
#define SCMD_CREDITS    2
#define SCMD_NEWS       3
#define SCMD_WIZLIST    4
#define SCMD_POLICIES   5
#define SCMD_VERSION    6
#define SCMD_IMMLIST    7
#define SCMD_MOTD       8
#define SCMD_IMOTD      9
#define SCMD_CLEAR      10
#define SCMD_WHOAMI     11
#define SCMD_NEWBM      12                /* newbie message */

/* do_gen_tog */
#define SCMD_NOHASSLE   0
#define SCMD_BRIEF      1
#define SCMD_COMPACT    2
#define SCMD_NOTELL     3
#define SCMD_NOWISH     4
#define SCMD_DEAF       5
#define SCMD_NOGOSSIP   6
#define SCMD_NOOOC      7
#define SCMD_NOMUZAK    8
#define SCMD_NOWIZ      9
#define SCMD_QUEST      10
#define SCMD_ROOMFLAGS  11
#define SCMD_NOREPEAT   12
#define SCMD_HOLYLIGHT  13
#define SCMD_SLOWNS     14
#define SCMD_AUTOEXIT   15
#define SCMD_AUTOASS    16
#define SCMD_PKILLPROT  17
#define SCMD_AUTOLOOT   18
#define SCMD_AUTOGOLD   19
#define SCMD_NOSPAM     20
#define SCMD_INFOBAR    21
#define SCMD_SCOREBAR   22
#define SCMD_METER	23
#define SCMD_AFK        24

/* do_wizutil */
#define SCMD_REROLL     0
#define SCMD_PARDON     1
#define SCMD_NOTITLE    2
#define SCMD_SQUELCH    3
#define SCMD_FREEZE     4
#define SCMD_THAW       5
#define SCMD_UNAFFECT   6

/* do_spec_com */
#define SCMD_WHISPER    0
#define SCMD_ASK        1

/* do_gen_com */
#define SCMD_HOLLER     0
#define SCMD_SHOUT      1
#define SCMD_GOSSIP     2
#define SCMD_WISH       3
#define SCMD_OOC        4

/* do_shutdown */
#define SCMD_SHUTDOW    0
#define SCMD_SHUTDOWN   1

/* do_quit */
#define SCMD_QUI        0
#define SCMD_QUIT       1

/* do_date */
#define SCMD_DATE       0
#define SCMD_UPTIME     1

/* do_commands */
#define SCMD_COMMANDS   0
#define SCMD_SOCIALS    1
#define SCMD_WIZHELP    2

/* do_drop */
#define SCMD_DROP       0
#define SCMD_JUNK       1
#define SCMD_DONATE     2

/* do_gen_write */
#define SCMD_BUG        0
#define SCMD_TYPO       1
#define SCMD_IDEA       2
#define SCMD_TODO       3

/* do_gen_vfile */
#define SCMD_V_TODO     0
#define SCMD_V_BUGS     1
#define SCMD_V_IDEAS    2
#define SCMD_V_TYPOS    3
#define SCMD_V_SYSLOG   4
#define SCMD_V_QUOTES   5
#define SCMD_V_OQUOTE   6
#define SCMD_V_SHOWPLRS 7

/* do_look */
#define SCMD_LOOK       0
#define SCMD_READ       1
#define SCMD_PEEK       2

/* do_qcomm */
#define SCMD_QSAY       0
#define SCMD_QECHO      1

/* do_pour */
#define SCMD_POUR       0
#define SCMD_FILL       1

/* do_poof */
#define SCMD_POOFIN     0
#define SCMD_POOFOUT    1

/* do_hit */
#define SCMD_HIT        0
#define SCMD_MURDER     1

/* do_eat */
#define SCMD_EAT        0
#define SCMD_TASTE      1
#define SCMD_DRINK      2
#define SCMD_SIP        3

/* do_use */
#define SCMD_USE        0
#define SCMD_QUAFF      1
#define SCMD_RECITE     2

/* do_echo */
#define SCMD_ECHO       0
#define SCMD_EMOTE      1

/* do_gen_door */
#define SCMD_OPEN       0
#define SCMD_CLOSE      1
#define SCMD_UNLOCK     2
#define SCMD_LOCK       3
#define SCMD_PICK       4

/*. do_olc .*/
#define SCMD_OLC_REDIT          0
#define SCMD_OLC_OEDIT          1
#define SCMD_OLC_ZEDIT          2
#define SCMD_OLC_MEDIT          3
#define SCMD_OLC_SEDIT          4
#define SCMD_OLC_SAVEINFO       5
#define SCMD_OLC_GEDIT          6

/* do_tog_rel */
#define SCMD_AFFIRM             1
#define SCMD_RENOUNCE           2

/* for events */
#define SCMD_DELAY              0
#define SCMD_RESUME             1

/* for do_infobar */
#define SCMDB_REDRAW		1	/* external use */
#define SCMDB_CLEAR		2

#define SCMDB_HIT		3 	/* internal use */
#define SCMDB_MOV		4
#define SCMDB_MANA		5
#define SCMDB_EXITS		9
#define SCMDB_ROOMDESC		10
#define SCMDB_GOLD		11
#define SCMDB_EXP		12
#define SCMDB_AFF		13
#define SCMDB_PRA		14
#define SCMDB_LEV		15
#define SCMDB_AC		16
#define SCMDB_FIGHTUPDATE       17
#define SCMDB_ALIGN		18
#define SCMDB_HITBON		19
#define SCMDB_DAMBON		20
#define SCMDB_CONDITION		21
#define SCMDB_RACE		23
#define SCMDB_LEVEL		24
#define SCMDB_CARRYING		26
#define SCMDB_COMBAT		27
#define SCMDB_GET		28
#define SCMDB_TITLE		29
#define SCMDB_WEAR              30
#define SCMDB_NEWROOM           31
#define SCMDB_GAINLEVEL         32
#define SCMDB_ENDFIGHT          33
#define SCMDB_RESIZE		34	/* for external use */
#define SCMDB_GENUPDATE		40 	/* now the universal update command */

#endif
