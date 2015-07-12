/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ACT_MOVE.C                                   Based on CircleMUD *
*  Usage: Movement commands, door handling, & sleep/rest/etc state        *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#define __ACT_MOVE_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/* external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern int rev_dir[];
extern char *dirs[];
extern int movement_loss[];

/* external functs */
int  special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
int  find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);
/* For TD 6/10/95 */
void mprog_greet_trigger(struct char_data * ch);
void mprog_entry_trigger(struct char_data * mob);
void Dismount(struct char_data *ch, struct char_data *h, int pos);
int  check_trigger_trap(struct char_data *ch, void *vo, int trigger_type);
ACMD(do_move);
ACMD(do_infobar);
void raw_kill(struct char_data *ch, struct char_data *ki);

/* simple function to determine if char can walk on water */
int has_boat(struct char_data *ch)
{
  struct obj_data  *obj;
  struct char_data *mount;
  int i;

  if (MOUNTED(ch)) {
     mount = MOUNTED(ch);
     if (IS_AFFECTED(mount, AFF_WATERWALK) || \
	 IS_ITEM_AFF(mount, AFFECTED_WATERWALK))
	return 1;
  }

  if (!MOUNTED(ch) && (IS_AFFECTED(ch, AFF_WATERWALK) || \
      IS_ITEM_AFF(ch, AFFECTED_WATERWALK)))
    return 1;

  /* non-wearable boats in inventory will do it */
  for (obj = ch->carrying; obj; obj = obj->next_content)
    if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, NULL) < 0))
      return 1;

  /* and any boat you're wearing will do it too */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
      return 1;

  /* Just so imms know that they would have needed a boat - */
  /*    This is so we can check defective gear....          */
  if (GET_LEVEL(ch) >= LVL_IMMORT) {
     send_to_char("Since you are an immortal you do not need a boat.\r\n", ch);
     return 1;
  }

  return 0;
}

/* simple function to determine if char can fly */
int can_fly(struct char_data *ch)
{
  struct char_data *mount;

  if (MOUNTED(ch)) {
     mount = MOUNTED(ch);
     if (IS_AFFECTED2(mount, AFF_FLYING) || \
	 IS_ITEM_AFF(mount, AFFECTED_FLYING))
	return 1;
  }

  if (!MOUNTED(ch) && (IS_AFFECTED2(ch, AFF_FLYING) || \
      IS_ITEM_AFF(ch, AFFECTED_FLYING)))
    return 1;

  /* Just so imms know that they would have needed to fly - */
  /*    This is so we can check defective gear....          */
  if (GET_LEVEL(ch) >= LVL_IMMORT) {
     send_to_char("Since you are immortal you automatically begin to float in the air.\r\n", ch);
     SET_BIT(AFF2_FLAGS(ch), AFF_FLYING);
     return 1;
  }

  return 0;
}


/* simple function to check for underwater breathing */
int can_breathe_underwater(struct char_data *ch)
{

  if (IS_AFFECTED(ch, AFF_WATERBREATH) || \
      IS_ITEM_AFF(ch, AFFECTED_WATERBREATH))
     return 1;

  /* Just so imms know that they would have needed to breathe - */
  /*    This is so we can check defective gear....              */
  if (GET_LEVEL(ch) >= LVL_IMMORT) {
     send_to_char("Gills spout from your neck so you can enter the water.\r\n", ch);
     act("Gills spout from $n's neck as $e enters the water.\r\n", TRUE, ch, 0, 0, TO_ROOM);
     return 1;
  }

  return 0;
}


/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns :
 *   1 : If succes.
 *   0 : If fail
 */
int do_simple_move(struct char_data *ch, int dir, int need_specials_check)
{
  int    was_in, need_movement, prob;
  struct char_data *mount = NULL;

  int special(struct char_data *ch, int cmd, char *arg);

  if (ch == NULL)
    return 0;
  /*
   * Check for special routines (North is 1 in command list, but 0 here) Note
   * -- only check if following; this avoids 'double spec-proc' bug
   */
  if (need_specials_check && special(ch, dir + 1, ""))
    return 0;

  /* BAD FIX: This keeps TD from crashing, but where did the PC come from? */
  if (ch->in_room == NOWHERE) {
     ch->in_room = real_room(TEMPLE_SQUARE_VNUM);
     sprintf(buf, "SYSERR: %s was in room NOWHERE, transporting to %d.\n",
	    GET_NAME(ch), TEMPLE_SQUARE_VNUM);
     log(buf);
  }

  if (MOUNTED(ch))
     mount = MOUNTED(ch);

  /* charmed? */
  if (ch->desc!=NULL && (IS_AFFECTED(ch, AFF_CHARM) && ch->master && ch->in_room == ch->master->in_room)) {
    send_to_char("The thought of leaving your master makes you weep.\r\n", ch);
    return 0;
  }

  if (ch->desc!=NULL && IS_AFFECTED2(ch, AFF_BOUND)) {
     send_to_char("You are cuffed, you're not going anywhere.\r\n", ch);
     return 0;
  }

  /* Another fix to keep TD from crashing */
  if (ch->desc != NULL && !(EXIT(ch, dir)->to_room)) {
     send_to_char("That direction doesn't seem to lead anywhere!\r\n", ch);
     send_to_char("We must be working in minimud mode, or someone made an exit that goes nowhere :(\r\n", ch);
     return 0;
  }

  if (!(EXIT(ch, dir)->to_room)) {
    return 0;
  }

  if (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_BLADEBARRIER)) {
    act("$n tries to pass through the wall of blades blocking the path and is wounded.", TRUE, ch, 0, 0, TO_ROOM);
    send_to_char("You try to pass through the wall of blades but are prevented!\r\n", ch);
    GET_HIT(ch) -= dice(6, 5);
    update_pos(ch);
    switch (GET_POS(ch)) {
      case POS_MORTALLYW:
        act("$n is mortally wounded, and will die soon, if not aided.", TRUE, ch, 0, 0, TO_ROOM);
        send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", ch);
        break;
      case POS_INCAP:
        act("$n is incapacitated and will slowly die, if not aided.", TRUE, ch, 0, 0, TO_ROOM);
        send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", ch);
        break;
      case POS_STUNNED:
        act("$n is stunned, but will probably regain consciousness again.", TRUE, ch, 0, 0, TO_ROOM);
        send_to_char("You're stunned, but will probably regain consciousness again.\r\n", ch);
        break;
      case POS_DEAD:
        act("$n is dead!  R.I.P.", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("You are dead!  Sorry...\r\n", ch);
        break;
      default:                      /* >= POSITION SLEEPING */
        if (GET_HIT(ch) < (GET_MAX_HIT(ch) >> 2) || GET_WOUNDS(ch) > 5) {
          sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
  	          CCRED(ch, C_SPR), CCNRM(ch, C_SPR));
          send_to_char(buf2, ch);
        }
        break;
    }   
    if (GET_POS(ch) == POS_DEAD) {
      if (!IS_NPC(ch)) {
         sprintf(buf2, "%s killed by a blade barrier at %s", GET_NAME(ch),
	      world[ch->in_room].name);
         mudlog(buf2, BRF, LVL_IMMORT, TRUE);
       }
       raw_kill(ch, NULL);
    }    
    return 0;
  }

  /* So mounts will not lead riders to their death - smart animals! */
  if ((MOUNTED(ch) || RIDDEN(ch)) && 
      (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_DEATH) || 
      (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_INDOORS) &&
       !IS_SET(ROOM_FLAGS(ch->in_room), ROOM_INDOORS)))) {
      send_to_char("Your mount refuses to go there.\r\n",ch);
      return 0;
  }

  /* if this room or the one we're going to needs a boat, check for one */
  if ((SECT(ch->in_room) == SECT_WATER_NOSWIM) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM)) {
    if (!has_boat(ch)) {
      send_to_char("You need a boat to go there.\r\n", ch);
      return 0;
    }
  }

  /* if this room or the one we're going to is in the air - check flight */
  if ((SECT(ch->in_room) == SECT_FLYING) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_FLYING)) {
    if (!can_fly(ch)) {
      send_to_char("You need to be able to fly to go there.\r\n", ch);
      return 0;
    }
  }

  /* Check for drowning people */
  if (SECT(ch->in_room) == SECT_UNDERWATER) {
     if (!can_breathe_underwater(ch)) {
	send_to_char("Your lungs begin to fill with sea water!\r\n", ch);
	damage(ch, ch, (GET_MAX_HIT(ch)/8), TYPE_DROWNING, -1);
	do_move(ch, "up", 0, SCMD_UP);
     }
  }

  /* if the room we're going to is underwater check for it */
  if (SECT(EXIT(ch, dir)->to_room) == SECT_UNDERWATER) {
    if (!can_breathe_underwater(ch)) {
      send_to_char("You need to be able to breathe underwater to go there.\r\n", ch);
      return 0;
    }
  }

  /* move points needed is avg. move loss for src and destination sect type */
  need_movement = (movement_loss[SECT(ch->in_room)] +
		   movement_loss[SECT(EXIT(ch, dir)->to_room)]) >> 1;
  /* move points needed for extreme weather situations */
  if (!ROOM_FLAGGED((EXIT(ch, dir)->to_room), ROOM_INDOORS)) {
    if ((WEATHER(ch->in_room) == SKY_POURING) || (WEATHER(ch->in_room) == SKY_SNOWSTORM))
      need_movement = need_movement + 2;
    else if ((WEATHER(ch->in_room) == SKY_THUNDERSTORM) || (WEATHER(ch->in_room) == SKY_BLIZZARD))
      need_movement = need_movement + 4;
    else if (WEATHER(ch->in_room) == SKY_TORNADO)
      need_movement = need_movement + 6;
  }

  if (GET_MOVE(ch) < need_movement && !IS_NPC(ch) && !MOUNTED(ch)) {
    if (need_specials_check && ch->master)
      send_to_char("You are too exhausted to follow.\r\n", ch);
    else
      send_to_char("You are too exhausted.\r\n", ch);

    return 0;
  } else if (MOUNTED(ch) && GET_MOVE(mount) < need_movement) {
    send_to_char("Your mount is too exhausted.\r\n", ch);
    return 0;
  }

  if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_ATRIUM)) {
    if (!House_can_enter(ch, world[EXIT(ch, dir)->to_room].number)) {
      send_to_char("That's private property -- no trespassing!\r\n", ch);
      return 0;
    }
  }
  if (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) &&
      num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) > 1) {
    send_to_char("There isn't enough room there for more than one person!\r\n", ch);
    return 0;
  }
  if (GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch) && !MOUNTED(ch))
    GET_MOVE(ch) -= need_movement;
  else if (MOUNTED(ch) && (GET_LEVEL(mount) < LVL_IMMORT))
    GET_MOVE(mount) -= need_movement;

  if ((((!IS_AFFECTED(ch, AFF_SNEAK)) && !SHADOWING(ch)) && !RIDDEN(ch)) &&
	(GET_COND(ch, DRUNK) <= 9)) {
     if (MOUNTED(ch))
	sprintf(buf2, "$n rides %s on $N.", dirs[dir]);
     else
	sprintf(buf2, "$n leaves %s.", dirs[dir]);
    act(buf2, TRUE, ch, 0, mount, TO_ROOM);
  }

  if (GET_COND(ch, DRUNK) > 9) {
     sprintf(buf2, "$n stumbles drunkenly to the %s.", dirs[dir]);
     act(buf2, TRUE, ch, 0, 0, TO_ROOM);
  }
  check_trigger_trap(ch, NULL, TRIG_LEAVE_ROOM);

  was_in = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);
  if (MOUNTED(ch)) {
    char_from_room(mount);
    char_to_room(mount, world[was_in].dir_option[dir]->to_room);
  }

  if ((((!IS_AFFECTED(ch, AFF_SNEAK)) && !SHADOWING(ch)) && !RIDDEN(ch)) &&
	(GET_COND(ch, DRUNK) <= 9)) {
     if (MOUNTED(ch))
	act("$n has arrived riding on $N.", TRUE, ch, 0, mount, TO_ROOM);
     else
	act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if (GET_COND(ch, DRUNK) > 9)
     act("$n stumbles into the room.", TRUE, ch, 0, 0, TO_ROOM);

  if (ch->desc != NULL)
    look_at_room(ch, 0);
  /* check to see if player set off trap entering room */
  check_trigger_trap(ch, NULL, TRIG_ENTER_ROOM);

  if ((MOUNTED(ch)) && (ch->in_room != MOUNTED(ch)->in_room))
     Dismount(ch, MOUNTED(ch), POS_STANDING);

  prob = number(1, 101);

  if ((IS_SET(ROOM_FLAGS(ch->in_room), ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT) &&
      (prob > GET_SKILL(ch, SKILL_TREADLIGHT))) {
    log_death_trap(ch);
    death_cry(ch);
    extract_char(ch);
    return 0;
  }
  mprog_entry_trigger(ch);
  mprog_greet_trigger(ch);
  return 1;
}


int perform_move(struct char_data *ch, int dir, int need_specials_check)
{
  int was_in;
  struct follow_type *k, *next;

  if (ch == NULL || dir < 0 || dir >= NUM_OF_DIRS)
    return 0;
  else if (!EXIT(ch, dir) || (EXIT(ch, dir)->to_room == NOWHERE) || \
	   IS_SET(EXIT(ch, dir)->exit_info, EX_ARROWSLIT))
    send_to_char("Alas, you cannot go that way...\r\n", ch);
  else if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED) &&
	   !IS_SET(EXIT(ch, dir)->exit_info, EX_SECRET)) {
    if (EXIT(ch, dir)->keyword) {
      sprintf(buf2, "The %s seems to be closed.\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf2, ch);
    } else
      send_to_char("It seems to be closed.\r\n", ch);
  } else {
    if (!ch->followers)
      return (do_simple_move(ch, dir, need_specials_check));

    was_in = ch->in_room;
    if (!do_simple_move(ch, dir, need_specials_check))
      return 0;

    for (k = ch->followers; k; k = next) {
      next = k->next;
      if ((k->follower->in_room == was_in) &&
	  (GET_POS(k->follower) >= POS_STANDING)) {
	act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
	perform_move(k->follower, dir, 1);
      }
    }
    return 1;
  }
  return 0;
}


ACMD(do_move)
{
  /*
   * This is basically a mapping of cmd numbers to perform_move indices.
   * It cannot be done in perform_move because perform_move is called
   * by other functions which do not require the remapping.
   */
  if (IS_AFFECTED2(ch, AFF_FIREWALL)) {
    send_to_char("You attempt to pass through the wall of fire and it burns you!\r\n", ch);
    GET_HIT(ch) -= dice(10, 5);
    update_pos(ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_BLADEBARRIER)) {
    send_to_char("The wall of blades prevents you from leaving!\r\n", ch);
    act("$n tries to pass through the wall of blades.", TRUE, ch, 0, 0, TO_ROOM);
    GET_HIT(ch) -= dice(5, 5);
    update_pos(ch);
    return;
  }
  perform_move(ch, subcmd - 1, 0);
}


int find_door(struct char_data *ch, char *type, char *dir, char *cmdname) {
  int door;

  if (*dir) {                   /* a direction was specified */
    if ((door = search_block(dir, dirs, FALSE)) == -1) {        /* Partial Match */
      send_to_char("That's not a direction.\r\n", ch);
      return -1;
    }
    if (EXIT(ch, door)) {
      if (EXIT(ch, door)->keyword) {
	if (isname(type, EXIT(ch, door)->keyword))
	  return door;
	else {
	  sprintf(buf2, "I see no %s there.\r\n", type);
	  send_to_char(buf2, ch);
	  return -1;
        }
      } else
        return door;
    } else {
      send_to_char("I really don't see how you can close anything there.\r\n", ch);
      return -1;
    }
  } else {                      /* try to locate the keyword */
    if (!*type) {
      sprintf(buf2, "What is it you want to %s?\r\n", cmdname);
      send_to_char(buf2, ch);
      return -1;
    }
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (isname(type, EXIT(ch, door)->keyword))
	    return door;

    sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
    send_to_char(buf2, ch);
    return -1;
  }
}


int has_key(struct char_data *ch, long key, int subcmd)
{
  struct obj_data *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == key)
      return 1;

  if (GET_EQ(ch, WEAR_WIELD))
    if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_WIELD)) == key)
      return 1;

  if (GET_EQ(ch, WEAR_HOLD))
    if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key)
      return 1;

  if ((GET_LEVEL(ch) >= LVL_IMMORT) && (subcmd == SCMD_LOCK ||
       subcmd == SCMD_UNLOCK)) {
     send_to_char("Your hand turns into the shape of a key!\r\n", ch);
     return 1;
  }
  return 0;
}



#define NEED_OPEN       1
#define NEED_CLOSED     2
#define NEED_UNLOCKED   4
#define NEED_LOCKED     8

char *cmd_door[] =
{
  "open",
  "close",
  "unlock",
  "lock",
  "pick"
};

const int flags_door[] =
{
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_OPEN,
  NEED_CLOSED | NEED_LOCKED,
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_CLOSED | NEED_LOCKED
};


#define EXITN(room, door)               (world[room].dir_option[door])
#define OPEN_DOOR(room, obj, door)      ((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)      ((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd)
{
  int other_room = 0;
  struct room_direction_data *back = 0;

  sprintf(buf, "$n %ss ", cmd_door[scmd]);
  if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
    if ((back = world[other_room].dir_option[rev_dir[door]]))
      if (back->to_room != ch->in_room)
	back = 0;

  switch (scmd) {
  case SCMD_OPEN:
    check_trigger_trap(ch, NULL, TRIG_OPEN_DOOR);
  case SCMD_CLOSE:
    check_trigger_trap(ch, NULL, TRIG_CLOSE_DOOR);
    OPEN_DOOR(ch->in_room, obj, door);
    if (back)
      OPEN_DOOR(other_room, obj, rev_dir[door]);
    send_to_char(OK, ch);
    break;
  case SCMD_UNLOCK:
  case SCMD_LOCK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("*Click*\r\n", ch);
    break;
  case SCMD_PICK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("The lock quickly yields to your skills.\r\n", ch);
    strcpy(buf, "$n skillfully picks the lock on ");
    break;
  }

  /* Notify the room */
  sprintf(buf + strlen(buf), "%s%s.", ((obj) ? "" : "the "), (obj) ? "$p" :
	  (EXIT(ch, door)->keyword ? "$F" : "door"));
  if (!(obj) || (obj->in_room != NOWHERE))
    act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);

  /* Notify the other room */
  if ((scmd == SCMD_OPEN || scmd == SCMD_CLOSE) && back) {
    sprintf(buf, "The %s is %s%s from the other side.\r\n",
	 (back->keyword ? fname(back->keyword) : "door"), cmd_door[scmd],
	    (scmd == SCMD_CLOSE) ? "d" : "ed");
     if (world[EXIT(ch, door)->to_room].people) {
       act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_ROOM);
       act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_CHAR);
     }
  }
}


int ok_pick(struct char_data *ch, long keynum, int pickproof, int scmd)
{
  int percent;
  struct affected_type af;

  percent = number(1, 101);

  if (scmd == SCMD_PICK) {
    if (keynum < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    else if (pickproof)
      send_to_char("It resists your attempts at picking it.\r\n", ch);
    else if (percent > GET_SKILL(ch, SKILL_PICK_LOCK)) {
      send_to_char("You failed to pick the lock.\r\n", ch);
      if (number(10, 110) > GET_SKILL(ch, SKILL_DETECT_TRAP)) {
	 send_to_char("Something sharp pokes your finger, you feel a burning sensation.\r\n", ch);
	 af.type = SPELL_POISON;
	 af.duration = 24;
	 af.modifier = -3;
	 af.location = APPLY_CON;
	 af.bitvector= AFF_POISON_II;
	 affect_to_char(ch,&af, TRUE);
      }
    } else
      return (1);
    return (0);
  }
  return (1);
}


#define DOOR_IS_OPENABLE(ch, obj, door) ((obj) ? \
			((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE))) :\
			(IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door)     ((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)))
#define DOOR_IS_UNLOCKED(ch, obj, door) ((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)))
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_PICKPROOF)) : \
			(IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)))

#define DOOR_IS_CLOSED(ch, obj, door)   (!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)   (!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)         ((obj) ? (GET_OBJ_VAL(obj, 2)) : \
					(EXIT(ch, door)->key))
#define DOOR_LOCK(ch, obj, door)        ((obj) ? (GET_OBJ_VAL(obj, 1)) : \
					(EXIT(ch, door)->exit_info))

ACMD(do_gen_door)
{
  int  door = 0;
  long keynum;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct obj_data *obj = NULL;
  struct char_data *victim = NULL;

  skip_spaces(&argument);
  if (!*argument) {
    sprintf(buf, "%s what?\r\n", cmd_door[subcmd]);
    send_to_char(CAP(buf), ch);
    return;
  }
  two_arguments(argument, type, dir);
  if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    door = find_door(ch, type, dir, cmd_door[subcmd]);

  if ((obj) || (door >= 0)) {
    keynum = DOOR_KEY(ch, obj, door);
    if (!(DOOR_IS_OPENABLE(ch, obj, door)))
      act("You can't $F that!", FALSE, ch, 0, cmd_door[subcmd], TO_CHAR);
    else if (!DOOR_IS_OPEN(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_OPEN)) {
	 send_to_char("But it's already closed!\r\n", ch);
    } else if (!DOOR_IS_CLOSED(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_CLOSED)) {
	 send_to_char("But it's currently open!\r\n", ch);
    } else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all..\r\n", ch);
    else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_UNLOCKED))
      send_to_char("It seems to be locked.\r\n", ch);
    else if (!has_key(ch, keynum, subcmd) && (GET_LEVEL(ch) < LVL_GOD) &&
	     ((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
      send_to_char("You don't seem to have the proper key.\r\n", ch);
    else if (ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), subcmd))
      do_doorcmd(ch, obj, door, subcmd);
  }
  if (PRF_FLAGGED(ch, PRF_INFOBAR))
    do_infobar(ch, 0, 0, SCMDB_EXITS);
  return;
}



ACMD(do_enter)
{
  int door;

  one_argument(argument, buf);

  if (*buf) {                   /* an argument was supplied, search for door
				 * keyword */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    sprintf(buf2, "There is no %s here.\r\n", buf);
    send_to_char(buf2, ch);
  } else if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
    send_to_char("You are already indoors.\r\n", ch);
  else {
    /* try to locate an entrance */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if ((!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	      IS_SET(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) &&
	      !IS_SET(EXIT(ch, door)->exit_info, EX_ARROWSLIT)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char("You can't seem to find anything to enter.\r\n", ch);
  }
}


ACMD(do_leave)
{
  int door;

  if (!IS_SET(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
    send_to_char("You are outside.. where do you want to go?\r\n", ch);
  else {
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if ((!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	    !IS_SET(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) &&
	     !IS_SET(EXIT(ch, door)->exit_info, EX_ARROWSLIT)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char("I see no obvious exits to the outside.\r\n", ch);
  }
}


ACMD(do_stand)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SITTING:
    act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_RESTING:
    act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_SLEEPING:
    act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Do you not consider fighting as standing?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_MOUNTED:
    act("You must dismount first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and put your feet on the ground.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and puts $s feet on the ground.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  }
}


ACMD(do_sit)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SITTING:
    send_to_char("You're sitting already.\r\n", ch);
    break;
  case POS_RESTING:
    act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SLEEPING:
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Sit down while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_MOUNTED:
    act("You must dismount first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_rest)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_SITTING:
    act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_RESTING:
    act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SLEEPING:
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Rest while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_MOUNTED:
    act("You must dismount first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and stop to rest your tired bones.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_sleep)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
  case POS_SITTING:
  case POS_RESTING:
    send_to_char("You go to sleep.\r\n", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  case POS_SLEEPING:
    send_to_char("You are already sound asleep.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
    break;
  case POS_MOUNTED:
    act("You must dismount first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and lie down to sleep.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and lie down to sleep.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  }
}


ACMD(do_wake)
{
  struct char_data *vict;
  int self = 0;

  one_argument(argument, arg);
  if (*arg) {
    if (GET_POS(ch) == POS_SLEEPING)
      send_to_char("Maybe you should wake yourself up first.\r\n", ch);
    else if ((vict = get_char_room_vis(ch, arg)) == NULL)
      send_to_char(NOPERSON, ch);
    else if (vict == ch)
      self = 1;
    else if (GET_POS(vict) > POS_SLEEPING)
      act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
    else if (IS_AFFECTED2(vict, AFF_SLEEP))
      act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
    else if (GET_POS(vict) < POS_SLEEPING)
      act("$E's in pretty bad shape!", FALSE, ch, 0, vict, TO_CHAR);
    else {
      act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
      act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      GET_POS(vict) = POS_SITTING;
    }
    if (!self)
      return;
  }
  if (IS_AFFECTED2(ch, AFF_SLEEP))
    send_to_char("You can't wake up!\r\n", ch);
  else if (GET_POS(ch) > POS_SLEEPING)
    send_to_char("You are already awake...\r\n", ch);
  else {
    send_to_char("You awaken, stand up and stretch the sleep from your bones.\r\n", ch);
    act("$n awakens, stands up and stretches.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
  }
}


ACMD(do_follow)
{
  struct char_data *leader;

  void stop_follower(struct char_data *ch);
  void add_follower(struct char_data *ch, struct char_data *leader);

  one_argument(argument, buf);

  if (*buf) {
    if (!(leader = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    }
  } else {
    send_to_char("Whom do you wish to follow?\r\n", ch);
    return;
  }

  if (ch->master == leader) {
    act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
    return;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {
    act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
  } else {                      /* Not Charmed follow person */
    if (leader == ch) {
      if (!ch->master) {
	send_to_char("You are already following yourself.\r\n", ch);
	return;
      }
      stop_follower(ch);
    } else {
      if (circle_follow(ch, leader)) {
	act("Sorry, but following in loops is not allowed.", FALSE, ch, 0, 0, TO_CHAR);
	return;
      }
      if (ch->master)
	stop_follower(ch);
      REMOVE_BIT(AFF2_FLAGS(ch), AFF_GROUP);
      add_follower(ch, leader);
    }
  }
}
