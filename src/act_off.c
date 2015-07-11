/************************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ACT_OFF.C                                    Based on CircleMUD *
*  Usage: Player-level commands of an offensive nature                    *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#define __ACT_OFF_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern int pk_allowed;

/* extern functions */
void   raw_kill(struct char_data * ch, struct char_data * vict);
void   Dismount(struct char_data *ch, struct char_data *h, int pos);
struct obj_data *make_headless_corpse(struct obj_data *i, int j, struct char_data *c);
void   make_head(struct obj_data *i, int j, struct char_data *c);
int    find_door(struct char_data *ch, char *t, char *d, char *cmdname);
void   learn_from_mistake(struct char_data *ch, int ski);
void   hunt_victim(struct  char_data *hunter);
ACMD(do_remove);
ACMD(do_wield);
ACMD(do_say);
extern int max_exp_gain;
void group_gain(struct char_data *ch, struct char_data *vict);
void change_alignment(struct char_data *ch, struct char_data *vict);
void die(struct char_data *vict, struct char_data *ch);

ACMD(do_assist)
{
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Whom do you wish to assist?\r\n", ch);
  else if (!(helpee = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (helpee == ch)
    send_to_char("You can't help yourself any more than this!\r\n", ch);
  else {
    for (opponent = world[ch->in_room].people;
	 opponent && (FIGHTING(opponent) != helpee);
	 opponent = opponent->next_in_room)
		;

    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!pk_allowed && !IS_NPC(opponent))  /* prevent accidental pkill */
      act("Use 'murder' if you really want to attack $N.", FALSE,
	  ch, 0, opponent, TO_CHAR);
    else {
      send_to_char("You join the fight!\r\n", ch);
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      hit(ch, opponent, TYPE_UNDEFINED);
    }
  }
}


ACMD(do_hit)
{
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Hit who?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else if (vict == ch) {
    send_to_char("You hit yourself...OUCH!.\r\n", ch);
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  } else {
    if (!pk_allowed) {
      if (!IS_NPC(vict) && !IS_NPC(ch) && (subcmd != SCMD_MURDER)) {
	send_to_char("Use 'murder' to hit another player.\r\n", ch);
	return;
      }
      if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(vict))
	return; /* you can't order a charmed pet to attack a player */

      if (!IS_NPC(vict) && !IS_NPC(ch) && (GET_LEVEL(vict) <= 5 ||
	  GET_LEVEL(ch) <= 5))
	return;
    }
    if ((GET_POS(ch) >= POS_STANDING) && (vict != FIGHTING(ch))) {
      hit(ch, vict, TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
    } else if (GET_SKILL(ch, SKILL_SWITCH_OPPONENTS) > number(1, 101)) {
      hit(ch, vict, TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE + 3);
    } else
      send_to_char("You do the best you can!\r\n", ch);
  }
}

#if 0
/*
 *  This function takes the place of all the do_<skillname> functions
 * by encapsuating them in one function which attempts to find the
 * target of the skill.  Kinda like do_cast.
 */
ACMD(do_violent_skill)
{
  struct char_data *vict = NULL;
  struct obj_data  *tobj = NULL, *weapon = NULL;
  int prob, percent = number(1, 101), i = 0, door = -1;
  bool target = FALSE;

  extern struct spell_info[];
  extern struct attack_hit_type attack_hit_text[];

  if (IS_NPC(ch))
    prob = dice(10, 10);
  else
    prob = GET_SKILL(ch, subcmd);

  one_argument(argument, arg);
  /* Now that we have the arg let's see who/what the target is */
  if (!target && IS_SET(spell_info[subcmd].targets, TAR_CHAR_ROOM)) {
    if ((vict = get_char_room_vis(ch, arg)) != NULL)
      target = TRUE;
  }
  if (!target && IS_SET(spell_info[subcmd].targets, TAR_CHAR_WORLD)) {
    if ((vict = get_char_vis(ch, arg)) != NULL)
      target = TRUE;
  }
  if (!target && IS_SET(spell_info[subcmd].targets, TAR_OBJ_INV)) {
    if ((tobj = get_obj_in_list_vis(ch, arg, ch->carrying)) != NULL)
      target = TRUE;
  }
  if (!target && IS_SET(spell_info[subcmd].targets, TAR_OBJ_EQUIP)) {
    for (i = 0; !target && i < NUM_WEARS; i++)
      if (GET_EQ(ch, i) && !str_cmp(t, GET_EQ(ch, i)->name)) {
	tobj = GET_EQ(ch, i);
	target = TRUE;
      }
  }
  if (!target && IS_SET(spell_info[subcmd].targets, TAR_OBJ_ROOM)) {
    if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
      target = TRUE;
  }
  if (!target && IS_SET(spell_info[subcmd].targets, TAR_OBJ_WORLD)) {
    if ((tobj = get_obj_vis(ch, t)))
      target = TRUE;
  }
  if (!target && IS_SET(spell_info[subcmd].targets, TAR_FIGHT_VICT)) {
    if (FIGHTING(ch) != NULL) {
      vict = FIGHTING(ch);
      target = TRUE;
    }
  }
  if (!target && IS_SET(spell_info[subcmd].targets, TAR_DOOR)) {
    if ((door = find_door(ch, arg, argument, spells[subcmd])))
      target = TRUE;
  }
  if (!target) {
    sprintf(buf2, "Whom or what do you wish to %s?\r\n", spells[subcmd]);
    send_to_char(buf2, ch);
    return;
  }
  if (vict && spell_info[subcmd].violent == TRUE) {
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
      send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
      return;
    }
  }
  /* At this point we MUST have a target victim, obj, or door. */
  if (IS_SET(spell_info[subcmd].targets, TAR_MUST_WIELD)) {
    /*
     * This skill requires a weapon to work -- NOTE: min_pos contains
     *   the weapon type that needs to be used.
     */
    if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_HOLD)) {
      sprintf(buf2, "You must be wielding a weapon to %s %s!" spells[subcmd], arg);
      act(buf2, FALSE, ch, tobj, vict, TO_CHAR);
      return;
    }
    if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) != ITEM_WEAPON &&
	GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) != ITEM_WEAPON) {
      sprintf(buf2, "You must be wielding a weapon to %s %s!" spells[subcmd], arg);
      act(buf2, FALSE, ch, tobj, vict, TO_CHAR);
      return;
    }
    if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_WEAPON)
      weapon = GET_EQ(ch, WEAR_WIELD);
    else if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_WEAPON)
      weapon = GET_EQ(ch, WEAR_HOLD);
    else {
      sprintf(buf2, "You must be wielding a weapon to %s %s!" spells[subcmd], arg);
      act(buf2, FALSE, ch, tobj, vict, TO_CHAR);
      return;
    }
    if (GET_OBJ_VAL(weapon, 3) != (spell_info[subcmd].min_position) - TYPE_HIT) {
      sprintf(buf2, "You must be wielding a weapon that %s.\r\n",
	      attack_hit_text[spell_info[subcmd].min_position].plural);
      send_to_char(buf2, ch);
      return;
    }
  }
  /* If we needed a certain weapon, we've got one */
  if (vict != NULL) {
    /* Okay if we get this far we can check to see if we hit the person */
    if (prob > percent)
      hit(ch, vict, subcmd);
    else /* Do not damage */
      damage(ch, vict, 0, subcmd, -1);
  } else if (tobj != NULL) { /* NONE */
    if (prob > percent) {
    } else {
    }
  } else if (door != -1) {  /* Doorbash */
    if (prob > percent) {
      if (IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
	/* reset ALL bits on the door, because this sumbitch destroyed it! */
	EXIT(ch, door)->exit_info = 0;
	act("$n slams into the door, ripping it right off the hinges!",
	    TRUE, ch, 0, 0, TO_ROOM);
	act("You slam into the door, ripping it right off the hinges!",
	    TRUE, ch, 0, 0, TO_CHAR);
      }
    } else {
      act("$n attempts to breakdown the door, but it proves too strong for $m.",
	    TRUE, ch, 0, 0, TO_ROOM);
      act("You slam into the door, but it proves too strong for you.",
	    TRUE, ch, 0, 0, TO_CHAR);
    }
  } else
    send_to_char("What did you want to do?\r\n", ch);
}
#endif


ACMD(do_behead)
{

  struct obj_data *i, *temp, *next_obj, *new_obj;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Behead what?\r\n", ch);
    return;
  }

  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You must be wielding a slashing weapon to make it a success!\r\n", ch);
    return;
  }

  if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) != ITEM_WEAPON ||
      (GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_WEAPON &&
       GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_SLASH - TYPE_HIT)) {
    send_to_char("You must be wielding a slashing weapon to make it a success!\r\n", ch);
    return;
  }
  /* We've gotten this far so let's see if we can find a corpse */
  i = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents);

  if (i != NULL && GET_OBJ_TYPE(i) == ITEM_CONTAINER &&
      GET_OBJ_VAL(i, 3)) {
    act("$n brutally decapitates $o.", FALSE, ch, i, 0, TO_ROOM);
    act("You brutally decapitate $o.", FALSE, ch, i, 0, TO_CHAR);
    make_head(i, 0, ch);
    new_obj = make_headless_corpse(i, 0, ch);
    for (temp = i->contains; temp; temp = next_obj) {
      next_obj = temp->next_content;
      obj_from_obj(temp);
      obj_to_obj(temp, new_obj);
    }
    extract_obj(i);
  } else
    send_to_char("Behead what?!?!?\r\n", ch);
  return;
}

ACMD(do_kill)
{
  struct char_data *vict;

  if ((GET_LEVEL(ch) < LVL_GOD) || IS_NPC(ch)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(vict = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\r\n", ch);
    else if (ch == vict)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else {
      act("You remove $N's head from $S body!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
      act("$N pulls your head off like a toy!", FALSE, vict, 0, ch, TO_CHAR);
      act("$n grabs $N's head and removes it from $S body!", FALSE, ch, 0, vict, TO_NOTVICT);
      raw_kill(vict, ch);
    }
  }
}


ACMD(do_backstab)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Backstab who?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("How can you sneak up on yourself?\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_HOLD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if ( (GET_EQ(ch, WEAR_WIELD) && ((GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_WEAPON) &&
	GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT)) || \
       (GET_EQ(ch, WEAR_HOLD) && ((GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_WEAPON) &&
	GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 3) != TYPE_PIERCE - TYPE_HIT)) ) {
    send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You can't backstab a fighting person -- they're too alert!\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
  }
  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  percent = number(1, 101);     /* 101% is a complete failure */
  prob    = (IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_BACKSTAB), 100) : GET_SKILL(ch, SKILL_BACKSTAB));

  if (AWAKE(vict) && (percent > prob)) {
    damage(ch, vict, 0, SKILL_BACKSTAB, -1);
    if (number(0, 10) > 8)
      learn_from_mistake(ch, SKILL_BACKSTAB);
  } else
    hit(ch, vict, SKILL_BACKSTAB);
}


ACMD(do_stab)
{
  struct char_data *vict;
  int    percent, prob;

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf))) {
    if (FIGHTING(ch))
      vict = FIGHTING(ch);
    else {
      send_to_char("Stab who?\r\n", ch);
      return;
    }
  }
  if (!vict || vict == ch)
    return;

  if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_HOLD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if ( (GET_EQ(ch, WEAR_WIELD) && ((GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_WEAPON) &&
	GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT)) || \
       (GET_EQ(ch, WEAR_HOLD) && ((GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_WEAPON) &&
	GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 3) != TYPE_PIERCE - TYPE_HIT)) ) {
    send_to_char("Only piercing weapons can be used for stabbing.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
  }
  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }
  percent = number(1, 101);     /* 101% is a complete failure */
  prob    = (IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_STAB), 100) : GET_SKILL(ch, SKILL_STAB));

  if (AWAKE(vict) && (percent > prob)) {
    damage(ch, vict, 0, SKILL_STAB, -1);
    if (number(0, 10) > 8)
      learn_from_mistake(ch, SKILL_STAB);
  } else
    hit(ch, vict, SKILL_STAB);
  WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
}



ACMD(do_cudgel)
{
  struct char_data *vict;
  struct obj_data *weapon = NULL;
  int    percent, prob;

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf))) {
    if (FIGHTING(ch))
      vict = FIGHTING(ch);
    else {
      send_to_char("Cudgel who?\r\n", ch);
      return;
    }
  }
  if (!vict || vict == ch)
    return;

  if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_HOLD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_EQ(ch, WEAR_WIELD) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_WEAPON)
    weapon = GET_EQ(ch, WEAR_WIELD);
  else
    weapon = GET_EQ(ch, WEAR_HOLD);

  if ((GET_OBJ_TYPE(weapon) != ITEM_WEAPON) ||
      (GET_OBJ_VAL(weapon, 3) != TYPE_BLUDGEON - TYPE_HIT &&
       GET_OBJ_VAL(weapon, 3) != TYPE_POUND - TYPE_HIT &&
       GET_OBJ_VAL(weapon, 3) != TYPE_CRUSH - TYPE_HIT &&
       GET_OBJ_VAL(weapon, 3) != TYPE_SMASH - TYPE_HIT)) {
    send_to_char("Only blunt weapons can be used to cudgel someone.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
  }
  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }
  percent = number(1, 101);     /* 101% is a complete failure */
  percent += GET_DEX(vict) + (GET_LEVEL(vict) - GET_LEVEL(ch));
  prob    = (IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_CUDGEL), 100) : GET_SKILL(ch, SKILL_CUDGEL));

  if (AWAKE(vict) && (percent > prob)) {
    damage(ch, vict, 0, SKILL_CUDGEL, -1);
    GET_POS(ch) = POS_SITTING;
    if (number(0, 10) > 8)
      learn_from_mistake(ch, SKILL_CUDGEL);
  } else
    hit(ch, vict, SKILL_CUDGEL);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_berserk)
{
  struct char_data *tch, *next_tch, *m;
  int    prcnt, prob;

  if (ch == NULL)
     return;

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if ((GET_MANA(ch) < 25)) {
     send_to_char("You attempt to go berserk, but you lack the incentive!\r\n",ch);
     return;
     WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
  } else if (GET_MOVE(ch) < 25) {
     send_to_char("You are too fatigued to go berserk.\r\n", ch);
     return;
  }
  prcnt = number(45, 101);
  prob  = IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_BERSERK), 100) : GET_SKILL(ch, SKILL_BERSERK);

  if (prob > prcnt) {
    send_to_char("ARGH!!!  YOU GO BERSERK!!\r\n", ch);
    act("$n starts beating everything in sight, better duck!!", FALSE, ch, 0, 0, TO_ROOM);
     if (GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch)) {
	GET_MOVE(ch) -= MIN(25, GET_MOVE(ch));
	GET_MANA(ch) -= MIN(20, GET_MANA(ch));
	GET_HIT(ch)  -= MIN(20, GET_HIT(ch));
     }

     if (ch->master != NULL)
	m = ch->master;
     else
	m = ch;

     for (tch = world[ch->in_room].people; tch; tch = next_tch) {
       next_tch = tch->next_in_room;
       if (IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM))
	 continue;
       if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
	 continue;
       if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM))
	 continue;
       if (!IS_NPC(tch) && IS_AFFECTED2(tch, AFF_GROUP))
	 continue;
       if (!IS_NPC(tch) && GET_LEVEL(tch) <= 5)
	 continue;
       if (tch == ch)
	 continue;
       GET_POS(ch) = POS_STANDING;
       damage(ch, tch, (GET_LEVEL(ch)*number(1,3)), TYPE_UNDEFINED, -1);
     }
  } else {
    send_to_char("You try to berserk, but fall on your face!\r\n", ch);
    GET_POS(ch) = POS_SITTING;
    act("$n tries to hit everything in sight--but trips.", FALSE, ch, 0, 0, TO_ROOM);
    if (number(0, 10) > 8)
      learn_from_mistake(ch, SKILL_BERSERK);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 8);
}


#if 0
ACMD(do_deathstroke)
{
  struct char_data *vict;
  byte percent, prob;

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Who's neck do you wish to break?\r\n", ch);
    return;
  }
  if (vict == ch) {
     send_to_char("You wrap your arms around your head and jerk it sharply to the right.\r\n"
		  "You neck feels much better now.\r\n", ch);
     return;
  }
  if (GET_EQ(ch, WEAR_WIELD) || GET_EQ(ch, WEAR_HOL)) {
    send_to_char("Your hands must be free to break someone's neck!\r\n", ch);
    return;
  }
  if (FIGHTING(ch)) {
    send_to_char("You can't break someone's neck while you're fighting!\r\n", ch);
    return;
  }
  if ((GET_LEVEL(ch) >= LVL_IMMORT) || (GET_LEVEL(vict) >= LVL_IMMORT)) {
     send_to_char("You are prevented from taking that course of action.\r\n", ch);
     return;
  }
  percent = number(50, 101);     /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_DEATHSTROKE);

  if (prob < 10) {
     act("You attempt to break $n's neck, but lose your grip!", FALSE, ch, 0, vict, TO_CHAR);
     act("$N attempts to break $n's neck, but loses $S grip!", FALSE, ch, 0, vict, TO_ROOM);
     act("$N attempts to break your neck, but loses $S grip!", FALSE, ch, 0, vict, TO_VICT);
     hit(ch, vict, TYPE_UNDEFINED);
  } else if (prob > percent) {
     act("You sneak up behind $n and break $s neck!", FALSE, ch, 0, vict, TO_CHAR);
     act("$N sneaks up behind $n and breaks $s neck!", FALSE, ch, 0, vict, TO_ROOM);
     send_to_char("Someone places their arms and hands around your head, the last thing you hear is a loud CRACK!\r\n", vict);
     GET_HIT(vict) = 1;
     damage(ch, vict, 500, TYPE_UNDEFINED, -1);
  } else
     hit(ch, vict, TYPE_UNDEFINED);

  WAIT_STATE(ch, 4 * PULSE_VIOLENCE);
}
#endif


ACMD(do_circle)
{
  struct char_data *vict;
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD), *wielded2 = GET_EQ(ch, WEAR_HOLD);
  byte   prob, percent;
  int    backstab_mult(int level);

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf))) {
    if (FIGHTING(ch))
      vict = FIGHTING(ch);
    else {
      send_to_char("Circle around and backstab who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("How can you circle around behind yourself?\r\n", ch);
    return;
  }
  if (GET_POS(ch) > POS_FIGHTING)
    do_backstab(ch, argument, cmd, subcmd);

  if (wielded == NULL && wielded2 == NULL) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (wielded && GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    if (wielded2 && GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 3) != TYPE_PIERCE - TYPE_HIT) {
      send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
      return;
    }
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {  /* Protect newbies */
    send_to_char("You cannot bring yourself to do it.\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
  }
  percent = number(10, 101);   /* 101% is a complete failure */
  prob    = (IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_CIRCLE), 100) : GET_SKILL(ch, SKILL_CIRCLE));

  if (!FIGHTING(vict)) {
    send_to_char("But you're victim is not fighting you!\r\n",ch);
    return;
  }
  if (FIGHTING(ch) && (prob > percent)) {
    act("You circle around behind $N and backstab $M!", FALSE, ch, 0, vict, TO_CHAR);
    act("$n circles around behind $N and backstabs $M.", FALSE, ch, 0, vict, TO_ROOM);
    act("$n has plunged a dagger into your back! How sneaky!", FALSE, ch, 0, vict, TO_VICT);
    damage(ch, FIGHTING(ch), (backstab_mult(GET_LEVEL(ch)) >> 1), SKILL_CIRCLE, -1);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  } else {
    send_to_char("Uh, oh, your target caught you trying to circle around.\r\n",ch);
    hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    if (number(0, 10) > 8)
      learn_from_mistake(ch, SKILL_CIRCLE);
  }
}


ACMD(do_garrotte)
{
  struct char_data *vict;
  struct obj_data *weapon = GET_EQ(ch, WEAR_WIELD);
  char   name[256], logbuf[256];
  int    exp = 0, prob = number(1, 101);

  one_argument(argument, name);

  if (!(vict = get_char_room_vis(ch, name))) {
    send_to_char("Whom do you wish to garrotte?\n\r", ch);
    return;
  }
  if ((GET_LEVEL(vict) <= 5) || ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {  /* Protect newbies */
    send_to_char("You cannot bring yourself to do it.\r\n", ch);
    return;
  }
  if (IS_IMMORT(vict) && !IS_IMMORT(ch)) {
    send_to_char("Fool! Never mess with the gods!\r\n",ch);
    raw_kill(ch, vict);
    sprintf(logbuf, "%s died trying to garrotte %s.\r\n",
	GET_NAME(ch), GET_NAME(vict));
    log(logbuf);
    return;
  }

  if (weapon == NULL || (weapon && GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_GARROTTE - TYPE_HIT))  {
    send_to_char("You need to be holding a garrotting weapon.\r\n",ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You can't garrotte a fighting person -- they're too alert!\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("How do you propose to garrote yourself? You can't get the leverage necessary!\n\r",ch);
    return;
  }

  act("$n sneaks up behind $N and wraps some wire around $S neck!",
	   FALSE, ch, 0, vict, TO_ROOM);
  act("Someone wraps a thin wire around your neck and begins choking you!",
	   FALSE, ch, 0, vict, TO_VICT);
  act("You sneak up behing $N and begin choking him with $p.",
	   FALSE, ch, weapon, vict, TO_CHAR);
  prob += (GET_LEVEL(vict) - GET_LEVEL(ch));

  if (GET_SKILL(ch, SKILL_GARROTTE) < prob) {
    act("$N struggles free from $n's wire!", FALSE, ch, 0, vict, TO_ROOM);
    act("You stuggle free from $n's wire!", FALSE, ch, 0, vict, TO_VICT);
    act("$N struggles free from your grasp!", FALSE, ch, 0, vict, TO_CHAR);
    hit(ch, vict, TYPE_UNDEFINED);
    return;
  } else {
    act("Your vision blacks out as $n cuts off all circulation to your brain!",
	    FALSE, ch, 0, vict, TO_VICT);
    if (GET_LEVEL(ch) >= LVL_IMMORT) {
      act("$n yanks tightly on the wire, snapping $N's head right off and onto the floor!",
	    FALSE, ch, 0, vict, TO_ROOM);
      act("You yank tightly on the wire and snap $N's head right off!!!",
	    FALSE, ch, 0, vict, TO_CHAR);
    } else
      act("$N's body falls lifelessly to the ground!", FALSE, ch, 0, vict, TO_NOTVICT);
    /* Die, vict, DIE! */
    raw_kill(vict, ch);
    /* Wait State the ch for awhile */
    WAIT_STATE(ch, PULSE_VIOLENCE * 5);
    exp = MIN((long)250000, (long)GET_EXP(vict) >> 2);
    send_to_char("You have become more experienced.\r\n", ch);
    gain_exp(ch, exp);
    gain_exp(vict, -(exp));
    GET_ALIGNMENT(ch) = MAX(-1000, (GET_ALIGNMENT(ch) - 300));
    /* log it if this wasn't a mob */
    if (!IS_NPC(vict)) {
      sprintf(logbuf, "%s assassinated by %s.\r\n", GET_NAME(vict),
	 GET_NAME(ch));
      log(logbuf);
    }
  }
  return;
}


ACMD(do_palm)
{
   struct char_data *vict = NULL;
   char name[256], logbuf[256];
   byte prob, percent, exp;

   one_argument(argument,name);

   if ((!*name) || (!(vict = get_char_room_vis(ch,name)))) {
     send_to_char("Palm who?", ch);
     return;
   }
   if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
     send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
     return;
   }
   if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
   }
   if ((IS_IMMORT(vict) && !IS_NPC(vict)) && IS_IMMORT(ch)) {
     send_to_char("It's not nice to try to kill other Gods!\n\r",ch);
     return;
   }
   if ((IS_IMMORT(vict) && !IS_NPC(vict)) && !IS_IMMORT(ch)) {
     send_to_char("You shouldn't have fucked with the Gods!\n\r",ch);
     raw_kill(ch, vict);
     if (!IS_NPC(vict)) {
       sprintf(logbuf, "%s killed by %s's palm.\r\n", GET_NAME(vict),
		GET_NAME(ch));
       log(logbuf);
     }
     return;
   }
   if (GET_MANA(ch) < 100) {
     send_to_char("You are unable to concentrate.\n\r",ch);
     return;
   }
   percent = number(10, 101);   /* 101% is a complete failure */
   prob    = IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_QUIVERING_PALM), 100) : GET_SKILL(ch, SKILL_QUIVERING_PALM);

   exp = MIN((long)250000, (long)((GET_MAX_LEVEL(vict) <= 9) ? 0 : (GET_EXP(vict)/GET_MAX_LEVEL(vict))));

   if (((prob > percent) || IS_IMMORT(ch)) &&
       (GET_MAX_LEVEL(ch)-4) > GET_MAX_LEVEL(vict)) {
     send_to_char("Your palm quivers as you shove it through your target's rib cage!\n\r",ch);
     send_to_char("You stare as a hand is forced into your chest.  You Die.\n\r",vict);
     act("$n forces their hand through $N's chest, killing $M instantly.\n\r",FALSE, ch, 0, vict,TO_ROOM);
     raw_kill(vict, ch);
     if (!IS_NPC(vict)) {
       sprintf(logbuf, "%s dies as %s shoves %s hand through %s.\r\n",
		    GET_NAME(vict), GET_NAME(ch), HSHR(ch), HMHR(vict));
       log(logbuf);
     }
     WAIT_STATE(ch, PULSE_VIOLENCE * 4);
     if (GET_MAX_LEVEL(ch) >= LVL_IMMORT)
       return;
     else
       send_to_char("You have become more experienced.\r\n", ch);
     GET_MANA(ch) -= MIN(750, GET_MANA(ch));
   } else {
     send_to_char("You slam your hand against your opponent's chest.\r\n",ch);
     act("$n slams $s hand against $N's chest.\r\n",FALSE, ch, 0, vict, TO_ROOM);
     act("$n slams $n hand into your chest. $e has a puzzled look on $s face.", FALSE, ch, 0, vict, TO_VICT);
     hit(ch, vict, TYPE_UNDEFINED);
     WAIT_STATE(ch, PULSE_VIOLENCE * 2);
   }
}


ACMD(do_pummel)
{
  struct char_data *vict;
  byte percent, prob;

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Whom do you wish to pummel?\r\n", ch);
    return;
  } else if (vict == ch) {
    send_to_char("How do you propose to pummel yourself?\r\n", ch);
    return;
  } else if (GET_EQ(ch, WEAR_WIELD) || GET_EQ(ch, WEAR_HOLD)) {
    send_to_char("Your hands must be free to pummel someone properly!\r\n", ch);
    return;
  } else if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  } else if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
  } else if (IS_IMMORT(ch) || IS_IMMORT(vict)) {
    send_to_char("What the hell would you need to pummel?\r\n", ch);
    return;
  }
  percent = number(10, 101);     /* 101% is a complete failure */
  prob    = IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_PUMMEL), 100) : GET_SKILL(ch, SKILL_PUMMEL);

  if (prob < 10) {
    act("You attempt to pummel $N, but hit your nose instead!", FALSE, ch, 0, vict, TO_CHAR);
    act("$n attempts to pummel $N, but punches $mself in the nose instead!", FALSE, ch, 0, vict, TO_ROOM);
    act("$n attempts to pummel you into submission, but punches $mself in the nose instead!", FALSE, ch, 0, vict, TO_VICT);
    hit(ch, vict, TYPE_UNDEFINED);
  } else if (prob > percent) {
    act("You pummel $N into submission.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n pummels $N into submission.", FALSE, ch, 0, vict, TO_ROOM);
    act("$n pummels you into submission!\r\n", FALSE, ch, 0, vict, TO_VICT);
    GET_POS(vict) = POS_STUNNED;
    damage(ch, vict, (GET_LEVEL(ch)*2), TYPE_UNDEFINED, -1);
  } else
    hit(ch, vict, TYPE_UNDEFINED);

  WAIT_STATE(ch, 4 * PULSE_VIOLENCE);
}


ACMD(do_bite)
{
   struct char_data *vict = NULL;
   char name[256];
   byte prob, percent;

   one_argument(argument, name);

   if (GET_RACE(ch) != RACE_VAMPIRE) {
     act("You open your mouth wide and try to bite $N's neck.", FALSE, ch, 0, vict, TO_CHAR);
     act("$n open $s mouth wide and tries to bite you on your neck, how kinky.", FALSE, ch, 0, vict, TO_VICT);
     act("$n open $s mouth wide and tries to bite $N's neck, how kinky.", FALSE, ch, 0, vict, TO_ROOM);
     return;
   } else if (!(vict = get_char_room_vis(ch, name))) {
     send_to_char("Bite who?\n\r", ch);
     return;
   } else if (FIGHTING(vict)) {
     send_to_char("You can't bite a fighting person -- your teeth will get broken!\r\n", ch);
     return;
   } else if (vict == ch) {
     send_to_char("Your own blood will not suffice.\n\r",ch);
     return;
   } else if (IS_IMMORT(vict)) {
     send_to_char("Your teeth crumble to dust as you try to bite your victim!\r\n",ch);
     GET_RACE(ch)    = RACE_HUMAN;
     GET_MAX_HIT(ch) = 1;
     return;
   }
   prob = (GET_MAX_LEVEL(ch) * number(1, 5));
   percent = number(1, 100);

   if (((prob >= percent) && (GET_RACE(ch) == RACE_VAMPIRE)) || IS_IMMORT(ch)) {
     send_to_char("You feel a sharp pain in your neck, you have been bitten!\r\n", vict);
     if (IS_IMMORT(ch)) {
       send_to_char("Sweet blood flows over your lips as you sink your teeth into your victim's neck!\n\r", ch);
       act("$n pulls $N's head back and bites deeply in $S neck!", FALSE, ch, 0, vict, TO_NOTVICT);
       raw_kill(vict, ch);
     } else {
       send_to_char("Warm blood flows over your lips as you sink your teeth into your victim's neck.\n\r", ch);
       act("$n sinks $s teeth into $N's neck!", FALSE, ch, 0, vict,TO_NOTVICT);
       send_to_char("Your thirst for blood has been satisfied.\r\n",ch);
       GET_MAX_HIT(vict) -= number(2, 18);
       GET_HIT(ch) += number(2, 20);
       if(!IS_IMMORT(ch)){
       GET_COND(ch, THIRST) = 23;
       GET_COND(ch, FULL)   = 23;
       } else
       GET_COND(ch, THIRST) = GET_COND(ch, FULL) = -1;
     }
     if (number(1, 100) > 90) {
       send_to_char("A strange feeling comes over you, suddenly have a loathing for sunlight and garlic.\r\n",vict);
       send_to_char("You have increased the members of your ancient race.\r\n",ch);
       GET_RACE(vict) = RACE_VAMPIRE;
       act("$N falls to the floor and screams in agony.  $e has been inducted into the ranks of the undead!", TRUE, ch, 0, vict, TO_NOTVICT);
     }
     return;
   }
}


ACMD(do_claw)
{
   struct char_data *vict;
   char name[256];
   byte prob, percent;

   one_argument(argument, name);

   if (!(vict = get_char_room_vis(ch, name))) {
     send_to_char("Maul who?\n\r", ch);
     return;
   } else if (GET_RACE(ch) != RACE_WEREWOLF) {
     act("You attempt to maul $N, but rip out your finger nails instead!", FALSE, ch, 0, vict, TO_CHAR);
     GET_HIT(ch) -= 1;
     act("$n attempts to maul $N, but rips out $s finger nails instead!", FALSE, ch, 0, vict, TO_ROOM);
     act("$n attempts to maul you, but $e rips out $s finger nails instead!", FALSE, ch, 0, vict, TO_VICT);
     hit(ch, vict, TYPE_UNDEFINED);
   } else if (vict == ch) {
     send_to_char("You cannot seem to get the leverage necessary to maul yourself.\n\r",ch);
     return;
   } else if (FIGHTING(vict) && GET_RACE(ch) == RACE_WEREWOLF) {
     send_to_char("You go into a mad rage and maul your victim!\r\n", ch);
     act("$n goes into a wild rage and mauls $N to pieces!", FALSE, ch, 0, vict, TO_ROOM);
     act("$n goes into a wild rage, tearing the flesh from your bones!", FALSE, ch, 0, vict, TO_VICT);
     GET_HIT(vict) = (GET_HIT(vict)/8);
     hit(ch, vict, TYPE_UNDEFINED);
   } else if (IS_IMMORT(vict)) {
     act("You attempt to maul $N but $E is too powerful for you!",TRUE, ch, 0, vict, TO_CHAR);
     act("$n has tried to maul you! Slay $m!", TRUE, ch, 0, vict, TO_VICT);
     return;
   }
   prob = (GET_MAX_LEVEL(ch) * number(1, 5));
   percent = number(10, 101);

   if (((prob > percent) && (GET_RACE(ch) == RACE_WEREWOLF)) || IS_IMMORT(ch)) {
     act("$n lunges at you and begins tearing at your flesh!", FALSE, ch, 0, vict, TO_VICT);
     if (IS_IMMORT(ch)) {
       act("You lunge at $N and begin tearing $m apart!\n\r", TRUE, ch, 0, vict, TO_CHAR);
       act("$n lunges at $N and begins to rip $m limb from limb!", TRUE, ch, 0, vict, TO_NOTVICT);
       raw_kill(vict, ch);
     } else {
       act("Warm blood flows down your arms as you rend $N limb from limb!", FALSE, ch, 0, vict, TO_CHAR);
       act("$n lunges at $N and begins to remove various appendages.", TRUE, ch, 0, vict,TO_NOTVICT);
       send_to_char("Your thirst for blood has been satisfied.\r\n",ch);
       GET_HIT(vict) = (GET_HIT(vict)/number(8, 18));
       if(!IS_IMMORT(ch)) {
       GET_COND(ch, THIRST) = MIN(24, GET_COND(ch, THIRST)+20);
       GET_COND(ch, FULL)   = MIN(24, GET_COND(ch, FULL)+20);
       }
       hit(ch, vict, TYPE_UNDEFINED);
     }
     if (number(1, 100) > 90) {
       send_to_char("A strange feeling comes over you, suddenly have a loathing for silver and the moon.\r\n",vict);
       send_to_char("You have increased the members of your ancient race.\r\n",ch);
       GET_RACE(vict) = RACE_WEREWOLF;
       act("$N falls to the floor and screams in agony.  $e has been made a werewolf!", TRUE, ch, 0, vict, TO_NOTVICT);
     }
     return;
   }
}


ACMD(do_gore)
{
   struct char_data *vict;
   char name[256], logbuf[256];
   byte prob, percent;

   one_argument(argument, name);

   if (!(vict = get_char_room_vis(ch, name))) {
     send_to_char("Gore who?\n\r", ch);
     return;
   } else if ((GET_RACE(ch) != RACE_KINTHALAS) || (GET_RACE(ch) != RACE_BYTERIAN)) {
     act("You attempt to gore $N, but bang your head against $M instead!", FALSE, ch, 0, vict, TO_CHAR);
     GET_HIT(ch) -= 5;
     act("$n attempts to gore $N, but bangs $s head against $M instead!", FALSE, ch, 0, vict, TO_ROOM);
     act("$n attempts to gore you, but $e bangs $s head against your chest instead.", FALSE, ch, 0, vict, TO_VICT);
     hit(ch, vict, TYPE_UNDEFINED);
   } else if (vict == ch) {
     send_to_char("You cannot seem to get your horns close enough to gore yourself.\n\r",ch);
     return;
   } else if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
     send_to_char("A wall of air prevents you!\r\n", ch);
     return;
   } else if (FIGHTING(vict) && ((GET_RACE(ch) == RACE_KINTHALAS) || \
	      (GET_RACE(ch) == RACE_BYTERIAN))) {
     send_to_char("You go into a wild rage and gore your victim!\r\n", ch);
     act("$n goes into a wild rage and gores $N!", FALSE, ch, 0, vict, TO_ROOM);
     act("$n goes into a wild rage, goring you in the chest!", FALSE, ch, 0, vict, TO_VICT);
     GET_HIT(vict) -= dice(2, GET_MAX_LEVEL(ch));
     hit(ch, vict, TYPE_UNDEFINED);
     WAIT_STATE(ch, PULSE_VIOLENCE * 6);
   } else if (IS_IMMORT(vict)) {
     act("You attempt to gore $N but $E is too powerful for you!",TRUE, ch, 0, vict, TO_CHAR);
     act("$n has tried to maul you! Slay $m!", TRUE, ch, 0, vict, TO_VICT);
     return;
   }
   prob = (GET_LEVEL(ch) * number(1, 5));
   percent = number(10, 101);

   if (((prob > percent) && ((GET_RACE(ch) == RACE_KINTHALAS) || (GET_RACE(ch) == RACE_BYTERIAN))) || \
	IS_IMMORT(ch)) {
     act("$n lunges at you and begins goring you!", FALSE, ch, 0, vict, TO_VICT);

     if (IS_IMMORT(ch)) {
       act("You lunge at $N and begin tearing $M apart!\n\r", TRUE, ch, 0, vict, TO_CHAR);
       act("$n lunges at $N and begins to gore $M!", TRUE, ch, 0, vict, TO_NOTVICT);
       raw_kill(vict, ch);
     } else {
       act("Warm blood flows down into your eyes as you rend $N limb from limb!", FALSE, ch, 0, vict, TO_CHAR);
       act("$n lunges at $N and begins gore $M deeply.", TRUE, ch, 0, vict,TO_NOTVICT);
       damage(ch, vict, (GET_LEVEL(ch) * number(1,3)), TYPE_UNDEFINED, -1);
       GET_POS(vict) = POS_SITTING;
       WAIT_STATE(ch, PULSE_VIOLENCE * 4);
     }
     if (!IS_NPC(vict)) {
       sprintf(logbuf, "%s gored by %s, OUCH!\r\n", GET_NAME(vict),
	       GET_NAME(ch));
       log(logbuf);
     }
     return;
   }
}


ACMD(do_order)
{
  char name[100], message[256];
  char buf[256];
  bool found = FALSE;
  int org_room;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);

  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
      return;
    }
    if (vict) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

      if ((vict->master != ch) || !IS_AFFECTED(vict, AFF_CHARM))
	act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
	send_to_char(OK, ch);
	command_interpreter(vict, message);
      }
    } else {                    /* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, vict, TO_ROOM);

      org_room = ch->in_room;

      for (k = ch->followers; k; k = k->next) {
	if (org_room == k->follower->in_room)
	  if (IS_AFFECTED(k->follower, AFF_CHARM)) {
	    found = TRUE;
	    command_interpreter(k->follower, message);
	  }
      }
      if (found)
	send_to_char(OK, ch);
      else
	send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
    }
  }
}



ACMD(do_flee)
{
  int i, attempt, loss;
  struct char_data *oppo;

  if (ROOM_FLAGGED(ch->in_room, ROOM_BLADEBARRIER)) {
    send_to_char("You attempt to flee, but the wall of blades prevents you!\r\n", ch);
    damage(ch, ch, dice(5, 5), TYPE_UNDEFINED, -1);
    return;
  }
  for (i = 0; i < MAX_DIR; i++) {
    attempt = number(0, MAX_DIR - 1);       /* Select a random direction */
    if (CAN_GO(ch, attempt) &&
	!IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH)) {
      act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
      if (FIGHTING(ch)) {
        oppo = FIGHTING(ch);
	if (GET_DEX(ch) >= MIN(22, (GET_LEVEL(oppo) - GET_LEVEL(ch)))) {
	  if (do_simple_move(ch, attempt, TRUE)) {
	    send_to_char("You flee head over heels.\r\n", ch);
	    if (MOUNTED(ch)) {
	      char_from_room(MOUNTED(ch));
	      char_to_room(MOUNTED(ch), ch->in_room);
	    }
#if 0
            if (IS_MOB(oppo) && MOB_FLAGGED(oppo, MOB_MEMORY) && 
                !MOB_FLAGGED(oppo, MOB_WIMPY) && !MOB_FLAGGED(oppo, MOB_SENTINEL)) {
	      HUNTING(oppo)  = ch;
              hunt_victim(oppo);
            } 
#endif     
            if (oppo) {
	      loss = GET_MAX_HIT(ch) - GET_HIT(ch);
	      loss *= MAX(1, GET_LEVEL(oppo) - GET_LEVEL(ch));
	      gain_exp(ch, -loss);
	      stop_fighting(oppo);
	      stop_fighting(ch);
	    }
	  } else {
	    act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
	  }
	}
	GET_MOVE(ch) -= (GET_MOVE(ch) >> 4);
      } else if (do_simple_move(ch, attempt, TRUE))
	send_to_char("You flee head over heels.\r\n", ch);
      return;
    }
  }
  send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}


ACMD(do_bash)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (!IS_NPC(ch) && GET_SKILL(ch, SKILL_BASH) <= 0) {
    send_to_char("You'd better leave all the martial arts to fighters.\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Bash who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_HOLD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  percent = number(1, 101);     /* 101% is a complete failure */
  prob    = IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_BASH), 100) : GET_SKILL(ch, SKILL_BASH);

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_BASH, -1);
    GET_POS(ch) = POS_SITTING;
    if (number(0, 10) > 8)
      learn_from_mistake(ch, SKILL_BASH);
  } else {
    damage(ch, vict, dice(2, GET_STR(ch)>>1), SKILL_BASH, -1);
    GET_POS(vict) = POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 4);
}


ACMD(do_rescue)
{
  struct char_data *vict, *tmp_ch;
  int percent, prob;

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Whom do you want to rescue?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("What about fleeing instead?\r\n", ch);
    return;
  }
  if (FIGHTING(ch) == vict) {
    send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
    return;
  }
  for (tmp_ch = world[ch->in_room].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room)
     ;

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if ((GET_SKILL(ch, SKILL_RESCUE) <= 0) && !IS_NPC(ch))
    send_to_char("But only true warriors can do this!", ch);
  else {
    percent = number(1, 101);   /* 101% is a complete failure */
    prob    = (IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_RESCUE), 100) : GET_SKILL(ch, SKILL_RESCUE));

    if (percent > prob) {
      send_to_char("You fail the rescue!\r\n", ch);
      if (number(0, 10) > 8)
	learn_from_mistake(ch, SKILL_RESCUE);
      return;
    }
    send_to_char("Banzai!  To the rescue...\r\n", ch);
    act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
    act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);

    if (FIGHTING(vict) == tmp_ch)
      stop_fighting(vict);
    if (FIGHTING(tmp_ch))
      stop_fighting(tmp_ch);
    if (FIGHTING(ch))
      stop_fighting(ch);

    set_fighting(ch, tmp_ch);
    set_fighting(tmp_ch, ch);

    WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
  }
}


ACMD(do_kick)
{
  struct char_data *vict;
  int percent = number(1, 101);

  if ((GET_SKILL(ch, SKILL_KICK) <= 0) && !IS_NPC(ch)) {
    send_to_char("You'd better leave all the martial arts to fighters.\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kick who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
  }
  if (percent > GET_SKILL(ch, SKILL_KICK)) {
    damage(ch, vict, 0, SKILL_KICK, -1);
    if (number(0, 10) > 8)
      learn_from_mistake(ch, SKILL_KICK);
  } else
    damage(ch, vict, GET_LEVEL(ch) >> 1, SKILL_KICK, -1);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_turn)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (!IS_NPC(ch) && GET_SKILL(ch, SKILL_TURN_UNDEAD) <= 0) {
    send_to_char("You'd best let real priests turn undead.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Turn which undead?\r\n", ch);
      return;
    }
  }

  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }

  percent = number(1, 101);     /* 101% is a complete failure */
  prob = IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_TURN_UNDEAD), 99) : GET_SKILL(ch, SKILL_TURN_UNDEAD);

  if (GET_RACE(vict) != RACE_UNDEAD)
    percent = 101;

  act("$n points $s finger at $N and says, 'Back through the gates of hell, demonspawn!'",
	TRUE, ch, 0,  vict, TO_NOTVICT);
  act("$n points $s finger at you and says, 'Back through the gates of hell, demonspawn!'",
	TRUE, ch, 0,  vict, TO_VICT);
  act("You point your finger at $N and say, 'Back through the gates of hell, demonspawn!'",
	TRUE, ch, 0,  vict, TO_CHAR);

  if (percent > prob)
    damage(ch, vict, 0, TYPE_UNDEFINED, -1);
  else {
    raw_kill(vict, ch);
    if (!IS_NPC(vict)) {
      sprintf(buf, "%s dies as %s sends %s back to hell.\r\n",
		    GET_NAME(vict), GET_NAME(ch), HMHR(vict));
      log(buf);
    }
    if (GET_LEVEL(ch) >= LVL_IMMORT)
      return;
    else {
      send_to_char("You have become more experienced.\r\n", ch);
      GET_EXP(ch) += GET_EXP(vict) >> 2;
    }
    GET_MANA(ch) = MAX(0, GET_MANA(ch)-GET_LEVEL(vict));
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


void lodge_obj_in_char(struct char_data *vict, struct obj_data *missile, int loc)
{
   if (GET_ITEMS_STUCK(vict, loc))
     extract_obj(GET_ITEMS_STUCK(vict, loc));

   GET_ITEMS_STUCK(vict, loc) = missile;
}

void unlodge_obj_in_char(struct char_data *vict, struct obj_data *missile, int loc)
{
   obj_to_char(GET_ITEMS_STUCK(vict, loc), vict);
   GET_ITEMS_STUCK(vict, loc) = NULL;
   GET_WOUNDS(vict)++;
}

/*
 * Fire Weapons:
 *   v[0] = Ammo Type
 *   v[1] = Current # Ammo
 *   v[2] = max allow ammo
 *   v[3] = missile vnum (if loaded)
 * Missile/Ammo:
 *   v[0] = Ammo Type
 *   v[1] = Ammo Amount
 *   v[2] = Num Dam Dice
 *   v[3] = Size Dam Dice
 */
ACMD(do_shoot)
{
  struct char_data *vict;
  struct obj_data  *missile;
  int    percent, prob, loc, dam = 0, exp = 0;

  extern char *locations[];

  one_argument(argument, arg);

  if (!IS_NPC(ch) && GET_SKILL(ch, SKILL_RANGE) <= 0) {
    send_to_char("You'd better not do that, could shoot yourself in the foot!\r\n", ch);
    return;
  }
  /* In this function we should pass GET_OBJ_VAL(obj, 0) -- the range */
  if (!(vict = get_char_scan_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Shoot who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) ||
      ROOM_FLAGGED(vict->in_room, ROOM_PEACEFUL)) {
    send_to_char("The room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a ranged weapon to shoot something.\r\n", ch);
    return;
  }
  if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) != ITEM_FIREWEAPON) {
    send_to_char("You need to wield a ranged weapon to shoot something.\r\n", ch);
    return;
  }
  if ((missile = read_object(GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3), VNUMBER)) == NULL) {
    send_to_char("You need to load your ranged weapon to shoot something.\r\n", ch);
    return;
  }
  if (!IS_HUMANOID(vict))
    loc = 11;
  else
    loc = number(0, 10);

  percent = number(1, 101);     /* 101% is a complete failure */
  prob    = (IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_RANGE), 100) : GET_SKILL(ch, SKILL_RANGE)) + (GET_DEX(ch) >> 1);
  /* We're making the shot no matter what happens, so let's remove the ammo */
  GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 1) -= 1;
  /* reset the vnum of the loaded missile to -1 i.e. NONE */
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 1) <= 0)
    GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) = -1;
  /* now take the shot */
  if (percent > prob && !IS_IMMORT(ch)) {
    act("$n shoots $p at $N.", TRUE, ch, missile, vict, TO_ROOM);
    act("You shoot $p at $N.", TRUE, ch, missile, vict, TO_CHAR);
    act("$p misses it's mark.", FALSE, ch, missile, NULL, TO_CHAR);
    obj_to_room(missile, vict->in_room);
    act("$p flies into the room and lands harmlessly on the ground.",
	TRUE, NULL, missile, NULL, TO_ROOM);
    if (number(0, 10) == 10)
      learn_from_mistake(ch, SKILL_RANGE);
  } else {
    act("$n shoots $p at $N.", TRUE, ch, missile, vict, TO_ROOM);
    act("You shoot $p at $N.", TRUE, ch, missile, vict, TO_CHAR);
    sprintf(buf, "$p shot from your bow lodges into $N's %s!", locations[loc]);
    act(buf, FALSE, ch, missile, vict, TO_CHAR);
    GET_HIT(vict) -= dice(GET_OBJ_VAL(missile, 2), GET_OBJ_VAL(missile, 3));
    sprintf(buf, "$p flies through the air lodging into $N's %s.", locations[loc]);
    act(buf, TRUE, NULL, missile, vict, TO_NOTVICT);
    sprintf(buf, "$p flies through the air lodging into your %s.", locations[loc]);
    act(buf, TRUE, vict, missile, NULL, TO_CHAR);
    lodge_obj_in_char(vict, missile, loc);
    WAIT_STATE(vict, PULSE_VIOLENCE);
    update_pos(vict);
    switch (GET_POS(vict)) {
      case POS_MORTALLYW:
      act("$n is mortally wounded, and will die soon, if not aided.", TRUE, vict, 0, 0, TO_ROOM);
      send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", vict);
      break;
      case POS_INCAP:
      act("$n is incapacitated and will slowly die, if not aided.", TRUE, vict, 0, 0, TO_ROOM);
      send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", vict);
      break;
      case POS_STUNNED:
      act("$n is stunned, but will probably regain consciousness again.", TRUE, vict, 0, 0, TO_ROOM);
      send_to_char("You're stunned, but will probably regain consciousness again.\r\n", vict);
      break;
      case POS_DEAD:
        act("$n is dead!  R.I.P.", FALSE, vict, 0, 0, TO_ROOM);
        send_to_char("You are dead!  Sorry...\r\n", vict);
        break;
      default:                      /* >= POSITION SLEEPING */
        if (dam > (GET_MAX_HIT(vict) >> 2))
          act("That really did HURT!", FALSE, vict, 0, 0, TO_CHAR);

        if (GET_HIT(vict) < (GET_MAX_HIT(vict) >> 2) || GET_WOUNDS(vict) > 5) {
          sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
  	          CCRED(vict, C_SPR), CCNRM(vict, C_SPR));
          send_to_char(buf2, vict);
          if (MOB_FLAGGED(vict, MOB_WIMPY) && (ch != vict))
	    do_flee(vict, "", 0, 0);
        }
        if (!IS_NPC(vict) && GET_WIMP_LEV(vict) && (vict != ch) &&
  	    GET_HIT(vict) < GET_WIMP_LEV(vict)) {
          send_to_char("You wimp out, and attempt to flee!\r\n", vict);
          do_flee(vict, "", 0, 0);
        }
        break;
    }   
    if (GET_POS(vict) == POS_DEAD) {
      if (IS_NPC(vict) || vict->desc)
        if (IS_AFFECTED2(ch, AFF_GROUP))
	  group_gain(ch, vict);
        else {
	  exp = MIN((long)max_exp_gain, (long)GET_EXP(vict));
	  /* Calculate level-difference bonus */
 	  exp += MAX(0L, (long)(exp * MIN(8, (GET_LEVEL(vict) - GET_LEVEL(ch)))) >> 3);
	  exp =  MAX((long)exp, 1L);
	  if (exp > 1)
	    send_to_char("You receive some experience points.\r\n", ch);
	  else
	    send_to_char("You receive one lousy experience point.\r\n", ch);
	  gain_exp(ch, exp);
	  change_alignment(ch, vict);
       }
       if (!IS_NPC(vict)) {
         sprintf(buf2, "%s killed by %s's volley of arrows at %s", GET_NAME(vict), GET_NAME(ch),
	      world[vict->in_room].name);
         mudlog(buf2, BRF, LVL_IMMORT, TRUE);
         if (MOB_FLAGGED(ch, MOB_MEMORY))
  	   forget(ch, vict);
       }
       die(vict, ch);
    }    
  }
  if (IS_NPC(vict)) {
    if (vict->in_room != ch->in_room) {
      HUNTING(vict) = ch;
      hunt_victim(vict);
    } else
      hit(vict, ch, TYPE_UNDEFINED);
  }
  GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) = -1;
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

/* throw obj vict */
ACMD(do_throw)
{
  int  i, prob, percent, loc, equipped = 0, exp = 0, dam = 0;
  long vnum;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct char_data *vict;
  struct obj_data *missile;
  extern char *locations[];
  extern struct index_data *obj_index;

  two_arguments(argument, arg1, arg2);

  if (!*arg1) {
    send_to_char("What do you want to throw?\r\n", ch);
    return;
  }
  if (!(missile = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
    for (i = 0; !missile && i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i) && (isname(arg1, GET_EQ(ch, i)->name) ||
	  is_abbrev(arg1, GET_EQ(ch, i)->name))) {
	missile = GET_EQ(ch, i);
        equipped = 1;
      }
    }
  }
  if (!missile) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", XANA(arg1), arg1);
    send_to_char(buf, ch);
    return;
  }
  if (!*arg2) {
    act("Who do you want to throw $p at?", FALSE, ch, missile, 0, TO_CHAR);
    return;
  }
  if (!(vict = get_char_scan_vis(ch, arg2))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      act("Who do you want to throw $p at?", FALSE, ch, missile, 0, TO_CHAR);
      return;
    }
  }
  if (vict == ch) {
    act("You bonk yourself on the head with $p.", FALSE, ch, missile, 0, TO_CHAR);
    act("$n bonks $mself on the head with $p.", TRUE, ch, missile, 0, TO_ROOM);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) ||
      ROOM_FLAGGED(vict->in_room, ROOM_PEACEFUL)) {
    send_to_char("The room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict) && (GET_LEVEL(ch) <= 5 || GET_LEVEL(vict) <= 5)) {
    send_to_char("A wall of air prevents you!\r\n", ch);
    return;
  }
  if (equipped && missile != GET_EQ(ch, WEAR_WIELD)) {
    act("You need to WIELD $p to throw it at $N!", FALSE, ch, missile, vict, TO_CHAR);
    return;
  }
  if (!IS_OBJ_STAT(missile, ITEM_THROWABLE)) {
    act("You can't throw $p!", FALSE, ch, missile, 0, TO_CHAR);
    return;
  }
  if (!IS_HUMANOID(vict))
    loc = 11;
  else
    loc = number(0, 10);

  percent = number(1, 101);     /* 101% is a complete failure */
  prob    = (IS_NPC(ch) ? number(GET_SKILL(ch, SKILL_THROWING), 100) : GET_SKILL(ch, SKILL_THROWING)) + (GET_DEX(ch) >> 1);

  vnum = GET_OBJ_VNUM(missile);
  extract_obj(missile);
  missile = read_object(vnum, VNUMBER);

  if (equipped) {
    act("$n throws $p at $N.", TRUE, ch, missile, vict, TO_ROOM);
    act("You throw $p at $N.", TRUE, ch, missile, vict, TO_CHAR);
    WAIT_STATE(vict, PULSE_VIOLENCE);
  } else {
    act("$n pulls out $p and throws it at $N.", TRUE, ch, missile, vict, TO_ROOM);
    act("You grasp $p and throw it at $N.", TRUE, ch, missile, vict, TO_CHAR);
    WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
  }

  if (percent > prob && !IS_IMMORT(ch)) {
    act("$p flies past $N and lands harmlessly on the ground.",
	TRUE, NULL, missile, vict, TO_NOTVICT);
    act("$p flies past you and lands harmlessly on the ground.",
	TRUE, vict, missile, NULL, TO_CHAR);
    obj_to_room(missile, vict->in_room);
    if (number(0, 10) == 10)
      learn_from_mistake(ch, SKILL_THROWING);
  } else {
    GET_HIT(vict) -= dice(GET_OBJ_VAL(missile, 1), GET_OBJ_VAL(missile, 2));
    sprintf(buf, "$p flies through the air, lodging into $N's %s.", locations[loc]);
    act(buf, TRUE, NULL, missile, vict, TO_NOTVICT);
    sprintf(buf, "$p flies through the air and lodges in your %s.", locations[loc]);
    act(buf, TRUE, vict, missile, NULL, TO_CHAR);
    act("$p looks like it made its mark!", TRUE, ch, missile, NULL, TO_CHAR);
    lodge_obj_in_char(vict, missile, loc);
    update_pos(vict);
    switch (GET_POS(vict)) {
      case POS_MORTALLYW:
      act("$n is mortally wounded, and will die soon, if not aided.", TRUE, vict, 0, 0, TO_ROOM);
      send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", vict);
      break;
      case POS_INCAP:
      act("$n is incapacitated and will slowly die, if not aided.", TRUE, vict, 0, 0, TO_ROOM);
      send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", vict);
      break;
      case POS_STUNNED:
      act("$n is stunned, but will probably regain consciousness again.", TRUE, vict, 0, 0, TO_ROOM);
      send_to_char("You're stunned, but will probably regain consciousness again.\r\n", vict);
      break;
      case POS_DEAD:
        act("$n is dead!  R.I.P.", FALSE, vict, 0, 0, TO_ROOM);
        send_to_char("You are dead!  Sorry...\r\n", vict);
        break;
      default:                      /* >= POSITION SLEEPING */
        if (dam > (GET_MAX_HIT(vict) >> 2))
          act("That really did HURT!", FALSE, vict, 0, 0, TO_CHAR);

        if (GET_HIT(vict) < (GET_MAX_HIT(vict) >> 2) || GET_WOUNDS(vict) > 5) {
          sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
  	          CCRED(vict, C_SPR), CCNRM(vict, C_SPR));
          send_to_char(buf2, vict);
          if (MOB_FLAGGED(vict, MOB_WIMPY) && (ch != vict))
	    do_flee(vict, "", 0, 0);
        }
        if (!IS_NPC(vict) && GET_WIMP_LEV(vict) && (vict != ch) &&
  	    GET_HIT(vict) < GET_WIMP_LEV(vict)) {
          send_to_char("You wimp out, and attempt to flee!\r\n", vict);
          do_flee(vict, "", 0, 0);
        }
        break;
    }   
    if (GET_POS(vict) == POS_DEAD) {
      if (IS_NPC(vict) || vict->desc)
        if (IS_AFFECTED2(ch, AFF_GROUP))
	  group_gain(ch, vict);
        else {
	  exp = MIN((long)max_exp_gain, (long)GET_EXP(vict));
	  /* Calculate level-difference bonus */
 	  exp += MAX(0L, (long)(exp * MIN(8, (GET_LEVEL(vict) - GET_LEVEL(ch)))) >> 3);
	  exp =  MAX((long)exp, 1L);
	  if (exp > 1)
	    send_to_char("You receive some experience points.\r\n", ch);
	  else
	    send_to_char("You receive one lousy experience point.\r\n", ch);
	  gain_exp(ch, exp);
	  change_alignment(ch, vict);
       }
       if (!IS_NPC(vict)) {
         sprintf(buf2, "%s killed by %s's volley of arrows at %s", GET_NAME(vict), GET_NAME(ch),
	      world[vict->in_room].name);
         mudlog(buf2, BRF, LVL_IMMORT, TRUE);
         if (MOB_FLAGGED(ch, MOB_MEMORY))
  	   forget(ch, vict);
       }
       die(vict, ch);
     }
  }
  if (IS_NPC(vict)) {
    HUNTING(vict) = ch;
    hunt_victim(vict);
    if (IS_HUMANOID(vict)) {
      act("$N says, 'I'm gonna kill $n!", TRUE, ch, 0, vict, TO_NOTVICT);
      if (!GET_EQ(vict, WEAR_WIELD))
       do_wield(vict, OBJN(missile, vict), 0, 0);
    }
  } /* End of IS_NPC */
}

