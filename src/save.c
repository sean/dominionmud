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
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  Envy Diku Mud improvements copyright (C) 1994 by Michael Quan, David   *
*  Love, Guilherme 'Willie' Arnold, and Mitchell Tse.                     *
*                                                                         *
*  Dominion Mud copyright (c) 1994, 1995, 1996, 1997, 2000, 2001 by       *
*  Sean Mountcastle, John Olvey and Kevin Huff                            *
\*********************************************************************** */

#define __SAVE_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/*
 * protos
 */
extern FILE * fpReserve;
extern struct descriptor_data *descriptor_list;
extern struct obj_data *object_list;
extern struct index_data *mob_index;

void rent_adjust( struct char_data * ch );
int  race_lookup( const char * str );
int skill_lookup( char * str );
int affect_lookup( char * str );
struct char_data * new_character( bool player );

void fread_to_eol(FILE *fp);
char *fread_word(FILE *fp);
char *fread_word_stat(FILE *fp, int *status);
char fread_letter(FILE *fp);
char *fread_string(FILE * fl, char *error);
int  fread_number(FILE *fp);
int  fread_number_stat(FILE *fp, int *status);

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	struct obj_data * rgObjNest	[ MAX_NEST ];

int stat;

/*
 * Local functions.
 */
void fwrite_char( struct char_data *ch,  FILE *fp );
void fwrite_obj( struct obj_data *obj, FILE *fp, int iNest );
void fwrite_obj_char( struct char_data *ch,  struct obj_data  *obj,
                      FILE *fp, int iNest );
void fwrite_alias( struct char_data * ch,
                   struct alias *alias, FILE *fp );

int  fread_char( struct char_data *ch,  FILE *fp );
struct obj_data *  fread_obj( FILE *fp );
int  fread_obj_char( struct char_data *ch,  FILE *fp );
int  fread_alias( struct char_data *ch,  FILE *fp );

/* Courtesy of Yaz of 4th Realm */
char *initial( const char *str )
{
    static char strint [ MAX_STRING_LENGTH ];

    strint[0] = LOWER( str[ 0 ] );
    return strint;
}

const char * player_dir( const char * name )
{
  const char * ae = "A-E";
  const char * fj = "F-J";
  const char * ko = "K-O";
  const char * pt = "P-T";
  const char * uz = "U-Z";

  if ( UPPER( name[ 0 ] ) < 'F' )
    return ae;
  else if ( UPPER( name[ 0 ] ) < 'K' )
    return fj;
  else if ( UPPER( name[ 0 ] ) < 'P' )
    return ko;
  else if ( UPPER( name[ 0 ] ) < 'U' )
    return pt;
  else
    return uz;
}

const char * capitalize( char * str )
{
  if ( islower( str[ 0 ] ) )
    str[ 0 ] = UPPER( str[ 0 ] );
  return str;
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
bool save_char_obj( struct char_data *ch )
{
    FILE *fp;
    char  buf     [ MAX_STRING_LENGTH ];
    char  strsave [ MAX_INPUT_LENGTH  ];
    bool  rval = FALSE;

    if ( IS_NPC( ch ) ) // || GET_LEVEL( ch ) < 2 )
      return rval;

    if ( GET_DESC(ch) && GET_DESC(ch)->original )
      ch = GET_DESC(ch)->original;

    // GET_SAVED(ch) = time( 0 );
    /* close down the reserve descriptor -- don't forget to reopen it */
    fclose( fpReserve );

    sprintf( strsave, "%s/%s/%s", PLAYER_DIR,
             player_dir( GET_NAME( ch ) ),
             capitalize( GET_NAME( ch ) ) );

    if ( !( fp = fopen( strsave, "w" ) ) ) {
      sprintf( buf, "save_char_obj: fopen %s: ", GET_NAME( ch ) );
      log( buf );
      perror( strsave );
      rval = FALSE;
    } else {
      fwrite_char( ch, fp );
      if ( ch->carrying )
        fwrite_obj_char( ch, ch->carrying, fp, 0 );
      if ( GET_ALIASES(ch) )
        fwrite_alias( ch, GET_ALIASES(ch), fp );
      fprintf( fp, "#END\n" );
      fclose( fp );
      rval = TRUE;
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return rval;
}

/*
 * Write the char to the file.
 *
 * @param ch the character to be written
 * @param fp the file to write to
 */
void fwrite_char( struct char_data *ch, FILE *fp )
{
  extern struct race_data * races;
  
  struct affected_type *paf;
  int          sn, i;
  
  fprintf( fp, "#%s\n", IS_NPC( ch ) ? "MOB" : "PLAYER"		);
  
  fprintf( fp, "Name        %s~\n",	GET_NAME(ch)			);
  fprintf( fp, "ShtDsc      %s~\n",	GET_SHORT_DESC(ch) ?
	   GET_SHORT_DESC(ch) : "" );
  fprintf( fp, "LngDsc      %s~\n",	GET_LONG_DESC(ch) ?
	   GET_LONG_DESC(ch) : "" );
  fprintf( fp, "Dscr        %s~\n",	GET_DESCRIPTION(ch) ?
	   GET_DESCRIPTION(ch) : "" );
  // fprintf( fp, "Prmpt       %s~\n",	ch->pcdata->prompt	);
  fprintf( fp, "Sx          %d\n",	GET_SEX(ch)			);
  fprintf( fp, "Race        %s~\n",	races[ (int)GET_RACE(ch) ].name );
  fprintf( fp, "Lvl         %d\n",	GET_LEVEL(ch)			);
  fprintf( fp, "Trst        %d\n",	GET_TRUST(ch)			);
  /*
    fprintf( fp, "Playd       %ld\n",
    GET_PLAYED(ch) + (int)( time( 0 ) - GET_LOGON(ch) )		);
  */
  // fprintf( fp, "Note        %ld\n",   (unsigned long)ch->last_note );
  fprintf( fp, "Room        %ld\n",
	   (  ch->in_room == NOWHERE && ch->was_in_room )
	   ? ch->was_in_room : ch->in_room );
  
  fprintf( fp, "HpMnMv      %d %d %d %d %d %d\n",
	   GET_HIT(ch), GET_MAX_HIT(ch),
	   GET_MANA(ch), GET_MAX_MANA(ch),
	   GET_MOVE(ch), GET_MAX_MOVE(ch) );
  fprintf( fp, "Gold        %ld\n",	GET_GOLD(ch)		);
  fprintf( fp, "Exp         %ld\n",	GET_EXP(ch)		);
  fprintf( fp, "Act         %lld\n",  PLR_FLAGS(ch)           );
  fprintf( fp, "Act2        %lld\n",  PLR2_FLAGS(ch)          );
  fprintf( fp, "Pref        %lld\n",  PRF_FLAGS(ch)		);
  fprintf( fp, "Pref2       %lld\n",  PRF2_FLAGS(ch)		);
  fprintf( fp, "AffdBy      %lld\n",	AFF_FLAGS(ch)		);
  fprintf( fp, "AffdBy2     %lld\n",	AFF2_FLAGS(ch)		);
  /* Bug fix from Alander */
  fprintf( fp, "Pos         %d\n",
	   GET_POS(ch) == POS_FIGHTING ? POS_STANDING : GET_POS(ch) );
  fprintf( fp, "Prac        %d\n",    GET_PRACTICES(ch)       );
  fprintf( fp, "PAlign      %d\n",	GET_PERMALIGN(ch)	);
  fprintf( fp, "CAlign      %d\n",	GET_ALIGNMENT(ch)	);
  fprintf( fp, "SavThr      " );
  for ( i = 0; i < NUM_SAVES; i++ )
    fprintf( fp, "%d%s",    GET_SAVE(ch, i), (i != NUM_SAVES-1 ? ", " : "\n")	);
  fprintf( fp, "Hitroll     %d\n",	GET_HITROLL(ch)		);
  fprintf( fp, "Damroll     %d\n",	GET_DAMROLL(ch)		);
  fprintf( fp, "Armr        " );
  for ( i = 0; i < ARMOR_LIMIT; i++ )
    fprintf( fp, "%d%s",   GET_AC(ch, i), (i != ARMOR_LIMIT-1 ? ", " : "\n") );
  fprintf( fp, "Wimp        %d\n",	GET_WIMP_LEV(ch)	);
  
  if ( IS_NPC( ch ) ) {
    fprintf( fp, "Vnum        %ld\n",	GET_MOB_VNUM(ch)	);
  } else {
    fprintf( fp, "Paswd       %s~\n",	GET_PASSWD(ch)		);
    fprintf( fp, "Poofin      %s~\n",	POOFIN(ch) ?
	     POOFIN(ch) : "" );
    fprintf( fp, "Poofout     %s~\n",	POOFOUT(ch) ?
	     POOFOUT(ch) : "" );
    fprintf( fp, "Ttle        %s~\n",	GET_TITLE(ch) ?
	     GET_TITLE(ch) : "" );
    fprintf( fp, "AtrPrm      %d/%d %d %d %d %d %d %d\n",
	     ch->real_abils.str,
	     ch->real_abils.str_add,
	     ch->real_abils.intel,
	     ch->real_abils.wis,
	     ch->real_abils.dex,
	     ch->real_abils.con,
	     ch->real_abils.cha,
	     ch->real_abils.will );
    
    fprintf( fp, "AtrMd       %d/%d %d %d %d %d %d %d\n",
	     ch->aff_abils.str, 
	     ch->aff_abils.str_add, 
	     ch->aff_abils.intel, 
	     ch->aff_abils.wis, 
	     ch->aff_abils.dex, 
	     ch->aff_abils.con, 
	     ch->aff_abils.cha, 
	     ch->aff_abils.will );
    
    fprintf( fp, "Conditions  " );
    for ( i = 0; i < MAX_COND; i++ )
      fprintf( fp, "%d%s", GET_COND(ch, i), (i != MAX_COND-1 ? ", " : "\n") );
    
    fprintf( fp, "Addictions  " );
    for ( i = 0; i < MAX_COND; i++ )
      fprintf( fp, "%d%s", GET_ADDICT(ch, i), (i != MAX_COND-1 ? ", " : "\n") );
    
    for ( sn = 0; sn < MAX_SKILLS; sn++ ) {
      if ( skill_name( sn ) &&
           strcmp( skill_name( sn ), "!UNUSED!" ) &&
           GET_SKILL(ch, sn) > 0 ) {
	fprintf( fp, "Skill       %d '%s'\n",
		 GET_SKILL(ch, sn),
		 skill_name( sn ) );
      }
    }
  }
  
  for ( paf = ch->affected; paf; paf = paf->next )  {
    fprintf( fp, "Afft       %18s~ %3d %3d %3d %lld\n",
	     skill_name( paf->type ),
	     paf->duration,
	     paf->modifier,
	     paf->location,
	     paf->bitvector );
  }
  
  for ( paf = ch->affected2; paf; paf = paf->next )  {
    fprintf( fp, "Afft2       %18s~ %3d %3d %3d %lld\n",
	     skill_name( paf->type ),
	     paf->duration,
	     paf->modifier,
	     paf->location,
	     paf->bitvector );
  }
  
  fprintf( fp, "End\n\n" );
  return;
}

void fwrite_obj_char( struct char_data * ch,
                      struct obj_data * obj,
                      FILE * fp, int iNest )
{
  fwrite_obj( obj, fp, iNest );
}

/*
 * Write an object and its contents.
 */
void fwrite_obj( struct obj_data *obj, FILE *fp, int iNest )
{
  // struct affected_type    * paf = NULL;
  struct extra_descr_data  * ed  = NULL;
  int i;

  /*
   * Slick recursion to write lists backwards,
   *   so loading them will load in forwards order.
   */
  if ( obj->next_content )
    fwrite_obj( obj->next_content, fp, iNest );
  
  /*
   * Castrate storage characters.
   */
  if ( obj->obj_flags.type_flag == ITEM_KEY )
    return;
  
  fprintf( fp, "#OBJECT\n" );
  fprintf( fp, "Nest         %d\n",	iNest			     );
  fprintf( fp, "Name         %s~\n",	obj->name		     );
  fprintf( fp, "ShortDescr   %s~\n",	obj->short_description	     );
  fprintf( fp, "Description  %s~\n",	obj->description	     );
  fprintf( fp, "ActionDescr  %s~\n",
	   obj->action_description ? obj->action_description : ""  );
  fprintf( fp, "Vnum         %ld\n",	obj->item_number	     );
  fprintf( fp, "ExtraFlags   %ld\n",	obj->obj_flags.extra_flags   );
  fprintf( fp, "WearFlags    %d\n",	obj->obj_flags.wear_flags    );
  fprintf( fp, "ItemType     %d\n",	obj->obj_flags.type_flag     );
  fprintf( fp, "Weight       %d\n",	obj->obj_flags.weight	     );
  fprintf( fp, "Timer        %d\n",	obj->obj_flags.timer	     );
  fprintf( fp, "Cost         %d\n",	obj->obj_flags.cost	     );
  fprintf( fp, "Values       %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
	   obj->obj_flags.value[0], obj->obj_flags.value[1],
	   obj->obj_flags.value[2], obj->obj_flags.value[3],
	   obj->obj_flags.value[4], obj->obj_flags.value[5],
	   obj->obj_flags.value[6], obj->obj_flags.value[7],
	   obj->obj_flags.value[8], obj->obj_flags.value[9] );
  
  switch ( obj->obj_flags.type_flag ) {
  case ITEM_POTION:
  case ITEM_SCROLL:
    if ( obj->obj_flags.value[1] > 0 ) {
      fprintf( fp, "Spell 1      '%s'\n", 
	       skill_name( obj->obj_flags.value[1] ) );
    }
    
    if ( obj->obj_flags.value[2] > 0 ) {
      fprintf( fp, "Spell 2      '%s'\n", 
	       skill_name( obj->obj_flags.value[2] ) );
    }
    
    if ( obj->obj_flags.value[3] > 0 ) {
      fprintf( fp, "Spell 3      '%s'\n", 
	       skill_name( obj->obj_flags.value[3] ) );
    }
    break;
    
  case ITEM_STAFF:
  case ITEM_WAND:
    if ( obj->obj_flags.value[3] > 0 ) {
      fprintf( fp, "Spell 3      '%s'\n", 
	       skill_name( obj->obj_flags.value[3] ) );
    }
    break;
    
  default:
    /* ERROR */
    break;
  }

  for ( i = 0; i < MAX_OBJ_AFFECT; i++ )
    fprintf( fp, "Affect       %d %d\n",
	     obj->affected[ i ].location,
	     obj->affected[ i ].modifier );
  /*
  paf = obj->affected;
  for ( paf = obj->affected; paf; paf = paf->next ) {
    fprintf( fp, "Affect       %d %d %d %d %ld\n",
	     paf->type, paf->duration,
	     paf->modifier, paf->location,
	     paf->bitvector );
  }
  */
  
  for ( ed = obj->ex_description; ed; ed = ed->next ) {
    fprintf( fp, "ExtraDescr   %s~ %s~\n",
	     ed->keyword, ed->description );
  }
  
  fprintf( fp, "End\n\n" );

  if ( obj->contains )
    fwrite_obj( obj->contains, fp, iNest + 1 );
  
  tail_chain( );
  return;
}

/**
 * @param name the name of the character to check for
 * @return true if the character exists, false otherwise
 */
bool char_exists( const char * name )
{
  FILE * fp = NULL;
  char strsave [ MAX_INPUT_LENGTH ];
  bool found = FALSE;
  
  fclose( fpReserve );

  sprintf( strsave, "%s/%s/%s%s", PLAYER_DIR,
           player_dir( name ),
           capitalize( (char*)name ), ".gz" );
  
  if ( ( fp = fopen( strsave, "r" ) ) ) {
    found = TRUE;
    fclose( fp );
  }
  
  /* now try normal file name, if not already found */
  if ( !found ) {
    sprintf( strsave, "%s/%s/%s", PLAYER_DIR,
             player_dir( name ),
             capitalize( (char*)name ) );
    
    if ( ( fp = fopen( strsave, "r" ) ) ) {
      found = TRUE;
      fclose( fp );
    }
  }

  fpReserve = fopen( NULL_FILE, "r" );

  return found;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( struct descriptor_data * d, const char * name )
{
  FILE      *fp;
  struct char_data *ch;
  char       strsave [ MAX_INPUT_LENGTH ];
  bool       found;
  char       sorry_player [] =
    "********************************************************\n\r"
    "** One or more of the critical fields in your player  **\n\r"
    "** file were corrupted since you last played.  Please **\n\r"
    "** contact an administrator or programmer to          **\n\r"
    "** investigate the recovery of your characters.       **\n\r"
    "********************************************************\n\r";
  char       sorry_object [] =
    "********************************************************\n\r"
    "** One or more of the critical fields in your player  **\n\r"
    "** file were corrupted leading to the loss of one or  **\n\r"
    "** more of your possessions.                          **\n\r"
    "********************************************************\n\r";
  char       sorry_alias [] =
    "********************************************************\n\r"
    "** One or more of the critical fields in your player  **\n\r"
    "** file were corrupted leading to the loss of one or  **\n\r"
    "** more of your aliases.                              **\n\r"
    "********************************************************\n\r";
  
  ch  				= new_character( TRUE );
  
  d->character			= ch;
  GET_DESC(ch)			= d;
  ch->player.name   = str_dup( name );
  /*
  ch->pcdata->prompt                  = str_dup( daPrompt );
  ch->last_note                       = 0;
  */
  strcpy( GET_PASSWD(ch), "" );
  POOFIN(ch)			= str_dup( "" );
  POOFOUT(ch)			= str_dup( "" );
  GET_TITLE(ch)			= str_dup( "" );
  GET_STR(ch)			= 10;
  GET_ADD(ch)			= 10; 
  GET_INT(ch)			= 10; 
  GET_WIS(ch)			= 10; 
  GET_DEX(ch)			= 10; 
  GET_CON(ch)			= 10; 
  GET_CHA(ch)			= 10; 
  GET_WILL(ch)			= 10; 
  GET_COND(ch, DRUNK)	= 48;
  GET_COND(ch, FULL)	= 48;
  GET_COND(ch, THIRST)	= 48;
  GET_COND(ch, TIRED)	= 48;
  
  // ch->pcdata->switched                = FALSE;
  
  found = FALSE;
  fclose( fpReserve );
  
  /* parsed player file directories by Yaz of 4th Realm */
  /* decompress if .gz file exists - Thx Alander */
  sprintf( strsave, "%s/%s/%s%s", PLAYER_DIR,
           player_dir( GET_NAME(ch) ),
           capitalize( GET_NAME(ch) ), ".gz" );
  
    if ( ( fp = fopen( strsave, "r" ) ) ) {
      char       buf     [ MAX_STRING_LENGTH ];
      
      fclose( fp );
      sprintf( buf, "gzip -dfq %s", strsave );
      system( buf );
    }

    sprintf( strsave, "%s/%s/%s", PLAYER_DIR,
             player_dir( GET_NAME(ch) ),
             capitalize( GET_NAME(ch) ) );

    if ( ( fp = fopen( strsave, "r" ) ) ) {
	char buf[ MAX_STRING_LENGTH ];
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char *word;
	    char  letter;
	    int   status;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
              log( "Load_char_obj: # not found." );
              break;
	    }

	    word = fread_word_stat( fp, &status );

	    if ( !str_cmp( word, "PLAYER" ) )
	    {
	        if ( fread_char ( ch, fp ) )
		{
		    sprintf( buf,
			    "Load_char_obj:  %s section PLAYER corrupt.\n\r",
			    name );
		    log( buf );
		    SEND_TO_Q( sorry_player, d );

		    /* 
		     * In case you are curious,
		     * it is ok to leave ch alone for close_socket
		     * to free.
		     * We want to now kick the bad character out as
		     * what we are missing are MANDATORY fields.  -Kahn
		     */
		    SET_BIT( PRF_FLAGS(ch), PLR_DELETED );
		    return TRUE;
		}
	    }
	    else if ( !str_cmp( word, "OBJECT" ) )
	    {
	        if ( !fread_obj_char( ch, fp ) )
		{
		    sprintf( buf,
			    "Load_char_obj:  %s section OBJECT corrupt.\n\r",
			    name );
		    log( buf );
		    SEND_TO_Q( sorry_object, d );
		    return FALSE;
		}
	    }
	    else if ( !str_cmp( word, "ALIAS"    ) ) {
              if ( !fread_alias( ch, fp ) )
              {
                sprintf( buf,
                         "Load_char_obj: %s section ALIAS corrupt.\n\r",
                         name );
                log( buf );
                SEND_TO_Q( sorry_alias, d );
                return FALSE;
              }
            }
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		log( "Load_char_obj: bad section." );
		break;
	    }
	} /* for */

	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );

    if ( found )
      rent_adjust( d->character );
    
    return found;
}

/*
 * Read in a char.
 */
int fread_char( struct char_data *ch, FILE *fp )
{
  int         error_count = 0;
  char        *word;
  char        buf [ MAX_STRING_LENGTH ];
  struct affected_type *paf;
  int         sn;
  int         i;
  int         j;
  int         status;
  int         status1;
  char        *p;
  int         tmpi;
  int         num_keys;
  int         last_key = 0;
  
  char        def_sdesc  [] = "Your short description was corrupted.";
  char        def_ldesc  [] = "Your long description was corrupted.";
  char        def_desc   [] = "Your description was corrupted.";
  char        def_title  [] = "Your title was corrupted.";
  
  struct key_data key_tab [] = {
    { "ShtDsc", TRUE,  (int) &def_sdesc,	{ &GET_SHORT_DESC(ch),   NULL } },
    { "LngDsc", TRUE,  (int) &def_ldesc,	{ &GET_LONG_DESC(ch),    NULL } },
    { "Dscr",   TRUE,  (int) &def_desc,		{ &GET_DESCRIPTION(ch),  NULL } },
    { "Sx",     FALSE, SEX_MALE,		{ &GET_SEX(ch),          NULL } },
    { "Race",   FALSE, MAND,			{ &GET_RACE(ch),         NULL } },
    { "Lvl",    FALSE, MAND,			{ &GET_LEVEL(ch),        NULL } },
    { "Trst",   FALSE, 0,			{ &GET_TRUST(ch),        NULL } },
    { "HpMnMv", FALSE, MAND,			{ &GET_HIT(ch),
						  &GET_MAX_HIT(ch),
						  &GET_MANA(ch),
						  &GET_MAX_MANA(ch),
						  &GET_MOVE(ch),
						  &GET_MAX_MOVE(ch),     NULL } },
    { "Gold",   FALSE, 0,			{ &GET_GOLD(ch),         NULL } },
    { "Exp",    FALSE, MAND,			{ &GET_EXP(ch),          NULL } },
    { "Act",     FALSE, DEFLT,			{ &PLR_FLAGS(ch),        NULL } },
    { "Act2",    FALSE, DEFLT,			{ &PLR2_FLAGS(ch),       NULL } },
    { "AffdBy",  FALSE, 0,			{ &AFF_FLAGS(ch),        NULL } },
    { "AffdBy2", FALSE, 0,			{ &AFF2_FLAGS(ch),       NULL } },
    { "Pref",    FALSE, 0,			{ &PRF_FLAGS(ch),        NULL } },
    { "Pref2",   FALSE, 0,			{ &PRF2_FLAGS(ch),       NULL } },
    { "Pos",    FALSE, POS_STANDING, 		{ &GET_POS(ch),          NULL } },
    { "Prac",   FALSE, MAND,			{ &GET_PRACTICES(ch),    NULL } },
    { "PAlign",  FALSE, 0,			{ &GET_PERMALIGN(ch),    NULL } },
    { "CAlign",  FALSE, 0,			{ &GET_ALIGNMENT(ch),    NULL } },
    { "Ttle",   TRUE,  (int) &def_title,	{ &GET_TITLE(ch),        NULL } },
#if 0
    { "SavThr", FALSE, MAND,			{ &ch->saving_throw,  NULL } },
    { "Hit",    FALSE, MAND,			{ &ch->hitroll,       NULL } },
    { "Dam",    FALSE, MAND,			{ &ch->damroll,       NULL } },
    { "Armr",   FALSE, MAND,			{ &ch->armor,         NULL } },
    { "Wimp",   FALSE, 10,			{ &ch->wimpy,         NULL } },
    { "Deaf",   FALSE, 0,			{ &ch->deaf,          NULL } },
    { "Immskll",TRUE,  DEFLT,			{ &ch->pcdata->immskll,
                                                  NULL } },
    { "AtrPrm", FALSE, MAND,			{ &ch->pcdata->perm_str,
						  &ch->pcdata->perm_int,
						  &ch->pcdata->perm_wis,
						  &ch->pcdata->perm_dex,
						  &ch->pcdata->perm_con,
                                                  NULL } },
    { "AtrMd",  FALSE, MAND,			{ &ch->pcdata->mod_str,
						  &ch->pcdata->mod_int,
						  &ch->pcdata->mod_wis,
						  &ch->pcdata->mod_dex,
						  &ch->pcdata->mod_con,
                                                  NULL } },
    { "Cond",   FALSE, DEFLT,			{ &ch->pcdata->condition [0],
						  &ch->pcdata->condition [1],
						  &ch->pcdata->condition [2],
                                                  NULL } },
    { "Pglen",  FALSE, 20,			{ &ch->pcdata->pagelen,
                                                  NULL } },
    { "Playd",   FALSE, 0,			{ &ch->played,        NULL } },
#endif
    { "Paswd",   TRUE,  MAND,			{ &GET_PASSWD(ch),    NULL } },
    { "Poofin",  TRUE,  DEFLT,			{ &POOFIN(ch),        NULL } },
    { "Poofout", TRUE,  DEFLT,			{ &POOFOUT(ch),       NULL } },
    { "\0",     FALSE, 0                                                   } };
  
  for ( num_keys = 0; *key_tab [num_keys].key; )
    num_keys++;
  
  for ( ; !feof (fp) ; )
  {
    
    word = fread_word_stat( fp, &status );
    
    if ( !word )
    {
      log( "fread_char:  Error reading key.  EOF?" );
      fread_to_eol( fp );
      break;
    }
    
    /* This little diddy searches for the keyword
       from the last keyword found */
    
    for ( i = last_key;
          i < last_key + num_keys &&
                str_cmp (key_tab [i % num_keys].key, word); )
      i++;
    
    i = i % num_keys;
    
    if ( !str_cmp (key_tab [i].key, word) )
      last_key = i;
    else
      i = num_keys;
    
    if ( *key_tab [i].key )         /* Key entry found in key_tab */
    {
      if ( key_tab [i].string == SPECIFIED )
        log( "Key already specified." );
      
      /* Entry is a string */
      
      else
        if ( key_tab [i].string )
        {
          if ( ( p = fread_string( fp, (char*)&status ) ) && !status )
          {
            free( *(char **)key_tab [i].ptrs [0] );
            *(char **)key_tab [i].ptrs [0] = p;
          }
        }
      
      /* Entry is an integer */
        else
          for ( j = 0; key_tab [i].ptrs [j]; j++ )
          {
            tmpi = fread_number_stat( fp, &status );
            if ( !status )
              *(int *)key_tab [i].ptrs [j] = tmpi;
          }
      
      if ( status )
      {
        fread_to_eol( fp );
        continue;
      }
      else
        key_tab [i].string = SPECIFIED;
    }
    
    else if ( *word == '*' || !str_cmp( word, "Nm" ) )
      fread_to_eol( fp );
    
    else if ( !str_cmp( word, "End" ) )
      break;
    
    else if ( !str_cmp( word, "Room" ) )
    {
      ch->in_room = fread_number_stat( fp, &status );
      if ( !ch->in_room )
        ch->in_room = NOWHERE;
    }
    
    else if ( !str_cmp( word, "Race" ) )
    {
      i  = race_lookup( fread_string( fp, (char*)&status ) );
      
      if ( status )
        log( "Fread_char: Unknown Race." );
      else
        GET_RACE(ch) = i;
    }
    
    else if ( !str_cmp( word, "Skill" ) )
    {
      i  = fread_number_stat( fp, &status );
      sn = skill_lookup( fread_word_stat( fp, &status1 ) );
      
      if ( status || status1 )
      {
        log( "Fread_char: Error reading skill." );
        fread_to_eol( fp );
        continue;
      }
      
      if ( sn < 0 )
        log( "Fread_char: unknown skill." );
      else
        GET_SKILL(ch, sn) = i;
    }
    
    else if ( !str_cmp ( word, "Afft" ) )
    {
      
      int status;

      CREATE(paf, struct affected_type, 1);
      memset(paf, 0, sizeof( struct affected_type ));
      
      paf->type           = skill_lookup( fread_string( fp,
                                                        (char*)&status ) );
      paf->duration       = fread_number_stat( fp, &status );
      paf->modifier       = fread_number_stat( fp, &status );
      paf->location       = fread_number_stat( fp, &status );
      paf->bitvector      = fread_number_stat( fp, &status );
      paf->next           = ch->affected;
      ch->affected        = paf;
    }
    
    else
    {
      sprintf( buf, "fread_char: Unknown key '%s' in pfile.", word );
      log( buf );
      fread_to_eol( fp );
    }
    
  }
  
  /* Require all manditory fields, set defaults */
  
  for ( i = 0; *key_tab [i].key; i++ )
  {
    
    if ( key_tab [i].string == SPECIFIED ||
         key_tab [i].deflt == DEFLT )
      continue;
    
    if ( key_tab [i].deflt == MAND )
    {
      sprintf( buf, "Manditory field '%s' missing from pfile.",
               key_tab [i].key );
      log( buf );
      error_count++;
      continue;
    }
    
    /* This if/else sets default strings and numbers */
    
    if ( key_tab [i].string && key_tab [i].deflt )
    {
      free( *(char **)key_tab [i].ptrs [0] );
      *(char **)key_tab [i].ptrs [0] =
        str_dup( (char *)key_tab [i].deflt );
    }
    else
      for ( j = 0; key_tab [i].ptrs [j]; j++ )
        *(int *)key_tab [i].ptrs [j] = key_tab [i].deflt;
  }
  
  /* Fixups */
  
  if ( GET_TITLE(ch) && isalnum( (int)*GET_TITLE(ch) ) )
  {
    sprintf( buf, " %s", GET_TITLE(ch) );
    free( GET_TITLE(ch) );
    GET_TITLE(ch) = str_dup( buf );
  }
  
  return error_count;
}

void recover( FILE *fp, long fpos )
{
    char        buf[ MAX_STRING_LENGTH ];

    fseek( fp, fpos, 0 );

    while ( !feof (fp) )
    {
        fpos = ftell( fp );

        if ( !fgets( buf, MAX_STRING_LENGTH, fp ) )
            return;

        if ( !strncmp( buf, "#OBJECT", 7 ) ||
             !strncmp( buf, "#END", 4 ) )
	{
            fseek( fp, fpos, 0 );
            return;
	}
    }
}

struct obj_data * fread_obj( FILE * fp )
{
  struct extra_descr_data *ed;
  struct obj_data         obj;
  struct obj_data         *new_obj;
  struct affected_type    *paf;
  char              buf[ MAX_STRING_LENGTH ];
  char             *spell_name = NULL;
  char             *p          = NULL;
  char             *word;
  char             *tmp_ptr;
  bool              fNest;
  bool              fVnum;
  long              fpos;
  int               iNest;
  int               iValue;
  int               status;
  int               sn;
  int               vnum;
  int               num_keys;
  int               last_key   = 0;
  int               i, j, tmpi, obj_idx = 0;

  char              corobj [] = "This object was corrupted.";

    struct key_data key_tab [] =
      {
	{ "Name",        TRUE,  MAND,             { &obj.name,        NULL } },
	{ "ShortDescr",  TRUE,  (int) &corobj,    { &obj.short_description, NULL } },
	{ "Description", TRUE,  (int) &corobj,    { &obj.description, NULL } },
        { "ActionDescr", TRUE,  NULL,             { &obj.action_description, NULL } },
        { "Vnum",        FALSE, MAND,             { &obj.item_number, NULL } },
	{ "ExtraFlags",  FALSE, MAND,             { &obj.obj_flags.extra_flags, NULL } },
	{ "WearFlags",   FALSE, MAND,             { &obj.obj_flags.wear_flags,  NULL } },
	{ "ItemType",    FALSE, MAND,             { &obj.obj_flags.type_flag,   NULL } },
	{ "Weight",      FALSE, 10,               { &obj.obj_flags.weight,      NULL } },
	{ "Timer",       FALSE, 0,                { &obj.obj_flags.timer,       NULL } },
	{ "Cost",        FALSE, 300,              { &obj.obj_flags.cost,        NULL } },
	{ "Values",      FALSE, MAND,             { &obj.obj_flags.value [0],
						    &obj.obj_flags.value [1],
						    &obj.obj_flags.value [2],
						    &obj.obj_flags.value [3],
						    &obj.obj_flags.value [4],
						    &obj.obj_flags.value [5],
						    &obj.obj_flags.value [6],
						    &obj.obj_flags.value [7],
						    &obj.obj_flags.value [8],
						    &obj.obj_flags.value [9],   NULL } },
	{ "\0",          FALSE, 0                                          } };

    memset( &obj, 0, sizeof( struct obj_data ) );

    obj.name               = str_dup( "" );
    obj.short_description  = str_dup( "" );
    obj.description        = str_dup( "" );
    obj.action_description = NULL;

    fNest           = FALSE;
    fVnum           = TRUE;
    iNest           = 0;

    CREATE(new_obj, struct obj_data, 1);
    memset(new_obj, 0, sizeof( struct obj_data ));

    for ( num_keys = 0; *key_tab [num_keys].key; )
        num_keys++;

    for ( fpos = ftell( fp ) ; !feof( fp ) ; )
    {

        word = fread_word_stat( fp, &status );

        for ( i = last_key;
              i < last_key + num_keys &&
                str_cmp( key_tab [i % num_keys].key, word ); )
            i++;

        i = i % num_keys;

        if ( !str_cmp( key_tab [i].key, word ) )
            last_key = i + 1;
        else
            i = num_keys;

        if ( *key_tab [i].key )         /* Key entry found in key_tab */
	{
            if ( key_tab [i].string == SPECIFIED )
              log( "Key already specified." );
            /* Entry is a string */
            else if ( key_tab [i].string )
	    {
                if ( ( p = fread_string( fp, (char*)&status ) ) && !status )
		{
                   free( * (char **) key_tab [i].ptrs [0] );
                   * (char **) key_tab [i].ptrs [0] = p;
		}
	    }

                        /* Entry is an integer */
            else
                for ( j = 0; key_tab [i].ptrs [j]; j++ )
		{
                    tmpi = fread_number_stat( fp, &status );
                    if ( !status )
                        * (int *) key_tab [i].ptrs [j] = tmpi;
		}

            if ( status )
	    {
                fread_to_eol( fp );
                continue;
	    }
	    else
                key_tab [i].string = SPECIFIED;
	}

        else if ( *word == '*' )
            fread_to_eol( fp );

        else if ( !str_cmp( word, "End" ) )
	{
            if ( !fNest || !fVnum )
	    {
                log( "Fread_obj: incomplete object." );

		recover( fp, fpos        );
		free( obj.name        );
		free( obj.short_description );
		free( obj.description );
		extract_obj( new_obj         );

		return FALSE;
	    }
            break;
	}

        else if ( !str_cmp( word, "Nest" ) )
	{

            iNest = fread_number_stat( fp, &status );

            if ( status )       /* Losing track of nest level is bad */
                iNest = 0;      /* This makes objs go to inventory */

            else if ( iNest < 0 || iNest >= MAX_NEST ) {
              char tmp[32];
              sprintf( tmp, "Fread_obj: bad nest %d.", iNest );
              log( tmp );
            } else {
                rgObjNest[iNest] = new_obj;
                fNest = TRUE;
	    }
	}

        else if ( !str_cmp( word, "Spell" ) )
	{

            iValue = fread_number_stat( fp, &status );

            if ( !status )
                spell_name = fread_word_stat( fp, &status );

            if ( status )       /* Recover is to skip spell */
	    {
                fread_to_eol( fp );
                continue;
	    }

            sn = skill_lookup( spell_name );

            if ( iValue < 0 || iValue > 9 ) {
              char tmp[32];
              sprintf( tmp, "Fread_obj: bad iValue %d.", iValue );
              log( tmp );
            } else if ( sn < 0 )
                log( "Fread_obj: unknown skill." );

            else
                obj.obj_flags.value [iValue] = sn;
	}

        else if ( !str_cmp( word, "Vnum" ) )
	{

            vnum = fread_number_stat( fp, &status );

            if ( status )               /* Can't live without vnum */
	    {
		recover( fp, fpos        );
		free( obj.name        );
		free( obj.short_description );
		free( obj.description );
		extract_obj( new_obj         );
		return FALSE;
	    }

            if ( !( obj.item_number = vnum ) ) {
              char tmp[32];
              sprintf( tmp, "Fread_obj: bad vnum %d.", vnum );
              log( tmp );
            } else
              fVnum = TRUE;
	}

	/* The following keys require extra processing */
        if ( !str_cmp( word, "Affect" ) )
	{
	  assert( obj_idx < MAX_OBJ_AFFECT );
	  obj.affected[obj_idx].location = fread_number_stat( fp, &status );
	  obj.affected[obj_idx].modifier = fread_number_stat( fp, &status );
	  obj_idx++;
	  /*
          CREATE(paf, struct affected_type, 1);
          memset(paf, 0, sizeof( struct affected_type ));

          paf->type       = fread_number_stat( fp, &status );
          paf->duration   = fread_number_stat( fp, &status );
          paf->modifier   = fread_number_stat( fp, &status );
          paf->location   = fread_number_stat( fp, &status );
          paf->bitvector  = fread_number_stat( fp, &status );
          
          paf->next = obj.affected;
          obj.affected = paf;
	  */
	}

        else if ( !str_cmp( word, "ExtraDescr" ) )
	{
	    tmp_ptr = fread_string( fp, (char*)&status );

            if ( !status )
                p = fread_string( fp, (char*)&status );

            if ( status )
	    {
		recover( fp, fpos        );
		free( obj.name        );
		free( obj.short_description );
		free( obj.description );
		extract_obj( new_obj         );
		return FALSE;
	    }

            CREATE(ed, struct extra_descr_data, 1);
            memset(ed, 0, sizeof( struct extra_descr_data ));

            ed->keyword     = tmp_ptr;
            ed->description = p;
            ed->next        = obj.ex_description;
            obj.ex_description = ed;
	}
    }
                /* Require all manditory fields, set defaults */

    for ( i = 0; *key_tab [i].key; i++ )
    {

        if ( key_tab [i].string == SPECIFIED ||
             key_tab [i].deflt == DEFLT )
            continue;

        if ( key_tab [i].deflt == MAND )
	{
            sprintf( buf, "Manditory obj field '%s' missing from pfile.",
		    key_tab [i].key );
            log( buf );

	    recover( fp, fpos        );
	    free( obj.name        );
	    free( obj.short_description );
	    free( obj.description );
	    extract_obj( new_obj         );

	    return FALSE;
	}

                /* This if/else sets default strings and numbers */

        if ( key_tab [i].string && key_tab [i].deflt )
            * (char **) key_tab [i].ptrs [0] =
                        str_dup ( (char *) key_tab [i].deflt );
        else
            for ( j = 0; key_tab [i].ptrs [j]; j++ )
                * (int *) key_tab [i].ptrs [j] = key_tab [i].deflt;
    }

    memcpy( new_obj, &obj, sizeof( struct obj_data ) );

    new_obj->next = object_list;
    object_list   = new_obj;

    // new_obj->pIndexData->count++;
    if ( iNest != 0 || rgObjNest[iNest] )
      obj_to_obj( new_obj, rgObjNest[iNest-1] );

    return new_obj;
}


int fread_obj_char( struct char_data *ch, FILE *fp )
{
  struct obj_data * obj = NULL;

  if ( (obj = fread_obj( fp )) ) {
    obj_to_char( obj, ch );
    return TRUE;
  }
  return FALSE;
}

void    fwrite_alias(  struct char_data *ch,
                       struct alias * tmp, FILE *fp )
{
  while ( tmp != NULL ) {
    fprintf( fp, "#ALIAS\n" );
    fprintf( fp, "Type         %d\n",	tmp->type        );
    fprintf( fp, "Alias        %s~\n",	tmp->alias       );
    fprintf( fp, "Replacement  %s~\n",	tmp->replacement );
    fprintf( fp, "End\n" );
    tmp = tmp->next;
  }
}

int     fread_alias( struct char_data *ch,  FILE *fp )
{
  struct alias *a, tmp;
  int           num_keys, status, i, j, last_key = 0, tmpi;
  long          fpos;
  char         *p, *word;
  
  struct key_data key_tab [] = {
    { "Type",        FALSE, MAND,		{ &tmp.type,        NULL } },
    { "Alias",       TRUE,  MAND,	        { &tmp.alias,       NULL } },
    { "Replacement", TRUE,  MAND,		{ &tmp.replacement, NULL } },
    { "\0",          FALSE, 0                                          } };
  
  
  memset( &tmp, 0, sizeof( struct alias ) );
  
  tmp.type        = ALIAS_SIMPLE;
  tmp.alias       = str_dup( "" );
  tmp.replacement = str_dup( "" );
  
  for ( num_keys = 0; *key_tab[num_keys].key; )
    num_keys++;
  
  for ( fpos = ftell( fp ) ; !feof( fp ) ; ) {
    
    word = fread_word_stat( fp, &status );
    
    for ( i = last_key;
          i < last_key + num_keys &&
            str_cmp( key_tab [i % num_keys].key, word ); )
      i++;
    
    i = i % num_keys;
    
    if ( !str_cmp( key_tab [i].key, word ) )
      last_key = i + 1;
    else
      i = num_keys;
    
    if ( *key_tab[i].key )         /* Key entry found in key_tab */
    {
      if ( key_tab[i].string == SPECIFIED )
        log( "Key already specified." );
      
      /* Entry is a string */
      
      else if ( key_tab[i].string )
      {
        if ( ( p = fread_string( fp, (char*)&status ) ) && !status )
        {
            free( * (char **) key_tab [i].ptrs [0] );
            * (char **) key_tab [i].ptrs [0] = p;
        }
      }
      
      /* Entry is an integer */
      else
        for ( j = 0; key_tab [i].ptrs [j]; j++ )
        {
          tmpi = fread_number_stat( fp, &status );
          if ( !status )
            * (int *) key_tab [i].ptrs [j] = tmpi;
        }
      
      if ( status )
      {
        fread_to_eol( fp );
        continue;
      }
      else
        key_tab [i].string = SPECIFIED;
    }
      /* entry is a comment */      
    else if ( *word == '*' )
      fread_to_eol( fp );
    /* end of alias */
    else if ( !str_cmp( word, "End" ) )
    {
      break;
    }
  }
  
  /* Require all manditory fields, set defaults */
  for ( i = 0; *key_tab [i].key; i++ )
  {
    if ( key_tab [i].string == SPECIFIED ||
         key_tab [i].deflt == DEFLT )
      continue;
    
    if ( key_tab [i].deflt == MAND )
    {
      sprintf( buf, "Manditory obj field '%s' missing from pfile.",
               key_tab [i].key );
      log( buf );
      
      recover( fp, fpos        );
      free( tmp.alias       );
      free( tmp.replacement );
      
      return FALSE;
    }
  }
  
  /* now add the alias to the character */
  CREATE(a, struct alias, 1);
  a->type  = tmp.type;
  a->alias = strdup( tmp.alias );
  a->replacement = strdup( tmp.replacement );
  a->next = GET_ALIASES( ch );
  GET_ALIASES( ch ) = a;
  free( tmp.alias       );
  free( tmp.replacement );
  
  return TRUE;
}

/**
 * save_all is used to save every single player currently in the game
 * (to protect their data against crashes).
 */
void save_all( void )
{
  struct descriptor_data * d;

  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLAYING) && !IS_NPC(d->character)) {
      if (PLR_FLAGGED(d->character, PLR_CRASH)) {
        /* we only save flagged folks this way */
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_CRASH);
	save_char_obj( d->character );
      }
    }
  }
}

/**
 * find_expensive_obj is a helper function to locate the most
 * expensive object on the character (this is what gets sold first
 * when s/he runs out of money for rent).
 *
 * @param ch the character whose inventory we're going to rummage
 * through.
 */
struct obj_data * find_expensive_obj( struct char_data * ch )
{
  struct obj_data * tobj = ch->carrying;
  struct obj_data * max  = ch->carrying;

  for (tobj = ch->carrying; tobj != NULL; tobj = tobj->next_content)
    if (GET_OBJ_RENT(tobj) > GET_OBJ_RENT(max))
      max = tobj;

  return max;
}

/**
 * rent_adjust is responsible for making the character pay for the
 * priviledge of renting.
 *
 * @param ch the character who owes us some money
 */
void rent_adjust( struct char_data * ch )
{
  struct rent_info rent; //  = GET_RENT( ch );
  struct obj_data * obj = NULL;
  int num_of_days = 0, cost = 0, orig_rent_code, j;
  
  if (rent.rentcode == RENT_RENTED || rent.rentcode == RENT_TIMEDOUT) {
    num_of_days = (float) (time(0) - rent.time) / SECS_PER_REAL_DAY;
    cost = (int) (rent.net_cost_per_diem * num_of_days);
    if (cost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
      /* unequip player with low money */
      for (j = 0; j < NUM_WEARS; j++)
        if (GET_EQ(ch,j))
          obj_to_char(unequip_char(ch, j), ch);
      /* start selling items at 1/2 their value */      
      while ( cost > GET_GOLD( ch ) + GET_BANK_GOLD( ch ) ) {
        obj = find_expensive_obj( ch );
        /* check to see if we've sold everything */
        if ( obj == NULL )
          break;
        /* give some gold, take the object */
        GET_GOLD( ch ) += (obj->obj_flags.cost / 2);
        extract_obj( obj );
      }
      sprintf(buf, "%s entering game, rented equipment lost (no $).",
	      GET_NAME(ch));
      mudlog(buf, BRF, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
      save_char( ch, NOWHERE );
    } else {
      GET_BANK_GOLD(ch) -= MAX((long)cost - GET_GOLD(ch), 0L);
      GET_GOLD(ch) = MAX((long)GET_GOLD(ch) - cost, 0L);
      save_char( ch, NOWHERE );
    }
  }

  switch (orig_rent_code = rent.rentcode) {
  case RENT_RENTED:
    sprintf(buf, "%s un-renting and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  case RENT_CRASH:
    sprintf(buf, "%s retrieving crash-saved items and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  case RENT_CRYO:
    sprintf(buf, "%s un-cryo'ing and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  case RENT_FORCED:
  case RENT_TIMEDOUT:
    sprintf(buf, "%s retrieving force-saved items and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  default:
    sprintf(buf, "WARNING: %s entering game with undefined rent code.", GET_NAME(ch));
    mudlog(buf, BRF, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  }
}

/**
 * list_rent is used to list the rent for a character who is renting
 *
 * @param ch the character who's interested in knowing the target's
 * rent cost.
 * @param name the target character's name
 */
void list_rent(struct char_data * ch, char *name)
{
  struct descriptor_data * d;
  struct obj_data        * obj;
  /* create a new descriptor */
  CREATE(d, struct descriptor_data, 1);  

  if ( load_char_obj( d, name ) ) {
    sprintf( buf, "Rented items for %s:\n\r",
             GET_NAME( d->character ) );
    send_to_char( buf, ch );
    buf[0] = '\0';
    obj = d->character->carrying;
    while ( obj != NULL ) {
      sprintf(buf, "%s [%ld] (%5dau) %-20s\r\n", buf,
              obj->item_number,
              GET_OBJ_RENT(obj),
              obj->short_description);
    }
    send_to_char( buf, ch );
    free_char( d->character );
    d->character = NULL;
  } else {
    sprintf( buf, "Unable to find information for %s.\n\r",
             name );
    send_to_char( buf, ch );
  }

  free( d );
}

/*
 * Read a number from a file.
 */
int fread_number_stat( FILE *fp, int *status )
{
    char c;
    bool sign;
    int  number;
    int  stat;

    do
    {
	c = getc( fp );
    }
    while ( isspace( c ) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit( c ) )
    {
        *status = 1;
	log( "Fread_number: bad format." );
	log( "   If bad object, check for missing '~' in value[] fields." );
	return 0;
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number_stat( fp, &stat );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word_stat( FILE *fp, int *status )
{
    static char  word [ MAX_INPUT_LENGTH ];
           char *pword;
           char  cEnd;

    *status = 0;

    do
    {
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace( *pword ) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    log( "Fread_word: word too long." );
    *status = 1;
    return NULL;
}


/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}

/*
 * Take a character data from the free list and clean it out.
 */
struct char_data * new_character( bool player )
{
  // we might need to allocate other structs
  return create_char( );
}
