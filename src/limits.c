/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  LIMITS.C                                     Based on CircleMUD *
*  Usage: Limit & Gain funcs for HMV, exp, hunger/thirst/tired, idle time *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __LIMITS_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct room_data *world;
extern int max_exp_gain;
extern int max_exp_loss;
/* TD - SPM */
static char make_title_buf[128];
extern char *pc_race_types[];
int    exp_needed(int level);
extern int regen_gain[];
ACMD(do_infobar);
int has_mail(long recip);

/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* manapoint gain pr. game hour */
int mana_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = GET_LEVEL(ch);
  } else {
    if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_NOHEAL))
      return 0;

    gain = (((GET_WIS(ch) + GET_INT(ch)) >> 1) + (GET_LEVEL(ch) >> 2));

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_MEDITATING:
      gain <<= 3;                /* x8 */
      break;
    case POS_SLEEPING:
      gain <<= 1;                /* x2 */
      break;
    case POS_RESTING:
      gain += (gain >> 1);       /* +half */
      break;
    case POS_SITTING:
      gain += (gain >> 2);       /* +quarter */
      break;
    }
  }

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;                  /* 1/4 */

  /* Tiredness can really mess up your mana gain */
  if (GET_COND(ch, TIRED) <= 1)
    gain >>= 3;                  /* 1/8 */

  if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_REGEN))
    gain <<= 1;                  /* x2 */

  return (gain);
}


int hit_gain(struct char_data * ch)
/* Hitpoint gain pr. game hour */
{
  int gain;

  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);
    /* Neat and fast */
  } else {
    gain = (GET_CON(ch) >> 1);

    /* Position calculations    */

    switch (GET_POS(ch)) {
    case POS_MEDITATING:
      gain <<= 1;               /* x2 */
      break;
    case POS_SLEEPING:
      gain += (gain >> 1);      /* +half */
      break;
    case POS_RESTING:
      gain += (gain >> 2);      /* +quarter */
      break;
    case POS_SITTING:
      gain += (gain >> 3);      /* +eighth */
      break;
    }
  }

  if (IS_AFFECTED2(ch, AFF_POISON_I))
    gain = -(gain >> 3);                 /* -1/8 */

  if (IS_AFFECTED2(ch, AFF_POISON_II))
    gain = -(gain >> 2);                 /* -1/4 */

  if (IS_AFFECTED2(ch, AFF_POISON_III))
    gain = -(gain >> 1);                 /* -1/2 */

  if (IS_AFFECTED2(ch, AFF_DISEASE) && GET_POS(ch) == POS_SLEEPING)
    gain >>= 4;                          /* 1/16 */
  else if (IS_AFFECTED2(ch, AFF_DISEASE))
    gain = 0;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;                          /* 1/4 */

  if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_REGEN))
    gain <<= 2;                          /* x4 */

  /* NO HP gain when severely wounded!! */
  if (GET_WOUNDS(ch) >= 1)
    gain = 0;

  return (gain);
}


int move_gain(struct char_data * ch)
/* move gain pr. game hour */
{
  int gain;

  if (IS_NPC(ch)) {
    return (GET_LEVEL(ch));
    /* Neat and fast */
  } else {
    gain = ((GET_CON(ch) + GET_DEX(ch)) >> 1);

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_MEDITATING:
      gain -= (gain >> 1);      /* -half */
      break;
    case POS_SLEEPING:
      gain += (gain >> 1);      /* +half */
      break;
    case POS_RESTING:
      gain += (gain >> 2);      /* +quarter */
      break;
    case POS_SITTING:
      gain += (gain >> 3);      /* +eighth */
      break;
    }
  }

  if (IS_AFFECTED2(ch, AFF_POISON_I) || \
      IS_AFFECTED2(ch, AFF_POISON_II) || \
      IS_AFFECTED2(ch, AFF_POISON_III) || \
      (GET_COND(ch, THIRST) == 0) || \
      (GET_COND(ch, FULL) == 0))
     gain >>= 2;                /* 1/4 */

  if (IS_AFFECTED2(ch, AFF_DISEASE) || (GET_COND(ch, TIRED) == 0))
     gain >>= 3;                /* 1/8 */

  if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_REGEN))
     gain <<= 3;                /* x8 */

  return (gain);
}

void set_title(struct char_data * ch, char *title)
{
  if (title == NULL) {
    strcpy(make_title_buf, "the ");
    sprinttype(ch->player.race, pc_race_types, buf);
    strcat(make_title_buf, buf);
    title = str_dup(make_title_buf);
  }

  if (strlen(title) > MAX_TITLE_LENGTH)
    title[MAX_TITLE_LENGTH] = '\0';

  if (GET_TITLE(ch) != NULL)
    free(GET_TITLE(ch));

  GET_TITLE(ch) = str_dup(title);
}


void check_autowiz(struct char_data * ch)
{
  char buf[100];
  extern int use_autowiz;
  extern int min_wizlist_lev;
  pid_t getpid(void);

  if (use_autowiz && GET_LEVEL(ch) >= LVL_IMMORT) {
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
	    WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
  }
}



void gain_exp(struct char_data * ch, int gain)
{
  int max_exp_number;

  if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) >= LVL_IMMORT)))
    return;

  max_exp_number = (exp_needed((GET_LEVEL(ch)+1)) -
		    exp_needed(GET_LEVEL(ch)));
  max_exp_number /= 16;        /* Min of 16 kills per level */

  if (IS_NPC(ch)) {
    GET_EXP(ch) += gain;
    return;
  }

  if (gain > 0)
    gain = MIN((long)max_exp_number, (long)gain);  /* put a cap on the max gain per kill */

  if (CAN_LEV(ch)) {
    if (IS_NPC(ch))
      advance_level(ch);
    else if (gain + GET_EXP(ch) >= exp_needed(GET_LEVEL(ch)+1)) {
      GET_EXP(ch) = exp_needed(GET_LEVEL(ch)+1) + 1;
      send_to_char("You must advance in level before you can become more experienced.\r\n", ch);
    } else
      GET_EXP(ch) += gain;
  } else if (gain > 0) {
    GET_EXP(ch) += gain;
  } else if (gain < 0) {
    gain = MAX((long)-max_exp_loss, (long)gain);    /* Cap max exp lost per death */
    GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
  }
  GET_SESS_EXP(ch) += gain;
}


void gain_exp_regardless(struct char_data * ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;

  GET_EXP(ch) += gain;
  GET_SESS_EXP(ch) += gain;

  if (GET_EXP(ch) < 0)
    GET_EXP(ch) = 0;

  if (!IS_NPC(ch)) {
    while (GET_LEVEL(ch) < LVL_IMPL &&
	   GET_EXP(ch) >= exp_needed(GET_LEVEL(ch) + 1)) {
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }

    if (is_altered) {
      if (num_levels == 1)
	send_to_char("You rise a level!\r\n", ch);
      else {
	sprintf(buf, "You rise %d levels!\r\n", num_levels);
	send_to_char(buf, ch);
      }
      check_autowiz(ch);
    }
  }
}


void gain_condition(struct char_data * ch, int condition, int value)
{
  bool intoxicated;

  if (GET_COND(ch, condition) == -1)    /* No change */
    return;

  if (IS_IMMORT(ch)) {
    GET_COND(ch, condition) = -1;
    return;
  }

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, (int)GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(24, (int)GET_COND(ch, condition));

  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_INFOBAR))
    do_infobar(ch, 0, 0, SCMDB_CONDITION);

  if (GET_COND(ch, condition) || PLR_FLAGGED(ch, PLR_WRITING))
    return;

  switch (condition) {
  case FULL:
    send_to_char("You are hungry.\r\n", ch);
    return;
  case THIRST:
    send_to_char("You are thirsty.\r\n", ch);
    return;
  case DRUNK:
    if (intoxicated)
      send_to_char("You are now sober.\r\n", ch);
    return;
  case TIRED:
    send_to_char("You are tired and require sleep.\r\n", ch);
    return;
  default:
    break;
  }
}


void check_idling(struct char_data * ch)
{
  extern int free_rent;
  void Crash_rentsave(struct char_data *ch, int cost);

  if (++(ch->char_specials.timer) > 10)
    if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE) {
      GET_WAS_IN(ch) = ch->in_room;
      if (FIGHTING(ch)) {
	stop_fighting(FIGHTING(ch));
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
      save_char(ch, NOWHERE);
      Crash_crashsave(ch);
      char_from_room(ch);
      char_to_room(ch, 1);
    } else if (ch->char_specials.timer > 48) {
      if (ch->in_room != NOWHERE)
	char_from_room(ch);
      char_to_room(ch, 3);
      if (ch->desc)
	close_socket(ch->desc);
      ch->desc = NULL;
      if (free_rent)
	Crash_rentsave(ch, 0);
      else
	Crash_idlesave(ch);
      sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
      mudlog(buf, CMP, LVL_GOD, TRUE);
      extract_char(ch);
    }
}

/* return a number from 1 to 100 for chance of death */
int die_of_oldage_chance(struct char_data *ch)
{
   struct time_info_data playing_time;
   struct time_info_data real_time_passed(time_t t2, time_t t1);

   playing_time = real_time_passed((time(0) - ch->player.time.logon), 0);

   /* You will not till until you've played atleast for 3 days straight */
   if (playing_time.hours < 36)
     return 11;
   /* 10 is _almost_ a 0% chance of dying */
   else if (IS_ANCIENT(ch))
     return 8;
   else if (IS_ELDERLY(ch))
     return 9;
   else if (IS_VERYOLD(ch))
     return 10;
   else
     return 11;
}

/* Update PCs, NPCs, and objects */
void point_update(void)
{
  void   update_char_objects(struct char_data * ch);      /* handler.c */
  void   extract_obj(struct obj_data * obj);      /* handler.c */
  void   raw_kill(struct char_data *i, struct char_data *k);
  struct affected_type af;
  struct char_data *i, *next_char, *tch, *tch_next;
  struct obj_data *j, *next_thing, *jj = NULL, *next_thing2;
  extern struct index_data *obj_index;

  /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;

    if (GET_POS(i) >= POS_STUNNED) {
      GET_HIT(i)  = MIN((int)GET_HIT(i)  + hit_gain(i), (int)GET_MAX_HIT(i));
      GET_MANA(i) = MIN((int)GET_MANA(i) + mana_gain(i), (int)GET_MAX_MANA(i));
      GET_MOVE(i) = MIN((int)GET_MOVE(i) + move_gain(i), (int)GET_MAX_MOVE(i));

      if (ROOM_FLAGGED(i->in_room, ROOM_BLADEBARRIER))
	REMOVE_BIT(ROOM_FLAGS(i->in_room), ROOM_BLADEBARRIER);
      /* Fix to allow ppl to heal */
      if (GET_WOUNDS(i) && (GET_HIT(i) == GET_MAX_HIT(i)))
	GET_WOUNDS(i)--;
      if (IS_AFFECTED(i, AFF_POISON_I))
	damage(i, i, 2, SPELL_POISON, -1);
      if (IS_AFFECTED2(i, AFF_POISON_II))
	damage(i, i, 8, SPELL_POISON, -1);
      if (IS_AFFECTED2(i, AFF_POISON_III))
	damage(i, i, 24, SPELL_POISON, -1);
      if (IS_AFFECTED2(i, AFF_DISEASE)) {
	damage(i, i, dice(3, 5), SPELL_DISEASE, -1);
	/* See if someone in the room catches the plague */
	for (tch = world[i->in_room].people; tch; tch = tch_next) {
	  tch_next = tch->next_in_room;
	  if (tch == i)
	    continue;
          if (IS_IMMORT(tch))
            continue;
	  if (!IS_AFFECTED2(tch, AFF_DISEASE) && (number(0, 10) == 10)) {
	    act("$n begins coughing severely, spewing bloody mucus on $N.", FALSE, i, 0, tch, TO_ROOM);
	    act("$n begins coughing severely, spewing bloody mucus on you!", FALSE, i, 0, tch, TO_VICT);
	    act("You begin coughing severely, spewing bloody mucus on $N.", FALSE, i, 0, tch, TO_CHAR);
	    /* Only one victim per person */
	    af.location = APPLY_CON;
	    af.duration = dice(2, GET_LEVEL(i));
	    af.modifier = -2;
	    af.bitvector = AFF_DISEASE;
	    affect_join(tch, &af, TRUE, FALSE, TRUE, FALSE, TRUE);
	    break;
	  }
	}
	/* End of plague code */
      }
      if (GET_WOUNDS(i) >= 1) {
	damage(i, i, GET_WOUNDS(i), TYPE_SUFFERING, -1);
	send_to_char("Your wounds are bleeding pretty severely, you should seek help.\r\n", i);
	j = read_object(OBJ_VNUM_POOLOBLOOD, VNUMBER);
	GET_OBJ_TIMER(j) = number(4, 8);
	obj_to_room(j, i->in_room);
      }
      update_pos(i);
    } else if (GET_POS(i) == POS_INCAP)
      damage(i, i, dice(2, 4), TYPE_SUFFERING, -1);
    else if (GET_POS(i) == POS_MORTALLYW)
      damage(i, i, dice(6, 8), TYPE_SUFFERING, -1);
    if (!IS_NPC(i)) {
      update_char_objects(i);

      if (GET_LEVEL(i) < LVL_GOD)
	check_idling(i);
   
      if (WEATHER(i->in_room) == SKY_SIZZLING)
        gain_condition(i, THIRST, -3);
      else if (WEATHER(i->in_room) == SKY_HEATWAVE)
        gain_condition(i, THIRST, -5);
      else
        gain_condition(i, THIRST, -1);

      gain_condition(i, FULL,  -1);
      gain_condition(i, DRUNK, -1);
      if (GET_POS(i) <= POS_SLEEPING)
        gain_condition(i, TIRED, 12);
      else
        gain_condition(i, TIRED, -1);

      /* death of old age */
      if (IS_VERYOLD(i) && !IS_IMMORTAL(i))
        if (number(1, 10) > die_of_oldage_chance(i)) {
  	  act("You feel a great pain in your chest!", FALSE, i, 0, 0, TO_CHAR);
	  act("$n clutches $s chest, and gropes about wildly!", FALSE, i, 0, 0, TO_ROOM);
	  SET_BIT(PLR_FLAGS(i), PLR_DELETED);
	  send_to_char("You have died to old-age!\r\n", i);
	  raw_kill(i, NULL);
        }
      if (!IS_NPC(i) && PRF_FLAGGED(i, PRF_INFOBAR))
        do_infobar(i, 0, 0, SCMDB_GENUPDATE);

      if (!IS_NPC(i) && has_mail(GET_IDNUM(i)) && 
          !(PLR_FLAGGED(i, PLR_WRITING) || PLR_FLAGGED(i, PLR_EDITING) || 
	    PLR_FLAGGED(i, PLR_MAILING))) {
        sprintf(buf2, "You have %sMAIL%s.\r\n",  CCCYN(i, C_SPR), CCNRM(i, C_SPR));
        send_to_char(buf2, i);
      }
      /* end of !IS_NPC */
    }
  }

  /* objects */
  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;       /* Next in object list */

    /* If this object i a blade barrier remove it */
    if (GET_OBJ_VNUM(j) == 3013)
      extract_obj(j);

    if (IS_OBJ_STAT(j, ITEM_NOSUN)) {
      if ((WEATHER(j->in_room) >= SUN_RISE && 
          WEATHER(j->in_room) < SUN_DARK) && 
         ((j->carried_by && SECT(j->carried_by->in_room) != SECT_INSIDE) ||
         (j->in_room != NOWHERE && SECT(j->in_room) != SECT_INSIDE) ||
         (j->carried_by && !IS_SET(ROOM_FLAGS(j->carried_by->in_room), ROOM_DARK)) ||
         ((j->in_room != NOWHERE && !IS_SET(ROOM_FLAGS(j->in_room), ROOM_DARK))))) {
        if (j->carried_by)
	  act("$p crumbles to dust in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
	  act("$p crumbles to dust.",
	      TRUE, world[j->in_room].people, j, 0, TO_ROOM | TO_CHAR);
	}
	if (j->carried_by)
	  obj_to_room(jj, j->carried_by->in_room);
	else if (j->in_room != NOWHERE)
	  obj_to_room(jj, j->in_room);
        extract_obj(j);
      }
    }
    /* If the item is equipped and its broken remove it */
    if (j->worn_by && j->worn_on != -1 && IS_OBJ_STAT(j, ITEM_BROKEN))
      unequip_char(j->worn_by, j->worn_on);

    /* If this is a corpse */
    if ((GET_OBJ_TYPE(j) == ITEM_CONTAINER) && GET_OBJ_VAL(j, 3)) {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
	GET_OBJ_TIMER(j)--;

      if (!GET_OBJ_TIMER(j)) {

	if (j->carried_by)
	  act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
	  if (GET_OBJ_VNUM(j) != OBJ_VNUM_POOLOBLOOD) {
	   act("A hot wind blows past and $p crumbles to dust.",
	      TRUE, world[j->in_room].people, j, 0, TO_ROOM);
	   act("A hot wind blows past and $p crumbles to dust.",
	      TRUE, world[j->in_room].people, j, 0, TO_CHAR);
	  }
	}
	for (jj = j->contains; jj; jj = next_thing2) {
	  next_thing2 = jj->next_content;       /* Next in inventory */
	  obj_from_obj(jj);

	  if (j->in_obj)
	    obj_to_obj(jj, j->in_obj);
	  else if (j->carried_by)
	    obj_to_room(jj, j->carried_by->in_room);
	  else if (j->in_room != NOWHERE)
	    obj_to_room(jj, j->in_room);
	  else
	    assert(FALSE);
	}
	extract_obj(j);
      }
    }
  }
}


/* Chars that regen a bit every 15 seconds - 4 times a tick */
void perform_spc_update(void)
{
  struct descriptor_data *d, *next_d = NULL;
  extern struct descriptor_data *descriptor_list;

  /* characters */
  for (d = descriptor_list; d; d = next_d) {
    next_d = d->next;
    if (!d) break;
    if (STATE(d) != CON_PLAYING)
      continue;
    if (IS_AFFECTED(d->character, AFF_REGENERATION))
      GET_HIT(d->character) = MIN((int)GET_HIT(d->character) + (GET_CON(d->character) >> 2), (int)GET_MAX_HIT(d->character));
    else if (GET_HIT_REGEN(d->character))
      GET_HIT(d->character) = MIN((int)GET_HIT(d->character) + GET_HIT_REGEN(d->character), (int)GET_MAX_HIT(d->character));
    if (GET_MANA_REGEN(d->character))
      GET_MANA(d->character) = MIN((int)GET_MANA(d->character) + GET_MANA_REGEN(d->character), (int)GET_MAX_MANA(d->character));
    if (GET_MOVE_REGEN(d->character))
      GET_MOVE(d->character) = MIN((int)GET_MOVE(d->character) + GET_MOVE_REGEN(d->character), (int)GET_MAX_MOVE(d->character));
    /* Add more for food/tired/thirst regen here */
    if (FIGHTING(d->character) && IS_NPC(d->character)) {
      if (IS_AFFECTED(FIGHTING(d->character), AFF_REGENERATION))
	GET_HIT(FIGHTING(d->character)) = MIN((int)GET_HIT(FIGHTING(d->character)) + (GET_CON(FIGHTING(d->character)) >> 2), (int)GET_MAX_HIT(FIGHTING(d->character)));
      else if (GET_HIT_REGEN(FIGHTING(d->character)))
	GET_HIT(FIGHTING(d->character)) = MIN((int)GET_HIT(FIGHTING(d->character)) + GET_HIT_REGEN(FIGHTING(d->character)), (int)GET_MAX_HIT(FIGHTING(d->character)));
      if (GET_MANA_REGEN(FIGHTING(d->character)))
	GET_MANA(FIGHTING(d->character)) = MIN((int)GET_MANA(FIGHTING(d->character)) + GET_MANA_REGEN(FIGHTING(d->character)), (int)GET_MAX_MANA(FIGHTING(d->character)));
      if (GET_MOVE_REGEN(FIGHTING(d->character)))
	GET_MOVE(FIGHTING(d->character)) = MIN((int)GET_MOVE(FIGHTING(d->character)) + GET_MOVE_REGEN(FIGHTING(d->character)), (int)GET_MAX_MOVE(FIGHTING(d->character)));
    }
  }
}
