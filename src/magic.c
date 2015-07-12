/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  MAGIC.C                                      Based on CircleMUD *
*  Usage: Low-level funcs for magic; spell template code                  *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __MAGIC_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *obj_index;

extern struct descriptor_data *descriptor_list;

extern int mini_mud;
extern int pk_allowed;

extern struct default_mobile_stats *mob_defaults;
extern char weapon_verbs[];
extern int *max_ac_applys;
extern struct apply_mod_defaults *apmd;

void clearMemory(struct char_data * ch);
void act(char *str, int i, struct char_data * c, struct obj_data * o,
	      void *vict_obj, int j);

void damage(struct char_data * ch, struct char_data * victim,
		 int damage, int weapontype, int hitloc);

void hit(struct char_data * ch, struct char_data * victim, int type);
void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int  dice(int number, int size);
extern struct spell_info_type spell_info[];

struct char_data *read_mobile(long, int);

/*
 * Saving throws for:
 * HuHaDwGnDrMiCeQuSiKaKeVaWe 13 (DraAniUndGiaHumGulDem)
 *   PARA, ROD, PETRI, BREATH, SPELL, COLD, FIRE, ACID, ELEC
 *     Levels 0-50
 * For The Dominon by Glasgian, needs fixing
 */
byte saving_throws(struct char_data *ch, int save)
{
   switch (save) {
      case 0: /* PARA   */
      case 1: /* ROD    */
      case 2: /* PETRI  */
      case 3: /* BREATH */
      case 4: /* SPELL  */
      case 5: /* COLD   */
      case 6: /* FIRE   */
      case 7: /* ACID   */
      case 8: /* ELEC   */
      default: /* negative of their level - 100 / their level */
	return ((-(GET_LEVEL(ch)-100))/(GET_LEVEL(ch)+1));
	break;
   }
}


int mag_savingthrow(struct char_data * ch, int type)
{
  int save;

  /* negative apply_saving_throw values make saving throws better! */

  save  = saving_throws(ch, type);

  save += GET_SAVE(ch, type);

  /* throwing a 0 is always a failure */
  if (MAX(1, save) < number(0, 99))
    return TRUE;
  else
    return FALSE;
}


/*
 * For blinding those with infravision who are fireballed
 *  or otherwise subjected to lotsa heat quickly in dark rooms.
 * By Glasgian of The Dominion - it works now 3/17/95
 */
void heat_blind(struct char_data *ch)
{
   struct affected_type af;
   byte tmp;

   tmp = number(1, 6);

   if (AFF2_FLAGGED(ch, AFF_BLIND))
     return;                     /* no affect */

   if ((IS_DARK(ch->in_room)) && (GET_LEVEL(ch) < LVL_IMMORT) &&
       ((AFF_FLAGGED(ch, AFF_INFRAVISION)) || (IS_ITEM_AFF(ch, AFFECTED_INFRA))))
   {
     send_to_char("Aaarrrggghhh!!  The heat blinds you!!\n\r", ch);
     af.type      = SPELL_BLIND;
     af.location  = APPLY_HITROLL;
     af.modifier  = -4;  /* Make hitroll worse */
     af.duration  = tmp;
     af.bitvector = AFF_BLIND;
     affect_to_char(ch, &af, TRUE);
     return;
   }
   return;
}


/* affect_update: called from comm.c (causes spells to wear off) */
void affect_update(void)
{
  static struct affected_type *af, *next;
  static struct char_data *i;
  extern char   *spell_wear_off_msg[];

  for (i = character_list; i; i = i->next) {
    /* Harmful effects */
    for (af = i->affected2; af; af = next) {
      next = af->next;
      if (af->duration >= 1)
	af->duration--;
      else if (af->duration == -1)      /* No action */
	af->duration = -1;              /* GODs only! unlimited */
      else {
	if ((af->type > 50) && (af->type <= MAX_SPELLS))
	  if (!af->next || (af->next->type != af->type) ||
	      (af->next->duration > 0))
	    if (*spell_wear_off_msg[af->type]) {
	      send_to_char(spell_wear_off_msg[af->type], i);
	      send_to_char("\r\n", i);
	    }
	    affect_remove(i, af, TRUE);
      }
    }
    /* Good effects */
    for (af = i->affected; af; af = next) {
      next = af->next;
      if (af->duration >= 1)
	af->duration--;
      else if (af->duration == -1)  /* No action */
	af->duration = -1;          /* GODs only! unlimited */
      else {
	if ((af->type > 0) && (af->type <= MAX_SPELLS))
	  if (!af->next || (af->next->type != af->type) ||
	      (af->next->duration > 0))
	    if (*spell_wear_off_msg[af->type]) {
	      send_to_char(spell_wear_off_msg[af->type], i);
	      send_to_char("\r\n", i);
	    }
	    affect_remove(i, af, FALSE);
      }
    }
  }
}


/*
 *  mag_materials:
 *  Checks for up to 3 vnums (spell reagents) in the player's inventory.
 *
 * No spells implemented in Circle 3.0 use mag_materials, but you can use
 * it to implement your own spells which require ingredients (i.e., some
 * heal spell which requires a rare herb or some such.)
 */
int mag_materials(struct char_data * ch, int item0, int item1, int item2,
		      int extract, int verbose)
{
  struct obj_data *tobj;
  struct obj_data *obj0 = NULL, *obj1 = NULL, *obj2 = NULL;

  for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
    if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0)) {
      obj0 = tobj;
      item0 = -1;
    } else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1)) {
      obj1 = tobj;
      item1 = -1;
    } else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2)) {
      obj2 = tobj;
      item2 = -1;
    }
  }
  if ((item0 > 0) || (item1 > 0) || (item2 > 0)) {
    if (verbose) {
      switch (number(0, 2)) {
      case 0:
	send_to_char("A wart sprouts on your nose.\r\n", ch);
	break;
      case 1:
	send_to_char("Your hair falls out in clumps.\r\n", ch);
	break;
      case 2:
	send_to_char("A huge corn develops on your big toe.\r\n", ch);
	break;
      }
    }
    return (FALSE);
  }
  if (extract) {
    if (item0 < 0) {
      obj_from_char(obj0);
      extract_obj(obj0);
    }
    if (item1 < 0) {
      obj_from_char(obj1);
      extract_obj(obj1);
    }
    if (item2 < 0) {
      obj_from_char(obj2);
      extract_obj(obj2);
    }
  }
  if (verbose) {
    send_to_char("A puff of smoke rises from your pack.\r\n", ch);
    act("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
  }
  return (TRUE);
}




/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 */

void mag_damage(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, int savetype)
{
  int dam = 0;

  if (victim == NULL || ch == NULL)
    return;

  switch (spellnum) {
    case SPELL_MAGIC_MISSILE:
    case SPELL_CHILL_TOUCH:       /* chill touch also has an affect */
      savetype = SAVING_COLD;
      dam = dice(1, 8) + level;
      break;
    case SPELL_BURNING_HANDS:
      savetype = SAVING_FIRE;
      dam = dice(3, 8) + 3;
      heat_blind(victim);
      break;
    case SPELL_SHOCKING_GRASP:
      savetype = SAVING_ELEC;
      dam = dice(5, 8) + 5;
      break;
    case SPELL_LIGHTNING_BOLT:
      savetype = SAVING_ELEC;
      dam = dice(7, 8) + level;
      break;
    case SPELL_COLOR_SPRAY:
      dam = dice(9, 8) + 9;
      break;
    case SPELL_FIREBALL:
      savetype = SAVING_FIRE;
      dam = 2 * level;
      heat_blind(victim);
      break;
    case SPELL_DISPEL_EVIL:
      dam = dice(6, 8) + 6;
      if (IS_EVIL(ch)) {
	victim = ch;
	dam = GET_HIT(ch) - 1;
      } else if (IS_GOOD(victim)) {
	act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
	dam = 0;
	return;
      }
      break;
    case SPELL_DISPEL_GOOD:
      dam = dice(6, 8) + 6;
      if (IS_GOOD(ch)) {
	victim = ch;
	dam = GET_HIT(ch) - 1;
      } else if (IS_EVIL(victim)) {
	act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
	dam = 0;
	return;
      }
      break;
    case SPELL_CALL_LIGHTNING:
      savetype = SAVING_ELEC;
      dam = dice(7, 8) + 7;
      break;
    case SPELL_HARM_LIGHT:
      savetype = SAVING_SPELL;
      dam = dice(1, 8) + 1 + (level >> 3);
      break;
    case SPELL_HARM_SERIOUS:
      savetype = SAVING_SPELL;
      dam = dice(2, 8) + 1 + (level >> 2);
      break;
    case SPELL_HARM_CRITICAL:
      savetype = SAVING_SPELL;
      dam = dice(3, 8) + 1 + (level >> 2);
      break;
    case SPELL_HARM:
      if (GET_LEVEL(victim) <= (level >> 1))
	dam = GET_HIT(victim) - dice(2, 4);
      else
	dam = dice(GET_LEVEL(ch) >> 1, 8) + 8;
      GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - dam);
      break;
    case SPELL_ENERGY_DRAIN:
      if (GET_LEVEL(victim) <= (GET_LEVEL(ch) >> 2))
	dam = GET_HIT(victim);
      else
	dam = dice(GET_LEVEL(ch) >> 2, 10);
      GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - dam);
      GET_EXP(victim) = MAX(0L, (long)GET_EXP(victim) - dam);
      GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch), GET_MOVE(ch) + (dam >> 1));
      break;
    case SPELL_POWER_KILL:
      if (!mag_savingthrow(victim, savetype)) {
	dam = dice(240, 2) + (level*10);
	GET_HIT(victim) = -2;
	GET_AGE_MOD(ch) += number(2, 4);
      } else {
	dam = 0;
      }
      break;
    case SPELL_EARTHQUAKE:
      dam = dice(4, 8) + level;
      break;
    case SPELL_ICE_SHOWER:
      savetype = SAVING_COLD;
      if (world[victim->in_room].sector_type == SECT_INSIDE)
	dam = 0;
      else
	dam = dice(level, 20) + (level*2);
      break;
    case SPELL_FIRE_STORM:
      savetype = SAVING_FIRE;
      if (world[victim->in_room].sector_type == SECT_INSIDE)
	dam = 0;
      else {
	dam = dice((level<<1), 40) + (level*2);
	heat_blind(victim);
      }
      break;
    case SPELL_METEOR_SWARM:
      savetype = SAVING_FIRE;
      if (world[victim->in_room].sector_type == SECT_INSIDE)
	dam = 0;
      else {
	dam = dice((level << 2), 50);
	heat_blind(victim);  /* Don't meteors get red hot too? */
      }
      break;
    case SPELL_FIRE_BREATH:
      savetype = SAVING_FIRE;
      dam = dice(level, 5);
      heat_blind(victim);
      break;
    case SPELL_GAS_BREATH:
      savetype = SAVING_ACID;
      dam = dice(level >> 1, level >> 2);
      break;
    case SPELL_FROST_BREATH:
      savetype = SAVING_COLD;
      dam = dice(level, 4);
      break;
    case SPELL_ACID_BREATH:
      savetype = SAVING_ACID;
      dam = dice(level, level >> 2);
      break;
    case SPELL_LIGHTNING_BREATH:
      savetype = SAVING_ELEC;
      dam = dice(level >> 2, level << 1);
      break;
  } /* switch(spellnum) */

  /* divide damage by two if victim makes his saving throw */
  if (mag_savingthrow(victim, savetype))
    dam >>= 1;

  /* and finally, inflict the damage */
  damage(ch, victim, dam, spellnum, -1);
}


/*
 * Every spell that does an affect comes through here.  This determines
 * the effect, whether it is added or replacement, whether it is legal or
 * not, etc.
 *
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
*/

#define MAX_SPELL_AFFECTS 5     /* change if more needed */

void mag_affects(int level, struct char_data * ch, struct char_data * victim,
		      int spellnum, int savetype)
{
  struct affected_type af[MAX_SPELL_AFFECTS];
  int    i;
  char   *to_vict = NULL, *to_room = NULL;
  bool   harmful = FALSE, accum_affect = FALSE, accum_duration = FALSE;

  if (victim == NULL || ch == NULL)
    return;

  for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
    af[i].type      = spellnum;
    af[i].bitvector = 0;
    af[i].modifier  = 0;
    af[i].location  = APPLY_NONE;
  }

  switch (spellnum) {
    case SPELL_CHILL_TOUCH:
    case SPELL_FROST_BREATH:
    case SPELL_ICE_SHOWER:
      savetype = SAVING_COLD;
      af[0].location = APPLY_STR;
      if (mag_savingthrow(victim, savetype))
	af[0].duration = dice(1, 6);
      else
	af[0].duration = dice(4, 6);
      af[0].modifier = -1;
      accum_duration = TRUE;
      to_vict = "You feel your strength wither!";
      break;
    case SPELL_ARMOR:
    case SPELL_GHOSTLY_SHIELD:
      af[0].location = APPLY_AC;
      af[0].modifier = 10;
      af[0].duration = dice(4, 6);
      accum_duration = TRUE;
      to_vict = "You feel someone protecting you.";
      break;
    case SPELL_FIREWALL:
      af[0].duration  = 1;
      af[0].bitvector = AFF_FIREWALL;
      accum_duration  = FALSE;
      harmful         = TRUE;
      act("A wall of fire errupts around $N!", FALSE, ch, 0, victim, TO_NOTVICT);
      to_vict = "A wall of fire errupts around you!";
      break;
    case SPELL_FLY:
      af[0].duration  = dice(4, 6);
      af[0].modifier  = 1;
      af[0].location  = APPLY_DEX;
      af[0].bitvector = AFF_FLYING;
      accum_duration  = TRUE;
      harmful         = TRUE;
      act("$N begins to float off the ground.", FALSE, ch, 0, victim, TO_NOTVICT);
      to_vict = "You begin to float off the ground.";
      break;
    case SPELL_HASTE:
      af[0].duration  = GET_LEVEL(ch) >> 1;
      af[0].location  = APPLY_HITROLL;
      af[0].modifier  = GET_LEVEL(ch) >> 2;
      af[0].bitvector = AFF_HASTE;
      accum_duration  = TRUE;
      to_vict = "You suddenly feel a burst of energy.";
      GET_AGE_MOD(victim) += number(1, 2);
      break;
    case SPELL_MIN_GLOBE:
      af[0].duration  = dice(1, 6) + (GET_LEVEL(ch) >> 3);
      af[0].location  = APPLY_AC;
      af[0].modifier  = dice(10, 2);
      af[0].bitvector = AFF_MIN_GLOBE;
      accum_duration  = TRUE;
      act("A glowing sphere encases $N.", FALSE, ch, 0, victim, TO_NOTVICT);
      to_vict = "You are encased in a glowing yellow sphere.";
      break;
    case SPELL_MAJ_GLOBE:
      af[0].duration  = dice(1, 6);
      af[0].location  = APPLY_SAVING_SPELL;
      af[0].modifier  = -10;
      af[0].bitvector = AFF_MAJ_GLOBE;
      accum_duration  = FALSE;
      to_vict = "A glowing shroud envelopes you!";
      act("A glowing shroud envelopes $N.", FALSE, ch, 0, victim, TO_NOTVICT);
      break;
    case SPELL_BLESS:
      af[0].location  = APPLY_HITROLL;
      af[0].modifier  = 2;
      af[0].duration  = dice(2, 6);

      af[1].location  = APPLY_SAVING_SPELL;
      af[1].modifier  = -1;
      af[1].duration  = dice(1, 6);

      accum_duration  = TRUE;
      to_vict = "You feel righteous.";
      break;
    case SPELL_BLIND:
      if (MOB_FLAGGED(victim,MOB_NOBLIND)) {
	act("You fail to blind $N.", FALSE, ch, 0, victim, TO_CHAR);
	return;
      }
      af[0].location  = APPLY_HITROLL;
      af[0].modifier  = -(dice(1, 4));
      af[0].duration  = dice(2, GET_LEVEL(ch));
      af[0].bitvector = AFF_BLIND;

      af[1].location  = APPLY_AC;
      af[1].modifier  = -(dice(2, 20));
      af[1].duration  = af[0].duration;
      af[1].bitvector = AFF_BLIND;

      harmful = TRUE;
      to_room = "$n seems to be blinded!";
      to_vict = "You have been blinded!";
      break;
    case SPELL_CURSE:
      af[0].location  = APPLY_HITROLL;
      af[0].duration  = 1 + (GET_LEVEL(ch) >> 1);
      af[0].modifier  = -1;
      af[0].bitvector = AFF_CURSE;

      af[1].location  = APPLY_DAMROLL;
      af[1].duration  = 1 + (GET_LEVEL(ch) >> 1);
      af[1].modifier  = -1;
      af[1].bitvector = AFF_CURSE;

      harmful        = TRUE;
      accum_duration = TRUE;
      accum_affect   = TRUE;
      to_room = "$n briefly glows red!";
      to_vict = "You feel very uncomfortable.";
      break;
    case SPELL_DETECT_INVIS:
      af[0].duration = 12 + level;
      af[0].bitvector = AFF_DETECT_INVIS;
      accum_duration = TRUE;
      to_vict = "Your eyes tingle.";
      break;
    case SPELL_SOUL_SEARCH:
      af[0].duration  = 12 + level;
      af[0].bitvector = AFF_DETECT_ALIGN;
      accum_duration  = TRUE;
      if (spellnum == SPELL_SOUL_SEARCH)
	to_vict = "Your eyes glow momentarily.";
      else
	to_vict = "Your eyes tingle.";
      break;
    case SPELL_DETECT_MAGIC:
      af[0].duration = 12 + level;
      af[0].bitvector = AFF_DETECT_MAGIC;
      accum_duration = TRUE;
      to_vict = "Your eyes tingle.";
      break;
    case SPELL_SILENCE:
      af[0].duration  = 2 * level;
      af[0].bitvector = AFF_SILENCE;
      accum_duration  = TRUE;
      harmful         = TRUE;
      to_vict = "You lose your voice!";
      to_room = "$n is unable to speak!";
      break;
    case SPELL_TROLLS_BLOOD:
      af[0].duration  = dice(1, level);
      af[0].bitvector = AFF_REGENERATION;
      accum_duration  = FALSE;
      to_vict = "Your blood burns and your wounds begin to heal.";
      break;
    case SPELL_MEDUSA_GAZE:
      af[0].duration  = level >> 2;
      af[0].bitvector = AFF_PETRIFIED;
      accum_duration  = FALSE;
      to_vict = "You have been turned to stone!";
      to_room = "$n's skin begins to harden into stone!";
      harmful = TRUE;
      break;
    case SPELL_FORCE_BOLT:
      af[0].duration  = dice(2, level);
      af[0].bitvector = AFF_PARALYSIS;
      accum_duration  = FALSE;
      to_vict = "You are stunned!";
      harmful = TRUE;
      break;
    case SPELL_DISEASE:
      af[0].location  = APPLY_CON;
      af[0].duration  = dice(2, level);
      af[0].modifier  = -2;
      af[0].bitvector = AFF_DISEASE;

      af[1].location  = APPLY_STR;
      af[1].duration  = dice(2, level);
      af[1].modifier  = -2;
      af[1].bitvector = AFF_DISEASE;
      harmful = TRUE;
      to_vict = "You feel very sick.";
      to_room = "Bloody puss-filled sores open upon $n's face!";
      break;
    case SPELL_INFRAVISION:
    case SPELL_UNDEAD_EYES:
      af[0].duration  = 12 + level;
      af[0].bitvector = AFF_INFRAVISION;
      accum_duration  = TRUE;
      if (spellnum == SPELL_INFRAVISION) {
	to_vict = "Your eyes glow red.";
	to_room = "$n's eyes glow red.";
      } else {
	to_vict = "Your eyes glow green.";
	to_room = "$n's eyes glow green.";
      }
      break;
    case SPELL_INVISIBLE:
    case SPELL_VANISH:
      if (!victim)
	victim = ch;

      af[0].duration  = 12 + (GET_LEVEL(ch) >> 2);
      af[0].modifier  = 40;
      af[0].location  = APPLY_AC;
      af[0].bitvector = AFF_INVISIBLE;
      accum_duration = TRUE;
      to_vict = "You vanish.";
      to_room = "$n slowly fades out of existence.";
      break;
    case SPELL_POISON:
      if (mag_savingthrow(victim, savetype)) {
	send_to_char(NOEFFECT, ch);
	return;
      }
      af[0].location = APPLY_STR;
      af[0].duration = dice(2, level);
      af[0].modifier = -2;
      switch (number(0, 2)) {
	case 0:  af[0].bitvector = AFF_POISON_I;   break;
	case 1:  af[0].bitvector = AFF_POISON_II;  break;
	case 2:  af[0].bitvector = AFF_POISON_III; break;
      }
      harmful = TRUE;
      accum_duration = TRUE;
      to_vict = "You feel very sick.";
      to_room = "$n gets violently ill!";
      break;
    case SPELL_PROT_FROM_EVIL:
      af[0].duration  = 24;
      af[0].bitvector = AFF_PROTECT_EVIL;
      accum_duration = FALSE;
      to_vict = "You feel invulnerable!";
      break;
    case SPELL_BLACKMANTLE:
      af[0].duration  = dice(2, level);
      af[0].bitvector = AFF_BLACKMANTLE;
      accum_duration  = TRUE;
      to_vict = "A black aura envelopes you.";
      to_room = "$n is surrounded by a black aura.";
      harmful = TRUE;
      break;
    case SPELL_SANCTUARY:
      af[0].duration  = dice(1, level);
      af[0].bitvector = AFF_SANCTUARY;
      accum_duration  = TRUE;
      to_vict = "A white aura momentarily surrounds you.";
      to_room = "$n is surrounded by a white aura.";
      break;
    case SPELL_SLEEP:
      if ((!pk_allowed && !IS_NPC(ch) && !IS_NPC(victim)) ||
	  (MOB_FLAGGED(victim, MOB_NOSLEEP)) ||
	  (mag_savingthrow(victim, savetype))) {
	send_to_char(NOEFFECT, ch);
	return;
      }
      af[0].duration  = 4 + (GET_LEVEL(ch) >> 2);
      af[0].bitvector = AFF_SLEEP;
      harmful = TRUE;

      if (GET_POS(victim) > POS_SLEEPING) {
	act("You feel very sleepy...  Zzzz......", FALSE, victim, 0, 0, TO_CHAR);
	act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
	GET_POS(victim) = POS_SLEEPING;
      }
      break;
    case SPELL_STRENGTH:
      af[0].location = APPLY_STR;
      af[0].duration = (level >> 1) + 4;
      af[0].modifier = 1 + (level > 18);
      accum_duration = TRUE;
      accum_affect   = TRUE;
      to_vict = "You feel stronger!";
      break;
    case SPELL_SENSE_LIFE:
      af[0].duration  = level;
      af[0].bitvector = AFF_SENSE_LIFE;
      accum_duration  = TRUE;
      to_vict = "Your feel your awareness improve.";
      break;
    case SPELL_WATERWALK:
      af[0].duration  = 24;
      af[0].bitvector = AFF_WATERWALK;
      accum_duration  = TRUE;
      to_vict = "You feel webbing between your toes.";
      break;
    case SPELL_WATERBREATH:
      af[0].type      = SPELL_WATERBREATH;
      af[0].duration  = 24;
      af[0].bitvector = AFF_WATERBREATH;
      accum_duration = TRUE;
      to_vict = "A strange feeling overcomes you, suddenly gills appear on your neck.";
      to_room = "Gills sprout from $n's neck!";
      break;
  }

  /*
   * If this is a mob that has this affect set in its mob file, do not
   * perform the affect.  This prevents people from un-sancting mobs
   * by sancting them and waiting for it to fade, for example.
   */
   if (IS_NPC(victim) && IS_AFFECTED(victim, af[0].bitvector|af[1].bitvector) &&
     !affected_by_spell(victim, spellnum, harmful)) {
     send_to_char(NOEFFECT, ch);
     return;
   }

  /*
   * If the victim is already affected by this spell, and the spell does
   * not have an accumulative effect, then fail the spell.
   */
  if (affected_by_spell(victim,spellnum,harmful) && !(accum_duration||accum_affect)) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  for (i = 0; i < MAX_SPELL_AFFECTS; i++)
    if (af[i].bitvector || (af[i].location != APPLY_NONE))
      affect_join(victim, af+i, accum_duration, FALSE, accum_affect, FALSE, harmful);

  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


/*
 * This function is used to provide services to mag_groups.  This function
 * is the one you should change to add new group spells.
 */
void perform_mag_groups(int level, struct char_data * ch,
			struct char_data * tch, int spellnum, int savetype)
{
  switch (spellnum) {
    case SPELL_GROUP_HEAL:
      mag_points(level, ch, tch, SPELL_HEAL, savetype);
      break;
    case SPELL_GROUP_ARMOR:
      mag_affects(level, ch, tch, SPELL_ARMOR, savetype);
      break;
    case SPELL_GROUP_RECALL:
      spell_recall(level, ch, tch, NULL);
      break;
    case SPELL_GROUP_PORTAL:
      spell_portal(level, ch, tch, NULL);
      break;
    case SPELL_GROUP_PASS_DOOR:
      spell_pass_door(level, ch, tch, NULL);
      break;
  }
}


/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */

void mag_groups(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *k;
  struct follow_type *f, *f_next;

  if (ch == NULL)
    return;

  if (!IS_AFFECTED2(ch, AFF_GROUP))
    return;
  if (ch->master != NULL)
    k = ch->master;
  else
    k = ch;
  for (f = k->followers; f; f = f_next) {
    f_next = f->next;
    tch = f->follower;
    if (tch->in_room != ch->in_room)
      continue;
    if (!IS_AFFECTED2(tch, AFF_GROUP))
      continue;
    if (ch == tch)
      continue;
    perform_mag_groups(level, ch, tch, spellnum, savetype);
  }

  if ((k != ch) && IS_AFFECTED2(k, AFF_GROUP))
    perform_mag_groups(level, ch, k, spellnum, savetype);
  perform_mag_groups(level, ch, ch, spellnum, savetype);
}


/*
 * mass spells affect every creature in the room except the caster.
 *
 * No spells of this class currently implemented as of Circle 3.0.
 */

void mag_masses(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *tch_next;

  for (tch = world[ch->in_room].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    /* don't hit the caster */
    if (tch == ch)
      continue;

    switch (spellnum) {
      case SPELL_PLAGUE:
	mag_affects(level, ch, tch, SPELL_DISEASE, savetype);
	break;
    }
  }
}


/*
 * Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 *  area spells have limited targets within the room.
*/

void mag_areas(byte level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *next_tch;
  char *to_char = NULL;
  char *to_room = NULL;

  if (ch == NULL)
    return;

  /*
   * to add spells to this fn, just add the message here plus an entry
   * in mag_damage for the damaging part of the spell.
   */
  switch (spellnum) {
    case SPELL_EARTHQUAKE:
      to_char = "You gesture and the earth begins to shake all around you!";
      to_room ="$n gracefully gestures and the earth begins to shake violently!";
      break;
    case SPELL_GAS_BREATH:
      to_char = "A horriffic stench emanates from your mouth!";
      to_room = "$n opens $s mouth and a green cloud with a horriffic stench envelopes you!";
      break;
    case SPELL_ACID_BREATH:
      to_char = "Acid streams from your lips!";
      to_room = "$n opens $s mouth and bathes you in a shower of acid!";
      break;
    case SPELL_ICE_SHOWER:
      if (world[ch->in_room].sector_type != SECT_INSIDE)  {
	to_char = "You call down a shower of tiny ice shards!";
	to_room = "$n calls down a torrent of tiny ice shards which rip through you!";
      }
      break;
    case SPELL_FIRE_STORM:
      if (world[ch->in_room].sector_type != SECT_INSIDE)  {
	to_char = "As you finish your spell, the area is enveloped in flames!";
	to_room = "$n fills the area with columns of fire which burns the flesh from your bones!";
      }
      break;
    case SPELL_METEOR_SWARM:
      if (world[ch->in_room].sector_type != SECT_INSIDE)  {
	to_char = "You summon meteors from the sky to rain down upon your enemies!";
	to_room = "$n cackles gleefully as you are pelted with tiny red hot meteors!";
      }
      break;
  }

  if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
  if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);

  for (tch = world[ch->in_room].people; tch; tch = next_tch) {
    next_tch = tch->next_in_room;
    /*
     * The skips: 1: the caster
     *            2: immortals
     *            3: if no pk on this mud, skips over all players
     *            4: pets (charmed NPCs)
     * players can only hit players in CRIMEOK rooms 4) players can only hit
     * charmed mobs in CRIMEOK rooms
     */
    if (tch == ch)
      continue;
    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
      continue;
    if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(tch))
      continue;
    if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM))
      continue;
    mag_damage(GET_LEVEL(ch), ch, tch, spellnum, 1);
  }
}


/*
 *  Every spell which summons/gates/conjours a mob comes through here.
 *
 *  None of these spells are currently implemented in Circle 3.0; these
 *  were taken as examples from the JediMUD code.  Summons can be used
 *  for spells like clone, ariel servant, etc.
 */
static char *mag_summon_msgs[] = {
  "\r\n",
  "$n makes a strange magical gesture; you feel a strong breeze!\r\n",
  "$n animates a corpse!\r\n",
  "$N appears from a cloud of thick blue smoke!\r\n",
  "$N appears from a cloud of thick green smoke!\r\n",
  "$N appears from a cloud of thick red smoke!\r\n",
  "$N disappears in a thick black cloud!\r\n"            /* 6 */
  "As $n makes a strange magical gesture, you feel a strong breeze.\r\n",
  "As $n makes a strange magical gesture, you feel a searing heat.\r\n",
  "As $n makes a strange magical gesture, you feel a sudden chill.\r\n",
  "As $n makes a strange magical gesture, you feel the dust swirl.\r\n",
  "$n magically divides!\r\n",                           /* 11 */
  "$n animates a corpse!\r\n",
  "$n makes strange magical gestures, and a huge portal opens next to $m.\r\n",
  "$N appears from a cloud of thick smoke!\r\n"          /* 14 */
};

static char *mag_summon_fail_msgs[] = {
  "\r\n",
  "There are no such creatures.\r\n",
  "Uh oh...\r\n",
  "Oh dear.\r\n",
  "Oh shit!\r\n",
  "The elements resist!\r\n",               /* 5 */
  "You failed.\r\n",
  "There is no corpse!\r\n",
  "You cannot conjure the illusion.\r\n"    /* 8 */
};

#define MOB_MONSUM_I            20
#define MOB_MONSUM_II           21
#define MOB_MONSUM_III          22
#define MOB_MONSUM_IV           23
#define MOB_GATE_I              24
#define MOB_GATE_II             25
#define MOB_GATE_III            26
#define MOB_GATE_IV             27  /* 28 is Seraph */
#define MOB_ILLUS_I             29
#define MOB_ILLUS_II            30
#define MOB_ILLUS_III           31
#define MOB_ILLUS_IV            32
#define MOB_ELEMENTAL_BASE      XX
#define MOB_CLONE               XX
#define MOB_ZOMBIE              33
#define MOB_AERIALSERVANT       34
#define MOB_FIRE_ELE            35
#define MOB_WATER_ELE           36
#define MOB_EARTH_ELE           37
#define MOB_AIR_ELE             38
#define MOB_SOUL_SUM_I          XX
#define MOB_SOUL_SUM_II         XX
#define MOB_SOUL_SUM_III        XX
#define MOB_SOUL_SUM_IV         XX
#define MOB_DOPPLEDANGER        89

void mag_summons(int level, struct char_data * ch, struct obj_data * obj,
		    struct char_data *victim, int spellnum, int savetype)
{
  struct char_data *mob = NULL;
  struct obj_data  *tobj, *next_obj;
  int    pfail = 0;
  int    msg = 0, fmsg = 0;
  int    num = 1, beatupchar = 0;
  int    a, i;
  int    mob_num = 0;
  int    handle_corpse = 0;

  if (ch == NULL)
    return;

  switch (spellnum) {
    case SPELL_ANIMATE_DEAD:
      if ((obj == NULL) || (GET_OBJ_TYPE(obj) != ITEM_CONTAINER) ||
	  (!GET_OBJ_VAL(obj, 3))) {
	act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
	return;
      }
      handle_corpse = 1;
      msg           = 12;
      mob_num       = MOB_ZOMBIE;
      pfail         = 2;
      break;
    case SPELL_MONSUM_I:
      msg     = 5;
      fmsg    = 2;
      mob_num = MOB_MONSUM_I;
      pfail   = 1;
      break;
    case SPELL_MONSUM_II:
      msg     = 5;
      fmsg    = 2;
      mob_num = MOB_MONSUM_II;
      pfail   = 2;
      break;
    case SPELL_MONSUM_III:
      msg     = 5;
      fmsg    = 2;
      mob_num = MOB_MONSUM_III;
      pfail   = 3;
      break;
    case SPELL_MONSUM_IV:
      a = number (0, 5);
      if (a >= 4) {
	msg = 13;
	num = number(2, 5);
	if (number(0, 2) < 2)
	  mob_num = MOB_MONSUM_II;
	else
	  mob_num = MOB_MONSUM_III;
	pfail = 5;
      } else {
	msg     = 5;
	mob_num = MOB_MONSUM_IV;
	pfail   = 6;
      }
      fmsg       = 4;
      beatupchar = 1;
      break;
    case SPELL_CLONE:
      msg     = 11;
      fmsg    = 6;
      mob_num = MOB_DOPPLEDANGER;
      pfail   = 8;
      break;
    case SPELL_GATE_I:
      msg     = 4;
      fmsg    = 2;
      mob_num = MOB_GATE_I;
      pfail   = 1;
      break;
    case SPELL_GATE_II:
      msg        = 4;
      fmsg       = 2;
      beatupchar = 1;
      mob_num    = MOB_GATE_II;
      pfail      = 2;
      break;
    case SPELL_GATE_III:
      msg        = 4;
      fmsg       = 2;
      beatupchar = 1;
      mob_num    = MOB_GATE_III;
      pfail      = 3;
      break;
    case SPELL_GATE_IV:
      msg        = 4;
      fmsg       = 2;
      beatupchar = 1;
      mob_num    = MOB_GATE_IV;
      pfail      = 5;
      if (IS_GOOD(ch))
	mob_num++;
      break;
    case SPELL_SUM_FIRE_ELE:
      msg     = 8;
      fmsg    = 5;
      mob_num = MOB_FIRE_ELE;
      pfail   = 10;
      break;
    case SPELL_SUM_WATER_ELE:
      msg     = 9;
      fmsg    = 5;
      mob_num = MOB_WATER_ELE;
      pfail   = 3;
      break;
    case SPELL_SUM_EARTH_ELE:
      msg     = 10;
      fmsg    = 5;
      mob_num = MOB_EARTH_ELE;
      pfail   = 3;
      break;
    case SPELL_SUM_AIR_ELE:
      msg     = 7;
      fmsg    = 5;
      mob_num = MOB_AIR_ELE;
      pfail   = 3;
      break;
    case SPELL_AERIAL_SERVANT:
      msg        = 1;
      fmsg       = 4;
      beatupchar = 1;
      mob_num    = MOB_AERIALSERVANT;
      pfail      = 2;
      break;
    case SPELL_ILLUSIONARY_PRES_I:
      msg        = 14;
      fmsg       = 8;
      mob_num    = MOB_ILLUS_I;
      pfail      = 0;
      break;
    case SPELL_ILLUSIONARY_PRES_II:
      msg     = 14;
      fmsg    = 8;
      mob_num = MOB_ILLUS_II;
      pfail   = 1;
      break;
    case SPELL_ILLUSIONARY_PRES_III:
      msg     = 14;
      fmsg    = 8;
      mob_num = MOB_ILLUS_III;
      pfail   = 2;
      break;
    case SPELL_ILLUSIONARY_PRES_IV:
      if (number(1, 10) >= 9) {
	num = number(3, 6);
	mob_num = MOB_ILLUS_III;
      } else
	mob_num = MOB_ILLUS_IV;
      msg     = 14;
      fmsg    = 8;
      pfail   = 3;
      break;
    default:
      return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("You are too giddy to have any followers!\r\n", ch);
    return;
  }
  if ((i = number(0, 10)) < pfail && !beatupchar) {
    send_to_char(mag_summon_fail_msgs[fmsg], ch);
    return;
  } else if (i < pfail && beatupchar) {
    send_to_char(mag_summon_fail_msgs[fmsg], ch);
    mob = read_mobile(mob_num, VNUMBER);
    char_to_room(mob, ch->in_room);
    hit(mob, ch, TYPE_UNDEFINED);
    return;
  }

  for (i = 0; i < num; i++) {
    if ((mob = read_mobile(mob_num, VNUMBER))) {
      char_to_room(mob, ch->in_room);
      IS_CARRYING_WEIGHT(mob) = 0;
      IS_CARRYING_N(mob) = 0;
      SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
      add_follower(mob, ch);
      act(mag_summon_msgs[fmsg], FALSE, ch, 0, mob, TO_ROOM);
      /* Clone the victim */
      if (spellnum == SPELL_CLONE) {
        if (GET_NAME(mob))
          free(GET_NAME(mob));
        strcpy(GET_NAME(mob), GET_NAME(victim));
        if (mob->player.short_descr)
          free(mob->player.short_descr);
        strcpy(mob->player.short_descr, GET_NAME(victim));
        if (mob->player.long_descr)
          free(mob->player.long_descr);
        sprintf(mob->player.long_descr, "%s is standing here.\r\n", GET_NAME(victim));
        GET_MAX_HIT(mob)  = GET_MAX_HIT(victim);
        GET_MAX_MOVE(mob) = GET_MAX_MOVE(victim);
        GET_LEVEL(mob)    = GET_LEVEL(victim);
        GET_EXP(mob) = 1;
      }
    } else {
      sprintf(buf, "SYSERR: (mag_summons) unable to load mob for %s, spell %d\n", GET_NAME(ch), spellnum);
      mudlog(buf, CMP, LVL_GOD, TRUE);
    }
  }
  if (handle_corpse) {
    for (tobj = obj->contains; tobj; tobj = next_obj) {
      next_obj = tobj->next_content;
      obj_from_obj(tobj);
      obj_to_char(tobj, mob);
    }
    extract_obj(obj);
  }
}


void mag_points(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, int savetype)
{
  int hit = 0;
  int move = 0, i;

  if (victim == NULL)
    return;

  if ((GET_HIT(victim) == GET_MAX_HIT(victim)) && 
      spellnum != SPELL_REFRESH && spellnum != SPELL_LIFESTEAL)
    act("$N doesn't seem to need any medical attention.", FALSE, ch, 0, victim, TO_CHAR);

  switch (spellnum) {
    case SPELL_HEAL_LIGHT:
      hit = dice(1, 8) + 1 + (level >> 2);
      send_to_char("You feel a little bit better.\r\n", victim);
      act("A few of $N's wounds close up.", TRUE, ch, 0, victim, TO_NOTVICT);
      break;
    case SPELL_HEAL_SERIOUS:
      hit = dice(2, 8) + 2 + (level >> 2);
      send_to_char("You feel better.\r\n", victim);
      act("Several of $N's wounds close up.", TRUE, ch, 0, victim, TO_NOTVICT);
      break;
    case SPELL_HEAL_CRITIC:
      hit = dice(3, 10) + level;
      send_to_char("You feel a lot better!\r\n", victim);
      act("Many of $N's wounds close up.", TRUE, ch, 0, victim, TO_NOTVICT);
      break;
    case SPELL_HEAL:
      hit = (25 * level) + dice(5, 8);
      send_to_char("A warm feeling floods your body.\r\n", victim);
      act("$N's wounds glow blue for a moment then disappear!", TRUE, ch, 0, victim, TO_NOTVICT);
      break;
    case SPELL_REFRESH:
      move = (dice(5, 5) * level);
      GET_COND(victim, TIRED) = 24;
      send_to_char("You feel fully rested.\r\n", victim);
      break;
    case SPELL_LIFESTEAL:
      hit = (dice(4, 5) + level);
      for (i = 0; i < 4; i++)
	GET_COND(victim, i) = 0;
      GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) + hit);
      hit = -(hit);
      break;
  }
  if (GET_WOUNDS(victim) >= 1) {
    hit = hit - GET_WOUNDS(victim);
    if (hit < 0) {
      GET_WOUNDS(victim) = -hit;
      hit = 0;
    } else
      GET_WOUNDS(victim) = 0;
  }
  GET_HIT(victim)  = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
  GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move);
  GET_HIT(victim)  = MAX(1, GET_HIT(victim));
  GET_MOVE(victim) = MAX(1, GET_MOVE(victim));
  update_pos(victim);
}


void mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
			int spellnum, int type)
{
  int spell = 0;
  char *to_vict = NULL, *to_room = NULL;

  if (victim == NULL)
    return;

  switch (spellnum) {
    case SPELL_CURE_BLIND:
    case SPELL_HEAL:
      spell   = SPELL_BLIND;
      to_vict = "Your vision returns!";
      to_room = "There's a momentary gleam in $n's eyes.";
      break;
    case SPELL_REMOVE_POISON:
    case SPELL_CURE_POISON:
      spell   = SPELL_POISON;
      to_vict = "A warm feeling runs through your body!";
      to_room = "$n looks better.";
      break;
    case SPELL_REMOVE_CURSE:
      spell   = SPELL_CURSE;
      to_vict = "You don't feel so unlucky.";
      break;
    case SPELL_CURE_DISEASE:
      spell   = SPELL_DISEASE;
      to_vict = "The disease is purged from your body.";
      break;
    case SPELL_CURE_PARALYSIS:
      spell   = SPELL_PARALYZE;
      to_vict = "You are able to move again!";
      break;
    default:
      sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_unaffects", spellnum);
      log(buf);
      return;
      break;
  }

  if (!affected_by_spell(victim, spell, TRUE)) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  affect_from_char(victim, spell, TRUE);
  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


void mag_alter_objs(int level, struct char_data * ch, struct obj_data * obj,
			 int spellnum, int savetype)
{
  char *to_char = NULL;
  char *to_room = NULL;

  if (obj == NULL)
    return;

  switch (spellnum) {
    case SPELL_BLESS:
      if (!IS_OBJ_STAT(obj, ITEM_BLESS) &&
	  (GET_OBJ_WEIGHT(obj) <= 5 * GET_LEVEL(ch))) {
	SET_BIT(GET_OBJ_EXTRA(obj), ITEM_BLESS);
	to_char = "$p glows briefly.";
      }
      break;
    case SPELL_CURSE:
      if (!IS_OBJ_STAT(obj, ITEM_NODROP)) {
	SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODROP);
	if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
	  GET_OBJ_VAL(obj, 2)--;
	to_char = "$p briefly glows red.";
      }
      break;
    case SPELL_INVISIBLE:
    case SPELL_VANISH:
      if (!IS_OBJ_STAT(obj, ITEM_NOINVIS | ITEM_INVISIBLE)) {
	SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
	to_char = "$p vanishes.";
      }
      break;
    case SPELL_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
	 (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
	 (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3)) {
	GET_OBJ_VAL(obj, 3) = number(1, 6);
	to_char = "$p steams briefly.";
      }
      break;
    case SPELL_REMOVE_CURSE:
      if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
	REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
	if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
	  GET_OBJ_VAL(obj, 2)++;
	to_char = "$p briefly glows blue.";
      }
      break;
    case SPELL_REMOVE_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
	 (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
	 (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3)) {
	GET_OBJ_VAL(obj, 3) = 0;
	to_char = "$p steams briefly.";
      }
      break;
  }
  if (to_char == NULL)
    send_to_char(NOEFFECT, ch);
  else
    act(to_char, TRUE, ch, obj, 0, TO_CHAR);

  if (to_room != NULL)
    act(to_room, TRUE, ch, obj, 0, TO_ROOM);
  else if (to_char != NULL)
    act(to_char, TRUE, ch, obj, 0, TO_ROOM);
}



void mag_creations(int level, struct char_data * ch, int spellnum)
{
  struct obj_data *tobj;
  long z;

  if (ch == NULL)
    return;
  level = MAX(MIN(level, LVL_IMPL), 1);

  switch (spellnum) {
    case SPELL_CREATE_FOOD:
      z = 65;
      break;
    default:
      send_to_char("Spell unimplemented, it would seem.\r\n", ch);
      return;
      break;
  }

  if (!(tobj = read_object(z, VNUMBER))) {
    send_to_char("I seem to have goofed.\r\n", ch);
    sprintf(buf, "SYSERR: spell_creations, spell %d, obj %ld: obj not found",
	    spellnum, z);
    log(buf);
    return;
  }
  obj_to_char(tobj, ch);
  act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
  act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
}

