/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ROUTINE.C                                                       *
*  Usage: Daily routines for NPCs, read from lib/data/routines            *
*  Programmer(s): Written for The Dominion, November 2026                 *
\*********************************************************************** */

/*
 * A routine is a timetable for one mobile (every copy of it): where it
 * should be at each hour of the day, what it says or does on the hour,
 * when it arrives somewhere or leaves somewhere, and a few idle lines.
 * The file format is described at the top of lib/data/routines.
 *
 * How it runs:
 *   routines_hour()   is called by another_hour() (weather.c) every mud
 *                     hour.  It wakes mobiles whose timetable has moved on
 *                     and fires the lines timed for that hour.
 *   routine_pulse()   is called by mobile_activity() (mobact.c) every
 *                     PULSE_MOBILE for each awake mobile with a routine.
 *                     It walks the mobile one room toward where it should
 *                     be (opening doors on the way), settles it there
 *                     (sit, rest or sleep) or says an idle line.
 *
 * Weather: in a downpour (pouring rain, a thunderstorm or a tornado, a
 * snowstorm or a blizzard) a mobile whose timetable would put it out of
 * doors waits indoors instead, or runs for the nearest cover if it is out
 * in it; one indoors doesn't set off through the open until it eases,
 * unless it is going home to bed.  Wandering mobiles in a town (a zone
 * where some routine goes) don't step out into it either.
 *
 * Nothing is saved: a mobile's place in its routine is worked out from
 * the time of day, so a mobile that is killed, purged or loaded by a zone
 * reset simply walks to wherever it should be.  Shopkeepers who leave
 * their shop put up a "closed" sign (object ROUTINE_SIGN_VNUM) and take
 * it down again when they come back.
 */

#define __ROUTINE_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct room_data *world;
extern struct char_data *character_list;
extern struct char_data *mob_proto;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern const char *dirs[];
extern struct time_info_data time_info;
extern long top_of_world;
extern long top_of_mobt;
extern long top_of_zone_table;
extern const int rev_dir[];

ACMD(do_say);
ACMD(do_gen_door);
long shop_room_of_keeper(struct char_data *keeper);

#define ROUTINE_FILE        "data/routines"
#define ROUTINE_SIGN_VNUM   32          /* "a CLOSED sign", obj/0.obj */
#define ROUTINE_MAX_STUCK   6           /* pulses before giving up */
#define ROUTINE_SHELTER     -3          /* routine_at of a mobile that
					   has taken cover from the rain */
#define ROUTINE_FRESH       -2          /* routine_at of a mobile that has
					   not yet settled anywhere: it
					   settles quietly where it stands */

/* kinds of step */
#define RS_TIMED   0                    /* <hour> go|say|emote ...     */
#define RS_ARRIVE  1                    /* arrive say|emote ...        */
#define RS_LEAVE   2                    /* leave say|emote ...         */
#define RS_IDLE    3                    /* idle [h-h] <pct> say|emote  */

/* actions */
#define RA_GO      0
#define RA_SAY     1
#define RA_EMOTE   2

struct routine_step {
  int  kind, action;
  int  hour, hour2;                     /* hour, or idle range          */
  int  pct;                             /* idle chance per pulse        */
  long room;                            /* go: room vnum                */
  int  go_idx;                          /* arrive/leave: its 'go' step  */
  int  pos;                             /* go: position on arrival      */
  char *text;
  struct routine_step *next;
};

struct routine_data {
  long vnum;
  struct routine_step *steps;           /* in file order */
};

static struct routine_data *routines = NULL;
static int num_routines = 0;
static char *town_zone = NULL;          /* zones some routine goes to */


/* ------------------------------------------------------------------ */
/* Loading                                                            */
/* ------------------------------------------------------------------ */

static void free_routines(void)
{
  struct routine_step *s, *next;
  int i;

  for (i = 0; i < num_routines; i++)
    for (s = routines[i].steps; s; s = next) {
      next = s->next;
      if (s->text)
	free(s->text);
      free(s);
    }
  if (routines)
    free(routines);
  routines = NULL;
  num_routines = 0;
}


static void routine_error(int line, const char *msg)
{
  sprintf(buf, "SYSERR: %s line %d: %s", ROUTINE_FILE, line, msg);
  log(buf);
}


static int parse_action(char **p, int *action)
{
  char word[MAX_INPUT_LENGTH];

  *p = one_argument(*p, word);
  if (!str_cmp(word, "go"))
    *action = RA_GO;
  else if (!str_cmp(word, "say"))
    *action = RA_SAY;
  else if (!str_cmp(word, "emote"))
    *action = RA_EMOTE;
  else
    return FALSE;
  return TRUE;
}


/* the rest of the line is the text; it may not contain '$' */
static char *parse_text(char *p, int line)
{
  skip_spaces(&p);
  if (!*p) {
    routine_error(line, "missing text");
    return NULL;
  }
  if (strchr(p, '$')) {
    routine_error(line, "'$' isn't allowed in routine text");
    return NULL;
  }
  return str_dup(p);
}


static int parse_position(char *word)
{
  if (!*word || !str_cmp(word, "stand"))
    return POS_STANDING;
  if (!str_cmp(word, "sit"))
    return POS_SITTING;
  if (!str_cmp(word, "rest"))
    return POS_RESTING;
  if (!str_cmp(word, "sleep"))
    return POS_SLEEPING;
  return -1;
}


/* Parses one line inside a #vnum block.  Returns a new step or NULL. */
static struct routine_step *parse_step(char *p, int line, int last_go)
{
  struct routine_step *s;
  char word[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];

  CREATE(s, struct routine_step, 1);
  p = one_argument(p, word);

  if (isdigit((unsigned char) *word)) {
    s->kind = RS_TIMED;
    s->hour = atoi(word);
    if (s->hour < 0 || s->hour > 23) {
      routine_error(line, "the hour must be 0-23");
      free(s);
      return NULL;
    }
  } else if (!str_cmp(word, "arrive") || !str_cmp(word, "leave")) {
    s->kind = (*word == 'a') ? RS_ARRIVE : RS_LEAVE;
    if ((s->go_idx = last_go) < 0) {
      routine_error(line, "'arrive' and 'leave' must follow a 'go' line");
      free(s);
      return NULL;
    }
  } else if (!str_cmp(word, "idle")) {
    s->kind = RS_IDLE;
    s->hour = 0;
    s->hour2 = 23;
    p = one_argument(p, arg);
    if (strchr(arg, '-')) {
      if (sscanf(arg, "%d-%d", &s->hour, &s->hour2) != 2 ||
	  s->hour < 0 || s->hour > 23 || s->hour2 < 0 || s->hour2 > 23) {
	routine_error(line, "bad hour range (use e.g. 6-20)");
	free(s);
	return NULL;
      }
      p = one_argument(p, arg);
    }
    s->pct = atoi(arg);
    if (s->pct < 1 || s->pct > 100) {
      routine_error(line, "the idle chance must be 1-100 (percent)");
      free(s);
      return NULL;
    }
  } else {
    routine_error(line, "expected an hour, 'arrive', 'leave' or 'idle'");
    free(s);
    return NULL;
  }

  if (!parse_action(&p, &s->action) ||
      (s->action == RA_GO && s->kind != RS_TIMED)) {
    routine_error(line, s->kind == RS_TIMED ? "expected go, say or emote" :
		  "expected say or emote");
    free(s);
    return NULL;
  }

  if (s->action == RA_GO) {
    p = one_argument(p, arg);
    s->room = atol(arg);
    if (!isdigit((unsigned char) *arg) || real_room(s->room) == NOWHERE) {
      routine_error(line, "no such room");
      free(s);
      return NULL;
    }
    p = one_argument(p, arg);
    if ((s->pos = parse_position(arg)) < 0) {
      routine_error(line, "the position must be stand, sit, rest or sleep");
      free(s);
      return NULL;
    }
  } else if (!(s->text = parse_text(p, line))) {
    free(s);
    return NULL;
  }
  return s;
}


/* Gives every prototype and every mobile in the game its routine (or none). */
static void attach_routines(void)
{
  struct char_data *ch;
  struct routine_step *s;
  long i, room;
  int r;

  for (i = 0; i <= top_of_mobt; i++) {
    mob_proto[i].mob_specials.routine = 0;
    mob_proto[i].mob_specials.routine_at = ROUTINE_FRESH;
    mob_proto[i].mob_specials.routine_stuck = 0;
    mob_proto[i].mob_specials.routine_shelter = 0;
  }
  for (r = 0; r < num_routines; r++)
    if ((i = real_mobile(routines[r].vnum)) >= 0)
      mob_proto[i].mob_specials.routine = r + 1;

  RECREATE(town_zone, char, top_of_zone_table + 1);
  memset(town_zone, 0, top_of_zone_table + 1);
  for (r = 0; r < num_routines; r++)
    for (s = routines[r].steps; s; s = s->next)
      if (s->kind == RS_TIMED && s->action == RA_GO &&
	  (room = real_room(s->room)) != NOWHERE)
	town_zone[world[room].zone] = TRUE;

  for (ch = character_list; ch; ch = ch->next)
    if (IS_NPC(ch) && GET_MOB_RNUM(ch) >= 0) {
      ch->mob_specials.routine = mob_proto[GET_MOB_RNUM(ch)].mob_specials.routine;
      ch->mob_specials.routine_at = ROUTINE_FRESH;
      ch->mob_specials.routine_stuck = 0;
      ch->mob_specials.routine_shelter = 0;
    }
}


/* Reads lib/data/routines.  Safe to call again ("reload routines"). */
void load_routines(void)
{
  FILE *fl;
  char line[MAX_STRING_LENGTH], *p;
  struct routine_step *s, *last = NULL;
  struct routine_data *cur = NULL;
  int lineno = 0, max = 0, r, skip = FALSE, gos = 0;
  long vnum;

  free_routines();

  if (!(fl = fopen(ROUTINE_FILE, "r"))) {
    log("No routines file (" ROUTINE_FILE "); NPCs keep no daily routines.");
    attach_routines();
    return;
  }

  while (fgets(line, sizeof(line), fl)) {
    lineno++;
    if ((p = strchr(line, '\n')))
      *p = '\0';
    if ((p = strchr(line, '\r')))
      *p = '\0';
    p = line;
    skip_spaces(&p);
    if (!*p || *p == ';')               /* blank line or comment */
      continue;
    if (*p == '$')
      break;

    if (*p == '#') {
      if (!isdigit((unsigned char) p[1])) /* "# a comment" */
	continue;
      vnum = atol(p + 1);
      cur = NULL;
      skip = FALSE;
      if (real_mobile(vnum) < 0) {
	routine_error(lineno, "no such mobile; its routine is skipped");
	skip = TRUE;
	continue;
      }
      for (r = 0; r < num_routines; r++)
	if (routines[r].vnum == vnum) {
	  routine_error(lineno, "this mobile already has a routine; skipped");
	  skip = TRUE;
	}
      if (skip)
	continue;
      if (num_routines >= max) {
	max = max ? max * 2 : 64;
	RECREATE(routines, struct routine_data, max);
      }
      cur = &routines[num_routines++];
      cur->vnum = vnum;
      cur->steps = last = NULL;
      gos = 0;
      continue;
    }

    if (skip)
      continue;
    if (!cur) {
      routine_error(lineno, "line outside a #<vnum> block");
      continue;
    }
    if ((s = parse_step(p, lineno, gos - 1))) {
      if (s->kind == RS_TIMED && s->action == RA_GO)
	gos++;
      if (last)
	last->next = s;
      else
	cur->steps = s;
      last = s;
    }
  }
  fclose(fl);

  sprintf(buf, "Loaded %d NPC routines.", num_routines);
  log(buf);
  attach_routines();
}


/* ------------------------------------------------------------------ */
/* Running                                                            */
/* ------------------------------------------------------------------ */

static struct routine_data *routine_of(struct char_data *ch)
{
  int r;

  if (!IS_NPC(ch) || !(r = ch->mob_specials.routine) || r > num_routines)
    return NULL;
  return &routines[r - 1];
}


/* The 'go' step in force now: the last one at or before this hour,  */
/* wrapping round to yesterday's last one.  Its index is returned in  */
/* *idx (counting go steps only); NULL if the routine has no 'go'.    */
static struct routine_step *current_go(struct routine_data *rt, int *idx)
{
  struct routine_step *s, *best = NULL, *latest = NULL;
  int i = 0, best_i = -1, latest_i = -1;

  for (s = rt->steps; s; s = s->next) {
    if (s->kind != RS_TIMED || s->action != RA_GO)
      continue;
    if (s->hour <= time_info.hours && (!best || s->hour >= best->hour)) {
      best = s;
      best_i = i;
    }
    if (!latest || s->hour >= latest->hour) {
      latest = s;
      latest_i = i;
    }
    i++;
  }
  if (!best) {
    best = latest;
    best_i = latest_i;
  }
  *idx = best_i;
  return best;
}


/* TRUE if the mobile's routine says where it should be (so it shouldn't */
/* wander about at random).                                              */
int routine_keeps_place(struct char_data *ch)
{
  struct routine_data *rt;
  int idx;

  return ((rt = routine_of(ch)) && current_go(rt, &idx) != NULL);
}


static void routine_line(struct char_data *ch, struct routine_step *s)
{
  char text[MAX_STRING_LENGTH];

  if (s->action == RA_SAY) {
    strcpy(text, s->text);
    do_say(ch, text, 0, 0);
  } else if (s->action == RA_EMOTE) {
    sprintf(text, "$n %s", s->text);
    act(text, FALSE, ch, 0, 0, TO_ROOM);
  }
}


static int pc_in_room(long room)
{
  struct char_data *tch;

  for (tch = world[room].people; tch; tch = tch->next_in_room)
    if (!IS_NPC(tch))
      return TRUE;
  return FALSE;
}


/* Says the arrive or leave lines of 'go' step number idx. */
static void step_lines(struct char_data *ch, struct routine_data *rt,
		       int kind, int idx)
{
  struct routine_step *s;

  for (s = rt->steps; s; s = s->next)
    if (s->kind == kind && s->go_idx == idx)
      routine_line(ch, s);
}


/* The CLOSED sign in a shop. */
static struct obj_data *find_sign(long room)
{
  struct obj_data *o;

  for (o = world[room].contents; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == ROUTINE_SIGN_VNUM)
      return o;
  return NULL;
}


static void shop_sign(struct char_data *ch, long shop_room, int up)
{
  struct obj_data *sign;

  if (shop_room == NOWHERE)
    return;
  sign = find_sign(shop_room);
  if (up && !sign) {
    if ((sign = read_object(ROUTINE_SIGN_VNUM, VNUMBER)))
      obj_to_room(sign, shop_room);
  } else if (!up && sign) {
    act("$n takes down $p.", FALSE, ch, sign, 0, TO_ROOM);
    extract_obj(sign);
  }
}


static int has_key(struct char_data *ch, long key);

/* A room to wait out the weather in: indoors and not someone's home. */
static int shelter_room(long room)
{
  return (!room_outdoors(room) &&
	  !ROOM_FLAGGED(room, ROOM_NOMOB | ROOM_HOUSE | ROOM_PRIVATE |
			ROOM_GODROOM));
}


/* One step of a breadth-first search, limited to the zones of the two */
/* rooms, avoiding death traps and doors that are locked against ch.   */
/* A dest of NOWHERE means the nearest shelter_room() in src's zone.  */
/* Returns a direction or -1.                                          */
static int routine_step_dir(struct char_data *ch, long src, long dest)
{
  static long *queue = NULL;
  static int *seen = NULL, *first = NULL, stamp = 0;
  static long size = 0;
  long head = 0, tail = 0, room, to;
  int dir, z1, z2;

  if (src == dest)
    return -1;
  if (size != top_of_world + 1) {
    size = top_of_world + 1;
    RECREATE(queue, long, size);
    RECREATE(seen, int, size);
    RECREATE(first, int, size);
    memset(seen, 0, sizeof(int) * size);
    stamp = 0;
  }
  if (++stamp == 0) {                   /* wrapped: start afresh */
    memset(seen, 0, sizeof(int) * size);
    stamp = 1;
  }

  z1 = world[src].zone;
  z2 = (dest == NOWHERE) ? z1 : world[dest].zone;
  seen[src] = stamp;
  queue[tail++] = src;

  while (head < tail) {
    room = queue[head++];
    for (dir = 0; dir < NUM_OF_DIRS; dir++) {
      if (!world[room].dir_option[dir] ||
	  (to = world[room].dir_option[dir]->to_room) == NOWHERE ||
	  to < 0 || to > top_of_world || seen[to] == stamp)
	continue;
      if (world[to].zone != z1 && world[to].zone != z2)
	continue;
      if (ROOM_FLAGGED(to, ROOM_DEATH | ROOM_BLADEBARRIER) ||
	  IS_SET(world[room].dir_option[dir]->exit_info, EX_ARROWSLIT))
	continue;
      if (IS_SET(world[room].dir_option[dir]->exit_info, EX_LOCKED) &&
	  !has_key(ch, world[room].dir_option[dir]->key))
	continue;
      if (exit_flooded(room, dir))
	continue;
      seen[to] = stamp;
      first[to] = (room == src) ? dir : first[room];
      if (dest == NOWHERE ? shelter_room(to) : to == dest)
	return first[to];
      queue[tail++] = to;
    }
  }
  return -1;
}


static int has_key(struct char_data *ch, long key)
{
  struct obj_data *o;

  if (key <= 0)
    return FALSE;
  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == key)
      return TRUE;
  return GET_EQ(ch, WEAR_HOLD) && GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key;
}


/* Opens a closed door in direction dir.  FALSE if it can't. */
static int open_way(struct char_data *ch, int dir)
{
  struct room_direction_data *ex = EXIT(ch, dir);
  char arg[MAX_INPUT_LENGTH];

  if (!IS_SET(ex->exit_info, EX_CLOSED))
    return TRUE;
  if (!IS_SET(ex->exit_info, EX_ISDOOR))
    return FALSE;
  sprintf(arg, "%s %s", ex->keyword ? fname(ex->keyword) : "door", dirs[dir]);
  if (IS_SET(ex->exit_info, EX_LOCKED)) {
    if (!has_key(ch, ex->key))
      return FALSE;
    do_gen_door(ch, arg, 0, SCMD_UNLOCK);
  }
  do_gen_door(ch, arg, 0, SCMD_OPEN);
  return !IS_SET(ex->exit_info, EX_CLOSED);
}


/* Closes the door behind a shopkeeper leaving the shop. */
static void close_behind(struct char_data *ch, long from_room)
{
  int dir;
  char arg[MAX_INPUT_LENGTH];
  struct room_direction_data *ex;

  for (dir = 0; dir < NUM_OF_DIRS; dir++)
    if ((ex = EXIT(ch, dir)) && ex->to_room == from_room &&
	IS_SET(ex->exit_info, EX_ISDOOR) && !IS_SET(ex->exit_info, EX_CLOSED)) {
      sprintf(arg, "%s %s", ex->keyword ? fname(ex->keyword) : "door", dirs[dir]);
      do_gen_door(ch, arg, 0, SCMD_CLOSE);
      return;
    }
}


/* Out in a downpour: one step toward the nearest cover.  FALSE if there */
/* is none to be had (then the mobile goes about its business).         */
static int seek_shelter(struct char_data *ch, struct routine_data *rt)
{
  long from, shop;
  int dir;
  char msg[MAX_STRING_LENGTH];

  if ((dir = routine_step_dir(ch, ch->in_room, NOWHERE)) < 0 || !EXIT(ch, dir))
    return FALSE;
  if (GET_POS(ch) < POS_STANDING) {
    act("$n gets to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    return TRUE;
  }
  if (!open_way(ch, dir))
    return FALSE;
  if (ch->mob_specials.routine_at >= 0)        /* packing up */
    step_lines(ch, rt, RS_LEAVE, ch->mob_specials.routine_at);
  if (ch->mob_specials.routine_shelter != 2) {
    act("$n hunches $s shoulders against the weather and hurries for cover.",
	TRUE, ch, 0, 0, TO_ROOM);
    ch->mob_specials.routine_shelter = 2;
  }
  ch->mob_specials.routine_at = -1;
  from = ch->in_room;
  shop = shop_room_of_keeper(ch);
  if (!perform_move(ch, dir, 1) || ch->in_room == from)
    return TRUE;
  if (from == shop) {
    close_behind(ch, from);
    shop_sign(ch, shop, TRUE);
  }
  if (shelter_room(ch->in_room)) {
    sprintf(msg, "$n comes in out of the %s, shaking the wet off $s clothes.",
	    downpour_word(world[ch->in_room].zone));
    act(msg, TRUE, ch, 0, 0, TO_ROOM);
    ch->mob_specials.routine_at = ROUTINE_SHELTER;
    ch->mob_specials.routine_shelter = 1;
  }
  return TRUE;
}


/* TRUE if a wandering mobile in a town would rather not step from    */
/* indoors out into a downpour (mobact.c).                             */
int routine_shuns_weather(struct char_data *ch, long to_room)
{
  int z;

  if (!IS_NPC(ch) || ch->in_room == NOWHERE || to_room == NOWHERE || !town_zone)
    return FALSE;
  z = world[ch->in_room].zone;
  return (town_zone[z] && weather_downpour(z) &&
	  !MOB_FLAGGED(ch, MOB_AGGRESSIVE | MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL |
			   MOB_AGGR_GOOD) &&
	  !room_outdoors(ch->in_room) && room_outdoors(to_room));
}


/* Called every PULSE_MOBILE for an awake, non-fighting mobile.  Returns */
/* TRUE if the mobile did something (and should do nothing else now).    */
int routine_pulse(struct char_data *ch)
{
  struct routine_data *rt;
  struct routine_step *go, *s;
  long dest, from, shop;
  int idx, dir, n, count, rain;

  if (!(rt = routine_of(ch)) || ch->in_room == NOWHERE)
    return FALSE;
  if (ch->master || IS_AFFECTED(ch, AFF_CHARM) || HUNTING(ch) ||
      GET_POS(ch) < POS_RESTING)
    return FALSE;

  go = current_go(rt, &idx);
  dest = go ? real_room(go->room) : NOWHERE;

  /* the weather: stay in, or run for cover */
  rain = weather_downpour(world[ch->in_room].zone);
  if (rain && dest != NOWHERE && room_outdoors(dest) && room_outdoors(ch->in_room)) {
    if (seek_shelter(ch, rt))
      return TRUE;
  } else if (rain && dest != NOWHERE && ch->in_room != dest && !room_outdoors(ch->in_room) &&
	     (room_outdoors(dest) ||
	      (go->pos != POS_SLEEPING &&
	       (dir = routine_step_dir(ch, ch->in_room, dest)) >= 0 && EXIT(ch, dir) &&
	       room_outdoors(EXIT(ch, dir)->to_room)))) {
    if (!ch->mob_specials.routine_shelter) {
      sprintf(buf, "$n looks out at the %s and decides to wait for it to ease.",
	      downpour_word(world[ch->in_room].zone));
      act(buf, TRUE, ch, 0, 0, TO_ROOM);
      ch->mob_specials.routine_shelter = 1;
      return TRUE;
    }
    goto chatter;
  }
  ch->mob_specials.routine_shelter = 0;   /* about its business again */

  if (dest != NOWHERE && ch->in_room != dest) {
    if (ch->mob_specials.routine_stuck >= ROUTINE_MAX_STUCK)
      return FALSE;                     /* gave up until the next hour */
    if (GET_POS(ch) < POS_STANDING) {
      act("$n gets to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_STANDING;
      return TRUE;
    }
    if ((dir = routine_step_dir(ch, ch->in_room, dest)) < 0 ||
	!EXIT(ch, dir) || !open_way(ch, dir)) {
      ch->mob_specials.routine_stuck++;
      return FALSE;
    }
    if (ch->mob_specials.routine_at >= 0)      /* setting off */
      step_lines(ch, rt, RS_LEAVE, ch->mob_specials.routine_at);
    ch->mob_specials.routine_at = -1;
    from = ch->in_room;
    shop = shop_room_of_keeper(ch);
    if (rain && !room_outdoors(from) && EXIT(ch, dir) &&
	room_outdoors(EXIT(ch, dir)->to_room)) {
      sprintf(buf, "$n pulls up $s collar and dashes out into the %s.",
	      downpour_word(world[from].zone));
      act(buf, TRUE, ch, 0, 0, TO_ROOM);
    }
    if (!perform_move(ch, dir, 1) || ch->in_room == from) {
      ch->mob_specials.routine_stuck++;
      return TRUE;
    }
    ch->mob_specials.routine_stuck = 0;
    if (from == shop) {
      close_behind(ch, from);
      shop_sign(ch, shop, TRUE);
    }
    return TRUE;
  }

  if (go && ch->mob_specials.routine_at != idx) {
    /* arrived (or was already there): settle in */
    if (ch->mob_specials.routine_at != ROUTINE_FRESH)
      step_lines(ch, rt, RS_ARRIVE, idx);
    ch->mob_specials.routine_at = idx;
    ch->mob_specials.routine_stuck = 0;
    if (ch->in_room == shop_room_of_keeper(ch))  /* closed while he sleeps */
      shop_sign(ch, ch->in_room, go->pos == POS_SLEEPING);
    if (go->pos != GET_POS(ch)) {
      switch (go->pos) {
      case POS_SITTING:
	act("$n sits down.", TRUE, ch, 0, 0, TO_ROOM);
	break;
      case POS_RESTING:
	act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
	break;
      case POS_SLEEPING:
	act("$n lies down and goes to sleep.", TRUE, ch, 0, 0, TO_ROOM);
	break;
      default:
	act("$n gets to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
	break;
      }
      GET_POS(ch) = go->pos;
    }
    return TRUE;
  }

  /* idle chatter, only when someone is there to hear it */
chatter:
  if (!pc_in_room(ch->in_room))
    return FALSE;
  for (count = 0, s = rt->steps; s; s = s->next)
    if (s->kind == RS_IDLE)
      count++;
  if (!count)
    return FALSE;
  n = number(1, count);
  for (s = rt->steps; s; s = s->next)
    if (s->kind == RS_IDLE && !--n)
      break;
  if (!s || number(1, 100) > s->pct)
    return FALSE;
  if (s->hour <= s->hour2 ?
      (time_info.hours < s->hour || time_info.hours > s->hour2) :
      (time_info.hours < s->hour && time_info.hours > s->hour2))
    return FALSE;                       /* ranges like 20-4 wrap round */
  routine_line(ch, s);
  return FALSE;
}


/* Called by another_hour() once the hour has changed. */
void routines_hour(void)
{
  struct char_data *ch, *next;
  struct routine_data *rt;
  struct routine_step *go, *s;
  int idx;

  if (!num_routines)
    return;

  for (ch = character_list; ch; ch = next) {
    next = ch->next;
    if (!(rt = routine_of(ch)) || ch->in_room == NOWHERE || FIGHTING(ch))
      continue;
    ch->mob_specials.routine_stuck = 0;

    /* time to be somewhere else: wake up */
    go = current_go(rt, &idx);
    if (go && ch->mob_specials.routine_at != idx &&
	GET_POS(ch) == POS_SLEEPING && !IS_AFFECTED2(ch, AFF_SLEEP)) {
      GET_POS(ch) = POS_STANDING;
      act("$n wakes up and stretches.", TRUE, ch, 0, 0, TO_ROOM);
    }
    if (!AWAKE(ch))
      continue;
    for (s = rt->steps; s; s = s->next)
      if (s->kind == RS_TIMED && s->action != RA_GO && s->hour == time_info.hours)
	routine_line(ch, s);
  }
}


/* "show routines" style summary for immortals: where is this mob going? */
void routine_status(struct char_data *ch, struct char_data *mob)
{
  struct routine_data *rt;
  struct routine_step *go;
  int idx;

  if (!(rt = routine_of(mob))) {
    send_to_char("Routine: none.\r\n", ch);
    return;
  }
  go = current_go(rt, &idx);
  if (go)
    sprintf(buf, "Routine: should be in room %ld since %d:00 (%s)%s.\r\n",
	    go->room, go->hour,
	    mob->mob_specials.routine_shelter ? "sheltering from the weather" :
	    mob->mob_specials.routine_at == idx ? "arrived" : "on the way",
	    mob->mob_specials.routine_stuck >= ROUTINE_MAX_STUCK ? ", stuck" : "");
  else
    strcpy(buf, "Routine: stays put (lines only).\r\n");
  send_to_char(buf, ch);
}


/* "show routines": every mobile in the game that has a routine. */
void routine_show(struct char_data *ch)
{
  struct char_data *mob;
  struct routine_data *rt;
  struct routine_step *go;
  char line[256], where[16];
  const char *state;
  int idx, n = 0;

  sprintf(buf, "NPC routines (%d loaded); it is %d:00.\r\n"
	  "  Vnum  Mobile                    In room  Should be  State\r\n",
	  num_routines, time_info.hours);
  for (mob = character_list; mob; mob = mob->next) {
    if (!(rt = routine_of(mob)) || mob->in_room == NOWHERE)
      continue;
    go = current_go(rt, &idx);
    if (!go)
      state = "talks only";
    else if (FIGHTING(mob))
      state = "fighting";
    else if (mob->mob_specials.routine_shelter)
      state = "sheltering";
    else if (mob->mob_specials.routine_stuck >= ROUTINE_MAX_STUCK)
      state = "STUCK";
    else if (GET_POS(mob) == POS_SLEEPING)
      state = "asleep";
    else if (mob->mob_specials.routine_at == idx)
      state = "settled";
    else
      state = "on the way";
    if (go)
      sprintf(where, "%ld", go->room);
    else
      strcpy(where, "-");
    sprintf(line, "%6ld  %-24.24s %8ld  %9s  %s\r\n", GET_MOB_VNUM(mob),
	    GET_NAME(mob), world[mob->in_room].number, where, state);
    if (strlen(buf) + strlen(line) + 16 >= MAX_STRING_LENGTH) {
      strcat(buf, "...\r\n");
      break;
    }
    strcat(buf, line);
    n++;
  }
  if (!n)
    strcat(buf, " (none in the game)\r\n");
  page_string(ch->desc, buf, 1);
}
