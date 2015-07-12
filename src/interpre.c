/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  INTERPRE.C                                   Based on CircleMUD *
*  Usage: Parse user commands, search for specials, call ACMD functions   *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __INTERPRETER_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern char *motd;
extern char *imotd;
extern char *newbm;
extern char *background;
extern char *WELC_MESSG;
extern char *START_MESSG;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int    top_of_p_table;
extern int    game_restrict;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;

/* external functions */
void echo_on(struct descriptor_data *d);
void echo_off(struct descriptor_data *d);
void do_start(struct char_data *ch);
void init_char(struct char_data *ch);
int  create_entry(char *name);
int  special(struct char_data *ch, int cmd, char *arg);
int  isbanned(char *hostname);
int  Valid_Name(char *newname);
/* TD shit */
void read_aliases(struct char_data *ch);
extern struct race_data * races;
int parse_race(char arg);
int count_words(char *str);
int has_mail(long recipient);
void redit_parse(struct descriptor_data *d, char *arg);
void medit_parse(struct descriptor_data *d, char *arg);
void oedit_parse(struct descriptor_data *d, char *arg);
void zedit_parse(struct descriptor_data *d, char *arg);
void sedit_parse(struct descriptor_data *d, char *arg);
void gedit_parse(struct descriptor_data *d, char *arg);
void hedit_parse(struct descriptor_data *d, char *arg);

/* prototypes for all do_x functions. */
ACMD(do_action);
ACMD(do_advance);
ACMD(do_alias);
ACMD(do_assist);
ACMD(do_at);
ACMD(do_backstab);
ACMD(do_ban);
ACMD(do_bash);
ACMD(do_cast);
ACMD(do_color);
ACMD(do_commands);
ACMD(do_study);
ACMD(do_evaluate);
ACMD(do_copyover);
ACMD(do_credits);
ACMD(do_date);
ACMD(do_dc);
ACMD(do_diagnose);
ACMD(do_display);
ACMD(do_drink);
ACMD(do_drop);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_enter);
ACMD(do_equipment);
ACMD(do_examine);
ACMD(do_exit);
ACMD(do_exits);
ACMD(do_flee);
ACMD(do_follow);
ACMD(do_force);
ACMD(do_gecho);
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_write);
ACMD(do_get);
ACMD(do_give);
ACMD(do_gold);
ACMD(do_goto);
ACMD(do_grab);
ACMD(do_group);
ACMD(do_gsay);
ACMD(do_hcontrol);
ACMD(do_help);
ACMD(do_hide);
ACMD(do_hit);
ACMD(do_house);
ACMD(do_info);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_kick);
ACMD(do_kill);
ACMD(do_last);
ACMD(do_leave);
ACMD(do_levels);
ACMD(do_create);
ACMD(do_look);
ACMD(do_mpasound);
ACMD(do_mpjunk);
ACMD(do_mpecho);
ACMD(do_mpechoat);
ACMD(do_mpechoaround);
ACMD(do_mpcast);
ACMD(do_mpkill);
ACMD(do_mpmload);
ACMD(do_mpoload);
ACMD(do_mppurge);
ACMD(do_mpgoto);
ACMD(do_mpat);
ACMD(do_mptransfer);
ACMD(do_mpforce);
ACMD(do_move);
ACMD(do_not_here);
ACMD(do_offer);
ACMD(do_olc);
ACMD(do_olc_master);
ACMD(do_order);
ACMD(do_page);
ACMD(do_poofset);
ACMD(do_pour);
ACMD(do_spheres);
ACMD(do_purge);
ACMD(do_put);
ACMD(do_qcomm);
ACMD(do_quit);
ACMD(do_reboot);
ACMD(do_remove);
ACMD(do_reply);
ACMD(do_report);
ACMD(do_rescue);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_return);
ACMD(do_save);
ACMD(do_say);
ACMD(do_score);
ACMD(do_send);
ACMD(do_set);
ACMD(do_show);
ACMD(do_shutdown);
ACMD(do_sit);
ACMD(do_skillset);
ACMD(do_sleep);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_spec_comm);
ACMD(do_split);
ACMD(do_stand);
ACMD(do_stat);
ACMD(do_steal);
ACMD(do_switch);
ACMD(do_syslog);
ACMD(do_teleport);
ACMD(do_tell);
ACMD(do_time);
ACMD(do_title);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_trans);
ACMD(do_unban);
ACMD(do_ungroup);
ACMD(do_use);
ACMD(do_users);
ACMD(do_visible);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_wake);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_where);
ACMD(do_who);
ACMD(do_wield);
ACMD(do_wimpy);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wizutil);
ACMD(do_write);
ACMD(do_zreset);
ACMD(do_garrotte);        /* TD 03/17/96 */
ACMD(do_berserk);         /* TD 04/25/95 */
ACMD(do_bite);            /* TD 03/17/94 */
ACMD(do_circle);          /* TD 03/17/94 */
ACMD(do_claw);            /* TD 03/17/94 */
ACMD(do_gore);            /* TD 04/28/95 */
ACMD(do_behead);          /* Date? */
ACMD(do_mount);           /* TD 04/28/93 */
ACMD(do_palm);            /* TD 04/28/93 */
ACMD(do_peace);           /* TD 04/06/95 */
ACMD(do_pummel);          /* TD 04/28/95 */
ACMD(do_remort);          /* TD 06/03/95 */
ACMD(do_sacrifice);       /* TD 04/08/95 */
ACMD(do_doorbash);        /* TD 04/28/95 */
ACMD(do_mana);            /* TD 03/17/95 */
ACMD(do_shadow);          /* TD 04/09/95 */
ACMD(do_tick);            /* TD 09/11/95 */
ACMD(do_world);           /* TD 09/22/95 */
ACMD(do_meditate);        /* TD 09/30/95 */
//ACMD(do_edit);            /* TD 12/29/95 */
ACMD(do_sign);            /* TD 01/21/96 */
ACMD(do_scan);            /* TD 03/23/96 */
ACMD(do_tog_rel);         /* TD 04/11/96 */
ACMD(do_skills);          /* TD --NOT--  */
ACMD(do_gen_vfile);       /* TD 04/19/96 */
ACMD(do_petition);        /* TD 07/15/96 */
ACMD(do_accept);          /* TD 07/15/96 */
ACMD(do_reject);          /* TD 07/15/96 */
ACMD(do_dismiss);         /* TD 07/15/96 */
ACMD(do_demote);          /* TD 07/15/96 */
ACMD(do_promote);         /* TD 07/15/96 */
ACMD(do_guild);           /* TD 07/15/96 */
ACMD(do_recall);	  /* TD 07/19/96 */
ACMD(do_infobar);         /* TD 07/21/96 */
ACMD(do_poison_weapon);   /* TD 08/01/96 */
ACMD(do_stab);            /* TD 08/09/96 */
ACMD(do_cudgel);          /* TD 08/12/96 */
ACMD(do_rlist);           /* TD 08/17/96 */
ACMD(do_olist);           /* TD 08/17/96 */
ACMD(do_mlist);           /* TD 08/17/96 */
ACMD(do_turn);            /* TD 08/17/96 */
ACMD(do_shoot);           /* TD 08/18/96 */
ACMD(do_load);            /* TD 08/18/96 */
ACMD(do_throw);           /* TD 08/24/96 */
ACMD(do_religion);        /* TD 09/07/96 */
ACMD(do_lm);              /* TD 08/16/96 */

/* This is the Master Command List(tm).

 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */

const struct command_info cmd_info[] = {
  { "RESERVED", 0, 0, 0, 0 },   /* this must be first -- for specprocs */

  /* directions must come before other commands but after RESERVED */
  { "north"    , POS_STANDING, do_move     , 0, SCMD_NORTH },
  { "east"     , POS_STANDING, do_move     , 0, SCMD_EAST },
  { "south"    , POS_STANDING, do_move     , 0, SCMD_SOUTH },
  { "west"     , POS_STANDING, do_move     , 0, SCMD_WEST },
  { "up"       , POS_STANDING, do_move     , 0, SCMD_UP },
  { "down"     , POS_STANDING, do_move     , 0, SCMD_DOWN },
  { "northeast", POS_STANDING, do_move     , 0, SCMD_NE },
  { "northwest", POS_STANDING, do_move     , 0, SCMD_NW },
  { "southeast", POS_STANDING, do_move     , 0, SCMD_SE },
  { "southwest", POS_STANDING, do_move     , 0, SCMD_SW },
  { "ne"       , POS_STANDING, do_move     , 0, SCMD_NE },
  { "nw"       , POS_STANDING, do_move     , 0, SCMD_NW },
  { "se"       , POS_STANDING, do_move     , 0, SCMD_SE },
  { "sw"       , POS_STANDING, do_move     , 0, SCMD_SW },

  /* now, the main list */
  { "at"       , POS_DEAD    , do_at       , LVL_IMMORT, 0 },
  { "ack"      , POS_RESTING , do_action   , 0, 0 },
  { "accept"   , POS_DEAD    , do_accept   , 1, 0 },
  { "addict"   , POS_STANDING, do_action   , 0, 0 },
  { "advance"  , POS_DEAD    , do_advance  , LVL_SUP, 0 },
  { "afk"      , POS_DEAD    , do_gen_tog  , 0, SCMD_AFK },
  { "affirm"   , POS_DEAD    , do_tog_rel  , 1, SCMD_AFFIRM },
  { "alias"    , POS_DEAD    , do_alias    , 0, 0 },
  { "accuse"   , POS_SITTING , do_action   , 0, 0 },
  { "agree"    , POS_RESTING , do_action   , 0, 0 },
  { "agreement", POS_DEAD    , do_gen_ps   , 0, SCMD_NEWBM },
  { "answer"   , POS_RESTING , do_action   , 0, 0 },
  { "apologize", POS_RESTING , do_action   , 0, 0 },
  { "applaud"  , POS_STANDING, do_action   , 0, 0 },
  { "appraise" , POS_STANDING, do_not_here , 0, 0 },
  { "areas"    , POS_DEAD    , do_world    , 0, 0 },
  { "assist"   , POS_FIGHTING, do_assist   , 1, 0 },
  { "ask"      , POS_RESTING , do_spec_comm, 0, SCMD_ASK },
  { "autoassist", POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOASS },
  { "autoexit"  , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOEXIT },
  { "autoloot"  , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOLOOT },
  { "autogold"  , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOGOLD },

  { "bl"       , POS_DEAD    , do_lm       , LVL_CREATOR, 0 },
  { "bounce"   , POS_STANDING, do_action   , 0, 0 },
  { "backstab" , POS_STANDING, do_backstab , 1, 0 },
  { "babble"   , POS_RESTING , do_action   , 0, 0 },
  { "badger"   , POS_STANDING, do_action   , 0, 0 },
  { "ban"      , POS_DEAD    , do_ban      , LVL_GRGOD, 0 },
  { "balance"  , POS_STANDING, do_not_here , 1, 0 },
  { "banzai"   , POS_STANDING, do_action   , 0, 0 },
  { "bash"     , POS_FIGHTING, do_bash     , 1, 0 },
  { "bark"     , POS_RESTING , do_action   , 0, 0 },
  { "bearhug"  , POS_STANDING, do_action   , 0, 0 },
  { "beckon"   , POS_STANDING, do_action   , 0, 0 },
  { "beg"      , POS_SITTING , do_action   , 0, 0 },
  { "behead"   , POS_STANDING, do_behead   , 0, 0 },
  { "berserk"  , POS_FIGHTING, do_berserk  , 0, 0 },
  { "bighug"   , POS_STANDING, do_action   , 0, 0 },
  { "bird"     , POS_SITTING , do_action   , 0, 0 },
  { "bite"     , POS_STANDING, do_bite     , 0, 0 },
  { "blah"     , POS_RESTING , do_action   , 0, 0 },
  { "bleed"    , POS_RESTING , do_action   , 0, 0 },
  { "blink"    , POS_RESTING , do_action   , 0, 0 },
  { "blush"    , POS_RESTING , do_action   , 0, 0 },
  { "blow"     , POS_RESTING , do_action   , 0, 0 },
  { "boggle"   , POS_RESTING , do_action   , 0, 0 },
  { "bonk"     , POS_RESTING , do_action   , 0, 0 },
  { "bow"      , POS_STANDING, do_action   , 0, 0 },
  { "brb"      , POS_RESTING , do_action   , 0, 0 },
  { "brief"    , POS_DEAD    , do_gen_tog  , 0, SCMD_BRIEF },
  { "burp"     , POS_RESTING , do_action   , 0, 0 },
  { "buy"      , POS_STANDING, do_not_here , 0, 0 },
  { "bug"      , POS_DEAD    , do_gen_write, 0, SCMD_BUG },

  { "cast"     , POS_SITTING , do_cast     , 1, 0 },
  { "cackle"   , POS_RESTING , do_action   , 0, 0 },
  { "cannonball", POS_STANDING, do_action  , 0, 0 },
  { "chastise" , POS_RESTING , do_action   , 0, 0 },
  { "chant"    , POS_STANDING, do_cast     , 0, 0 },
  { "cheer"    , POS_STANDING, do_action   , 0, 0 },
  { "check"    , POS_STANDING, do_not_here , 1, 0 },
  { "chuckle"  , POS_RESTING , do_action   , 0, 0 },
  { "choke"    , POS_RESTING , do_action   , 0, 0 },
  { "circle"   , POS_FIGHTING, do_circle   , 1, 0 },
  { "collapse" , POS_STANDING, do_action   , 0, 0 },
  { "close"    , POS_SITTING , do_gen_door , 0, SCMD_CLOSE },
  { "clap"     , POS_RESTING , do_action   , 0, 0 },
  { "claw"     , POS_STANDING, do_claw     , 0, 0 },
  { "clear"    , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
  { "cls"      , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
  { "color"    , POS_DEAD    , do_color    , 0, 0 },
  { "comfort"  , POS_RESTING , do_action   , 0, 0 },
  { "comb"     , POS_RESTING , do_action   , 0, 0 },
  { "commands" , POS_DEAD    , do_commands , 0, SCMD_COMMANDS },
  { "compact"  , POS_DEAD    , do_gen_tog  , 0, SCMD_COMPACT },
  { "complain" , POS_RESTING , do_action   , 0, 0 },
  { "cough"    , POS_RESTING , do_action   , 0, 0 },
  { "cower"    , POS_STANDING, do_action   , 0, 0 },
  { "create"   , POS_DEAD    , do_create   , LVL_BUILDER, 0 },
  { "credits"  , POS_DEAD    , do_gen_ps   , 0, SCMD_CREDITS },
  { "cringe"   , POS_RESTING , do_action   , 0, 0 },
  { "cry"      , POS_RESTING , do_action   , 0, 0 },
  { "cuddle"   , POS_RESTING , do_action   , 0, 0 },
  { "cudgel"   , POS_FIGHTING, do_cudgel   , 1, 0 },
  { "curse"    , POS_RESTING , do_action   , 0, 0 },
  { "curtsey"  , POS_STANDING, do_action   , 0, 0 },

  { "dance"    , POS_STANDING, do_action   , 0, 0 },
  { "date"     , POS_DEAD    , do_date     , LVL_IMMORT, SCMD_DATE },
  { "daydream" , POS_SLEEPING, do_action   , 0, 0 },
  { "dc"       , POS_DEAD    , do_dc       , LVL_GOD, 0 },
  { "deposit"  , POS_STANDING, do_not_here , 1, 0 },
  { "demote"   , POS_DEAD    , do_demote   , 1, 0 },
  { "diagnose" , POS_RESTING , do_diagnose , 0, 0 },
  { "display"  , POS_DEAD    , do_display  , 0, 0 },
  { "disclaimer", POS_DEAD   , do_gen_ps   , 0, SCMD_NEWBM },
  { "dismount" , POS_MOUNTED , do_mount    , 1, 0 },
  { "dismiss"  , POS_DEAD    , do_dismiss  , 1, 0 },
  { "dive"     , POS_STANDING, do_action   , 0, 0 },
  { "doh"      , POS_RESTING , do_action   , 0, 0 },
  { "dream"    , POS_RESTING , do_action   , 0, 0 },
  { "drink"    , POS_RESTING , do_drink    , 0, SCMD_DRINK },
  { "drop"     , POS_RESTING , do_drop     , 0, SCMD_DROP },
  { "drool"    , POS_RESTING , do_action   , 0, 0 },
  { "duck"     , POS_STANDING, do_action   , 0, 0 },

  { "eat"      , POS_RESTING , do_eat      , 0, SCMD_EAT },
  { "echo"     , POS_SLEEPING, do_echo     , LVL_IMMORT, SCMD_ECHO },
  { "emote"    , POS_RESTING , do_echo     , 1, SCMD_EMOTE },
  { ":"        , POS_RESTING , do_echo     , 1, SCMD_EMOTE },
  { "embrace"  , POS_STANDING, do_action   , 0, 0 },
  { "enter"    , POS_STANDING, do_enter    , 0, 0 },
  { "equipment", POS_RESTING , do_equipment, 0, 0 },
  { "evaluate" , POS_RESTING , do_evaluate , 0, 0 },
  { "exits"    , POS_RESTING , do_exits    , 0, 0 },
  { "examine"  , POS_SITTING , do_examine  , 0, 0 },
  { "eyebrow"  , POS_RESTING , do_action   , 0, 0 },
  //  { "edit"     , POS_DEAD    , do_edit     , LVL_IMPL, 0 },

  { "force"    , POS_SLEEPING, do_force    , LVL_GOD, 0 },
  { "fart"     , POS_RESTING , do_action   , 0, 0 },
  { "fill"     , POS_STANDING, do_pour     , 0, SCMD_FILL },
  { "flash"    , POS_STANDING, do_action   , 0, 0 },
  { "flee"     , POS_FIGHTING, do_flee     , 1, 0 },
  { "flex"     , POS_STANDING, do_action   , 0, 0 },
  { "flip"     , POS_STANDING, do_action   , 0, 0 },
  { "flirt"    , POS_RESTING , do_action   , 0, 0 },
  { "follow"   , POS_RESTING , do_follow   , 0, 0 },
  { "fondle"   , POS_RESTING , do_action   , 0, 0 },
  { "freeze"   , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_FREEZE },
  { "french"   , POS_RESTING , do_action   , 0, 0 },
  { "frown"    , POS_RESTING , do_action   , 0, 0 },
  { "fume"     , POS_RESTING , do_action   , 0, 0 },

  { "gain"     , POS_STANDING, do_not_here , 0, 0 },
  { "garrotte" , POS_STANDING, do_garrotte , 1, 0 },
  { "gawk"     , POS_SITTING , do_action   , 0, 0 },
  { "get"      , POS_RESTING , do_get      , 0, 0 },
  { "gasp"     , POS_RESTING , do_action   , 0, 0 },
  { "gecho"    , POS_DEAD    , do_gecho    , LVL_GOD, 0 },
  { "give"     , POS_RESTING , do_give     , 0, 0 },
  { "giggle"   , POS_RESTING , do_action   , 0, 0 },
  { "glare"    , POS_RESTING , do_action   , 0, 0 },
  { "goto"     , POS_SLEEPING, do_goto     , LVL_IMMORT, 0 },
  { "gold"     , POS_RESTING , do_gold     , 0, 0 },
  { "goose"    , POS_STANDING, do_action   , 0, 0 },
  { "gossip"   , POS_RESTING , do_gen_comm , 0, SCMD_GOSSIP },
  { "gore"     , POS_FIGHTING, do_gore     , 0, 0 },
  { "grimace"  , POS_RESTING , do_action   , 0, 0 },
  { "group"    , POS_RESTING , do_group    , 1, 0 },
  { "grab"     , POS_RESTING , do_grab     , 0, 0 },
  { "greet"    , POS_RESTING , do_action   , 0, 0 },
  { "grin"     , POS_RESTING , do_action   , 0, 0 },
  { "groan"    , POS_RESTING , do_action   , 0, 0 },
  { "grope"    , POS_RESTING , do_action   , 0, 0 },
  { "grovel"   , POS_RESTING , do_action   , 0, 0 },
  { "growl"    , POS_RESTING , do_action   , 0, 0 },
  { "grumble"  , POS_STANDING, do_action   , 0, 0 },
  { "gsay"     , POS_SLEEPING, do_gsay     , 0, 0 },
  { "gtell"    , POS_SLEEPING, do_gsay     , 0, 0 },
  { "guild"    , POS_DEAD    , do_guild    , 1, 0 },
  { "gt"       , POS_RESTING , do_guild    , 1, 0 },
  { "guitar"   , POS_STANDING, do_action   , 0, 0 },
  { "gedit"    , POS_DEAD    , do_olc_master, LVL_BUILDER, SCMD_OLC_GEDIT},

  { "help"     , POS_DEAD    , do_help     , 0, 0 },
  { "hedit"    , POS_DEAD    , do_olc_master, LVL_BUILDER, SCMD_OLC_HEDIT},
  { "hand"     , POS_STANDING, do_action   , 0, 0 },
  { "halo"     , POS_RESTING , do_action   , 0, 0 },
  { "happy"    , POS_RESTING , do_action   , 0, 0 },
  { "headache" , POS_RESTING , do_action   , 0, 0 },
  { "handbook" , POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_HANDBOOK },
  { "hcontrol" , POS_DEAD    , do_hcontrol , LVL_GRGOD, 0 },
  { "hiccup"   , POS_RESTING , do_action   , 0, 0 },
  { "hide"     , POS_RESTING , do_hide     , 1, 0 },
  { "hit"      , POS_FIGHTING, do_hit      , 0, SCMD_HIT },
  { "highfive" , POS_STANDING, do_action   , 0, 0 },
  { "hold"     , POS_RESTING , do_grab     , 1, 0 },
  { "holler"   , POS_RESTING , do_gen_comm , 1, SCMD_HOLLER },
  { "holylight", POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_HOLYLIGHT },
  { "hop"      , POS_RESTING , do_action   , 0, 0 },
  { "house"    , POS_RESTING , do_house    , 0, 0 },
  { "howl"     , POS_RESTING , do_action   , 0, 0 },
  { "hmm"      , POS_RESTING , do_action   , 0, 0 },
  { "hug"      , POS_RESTING , do_action   , 0, 0 },

  { "inventory", POS_RESTING , do_inventory, 0, 0 },
  { "infobar"  , POS_DEAD    , do_gen_tog  , 0, SCMD_INFOBAR },
  { "idea"     , POS_DEAD    , do_gen_write, 0, SCMD_IDEA },
  { "imotd"    , POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_IMOTD },
  { "immlist"  , POS_DEAD    , do_gen_ps   , 0, SCMD_IMMLIST },
  { "info"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_INFO },
  { "insult"   , POS_RESTING , do_insult   , 0, 0 },
  { "invis"    , POS_DEAD    , do_invis    , LVL_IMMORT, 0 },

  { "junk"     , POS_RESTING , do_drop     , 0, SCMD_JUNK },

  { "kill"     , POS_FIGHTING, do_kill     , 0, 0 },
  { "kick"     , POS_FIGHTING, do_kick     , 1, 0 },
  { "kiss"     , POS_RESTING , do_action   , 0, 0 },

  { "look"     , POS_RESTING , do_look     , 0, SCMD_LOOK },
  { "laugh"    , POS_RESTING , do_action   , 0, 0 },
  { "last"     , POS_DEAD    , do_last     , LVL_GOD, 0 },
  { "leave"    , POS_STANDING, do_leave    , 0, 0 },
  { "levels"   , POS_DEAD    , do_levels   , 0, 0 },
  { "list"     , POS_STANDING, do_not_here , 0, 0 },
  { "lick"     , POS_RESTING , do_action   , 0, 0 },
  { "lock"     , POS_SITTING , do_gen_door , 0, SCMD_LOCK },
  { "load"     , POS_STANDING, do_load     , 1, 0 },
  { "love"     , POS_RESTING , do_action   , 0, 0 },
  { "lm"       , POS_DEAD    , do_lm       , LVL_CREATOR, 1 },

  { "maim"     , POS_STANDING, do_action   , 0, 0 },
  { "meow"     , POS_RESTING , do_action   , 0, 0 },
  { "medit"    , POS_DEAD    , do_olc_master, LVL_BUILDER, SCMD_OLC_MEDIT},
  { "mmm"      , POS_RESTING , do_action   , 0, 0 },
  { "moan"     , POS_RESTING , do_action   , 0, 0 },
  { "mock"     , POS_RESTING , do_action   , 0, 0 },
  { "moon"     , POS_STANDING, do_action   , 0, 0 },
  { "moonwalk" , POS_STANDING, do_action   , 0, 0 },
  { "motd"     , POS_DEAD    , do_gen_ps   , 0, SCMD_MOTD },
  { "mount"    , POS_STANDING, do_mount    , 1, 0 },
  { "mosh"     , POS_STANDING, do_action   , 0, 0 },
  { "mail"     , POS_STANDING, do_not_here , 1, 0 },
  { "massage"  , POS_RESTING , do_action   , 0, 0 },
  { "maul"     , POS_STANDING, do_claw     , 1, 0 },
  { "mute"     , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_SQUELCH },
  { "murder"   , POS_FIGHTING, do_hit      , 0, SCMD_MURDER },
  { "meditate" , POS_RESTING , do_meditate , 0, 0 },
  { "meter"    , POS_DEAD    , do_gen_tog  , 0, SCMD_METER },
  { "mlist"    , POS_DEAD    , do_mlist    , LVL_BUILDER, 0 },
  { "mpasound" , POS_DEAD    , do_mpasound , 0, 0 },
  { "mpjunk"   , POS_DEAD    , do_mpjunk   , 0, 0 },
  { "mpecho"   , POS_DEAD    , do_mpecho   , 0, 0 },
  { "mpechoat" , POS_DEAD    , do_mpechoat , 0, 0 },
  { "mpechoaround" , POS_DEAD, do_mpechoaround, 0, 0 },
  { "mpcast"   , POS_DEAD    , do_mpcast   , 0, 0 },
  { "mpkill"   , POS_DEAD    , do_mpkill   , 0, 0 },
  { "mpmload"  , POS_DEAD    , do_mpmload  , 0, 0 },
  { "mpoload"  , POS_DEAD    , do_mpoload  , 0, 0 },
  { "mppurge"  , POS_DEAD    , do_mppurge  , 0, 0 },
  { "mpgoto"   , POS_DEAD    , do_mpgoto   , 0, 0 },
  { "mpat"     , POS_DEAD    , do_mpat     , 0, 0 },
  { "mptransfer", POS_DEAD   , do_mptransfer, 0, 0 },
  { "mpforce"  , POS_DEAD    , do_mpforce  , 0, 0 },

  { "news"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_NEWS },
  { "ni"       , POS_RESTING , do_action   , 0, 0 },
  { "nibble"   , POS_RESTING , do_action   , 0, 0 },
  { "nod"      , POS_RESTING , do_action   , 0, 0 },
  { "nogossip" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOGOSSIP },
  { "nohassle" , POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_NOHASSLE },
  { "noooc"    , POS_DEAD    , do_gen_tog  , 1, SCMD_NOOOC },
  { "norepeat" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOREPEAT },
  { "nose"     , POS_STANDING, do_action   , 0, 0 },
  { "noshout"  , POS_SLEEPING, do_gen_tog  , 1, SCMD_DEAF },
  { "notell"   , POS_DEAD    , do_gen_tog  , 1, SCMD_NOTELL },
  { "notitle"  , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_NOTITLE },
  { "nowiz"    , POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_NOWIZ },
  { "nudge"    , POS_RESTING , do_action   , 0, 0 },
  { "nuke"     , POS_STANDING, do_action   , 0, 0 },
  { "nuzzle"   , POS_RESTING , do_action   , 0, 0 },

  { "order"    , POS_RESTING , do_order    , 1, 0 },
  { "offer"    , POS_STANDING, do_not_here , 1, 0 },
  { "ooc"      , POS_SLEEPING, do_gen_comm , 0, SCMD_OOC },
  { "ooo"      , POS_RESTING , do_action   , 0, 0 },
  { "oops"     , POS_RESTING , do_action   , 0, 0 },
  { "open"     , POS_SITTING , do_gen_door , 0, SCMD_OPEN },
  { "olc"      , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_SAVEINFO},
  { "oedit"    , POS_DEAD    , do_olc_master, LVL_BUILDER, SCMD_OLC_OEDIT},
  { "olist"    , POS_DEAD    , do_olist    , LVL_BUILDER, 0 },

  { "put"      , POS_RESTING , do_put      , 0, 0 },
  { "palm"     , POS_STANDING, do_palm     , 1, 0 },
  { "pat"      , POS_RESTING , do_action   , 0, 0 },
  { "page"     , POS_DEAD    , do_page     , LVL_GOD, 0 },
  { "pardon"   , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_PARDON },
  { "peer"     , POS_RESTING , do_action   , 0, 0 },
  { "peek"     , POS_RESTING , do_look     , 0, SCMD_PEEK },
  { "peace"    , POS_DEAD    , do_peace    , LVL_IMMORT, 0 },
  { "peck"     , POS_STANDING, do_action   , 0, 0 },
  { "petition" , POS_DEAD    , do_petition , 1, 0 }, 
  { "pick"     , POS_STANDING, do_gen_door , 1, SCMD_PICK },
  { "pinch"    , POS_STANDING, do_action   , 0, 0 },
  { "play"     , POS_STANDING, do_not_here , 1, 0 },
  { "point"    , POS_RESTING , do_action   , 0, 0 },
  { "poison"   , POS_RESTING , do_poison_weapon, 0, 0 },
  { "poke"     , POS_RESTING , do_action   , 0, 0 },
  { "policy"   , POS_DEAD    , do_gen_ps   , 0, SCMD_POLICIES },
  { "ponder"   , POS_RESTING , do_action   , 0, 0 },
  { "poofin"   , POS_DEAD    , do_poofset  , LVL_IMMORT, SCMD_POOFIN },
  { "poofout"  , POS_DEAD    , do_poofset  , LVL_IMMORT, SCMD_POOFOUT },
  { "pour"     , POS_STANDING, do_pour     , 0, SCMD_POUR },
  { "pout"     , POS_RESTING , do_action   , 0, 0 },
  { "practice" , POS_RESTING , do_not_here , 1, 0 },
  { "prance"   , POS_STANDING, do_action   , 0, 0 },
  { "prompt"   , POS_DEAD    , do_display  , 0, 0 },
  { "profess"  , POS_RESTING , do_action   , 0, 0 },
  { "propose"  , POS_STANDING, do_action   , 0, 0 },
  { "pray"     , POS_SITTING , do_cast     , 0, 0 },
  { "promote"  , POS_DEAD    , do_promote  , 1, 0 },
  { "puke"     , POS_RESTING , do_action   , 0, 0 },
  { "punch"    , POS_RESTING , do_action   , 0, 0 },
  { "pummel"   , POS_FIGHTING, do_pummel   , 0, 0 },
  { "purr"     , POS_RESTING , do_action   , 0, 0 },
  { "purge"    , POS_DEAD    , do_purge    , LVL_GOD, 0 },
  { "purse"    , POS_RESTING , do_gold     , 0, 0 },

  { "quaff"    , POS_RESTING , do_use      , 0, SCMD_QUAFF },
  { "qecho"    , POS_DEAD    , do_qcomm    , LVL_IMMORT, SCMD_QECHO },
  { "quest"    , POS_DEAD    , do_gen_tog  , 0, SCMD_QUEST },
  { "qui"      , POS_DEAD    , do_quit     , 0, 0 },
  { "quit"     , POS_DEAD    , do_quit     , 0, 0 },
  { "quit!"    , POS_DEAD    , do_quit     , 0, SCMD_QUIT },
  { "quiet"    , POS_STANDING, do_action   , 0, 0 },
  { "qsay"     , POS_RESTING , do_qcomm    , 0, SCMD_QSAY },

  { "reply"    , POS_SLEEPING, do_reply    , 0, 0 },
  { "redraw"   , POS_DEAD    , do_infobar  , 0, SCMDB_REDRAW },
  { "/"        , POS_STANDING, do_recall   , 0, 0 },
  { "repair"   , POS_STANDING, do_not_here , 0, 0 },
  { "rest"     , POS_RESTING , do_rest     , 0, 0 },
  { "resize"   , POS_DEAD    , do_infobar  , 0, SCMDB_RESIZE },
  { "read"     , POS_RESTING , do_look     , 0, SCMD_READ },
  { "reject"   , POS_DEAD    , do_reject   , 1, 0 }, 
  { "reload"   , POS_DEAD    , do_reboot   , LVL_SUP, 0 },
  { "recite"   , POS_RESTING , do_use      , 0, SCMD_RECITE },
  { "receive"  , POS_STANDING, do_not_here , 1, 0 },
  { "recall"   , POS_STANDING, do_recall   , 0, 0 },
  { "religion" , POS_DEAD    , do_religion , 0, 0 },
  { "rt"       , POS_RESTING , do_religion , 0, 0 },
  { "remove"   , POS_RESTING , do_remove   , 0, 0 },
  { "remort"   , POS_DEAD    , do_remort   , LVL_FREEZE, 0 },
  { "rent"     , POS_STANDING, do_not_here , 1, 0 },
  { "renounce" , POS_DEAD    , do_tog_rel  , 2, SCMD_RENOUNCE },
  { "report"   , POS_RESTING , do_report   , 0, 0 },
  { "reroll"   , POS_DEAD    , do_wizutil  , LVL_GRGOD, SCMD_REROLL },
  { "rescue"   , POS_FIGHTING, do_rescue   , 1, 0 },
  { "restore"  , POS_DEAD    , do_restore  , LVL_GOD, 0 },
  { "return"   , POS_DEAD    , do_return   , 0, 0 },
  { "ride"     , POS_STANDING, do_mount    , 0, 0 },
  { "rofl"     , POS_STANDING, do_action   , 0, 0 },
  { "roll"     , POS_RESTING , do_action   , 0, 0 },
  { "roomflags", POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_ROOMFLAGS },
  { "rose"     , POS_RESTING , do_action   , 0, 0 },
  { "rub"      , POS_STANDING, do_action   , 0, 0 },
  { "ruffle"   , POS_STANDING, do_action   , 0, 0 },
  { "redit"    , POS_DEAD    , do_olc_master, LVL_BUILDER, SCMD_OLC_REDIT},
  { "rlist"    , POS_DEAD    , do_rlist    , LVL_BUILDER, 0 },
  { "reboot"   , POS_DEAD    , do_copyover , LVL_SUP, 0 },

  { "say"      , POS_RESTING , do_say      , 0, 0 },
  { "'"        , POS_RESTING , do_say      , 0, 0 },
  { "save"     , POS_SLEEPING, do_save     , 0, 0 },
  { "sad"      , POS_RESTING , do_action   , 0, 0 },
  { "score"    , POS_DEAD    , do_score    , 0, 0 },
  { "scorebar" , POS_DEAD    , do_gen_tog  , 0, SCMD_SCOREBAR },
  { "scan"     , POS_STANDING, do_scan     , 0, 0 },
  { "scream"   , POS_RESTING , do_action   , 0, 0 },
  { "sell"     , POS_STANDING, do_not_here , 0, 0 },
  { "send"     , POS_SLEEPING, do_send     , LVL_GOD, 0 },
  { "serenade" , POS_STANDING, do_action   , 0, 0 },
  { "set"      , POS_DEAD    , do_set      , LVL_GOD, 0 },
  { "shh"      , POS_RESTING , do_action   , 0, 0 },
  { "shout"    , POS_RESTING , do_gen_comm , 0, SCMD_SHOUT },
  { "shake"    , POS_RESTING , do_action   , 0, 0 },
  { "shiver"   , POS_RESTING , do_action   , 0, 0 },
  { "shadow"   , POS_STANDING, do_shadow   , 1, 0 },
  { "show"     , POS_DEAD    , do_show     , LVL_IMMORT, 0 },
  { "showplrs" , POS_DEAD    , do_gen_vfile, LVL_SUP, SCMD_V_SHOWPLRS},
  { "shoot"    , POS_STANDING, do_shoot    , 1, 0 },
  { "shrug"    , POS_RESTING , do_action   , 0, 0 },
  { "shutdow"  , POS_DEAD    , do_shutdown , LVL_SUP, 0 },
  { "shutdown" , POS_DEAD    , do_shutdown , LVL_SUP, SCMD_SHUTDOWN },
  { "sigh"     , POS_RESTING , do_action   , 0, 0 },
  { "sign"     , POS_RESTING , do_sign     , 0, 0 },
  { "sing"     , POS_RESTING , do_cast     , 0, 0 },
  { "sip"      , POS_RESTING , do_drink    , 0, SCMD_SIP },
  { "sit"      , POS_RESTING , do_sit      , 0, 0 },
/*  { "skills"   , POS_DEAD    , do_skills   , 0, 0 }, */
  { "skillset" , POS_SLEEPING, do_skillset , LVL_GRGOD, 0 },
  { "sleep"    , POS_SLEEPING, do_sleep    , 0, 0 },
  { "slap"     , POS_RESTING , do_action   , 0, 0 },
  { "slowns"   , POS_DEAD    , do_gen_tog  , LVL_IMPL, SCMD_SLOWNS },
  { "smile"    , POS_RESTING , do_action   , 0, 0 },
  { "smirk"    , POS_RESTING , do_action   , 0, 0 },
  { "smoke"    , POS_STANDING, do_action   , 0, 0 },
  { "smooch"   , POS_RESTING , do_action   , 0, 0 },
  { "snicker"  , POS_RESTING , do_action   , 0, 0 },
  { "snap"     , POS_RESTING , do_action   , 0, 0 },
  { "snarl"    , POS_RESTING , do_action   , 0, 0 },
  { "sneeze"   , POS_RESTING , do_action   , 0, 0 },
  { "sneak"    , POS_STANDING, do_sneak    , 1, 0 },
  { "sniff"    , POS_RESTING , do_action   , 0, 0 },
  { "snore"    , POS_SLEEPING, do_action   , 0, 0 },
  { "snowball" , POS_STANDING, do_action   , LVL_IMMORT, 0 },
  { "snoop"    , POS_DEAD    , do_snoop    , LVL_GOD, 0 },
  { "snuggle"  , POS_RESTING , do_action   , 0, 0 },
  { "sob"      , POS_RESTING , do_action   , 0, 0 },
  { "socials"  , POS_DEAD    , do_commands , 0, SCMD_SOCIALS },
  { "split"    , POS_SITTING , do_split    , 1, 0 },
  { "spam"     , POS_STANDING, do_action   , 0, 0 },
  { "spank"    , POS_RESTING , do_action   , 0, 0 },
  { "spheres"  , POS_DEAD    , do_spheres  , 1, 0 },
  { "spit"     , POS_STANDING, do_action   , 0, 0 },
  { "squeeze"  , POS_RESTING , do_action   , 0, 0 },
  { "squeal"   , POS_RESTING , do_action   , 0, 0 },
  { "stand"    , POS_RESTING , do_stand    , 0, 0 },
  { "stab"     , POS_FIGHTING, do_stab     , 1, 0 },
  { "stagger"  , POS_STANDING, do_action   , 0, 0 },
  { "stare"    , POS_RESTING , do_action   , 0, 0 },
  { "stat"     , POS_DEAD    , do_stat     , LVL_IMMORT, 0 },
  { "steal"    , POS_STANDING, do_steal    , 1, 0 },
  { "steam"    , POS_RESTING , do_action   , 0, 0 },
  { "stifle"   , POS_RESTING , do_action   , 0, 0 },
  { "stoke"    , POS_RESTING , do_action   , 0, 0 },
  { "stretch"  , POS_STANDING, do_action   , 0, 0 },
  { "stroke"   , POS_RESTING , do_action   , 0, 0 },
  { "strip"    , POS_STANDING, do_action   , 0, 0 },
  { "strut"    , POS_STANDING, do_action   , 0, 0 },
  { "study"    , POS_RESTING , do_study    , 0, 0 },
  { "sulk"     , POS_RESTING , do_action   , 0, 0 },
  { "swear"    , POS_RESTING , do_action   , 0, 0 },
  { "switch"   , POS_DEAD    , do_switch   , LVL_BUILDER, 0 },
  { "swoon"    , POS_STANDING, do_action   , 0, 0 },
  { "syslog"   , POS_DEAD    , do_syslog   , LVL_IMMORT, 0 },
  { "sedit"    , POS_DEAD    , do_olc_master, LVL_BUILDER, SCMD_OLC_SEDIT},

  { "tell"     , POS_DEAD    , do_tell     , 0, 0 },
  { "tackle"   , POS_RESTING , do_action   , 0, 0 },
  { "take"     , POS_RESTING , do_get      , 0, 0 },
  { "tango"    , POS_STANDING, do_action   , 0, 0 },
  { "taunt"    , POS_RESTING , do_action   , 0, 0 },
  { "taste"    , POS_RESTING , do_eat      , 0, SCMD_TASTE },
  { "teleport" , POS_DEAD    , do_teleport , LVL_GOD, 0 },
  { "thank"    , POS_RESTING , do_action   , 0, 0 },
  { "think"    , POS_RESTING , do_action   , 0, 0 },
  { "thaw"     , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_THAW },
  { "throw"    , POS_FIGHTING, do_throw    , 0, 0 },
  { "title"    , POS_DEAD    , do_title    , 0, 0 },
  { "tick"     , POS_DEAD    , do_tick     , LVL_SUP, 0 },
  { "tickle"   , POS_RESTING , do_action   , 0, 0 },
  { "time"     , POS_DEAD    , do_time     , 0, 0 },
  { "toggle"   , POS_DEAD    , do_toggle   , 0, 0 },
  { "tongue"   , POS_RESTING , do_action   , 0, 0 },
  { "torture"  , POS_STANDING, do_action   , 0, 0 },
  { "touch"    , POS_SITTING , do_not_here , 0, 0 },
  { "track"    , POS_STANDING, do_track    , 0, 0 },
  { "transfer" , POS_SLEEPING, do_trans    , LVL_DEITY, 0 },
  { "twiddle"  , POS_RESTING , do_action   , 0, 0 },
  { "typo"     , POS_DEAD    , do_gen_write, 0, SCMD_TYPO },
  { "type"     , POS_RESTING , do_action   , 0, 0 },
  { "todo"     , POS_DEAD    , do_gen_write, LVL_BUILDER, SCMD_TODO },
  { "turn"     , POS_FIGHTING, do_turn     , 0, 0 },

  { "unlock"   , POS_SITTING , do_gen_door , 0, SCMD_UNLOCK },
  { "ungroup"  , POS_DEAD    , do_ungroup  , 0, 0 },
  { "unban"    , POS_DEAD    , do_unban    , LVL_GRGOD, 0 },
  { "unaffect" , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_UNAFFECT },
  { "uptime"   , POS_DEAD    , do_date     , LVL_IMMORT, SCMD_UPTIME },
  { "use"      , POS_SITTING , do_use      , 1, SCMD_USE },
  { "users"    , POS_DEAD    , do_users    , LVL_IMMORT, 0 },

  { "value"    , POS_STANDING, do_not_here , 0, 0 },
  { "version"  , POS_DEAD    , do_gen_ps   , 0, SCMD_VERSION },
  { "visible"  , POS_RESTING , do_visible  , 1, 0 },
  { "vnum"     , POS_DEAD    , do_vnum     , LVL_IMMORT, 0 },
  { "vstat"    , POS_DEAD    , do_vstat    , LVL_IMMORT, 0 },
  { "vbugs"    , POS_DEAD    , do_gen_vfile, LVL_GOD, SCMD_V_BUGS },
  { "videas"   , POS_DEAD    , do_gen_vfile, LVL_GOD, SCMD_V_IDEAS },
  { "vsyslog"  , POS_DEAD    , do_gen_vfile, LVL_GRGOD, SCMD_V_SYSLOG },
  { "vtypos"   , POS_DEAD    , do_gen_vfile, LVL_GOD, SCMD_V_TYPOS },
  { "vtodo"    , POS_DEAD    , do_gen_vfile, LVL_GOD, SCMD_V_TODO },

  { "wake"     , POS_SLEEPING, do_wake     , 0, 0 },
  { "wave"     , POS_RESTING , do_action   , 0, 0 },
  { "wear"     , POS_RESTING , do_wear     , 0, 0 },
  { "weather"  , POS_RESTING , do_weather  , 0, 0 },
  { "wedgie"   , POS_STANDING, do_action   , 0, 0 },
  { "who"      , POS_DEAD    , do_who      , 0, 0 },
  { "whoami"   , POS_DEAD    , do_gen_ps   , 0, SCMD_WHOAMI },
  { "where"    , POS_RESTING , do_where    , LVL_IMMORT, 0 },
  { "whisper"  , POS_RESTING , do_spec_comm, 0, SCMD_WHISPER },
  { "whine"    , POS_RESTING , do_action   , 0, 0 },
  { "whistle"  , POS_RESTING , do_action   , 0, 0 },
  { "whip"     , POS_STANDING, do_action   , 0, 0 },
  { "wield"    , POS_RESTING , do_wield    , 0, 0 },
  { "wince"    , POS_RESTING , do_action   , 0, 0 },
  { "wiggle"   , POS_STANDING, do_action   , 0, 0 },
  { "wimpy"    , POS_DEAD    , do_wimpy    , 0, 0 },
  { "wink"     , POS_RESTING , do_action   , 0, 0 },
  { "wish"     , POS_RESTING , do_gen_comm , 0, SCMD_WISH },
  { "withdraw" , POS_STANDING, do_not_here , 1, 0 },
  { "wiznet"   , POS_DEAD    , do_wiznet   , LVL_IMMORT, 0 },
  { ";"        , POS_DEAD    , do_wiznet   , LVL_IMMORT, 0 },
  { "wizhelp"  , POS_SLEEPING, do_commands , LVL_IMMORT, SCMD_WIZHELP },
  { "wizlist"  , POS_DEAD    , do_gen_ps   , 0, SCMD_WIZLIST },
  { "wizlock"  , POS_DEAD    , do_wizlock  , LVL_SUP, 0 },
  { "worship"  , POS_RESTING , do_action   , 0, 0 },
  { "worry"    , POS_RESTING , do_action   , 0, 0 },
  { "wolfwhistle", POS_STANDING, do_action , 0, 0 },
  { "wrap"     , POS_STANDING, do_action   , 0, 0 },
  { "wrestle"  , POS_STANDING, do_action   , 0, 0 },
  { "write"    , POS_STANDING, do_write    , 1, 0 },

  { "yawn"     , POS_RESTING , do_action   , 0, 0 },
  { "yodel"    , POS_RESTING , do_action   , 0, 0 },

  { "zipper"   , POS_STANDING, do_action   , 0, 0 },
  { "zreset"   , POS_DEAD    , do_zreset   , LVL_GRGOD, 0 },
  { "zedit"    , POS_DEAD    , do_olc_master, LVL_BUILDER, SCMD_OLC_ZEDIT},

  { "\n", 0, 0, 0, 0 } };       /* this must be last */


char *fill[] =
{
  "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

char *reserved[] =
{
  "self",
  "me",
  "all",
  "room",
  "someone",
  "something",
  "\n"
};

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void command_interpreter(struct char_data *ch, char *argument)
{
  int cmd, length;
  extern int no_specials;
  char *line;

  REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);

  /* just drop to next line for hitting CR */
  skip_spaces(&argument);
  if (!*argument)
    return;

  if (GET_POS(ch) == POS_MEDITATING) {
     GET_POS(ch) = POS_RESTING;
     send_to_char("You stop meditating and begin to rest.\r\n", ch);
  }
  /*
   * special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
  if (!isalpha((int)*argument)) {
    arg[0] = argument[0];
    arg[1] = '\0';
    line = argument + 1;
  } else
    line = any_one_arg(argument, arg);

  /* otherwise, find the command */
  for (length = strlen(arg), cmd = 0; *cmd_info[cmd].command != '\n';
       cmd++) {
    if (!strncmp(cmd_info[cmd].command, arg, length)) {
      if ((GET_LEVEL(ch) >= cmd_info[cmd].minimum_level)
	  || GET_TRUST(ch) >= cmd_info[cmd].minimum_level) 
	break;
    }
  }

  if (*cmd_info[cmd].command == '\n')
    send_to_char("Huh?!?\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_ASSHOLE) && (cmd_info[cmd].minimum_level > MAX_MORT_LEVEL))
    send_to_char("You obviously have pissed someone off, because you cannot do that right now.\r\n", ch);
  else if (IS_AFFECTED2(ch, AFF_PETRIFIED))
    send_to_char("You try, but in your petrified state you are unable to do anything!\r\n", ch);
  else if (IS_AFFECTED2(ch, AFF_PARALYSIS))
    send_to_char("You are in a state of paralysis and unable to anything!\r\n", ch);
  else if (cmd_info[cmd].command_pointer == NULL)
    send_to_char("Sorry, that command hasn't been implemented yet.\r\n", ch);
  else if (IS_NPC(ch) && cmd_info[cmd].minimum_level >= LVL_IMMORT)
    send_to_char("You can't use immortal commands while switched.\r\n", ch);
  else if (GET_POS(ch) < cmd_info[cmd].minimum_position)
    switch (GET_POS(ch)) {
    case POS_DEAD:
      send_to_char("Lie still; you are DEAD!!! :-(\r\n", ch);
      break;
    case POS_INCAP:
    case POS_MORTALLYW:
      send_to_char("You are in a pretty bad shape, unable to do anything!\r\n", ch);
      break;
    case POS_STUNNED:
      send_to_char("All you can do right now is think about the stars!\r\n", ch);
      break;
    case POS_SLEEPING:
      send_to_char("In your dreams, or what?\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("Maybe you should get on your feet first?\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("No way!  You're fighting for your life!\r\n", ch);
      break;
  } else if (no_specials || !special(ch, cmd, line))
    ((*cmd_info[cmd].command_pointer) (ch, line, cmd, cmd_info[cmd].subcmd));
}

/**************************************************************************
 * Routines to handle aliasing                                            *
 **************************************************************************/


struct alias *find_alias(struct alias *alias_list, char *str)
{
  while (alias_list != NULL) {
    if (*str == *alias_list->alias)     /* hey, every little bit counts :-) */
      if (!strcmp(str, alias_list->alias))
	return alias_list;

    alias_list = alias_list->next;
  }

  return NULL;
}


void free_alias(struct alias *a)
{
  if (a->alias)
    free(a->alias);
  if (a->replacement)
    free(a->replacement);
  free(a);
}


/* The interface to the outside world: do_alias */
ACMD(do_alias)
{
  char *repl;
  struct alias *a, *temp;

  if (IS_NPC(ch))
    return;

  repl = any_one_arg(argument, arg);

  if (!*arg) {                  /* no argument specified -- list currently defined aliases */
    send_to_char("Currently defined aliases:\r\n", ch);
    if ((a = GET_ALIASES(ch)) == NULL)
      send_to_char(" None.\r\n", ch);
    else {
      while (a != NULL) {
	sprintf(buf, "%-15s %s\r\n", a->alias, a->replacement);
	send_to_char(buf, ch);
	a = a->next;
      }
    }
  } else {                      /* otherwise, add or remove aliases */
    /* is this an alias we've already defined? */
    if ((a = find_alias(GET_ALIASES(ch), arg)) != NULL) {
      REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
      free_alias(a);
    }
    /* if no replacement string is specified, assume we want to delete */
    if (!*repl) {
      if (a == NULL)
	send_to_char("No such alias.\r\n", ch);
      else
	send_to_char("Alias deleted.\r\n", ch);
    } else {                    /* otherwise, either add or redefine an alias */
      if (!str_cmp(arg, "alias")) {
	send_to_char("You can't alias 'alias'.\r\n", ch);
	return;
      }
      CREATE(a, struct alias, 1);
      a->alias = str_dup(arg);
      delete_doubledollar(repl);
      a->replacement = str_dup(repl);
      if (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR))
	a->type = ALIAS_COMPLEX;
      else
	a->type = ALIAS_SIMPLE;
      a->next = GET_ALIASES(ch);
      GET_ALIASES(ch) = a;
      send_to_char("Alias added.\r\n", ch);
    }
  }
}

/*
 * Valid numeric replacements are only &1 .. &9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "&*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias *a)
{
  struct txt_q temp_queue;
  char *tokens[NUM_TOKENS], *temp, *write_point;
  int num_of_tokens = 0, num;

  /* First, parse the original string */
  temp = strtok(strcpy(buf2, orig), " ");
  while (temp != NULL && num_of_tokens < NUM_TOKENS) {
    tokens[num_of_tokens++] = temp;
    temp = strtok(NULL, " ");
  }

  /* initialize */
  write_point = buf;
  temp_queue.head = temp_queue.tail = NULL;

  /* now parse the alias */
  for (temp = a->replacement; *temp; temp++) {
    if (*temp == ALIAS_SEP_CHAR) {
      *write_point = '\0';
      buf[MAX_INPUT_LENGTH - 1] = '\0';
      write_to_q(buf, &temp_queue, 1);
      write_point = buf;
    } else if (*temp == ALIAS_VAR_CHAR) {
      temp++;
      if ((num = *temp - '1') < num_of_tokens && num >= 0) {
	strcpy(write_point, tokens[num]);
	write_point += strlen(tokens[num]);
      } else if (*temp == ALIAS_GLOB_CHAR) {
	strcpy(write_point, orig);
	write_point += strlen(orig);
      } else if ((*(write_point++) = *temp) == '$')     /* redouble $ for act safety */
	*(write_point++) = '$';
    } else
      *(write_point++) = *temp;
  }

  *write_point = '\0';
  buf[MAX_INPUT_LENGTH - 1] = '\0';
  write_to_q(buf, &temp_queue, 1);

  /* push our temp_queue on to the _front_ of the input queue */
  if (input_q->head == NULL)
    *input_q = temp_queue;
  else {
    temp_queue.tail->next = input_q->head;
    input_q->head = temp_queue.head;
  }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(struct descriptor_data *d, char *orig)
{
  char first_arg[MAX_INPUT_LENGTH], *ptr;
  struct alias *a, *tmp;

  /* bail out immediately if the guy doesn't have any aliases */
  if ((tmp = GET_ALIASES(d->character)) == NULL)
    return 0;

  /* find the alias we're supposed to match */
  ptr = any_one_arg(orig, first_arg);

  /* bail out if it's null */
  if (!*first_arg)
    return 0;

  /* if the first arg is not an alias, return without doing anything */
  if ((a = find_alias(tmp, first_arg)) == NULL)
    return 0;

  if (a->type == ALIAS_SIMPLE) {
    strcpy(orig, a->replacement);
    return 0;
  } else {
    perform_complex_alias(&d->input, ptr, a);
    return 1;
  }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, char **list, bool exact)
{
  register int i, l;

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcmp(arg, *(list + i)))
	return (i);
  } else {
    if (!l)
      l = 1;                    /* Avoid "" to match the first available
				 * string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncmp(arg, *(list + i), l))
	return (i);
  }

  return -1;
}


int is_number(char *str)
{
  while (*str)
    if (!isdigit((int)*(str++)))
      return 0;

  return 1;
}


void skip_spaces(char **string)
{
  for (; **string && isspace((int)**string); (*string)++);
}


char *delete_doubledollar(char *string)
{
  char *read, *write;

  if ((write = strchr(string, '$')) == NULL)
    return string;

  read = write;

  while (*read)
    if ((*(write++) = *(read++)) == '$')
      if (*read == '$')
	read++;

  *write = '\0';

  return string;
}


int fill_word(char *argument)
{
  return (search_block(argument, fill, TRUE) >= 0);
}


int reserved_word(char *argument)
{
  return (search_block(argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;
    while (*argument && !isspace((int)*argument)) {
      *(first_arg++) = LOWER(*argument);
      argument++;
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/*
 * one_word is like one_argument, except that words in quotes ("") are
 * considered one word.
 */
char *one_word(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;

    if (*argument == '\"') {
      argument++;
      while (*argument && *argument != '\"') {
	*(first_arg++) = LOWER(*argument);
	argument++;
      }
      argument++;
    } else {
      while (*argument && !isspace((int)*argument)) {
	*(first_arg++) = LOWER(*argument);
	argument++;
      }
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
  skip_spaces(&argument);

  while (*argument && !isspace((int)*argument)) {
    *(first_arg++) = LOWER(*argument);
    argument++;
  }

  *first_arg = '\0';

  return argument;
}


char *any_two_args(char *argument, char *first_arg, char *second_arg)
{
  return any_one_arg(any_one_arg(argument, first_arg), second_arg);   
}

/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
  return one_argument(one_argument(argument, first_arg), second_arg);   /* :-) */
}

char *three_arguments(char *argument, char *first_arg, char *second_arg, char *third_arg)
{
  return one_argument(one_argument(one_argument(argument, first_arg), second_arg), third_arg);   /* :-) */
}

/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 *
 * returns 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
    return 0;

  for (; *arg1 && *arg2; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return 0;

  if (!*arg1)
    return 1;
  else
    return 0;
}



/* return first space-delimited token in arg1; remainder of string in arg2 */
void half_chop(char *string, char *arg1, char *arg2)
{
  char *temp;

  temp = any_one_arg(string, arg1);
  skip_spaces(&temp);
  strcpy(arg2, temp);
}



/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(char *command)
{
  int cmd;

  for (cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(cmd_info[cmd].command, command))
      return cmd;

  return -1;
}


int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j;

  /* special in room? */
  if (GET_ROOM_SPEC(ch->in_room) != NULL)
    if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, cmd, arg))
      return 1;

  /* special in equipment list? */
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j) && GET_OBJ_SPEC(GET_EQ(ch, j)) != NULL)
      if (GET_OBJ_SPEC(GET_EQ(ch, j)) (ch, GET_EQ(ch, j), cmd, arg))
	return 1;

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;

  /* special in mobile present? */
  for (k = world[ch->in_room].people; k; k = k->next_in_room)
    if (GET_MOB_SPEC(k) != NULL)
      if (GET_MOB_SPEC(k) (ch, k, cmd, arg))
	return 1;

  /* special in object present? */
  for (i = world[ch->in_room].contents; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;

  return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    if (!str_cmp((player_table + i)->name, name) ||
        is_abbrev(name, (player_table + i)->name))
      return i;
  }

  return -1;
}


int _parse_name(char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace((int)*arg); arg++);

  for (i = 0; (*name = *arg); arg++, i++, name++)
    if (!isalpha((int)*arg))
      return 1;

  if (!i)
    return 1;

  return 0;
}


#define RECON           1
#define USURP           2
#define UNSWITCH        3

int perform_dupe_check(struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  struct char_data *target = NULL, *ch, *next_ch;
  int mode = 0;

  int id = GET_IDNUM(d->character);

  /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

  for (k = descriptor_list; k; k = next_k) {
    next_k = k->next;

    if (k == d)
      continue;

    if (k->original && (GET_IDNUM(k->original) == id)) {    /* switched char */
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
      if (!target) {
	target = k->original;
	mode = UNSWITCH;
      }
      if (k->character)
	k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
    } else if (k->character && (GET_IDNUM(k->character) == id)) {
      if (!target && STATE(k) == CON_PLAYING) {
	SEND_TO_Q("\r\nThis body has been usurped!\r\n", k);
	target = k->character;
	mode = USURP;
      }
      k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
    }
  }

 /*
  * now, go through the character list, deleting all characters that
  * are not already marked for deletion from the above step (i.e., in the
  * CON_HANGUP state), and have not already been selected as a target for
  * switching into.  In addition, if we haven't already found a target,
  * choose one if one is available (while still deleting the other
  * duplicates, though theoretically none should be able to exist).
  */

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if (IS_NPC(ch))
      continue;
    if (GET_IDNUM(ch) != id)
      continue;

    /* ignore chars with descriptors (already handled by above step) */
    if (ch->desc)
      continue;

    /* don't extract the target char we've found one already */
    if (ch == target)
      continue;

    /* we don't already have a target and found a candidate for switching */
    if (!target) {
      target = ch;
      mode = RECON;
      continue;
    }

    /* we've found a duplicate - blow him away, dumping his eq in limbo. */
    if (ch->in_room != NOWHERE)
      char_from_room(ch);
    char_to_room(ch, 1);
    extract_char(ch);
  }

  /* no target for swicthing into was found - allow login to continue */
  if (!target)
    return 0;

  /* Okay, we've found a target.  Connect d to target. */
  free_char(d->character); /* get rid of the old char */
  d->character = target;
  d->character->desc = d;
  d->original = NULL;
  d->character->char_specials.timer = 0;
  REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING);
  STATE(d) = CON_PLAYING;
  if (PRF_FLAGGED(d->character, PRF_INFOBAR))
    do_infobar(d->character, 0, 0, SCMDB_REDRAW);

  switch (mode) {
  case RECON:
    SEND_TO_Q("Reconnecting.\r\n", d);
    act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(d->character)), TRUE);
    break;
  case USURP:
    SEND_TO_Q("You take over your own body, already in use!\r\n", d);
    act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
	"$n's body has been taken over by a new spirit!",
	TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s has re-logged in ... disconnecting old socket.",
	    GET_NAME(d->character));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(d->character)), TRUE);
    break;
  case UNSWITCH:
    SEND_TO_Q("Reconnecting to unswitched char.", d);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(d->character)), TRUE);
    break;
  }

  return 1;
}

/* load the player, put them in the right room - used by copyover_recover too */
int enter_player_game (struct descriptor_data *d)
{
  int load_result;
  room_num load_room;
  extern sh_int r_mortal_start_room;
  extern sh_int r_immort_start_room;
  extern sh_int r_frozen_start_room;
  
  reset_char(d->character);
  read_aliases(d->character);
  
  if (PLR_FLAGGED(d->character, PLR_INVSTART))
    GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
  
  /*
   * We have to place the character in a room before equipping them
   * or equip_char() will gripe about the person in NOWHERE.
   */
  if ((load_room = GET_LOADROOM(d->character)) != NOWHERE)
    load_room = real_room(load_room);
  
  /* If char was saved with NOWHERE, or real_room above failed... */
  if (load_room == NOWHERE) {
    if (GET_LEVEL(d->character) >= LVL_IMMORT)
      load_room = r_immort_start_room;
    else
      load_room = r_mortal_start_room;
  }
  
  if (PLR_FLAGGED(d->character, PLR_FROZEN))
    load_room = r_frozen_start_room;
  
  d->character->next = character_list;
  character_list = d->character;
  char_to_room(d->character, load_room);
  load_result = Crash_load(d->character);
  save_char(d->character, NOWHERE);
  
  return load_result;
}

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
  char buf[128];
  int player_i, load_result;
  static int points;
  char tmp_name[MAX_INPUT_LENGTH];
  struct char_file_u tmp_store;
  extern char *SECGREETING;
  extern char *THIRGREETING;
  extern char *FORGREETING;
  extern char *ANSI_GREET;
  extern char *NAME_POLICY;
  extern char *MENU;
  extern char *ANSI_MENU;
  extern char *ANSI_NAME_POLICY;
  extern int max_bad_pws;

  int load_char(char *name, struct char_file_u *char_element);
  void set_base_attribs(struct char_data *ch);
  void print_plr_attribs(struct char_data *ch, int points);
  int add_attrib(struct char_data *ch, int points, int attrib);
  int sub_attrib(struct char_data *ch, int points, int attrib);
  char * rsel_to_name( char c );
  const char * build_race_menu( bool c );
  void olc_parse(struct descriptor_data *d, char *arg);

  skip_spaces(&arg);

  switch (STATE(d)) {

    /*. OLC states .*/
    case CON_EDITTING:
      olc_parse(d, arg);
      break;
    /*. End of OLC states .*/

    case CON_NEW_CONN:
      if (!*arg) {
	close_socket(d);
      } else if (*arg == 'y' || *arg == 'Y') {
	GET_ANSI(d) = TRUE;
	SEND_TO_Q(ANSI_GREET, d);
      } else /* No ANSI color :( */ {
	GET_ANSI(d) = FALSE;
	switch (number(1, 3)) {
	  case 1:
	    SEND_TO_Q(SECGREETING, d);
	    break;
	  case 2:
	    SEND_TO_Q(THIRGREETING, d);
	    break;
	  case 3:
	  default:
	    SEND_TO_Q(FORGREETING, d);
	    break;
	}
      }
      STATE(d) = CON_GET_NAME;
      break;

  case CON_GET_NAME:            /* wait for input of name */
    if (d->character == NULL) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      CREATE(d->character->player_specials, struct player_special_data, 1);
      d->character->desc = d;
    }
    if (!*arg)
      close_socket(d);
    else {
      /* code I was going to add for command line operation is also in old src */
      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 3 ||
	  strlen(tmp_name) > MAX_NAME_LENGTH ||
	  fill_word(strcpy(buf, tmp_name)) || reserved_word(buf)) {
	SEND_TO_Q("Invalid name, please try another.\r\n"
		  "Name: ", d);
	return;
      }
      if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
	store_to_char(&tmp_store, d->character);
	GET_PFILEPOS(d->character) = player_i;

	if (PLR_FLAGGED(d->character, PLR_DELETED)) {
	  free_char(d->character);
	  CREATE(d->character, struct char_data, 1);
	  clear_char(d->character);
	  CREATE(d->character->player_specials, struct player_special_data, 1);
	  d->character->desc = d;
	  CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	  strcpy(d->character->player.name, CAP(tmp_name));
	  GET_PFILEPOS(d->character) = player_i;
	  if (GET_ANSI(d) == TRUE)
	    SEND_TO_Q(ANSI_NAME_POLICY, d);
	  else
	    SEND_TO_Q(NAME_POLICY, d);
	  /* Now see if the name is correct */
	  sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	  SEND_TO_Q(buf, d);
	  STATE(d) = CON_NAME_CNFRM;
	} else {
	  /* undo it just in case they are set */
	  REMOVE_BIT(PLR_FLAGS(d->character),
		     PLR_WRITING | PLR_MAILING | PLR_CRYO);

	  SEND_TO_Q("Password: ", d);
	  echo_off(d);
	  d->idle_tics = 0;
	  STATE(d) = CON_PASSWORD;
	}
      } else {
	/* player unknown -- make new character */

	if (!Valid_Name(tmp_name)) {
	  SEND_TO_Q("Invalid name, please try another.\r\n", d);
	  SEND_TO_Q("Name: ", d);
	  return;
	}
	CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	strcpy(d->character->player.name, CAP(tmp_name));
        if (GET_ANSI(d) == TRUE)
          SEND_TO_Q(ANSI_NAME_POLICY, d);
        else
          SEND_TO_Q(NAME_POLICY, d);
	/* now see if name is correct */
	sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	SEND_TO_Q(buf, d);
	STATE(d) = CON_NAME_CNFRM;
      }
    }
    break;
  case CON_NAME_CNFRM:          /* wait for conf. of new name    */
    if (UPPER(*arg) == 'Y') {
      if (isbanned(d->host) >= BAN_NEW) {
	sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_GOD, TRUE);
	SEND_TO_Q("Sorry, new characters are not allowed from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	return;
      }
      if (game_restrict) {
	SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
	sprintf(buf, "Request for new char %s denied from %s (wizlock)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_GOD, TRUE);
	STATE(d) = CON_CLOSE;
	return;
      }
      SEND_TO_Q("New character.\r\n", d);
      sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      echo_off(d);
      STATE(d) = CON_NEWPASSWD;
    } else if (*arg == 'n' || *arg == 'N') {
      SEND_TO_Q("Okay, what IS it, then? ", d);
      free(d->character->player.name);
      d->character->player.name = NULL;
      STATE(d) = CON_GET_NAME;
    } else {
      SEND_TO_Q("Please type Yes or No: ", d);
    }
    break;
  case CON_PASSWORD:            /* get pwd for known player      */
    /*
     * To really prevent duping correctly, the player's record should
     * be reloaded from disk at this point (after the password has been
     * typed).  However I'm afraid that trying to load a character over
     * an already loaded character is going to cause some problem down the
     * road that I can't see at the moment.  So to compensate, I'm going to
     * (1) add a 15 or 20-second time limit for entering a password, and (2)
     * re-add the code to cut off duplicates when a player quits.  JE 6 Feb 96
     */
    echo_on(d);    /* turn echo back on */

    if (!*arg)
      close_socket(d);
    else {
      if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
	sprintf(buf, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
	mudlog(buf, BRF, LVL_GOD, TRUE);
	GET_BAD_PWS(d->character)++;
	save_char(d->character, NOWHERE);
	if (++(d->bad_pws) >= max_bad_pws) {    /* 3 strikes and you're out. */
	  SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
	  STATE(d) = CON_CLOSE;
	} else {
	  SEND_TO_Q("Wrong password.\r\nPassword: ", d);
	  echo_off(d);
	}
	return;
      }
      load_result = GET_BAD_PWS(d->character);
      GET_BAD_PWS(d->character) = 0;

      if (isbanned(d->host) == BAN_SELECT &&
	  !PLR_FLAGGED(d->character, PLR_SITEOK)) {
	SEND_TO_Q("Sorry, this char has not been cleared for login from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Connection attempt for %s denied from %s",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_GOD, TRUE);
	return;
      }
      if (GET_LEVEL(d->character) < game_restrict) {
	SEND_TO_Q("The game is temporarily restricted.. try again later.\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_GOD, TRUE);
	return;
      }
      /* check and make sure no other copies of this player are logged in */
      if (perform_dupe_check(d))
	return;

      if (GET_ANSI(d) == TRUE)
        SEND_TO_Q(KYEL, d);
      if (GET_LEVEL(d->character) >= LVL_IMMORT)
	SEND_TO_Q(imotd, d);
      else
	SEND_TO_Q(motd, d);
      sprintf(buf, "\r\n\n%s*** PLEASE PRESS RETURN TO CONTINUE ***%s", 
              GET_ANSI(d) == TRUE ? KBBL : "",
              GET_ANSI(d) == TRUE ? KNRM : "");
      SEND_TO_Q(buf, d);
      
      sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
      mudlog(buf, BRF, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(d->character)), TRUE);

      if (load_result) {
	sprintf(buf, "\r\n\r\n\007\007\007"
		"%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
		CCRED(d->character, C_SPR), load_result,
		(load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
	SEND_TO_Q(buf, d);
	GET_BAD_PWS(d->character) = 0;
      }
      STATE(d) = CON_RMOTD;
    }
    break;

  case CON_NEWPASSWD:
  case CON_CHPWD_GETNEW:
    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
	!str_cmp(arg, GET_NAME(d->character))) {
      SEND_TO_Q("\r\nIllegal password.\r\n", d);
      SEND_TO_Q("Password: ", d);
      return;
    }
    strncpy(GET_PASSWD(d->character), CRYPT(arg, GET_NAME(d->character)), MAX_PWD_LENGTH);
    *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';

    SEND_TO_Q("\r\nPlease retype password: ", d);
    if (STATE(d) == CON_NEWPASSWD)
      STATE(d) = CON_CNFPASSWD;
    else
      STATE(d) = CON_CHPWD_VRFY;

    break;

  case CON_CNFPASSWD:
  case CON_CHPWD_VRFY:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
		MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
      SEND_TO_Q("Password: ", d);
      if (STATE(d) == CON_CNFPASSWD)
	STATE(d) = CON_NEWPASSWD;
      else
	STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    echo_on(d);

    if (STATE(d) == CON_CNFPASSWD) {
      if (GET_ANSI(d) == TRUE) {
	SEND_TO_Q("\r\n[1m[36mTo back up a menu at anytime type '[33m/[36m'.", d);
	SEND_TO_Q("\r\nWhat is your sex ([33mM/F[36m)?[0m ", d);
      } else {
	SEND_TO_Q("\r\nTo back up a menu at anytime type '/' (Except for this one)."
		  "\r\nWhat is your sex (M/F)? ", d);
      }
      STATE(d) = CON_QSEX;
    } else {
      save_char(d->character, NOWHERE);
      echo_on(d);
      SEND_TO_Q("\r\nDone.\n\r", d);
      if (GET_ANSI(d) == TRUE)
	SEND_TO_Q(ANSI_MENU, d);
      else
	SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    }

    break;

  case CON_QSEX:                /* query sex of new user         */
    switch (*arg) {
    case 'm':
    case 'M':
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      d->character->player.sex = SEX_FEMALE;
      break;
    default:
      SEND_TO_Q("That is not a sex..\r\n"
		"What IS your sex? ", d);
      return;
      break;
    }

    /* New stuff for TD - SPM */
    SEND_TO_Q( build_race_menu( GET_ANSI(d) ), d );
    /* reset the argument string */
    (*arg) = ('\0');
    STATE(d) = CON_QRACE;
    break;

  case CON_QRACE:
    if ((*arg) == ('?')) {
      do_help( d->character, rsel_to_name( *(++arg) ), 0, 0 );
      SEND_TO_Q("\r\n\r\nPlease choose a race: ", d);
      return;
    } else if ((*arg) == ('/')) {
      SEND_TO_Q("\r\n\r\nWhat is your sex (Male/Female)? ", d);
      STATE(d) = CON_QSEX;
      return;
    }

    if ((GET_RACE(d->character) = parse_race(*arg)) == (UNDEFINED_RACE)) {
      SEND_TO_Q("\r\nThat's not a race!\r\n", d);
      SEND_TO_Q( build_race_menu( GET_ANSI(d) ), d );
      return;
    }

    sprintf( buf, "Okay, you are now %s the %s\r\n",
             GET_NAME(d->character),
             races[ GET_RACE( d->character ) ].name );
    SEND_TO_Q(buf, d);

    /* Set-up the base attributes of the player */
    points = number(2, 6);
    set_base_attribs(d->character);
    print_plr_attribs(d->character, points);

    (*arg) = ('\0');
    STATE(d) = CON_QATTRIBS;
    break;

  case CON_QATTRIBS:
    switch (*arg) {
       case '/':
         SEND_TO_Q( build_race_menu( GET_ANSI(d) ), d );
	 STATE(d) = CON_QRACE;
	 return;
	 break;
       case '?':
	 do_help(d->character, "attribute", 0, 0);
	 print_plr_attribs(d->character, points);
	 break;
       case 'q':
       case 'Q':
       case '0':
	 d->character->aff_abils = d->character->real_abils;
	 SEND_TO_Q("Thank you, your attributes have been determined.\r\n", d);
	 if (GET_ANSI(d) == TRUE)
	   SEND_TO_Q("\r\n[1m[36mWhat is your alignment " \
                     "([[33mG[36m]ood, [[33mN[36m]eut, [[33mE[36m]vil, " \
                     "[[33mL[36m]awful, [[33mC[36m]haotic, [[33m?[36m] Help, [[33m/[36m] Back)?[0m ", d);
	 else
	   SEND_TO_Q("\r\nWhat is your alignment? ([G]ood, [N]eut, [E]vil, [L]awful, [C]haotic, [?] Help, [/] Back) ", d);
	 (*arg) = ('\0');
	 STATE(d) = CON_QALIGN;
	 return;
	 break;
       case 'r':
       case 'R':
         if (points >= 1) {
  	   set_base_attribs(d->character);
	   points--;
         } else SEND_TO_Q("\r\nYou do not have enough points to reroll.\r\n", d);
	 print_plr_attribs(d->character, points);
	 break;
       case '+':
	 if (points >= 1) {
	   switch (*(arg+1)) {
	     case 's':
	     case 'S':
	       points = add_attrib(d->character, points, 0);
	       break;
	     case 'i':
	     case 'I':
	       points = add_attrib(d->character, points, 1);
	       break;
	     case 'w':
	     case 'W':
	       points = add_attrib(d->character, points, 2);
	       break;
	     case 'd':
	     case 'D':
	       points = add_attrib(d->character, points, 3);
	       break;
	     case 'c':
	     case 'C':
	       points = add_attrib(d->character, points, 4);
	       break;
	     case 'h':
	     case 'H':
	       points = add_attrib(d->character, points, 5);
	       break;
	     case 'l':
	     case 'L':
	       points = add_attrib(d->character, points, 6);
	       break;
	   }
	 } else
	   SEND_TO_Q("\r\nYou cannot raise anymore attributes.\r\n", d);
	 print_plr_attribs(d->character, points);
	 break;
       case '-':
	 if (points < 25) {
	   switch (*(arg+1)) {
	     case 's':
	     case 'S':
	       points = sub_attrib(d->character, points, 0);
	       break;
	     case 'i':
	     case 'I':
	       points = sub_attrib(d->character, points, 1);
	       break;
	     case 'w':
	     case 'W':
	       points = sub_attrib(d->character, points, 2);
	       break;
	     case 'd':
	     case 'D':
	       points = sub_attrib(d->character, points, 3);
	       break;
	     case 'c':
	     case 'C':
	       points = sub_attrib(d->character, points, 4);
	       break;
	     case 'h':
	     case 'H':
	       points = sub_attrib(d->character, points, 5);
	       break;
	     case 'l':
	     case 'L':
	       points = sub_attrib(d->character, points, 6);
	       break;
	   }
	 } else
	   SEND_TO_Q("\r\nYou cannot lower anymore attributes.\r\n", d);
	 print_plr_attribs(d->character, points);
	 break;
       default:
	 SEND_TO_Q("That is not a valid option, if you are done press 'Q'.\r\n", d);
	 print_plr_attribs(d->character, points);
	 break;
    }
    break;

  case CON_QALIGN:                /* query align of new user         */
    switch (*arg) {
       case '/':
	 print_plr_attribs(d->character, points);
	 (*arg) = ('\0');
	 STATE(d) = CON_QATTRIBS;
	 return;
	 break;
       case '?': /* Help */
	 SEND_TO_Q("\r\nGuilds and religions have alignment restrictions"
		   "\r\nas well as some of the skills/spells.", d);
	 SEND_TO_Q("\r\nWhat else could you possibly need help with?"
		   "\r\n[G]ood, [N]eutral, [E]vil, [L]awful or [C]haotic? ", d);
	 break;
       case 'e':
       case 'E':
	 GET_PERMALIGN(d->character) = ALIGN_EVIL;
	 break;
       case 'g':
       case 'G':
	 GET_PERMALIGN(d->character) = ALIGN_GOOD;
	 break;
       case 'n':
       case 'N':
	 GET_PERMALIGN(d->character) = ALIGN_NEUTRAL;
	 break;
       case 'l':
       case 'L':
	 GET_PERMALIGN(d->character) = ALIGN_LAWFUL;
	 break;
       case 'c':
       case 'C':
	 GET_PERMALIGN(d->character) = ALIGN_CHAOTIC;
	 break;
       default:
	 SEND_TO_Q("That is not an alignment.\r\n", d);
	 if (GET_ANSI(d) == TRUE)
	   SEND_TO_Q("\r\n[1m[36mWhat is your alignment " \
                     "([[33mG[36m]ood, [[33mN[36m]eut, [[33mE[36m]vil, " \
                     "[[33mL[36m]awful, [[33mC[36m]haotic, [[33m?[36m] Help)?[0m ", d);
	 else
	   SEND_TO_Q("\r\nWhat is your alignment? ([G]ood, [N]eutral, [E]vil, [L]awful, [C]haotic, [?] Help) ", d);
	 return;
	 break;
    }

    if (GET_ANSI(d) == TRUE)
      SEND_TO_Q("[1m[36m\r\nNow you get to pick your handedness. The hand you pick\r\n"
	      "as your primary hand will be the strongest, and be able to\r\n"
	      "do more things than your secondary hand. ('/' to go back a selection)\r\n"
	      "\r\nWhich hand is your predominant one ([[33mR[36m]ight/[[33mL[36m]eft)?[0m ", d);
    else
      SEND_TO_Q("\r\nNow you get to pick your handedness. The hand you pick\r\n"
	      "as your primary hand will be the strongest, and be able to\r\n"
	      "do more things than your secondary hand. ('/' to go Back a selection)\r\n"
	      "\r\nWhich hand is your predominant one (Right/Left)? ", d);
    STATE(d) = CON_QHANDEDNESS;
    break;

  case CON_QHANDEDNESS:
    switch (*arg) {
       case '/':
	 if (GET_ANSI(d) == TRUE)
	   SEND_TO_Q("\r\n[1m[36mWhat is your alignment ([[33mG[36m]ood, [[33mN[36m]eutral, [[33mE[36m]vil, [[33m?[36m] Help)?[0m ", d);
	 else
	   SEND_TO_Q("\r\nWhat is your alignment? ([G]ood, [N]eutral, [E]vil, [?] Help) ", d);
	 (*arg) = ('\0');
	 STATE(d) = CON_QALIGN;
	 return;
	 break;
       case 'r':
       case 'R':
       case '1':
	 SEND_TO_Q("Very good you are now right-handed.\r\n", d);
	 GET_HANDEDNESS(d->character) = 1;
	 break;
       case 'l':
       case 'L':
       case '2':
	 SEND_TO_Q("Very good you are now left-handed.\r\n", d);
	 GET_HANDEDNESS(d->character) = 0;
	 break;
       default:
	 if (GET_ANSI(d) == TRUE)
	   SEND_TO_Q("[1m[36mYou only have TWO hands, either [[33mR[36m]ight or [[33mL[36m]eft!\r\n"
		   "Now, which hand is your predominant one?[0m ", d);
	 else
	   SEND_TO_Q("You only have TWO hands, either [R]ight or [L]eft!\r\n"
		   "Now, which hand is your predominant one? ", d);
	 return;
    }

    if (GET_ANSI(d) == TRUE)
      SEND_TO_Q("[1m[36m\r\nPlease enter your complete email address.[0m\r\n",d);
    else
      SEND_TO_Q("\r\nPlease enter your complete email address.\r\n",d);
    STATE(d) = CON_QEMAIL;
    break;

  case CON_QEMAIL:
    if (*arg == '/') {
      if (GET_ANSI(d) == TRUE)
	SEND_TO_Q("\r\n[1m[36mWhich hand is your predominant one, [[33mR[36m]ight or [[33mL[36m]eft?[0m \r\n", d);
      else
	SEND_TO_Q("\r\nWhich had is your predominant one, [R]ight or [L]eft? \r\n", d);
      STATE(d) = CON_QHANDEDNESS;
      return;
    } else {
       d->character->char_specials.email = str_dup(arg);
       if (strlen(d->character->char_specials.email) <= 4)
	 SEND_TO_Q("That is not a valid email address, please re-enter.\r\n", d);
       else {
	 STATE(d) = CON_QEMAIL_CNFRM;
	 sprintf(buf, "%sIs '%s%s%s' your email address?%s ", GET_ANSI(d) == TRUE ? KBCN : "",
		GET_ANSI(d) == TRUE ? KYEL : "",  d->character->char_specials.email,
		GET_ANSI(d) == TRUE ? KBCN : "", GET_ANSI(d) == TRUE ? KNRM : "");
	 SEND_TO_Q(buf, d);
       }
    }
    break;

  case CON_QEMAIL_CNFRM:
    switch (*arg) {
       case '/':
	 SEND_TO_Q("\r\nPlease enter your complete email address.\r\n",d);
	 STATE(d) = CON_QEMAIL;
	 return;
	 break;
       case 'y':
       case 'Y':
	 SEND_TO_Q("\r\nThank You.\r\n", d);
	 break;
       case 'n':
       case 'N':
       default:
	 SEND_TO_Q("\r\nOkay, what is it then? ", d);
	 STATE(d) = CON_QEMAIL;
	 return;
	 break;
    }

#if 0        /* taken out temporarily */
    SEND_TO_Q("Now you will enter your short description.  This is very\r\n"
	      "important in gameplay, so choose something good, that is\r\n"
	      "not offensive, and that will look nice when your postion\r\n"
	      "is appended to the end of it.  You will have a chance to\r\n"
	      "see what it looks like once you have typed it in.\r\n"
	      "Please enter your short description (1 line, < 40 characters)\r\n", d);
    STATE(d) = CON_QSHORT_DESC;
    break;

  case CON_QSHORT_DESC:
    if (strlen(arg) < 10) {
       SEND_TO_Q("You need a short description a bit longer than that.\r\n", d);
       return;
    } else if (strlen(arg) > 45) {
       SEND_TO_Q("Your short description must be less than 45 characters long.\r\n", d);
       return;
    } else {
       GET_LDESC(d->character) = str_dup(arg);
       sprintf(buf, "%s is standing here.\r\n", GET_LDESC(d->character));
       SEND_TO_Q(buf, d);
       SEND_TO_Q("Is this the short description that you want (Y/N)? ", d);
       STATE(d) = CON_SHORT_CNFRM;
    }
    break;

  case CON_SHORT_CNFRM:
    switch (*arg) {
      case 'y':
      case 'Y':
	SEND_TO_Q("Very good, now we need to set-up some keywords for you.\r\n", d);
	break;
      case 'n':
      case 'N':
	SEND_TO_Q("Okay then, re-enter your short description please.\r\n", d);
	STATE(d) = CON_QSHORT_DESC;
	return;
	break;
      default:
	sprintf(buf, "%s is standing here.\r\n", GET_LDESC(d->character));
	SEND_TO_Q(buf, d);
	SEND_TO_Q("Is this the short description that you want (Y/N)? ", d);
	return;
	break;
    }

    SEND_TO_Q("Please enter 3 or more keywords for yourself, they MUST\r\n"
	      "be separated by spaces, and they must be words contained\r\n"
	      "in your short description.  If you fail to abide by these\r\n"
	      "guidelines your character may be deleted!\r\n", d);
    STATE(d) = CON_QKEYWORDS;
    break;

  case CON_QKEYWORDS:
    if (count_words(arg) < 2) {
       SEND_TO_Q("You need atleast 3 words!\r\n", d);
       return;
    } else if (strlen(arg) > 40) {
       SEND_TO_Q("Your keyword listing is too long, please shorten it.\r\n", d);
       return;
    } else {
       sprintf(buf, " %s %s",
               LOWER( races[ GET_RACE(d->character) ].name ),
               LOWER(GET_NAME(d->character)));
       strcat(arg, buf);
       GET_KWDS(d->character) = str_dup(arg);
       sprintf(buf, "Your keywords are: %s\r\n", GET_KWDS(d->character));
       SEND_TO_Q(buf, d);
       SEND_TO_Q("Are these the keywords that you want (Y/N)? ", d);
       STATE(d) = CON_KEYWDS_CNFRM;
    }
    break;

  case CON_KEYWDS_CNFRM:
    switch (*arg) {
      case 'n':
      case 'N':
	SEND_TO_Q("Okay then, re-enter your keywords please.\r\n", d);
	STATE(d) = CON_QKEYWORDS;
	return;
	break;
      case 'y':
      case 'Y':
	SEND_TO_Q("Thank you.\r\n", d);
	break;
      default:
	sprintf(buf, "Your keywords are: %s\r\n", GET_KWDS(d->character));
	SEND_TO_Q(buf, d);
	SEND_TO_Q("Are these the keywords that you want (Y/N)? ", d);
	return;
	break;
    }
#endif

    if (GET_PFILEPOS(d->character) < 0)
      GET_PFILEPOS(d->character) = create_entry(GET_NAME(d->character));
    init_char(d->character);
    save_char(d->character, NOWHERE);
    page_string(d, newbm, 0);
    STATE(d) = CON_NEWM;

    sprintf(buf, "%s [%s] [%s] new player.", GET_NAME(d->character), d->host, d->character->char_specials.email);
    mudlog(buf, NRM, LVL_IMMORT, TRUE);
    break;

  case CON_NEWM:                /* read CR after printing Newbie Message */
    if (GET_ANSI(d) == TRUE)
      SEND_TO_Q(KYEL, d);
    SEND_TO_Q(motd, d);
    sprintf(buf, "\r\n\n%s*** PLEASE PRESS RETURN TO CONTINUE ***%s",
	 	GET_ANSI(d) == TRUE ? KBBL : "",
		GET_ANSI(d) == TRUE ? KNRM : "");
    SEND_TO_Q(buf, d);
    STATE(d) = CON_RMOTD;
    break;

  case CON_RMOTD:               /* read CR after printing motd   */
    if (GET_ANSI(d) == TRUE)
      SEND_TO_Q(ANSI_MENU, d);
    else
      SEND_TO_Q(MENU, d);
    STATE(d) = CON_MENU;
    break;

  case CON_MENU:                /* get selection from main menu  */
    if (GET_ANSI(d) == TRUE)
      SET_BIT(PRF_FLAGS(d->character), PRF_COLOR_1 | PRF_COLOR_2);

    switch (*arg) {
    case '0':
      do_gen_vfile(d->character, "", 0, SCMD_V_QUOTES);
      close_socket(d);
      break;

    case '1':
      load_result = enter_player_game( d );
      act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);

      STATE(d) = CON_PLAYING;
      if (!GET_LEVEL(d->character)) {
	do_start(d->character);
	send_to_char(START_MESSG, d->character);
      }
      sprintf(buf2, "You last logged on from %s%s%s on %s%s%s",
	      CCYEL(d->character, C_SPR), tmp_store.host,
	      CCNRM(d->character, C_SPR), CCYEL(d->character, C_SPR),
              ctime(&tmp_store.last_logon), CCNRM(d->character, C_SPR));
      send_to_char(buf2, d->character);
      if (PRF_FLAGGED(d->character, PRF_INFOBAR))
	do_infobar(d->character, 0, 0, SCMDB_REDRAW);
      look_at_room(d->character, 0);
      if (has_mail(GET_IDNUM(d->character))) {
	sprintf(buf2,  "You have %smail%s waiting for you.\r\n",
		CCCYN(d->character, C_SPR), CCNRM(d->character, C_SPR));
	send_to_char(buf2, d->character);
      }
      if (load_result == 2) {   /* rented items lost */
	send_to_char("\r\n\007You could not afford your rent!\r\n"
	  "Your possesions have been donated to the Monk's Monestary!\007\r\n",
		     d->character);
      }
      read_aliases(d->character); /* read in the players aliases - TD */

      d->prompt_mode = 1;
      break;

    case '2':
      SEND_TO_Q("Enter the text you'd like others to see when they look at you.\r\n", d);
      SEND_TO_Q("Terminate with a '@' on a new line.\r\n", d);
      if (d->character->player.description) {
	SEND_TO_Q("Old description:\r\n", d);
	SEND_TO_Q(d->character->player.description, d);
	free(d->character->player.description);
	d->character->player.description = NULL;
      }
      d->str = &d->character->player.description;
      d->max_str = EXDSCR_LENGTH;
      STATE(d) = CON_EXDESC;
      break;

    case '3':
      page_string(d, background, 0);
      STATE(d) = CON_RMOTD;
      break;

    case '4':
      SEND_TO_Q("\r\nEnter your old password: ", d);
      echo_off(d);
      STATE(d) = CON_CHPWD_GETOLD;
      break;

    case '5':
      SEND_TO_Q("\r\nEnter your password for verification: ", d);
      echo_off(d);
      STATE(d) = CON_DELCNF1;
      break;

    case '6':
      SEND_TO_Q(CLEAR_RESET, d);
      if (d->character) {
        do_who(d->character, "", 0, 0);         
        sprintf(buf, "\r\n\n%s*** PLEASE PRESS ENTER TO RETURN TO MAIN MENU ***%s", 
              GET_ANSI(d) == TRUE ? KBBL : "",
              GET_ANSI(d) == TRUE ? KNRM : "");
        SEND_TO_Q(buf, d);
        STATE(d) = CON_RMOTD;
      }
      break;
    default:
      SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
      if (GET_ANSI(d) == TRUE)
        SEND_TO_Q(ANSI_MENU, d);
      else
        SEND_TO_Q(MENU, d);
      break;
    }

    break;

  case CON_CHPWD_GETOLD:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      echo_on(d);
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      if (GET_ANSI(d) == TRUE)
      SEND_TO_Q(ANSI_MENU, d);
      else
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
      return;
    } else {
      SEND_TO_Q("\r\nEnter a new password: ", d);
      STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    break;

  case CON_DELCNF1:
    echo_on(d);
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      if (GET_ANSI(d) == TRUE)
        SEND_TO_Q(ANSI_MENU, d);
      else
        SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    } else {
      SEND_TO_Q("\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER PERMANENTLY.\r\n"
		"ARE YOU ABSOLUTELY SURE?\r\n\r\n"
		"Please type \"yes\" to confirm: ", d);
      STATE(d) = CON_DELCNF2;
    }
    break;

  case CON_DELCNF2:
    if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
      if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
	SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n", d);
	SEND_TO_Q("Character not deleted.\r\n\r\n", d);
	STATE(d) = CON_CLOSE;
	return;
      }
      if (GET_LEVEL(d->character) < LVL_GRGOD)
	SET_BIT(PLR_FLAGS(d->character), PLR_DELETED);
      save_char(d->character, NOWHERE);
      Crash_delete_file(GET_NAME(d->character));
      sprintf(buf, "Character '%s' deleted!\r\n"
	      "Goodbye.\r\n", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character),
	      GET_LEVEL(d->character));
      mudlog(buf, NRM, LVL_GOD, TRUE);
      STATE(d) = CON_CLOSE;
      return;
    } else {
      SEND_TO_Q("\r\nCharacter not deleted.\r\n", d);
      if (GET_ANSI(d) == TRUE)
        SEND_TO_Q(ANSI_MENU, d);
      else
        SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    }
    break;

  case CON_CLOSE:
    do_gen_vfile(d->character, "", 0, SCMD_V_QUOTES);
    close_socket(d);
    break;

  default:
    log("SYSERR: Nanny: illegal state of con'ness; closing connection");
    close_socket(d);
    break;
  }
}
