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
        fscanf( fl, "%s\n", buf2 );
        races[ index ].name   = strdup( buf2 );
        races[ index ].abbrev = fread_string( fl, buf2 );
        fscanf( fl, "%c %c %d\n",
                &sel, &cval, &ival );
        races[ index ].selector = LOWER( sel );
        assert( races[ index ].selector >= 'a' &&
                races[ index ].selector <= 'z' );
        races[ index ].isHumanoid = ( LOWER( cval ) == 't' ? TRUE : FALSE );
        races[ index ].points.rsize = ival;
        fread_to_eol( fl );

        races[ index ].points.bitvec = 1<<index;

        fscanf( fl, "%d %ld %c",
                &(races[ index ].points.regen),
                &(races[ index ].points.start),
                &cval );
        races[ index ].isNPC = ( LOWER( cval ) == 't' ? TRUE : FALSE );
        
        /* incase there are any comments, always read to the end of line */
        fread_to_eol( fl );

        fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_str),
                &(races[ index ].points.abils.max_str) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_int),
                &(races[ index ].points.abils.max_int) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_wis),
                &(races[ index ].points.abils.max_wis) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_dex),
                &(races[ index ].points.abils.max_dex) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_con),
                &(races[ index ].points.abils.max_con) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_cha),
                &(races[ index ].points.abils.max_cha) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.abils.min_wil),
                &(races[ index ].points.abils.max_wil) );
        fread_to_eol( fl );

        fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_hi),
                &(races[ index ].points.points.max_hi) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_da),
                &(races[ index ].points.points.max_da) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_hp),
                &(races[ index ].points.points.max_hp) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_ma),
                &(races[ index ].points.points.max_ma) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_mo),
                &(races[ index ].points.points.max_mo) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_lhp),
                &(races[ index ].points.points.max_lhp) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_lma),
                &(races[ index ].points.points.max_lma) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.points.min_lmo),
                &(races[ index ].points.points.max_lmo) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d",
                &(races[ index ].points.points.bage),
                &(races[ index ].points.points.max_bage) );
        fread_to_eol( fl );
        fscanf( fl, "%d %c",
                &(races[ index ].points.points.old_age),
                &cval );
        races[ index ].canSpeak = ( LOWER( cval ) == 't' ? TRUE : FALSE );
        fread_to_eol( fl );

        fscanf( fl, "%d %d %d",
                &(races[ index ].points.size[0].base_height),
                &(races[ index ].points.size[0].mod_num_height),
                &(races[ index ].points.size[0].mod_size_height) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d %d",
                &(races[ index ].points.size[0].base_weight),
                &(races[ index ].points.size[0].mod_num_weight),
                &(races[ index ].points.size[0].mod_size_weight) );
        fread_to_eol( fl );

        fscanf( fl, "%d %d %d",
                &(races[ index ].points.size[1].base_height),
                &(races[ index ].points.size[1].mod_num_height),
                &(races[ index ].points.size[1].mod_size_height) );
        fread_to_eol( fl );
        fscanf( fl, "%d %d %d",
                &(races[ index ].points.size[1].base_weight),
                &(races[ index ].points.size[1].mod_num_weight),
                &(races[ index ].points.size[1].mod_size_weight) );
        
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
      sprintf( rval, "%s  [%s%c%s] %s\r\n",
               rval, ( color == TRUE ? KBBL : "" ),
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
  skill_sphere(SKILL_READ_MAGIC,    SPHERE_GENERAL, 1);
  skill_sphere(SKILL_RIDE,          SPHERE_GENERAL, 1);
  skill_sphere(SKILL_SIGN,          SPHERE_GENERAL, 1);
  skill_sphere(SKILL_SWIM,          SPHERE_GENERAL, 1);
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
  spell_sphere(SPELL_MEDUSA_GAZE,        SPHERE_ACTS_OF_DEITIES,  60,  6);
  spell_sphere(SPELL_METEOR_SWARM,       SPHERE_ACTS_OF_DEITIES,  90,  9);
  spell_sphere(SPELL_PLAGUE,             SPHERE_ACTS_OF_DEITIES,  95, 12);
  spell_sphere(SPELL_PORTAL,             SPHERE_ACTS_OF_DEITIES,  35,  5);
  spell_sphere(SPELL_RESURRECTION,       SPHERE_ACTS_OF_DEITIES, 100, 20);

  /* Prayer -  Spell                     Afflictions */
  spell_sphere(SPELL_BLIND,              SPHERE_AFFLICTIONS,  1, 0);
  spell_sphere(SPELL_CRIPPLE,            SPHERE_AFFLICTIONS, 10, 0);
  spell_sphere(SPELL_DISEASE,            SPHERE_AFFLICTIONS, 10, 0);
  spell_sphere(SPELL_HARM_LIGHT,         SPHERE_AFFLICTIONS,  1, 0);
  spell_sphere(SPELL_HARM_SERIOUS,       SPHERE_AFFLICTIONS, 10, 0);
  spell_sphere(SPELL_HARM_CRITICAL,      SPHERE_AFFLICTIONS, 20, 1);
  spell_sphere(SPELL_HARM,               SPHERE_AFFLICTIONS, 60, 5);
  spell_sphere(SPELL_POISON,             SPHERE_AFFLICTIONS, 30, 2);
  spell_sphere(SPELL_PARALYZE,           SPHERE_AFFLICTIONS, 40, 2);

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
  spell_sphere(SPELL_KNIT_BONE,          SPHERE_CURES, 40, 2);
  spell_sphere(SPELL_RESTORE_LIMB,       SPHERE_CURES, 70, 6);

  /* Prayer -  Spell                     Hand Of Deities */
  spell_sphere(SPELL_ARMOR,              SPHERE_HAND_OF_DEITIES,   1, 0);
  spell_sphere(SPELL_ASTRAL_WALK,        SPHERE_HAND_OF_DEITIES, 100, 10);
  spell_sphere(SPELL_BALANCE,            SPHERE_HAND_OF_DEITIES,  70, 8);
  spell_sphere(SPELL_BLACKMANTLE,        SPHERE_HAND_OF_DEITIES,  75, 8);
  spell_sphere(SPELL_BLESS,              SPHERE_HAND_OF_DEITIES,   2, 0);
  spell_sphere(SPELL_CORRUPT,            SPHERE_HAND_OF_DEITIES,  70, 8);
  spell_sphere(SPELL_CREATE_FOOD,        SPHERE_HAND_OF_DEITIES,  10, 0);
  spell_sphere(SPELL_CREATE_WATER,       SPHERE_HAND_OF_DEITIES,  10, 0);
  spell_sphere(SPELL_FLAMESTRIKE,        SPHERE_HAND_OF_DEITIES,  90, 5);
  spell_sphere(SPELL_GROUP_ARMOR,        SPHERE_HAND_OF_DEITIES,  55, 2);
  spell_sphere(SPELL_INFRAVISION,        SPHERE_HAND_OF_DEITIES,  15, 0);
  spell_sphere(SPELL_PURIFY,             SPHERE_HAND_OF_DEITIES,  70, 8);
  spell_sphere(SPELL_REMOVE_CURSE,       SPHERE_HAND_OF_DEITIES,  40, 1);
  spell_sphere(SPELL_SANCTUARY,          SPHERE_HAND_OF_DEITIES,  60, 5);
  spell_sphere(SPELL_SILENCE,            SPHERE_HAND_OF_DEITIES,  50, 2);
  spell_sphere(SPELL_WATERBREATH,        SPHERE_HAND_OF_DEITIES,  30, 0);
  spell_sphere(SPELL_WATERWALK,          SPHERE_HAND_OF_DEITIES,  35, 0);
  spell_sphere(SPELL_SUMMON,             SPHERE_HAND_OF_DEITIES, 100, 25);
  spell_sphere(SPELL_WORD_OF_RECALL,     SPHERE_HAND_OF_DEITIES, 100, 50);

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
  spell_sphere(SPELL_KNOCK,                SPHERE_ENCHANTMENT, 15, 1);
  spell_sphere(SPELL_MIN_GLOBE,            SPHERE_ENCHANTMENT, 50, 5);
  spell_sphere(SPELL_MAJ_GLOBE,            SPHERE_ENCHANTMENT, 75, 8);
  spell_sphere(SPELL_PASS_DOOR,            SPHERE_ENCHANTMENT, 20, 1);
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
  spell_sphere(SPELL_SOUL_SUM_I,           SPHERE_ENTROPIC, 10,  1);
  spell_sphere(SPELL_SOUL_SUM_II,          SPHERE_ENTROPIC, 20,  2);
  spell_sphere(SPELL_SOUL_SUM_III,         SPHERE_ENTROPIC, 30,  3);
  spell_sphere(SPELL_SOUL_SUM_IV,          SPHERE_ENTROPIC, 40,  4);
  /* Arcane -  Spell                       Invocation */
  spell_sphere(SPELL_DISPEL_MAGIC,         SPHERE_INVOCATION, 10,  0);
  spell_sphere(SPELL_ENERGY_DRAIN,         SPHERE_INVOCATION, 10,  1);
  spell_sphere(SPELL_ICE_SHOWER,           SPHERE_INVOCATION, 30, 10);
  spell_sphere(SPELL_FIREBALL,             SPHERE_INVOCATION, 25,  1);
  spell_sphere(SPELL_FIRE_STORM,           SPHERE_INVOCATION, 50, 20);
  spell_sphere(SPELL_FLAME_BOLT,           SPHERE_INVOCATION, 25,  2);
  spell_sphere(SPELL_FORCE_BOLT,           SPHERE_INVOCATION, 50, 12);
  spell_sphere(SPELL_LIGHTNING_BOLT,       SPHERE_INVOCATION, 20,  0);
  spell_sphere(SPELL_MAGIC_MISSILE,        SPHERE_INVOCATION,  1,  0);

  /* Arcane -  Spell                       Phantasmic */
  spell_sphere(SPELL_COLOR_SPRAY,          SPHERE_PHANTASMIC, 25, 2);
  spell_sphere(SPELL_DISPEL_ILLUSION,      SPHERE_PHANTASMIC, 40, 2);
  spell_sphere(SPELL_DOUBLE_VISION,        SPHERE_PHANTASMIC, 15, 0);
  spell_sphere(SPELL_FALSE_GLOW,           SPHERE_PHANTASMIC,  1, 0);
  spell_sphere(SPELL_HYPNOSIS,             SPHERE_PHANTASMIC, 70, 8);
  spell_sphere(SPELL_ILLUSIONARY_PRES_I,   SPHERE_PHANTASMIC, 10, 1);
  spell_sphere(SPELL_ILLUSIONARY_PRES_II,  SPHERE_PHANTASMIC, 20, 5);
  spell_sphere(SPELL_ILLUSIONARY_PRES_III, SPHERE_PHANTASMIC, 30, 10);
  spell_sphere(SPELL_ILLUSIONARY_PRES_IV,  SPHERE_PHANTASMIC, 40, 20);
  spell_sphere(SPELL_INVISIBLE,            SPHERE_PHANTASMIC,  1, 0);
  spell_sphere(SPELL_MASK_PRESENCE,        SPHERE_PHANTASMIC, 25, 5);

  /* Thievery  Skill                       Looting */
  skill_sphere(SKILL_DETECT_TRAP,          SPHERE_LOOTING, 10);
  skill_sphere(SKILL_REMOVE_TRAP,          SPHERE_LOOTING, 30);
  skill_sphere(SKILL_SET_TRAP,             SPHERE_LOOTING, 50);
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
  skill_sphere(SKILL_MANUFACTURE_POISONS,  SPHERE_MURDER, 80);

  /* Thievery  Skill                       Thievery */
  skill_sphere(SKILL_DISGUISE,             SPHERE_THIEVING, 80);
  skill_sphere(SKILL_SPY,                  SPHERE_THIEVING, 20);
  skill_sphere(SKILL_HIDE,                 SPHERE_THIEVING,  5);
  skill_sphere(SKILL_SHADOW,               SPHERE_THIEVING, 40);
  skill_sphere(SKILL_SNEAK,                SPHERE_THIEVING,  1);

  /* Combat    Skill                       Hand to Hand */
  skill_sphere(SKILL_BASH,                 SPHERE_HAND_TO_HAND,   5);
  skill_sphere(SKILL_BERSERK,              SPHERE_HAND_TO_HAND,  70);
  skill_sphere(SKILL_BODYSLAM,             SPHERE_HAND_TO_HAND,  40);
  skill_sphere(SKILL_DEATHSTROKE,          SPHERE_HAND_TO_HAND, 100);
  skill_sphere(SKILL_DISARM,               SPHERE_HAND_TO_HAND,  30);
  skill_sphere(SKILL_DOORBASH,             SPHERE_HAND_TO_HAND,  80);
  skill_sphere(SKILL_HEADBUTT,             SPHERE_HAND_TO_HAND,  10);
  skill_sphere(SKILL_PUGILISM,             SPHERE_HAND_TO_HAND,  30);
  skill_sphere(SKILL_WRESTLING,            SPHERE_HAND_TO_HAND,  70);
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
  skill_sphere(SKILL_FEIGN_DEATH,          SPHERE_MARTIAL_ARTS, 60);
  skill_sphere(SKILL_FIRST_AID,            SPHERE_MARTIAL_ARTS, 1);
  skill_sphere(SKILL_ZEN_THINKING,         SPHERE_MARTIAL_ARTS, 50);
  skill_sphere(SKILL_QUIVERING_PALM,       SPHERE_MARTIAL_ARTS, 100);
  skill_sphere(SKILL_KARATE,               SPHERE_MARTIAL_ARTS, 75);

  /* Combat    Skill                       Weapons Prof */
  skill_sphere(SKILL_BLUNT,                SPHERE_WEAPONS,  1);
  skill_sphere(SKILL_SHARPEN,              SPHERE_WEAPONS, 40);
  skill_sphere(SKILL_TACTICS,              SPHERE_WEAPONS, 20);
  skill_sphere(SKILL_SMYTHE,               SPHERE_WEAPONS, 60);
  skill_sphere(SKILL_PIERCE,               SPHERE_WEAPONS,  1);
  skill_sphere(SKILL_RANGE,                SPHERE_WEAPONS, 10);
  skill_sphere(SKILL_SWORDS,               SPHERE_WEAPONS,  1);
  skill_sphere(SKILL_POLEARMS,             SPHERE_WEAPONS,  1);
  skill_sphere(SKILL_THROWING,             SPHERE_WEAPONS,  1);
}

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
