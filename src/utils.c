/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  UTILS.C                                      Based on CircleMUD *
*  Usage: Various internal functions of a utility nature                  *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All Modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct time_data time_info;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct cha_app_type cha_app[];
extern struct descriptor_data *descriptor_list;
extern struct event_data *event_list;
extern int event_top;
ACMD(do_look);
void death_cry(struct char_data *ch);

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
  /* error checking in case people call number() incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
  }

  return ((random() % (to - from + 1)) + from);
}


/* simulates dice roll */
int dice(int number, int size)
{
  int sum = 0;

  if (size <= 0 || number <= 0)
    return 0;

  while (number-- > 0)
    sum += ((random() % size) + 1);

  return sum;
}

#ifdef __cplusplus
template<class kind> 
kind MIN(kind a, kind b)
{
  return a < b ? a : b;
}

template<class kind> 
kind MAX(kind a, kind b)
{
  return a > b ? a : b;
}
#else
long MIN(long a, long b) { return a < b ? a : b; }
long MAX(long a, long b) { return a > b ? a : b; }
#endif

/* Copy one stirng to another */
char * str_cpy(char *s, const char *t)
{
  while ((*s++ = *t++))
    /* DO NOTHING */;
  return s;
}

/* Create a duplicate of a string */
char *str_dup(const char *source)
{
  char *newp;

  CREATE(newp, char, strlen(source) + 1);
  return (strcpy(newp, source));
}

/* str_cmp: a case-insensitive version of strcmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(const char *arg1, const char *arg2)
{
  int chk, i;

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++) {
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
      if (chk < 0) {
	      return (-1);
      } else {
	      return (1);
      }
    }
  }
  return (0);
}


/* strn_cmp: a case-insensitive version of strncmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(const char *arg1, const char *arg2, size_t n)
{
  int chk, i;

  for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--) {
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
      if (chk < 0) {
	      return (-1);
      } else {
	      return (1);
      }
    }
  }
  return (0);
}


/* log a death trap hit */
void log_death_trap(struct char_data * ch)
{
  char buf[150];
  extern struct room_data *world;

  sprintf(buf, "%s hit death trap #%ld (%s)", GET_NAME(ch),
	  world[ch->in_room].number, world[ch->in_room].name);
  mudlog(buf, BRF, LVL_IMMORT, TRUE);
}


/* writes a string to the log */
void mud_log(char *str)
{
  time_t ct;
  char *tmstr;

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stderr, "%-19.19s :: %s\n", tmstr, str);
}


/* the "touch" command, essentially. */
int touch(const char *path)
{
  FILE *fl;

  if (!(fl = fopen(path, "a"))) {
    perror(path);
    return -1;
  } else {
    fclose(fl);
    return 0;
  }
}


/*
 * mudlog -- log mud messages to a file & to online imm's syslogs
 * based on syslog by Fen Jul 3, 1992
 */
void mudlog(char *str, char type, int level, byte file)
{
  char buf[MAX_STRING_LENGTH];
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *i;
  char *tmp, tp;
  time_t ct;

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (file)
    fprintf(stderr, "%-19.19s :: %s\n", tmp, str);
  if (level < 0)
    return;

  sprintf(buf, "[ %s ]\r\n", str);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && !PLR_FLAGGED(i->character, PLR_WRITING)) {
      tp = ((PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) +
	    (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0));

      if ((GET_LEVEL(i->character) >= level) && (tp >= type)) {
	send_to_char(CCGRN(i->character, C_NRM), i->character);
	send_to_char(buf, i->character);
	send_to_char(CCNRM(i->character, C_NRM), i->character);
      }
    }
}



void sprintbit(long vektor, char *names[], char *result)
{
  long nr;

  *result = '\0';

  if (vektor < 0) {
    strcpy(result, "SPRINTBIT ERROR!");
    return;
  }
  for (nr = 0; vektor; vektor >>= 1) {
    if (IS_SET(1, vektor)) {
      if (*names[nr] != '\n') {
	strcat(result, names[nr]);
	strcat(result, " ");
      } else
	strcat(result, "UNDEFINED ");
    }
    if (*names[nr] != '\n')
      nr++;
  }

  if (!*result)
    strcat(result, "NOBITS ");
}



void sprinttype(int type, char *names[], char *result)
{
  int nr;

  if (type == -1) {
    strcpy(result, "UNDEFINED");
    return;
  }

  for (nr = 0; (*names[nr] != '\n'); nr++);
  if (type < nr)
    strcpy(result, names[type]);
  else
    strcpy(result, "UNDEFINED");
}

int remove_event(struct char_data *ch)
{
  struct event_data *eventlst = event_list;
  struct event_data *temp;

  while (eventlst != NULL) {
    if (eventlst->ch == ch) {
      REMOVE_FROM_LIST(eventlst, event_list, next);
      --event_top;
      return 1;
    }
    eventlst = eventlst->next;
  }
  return 0;
}

int add_event(struct char_data *ch, char *args, int cmd, int count)
{
  struct event_data *event;
  struct event_data *eventlst = event_list;

  event = (struct event_data *) malloc(sizeof(struct event_data));
  event->ch   = ch;
  event->args = str_dup(args);
  event->cmd  = cmd;
  event->count= count;
  event->next = NULL;

  WAIT_STATE(ch, (PULSE_VIOLENCE * count/2));

  if (eventlst == NULL) {
    eventlst = event;
    event_top++;
    return 1;
  } else if (eventlst != NULL) 
    while (eventlst->next != NULL) 
      eventlst = eventlst->next;

  if (eventlst->next == NULL) {
    eventlst->next = event;
    ++event_top;
    return 1;
  }
  return 0;
}

/* This function handles actions that have been delayed */
void event_activity(void)
{
  struct event_data *event, *temp;
  int i, j = 0;

  for (event = event_list, i = 0; i < event_top && event != NULL; ++i) {
     (event->count)--;
     strcpy(buf2, "Working: ");
     for (j = 0; j < event->count; j++)
	strcat(buf2, "#");
     strcat(buf2, "\r\n");
     send_to_char(buf2, event->ch);
     if (event->count < 1) {
       ((*cmd_info[event->cmd].command_pointer) (event->ch, event->args, event->cmd, SCMD_RESUME));
       REMOVE_FROM_LIST(event, event_list, next);
       --event_top;
     }
     event = event->next;
  }
}


/*
 * Check if room should tele chars to new room done by Glasgian
 * of The Dominion based on SillyMUD's code.
 */
void perform_teleport_pulse(void)
{
   register struct descriptor_data *d;
   struct   descriptor_data *next_d;
   struct   char_data *tmp, *temp_char;
   struct   obj_data *obj_object, *temp_obj;
   extern   struct room_data *world;
   room_num dest, start;

   /* Go thru all of the descriptors */
   for (d = descriptor_list; d; d = next_d) {
     next_d = d->next;
     /* Only those in the playing state should be moved */
     if (STATE(d) != CON_PLAYING)
       continue;
     /* Make sure we have a valid destination */
     if ((world[d->character->in_room].tele_targ <= 0) ||
	 (world[d->character->in_room].tele_targ ==
	  world[d->character->in_room].number))
       continue;
     else
       dest = real_room(world[d->character->in_room].tele_targ);
     /* See if it's time to teleport yet */
     if (world[d->character->in_room].tele_time > world[d->character->in_room].tele_cnt) {
       /* It's not time yet, increment the counter. */
       world[d->character->in_room].tele_cnt++;
       continue;
     } else {
       start = d->character->in_room;
       /* Destination doesn't exist - teleporting for this room disabled */
       if (dest <= 0) {
	 sprintf(buf, "Room V# %ld, has invalid tele_targ.\n",
		       world[d->character->in_room].number);
	 log(buf);
	 continue;
       }
       /* Well, we have a destination, let's start moving them. */
       for (tmp = world[d->character->in_room].people; tmp; tmp = temp_char) {
	 temp_char = tmp->next_in_room;

	 if (!tmp) break;
	 /* move the character to the next room */
	 char_from_room(tmp);
	 char_to_room(tmp, dest);

	 /* Make the player look if the bit is set */
	 if (IS_SET(TELE_LOOK, world[start].tele_mask) && !IS_NPC(tmp))
	   do_look(tmp, "", 0, 0);
	 /* Is the destination a death trap? */
	 if (IS_SET(ROOM_FLAGS(dest), ROOM_DEATH) && (!IS_IMMORT(tmp))) {
	   log_death_trap(tmp);
	   death_cry(tmp);
	   extract_char(tmp);
	 }
	 world[start].tele_cnt = 0;   /* reset it for next count */
	 /* If the random bit is set, randomize the tele_time */
	 if (IS_SET(TELE_RANDOM, world[start].tele_mask))
	   world[start].tele_time = (number(1,10)*number(10, 100));
       }
       /* Now move all the objects as well. */
       for (obj_object = world[d->character->in_room].contents; obj_object; obj_object = temp_obj) {
	 temp_obj = obj_object->next;
	 obj_from_room(obj_object);
	 obj_to_room(obj_object, dest);
       }
     }
     /* End of descriptor loop */
   }
}


/*
 * Check if river is to sweep chars to next room done
 * by Glasgian of The Dominion based on SillyMUD's code.
 */
void perform_current_sweep(void)
{
  register struct descriptor_data *d;
  struct descriptor_data *next_d;
  struct char_data *tmp, *temp_char;
  extern struct room_data *world;
  struct obj_data *obj_object, *temp_obj;
  room_num dest, start;
  int rivdir = 0;

  const char *directions[] = {
     "North",
     "East",
     "South",
     "West",
     "Up",
     "Down",
     "Northeast",
     "Northwest",
     "Southeast",
     "Southwest"
  };
  /* Only check in rooms with players ... saves time and energy! */
  for (d = descriptor_list; d; d = next_d) {
    next_d = d->next;
    /* Check this they are in a room which is swept with currents */
    if (STATE(d) != CON_PLAYING)
      continue;
    if ((((world[d->character->in_room].sector_type != SECT_WATER_NOSWIM) &&
	(world[d->character->in_room].sector_type != SECT_UNDERWATER)) &&
	 (world[d->character->in_room].sector_type != SECT_FLYING)))
      continue;
    else if ((world[d->character->in_room].sector_type == SECT_WATER_NOSWIM) ||
	      (world[d->character->in_room].sector_type == SECT_UNDERWATER) ||
	      (world[d->character->in_room].sector_type == SECT_FLYING)) {
      if (((world[d->character->in_room].river_speed > 0) &&
	  ((world[d->character->in_room].river_dir <= 5) &&
	   (world[d->character->in_room].river_dir >= 0)))) {
	rivdir = world[d->character->in_room].river_dir;
	start  = d->character->in_room;
	dest   = real_room(world[d->character->in_room].dir_option[rivdir]->to_room);
	if (dest <= 0) {
	  sprintf(buf, "Room V# %ld, has invalid river_dir.\n",
		  world[d->character->in_room].number);
	  log(buf);
	  continue;
	} else {
	  if (world[d->character->in_room].river_speed >= world[d->character->in_room].tele_cnt) {
	    /* It's safe to sweep everything down river */
	    for (obj_object = world[d->character->in_room].contents; obj_object; obj_object = temp_obj) {
	      temp_obj = obj_object->next;
	      obj_from_room(obj_object);
	      obj_to_room(obj_object, dest);
	    }
	    /* Go through all the people in the room and move them. */
	    for (tmp = world[d->character->in_room].people; tmp; tmp = temp_char) {
	      temp_char = tmp->next_in_room;
	      if (!tmp) break;
	      /* Immortals aren't swept away*/
	      if (IS_IMMORTAL(tmp) && IS_SET(PRF_FLAGS(tmp), PRF_NOHASSLE)) {
		send_to_char("The waters swirl and eddy about you.\r\n", tmp);
		continue;
	      } else if (((IS_AFFECTED2(tmp, AFF_FLYING) ||
			  GET_POS(tmp) == POS_FLYING)   ||
			  IS_ITEM_AFF(tmp, AFFECTED_FLYING)) ||
			  ((IS_AFFECTED2(MOUNTED(tmp), AFF_FLYING) ||
			   GET_POS(MOUNTED(tmp)) == POS_FLYING) ||
			   IS_ITEM_AFF(MOUNTED(tmp), AFFECTED_FLYING))) {
		send_to_char("The waters swirl and eddy just below your feet.\r\n", tmp);
		continue;
	      } else {
		/* stop any battles */
		if (FIGHTING(tmp))
		  stop_fighting(tmp);
		sprintf(buf, "The strong currents of the river force you %s ...\r\n", directions[rivdir]);
		send_to_char(buf, tmp);
		char_from_room(tmp);
		char_to_room(tmp, dest);
		do_look(tmp, "", 0, 0);
		/* Check for a DT */
		if (IS_SET(ROOM_FLAGS(dest), ROOM_DEATH) && (!IS_IMMORT(tmp))) {
		  log_death_trap(tmp);
		  death_cry(tmp);
		  extract_char(tmp);
		}
	      }
	      world[start].tele_cnt = 10;   /* reset it for next count */
	    }
	  } else
	    /* start the count down */
	    world[start].tele_cnt--;
	}
      /* This isthe code for wind currents */
      } else if (world[d->character->in_room].sector_type == SECT_FLYING) {
	 if (((world[d->character->in_room].river_speed > 0) &&
	     ((world[d->character->in_room].river_dir <= 5) &&
	      (world[d->character->in_room].river_dir >= 0)))) {
	   rivdir = world[d->character->in_room].river_dir;
	   start  = d->character->in_room;
	   dest   = real_room(world[d->character->in_room].dir_option[rivdir]->to_room);
	   if (dest <= 0) {
	     sprintf(buf, "Room V# %ld, has invalid wind_current_dir.\n",
		     world[d->character->in_room].number);
	     log(buf);
	     continue;
	   } else {
	     if (world[d->character->in_room].river_speed >= world[d->character->in_room].tele_cnt) {
	       /* It's safe to move everything thru the air */
	       for (obj_object = world[d->character->in_room].contents; obj_object; obj_object = temp_obj) {
		 temp_obj = obj_object->next;
		 obj_from_room(obj_object);
		 obj_to_room(obj_object, dest);
	       }
	       /* Go through all the people in the room and move them. */
	       for (tmp = world[d->character->in_room].people; tmp; tmp = temp_char) {
		 temp_char = tmp->next_in_room;
		 if (!tmp) break;
		 /* Don't sweep immortals */
		 if (IS_IMMORTAL(tmp) && IS_SET(PRF_FLAGS(tmp), PRF_NOHASSLE)) {
		   send_to_char("The winds rise up around you and buffer you.\r\n", tmp);
		   continue;
		 } else {
		   /* stop any battles */
		   if (FIGHTING(tmp))
		     stop_fighting(tmp);
		   sprintf(buf, "The strong winds force you %s ...\r\n", directions[rivdir]);
		   send_to_char(buf, tmp);
		   char_from_room(tmp);
		   char_to_room(tmp, dest);
		   do_look(tmp, "", 0, 0);
		   /* Check for DT's */
		   if (IS_SET(ROOM_FLAGS(dest), ROOM_DEATH) && (!IS_IMMORT(tmp))) {
		     log_death_trap(tmp);
		     death_cry(tmp);
		     extract_char(tmp);
		   }
		 }
		 world[start].tele_cnt = 10;   /* reset it for next count */
	       }
	     } else
	       /* start the count down */
	       world[start].tele_cnt--;
	   }
	 }
      }
    }
  }
}



/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.seconds = (secs / SECS_PER_REAL_SEC) % 60;
  secs -= SECS_PER_REAL_SEC * now.seconds;

  now.minutes = (secs / SECS_PER_REAL_MIN) % 60;
  secs -= SECS_PER_REAL_MIN * now.minutes;

  now.hours = (secs / SECS_PER_REAL_HOUR) % 24; /* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;

  now.day = (secs / SECS_PER_REAL_DAY); /* 0..34 days  */
  secs -= SECS_PER_REAL_DAY * now.day;

  now.month = -1;
  now.year = -1;

  return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.seconds = (secs / SECS_PER_REAL_SEC) % 60;
  secs -= SECS_PER_REAL_SEC * now.seconds;

  now.minutes = (secs / SECS_PER_REAL_MIN) % 60;
  secs -= SECS_PER_REAL_MIN * now.minutes;

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35;     /* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR);        /* 0..XX? years */

  return now;
}


struct time_info_data age(struct char_data * ch)
{
  struct time_info_data player_age;

  player_age = mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += GET_AGE_MOD(ch);

  return player_age;
}


/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data * ch, struct char_data * victim)
{
  struct char_data *k;

  for (k = victim; k; k = k->master) {
    if (k == ch)
      return TRUE;
  }

  return FALSE;
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data * ch)
{
  struct follow_type *j, *k;

  assert(ch->master);

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM, FALSE))
      affect_from_char(ch, SPELL_CHARM, FALSE);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", TRUE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", TRUE, ch, 0, ch->master, TO_VICT);
  }

  if (ch->master->followers->follower == ch) {  /* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
    if (IS_NPC(ch))
       GET_FOLS(ch->master)--;
  } else {                      /* locate follower who is not head of list */
    for (k = ch->master->followers; k->next->follower != ch; k = k->next);

    j = k->next;
    k->next = j->next;
    free(j);
  }

  ch->master = NULL;
  REMOVE_BIT(AFF_FLAGS(ch), AFF_CHARM);
  REMOVE_BIT(AFF_FLAGS2(ch), AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data * ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k = ch->followers; k; k = j) {
    j = k->next;
    stop_follower(k->follower);
  }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data * ch, struct char_data * leader)
{
  struct follow_type *k;

  assert(!ch->master);

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  if (IS_NPC(ch))
     GET_FOLS(leader)++;

  if (GET_FOLS(leader) > cha_app[(int) GET_CHA(leader)].num_fols)
     stop_follower(ch);

  if (RIDDEN(ch))
     return;
  else if (SHADOWING(ch))
     return;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (CAN_SEE(leader, ch))
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n starts to follow $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file.
 */
int get_line(FILE * fl, char *buf)
{
  char temp[256];
  int lines = 0;

  do {
    lines++;
    fgets(temp, 256, fl);
    if (*temp)
      temp[strlen(temp) - 1] = '\0';
  } while (!feof(fl) && (*temp == '*' || !*temp));

  if (feof(fl))
    return 0;
  else {
    strcpy(buf, temp);
    return lines;
  }
}


int get_filename(char *orig_name, char *filename, int mode)
{
  char *prefix, *middle, *suffix, *ptr, name[64];

  switch (mode) {
  case CRASH_FILE:
    prefix = "plrobjs";
    suffix = "objs";
    break;
  case ETEXT_FILE:
    prefix = "plrtext";
    suffix = "text";
    break;
  case ALIAS_FILE :
    prefix = "plralias";
    suffix = "alias";
    break;
  default:
    return 0;
    break;
  }

  if (!*orig_name)
    return 0;

  strncpy(name, orig_name, sizeof(name));
  for (ptr = name; *ptr; ptr++)
    *ptr = LOWER(*ptr);

  switch (LOWER(*name)) {
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    middle = "A-E";
    break;
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    middle = "F-J";
    break;
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    middle = "K-O";
    break;
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
    middle = "P-T";
    break;
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    middle = "U-Z";
    break;
  default:
    middle = "ZZZ";
    break;
  }

  sprintf(filename, "%s/%s/%s.%s", prefix, middle, name, suffix);
  return 1;
}


int num_pc_in_room(struct room_data *room)
{
  int i = 0;
  struct char_data *ch;

  for (ch = room->people; ch != NULL; ch = ch->next_in_room)
    if (!IS_NPC(ch))
      i++;

  return i;
}

/* Main part of color code - Written by Glasgian of TD */
void parse_color_and_send(struct char_data *ch, unsigned char *src)
{
   register char *i = NULL, *buf;
   static char lbuf[MAX_STRING_LENGTH];

   buf = lbuf;

   if (ch == NULL || !ch->desc)
     return;

   for (;;) {
     if (*src == '%') {
       switch (*(++src)) {
	 case '%':  /* Print a % sign */
	   i = "%";
	   break;
	 case 'b':  /* BOLD */
	 case 'B':
	   i = MOD_BOLD;
	   break;
	 case 'i':  /* BLINK */
	 case 'I':
	   i = MOD_BLINK;
	   break;
	 case 'n':  /* NORMAL */
	 case 'N':
	   i = MOD_NORMAL;
	   break;
	 case 'f':
	 case 'F':
	   i = MOD_FAINT;
	   break;
	 case 'r':
	 case 'R':
	   i = MOD_REVERSE;
	   break;
	 case 'u':
	 case 'U':
	   i = MOD_UNDERLINE;
	   break;
	 case '0':
	   i = FG_BLACK;
	   break;
	 case '1':
	   i = FG_RED;
	   break;
	 case '2':
	   i = FG_GREEN;
	   break;
	 case '3':
	   i = FG_BROWN;
	   break;
	 case '4':
	   i = FG_BLUE;
	   break;
	 case '5':
	   i = FG_MAGENTA;
	   break;
	 case '6':
	   i = FG_CYAN;
	   break;
	 case '7':
	   i = FG_LT_GRAY;
	   break;
	 case '8':
	   i = BK_BLACK;
	   break;
	 case '9':
	   i = BK_RED;
	   break;
	 case 'z':
	   i = BK_GREEN;
	   break;
	 case 'y':
	   i = BK_BROWN;
	   break;
	 case 'x':
	   i = BK_BLUE;
	   break;
	 case 'w':
	   i = BK_MAGENTA;
	   break;
	 case 'v':
	   i = BK_CYAN;
	   break;
	 case 't':
	   i = BK_LT_GRAY;
	   break;
	 default:
	   break;
       }
       if (*i != '%' && !PRF_FLAGGED(ch, PRF_COLOR_1))
	 *i = '\0';
       while ((*buf = *(i++)))
	 buf++;
       src++;
     } else if (!(*(buf++) = *(src++)))
       break;
   }
   *(--buf) = '\r';
   *(++buf) = '\n';
   *(++buf) = '\0';

   if (ch->desc != NULL)
     SEND_TO_Q(CAP(lbuf), ch->desc);
}
