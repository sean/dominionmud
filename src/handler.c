/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  HANDLER.C                                    Based on CircleMUD *
*  Usage: Internal Funcs: moving and finding chars/objs                   *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __HANDLER_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/* external vars */
extern long   top_of_world;
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct race_data *races;
extern char *MENU;
extern char *ANSI_MENU;
extern int NUM_RACES;

/* external functions */
void free_char(struct char_data * ch);
void stop_fighting(struct char_data * ch);
void remove_follower(struct char_data * ch);
void clearMemory(struct char_data * ch);
void check_falling(struct char_data * ch);
void check_falling_obj(struct obj_data * obj);
ACMD(do_infobar);
ACMD(do_return);
void die_follower(struct char_data * ch);
ACMD(do_look);
void death_cry(struct char_data *ch);
void raw_kill(struct char_data *ch, struct char_data *k);

/**
 */
char *fname(char *namelist)
{
  static char holder[30];
  register char *point;

  for (point = holder; isalpha((int)*namelist); namelist++, point++)
    *point = *namelist;

  *point = '\0';

  return (holder);
}

/**
 */
int isname(char *str, char *namelist)
{
  register char *curname, *curstr;

  curname = namelist;
  for (;;) {
    for (curstr = str;; curstr++, curname++) {
      if (!*curstr && !isalpha((int)*curname))
	return (1);

      if (!*curname)
	return (0);

      if (!*curstr || *curname == ' ')
	break;

      if (LOWER(*curstr) != LOWER(*curname))
	break;
    }

    /* skip to next name */

    for (; isalpha((int)*curname); curname++);
    if (!*curname)
      return (0);
    curname++;                  /* first char of new name */
  }
}

/**
 */
void affect_modify(struct char_data * ch, byte loc, sbyte mod, long bitv,
			bool add, bool harmful)
{
  int i = 0;

#if 0
  if (loc != APPLY_AFFECTS) {
    if (add) {
      if (harmful == TRUE)
	SET_BIT(AFF_FLAGS2(ch), bitv);
      else
	SET_BIT(AFF_FLAGS(ch), bitv);
    } else {
      if (harmful == TRUE)
	REMOVE_BIT(AFF_FLAGS2(ch), bitv);
      else
	REMOVE_BIT(AFF_FLAGS(ch), bitv);
      mod = -mod;
    }
  }
#endif
  
  if (add) {
    if (harmful == TRUE)
      SET_BIT(AFF2_FLAGS(ch), bitv);
    else
      SET_BIT(AFF_FLAGS(ch), bitv);
  } else {
    if (harmful == TRUE)
      REMOVE_BIT(AFF2_FLAGS(ch), bitv);
    else
      REMOVE_BIT(AFF_FLAGS(ch), bitv);
    mod = -mod;
  }

  switch (loc) {
    case APPLY_NONE:
      break;
    case APPLY_STR:
      GET_STR(ch) = MAX(3, MIN(25, (int)mod + GET_STR(ch)));
      break;
    case APPLY_DEX:
      GET_DEX(ch) = MAX(3, MIN(25, (int)mod + GET_DEX(ch)));
      break;
    case APPLY_INT:
      GET_INT(ch) = MAX(3, MIN(25, (int)mod + GET_INT(ch)));
      break;
    case APPLY_WIS:
      GET_WIS(ch) = MAX(3, MIN(25, (int)mod + GET_WIS(ch)));
      break;
    case APPLY_CON:
      GET_CON(ch) = MAX(3, MIN(25, (int)mod + GET_CON(ch)));
      break;
    case APPLY_CHA:
      GET_CHA(ch) = MAX(3, MIN(25, (int)mod + GET_CHA(ch)));
      break;
    case APPLY_WILL:
      GET_WILL(ch) = MAX(3, MIN(25, (int)mod + GET_WILL(ch)));
      break;
    case APPLY_AGE:
      ch->player.time.birth -= (mod * SECS_PER_MUD_YEAR);
      break;
    case APPLY_CHAR_WEIGHT:
      GET_WEIGHT(ch) += mod;
      break;
    case APPLY_CHAR_HEIGHT:
      GET_HEIGHT(ch) += mod;
      break;
    case APPLY_MANA:
      GET_MAX_MANA(ch) += mod;
      break;
    case APPLY_HIT:
      GET_MAX_HIT(ch) += mod;
      break;
    case APPLY_MOVE:
      GET_MAX_MOVE(ch) += mod;
      break;
    case APPLY_HITROLL:
      GET_HITROLL(ch) = MIN(100, MAX(1, (int)(GET_HITROLL(ch) + mod)));
      break;
    case APPLY_DAMROLL:
      GET_DAMROLL(ch) = MIN(100, MAX(1, (int)(GET_DAMROLL(ch) + mod)));
      break;
    case APPLY_AC:
      for (i = 0; i < ARMOR_LIMIT; ++i)
	ch->points.armor[i] = MIN(100, MAX(1, (int)GET_AC(ch, i) + mod));
      break;
    case APPLY_GOLD:
    case APPLY_EXP:
      /* NADA */
      break;
    case APPLY_SAVING_PARA:
      GET_SAVE(ch, SAVING_PARA) += mod;
      break;
    case APPLY_SAVING_ROD:
      GET_SAVE(ch, SAVING_ROD) += mod;
      break;
    case APPLY_SAVING_PETRI:
      GET_SAVE(ch, SAVING_PETRI) += mod;
      break;
    case APPLY_SAVING_BREATH:
      GET_SAVE(ch, SAVING_BREATH) += mod;
      break;
    case APPLY_SAVING_SPELL:
      GET_SAVE(ch, SAVING_SPELL) += mod;
      break;
    case APPLY_SAVING_COLD:
      GET_SAVE(ch, SAVING_COLD) += mod;
      break;
    case APPLY_SAVING_FIRE:
      GET_SAVE(ch, SAVING_FIRE) += mod;
      break;
    case APPLY_SAVING_ACID:
      GET_SAVE(ch, SAVING_ACID) += mod;
      break;
    case APPLY_SAVING_ELEC:
      GET_SAVE(ch, SAVING_ELEC) += mod;
      break;
    case APPLY_AFFECTS:
      if (mod < 0)
	mod = -mod;
      if (mod < 1 || mod > 12)
	break;
      switch (mod) {
	case 1:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_SANCTUARY);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_SANCTUARY);
	  break;
	case 2:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_DETECT_INVIS);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_DETECT_INVIS);
	  break;
	case 3:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_DETECT_ALIGN);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_DETECT_ALIGN);
	  break;
	case 4:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_DETECT_MAGIC);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_DETECT_MAGIC);
	  break;
	case 5:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_INFRA);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_INFRA);
	  break;
	case 6:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_SENSE_LIFE);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_SENSE_LIFE);
	  break;
	case 7:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_WATERWALK);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_WATERWALK);
	  break;
	case 8:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_WATERBREATH);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_WATERBREATH);
	  break;
	case 9:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_HASTE);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_HASTE);
	  break;
	case 10:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_INVIS);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_INVIS);
	  break;
	case 11:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_FLYING);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_FLYING);
	  break;
	case 12:
	  if (!add)
	    REMOVE_BIT(ITEM_AFF(ch), AFFECTED_PEEKING);
	  else
	    SET_BIT(ITEM_AFF(ch), AFFECTED_PEEKING);
	  break;
	default:
	  send_to_char("This object affect is unused at this time, please bring this\r\n"
		     "object to the attention of the nearest god, so that it can be corrected.\r\n", ch);
	  break;
      }
      break;
    case APPLY_HIT_REGEN:
      GET_HIT_REGEN(ch) += mod;
      break;
    case APPLY_MANA_REGEN:
      GET_MANA_REGEN(ch) += mod;
      break;
    case APPLY_MOVE_REGEN:
      GET_MOVE_REGEN(ch) += mod;
      break;
    default:
      sprintf(buf, "Apply type: %s (ch) %d (loc) %d (mod)\r\n", GET_NAME(ch), loc, mod);
      log("SYSERR: Unknown apply adjust attempt (handler.c, affect_modify).");
      log(buf);
      break;
  } /* switch */
}

/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data * ch)
{
  struct affected_type *af;
  int i, j;

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, FALSE, FALSE);
  }

  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE, FALSE);
  for (af = ch->affected2; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE, TRUE);

  ch->aff_abils = ch->real_abils;

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, TRUE, FALSE);
  }
  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE, FALSE);
  for (af = ch->affected2; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE, TRUE);

  /* Make certain values are between 0..25, not < 0 and not > 25! */
  GET_STR(ch)  = MAX(2, MIN((int)GET_STR(ch), 25));
  GET_DEX(ch)  = MAX(2, MIN((int)GET_DEX(ch), 25));
  GET_INT(ch)  = MAX(2, MIN((int)GET_INT(ch), 25));
  GET_WIS(ch)  = MAX(2, MIN((int)GET_WIS(ch), 25));
  GET_CON(ch)  = MAX(2, MIN((int)GET_CON(ch), 25));
  GET_CHA(ch)  = MAX(2, MIN((int)GET_CHA(ch), 25));
  GET_WILL(ch) = MAX(2, MIN((int)GET_WILL(ch), 25));

  if (GET_STR(ch) == 18)
    GET_ADD(ch) = MIN(number(1, 100), 100);

  /* Added for TD to make sure that the races ALWAYS have their spec abils */
  if ( !IS_NPC( ch ) && RACE_HAS_INFRA( ch ) && !IS_AFFECTED(ch, AFF_INFRAVISION) )
    SET_BIT(ch->char_specials.saved.affected_by, AFF_INFRAVISION);
  
  if ( !IS_NPC( ch ) && IS_MINOTAUR( ch ) && !IS_AFFECTED(ch, AFF_SENSE_LIFE) )
    SET_BIT(ch->char_specials.saved.affected_by, AFF_SENSE_LIFE);
  
  if ( !IS_NPC( ch ) && (GET_LEVEL(ch) >= 5) && IS_SET(PLR_FLAGS(ch), PLR_NEWBIE))
    REMOVE_BIT(PLR_FLAGS(ch), PLR_NEWBIE);
  if ( !IS_NPC( ch ) && (GET_POS(ch) == POS_FLYING) && (!IS_AFFECTED(ch, AFF_FLYING)))
    GET_POS(ch) = POS_STANDING;
}



/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(struct char_data * ch, struct affected_type * af, bool harmful)
{
  struct affected_type *affected_alloc;

  CREATE(affected_alloc, struct affected_type, 1);

  if (harmful == FALSE) {
    *affected_alloc      = *af;
    affected_alloc->next = ch->affected;
    ch->affected         = affected_alloc;
  } else {
    *affected_alloc      = *af;
    affected_alloc->next = ch->affected2;
    ch->affected2        = affected_alloc;
  }
  affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE, harmful);
  affect_total(ch);
}



/*
 * Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL!  Frees mem and calls
 * affect_location_apply
 */
void affect_remove(struct char_data * ch, struct affected_type * af, bool harmful)
{
  struct affected_type *temp;

  if (harmful == FALSE) {
    if (ch->affected == NULL)
      return;
  } else {
    if (ch->affected2 == NULL)
      return;
  }

  affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE, harmful);

  if (harmful == FALSE) {
    REMOVE_FROM_LIST(af, ch->affected, next);
  } else {
    REMOVE_FROM_LIST(af, ch->affected2, next);
  }
  free(af);
  affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char(struct char_data * ch, sh_int type, bool harmful)
{
  struct affected_type *hjp;

  if (harmful == FALSE) {
     for (hjp = ch->affected; hjp; hjp = hjp->next) {
       if (hjp->type == type)
	 affect_remove(ch, hjp, harmful);
     }
  } else { /* TRUE */
     for (hjp = ch->affected2; hjp; hjp = hjp->next) {
       if (hjp->type == type)
	 affect_remove(ch, hjp, harmful);
     }
  }
}



/*
 * Return if a char is affected by a spell (SPELL_XXX), NULL indicates
 * not affected
 */
bool affected_by_spell(struct char_data * ch, sh_int type, bool harmful)
{
  struct affected_type *hjp;

  for (hjp = (harmful == TRUE ? ch->affected2 : ch->affected); hjp; hjp = hjp->next) {
    if (hjp->type == type)
      return TRUE;
  }
  return FALSE;
}



void affect_join(struct char_data * ch, struct affected_type * af,
      bool add_dur, bool avg_dur, bool add_mod, bool avg_mod, bool harmful)
{
  struct affected_type *hjp;
  bool   found = FALSE;

  if (harmful == FALSE) {
    for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
      if ((hjp->type == af->type) && (hjp->location == af->location)) {
	if (add_dur)
	  af->duration += hjp->duration;
	if (avg_dur)
	  af->duration >>= 1;
	if (add_mod)
	  af->modifier += hjp->modifier;
	if (avg_mod)
	  af->modifier >>= 1;
	affect_remove(ch, hjp, FALSE);
	affect_to_char(ch, af, FALSE);
	found = TRUE;
      }
    }
  } else {
    for (hjp = ch->affected2; !found && hjp; hjp = hjp->next) {
      if ((hjp->type == af->type) && (hjp->location == af->location)) {
	if (add_dur)
	  af->duration += hjp->duration;
	if (avg_dur)
	  af->duration >>= 1;
	if (add_mod)
	  af->modifier += hjp->modifier;
	if (avg_mod)
	  af->modifier >>= 1;
	affect_remove(ch, hjp, TRUE);
	affect_to_char(ch, af, TRUE);
	found = TRUE;
      }
    }
  }
  if (!found)
    affect_to_char(ch, af, harmful);
}


/* move a player out of a room */
void char_from_room(struct char_data * ch)
{
  struct char_data *temp;

  if (ch == NULL || ch->in_room == NOWHERE) {
    log("SYSERR: NULL or NOWHERE in handler.c, char_from_room");
    exit(1);
  }

  if (FIGHTING(ch) != NULL)
    stop_fighting(ch);

  if (GET_EQ(ch, WEAR_HOLD) != NULL) {
    if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_LIGHT)
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 2))    /* Light is ON */
	world[ch->in_room].light--;
  }

  REMOVE_FROM_LIST(ch, world[ch->in_room].people, next_in_room);
  ch->in_room = NOWHERE;
  ch->next_in_room = NULL;
}


/* place a character in a room */
void char_to_room(struct char_data * ch, long room)
{
  if (!ch || room < 0 || room > top_of_world) {
    log("SYSERR: Illegal value(s) passed to char_to_room");
  } else {
    ch->next_in_room = world[room].people;
    world[room].people = ch;
    ch->in_room = room;
  }
  if (GET_EQ(ch, WEAR_HOLD) != NULL) {
    if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_LIGHT)
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 2))   /*  Light is ON */
	world[ch->in_room].light++;
  }
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_INFOBAR))
    do_infobar(ch, 0, 0, SCMDB_NEWROOM);

  check_falling(ch);
}


/* give an object to a char   */
void obj_to_char(struct obj_data * object, struct char_data * ch)
{
  if (object && ch) {
    object->next_content = ch->carrying;
    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    IS_CARRYING_WEIGHT(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;

    /* set flag for crash-save system */
    SET_BIT(PLR_FLAGS(ch), PLR_CRASH);
    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_INFOBAR))
      do_infobar(ch, 0, 0, SCMDB_GET);
  } else
    log("SYSERR: NULL obj or char passed to obj_to_char");
}


/* take an object from a char */
void obj_from_char(struct obj_data * object)
{
  struct obj_data *temp;

  if (object == NULL) {
    log("SYSERR: NULL object passed to obj_from_char");
    return;
  }
  REMOVE_FROM_LIST(object, object->carried_by->carrying, next_content);

  /* set flag for crash-save system */
  SET_BIT(PLR_FLAGS(object->carried_by), PLR_CRASH);

  IS_CARRYING_WEIGHT(object->carried_by) -= GET_OBJ_WEIGHT(object);
  IS_CARRYING_N(object->carried_by)--;

  if (!IS_NPC(object->carried_by) && PRF_FLAGGED(object->carried_by, PRF_INFOBAR))
    do_infobar(object->carried_by, 0, 0, SCMDB_GET);
  object->carried_by = NULL;
  object->next_content = NULL;
}



/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data * ch, int eq_pos)
{
  assert(GET_EQ(ch, eq_pos));

  if (!(GET_OBJ_TYPE(GET_EQ(ch, eq_pos)) == ITEM_ARMOR))
    return 0;

  switch (eq_pos) {

  case WEAR_BODY:
    ch->points.armor[ARMOR_BODY] += GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0);
    break;
  case WEAR_HEAD:
    ch->points.armor[ARMOR_HEAD] += GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0);
    break;
  case WEAR_LEGS:
  case WEAR_FOOT_L:
  case WEAR_FOOT_R:
    ch->points.armor[ARMOR_LEG_L] += (GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0)/2);
    ch->points.armor[ARMOR_LEG_R] += (GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0)/2);
    break;
  case WEAR_ARM_L:
  case WEAR_HAND_L:
    ch->points.armor[ARMOR_ARM_L] += (GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0)/2);
    break;
  case WEAR_ARM_R:
  case WEAR_HAND_R:
    ch->points.armor[ARMOR_ARM_R] += (GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0)/2);
    break;
  case WEAR_HOLD:
  case WEAR_WIELD:
    ch->points.armor[ARMOR_SHIELD] += (GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0)/2);
    break;
  default:
    break;
  }

  return 1;
}


void equip_char(struct char_data * ch, struct obj_data * obj, int pos)
{
  int j;

  assert(pos >= 0 && pos < NUM_WEARS);

  if (GET_EQ(ch, pos)) {
    sprintf(buf, "SYSERR: Char is already equipped: %s, %s", GET_NAME(ch),
	    obj->short_description);
    log(buf);
    return;
  }
  if (obj->carried_by) {
    log("SYSERR: EQUIP: Obj is carried_by when equip.");
    return;
  }
  if (obj->in_room != NOWHERE) {
    log("SYSERR: EQUIP: Obj is in_room when equip.");
    return;
  }
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
    //    || invalid_race(ch, obj)) {
    act("You are zapped by $p and instantly let go of it.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n is zapped by $p and instantly lets go of it.", FALSE, ch, obj, 0, TO_ROOM);
    obj_to_char(obj, ch);     /* changed to drop in inventory instead of
                               * ground */
    return;
  }

  GET_EQ(ch, pos) = obj;
  obj->worn_by = ch;
  obj->worn_on = pos;

  if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    apply_ac(ch, pos);

  if (ch->in_room != NOWHERE) {
    if (pos == WEAR_HOLD && GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      if (GET_OBJ_VAL(obj, 2))  /* if light is ON */
	world[ch->in_room].light++;
  }
#if 0
 else
    log("SYSERR: ch->in_room = NOWHERE when equipping char.");
#endif
  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, TRUE, FALSE);
  affect_total(ch);
}



struct obj_data *unequip_char(struct char_data * ch, int pos)
{
  int j;
  struct obj_data *obj;

  assert(pos >= 0 && pos < NUM_WEARS);
  assert(GET_EQ(ch, pos));

  obj = GET_EQ(ch, pos);
  obj->worn_by = NULL;
  obj->worn_on = -1;

  if (GET_OBJ_TYPE(obj) == ITEM_ARMOR) {
     switch (pos) {
       case WEAR_BODY:
	 ch->points.armor[ARMOR_BODY] -= GET_OBJ_VAL(GET_EQ(ch, pos), 0);
	 break;
       case WEAR_HEAD:
	 ch->points.armor[ARMOR_HEAD] -= GET_OBJ_VAL(GET_EQ(ch, pos), 0);
	 break;
       case WEAR_LEGS:
       case WEAR_FOOT_L:
       case WEAR_FOOT_R:
	 ch->points.armor[ARMOR_LEG_L] -= (GET_OBJ_VAL(GET_EQ(ch, pos), 0)/2);
	 ch->points.armor[ARMOR_LEG_R] -= (GET_OBJ_VAL(GET_EQ(ch, pos), 0)/2);
	 break;
       case WEAR_ARM_L:
       case WEAR_HAND_L:
	 ch->points.armor[ARMOR_ARM_L] -= (GET_OBJ_VAL(GET_EQ(ch, pos), 0)/2);
	 break;
       case WEAR_ARM_R:
       case WEAR_HAND_R:
	 ch->points.armor[ARMOR_ARM_R] -= (GET_OBJ_VAL(GET_EQ(ch, pos), 0)/2);
	 break;
       case WEAR_HOLD:
       case WEAR_WIELD:
	 ch->points.armor[ARMOR_SHIELD] -= (GET_OBJ_VAL(GET_EQ(ch, pos), 0)/2);
	 break;
       default:
	 break;
     }
  }

  if (ch->in_room != NOWHERE) {
    if (pos == WEAR_HOLD && GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      if (GET_OBJ_VAL(obj, 2))  /* if light is ON */
	world[ch->in_room].light--;
  } 
#if 0
else
    log("SYSERR: ch->in_room = NOWHERE when unequipping char.");
#endif
  GET_EQ(ch, pos) = NULL;

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, FALSE, FALSE);

  affect_total(ch);

  return (obj);
}


int get_number(char **name)
{
  int i;
  char *ppos;
  char number[MAX_INPUT_LENGTH];

  *number = '\0';

  if ((ppos = strchr(*name, '.'))) {
    *ppos++ = '\0';
    strcpy(number, *name);
    strcpy(*name, ppos);

    for (i = 0; *(number + i); i++)
      if (!isdigit((int)*(number + i)))
	return 0;

    return (atoi(number));
  }
  return 1;
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(long num, struct obj_data * list)
{
  struct obj_data *i;

  for (i = list; i; i = i->next_content)
    if (GET_OBJ_RNUM(i) == num)
      return i;

  return NULL;
}


int find_lodged_obj(struct char_data *ch, char *arg)
{
  int i;

  for (i = 0; i < 12; i++) {
    if (GET_ITEMS_STUCK(ch, i) != NULL)
      if (isname(arg, (GET_ITEMS_STUCK(ch, i))->name) ||
  	  is_abbrev(arg, (GET_ITEMS_STUCK(ch, i))->name))
        return i;
  }
  return -1;
}

/* search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(long nr)
{
  struct obj_data *i;

  for (i = object_list; i; i = i->next)
    if (GET_OBJ_RNUM(i) == nr)
      return i;

  return NULL;
}



/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, long room)
{
  struct char_data *i;
  int j = 0, number = -1;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = world[room].people; i && (j <= number); i = i->next_in_room)
     if (isname(tmp, i->player.name) || is_abbrev(tmp, i->player.name))
      if (++j == number)
	return i;

  return NULL;
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(long nr)
{
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    if (GET_MOB_RNUM(i) == nr)
      return i;

  return NULL;
}



/* put an object in a room */
void obj_to_room(struct obj_data * object, long room)
{
  if (!object || room < 0 || room > top_of_world) {
    log("SYSERR: Illegal value(s) passed to obj_to_room");
  } else {
    object->next_content = world[room].contents;
    world[room].contents = object;
    object->in_room = room;
    object->carried_by = NULL;
    if (ROOM_FLAGGED(room, ROOM_HOUSE))
      SET_BIT(ROOM_FLAGS(room), ROOM_HOUSE_CRASH);
    check_falling_obj(object);
  }
}


/* Take an object from a room */
void obj_from_room(struct obj_data * object)
{
  struct obj_data *temp;

  if (!object || object->in_room == NOWHERE) {
    log("SYSERR: NULL object or obj not in a room passed to obj_from_room");
    return;
  }

  REMOVE_FROM_LIST(object, world[object->in_room].contents, next_content);

  if (ROOM_FLAGGED(object->in_room, ROOM_HOUSE))
    SET_BIT(ROOM_FLAGS(object->in_room), ROOM_HOUSE_CRASH);
  object->in_room = NOWHERE;
  object->next_content = NULL;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data * obj, struct obj_data * obj_to)
{
  struct obj_data *tmp_obj;

  if (!obj || !obj_to || obj == obj_to) {
#if 0
    log("SYSERR: NULL object or same source and target obj passed to obj_to_obj");
#endif
    return;
  }

  obj->next_content = obj_to->contains;
  obj_to->contains = obj;
  obj->in_obj = obj_to;

  for (tmp_obj = obj->in_obj; tmp_obj->in_obj; tmp_obj = tmp_obj->in_obj)
    GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);

  /* top level object.  Subtract weight from inventory if necessary. */
  GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
  if (tmp_obj->carried_by)
    IS_CARRYING_WEIGHT(tmp_obj->carried_by) += GET_OBJ_WEIGHT(obj);
}


/* remove an object from an object */
void obj_from_obj(struct obj_data * obj)
{
  struct obj_data *temp, *obj_from;

  if (obj->in_obj == NULL) {
    log("error (handler.c): trying to illegally extract obj from obj");
    return;
  }
  obj_from = obj->in_obj;
  REMOVE_FROM_LIST(obj, obj_from->contains, next_content);

  /* Subtract weight from containers container */
  for (temp = obj->in_obj; temp->in_obj; temp = temp->in_obj)
    GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);

  /* Subtract weight from char that carries the object */
  GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);
  if (temp->carried_by)
    IS_CARRYING_WEIGHT(temp->carried_by) -= GET_OBJ_WEIGHT(obj);

  obj->in_obj = NULL;
  obj->next_content = NULL;
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data * list, struct char_data * ch)
{
  if (list) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}


/* Extract an object from the world */
void extract_obj(struct obj_data * obj)
{
  struct obj_data *temp;

  if (obj->worn_by != NULL)
    if (unequip_char(obj->worn_by, obj->worn_on) != obj)
      log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
  if (obj->in_room != NOWHERE)
    obj_from_room(obj);
  else if (obj->carried_by)
    obj_from_char(obj);
  else if (obj->in_obj)
    obj_from_obj(obj);

  /* Get rid of the contents of the object, as well. */
  while (obj->contains)
    extract_obj(obj->contains);

  REMOVE_FROM_LIST(obj, object_list, next);

  if (GET_OBJ_RNUM(obj) >= 0)
    (obj_index[GET_OBJ_RNUM(obj)].number)--;
  free_obj(obj);
}



void update_object(struct obj_data * obj, int use)
{
  if (obj == NULL)
    return;

  if (GET_OBJ_TIMER(obj) > 0)
    GET_OBJ_TIMER(obj) -= use;
  if (GET_OBJ_TYPE(obj) == ITEM_FOOD)
    GET_OBJ_VAL(obj, 9) += 1;
  if (obj->contains)
    update_object(obj->contains, use);
  if (obj->next_content)
    update_object(obj->next_content, use);
}


void update_char_objects(struct char_data * ch)
{
  int i;

  if (GET_EQ(ch, WEAR_HOLD) != NULL) {
    if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_LIGHT)
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 2) > 0) {
	i = --GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 2);
	if (i == 1) {
	  act("Your light begins to flicker and fade.", FALSE, ch, 0, 0, TO_CHAR);
	  act("$n's light begins to flicker and fade.", FALSE, ch, 0, 0, TO_ROOM);
	} else if (i == 0) {
	  act("Your light sputters out and dies.", FALSE, ch, 0, 0, TO_CHAR);
	  act("$n's light sputters out and dies.", FALSE, ch, 0, 0, TO_ROOM);
	  world[ch->in_room].light--;
	}
      }
  }

  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      update_object(GET_EQ(ch, i), 2);

  if (ch->carrying)  /* shouldn't be done for NPCs */
    update_object(ch->carrying, 1);
}

/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data * ch)
{
  struct char_data *k, *temp;
  struct descriptor_data *t_desc;
  struct obj_data *obj;
  int i, freed = 0;

  extern struct char_data *combat_list;

  assert( ch != NULL );
  assert( ch->in_room != NOWHERE );

  if (!IS_NPC(ch) && !ch->desc) {
    for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
      if (t_desc->original == ch)
	do_return(t_desc->character, "", 0, 0);
  }

  if (ch->followers || ch->master)
    die_follower(ch);

  /* Forget snooping, if applicable */
  if (ch->desc) {
    if (ch->desc->snooping) {
      ch->desc->snooping->snoop_by = NULL;
      ch->desc->snooping = NULL;
    }
    if (ch->desc->snoop_by) {
      SEND_TO_Q("Your victim is no longer among us.\r\n",
		ch->desc->snoop_by);
      ch->desc->snoop_by->snooping = NULL;
      ch->desc->snoop_by = NULL;
    }
  }

  /* transfer objects to room, if any */
  while (ch->carrying) {
    obj = ch->carrying;
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
  }

  /* transfer equipment to room, if any */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      obj_to_room(unequip_char(ch, i), ch->in_room);

  if (FIGHTING(ch))
    stop_fighting(ch);

  for (k = combat_list; k; k = temp) {
    temp = k->next_fighting;
    if (FIGHTING(k) == ch)
      stop_fighting(k);
  }

  char_from_room(ch);

  /* pull the char from the list */
  REMOVE_FROM_LIST(ch, character_list, next);

  if (ch->desc && ch->desc->original)
    do_return(ch, "", 0, 0);

  if (!IS_NPC(ch)) {
    save_char(ch, NOWHERE);
    Crash_delete_crashfile(ch);
  } else {
    if (GET_MOB_RNUM(ch) > -1)          /* if mobile */
      mob_index[GET_MOB_RNUM(ch)].number--;
    clearMemory(ch);            /* Only NPC's can have memory */
    free_char(ch);
    freed = 1;
  }

  if (!freed) {
#if 0
    if (GET_FATE_PTS(ch) <= 0 && !IS_IMMORT(ch)) {
      SET_BIT(PLR_FLAGS(ch), PLR_DELETED);
      SEND_TO_Q("Your luck has run out, may you rest in eternal peace.\r\n", ch->desc);
      if (ch->desc)
	STATE(ch->desc) = CON_CLOSE;
    } else
#endif
    if (ch->desc) {
      STATE(ch->desc) = CON_MENU;
      if (GET_ANSI(ch->desc) == TRUE)
	SEND_TO_Q(ANSI_MENU, ch->desc);
      else
	SEND_TO_Q(MENU, ch->desc);
    } else {  
      /* if a player gets purged from within the game */
      free_char(ch);
    }
  }
}



/* ***********************************************************************
* Here follows high-level versions of some earlier routines, ie functions*
* which incorporate the actual player-data                               *.
*********************************************************************** */


struct char_data *get_player_vis(struct char_data * ch, char *name, long inroom)
{
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    if (!IS_NPC(i) && (!inroom || i->in_room == ch->in_room) &&
	(!str_cmp(i->player.name, name) || is_abbrev(name, i->player.name))
	&& CAN_SEE(ch, i))
      return i;

  return NULL;
}


struct char_data *get_char_room_vis(struct char_data * ch, char *name)
{
  struct char_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* JE 7/18/94 :-) :-) */
  if (!str_cmp(name, "self") || !str_cmp(name, "me"))
    return ch;

  /* 0.<name> means PC with name */
  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return get_player_vis(ch, tmp, 1);

  for (i = world[ch->in_room].people; i && j <= number; i = i->next_in_room)
     if (isname(tmp, i->player.name) || is_abbrev(tmp, i->player.name))
      if (CAN_SEE(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}


struct char_data *get_char_scan_vis(struct char_data * ch, char *name)
{
  struct char_data *i;
  int    j = 0, number = 0, door;
  char   tmpname[MAX_INPUT_LENGTH];
  char   *tmp = tmpname;

  /* JE 7/18/94 :-) :-) */
  if (!str_cmp(name, "self") || !str_cmp(name, "me"))
    return ch;

  /* 0.<name> means PC with name */
  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return get_player_vis(ch, tmp, 1);

  /* First see if the person is in the same room */
  for (i = world[ch->in_room].people; i && j <= number; i = i->next_in_room)
     if (isname(tmp, i->player.name) || is_abbrev(tmp, i->player.name))
      if (CAN_SEE(ch, i))
	if (++j == number)
	  return i;
  /* Now check the surrounding rooms */
  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      if (world[EXIT(ch, door)->to_room].people) {
	j = 0;
	for (i = world[EXIT(ch, door)->to_room].people; i && j <= number; i = i->next_in_room) {
	  if (isname(tmp, i->player.name) || is_abbrev(tmp, i->player.name))
	    if (CAN_SEE(ch, i))
	      if (++j == number)
		return i;
        }
      } else if (_2ND_EXIT(ch, door) && _2ND_EXIT(ch, door)->to_room !=
		 NOWHERE && !IS_SET(_2ND_EXIT(ch, door)->exit_info, EX_CLOSED)) {
	/* check the second room away */
	if (world[_2ND_EXIT(ch, door)->to_room].people) {
	  j = 0;
	  for (i = world[_2ND_EXIT(ch, door)->to_room].people; i && j <= number; i = i->next_in_room)
	    if (isname(tmp, i->player.name) || is_abbrev(tmp, i->player.name))
	      if (CAN_SEE(ch, i))
		if (++j == number)
		  return i;
	} else if (_3RD_EXIT(ch, door) && _3RD_EXIT(ch, door)->to_room !=
		   NOWHERE && !IS_SET(_3RD_EXIT(ch, door)->exit_info, EX_CLOSED)) {
	  /* check the third room */
	  if (world[_3RD_EXIT(ch, door)->to_room].people) {
	    j = 0;
	    for (i = world[_3RD_EXIT(ch, door)->to_room].people; i && j <= number; i = i->next_in_room)
	      if (isname(tmp, i->player.name) || is_abbrev(tmp, i->player.name))
		if (CAN_SEE(ch, i))
		  if (++j == number)
		    return i;
	  } else if (_4TH_EXIT(ch, door) && _4TH_EXIT(ch, door)->to_room !=
		   NOWHERE && !IS_SET(_4TH_EXIT(ch, door)->exit_info, EX_CLOSED)) {
	    /* check the third room */
	    if (world[_4TH_EXIT(ch, door)->to_room].people) {
	      j = 0;
	      for (i = world[_4TH_EXIT(ch, door)->to_room].people; i && j <= number; i = i->next_in_room)
		if (isname(tmp, i->player.name) || is_abbrev(tmp, i->player.name))
		  if (CAN_SEE(ch, i))
		    if (++j == number)
		      return i;
	    } else if (_5TH_EXIT(ch, door) && _5TH_EXIT(ch, door)->to_room !=
		   NOWHERE && !IS_SET(_5TH_EXIT(ch, door)->exit_info, EX_CLOSED)) {
	      /* check the third room */
	      if (world[_5TH_EXIT(ch, door)->to_room].people) {
		j = 0;
		for (i = world[_5TH_EXIT(ch, door)->to_room].people; i && j <= number; i = i->next_in_room)
		  if (isname(tmp, i->player.name) || is_abbrev(tmp, i->player.name))
		    if (CAN_SEE(ch, i))
		      if (++j == number)
			return i;
	      }
	    }
	  }
	}
      }
    }
  }
  return NULL;
}


struct char_data *get_char_vis(struct char_data * ch, char *name)
{
  struct char_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* check the room first */
  if ((i = get_char_room_vis(ch, name)) != NULL)
    return i;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return get_player_vis(ch, tmp, 0);

  for (i = character_list; i && (j <= number); i = i->next)
     if ( (isname(tmp, i->player.name) ||
	  is_abbrev(tmp, i->player.name)) && CAN_SEE(ch, i))
       if (++j == number)
	 return i;

  return NULL;
}




struct char_data *get_char(char *name)
{
  struct char_data *i;
  int j = 0, number = -1;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);

  for (i = character_list; i && (j <= number); i = i->next)
     if (isname(tmp, i->player.name) ||
	 is_abbrev(tmp, i->player.name))
      if (++j == number)
	return i;

  return NULL;
}



struct obj_data *get_obj_in_list_vis(struct char_data * ch, char *name,
					      struct obj_data * list)
{
  struct obj_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = list; i && (j <= number); i = i->next_content)
     if (isname(tmp, i->name) || is_abbrev(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}




/* search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data * ch, char *name)
{
  struct obj_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* scan items carried */
  if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
    return i;

  /* scan room */
  if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
    return i;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list; i && (j <= number); i = i->next)
     if (isname(tmp, i->name) || is_abbrev(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}



struct obj_data *get_object_in_equip_vis(struct char_data * ch,
			   char *arg, struct obj_data * equipment[], int *j)
{
  for ((*j) = 0; (*j) < NUM_WEARS; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch, equipment[(*j)]))
	if (isname(arg, equipment[(*j)]->name) || 
	    is_abbrev(arg, equipment[(*j)]->name))
	  return (equipment[(*j)]);

  return NULL;
}


char *money_desc(long amount)
{
  static char buf[128];

  if (amount <= 0) {
    log("SYSERR: Try to create negative or 0 money.");
    return NULL;
  }
  if (amount == 1)
    strcpy(buf, "a gold coin");
  else if (amount <= 10)
    strcpy(buf, "a tiny pile of gold coins");
  else if (amount <= 20)
    strcpy(buf, "a handful of gold coins");
  else if (amount <= 75)
    strcpy(buf, "a little pile of gold coins");
  else if (amount <= 200)
    strcpy(buf, "a small pile of gold coins");
  else if (amount <= 1000)
    strcpy(buf, "a pile of gold coins");
  else if (amount <= 5000)
    strcpy(buf, "a big pile of gold coins");
  else if (amount <= 10000)
    strcpy(buf, "a large heap of gold coins");
  else if (amount <= 20000)
    strcpy(buf, "a huge mound of gold coins");
  else if (amount <= 75000)
    strcpy(buf, "an enormous mound of gold coins");
  else if (amount <= 150000)
    strcpy(buf, "a small mountain of gold coins");
  else if (amount <= 250000)
    strcpy(buf, "a mountain of gold coins");
  else if (amount <= 500000)
    strcpy(buf, "a huge mountain of gold coins");
  else if (amount <= 1000000)
    strcpy(buf, "an enormous mountain of gold coins");
  else
    strcpy(buf, "an absolutely colossal mountain of gold coins");

  return buf;
}


struct obj_data *create_money(long amount)
{
  struct obj_data *obj;
  struct extra_descr_data *new_descr;
  char buf[200];

  if (amount <= 0) {
    log("SYSERR: Try to create negative or 0 money.");
    return NULL;
  }
  obj = create_obj();
  CREATE(new_descr, struct extra_descr_data, 1);

  if (amount == 1) {
    obj->name = str_dup("coin gold");
    obj->short_description = str_dup("a gold coin");
    obj->description = str_dup("One miserable gold coin is lying here.");
    new_descr->keyword = str_dup("coin gold");
    new_descr->description = str_dup("It's just one miserable little gold coin.");
  } else {
    obj->name = str_dup("coins gold");
    obj->short_description = str_dup(money_desc(amount));
    sprintf(buf, "%s is lying here.", money_desc(amount));
    obj->description = str_dup(CAP(buf));

    new_descr->keyword = str_dup("coins gold");
    if (amount < 10) {
      sprintf(buf, "There are %ld coins.", amount);
      new_descr->description = str_dup(buf);
    } else if (amount < 100) {
      sprintf(buf, "There are about %ld coins.", 10 * (amount / 10));
      new_descr->description = str_dup(buf);
    } else if (amount < 1000) {
      sprintf(buf, "It looks to be about %ld coins.", 100 * (amount / 100));
      new_descr->description = str_dup(buf);
    } else if (amount < 100000) {
      sprintf(buf, "You guess there are, maybe, %ld coins.",
	      1000 * ((amount / 1000) + number(0, (amount / 1000))));
      new_descr->description = str_dup(buf);
    } else
      new_descr->description = str_dup("There are a LOT of coins.");
  }

  new_descr->next = NULL;
  obj->ex_description = new_descr;

  GET_OBJ_TYPE(obj)   = ITEM_MONEY;
  GET_OBJ_WEAR(obj)   = ITEM_WEAR_TAKE;
  GET_OBJ_VAL(obj, 0) = amount;
  GET_OBJ_COST(obj)   = amount;
  obj->item_number    = NOTHING;

  return obj;
}


/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, struct char_data * ch,
		     struct char_data ** tar_ch, struct obj_data ** tar_obj)
{
  int i, found;
  char name[256];

  one_argument(arg, name);

  if (!*name)
    return (0);

  *tar_ch = NULL;
  *tar_obj = NULL;

  if (IS_SET(bitvector, FIND_CHAR_ROOM)) {      /* Find person in room */
    if ((*tar_ch = get_char_room_vis(ch, name))) {
      return (FIND_CHAR_ROOM);
    }
  }
  if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
    if ((*tar_ch = get_char_vis(ch, name))) {
      return (FIND_CHAR_WORLD);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_INV)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
      return (FIND_OBJ_INV);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
    for (found = FALSE, i = 0; i < NUM_WEARS && !found; i++)
#if 0
      if (GET_EQ(ch, i) && isname(name, GET_EQ(ch, i)->name) == 0) {
#endif
      if (GET_EQ(ch, i) && (isname(name, GET_EQ(ch, i)->name) ||
          is_abbrev(name, GET_EQ(ch, i)->name))) {
	*tar_obj = GET_EQ(ch, i);
	found = TRUE;
      }
    if (found) {
      return (FIND_OBJ_EQUIP);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents))) {
      return (FIND_OBJ_ROOM);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
    if ((*tar_obj = get_obj_vis(ch, name))) {
      return (FIND_OBJ_WORLD);
    }
  }
  return (0);
}


/* a function to scan for "all" or "all.x" */
int find_all_dots(char *arg)
{
  if (!strcmp(arg, "all"))
    return FIND_ALL;
  else if (!strncmp(arg, "all.", 4)) {
    strcpy(arg, arg + 4);
    return FIND_ALLDOT;
  } else
    return FIND_INDIV;
}


void check_falling(struct char_data * ch)
{
   long targ;

   if ((IS_IMMORTAL(ch)) || (IS_AFFECTED2(ch, AFF_FLYING)))
     return;

   if (ch->in_room == NOWHERE)
     return;

   if (SECT(ch->in_room) != SECT_FLYING)
     return;

   act("The world spins, and you sky-dive out of control",
      TRUE, ch, 0, 0, TO_CHAR);

   if (EXIT(ch, DOWN) && EXIT(ch, DOWN)->to_room > -1) {
     targ = real_room(EXIT(ch, DOWN)->to_room);
     if (targ > 0) {
       act("$n plunges towards oblivion.", FALSE, ch, 0, 0, TO_ROOM);
       send_to_char("You plunge from the sky.\n\r", ch);
       char_from_room(ch);
       char_to_room(ch, targ);
       act("$n falls from the sky.", FALSE, ch, 0, 0, TO_ROOM);
       do_look(ch, "", 0, 0);
       damage(ch, ch, dice(10, 5), TYPE_UNDEFINED, -1);
       if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_DEATH)) {
	 log_death_trap(ch);
	 death_cry(ch);
	 extract_char(ch);
	 return;
       }
     }
   } else {
     if (GET_HIT(ch) <= GET_MAX_HIT(ch) >> 5) {
       send_to_char("You are smashed into tiny pieces.\n\r", ch);
       if ((SECT(ch->in_room) == SECT_WATER_SWIM) ||
	   (SECT(ch->in_room) == SECT_WATER_NOSWIM))
	 act("$n is smashed to a pulp by $s impact with the water",
	    FALSE, ch, 0, 0, TO_ROOM);
       else
	 act("$n is smashed to a bloody pulp by $s impact with the ground",
	    FALSE, ch, 0, 0, TO_ROOM);
       act("You are drenched with blood and gore.",
	  FALSE, ch, 0, 0, TO_ROOM);
       GET_HIT(ch) = 0;
       raw_kill(ch, NULL);
       sprintf(buf, "%s has fallen to death.", GET_NAME(ch));
       log(buf);
     } else {
       send_to_char("You land with a resounding THUMP!\n\r", ch);
       GET_HIT(ch) = 1;
       GET_POS(ch) = POS_STUNNED;
       if ((SECT(ch->in_room) == SECT_WATER_SWIM) ||
	   (SECT(ch->in_room) == SECT_WATER_NOSWIM))
	 act("$n drenches you as $e hits the water!",
	    FALSE, ch, 0, 0, TO_ROOM);
       else
	 act("$n lands with a resounding THUMP as $e hits the ground!",
	    FALSE, ch, 0, 0, TO_ROOM);
     }
   }
}

void check_falling_obj(struct obj_data * obj)
{
   long targ, oldroom; 

   if (obj->in_room == NOWHERE)
     return;

   if ((SECT(obj->in_room) != SECT_FLYING) &&
       (SECT(obj->in_room) != SECT_WATER_SWIM) &&
       (SECT(obj->in_room) != SECT_WATER_NOSWIM))
     return;
   /* send the message */
   if (SECT(obj->in_room) == SECT_FLYING)
     act("$p plunges towards oblivion.", FALSE, 0, obj, 0, TO_ROOM);
   else if ((SECT(obj->in_room) == SECT_WATER_SWIM) ||
            (SECT(obj->in_room) == SECT_WATER_NOSWIM))
     act("$p sinks beneath the surface.", FALSE, 0, obj, 0, TO_ROOM);

   /* now let's move or extract the object */
   if (EXIT(obj, DOWN) && EXIT(obj, DOWN)->to_room > -1) {
     targ = real_room(EXIT(obj, DOWN)->to_room);
     oldroom = obj->in_room;
     if (targ > 0) {
       obj_from_room(obj);
       obj_to_room(obj, targ);
       if (SECT(oldroom) == SECT_FLYING) 
         act("$p falls from the sky.", FALSE, 0, obj, 0, TO_ROOM);
       else if ((SECT(oldroom) == SECT_WATER_SWIM) ||
	        (SECT(oldroom) == SECT_WATER_NOSWIM))
         act("$p floats down from above.", FALSE, 0, obj, 0, TO_ROOM);
     } else
       extract_obj(obj);
   } else
     extract_obj(obj);
}

