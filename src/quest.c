/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  QUEST.C                                                         *
*  Usage: Quests handed out by NPCs, read from lib/data/quests            *
*  Programmer(s): Written for The Dominion, November 2026                 *
\*********************************************************************** */

/*
 * A quest is offered by a quest giver (any mobile named on a "giver"
 * line) to characters of the right level, and asks for one of:
 *
 *   kill <mob> <n>          slay n of a mobile (the killer and his group
 *                           in the room all get the credit)
 *   fetch <obj> <n>         bring n of an object back to the giver
 *   visit <room>            set foot in a room, then report back
 *   deliver <mob> <obj>     the giver hands over a letter or parcel (obj),
 *                           to be given to the mobile; the quest is done
 *                           when it is
 *
 * Kill, fetch and visit quests are handed in with "quest complete" in the
 * giver's presence (fetch items may also simply be given to him).  The
 * reward is gold, experience counted in "kills' worth" at the player's
 * level (so it stays worth having as he grows, but less if he has
 * outgrown the quest) and perhaps an object.  Most quests can be done
 * once per character; "daily" ones again the next day.
 *
 * What a character is doing and has done lives in
 * player_specials->saved.quests, written to the account file as
 * QuestActive and QuestDone lines (pfile.c).
 *
 * Hooks: quest_kill() from die() (fight.c), quest_give() from
 * perform_give() (act_item.c), quest_enter() from do_simple_move()
 * (act_move.c); "show quests" and "reload quests" for immortals.
 */

#define __QUEST_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct room_data *world;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct char_data *mob_proto;
extern struct obj_data *obj_proto;
extern struct zone_data *zone_table;
extern long top_of_zone_table;

long exp_needed(int level);
ACMD(do_gen_tog);

#define QUEST_FILE      "data/quests"

#define QT_KILL     0
#define QT_FETCH    1
#define QT_VISIT    2
#define QT_DELIVER  3

static const char *quest_type_names[] = { "kill", "fetch", "visit", "deliver" };

struct quest_data {
  int   vnum;
  char *title;
  long  giver;                  /* mob vnum                               */
  int   min_level, max_level;   /* max 0: no upper limit                  */
  int   daily;
  int   after;                  /* quest that must be done first, or 0    */
  int   type;
  long  target;                 /* mob, obj or room vnum                  */
  int   count;
  long  item;                   /* deliver: the obj vnum handed over      */
  int   gold, exp;              /* exp in kills' worth                    */
  long  reward_obj;             /* or 0                                   */
  int   has_reward;             /* a reward line was given                */
  char *offer;                  /* what the giver says when asked         */
  char *done;                   /* ... and on completion                  */
};

static struct quest_data *quests = NULL;
static int num_quests = 0;


/* ------------------------------------------------------------------ */
/* Loading                                                            */
/* ------------------------------------------------------------------ */

static void free_quests(void)
{
  int i;

  for (i = 0; i < num_quests; i++) {
    free(quests[i].title);
    if (quests[i].offer)
      free(quests[i].offer);
    if (quests[i].done)
      free(quests[i].done);
  }
  free(quests);
  quests = NULL;
  num_quests = 0;
}


static void quest_error(int line, int vnum, const char *msg)
{
  sprintf(buf, "SYSERR: " QUEST_FILE " line %d (quest %d): %s", line, vnum, msg);
  log(buf);
}


/* Appends text to *dst with a space between. */
static void append_text(char **dst, const char *text)
{
  char *n;

  if (!*dst) {
    *dst = str_dup(text);
    return;
  }
  CREATE(n, char, strlen(*dst) + strlen(text) + 2);
  sprintf(n, "%s %s", *dst, text);
  free(*dst);
  *dst = n;
}


/* The reward of a quest with no "reward" line, from its level and kind: */
/* generous, about 5-10 kills' worth of experience at the player's level */
/* and gold to match; daily errands about a third of that.              */
static void default_reward(struct quest_data *q)
{
  int l = MAX(1, q->min_level);

  q->gold = 40 * l + 2 * l * l;
  switch (q->type) {
  case QT_KILL:    q->exp = MIN(10, 5 + q->count / 2); break;
  case QT_FETCH:   q->exp = 7; break;
  case QT_VISIT:   q->exp = 6; break;
  default:         q->exp = 5; break;
  }
  if (q->daily) {
    q->gold = q->gold * 2 / 5;
    q->exp = 3;
  }
  q->gold = MAX(10, (q->gold + 5) / 10 * 10);
}


/* Checks a finished quest; FALSE (and a SYSERR) if it can't be used. */
static int check_quest(struct quest_data *q, int line)
{
  if (!q->has_reward)
    default_reward(q);
  if (!q->max_level)
    q->max_level = MAX(1, q->min_level) + 15;
  if (!q->title || !*q->title)
    quest_error(line, q->vnum, "no title");
  else if (real_mobile(q->giver) < 0)
    quest_error(line, q->vnum, "no such giver mobile");
  else if (q->type < 0)
    quest_error(line, q->vnum, "no kill, fetch, visit or deliver line");
  else if ((q->type == QT_KILL || q->type == QT_DELIVER) && real_mobile(q->target) < 0)
    quest_error(line, q->vnum, "no such target mobile");
  else if (q->type == QT_FETCH && real_object(q->target) < 0)
    quest_error(line, q->vnum, "no such object to fetch");
  else if (q->type == QT_VISIT && real_room(q->target) == NOWHERE)
    quest_error(line, q->vnum, "no such room to visit");
  else if (q->type == QT_DELIVER && real_object(q->item) < 0)
    quest_error(line, q->vnum, "no such object to deliver");
  else if (q->reward_obj && real_object(q->reward_obj) < 0)
    quest_error(line, q->vnum, "no such reward object");
  else if (!q->offer)
    quest_error(line, q->vnum, "no offer text");
  else
    return TRUE;
  return FALSE;
}


static struct quest_data *find_quest(int vnum)
{
  int i;

  for (i = 0; i < num_quests; i++)
    if (quests[i].vnum == vnum)
      return &quests[i];
  return NULL;
}


/* Reads lib/data/quests.  Safe to call again ("reload quests"). */
void load_quests(void)
{
  FILE *fl;
  char line[MAX_STRING_LENGTH], word[MAX_INPUT_LENGTH], *p;
  struct quest_data *cur = NULL;
  int lineno = 0, max = 0, start = 0, i, bad = 0;
  long a, b, c;

  free_quests();
  if (!(fl = fopen(QUEST_FILE, "r"))) {
    log("No quests file (" QUEST_FILE "); nobody hands out quests.");
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

    if (*p == '#') {
      if (cur && !check_quest(cur, start)) {
	num_quests--;                   /* drop the bad one */
	bad++;
	free(cur->title);
	if (cur->offer) free(cur->offer);
	if (cur->done) free(cur->done);
      }
      cur = NULL;
      if (!isdigit((unsigned char) p[1]))
	continue;
      if (num_quests >= max) {
	max = max ? max * 2 : 128;
	RECREATE(quests, struct quest_data, max);
      }
      cur = &quests[num_quests++];
      memset(cur, 0, sizeof(*cur));
      cur->vnum = atoi(p + 1);
      cur->type = -1;
      start = lineno;
      while (*p && !isspace((unsigned char) *p))
	p++;
      skip_spaces(&p);
      cur->title = str_dup(p);
      for (i = 0; i < num_quests - 1; i++)
	if (quests[i].vnum == cur->vnum)
	  quest_error(lineno, cur->vnum, "quest number used twice");
      continue;
    }

    if (!cur)
      continue;                         /* inside a skipped quest */
    p = one_argument(p, word);
    skip_spaces(&p);
    a = b = c = 0;
    if (!str_cmp(word, "giver"))
      cur->giver = atol(p);
    else if (!str_cmp(word, "level")) {
      sscanf(p, "%ld %ld", &a, &b);
      cur->min_level = (int) a;
      cur->max_level = (int) b;
    } else if (!str_cmp(word, "daily"))
      cur->daily = TRUE;
    else if (!str_cmp(word, "after"))
      cur->after = atoi(p);
    else if (!str_cmp(word, "kill") || !str_cmp(word, "fetch")) {
      cur->type = (*word == 'k') ? QT_KILL : QT_FETCH;
      sscanf(p, "%ld %ld", &a, &b);
      cur->target = a;
      cur->count = MAX(1, (int) b);
    } else if (!str_cmp(word, "visit")) {
      cur->type = QT_VISIT;
      cur->target = atol(p);
      cur->count = 1;
    } else if (!str_cmp(word, "deliver")) {
      cur->type = QT_DELIVER;
      sscanf(p, "%ld %ld", &a, &b);
      cur->target = a;
      cur->item = b;
      cur->count = 1;
    } else if (!str_cmp(word, "reward")) {
      sscanf(p, "%ld %ld %ld", &a, &b, &c);
      cur->gold = (int) a;
      cur->exp = (int) b;
      cur->reward_obj = c;
      cur->has_reward = TRUE;
    } else if (!str_cmp(word, "offer"))
      append_text(&cur->offer, p);
    else if (!str_cmp(word, "done"))
      append_text(&cur->done, p);
    else
      quest_error(lineno, cur->vnum, "unknown line");
  }
  fclose(fl);
  if (cur && !check_quest(cur, start)) {
    num_quests--;
    bad++;
    free(cur->title);
    if (cur->offer) free(cur->offer);
    if (cur->done) free(cur->done);
  }
  for (i = 0; i < num_quests; i++)
    if (quests[i].after && !find_quest(quests[i].after))
      quest_error(0, quests[i].vnum, "'after' names a quest that doesn't exist");

  sprintf(buf, "Loaded %d quests%s.", num_quests, bad ? " (some skipped)" : "");
  log(buf);
}


/* ------------------------------------------------------------------ */
/* A character's record                                               */
/* ------------------------------------------------------------------ */

#define QREC(ch)   (&((ch)->player_specials->saved.quests))

static struct quest_active *active_slot(struct char_data *ch, int vnum)
{
  int i;

  for (i = 0; i < MAX_ACTIVE_QUESTS; i++)
    if (QREC(ch)->active[i].vnum == vnum)
      return &QREC(ch)->active[i];
  return NULL;
}


static struct quest_done *done_entry(struct char_data *ch, int vnum)
{
  int i;

  for (i = 0; i < QREC(ch)->num_done; i++)
    if (QREC(ch)->done[i].vnum == vnum)
      return &QREC(ch)->done[i];
  return NULL;
}


static int same_day(time_t a, time_t b)
{
  struct tm ta = *localtime(&a), tb = *localtime(&b);

  return ta.tm_year == tb.tm_year && ta.tm_yday == tb.tm_yday;
}


/* Done, and can't be done again (yet). */
static int quest_finished(struct char_data *ch, struct quest_data *q)
{
  struct quest_done *d = done_entry(ch, q->vnum);

  if (!d)
    return FALSE;
  return !q->daily || same_day(d->when, time(0));
}


static int quest_complete_now(struct quest_data *q, struct quest_active *a)
{
  return a->count >= q->count;
}


#define QS_HIDDEN     0         /* not shown at all                       */
#define QS_LOW        1         /* needs a higher level                   */
#define QS_OPEN       2
#define QS_ACTIVE     3
#define QS_READY      4         /* ready to hand in                       */
#define QS_DONE       5

static int quest_state(struct char_data *ch, struct quest_data *q)
{
  struct quest_active *a;
  struct quest_data *pre;

  if ((a = active_slot(ch, q->vnum)))
    return quest_complete_now(q, a) ? QS_READY : QS_ACTIVE;
  if (quest_finished(ch, q))
    return QS_DONE;
  if (q->after && (pre = find_quest(q->after)) && !done_entry(ch, pre->vnum))
    return QS_HIDDEN;
  if (q->max_level && GET_LEVEL(ch) > q->max_level + 10 && GET_LEVEL(ch) < LVL_IMMORT)
    return QS_HIDDEN;                   /* far beneath him */
  if (GET_LEVEL(ch) < q->min_level)
    return QS_LOW;
  return QS_OPEN;
}


/* ------------------------------------------------------------------ */
/* Names of things                                                    */
/* ------------------------------------------------------------------ */

static const char *mob_name(long vnum)
{
  long r = real_mobile(vnum);

  return r < 0 ? "someone" : mob_proto[r].player.short_descr;
}


static const char *obj_name(long vnum)
{
  long r = real_object(vnum);

  return r < 0 ? "something" : obj_proto[r].short_description;
}


static const char *zone_name_of(long vnum)
{
  long z;

  for (z = 0; z <= top_of_zone_table; z++)
    if (vnum >= zone_table[z].number * 100 && vnum <= zone_table[z].top)
      return zone_table[z].name;
  return "somewhere";
}


/* "Slay 4 of the kobold scouts" and so on, with progress if active. */
static void objective(struct quest_data *q, struct quest_active *a, char *out)
{
  long r;

  switch (q->type) {
  case QT_KILL:
    if (q->count > 1)
      sprintf(out, "Slay %d of %s", q->count, mob_name(q->target));
    else
      sprintf(out, "Slay %s", mob_name(q->target));
    break;
  case QT_FETCH:
    if (q->count > 1)
      sprintf(out, "Bring %s back %d of %s", mob_name(q->giver), q->count,
	      obj_name(q->target));
    else
      sprintf(out, "Bring %s back %s", mob_name(q->giver), obj_name(q->target));
    break;
  case QT_VISIT:
    r = real_room(q->target);
    sprintf(out, "Find %s (%s) and report back",
	    r == NOWHERE ? "the place" : world[r].name, zone_name_of(q->target));
    break;
  case QT_DELIVER:
    sprintf(out, "Take %s to %s", obj_name(q->item), mob_name(q->target));
    break;
  default:
    strcpy(out, "Something");
  }
  if (a && q->count > 1 && q->type != QT_VISIT)
    sprintf(out + strlen(out), " (%d/%d)", (int) MIN(a->count, q->count), q->count);
  if (a && quest_complete_now(q, a) && q->type != QT_DELIVER)
    strcat(out, " - done, go back to collect your reward");
}


/* ------------------------------------------------------------------ */
/* Givers                                                             */
/* ------------------------------------------------------------------ */

static int is_giver(struct char_data *mob)
{
  int i;

  if (!IS_NPC(mob) || GET_MOB_RNUM(mob) < 0)
    return FALSE;
  for (i = 0; i < num_quests; i++)
    if (quests[i].giver == GET_MOB_VNUM(mob))
      return TRUE;
  return FALSE;
}


/* The giver of q in ch's room, awake and not busy; or NULL. */
static struct char_data *giver_here(struct char_data *ch, struct quest_data *q)
{
  struct char_data *m;

  for (m = world[ch->in_room].people; m; m = m->next_in_room)
    if (IS_NPC(m) && GET_MOB_VNUM(m) == q->giver && AWAKE(m) && !FIGHTING(m) &&
	CAN_SEE(ch, m))
      return m;
  return NULL;
}


/* The quests on offer in ch's room, in a fixed order, for numbering. */
static int quests_here(struct char_data *ch, struct quest_data **list, int max)
{
  int i, n = 0, st;

  for (i = 0; i < num_quests && n < max; i++)
    if (giver_here(ch, &quests[i]) &&
	(st = quest_state(ch, &quests[i])) != QS_HIDDEN)
      list[n++] = &quests[i];
  return n;
}


/* A quest notice, in green. */
static void qmsg(struct char_data *ch, const char *text)
{
  char out[MAX_STRING_LENGTH];

  snprintf(out, sizeof(out), "%s%s%s\r\n", CCGRN(ch, C_NRM), text, CCNRM(ch, C_NRM));
  send_to_char(out, ch);
}


static void giver_says(struct char_data *giver, struct char_data *ch, const char *text)
{
  char msg[MAX_STRING_LENGTH];

  snprintf(msg, sizeof(msg), "$n says to you, '%s'", text);
  act(msg, FALSE, giver, 0, ch, TO_VICT);
  act("$n speaks quietly with $N.", TRUE, giver, 0, ch, TO_NOTVICT);
}


/* ------------------------------------------------------------------ */
/* Rewards                                                            */
/* ------------------------------------------------------------------ */

static void give_reward(struct char_data *ch, struct quest_data *q)
{
  struct obj_data *obj;
  long per, gap;
  int kills = q->exp, over, i;
  char msg[MAX_STRING_LENGTH];

  *msg = '\0';
  if (q->gold > 0) {
    GET_GOLD(ch) += q->gold;
    sprintf(msg, "You receive %d gold coin%s.\r\n", q->gold, q->gold == 1 ? "" : "s");
    send_to_char(msg, ch);
  }
  if (kills > 0 && GET_LEVEL(ch) < LVL_IMMORT) {
    /* outgrown it: half as much for every 5 levels past the top */
    over = q->max_level ? GET_LEVEL(ch) - q->max_level : 0;
    for (; over >= 5 && kills > 1; over -= 5)
      kills /= 2;
    gap = exp_needed(GET_LEVEL(ch) + 1) - exp_needed(GET_LEVEL(ch));
    per = MAX(1L, gap / 16);
    send_to_char("You feel more experienced.\r\n", ch);
    for (i = 0; i < kills && !CAN_LEV(ch); i++)  /* stops at the cap */
      gain_exp(ch, (int) per);
  }
  if (q->reward_obj && (obj = read_object(q->reward_obj, VNUMBER))) {
    obj_to_char(obj, ch);
    act("You are given $p.", FALSE, ch, obj, 0, TO_CHAR);
  }
}


static void finish_quest(struct char_data *ch, struct quest_data *q,
			 struct char_data *giver)
{
  struct quest_active *a = active_slot(ch, q->vnum);
  struct quest_done *d;
  char msg[MAX_STRING_LENGTH];

  if (a)
    memset(a, 0, sizeof(*a));
  if (!(d = done_entry(ch, q->vnum)) && QREC(ch)->num_done < MAX_QUESTS_DONE)
    d = &QREC(ch)->done[QREC(ch)->num_done++];
  if (d) {
    d->vnum = q->vnum;
    d->when = time(0);
  }
  if (giver)
    giver_says(giver, ch, q->done ? q->done : "Well done.  Here, you've earned this.");
  sprintf(msg, "Quest complete: %s", q->title);
  qmsg(ch, msg);
  give_reward(ch, q);
  sprintf(buf, "%s completed quest %d (%s).", GET_NAME(ch), q->vnum, q->title);
  mudlog(buf, CMP, LVL_GOD, TRUE);
  save_char(ch, NOWHERE);
}


/* ------------------------------------------------------------------ */
/* Hooks                                                              */
/* ------------------------------------------------------------------ */

static void kill_credit(struct char_data *ch, struct char_data *victim)
{
  struct quest_active *a;
  struct quest_data *q;
  int i;
  char msg[MAX_STRING_LENGTH];

  if (IS_NPC(ch) || !ch->desc)
    return;
  for (i = 0; i < MAX_ACTIVE_QUESTS; i++) {
    a = &QREC(ch)->active[i];
    if (!a->vnum || !(q = find_quest(a->vnum)) || q->type != QT_KILL ||
	q->target != GET_MOB_VNUM(victim) || a->count >= q->count)
      continue;
    a->count++;
    if (a->count >= q->count)
      sprintf(msg, "Quest: %s - that's all of them.  Go back to %s.",
	      q->title, mob_name(q->giver));
    else
      sprintf(msg, "Quest: %s (%d/%d).", q->title, a->count, q->count);
    qmsg(ch, msg);
  }
}


/* victim is about to die at killer's hands. */
void quest_kill(struct char_data *victim, struct char_data *killer)
{
  struct char_data *pc, *leader;
  struct follow_type *f;

  if (!num_quests || !IS_NPC(victim) || !killer || GET_MOB_RNUM(victim) < 0)
    return;
  pc = killer;
  if (IS_NPC(pc) && pc->master && !IS_NPC(pc->master))
    pc = pc->master;                    /* a pet or charmie's master */
  if (IS_NPC(pc) || pc->in_room == NOWHERE)
    return;

  if (!IS_AFFECTED2(pc, AFF_GROUP)) {
    kill_credit(pc, victim);
    return;
  }
  leader = pc->master ? pc->master : pc;
  if (IS_AFFECTED2(leader, AFF_GROUP) && leader->in_room == pc->in_room)
    kill_credit(leader, victim);
  for (f = leader->followers; f; f = f->next)
    if (IS_AFFECTED2(f->follower, AFF_GROUP) && f->follower->in_room == pc->in_room)
      kill_credit(f->follower, victim);
}


/* ch gives obj to vict: TRUE if a quest wants it.  With test FALSE it */
/* is used up (extracted) and the quest moves on.                      */
int quest_give(struct char_data *ch, struct char_data *vict, struct obj_data *obj,
	       int test)
{
  struct quest_active *a;
  struct quest_data *q;
  int i;
  char msg[MAX_STRING_LENGTH];

  if (!num_quests || IS_NPC(ch) || !IS_NPC(vict) || GET_OBJ_RNUM(obj) < 0)
    return FALSE;
  for (i = 0; i < MAX_ACTIVE_QUESTS; i++) {
    a = &QREC(ch)->active[i];
    if (!a->vnum || !(q = find_quest(a->vnum)))
      continue;
    if (q->type == QT_DELIVER && q->target == GET_MOB_VNUM(vict) &&
	q->item == GET_OBJ_VNUM(obj)) {
      if (test)
	return TRUE;
      extract_obj(obj);
      a->count = 1;
      finish_quest(ch, q, vict);
      return TRUE;
    }
    if (q->type == QT_FETCH && q->giver == GET_MOB_VNUM(vict) &&
	q->target == GET_OBJ_VNUM(obj) && a->count < q->count) {
      if (test)
	return TRUE;
      extract_obj(obj);
      a->count++;
      if (a->count >= q->count)
	finish_quest(ch, q, vict);
      else {
	sprintf(msg, "Quest: %s (%d/%d).", q->title, a->count, q->count);
	qmsg(ch, msg);
      }
      return TRUE;
    }
  }
  return FALSE;
}


/* ch has arrived in a room, by any means (char_to_room()). */
void quest_arrive(struct char_data *ch)
{
  struct quest_active *a;
  struct quest_data *q;
  int i;
  char msg[MAX_STRING_LENGTH];

  if (!num_quests || IS_NPC(ch) || !ch->desc || ch->in_room == NOWHERE)
    return;
  for (i = 0; i < MAX_ACTIVE_QUESTS; i++) {
    a = &QREC(ch)->active[i];
    if (a->vnum && (q = find_quest(a->vnum)) && q->type == QT_VISIT &&
	!a->count && world[ch->in_room].number == q->target) {
      a->count = 1;
      sprintf(msg, "Quest: %s - you have found it.  Go back to %s.",
	      q->title, mob_name(q->giver));
      qmsg(ch, msg);
    }
  }
}


/* ch has just walked into a room: a giver with something for him */
/* catches his eye.                                               */
void quest_enter(struct char_data *ch)
{
  struct char_data *m;
  int i, offer = FALSE, ready = FALSE;

  if (!num_quests || IS_NPC(ch) || !ch->desc || ch->in_room == NOWHERE)
    return;

  /* a giver with something for you catches your eye */
  for (m = world[ch->in_room].people; m; m = m->next_in_room) {
    if (!IS_NPC(m) || !AWAKE(m) || FIGHTING(m) || !CAN_SEE(ch, m) || !is_giver(m))
      continue;
    for (i = 0; i < num_quests; i++) {
      if (quests[i].giver != GET_MOB_VNUM(m))
	continue;
      switch (quest_state(ch, &quests[i])) {
      case QS_OPEN:  offer = TRUE; break;
      case QS_READY: ready = TRUE; break;
      }
    }
    if (ready)
      act("$N looks at you expectantly.  (Type QUEST COMPLETE to hand in your quest.)",
	  FALSE, ch, 0, m, TO_CHAR);
    else if (offer)
      act("$N catches your eye; $E seems to want something done.  (Type QUEST.)",
	  FALSE, ch, 0, m, TO_CHAR);
    if (ready || offer)
      return;
  }
}


/* ------------------------------------------------------------------ */
/* The quest command                                                  */
/* ------------------------------------------------------------------ */

static const char *state_words[] = {
  "", "needs a higher level", "", "in progress", "ready to hand in", "done"
};

static void quest_log(struct char_data *ch)
{
  struct quest_active *a;
  struct quest_data *q;
  char line[MAX_STRING_LENGTH], obj[512];
  int i, n = 0;

  strcpy(buf, "Your quests:\r\n");
  for (i = 0; i < MAX_ACTIVE_QUESTS; i++) {
    a = &QREC(ch)->active[i];
    if (!a->vnum || !(q = find_quest(a->vnum)))
      continue;
    objective(q, a, obj);
    sprintf(line, "  %d. %s, for %s (%s)\r\n       %s.\r\n", ++n, q->title,
	    mob_name(q->giver), zone_name_of(q->vnum), obj);
    strcat(buf, line);
  }
  if (!n)
    strcat(buf, "  None.  Quest givers catch your eye as you pass; type QUEST near one.\r\n");
  sprintf(line, "You have finished %d quest%s.\r\n", QREC(ch)->num_done,
	  QREC(ch)->num_done == 1 ? "" : "s");
  strcat(buf, line);
  send_to_char(buf, ch);
}


static void quest_list_here(struct char_data *ch)
{
  struct quest_data *list[64];
  char line[MAX_STRING_LENGTH];
  int n, i, st;

  n = quests_here(ch, list, 64);
  strcpy(buf, "Quests offered here:\r\n");
  for (i = 0; i < n; i++) {
    st = quest_state(ch, list[i]);
    sprintf(line, "  %2d. %-40s %s%s%s%s\r\n", i + 1, list[i]->title,
	    mob_name(list[i]->giver), *state_words[st] ? " - " : "", state_words[st],
	    list[i]->daily ? " (daily)" : "");
    if (st == QS_LOW)
      sprintf(line, "  %2d. %-40s %s - needs level %d\r\n", i + 1, list[i]->title,
	      mob_name(list[i]->giver), list[i]->min_level);
    strcat(buf, line);
  }
  strcat(buf, "QUEST INFO <n> to hear about one, QUEST ACCEPT <n> to take it on,\r\n"
	 "QUEST COMPLETE to hand in, QUEST LOG for your own list.\r\n");
  send_to_char(buf, ch);
}


/* TRUE if word appears in title, ignoring case. */
static int title_has(const char *title, const char *word)
{
  size_t n = strlen(word);

  for (; *title; title++)
    if (!strn_cmp((char *) title, (char *) word, (int) n))
      return TRUE;
  return FALSE;
}


/* The n'th quest here, from an argument like "2". */
static struct quest_data *pick_here(struct char_data *ch, char *arg)
{
  struct quest_data *list[64];
  int n = quests_here(ch, list, 64), i;

  if (!n) {
    send_to_char("Nobody here has a quest for you.\r\n", ch);
    return NULL;
  }
  if (!*arg && n == 1)
    return list[0];
  if (is_number(arg) && (i = atoi(arg)) >= 1 && i <= n)
    return list[i - 1];
  for (i = 0; i < n; i++)                /* or a word of the title */
    if (*arg && title_has(list[i]->title, arg))
      return list[i];
  send_to_char("Which quest?  Type QUEST to see the list.\r\n", ch);
  return NULL;
}


static void quest_info(struct char_data *ch, struct quest_data *q)
{
  struct char_data *giver = giver_here(ch, q);
  char obj[512], line[MAX_STRING_LENGTH];

  if (giver)
    giver_says(giver, ch, q->offer);
  objective(q, active_slot(ch, q->vnum), obj);
  sprintf(line, "%s: %s.\r\nSuggested level %d%s%s.  Reward:", q->title, obj,
	  q->min_level, q->daily ? "; can be done once a day" : "",
	  quest_state(ch, q) == QS_DONE ? "; you have done it" : "");
  if (q->gold)
    sprintf(line + strlen(line), " %d gold,", q->gold);
  if (q->exp)
    strcat(line, q->exp >= 8 ? " a great deal of experience," :
	   q->exp >= 4 ? " good experience," : " some experience,");
  if (q->reward_obj)
    sprintf(line + strlen(line), " %s,", obj_name(q->reward_obj));
  line[strlen(line) - 1] = '.';
  strcat(line, "\r\n");
  send_to_char(line, ch);
}


static void quest_accept(struct char_data *ch, struct quest_data *q)
{
  struct char_data *giver = giver_here(ch, q);
  struct quest_active *a;
  struct obj_data *obj;
  int st = quest_state(ch, q), i;

  if (!giver)
    return;
  if (st == QS_DONE) {
    giver_says(giver, ch, q->daily ? "You've done that for me today.  Come back tomorrow."
	       : "You've already done that for me, and I'm grateful.");
    return;
  }
  if (st == QS_LOW) {
    giver_says(giver, ch, "I'm sorry, that's work for someone more seasoned.  "
	       "Come back when you've a few more scars.");
    return;
  }
  if (st == QS_ACTIVE || st == QS_READY) {
    if (q->type == QT_DELIVER) {        /* lost the parcel?  another one */
      for (obj = ch->carrying; obj; obj = obj->next_content)
	if (GET_OBJ_VNUM(obj) == q->item)
	  break;
      if (!obj && (obj = read_object(q->item, VNUMBER))) {
	obj_to_char(obj, ch);
	act("$N sighs and hands you another $p.", FALSE, ch, obj, giver, TO_CHAR);
	return;
      }
    }
    send_to_char("You are already on that quest.  (QUEST LOG)\r\n", ch);
    return;
  }
  for (i = 0, a = NULL; i < MAX_ACTIVE_QUESTS; i++)
    if (!QREC(ch)->active[i].vnum) {
      a = &QREC(ch)->active[i];
      break;
    }
  if (!a) {
    sprintf(buf, "You are already on %d quests; finish or abandon one first.\r\n",
	    MAX_ACTIVE_QUESTS);
    send_to_char(buf, ch);
    return;
  }
  a->vnum = q->vnum;
  a->count = 0;
  a->taken = time(0);
  sprintf(buf, "You take on the quest: %s.", q->title);
  qmsg(ch, buf);
  act("$N nods to you.  \"Good.  I'll be waiting.\"", FALSE, ch, 0, giver, TO_CHAR);
  if (q->type == QT_DELIVER && (obj = read_object(q->item, VNUMBER))) {
    obj_to_char(obj, ch);
    act("$N hands you $p.", FALSE, ch, obj, giver, TO_CHAR);
    act("$N hands $n $p.", TRUE, ch, obj, giver, TO_ROOM);
  }
  if (q->type == QT_VISIT && world[ch->in_room].number == q->target)
    a->count = 1;
  save_char(ch, NOWHERE);
}


/* Hands in whatever is ready, to givers here. */
static void quest_complete(struct char_data *ch)
{
  struct quest_active *a;
  struct quest_data *q;
  struct char_data *giver;
  struct obj_data *obj, *next;
  int i, any = FALSE, have;

  for (i = 0; i < MAX_ACTIVE_QUESTS; i++) {
    a = &QREC(ch)->active[i];
    if (!a->vnum || !(q = find_quest(a->vnum)) || !(giver = giver_here(ch, q)))
      continue;
    if (q->type == QT_FETCH && a->count < q->count) {
      /* hand over what he carries */
      for (have = 0, obj = ch->carrying; obj; obj = obj->next_content)
	if (GET_OBJ_VNUM(obj) == q->target)
	  have++;
      if (have + a->count >= q->count) {
	for (obj = ch->carrying; obj && a->count < q->count; obj = next) {
	  next = obj->next_content;
	  if (GET_OBJ_VNUM(obj) == q->target) {
	    act("You give $p to $N.", FALSE, ch, obj, giver, TO_CHAR);
	    extract_obj(obj);
	    a->count++;
	  }
	}
      }
    }
    if (q->type == QT_DELIVER)
      continue;                         /* done by giving it to the recipient */
    any = TRUE;
    if (quest_complete_now(q, a))
      finish_quest(ch, q, giver);
    else
      giver_says(giver, ch, "That isn't finished yet, is it?");
  }
  if (!any)
    send_to_char("There's nobody here to hand a finished quest to.\r\n", ch);
}


static void quest_abandon(struct char_data *ch, char *arg)
{
  struct quest_active *a;
  struct quest_data *q;
  int i, n = 0, want = atoi(arg);

  for (i = 0; i < MAX_ACTIVE_QUESTS; i++) {
    a = &QREC(ch)->active[i];
    if (!a->vnum)
      continue;
    if (++n == want) {
      q = find_quest(a->vnum);
      sprintf(buf, "You give up the quest: %s.\r\n", q ? q->title : "(gone)");
      send_to_char(buf, ch);
      memset(a, 0, sizeof(*a));
      save_char(ch, NOWHERE);
      return;
    }
  }
  send_to_char("Abandon which?  QUEST ABANDON <number from QUEST LOG>.\r\n", ch);
}


ACMD(do_quest)
{
  char sub[MAX_INPUT_LENGTH], rest[MAX_INPUT_LENGTH];
  struct quest_data *q, *list[64];
  struct quest_active *a;
  int i;

  if (IS_NPC(ch))
    return;
  half_chop(argument, sub, rest);

  /* drop quests that have vanished from the file */
  for (i = 0; i < MAX_ACTIVE_QUESTS; i++) {
    a = &QREC(ch)->active[i];
    if (a->vnum && !find_quest(a->vnum))
      memset(a, 0, sizeof(*a));
  }

  if (!*sub) {
    if (quests_here(ch, list, 64))
      quest_list_here(ch);
    else
      quest_log(ch);
  } else if (is_abbrev(sub, "list"))
    quest_list_here(ch);
  else if (is_abbrev(sub, "log") || is_abbrev(sub, "journal"))
    quest_log(ch);
  else if (is_abbrev(sub, "info") || is_abbrev(sub, "ask")) {
    if ((q = pick_here(ch, rest)))
      quest_info(ch, q);
  } else if (is_abbrev(sub, "accept") || is_abbrev(sub, "take")) {
    if ((q = pick_here(ch, rest)))
      quest_accept(ch, q);
  } else if (is_abbrev(sub, "complete") || is_abbrev(sub, "finish") ||
	     is_abbrev(sub, "hand"))
    quest_complete(ch);
  else if (is_abbrev(sub, "abandon"))
    quest_abandon(ch, rest);
  else if (is_abbrev(sub, "channel"))
    do_gen_tog(ch, "", 0, SCMD_QUEST);
  else
    send_to_char("Usage: QUEST [LIST | LOG | INFO <n> | ACCEPT <n> | COMPLETE | "
		 "ABANDON <n> | CHANNEL]\r\n", ch);
}


/* ------------------------------------------------------------------ */
/* Immortals                                                          */
/* ------------------------------------------------------------------ */

/* "show quests [zone]": every quest, or one zone's. */
void quest_show(struct char_data *ch, char *arg)
{
  char line[MAX_STRING_LENGTH], obj[512];
  int i, zone = -1, n = 0;
  size_t len;
  char *out;

  if (*arg)
    zone = atoi(arg);
  CREATE(out, char, 64 * 1024);
  sprintf(out, "%d quests loaded.\r\n", num_quests);
  len = strlen(out);
  for (i = 0; i < num_quests; i++) {
    if (zone >= 0 && quests[i].vnum / 100 != zone)
      continue;
    objective(&quests[i], NULL, obj);
    snprintf(line, sizeof(line), "%6d %-30.30s lv%-3d %-7s %s%s\r\n", quests[i].vnum,
	     quests[i].title, quests[i].min_level, quest_type_names[quests[i].type],
	     obj, quests[i].daily ? " (daily)" : "");
    if (len + strlen(line) + 32 >= 64 * 1024) {
      strcat(out, "...\r\n");
      break;
    }
    strcpy(out + len, line);
    len += strlen(line);
    n++;
  }
  if (zone >= 0) {
    sprintf(line, "%d in zone %d.\r\n", n, zone);
    strcat(out, line);
  }
  page_string(ch->desc, out, 1);
  free(out);
}
