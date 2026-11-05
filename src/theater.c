/* ***********************************************************************\
*  File:  THEATER.C                                                       *
*  Usage: The Lyceum, the theatre on Southwall Road in Elrhinn (zone 36)  *
*  Programmer(s): Written for The Dominion, November 2026                 *
\*********************************************************************** */

/*
 * The schedule, the ticket price and the plays come from lib/data/theater
 * (see its header).  Each mud hour theater_hour() looks at the day's
 * schedule: an hour before curtain the house opens (the doors are thrown
 * open, the performers come up to the wings, a handful of theatregoers
 * arrive), and at curtain the performers step out onto the stage and
 * theater_pulse() (every 2 seconds) watches for fights and plays the
 * day's play a line at a time, every THEATER_PULSE seconds, ending with a
 * curtain call.
 *
 * Players need a ticket from Madame Orsolya at the box office (BUY TICKET,
 * LIST) to get past the usher while the house is open; it is torn at the
 * door and lets them back in for the rest of that show.  Anyone who starts
 * a fight in the foyer, the auditorium or backstage annoys the performers,
 * and the usher throws him out into the street; a show disrupted three
 * times is abandoned.
 *
 * Elrhinn's townsfolk with daily routines (routine.c) go to the theatre a
 * couple of times a month: theater_outing() tells the routine engine when
 * one of them has a seat tonight.
 */

#define __THEATER_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct room_data *world;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern const char *weekdays[];

void ASSIGNMOB(long mob, SPECIAL(fname));
void ASSIGNROOM(long room, SPECIAL(fname));
void look_at_room(struct char_data *ch, int mode);


#define THEATER_FILE   "data/theater"
#define THEATER_PULSE  10               /* real seconds between lines   */

/* rooms */
#define T_STREET     3085               /* Eastern Southwall Road       */
#define T_FOYER      3600
#define T_HALL       3601
#define T_WINGS      3602
#define T_DRESSING   3603
#define T_LAST_ROOM  3604               /* the house: 3600-3604         */

/* mobiles */
#define T_ORSOLYA    3600
#define T_USHER      3601
#define T_BRAMWELL   3605
#define T_EXTRA_LO   3606               /* theatregoers, 3606-3610      */
#define T_EXTRA_HI   3610
#define T_TICKET     3600               /* object                       */

#define MAX_CAST       6
#define MAX_SHOWS      2                /* a day                        */
#define MAX_ADMITTED   64

#define LT_NARRATE   0
#define LT_SAY       1
#define LT_EMOTE     2
#define LT_AUDIENCE  3

struct play_line {
  int role, type;
  char *text;
};

struct play {
  char *title;
  long cast[MAX_CAST + 1];              /* role 1.. -> mob vnum         */
  struct play_line *lines;
  int num_lines;
};

#define HOUSE_CLOSED   0
#define HOUSE_OPEN     1
#define HOUSE_PLAYING  2

static struct play *plays = NULL;
static int num_plays = 0;
static int schedule[7][MAX_SHOWS];      /* curtain hours, -1 = none     */
static int ticket_price = 20;
static long default_cast[MAX_CAST + 1];
static char **annoyed = NULL;           /* what performers say at a fight */
static int num_annoyed = 0;

static int house = HOUSE_CLOSED;
static int show_no = 0;                 /* which show today (0, 1)      */
static int cur_play = 0, cur_line = 0;
static int disruptions = 0, paused = 0, brawling = 0;
static long admitted[MAX_ADMITTED];   /* tickets torn this show       */
static int num_admitted = 0;
static long barred[MAX_ADMITTED];     /* thrown out this show         */
static int num_barred = 0;


/* ------------------------------------------------------------------ */
/* Loading                                                            */
/* ------------------------------------------------------------------ */

static void theater_error(int line, const char *msg)
{
  sprintf(buf, "SYSERR: " THEATER_FILE " line %d: %s", line, msg);
  log(buf);
}


void load_theater(void)
{
  FILE *fl;
  char line[MAX_STRING_LENGTH], word[MAX_INPUT_LENGTH], *p;
  struct play *pl = NULL;
  struct play_line *ln;
  int lineno = 0, i, j, max_lines = 0, a, b, c;

  for (i = 0; i < num_plays; i++) {
    for (j = 0; j < plays[i].num_lines; j++)
      free(plays[i].lines[j].text);
    free(plays[i].lines);
    free(plays[i].title);
  }
  free(plays);
  plays = NULL;
  num_plays = 0;
  for (i = 0; i < num_annoyed; i++)
    free(annoyed[i]);
  free(annoyed);
  annoyed = NULL;
  num_annoyed = 0;
  for (i = 0; i < 7; i++)
    for (j = 0; j < MAX_SHOWS; j++)
      schedule[i][j] = -1;
  memset(default_cast, 0, sizeof(default_cast));

  if (!(fl = fopen(THEATER_FILE, "r"))) {
    log("No theatre file (" THEATER_FILE "); the Lyceum is dark.");
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
    if (!*p || *p == ';')
      continue;
    if (*p == '$')
      break;

    if (*p == '#') {                    /* a new play */
      RECREATE(plays, struct play, num_plays + 1);
      pl = &plays[num_plays++];
      memset(pl, 0, sizeof(*pl));
      memcpy(pl->cast, default_cast, sizeof(pl->cast));
      p++;
      while (isdigit((unsigned char) *p))
	p++;
      skip_spaces(&p);
      pl->title = str_dup(*p ? p : "an untitled play");
      max_lines = 0;
      continue;
    }

    if (*p == '*' || isdigit((unsigned char) *p)) {   /* a line of a play */
      if (!pl) {
	theater_error(lineno, "play line before any play");
	continue;
      }
      if (pl->num_lines >= max_lines) {
	max_lines = max_lines ? max_lines * 2 : 32;
	RECREATE(pl->lines, struct play_line, max_lines);
      }
      ln = &pl->lines[pl->num_lines];
      if (*p == '*') {
	ln->role = 0;
	ln->type = LT_NARRATE;
	p++;
      } else {
	ln->role = atoi(p);
	while (isdigit((unsigned char) *p))
	  p++;
	p = one_argument(p, word);
	if (ln->role < 1 || ln->role > MAX_CAST) {
	  theater_error(lineno, "no such role");
	  continue;
	}
	if (!str_cmp(word, "say"))
	  ln->type = LT_SAY;
	else if (!str_cmp(word, "emote"))
	  ln->type = LT_EMOTE;
	else if (!str_cmp(word, "audience"))
	  ln->type = LT_AUDIENCE;
	else {
	  theater_error(lineno, "say, emote or audience?");
	  continue;
	}
      }
      skip_spaces(&p);
      ln->text = str_dup(p);
      pl->num_lines++;
      continue;
    }

    p = one_argument(p, word);
    skip_spaces(&p);
    if (!str_cmp(word, "price"))
      ticket_price = MAX(0, atoi(p));
    else if (!str_cmp(word, "schedule")) {
      a = b = c = -1;
      if (sscanf(p, "%d %d %d", &a, &b, &c) < 1 || a < 0 || a > 6)
	theater_error(lineno, "schedule <weekday 0-6> [<hour> [<hour>]]");
      else {
	schedule[a][0] = (b >= 1 && b <= 23) ? b : -1;
	schedule[a][1] = (c >= 1 && c <= 23) ? c : -1;
      }
    } else if (!str_cmp(word, "cast")) {
      long *to = pl ? pl->cast : default_cast;
      for (i = 1; i <= MAX_CAST; i++) {
	p = one_argument(p, word);
	to[i] = *word ? atol(word) : 0;
	if (*word && real_mobile(to[i]) < 0)
	  theater_error(lineno, "no such mobile in the cast");
      }
    } else if (!str_cmp(word, "annoyed")) {
      RECREATE(annoyed, char *, num_annoyed + 1);
      annoyed[num_annoyed++] = str_dup(p);
    } else
      theater_error(lineno, "unknown line");
  }
  fclose(fl);
  sprintf(buf, "Loaded %d plays for the Lyceum.", num_plays);
  log(buf);
}


/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static int weekday_now(void)
{
  return ((35 * time_info.month) + time_info.day + 1) % 7;
}


/* Today's play: one a day, in rotation. */
static int todays_play(void)
{
  return num_plays ? ((35 * time_info.month) + time_info.day) % num_plays : 0;
}


static int shows_today(void)
{
  return (schedule[weekday_now()][0] >= 0) + (schedule[weekday_now()][1] >= 0);
}


/* The mobile with vnum, wherever it is (the first one). */
static struct char_data *find_mob(long vnum)
{
  struct char_data *c;

  for (c = character_list; c; c = c->next)
    if (IS_NPC(c) && GET_MOB_VNUM(c) == vnum && c->in_room != NOWHERE)
      return c;
  return NULL;
}


static long room_of(long vnum)
{
  return real_room(vnum);
}


static int in_house(long rnum)
{
  return rnum != NOWHERE && world[rnum].number >= T_FOYER &&
    world[rnum].number <= T_LAST_ROOM;
}


static int is_staff(struct char_data *c)
{
  int i;
  long v;

  if (!IS_NPC(c))
    return FALSE;
  v = GET_MOB_VNUM(c);
  if (v == T_ORSOLYA || v == T_USHER || v == T_BRAMWELL)
    return TRUE;
  for (i = 1; i <= MAX_CAST; i++)
    if (num_plays && plays[cur_play].cast[i] == v)
      return TRUE;
  return FALSE;
}


static int is_extra(struct char_data *c)
{
  return IS_NPC(c) && GET_MOB_VNUM(c) >= T_EXTRA_LO && GET_MOB_VNUM(c) <= T_EXTRA_HI;
}


/* Moves c to room vnum to with messages either side (no exits needed). */
static void move_to(struct char_data *c, long to, char *leave, char *arrive)
{
  long r = room_of(to);

  if (r == NOWHERE || c->in_room == r)
    return;
  if (leave)
    act(leave, TRUE, c, 0, 0, TO_ROOM);
  char_from_room(c);
  char_to_room(c, r);
  if (arrive)
    act(arrive, TRUE, c, 0, 0, TO_ROOM);
}


static void to_room_vnum(long vnum, const char *text)
{
  long r = room_of(vnum);

  if (r != NOWHERE && world[r].people)
    send_to_room((char *) text, r);
}


/* Doors of the house: 3085 S / 3600 N, and 3600 S / 3601 N. */
static void set_door(long vnum, int dir, int open)
{
  long r = room_of(vnum);

  if (r == NOWHERE || !world[r].dir_option[dir] ||
      !IS_SET(world[r].dir_option[dir]->exit_info, EX_ISDOOR))
    return;
  if (open)
    REMOVE_BIT(world[r].dir_option[dir]->exit_info, EX_CLOSED | EX_LOCKED);
  else
    SET_BIT(world[r].dir_option[dir]->exit_info, EX_CLOSED);
}


static void open_doors(int open)
{
  set_door(T_STREET, SOUTH, open);
  set_door(T_FOYER, NORTH, open);
  set_door(T_FOYER, SOUTH, open);
  set_door(T_HALL, NORTH, open);
}


/* The performer playing role, if he's on the stage and able. */
static struct char_data *performer(int role)
{
  long vnum, hall = room_of(T_HALL);
  struct char_data *c;

  if (!num_plays || role < 1 || role > MAX_CAST || !(vnum = plays[cur_play].cast[role]))
    return NULL;
  for (c = world[hall].people; c; c = c->next_in_room)
    if (IS_NPC(c) && GET_MOB_VNUM(c) == vnum && AWAKE(c) && !FIGHTING(c))
      return c;
  return NULL;
}


/* Someone in the stalls for the performers to pick on: a player if */
/* there is one, otherwise a townsman.                               */
static struct char_data *audience_member(void)
{
  struct char_data *c, *pick = NULL;
  long hall = room_of(T_HALL);
  int n = 0, pcs = 0;

  for (c = world[hall].people; c; c = c->next_in_room)
    if (!IS_NPC(c) && GET_LEVEL(c) < LVL_IMMORT && !AFF_FLAGGED(c, AFF_INVISIBLE))
      pcs++;
  for (c = world[hall].people; c; c = c->next_in_room) {
    if (is_staff(c) || (pcs && IS_NPC(c)) || AFF_FLAGGED(c, AFF_INVISIBLE) ||
	(!IS_NPC(c) && GET_LEVEL(c) >= LVL_IMMORT && pcs))
      continue;
    if (!number(0, n++))
      pick = c;
  }
  return pick;
}


/* ------------------------------------------------------------------ */
/* The house                                                          */
/* ------------------------------------------------------------------ */

static void open_house(int which)
{
  struct char_data *c;
  long foyer = room_of(T_FOYER);
  int i, n;

  if (!num_plays || foyer == NOWHERE)
    return;
  house = HOUSE_OPEN;
  show_no = which;
  cur_play = todays_play();
  cur_line = disruptions = paused = brawling = 0;
  num_admitted = num_barred = 0;
  open_doors(TRUE);

  sprintf(buf, "The doors of the Lyceum are thrown open, and a boy with a bell cries "
	  "'%s!  Tonight at the Lyceum!  Curtain in an hour!'\r\n", plays[cur_play].title);
  to_room_vnum(T_STREET, buf);
  to_room_vnum(T_FOYER, "The usher throws open the doors.  The house is open!\r\n");

  /* the company comes up to the wings */
  for (i = 1; i <= MAX_CAST; i++)
    if (plays[cur_play].cast[i] && (c = find_mob(plays[cur_play].cast[i])) &&
	in_house(c->in_room) && !FIGHTING(c)) {
      if (GET_POS(c) < POS_STANDING)
	GET_POS(c) = POS_STANDING;
      move_to(c, T_WINGS, "$n gathers up $s costume and hurries off to the wings.",
	      "$n hurries in, fastening $s costume.");
    }

  /* a few theatregoers */
  n = number(2, 5);
  for (i = 0; i < n; i++)
    if ((c = read_mobile(number(T_EXTRA_LO, T_EXTRA_HI), VNUMBER))) {
      char_to_room(c, foyer);
      act("$n comes in from the street, chattering about the play.", TRUE, c, 0, 0, TO_ROOM);
    }
}


static void end_show(int abandoned)
{
  struct char_data *c, *next;
  long hall = room_of(T_HALL), foyer = room_of(T_FOYER);
  int i;

  if (!abandoned) {
    to_room_vnum(T_HALL, "The curtain falls, and the house erupts in applause.  The "
		 "company comes out hand in hand to bow, and bow again.\r\n");
    for (i = 1; i <= MAX_CAST; i++)
      if ((c = performer(i)))
	act("$n bows low, sweeping the boards with $s hand.", TRUE, c, 0, 0, TO_ROOM);
  }
  /* the company goes back to the dressing rooms */
  for (i = 1; i <= MAX_CAST; i++)
    if (plays[cur_play].cast[i] && (c = find_mob(plays[cur_play].cast[i])) &&
	in_house(c->in_room) && !FIGHTING(c))
      move_to(c, T_DRESSING, "$n slips away behind the curtain.",
	      "$n comes in, tugging off $s wig.");
  /* the theatregoers go home */
  for (i = 0; i < 2; i++) {
    long r = i ? foyer : hall;

    if (r == NOWHERE)
      continue;
    for (c = world[r].people; c; c = next) {
      next = c->next_in_room;
      if (is_extra(c) && !FIGHTING(c)) {
	act("$n gathers $s things and files out with the crowd.", TRUE, c, 0, 0, TO_ROOM);
	extract_char(c);
      }
    }
  }
  to_room_vnum(T_STREET, "The audience spills out of the Lyceum onto Southwall Road, "
	       "talking about the show.\r\n");
  open_doors(FALSE);                    /* the townsfolk open them again to leave */
  house = HOUSE_CLOSED;
  num_admitted = num_barred = 0;
}


static void begin_show(void)
{
  struct char_data *c, *next;
  long hall = room_of(T_HALL), foyer = room_of(T_FOYER);
  int i;

  /* the stragglers take their seats */
  for (c = world[foyer].people; c; c = next) {
    next = c->next_in_room;
    if (is_extra(c) && !FIGHTING(c)) {
      move_to(c, T_HALL, "$n goes in to take $s seat.", "$n finds a seat in the stalls.");
      GET_POS(c) = POS_SITTING;
    }
  }
  /* the company takes the stage */
  for (i = 1; i <= MAX_CAST; i++)
    if (plays[cur_play].cast[i] && (c = find_mob(plays[cur_play].cast[i])) &&
	in_house(c->in_room) && !FIGHTING(c) && AWAKE(c))
      move_to(c, T_HALL, NULL, "$n steps out onto the stage.");

  if (!performer(1)) {
    to_room_vnum(T_HALL, "Madame Orsolya pushes through the curtain, wringing her hands: "
		 "'Ladies and gentlemen, I regret that tonight's performance is "
		 "cancelled.'  The audience groans.\r\n");
    end_show(TRUE);
    return;
  }
  house = HOUSE_PLAYING;
  cur_line = 0;
  (void) hall;
  sprintf(buf, "The lamps are lowered, the footlights blaze up, and the curtain rises on "
	  "%s.\r\n", plays[cur_play].title);
  to_room_vnum(T_HALL, buf);
  to_room_vnum(T_FOYER, "From behind the padded doors comes a hush, and then a voice "
	       "rings out from the stage.\r\n");
}


/* Each mud hour (weather.c). */
void theater_hour(void)
{
  int w = weekday_now(), i, h = time_info.hours;

  if (!num_plays)
    return;
  for (i = 0; i < MAX_SHOWS; i++) {
    if (schedule[w][i] < 0)
      continue;
    if (house == HOUSE_CLOSED && h == (schedule[w][i] + 23) % 24)
      open_house(i);
    else if (house == HOUSE_OPEN && show_no == i && h == schedule[w][i])
      begin_show();
  }
}


/* ------------------------------------------------------------------ */
/* Fights                                                             */
/* ------------------------------------------------------------------ */

static void throw_out(struct char_data *c, struct char_data *usher)
{
  long street = room_of(T_STREET);

  if (FIGHTING(c))
    stop_fighting(c);
  if (street == NOWHERE)
    return;
  act("$N seizes $n by the collar and the seat of $s breeches and hurls $m out into "
      "the street!", FALSE, c, 0, usher, TO_ROOM);
  act("$N seizes you by the collar and the seat of your breeches and hurls you out "
      "into the street!", FALSE, c, 0, usher, TO_CHAR);
  if (!IS_NPC(c) && num_barred < MAX_ADMITTED && house != HOUSE_CLOSED)
    barred[num_barred++] = GET_IDNUM(c);        /* not back in tonight */
  char_from_room(c);
  char_to_room(c, street);
  GET_POS(c) = POS_SITTING;
  act("$n comes flying out of the Lyceum and lands in the road.", TRUE, c, 0, 0, TO_ROOM);
  look_at_room(c, 0);
}


/* Anyone fighting in the house annoys the performers, and the usher */
/* throws the troublemakers out.  TRUE if there was a fight.          */
static int handle_fights(void)
{
  struct char_data *c, *next, *usher = find_mob(T_USHER), *opp, *who, *perf, *brawler;
  long r;
  int vnum, found = FALSE, said = FALSE;

  for (vnum = T_FOYER; vnum <= T_LAST_ROOM; vnum++) {
    if ((r = room_of(vnum)) == NOWHERE)
      continue;
    for (c = world[r].people; c; c = next) {
      next = c->next_in_room;
      if (!(opp = FIGHTING(c)))
	continue;
      found = TRUE;
      /* a performer in the room who isn't in it himself is annoyed, */
      /* at the player if one is fighting                             */
      brawler = (IS_NPC(c) && !IS_NPC(opp)) ? opp : c;
      if (!said && num_annoyed)
	for (perf = world[r].people; perf; perf = perf->next_in_room)
	  if (is_staff(perf) && !FIGHTING(perf) && AWAKE(perf) &&
	      GET_MOB_VNUM(perf) != T_USHER) {
	    sprintf(buf, "$n %s", annoyed[number(0, num_annoyed - 1)]);
	    act(buf, FALSE, perf, 0, brawler, TO_ROOM);
	    said = TRUE;
	    break;
	  }
      /* the usher comes and throws the player (or his pet) out */
      if (usher && AWAKE(usher) && c != usher) {
	who = c;
	if (IS_NPC(c) && !IS_NPC(opp))
	  who = opp;                    /* the player started it */
	if (IS_NPC(who) && who->master && !IS_NPC(who->master) &&
	    who->master->in_room == who->in_room)
	  who = who->master;
	if (who != usher && (IS_NPC(who) ? !is_staff(who) : GET_LEVEL(who) < LVL_IMMORT)) {
	  if (usher->in_room != who->in_room) {
	    act("$n hears the commotion and lumbers off at a run.", TRUE, usher, 0, 0, TO_ROOM);
	    char_from_room(usher);
	    char_to_room(usher, who->in_room);
	    act("$n bursts in, cracking $s knuckles.", TRUE, usher, 0, 0, TO_ROOM);
	  }
	  if (FIGHTING(who) && FIGHTING(FIGHTING(who)) == who)
	    stop_fighting(FIGHTING(who));
	  throw_out(who, usher);
	  next = world[r].people;       /* the room has changed */
	}
      }
    }
  }
  if (usher && !FIGHTING(usher) && usher->in_room != room_of(T_FOYER) && !found &&
      house != HOUSE_CLOSED)
    move_to(usher, T_FOYER, "$n straightens $s coat and goes back to the doors.",
	    "$n comes back to $s post by the doors.");
  return found;
}


/* ------------------------------------------------------------------ */
/* The performance                                                    */
/* ------------------------------------------------------------------ */

static void play_line(struct play_line *ln)
{
  struct char_data *who, *target;
  char msg[MAX_STRING_LENGTH];

  if (ln->type == LT_NARRATE) {
    snprintf(msg, sizeof(msg), "%s\r\n", ln->text);
    to_room_vnum(T_HALL, msg);
    return;
  }
  if (!(who = performer(ln->role)))
    return;                             /* not on tonight: skip his lines */
  switch (ln->type) {
  case LT_SAY:
    snprintf(msg, sizeof(msg), "$n declaims, '%s'", ln->text);
    act(msg, FALSE, who, 0, 0, TO_ROOM);
    break;
  case LT_EMOTE:
    snprintf(msg, sizeof(msg), "$n %s", ln->text);
    act(msg, FALSE, who, 0, 0, TO_ROOM);
    break;
  case LT_AUDIENCE:
    if (!(target = audience_member()))
      return;                           /* an empty house */
    snprintf(msg, sizeof(msg), "$n %s", ln->text);
    act(msg, FALSE, who, 0, target, TO_NOTVICT);
    act(msg, FALSE, who, 0, target, TO_VICT);
    break;
  }
}


/* Every 2 seconds (comm.c). */
void theater_pulse(void)
{
  static int beat = 0;
  struct char_data *c;
  int fight;

  if (!num_plays)
    return;
  fight = handle_fights();
  if (fight && house == HOUSE_PLAYING && !brawling) {
    brawling = 1;                       /* one brawl, one disruption */
    if (++disruptions >= 3) {
      if ((c = performer(1)))
	act("$n flings down $s fan.  'That is IT!  I will not perform for ANIMALS!'  "
	    "$e storms off, and the curtain comes down early, to boos.",
	    FALSE, c, 0, 0, TO_ROOM);
      else
	to_room_vnum(T_HALL, "The curtain comes down early, to boos.\r\n");
      end_show(TRUE);
      return;
    }
    paused = 1;
    to_room_vnum(T_HALL, "The actors freeze in mid-scene and glare out at the stalls.\r\n");
    return;
  }
  if (!fight)
    brawling = 0;
  if (++beat < THEATER_PULSE / 2)
    return;                             /* the rest every THEATER_PULSE seconds */
  beat = 0;

  if (house == HOUSE_OPEN) {            /* the audience drifts in */
    long foyer = room_of(T_FOYER);
    struct char_data *next;

    for (c = world[foyer].people; c; c = next) {
      next = c->next_in_room;
      if (is_extra(c) && !FIGHTING(c) && !number(0, 3)) {
	move_to(c, T_HALL, "$n goes in to take $s seat.", "$n finds a seat in the stalls.");
	GET_POS(c) = POS_SITTING;
      }
    }
    return;
  }
  if (house != HOUSE_PLAYING)
    return;
  if (paused) {
    paused = 0;
    if ((c = performer(1)) || (c = performer(2)))
      act("$n clears $s throat pointedly.  'As I was SAYING...'", FALSE, c, 0, 0, TO_ROOM);
    return;
  }
  if (cur_line >= plays[cur_play].num_lines) {
    end_show(FALSE);
    return;
  }
  play_line(&plays[cur_play].lines[cur_line++]);
}


/* ------------------------------------------------------------------ */
/* The townsfolk's nights out (routine.c)                             */
/* ------------------------------------------------------------------ */

/* The auditorium if mob, one of Elrhinn's townsfolk with a routine, */
/* has a seat at tonight's show, else NOWHERE.  Each goes on two days */
/* of the month (if there is a show that day).                         */
long theater_outing(struct char_data *mob)
{
  long v = GET_MOB_VNUM(mob);
  int d1, d2;

  if (house == HOUSE_CLOSED || !IS_NPC(mob) || v < 3000 || v > 3499)
    return NOWHERE;
  d1 = (int) ((v * 11) % 35);
  d2 = (d1 + 17) % 35;
  if (time_info.day != d1 && time_info.day != d2)
    return NOWHERE;
  if (shows_today() > 1 && show_no != (int) (v % 2))
    return NOWHERE;                     /* the other show */
  return room_of(T_HALL);
}


/* ------------------------------------------------------------------ */
/* The box office and the usher                                       */
/* ------------------------------------------------------------------ */

static void show_times(char *out)
{
  static const char *hours[] = {
    "midnight", "the 1st hour of the morning", "the 2nd hour of the morning",
    "the 3rd hour of the morning", "the 4th hour of the morning",
    "the 5th hour of the morning", "the 6th hour of the morning",
    "the 7th hour of the morning", "the 8th hour of the morning",
    "the 9th hour of the morning", "the 10th hour of the morning",
    "the 11th hour of the morning", "noon", "the 1st hour of the afternoon",
    "the 2nd hour of the afternoon", "the 3rd hour of the afternoon",
    "the 4th hour of the afternoon", "the 5th hour of the evening",
    "the 6th hour of the evening", "the 7th hour of the evening",
    "the 8th hour of the evening", "the 9th hour of the evening",
    "the 10th hour of the evening", "the 11th hour of the evening"
  };
  int w = weekday_now();

  if (schedule[w][0] < 0)
    sprintf(out, "Today is %s, and the house is dark.", weekdays[w]);
  else if (schedule[w][1] < 0)
    sprintf(out, "Today, %s, there is one performance, at %s.", weekdays[w],
	    hours[schedule[w][0]]);
  else
    sprintf(out, "Today, %s, there is a matinee at %s and a late performance at %s.",
	    weekdays[w], hours[schedule[w][0]], hours[schedule[w][1]]);
}


SPECIAL(theater_box_office)
{
  struct char_data *keeper = (struct char_data *) me;
  struct obj_data *ticket;
  char arg[MAX_INPUT_LENGTH], times[256];
  int playing;

  if (!AWAKE(keeper) || FIGHTING(keeper) || IS_NPC(ch))
    return FALSE;
  if (CMD_IS("list")) {
    playing = todays_play();
    show_times(times);
    sprintf(buf, "$n peers at you through the grille.  '%s  We are playing %s.  A seat "
	    "is %d gold - BUY TICKET, dearie.'", times,
	    num_plays ? plays[playing].title : "nothing", ticket_price);
    act(buf, FALSE, keeper, 0, ch, TO_VICT);
    return TRUE;
  }
  if (!CMD_IS("buy"))
    return FALSE;
  one_argument(argument, arg);
  if (!*arg || !isname(arg, "ticket tickets seat"))
    return FALSE;                       /* not ours */
  if (GET_GOLD(ch) < ticket_price) {
    act("$n looks you up and down.  'A seat is not free, dearie.  Come back with some coin.'",
	FALSE, keeper, 0, ch, TO_VICT);
    return TRUE;
  }
  if (!(ticket = read_object(T_TICKET, VNUMBER)))
    return TRUE;
  GET_GOLD(ch) -= ticket_price;
  obj_to_char(ticket, ch);
  sprintf(buf, "You push %d gold through the grille, and $N slides back $p.", ticket_price);
  act(buf, FALSE, ch, ticket, keeper, TO_CHAR);
  act("$n buys $p from $N.", TRUE, ch, ticket, keeper, TO_ROOM);
  act("$n says to you, 'Enjoy the show.  And no brawling.'", FALSE, keeper, 0, ch, TO_VICT);
  return TRUE;
}


static int was_admitted(struct char_data *ch)
{
  int i;

  for (i = 0; i < num_admitted; i++)
    if (admitted[i] == GET_IDNUM(ch))
      return TRUE;
  return FALSE;
}


SPECIAL(theater_usher)
{
  struct char_data *usher = (struct char_data *) me;
  struct obj_data *obj;
  int i;

  if (!IS_MOVE(cmd) || cmd - 1 != SOUTH || IS_NPC(ch) || GET_LEVEL(ch) >= LVL_IMMORT ||
      !AWAKE(usher) || FIGHTING(usher) || world[ch->in_room].number != T_FOYER ||
      usher->in_room != ch->in_room || house == HOUSE_CLOSED)
    return FALSE;
  for (i = 0; i < num_barred; i++)
    if (barred[i] == GET_IDNUM(ch)) {
      act("$N plants a hand on your chest.  'You?  Not tonight, sunshine.'",
	  FALSE, ch, 0, usher, TO_CHAR);
      act("$N plants a hand on $n's chest and shakes $s head.", TRUE, ch, 0, usher, TO_ROOM);
      return TRUE;
    }
  if (was_admitted(ch))
    return FALSE;
  for (obj = ch->carrying; obj; obj = obj->next_content)
    if (GET_OBJ_VNUM(obj) == T_TICKET)
      break;
  if (!obj) {
    act("$N steps in front of you.  'Ticket, please.  The box office is right there.'",
	FALSE, ch, 0, usher, TO_CHAR);
    act("$N steps in front of $n.", TRUE, ch, 0, usher, TO_ROOM);
    return TRUE;
  }
  act("$N tears $p in half and waves you through.", FALSE, ch, obj, usher, TO_CHAR);
  act("$N tears $n's ticket and waves $m through.", TRUE, ch, 0, usher, TO_ROOM);
  extract_obj(obj);
  if (num_admitted < MAX_ADMITTED)
    admitted[num_admitted++] = GET_IDNUM(ch);
  return FALSE;                         /* on he goes */
}


/* The auditorium: no wandering backstage during a performance. */
SPECIAL(theater_hall)
{
  if (!IS_MOVE(cmd) || cmd - 1 != WEST || house != HOUSE_PLAYING ||
      GET_LEVEL(ch) >= LVL_IMMORT || is_staff(ch))
    return FALSE;
  send_to_char("A stagehand's arm shoots out of the curtain and pushes you back.  "
	       "'Not during the performance!'\r\n", ch);
  act("A stagehand's arm shoots out of the curtain and pushes $n back.", TRUE, ch, 0, 0, TO_ROOM);
  return TRUE;
}


void assign_theater(void)
{
  ASSIGNMOB(T_ORSOLYA, theater_box_office);
  ASSIGNMOB(T_USHER, theater_usher);
  ASSIGNROOM(T_HALL, theater_hall);
}


/* "show theater" for immortals; also "theater open|start|end" tests. */
void theater_show(struct char_data *ch, char *arg)
{
  static const char *states[] = { "closed", "open", "playing" };
  char times[256];

  if (!str_cmp(arg, "open") && house == HOUSE_CLOSED)
    open_house(0);
  else if (!str_cmp(arg, "start") && house == HOUSE_OPEN)
    begin_show();
  else if (!str_cmp(arg, "end") && house != HOUSE_CLOSED)
    end_show(FALSE);
  show_times(times);
  sprintf(buf, "The Lyceum: %d plays, ticket %d gold.  %s\r\nHouse %s; tonight %s, "
	  "line %d of %d, %d disruptions.\r\n", num_plays, ticket_price, times,
	  states[house], num_plays ? plays[house ? cur_play : todays_play()].title : "-", cur_line,
	  num_plays ? plays[cur_play].num_lines : 0, disruptions);
  send_to_char(buf, ch);
}
