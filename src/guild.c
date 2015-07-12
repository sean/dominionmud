/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  GUILD.C                                      Based on CircleMUD *
*  Usage: Source file for guild specific code                             *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

#define MAX_GUILD_LVL 5
#define GUILD_ACC_LVL 4
#define GUILD_DIS_LVL 4

/* protos */
SPECIAL(guild_guard);
void ASSIGNMOB(long mob, SPECIAL(fname));
long asciiflag_conv(char *);
extern FILE *player_fl;
extern int    top_of_p_table;
extern struct player_index_element *player_table;
extern struct descriptor_data *descriptor_list;

/* Guild globals */
struct guild_type *guild_info = NULL;    /* queue of guilds  */
unsigned int num_of_guilds = 0;          /* number of guilds */

void show_guilds( struct char_data * ch )
{
  struct guild_type * gptr = NULL;
  int i = 0;
  
  for (gptr = guild_info; gptr; gptr = gptr->next, i++) {
    sprintf(buf, "[%2d] %20.20s : %30.30s\r\n",
            i, gptr->name,
            (gptr->desc != NULL ? gptr->desc : "No Description Available"));
    send_to_char(buf, ch);
  }
}

void free_guild(struct guild_type *g)
{
  int i;
  assert( g != NULL );
  
  free(g->name);
  free(g->xlvlname);
  free(g->hlvlname);
  free(g->mlvlname);
  free(g->blvlname);
  free(g->leadersname);
  for (i = 0; i < MAX_PETITIONERS; i++)
    free(g->petitioners[i]);
  free(g);
}

struct guild_type *enqueue_guild(void)
{
  struct guild_type * gptr = NULL;

  if ((gptr = (struct guild_type *) malloc(sizeof(struct guild_type))) == NULL) {
    fprintf(stderr, "SYSERR: Out of memory when creating guild!");
    exit(1);
  } else
    gptr->next = NULL;

  /* This is the first guild loaded if true */
  if ( guild_info == NULL )
    guild_info = gptr;
  else {
    // insert at the end of the list
    struct guild_type * tmp;
    for (tmp = guild_info; tmp->next != NULL; tmp = tmp->next)
      /* Loop does the work */;
    tmp->next = gptr;
  }
  return gptr;
}

void dequeue_guild(int guildnum)
{
  struct guild_type *gptr = NULL, *temp = NULL;

  assert( guildnum >= 0 && guildnum < num_of_guilds );

  for ( gptr = guild_info;
        gptr->number != guildnum;
        temp = gptr, gptr = gptr->next )
    /* loop does all the work */;

  /* check if the first one is the one being removed */
  if ( gptr == guild_info )
    guild_info = gptr->next;
  else
    temp->next = gptr->next;

  free_guild( gptr );
  num_of_guilds--;
}

/* Loads the guilds from the text file */
void load_guilds(void)
{
  FILE   *guild_file = NULL;
  int    guildnum = 0, line_num = 0, tmp, i = 0;
  long   tmp2, tmp3;
  char   name[80], flags[80];
  char   *ptr;
  struct guild_type *gptr = NULL;
  /* protos */
  int count_hash_records(FILE *);
  
  if ((guild_file = fopen(GUILD_FILE, "rt")) == NULL) {
    fprintf(stderr, "SYSERR: Unable to open guild file!");
    exit(0);
  }

  /* count the number of guilds */
  guildnum = count_hash_records( guild_file );

  /* rewind to the beginning of the file */
  if ( fseek( guild_file, 0, SEEK_SET ) != 0 ) {
    fprintf( stderr, "Erorr rewinding %s\n", GUILD_FILE );
    exit(0);
  }
  
  /* Setup the global total number of guilds */
  num_of_guilds = guildnum;

  /* process each guild in order */
  for (guildnum = 0; guildnum < num_of_guilds; guildnum++) {
    /* Get the info for the individual guilds */
    line_num += get_line(guild_file, buf);
    if (sscanf(buf, "#%d", &tmp) != 1) {
      fprintf(stderr, "Format error in guild (No Unique GID), line %d\n", line_num);
      exit(0);
    }
    /* create some guild shaped memory space */
    if ((gptr = enqueue_guild()) != NULL) {
      gptr->number = tmp;
      /* Now get the name of the guild */
      line_num += get_line(guild_file, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->name = str_dup(buf);
      /* Now get the leader's title */
      line_num += get_line(guild_file, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->xlvlname = str_dup(buf);
      /* Now get the high lvl's title */
      line_num += get_line(guild_file, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->hlvlname = str_dup(buf);
      /* Now get the Med Lvl's title */
      line_num += get_line(guild_file, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->mlvlname = str_dup(buf);
      /* Now get the newbie's title */
      line_num += get_line(guild_file, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
        *ptr = '\0';
      gptr->blvlname = str_dup(buf);
      /* Now get the look member string */
      line_num += get_line(guild_file, buf);
      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
	*ptr = '\0';
      gptr->member_look_str = str_dup(buf);
      /* Now get restricts, entrance room and direction */
      line_num += get_line(guild_file, buf);
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
      line_num += get_line(guild_file, buf);
      /* Loop to get Members' Names */
      for (;;) {
        line_num += get_line(guild_file, buf);
        if (*buf == ';')
	  break;
        sscanf(buf, "%s %d", name, &tmp);
        if (tmp == MAX_GUILD_LVL)
 	  gptr->leadersname = str_dup(name);
      }
      /* Okay we have the leader's name ... now for the petitioners */
      for (i = 0; i < MAX_PETITIONERS; i++) {
        line_num += get_line(guild_file, buf);
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
  fclose(guild_file);
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
  fprintf(gfile, "%d\n", num_of_guilds);
  /* Save each guild */
  while (guildnum < num_of_guilds && gptr != NULL) {
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
     send_to_char("Sorry, but gods cannot join guilds -- it would upset the balance of power!\r\n",  ch);
     return;
   }
   if (GET_GUILD(ch) != GUILD_NONE) {
     send_to_char("If you'd like to leave your guild speak with your leader.\r\n", ch);
     return;
   }

   if (!*argument) {
     send_to_char("Which guild do you want to petition for membership?\r\n", ch);
     show_guilds( ch );
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
     send_to_char("There is no guild by that name, please type the WHOLE name.\r\n",
                  ch);
   else {
     for (i = 0; i < MAX_PETITIONERS; i++) {
       if ( gptr->petitioners[i] != NULL &&
            !strcmp(GET_NAME(ch), gptr->petitioners[i]) ) {
         send_to_char("You are already petitioning this guild, please be patient.\r\n", ch);
         return;
       } else if (gptr->petitioners[i] == NULL)
	 break;
     }
     
     if ((gptr->petitioners[i] == NULL) && (i < MAX_PETITIONERS)) {
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
