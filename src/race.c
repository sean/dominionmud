/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  RACE.C                                       Based on CircleMUD *
*  Usage: Source file for race-specific code (exp tables as well)         *
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

struct race_data * races;
int NUM_RACES = 0;

void parse_races( void )
{
  int index = 0, c, ival;
  char cval, sel;
  FILE * fl = 0;

  if ( !(fl = fopen( RACE_FILE, "r" ) ) ) {
    plog( "parse_races: unable to open races file (%s).\n",
         RACE_FILE );
    exit( 1 );
  }

  /* reset the number of races */
  NUM_RACES = 0;
  // first read through the file and count the number of races.
  while ( !feof( fl ) ) {
    c = getc( fl );

    if ( c == '#' ) {
      c = getc( fl );
      if ( isalpha( c ) )
        NUM_RACES++;
    }
    fread_to_eol( fl );
  }

  plog( "Loading races (%d) ...", NUM_RACES );
  
  CREATE( races, struct race_data, NUM_RACES );
  // reset the file pointer
  fseek( fl, 0L, SEEK_SET );
  while ( !feof( fl ) ) {
    // parse one race
    c = getc( fl );

    if ( c == '#' ) {
      c = getc( fl );
      if ( !isspace( c ) && isalpha( c ) ) {
        ungetc( c, fl );
        READ_CHECK(fscanf( fl, "%s\n", buf2 ), 1, RACE_FILE);
        races[ index ].name   = strdup( buf2 );
        races[ index ].abbrev = fread_string( fl, buf2 );
        READ_CHECK(fscanf( fl, "%c %c %d\n",
                &sel, &cval, &ival ), 3, RACE_FILE);
        races[ index ].selector = LOWER( sel );
        assert( races[ index ].selector >= 'a' &&
                races[ index ].selector <= 'z' );
        races[ index ].isHumanoid = ( LOWER( cval ) == 't' ? TRUE : FALSE );
        races[ index ].points.rsize = ival;
        fread_to_eol( fl );

        races[ index ].points.bitvec = 1<<index;

        READ_CHECK(fscanf( fl, "%d %ld %c",
                &(races[ index ].points.regen),
                &(races[ index ].points.start),
                &cval ), 3, RACE_FILE);
        races[ index ].isNPC = ( LOWER( cval ) == 't' ? TRUE : FALSE );
        
        /* incase there are any comments, always read to the end of line */
        fread_to_eol( fl );

        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_str),
                &(races[ index ].points.abils.max_str) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_int),
                &(races[ index ].points.abils.max_int) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_wis),
                &(races[ index ].points.abils.max_wis) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_dex),
                &(races[ index ].points.abils.max_dex) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_con),
                &(races[ index ].points.abils.max_con) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_cha),
                &(races[ index ].points.abils.max_cha) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_wil),
                &(races[ index ].points.abils.max_wil) ), 2, RACE_FILE);
        fread_to_eol( fl );

        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_hi),
                &(races[ index ].points.points.max_hi) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_da),
                &(races[ index ].points.points.max_da) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_hp),
                &(races[ index ].points.points.max_hp) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_ma),
                &(races[ index ].points.points.max_ma) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_mo),
                &(races[ index ].points.points.max_mo) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_lhp),
                &(races[ index ].points.points.max_lhp) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_lma),
                &(races[ index ].points.points.max_lma) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_lmo),
                &(races[ index ].points.points.max_lmo) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d",
                &(races[ index ].points.points.bage),
                &(races[ index ].points.points.max_bage) ), 2, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %c",
                &(races[ index ].points.points.old_age),
                &cval ), 2, RACE_FILE);
        races[ index ].canSpeak = ( LOWER( cval ) == 't' ? TRUE : FALSE );
        fread_to_eol( fl );

        READ_CHECK(fscanf( fl, "%d %d %d",
                &(races[ index ].points.size[0].base_height),
                &(races[ index ].points.size[0].mod_num_height),
                &(races[ index ].points.size[0].mod_size_height) ), 3, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d %d",
                &(races[ index ].points.size[0].base_weight),
                &(races[ index ].points.size[0].mod_num_weight),
                &(races[ index ].points.size[0].mod_size_weight) ), 3, RACE_FILE);
        fread_to_eol( fl );

        READ_CHECK(fscanf( fl, "%d %d %d",
                &(races[ index ].points.size[1].base_height),
                &(races[ index ].points.size[1].mod_num_height),
                &(races[ index ].points.size[1].mod_size_height) ), 3, RACE_FILE);
        fread_to_eol( fl );
        READ_CHECK(fscanf( fl, "%d %d %d",
                &(races[ index ].points.size[1].base_weight),
                &(races[ index ].points.size[1].mod_num_weight),
                &(races[ index ].points.size[1].mod_size_weight) ), 3, RACE_FILE);
        
        index++;
      } else { /* comment */
        fread_to_eol( fl );
      }
    } else { /* blank line? */
      fread_to_eol( fl );
    }
  }

  if ( index < NUM_RACES ) {
    plog( "parse_races: failed to parse all races! (%d/%d)\n",
         index, NUM_RACES );
    exit( 1 );
  }
}

/**
 * @param c the selector for a particular race
 * @return a string containing the name of the race
 */
char * rsel_to_name( char c )
{
  int idx = 0;
  char * rval = 0;

  while ( idx <= NUM_RACES ) {
    if ( races[ idx ].selector == LOWER(c) ) {
      rval = races[ idx ].name;
      break;
    }
  }

  return rval;
}

/**
 * @param color if true use color, if false don't
 * @return a string containing the race menu
 */
const char * build_race_menu( bool color )
{
  char * rval = buf2;
  int idx = 0;

  strcat( rval, "\r\nSelect a race:\r\n" );
  while ( idx < NUM_RACES ) {
    if ( races[ idx ].isNPC != TRUE ) {
      sprintf( rval + strlen(rval), "  [%s%c%s] %s\r\n", ( color == TRUE ? KBBL : "" ),
               races[ idx ].selector,
               ( color == TRUE ? KNRM : "" ),
               races[ idx ].name );
    }
    idx++;
  }
  strcat( rval, "\r\n\r\nYour choice? " );

  return rval;
}

/**
 * @param a character which maps to a particular race.
 * @return a unique integer representing the race specified.
 */
int parse_race( char arg )
{
  int i;
  arg = LOWER(arg);

  for ( i = 0; i <= NUM_RACES; i++ ) {
    if ( races[ i ].selector == LOWER(arg) )
      return i;
  }

  return UNDEFINED_RACE;
}

/**
 */
long find_race_bitvector( char arg )
{
  int i;
  arg = LOWER(arg);

  for ( i = 0; i <= NUM_RACES; i++ ) {
    if ( races[ i ].selector == arg )
      return races[ i ].points.bitvec;
  }

  return 0;
}

/**
 */
room_num get_hometown(struct char_data *ch)
{
  int race = GET_RACE( ch );
  assert( race >= 0 && race <= NUM_RACES );
  return races[ race ].points.start;
}

/**
 */
void set_base_attribs(struct char_data *ch)
{
  int i = GET_RACE( ch );
  
  ch->real_abils.str =
    number( races[ i ].points.abils.min_str,
            races[ i ].points.abils.max_str );
  ch->real_abils.intel =
    number( races[ i ].points.abils.min_int,
            races[ i ].points.abils.max_int );
  ch->real_abils.wis =
    number( races[ i ].points.abils.min_wis,
            races[ i ].points.abils.max_wis );
  ch->real_abils.dex =
    number( races[ i ].points.abils.min_dex,
            races[ i ].points.abils.max_dex );
  ch->real_abils.con =
    number( races[ i ].points.abils.min_con,
            races[ i ].points.abils.max_con );
  ch->real_abils.cha =
    number( races[ i ].points.abils.min_cha,
            races[ i ].points.abils.max_cha );
  ch->real_abils.will =
    number( races[ i ].points.abils.min_wil,
            races[ i ].points.abils.max_wil );
  /* now assign them to the character */
  ch->aff_abils = ch->real_abils;
}

void print_plr_attribs(struct char_data *ch, int points)
{
    int  i = 0;
    char tempb[80];

#if SHOW_ATTRIBS
    sprintf(buf, "    +-[S]tr +-[I]nt +-[W]is +-[D]ex +-[C]on +-C[H]a +-Wi[L]l [Q]uit [?]Help\r\n");
    sprintf(buf + strlen(buf), "       %-2d    %-2d    %-2d    %-2d    %-2d    %-2d   %-2d\r\n",
	    ch->real_abils.str, ch->real_abils.intel, ch->real_abils.wis,
	    ch->real_abils.dex, ch->real_abils.con, ch->real_abils.cha,
	    ch->real_abils.will);
    sprintf(buf + strlen(buf), "You have %d points remaining: ", points);
#else
    sprintf(buf, "%sYour attributes are:\r\n", GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.str / 2); i++)
	strcat(tempb, "*");
    sprintf(buf + strlen(buf), "\r\n+-[%sS%s]tr: %s%s%s\r\n",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.intel / 2); i++)
	strcat(tempb, "*");
    sprintf(buf + strlen(buf), "+-[%sI%s]nt: %s%s%s\r\n",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.wis / 2); i++)
	strcat(tempb, "*");
    sprintf(buf + strlen(buf), "+-[%sW%s]is: %s%s%s\r\n",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.dex / 2); i++)
	strcat(tempb, "*");
    sprintf(buf + strlen(buf), "+-[%sD%s]ex: %s%s%s\r\n",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.con / 2); i++)
	strcat(tempb, "*");
    sprintf(buf + strlen(buf), "+-[%sC%s]on: %s%s%s\r\n",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.cha / 2); i++)
	strcat(tempb, "*");
    sprintf(buf + strlen(buf), "+-C[%sh%s]a: %s%s%s\r\n",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    tempb[0] = '\0';
    for (i = 0; i < (ch->real_abils.will / 2); i++)
	strcat(tempb, "*");
    sprintf(buf + strlen(buf), "+-Wi[%sl%s]: %s%s%s\r\n",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KBBL : "", tempb,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    sprintf(buf + strlen(buf), "  [%sR%s]eroll base stats (costs 1 point).\r\n",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", GET_ANSI(ch->desc) == TRUE ? KBCN : "");
    sprintf(buf + strlen(buf), "\r\nRemaining Points: %s%-2d%s\r\n",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "", points,
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "");
#endif
    sprintf(buf + strlen(buf), "\r\n([%sQ%s]uit, [%s?%s] Help) Your choice?%s ",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "",
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KYEL : "",
	    GET_ANSI(ch->desc) == TRUE ? KBCN : "",
	    GET_ANSI(ch->desc) == TRUE ? KNRM : "");
    send_to_char(buf, ch);
}

int add_attrib(struct char_data *ch, int points, int attrib)
{
  int i = GET_RACE( ch );
  
  switch (attrib) {
    case 0:  /* str */
      if ( ch->real_abils.str < races[ i ].points.abils.max_str ) {
        ch->real_abils.str += 1;
        points--;
      } else
        send_to_char("Your strength is at its maximum.\r\n", ch);
      break;
    case 1:  /* int */
      if ( ch->real_abils.intel < races[ i ].points.abils.max_int ) {
        ch->real_abils.intel += 1;
        points--;
      } else
        send_to_char("Your intelligence is at its maximum.\r\n", ch);
      break;
    case 2:  /* wis */
      if ( ch->real_abils.wis < races[ i ].points.abils.max_wis ) {
        ch->real_abils.wis += 1;
        points--;
      } else
        send_to_char("Your wisdom is at its maximum.\r\n", ch);
      break;
    case 3:  /* dex */
      if ( ch->real_abils.dex < races[ i ].points.abils.max_dex ) {
        ch->real_abils.dex += 1;
        points--;
      } else
        send_to_char("Your dexterity is at its maximum.\r\n", ch);
      break;
    case 4:  /* con */
      if ( ch->real_abils.con < races[ i ].points.abils.max_con ) {
        ch->real_abils.con += 1;
        points--;
      } else
        send_to_char("Your constitution is at its maximum.\r\n", ch);
      break;
    case 5:  /* cha */
      if ( ch->real_abils.cha < races[ i ].points.abils.max_cha) {
        ch->real_abils.cha += 1;
        points--;
      } else
        send_to_char("Your charisma is at its maximum.\r\n", ch);
      break;
    case 6:  /* will */
      if ( ch->real_abils.will < races[ i ].points.abils.max_wil ) {
        ch->real_abils.will += 1;
        points--;
      } else
        send_to_char("Your will power is at its maximum.\r\n", ch);
      break;
    default:
      send_to_char( "Hunh?  Which attribute was that?\r\n", ch);
      break;
  }
  return points;
}

/**
 */
int sub_attrib(struct char_data *ch, int points, int attrib)
{
  int i = GET_RACE( ch );
  
  switch (attrib) {
    case 0:  /* str */
      if ( ch->real_abils.str > races[ i ].points.abils.min_str ) {
        ch->real_abils.str -= 1;
        points++;
      } else
        send_to_char("Your strength cannot go any lower.\r\n", ch);
      break;
    case 1:  /* int */
      if ( ch->real_abils.intel > races[ i ].points.abils.min_int ) {
        ch->real_abils.intel -= 1;
        points++;
      } else
        send_to_char("Your intelligence cannot go any lower.\r\n", ch);
      break;
    case 2:  /* wis */
      if ( ch->real_abils.wis > races[ i ].points.abils.min_wis ) {
        ch->real_abils.wis -= 1;
        points++;
      } else
        send_to_char("Your wisdom cannot go any lower.\r\n", ch);
      break;
    case 3:  /* dex */
      if ( ch->real_abils.dex > races[ i ].points.abils.min_dex ) {
        ch->real_abils.dex -= 1;
        points++;
      } else
        send_to_char("Your dexterity cannot go any lower.\r\n", ch);
      break;
    case 4:  /* con */
      if ( ch->real_abils.con > races[ i ].points.abils.min_con ) {
        ch->real_abils.con -= 1;
        points++;
      } else
        send_to_char("Your constitution cannot go any lower.\r\n", ch);
      break;
    case 5:  /* cha */
      if ( ch->real_abils.cha > races[ i ].points.abils.min_cha ) {
        ch->real_abils.cha -= 1;
        points++;
      } else
        send_to_char("Your charisma cannot go any lower.\r\n", ch);
      break;
    case 6:  /* will */
      if ( ch->real_abils.will > races[ i ].points.abils.min_wil ) {
        ch->real_abils.will -= 1;
        points++;
      } else
        send_to_char("Your will power cannot go any lower.\r\n", ch);
      break;
    default:
      send_to_char( "Hunh? Which attribute was that?\r\n", ch );
      break;
  }
  return points;
}

#if 0
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

  ch->real_abils.wis   = table[0];
  ch->real_abils.intel = table[1];
  ch->real_abils.str   = table[2];
  ch->real_abils.dex   = table[3];
  ch->real_abils.con   = table[4];
  ch->real_abils.cha   = table[5];
  ch->real_abils.will  = table[6];
  if (ch->real_abils.str == 18)
    ch->real_abils.str_add = number(0, 100);
  else
    ch->real_abils.str_add = 0;

  ch->aff_abils = ch->real_abils;
}

/**
 */
void set_height_weight(struct char_data *ch)
{
  int i = GET_RACE( ch );
  if ( IS_MALE( ch ) ) {
    GET_HEIGHT( ch ) = races[ i ].points.size[0].base_height +
      dice( races[ i ].points.size[0].mod_num_height,
            races[ i ].points.size[0].mod_size_height );
    GET_WEIGHT( ch ) = races[ i ].points.size[0].base_weight +
      dice( races[ i ].points.size[0].mod_num_weight,
            races[ i ].points.size[0].mod_size_weight );
  } else {
    GET_HEIGHT( ch ) = races[ i ].points.size[1].base_height +
      dice( races[ i ].points.size[1].mod_num_height,
            races[ i ].points.size[1].mod_size_height );
    GET_WEIGHT( ch ) = races[ i ].points.size[1].base_weight +
      dice( races[ i ].points.size[1].mod_num_weight,
            races[ i ].points.size[1].mod_size_weight );
  }
}

/* Some initializations for characters, including initial skills */
void do_start(struct char_data * ch)
{
  void advance_level(struct char_data * ch);
  void newbie_basket(struct char_data * ch);

  int i = GET_RACE( ch );
  
  GET_LEVEL(ch)     = 1;
  GET_EXP(ch)       = 1;
  GET_GUILD(ch)     = GUILD_NONE;
  GET_GUILD_LEV(ch) = 0;
  GET_PRACTICES(ch) = number(5, GET_WIS(ch));

  set_title(ch, NULL);
  GET_AGE_MOD(ch)   = number( races[ i ].points.points.bage,
                              races[ i ].points.points.max_bage );
  set_height_weight(ch);

  if (GET_PERMALIGN(ch) == ALIGN_GOOD)
    GET_ALIGNMENT(ch) = 1000;
  else if (GET_PERMALIGN(ch) == ALIGN_EVIL)
    GET_ALIGNMENT(ch) = -1000;
  else
    GET_ALIGNMENT(ch) = 0;

  /* Check array to see mins and maxs for race's points */
  ch->points.hitroll  = number( races[ i ].points.points.min_hi,
                                races[ i ].points.points.max_hi );
  ch->points.damroll  = number( races[ i ].points.points.min_da,
                                races[ i ].points.points.max_da );
  ch->points.max_hit  = number( races[ i ].points.points.min_hp,
                                races[ i ].points.points.max_hp );
  ch->points.max_mana = number( races[ i ].points.points.min_ma,
                                races[ i ].points.points.max_ma );
  ch->points.max_move = number( races[ i ].points.points.min_mo,
                                races[ i ].points.points.max_mo );

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
  int add_hp = 0, add_mana = 0, add_move = 0, i = GET_RACE( ch );

  extern struct wis_app_type wis_app[];
  extern struct con_app_type con_app[];

  add_hp   = number( races[ i ].points.points.min_lhp,
                     races[ i ].points.points.max_lhp );
  add_hp  += con_app[GET_CON(ch)].hitp;
  add_mana = number( races[ i ].points.points.min_lma,
                     races[ i ].points.points.max_lma );
  add_move = number( races[ i ].points.points.min_lmo,
                     races[ i ].points.points.max_lmo );

  ch->points.max_hit  += MAX(1, add_hp);
  ch->points.max_move += MAX(1, add_move);
  ch->points.max_mana += MAX(0, add_mana);

  GET_PRACTICES(ch)   += MAX(1, (int)wis_app[GET_WIS(ch)].bonus);

  GET_HITROLL(ch)     += number(0, 1);
  GET_DAMROLL(ch)     += number(0, 1);

  if ( IS_IMMORT(ch) ) {
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
  struct extra_descr_data *iterator;

  obj = read_object(OBJ_VNUM_DEFAULT_LIGHT, VNUMBER);
  equip_char(ch, obj, WEAR_HOLD);

  obj = read_object(OBJ_VNUM_DEFAULT_DAGGER, VNUMBER);
  equip_char(ch, obj, WEAR_WIELD);
  /* Personalize the newbie daggerr */
  sprintf(buf, "It appears to have a monogram denoting that it is owned by %s.\r\n", GET_NAME(ch));
  iterator = obj->ex_description;
  while (iterator != NULL)
    iterator = iterator->next;
  iterator = (struct extra_descr_data *) malloc(sizeof(struct extra_descr_data));
  iterator->keyword = str_dup("inscription");
  iterator->description = str_dup(buf);
  /* End of personalization */
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

static long min_exp_by_level[ MAX_LEVEL + 1 ];

/**
 * This method is responsible for loading all of the experience tables
 * from data/exp_table.
 */
void parse_experience( void )
{
  int index = 0;
  int line  = 1;
  long exp  = 0;
  FILE * fp = 0;

  log( "Loading experience table ..." );
  
  if ( !(fp = fopen( EXP_FILE, "r" ) ) ) {
    plog( "Unable to open experience table (%s)!",
          EXP_FILE );
    exit( 1 );
  }

  while ( !feof( fp ) ) {
    char c = getc( fp );

    if ( c == '#' ) { /* comment? */
      fread_to_eol( fp );
      line++;
    } else if ( c == '$' ) {
      // EOF is coming ...
      break;
    } else if ( isdigit( c ) ) { /* exp value? */
      ungetc( c, fp );
      exp = fread_long( fp );
      min_exp_by_level[ index++ ] = exp;
      fread_to_eol( fp );
      line++;

      if ( index > MAX_LEVEL + 1 ) {
        plog( "parse_experience: too many entries in exp_table! (%d/%d)\n",
             index, MAX_LEVEL );
        exit( 1 );
      }
    } else {
      plog( "parse_experience: error on line %d.\n", line );
      exit( 1 );
    }
  }

  if ( index < MAX_LEVEL ) {
    plog( "parse_experience: not enough entries in exp_table! (%d/%d)\n",
          index, MAX_LEVEL );
    exit( 1 );
  }
}

long exp_needed(int level)
{
    if ((unsigned) level > MAX_LEVEL) {
      plog("SYSERR: level argument (%d) to min_exp out of range!", level);
    }
    return (min_exp_by_level[level]);
}


/* Spells, skills and spheres are defined in lib/data/spells (load_spells()). */

/**
 * @param
 * @return
 */
int backstab_mult(int level)
{
  return MAX(1, ( level / 4 ) - 1);
}

int race_lookup( const char * name )
{
  int i;
  for ( i = 0; i < NUM_RACES; i++ )
    if ( !strcmp( name, races[ i ].name ) )
      return i;
  return 0;
}
