/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  RELIGION.C                                   Based on CircleMUD *
*  Usage: Source file for religion specific code                          *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/* protos */
extern int    top_of_p_table;
extern struct player_index_element *player_table;
extern struct descriptor_data *descriptor_list;
void LowerString(char *s);

struct religion_data * religions;
int NUM_RELIGIONS = 0;

int parse_alignment( char cval );

/**
 */
void parse_religions( void )
{
  int  index = 0, c, line = 1;
  char cval, gval, *p;
  long lval;
  FILE * fl = 0;

  if ( !(fl = fopen( RELIGION_FILE, "r" )) ) {
    plog( "parse_religions: unable to open religion file (%s).\n",
          RELIGION_FILE );
    exit( 1 );
  }

  /* reset the number of religions */
  NUM_RELIGIONS = 0;
  // first read through the file and count the number of races.
  while ( !feof( fl ) ) {
    c = getc( fl );

    if ( c == '#' ) {
      c = getc( fl );
      if ( isalpha( c ) )
        NUM_RELIGIONS++;
    }
    fread_to_eol( fl );
  }

  plog( "Loading religions (%d) ...", NUM_RELIGIONS );

  CREATE( religions, struct religion_data, NUM_RELIGIONS );
  // reset the file pointer
  fseek( fl, 0L, SEEK_SET );

  while ( !feof( fl ) ) {
    // parse one religion
    c = getc( fl );

    if ( c == '#' ) {
      c = getc( fl );
      if ( !isspace( c ) && isalpha( c ) ) {
        ungetc( c, fl );
        fgets( buf2, 128, fl );
        // get rid of newline character
        if ( (p = strstr( buf2, "\n" )) != NULL )
          buf2[ strlen( buf2 ) - strlen( p ) ] = '\0';
        religions[ index ].name   = strdup( buf2 );
        line++;
        religions[ index ].abbrev = fread_string( fl, buf2 );
        line++;
        religions[ index ].brief  = fread_string( fl, buf2 );
        line++;
        // for the long statement, which may span multiple lines, we
        // know it's over when we find the ~
        buf2[0] = buf[0] = '\0';
        while ( strstr( buf2, "~" ) == NULL) {
          fgets( buf2, 128, fl );
          line++;
          // remove any embedded comments
          if ( (p = strstr( buf2, "#" )) != NULL ) {
            assert( buf2[ p - buf2 ] == '#' );
            buf2[ p - buf2 ] = '\0';
          }
          remove_trailing_spaces( buf2 );
          strcat( buf, buf2 );
        }
        assert( buf[ strlen( buf ) - 1 ] == '~' );
        buf[ strlen( buf ) - 1 ] = '\0';
        religions[ index ].long_desc = strdup( buf );
        religions[ index ].hint   = fread_string( fl, buf2 );
        line++;
        fscanf( fl, "%c %c %ld", &gval, &cval, &lval );
        religions[ index ].sex    = UPPER(gval) == 'M' ? SEX_MALE :
          UPPER(gval) == 'F' ? SEX_FEMALE : SEX_NEUTRAL;
        religions[ index ].align  = parse_alignment( cval );
        religions[ index ].temple = lval;
        line++;

        index++;
      } else { /* comment */
        fread_to_eol( fl ); line++;
      }
    } else { /* blank line */
      fread_to_eol( fl ); line++;
    }
  }

  if ( index < NUM_RELIGIONS ) {
    plog( "parse_religions: failed to parse all religions! (%d/%d)\n",
          index, NUM_RELIGIONS );
    exit( 1 );
  }
}

int parse_alignment( char cval )
{
  int align;
  
  switch ( UPPER(cval) ) {
    case 'G':
      align = ALIGN_GOOD;
      break;
    case 'N':
      align = ALIGN_NEUTRAL;
      break;
    case 'E':
      align = ALIGN_EVIL;
      break;
    case 'L':
      align = ALIGN_LAWFUL;
      break;
    case 'C':
      align = ALIGN_CHAOTIC;
      break;
    case 'U':
    default:
      align = ALIGN_UNDEFINED;
      break;
  }
  return align;
}

const char *relg_name(struct char_data  *ch)
{
  if (!( GET_REL(ch) >= 0 && GET_REL(ch) <= NUM_RELIGIONS ))
    GET_REL( ch ) = 0;

  return religions[ GET_REL(ch) ].name;
}

const char *align_to_name( enum aligns alignment )
{
  const char * align_names [] = {
    "Good",
    "Neutral",
    "Evil",
    "Lawful",
    "Chaotic"
  };

  if ( alignment != ALIGN_UNDEFINED )
    return align_names[ alignment ];
  return "Undefined";
}

bool align_out_of_sync( struct char_data * ch )
{
  return ( (GET_PERMALIGN(ch) == ALIGN_EVIL    && !IS_EVIL(ch)) ||
           (GET_PERMALIGN(ch) == ALIGN_GOOD    && !IS_GOOD(ch)) ||
           (GET_PERMALIGN(ch) == ALIGN_NEUTRAL && !IS_NEUTRAL(ch)) ||
           (GET_PERMALIGN(ch) == ALIGN_LAWFUL  && !IS_LAWFUL(ch)) ||
           (GET_PERMALIGN(ch) == ALIGN_CHAOTIC && !IS_CHAOTIC(ch)) );
}

bool religion_out_of_sync( struct char_data * ch )
{
  return ( (religions[ GET_REL(ch) ].align == ALIGN_EVIL    && !IS_EVIL(ch)) ||
           (religions[ GET_REL(ch) ].align == ALIGN_GOOD    && !IS_GOOD(ch)) ||
           (religions[ GET_REL(ch) ].align == ALIGN_NEUTRAL && !IS_NEUTRAL(ch)) ||
           (religions[ GET_REL(ch) ].align == ALIGN_LAWFUL  && !IS_LAWFUL(ch)) ||
           (religions[ GET_REL(ch) ].align == ALIGN_CHAOTIC && !IS_CHAOTIC(ch)) );
}

/* general function for players to toggle their religion */
ACMD(do_tog_rel)
{
   struct affected_type af;
   int    i = 0;
   bool   found = FALSE;
   char   arg1[80];

   one_argument(argument, arg1);

   if (!*arg1) {
     send_to_char("Available religions are:  (Align Req'd)\r\n", ch);
     for ( i = 1; i < NUM_RELIGIONS; i++ ) {
       sprintf( buf, "%12s -- %45s (%s)\r\n",
                religions[ i ].name, religions[ i ].brief,
                align_to_name( religions[ i ].align ) );
       send_to_char( buf, ch );
     }
     return;
   }

   if ((subcmd == SCMD_AFFIRM) && HAS_RELIGION(ch)) {
     send_to_char("Nice try, but you have to renounce your current religion to affirm another one!\r\n",ch);
     return;
   } else if ((subcmd == SCMD_RENOUNCE) && !HAS_RELIGION(ch)) {
     send_to_char("Why would you want to renounce a god that you don't even believe in?\r\n",ch);
     return;
   } else if ((subcmd == SCMD_AFFIRM) && IS_AFFECTED2(ch, AFF_RELIGED)) {
     send_to_char("You cannot affirm a religion at this time.\r\n",ch);
     return;
   }
   LowerString(arg1);

   for ( i = 0; i < NUM_RELIGIONS; i++ ) {
     if (!str_cmp( arg1, religions[ i ].name ) ||
         is_abbrev( arg1, religions[ i ].name ) ) {
       found = TRUE;
       break;
     }
   }
   /* check to make sure we found a god */
   if ( !found ) {
     send_to_char("What god was that?\r\n[Use 'affirm' or 'renounce' with no arguments to see available religions.]\n\r", ch);
     return;
   }
   /* okay we have a god's name */
   if (subcmd == SCMD_RENOUNCE) {
     if (GET_REL(ch) != i)
       send_to_char("Why do you want to renouce a god you don't even believe in?\r\n", ch);
     else {
       sprintf(buf, "You renounce %s and all the teachings of %s religion!\r\n",
               religions[ i ].name, HISHER( religions[ i ].sex ) );
       send_to_char(buf, ch);
       GET_REL(ch)  = 0;                    /* None                 */
       af.location  = APPLY_WIS;
       af.modifier  = -4;                   /* Make wisdom worse    */
       af.duration  = 12;                   /* ... for twelve hours */
       af.bitvector = AFF_BLIND;            /* and blind them       */
       affect_to_char(ch, &af, TRUE);
       af.duration  = 12;
       af.bitvector = AFF_RELIGED;
       affect_to_char(ch, &af, TRUE);
     }
   } else if (subcmd == SCMD_AFFIRM) {
     if ( religions[ i ].align != ALIGN_UNDEFINED ) {
       if (GET_PERMALIGN(ch) != religions[ i ].align) {
	 sprintf(buf, "Your soul is not in accordance with the teachings of %s's religion.\r\n",
                 religions[ i ].name );
	 send_to_char(buf, ch);
	 return;
       }
     }
     GET_REL(ch) = i;
     sprintf(buf, "You are now a member of %s's religion.\r\n",
             religions[ i ].name );
     send_to_char(buf, ch);
     if ( religions[ i ].hint != NULL )
       send_to_char( religions[ i ].hint, ch);
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
     if (!HAS_RELIGION(ch))
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
