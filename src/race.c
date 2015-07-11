/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  RACE.C                                       Based on CircleMUD *
*  Usage: Source file for race-specific code (exp tables as well)         *
*         Guilds and Religion have been added as well.                    *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new races to be added.  If you're adding a new race,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new race.
 */


#include "conf.h"
#include "sysdep.h"

#include "protos.h"

void equip_char(struct char_data * ch, struct obj_data * obj, int pos);
void obj_to_char(struct obj_data * object, struct char_data * ch);
void obj_to_obj(struct obj_data * obj, struct obj_data * obj_to);
ACMD(do_help);
void save_guilds(void);
ACMD(do_infobar);
SPECIAL(guild_guard);
void ASSIGNMOB(long mob, SPECIAL(fname));
long asciiflag_conv(char *);
extern FILE *player_fl;
extern int    top_of_p_table;
extern struct player_index_element *player_table;
extern struct descriptor_data *descriptor_list;
void LowerString(char *s);

const char *pc_race_types[] = {
  "Human",
  "Athasianae",           /* Half-Elves      */
  "Thurgar",              /* High Dwarves    */
  "Gnome",
  "Dargonae",             /* Drow - Dark Elf */
  "Kinthalian Minotaur",  /* Big Minotaurs   */
  "Centaur",
  "Armachnae",            /* High Elves      */
  "Tarmirnae",            /* Grey Elves      */
  "Radinae",              /* Wild Elves      */
  "Kender",
  "Daerwar",              /* Smart Dwarves   */
  "Kaergar",              /* Holy Dwarves    */
  "Zakhar",               /* Dark Dwarves    */
  "Halfing",
  "Half-giant",
  "Sessanathi",           /* Lizard-man      */
  "Half-ogre",            /* Half-Ogres      */
  "Byterian Minotaur",    /* Small Minotaur  */
  "Gully Dwarf",          /* PC races end here          */
  "Vampire",              /* These are AFF bits for PCs */
  "Werewolf",             /*   "        "        "      */
  "Animal",
  "Undead",
  "Giant",
  "Humanoid",
  "Veggie",
  "Demon",
  "magical",
  "water-based",
  "Illusionary",
  "Red Dragon",
  "Blue Dragon",
  "Green Dragon",
  "Black Dragon",
  "White Dragon",
  "Bronze Dragon",
  "Copper Dragon",
  "Silver Dragon",
  "Gold Dragon",
  "Platinum Dragon",
  "Chromatic Dragon",     /* Tiamat Only */
  "Pixie",
  "Spider",
  "Serpent",
  "\n"
};


/* The abbreviations for race in who, act_info.c: */
const char *race_abbrev[] = {
  "Hum",
  "Ath",           /* Half-Elves      */
  "Thu",           /* High Dwarves    */
  "Gno",
  "Dar",           /* Drow - Dark Elf */
  "KiM",           /* Big Minotaurs   */
  "Cen",
  "Arm",           /* High Elves      */
  "Tar",           /* Grey Elves      */
  "Rad",           /* Wild Elves      */
  "Ken",
  "Dae",           /* Smart Dwarves   */
  "Kae",           /* Holy Dwarves    */
  "Zak",           /* Dark Dwarves    */
  "Hal",
  "HaG",           /* Half-Giant      */
  "Ses",           /* Lizard-man      */
  "HaO",           /* Half-Ogres      */
  "ByM",           /* Small Minotaur  */    /* Players END HERE */
  "GuD",           /* Gully Dwarf     */
  "Vam",           /* Vampire         */
  "Wer",           /* Werewolf        */
  "Ani",           /* Animal          */
  "Und",           /* Undead          */
  "Gia",           /* Giant           */
  "Hod",           /* Humanoid        */
  "Veg",           /* Vegitation      */
  "Dem",           /* Demon           */
  "Mag",           /* Magical Being   */
  "Wat",           /* Water           */
  "Ill",           /* Illusion        */
  "ReD",           /* Chromatic Dragons */
  "BlD",
  "GrD",
  "BkD",
  "WhD",
  "BrD",            /* Metallic Dragons */
  "CoD",
  "SiD",
  "GoD",
  "PlD",
  "ChD",            /* Tiamat Only     */
  "Pix",            /* Pixie           */
  "And",
  "Ser",
  "\n"
};

/* The menu for choosing a race in interpreter.c: */
const char *race_menu =
"\r\n"
"Select a race:\r\n"
"  [H]uman                  [A]thasianae\r\n"  /* Half-Elves      */
"  [T]hurgar                [G]nome\r\n"       /* High Dwarves    */
"  [D]argonae               Ki[N]thalas Minotaur\r\n"/* Drow - Dark Elf */
"  [C]entaur                Ar[M]achnae\r\n"   /* High Elves      */
"  Tarm[I]rnae              [R]adinae\r\n"     /* Grey/Wild Elves */
"  K[E]nder                 Daer[W]ar\r\n"     /* Smart Dwarves   */
"  [K]aergar                [Z]akhar\r\n"      /* Holy/Dark Dwarve*/
"  Hal[F]ling               Ha[L]f-Giant\r\n"
"  [S]essanathi             Half-[O]gre\r\n"   /* Lizard-man      */
"  [B]yterian Minotaur\r\n\r\n"                /* Small Minotaur  */
"  [?x] For Help on Race [x]\r\n"
"\r\n\r\nYour Choice? ";


const char *ansi_race_menu =
"\r\n[1m"
"[36mSelect a race:\r\n"
"  [[33mH[36m]uman                  [[33mA[36m]thasianae\r\n"  /* Half-Elves      */
"  [[33mT[36m]hurgar                [[33mG[36m]nome\r\n"       /* High Dwarves    */
"  [[33mD[36m]argonae               Ki[[33mN[36m]thalas Minotaur\r\n"/* Drow - Dark Elf */
"  [[33mC[36m]entaur                Ar[[33mM[36m]achnae\r\n"   /* High Elves      */
"  Tarm[[33mI[36m]rnae              [[33mR[36m]adinae\r\n"     /* Grey/Wild Elves */
"  K[[33mE[36m]nder                 Daer[[33mW[36m]ar\r\n"     /* Smart Dwarves   */
"  [[33mK[36m]aergar                [[33mZ[36m]akhar\r\n"      /* Holy/Dark Dwarve*/
"  Hal[[33mF[36m]ling               Ha[[33mL[36m]f-Giant\r\n"
"  [[33mS[36m]essanathi             Half-[[33mO[36m]gre\r\n"   /* Lizard-man      */
"  [[33mB[36m]yterian Minotaur\r\n\r\n"                /* Small Minotaur  */
"  [[33m?x[36m] For Help on Race [x]\r\n"
"\r\n\r\nYour Choice? ";

/*
 * The code to interpret a race letter (used in interpreter.c when a
 * new character is selecting a class).
 */

int parse_race(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case 'h':
    return RACE_HUMAN;
    break;
  case 'a':
    return RACE_ATHASIANAE;
    break;
  case 't':
    return RACE_THURGAR;
    break;
  case 'g':
    return RACE_GNOME;
    break;
  case 'd':
    return RACE_DARGONAE;
    break;
  case 'n':
    return RACE_KINTHALAS;
    break;
  case 'c':
    return RACE_CENTAUR;
    break;
  case 'm':
    return RACE_ARMACHNAE;
    break;
  case 'i':
    return RACE_TARMIRNAE;
    break;
  case 'r':
    return RACE_RADINAE;
    break;
  case 'e':
    return RACE_KENDER;
    break;
  case 'w':
    return RACE_DAERWAR;
    break;
  case 'k':
    return RACE_KAERGAR;
    break;
  case 'z':
    return RACE_ZAKHAR;
    break;
  case 'f':
    return RACE_HALFLING;
    break;
  case 'l':
    return RACE_HALFGIANT;
    break;
  case 's':
    return RACE_SESSANATHI;
    break;
  case 'o':
    return RACE_HALFOGRE;
    break;
  case 'b':
    return RACE_BYTERIAN;
    break;
  default:
    return RACE_UNDEFINED;
    break;
  }
}


long find_race_bitvector(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
    case 'h':
      return 1;
      break;
    case 'a':
      return 2;
      break;
    case 't':
      return 4;
      break;
    case 'g':
      return 8;
      break;
    case 'd':
      return 16;
      break;
    case 'n':
      return 32;
      break;
    case 'c':
      return 64;
      break;
    case 'm':
      return 128;
      break;
    case 'i':
      return 256;
      break;
    case 'r':
      return 512;
      break;
    case 'e':
      return 1024;
      break;
    case 'w':
      return 2048;
      break;
    case 'k':
      return 4096;
      break;
    case 'z':
      return 8192;
      break;
    case 'f':
      return 16384;
      break;
    case 'l':
      return 32768;
      break;
    case 's':
      return 65536;
      break;
    case 'o':
      return 131072;
      break;
    case 'b':
      return 262144;
      break;
    case '?':
    default:
      return 0;
      break;
  }
}


/* End of new menus */

const int regen_gain[] = {
   0,    /* Humans          */
   0,    /* Half-Elves      */
   0,    /* High Dwarves    */
   0,    /* Gnomes          */
   0,    /* Drow - Dark Elf */
   0,    /* Big Minotaurs   */
   0,    /* Centaurs        */
   0,    /* High Elves      */
   0,    /* Grey Elves      */
   0,    /* Wild Elves      */
   0,    /* Kender          */
   0,    /* Smart Dwarves   */
   0,    /* Holy Dwarves    */
   0,    /* Dark Dwarves    */
   0,    /* Halfing         */
   1,    /* Half-Giant      */
   5,    /* Lizard-man      */
   1,    /* Half-Ogres      */
   0,    /* Small Minotaur  */
   8,    /* Vampire         */
   3,    /* Werewolf        */
   0,    /* animal          */
   2,    /* undead          */
   1,    /* giant           */
   0,    /* humaniod        */
   0,    /* Gully Dwarf     */
   8,    /* Demon           */
   6,    /* magical         */
   0,    /* water race      */
   0,    /* Illusionary     */
   4,    /* Red Dragons     */
   6,    /* Blue Dragons    */
   6,    /* Green Dragons   */
   4,    /* Black Dragons   */
   2,    /* White Dragons   */
   2,    /* Bronze Dragons  */
   2,    /* Copper Dragons  */
   4,    /* Silver Dragons  */
   6,    /* Gold Dragons    */
   8     /* Platinum Dragons*/
};

room_num get_hometown(byte race) {
   switch (race) {
     case RACE_HUMAN:
     default:
       return 3021;
   }
}

/* Array containing all of the pertinent info for the races */
struct {
  int min_str, max_str;
  int min_int, max_int;
  int min_wis, max_wis;
  int min_dex, max_dex;
  int min_con, max_con;
  int min_cha, max_cha;
  int min_wil, max_wil;
} attributes[] = {
  /* Str     Int     Wis     Dex     Con     Cha    Will */
  { 6, 18,  6, 18,  6, 18,  6, 18,  6, 18,  6, 18,  6, 18},  /* Human     */
  { 8, 16,  8, 18,  8, 18,  8, 18,  5, 16, 10, 18, 10, 18},  /* Athasianae*/
  {12, 18,  8, 16,  8, 16,  4, 16, 12, 19,  6, 14,  6, 16},  /* Thurgar   */
  { 6, 15, 12, 19, 12, 19, 10, 18,  6, 14,  6, 14,  8, 16},  /* Gnome     */
  { 8, 17, 10, 19, 10, 18, 10, 18, 10, 17, 10, 16,  8, 16},  /* Dargonae  */
  {12, 19,  8, 14,  8, 14,  6, 17, 12, 19,  8, 15,  6, 15},  /* Kinthalas */
  { 8, 17, 10, 18, 10, 18, 10, 19, 10, 18,  6, 15,  8, 16},  /* Centaur   */
  { 8, 17, 10, 19, 10, 18,  8, 19,  8, 16, 10, 19, 10, 18},  /* Armachnae */
  { 8, 17, 10, 18, 10, 19,  8, 19,  8, 16, 10, 19, 10, 18},  /* Tarmirnae */
  { 9, 18, 10, 16, 10, 16,  8, 19,  8, 18, 10, 18, 10, 18},  /* Radinae   */
  { 6, 12, 12, 17, 12, 17, 16, 20,  6, 12,  6, 16, 10, 18},  /* Kender    */
  {12, 18,  8, 17,  8, 17,  8, 16, 12, 19,  6, 15, 10, 18},  /* Daerwar   */
  {12, 18,  8, 17,  8, 17,  8, 16, 12, 19,  6, 15, 10, 18},  /* Kaergar   */
  {12, 18,  8, 17,  8, 17,  8, 16, 12, 19,  6, 15, 10, 18},  /* Zakhar    */
  {10, 16,  8, 16,  8, 16, 14, 20,  8, 16,  6, 12, 10, 14},  /* Halfling  */
  {16, 24,  4,  8,  4,  8,  8, 16, 16, 24,  6, 10,  2,  6},  /* HalfGiant */
  { 8, 16,  8, 18,  8, 18,  8, 18,  8, 18,  8, 14, 10, 19},  /* Sessanathi*/
  {16, 22,  6, 12,  6, 12, 10, 18, 16, 22,  6,  8,  6, 12},  /* HalfOgre  */
  { 8, 16, 10, 18, 10, 18, 10, 18, 12, 16, 10, 16, 10, 17}   /* Byterian  */
};

void set_base_attribs(struct char_data *ch)
{
    ch->real_abils.intel =
	number(attributes[(int) GET_RACE(ch)].min_int, attributes[(int) GET_RACE(ch)].max_int);
    ch->real_abils.wis   =
	number(attributes[(int) GET_RACE(ch)].min_wis, attributes[(int) GET_RACE(ch)].max_wis);
    ch->real_abils.will =
	number(attributes[(int) GET_RACE(ch)].min_wil, attributes[(int) GET_RACE(ch)].max_wil);
    ch->real_abils.dex =
	number(attributes[(int) GET_RACE(ch)].min_dex, attributes[(int) GET_RACE(ch)].max_dex);
    ch->real_abils.str =
	number(attributes[(int) GET_RACE(ch)].min_str, attributes[(int) GET_RACE(ch)].max_str);
    ch->real_abils.con =
	number(attributes[(int) GET_RACE(ch)].min_con, attributes[(int) GET_RACE(ch)].max_con);
    ch->real_abils.cha =
	number(attributes[(int) GET_RACE(ch)].min_cha, attributes[(int) GET_RACE(ch)].max_cha);
    /* now assign them to the character */
    ch->aff_abils = ch->real_abils;
}

void print_plr_attribs(struct char_data *ch, int points)
{
    int  i = 0;
    char tempb[80];

#if SHOW_ATTRIBS
    sprintf(buf, "    +-[S]tr +-[I]nt +-[W]is +-[D]ex +-[C]on +-C[H]a +-Wi[L]l [Q]uit [?]Help\r\n");
    sprintf(buf, "%s       %-2d    %-2d    %-2d    %-2d    %-2d    %-2d   %-2d\r\n", buf,
	    ch->real_abils.str, ch->real_abils.intel, ch->real_abils.wis,
	    ch->real_abils.dex, ch->real_abils.con, ch->real_abils.cha,
	    ch->real_abils.will);
    sprintf(buf, "%sYou have %d points remaining: ", buf, points);
#else
    sprintf(buf, "%sYour attributes are:\r\n", GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.str / 2); i++)
	strcat(tempb, "*");
    sprintf(buf, "%s\r\n+-[%sS%s]tr: %s%s%s\r\n", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.intel / 2); i++)
	strcat(tempb, "*");
    sprintf(buf, "%s+-[%sI%s]nt: %s%s%s\r\n", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.wis / 2); i++)
	strcat(tempb, "*");
    sprintf(buf, "%s+-[%sW%s]is: %s%s%s\r\n", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.dex / 2); i++)
	strcat(tempb, "*");
    sprintf(buf, "%s+-[%sD%s]ex: %s%s%s\r\n", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.con / 2); i++)
	strcat(tempb, "*");
    sprintf(buf, "%s+-[%sC%s]on: %s%s%s\r\n", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.cha / 2); i++)
	strcat(tempb, "*");
    sprintf(buf, "%s+-C[%sh%s]a: %s%s%s\r\n", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.will / 2); i++)
	strcat(tempb, "*");
    sprintf(buf, "%s+-Wi[%sl%s]: %s%s%s\r\n", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    sprintf(buf, "%s  [%sR%s]eroll base stats (costs 1 point).\r\n", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    sprintf(buf, "%s\r\nRemaining Points: %s%-2d%s\r\n", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", points,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
#endif
    sprintf(buf, "%s\r\n([%sQ%s]uit, [%s?%s] Help) Your choice?%s ", buf,
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "",
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "",
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KNRM : "");
    send_to_char(buf, ch);
}

int add_attrib(struct char_data *ch, int points, int attrib)
{
    switch (attrib) {
       case 0:  /* str */
	 if (ch->real_abils.str < attributes[(int) GET_RACE(ch)].max_str) {
	    ch->real_abils.str += 1;
	    points--;
	 } else
	    send_to_char("Your strength is at its maximum.\r\n", ch);
	 break;
       case 1:  /* int */
	 if (ch->real_abils.intel < attributes[(int)GET_RACE(ch)].max_int) {
	    ch->real_abils.intel += 1;
	    points--;
	 } else
	    send_to_char("Your intelligence is at its maximum.\r\n", ch);
	 break;
       case 2:  /* wis */
	 if (ch->real_abils.wis < attributes[(int)GET_RACE(ch)].max_wis) {
	    ch->real_abils.wis += 1;
	    points--;
	 } else
	    send_to_char("Your wisdom is at its maximum.\r\n", ch);
	 break;
       case 3:  /* dex */
	 if (ch->real_abils.dex < attributes[(int)GET_RACE(ch)].max_dex) {
	    ch->real_abils.dex += 1;
	    points--;
	 } else
	    send_to_char("Your dexterity is at its maximum.\r\n", ch);
	 break;
       case 4:  /* con */
	 if (ch->real_abils.con < attributes[(int)GET_RACE(ch)].max_con) {
	    ch->real_abils.con += 1;
	    points--;
	 } else
	    send_to_char("Your constitution is at its maximum.\r\n", ch);
	 break;
       case 5:  /* cha */
	 if (ch->real_abils.cha < attributes[(int)GET_RACE(ch)].max_cha) {
	    ch->real_abils.cha += 1;
	    points--;
	 } else
	    send_to_char("Your charisma is at its maximum.\r\n", ch);
	 break;
       case 6:  /* will */
	 if (ch->real_abils.will < attributes[(int)GET_RACE(ch)].max_wil) {
	    ch->real_abils.will += 1;
	    points--;
	 } else
	    send_to_char("Your will power is at its maximum.\r\n", ch);
	 break;
    }
    return points;
}


int sub_attrib(struct char_data *ch, int points, int attrib)
{
    switch (attrib) {
       case 0:  /* str */
	 if (ch->real_abils.str > attributes[(int)GET_RACE(ch)].min_str) {
	    ch->real_abils.str -= 1;
	    points++;
	 } else
	    send_to_char("Your strength cannot go any lower.\r\n", ch);
	 break;
       case 1:  /* int */
	 if (ch->real_abils.intel > attributes[(int)GET_RACE(ch)].min_int) {
	    ch->real_abils.intel -= 1;
	    points++;
	 } else
	    send_to_char("Your intelligence cannot go any lower.\r\n", ch);
	 break;
       case 2:  /* wis */
	 if (ch->real_abils.wis > attributes[(int)GET_RACE(ch)].min_wis) {
	    ch->real_abils.wis -= 1;
	    points++;
	 } else
	    send_to_char("Your wisdom cannot go any lower.\r\n", ch);
	 break;
       case 3:  /* dex */
	 if (ch->real_abils.dex > attributes[(int)GET_RACE(ch)].min_dex) {
	    ch->real_abils.dex -= 1;
	    points++;
	 } else
	    send_to_char("Your dexterity cannot go any lower.\r\n", ch);
	 break;
       case 4:  /* con */
	 if (ch->real_abils.con > attributes[(int)GET_RACE(ch)].min_con) {
	    ch->real_abils.con -= 1;
	    points++;
	 } else
	    send_to_char("Your constitution cannot go any lower.\r\n", ch);
	 break;
       case 5:  /* cha */
	 if (ch->real_abils.cha > attributes[(int)GET_RACE(ch)].min_cha) {
	    ch->real_abils.cha -= 1;
	    points++;
	 } else
	    send_to_char("Your charisma cannot go any lower.\r\n", ch);
	 break;
       case 6:  /* will */
	 if (ch->real_abils.will > attributes[(int)GET_RACE(ch)].min_wil) {
	    ch->real_abils.will -= 1;
	    points++;
	 } else
	    send_to_char("Your will power cannot go any lower.\r\n", ch);
	 break;
    }
    return points;
}

#if LATER
/* Some races can't use certain items I guess... */
int invalid_race(struct char_data *ch, struct obj_data *obj) {
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_HUMAN)      && IS_HUMAN(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_HALFELF)    && IS_HALFELF(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_DWARF)      && IS_DWARF(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GNOME)      && IS_GNOME(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_DROW)       && IS_DROW(ch))  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_MINOTAUR)   && IS_MINOTAUR(ch))  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_CENTAUR)    && IS_CENTAUR(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_ELF)        && IS_ELF(ch)))
     return 1;
  else
     return 0;
}
#endif

/*
 * Roll the 6 stats for a character... each stat is made of the sum of
 * the best 3 out of 4 rolls of a 6-sided die.  Each race then decides
 * which priority will be given for the best to worst stats.  This is
 * only used now when a god rerolls a player.
 */
void roll_real_abils(struct char_data * ch)
{
  int i, j, k, temp;
  ubyte table[7];
  ubyte rolls[4];

  for (i = 0; i < 7; i++)
    table[i] = 0;

  for (i = 0; i < 7; i++) {

    for (j = 0; j < 4; j++)
      rolls[j] = number(1, 6);

    temp = rolls[0] + rolls[1] + rolls[2] + rolls[3] -
      MIN(rolls[0], MIN(rolls[1], MIN(rolls[2], rolls[3])));

    for (k = 0; k < 7; k++)
      if (table[k] < temp) {
	temp ^= table[k];
	table[k] ^= temp;
	temp ^= table[k];
      }
  }

  ch->real_abils.str_add = 0;

  switch (GET_RACE(ch)) {
     case RACE_HUMAN:
     default:
       ch->real_abils.wis   = table[0];
       ch->real_abils.intel = table[1];
       ch->real_abils.str   = table[2];
       ch->real_abils.dex   = table[3];
       ch->real_abils.con   = table[4];
       ch->real_abils.cha   = table[5];
       ch->real_abils.will  = table[6];
       if (ch->real_abils.str == 18)
	 ch->real_abils.str_add = number(0, 100);
       break;
  }
  ch->aff_abils = ch->real_abils;
}

struct {
  int min_hi, max_hi;    /* Hitroll */
  int min_da, max_da;    /* Damroll */
  int min_hp, max_hp;    /* Hit Pts */
  int min_ma, max_ma;    /* Piety   */
  int min_mo, max_mo;    /* Movemnt */
  int min_lhp, max_lhp;  /* Level Advance HPs */
  int min_lma, max_lma;  /* Level Advance Man */
  int min_lmo, max_lmo;  /* Level Advance Mov */
  int bage, max_bage;    /* Base age */
} race_pts[] = {
  /*   Hit       Dam      HPs       Man       Mov     LHPs   LMPs   LVPs   BAge*/
  {  1,   5,   1,   5,  12,  18,  80, 110,  80, 110,  1, 4,  3, 8,  1, 5,  16, 20},  /* Human     */
  {  1,   4,   1,   4,  10,  18,  85, 110,  80, 115,  1, 3,  4, 10, 2, 4,  16, 22},  /* Athasianae*/
  {  2,   6,   2,   6,  10,  20,  50,  80,  70, 100,  2, 4,  1, 2,  2, 3,  40, 90},  /* Thurgar   */
  {  1,   2,   0,   2,   8,  14, 100, 200,  80, 110,  1, 2,  6, 16, 3, 4,  60, 90},  /* Gnome     */
  {  3,   6,   1,   4,  10,  18,  90, 125,  80, 110,  1, 3,  4, 12, 2, 4,  100, 130},  /* Dargonae  */
  {  5,  10,   8,  16,  14,  24,  50,  70,  65, 100,  3, 6,  1, 2,  3, 9,  30, 60},  /* Kinthalas */
  {  4,   8,   8,  12,  12,  22,  80, 100, 100, 125,  2, 5,  2, 3,  9, 18, 30, 60},  /* Centaur   */
  {  0,   4,   1,   3,   8,  16, 100, 125,  80, 110,  1, 2,  4, 12, 2, 4,  100, 130},  /* Armachnae */
  {  0,   4,   1,   3,   8,  16, 100, 120,  80, 120,  1, 2,  4, 12, 2, 4,  100, 130},  /* Tarmirnae */
  {  2,   6,   3,   9,  12,  22,  40,  80,  80, 130,  2, 4,  1, 2,  6, 12, 90, 130},  /* Radinae   */
  {  0,   2,   1,   2,   8,  14,  40,  80, 125, 180,  1, 2,  1, 2,  8, 16, 20, 32},  /* Kender    */
  {  1,   3,   2,   3,  12,  20,  80, 120,  70, 100,  1, 3,  4, 8,  1, 2,  40, 90},  /* Daerwar   */
  {  1,   3,   2,   3,  12,  20,  40, 100,  70, 100,  1, 3,  1, 4,  2, 4,  40, 90},  /* Kaergar   */
  {  1,   3,   2,   3,  10,  20,  30,  50,  70, 100,  1, 3,  1, 2,  1, 1,  40, 90},  /* Zakhar    */
  {  1,   2,   1,   2,   8,  16,  35,  70, 110, 150,  1, 2,  1, 2,  7, 14, 20, 32},  /* Halfling  */
  {  8,  16,  10,  22,  24,  48,  10,  20,  30,  80,  4, 8,  0, 1,  3, 9,  18, 24},  /* HalfGiant */
  {  2,   3,   1,   2,  10,  20,  70, 110, 100, 125,  1, 5,  3, 9,  2, 8,  110, 135},  /* Sessanathi*/
  {  6,  12,   8,  16,  18,  32,   0,  20,  40,  90,  4, 6,  0, 0,  2, 6,  16, 22},  /* HalfOgre  */
  {  4,   6,   4,   8,  12,  22,  80, 110,  65, 100,  2, 4,  2, 3,  2, 8,  30, 60}  /* Byterian  */
};

void set_height_weight(struct char_data *ch)
{
   switch (GET_RACE(ch)) {
     case RACE_HUMAN:
     case RACE_SESSANATHI:
       if (IS_MALE(ch)) {
         GET_HEIGHT(ch) = (60 + dice(2, 10));
         GET_WEIGHT(ch) = (140 + dice(6, 10));
       } else { /* Females */
         GET_HEIGHT(ch) = (58 + dice(2, 10));
         GET_WEIGHT(ch) = (90 + dice(4, 10));
       }
       break;
     case RACE_ATHASIANAE:
       if (IS_MALE(ch)) {
         GET_HEIGHT(ch) = (62 + dice(2, 6));
         GET_WEIGHT(ch) = (110 + dice(3, 12));
       } else { /* Females */
         GET_HEIGHT(ch) = (60 + dice(2, 6));
         GET_WEIGHT(ch) = (80 + dice(3, 12));
       }
       break;
     case RACE_DARGONAE:
     case RACE_ARMACHNAE:
     case RACE_TARMIRNAE:
     case RACE_RADINAE:
       if (IS_MALE(ch)) {
         GET_HEIGHT(ch) = (62 + dice(2, 10));
         GET_WEIGHT(ch) = (90 + dice(3, 10));
       } else { /* Females */
         GET_HEIGHT(ch) = (60 + dice(2, 10));
         GET_WEIGHT(ch) = (70 + dice(3, 10));
       }
       break;
     case RACE_THURGAR:
     case RACE_DAERWAR:
     case RACE_KAERGAR:
     case RACE_ZAKHAR:
       if (IS_MALE(ch)) {
         GET_HEIGHT(ch) = (43 + dice(1, 10));
         GET_WEIGHT(ch) = (130 + dice(4, 10));
       } else { /* Females */
         GET_HEIGHT(ch) = (41 + dice(1, 10));
         GET_WEIGHT(ch) = (105 + dice(4, 10));
       }
       break;
     case RACE_GNOME:
       if (IS_MALE(ch)) {
         GET_HEIGHT(ch) = (38 + dice(1, 6));
         GET_WEIGHT(ch) = (70 + dice(5, 4));
       } else { /* Females */
         GET_HEIGHT(ch) = (36 + dice(1, 6));
         GET_WEIGHT(ch) = (68 + dice(4, 4));
       }
       break;    
     case RACE_KENDER:
     case RACE_HALFLING:
       if (IS_MALE(ch)) {
         GET_HEIGHT(ch) = (32 + dice(2, 8));
         GET_WEIGHT(ch) = (52 + dice(5, 4));
       } else { /* Females */
         GET_HEIGHT(ch) = (30 + dice(3, 8));
         GET_WEIGHT(ch) = (48 + dice(5, 4));
       }
       break;    
     case RACE_KINTHALAS:
     case RACE_BYTERIAN:
       if (IS_MALE(ch)) {
         GET_HEIGHT(ch) = (65 + dice(3, 10));
         GET_WEIGHT(ch) = (160 + dice(6, 10));
       } else { /* Females */
         GET_HEIGHT(ch) = (62 + dice(3, 8));
         GET_WEIGHT(ch) = (120 + dice(6, 10));
       }
       break;    
     case RACE_CENTAUR:
       GET_HEIGHT(ch) = (78  + dice(3, 10));
       GET_WEIGHT(ch) = (750 + dice(10, 12));
       break;
     case RACE_HALFGIANT:
       GET_HEIGHT(ch) = (92  + dice(2, 10));
       GET_WEIGHT(ch) = (360 + dice(6, 10));
       break;
     case RACE_HALFOGRE:
       GET_HEIGHT(ch) = (76  + dice(2, 10));
       GET_WEIGHT(ch) = (200 + dice(6, 10));
       break;
     default:
       GET_HEIGHT(ch) = (60  + dice(2, 10));
       GET_WEIGHT(ch) = (100 + dice(4, 10));
       break;
   }
}

/* Some initializations for characters, including initial skills */
void do_start(struct char_data * ch)
{
  void advance_level(struct char_data * ch);
  void newbie_basket(struct char_data * ch);

  GET_LEVEL(ch)     = 1;
  GET_EXP(ch)       = 1;
  GET_GUILD(ch)     = GUILD_NONE;
  GET_GUILD_LEV(ch) = 0;
  GET_PRACTICES(ch) = number(5, GET_WIS(ch));

  set_title(ch, NULL);
  GET_AGE_MOD(ch)   = number(race_pts[(int) GET_RACE(ch)].bage, race_pts[(int) GET_RACE(ch)].max_bage);
  set_height_weight(ch);

  if (GET_PERMALIGN(ch) == ALIGN_GOOD)
    GET_ALIGNMENT(ch) = 1000;
  else if (GET_PERMALIGN(ch) == ALIGN_EVIL)
    GET_ALIGNMENT(ch) = -1000;
  else
    GET_ALIGNMENT(ch) = 0;

  /* Check array to see mins and maxs for race's points */
  ch->points.hitroll  = number(race_pts[(int) GET_RACE(ch)].min_hi, race_pts[(int) GET_RACE(ch)].max_hi);
  ch->points.damroll  = number(race_pts[(int) GET_RACE(ch)].min_da, race_pts[(int) GET_RACE(ch)].max_da);
  ch->points.max_hit  = number(race_pts[(int) GET_RACE(ch)].min_hp, race_pts[(int) GET_RACE(ch)].max_hp);
  ch->points.max_mana = number(race_pts[(int) GET_RACE(ch)].min_ma, race_pts[(int) GET_RACE(ch)].max_ma);
  ch->points.max_move = number(race_pts[(int) GET_RACE(ch)].min_mo, race_pts[(int) GET_RACE(ch)].max_mo);

  /* now raise the player a level */
  advance_level(ch);

  /* Restore the character to full health, mana, move */
  GET_HIT(ch)  = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  /* Retore the character to perfect condition */
  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL)   = 24;
  GET_COND(ch, TIRED)  = 24;
  GET_COND(ch, DRUNK)  = 0;
  GET_WOUNDS(ch)       = 0;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  newbie_basket(ch);
}


/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each race every time they gain a level.
 */
void advance_level(struct char_data * ch)
{
  int add_hp = 0, add_mana = 0, add_move = 0, i;

  extern struct wis_app_type wis_app[];
  extern struct con_app_type con_app[];

  add_hp   = number(race_pts[(int) GET_RACE(ch)].min_lhp, race_pts[(int) GET_RACE(ch)].max_lhp);
  add_hp  += con_app[GET_CON(ch)].hitp;
  add_mana = number(race_pts[(int) GET_RACE(ch)].min_lma, race_pts[(int) GET_RACE(ch)].max_lma);
  add_move = number(race_pts[(int) GET_RACE(ch)].min_lmo, race_pts[(int) GET_RACE(ch)].max_lmo);

  ch->points.max_hit  += MAX(1, add_hp);
  ch->points.max_move += MAX(1, add_move);
  ch->points.max_mana += MAX(0, add_mana);

  GET_PRACTICES(ch)   += MAX(1, (int)wis_app[GET_WIS(ch)].bonus);

  GET_HITROLL(ch)     += number(0, 1);
  GET_DAMROLL(ch)     += number(0, 1);

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    for (i = 0; i < 3; i++)
       GET_COND(ch, i) = (char) -1;
    SET_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT);
  }
  save_char(ch, NOWHERE);

  sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
  mudlog(buf, BRF, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
  if (PRF_FLAGGED(ch, PRF_INFOBAR))
    do_infobar(ch, 0, 0, SCMDB_GAINLEVEL);
}

/* give the char some armor and a weapon... does anyone keep this stuff? */
void newbie_basket(struct char_data *ch)
{
  struct obj_data *obj, *obj2;
#if 0
  struct extra_descr_data *iterator;
#endif

  obj = read_object(OBJ_VNUM_DEFAULT_LIGHT, VNUMBER);
  equip_char(ch, obj, WEAR_HOLD);

  obj = read_object(OBJ_VNUM_DEFAULT_DAGGER, VNUMBER);
  equip_char(ch, obj, WEAR_WIELD);
#if 0
  /* Personalize the newbie daggerr */
  sprintf(buf, "It appears to have a monogram denoting that it is owned by %s.\r\n", GET_NAME(ch));
  iterator = obj->ex_description;
  while (iterator != NULL)
    iterator = iterator->next;
  iterator = (struct extra_descr_data *) malloc(sizeof(struct extra_descr_data));
  iterator->keyword = str_dup("inscription");
  iterator->description = str_dup(buf);
  /* End of personalization */
#endif
  obj = read_object(OBJ_VNUM_DEFAULT_VEST, VNUMBER);
  equip_char(ch, obj, WEAR_BODY);

  obj = read_object(OBJ_VNUM_DEFAULT_BELT, VNUMBER);
  equip_char(ch, obj, WEAR_WAIST);

  obj = read_object(OBJ_VNUM_DEFAULT_LEGGINGS, VNUMBER);
  equip_char(ch, obj, WEAR_LEGS);

  obj2 = read_object(OBJ_VNUM_DEFAULT_PACK, VNUMBER);
  obj_to_char(obj2, ch);

  obj = read_object(OBJ_VNUM_DEFAULT_FOOD, VNUMBER);
  obj_to_obj(obj, obj2);

  obj = read_object(OBJ_VNUM_DEFAULT_FOOD, VNUMBER);
  obj_to_obj(obj, obj2);

  obj = read_object(OBJ_VNUM_DEFAULT_FOOD, VNUMBER);
  obj_to_obj(obj, obj2);

  obj = read_object(OBJ_VNUM_DEFAULT_MAP, VNUMBER);
  obj_to_obj(obj, obj2);

  obj = read_object(OBJ_VNUM_DEFAULT_DRINK, VNUMBER);
  obj_to_char(obj, ch);

  /* we need to give the players a holy symbol or spell book when they    *
   * start and make these items used in the casting or praying for spells */
  GET_GOLD(ch) = number(5, 25);
}


/* This function returns the amount of exp needed to reach the passed level */
static const long min_exp_by_level[LVL_CREATOR + 1] =
{
    /*  0                         5  level  */
	0, 1, 500, 1500, 2700, 5000,
    /*                            10 level */
    10000, 20000, 40000, 60000, 90000,
    /*                                 15 level */
    135000, 250000, 375000, 750000, 1125000,
    /*                                  20 level */
    1500000, 1875000, 2250000, 2625000, 3000000,
    /*                                    25 level */
    3475000, 3950000, 4400000, 4950000, 5420000,
    /*                                        30 level */
    6275000, 7550000, 9000000, 10800000, 12950000,
    /*                                         35 level */
    15500000, 18000000, 21600000, 25650000, 29900000,
    /*                                          40 level */
    34000000, 39600000, 46900000, 52000000, 63000000,
    /*                                              45 level */
    74500000, 86000000, 99900000, 199990000, 850000000,
    /*                                                 50 level */
    950000000, 954000000, 956000000, 958000000, 980000000
};

long exp_needed(int level)
{
    if ((unsigned) level > LVL_CREATOR) {
       log("SYSERR: level argument to min_exp out of range!");
    }
    return (min_exp_by_level[level]);
}


/*
 * SPELLS AND SKILLS.  This area defines which spells are assigned to
 * which classes, and the minimum level the character must be to use
 * the spell or skill.
 */
void init_spell_spheres(void)
{
  /* General   Skill                Sphere */
  skill_sphere(SKILL_BANDAGE,       SPHERE_GENERAL, 1);
  skill_sphere(SKILL_EVALUATE,      SPHERE_GENERAL, 25);
#if 0
  skill_sphere(SKILL_BRIBERY,       SPHERE_GENERAL, 1);
  skill_sphere(SKILL_CLIMB,         SPHERE_GENERAL, 1);
  skill_sphere(SKILL_DRAGON_RIDE,   SPHERE_GENERAL, 50);
  skill_sphere(SKILL_HAGGLE,        SPHERE_GENERAL, 25);
  skill_sphere(SKILL_HUNT,          SPHERE_GENERAL, 50);
  skill_sphere(SKILL_KNOW_UNDEAD,   SPHERE_GENERAL, 1);
  skill_sphere(SKILL_KNOW_VEGGIE,   SPHERE_GENERAL, 1);
  skill_sphere(SKILL_KNOW_DEMON,    SPHERE_GENERAL, 1);
  skill_sphere(SKILL_KNOW_ANIMAL,   SPHERE_GENERAL, 1);
  skill_sphere(SKILL_KNOW_REPTILE,  SPHERE_GENERAL, 1);
  skill_sphere(SKILL_KNOW_HUMANOID, SPHERE_GENERAL, 1);
  skill_sphere(SKILL_KNOW_OTHER,    SPHERE_GENERAL, 1);
  skill_sphere(SKILL_MARKSMANSHIP,  SPHERE_GENERAL, 25);
#endif
  skill_sphere(SKILL_READ_MAGIC,    SPHERE_GENERAL, 1);
  skill_sphere(SKILL_RIDE,          SPHERE_GENERAL, 1);
#if 0
  skill_sphere(SKILL_SIGN,          SPHERE_GENERAL, 1);
  skill_sphere(SKILL_SWIM,          SPHERE_GENERAL, 1);
#endif
  skill_sphere(SKILL_TRACK,         SPHERE_GENERAL, 1);

  /* Prayer    Spell-Sphere              Sphere */
  skill_sphere(SPHERE_ACTS_OF_DEITIES,   SPHERE_PRAYER, 40);
  skill_sphere(SPHERE_AFFLICTIONS,       SPHERE_PRAYER, 30);
  skill_sphere(SPHERE_CURES,             SPHERE_PRAYER, 1);
  skill_sphere(SPHERE_HAND_OF_DEITIES,   SPHERE_PRAYER, 20);
  skill_sphere(SPHERE_FAITH,             SPHERE_PRAYER, 10);

  /* Arcane    Spell-Sphere              Sphere */
  skill_sphere(SPHERE_CONJURATION,       SPHERE_ARCANE, 40);
  skill_sphere(SPHERE_DIVINATION,        SPHERE_ARCANE, 1);
  skill_sphere(SPHERE_ENCHANTMENT,       SPHERE_ARCANE, 10);
  skill_sphere(SPHERE_ENTROPIC,          SPHERE_ARCANE, 30);
  skill_sphere(SPHERE_INVOCATION,        SPHERE_ARCANE, 20);
  skill_sphere(SPHERE_PHANTASMIC,        SPHERE_ARCANE, 50);

  /* Thievery  Skill-Sphere              Sphere */
  skill_sphere(SPHERE_LOOTING,           SPHERE_GTHIEVERY, 10);
  skill_sphere(SPHERE_MURDER,            SPHERE_GTHIEVERY, 20);
  skill_sphere(SPHERE_THIEVING,          SPHERE_GTHIEVERY, 1);

  /* Combat    Skill-Sphere              Sphere */
  skill_sphere(SPHERE_HAND_TO_HAND,      SPHERE_COMBAT, 10);
  skill_sphere(SPHERE_MARTIAL_ARTS,      SPHERE_COMBAT, 20);
  skill_sphere(SPHERE_WEAPONS,           SPHERE_COMBAT, 1);

  /* Prayer -  Spell                     Acts of Deities */
  spell_sphere(SPELL_CALL_LIGHTNING,     SPHERE_ACTS_OF_DEITIES,  15,  0);
  spell_sphere(SPELL_CHARM,              SPHERE_ACTS_OF_DEITIES,  70,  3);
  spell_sphere(SPELL_CONTROL_WEATHER,    SPHERE_ACTS_OF_DEITIES,  10,  2);
  spell_sphere(SPELL_CURSE,              SPHERE_ACTS_OF_DEITIES,   1,  0);
  spell_sphere(SPELL_EARTHQUAKE,         SPHERE_ACTS_OF_DEITIES,  15,  4);
  spell_sphere(SPELL_GATE_I,             SPHERE_ACTS_OF_DEITIES,  20,  0);
  spell_sphere(SPELL_GATE_II,            SPHERE_ACTS_OF_DEITIES,  30,  1);
  spell_sphere(SPELL_GATE_III,           SPHERE_ACTS_OF_DEITIES,  40,  2);
  spell_sphere(SPELL_GATE_IV,            SPHERE_ACTS_OF_DEITIES,  50,  3);
  spell_sphere(SPELL_GROUP_PORTAL,       SPHERE_ACTS_OF_DEITIES,  85, 10);
#if 0
  spell_sphere(SPELL_MEDUSA_GAZE,        SPHERE_ACTS_OF_DEITIES,  60,  6);
#endif
  spell_sphere(SPELL_METEOR_SWARM,       SPHERE_ACTS_OF_DEITIES,  90,  9);
  spell_sphere(SPELL_PLAGUE,             SPHERE_ACTS_OF_DEITIES,  95, 12);
  spell_sphere(SPELL_PORTAL,             SPHERE_ACTS_OF_DEITIES,  35,  5);
#if 0
  spell_sphere(SPELL_RESURRECTION,       SPHERE_ACTS_OF_DEITIES, 100, 20);
#endif

  /* Prayer -  Spell                     Afflictions */
  spell_sphere(SPELL_BLIND,              SPHERE_AFFLICTIONS,  1, 0);
#if 0
  spell_sphere(SPELL_CRIPPLE,            SPHERE_AFFLICTIONS, 10, 0);
#endif
  spell_sphere(SPELL_DISEASE,            SPHERE_AFFLICTIONS, 10, 0);
  spell_sphere(SPELL_HARM_LIGHT,         SPHERE_AFFLICTIONS,  1, 0);
  spell_sphere(SPELL_HARM_SERIOUS,       SPHERE_AFFLICTIONS, 10, 0);
  spell_sphere(SPELL_HARM_CRITICAL,      SPHERE_AFFLICTIONS, 20, 1);
  spell_sphere(SPELL_HARM,               SPHERE_AFFLICTIONS, 60, 5);
  spell_sphere(SPELL_POISON,             SPHERE_AFFLICTIONS, 30, 2);
#if 0
  spell_sphere(SPELL_PARALYZE,           SPHERE_AFFLICTIONS, 40, 2);
#endif

  /* Prayer -  Spell                     Cures */
  spell_sphere(SPELL_CURE_BLIND,         SPHERE_CURES,  5, 1);
  spell_sphere(SPELL_CURE_DISEASE,       SPHERE_CURES, 25, 1);
  spell_sphere(SPELL_CURE_PARALYSIS,     SPHERE_CURES, 35, 2);
  spell_sphere(SPELL_CURE_POISON,        SPHERE_CURES, 15, 1);
  spell_sphere(SPELL_GROUP_HEAL,         SPHERE_CURES, 95, 10);
  spell_sphere(SPELL_HEAL_LIGHT,         SPHERE_CURES,  1, 0);
  spell_sphere(SPELL_HEAL_SERIOUS,       SPHERE_CURES, 10, 0);
  spell_sphere(SPELL_HEAL_CRITIC,        SPHERE_CURES, 30, 1);
  spell_sphere(SPELL_HEAL,               SPHERE_CURES, 60, 5);
  spell_sphere(SPELL_REFRESH,            SPHERE_CURES, 50, 3);
#if 0
  spell_sphere(SPELL_KNIT_BONE,          SPHERE_CURES, 40, 2);
  spell_sphere(SPELL_RESTORE_LIMB,       SPHERE_CURES, 70, 6);
#endif
  /* Prayer -  Spell                     Hand Of Deities */
  spell_sphere(SPELL_ARMOR,              SPHERE_HAND_OF_DEITIES,   1, 0);
#if 0
  spell_sphere(SPELL_ASTRAL_WALK,        SPHERE_HAND_OF_DEITIES, 100, 10);
#endif
  spell_sphere(SPELL_BALANCE,            SPHERE_HAND_OF_DEITIES,  70, 8);
  spell_sphere(SPELL_BLACKMANTLE,        SPHERE_HAND_OF_DEITIES,  75, 8);
  spell_sphere(SPELL_BLESS,              SPHERE_HAND_OF_DEITIES,   2, 0);
  spell_sphere(SPELL_CORRUPT,            SPHERE_HAND_OF_DEITIES,  70, 8);
  spell_sphere(SPELL_CREATE_FOOD,        SPHERE_HAND_OF_DEITIES,  10, 0);
  spell_sphere(SPELL_CREATE_WATER,       SPHERE_HAND_OF_DEITIES,  10, 0);
#if 0
  spell_sphere(SPELL_FLAMESTRIKE,        SPHERE_HAND_OF_DEITIES,  90, 5);
#endif
  spell_sphere(SPELL_GROUP_ARMOR,        SPHERE_HAND_OF_DEITIES,  55, 2);
  spell_sphere(SPELL_INFRAVISION,        SPHERE_HAND_OF_DEITIES,  15, 0);
  spell_sphere(SPELL_PURIFY,             SPHERE_HAND_OF_DEITIES,  70, 8);
  spell_sphere(SPELL_REMOVE_CURSE,       SPHERE_HAND_OF_DEITIES,  40, 1);
  spell_sphere(SPELL_SANCTUARY,          SPHERE_HAND_OF_DEITIES,  60, 5);
  spell_sphere(SPELL_SILENCE,            SPHERE_HAND_OF_DEITIES,  50, 2);
  spell_sphere(SPELL_WATERBREATH,        SPHERE_HAND_OF_DEITIES,  30, 0);
  spell_sphere(SPELL_WATERWALK,          SPHERE_HAND_OF_DEITIES,  35, 0);
#if 0
  spell_sphere(SPELL_SUMMON,             SPHERE_HAND_OF_DEITIES, 100, 25);
  spell_sphere(SPELL_WORD_OF_RECALL,     SPHERE_HAND_OF_DEITIES, 100);
#endif

  /* Prayer -  Spell                     Faith */
  spell_sphere(SPELL_DISPEL_EVIL,        SPHERE_FAITH, 50, 1);
  spell_sphere(SPELL_DISPEL_GOOD,        SPHERE_FAITH, 50, 1);
  skill_sphere(SKILL_TURN_UNDEAD,        SPHERE_FAITH, 35);
  spell_sphere(SPELL_PROT_FROM_EVIL,     SPHERE_FAITH, 10, 0);
  spell_sphere(SPELL_PROT_FROM_GOOD,     SPHERE_FAITH, 10, 0);

  /* Arcane -  Spell                       Conjuration */
  spell_sphere(SPELL_AERIAL_SERVANT,       SPHERE_CONJURATION,  90, 20);
  spell_sphere(SPELL_BLADEBARRIER,         SPHERE_CONJURATION,  75, 2);
  spell_sphere(SPELL_MONSUM_I,             SPHERE_CONJURATION,   1, 0);
  spell_sphere(SPELL_MONSUM_II,            SPHERE_CONJURATION,  10, 1);
  spell_sphere(SPELL_MONSUM_III,           SPHERE_CONJURATION,  20, 2);
  spell_sphere(SPELL_MONSUM_IV,            SPHERE_CONJURATION,  30, 3);
  spell_sphere(SPELL_POWER_KILL,           SPHERE_CONJURATION, 100, 30);
  spell_sphere(SPELL_SUM_AIR_ELE,          SPHERE_CONJURATION,  40, 5);
  spell_sphere(SPELL_SUM_EARTH_ELE,        SPHERE_CONJURATION,  60, 15);
  spell_sphere(SPELL_SUM_FIRE_ELE,         SPHERE_CONJURATION,  70, 20);
  spell_sphere(SPELL_SUM_WATER_ELE,        SPHERE_CONJURATION,  50, 10);
  spell_sphere(SPELL_TELEPORT,             SPHERE_CONJURATION,   5, 1);

  /* Arcane -  Spell                       Divination */
  spell_sphere(SPELL_SOUL_SEARCH,          SPHERE_DIVINATION,   1,  0);
  spell_sphere(SPELL_DETECT_ILLUSION,      SPHERE_DIVINATION,  20,  1);
  spell_sphere(SPELL_DETECT_INVIS,         SPHERE_DIVINATION,   5,  0);
  spell_sphere(SPELL_DETECT_MAGIC,         SPHERE_DIVINATION,  15,  0);
  spell_sphere(SPELL_DETECT_POISON,        SPHERE_DIVINATION,  10,  0);
  spell_sphere(SPELL_LOCATE_OBJECT,        SPHERE_DIVINATION, 100, 10);
  spell_sphere(SPELL_SENSE_LIFE,           SPHERE_DIVINATION,  10,  0);
  spell_sphere(SPELL_WIZARD_EYE,           SPHERE_DIVINATION,  35,  4);

  /* Arcane -  Spell                       Enchantment */
  spell_sphere(SPELL_BLACKMANTLE,          SPHERE_ENCHANTMENT, 60, 10);
  spell_sphere(SPELL_BURNING_HANDS,        SPHERE_ENCHANTMENT,  1, 0);
  spell_sphere(SPELL_CALM,                 SPHERE_ENCHANTMENT, 25, 2);
  spell_sphere(SPELL_CHILL_TOUCH,          SPHERE_ENCHANTMENT,  5, 0);
  spell_sphere(SPELL_CURSE_ARMOR,          SPHERE_ENCHANTMENT, 40, 5);
  spell_sphere(SPELL_CURSE_WEAPON,         SPHERE_ENCHANTMENT, 45, 5);
  spell_sphere(SPELL_ENCHANT_ARMOR,        SPHERE_ENCHANTMENT, 50, 8);
  spell_sphere(SPELL_ENCHANT_WEAPON,       SPHERE_ENCHANTMENT, 55, 8);
  spell_sphere(SPELL_FIREWALL,             SPHERE_ENCHANTMENT, 65, 1);
  spell_sphere(SPELL_FLY,                  SPHERE_ENCHANTMENT, 35, 3);
  spell_sphere(SPELL_GHOSTLY_SHIELD,       SPHERE_ENCHANTMENT,  1, 0);
  spell_sphere(SPELL_GROUP_PASS_DOOR,      SPHERE_ENCHANTMENT, 80, 2);
  spell_sphere(SPELL_HASTE,                SPHERE_ENCHANTMENT, 65, 6);
#if 0
  spell_sphere(SPELL_KNOCK,                SPHERE_ENCHANTMENT, 15, 1);
  spell_sphere(SPELL_MIN_GLOBE,            SPHERE_ENCHANTMENT, 50, 5);
  spell_sphere(SPELL_MAJ_GLOBE,            SPHERE_ENCHANTMENT, 75, 8);
  spell_sphere(SPELL_PASS_DOOR,            SPHERE_ENCHANTMENT, 20, 1);
#endif
  spell_sphere(SPELL_SHOCKING_GRASP,       SPHERE_ENCHANTMENT, 10, 0);
  spell_sphere(SPELL_SLEEP,                SPHERE_ENCHANTMENT,  5, 1);
  spell_sphere(SPELL_STRENGTH,             SPHERE_ENCHANTMENT, 10, 1);
  spell_sphere(SPELL_TROLLS_BLOOD,         SPHERE_ENCHANTMENT, 85, 8);
  spell_sphere(SPELL_UNDEAD_EYES,          SPHERE_ENCHANTMENT, 15, 1);

  /* Arcane -  Spell                       Entropic */
  spell_sphere(SPELL_ANIMATE_DEAD,         SPHERE_ENTROPIC,  1,  0);
  spell_sphere(SPELL_CLONE,                SPHERE_ENTROPIC,100,  1);
  spell_sphere(SPELL_LIFESTEAL,            SPHERE_ENTROPIC, 50, 10);
  spell_sphere(SPELL_REMOVE_POISON,	   SPHERE_ENTROPIC, 20, 10);
#if  0
  spell_sphere(SPELL_SOUL_SUM_I,           SPHERE_ENTROPIC, 10,  1);
  spell_sphere(SPELL_SOUL_SUM_II,          SPHERE_ENTROPIC, 20,  2);
  spell_sphere(SPELL_SOUL_SUM_III,         SPHERE_ENTROPIC, 30,  3);
  spell_sphere(SPELL_SOUL_SUM_IV,          SPHERE_ENTROPIC, 40,  4);
#endif
  /* Arcane -  Spell                       Invocation */
  spell_sphere(SPELL_DISPEL_MAGIC,         SPHERE_INVOCATION, 10,  0);
  spell_sphere(SPELL_ENERGY_DRAIN,         SPHERE_INVOCATION, 10,  1);
  spell_sphere(SPELL_ICE_SHOWER,           SPHERE_INVOCATION, 30, 10);
  spell_sphere(SPELL_FIREBALL,             SPHERE_INVOCATION, 25,  1);
  spell_sphere(SPELL_FIRE_STORM,           SPHERE_INVOCATION, 50, 20);
#if 0
  spell_sphere(SPELL_FLAME_BOLT,           SPHERE_INVOCATION, 25,  2);
  spell_sphere(SPELL_FORCE_BOLT,           SPHERE_INVOCATION, 50, 12);
#endif
  spell_sphere(SPELL_LIGHTNING_BOLT,       SPHERE_INVOCATION, 20,  0);
  spell_sphere(SPELL_MAGIC_MISSILE,        SPHERE_INVOCATION,  1,  0);

  /* Arcane -  Spell                       Phantasmic */
  spell_sphere(SPELL_COLOR_SPRAY,          SPHERE_PHANTASMIC, 25, 2);
  spell_sphere(SPELL_DISPEL_ILLUSION,      SPHERE_PHANTASMIC, 40, 2);
#if 0
  spell_sphere(SPELL_DOUBLE_VISION,        SPHERE_PHANTASMIC, 15, 0);
  spell_sphere(SPELL_FALSE_GLOW,           SPHERE_PHANTASMIC,  1, 0);
  spell_sphere(SPELL_HYPNOSIS,             SPHERE_PHANTASMIC, 70, 8);
#endif
  spell_sphere(SPELL_ILLUSIONARY_PRES_I,   SPHERE_PHANTASMIC, 10, 1);
  spell_sphere(SPELL_ILLUSIONARY_PRES_II,  SPHERE_PHANTASMIC, 20, 5);
  spell_sphere(SPELL_ILLUSIONARY_PRES_III, SPHERE_PHANTASMIC, 30, 10);
  spell_sphere(SPELL_ILLUSIONARY_PRES_IV,  SPHERE_PHANTASMIC, 40, 20);
  spell_sphere(SPELL_INVISIBLE,            SPHERE_PHANTASMIC,  1, 0);
  spell_sphere(SPELL_MASK_PRESENCE,        SPHERE_PHANTASMIC, 25, 5);

  /* Thievery  Skill                       Looting */
#if 0
  skill_sphere(SKILL_DETECT_TRAP,          SPHERE_LOOTING, 10);
  skill_sphere(SKILL_REMOVE_TRAP,          SPHERE_LOOTING, 30);
  skill_sphere(SKILL_SET_TRAP,             SPHERE_LOOTING, 50);
#endif
  skill_sphere(SKILL_PEEK,                 SPHERE_LOOTING,  1);
  skill_sphere(SKILL_PICK_LOCK,            SPHERE_LOOTING, 10);
  skill_sphere(SKILL_SEARCH,               SPHERE_LOOTING,  5);
  skill_sphere(SKILL_STEAL,                SPHERE_LOOTING, 10);
  skill_sphere(SKILL_TREADLIGHT,           SPHERE_LOOTING, 30);

  /* Thievery  Skill                       Murder */
  skill_sphere(SKILL_BACKSTAB,             SPHERE_MURDER,  1);
  skill_sphere(SKILL_CIRCLE,               SPHERE_MURDER, 50);
  skill_sphere(SKILL_POISON_WEAPON,        SPHERE_MURDER, 60);
  skill_sphere(SKILL_GARROTTE,             SPHERE_MURDER, 95);
  skill_sphere(SKILL_STAB,                 SPHERE_MURDER, 20);
  skill_sphere(SKILL_CUDGEL,               SPHERE_MURDER, 90);
#if 0
  skill_sphere(SKILL_MANUFACTURE_POISONS,  SPHERE_MURDER, 80);
#endif

  /* Thievery  Skill                       Thievery */
#if 0
  skill_sphere(SKILL_DISGUISE,             SPHERE_THIEVING, 80);
  skill_sphere(SKILL_SPY,                  SPHERE_THIEVING, 20);
#endif
  skill_sphere(SKILL_HIDE,                 SPHERE_THIEVING,  5);
  skill_sphere(SKILL_SHADOW,               SPHERE_THIEVING, 40);
  skill_sphere(SKILL_SNEAK,                SPHERE_THIEVING,  1);

  /* Combat    Skill                       Hand to Hand */
  skill_sphere(SKILL_BASH,                 SPHERE_HAND_TO_HAND,   5);
  skill_sphere(SKILL_BERSERK,              SPHERE_HAND_TO_HAND,  70);
#if 0
  skill_sphere(SKILL_BODYSLAM,             SPHERE_HAND_TO_HAND,  40);
  skill_sphere(SKILL_DEATHSTROKE,          SPHERE_HAND_TO_HAND, 100);
  skill_sphere(SKILL_DISARM,               SPHERE_HAND_TO_HAND,  30);
  skill_sphere(SKILL_DOORBASH,             SPHERE_HAND_TO_HAND,  80);
  skill_sphere(SKILL_HEADBUTT,             SPHERE_HAND_TO_HAND,  10);
  skill_sphere(SKILL_PUGILISM,             SPHERE_HAND_TO_HAND,  30);
  skill_sphere(SKILL_WRESTLING,            SPHERE_HAND_TO_HAND,  70);
#endif
  skill_sphere(SKILL_DBL_ATTACK,           SPHERE_HAND_TO_HAND,  40);
  skill_sphere(SKILL_GRAPPLE,              SPHERE_HAND_TO_HAND,  20);
  skill_sphere(SKILL_KICK,                 SPHERE_HAND_TO_HAND,   1);
  skill_sphere(SKILL_PUMMEL,               SPHERE_HAND_TO_HAND,  20);
  skill_sphere(SKILL_PUNCH,                SPHERE_HAND_TO_HAND,   5);
  skill_sphere(SKILL_RESCUE,               SPHERE_HAND_TO_HAND,  50);
  skill_sphere(SKILL_SWITCH_OPPONENTS,     SPHERE_HAND_TO_HAND,  60);
  skill_sphere(SKILL_TRI_ATTACK,           SPHERE_HAND_TO_HAND,  90);

  /* Combat    Skill                       Martial Arts */
  skill_sphere(SKILL_DODGE,                SPHERE_MARTIAL_ARTS, 5);
#if 0
  skill_sphere(SKILL_FEIGN_DEATH,          SPHERE_MARTIAL_ARTS, 60);
  skill_sphere(SKILL_FIRST_AID,            SPHERE_MARTIAL_ARTS, 1);
  skill_sphere(SKILL_ZEN_THINKING,         SPHERE_MARTIAL_ARTS, 50);
#endif
  skill_sphere(SKILL_QUIVERING_PALM,       SPHERE_MARTIAL_ARTS, 100);
  skill_sphere(SKILL_KARATE,               SPHERE_MARTIAL_ARTS, 75);

  /* Combat    Skill                       Weapons Prof */
  skill_sphere(SKILL_BLUNT,                SPHERE_WEAPONS,  1);
#if 0
  skill_sphere(SKILL_SHARPEN,              SPHERE_WEAPONS, 40);
  skill_sphere(SKILL_TACTICS,              SPHERE_WEAPONS, 20);
  skill_sphere(SKILL_SMYTHE,               SPHERE_WEAPONS, 60);
#endif
  skill_sphere(SKILL_PIERCE,               SPHERE_WEAPONS,  1);
  skill_sphere(SKILL_RANGE,                SPHERE_WEAPONS, 10);
  skill_sphere(SKILL_SWORDS,               SPHERE_WEAPONS,  1);
  skill_sphere(SKILL_POLEARMS,             SPHERE_WEAPONS,  1);
  skill_sphere(SKILL_THROWING,             SPHERE_WEAPONS,  1);
}


/*
 * This simply calculates the backstab multiplier based on a character's
 * level.  This used to be an array, but was changed to be a function so
 * that it would be easier to add more levels to your MUD.  This doesn't
 * really create a big performance hit because it's not used very often.
 */
int backstab_mult(int level)
{
  if (level <= 0)
    return 1;     /* level 0 */
  else if (level <= 7)
    return 2;     /* level 1 - 7 */
  else if (level <= 13)
    return 3;     /* level 8 - 13 */
  else if (level <= 20)
    return 4;     /* level 14 - 20 */
  else if (level <= 28)
    return 5;     /* level 21 - 28 */
  else if (level <= 36)
    return 6;     /* level 29 - 36 */
  else if (level < LVL_IMMORT)
    return 7;     /* all remaining mortal levels */
  else
    return 20;    /* immortals */
}

/* Guilds start here */
#define MAX_GUILD_LVL 5
#define GUILD_ACC_LVL 4
#define GUILD_DIS_LVL 4
/* Guild globals */
struct guild_type *guild_info = NULL;    /* queue of guilds  */
int    gnum  = 0;                        /* number of guilds */


void free_guild(struct guild_type *g)
{
   int    i;

   if (g != NULL) {
     free(g->name);
     free(g->xlvlname);
     free(g->hlvlname);
     free(g->mlvlname);
     free(g->blvlname);
     free(g->leadersname);
     for (i = 0; i < 20; i++)
       free(g->petitioners[i]);
     free(g);
   }
}

struct guild_type *enqueue_guild(void)
{
   struct guild_type *gptr;

   /* This is the first guild loaded if true */
   if (guild_info == NULL) {
     if ((guild_info = (struct guild_type *) malloc(sizeof(struct guild_type))) == NULL) {
       fprintf(stderr, "SYSERR: Out of memory for guilds!");
       exit(1);
     } else {
       guild_info->next = NULL;
       return (guild_info);
     }
   } else { /* guild_info is not NULL */
     for (gptr = guild_info; gptr->next != NULL; gptr = gptr->next)
       /* Loop does the work */;
     if ((gptr->next = (struct guild_type *) malloc(sizeof(struct guild_type))) == NULL) {
       fprintf(stderr, "SYSERR: Out of memory for guilds!");
       exit(1);
     } else {
       gptr->next->next = NULL;
       return (gptr->next);
     }
   }
   return NULL;
}

void dequeue_guild(int guildnum)
{
   struct guild_type *gptr = NULL, *temp;

   if (guildnum < 0 || guildnum > gnum) {
     log("SYSERR: Attempting to dequeue invalid guild!\r\n");
     exit(1);
   } else {
     if (guild_info->number != guildnum) {
       for (gptr = guild_info; gptr->next && gptr->next->number != guildnum; gptr = gptr->next)
	 ;
       if (gptr->next != NULL && gptr->next->number==guildnum) {
	 temp       = gptr->next;
	 gptr->next = temp->next;
	 free_guild(temp);
       }
     } else {
       /* The first one is the one being removed */
       gptr = guild_info;
       guild_info = guild_info->next;
       free_guild(gptr);
     }
   }
}

/* Loads the guilds from the text file */
void load_guilds(void)
{
  FILE   *fl = NULL;
  int    guildnum = 0, line_num = 0, tmp, i = 0;
  long   tmp2, tmp3;
  char   name[80], flags[80];
  char   *ptr;
  struct guild_type *gptr = NULL;

  if ((fl = fopen(GUILD_FILE, "rt")) == NULL) {
    fprintf(stderr, "SYSERR: Unable to open guild file!");
    exit(0);
  }

  line_num += get_line(fl, buf);
  if (sscanf(buf, "%d", &guildnum) != 1) {
    fprintf(stderr, "Format error in guild, line %d (number of guilds)\n", line_num);
    exit(0);
  }
  /* Setup the global total number of guilds */
  gnum = guildnum;

  /* process each guild in order */
  for (guildnum = 0; guildnum < gnum; guildnum++) {
    /* Get the info for the individual guilds */
    line_num += get_line(fl, buf);
    if (sscanf(buf, "#%d", &tmp) != 1) {
      fprintf(stderr, "Format error in guild (No Unique GID), line %d\n", line_num);
      exit(0);
    }
    /* create some guild shaped memory space */
    if ((gptr = enqueue_guild()) != NULL) {
      gptr->number = tmp;
      /* Now get the name of the guild */
      line_num += get_line(fl, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->name = str_dup(buf);
      /* Now get the leader's title */
      line_num += get_line(fl, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->xlvlname = str_dup(buf);
      /* Now get the high lvl's title */
      line_num += get_line(fl, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->hlvlname = str_dup(buf);
      /* Now get the Med Lvl's title */
      line_num += get_line(fl, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->mlvlname = str_dup(buf);
      /* Now get the newbie's title */
      line_num += get_line(fl, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->blvlname = str_dup(buf);
      /* Now get the look member string */
      line_num += get_line(fl, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
	*ptr = '\0';
      gptr->member_look_str = str_dup(buf);
      /* Now get restricts, entrance room and direction */
      line_num += get_line(fl, buf);
      if (sscanf(buf, "%s %ld %d %ld", flags, &tmp2, &tmp, &tmp3) != 4) {
        fprintf(stderr, "Format error in guild, line %d (flags, room, dir, mob)\n", line_num);
        exit(0);
      }
      gptr->restrictions    = asciiflag_conv(flags);
      gptr->guild_entr_room = tmp2;
      /* This is an SCMD_direction number */
      gptr->direction       = tmp;
      gptr->guardian_mob_vn = tmp3;
      /* now assign the special to the guardian mobile */
      ASSIGNMOB(tmp3, guild_guard);
      /* Skip this line it's just a header */
      line_num += get_line(fl, buf);
      /* Loop to get Members' Names */
      for (;;) {
        line_num += get_line(fl, buf);
        if (*buf == ';')
	  break;
        sscanf(buf, "%s %d", name, &tmp);
        if (tmp == MAX_GUILD_LVL)
 	  gptr->leadersname = str_dup(name);
      }
      /* Okay we have the leader's name ... now for the petitioners */
      for (i = 0; i < 20; i++) {
        line_num += get_line(fl, buf);
        /* We're done when we hit the $ character */
        if (*buf == '$')
	  break;
        else if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
 	  *ptr = '\0';
        gptr->petitioners[i] = str_dup(buf);
      }
    } else break;
    /* process the next guild */
  }
  /* done processing guilds -- close the file */
  fclose(fl);
}


void save_guilds(void)
{
  FILE   *gfile;
  int    guildnum = 0, i;
  char   flags[80];
  struct guild_type *gptr = guild_info;

  void   sprintbits(long bits, char *s);

  if (gptr == NULL) {
    fprintf(stderr, "SYSERR: No guilds to save!!\n");
    return;
  }
  if ((gfile = fopen(GUILD_FILE, "wt")) == NULL) {
    fprintf(stderr, "SYSERR: Cannot save guilds!\n");
    exit(0);
  }
  /* The total number of guilds */
  fprintf(gfile, "%d\n", gnum);
  /* Save each guild */
  while (guildnum < gnum && gptr != NULL) {
    sprintbits(gptr->restrictions, flags);
    if (flags[0] == '\0')
      strcpy(flags, "0");
    fprintf(gfile,  "#%d\n"
		    "%s~\n"
		    "%s~\n"
		    "%s~\n"
		    "%s~\n"
		    "%s~\n"
		    "%s~\n"
		    "%s %ld %d %ld\n"
		    "; Leader\n"
		    "%s %d\n"
		    "; Petitioners (Max of 20)\n",
		    gptr->number,   gptr->name,
		    gptr->xlvlname, gptr->hlvlname,
		    gptr->mlvlname, gptr->blvlname, gptr->member_look_str,
		    flags, gptr->guild_entr_room, gptr->direction,
		    gptr->guardian_mob_vn, gptr->leadersname,
		    MAX_GUILD_LVL);
    for (i = 0; i < 20; i++) {
      if (gptr->petitioners[i] == NULL)
	break;
      else /* Print the name */
	fprintf(gfile, "%s~\n", gptr->petitioners[i]);
    }
    fprintf(gfile, "$\n");
    /* process the next guild */
    gptr = gptr->next;
    guildnum++;
  }
  /* done processing guilds */
  fclose(gfile);
}


/* returns the guild member's guild level */
int guild_level(struct char_data *gm)
{
   if (GET_GUILD_LEV(gm) > MAX_GUILD_LVL || GET_GUILD_LEV(gm) < 0) {
       sprintf(buf, "SYSERR: %s's guild_level out of range!", GET_NAME(gm));
       mudlog(buf, NRM, LVL_GRGOD, TRUE);
       return 0;
   }
   return (GET_GUILD_LEV(gm));
}

/* Petition a guild for membership */
ACMD(do_petition)
{
   int i;
   int found = FALSE;
   struct guild_type *gptr = NULL;

   if (IS_NPC(ch))
     return;

   if (IS_IMMORT(ch)) {
     send_to_char("sorry, but gods cannot join guilds.\r\n",  ch);
     return;
   }
   if (GET_GUILD(ch) != GUILD_NONE) {
     send_to_char("If you'd like to leave your guild speak with your leader.\r\n", ch);
     return;
   }

   if (!*argument) {
     send_to_char("Which guild do you want to petition for membership?\r\n", ch);
     for (gptr = guild_info; gptr; gptr = gptr->next) {
       sprintf(buf, "%s\r\n", gptr->number ? gptr->name : "");
       send_to_char(buf, ch);
     }
     return;
   }
   skip_spaces(&argument);

   for (gptr = guild_info; gptr; gptr = gptr->next) {
     if ((isname(argument, gptr->name)) || is_abbrev(argument, gptr->name)) 
       found = TRUE;
     if (found && (gptr->number != 0))
       break;
   }

   if (!found)
     send_to_char("There is no guild by that name, please type the WHOLE name.\r\n", ch);
   else {
     for (i = 0; i < 20; i++)
       if (gptr->petitioners[i] == NULL)
	 break;
     if ((gptr->petitioners[i] == NULL) && (i < 20)) {
       gptr->petitioners[i] = str_dup(GET_NAME(ch));
       save_guilds();
       sprintf(buf, "Your petition for admittance to %s has been noted.\r\n"
		    "You will receive notification when you have been accepted.\r\n",
		    gptr->name);
       send_to_char(buf, ch);
     } else
       send_to_char("This guild is already being petitioned by 20 other players, try again later.\r\n", ch);
   }
   return;
}

ACMD(do_accept)
{
   int    i, found = 0, player_i = 100;
   struct char_data *vict = NULL, *cbuf = NULL;
   struct char_file_u tmp_store;
   struct guild_type *gptr = guild_info;
   /* Proto so that we can send notify mail to petitioner */
   void store_mail(long to, long from, char *message_pointer);
   extern struct room_data world[];

   /* No funky stuff */
   if (IS_NPC(ch))
     return;

   if (GET_GUILD(ch) == GUILD_NONE || GET_GUILD(ch) == GUILD_UNDEFINED) {
     send_to_char("But you're not a member of any guild!\r\n", ch);
     return;
   }

   if (GET_GUILD_LEV(ch) < GUILD_ACC_LVL) {
     send_to_char("You are not high enough level within the guild to accept new members!\r\n", ch);
     return;
   }

   while (gptr && gptr->number != GET_GUILD(ch))
     gptr = gptr->next;

   if (gptr == NULL)
     return;

   one_argument(argument, arg);

   if (!*arg) {
     strcpy(buf, "Current petitioners are:\r\n");
     for (i = 0; i < 20; i++) {
       if (gptr->petitioners[i] != NULL)
	 sprintf(buf, "%s%s%s", buf, gptr->petitioners[i],
		  ((i+1) % 5 ? "\r\n" : " "));
     }
     strcat(buf,  "Whom do you wish to accept into the guild?\r\n");
     send_to_char(buf, ch);
     return;
   }
   /* Find the person who is being accepted */
   for (i = 0; i < 20; i++) {
     if (gptr->petitioners[i] != NULL) {
       if (!str_cmp(arg, gptr->petitioners[i]) || is_abbrev(arg, gptr->petitioners[i])) {
	 found = 1;
	 break;
       }
     }
   }
   if (!found) {
     send_to_char("There is no one petitioning your guild by that name.\r\n", ch);
     return;
   }
   /* Okay we have the name, now let's set the PC as a member */
   if (!(vict = get_player_vis(ch, arg, 0))) {
     /* we need to load the file up :( */
     CREATE(cbuf, struct char_data, 1);
     clear_char(cbuf);
     if ((player_i = load_char(arg, &tmp_store)) > -1) {
       store_to_char(&tmp_store, cbuf);
       vict = cbuf;
     }
   }
   if (vict != NULL &&
       ((GET_GUILD(vict) == GUILD_NONE) ||
	(GET_GUILD(vict) == GUILD_UNDEFINED))) {
     if (IS_IMMORT(vict)) {
       send_to_char("Gods cannot be accepted into ANY guild.\r\n", ch);
     } else {
     GET_GUILD(vict)     = GET_GUILD(ch);
     GET_GUILD_LEV(vict) = 1;
     /* send some sort of mail to player notifying him/her of acceptance */
     sprintf(buf, "%s,\r\n\r\n"
		  "   Congratulations!  You have been accepted into the ranks of\r\n"
		  "%s.  The leader of our guild is %s, and the entrance to our\r\n"
		  "guild hall is located at %s.  Good luck.\r\n",
		  GET_NAME(vict) ? GET_NAME(vict) : "Hey you!", 
		  gptr->name ? gptr->name : "Some Guild",
		  gptr->leadersname ? gptr->leadersname : "Someone",
                  (real_room(gptr->guild_entr_room) > 0) ?
		  world[real_room(gptr->guild_entr_room)].name : "NOWHERE");
     store_mail(GET_IDNUM(vict), GET_IDNUM(ch), buf);
     /* now that the mail has been sent, let's continue. */
     sprintf(buf, "%s is now a %s in %s.\r\n", GET_NAME(vict), gptr->blvlname, gptr->name);
     send_to_char(buf, ch);
     }
     if (cbuf != NULL) {
       char_to_store(vict, &tmp_store);
       fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
       fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
       free_char(cbuf);
       send_to_char("Saved in file.\r\n", ch);
     } else
       save_char(vict, NOWHERE);
     /* now remove the player from the petition list */
     for (; i < 19; i++) {
       if (gptr->petitioners[i+1])
	 gptr->petitioners[i] = str_dup(gptr->petitioners[i+1]);
       else {
	 free(gptr->petitioners[i]);
	 gptr->petitioners[i] = NULL;
       }
     }
     save_guilds();
   }
}

ACMD(do_reject)
{
   int i, found = 0, player_i = 100;
   struct char_data *vict = NULL, *cbuf = NULL;
   struct char_file_u tmp_store;
   struct guild_type *gptr = guild_info;

   if (IS_NPC(ch))
     return;

   if (GET_GUILD(ch) == GUILD_NONE || GET_GUILD(ch) == GUILD_UNDEFINED) {
     send_to_char("But you're not a member of any guild!\r\n", ch);
     return;
   }

   if (GET_GUILD_LEV(ch) < GUILD_ACC_LVL) {
     send_to_char("You are not high enough level within the guild to reject petitioners!\r\n", ch);
     return;
   }

   while (gptr && gptr->number != GET_GUILD(ch))
     gptr = gptr->next;

   if (gptr == NULL)
     return;
   one_argument(argument, arg);

   if (!*arg) {
     strcpy(buf, "Current petitioners are:\r\n");
     for (i = 0; i < 20; i++) {
	if (gptr->petitioners[i] != NULL)
	  sprintf(buf, "%s%s%s", buf, gptr->petitioners[i],
		  ((i+1) % 5 ? "\r\n" : " "));
     }
     strcat(buf,  "Whom do you wish to reject from the guild?\r\n");
     send_to_char(buf, ch);
     return;
   }
   /* Find the person who is being rejected */
   for (i = 0; i < 20; i++) {
     if (gptr->petitioners[i] != NULL)
       if (isname(arg, gptr->petitioners[i]) || is_abbrev(arg, gptr->petitioners[i])) {
	 found = 1;
	 break;
       }
   }
   if (!found) {
     send_to_char("There is no one petitioning your guild by that name.\r\n", ch);
     return;
   }
   /* Okay we have the name, now let's set the PC as a member */
   if (!(vict = get_player_vis(ch, arg, 0))) {
     /* we need to load the file up :( */
     CREATE(cbuf, struct char_data, 1);
     clear_char(cbuf);
     if ((player_i = load_char(arg, &tmp_store)) > -1) {
       store_to_char(&tmp_store, cbuf);
       vict = cbuf;
     }
   }
   if (vict != NULL) {
     GET_GUILD(vict)     = GUILD_NONE;
     GET_GUILD_LEV(vict) = 0;
     /* send some sort of mail to player notifying him/her of rejection */

     sprintf(buf, "%s has been rejected from %s.\r\n", GET_NAME(vict), gptr->name);
     send_to_char(buf, ch);
     if (cbuf != NULL) {
       char_to_store(vict, &tmp_store);
       fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
       fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
       free_char(cbuf);
     } else
       save_char(vict, NOWHERE);
     /* now remove the player from the petition list */
     for (; i < 19; i++) {
       if (gptr->petitioners[i+1])
	 gptr->petitioners[i] = str_dup(gptr->petitioners[i+1]);
       else {
	 free(gptr->petitioners[i]);
	 gptr->petitioners[i] = NULL;
       }
     }
     save_guilds();
   }
}


ACMD(do_dismiss)
{
   int player_i = 0;
   struct char_data *vict = NULL, *cbuf = NULL;
   struct char_file_u tmp_store;
   struct guild_type *gptr = guild_info;

   if (IS_NPC(ch))
     return;

   if (GET_GUILD(ch) == GUILD_NONE || GET_GUILD(ch) == GUILD_UNDEFINED) {
     send_to_char("But you're not a member of any guild!\r\n", ch);
     return;
   }

   if (GET_GUILD_LEV(ch) < GUILD_DIS_LVL) {
     send_to_char("You are not high enough level within the guild to dismiss members!\r\n", ch);
     return;
   }

   while (gptr && gptr->number != GET_GUILD(ch))
     gptr = gptr->next;

   if (gptr == NULL)
     return;

   one_argument(argument, arg);

   if (!*arg) {
     send_to_char("Whom do you wish to dismiss?\r\n", ch);
     return;
   }

   /* Okay we have the name, now let's try to dismiss this troublemaker */
   if (!(vict = get_player_vis(ch, arg, 0))) {
     /* we need to load the file up :( */
     CREATE(cbuf, struct char_data, 1);
     clear_char(cbuf);
     if (load_char(arg, &tmp_store) > -1) {
       store_to_char(&tmp_store, cbuf);
       vict = cbuf;
     }
   }
   if (vict != NULL && GET_GUILD(vict) == GET_GUILD(ch)) {
     if (GET_GUILD_LEV(vict) >= GET_GUILD_LEV(ch)) {
       send_to_char("You may not dismiss those of equal or greater guild status than yourself!\r\n", ch);
       return;
     }
     GET_GUILD(vict)     = GUILD_NONE;
     GET_GUILD_LEV(vict) = 0;
     /* send some sort of mail to player notifying him/her of rejection */

     sprintf(buf, "%s is no longer a member of %s.\r\n", GET_NAME(vict), gptr->name);
     send_to_char(buf, ch);
     send_to_char("Be sure to notify the player of this!\r\n", ch);
     if (cbuf != NULL) {
       char_to_store(vict, &tmp_store);
       fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
       fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
       free_char(cbuf);
     } else
       save_char(vict, NOWHERE);
   }
}


ACMD(do_promote)
{
   struct char_data *vict = NULL;
   struct guild_type *gptr = NULL;

   if (IS_NPC(ch))
     return;

   if (GET_GUILD(ch) == GUILD_NONE || GET_GUILD(ch) == GUILD_UNDEFINED) {
     send_to_char("But you're not a member of any guild!\r\n", ch);
     return;
   }

   if (GET_GUILD_LEV(ch) < MAX_GUILD_LVL) {
     send_to_char("You are not high enough level within the guild to promote members!\r\n", ch);
     return;
   }

   for (gptr = guild_info; gptr && gptr->number != GET_GUILD(ch); gptr = gptr->next)
     /* the loop should do all the work */;

   if (gptr != NULL)
     return;

   one_argument(argument, arg);

   if (!*arg) {
     send_to_char("Whom do you wish to promote?\r\n", ch);
     return;
   }
   /* Okay we have the name, now let's try to advance this member */
   if (!(vict = get_player_vis(ch, arg, 0))) {
     send_to_char("That person doesn't seem to be around at the moment.\r\n", ch);
     return;
   }
   if (GET_GUILD(vict) == GET_GUILD(ch)) {
     if (GET_GUILD_LEV(vict) == (GET_GUILD_LEV(ch) - 1)) {
       sprintf(buf, "There can be only one %s!\r\n"
		    "Tell a God that you wish to resign, if you want %s to lead %s\r\n",
	  gptr->xlvlname, GET_NAME(vict), gptr->name);
       send_to_char(buf, ch);
       return;
     }
     GET_GUILD_LEV(vict)++;       /* Advance PC's Guild Lev by One */
     /* send some sort of mail to player notifying him/her of promotion */
     sprintf(buf, "%s is now a %s.\r\n", GET_NAME(vict),
	     (GET_GUILD_LEV(vict) == 5 ? gptr->xlvlname :
	      (GET_GUILD_LEV(vict) == 4 ? gptr->hlvlname :
	       (GET_GUILD_LEV(vict) == 1 ? gptr->blvlname :
		(GET_GUILD_LEV(vict) == 0 ? "ERROR!" : gptr->mlvlname)))));
     send_to_char(buf, ch);
     save_char(vict, NOWHERE);
   }
}

ACMD(do_demote)
{
   struct char_data *vict = NULL;
   struct guild_type *gptr = NULL;

   if (IS_NPC(ch))
     return;

   if (GET_GUILD(ch) == GUILD_NONE || GET_GUILD(ch) == GUILD_UNDEFINED) {
     send_to_char("But you're not a member of any guild!\r\n", ch);
     return;
   }

   if (GET_GUILD_LEV(ch) < MAX_GUILD_LVL) {
     send_to_char("You are not high enough level within the guild to demote members!\r\n", ch);
     return;
   }

   for (gptr = guild_info; gptr && gptr->number != GET_GUILD(ch); gptr = gptr->next)
     /* the loop should do all the work */;

   if (gptr != NULL)
     return;

   one_argument(argument, arg);

   if (!*arg) {
     send_to_char("Whom do you wish to demote?\r\n", ch);
     return;
   }

   /* Okay we have the name, now let's try to demote this member */
   if (!(vict = get_player_vis(ch, arg, 0))) {
     send_to_char("That person doesn't seem to be around at the moment.\r\n", ch);
     return;
   }
   if (GET_GUILD(vict) == GET_GUILD(ch)) {
     if (GET_GUILD_LEV(vict) == 1) {
       sprintf(buf, "%s is already a %s, use dismiss instead!\r\n",
	       GET_NAME(vict), gptr->blvlname);
       send_to_char(buf, ch);
       return;
     }
     GET_GUILD_LEV(vict)--;       /* Lower PC's Guild Lev by One */
     /* send some sort of mail to player notifying him/her of promotion */

     sprintf(buf, "%s is now a %s.\r\n", GET_NAME(vict),
	     (GET_GUILD_LEV(vict) == 5 ? gptr->xlvlname :
	      (GET_GUILD_LEV(vict) == 4 ? gptr->hlvlname :
	       (GET_GUILD_LEV(vict) == 1 ? gptr->blvlname :
		(GET_GUILD_LEV(vict) == 0 ? "ERROR!" : gptr->mlvlname)))));
     send_to_char(buf, ch);
     save_char(vict, NOWHERE);
   }
}


ACMD(do_guild)
{
   /* !*arg == show members | *arg == say to members */
   int    j;
   struct descriptor_data  *i;
   struct char_file_u plr_rec;
   struct guild_type *gptr = NULL;

   if (IS_NPC(ch))
     return;

   if (GET_GUILD(ch) == GUILD_NONE || GET_GUILD(ch) == GUILD_UNDEFINED) {
     send_to_char("You must be a member of a guild to do this.\n\r", ch);
     return;
   }

   for (gptr = guild_info; gptr && gptr->number != GET_GUILD(ch); gptr = gptr->next)
     /* The for loop should do all the work */;

   if (gptr == NULL) {
     send_to_char("It seems as though you're guild is invalid!\r\n", ch);
     return;
   }

   if (!(*argument)) {
     send_to_char(CCBMG(ch, C_NRM), ch);
     send_to_char("___________________________________________",ch);
     send_to_char("\r\n\r\n",ch);
     sprintf(buf, "Other members of %s", gptr->name);
     sprintf(buf, "%s...\r\n___________________________________________%s\r\n\r\n",
	    buf, CCNRM(ch, C_NRM));
     send_to_char(buf, ch);
     /* Go through the entire list of players color coding those who are on */
     for (j = 0; j < top_of_p_table; j++) {
	if (load_char(player_table[j].name, &plr_rec) > -1)
	  if (plr_rec.assocs[1] == GET_GUILD(ch)) {
	    sprintf(buf, "%s the %s\r\n", plr_rec.name,
	     (plr_rec.assocs[2] == 5 ? gptr->xlvlname :
	      (plr_rec.assocs[2] == 4 ? gptr->hlvlname :
	       (plr_rec.assocs[2] == 1 ? gptr->blvlname :
		(plr_rec.assocs[2] == 0 ? "!ERROR!" : gptr->mlvlname)))));
	    send_to_char(buf, ch);
	  }
     }
   } else {  /* We have an argument */
     if (GET_POS(ch) <= POS_RESTING) {
       send_to_char("You are in no position to talk with the other members of your guild.\r\n", ch);
       return;
     }
     if (IS_AFFECTED2(ch, AFF_SILENCE) && (*argument)) {
       send_to_char("Your throat is too tight to get the words out.\r\n", ch);
       return;
     }
     skip_spaces(&argument);
     /* The ch sees what he/she said */
     sprintf(buf, "\r\n%sYou guild tell> %s%s\r\n", CCBCN(ch, C_SPR), argument, CCNRM(ch, C_NRM));
     send_to_char(buf, ch);

     for (i = descriptor_list; i; i = i->next)
	if (i->character != ch && !i->connected &&
	    (GET_GUILD(ch) == (i->character->player.assocs[1])) &&
	    GET_POS(i->character) >= POS_RESTING) {
	  sprintf(buf2, "\r\n%s%s guild tells> %s%s\r\n",
		    CCBCN(i->character, C_SPR), GET_NAME(ch), argument,
		    CCNRM(i->character, C_SPR));
	  send_to_char(buf2, i->character);
     }
   }
   return;
}


/* Religions start here */
const char *pc_religion_types[] = {
  "None",                            /*  0 */

  "Kelloran",       /* Light and Healing -- (male)   1 */
  "Luminast",
  "Herille",
  "Bronwyn",        /* Light and Healing -- (female) 4 */
  "Falentai",

  "Aduran",         /* Nature/Neutral -- (male)      6 */
  "Corax",
  "Wendigo",
  "Mesari",         /* Nature/Neutral -- (female)    9 */
  "Sidthora",

  "Solinara",       /* Lawful/Knowledge -- (female)  11 */
  "Morinali",
  "Ischarid",
  "Korr",           /* Lawful/Knowledge  -- (male)   14 */
  "Hindachon",

  "Maletis",        /* Darkness/Disease/Chaos -- (male) 16 */
  "Syesin",
  "Cybothryx",
  "Gecarna",        /* Darkness/Disease/Chaos -- (female) 19 */
  "Tryixa",

  "Vasari",         /* War/Power/Honor/Greed/Evil -- (male) 21 */
  "Coudron",
  "Molerian",
  "N'kerian",
  "Mishra",         /* War/Power/Honor/Greed/Evil -- (female) 25 */
  "\n"
};


const char *relg_name(struct char_data  *ch)
{
   switch (GET_REL(ch)) {
     case REL_GOOD:
       if (IS_DWARVEN(ch))
	 return (pc_religion_types[2]);
       else if (IS_ELVEN(ch))
	 return (pc_religion_types[5]);
       else if (IS_MINOTAUR(ch))
	 return (pc_religion_types[3]);
       else if (IS_SESSANATHI(ch))
	 return (pc_religion_types[4]);
       else
	 return (pc_religion_types[1]);
       break;
     case REL_NEUT:
       if (IS_DWARVEN(ch))
	 return (pc_religion_types[7]);
       else if (IS_ELVEN(ch))
	 return (pc_religion_types[10]);
       else if (IS_MINOTAUR(ch))
	 return (pc_religion_types[8]);
       else if (IS_SESSANATHI(ch))
	 return (pc_religion_types[9]);
       else
	 return (pc_religion_types[6]);
       break;
     case REL_LAWFL:
       if (IS_DWARVEN(ch))
	 return (pc_religion_types[12]);
       else if (IS_ELVEN(ch))
	 return (pc_religion_types[15]);
       else if (IS_MINOTAUR(ch))
	 return (pc_religion_types[13]);
       else if (IS_SESSANATHI(ch))
	 return (pc_religion_types[14]);
       else
	 return (pc_religion_types[11]);
       break;
     case REL_CHAOS:
       if (IS_DWARVEN(ch))
	 return (pc_religion_types[17]);
       else if (IS_ELVEN(ch))
	 return (pc_religion_types[20]);
       else if (IS_MINOTAUR(ch))
	 return (pc_religion_types[18]);
       else if (IS_SESSANATHI(ch))
	 return (pc_religion_types[19]);
       else
	 return (pc_religion_types[16]);
       break;
     case REL_EVIL:
       if (IS_DWARVEN(ch))
	 return (pc_religion_types[22]);
       else if (IS_ELVEN(ch))
	 return (pc_religion_types[25]);
       else if (IS_MINOTAUR(ch))
	 return (pc_religion_types[23]);
       else if (IS_SESSANATHI(ch))
	 return (pc_religion_types[24]);
       else
	 return (pc_religion_types[21]);
       break;
     case REL_NONE:
     default:
       return (pc_religion_types[0]);
       break;
   }
}

int religion_info[][3] = {
 /* {religion#,      temple foyer vnum,   SCMD_direction}, */
 {-1, -1, -1}
};

char *human_rels =

  "Kelloran   - God of Light and Healing                   (Good)\r\n"
  "Aduran     - God of Nature and Balance                  (Neut)\r\n"
  "Solinara   - Goddess of Law and Knowledge               (Any) \r\n"
  "Maletis    - God of War, Disease and Chaos              (Any) \r\n"
  "Vasari     - God of Power, Honor, Greed and Darkness    (Evil)\r\n"
  "\r\n";

char *dwarven_rels =

  "Luminast   - God of Light and Healing                   (Good)\r\n"
  "Corax      - God of Nature and Balance                  (Neut)\r\n"
  "Morinali   - Goddess of Law and Knowledge               (Any) \r\n"
  "Syesin     - God of War, Disease and Chaos              (Any) \r\n"
  "Coudron    - God of Power, Honor, Greed and Darkness    (Evil)\r\n"
  "\r\n";

char *mino_rels =

  "Herille    - God of Light and Healing                   (Good)\r\n"
  "Wendigo    - God of Nature and Balance                  (Neut)\r\n"
  "Ischarid   - Goddess of Law and Knowledge               (Any) \r\n"
  "Cybothryx  - God of War, Disease and Chaos              (Any) \r\n"
  "Molerian   - God of Power, Honor, Greed and Darkness    (Evil)\r\n"
  "\r\n";

char *sess_rels =

  "Bronwyn    - Goddess of Light and Healing               (Good)\r\n"
  "Mesari     - Goddess of Nature and Balance              (Neut)\r\n"
  "Korr       - God of Law and Knowledge                   (Any) \r\n"
  "Gecarna    - Goddess of War, Disease and Chaos          (Any) \r\n"
  "N'kerian   - God of Power, Honor, Greed and Darkness    (Evil)\r\n"
  "\r\n";

char *elven_rels =

  "Falentai   - Goddess of Light and Healing               (Good)\r\n"
  "Sidthora   - Goddess of Nature and Balance              (Neut)\r\n"
  "Hindachon  - God of Law and Knowledge                   (Any) \r\n"
  "Tryixa     - Goddess of War, Disease and Chaos          (Any) \r\n"
  "Mishra     - Goddess of Darkness, Power, Honor, & Greed (Evil)\r\n"
  "\r\n";

/* general function for players to toggle their religion */
ACMD(do_tog_rel)
{
   struct affected_type af;
   int    i = 0;
   char   arg1[80];

   struct {
     int religion;
     char *godname;
     int permalign;
     char *clue;
   } affirm_array[] = {
     /* Good */
     {REL_GOOD, "Kelloran", ALIGN_GOOD, ""},
     {REL_GOOD, "Luminast", ALIGN_GOOD, ""},
     {REL_GOOD, "Herille",  ALIGN_GOOD, ""},
     {REL_GOOD, "Bronwyn",  ALIGN_GOOD, ""},
     {REL_GOOD, "Falentai", ALIGN_GOOD, ""},
     /* Neutral */
     {REL_NEUT, "Aduran",   ALIGN_NEUT, ""},
     {REL_NEUT, "Corax",    ALIGN_NEUT, ""},
     {REL_NEUT, "Wendigo",  ALIGN_NEUT, ""},
     {REL_NEUT, "Mesari",   ALIGN_NEUT, ""},
     {REL_NEUT, "Sidthora", ALIGN_NEUT, ""},
     /* Evil */
     {REL_EVIL, "Vasari",   ALIGN_EVIL, ""},
     {REL_EVIL, "Coudron",  ALIGN_EVIL, ""},
     {REL_EVIL, "Molerian", ALIGN_EVIL, ""},
     {REL_EVIL, "N'kerian", ALIGN_EVIL, ""},
     {REL_EVIL, "Mishra",   ALIGN_EVIL, ""},
     /* Lawful */
     {REL_LAWFL, "Solinara", -1, ""},
     {REL_LAWFL, "Morinali", -1, ""},
     {REL_LAWFL, "Ischarid", -1, ""},
     {REL_LAWFL, "Korr",     -1, ""},
     {REL_LAWFL, "Hindachon",-1, ""},
     /* Chaotic */
     {REL_CHAOS, "Maletis",  -1, ""},
     {REL_CHAOS, "Syesin",   -1, ""},
     {REL_CHAOS, "Cybothryx",-1, ""},
     {REL_CHAOS, "Gecarna",  -1, ""},
     {REL_CHAOS, "Tryixa",   -1, ""},
/*
     {REL_GLASGIAN, "glasgian", ALIGN_GOOD, "Look for His church in the township of Elrhinn.\r\n"},
*/
     {-1, NULL, -1, NULL}
   };

   one_argument(argument, arg1);

   if (!*arg1) {
     send_to_char("Available religions are:                             (Align Req'd)\r\n", ch);
     if (IS_DWARVEN(ch))
       send_to_char(dwarven_rels, ch);
     else if (IS_ELVEN(ch))
       send_to_char(elven_rels, ch);
     else if (IS_MINOTAUR(ch))
       send_to_char(mino_rels, ch);
     else if (IS_SESSANATHI(ch))
       send_to_char(sess_rels, ch);
     else
       send_to_char(human_rels, ch);
     return;
   }
   if ((subcmd == SCMD_AFFIRM) && GET_REL(ch) != REL_NONE) {
     send_to_char("Nice try, but you have to renounce your current religion to affirm another one!\r\n",ch);
     return;
   } else if ((subcmd == SCMD_RENOUNCE) && GET_REL(ch) == REL_NONE) {
     send_to_char("Why would you want to renounce a god that you don't even believe in?\r\n",ch);
     return;
   } else if ((subcmd == SCMD_AFFIRM) && IS_AFFECTED2(ch, AFF_RELIGED)) {
     send_to_char("You cannot affirm a religion at this time.\r\n",ch);
     return;
   }
   LowerString(arg1);
   for (i = 0; affirm_array[i].religion != -1; ++i)
      if (!strcmp(arg1, affirm_array[i].godname) || is_abbrev(arg1, affirm_array[i].godname))
	break;
   /* check to make sure we found a god */
   if (affirm_array[i].godname == NULL) {
     send_to_char("What god was that?\r\n[Use 'affirm' or 'renounce' with no arguments to see available religions.]\n\r", ch);
     return;
   }
   /* okay we have a god's name */
   if (subcmd == SCMD_RENOUNCE) {
     if (GET_REL(ch) != affirm_array[i].religion)
       send_to_char("Why do you want to renouce a god you don't even believe in?\r\n", ch);
     else {
       sprintf(buf, "You renounce %s and all the teachings of that religion!\r\n",
	      affirm_array[i].godname);
       send_to_char(buf, ch);
       GET_REL(ch)  = REL_NONE;
       af.location  = APPLY_WIS;
       af.modifier  = -4;                   /* Make wisdom worse */
       af.duration  = 48;                   /* ... for two days  */
       af.bitvector = AFF_BLIND;            /* and blind them    */
       affect_to_char(ch, &af, TRUE);
       af.duration  = 48;
       af.bitvector = AFF_RELIGED;
       affect_to_char(ch, &af, TRUE);
     }
   } else if (subcmd == SCMD_AFFIRM) {
     if (affirm_array[i].permalign != -1) {
       if (GET_PERMALIGN(ch) != affirm_array[i].permalign) {
	 sprintf(buf, "Your soul is not in accordance with the teachings of %s's religion.\r\n",
	       affirm_array[i].godname);
	 send_to_char(buf, ch);
	 return;
       }
     }
     GET_REL(ch) = affirm_array[i].religion;
     sprintf(buf, "You are now a member of %s's religion.\r\n",
		affirm_array[i].godname);
     send_to_char(buf, ch);
     if (affirm_array[i].clue != NULL)
       send_to_char(affirm_array[i].clue, ch);
   }
   return;
}


ACMD(do_religion)
{
   /* !*arg == show members | *arg == say to members */
   int    j;
   struct descriptor_data  *i;
   struct char_file_u plr_rec;
   extern struct room_data world[];

   if (IS_NPC(ch))
     return;

   if (!*argument) {
     if (GET_REL(ch) == REL_NONE)
       send_to_char("You must follow a god to do this.\n\r", ch);
     else {
       send_to_char(CCYEL(ch, C_NRM), ch);
       send_to_char("_______________________________________\n\r\n\r",ch);
       sprintf(buf, "Other members of %s", relg_name(ch));
       send_to_char(buf, ch);
       send_to_char("'s religion...\r\n_______________________________________\n\r\n\r",ch);
       send_to_char(CCNRM(ch, C_NRM), ch);
       for (j = 0; j < top_of_p_table; j++) {
	 if (load_char(player_table[j].name, &plr_rec) > -1)
	   if (plr_rec.assocs[0] == GET_REL(ch)) {
	     sprintf(buf, "%s %s\r\n", plr_rec.name, plr_rec.title);
             send_to_char(buf, ch);
           }
       }
     }
   } else {
     if (GET_POS(ch) <= POS_RESTING) {
       send_to_char("You are in no position to talk with the other members of your religion.\r\n", ch);
       return;
     }
     if (IS_AFFECTED2(ch, AFF_SILENCE) && (*argument)) {
       send_to_char("Your throat is too tight to get the words out.\r\n", ch);
       return;
     }
     skip_spaces(&argument);
     /* now send it to everyone */
     for (i = descriptor_list; i; i = i->next)
	if (i->character != ch && !i->connected &&
	    (GET_REL(ch) == (i->character->player.assocs[0])) &&
	    (GET_POS(i->character) >= POS_RESTING)) {
	  sprintf(buf2, "\r\n%s%s religion tells> %s%s\r\n",
		    CCBCN(i->character, C_SPR), 
		    (CAN_SEE(i->character, ch) ? GET_NAME(ch) : "Someone"),
		    argument, CCNRM(i->character, C_SPR));
	  send_to_char(buf2, i->character);
        }
     /* The ch sees what he/she said */
     sprintf(buf, "\r\n%sYou religion tell> %s%s\r\n", CCBCN(ch, C_SPR), argument, CCNRM(ch, C_NRM));
     send_to_char(buf, ch);
   }
}

