/* ***********************************************************************\
*  File:  SHIP.C                                                          *
*  Usage: The Sea Wren, its voyages, and the pirates who board it         *
*  Programmer(s): Written for The Dominion, November 2026                 *
\*********************************************************************** */

/*
 * The Sea Wren (zone 42) sails a route of ports read from lib/data/ships,
 * back and forth: docked for a few hours, then out to sea for a few, with
 * no way off while she sails.  Docking puts a gangplank between the port
 * room and the main deck (an exit made and unmade in world[]).  Passengers
 * pay the purser the fare for each leg as she casts off; whoever can't is
 * put ashore.  Captains don't sail in a downpour.
 *
 * Pirates: on some legs a pirate sloop may come alongside and board her
 * (4210-4212).  They fight the crew and passengers until they are all dead
 * (victory) or the captain falls or time runs out, when they plunder a
 * tenth of every passenger's gold and leave.
 *
 * The Pirate King: when a player aboard is on quest 4201 (from the
 * captain) and the ship runs round the cape, Morrow Blackwake's Red Maw
 * comes instead, with his reavers (4213).  Kill them all and he sheers
 * off; but if anyone SURRENDERs, or the captain falls, or the fight goes
 * on long enough, he takes the ship: the passengers' belongings go into
 * sea chests in his storeroom (obj 4300, room 4306), their gold into his
 * coffers, and they into his brig (4300) on Blackwake's Isle (zone 43).
 * Escaping, getting their things back (OPEN CHEST in the store) and
 * finding the King is up to them.  The ship is ransomed back to Vice
 * Island a few hours later.
 *
 * Captivity is crash-safe: the gold taken is saved in the player file
 * (PirateGold), and the crash save writes the chest's contents as if the
 * captive were carrying them (ship_lend_gear()), so a reboot or a crash
 * hands everything back and he wakes free at Vice Island
 * (ship_login()).  Quitting, renting or idling out releases him first
 * (ship_release()); dying returns his things to his corpse, which washes
 * ashore at Vice Island (ship_corpse_room()).
 */

#define __SHIP_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct room_data *world;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;

void ASSIGNMOB(long mob, SPECIAL(fname));
void ASSIGNROOM(long room, SPECIAL(fname));
void look_at_room(struct char_data *ch, int mode);
void hit(struct char_data *ch, struct char_data *victim, int type);
void reset_zone(int zone);
void Crash_crashsave(struct char_data *ch);

#define SHIP_FILE      "data/ships"

#define SHIP_LO        4200             /* the ship's rooms             */
#define SHIP_HI        4299
#define CAPTAIN        4200
#define PURSER         4201
#define SAILOR         4202
#define COOK           4203
#define CUTTHROAT      4210
#define BOARDER        4211
#define BOSUN          4212
#define REAVER         4213
#define ISLE_LO        4300
#define ISLE_HI        4399
#define BRIG           4300
#define STORE          4306
#define SEA_CHEST      4300
#define KING_QUEST     4201

#define MAX_PORTS      8
#define MAX_LINES      40
#define MAX_CAPTIVES   64
#define MAX_LENT       400

#define SHIP_DOCKED    0
#define SHIP_SAILING   1
#define SHIP_CAPTURED  2

#define RAID_NONE      0
#define RAID_PIRATES   1
#define RAID_KING      2

struct port {
  long room;                            /* vnum                         */
  int dir;                              /* port room -> deck            */
  char *name;
};

struct leg {
  int hours, chance, cape;
};

static char *ship_name = NULL;
static long deck_vnum = 4200;
static long haven_vnum = 10590;
static int fare = 25, docked_hours = 2, captured_hours = 6;
static struct port ports[MAX_PORTS];
static struct leg legs[MAX_PORTS];
static int num_ports = 0;
static char *sea_lines[MAX_LINES], *cape_lines[MAX_LINES];
static int num_sea = 0, num_cape = 0;

static int state = SHIP_DOCKED, at = 0, heading = 1, bound = 0, hours_left = 0;
static int cur_leg = 0, hour_in_leg = 0, waited = 0;
static int raid = RAID_NONE, raid_at = 0, raid_pulses = 0, surrendered = 0;

struct captive {
  long idnum;
  struct obj_data *chest;
};
static struct captive captives[MAX_CAPTIVES];
static int num_captives = 0;
static struct obj_data *lent[MAX_LENT];
static int num_lent = 0;
static long washed_ashore = 0;          /* idnum whose corpse goes to haven */

static const char *dir_names[] = { "north", "east", "south", "west", "up", "down", "\n" };


/* ------------------------------------------------------------------ */
/* Loading                                                            */
/* ------------------------------------------------------------------ */

static void ship_error(int line, const char *msg)
{
  sprintf(buf, "SYSERR: " SHIP_FILE " line %d: %s", line, msg);
  log(buf);
}


static void gangplank(int p, int down);

void load_ships(void)
{
  FILE *fl;
  char line[MAX_STRING_LENGTH], word[MAX_INPUT_LENGTH], *p;
  int lineno = 0, i, a, b, nlegs = 0;

  if (!(fl = fopen(SHIP_FILE, "r"))) {
    log("No ships file (" SHIP_FILE "); no ships sail.");
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
    p = one_argument(p, word);
    skip_spaces(&p);
    if (!str_cmp(word, "ship"))
      ship_name = str_dup(p);
    else if (!str_cmp(word, "deck"))
      deck_vnum = atol(p);
    else if (!str_cmp(word, "haven"))
      haven_vnum = atol(p);
    else if (!str_cmp(word, "fare"))
      fare = MAX(0, atoi(p));
    else if (!str_cmp(word, "docked"))
      docked_hours = MAX(1, atoi(p));
    else if (!str_cmp(word, "captured"))
      captured_hours = MAX(1, atoi(p));
    else if (!str_cmp(word, "port")) {
      char dir[MAX_INPUT_LENGTH];

      if (num_ports >= MAX_PORTS) {
	ship_error(lineno, "too many ports");
	continue;
      }
      ports[num_ports].room = atol(p);
      p = one_argument(p, word);
      p = one_argument(p, dir);
      skip_spaces(&p);
      if ((ports[num_ports].dir = search_block(dir, (char **) dir_names, TRUE)) < 0 ||
	  ports[num_ports].dir == WEST) {
	ship_error(lineno, "port <room> <north|east|south|up|down> <name>");
	continue;
      }
      if (real_room(ports[num_ports].room) == NOWHERE) {
	ship_error(lineno, "no such port room");
	continue;
      }
      ports[num_ports].name = str_dup(*p ? p : "port");
      num_ports++;
    } else if (!str_cmp(word, "leg")) {
      a = b = 0;
      if (nlegs >= MAX_PORTS || sscanf(p, "%d %d", &a, &b) < 1) {
	ship_error(lineno, "leg <hours> [<pirate chance %>] [cape]");
	continue;
      }
      legs[nlegs].hours = MAX(1, a);
      legs[nlegs].chance = MAX(0, MIN(100, b));
      legs[nlegs].cape = (strstr(p, "cape") != NULL);
      nlegs++;
    } else if (!str_cmp(word, "sea") && num_sea < MAX_LINES)
      sea_lines[num_sea++] = str_dup(p);
    else if (!str_cmp(word, "cape") && num_cape < MAX_LINES)
      cape_lines[num_cape++] = str_dup(p);
    else
      ship_error(lineno, "unknown line");
  }
  fclose(fl);

  if (num_ports < 2 || real_room(deck_vnum) == NOWHERE) {
    log("SYSERR: " SHIP_FILE ": a ship needs a deck and at least two ports.");
    num_ports = 0;
    return;
  }
  for (i = nlegs; i < num_ports - 1; i++) {
    legs[i].hours = 3;
    legs[i].chance = 0;
    legs[i].cape = 0;
  }
  for (i = 0; i < num_ports; i++)
    if (world[real_room(ports[i].room)].dir_option[ports[i].dir]) {
      sprintf(buf, "SYSERR: " SHIP_FILE ": port %ld already has an exit %s.",
	      ports[i].room, dir_names[ports[i].dir]);
      log(buf);
    }
  /* she starts the day docked at the first port */
  state = SHIP_DOCKED;
  at = 0;
  heading = 1;
  hours_left = docked_hours;
  gangplank(at, TRUE);
  sprintf(buf, "Loaded the %s: %d ports.", ship_name ? ship_name : "ship", num_ports);
  log(buf);
}


/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static int aboard_room(long rnum)
{
  return rnum != NOWHERE && world[rnum].number >= SHIP_LO && world[rnum].number <= SHIP_HI;
}


static int is_pirate(struct char_data *c)
{
  return IS_NPC(c) && GET_MOB_VNUM(c) >= CUTTHROAT && GET_MOB_VNUM(c) <= REAVER;
}


static int is_crew(struct char_data *c)
{
  return IS_NPC(c) && (GET_MOB_VNUM(c) == CAPTAIN || GET_MOB_VNUM(c) == SAILOR ||
		       GET_MOB_VNUM(c) == COOK || GET_MOB_VNUM(c) == PURSER);
}


/* Still in the game?  (Pointers go stale when someone dies.) */
static int valid_char(struct char_data *c)
{
  struct char_data *x;

  for (x = character_list; x; x = x->next)
    if (x == c)
      return TRUE;
  return FALSE;
}


static void to_ship(const char *text)
{
  long r;
  int v;

  for (v = SHIP_LO; v <= SHIP_HI; v++)
    if ((r = real_room(v)) != NOWHERE && world[r].people)
      send_to_room((char *) text, r);
}


static void to_port(int p, const char *text)
{
  long r = real_room(ports[p].room);

  if (r != NOWHERE && world[r].people)
    send_to_room((char *) text, r);
}


/* The mortal players aboard, into list; returns how many. */
static int players_aboard(struct char_data **list, int max)
{
  struct char_data *c;
  int n = 0;

  for (c = character_list; c && n < max; c = c->next)
    if (!IS_NPC(c) && GET_LEVEL(c) < LVL_IMMORT && aboard_room(c->in_room))
      list[n++] = c;
  return n;
}


static struct char_data *find_aboard(long vnum)
{
  struct char_data *c;

  for (c = character_list; c; c = c->next)
    if (IS_NPC(c) && GET_MOB_VNUM(c) == vnum && aboard_room(c->in_room))
      return c;
  return NULL;
}


static void make_exit(long from, int dir, long to)
{
  struct room_direction_data *ex;

  if (world[from].dir_option[dir])
    return;
  CREATE(ex, struct room_direction_data, 1);
  ex->general_description = str_dup("A gangplank runs between the ship and the shore.\r\n");
  ex->keyword = str_dup("gangplank");
  ex->exit_info = 0;
  ex->key = -1;
  ex->to_room = to;
  ex->to_room_vnum = world[to].number;
  world[from].dir_option[dir] = ex;
}


static void remove_exit(long from, int dir, long to)
{
  struct room_direction_data *ex = world[from].dir_option[dir];

  if (!ex || ex->to_room != to)
    return;                             /* not ours */
  if (ex->general_description)
    free(ex->general_description);
  if (ex->keyword)
    free(ex->keyword);
  free(ex);
  world[from].dir_option[dir] = NULL;
}


static void gangplank(int p, int down)
{
  long port = real_room(ports[p].room), deck = real_room(deck_vnum);

  if (port == NOWHERE || deck == NOWHERE)
    return;
  if (down) {
    make_exit(port, ports[p].dir, deck);
    make_exit(deck, WEST, port);
  } else {
    remove_exit(port, ports[p].dir, deck);
    remove_exit(deck, WEST, port);
  }
}


static void restore_crew(void)
{
  long deck = real_room(deck_vnum);

  if (deck != NOWHERE)
    reset_zone(world[deck].zone);       /* the missing crew sign on again */
}


/* ------------------------------------------------------------------ */
/* Captives                                                           */
/* ------------------------------------------------------------------ */

static struct captive *captive_of(long idnum)
{
  int i;

  for (i = 0; i < num_captives; i++)
    if (captives[i].idnum == idnum)
      return &captives[i];
  return NULL;
}


static void drop_captive(struct captive *c)
{
  *c = captives[--num_captives];
}


int ship_captive(struct char_data *ch)
{
  return !IS_NPC(ch) && ch->player_specials->saved.pirate_captive;
}


/* Everything ch has goes into a sea chest in the store, his gold into */
/* the King's coffers, and he into the brig.                           */
static void take_captive(struct char_data *ch)
{
  struct obj_data *chest, *o;
  long store = real_room(STORE), brig = real_room(BRIG);
  int j;
  char text[MAX_STRING_LENGTH];

  if (store == NOWHERE || brig == NOWHERE || num_captives >= MAX_CAPTIVES ||
      !(chest = read_object(SEA_CHEST, VNUMBER)))
    return;
  if (FIGHTING(ch))
    stop_fighting(ch);
  sprintf(text, "a sea chest marked '%s'", GET_NAME(ch));
  chest->short_description = str_dup(text);
  sprintf(text, "A sea chest with '%s' chalked on the lid stands here.", GET_NAME(ch));
  chest->description = str_dup(text);
  GET_OBJ_VAL(chest, 1) = CONT_CLOSED | CONT_LOCKED | CONT_PICKPROOF;

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j))
      obj_to_obj(unequip_char(ch, j), chest);
  while ((o = ch->carrying)) {
    obj_from_char(o);
    obj_to_obj(o, chest);
  }
  obj_to_room(chest, store);
  ch->player_specials->saved.pirate_gold = GET_GOLD(ch);
  ch->player_specials->saved.pirate_captive = 1;
  GET_GOLD(ch) = 0;
  captives[num_captives].idnum = GET_IDNUM(ch);
  captives[num_captives].chest = chest;
  num_captives++;

  send_to_char("Rough hands strip you of everything you own - armour, weapons, pack and "
	       "purse - and bind your wrists.  A sack goes over your head.  You are rowed "
	       "across the water for a long time, then dragged up stone steps and thrown "
	       "down on damp planks.  A door slams.  A key turns.\r\n\r\n", ch);
  char_from_room(ch);
  char_to_room(ch, brig);
  GET_POS(ch) = POS_SITTING;
  act("The cell door bangs open and $n is thrown in, and the door slams again.",
      TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  save_char(ch, NOWHERE);
  Crash_crashsave(ch);
}


/* Give a captive his things back (to his inventory) and set him free. */
void ship_release(struct char_data *ch, int quietly)
{
  struct captive *c;
  struct obj_data *o;

  if (IS_NPC(ch) || !ch->player_specials->saved.pirate_captive)
    return;
  if ((c = captive_of(GET_IDNUM(ch)))) {
    if (c->chest) {
      while ((o = c->chest->contains)) {
	obj_from_obj(o);
	obj_to_char(o, ch);
      }
      extract_obj(c->chest);
    }
    drop_captive(c);
  }
  GET_GOLD(ch) += ch->player_specials->saved.pirate_gold;
  ch->player_specials->saved.pirate_gold = 0;
  ch->player_specials->saved.pirate_captive = 0;
  if (!quietly)
    send_to_char("You have your things back.\r\n", ch);
}


/* Renting, quitting or idling out: the save that follows writes the */
/* captive's things and takes them out of the game, so hand them to   */
/* him and forget the chest.  He stays a captive in his player file,  */
/* and ship_login() ransoms him when he comes back.                   */
void ship_rent(struct char_data *ch)
{
  struct captive *c;
  struct obj_data *o;

  if (IS_NPC(ch) || !ch->player_specials->saved.pirate_captive ||
      !(c = captive_of(GET_IDNUM(ch))))
    return;
  if (c->chest) {
    while ((o = c->chest->contains)) {
      obj_from_obj(o);
      obj_to_char(o, ch);
    }
    extract_obj(c->chest);
  }
  drop_captive(c);
}


/* The crash save writes a captive's chest as if he carried it: lend */
/* it to him (TRUE if there was anything), then take it back.         */
int ship_lend_gear(struct char_data *ch, int lend)
{
  struct captive *c;
  struct obj_data *o;
  int i;

  if (IS_NPC(ch) || !ch->player_specials->saved.pirate_captive ||
      !(c = captive_of(GET_IDNUM(ch))) || !c->chest)
    return FALSE;
  if (lend) {
    num_lent = 0;
    while ((o = c->chest->contains) && num_lent < MAX_LENT) {
      obj_from_obj(o);
      obj_to_char(o, ch);
      lent[num_lent++] = o;
    }
    return num_lent > 0;
  }
  for (i = 0; i < num_lent; i++)
    if (lent[i]->carried_by == ch) {
      obj_from_char(lent[i]);
      obj_to_obj(lent[i], c->chest);
    }
  num_lent = 0;
  return TRUE;
}


/* Logging in: a captive whose chest is gone (a reboot, a crash) has */
/* his things already (the crash file) and gets his gold back.        */
void ship_login(struct char_data *ch)
{
  long haven = real_room(haven_vnum);

  if (IS_NPC(ch) || !ch->player_specials->saved.pirate_captive ||
      captive_of(GET_IDNUM(ch)))
    return;
  ship_release(ch, TRUE);
  if (haven != NOWHERE) {
    char_from_room(ch);
    char_to_room(ch, haven);
  }
  send_to_char("\r\nYou wake on the docks of Vice Island with a sore head.  Somebody paid "
	       "your ransom, and the pirates gave back everything they took.\r\n\r\n", ch);
}


/* A captive about to die: his things go back to him (and so into his */
/* corpse), and the corpse washes ashore at Vice Island.               */
long ship_corpse_room(struct char_data *ch)
{
  long haven = real_room(haven_vnum);

  if (!IS_NPC(ch) && ch->player_specials->saved.pirate_captive) {
    ship_release(ch, TRUE);
    washed_ashore = GET_IDNUM(ch);
  }
  if (!IS_NPC(ch) && washed_ashore == GET_IDNUM(ch) && haven != NOWHERE) {
    washed_ashore = 0;
    return haven;
  }
  return ch->in_room;
}


/* ------------------------------------------------------------------ */
/* Voyages                                                            */
/* ------------------------------------------------------------------ */

static int king_hunter_aboard(void)
{
  struct char_data *list[64];
  int n = players_aboard(list, 64), i;

  for (i = 0; i < n; i++)
    if (quest_on(list[i], KING_QUEST))
      return TRUE;
  return FALSE;
}


static void depart(void)
{
  struct char_data *list[64];
  long port = real_room(ports[at].room);
  int n, i, next = at + heading;
  char text[MAX_STRING_LENGTH];

  if (next < 0 || next >= num_ports) {
    heading = -heading;
    next = at + heading;
  }
  /* the fares */
  n = players_aboard(list, 64);
  for (i = 0; i < n; i++) {
    if (GET_GOLD(list[i]) >= fare) {
      GET_GOLD(list[i]) -= fare;
      sprintf(text, "The purser collects your fare: %d gold for the passage to %s.\r\n",
	      fare, ports[next].name);
      send_to_char(text, list[i]);
    } else if (port != NOWHERE) {
      send_to_char("The purser looks at your empty purse, takes you by the elbow and "
		   "walks you down the gangplank.\r\n", list[i]);
      act("The purser walks $n down the gangplank.", TRUE, list[i], 0, 0, TO_ROOM);
      if (FIGHTING(list[i]))
	stop_fighting(list[i]);
      char_from_room(list[i]);
      char_to_room(list[i], port);
      look_at_room(list[i], 0);
    }
  }
  gangplank(at, FALSE);
  sprintf(text, "The gangplank is hauled aboard, the lines are cast off, and the %s "
	  "stands out to sea, bound for %s.\r\n", ship_name, ports[next].name);
  to_ship(text);
  sprintf(text, "The %s casts off and stands out to sea, bound for %s.\r\n",
	  ship_name, ports[next].name);
  to_port(at, text);

  bound = next;
  cur_leg = MIN(at, next);
  hours_left = legs[cur_leg].hours;
  hour_in_leg = 0;
  state = SHIP_SAILING;
  raid = RAID_NONE;
  raid_at = 0;
  if (legs[cur_leg].cape && king_hunter_aboard())
    raid_at = -1;                       /* -1: the King, in the first hour */
  else if (legs[cur_leg].chance && number(1, 100) <= legs[cur_leg].chance)
    raid_at = number(1, MAX(1, hours_left - 1));
}


static void arrive(int p)
{
  char text[MAX_STRING_LENGTH];

  at = p;
  state = SHIP_DOCKED;
  hours_left = docked_hours;
  waited = 0;
  gangplank(at, TRUE);
  restore_crew();
  sprintf(text, "The %s glides in to %s.  Lines are thrown ashore, and the gangplank "
	  "goes down over the west rail.\r\n", ship_name, ports[at].name);
  to_ship(text);
  sprintf(text, "The %s comes in under shortened sail and ties up here; her gangplank "
	  "goes down, %s of you.\r\n", ship_name, dir_names[ports[at].dir]);
  to_port(at, text);
}


/* ------------------------------------------------------------------ */
/* Raids                                                              */
/* ------------------------------------------------------------------ */

/* level > 0: an ordinary raid, sized to the passengers; party > 1: a boss */
static void board(long vnum, long deck, int level, int party)
{
  struct char_data *m;

  if (!(m = read_mobile(vnum, VNUMBER)))
    return;
  if (level > 0)
    mob_set_level(m, level, party);
  char_to_room(m, deck);
  act("$n swings over the rail on a rope and lands on the deck, blade out!",
      FALSE, m, 0, 0, TO_ROOM);
}


static void start_raid(int type)
{
  struct char_data *list[64];
  long deck = real_room(deck_vnum);
  int n = players_aboard(list, 64), i;

  raid = type;
  raid_pulses = 0;
  surrendered = 0;
  if (type == RAID_KING) {
    to_ship("\r\nOut of the haze off the cape comes a great ship with blood-red sails, a "
	    "black wake flying at her masthead: the Red Maw!  The bell clangs madly.  "
	    "At her rail stands a tall man in a red coat, laughing - Morrow Blackwake, "
	    "the Pirate King.  Grappling hooks bite into the Sea Wren's rail.\r\n");
    to_ship("Blackwake's voice carries across the water: 'Strike your colours, Halloran!  "
	    "Surrender, and nobody need die today!'\r\n\r\n");
    for (i = 0; i < MIN(8, 3 + n); i++)
      board(REAVER, deck, 0, 0);
  } else {
    to_ship("\r\nA cry from the rigging: 'SAIL HO!  Pirates!'  A black-sailed sloop "
	    "comes racing out of the swell, and grappling hooks thud into the rail.  "
	    "The crew snatch up cutlasses and belaying pins.\r\n\r\n");
    /* pirates of the passengers' level (their average, 5 to 60); the */
    /* bosun is a boss for two, and the crew fight alongside them      */
    int lvl = 0;

    for (i = 0; i < n; i++)
      lvl += GET_LEVEL(list[i]);
    lvl = n ? lvl / n : 20;
    lvl = MAX(5, MIN(60, lvl));
    board(BOSUN, deck, lvl + 1, 2);
    for (i = 0; i < MIN(5, 1 + n); i++)
      board(number(0, 1) ? BOARDER : CUTTHROAT, deck, MAX(1, lvl - number(0, 2)), 1);
  }
}


static void end_raid(void)
{
  struct char_data *c, *next;

  for (c = character_list; c; c = next) {
    next = c->next;
    if (is_pirate(c) && aboard_room(c->in_room)) {
      if (FIGHTING(c))
	stop_fighting(c);
      act("$n grabs a rope and swings back over the rail.", TRUE, c, 0, 0, TO_ROOM);
      extract_char(c);
    }
  }
  raid = RAID_NONE;
}


static void plunder(void)
{
  struct char_data *list[64], *c;
  int n = players_aboard(list, 64), i;
  long take;
  char text[MAX_STRING_LENGTH];

  if (!find_aboard(CAPTAIN))
    to_ship("\r\nThe captain is down, and the crew throw down their arms.  The pirates "
	    "go through the passengers' pockets, laughing.\r\n");
  else
    to_ship("\r\nThe pirate bosun's whistle shrills: enough!  The boarders fall back to "
	    "the rail, snatching purses as they go.\r\n");
  for (i = 0; i < n; i++) {
    c = list[i];
    if (FIGHTING(c))
      stop_fighting(c);
    if ((take = GET_GOLD(c) / 10) > 0) {
      GET_GOLD(c) -= take;
      sprintf(text, "A pirate cuts your purse strings and helps himself to %ld gold.\r\n", take);
      send_to_char(text, c);
    }
  }
  end_raid();
  to_ship("The pirate sloop sheers off with her plunder, and the Sea Wren limps on.\r\n");
}


static void capture(void)
{
  struct char_data *list[64];
  int n = players_aboard(list, 64), i;

  to_ship("\r\nCaptain Gray lowers his cutlass.  'Strike the colours,' he says heavily, "
	  "and the Sea Wren's flag comes down.  Blackwake's reavers swarm over the "
	  "deck, and Morrow Blackwake himself strolls across a plank from the Red Maw, "
	  "smiling.  'Passengers to the boats,' he says.  'They're worth more than the "
	  "cargo.'\r\n\r\n");
  for (i = 0; i < n; i++)
    if (valid_char(list[i]))
      take_captive(list[i]);
  end_raid();
  gangplank(at, FALSE);
  state = SHIP_CAPTURED;
  hours_left = captured_hours;
}


static void raid_pulse(void)
{
  struct char_data *pirates[32], *c, *target, *pick;
  long v, r;
  int np = 0, i, n, crew = 0;

  raid_pulses++;
  for (c = character_list; c && np < 32; c = c->next)
    if (is_pirate(c) && aboard_room(c->in_room))
      pirates[np++] = c;
    else if (is_crew(c) && aboard_room(c->in_room))
      crew++;

  if (!np) {                            /* beaten off */
    if (raid == RAID_KING)
      to_ship("\r\nThe last of the reavers goes down.  Across the water Blackwake shrugs "
	      "and calls, 'Another day, then, Halloran!'  The Red Maw cuts her grapples and "
	      "falls away astern.\r\n");
    else
      to_ship("\r\nThe last of the boarders goes down, and the pirate sloop cuts her lines "
	      "and flees.  The crew cheer themselves hoarse.\r\n");
    raid = RAID_NONE;
    return;
  }
  if (raid == RAID_KING &&
      (surrendered || !find_aboard(CAPTAIN) || raid_pulses > 150)) {
    capture();
    return;
  }
  if (raid == RAID_PIRATES && (!find_aboard(CAPTAIN) || raid_pulses > 120)) {
    plunder();
    return;
  }

  /* the fighting: idle pirates find someone to cut, idle crew pile in */
  for (i = 0; i < np; i++) {
    c = pirates[i];
    if (!valid_char(c) || FIGHTING(c) || GET_POS(c) < POS_FIGHTING)
      continue;
    pick = NULL;
    n = 0;
    for (v = SHIP_LO; v <= SHIP_HI; v++) {
      if ((r = real_room(v)) == NOWHERE)
	continue;
      for (target = world[r].people; target; target = target->next_in_room)
	if ((is_crew(target) || (!IS_NPC(target) && GET_LEVEL(target) < LVL_IMMORT)) &&
	    GET_POS(target) > POS_DEAD && (r == c->in_room ? !number(0, n++) : !n && !pick &&
					     !number(0, 3)))
	  pick = target;
    }
    if (!pick)
      continue;
    if (pick->in_room != c->in_room) {
      act("$n runs off in search of someone to fight.", TRUE, c, 0, 0, TO_ROOM);
      char_from_room(c);
      char_to_room(c, pick->in_room);
      act("$n comes running, blade out!", TRUE, c, 0, 0, TO_ROOM);
    }
    hit(c, pick, TYPE_UNDEFINED);
  }
  for (c = character_list; c; c = c->next)
    if (is_crew(c) && aboard_room(c->in_room) && !FIGHTING(c) && AWAKE(c) &&
	GET_MOB_VNUM(c) != PURSER)
      for (target = world[c->in_room].people; target; target = target->next_in_room)
	if (is_pirate(target)) {
	  hit(c, target, TYPE_UNDEFINED);
	  break;
	}
  (void) crew;
}


/* ------------------------------------------------------------------ */
/* The clock                                                          */
/* ------------------------------------------------------------------ */

void ship_hour(void)
{
  char text[MAX_STRING_LENGTH];
  long port;
  int next;

  if (num_ports < 2)
    return;
  switch (state) {
  case SHIP_DOCKED:
    if (--hours_left > 0) {
      if (hours_left == 1) {
	next = at + heading;
	if (next < 0 || next >= num_ports)
	  next = at - heading;
	sprintf(text, "The purser calls: 'All aboard for %s!  We sail within the hour.  "
		"%d gold the passage!'\r\n", ports[next].name, fare);
	to_ship(text);
	to_port(at, text);
      }
      break;
    }
    port = real_room(ports[at].room);
    if (port != NOWHERE && weather_downpour(world[port].zone) && waited < 2) {
      waited++;
      hours_left = 1;
      to_ship("Captain Gray looks at the sky and shakes his head.  'We'll not put out in "
	      "this.  We wait.'\r\n");
      break;
    }
    depart();
    break;

  case SHIP_SAILING:
    hour_in_leg++;
    if (raid == RAID_NONE && raid_at && (raid_at == -1 || raid_at == hour_in_leg)) {
      start_raid(raid_at == -1 ? RAID_KING : RAID_PIRATES);
      raid_at = 0;
    } else if (raid == RAID_NONE) {
      if (legs[cur_leg].cape && num_cape)
	sprintf(text, "%s\r\n", cape_lines[number(0, num_cape - 1)]);
      else if (num_sea)
	sprintf(text, "%s\r\n", sea_lines[number(0, num_sea - 1)]);
      else
	*text = '\0';
      to_ship(text);
    }
    if (--hours_left <= 0) {
      if (raid != RAID_NONE)
	hours_left = 1;                 /* still fighting */
      else
	arrive(bound);
    }
    break;

  case SHIP_CAPTURED:
    if (--hours_left <= 0) {
      int i;

      for (i = 0; i < num_ports; i++)
	if (ports[i].room == haven_vnum)
	  break;
      if (i >= num_ports)
	i = 0;
      heading = (i == num_ports - 1) ? -1 : 1;
      arrive(i);
      to_port(at, "Her flag is torn and her hold is empty: she has been ransomed from "
	      "the pirates.\r\n");
    }
    break;
  }
}


/* Every 2 seconds (comm.c). */
void ship_pulse(void)
{
  if (raid != RAID_NONE)
    raid_pulse();
}


/* ------------------------------------------------------------------ */
/* Commands and specials                                              */
/* ------------------------------------------------------------------ */

ACMD(do_surrender)
{
  if (!aboard_room(ch->in_room) || raid == RAID_NONE) {
    send_to_char("There is nobody here to surrender to.\r\n", ch);
    return;
  }
  if (raid == RAID_PIRATES) {
    send_to_char("The pirates laugh at you.  'We don't take prisoners, mate.  We take "
		 "PURSES!'\r\n", ch);
    return;
  }
  if (FIGHTING(ch))
    stop_fighting(ch);
  send_to_char("You throw down your weapon and raise your hands.\r\n", ch);
  act("$n throws down $s weapon and raises $s hands.", TRUE, ch, 0, 0, TO_ROOM);
  to_ship("Blackwake roars with laughter from the Red Maw: 'Wise!  WISE!  Hold, lads!'\r\n");
  surrendered = 1;
}


static void timetable(char *out)
{
  int next = at + heading;

  if (next < 0 || next >= num_ports)
    next = at - heading;
  if (state == SHIP_DOCKED)
    sprintf(out, "We are at %s, and sail for %s in about %d hour%s.  The passage is %d "
	    "gold a port, paid as we cast off.", ports[at].name, ports[next].name,
	    hours_left, hours_left == 1 ? "" : "s", fare);
  else if (state == SHIP_SAILING)
    sprintf(out, "We are bound for %s, about %d hour%s off.", ports[bound].name,
	    hours_left, hours_left == 1 ? "" : "s");
  else
    strcpy(out, "We are in the hands of pirates, and you are asking me about timetables?");
}


SPECIAL(ship_purser)
{
  struct char_data *purser = (struct char_data *) me;
  char out[512], text[MAX_STRING_LENGTH];
  int i;

  if (!CMD_IS("list") || !AWAKE(purser) || FIGHTING(purser))
    return FALSE;
  timetable(out);
  sprintf(text, "$n consults his ledger.  '%s  Our ports: ", out);
  for (i = 0; i < num_ports; i++)
    sprintf(text + strlen(text), "%s%s", ports[i].name, i < num_ports - 1 ? "; " : ".'");
  act(text, FALSE, purser, 0, ch, TO_VICT);
  return TRUE;
}


/* The store on the isle: OPEN CHEST opens the one with your name on it. */
SPECIAL(ship_store)
{
  char arg[MAX_INPUT_LENGTH];
  struct captive *c;

  if (!CMD_IS("open") && !CMD_IS("unlock"))
    return FALSE;
  one_argument(argument, arg);
  if (!*arg || !isname(arg, "chest chests sea"))
    return FALSE;
  if (IS_NPC(ch) || !(c = captive_of(GET_IDNUM(ch))) || !c->chest ||
      c->chest->in_room != ch->in_room) {
    send_to_char("None of these chests has your name on it, and they are all locked.\r\n", ch);
    return TRUE;
  }
  act("You find the chest with your name chalked on the lid, break the cheap lock and "
      "take back everything that is yours.", FALSE, ch, 0, 0, TO_CHAR);
  act("$n breaks open a sea chest and takes back $s belongings.", TRUE, ch, 0, 0, TO_ROOM);
  ship_release(ch, FALSE);
  save_char(ch, NOWHERE);
  return TRUE;
}


void assign_ships(void)
{
  ASSIGNMOB(PURSER, ship_purser);
  ASSIGNROOM(STORE, ship_store);
}


/* "show ships [sail|arrive|raid|king|free]" for immortals. */
void ship_show(struct char_data *ch, char *arg)
{
  static const char *states[] = { "docked", "sailing", "captured" };
  static const char *raids[] = { "none", "pirates", "the King" };
  char out[512];

  if (num_ports < 2) {
    send_to_char("No ship is sailing.\r\n", ch);
    return;
  }
  if (!str_cmp(arg, "sail") && state == SHIP_DOCKED)
    depart();
  else if (!str_cmp(arg, "arrive") && state == SHIP_SAILING && raid == RAID_NONE)
    arrive(bound);
  else if (!str_cmp(arg, "raid") && state == SHIP_SAILING && raid == RAID_NONE)
    start_raid(RAID_PIRATES);
  else if (!str_cmp(arg, "king") && state == SHIP_SAILING && raid == RAID_NONE)
    start_raid(RAID_KING);
  else if (!str_cmp(arg, "free") && raid != RAID_NONE)
    end_raid();
  timetable(out);
  sprintf(buf, "The %s: %s, raid %s (%d pulses), %d captives.\r\n%s\r\n", ship_name,
	  states[state], raids[raid], raid_pulses, num_captives, out);
  send_to_char(buf, ch);
}
