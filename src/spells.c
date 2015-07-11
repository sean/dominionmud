/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SPELLS.C                                     Based on CircleMUD *
*  Usage: Implementation of "manual spells."                              *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All Modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct cha_app_type cha_app[];
extern struct int_app_type int_app[];
extern struct index_data *obj_index;

extern struct descriptor_data *descriptor_list;

extern int mini_mud;
extern int pk_allowed;

extern struct default_mobile_stats *mob_defaults;
extern char   weapon_verbs[];
extern int    *max_ac_applys;
extern struct apply_mod_defaults *apmd;

void clearMemory(struct char_data * ch);
void act(char *str, int i, struct char_data * c, struct obj_data * o,
	      void *vict_obj, int j);

void damage(struct char_data * ch, struct char_data * victim,
		 int damage, int weapontype, int hitloc);

void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int  mag_savingthrow(struct char_data * ch, int type);
ACMD(do_peace);

/*
 * Special spells appear below.
 */
ASPELL(spell_calm)
{
  if (number(1, 101) < GET_SKILL(ch, SPELL_CALM))
    do_peace(ch, NULL, 0, 0);
  else
    send_to_char("You fail.\r\n",ch);
}


ASPELL(spell_create_water)
{
  int water = 0;

  void name_to_drinkcon(struct obj_data * obj, int type);
  void name_from_drinkcon(struct obj_data * obj);

  if (ch == NULL || obj == NULL)
    return;
  level = MAX((int)MIN((int)level, (int)LVL_IMPL), (int)1);

  if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
    if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
      #if 0
      name_from_drinkcon(obj);
      #endif
      GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
      #if 0
      name_to_drinkcon(obj, LIQ_SLIME);
      #endif
    } else {
      water = MAX((int)(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1)), (int)0);
      if (water > 0) {
	GET_OBJ_VAL(obj, 2)  = LIQ_WATER;
	GET_OBJ_VAL(obj, 1) += water;
	weight_change_object(obj, water);
	#if 0
        name_from_drinkcon(obj);
	name_to_drinkcon(obj, LIQ_WATER);
	#endif
        act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
  }
}


ASPELL(spell_pass_door)
{

}


ASPELL(spell_knock)
{

}

ASPELL(spell_blade_barrier)
{
   if (!ROOM_FLAGGED(ch->in_room, ROOM_BLADEBARRIER)) {
     SET_BIT(ROOM_FLAGS(ch->in_room), ROOM_BLADEBARRIER);
     obj_to_room(read_object(3013, VNUMBER), ch->in_room);
     act("You conjure a wall of blades which encircles the room.", FALSE, ch, 0, 0, TO_CHAR);
     act("$n conjure's a wall of blades which encircles the room.", FALSE, ch, 0, 0, TO_ROOM); 
   } else
     send_to_char("You fail to summon the blades.\r\n", ch);
}

ASPELL(spell_control_weather)
{
   void weather_change(void);
   /* Sorry no args at this time */
   weather_change();
}


ASPELL(spell_wizard_eye)
{
  room_num temp_room;

  if (victim == NULL)
    return;

  if (GET_LEVEL(victim) >= GET_LEVEL(ch)) {
    send_to_char("You failed.\r\n", ch);
    return;
  }
  act("$n's eyes become glassy and $e gets a far away look.",
      TRUE, ch, NULL, NULL, TO_ROOM);
  act("$N's eyes become glassy for a moment, then return to normal.",
      TRUE, NULL, NULL, victim, TO_NOTVICT);
  temp_room   = ch->in_room;
  ch->in_room = victim->in_room;
  look_at_room(ch, 1);
  ch->in_room = temp_room;
}

ASPELL(spell_purify)
{
  if (victim == NULL || ch == NULL)
    return;

  if (!IS_PERMGOOD(victim)) {
    if (victim != ch) {
      act("You attempt to purify $N.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n calls upon $s god to purify your soul.", FALSE, ch, 0, victim, TO_VICT);
      act("$n calls upon $s god to purfy $N's soul.", FALSE, ch, 0, victim, TO_ROOM);
    } else {
      act("You purify your soul.", FALSE, ch, 0, victim, TO_CHAR);
    }
    /* First move the align towards good */
    GET_ALIGNMENT(victim) = MIN(1000, GET_ALIGNMENT(victim) + dice(5, 20));
    /* then burn the victim a bit */
    GET_HIT(victim) = MAX(1, (int)GET_HIT(victim) - (GET_LEVEL(ch)*dice(2, 4)));
    /* now set the new perm align */
    if (GET_ALIGNMENT(victim) < -250)
      GET_PERMALIGN(victim) = ALIGN_EVIL;
    else if (GET_ALIGNMENT(victim) < 250)
      GET_PERMALIGN(victim) = ALIGN_NEUT;
    else
      GET_PERMALIGN(victim) = ALIGN_GOOD;
  } else {
    if (ch != victim) {
      act("You cleanse $N's soul.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n cleanses your soul.", FALSE, ch, 0, victim, TO_VICT);
      act("$n attempts to cleanse $N's soul.", FALSE, ch, 0, victim, TO_ROOM);
    } else {
      act("You cleanse your soul.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n attempts to cleanse $s soul.", FALSE, ch, 0, victim, TO_ROOM);
    }
    GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + (GET_LEVEL(ch)*dice(2, 4)));
    GET_ALIGNMENT(victim) = MIN(1000, GET_ALIGNMENT(victim) + dice(5, 20));
    if (IS_SET(AFF_FLAGS2(victim), AFF_POISON_I))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_POISON_I);
    if (IS_SET(AFF_FLAGS2(victim), AFF_POISON_II))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_POISON_II);
    if (IS_SET(AFF_FLAGS2(victim), AFF_POISON_III))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_POISON_III);
    if (IS_SET(AFF_FLAGS2(victim), AFF_DISEASE))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_DISEASE);
    if (IS_SET(AFF_FLAGS2(victim), AFF_PARALYSIS))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_PARALYSIS);
    if (IS_SET(AFF_FLAGS2(victim), AFF_PETRIFIED))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_PETRIFIED);
    if (IS_SET(AFF_FLAGS2(victim), AFF_CURSE))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_CURSE);
    if (IS_SET(AFF_FLAGS2(victim), AFF_BLIND))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_BLIND);
  }
}

ASPELL(spell_corrupt)
{
  if (victim == NULL || ch == NULL)
    return;

  if (!IS_PERMEVIL(victim)) {
    if (victim != ch) {
      act("You attempt to corrupt $N.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n calls upon $s god to corrupt your soul.", FALSE, ch, 0, victim, TO_VICT);
      act("$n calls upon $s god to corrupt $N's soul.", FALSE, ch, 0, victim, TO_ROOM);
    } else {
      act("You corrupt your soul.", FALSE, ch, 0, victim, TO_CHAR);
    }
    /* First move the align towards evil */
    GET_ALIGNMENT(victim) = MAX(-1000, GET_ALIGNMENT(victim) - dice(5, 20));
    /* then burn the victim a bit */
    GET_HIT(victim) = MAX(1, GET_HIT(victim) - (GET_LEVEL(ch)*dice(2, 4)));
    /* now set the new perm align */
    if (GET_ALIGNMENT(victim) < -250)
      GET_PERMALIGN(victim) = ALIGN_EVIL;
    else if (GET_ALIGNMENT(victim) < 250)
      GET_PERMALIGN(victim) = ALIGN_NEUT;
    else
      GET_PERMALIGN(victim) = ALIGN_GOOD;
  } else {
    if (ch != victim) {
      act("You corrupt $N's soul.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n corrupts your soul.", FALSE, ch, 0, victim, TO_VICT);
      act("$n attempts to corrupt $N's soul.", FALSE, ch, 0, victim, TO_ROOM);
    } else {
      act("You cleanse your soul.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n attempts to cleanse $s soul.", FALSE, ch, 0, victim, TO_ROOM);
    }
    GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + (GET_LEVEL(ch)*dice(2, 4)));
    GET_ALIGNMENT(victim) = MAX(-1000, GET_ALIGNMENT(victim) - dice(5, 20));
    if (IS_SET(AFF_FLAGS2(victim), AFF_POISON_I))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_POISON_I);
    if (IS_SET(AFF_FLAGS2(victim), AFF_POISON_II))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_POISON_II);
    if (IS_SET(AFF_FLAGS2(victim), AFF_POISON_III))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_POISON_III);
    if (IS_SET(AFF_FLAGS2(victim), AFF_DISEASE))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_DISEASE);
    if (IS_SET(AFF_FLAGS2(victim), AFF_PARALYSIS))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_PARALYSIS);
    if (IS_SET(AFF_FLAGS2(victim), AFF_PETRIFIED))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_PETRIFIED);
    if (IS_SET(AFF_FLAGS2(victim), AFF_CURSE))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_CURSE);
    if (IS_SET(AFF_FLAGS2(victim), AFF_BLIND))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_BLIND);
  }
}

ASPELL(spell_balance)
{
  if (victim == NULL || ch == NULL)
    return;

  if (!IS_PERMNEUT(victim)) {
    if (victim != ch) {
      act("You attempt to balance $N's soul.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n calls upon $s god to balance your soul.", FALSE, ch, 0, victim, TO_VICT);
      act("$n calls upon $s god to balance $N's soul.", FALSE, ch, 0, victim, TO_ROOM);
    } else {
      act("You balance your soul.", FALSE, ch, 0, victim, TO_CHAR);
    }
    /* First move the align towards neutral */
    if (GET_ALIGNMENT(victim) < 0)
      GET_ALIGNMENT(victim) = MIN(1000, GET_ALIGNMENT(victim) + dice(5, 20));
    else
      GET_ALIGNMENT(victim) = MAX(-1000, GET_ALIGNMENT(victim) - dice(5, 20));
    /* then burn the victim a bit */
    GET_HIT(victim) = MAX(1, GET_HIT(victim) - (GET_LEVEL(ch)*dice(2, 4)));
    /* now set the new perm align */
    if (GET_ALIGNMENT(victim) < -250)
      GET_PERMALIGN(victim) = ALIGN_EVIL;
    else if (GET_ALIGNMENT(victim) < 250)
      GET_PERMALIGN(victim) = ALIGN_NEUT;
    else
      GET_PERMALIGN(victim) = ALIGN_GOOD;
  } else {
    if (ch != victim) {
      act("You balance $N's soul.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n balances your soul.", FALSE, ch, 0, victim, TO_VICT);
      act("$n attempts to balance $N's soul.", FALSE, ch, 0, victim, TO_ROOM);
    } else {
      act("You balance your soul.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n attempts to balance $s soul.", FALSE, ch, 0, victim, TO_ROOM);
    }
    GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + (GET_LEVEL(ch)*dice(2, 4)));
    if (GET_ALIGNMENT(victim) < 0)
      GET_ALIGNMENT(victim) = MIN(1000, GET_ALIGNMENT(victim) + dice(5, 20));
    else
      GET_ALIGNMENT(victim) = MAX(-1000, GET_ALIGNMENT(victim) - dice(5, 20));
    if (IS_SET(AFF_FLAGS2(victim), AFF_POISON_I))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_POISON_I);
    if (IS_SET(AFF_FLAGS2(victim), AFF_POISON_II))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_POISON_II);
    if (IS_SET(AFF_FLAGS2(victim), AFF_POISON_III))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_POISON_III);
    if (IS_SET(AFF_FLAGS2(victim), AFF_DISEASE))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_DISEASE);
    if (IS_SET(AFF_FLAGS2(victim), AFF_PARALYSIS))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_PARALYSIS);
    if (IS_SET(AFF_FLAGS2(victim), AFF_PETRIFIED))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_PETRIFIED);
    if (IS_SET(AFF_FLAGS2(victim), AFF_CURSE))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_CURSE);
    if (IS_SET(AFF_FLAGS2(victim), AFF_BLIND))
      REMOVE_BIT(AFF_FLAGS2(victim), AFF_BLIND);
  }
}


ACMD(do_recall)
{
  extern sh_int mortal_start_room;
  room_num rec_loc = real_room(mortal_start_room);

  if (FIGHTING(ch) || GET_LEVEL(ch) >= 5)
    return;
  else {
    act("$n calls upon $s god and disappears.", TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, rec_loc);
    act("$n appears in the middle of the room.", TRUE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);
  }
}


ASPELL(spell_recall)
{
  room_num zn = 0, rnum;
  zn   = world[ch->in_room].zone;
  rnum = zone_table[zn].recall_loc;

  if (victim == NULL || IS_NPC(victim))
    return;

  if (!IS_IMMORT(ch) && rnum < 100)
    return;
  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, rnum);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
}


ASPELL(spell_teleport)
{
  int    to_room;
  extern int top_of_world;
  extern struct zone_data *zone_table;

  if (victim != NULL)
    return;

  do {
    to_room = number(0, top_of_world);
  } while (ROOM_FLAGGED(to_room, (ROOM_PRIVATE | ROOM_DEATH)) ||
           (zone_table[to_room].number == 0));

  act("$n slowly fades out of existence and is gone.",
      FALSE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, to_room);
  act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
}


#define ASTRAL_ENTRANCE   13006
#define ASTRAL_ZONE       130

ASPELL(spell_astral_walk)
{
  extern sh_int r_mortal_start_room;
  int    to_astral;
  to_astral = real_room(ASTRAL_ENTRANCE);

  if (victim == NULL || IS_NPC(victim))
    return;

  /* So it doesn't crash the game */
  if (victim) {
    send_to_char("You are unable to pass through the barrier into the Astral Plane!\r\n",  victim);
    return;
  }

  if (world[victim->in_room].zone == ASTRAL_ZONE) {
    act("$n wavers and fades out of existence.",
	FALSE, victim, 0, 0, TO_ROOM);
     char_from_room(victim);
     char_to_room(victim, r_mortal_start_room);
     act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
     look_at_room(victim, 0);
  } else /*if (world[victim->in_room].zone != ASTRAL_ZONE)*/ {
     act("$n wavers and fades out of existence.",
	FALSE, victim, 0, 0, TO_ROOM);
     char_from_room(victim);
     char_to_room(victim, to_astral);
     act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
     look_at_room(victim, 0);
  }
}


ASPELL(spell_portal)
{
  if (ch == NULL || victim == NULL)
    return;

  if (ch == victim)
    return;

  if (GET_LEVEL(victim) > MIN(LVL_IMMORT - 1, GET_LEVEL(ch) + 3)) {
    send_to_char("You failed.\r\n", ch);
    return;
  }

  act("A mysterious portal opens before $n and $e enters it.", TRUE, ch, 0, 0, TO_ROOM);

  char_from_room(ch);
  char_to_room(ch, victim->in_room);

  if (world[victim->in_room].sector_type < SECT_CITY)
    act("The wall shimmers and $n steps through.", TRUE, ch, 0, 0, TO_ROOM);
  else
    act("A blackhole opens and $n steps through.", TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
}


#define SUMMON_FAIL "You failed.\r\n"

ASPELL(spell_summon)
{
  if (ch == NULL || victim == NULL)
    return;

  if (GET_LEVEL(victim) > MIN(LVL_IMMORT - 1, level + 3)) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  if (!pk_allowed) {
    if (MOB_FLAGGED(victim, MOB_AGGRESSIVE)) {
      act("As the words escape your lips and $N travels\r\n"
	  "through time and space towards you, you realize that $E is\r\n"
	  "aggressive and might harm you, so you wisely send $M back.",
	  FALSE, ch, 0, victim, TO_CHAR);
      return;
    }
  }

  if (MOB_FLAGGED(victim, MOB_NOCHARM) ||
      (IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL))) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  char_from_room(victim);
  char_to_room(victim, ch->in_room);

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  look_at_room(victim, 0);
}



ASPELL(spell_locate_object)
{
  struct obj_data *i;
  char name[MAX_INPUT_LENGTH];
  int j;

  strcpy(name, fname(obj->name));
  j = level >> 1;

  for (i = object_list; i && (j > 0); i = i->next) {
    if (!isname(name, i->name))
      continue;

    if (i->carried_by)
      sprintf(buf, "%s is being carried by %s.\n\r",
	      i->short_description, PERS(i->carried_by, ch));
    else if (i->in_room != NOWHERE)
      sprintf(buf, "%s is in %s.\n\r", i->short_description,
	      world[i->in_room].name);
    else if (i->in_obj)
      sprintf(buf, "%s is in %s.\n\r", i->short_description,
	      i->in_obj->short_description);
    else if (i->worn_by)
      sprintf(buf, "%s is being worn by %s.\n\r",
	      i->short_description, PERS(i->worn_by, ch));
    else
      sprintf(buf, "%s's location is uncertain.\n\r",
	      i->short_description);

    CAP(buf);
    send_to_char(buf, ch);
    j--;
  }

  if (j == level >> 1)
    send_to_char("You sense nothing.\n\r", ch);
}



ASPELL(spell_charm)
{
  struct affected_type af;

  if (victim == NULL || ch == NULL)
    return;

  if (victim == ch)
    send_to_char("You like yourself even better!\r\n", ch);
  else if (IS_AFFECTED(victim, AFF_SANCTUARY))
    send_to_char("Your victim is protected by sanctuary!\r\n", ch);
  else if (MOB_FLAGGED(victim, MOB_NOCHARM))
    send_to_char("Your victim resists!\r\n", ch);
  else if (IS_AFFECTED(ch, AFF_CHARM))
    send_to_char("You can't have any followers of your own!\r\n", ch);
  else if (IS_AFFECTED(victim, AFF_CHARM) || level < GET_LEVEL(victim))
    send_to_char("You fail.\r\n", ch);
  else if (circle_follow(victim, ch))
    send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
  else if (mag_savingthrow(victim, SAVING_PARA))
    send_to_char("Your victim resists!\r\n", ch);
  else {
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type = SPELL_CHARM;

    if (GET_INT(victim))
      af.duration = 24 * 18 / GET_INT(victim);
    else
      af.duration = 24 * 18;

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af, FALSE);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    if (IS_NPC(victim)) {
      REMOVE_BIT(MOB_FLAGS(victim), MOB_AGGRESSIVE);
      REMOVE_BIT(MOB_FLAGS(victim), MOB_SPEC);
    }
  }
}



ASPELL(spell_identify)
{
  int i;
  int found;

  struct time_info_data age(struct char_data * ch);

  extern char *spells[];

  extern char *item_types[];
  extern char *extra_bits[];
  extern char *apply_types[];
  extern char *affected_bits[];

  if (obj) {
    send_to_char("You feel informed:\r\n", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }
    send_to_char("Item is: ", ch);
    sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    sprintf(buf, "Weight: %d, Value: %d, Rent: %d\r\n",
	    GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_RENT(obj));
    send_to_char(buf, ch);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);

      if (GET_OBJ_VAL(obj, 1) >= 1)
	sprintf(buf, "%s %s", buf, spells[GET_OBJ_VAL(obj, 1)]);
      if (GET_OBJ_VAL(obj, 2) >= 1)
	sprintf(buf, "%s %s", buf, spells[GET_OBJ_VAL(obj, 2)]);
      if (GET_OBJ_VAL(obj, 3) >= 1)
	sprintf(buf, "%s %s", buf, spells[GET_OBJ_VAL(obj, 3)]);
      sprintf(buf, "%s\r\n", buf);
      send_to_char(buf, ch);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);
      sprintf(buf, "%s %s\r\n", buf, spells[GET_OBJ_VAL(obj, 3)]);
      sprintf(buf, "%sIt has %ld maximum charge%s and %ld remaining.\r\n", buf,
	      GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 1) == 1 ? "" : "s",
	      GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
      break;
    case ITEM_MISSILE:
    case ITEM_WEAPON:
      sprintf(buf, "Damage Dice is '%ldD%ld'", GET_OBJ_VAL(obj, 1),
	      GET_OBJ_VAL(obj, 2));
      sprintf(buf, "%s for an average per-round damage of %.1f.\r\n", buf,
	      (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1)));
      send_to_char(buf, ch);
      break;
    case ITEM_ARMOR:
      sprintf(buf, "AC-apply is %ld\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
      break;
    }
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
	  (obj->affected[i].modifier != 0)) {
	if (!found) {
	  send_to_char("Can affect you as :\r\n", ch);
	  found = TRUE;
	}
	sprinttype(obj->affected[i].location, apply_types, buf2);
	sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
	send_to_char(buf, ch);
      }
    }
  } else if (victim) {		/* victim */
    sprintf(buf, "Name: %s\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    if (!IS_NPC(victim)) {
      sprintf(buf, "%s is %d years, %d months, %d days and %d hours old.\r\n",
	      GET_NAME(victim), age(victim).year, age(victim).month,
	      age(victim).day, age(victim).hours);
      send_to_char(buf, ch);
    }
    sprintf(buf, "Height %d cm, Weight %d pounds\r\n",
	    GET_HEIGHT(victim), GET_WEIGHT(victim));
    send_to_char(buf, ch);

  }
}


ASPELL(spell_dispel_illusion)
{
    if ((victim == NULL || ch == NULL) || (victim == ch))
      return;

    if (GET_RACE(victim) != RACE_ILLUSIONARY) {
      act("You try to dispel $N, but fail!\r\n",
	   FALSE, ch, 0, victim, TO_CHAR);
      act("$n tries to dispel you, but fails.\r\n",
	   FALSE, ch, 0, victim, TO_VICT);
      act("$n tries to dispel $N, but nothing happens.\r\n",
	   FALSE, ch, 0, victim, TO_ROOM);
    } else if (GET_RACE(victim) == RACE_ILLUSIONARY) {
      /* Auto kill for illusions, so pcs get exp */
      GET_HIT(victim) = -10;
      update_pos(victim);
    }
    return;
}


ASPELL(spell_dispel_magic)
{
   if ((GET_LEVEL(ch) > GET_LEVEL(victim)) && (victim->affected || victim->affected2)) {
     while (victim->affected)
       affect_remove(victim, victim->affected, FALSE);
     while (victim->affected2)
       affect_remove(victim, victim->affected2, TRUE);
     act("$n makes some strange gestures with $s hands and you feel different.",
	  FALSE, ch, 0, victim, TO_VICT);
     act("$n makes some strange gestures with $s hands and dispels $N's magic!",
	  FALSE, ch, 0, victim, TO_ROOM);
     act("You make some gestures and remove all of $N's affections.",
	  FALSE, ch, 0, victim, TO_CHAR);
   } else {
     send_to_char("You fail to dispel the magic!\r\n", ch);
     act("$n just tried to dispel the magic affecting you!",
	  FALSE, ch, 0, victim, TO_VICT);
   }
}



ASPELL(spell_enchant_weapon)
{
  int i;

  if (ch == NULL || obj == NULL)
    return;

  if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
	return;

    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = (level * .5);

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = (level * .5);

    if (IS_GOOD(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows blue.", FALSE, ch, 
obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}


ASPELL(spell_curse_weapon)
{
  int i;

  if (ch == NULL || obj == NULL)
    return;

  if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET(GET_OBJ_EXTRA(obj), ITEM_NODROP)) {

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      obj->affected[i].location = APPLY_NONE;

    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODROP);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = -(1 + (2 * MAX(1, level-30)));

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = -(1 + (2 * MAX(1, level-25)));

    /* It only does 1d2 now */
    GET_OBJ_VAL(obj, 1) = MAX(1L, GET_OBJ_VAL(obj, 1) - dice(1, 4));
    GET_OBJ_VAL(obj, 2) = MAX(2L, GET_OBJ_VAL(obj, 2) - dice(2, 4));

    if (IS_GOOD(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}



ASPELL(spell_enchant_armor)
{
  int i;

  if (ch == NULL || obj == NULL)
    return;

  if ((GET_OBJ_TYPE(obj) == ITEM_ARMOR) &&
      !IS_SET(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
	return;

    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = (1 + MAX(2, level-38));

    obj->affected[1].location = APPLY_AC;
    obj->affected[1].modifier = + (1 + MAX(2, level-30));

    GET_OBJ_VAL(obj, 0) += 4;
    GET_OBJ_VAL(obj, 4)  = 0;

    if (IS_GOOD(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}


ASPELL(spell_curse_armor)
{
  int i;

  if (ch == NULL || obj == NULL)
    return;

  if ((GET_OBJ_TYPE(obj) == ITEM_ARMOR) &&
      !IS_SET(GET_OBJ_EXTRA(obj), ITEM_NODROP)) {

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
	return;

    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODROP);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = - (1 + (level >= 18));

    obj->affected[1].location = APPLY_AC;
    obj->affected[1].modifier = - (1 + (level >= 20));

    GET_OBJ_VAL(obj, 0) -= 4;
    GET_OBJ_VAL(obj, 4)  = 0;

    if (IS_GOOD(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}




ASPELL(spell_detect_poison)
{
  if (victim) {
    if (victim == ch) {
      if (IS_AFFECTED2(victim, AFF_POISON_I) || \
	  IS_AFFECTED2(victim, AFF_POISON_II) || \
	  IS_AFFECTED2(victim, AFF_POISON_III))
	send_to_char("You can sense poison in your blood.\r\n", ch);
      else
	send_to_char("You feel healthy.\r\n", ch);
    } else {
      if (IS_AFFECTED2(victim, AFF_POISON_I) || \
	  IS_AFFECTED2(victim, AFF_POISON_II) || \
	  IS_AFFECTED2(victim, AFF_POISON_III))
	act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
      else
	act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  if (obj) {
    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
    case ITEM_FOOD:
      if (GET_OBJ_VAL(obj, 3))
	act("You sense that $p has been contaminated.",FALSE,ch,obj,0,TO_CHAR);
      else
	act("You sense that $p is safe for consumption.", FALSE, ch, obj, 0,
	    TO_CHAR);
      break;
    default:
      send_to_char("You sense that it should not be consumed.\r\n", ch);
    }
  }
}
