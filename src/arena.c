/* ***********************************************************************\
*  File:  ARENA.C                                                         *
*  Usage: Scheduled bouts, betting and challenges at the Arena (zone 53)  *
*  Programmer(s): Written for The Dominion, November 2026                 *
\*********************************************************************** */

/*
 * Bouts.  At the hours listed in lib/data/arena two of the Arena's
 * fighters meet in the ring (room 5471).  Two hours before, the card is
 * posted: the two are brought up into the pens (5472), the odds are worked
 * out from the fighters themselves (win_chance(), a few hundred simulated
 * fights by fight.c's rules), and betting opens.  At the bell the two are
 * put in the ring and fight until one dies, or the judges give it to the
 * one less hurt.  Everyone in the stands, the booth and at the gates
 * hears the crowd and the herald.
 *
 * Betting.  The bookmaker (5318) in his booth (5470, below the bleachers)
 * takes bets while the card is up: BET <amount> <fighter>, ODDS, BETS.
 * The minimum stake is 10 gold and there is no maximum.  The board in the
 * booth shows the odds as a bookmaker would ("7 to 4"), always a little
 * shorter than the true chances (the house edge in lib/data/arena), so
 * over time the house wins.  A stake is taken when the bet is placed:
 * losing stakes simply vanish (the bookmaker holds no gold, so there is
 * nothing to take from him; and he is level 105).  Winners are paid at
 * the end of the bout wherever they are, or when they next come into the
 * game (until a reboot).  If both fighters fall, stakes are returned.
 *
 * Challenges.  The arena master (5319) at the Northern Gate matches a
 * player against a gladiator (5330) of his own level: CHALLENGE NOVICE,
 * VETERAN or CHAMPION, and GOLD or WEAPON for the prize.  It is not to the
 * death: a beaten challenger is pulled out at one hit point
 * (arena_spare(), called from damage()).  The prize is gold, or one of
 * the prize weapons (lib/data/arena) of his level; challenges can't be
 * fought while a card is up, and each player may challenge once every
 * few minutes.
 */

#define __ARENA_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct time_info_data time_info;
extern struct str_app_type str_app[];
extern struct dex_app_type dex_app[];

void ASSIGNMOB(long mob, SPECIAL(fname));
void ASSIGNROOM(long room, SPECIAL(fname));
void look_at_room(struct char_data *ch, int mode);
void hit(struct char_data *ch, struct char_data *victim, int type);

#define ARENA_FILE     "data/arena"

#define BOOKIE         5318
#define MASTER         5319
#define GLADIATOR      5330
#define BOOTH          5470
#define RING           5471
#define PENS           5472
#define GATE           5320

#define MAX_FIGHTERS   32
#define MAX_PRIZES     24
#define MAX_WATCH      64
#define MAX_BETS       256
#define MAX_OWED       64
#define MAX_COOL       64

#define ARENA_IDLE     0
#define ARENA_CARD     1                /* posted; betting open           */
#define ARENA_BOUT     2                /* in the ring                    */

#define BOUT_PULSES    90               /* 3 minutes, then the judges     */
#define CHALLENGE_PULSES 120
#define COOLDOWN_SECS  300

static int bout_hour[24], num_bout_hours = 0;
static int edge = 12, min_bet = 10, card_lead = 2;
static long fighters[MAX_FIGHTERS];
static int num_fighters = 0;
static long watch[MAX_WATCH];
static int num_watch = 0;
static struct { int level; long vnum; } prizes[MAX_PRIZES];
static int num_prizes = 0;

static int state = ARENA_IDLE, bout_at = -1, pulses = 0, delayed = 0;
static struct char_data *corner[2] = { NULL, NULL };
static char *corner_name[2] = { NULL, NULL };
static int chance_a = 500;             /* per mille: corner 0 wins        */
static int odds_num[2], odds_den[2];   /* the board: profit num/den        */
static int hurt[2];                    /* last reported quarter            */

struct bet { long idnum; int side; long stake; };
static struct bet bets[MAX_BETS];
static int num_bets = 0;
static struct { long idnum; long gold; } owed[MAX_OWED];
static int num_owed = 0;
static struct { long idnum; time_t until; } cool[MAX_COOL];
static int num_cool = 0;

/* the challenge */
static struct char_data *challenger = NULL, *gladiator = NULL;
static long chal_id = 0;
static int chal_tier = 0, chal_weapon = 0, chal_pulses = 0, chal_lost = 0;
static const char *tiers[] = { "novice", "veteran", "champion", "\n" };


/* ------------------------------------------------------------------ */
/* Loading                                                            */
/* ------------------------------------------------------------------ */

static void arena_error(int line, const char *msg)
{
  sprintf(buf, "SYSERR: " ARENA_FILE " line %d: %s", line, msg);
  log(buf);
}


void load_arena(void)
{
  FILE *fl;
  char line[MAX_STRING_LENGTH], word[MAX_INPUT_LENGTH], *p;
  int lineno = 0, a, b;
  long r;

  num_bout_hours = num_fighters = num_watch = num_prizes = 0;
  if (!(fl = fopen(ARENA_FILE, "r"))) {
    log("No arena file (" ARENA_FILE "); no bouts at the Arena.");
    return;
  }
  while (fgets(line, sizeof(line), fl)) {
    lineno++;
    if ((p = strchr(line, '\n')))
      *p = '\0';
    p = line;
    skip_spaces(&p);
    if (!*p || *p == ';')
      continue;
    if (*p == '$')
      break;
    p = one_argument(p, word);
    if (!str_cmp(word, "bouts")) {
      while (*p) {
	p = one_argument(p, word);
	if (*word && num_bout_hours < 24 && (a = atoi(word)) >= 0 && a < 24)
	  bout_hour[num_bout_hours++] = a;
      }
    } else if (!str_cmp(word, "edge"))
      edge = MAX(1, MIN(50, atoi(p)));
    else if (!str_cmp(word, "minbet"))
      min_bet = MAX(1, atoi(p));
    else if (!str_cmp(word, "card"))
      card_lead = MAX(1, MIN(6, atoi(p)));
    else if (!str_cmp(word, "fighter")) {
      if (num_fighters < MAX_FIGHTERS && real_mobile(atol(p)) >= 0)
	fighters[num_fighters++] = atol(p);
      else
	arena_error(lineno, "no such fighter, or too many");
    } else if (!str_cmp(word, "watch")) {
      while (*p) {
	p = one_argument(p, word);
	if (!*word)
	  break;
	a = b = atoi(word);
	if (strchr(word, '-'))
	  sscanf(word, "%d-%d", &a, &b);
	for (; a <= b && num_watch < MAX_WATCH; a++)
	  if (real_room(a) != NOWHERE)
	    watch[num_watch++] = a;
      }
    } else if (!str_cmp(word, "prize")) {
      if (num_prizes < MAX_PRIZES && sscanf(p, "%d %ld", &a, &r) == 2 &&
	  real_object(r) >= 0) {
	prizes[num_prizes].level = a;
	prizes[num_prizes].vnum = r;
	obj_level_set(real_object(r), a);       /* levels.c */
	num_prizes++;
      } else
	arena_error(lineno, "prize <level> <object vnum>");
    } else
      arena_error(lineno, "unknown line");
  }
  fclose(fl);
  if (num_fighters < 2 || !num_bout_hours || real_room(RING) == NOWHERE ||
      real_room(PENS) == NOWHERE) {
    log("SYSERR: " ARENA_FILE ": the Arena needs a ring, pens, bout hours and two fighters.");
    num_bout_hours = 0;
  }
  sprintf(buf, "Loaded the Arena: %d fighters, %d bouts a day, %d prizes.",
	  num_fighters, num_bout_hours, num_prizes);
  log(buf);
}


/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static int valid_char(struct char_data *c)
{
  struct char_data *x;

  for (x = character_list; x; x = x->next)
    if (x == c)
      return TRUE;
  return FALSE;
}


static struct char_data *player_by_id(long idnum)
{
  struct char_data *c;

  for (c = character_list; c; c = c->next)
    if (!IS_NPC(c) && GET_IDNUM(c) == idnum)
      return c;
  return NULL;
}


static void to_watchers(const char *text)
{
  long r;
  int i;

  for (i = 0; i < num_watch; i++)
    if ((r = real_room(watch[i])) != NOWHERE && world[r].people)
      send_to_room((char *) text, r);
  if ((r = real_room(RING)) != NOWHERE && world[r].people)
    send_to_room((char *) text, r);
}


/* name with a capital, for the start of a sentence */
static const char *cap(const char *name)
{
  static char out[2][MAX_INPUT_LENGTH];
  static int which = 0;

  which = !which;
  strncpy(out[which], name ? name : "", MAX_INPUT_LENGTH - 1);
  out[which][MAX_INPUT_LENGTH - 1] = '\0';
  *out[which] = UPPER(*out[which]);
  return out[which];
}


static const char *hour_name(int h)
{
  static char out[32];

  sprintf(out, "the %d%s hour of the %s", ((h % 12 == 0) ? 12 : h % 12),
	  ((h % 12 == 1) ? "st" : (h % 12 == 2) ? "nd" : (h % 12 == 3) ? "rd" : "th"),
	  h < 12 ? "morning" : "evening");
  return out;
}


/* The next bout hour after (or at, if include) hour h; -1 if none. */
static int next_bout(int h, int include)
{
  int i, d, best = -1, bd = 99;

  for (i = 0; i < num_bout_hours; i++) {
    d = (bout_hour[i] - h + 24) % 24;
    if (!include && !d)
      d = 24;
    if (d < bd) {
      bd = d;
      best = bout_hour[i];
    }
  }
  return best;
}


/* The odds a bookmaker would post for a true chance p (per mille) less  */
/* the house edge: the largest of the usual prices whose profit is no    */
/* more than the fair one.                                               */
static void price(int p, int *num, int *den)
{
  static const int list[][2] = {
    {1, 20}, {1, 10}, {1, 8}, {1, 6}, {1, 5}, {2, 9}, {1, 4}, {2, 7}, {1, 3}, {4, 11},
    {2, 5}, {4, 9}, {1, 2}, {8, 15}, {4, 7}, {8, 13}, {4, 6}, {8, 11}, {4, 5}, {5, 6},
    {10, 11}, {1, 1}, {11, 10}, {6, 5}, {5, 4}, {11, 8}, {6, 4}, {13, 8}, {7, 4},
    {15, 8}, {2, 1}, {9, 4}, {5, 2}, {11, 4}, {3, 1}, {10, 3}, {7, 2}, {4, 1},
    {9, 2}, {5, 1}, {6, 1}, {7, 1}, {8, 1}, {10, 1}, {12, 1}, {16, 1}, {20, 1},
    {25, 1}, {33, 1}, {50, 1}
  };
  double fair = 1000.0 / MAX(1, p) * (100 - edge) / 100.0 - 1;   /* profit */
  int i, n = sizeof(list) / sizeof(list[0]);

  *num = list[0][0];
  *den = list[0][1];
  for (i = 0; i < n; i++)
    if ((double) list[i][0] / list[i][1] <= fair) {
      *num = list[i][0];
      *den = list[i][1];
    }
}


/* TRUE if the price on side is short of the true chance (it always */
/* is, unless a fighter is an overwhelming favourite).              */
static int house_ahead(int side)
{
  int p = side ? 1000 - chance_a : chance_a;

  return (double) odds_num[side] / odds_den[side] < 1000.0 / MAX(1, p) - 1;
}


static long payout(long stake, int side)
{
  return stake + stake * odds_num[side] / odds_den[side];
}


/* A fighter's numbers for the simulation, as hit() uses them. */
struct sim { double hp, def, hr, ac, dmg; int n, size, todam, attacks4, sanct; };

static void sim_of(struct char_data *m, struct sim *s)
{
  int i, ac = 0;

  s->hp = GET_HIT(m);
  s->def = dex_app[GET_DEX(m)].defensive;
  s->hr = GET_HITROLL(m);
  for (i = 0; i < ARMOR_LIMIT; i++)
    ac += GET_AC(m, i);
  s->ac = (double) ac / ARMOR_LIMIT;
  s->n = m->mob_specials.damnodice;
  s->size = m->mob_specials.damsizedice;
  s->todam = str_app[STRENGTH_APPLY_INDEX(m)].todam;
  s->attacks4 = IS_AFFECTED(m, AFF_HASTE) ? 100 : 20;
  s->sanct = IS_AFFECTED(m, AFF_SANCTUARY);
}


static int sim_round(struct sim *a, struct sim *b)
{
  int swings = 1 + (number(1, 100) <= a->attacks4), i, d, tot = 0;

  for (i = 0; i < swings; i++) {
    if (number(1, 100) + a->hr < b->def)
      continue;
    d = a->todam + dice(a->n, a->size);
    if (b->sanct)
      d >>= 1;
    d -= (int) (d * b->ac / 100);
    tot += MAX(1, d);
  }
  return tot;
}


/* Per mille: how often a beats b, over a few hundred fights. */
static int win_chance(struct char_data *a, struct char_data *b)
{
  struct sim sa, sb;
  double ha, hb;
  int i, r, wins = 0, runs = 400;

  sim_of(a, &sa);
  sim_of(b, &sb);
  for (i = 0; i < runs; i++) {
    ha = sa.hp;
    hb = sb.hp;
    for (r = 0; r < BOUT_PULSES && ha > 0 && hb > 0; r++) {
      hb -= sim_round(&sa, &sb);
      if (hb <= 0)
	break;
      ha -= sim_round(&sb, &sa);
    }
    if (hb <= 0 || (ha > 0 && ha / sa.hp > hb / sb.hp))
      wins++;
  }
  return MAX(1, MIN(999, wins * 1000 / runs));
}


static void remove_fighter(struct char_data *m)
{
  if (m && valid_char(m)) {
    if (FIGHTING(m))
      stop_fighting(m);
    extract_char(m);
  }
}


static void clear_ring_floor(void)
{
  struct obj_data *o, *next;
  long r = real_room(RING);

  if (r == NOWHERE)
    return;
  for (o = world[r].contents; o; o = next) {
    next = o->next_content;
    extract_obj(o);                     /* corpses and dropped gear */
  }
}


/* ------------------------------------------------------------------ */
/* Bouts and bets                                                     */
/* ------------------------------------------------------------------ */

static void post_card(int hour)
{
  long pens = real_room(PENS);
  int tries, a, b, i;
  char text[MAX_STRING_LENGTH];

  if (pens == NOWHERE)
    return;
  for (tries = 0; tries < 12; tries++) {
    a = number(0, num_fighters - 1);
    do {
      b = number(0, num_fighters - 1);
    } while (b == a);
    corner[0] = read_mobile(fighters[a], VNUMBER);
    corner[1] = read_mobile(fighters[b], VNUMBER);
    if (!corner[0] || !corner[1]) {
      if (corner[0]) extract_char(corner[0]);
      if (corner[1]) extract_char(corner[1]);
      corner[0] = corner[1] = NULL;
      return;
    }
    char_to_room(corner[0], pens);
    char_to_room(corner[1], pens);
    chance_a = win_chance(corner[0], corner[1]);
    if ((chance_a >= 150 && chance_a <= 850) || tries == 11)
      break;
    extract_char(corner[0]);            /* too one-sided: another pairing */
    extract_char(corner[1]);
  }
  for (i = 0; i < 2; i++) {
    if (corner_name[i])
      free(corner_name[i]);
    corner_name[i] = str_dup(GET_NAME(corner[i]));
    hurt[i] = 4;
  }
  price(chance_a, &odds_num[0], &odds_den[0]);
  price(1000 - chance_a, &odds_num[1], &odds_den[1]);
  num_bets = 0;
  bout_at = hour;
  state = ARENA_CARD;
  sprintf(text, "\r\nA herald bawls from the wall: 'At %s: %s against %s!  "
	  "Bets at the bookmaker's booth below the stands!'\r\n",
	  hour_name(hour), corner_name[0], corner_name[1]);
  (void) cap;
  to_watchers(text);
}


static void start_bout(void)
{
  long ring = real_room(RING);
  int i;

  for (i = 0; i < 2; i++)
    if (!corner[i] || !valid_char(corner[i]) || ring == NOWHERE) {
      to_watchers("\r\nThe herald announces that the bout is off.  Stakes are returned.\r\n");
      for (i = 0; i < num_bets; i++) {
	struct char_data *c = player_by_id(bets[i].idnum);

	if (c) {
	  GET_GOLD(c) += bets[i].stake;
	  send_to_char("The bookmaker's runner returns your stake.\r\n", c);
	}
      }
      num_bets = 0;
      remove_fighter(corner[0]);
      remove_fighter(corner[1]);
      corner[0] = corner[1] = NULL;
      state = ARENA_IDLE;
      return;
    }
  for (i = 0; i < 2; i++) {
    char_from_room(corner[i]);
    char_to_room(corner[i], ring);
  }
  sprintf(buf, "\r\nThe gates of the pens crash open and %s and %s stride into the ring.  "
	  "The bell rings, betting is closed, and the crowd comes to its feet!\r\n",
	  corner_name[0], corner_name[1]);
  to_watchers(buf);
  state = ARENA_BOUT;
  pulses = 0;
  hit(corner[0], corner[1], TYPE_UNDEFINED);
}


static void settle(int winner)          /* -1: no contest */
{
  struct char_data *c;
  long pay;
  int i, j;

  for (i = 0; i < num_bets; i++) {
    if (winner >= 0 && bets[i].side != winner)
      continue;                         /* the stake is gone */
    pay = (winner < 0) ? bets[i].stake : payout(bets[i].stake, winner);
    if ((c = player_by_id(bets[i].idnum))) {
      GET_GOLD(c) += pay;
      sprintf(buf, "A runner from the bookmaker presses %ld gold into your hand%s.\r\n",
	      pay, winner < 0 ? " - your stake back" : ": your winnings");
      send_to_char(buf, c);
    } else {
      for (j = 0; j < num_owed && owed[j].idnum != bets[i].idnum; j++);
      if (j == num_owed && num_owed < MAX_OWED) {
	owed[num_owed].idnum = bets[i].idnum;
	owed[num_owed++].gold = 0;
      }
      if (j < MAX_OWED)
	owed[j].gold += pay;
    }
  }
  num_bets = 0;
}


static void end_bout(int winner)
{
  char text[MAX_STRING_LENGTH];

  if (winner >= 0)
    sprintf(text, "\r\nThe herald cries: '%s is the winner!'  The crowd roars, and "
	    "coins change hands all through the stands.\r\n", cap(corner_name[winner]));
  else
    sprintf(text, "\r\nThe herald cries: 'No contest!  All stakes are returned!'\r\n");
  to_watchers(text);
  settle(winner);
  remove_fighter(corner[0]);
  remove_fighter(corner[1]);
  corner[0] = corner[1] = NULL;
  clear_ring_floor();
  state = ARENA_IDLE;
}


static void call_blows(void)
{
  static const char *quarter[] = { "is staggering, close to finished",
    "is badly hurt", "is bloodied", "is wounded" };
  static const char *crowd[] = {
    "The crowd stamps and howls.", "Someone in the stands throws an apple core.",
    "A chant goes up from the cheap seats.", "The bookmaker's runners dash along the rows.",
    "A fan nearly falls out of the top row in excitement."
  };
  int i, q;

  for (i = 0; i < 2; i++) {
    if (!valid_char(corner[i]))
      continue;
    q = MAX(0, MIN(4, 4 * GET_HIT(corner[i]) / MAX(1, GET_MAX_HIT(corner[i]))));
    if (q < hurt[i]) {
      hurt[i] = q;
      sprintf(buf, "From the ring: %s %s!\r\n", cap(corner_name[i]), quarter[MIN(3, q)]);
      to_watchers(buf);
    }
  }
  if (!number(0, 7)) {
    sprintf(buf, "%s\r\n", crowd[number(0, 4)]);
    to_watchers(buf);
  }
}


static void bout_pulse(void)
{
  int alive[2], i;

  pulses++;
  for (i = 0; i < 2; i++)
    alive[i] = corner[i] && valid_char(corner[i]) && GET_HIT(corner[i]) > 0;
  if (!alive[0] || !alive[1]) {           /* the loser is dead or down */
    end_bout(alive[0] ? 0 : alive[1] ? 1 : -1);
    return;
  }
  call_blows();
  if (pulses >= BOUT_PULSES) {          /* the judges: the less hurt wins */
    double h0 = (double) GET_HIT(corner[0]) / MAX(1, GET_MAX_HIT(corner[0]));
    double h1 = (double) GET_HIT(corner[1]) / MAX(1, GET_MAX_HIT(corner[1]));

    to_watchers("\r\nThe bell!  Time!  The fighters are dragged apart and the judges confer.\r\n");
    end_bout(h0 > h1 ? 0 : h1 > h0 ? 1 : -1);
    return;
  }
  /* keep them at it (a fighter who was knocked down) */
  if (!FIGHTING(corner[0]))
    hit(corner[0], corner[1], TYPE_UNDEFINED);
}


/* ------------------------------------------------------------------ */
/* Challenges                                                         */
/* ------------------------------------------------------------------ */

static void end_challenge(int won)
{
  struct char_data *ch = challenger;
  struct obj_data *prize;
  long gate = real_room(GATE), gold;
  int i, best = -1, cap;
  char text[MAX_STRING_LENGTH];

  if (gladiator && valid_char(gladiator))
    remove_fighter(gladiator);
  gladiator = NULL;
  challenger = NULL;
  clear_ring_floor();
  if (!ch || !valid_char(ch))
    return;
  if (FIGHTING(ch))
    stop_fighting(ch);
  if (won) {
    sprintf(text, "\r\nThe herald cries: '%s has beaten the %s gladiator!'  The crowd "
	    "cheers.\r\n", GET_NAME(ch), tiers[chal_tier]);
    to_watchers(text);
    if (chal_weapon) {
      cap = GET_LEVEL(ch) + (chal_tier == 2 ? 5 : 0);
      for (i = 0; i < num_prizes; i++)
	if (prizes[i].level <= cap && (best < 0 || prizes[i].level > prizes[best].level))
	  best = i;
    }
    if (best >= 0 && (prize = read_object(prizes[best].vnum, VNUMBER))) {
      obj_to_char(prize, ch);
      act("The arena master hands you $p.", FALSE, ch, prize, 0, TO_CHAR);
    } else {
      gold = (long) (2 * GET_LEVEL(ch) + 0.15 * GET_LEVEL(ch) * GET_LEVEL(ch)) *
	     (chal_tier == 0 ? 2 : chal_tier == 1 ? 5 : 12);
      gold = MAX(10, gold);
      GET_GOLD(ch) += gold;
      sprintf(text, "The arena master counts %ld gold into your hand.\r\n", gold);
      send_to_char(text, ch);
    }
  } else {
    sprintf(text, "\r\nThe herald cries: '%s yields!'  The crowd jeers, then applauds a "
	    "brave try.\r\n", GET_NAME(ch));
    to_watchers(text);
    send_to_char("Strong hands drag you out of the ring before the gladiator can finish "
		 "you.\r\n", ch);
  }
  if (gate != NOWHERE) {
    char_from_room(ch);
    char_to_room(ch, gate);
    GET_POS(ch) = POS_STANDING;
    look_at_room(ch, 0);
  }
}


/* From damage(): a blow that would finish a challenger is taken back, */
/* and the challenge ends at the next pulse (not here: the gladiator is */
/* in the middle of swinging).  TRUE if the blow is taken back.         */
int arena_spare(struct char_data *ch, struct char_data *victim, int dam)
{
  if (!challenger || victim != challenger || IS_NPC(victim) ||
      victim->in_room != real_room(RING) || (dam < GET_HIT(victim) && !chal_lost))
    return FALSE;
  GET_HIT(victim) = MAX(1, GET_HIT(victim));
  if (FIGHTING(victim))
    stop_fighting(victim);
  if (ch && FIGHTING(ch))
    stop_fighting(ch);
  chal_lost = 1;
  return TRUE;
}


static void challenge_pulse(void)
{
  chal_pulses++;
  if (chal_lost) {
    chal_lost = 0;
    end_challenge(FALSE);
    return;
  }
  if (!challenger || !valid_char(challenger) || challenger->in_room != real_room(RING)) {
    challenger = NULL;                  /* gone (link-dead, transported) */
    end_challenge(FALSE);
    return;
  }
  if (!gladiator || !valid_char(gladiator) || GET_HIT(gladiator) <= 0) {
    gladiator = NULL;
    end_challenge(TRUE);
    return;
  }
  if (chal_pulses >= CHALLENGE_PULSES) {
    to_watchers("\r\nThe bell!  Time!  The judges give it to the gladiator.\r\n");
    end_challenge(FALSE);
    return;
  }
  if (!FIGHTING(gladiator))
    hit(gladiator, challenger, TYPE_UNDEFINED);
}


/* ------------------------------------------------------------------ */
/* The clock                                                          */
/* ------------------------------------------------------------------ */

void arena_hour(void)
{
  int h = time_info.hours, n;

  if (!num_bout_hours)
    return;
  if (state == ARENA_IDLE && !challenger) {
    n = next_bout(h, FALSE);
    if (n >= 0 && (n - h + 24) % 24 == card_lead)
      post_card(n);
  } else if (state == ARENA_CARD && h == bout_at) {
    if (challenger)
      delayed = 1;                      /* after the challenge */
    else
      start_bout();
  } else if (state == ARENA_CARD && h != bout_at) {
    sprintf(buf, "The herald reminds the crowd: %s against %s at %s.  Get your bets in!\r\n",
	    corner_name[0], corner_name[1], hour_name(bout_at));
    to_watchers(buf);
  }
}


/* Every 2 seconds (comm.c). */
void arena_pulse(void)
{
  if (state == ARENA_BOUT)
    bout_pulse();
  if (challenger)
    challenge_pulse();
  else if (delayed && state == ARENA_CARD) {
    delayed = 0;
    start_bout();
  }
}


/* Logging in: winnings from a bout fought while you were away. */
void arena_login(struct char_data *ch)
{
  int i;

  if (IS_NPC(ch))
    return;
  for (i = 0; i < num_owed; i++)
    if (owed[i].idnum == GET_IDNUM(ch)) {
      GET_GOLD(ch) += owed[i].gold;
      sprintf(buf, "A runner from the Arena's bookmaker finds you: %ld gold in winnings.\r\n",
	      owed[i].gold);
      send_to_char(buf, ch);
      owed[i] = owed[--num_owed];
      return;
    }
}


/* ------------------------------------------------------------------ */
/* The bookmaker, the board, the arena master                         */
/* ------------------------------------------------------------------ */

static void show_board(struct char_data *ch)
{
  char out[MAX_STRING_LENGTH];
  int i, n = next_bout(time_info.hours, TRUE);

  strcpy(out, "            THE ARENA OF THE DESERT OF DEMISE\r\n\r\nBouts daily at");
  for (i = 0; i < num_bout_hours; i++)
    sprintf(out + strlen(out), "%s %s", i ? (i == num_bout_hours - 1 ? " and" : ",") : "",
	    hour_name(bout_hour[i]));
  strcat(out, ".\r\n\r\n");
  if (state == ARENA_CARD) {
    sprintf(out + strlen(out), "NEXT BOUT, at %s.  Betting is OPEN.\r\n\r\n", hour_name(bout_at));
    for (i = 0; i < 2; i++)
      sprintf(out + strlen(out), "  %d. %-34s %2d to %-2d %s   (a winning 10 pays %ld)\r\n",
	      i + 1, corner_name[i],
	      odds_num[i] >= odds_den[i] ? odds_num[i] : odds_den[i],
	      odds_num[i] >= odds_den[i] ? odds_den[i] : odds_num[i],
	      odds_num[i] > odds_den[i] ? "against" : odds_num[i] < odds_den[i] ? "on     " : "(evens)",
	      payout(10, i));
    sprintf(out + strlen(out), "\r\nMinimum stake %d gold.  BET <amount> <fighter> with the "
	    "bookmaker.\r\n", min_bet);
  } else if (state == ARENA_BOUT)
    sprintf(out + strlen(out), "IN THE RING NOW: %s against %s.  Betting is closed.\r\n",
	    corner_name[0], corner_name[1]);
  else
    sprintf(out + strlen(out), "The card for the next bout (at %s) goes up %d hours "
	    "before.\r\n", n >= 0 ? hour_name(n) : "a later hour", card_lead);
  strcat(out, "\r\nChallengers: see the arena master at the Northern Gate.\r\n");
  send_to_char(out, ch);
}


SPECIAL(arena_booth)
{
  char arg[MAX_INPUT_LENGTH];

  if (!CMD_IS("look") && !CMD_IS("read") && !CMD_IS("odds"))
    return FALSE;
  one_argument(argument, arg);
  if (!CMD_IS("odds") && (!*arg || !isname(arg, "board odds chalkboard slate")))
    return FALSE;
  show_board(ch);
  return TRUE;
}


static int side_named(char *arg)
{
  int i;

  if (!str_cmp(arg, "1") || !str_cmp(arg, "2"))
    return atoi(arg) - 1;
  for (i = 0; i < 2; i++)
    if (corner[i] && valid_char(corner[i]) && isname(arg, GET_NAME(corner[i])))
      return i;
  for (i = 0; i < 2; i++)
    if (corner[i] && valid_char(corner[i]) && isname(arg, corner[i]->player.name))
      return i;
  return -1;
}


SPECIAL(arena_bookie)
{
  struct char_data *bookie = (struct char_data *) me;
  char a1[MAX_INPUT_LENGTH], a2[MAX_INPUT_LENGTH], text[MAX_STRING_LENGTH];
  long stake;
  int side, i, n = 0;

  GET_GOLD(bookie) = 0;                 /* he holds nothing worth taking */
  if (!cmd || IS_NPC(ch))
    return FALSE;
  if (CMD_IS("odds") || CMD_IS("list")) {
    show_board(ch);
    return TRUE;
  }
  if (CMD_IS("bets")) {
    for (i = 0; i < num_bets; i++)
      if (bets[i].idnum == GET_IDNUM(ch)) {
	sprintf(text, "%s%ld gold on %s, to win %ld.\r\n", n++ ? "" : "Your tickets:\r\n",
		bets[i].stake, corner_name[bets[i].side], payout(bets[i].stake, bets[i].side));
	send_to_char(text, ch);
      }
    if (!n)
      send_to_char("You hold no tickets for the next bout.\r\n", ch);
    return TRUE;
  }
  if (!CMD_IS("bet"))
    return FALSE;
  two_arguments(argument, a1, a2);
  if (state != ARENA_CARD) {
    act("$n shakes his head.  'Book's closed.  Watch the board.'", FALSE, bookie, 0, ch, TO_VICT);
    return TRUE;
  }
  if (!*a1 || !*a2) {
    send_to_char("Bet how much on whom?  (BET <amount> <fighter>, e.g. BET 50 1)\r\n", ch);
    return TRUE;
  }
  stake = !str_cmp(a1, "all") ? GET_GOLD(ch) : atol(a1);
  if ((side = side_named(a2)) < 0) {
    sprintf(text, "$n squints at you.  'There's %s and there's %s.  Which?'",
	    corner_name[0], corner_name[1]);
    act(text, FALSE, bookie, 0, ch, TO_VICT);
    return TRUE;
  }
  if (!house_ahead(side)) {
    sprintf(text, "$n shakes his head.  'No book on %s - a certainty.'", corner_name[side]);
    act(text, FALSE, bookie, 0, ch, TO_VICT);
    return TRUE;
  }
  if (stake < min_bet) {
    sprintf(text, "$n sniffs.  'Minimum's %d gold, friend.'", min_bet);
    act(text, FALSE, bookie, 0, ch, TO_VICT);
    return TRUE;
  }
  if (stake > GET_GOLD(ch)) {
    act("$n laughs.  'Show me the gold first.'", FALSE, bookie, 0, ch, TO_VICT);
    return TRUE;
  }
  if (num_bets >= MAX_BETS) {
    act("$n waves you off.  'Book's full.'", FALSE, bookie, 0, ch, TO_VICT);
    return TRUE;
  }
  GET_GOLD(ch) -= stake;                /* the stake leaves the world... */
  bets[num_bets].idnum = GET_IDNUM(ch);  /* ...unless it comes back as winnings */
  bets[num_bets].side = side;
  bets[num_bets].stake = stake;
  num_bets++;
  sprintf(text, "$n scribbles a ticket: %ld gold on %s at %d to %d, to win %ld.",
	  stake, corner_name[side], odds_num[side], odds_den[side], payout(stake, side));
  act(text, FALSE, bookie, 0, ch, TO_VICT);
  act("$n places a bet with $N.", TRUE, ch, 0, bookie, TO_ROOM);
  save_char(ch, NOWHERE);
  return TRUE;
}


SPECIAL(arena_master)
{
  struct char_data *master = (struct char_data *) me, *g;
  char a1[MAX_INPUT_LENGTH], a2[MAX_INPUT_LENGTH], text[MAX_STRING_LENGTH];
  long ring = real_room(RING);
  int tier = 1, weapon = 0, i, lvl;

  if (!cmd || IS_NPC(ch) || (!CMD_IS("challenge") && !CMD_IS("list")))
    return FALSE;
  if (CMD_IS("list")) {
    send_to_char("The arena master rattles it off:\r\n"
		 "  CHALLENGE NOVICE   - a gladiator a little below you; a small purse.\r\n"
		 "  CHALLENGE VETERAN  - one a little above you; a good purse, or a prize weapon.\r\n"
		 "  CHALLENGE CHAMPION - the best we have; a fat purse, or a finer weapon.\r\n"
		 "Add GOLD or WEAPON for the prize you want.  It's not to the death: lose,\r\n"
		 "and we drag you out.\r\n", ch);
    return TRUE;
  }
  two_arguments(argument, a1, a2);
  for (i = 0; i < 2; i++) {
    char *w = i ? a2 : a1;
    int t;

    if (!*w)
      continue;
    if ((t = search_block(w, (char **) tiers, FALSE)) >= 0)
      tier = t;
    else if (is_abbrev(w, "weapon"))
      weapon = 1;
    else if (!is_abbrev(w, "gold")) {
      send_to_char("CHALLENGE [novice|veteran|champion] [gold|weapon]\r\n", ch);
      return TRUE;
    }
  }
  if (IS_IMMORT(ch)) {
    send_to_char("The arena master won't match a god against his people.\r\n", ch);
    return TRUE;
  }
  if (FIGHTING(ch)) {
    send_to_char("Finish the fight you're in first.\r\n", ch);
    return TRUE;
  }
  if (challenger || state != ARENA_IDLE || ring == NOWHERE) {
    act("$n shakes his head.  'The ring's spoken for.  Watch the board.'", FALSE, master, 0, ch, TO_VICT);
    return TRUE;
  }
  if (weapon && tier == 0) {
    act("$n snorts.  'Prize weapons are for veterans and champions.  Gold it is.'",
	FALSE, master, 0, ch, TO_VICT);
    weapon = 0;
  }
  for (i = 0; i < num_cool; i++)
    if (cool[i].idnum == GET_IDNUM(ch) && cool[i].until > time(0)) {
      act("$n holds up a hand.  'Catch your breath first.  Come back in a few minutes.'",
	  FALSE, master, 0, ch, TO_VICT);
      return TRUE;
    }
  if (!(g = read_mobile(GLADIATOR, VNUMBER)))
    return TRUE;
  lvl = GET_LEVEL(ch) + (tier == 0 ? -2 : 2);
  mob_set_level(g, MAX(1, lvl), 1);
  if (tier == 2) {                      /* champion: half again as tough */
    GET_MAX_HIT(g) = GET_HIT(g) = GET_MAX_HIT(g) * 3 / 2;
    g->mob_specials.damsizedice = MAX(1, g->mob_specials.damsizedice * 6 / 5);
  }
  GET_EXP(g) = 0;                       /* the prize is the prize */
  GET_GOLD(g) = 0;

  for (i = 0; i < num_cool && cool[i].idnum != GET_IDNUM(ch); i++);
  if (i == num_cool && num_cool < MAX_COOL)
    cool[num_cool++].idnum = GET_IDNUM(ch);
  if (i < MAX_COOL)
    cool[i].until = time(0) + COOLDOWN_SECS;

  challenger = ch;
  gladiator = g;
  chal_id = GET_IDNUM(ch);
  chal_tier = tier;
  chal_weapon = weapon;
  chal_pulses = 0;
  chal_lost = 0;
  char_to_room(g, ring);
  act("$n takes your name and waves you through to the ring.", FALSE, master, 0, ch, TO_VICT);
  act("$n is led through to the ring.", TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, ring);
  look_at_room(ch, 0);
  sprintf(text, "\r\nThe herald cries: '%s challenges a %s gladiator!'  Heads turn in the "
	  "stands.\r\n", GET_NAME(ch), tiers[tier]);
  to_watchers(text);
  hit(g, ch, TYPE_UNDEFINED);
  (void) chal_id;
  return TRUE;
}


void assign_arena(void)
{
  ASSIGNMOB(BOOKIE, arena_bookie);
  ASSIGNMOB(MASTER, arena_master);
  ASSIGNROOM(BOOTH, arena_booth);
}


/* "show arena [card|bout|end]" for immortals. */
void arena_show(struct char_data *ch, char *arg)
{
  static const char *states[] = { "idle", "card posted", "bout on" };
  char out[MAX_STRING_LENGTH];
  int n;

  if (!num_bout_hours) {
    send_to_char("The Arena isn't running.\r\n", ch);
    return;
  }
  if (!str_cmp(arg, "card") && state == ARENA_IDLE && !challenger) {
    n = next_bout(time_info.hours, TRUE);
    post_card(n >= 0 ? n : time_info.hours);
  } else if (!str_cmp(arg, "bout") && state == ARENA_CARD && !challenger)
    start_bout();
  else if (!str_cmp(arg, "end") && state == ARENA_BOUT)
    end_bout(-1);
  sprintf(out, "Arena: %s; next bout hour %d; %d bets; %d owed; challenger %s.\r\n",
	  states[state], bout_at, num_bets, num_owed, challenger ? GET_NAME(challenger) : "none");
  if (state != ARENA_IDLE)
    sprintf(out + strlen(out), "%s (%d.%d%%) vs %s: board %d-%d / %d-%d (edge %d%%)\r\n",
	    corner_name[0], chance_a / 10, chance_a % 10, corner_name[1],
	    odds_num[0], odds_den[0], odds_num[1], odds_den[1], edge);
  send_to_char(out, ch);
}
