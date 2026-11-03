/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  CASTLE.C                                     Based on CircleMUD *
*  Usage: Special procedures for the Castle of Llyndyr (zone 383)         *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

/**********************************************************************\
|* Based on the special procedures for King's Castle by               *|
|* Pjotr (d90-pem@nada.kth.se), coded by Sapowox (d90-jkr@nada.kth.se) *|
|*                                                                    *|
|* Reworked for the Castle of Llyndyr: King Welmar is now the Emperor, *|
|* his daily routine follows the rooms of Llyndyr, and the staff are  *|
|* identified by vnum instead of by name.                             *|
\**********************************************************************/

#define __CASTLE_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/*   external vars  */
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct time_info_data time_info;
extern const int rev_dir[];

void add_follower(struct char_data *ch, struct char_data *leader);
void ASSIGNMOB(long mob, SPECIAL(fname));

/* ------------------------------------------------------------------ */
/* Vnums used by the castle.  Mobiles that are not listed in          */
/* member_of_staff() (the Demon Knight, the SaberBeast and the        */
/* creatures of the tunnels) are not part of the household.          */
/* ------------------------------------------------------------------ */
#define LLYNDYR(n)              (38300 + (n))

#define MOB_UNDEAD_GUARDIAN     LLYNDYR(0)
#define MOB_CAPTAIN             LLYNDYR(1)
#define MOB_MASTER_AT_ARMS      LLYNDYR(2)
#define MOB_EMPEROR             LLYNDYR(3)
#define MOB_CASTLE_GUARDIAN     LLYNDYR(5)
#define MOB_BODYGUARD_VARN      LLYNDYR(6)
#define MOB_BODYGUARD_VORN      LLYNDYR(7)
#define MOB_WARDEN_ORSO         LLYNDYR(8)
#define MOB_WARDEN_ILDO         LLYNDYR(9)
#define MOB_CHAMBERLAIN         LLYNDYR(10)
#define MOB_CHAMBERMAID         LLYNDYR(11)
#define MOB_GAMBLER             LLYNDYR(12)
#define MOB_INNKEEPER           LLYNDYR(13)
#define MOB_ARMORER             LLYNDYR(14)
#define MOB_APOTHECARY          LLYNDYR(15)
#define MOB_ARCHER              LLYNDYR(20)
#define MOB_RECRUIT_ALDRIC      LLYNDYR(21)
#define MOB_RECRUIT_BREN        LLYNDYR(22)
#define MOB_QUARTERMASTER       LLYNDYR(23)

#define ROOM_HALL               LLYNDYR(4)      /* A Two-Way Passage      */
#define ROOM_THRONE             LLYNDYR(5)      /* Domain of the Emperor  */
#define ROOM_BEDCHAMBER         LLYNDYR(24)     /* the Emperor's bedroom  */

SPECIAL(CastleGuard);
SPECIAL(James);
SPECIAL(cleaning);
SPECIAL(DicknDavid);
SPECIAL(tim);
SPECIAL(tom);
SPECIAL(emperor);
SPECIAL(training_master);
SPECIAL(peter);
SPECIAL(jerry);


/* Routine assign_llyndyr */
/* Assigns the castle procedures; called from assign_mobiles().  */
/* ASSIGNMOB logs (instead of crashing) if a mobile is missing.  */
void assign_llyndyr(void)
{
  ASSIGNMOB(MOB_UNDEAD_GUARDIAN, CastleGuard);  /* at the gate            */
  ASSIGNMOB(MOB_CAPTAIN, peter);                /* Captain of the Guard   */
  ASSIGNMOB(MOB_MASTER_AT_ARMS, training_master);
  ASSIGNMOB(MOB_EMPEROR, emperor);              /* the Emperor himself    */
  ASSIGNMOB(MOB_CASTLE_GUARDIAN, CastleGuard);
  ASSIGNMOB(MOB_BODYGUARD_VARN, tom);           /* the twin bodyguards    */
  ASSIGNMOB(MOB_BODYGUARD_VORN, tim);
  ASSIGNMOB(MOB_WARDEN_ORSO, DicknDavid);       /* wardens of the throne  */
  ASSIGNMOB(MOB_WARDEN_ILDO, DicknDavid);
  ASSIGNMOB(MOB_CHAMBERLAIN, James);
  ASSIGNMOB(MOB_CHAMBERMAID, cleaning);
  ASSIGNMOB(MOB_GAMBLER, jerry);
  ASSIGNMOB(MOB_ARCHER, CastleGuard);
  ASSIGNMOB(MOB_RECRUIT_ALDRIC, CastleGuard);
  ASSIGNMOB(MOB_RECRUIT_BREN, CastleGuard);
  /* The innkeeper, armorer, apothecary and quartermaster are shop   */
  /* keepers (shp/383.shp); they are still members of the staff.     */
}


/* Function member_of_staff */
/* TRUE if the character belongs to the household of Llyndyr.  */
/* Used mainly by the BANZAII-ing NPCs.                         */
int member_of_staff(struct char_data * chChar)
{
  long ch_num;

  if (!chChar || !IS_NPC(chChar))
    return (FALSE);

  ch_num = GET_MOB_VNUM(chChar);
  return ((ch_num >= MOB_UNDEAD_GUARDIAN && ch_num <= MOB_APOTHECARY &&
	   ch_num != LLYNDYR(4)) ||             /* not the Demon Knight */
	  (ch_num >= MOB_ARCHER && ch_num <= MOB_QUARTERMASTER));
}


/* Function member_of_royal_guard */
/* TRUE if the character is a guard on duty.               */
/* Used by the Captain of the Guard for his inspections.   */
int member_of_royal_guard(struct char_data * chChar)
{
  long ch_num;

  if (!chChar || !IS_NPC(chChar))
    return FALSE;

  ch_num = GET_MOB_VNUM(chChar);
  return (ch_num == MOB_CASTLE_GUARDIAN ||
	  ch_num == MOB_ARCHER ||
	  ch_num == MOB_WARDEN_ORSO ||
	  ch_num == MOB_WARDEN_ILDO ||
	  ch_num == MOB_RECRUIT_ALDRIC ||
	  ch_num == MOB_RECRUIT_BREN);
}


/* Function find_npc_by_vnum */
/* Returns an NPC with the given vnum in the same room, or NULL. */
struct char_data *find_npc_by_vnum(struct char_data * chAtChar, long vnum)
{
  struct char_data *ch;

  if (chAtChar->in_room == NOWHERE)
    return NULL;

  for (ch = world[chAtChar->in_room].people; ch; ch = ch->next_in_room)
    if (IS_NPC(ch) && GET_MOB_VNUM(ch) == vnum)
      return (ch);

  return NULL;
}


/* Function find_guard */
/* Returns a guard on duty in the same room (never chAtChar itself). */
struct char_data *find_guard(struct char_data * chAtChar)
{
  struct char_data *ch;

  for (ch = world[chAtChar->in_room].people; ch; ch = ch->next_in_room)
    if (ch != chAtChar && !FIGHTING(ch) && member_of_royal_guard(ch))
      return ch;

  return NULL;
}


/* Function get_victim */
/* Returns a randomly chosen character in the same room who is   */
/* fighting a member of the castle staff (or NULL now and then). */
struct char_data *get_victim(struct char_data * chAtChar)
{
  struct char_data *ch;
  int iNum_bad_guys = 0, iVictim;

  for (ch = world[chAtChar->in_room].people; ch; ch = ch->next_in_room)
    if (FIGHTING(ch) && !member_of_staff(ch) &&
	member_of_staff(FIGHTING(ch)))
      iNum_bad_guys++;

  if (!iNum_bad_guys)
    return NULL;

  iVictim = number(0, iNum_bad_guys);   /* How nice, we give them a chance */
  if (!iVictim)
    return NULL;

  iNum_bad_guys = 0;

  for (ch = world[chAtChar->in_room].people; ch; ch = ch->next_in_room)
    if (FIGHTING(ch) && !member_of_staff(ch) &&
	member_of_staff(FIGHTING(ch)) &&
	++iNum_bad_guys == iVictim)
      return ch;

  return NULL;
}


/* Function banzaii */
/* Makes a character charge anyone attacking the castle staff. */
int banzaii(struct char_data * ch)
{
  struct char_data *chOpponent = NULL;

  if (!AWAKE(ch) || GET_POS(ch) == POS_FIGHTING || FIGHTING(ch))
    return FALSE;

  if ((chOpponent = get_victim(ch))) {
    act("$n roars: 'For Llyndyr and the Emperor!  Cut them down!'",
	FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, chOpponent, TYPE_UNDEFINED);
    return TRUE;
  }
  return FALSE;
}


/* Function do_npc_rescue */
/* Makes ch_hero rescue ch_victim.  Used by the bodyguards. */
int do_npc_rescue(struct char_data * ch_hero, struct char_data * ch_victim)
{
  struct char_data *ch_bad_guy;

  for (ch_bad_guy = world[ch_hero->in_room].people;
       ch_bad_guy && (FIGHTING(ch_bad_guy) != ch_victim);
       ch_bad_guy = ch_bad_guy->next_in_room);

  if (ch_bad_guy) {
    if (ch_bad_guy == ch_hero)
      return FALSE;             /* NO WAY I'll rescue the one I'm fighting! */
    act("You bravely rescue $N.", FALSE, ch_hero, 0, ch_victim, TO_CHAR);
    act("You are rescued by $N, your loyal friend!",
	FALSE, ch_victim, 0, ch_hero, TO_CHAR);
    act("$n heroically rescues $N.", FALSE, ch_hero, 0, ch_victim, TO_NOTVICT);

    if (FIGHTING(ch_bad_guy))
      stop_fighting(ch_bad_guy);
    if (FIGHTING(ch_hero))
      stop_fighting(ch_hero);

    set_fighting(ch_hero, ch_bad_guy);
    set_fighting(ch_bad_guy, ch_hero);
    return TRUE;
  }
  return FALSE;
}


/* Procedure to stop a character entering a room.             */
/* guard is the blocking mobile, ch the one trying to move.   */
/* Members of the staff and immortals are let through.        */
int block_way(struct char_data * guard, struct char_data * ch, int cmd,
	      long iIn_room, int iProhibited_direction)
{
  /* Movement commands are numbered from 1 (north) in the command list. */
  if (cmd != iProhibited_direction + 1)
    return FALSE;

  if (guard == ch || !AWAKE(guard) || FIGHTING(guard) ||
      ch->in_room != real_room(iIn_room))
    return FALSE;

  if (member_of_staff(ch) || (!IS_NPC(ch) && GET_LEVEL(ch) >= LVL_IMMORT))
    return FALSE;

  act("$N roars at $n and shoves $m back.", FALSE, ch, 0, guard, TO_ROOM);
  act("$N roars: 'None pass here without the Emperor's leave!', and shoves "
      "you back.", FALSE, ch, 0, guard, TO_CHAR);
  return (TRUE);
}


/* Routine to check if an object is trash.             */
/* Used by the Chamberlain and the chambermaid.        */
/* Containers (corpses!), money, keys and anything     */
/* else of value are left alone.                       */
int is_trash(struct obj_data * i)
{
  if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
    return FALSE;

  switch (GET_OBJ_TYPE(i)) {
  case ITEM_TRASH:
    return TRUE;
  case ITEM_DRINKCON:
    return (GET_OBJ_VAL(i, 1) == 0);    /* only empty ones */
  case ITEM_OTHER:
  case ITEM_WORN:
    return (GET_OBJ_COST(i) <= 10);
  default:
    return FALSE;
  }
}


/* Counts the objects a character carries, so the cleaners do not */
/* hoard forever.                                                  */
static int carried_count(struct char_data *ch)
{
  struct obj_data *o;
  int n = 0;

  for (o = ch->carrying; o; o = o->next_content)
    n++;
  return n;
}


/* ------------------------------------------------------------------ */
/* The Emperor                                                        */
/* ------------------------------------------------------------------ */

/* Opens (and unlocks) or closes (and locks) the door in direction   */
/* dir, on both sides.  The Emperor carries the keys of his palace,  */
/* so this is done directly rather than through do_gen_door.         */
static void castle_door(struct char_data *ch, int dir, bool open, bool lock)
{
  struct room_direction_data *ex, *back;
  long other;
  char dname[MAX_INPUT_LENGTH];

  if (dir < 0 || dir >= NUM_OF_DIRS || !(ex = EXIT(ch, dir)) ||
      !IS_SET(ex->exit_info, EX_ISDOOR) || ex->to_room == NOWHERE)
    return;

  other = ex->to_room;
  back = world[other].dir_option[rev_dir[dir]];
  if (back && back->to_room != ch->in_room)
    back = NULL;

  strcpy(dname, ex->keyword ? fname(ex->keyword) : "door");

  if (open) {
    if (!IS_SET(ex->exit_info, EX_CLOSED))
      return;
    if (IS_SET(ex->exit_info, EX_LOCKED)) {
      sprintf(buf, "$n unlocks the %s and pushes it open.", dname);
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
    } else {
      sprintf(buf, "$n opens the %s.", dname);
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
    }
    REMOVE_BIT(ex->exit_info, EX_CLOSED | EX_LOCKED);
    if (back) {
      REMOVE_BIT(back->exit_info, EX_CLOSED | EX_LOCKED);
      sprintf(buf, "The %s is opened from the other side.\r\n", dname);
      send_to_room(buf, other);
    }
  } else {
    if (IS_SET(ex->exit_info, EX_CLOSED) &&
	(!lock || IS_SET(ex->exit_info, EX_LOCKED)))
      return;
    sprintf(buf, lock ? "$n closes the %s and locks it." :
	    "$n closes the %s.", dname);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    SET_BIT(ex->exit_info, EX_CLOSED);
    if (lock)
      SET_BIT(ex->exit_info, EX_LOCKED);
    if (back) {
      SET_BIT(back->exit_info, EX_CLOSED);
      if (lock)
	SET_BIT(back->exit_info, EX_LOCKED);
      sprintf(buf, "The %s is closed from the other side.\r\n", dname);
      send_to_room(buf, other);
    }
  }
}


/* Function fry_victim */
/* Finds a suitable victim and casts something _NASTY_ at him. */
/* Used by the Emperor while he fights.                        */
void fry_victim(struct char_data * ch)
{
  struct char_data *tch;

  if (number(0, 3))             /* about one call in four */
    return;

  if (!(tch = get_victim(ch)) && !(tch = FIGHTING(ch)))
    return;
  if (tch->in_room != ch->in_room)
    return;

  switch (number(0, 8)) {
  case 1:
  case 2:
  case 3:
    act("You raise your hand in a dramatic gesture.", 1, ch, 0, 0, TO_CHAR);
    act("$n raises $s hand in a dramatic gesture.", 1, ch, 0, 0, TO_ROOM);
    cast_spell(ch, tch, NULL, SPELL_COLOR_SPRAY);
    break;
  case 4:
  case 5:
    act("You concentrate and mutter a word of shadow.", 1, ch, 0, 0, TO_CHAR);
    act("$n concentrates and mutters a word of shadow.", 1, ch, 0, 0, TO_ROOM);
    cast_spell(ch, tch, NULL, SPELL_HARM);
    break;
  case 6:
  case 7:
    act("You look deeply into the eyes of $N.", 1, ch, 0, tch, TO_CHAR);
    act("$n looks deeply into the eyes of $N.", 1, ch, 0, tch, TO_NOTVICT);
    act("You see an ill-boding flame in the eyes of $n.", 1, ch, 0, tch, TO_VICT);
    cast_spell(ch, tch, NULL, SPELL_FIREBALL);
    break;
  default:
    if (GET_HIT(ch) < GET_MAX_HIT(ch) / 2)
      cast_spell(ch, ch, NULL, SPELL_HEAL);
    break;
  }
}


/*
 * The Emperor's day.  Each step is one call of his special procedure
 * (once every ten seconds or so):
 *
 *   0-9   walk in that direction (0 north, 1 east, 2 south, 3 west ...)
 *   o<d>  unlock and open the door in direction d
 *   l<d>  close and lock the door in direction d
 *   x<d>  close the door in direction d without locking it
 *   A-D   proclaim line A-D of the monolog
 *   P     pause
 *   W     wake up and stand          S  go to sleep
 *   r     sit down on the throne     s  stand up
 *   G     greet his bodyguards       g  greet the court
 *   .     end of the path
 *
 * Morning (8:00, bedchamber): he is woken by his bodyguards (or the
 *   castle bell, if they are dead), walks to his throne, locks his
 *   bedchamber behind him and opens the throne room for the day.
 * Noon (12:00, throne room): he goes out into the hall to address
 *   the court, then returns to his throne.
 * Night (21:00, throne room): he locks the throne room and retires to
 *   his bedchamber.
 */
static char *monolog[] = {
  "$n proclaims 'Hear me, Llyndyr!  The banners of the Black Crown fly from sea to hills.'",
  "$n proclaims 'Every land that defied us now fills my treasury.'",
  "$n proclaims 'Serve me well, and you shall share in the spoils of empire.'",
  "$n proclaims 'Fail me, and the SaberBeast shall dine well tonight!'"};

static const char morning_path[] = "Wo33l1o2rG.";
static const char noon_path[]    = "so22ABCDP0rg.";
static const char night_path[]   = "sl2o11l3S.";

static const char *emperor_path = NULL;
static int emperor_index = 0;
static int emperor_hour = -1;           /* hour the current path began */


/* Begins a path, unless one was already begun this hour. */
static void emperor_begin(const char *path)
{
  if (emperor_hour == time_info.hours)
    return;
  emperor_path = path;
  emperor_index = 0;
  emperor_hour = time_info.hours;
}


/* Mobiles that are asleep are skipped by mobile_activity(), so the  */
/* Emperor cannot wake himself.  Every member of the staff calls     */
/* this; at eight o'clock it wakes a sleeping Emperor and starts his */
/* morning walk.                                                     */
static void morning_bell(struct char_data *waker)
{
  struct char_data *emp;
  long room;

  if (time_info.hours != 8 || emperor_hour == 8)
    return;
  if ((room = real_room(ROOM_BEDCHAMBER)) == NOWHERE)
    return;

  for (emp = world[room].people; emp; emp = emp->next_in_room)
    if (IS_NPC(emp) && GET_MOB_VNUM(emp) == MOB_EMPEROR)
      break;

  if (!emp || GET_POS(emp) != POS_SLEEPING)
    return;

  /* leave it to the bodyguards, if they are there to do it */
  if (!waker || waker->in_room != room) {
    struct char_data *tch;

    for (tch = world[room].people; tch; tch = tch->next_in_room)
      if (tch != emp && member_of_staff(tch) && AWAKE(tch) && !FIGHTING(tch))
	return;
  }

  if (waker && waker->in_room == room)
    act("$n gently wakes $N.", FALSE, waker, 0, emp, TO_ROOM);
  else
    send_to_room("Somewhere in the castle a bell tolls the eighth hour.\r\n",
		 room);
  GET_POS(emp) = POS_STANDING;
  act("$n awakens and rises from $s great bed.", FALSE, emp, 0, 0, TO_ROOM);

  emperor_begin(morning_path);
  emperor_index = 1;            /* he is already awake: skip the 'W' */
}


/* Function emperor */
/* Controls the actions and movements of the Emperor. */
SPECIAL(emperor)
{
  struct char_data *self = (struct char_data *) me;
  char step;
  int dir;

  if (cmd || ch != self)
    return FALSE;

  if (GET_POS(ch) == POS_FIGHTING || FIGHTING(ch)) {
    fry_victim(ch);
    return FALSE;
  }

  if (GET_POS(ch) < POS_SLEEPING)
    return FALSE;

  /* a path left over from an Emperor who died part-way through it */
  if (emperor_path && (time_info.hours - emperor_hour + 24) % 24 > 1)
    emperor_path = NULL;

  if (!emperor_path) {
    if (time_info.hours == 12 && ch->in_room == real_room(ROOM_THRONE))
      emperor_begin(noon_path);
    else if (time_info.hours == 21 && ch->in_room == real_room(ROOM_THRONE))
      emperor_begin(night_path);
  }

  if (GET_POS(ch) == POS_SLEEPING || banzaii(ch) || !emperor_path)
    return FALSE;

  step = emperor_path[emperor_index++];

  switch (step) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    GET_POS(ch) = POS_STANDING;
    if (!perform_move(ch, step - '0', 1))
      emperor_path = NULL;      /* something is in the way: give up */
    break;

  case 'o':
  case 'l':
  case 'x':
    dir = emperor_path[emperor_index] - '0';
    if (emperor_path[emperor_index])
      emperor_index++;
    castle_door(ch, dir, step == 'o', step == 'l');
    break;

  case 'A':
  case 'B':
  case 'C':
  case 'D':
    act(monolog[step - 'A'], FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'P':
    break;

  case 'W':
    GET_POS(ch) = POS_STANDING;
    act("$n awakens and stands up.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POS_SLEEPING;
    act("$n lies down on $s great bed and is soon asleep.",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'r':
    GET_POS(ch) = POS_SITTING;
    act("$n seats $mself upon the golden throne.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 's':
    if (GET_POS(ch) == POS_SITTING)
      act("$n rises from $s throne.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;

  case 'G':
    act("$n says 'To your posts, my shadows.  Let no one through.'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'g':
    act("$n says 'Approach, if you dare, and bend the knee.'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case '.':
  default:
    emperor_path = NULL;
    break;
  }

  return FALSE;
}


/* ------------------------------------------------------------------ */
/* The household                                                      */
/* ------------------------------------------------------------------ */

/* Function training_master */
/* The master-at-arms drills his two recruits in the training hall. */
SPECIAL(training_master)
{
  struct char_data *pupil1, *pupil2, *tch;

  if (cmd || ch != (struct char_data *) me)
    return FALSE;

  if (!AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  morning_bell(ch);

  if (!banzaii(ch) && !number(0, 2)) {
    if ((pupil1 = find_npc_by_vnum(ch, MOB_RECRUIT_ALDRIC)) &&
	(pupil2 = find_npc_by_vnum(ch, MOB_RECRUIT_BREN)) &&
	(!FIGHTING(pupil1) && !FIGHTING(pupil2))) {
      if (number(0, 1)) {
	tch = pupil1;
	pupil1 = pupil2;
	pupil2 = tch;
      }
      switch (number(0, 7)) {
      case 0:
	act("$n hits $N on $S head with a powerful blow.",
	    FALSE, pupil1, 0, pupil2, TO_NOTVICT);
	act("You hit $N on $S head with a powerful blow.",
	    FALSE, pupil1, 0, pupil2, TO_CHAR);
	act("$n hits you on your head with a powerful blow.",
	    FALSE, pupil1, 0, pupil2, TO_VICT);
	break;
      case 1:
	act("$n hits $N in $S chest with a thrust.",
	    FALSE, pupil1, 0, pupil2, TO_NOTVICT);
	act("You manage to thrust $N in the chest.",
	    FALSE, pupil1, 0, pupil2, TO_CHAR);
	act("$n manages to thrust you in your chest.",
	    FALSE, pupil1, 0, pupil2, TO_VICT);
	break;
      case 2:
	send_to_char("You command your pupils to bow.\r\n", ch);
	act("$n commands $s pupils to bow.", FALSE, ch, 0, 0, TO_ROOM);
	act("$n bows before $N.",
	    FALSE, pupil1, 0, pupil2, TO_NOTVICT);
	act("$N bows before $n.",
	    FALSE, pupil1, 0, pupil2, TO_NOTVICT);
	act("You bow before $N, who returns your gesture.",
	    FALSE, pupil1, 0, pupil2, TO_CHAR);
	act("You bow before $n, who returns your gesture.",
	    FALSE, pupil1, 0, pupil2, TO_VICT);
	break;
      case 3:
	act("$N yells at $n, as $e fumbles and drops $s sword.",
	    FALSE, pupil1, 0, ch, TO_NOTVICT);
	act("$n quickly picks up $s weapon.", FALSE, pupil1, 0, 0, TO_ROOM);
	act("$N yells at you, as you fumble, losing your weapon.",
	    FALSE, pupil1, 0, ch, TO_CHAR);
	send_to_char("You quickly pick up your weapon again.\r\n", pupil1);
	act("You yell at $n, as $e fumbles, losing $s weapon.",
	    FALSE, pupil1, 0, ch, TO_VICT);
	break;
      case 4:
	act("$N tricks $n, and slashes $m across the back.",
	    FALSE, pupil1, 0, pupil2, TO_NOTVICT);
	act("$N tricks you, and slashes you across your back.",
	    FALSE, pupil1, 0, pupil2, TO_CHAR);
	act("You trick $n, and quickly slash $m across $s back.",
	    FALSE, pupil1, 0, pupil2, TO_VICT);
	break;
      case 5:
	act("$n lunges a blow at $N but $N parries skillfully.",
	    FALSE, pupil1, 0, pupil2, TO_NOTVICT);
	act("You lunge a blow at $N but $E parries skillfully.",
	    FALSE, pupil1, 0, pupil2, TO_CHAR);
	act("$n lunges a blow at you, but you skillfully parry it.",
	    FALSE, pupil1, 0, pupil2, TO_VICT);
	break;
      case 6:
	act("$n clumsily tries to kick $N, but misses.",
	    FALSE, pupil1, 0, pupil2, TO_NOTVICT);
	act("You clumsily miss $N with your poor excuse for a kick.",
	    FALSE, pupil1, 0, pupil2, TO_CHAR);
	act("$n fails an unusually clumsy attempt at kicking you.",
	    FALSE, pupil1, 0, pupil2, TO_VICT);
	break;
      default:
	send_to_char("You show your pupils an advanced technique.\r\n", ch);
	act("$n shows $s pupils an advanced technique.",
	    FALSE, ch, 0, 0, TO_ROOM);
	break;
      }
    }
  }
  return FALSE;
}


/* The twin bodyguards: they follow the Emperor, rescue him and each */
/* other, and keep everyone else out of his bedchamber.              */
static int bodyguard(struct char_data *ch, struct char_data *self, int cmd,
		     long twin_vnum)
{
  struct char_data *emp, *twin;

  if (cmd)                      /* ch is whoever typed the command */
    return block_way(self, ch, cmd, ROOM_THRONE, EAST);

  if (!AWAKE(self))
    return FALSE;

  morning_bell(self);

  if ((emp = find_npc_by_vnum(self, MOB_EMPEROR))) {
    if (!self->master && !FIGHTING(self))
      add_follower(self, emp);
    if (FIGHTING(emp) && do_npc_rescue(self, emp))
      return TRUE;
  }
  if ((twin = find_npc_by_vnum(self, twin_vnum)))
    if (FIGHTING(twin) && 2 * GET_HIT(twin) < GET_HIT(self) &&
	do_npc_rescue(self, twin))
      return TRUE;

  if (GET_POS(self) != POS_FIGHTING)
    banzaii(self);

  return FALSE;
}


SPECIAL(tom)
{
  return bodyguard(ch, (struct char_data *) me, cmd, MOB_BODYGUARD_VORN);
}


SPECIAL(tim)
{
  return bodyguard(ch, (struct char_data *) me, cmd, MOB_BODYGUARD_VARN);
}


/* Routine for the Chamberlain */
/* Complains about any trash he finds and takes it away. */
SPECIAL(James)
{
  struct obj_data *i;

  if (cmd || ch != (struct char_data *) me)
    return (FALSE);
  if (!AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
    return (FALSE);

  morning_bell(ch);

  if (carried_count(ch) >= 20)
    return FALSE;

  for (i = world[ch->in_room].contents; i; i = i->next_content)
    if (is_trash(i)) {
      act("$n sniffs: 'Disgraceful!  I shall have words with that maid.'",
	  FALSE, ch, 0, 0, TO_ROOM);
      act("$n picks up $p between finger and thumb.", FALSE, ch, i, 0, TO_ROOM);
      obj_from_room(i);
      obj_to_char(i, ch);
      return TRUE;
    }

  return FALSE;
}


/* Routine for the chambermaid */
/* Picks up any trash she finds... */
SPECIAL(cleaning)
{
  struct obj_data *i, *next;

  if (cmd || ch != (struct char_data *) me || !AWAKE(ch))
    return (FALSE);

  morning_bell(ch);

  if (carried_count(ch) >= 20)
    return FALSE;

  for (i = world[ch->in_room].contents; i; i = next) {
    next = i->next_content;
    if (is_trash(i)) {
      act("$n sweeps up some rubbish.", FALSE, ch, 0, 0, TO_ROOM);
      obj_from_room(i);
      obj_to_char(i, ch);
      return TRUE;
    }
  }
  return FALSE;
}


/* Routine CastleGuard */
/* Standard routine for ordinary castle guards. */
SPECIAL(CastleGuard)
{
  if (cmd || ch != (struct char_data *) me)
    return FALSE;
  if (!AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  morning_bell(ch);
  return (banzaii(ch));
}


/* Routine DicknDavid */
/* The two wardens keep the door of the throne room. */
SPECIAL(DicknDavid)
{
  struct char_data *self = (struct char_data *) me;

  if (cmd)
    return (block_way(self, ch, cmd, ROOM_HALL, NORTH));

  if (!AWAKE(self))
    return (FALSE);

  morning_bell(self);

  if (GET_POS(self) != POS_FIGHTING)
    banzaii(self);

  return FALSE;
}


/* Routine peter */
/* Routine for the Captain of the Guard. */
SPECIAL(peter)
{
  struct char_data *ch_guard;

  if (cmd || ch != (struct char_data *) me)
    return (FALSE);
  if (!AWAKE(ch) || GET_POS(ch) == POS_FIGHTING)
    return (FALSE);

  morning_bell(ch);

  if (banzaii(ch))
    return FALSE;

  if (!(number(0, 3)) && (ch_guard = find_guard(ch)))
    switch (number(0, 5)) {
    case 0:
      act("$N snaps to attention as $n inspects $M.",
	  FALSE, ch, 0, ch_guard, TO_NOTVICT);
      act("$N snaps to attention as you inspect $M.",
	  FALSE, ch, 0, ch_guard, TO_CHAR);
      act("You snap to attention as $n inspects you.",
	  FALSE, ch, 0, ch_guard, TO_VICT);
      break;
    case 1:
      act("$N looks very small, as $n roars at $M.",
	  FALSE, ch, 0, ch_guard, TO_NOTVICT);
      act("$N looks very small as you roar at $M.",
	  FALSE, ch, 0, ch_guard, TO_CHAR);
      act("You feel very small as $n roars at you.",
	  FALSE, ch, 0, ch_guard, TO_VICT);
      break;
    case 2:
      act("$n gives $N $s orders for the watch.",
	  FALSE, ch, 0, ch_guard, TO_NOTVICT);
      act("You give $N $S orders for the watch.",
	  FALSE, ch, 0, ch_guard, TO_CHAR);
      act("$n gives you your orders for the watch.",
	  FALSE, ch, 0, ch_guard, TO_VICT);
      break;
    case 3:
      act("$n looks at you.", FALSE, ch, 0, ch_guard, TO_VICT);
      act("$n looks at $N.", FALSE, ch, 0, ch_guard, TO_NOTVICT);
      act("$n growls: 'Those boots need polishing!'",
	  FALSE, ch, 0, ch_guard, TO_ROOM);
      act("You growl at $N.", FALSE, ch, 0, ch_guard, TO_CHAR);
      break;
    case 4:
      act("$n looks at you.", FALSE, ch, 0, ch_guard, TO_VICT);
      act("$n looks at $N.", FALSE, ch, 0, ch_guard, TO_NOTVICT);
      act("$n growls: 'Straighten that cloak!  You wear the Emperor's colours!'",
	  FALSE, ch, 0, ch_guard, TO_ROOM);
      act("You growl at $N.", FALSE, ch, 0, ch_guard, TO_CHAR);
      break;
    default:
      act("$n looks at you.", FALSE, ch, 0, ch_guard, TO_VICT);
      act("$n looks at $N.", FALSE, ch, 0, ch_guard, TO_NOTVICT);
      act("$n growls: 'That chain mail looks rusty!  CLEAN IT!!!'",
	  FALSE, ch, 0, ch_guard, TO_ROOM);
      act("You growl at $N.", FALSE, ch, 0, ch_guard, TO_CHAR);
      break;
    }

  return FALSE;
}


/* Procedure for the off-duty guardsman dicing in the inn. */
/* His partner is any guard on duty who happens to be there. */
/* Code by Sapowox modified by Pjotr. (Original code from Master) */
SPECIAL(jerry)
{
  struct char_data *gambler1, *gambler2, *tch;

  if (cmd || ch != (struct char_data *) me)
    return FALSE;
  if (!AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  morning_bell(ch);

  if (!banzaii(ch) && !number(0, 2)) {
    if ((gambler1 = ch) &&
	(gambler2 = find_guard(ch)) &&
	(!FIGHTING(gambler1) && !FIGHTING(gambler2))) {
      if (number(0, 1)) {
	tch = gambler1;
	gambler1 = gambler2;
	gambler2 = tch;
      }
      switch (number(0, 5)) {
      case 0:
	act("$n rolls the dice and cheers loudly at the result.",
	    FALSE, gambler1, 0, gambler2, TO_NOTVICT);
	act("You roll the dice and cheer. GREAT!",
	    FALSE, gambler1, 0, gambler2, TO_CHAR);
	act("$n cheers loudly as $e rolls the dice.",
	    FALSE, gambler1, 0, gambler2, TO_VICT);
	break;
      case 1:
	act("$n curses the Goddess of Luck roundly as $e sees $N's roll.",
	    FALSE, gambler1, 0, gambler2, TO_NOTVICT);
	act("You curse the Goddess of Luck as $N rolls.",
	    FALSE, gambler1, 0, gambler2, TO_CHAR);
	act("$n swears angrily. You are in luck!",
	    FALSE, gambler1, 0, gambler2, TO_VICT);
	break;
      case 2:
	act("$n sighs loudly and gives $N some gold.",
	    FALSE, gambler1, 0, gambler2, TO_NOTVICT);
	act("You sigh loudly at the pain of having to give $N some gold.",
	    FALSE, gambler1, 0, gambler2, TO_CHAR);
	act("$n sighs loudly as $e gives you your rightful win.",
	    FALSE, gambler1, 0, gambler2, TO_VICT);
	break;
      case 3:
	act("$n smiles ruefully as $N's roll tops $s.",
	    FALSE, gambler1, 0, gambler2, TO_NOTVICT);
	act("You smile sadly as you see that $N beats you. Again.",
	    FALSE, gambler1, 0, gambler2, TO_CHAR);
	act("$n smiles ruefully as your roll tops $s.",
	    FALSE, gambler1, 0, gambler2, TO_VICT);
	break;
      case 4:
	act("$n excitedly follows the dice with $s eyes.",
	    FALSE, gambler1, 0, gambler2, TO_NOTVICT);
	act("You excitedly follow the dice with your eyes.",
	    FALSE, gambler1, 0, gambler2, TO_CHAR);
	act("$n excitedly follows the dice with $s eyes.",
	    FALSE, gambler1, 0, gambler2, TO_VICT);
	break;
      default:
	act("$n says 'Well, my luck has to change soon', as $e shakes the dice.",
	    FALSE, gambler1, 0, gambler2, TO_NOTVICT);
	act("You say 'Well, my luck has to change soon' and shake the dice.",
	    FALSE, gambler1, 0, gambler2, TO_CHAR);
	act("$n says 'Well, my luck has to change soon', as $e shakes the dice.",
	    FALSE, gambler1, 0, gambler2, TO_VICT);
	break;
      }
    }
  }
  return FALSE;
}
