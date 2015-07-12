/************************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  FIGHT.C                                      Based on CircleMUD *
*  Usage: Combat System                                                   *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __FIGHT_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct material_type const material_list[];

/* Structures */
struct char_data *combat_list = NULL;   /* head of l-list of fighting chars */
struct char_data *next_combat_list = NULL;

/* External structures */
extern struct zone_data *zone_table;              /* zone table             */
extern struct index_data *mob_index;
extern struct race_data *races;
extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern int pk_allowed;          /* see config.c */
extern int auto_save;           /* see config.c */
extern int max_exp_gain;        /* see config.c */
extern int NUM_RACES;

/* External procedures */
char *fread_action(FILE * fl, int nr);
char *fread_string(FILE * fl, char *error);
void stop_follower(struct char_data * ch);
ACMD(do_flee);
ACMD(do_assist);
ACMD(do_get);
void hit(struct char_data * ch, struct char_data * victim, int type);
void forget(struct char_data * ch, struct char_data * victim);
void remember(struct char_data * ch, struct char_data * victim);
int  ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim);
void mprog_hitprcnt_trigger(struct char_data * mob, struct char_data * ch);
void mprog_death_trigger(struct char_data * mob, struct char_data * killer);
void mprog_fight_trigger(struct char_data * mob, struct char_data * ch);
int  monk_damage(struct char_data *ch);
void Dismount(struct char_data *ch, struct char_data *h, int pos);
void MountThrow(struct char_data *ch, struct char_data *h, int pos);
void MountTrap(struct char_data *ch, struct char_data *h, int pos);
void scrap_obj(struct obj_data *o, struct char_data *c, int mode);
ACMD(do_infobar);
long exp_needed(int level);
int  check_newbie(struct char_data *ch, struct char_data *victim);

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit", "hits"},              /* 0 */
  {"sting", "stings"},
  {"whip", "whips"},
  {"slash", "slashes"},
  {"bite", "bites"},
  {"bludgeon", "bludgeons"},    /* 5 */
  {"crush", "crushes"},
  {"pound", "pounds"},
  {"claw", "claws"},
  {"maul", "mauls"},
  {"thrash", "thrashes"},       /* 10 */
  {"pierce", "pierces"},
  {"blast", "blasts"},
  {"punch", "punches"},
  {"stab", "stabs"},
  {"smash", "smashes"},         /* 15 */
  {"smite", "smites"},
};

 /* Location of attack texts */
struct attack_hit_type location_hit_text[] =
{
  {"in $S body",      "body"},          /* 0 */
  {"in $S left leg",  "left leg"},      /* 1 */
  {"in $S right leg", "right leg"},     /* 2 */
  {"in $S left arm",  "left arm"},      /* 3 */
  {"in $S right arm", "right arm"},     /* 4 */
  {"in $S left foot", "left foot"},     /* 5 */
  {"in $S right foot","right foot"},    /* 6 */
  {"in $S left hand", "left hand"},     /* 7 */
  {"in $S right hand","right hand"},    /* 8 */
  {"in $S chest",     "chest"},         /* 9 */
  {"in $S back",      "back"},          /* 10 */
  {"in $S stomach",   "stomach"},       /* 11 */
  {"in $S head",      "head"},          /* 12 */    /* End of humanoids */
  {"in $S tail",      "tail"},             /* 13 */
  {"in $S right wing","right wing"},       /* 14 */
  {"in $S left wing", "left wing"},        /* 15 */
  {"in $S right foreleg", "right foreleg"},/* 16 */
  {"in $S left foreleg", "left foreleg"}   /* 17 */
};

#ifndef IS_WEAPON
#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))
#endif

/* The Fight related routines */

void appear(struct char_data * ch)
{
  assert( ch != NULL );
  
  if (affected_by_spell(ch, SPELL_INVISIBLE,FALSE))
    affect_from_char(ch, SPELL_INVISIBLE, FALSE);
  
  REMOVE_BIT(AFF_FLAGS(ch), AFF_INVISIBLE | AFF_HIDE);
  REMOVE_BIT(ITEM_AFF(ch), AFFECTED_INVIS);
  
  if (GET_LEVEL(ch) < LVL_IMMORT)
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  else
    act("You feel a strange presence as $n appears, seemingly from nowhere.",
	FALSE, ch, 0, 0, TO_ROOM);
}

void load_messages(void)
{
  FILE *fl;
  int i, type;
  struct message_type *messages;
  char chk[128];

  if (!(fl = fopen(MESS_FILE, "r"))) {
    sprintf(buf2, "Error reading combat message file %s", MESS_FILE);
    perror(buf2);
    exit(1);
  }
  for (i = 0; i < MAX_MESSAGES; i++) {
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks = 0;
    fight_messages[i].msg = 0;
  }

  fgets(chk, 128, fl);
  while (!feof(fl) && (*chk == '\n' || *chk == '*'))
    fgets(chk, 128, fl);

  while (*chk == 'M') {
    fgets(chk, 128, fl);
    sscanf(chk, " %d\n", &type);
    for (i = 0; (i < MAX_MESSAGES) &&
           (fight_messages[i].a_type != type) &&
           (fight_messages[i].a_type); i++);
    if (i >= MAX_MESSAGES) {
      fprintf(stderr,
              "Too many combat messages. Increase MAX_MESSAGES and recompile.");
      exit(1);
    }
    CREATE(messages, struct message_type, 1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type = type;
    messages->next = fight_messages[i].msg;
    fight_messages[i].msg = messages;

    messages->die_msg.attacker_msg = fread_action(fl, i);
    messages->die_msg.victim_msg = fread_action(fl, i);
    messages->die_msg.room_msg = fread_action(fl, i);
    messages->miss_msg.attacker_msg = fread_action(fl, i);
    messages->miss_msg.victim_msg = fread_action(fl, i);
    messages->miss_msg.room_msg = fread_action(fl, i);
    messages->hit_msg.attacker_msg = fread_action(fl, i);
    messages->hit_msg.victim_msg = fread_action(fl, i);
    messages->hit_msg.room_msg = fread_action(fl, i);
    messages->god_msg.attacker_msg = fread_action(fl, i);
    messages->god_msg.victim_msg = fread_action(fl, i);
    messages->god_msg.room_msg = fread_action(fl, i);
    fgets(chk, 128, fl);
    while (!feof(fl) && (*chk == '\n' || *chk == '*'))
      fgets(chk, 128, fl);
  }

  fclose(fl);
}

void update_pos(struct char_data * victim)
{
  assert( victim != NULL );
  
  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
    return;
  else if (GET_HIT(victim) > 0)
    GET_POS(victim) = POS_STANDING;
  else if (GET_HIT(victim) <= -11)
    GET_POS(victim) = POS_DEAD;
  else if (GET_HIT(victim) <= -6)
    GET_POS(victim) = POS_MORTALLYW;
  else if (GET_HIT(victim) <= -3)
    GET_POS(victim) = POS_INCAP;
  else
    GET_POS(victim) = POS_STUNNED;
}


void check_killer(struct char_data * ch, struct char_data * vict)
{
  assert( ch != NULL );
  assert( vict != NULL );
  
  if (!PLR_FLAGGED(vict, PLR_KILLER) && !PLR_FLAGGED(vict, PLR_THIEF)
      && !PLR_FLAGGED(ch, PLR_KILLER) && !IS_NPC(ch) && !IS_NPC(vict) &&
      (ch != vict)) {
    /* Added for TD so areas can be defined as pkill areas */
    if ((zone_table[(world[ch->in_room].zone)].pkill != 1) || (!pk_allowed)) {
      char buf[256];

      SET_BIT(PLR_FLAGS(ch), PLR_KILLER);
      sprintf(buf, "PC Killer bit set on %s for initiating attack on %s at %s.",
              GET_NAME(ch), GET_NAME(vict), world[vict->in_room].name);
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
    }
  }
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data * ch, struct char_data * vict)
{
  assert( ch != NULL );
  assert( vict != NULL );
  
  if (ch == vict)
    return;

  assert(!FIGHTING(ch));

  ch->next_fighting = combat_list;
  combat_list = ch;

  if (IS_AFFECTED(ch, AFF_SLEEP))
    affect_from_char(ch, SPELL_SLEEP, TRUE);

  FIGHTING(ch) = vict;
  GET_POS(ch) = POS_FIGHTING;

  if (!pk_allowed)
    check_killer(ch, vict);
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data * ch)
{
  struct char_data * temp;
  assert( ch != NULL );

  if (ch == next_combat_list)
    next_combat_list = ch->next_fighting;

  REMOVE_FROM_LIST(ch, combat_list, next_fighting);
  ch->next_fighting = NULL;
  FIGHTING(ch) = NULL;
  if (!MOUNTED(ch))
    GET_POS(ch) = POS_STANDING;
  else
    GET_POS(ch) = POS_MOUNTED;
  update_pos(ch);

  if (PRF_FLAGGED(ch, PRF_INFOBAR))
    do_infobar(ch, 0, 0, SCMDB_ENDFIGHT);
}

/* creates various bodyparts based on hitlocation */
void create_bodypart(struct char_data *victim, int hitloc)
{
  /* perhaps this should be incorporated into the special damage messages */
}

void make_corpse(struct char_data * ch, int dam_type)
{
  struct obj_data *corpse, *o;
  struct obj_data *money;
  int i;
  extern int max_npc_corpse_time, max_pc_corpse_time;

  struct obj_data *create_money(long amount);

  assert( ch != NULL );

  corpse = create_obj();

  corpse->item_number = NOTHING;
  corpse->in_room = NOWHERE;
  corpse->name = str_dup("corpse");
  sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
  corpse->description = str_dup(buf2);

  sprintf(buf2, "the corpse of %s", GET_NAME(ch));
  corpse->short_description = str_dup(buf2);
 
  sprintf(buf2, "%s", GET_NAME(ch));
  corpse->corpsename = str_dup(buf2);
 
  GET_OBJ_TYPE(corpse)   = ITEM_CONTAINER;
  GET_OBJ_WEAR(corpse)   = ITEM_WEAR_TAKE;
  GET_OBJ_EXTRA(corpse)  = ITEM_NODONATE;
  GET_OBJ_VAL(corpse, 0) = 0;   /* You can't store stuff in a corpse */
  GET_OBJ_VAL(corpse, 3) = 1;   /* corpse identifier */
  GET_OBJ_VAL(corpse, 4) = GET_RACE(ch);
  GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
  GET_OBJ_RENT(corpse)   = 100000;
  if (IS_NPC(ch))
    GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
  else
    GET_OBJ_TIMER(corpse) = max_pc_corpse_time;

  /* transfer character's inventory to the corpse */
  corpse->contains = ch->carrying;
  for (o = corpse->contains; o != NULL; o = o->next_content)
    o->in_obj = corpse;
  object_list_new_owner(corpse, NULL);

  /* transfer character's equipment to the corpse */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      obj_to_obj(unequip_char(ch, i), corpse);

  /* transfer any items stuck in the character */
  for (i = 0;  i < 12; ++i) {
    obj_to_obj(GET_ITEMS_STUCK(ch, i), corpse);
    GET_ITEMS_STUCK(ch, i) = NULL;
  }
  /* transfer gold */
  if (GET_GOLD(ch) > 0) {
    /* following 'if' clause added to fix gold duplication loophole */
    if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
      money = create_money(GET_GOLD(ch));
      obj_to_obj(money, corpse);
    }
    GET_GOLD(ch) = 0;
  }
  ch->carrying = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_WEIGHT(ch) = 0;

  obj_to_room(corpse, ch->in_room);
}

struct obj_data *make_headless_corpse(struct obj_data * ocorpse,
				      int dam_typ,
                                      struct char_data * ch)
{
  struct obj_data *corpse;
  extern struct race_data * races;

  struct obj_data *create_money(long amount);

  assert( ch != NULL );

  corpse = create_obj();

  corpse->item_number = NOTHING;
  corpse->in_room = NOWHERE;
  corpse->name = str_dup("corpse");
  if (GET_OBJ_VAL(ocorpse, 4) != -1) 
    sprintf(buf2, "The headless corpse of %s %s is lying here.", 
            XANA( races[ GET_OBJ_VAL(ocorpse, 4) ].name ),
            races[ GET_OBJ_VAL(ocorpse, 4) ].name );
  else
     sprintf(buf2, "A headless corpse is lying here.");

  corpse->description = str_dup(buf2);

  corpse->short_description = str_dup("a headless corpse");

  GET_OBJ_TYPE(corpse)   = ITEM_CONTAINER;
  GET_OBJ_WEAR(corpse)   = ITEM_WEAR_TAKE;
  GET_OBJ_EXTRA(corpse)  = ITEM_NODONATE;
  GET_OBJ_VAL(corpse, 0) = 0;   /* You can't store stuff in a corpse */
  GET_OBJ_VAL(corpse, 3) = 1;   /* corpse identifier */
  GET_OBJ_WEIGHT(corpse) = GET_OBJ_WEIGHT(ocorpse);
  GET_OBJ_RENT(corpse)   = 100000;
  GET_OBJ_TIMER(corpse)  = GET_OBJ_TIMER(ocorpse);

  obj_to_room(corpse, ch->in_room);
  return corpse;
}

void make_head(struct obj_data * ocorpse,
               int dam_type,
               struct char_data * ch)
{
  struct obj_data *head;
  extern int max_pc_corpse_time;
  assert( ch != NULL );

  head = create_obj();

  head->item_number = NOTHING;
  head->in_room = NOWHERE;
  sprintf(buf2, "head %s", ocorpse->corpsename);
  head->name = str_dup(buf2);
  sprintf(buf2, "The severed head of %s is lying here.", ocorpse->corpsename);
  head->description = str_dup(buf2);

  sprintf(buf2, "the severed head of %s", ocorpse->corpsename);
  head->short_description = str_dup(buf2);

  GET_OBJ_TYPE(head)   = ITEM_OTHER;
  GET_OBJ_WEAR(head)   = ITEM_WEAR_TAKE;
  GET_OBJ_EXTRA(head)  = ITEM_NODONATE;
  GET_OBJ_VAL(head, 0) = 0;   /* You can't store stuff in a corpse */
  GET_OBJ_VAL(head, 3) = 1;   /* corpse identifier */
  GET_OBJ_WEIGHT(head) = 0;
  GET_OBJ_RENT(head)   = 100000;
  GET_OBJ_TIMER(head)  = max_pc_corpse_time * 2;  /* Skulls last longer */

  obj_to_room(head, ch->in_room);
}

/* When ch kills victim */
void change_alignment(struct char_data * ch, struct char_data * victim)
{
  assert( ch != NULL );
  assert( victim != NULL );
  /*
   * new alignment change algorithm: if you kill a monster with alignment A,
   * you move 1/16th of the way to having alignment -A.  Simple and fast.
   */
  if ((IS_GOOD(ch) && !IS_EVIL(victim)) ||
      (IS_EVIL(ch) && !IS_GOOD(victim)))
    GET_ALIGNMENT(ch) += ((-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) >> 5);
  else
    /* changed from 4 to 6 so its 1/64th of the way - slower change */
    GET_ALIGNMENT(ch) += ((-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) >> 6);
}

void death_cry(struct char_data * ch)
{
  int door, was_in;

  assert( ch != NULL );
  act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);
  was_in = ch->in_room;

  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (CAN_GO(ch, door)) {
      ch->in_room = world[was_in].dir_option[door]->to_room;
      act("Your blood freezes as you hear someone's death cry.",
          FALSE, ch, 0, 0, TO_ROOM);
      ch->in_room = was_in;
    }
  }
}

void raw_kill(struct char_data * ch, struct char_data * killer)
{
  assert( ch != NULL );
  assert( killer != NULL );
  
  if (FIGHTING(ch))
    stop_fighting(ch);

  while (ch->affected)
    affect_remove(ch, ch->affected, FALSE);
  while (ch->affected2)
    affect_remove(ch, ch->affected2, TRUE);

  if (!IS_IMMORT(ch)) {
    GET_COND(ch, THIRST) = 24;
    GET_COND(ch, FULL)   = 24;
    GET_COND(ch, TIRED)  = 24;
    GET_COND(ch, DRUNK)  = 0;
  } else {
    GET_COND(ch, THIRST) = GET_COND(ch, FULL) = GET_COND(ch, DRUNK)  = -1;
    GET_COND(ch, TIRED)  = -1;
  }

  GET_WOUNDS(ch)       = 0;

  if (killer) {
    mprog_death_trigger(ch, killer);
    if (IS_NPC(killer) && HUNTING(killer) == ch)
      HUNTING(killer) = NULL;
  }
  
  if ( !IS_NPC(ch) )
    send_to_char("\r\n\r\nRedemption comes not from a change of body, " \
                 "but a change of heart.\r\n\r\n", ch);

  death_cry(ch); 
  make_corpse(ch, 0);
  extract_char(ch);

  if (killer) {
    if (PRF_FLAGGED(killer, PRF_AUTOLOOT))
      do_get(killer, "all corpse", 0, 0);
    else if (PRF_FLAGGED(killer, PRF_AUTOGOLD) || 
             (IS_NPC(killer) && !IS_ANIMAL(killer)))
      do_get(killer, "coin corpse", 0, 0);
  }
}

/**
 */
void die(struct char_data * ch, struct char_data * killer)
{
  assert( ch != NULL );
  assert( killer != NULL );
  
  GET_EXP(ch) = (GET_LEVEL(ch) >= 4 ?
                 MAX(1L, (long)exp_needed(MAX(0, (GET_LEVEL(ch)-1)))) :
                 MAX(1L, (long)GET_EXP(ch) - (GET_EXP(ch) >> 3)));

  if (!IS_NPC(ch))
    REMOVE_BIT(PLR_FLAGS(ch), PLR_KILLER | PLR_THIEF);
  if (!IS_NPC(ch) && !IS_NPC(killer))
    SET_BIT(PLR_FLAGS(ch), PLR_NEWBIE);
  if (RIDDEN(ch)) {
    if (number(0,100) <= 70)
      Dismount(RIDDEN(ch), MOUNTED(ch), POS_SITTING);
    else if (number(0,100) <= 90)
      MountThrow(RIDDEN(ch), MOUNTED(ch), POS_SITTING);
    else
      MountTrap(RIDDEN(ch), MOUNTED(ch), POS_INCAP);
  } if (MOUNTED(ch))
    Dismount(MOUNTED(ch), RIDDEN(ch), POS_SITTING);

  raw_kill(ch, killer);
}

void perform_group_gain(struct char_data * ch, int base, struct char_data * victim)
{
  int share;
  assert( ch != NULL );
  assert( victim != NULL );

  share = MIN((int)max_exp_gain, MAX(1, (int)GET_LEVEL(ch) * base));

  send_to_char("You receive your share of the experience.\r\n", ch);

  gain_exp(ch, share);
  change_alignment(ch, victim);
}

void group_gain(struct char_data * ch, struct char_data * victim)
{
  int    base, tmp = 0;
  struct char_data *k;
  struct follow_type *f;

  assert( ch != NULL );
  assert( victim != NULL );
  
  if (!(k = ch->master))
    k = ch;

  if (IS_AFFECTED2(k, AFF_GROUP) && (k->in_room == ch->in_room))
    tmp = GET_LEVEL(k);

  for (f = k->followers; f; f = f->next) {
    if (IS_AFFECTED2(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
      tmp += GET_LEVEL(f->follower);
  }
  /* Divide the kill's exp into shares based on levels */
  base = (GET_EXP(victim)/tmp);
  /* changed for TD as a test - SPM 5/24/95   */
  /* round up to the next highest tot_members */
  /* base = (GET_EXP(victim) / 3) + tot_members - 1; */

  base = MAX(1, (int)base);

  if (IS_AFFECTED2(k, AFF_GROUP) && k->in_room == ch->in_room)
    perform_group_gain(k, base, victim);

  for (f = k->followers; f; f = f->next) {
    if (IS_AFFECTED2(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
      perform_group_gain(f->follower, base, victim);
  }
}

char *replace_string(char *str, char *weapon_singular, char *weapon_plural,
		     char *location_hit, char *location_hit_s)
{
  static char buf[256];
  char *cp = buf;
  assert( str != NULL );

  for (; *str; str++) {
    if (*str == '#') {
      switch (*(++str)) {
      case 'W':
	for (; *weapon_plural; *(cp++) = *(weapon_plural++));
	break;
      case 'w':
	for (; *weapon_singular; *(cp++) = *(weapon_singular++));
	break;
	/* added this to show where the person was hit */
      case 'L' :
	for (; *location_hit; *(cp++) = *(location_hit++));
	break;
      case 'l' :
	for (; *location_hit_s; *(cp++) = *(location_hit_s++));
	break;
      default:
	*(cp++) = '#';
	break;
      }
    } else
      *(cp++) = *str;

    *cp = 0;
  }                             /* For */

  return (buf);
}


char * special_hit_text(struct char_data * ch, struct char_data * victim,
			int w_type, int dam, int hitloc, int to_who)
{
  int i;

  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_vict;
  } spec_hit_weapons[] = {
    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */
    {
      "$n's #w tears off $N's arm at the shoulder, leaving protuding fragments of bone.",
      "Your #w tears off $N's arm at the shoulder, leaving bloody fragments of bone.",
      "$n's #w tears off your arm at the shoulder! Blood spurts from a major artery.",
    },

    {
      "$n's #w smashes through $N's arm and into $S chest, caving in $S ribcage!",
      "Your #w smashes through $N's arm and into $S chest, caving in $S ribcage!",
      "$n's #w smashes through your arm and into your chest, caving it in!",
    },
    /* 0 and 1 are arms - 1 is insta-kill */
    {
      "$n #W $N's jaw, breaking $S jawbone and causing the loss of several teeth.",
      "You #w $N's jaw, breaking $S jawbone and causing the loss of several teeth.",
      "$n #W your jaw, breaking your jawbone and causing you to lose several teeth.",
    },

    {
      "$n #W $N's jaw, forcing it upward into the lower part of $S's brain!",
      "You #w $N's jaw, forcing it upward into the lower part of $S's brain!",
      "$n #W your jaw, forcing it upward into the lower part of your brain!!!",
    },

    {
      "$n #W $N's neck, you hear the SNAP of smashing $S vertebrae!!!",
      "You #w $N's neck, you hear the SNAP of smashing $S vertebrae!!!",
      "$n #W your neck, the last thing you hear is the SNAP of your vertebrae!!!",
    },

    {
      "$n's #w shatters $N's skull! You are covered in blood and pieces of brain!",
      "Your #w shatters $N's skull! You are covered in blood and pieces of $S brain!",
      "$n's #w shatters your skull!!!  You are dead!",
    },
    /*  2 - 5 are head shots - 3, 4, and 5 are insta-kills */
    {
      "$n's #w strikes $N's groin.  $E is doubled up in agony.",
      "Your #w strikes $N's groin.  $E is doubled up in agony.",
      "$n's #w strikes your groin.  You are doubled up in agony.",
    },

    {
      "$n #W $N, lifting $M up into the air and slams $M to the ground!!!",
      "Your #w lifts $N up into the air and slams him down to the ground!!!",
      "$n's #w lifts you up into the air and slams you back down to the ground!",
    },

    {
      "There is an audible CRACK as $n's #w breaks several of $N's ribs.",
      "There is an audible CRACK as your #w breaks several of $N's ribs.",
      "There is an audible CRACK as $n's #w breaks several of your ribs!",
    },

    {
      "$n's #w causes a loud CRUNCH as it smashes $N's spine!!!",
      "Your #w causes a loud CRUNCH as it smashes into $N's spine!!!",
      "$n's #w causes a loud CRUNCH as it smashes into your spine!!!",
    },

    {
      "$n's #w caves in $N's chest, rupturing several internal organs!",
      "Your #w caves in $N's chest, rupturing several of $S organs!",
      "$n's #w caves in your chest, rupturing several of your organs!",
    },

    {
      "$n's #w ruptures $N's abdominal cavity, spilling $S entrails all over!",
      "Your #w ruptures $N's abdominal cavity, spilling $S entrails all over!",
      "$n's #w ruptures your abdominal cavity, spilling your entrails all over!",
    },

    {
      "$n's #w rips through $N's body, tearing $M in half!!",
      "Your #w rips through $N's body, tearing $M in half!!",
      "$n's #w rips through your body, tearing you in half!!",
    },
    /* 6 - 12 are body shots - 9 - 12 or 11 and 12 are insta-kills */
    {
      "$n's #w strikes $N's shin, shattering the bones.",
      "Your #w strikes $N's ankle, shattering the bones.",
      "$n's #w strikes your ankle, shattering the bones.",
    },

    {
      "$n's #w opens a deep wound in $N's leg, cutting through muscle and sinew.",
      "Your #w opens a deep wound in $N's leg, cutting through muscle and sinew.",
      "$n's #w opens a deep wound in your leg, cutting through muscle and sinew!",
    },

    {
      "$n's #w smashes $N's leg, severing a major artery!",
      "Your #w smashes $N's leg, severing a major artery!",
      "$n's #w smashes your leg, severing a major artery!",
    },

    {
      "There is a sickening crunch as $n #W the bones of $N's hip and thigh!",
      "There is a sickening crunch as your #w smashes $N's his and thigh!",
      "There is a sickening crunch as $n #W the bones of your hip and thigh!",
    },

    {
      "$N stares with horror as blood pumps from the mangled stump of $S ankle!",
      "$N stares with horror as blood pumps from the mangled stump of $S ankle!",
      "You stare with horror as blood pumps from the mangled stump of your ankle!",
    },

    {
      "$n's #w tears off $N's leg at the knee, splintering bone and mangling flesh!",
      "Your #w tears off $N's leg at the knee, splintering bone and mangling flesh!",
      "$n's #w tears off your leg at the knee, splintering bone and mangling flesh!",
    },

    {
      "$n's #w tears off $N's leg at the hip, splintering bone and mangling flesh!",
      "Your #w tears off $N's leg at the hip, splintering bone and mangling flesh!",
      "$n's #w tears off your leg at the hip, splintering bone and mangling flesh!",
    }
    /* 13 - 19 are leg shots - 15 thru 19 are insta-kills */
  };

  switch (hitloc) {
     case 1:
     case 2:
     case 5:
     case 6:  /* legs */
       switch (to_who) {
	 case 0:     /* to room */
	   if (dam >= GET_HIT(victim))              /* fatal */
	      return (spec_hit_weapons[1].to_room);
	   else
	      return (spec_hit_weapons[0].to_room);
	   break;
	 case 1:     /* to char */
	   if (dam >= GET_HIT(victim))             /* fatal */
	      return (spec_hit_weapons[1].to_char);
	   else
	      return (spec_hit_weapons[0].to_char);
	   break;
	 case 2:     /* to vict */
	   if (dam >= GET_HIT(victim))            /* fatal */
	      return (spec_hit_weapons[1].to_vict);
	   else
	      return (spec_hit_weapons[0].to_vict);
	   break;
       }
       break;
     case 3:
     case 4:
     case 7:
     case 8:  /* arms */
       if (dam >= GET_HIT(victim))
	 i = number(9, 12);
       else
	 i = number(6, 8);
       switch (to_who) {
	 case 0:     /* to room */
	   return (spec_hit_weapons[i].to_room);
	   break;
	 case 1:     /* to char */
	   return (spec_hit_weapons[i].to_char);
	   break;
	 case 2:     /* to vict */
	   return (spec_hit_weapons[i].to_vict);
	   break;
       }
       break;
     case 12:     /* head hits */
       if (dam >= GET_HIT(victim))
	 i = number(3, 5);
       else
	 i = 2;
       switch (to_who) {
	 case 0:     /* to room */
	   return (spec_hit_weapons[i].to_room);
	   break;
	 case 1:     /* to char */
	   return (spec_hit_weapons[i].to_char);
	   break;
	 case 2:     /* to vict */
	   return (spec_hit_weapons[i].to_vict);
	   break;
       }
       break;
     case 0:      /* body hits */
     case 9:
     case 10:
     case 11:
     default:
       if (dam >= GET_HIT(victim))
	 i = number(9, 12);
       else
	 i = number(6, 8);
       switch (to_who) {
	 case 0:     /* to room */
	   return (spec_hit_weapons[i].to_room);
	   break;
	 case 1:     /* to char */
	   return (spec_hit_weapons[i].to_char);
	   break;
	 case 2:     /* to vict */
	   return (spec_hit_weapons[i].to_vict);
	   break;
       }
       break;
  }
  return NULL;
}

char * miss_text(struct char_data * ch, struct char_data * victim,
		  int w_type, int to_who)
{
  int i = 0;

  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } miss_weapons[] = {
    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */
    {
      "$n tries to #w $N, but misses.", /* 0: 0     */
      "You try to #w $N, but miss.",
      "$n tries to #w you, but misses.",
    },

    {
      "$n attempts to #w $N, but $E jumps out of the way.",     /* 1: 1..2  */
      "You attempt to #w $N, $E jumps out of the way.",
      "$n attempts to #w you, but you quickly jump out of the way.",
    },

    {
      "$n loses $s balance as $N nimbly dodges out of $s #w.",
      "$N dodges out of the way of your #w.",
      "You nimbly dodge out of the way of $n's #w.",
    },

    {
      "Sparks fly as $N parries $n's #w.",
      "Sparks fly as $N parries your #w.",
      "Sparks fly as you parry $n's #w.",
    },

    {
      "$N jumps back as $n's #w passes through where $S #L used to be.",
      "You #w the air where $N's #L used to be.",
      "You nimbly leap out of the way of $n's #w.",
    },

    {
      "$N ducks down low as $n's #w flies through the air above $M.",
      "$N ducks down low as your #w flies through the air above $M.",
      "You duck down low as $n's #w flies through the air above you.",
    }
  };

  if (w_type == 0 || w_type == 13 || w_type == 4 || w_type == 8)
    i = number(0, 2);
  else
    i = number(0, 5);
  switch (to_who) {
     case 0:   /* to room */
       return (miss_weapons[i].to_room);
       break;
     case 1:   /* to damager */
       return (miss_weapons[i].to_char);
       break;
     case 2:   /* to damagee */
       return (miss_weapons[i].to_victim);
       break;
  }
  return NULL;
}

/* damages victims eq and ch's weapon if damage is severe */
void damage_equipment(int dam,
                      struct char_data * ch,
                      struct char_data * victim,
                      int hitloc)
{
  struct obj_data *obj = NULL;
  assert( ch != NULL );
  assert( victim != NULL );
  
  /* this function might serve us better if it was placed right before *
   * damage was dealt and it returned the amount of damage the victim  *
   * was to take, and have the victim's armor in that location take    *
   * some of the damage from the total.                                */
  
  /* If dam is > 1/4 of vicitm's MAX HPs then ch's weapon takes a beating */
  if (dam >= (GET_MAX_HIT(victim) / 4)) {
    if ((obj = GET_EQ(ch, WEAR_WIELD)) != NULL) {
      if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
        GET_OBJ_VAL(obj, 9) += number(1, 4);
    } else if ((obj = GET_EQ(ch, WEAR_HOLD)) != NULL) {
      if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
        GET_OBJ_VAL(obj, 9) += number(1, 4);
    }
  }
  
  /* now for the victim's eq - this is also good for knocking weapons *
   * out of the victim's hand! */
  switch (hitloc) {
  case 1:
  case 2:  /* legs */
    if ((obj = GET_EQ(victim, WEAR_LEGS)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    break;
  case 3:  /* left arm */
    if ((obj = GET_EQ(victim, WEAR_ARM_L)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    break;
  case 4:  /* right arm */
    if ((obj = GET_EQ(victim, WEAR_ARM_R)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    break;
  case 5:  /* left foot */
    if ((obj = GET_EQ(victim, WEAR_FOOT_L)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    break;
  case 6:  /* right foot */
    if ((obj = GET_EQ(victim, WEAR_FOOT_R)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    break;
  case 7:  /* left hand */
    /* damage the object and drop it */
    if ((obj = GET_EQ(victim, WEAR_HAND_L)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    if (obj) {
      obj_to_room(unequip_char(victim, WEAR_HAND_L), victim->in_room);
      act("$n knocks $o from $N's hand!", FALSE, ch, obj, victim, TO_NOTVICT);
      act("You knock $o from $N's hand!", FALSE, ch, obj, victim, TO_CHAR);
      sprintf(buf2, "%s$n knocks %s$o%s%s from your hand!%s",
              CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), CCNRM(ch, C_SPR),
              CCMAG(ch, C_SPR), CCNRM(ch, C_SPR));
      act(buf2, FALSE, ch, obj, victim, TO_VICT);
    }
    break;
  case 8:  /* right hand */
    /* damage the object and drop it */
    if ((obj = GET_EQ(victim, WEAR_HAND_R)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    if (obj) {
      obj_to_room(unequip_char(victim, WEAR_HAND_R), victim->in_room);
      act("$n knocks $o from $N's hand!", FALSE, ch, obj, victim, TO_NOTVICT);
      act("You knock $o from $N's hand!", FALSE, ch, obj, victim, TO_CHAR);
      sprintf(buf2, "%s$n knocks %s$o%s%s from your hand!%s",
              CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), CCNRM(ch, C_SPR),
              CCMAG(ch, C_SPR), CCNRM(ch, C_SPR));
      act(buf2, FALSE, ch, obj, victim, TO_VICT);
    }
    break;
  case 10:    /* back */
    if ((obj = GET_EQ(victim, WEAR_BACK)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    break;
  case 11:   /* stomach / waist */
    if ((obj = GET_EQ(victim, WEAR_WAIST)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    break;
  case 12:     /* head */
    if ((obj = GET_EQ(victim, WEAR_HEAD)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, 4);
    if ((number(0, 10) > 6) && obj) {    /* knock off helm */
      obj_to_room(unequip_char(victim, WEAR_HEAD), victim->in_room);
      act("$n knocks $o from $N's head!", FALSE, ch, obj, victim, TO_NOTVICT);
      act("You knock $o from $N's head!", FALSE, ch, obj, victim, TO_CHAR);
      sprintf(buf2, "%s$n knocks %s$o%s%s from your head!%s",
              CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), CCNRM(ch, C_SPR),
              CCMAG(ch, C_SPR), CCNRM(ch, C_SPR));
      act(buf2, FALSE, ch, obj, victim, TO_VICT);
    }
    break;
  case 9:
  case 0:
  default:    /* body */
    if ((obj = GET_EQ(victim, WEAR_BODY)) != NULL)
      GET_OBJ_VAL(obj, 9) += number(1, dam >> 4);
    break;
  }
  /* make sure magical objs are unbreakable */
  if (obj && GET_OBJ_MATERIAL(obj) == MAT_MAGICAL)
    GET_OBJ_VAL(obj, 9) = 0;
  /* see if obj has been destroyed */
  if (obj && GET_OBJ_VAL(obj, 7) != -1) {
    if (GET_OBJ_VAL(obj, 9) >
        material_list[GET_OBJ_MATERIAL(obj)].sturdiness) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_BROKEN);
      scrap_obj(obj, ch, 1);
    }
  }
}

/* returns 1 if the victim does have that bodypart */
int has_bodypart(struct char_data * victim, int part)
{
  assert( victim != NULL );
  
  if (IS_HUMANOID(victim) && part >= 13)
    return 0;
  else
    return 1;
}

/* return a valid hit location based on victim's race */
int hit_location(struct char_data *victim, int percent)
{
  int hitloc, i = 0;
  assert( victim != NULL );

  if (IS_HUMANOID(victim)) {
    /* determines a hit location and makes sure the player has that part */
    if (percent <= 10)         /* head shot */
      hitloc = 12;
    else if (percent < 30)     /* right arm */
      hitloc = 4;
    else if (percent < 50)     /* left arm  */
      hitloc = 3;
    else if (percent < 80)     /* body shot */
      hitloc = 0;
    else if (percent < 88)     /* right leg */
      hitloc = 2;
    else if (percent < 96)     /* left leg  */
      hitloc = 1;
    else                       /* chest, back, or stomach */
      hitloc = number(9, 11);
  } else if (IS_DRAGON(victim)) {
    if (percent <= 5)
      hitloc = 12;            /* head */
    else if (percent <= 20)
      hitloc = 16;            /* r foreleg */
    else if (percent <= 35)
      hitloc = 17;            /* l foreleg */
    else if (percent <= 65)
      hitloc = 0;             /* body */
    else if (percent <= 75)
      hitloc = 14;            /* right wing */
    else if (percent <= 85)
      hitloc = 15;            /* left wing  */
    else if (percent <= 90)
      hitloc = 13;            /* tail */
    else if (percent <= 95)
      hitloc = 1;             /* left leg  */
    else
      hitloc = 2;           /* right leg */
  } else
    hitloc = 0;  /* body hit */
  
  if (IS_HUMANOID(victim) && (hitloc > 0 && hitloc <= 4)) {
    /* to see if we hit the foot or hand instead of leg or arm */
    i = number(1, 100);
    if (i <= 20)
      hitloc += 4;
  }
  return hitloc;
}


/* message for doing damage with a weapon - returns the hitloc */
void dam_message(int dam, struct char_data * ch,
                 struct char_data * victim,
                 int w_type, int hitloc)
{
  char *buf;
  int msgnum, special_text = 0;

  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_weapons[] = {
    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */
    {
      "$n tries to #w $N, but misses.", /* 0: 0     */
      "You try to #w $N, but miss.",
      "$n tries to #w you, but misses.",
    },

    {
      "$n's #w scrapes $N lightly on $S #L.",       /* 1:  1 */
      "Your #w scrapes $N lightly on $S #L.",
      "$n's #w scrapes you lightly on your #L.",
    },

    {
      "$n lightly strikes $N with $S #w.",         /*  2:  2..3 */
      "You lightly strike $N with your #w.",
      "$n lightly strikes you with $S #w.",
    },

    {
      "$n barely #W $N.",                          /*  3:  4..5 */
      "You barely #w $N.",
      "$n barely #W you.",
    },

    {
      "$N grimaces in pain as $n #W $S #L.",      /*  4:  6..7 */
      "$N grimaces in pain as you #w $S #L.",
      "You grimace in pain as $n #W your #L.",
    },

    {
      "$n deftly #W $N.",                         /*  5:  8..9 */
      "You deftly #w $N.",
      "$n deftly #W you.",
    },

    {
      "$N gapes at $S #L as $n #W $M deeply.",    /*  6:  10..11 */
      "$N gapes at $S #L as you #w $M deeply.",
      "You gape at your #L as $n #W you deeply.",
    },

    {
      "$n #W $N hard.",                           /*  7:  12 */
      "You #w $N hard.",
      "$n #W you hard.",
    },

    {
      "$n #W $N very hard.",                      /*  8:  13..14 */
      "You #w $N very hard.",
      "$n #W you very hard.",
    },

    {
      "$n #W $N, opening a deep wound in $S #L.",   /* 9: 15..16 */
      "You #w $N, opening a deep wound in $S #L.",
      "$n #W you, opening a deep wound in your #L.",
    },

    {
      "$n #W $N severely on $S #L.",                 /* 10: 17..19 */
      "You #w $N severely on $S #L.",
      "$n #W you severely on your #L.",
    },

    {
      "$n tears into $N with $s #w, spraying $S blood everywhere.",    /* 11: 20+ */
      "You tear into $N with your #w, spraying $S blood everywhere.",
      "$n tears into you with $s #w, spraying your blood everywhere.",
    },

    {
      "$n massacres $N to small fragments with $s #w.", /* 7: 19..23 */
      "You massacre $N to small fragments with your #w.",
      "$n massacres you to small fragments with $s #w.",
    },

    {
      "$n OBLITERATES $N with $s deadly #w!!",  /* 8: > 23   */
      "You OBLITERATE $N with your deadly #w!!",
      "$n OBLITERATES you with $s deadly #w!!",
    },

    {
      "$n ANNHILATES $N with $s deadly #w #l!!", /* 9: > 40 */
      "You ANNHILATE $N with your deadly #w #l!!",
      "$n ANNHILATES you with $s deadly #w on your #L!!",
    },

    {
      "$n <<<ERADICATES>>> $N with $s deadly #w #l!!", /* 10: > 75 */
      "You <<<ERADICATE>>> $N with your deadly #w #l!!",
      "$n <<<ERADICATES>>> you with $s deadly #w on your #L!!",
    },

    {
      "$n >>>LIQUIFIES<<< $N with $s deadly #w #l!!", /* 11: > 90 */
      "You >>>LIQUIFY<<< $N with your deadly #w #l!!",
      "$n >>>LIQUIFIES<<< you with $s deadly #w on your #L!!",
    },

    {
      "$n does UNSPEAKABLE things to $N with $s deadly #w #l!!",  /* 12: > 100 */
      "You do UNSPEAKABLE things to $N with your deadly #w #l!!",
      "$n does UNSPEAKABLE things to you with $s deadly #w on your #L!!",
    }
  };
  assert( ch != NULL );
  assert( victim != NULL );

  w_type -= TYPE_HIT;                /* Change to base of table with text */

  /* if damage is > than 1/2th of the victim's total HPs send a special mesg */
  if (dam < (GET_MAX_HIT(victim) << 1)) {
     if (dam == 0)         msgnum = 0;
     else if (dam <= 2)    msgnum = 1;
     else if (dam <= 4)    msgnum = 2;
     else if (dam <= 6)    msgnum = 3;
     else if (dam <= 8)    msgnum = 4;
     else if (dam <= 10)    msgnum = 5;
     else if (dam <= 12)   msgnum = 6;
     else if (dam <= 16)   msgnum = 7;
     else if (dam <= 20)   msgnum = 8;
     else if (dam <= 24)   msgnum = 9;
     else if (dam <= 30)   msgnum = 10;
     else                  msgnum = 11;
  } else /* damage is greater than 1/2 of the victim's total hps! */ {
     special_text = 1;
     msgnum = 1;
     /* special hit message to onlookers */
     buf = replace_string(special_hit_text(ch, victim, w_type, dam, hitloc, 0),
	     attack_hit_text[w_type].singular, attack_hit_text[w_type].plural,
	     location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
     act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);
     /* special hit message to damager */
     if (!PRF_FLAGGED(ch, PRF_NOSPAM) || (PRF_FLAGGED(ch, PRF_NOSPAM) && dam >= 1)) {
       send_to_char(CCYEL(ch, C_CMP), ch);
       buf = replace_string(special_hit_text(ch, victim, w_type, dam, hitloc, 1),
	     attack_hit_text[w_type].singular, attack_hit_text[w_type].plural,
	     location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
       act(buf, FALSE, ch, NULL, victim, TO_CHAR);
       send_to_char(CCNRM(ch, C_CMP), ch);
     }
     /* special hit message to damagee */
     if (!PRF_FLAGGED(victim, PRF_NOSPAM) || (PRF_FLAGGED(victim, PRF_NOSPAM) && dam >= 1)) {
       send_to_char(CCRED(victim, C_CMP), victim);
       buf = replace_string(special_hit_text(ch, victim, w_type, dam, hitloc, 2),
	     attack_hit_text[w_type].singular, attack_hit_text[w_type].plural,
	     location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
       act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
       send_to_char(CCNRM(victim, C_CMP), victim);
     }
  }

  if (msgnum != 0 && !special_text) {
     /* damage message to onlookers */
     buf = replace_string(dam_weapons[msgnum].to_room,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural,
	  location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
     act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);
     /* damage message to damager */
     if (!PRF_FLAGGED(ch, PRF_NOSPAM) || (PRF_FLAGGED(ch, PRF_NOSPAM) && dam >= 1)) {
       send_to_char(CCYEL(ch, C_CMP), ch);
       buf = replace_string(dam_weapons[msgnum].to_char,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural,
	  location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
       act(buf, FALSE, ch, NULL, victim, TO_CHAR);
       send_to_char(CCNRM(ch, C_CMP), ch);
     }
     /* damage message to damagee */
     if (!PRF_FLAGGED(victim, PRF_NOSPAM) || (PRF_FLAGGED(victim, PRF_NOSPAM) && dam >= 1)) {
       send_to_char(CCRED(victim, C_CMP), victim);
       buf = replace_string(dam_weapons[msgnum].to_victim,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural,
	  location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
       act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
       send_to_char(CCNRM(victim, C_CMP), victim);
     }
  } else {
     /* miss message to onlookers */
     buf = replace_string(miss_text(ch, victim, w_type, 0),
	     attack_hit_text[w_type].singular, attack_hit_text[w_type].plural,
	     location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
     act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);
     /* miss message to damager */
     if (!PRF_FLAGGED(victim, PRF_NOSPAM) || (PRF_FLAGGED(victim, PRF_NOSPAM) && dam >= 1)) {
       send_to_char(CCYEL(ch, C_CMP), ch);
       buf = replace_string(miss_text(ch, victim, w_type, 1),
	     attack_hit_text[w_type].singular, attack_hit_text[w_type].plural,
	     location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
       act(buf, FALSE, ch, NULL, victim, TO_CHAR);
       send_to_char(CCNRM(ch, C_CMP), ch);
     }
     /* miss message to damagee */
     if (!PRF_FLAGGED(victim, PRF_NOSPAM) || (PRF_FLAGGED(victim, PRF_NOSPAM) && dam >= 1)) {
       send_to_char(CCRED(victim, C_CMP), victim);
       buf = replace_string(miss_text(ch, victim, w_type, 2),
	     attack_hit_text[w_type].singular, attack_hit_text[w_type].plural,
	     location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
       act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
       send_to_char(CCNRM(victim, C_CMP), victim);
     }
  }
}


/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
int skill_message(int dam, struct char_data * ch,
                  struct char_data * vict,
                  int attacktype)
{
  int i, j, nr;
  struct message_type *msg;
  struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);

  assert( ch != NULL );
  assert( vict != NULL );

  for (i = 0; i < MAX_MESSAGES; i++) {
    if (fight_messages[i].a_type == attacktype) {
      nr = dice(1, fight_messages[i].number_of_attacks);
      for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
	msg = msg->next;

      if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
	act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
	act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      } else if (dam != 0) {
	if (GET_POS(vict) == POS_DEAD) {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->die_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->die_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	} else {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->hit_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->hit_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->hit_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	}
      } else if (ch != vict) {  /* Dam == 0 */
	send_to_char(CCYEL(ch, C_CMP), ch);
	act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	send_to_char(CCNRM(ch, C_CMP), ch);

	send_to_char(CCRED(vict, C_CMP), vict);
	act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	send_to_char(CCNRM(vict, C_CMP), vict);

	act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      }
      return 1;
    }
  }
  return 0;
}


void damage(struct char_data * ch,
            struct char_data * victim,
            int dam, int attacktype, int hitloc)
{
  int exp;
  assert( ch != NULL );
  assert( victim != NULL );

  if ((victim == NULL) || (ch == NULL) || (victim == ch))
    return;  /* Sanity checks - everywhere! */

  if (GET_POS(victim) <= POS_DEAD) {
    log("SYSERR: Attempt to damage a corpse.");
    return;                     /* -je, 7/7/92 */
  }
  /* peaceful rooms */
  if (ch != victim && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  /* shopkeeper protection */
  if (!ok_damage_shopkeeper(ch, victim))
    return;
  /* you can do crap while incapacitated */
  if (GET_HIT(ch) <= 0)
    return;
  /* you can't damage an immort unless you're an immort or NPC */
  if (IS_IMMORT(victim) && !IS_NPC(ch) && !IS_IMMORT(ch))
    dam = 0;
  /* to make sure there are no schitzo's here */
  if (victim != ch) {
    if (GET_POS(ch) > POS_STUNNED) {
      if (!(FIGHTING(ch)))
	set_fighting(ch, victim);

      if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
	  !number(0, 10) && IS_AFFECTED(victim, AFF_CHARM) &&
	  (victim->master->in_room == ch->in_room)) {
	if (FIGHTING(ch))
	  stop_fighting(ch);
	hit(ch, victim->master, TYPE_UNDEFINED);
	return;
      }
    }
    if (GET_POS(victim) > POS_STUNNED && !FIGHTING(victim)) {
      set_fighting(victim, ch);
      if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch) &&
	  (GET_LEVEL(ch) < LVL_IMMORT))
	remember(victim, ch);
    }
  }
  if (victim->master == ch)
    stop_follower(victim);

  if (IS_AFFECTED(ch, AFF_INVISIBLE | AFF_HIDE) || \
      IS_ITEM_AFF(ch, AFFECTED_INVIS))
    appear(ch);

  check_killer(ch, victim);

  if (check_newbie(ch, victim))
    return;
  /* Make sure there's no negative damage or anything funky */
  dam = MAX(MIN((int)dam, (int)MAX_DAMAGE), 0);
  /* Make sure we hit the guy if he's incap */
  if (GET_HIT(victim) <= 0)
    dam = MAX(MIN((int)MAX_DAMAGE, (int)GET_STR(ch)), 0);
  /* Subtract the vict's hit as damage */
  GET_HIT(victim) -= dam;
  /* apply wounds = 1/16 of damage */
  if (GET_HIT(victim) < (GET_MAX_HIT(victim) >> 4))
    GET_WOUNDS(victim) += (dam >> 4);

  if (ch != victim) {
    gain_exp(ch, GET_LEVEL(victim) * dam);
    GET_EXP(victim) = MAX(1L, (long)GET_EXP(victim) - (GET_LEVEL(victim)*dam));
  }

  update_pos(victim);
  /*
   * skill_message sends a message from the messages file in lib/misc.
   * dam_message just sends a generic "You hit $n extremely hard.".
   * skill_message is preferable to dam_message because it is more
   * descriptive.
   *
   * If we are _not_ attacking with a weapon (i.e. a spell), always use
   * skill_message. If we are attacking with a weapon: If this is a miss or a
   * death blow, send a skill_message if one exists; if not, default to a
   * dam_message. Otherwise, always send a dam_message.
   */
  if (!IS_WEAPON(attacktype))
    skill_message(dam, ch, victim, attacktype);
  else {
    if (hitloc == -1)
      hitloc = hit_location(victim, number(1, 100));
    if (GET_POS(victim) == POS_DEAD || dam == 0) {
      if (!skill_message(dam, ch, victim, attacktype))
	dam_message(dam, ch, victim, attacktype, hitloc);
    } else
	dam_message(dam, ch, victim, attacktype, hitloc);
  }
  /* lets damage some of the victim's eq */
  if (dam > 0)
    damage_equipment(dam, ch, victim, hitloc);

  /* Use send_to_char -- act() doesn't send message if you are DEAD. */
  switch (GET_POS(victim)) {
  case POS_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
    break;
  case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", victim);
    break;
  case POS_STUNNED:
    act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
    break;
  case POS_DEAD:
    act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You are dead!  Sorry...\r\n", victim);
    break;

  default:                      /* >= POSITION SLEEPING */
    if (dam > (GET_MAX_HIT(victim) >> 2))
      act("That really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

    if (GET_HIT(victim) < (GET_MAX_HIT(victim) >> 2)) {
      sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
	      CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
      send_to_char(buf2, victim);
      if (MOB_FLAGGED(victim, MOB_WIMPY) && (ch != victim))
	do_flee(victim, "", 0, 0);
    }
    if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) &&
	GET_HIT(victim) < GET_WIMP_LEV(victim)) {
      send_to_char("You wimp out, and attempt to flee!\r\n", victim);
      do_flee(victim, "", 0, 0);
    }
    break;
  }

  if (!IS_NPC(victim) && !(victim->desc)) {
    do_flee(victim, "", 0, 0);
    if (!FIGHTING(victim)) {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
      GET_WAS_IN(victim) = victim->in_room;
      char_from_room(victim);
      char_to_room(victim, 0);
    }
  }
  if (!AWAKE(victim))
    if (FIGHTING(victim))
      stop_fighting(victim);

  if (GET_POS(victim) == POS_DEAD) {
    if (IS_NPC(victim) || victim->desc) {
      if (IS_AFFECTED2(ch, AFF_GROUP))
	group_gain(ch, victim);
      else {
	exp = MIN((long)max_exp_gain, (long)GET_EXP(victim));

	/* Calculate level-difference bonus */
	exp += MAX(0L, (long)(exp * MIN((int)8, (int)(GET_LEVEL(victim) - GET_LEVEL(ch)))) >> 3);
	exp =  MAX((long)exp, 1L);

	if (exp > 1)
	  send_to_char("You receive some experience points.\r\n", ch);
	else
	  send_to_char("You receive one lousy experience point.\r\n", ch);
	gain_exp(ch, exp);
	change_alignment(ch, victim);
      }
    }
    if (!IS_NPC(victim)) {
      sprintf(buf2, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch),
	      world[victim->in_room].name);
      mudlog(buf2, BRF, LVL_IMMORT, TRUE);
      if (MOB_FLAGGED(ch, MOB_MEMORY))
	forget(ch, victim);
    }
    // if you slaughter your enemy start making some body parts
    if (dam > (GET_MAX_HIT(victim) >> 2))
      create_bodypart(victim, hitloc);
    die(victim, ch);
  }
}

int check_newbie(struct char_data *ch, struct char_data *victim)
{
  assert( ch != NULL );
  assert( victim != NULL );
  
  if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_NEWBIE) && !IS_NPC(victim)) {
    if (GET_LEVEL(ch) < 5) {
      send_to_char("Silly newbie, player killing isn't for kids!\r\n",ch);
      send_to_char("What gaul that newbie had, thinking to kill you!\r\n",victim);
    } else
      send_to_char("If you wish to pkill type 'pkill'.  But this will remove your protection as well.\r\n",ch);
    stop_fighting(ch);
    stop_fighting(victim);
    return 1;
  }
  if (!IS_NPC(ch) && !IS_NPC(victim) && PLR_FLAGGED(victim, PLR_NEWBIE)) {
    if (GET_LEVEL(ch) < 5)
      send_to_char("You try to kill that annoying newbie, but divine forces prevent you!\r\n",ch);
    else
      act("You try to kill $N, but currently $E is protected by divine forces.\r\n",
          TRUE, ch, 0, victim, TO_CHAR);
    act("$n raises $s weapon to attack you, but is blocked by an invisible wall!\r\n",
        TRUE, ch, 0, victim, TO_VICT);
    act("$n raises $s weapon to attack $N, but $e is blocked by an invisible wall!\r\n",
        TRUE, ch, 0, victim, TO_ROOM);
    stop_fighting(ch);
    stop_fighting(victim);
    return 1;
  }
  return 0;
}


int check_weapon_bonus(struct  char_data *ch, int weapon_type)
{
  int percent = number(1, 101);
  assert( ch != NULL );
  
  if (IS_STAB(weapon_type) && percent < GET_SKILL(ch, SKILL_STAB))
    return (GET_LEVEL(ch) >> 4);
  if (IS_CUDGEL(weapon_type) && percent < GET_SKILL(ch, SKILL_CUDGEL))
    return (GET_LEVEL(ch) >> 4);
  if (IS_PIERCE(weapon_type) && percent < GET_SKILL(ch, SKILL_PIERCE))
    return (GET_LEVEL(ch) >> 4);
  if (IS_BLUNT(weapon_type) && percent  < GET_SKILL(ch, SKILL_BLUNT))
    return (GET_LEVEL(ch) >> 4);
  if (IS_SWORD(weapon_type) && percent  < GET_SKILL(ch, SKILL_SWORDS))
    return (GET_LEVEL(ch) >> 4);
  
  return 0;
}


void hit(struct char_data * ch, struct char_data * victim, int type)
{
  struct obj_data * wielded, * wielded2;
  int    w_type, dam, diceroll, hitloc = 0, weap_bonus = 0, poison = 0;
  struct affected_type af;

  extern struct str_app_type str_app[];
  extern struct dex_app_type dex_app[];
  int backstab_mult(int level);

  assert( ch != NULL );
  assert( victim != NULL );
  assert( victim != ch );

  wielded = GET_EQ(ch, WEAR_WIELD);
  wielded2 = GET_EQ(ch, WEAR_HOLD);
  
  if (ch->in_room != victim->in_room) {
    if (FIGHTING(ch) == victim)
      stop_fighting(ch);
    return;
  }

  if (IS_AFFECTED2(victim, AFF_FIREWALL)) {
    act("You are unable to pass through the wall to hit $M.",
        FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  /* MOBProg triggers */
  mprog_hitprcnt_trigger(ch, victim);
  mprog_fight_trigger(ch, victim);
  
  if (wielded && GET_OBJ_TYPE(wielded) != ITEM_WEAPON)
    wielded = NULL;   /* Not a weapon - can't use it */
  if (wielded2 && GET_OBJ_TYPE(wielded2)  != ITEM_WEAPON)
    wielded2 = NULL;  /*  Not a weapon - can't use it */

  if (wielded && wielded2) {
    /* Char is wielding two weapons, randomly decide which one to use */
    if (number(0, 6) >= 2)
      w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
    else
      w_type = GET_OBJ_VAL(wielded2, 3) + TYPE_HIT;
  } else if (wielded || wielded2) {
    if (wielded)
      w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
    else
      w_type = GET_OBJ_VAL(wielded2, 3) + TYPE_HIT;
  } else {
    if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))
      w_type = ch->mob_specials.attack_type + TYPE_HIT;
    else
      w_type = TYPE_HIT;
  }

  /* code to prevent newbies from being killed */
  check_newbie(ch, victim);

  /* we have w_type now check if the ch has a proficiency in that weapon */
  weap_bonus = check_weapon_bonus(ch, w_type);

  /* Now that we've gotten past all that initial shit we'll try and redo *
   * the combat system so that it works with all of the new mods for TD  */
  diceroll = number(1, 100);              /* random percentage to hit */

  diceroll += GET_HITROLL(ch);            /* Add to it % from hitroll */
  diceroll += weap_bonus;                 /* + weapon bonus           */

  /* see if diceroll is greater than victim's dexterity app or *
   * if the vict is sleeping/mortally wounded etc.             */
  if ((GET_POS(victim) >= POS_FIGHTING) &&
      (diceroll < dex_app[GET_DEX(victim)].defensive)) {
    /* diceroll is smaller AND victim is not sitting/sleeping/incap/etc. -- no hit */
     if (type == SKILL_BACKSTAB)
       damage(ch, victim, 0, SKILL_BACKSTAB, -1);
     else if (type == SKILL_STAB)
       damage(ch, victim, 0, SKILL_STAB, -1);
     else if (type == SKILL_CUDGEL)
       damage(ch, victim, 0, SKILL_CUDGEL, -1);
     else
       damage(ch, victim, 0, w_type, -1);
  } else {   /* we have a hit!! */
    /* first let's find out where we hit the person */
    do {
      hitloc = hit_location(victim, number(1, 100));
    } while (!has_bodypart(victim, hitloc));

    /* ** we have the hitlocation, now let's calculate the damage ** */
    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;

    /* check if weapon adds to damage */
    if (wielded && (w_type == (GET_OBJ_VAL(wielded, 3) + TYPE_HIT))) {
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));
      poison = GET_OBJ_VAL(wielded, 4);
    } else if (wielded2 && (w_type == (GET_OBJ_VAL(wielded2, 3) + TYPE_HIT))) {
      dam += dice(GET_OBJ_VAL(wielded2, 1), GET_OBJ_VAL(wielded2, 2));
      poison = GET_OBJ_VAL(wielded2, 4);
    }

    /* guess not -- okay is the char a mob? */
    else if (IS_NPC(ch))
      dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);

    /* guess not -- umm does the ch know about Zen Thinking? */
    else if (GET_SKILL(ch, SKILL_KARATE) >= number(10, 101))
      dam += monk_damage(ch);

    /* guess not -- well let's use the # for obj_type of the wielded obj */
    else if (wielded)
      dam += GET_OBJ_TYPE(wielded);

    /* guess not -- oh well, just use ch's fists then */
    else  dam += number(0, 2);    /* Max. 2 dam with bare hands */

    if (GET_POS(victim) < POS_FIGHTING)
      dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */

    /* Damroll determines the percentage of damage done -- atleast 1/2 */
    dam = MAX((int)dam, (int)((dam * (GET_DAMROLL(ch) + 50)) / 100));

    if (IS_AFFECTED(victim, AFF_SANCTUARY) || \
	IS_ITEM_AFF(ch, AFFECTED_SANCTUARY))
      dam >>= 1;          /* 1/2 damage when sanctuary */

    if (IS_AFFECTED2(victim, AFF_BLACKMANTLE))
      dam <<= 1;          /* double damage when black mantled */

    /* AC is now a percentage from 0 to 100 which is a factor of how much *
     * damage the armor will absorb */
    if (hitloc == 12)        /* head */
      dam -= ((dam * GET_AC(victim, ARMOR_HEAD)) / 100);
    else if (hitloc == 1)    /* left leg */
      dam -= ((dam * GET_AC(victim, ARMOR_LEG_L)) / 100);
    else if (hitloc == 2)    /* right leg */
      dam -= ((dam * GET_AC(victim, ARMOR_LEG_R)) / 100);
    else if (hitloc == 3)    /* left arm */
      dam -= ((dam * GET_AC(victim, ARMOR_ARM_L)) / 100);
    else if (hitloc == 4)    /* right arm */
      dam -= ((dam * GET_AC(victim, ARMOR_ARM_R)) / 100);
    else if (hitloc == 7 || hitloc == 8)  /* hands */
      dam -= ((dam * GET_AC(victim, ARMOR_SHIELD)) / 100);
    else
      dam -= ((dam * GET_AC(victim, ARMOR_BODY)) / 100);

    /* If the victim is very fatigued do double damage */
    if (GET_MOVE(victim) <= 5 && GET_MOVE(ch) > 2)
      dam <<= 1;
    /* If the ch is very fatigued do 1/4 damage */
    else if (GET_MOVE(ch) <= 2)
      dam >>= 2;
    /* Second half of bonus for weapon */
    dam += weap_bonus;

    /* make sure we're doing atleast 1 hp damage since we did hit */
    dam = MAX(1, (int)dam);

    /* check for poison */
    if (dam > 1 && number(0, 1) && poison) {
      switch (poison) {
	case 2:
	  af.type      = SPELL_POISON;
	  af.bitvector = AFF_POISON_II;
	  break;
	case 3:
	  af.type      = SPELL_POISON;
	  af.bitvector = AFF_POISON_III;
	  break;
        case 4:
	  af.type      = SPELL_DISEASE;
	  af.bitvector = AFF_DISEASE;
	  break;
	case 1:
	  af.type      = SPELL_POISON;
	  af.bitvector = AFF_POISON_I;
	  break;
        default:
          break;
      }
      af.duration  = GET_LEVEL(ch);  
      af.modifier  = 0;
      af.location  = APPLY_NONE;
      affect_join(victim, &af, FALSE, FALSE, FALSE, FALSE, TRUE);
    }
    /* now do the damage */
    if (type == SKILL_BACKSTAB) {
      dam *= backstab_mult(GET_LEVEL(ch));
      damage(ch, victim, dam, SKILL_BACKSTAB, hitloc);
    } else if (type == SKILL_STAB) {
      dam *= (backstab_mult(GET_LEVEL(ch)) >> 1);
      damage(ch, victim, dam, SKILL_STAB, hitloc);
    } else if (type == SKILL_CUDGEL) {
      if (number(0, 4) == 4) {
	af.bitvector = AFF_SLEEP;            
	af.duration  = dice(1, 2);
	af.type      = SPELL_SLEEP;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	affect_join(victim, &af, FALSE, FALSE, FALSE, FALSE, TRUE);
	stop_fighting(FIGHTING(victim));
        stop_fighting(victim);
	stop_fighting(FIGHTING(ch));
        stop_fighting(ch);
      } else
	damage(ch, victim, dice(GET_LEVEL(ch), GET_STR(ch)), SKILL_CUDGEL, -1);
    } else
      damage(ch, victim, dam, w_type, hitloc);
  }
  
  /* fighting takes away movement points because it is so fatiguing */
  if (GET_MOVE(ch) > 1)
    GET_MOVE(ch) -= number(0, 1);

  if ( GET_MOVE(ch) < 10 )
    send_to_char("You are very fatigued, you may want " \
                 "to flee before you collapse.\r\n", ch);
}

/* Creates a copy of the mobile */
void replicate_mob(struct char_data *ch)
{
  struct char_data *mob;
  assert( ch != NULL );

  /* NPC's ONLY */
  if (!IS_NPC(ch) || !FIGHTING(ch))
    return;
  
  if (!(number(0, 3))) {
    act("As drops of $n's blood hit the ground, " \
        "they merge together forming another copy of itself!",
        TRUE, ch, 0, 0, TO_ROOM);
    mob = read_mobile(GET_MOB_RNUM(ch), REAL);
    char_to_room(mob, ch->in_room);
    /* Automatically assist */
    hit(mob, FIGHTING(ch), TYPE_UNDEFINED);
  }
}

/* control the fights going on.  Called every 2 seconds from comm.c. */
void perform_violence(void)
{
  struct char_data *ch, *leader;
  struct follow_type *f;
  extern struct index_data *mob_index;

  /* changed so ch's don't get extra attacks all the time */
  int dblprcnt = number(50, 125);
  int triprcnt = number(75, 125);

  for (ch = combat_list; ch; ch = next_combat_list) {
    next_combat_list = ch->next_fighting;
    
    /* New code for mobs that replicate themselves! */
    if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_REPLICANT) &&
        (GET_HIT(ch) < GET_MAX_HIT(ch)))
      if (FIGHTING(ch))
	replicate_mob(ch);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_INFOBAR) && FIGHTING(ch))
      do_infobar(ch, 0, 0, SCMDB_COMBAT);

    /* If the combatants are not in the same room or don't exist stop it! */
    if (FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) {
      stop_fighting(ch);
      continue;
    }

    /* Keep ppl from beating up on themselves */
    if (ch == FIGHTING(ch)) {
      stop_fighting(ch);
      return;
    }
    /* People who are slept don't fight back. */
    if (GET_POS(ch) <= POS_SLEEPING || IS_AFFECTED2(ch, AFF_SLEEP))
      continue;
    
    /* If we have a charmed pet and the master isn't in the room FLEE */
    if (IS_NPC(ch)) {
      if (IS_AFFECTED(ch, AFF_CHARM) &&
	  (ch->master->in_room != ch->in_room))
	 do_flee(ch, "", 0, 0);

      /* Lower the mob's wait state */
      if (GET_MOB_WAIT(ch) > 0) {
	GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
	continue;
      }
      GET_MOB_WAIT(ch) = 0;

      /* Stand up if we've been knocked  down */
      if (GET_POS(ch) < POS_FIGHTING && IS_NPC(ch)) {
	GET_POS(ch) = POS_FIGHTING;
	act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
      }
    }

    /* This player misses attacks while sitting down */
    if (GET_POS(ch) < POS_FIGHTING) {
      sprintf(buf2, "%sYou can't fight while %ssitting%s%s!!%s\r\n",
           CCMAG(ch, C_SPR), CCFSH(ch, C_SPR), CCNRM(ch, C_SPR),
           CCMAG(ch, C_SPR), CCNRM(ch, C_SPR));
      send_to_char(buf2, ch);
      continue;
    }

    /* First hit of the round */
    hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    /* Fire off a special proc */
    if (MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
      (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");

    /* new additional hits by SPM for The Dominion */
    if ((FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) ||
	GET_HIT(FIGHTING(ch)) <= -1)
       return;
    else {
       if (MOB_FLAGGED(ch, MOB_SPEC) &&
           (number(25, 100) > dblprcnt) &&
           mob_index[GET_MOB_RNUM(ch)].func != NULL)
	 (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
       if (GET_SKILL(ch, SKILL_DBL_ATTACK) > dblprcnt)
	 hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    }
    /* Third attack */
    if ((FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) ||
	GET_HIT(FIGHTING(ch)) <= -1)
       return;
    else {
       if (GET_SKILL(ch, SKILL_TRI_ATTACK) > triprcnt)
	 hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
       if (MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
	 (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
    }
    /* Fourth attack -- Haste Only */
    if ((FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) ||
	GET_HIT(FIGHTING(ch)) <= -1)
       return;
    else {
       if (MOB_FLAGGED(ch, MOB_SPEC) &&
           (GET_LEVEL(ch) >= 35) &&
           mob_index[GET_MOB_RNUM(ch)].func != NULL)
	 (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
       if ((IS_AFFECTED(ch, AFF_HASTE) || IS_ITEM_AFF(ch, AFFECTED_HASTE)) || \
	   (number(1, 125) > triprcnt))
	 hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    }

    /* have mounts autoassist too */
    if (MOUNTED(ch) && (FIGHTING(ch) != MOUNTED(ch)))
      hit(MOUNTED(ch), FIGHTING(ch), TYPE_UNDEFINED);

    /* Check if player's want to autoassist as well */
    if (!IS_NPC(ch) && FIGHTING(ch) && IS_NPC(FIGHTING(ch)) && 
	IS_AFFECTED2(ch, AFF_GROUP))  {

      if (!(leader = ch->master))
	leader = ch;

      /* should the leader join? */
      if ((ch != leader) && (IN_ROOM(ch) == IN_ROOM(leader)) &&
	  (!FIGHTING(leader)) && PRF_FLAGGED(leader, PRF_AUTOASS) &&
          GET_HIT( leader ) > 1)
	do_assist(leader, GET_NAME(ch), 0, 0);

      /* what about the rest of the group? */
      for (f=leader->followers; f; f=f->next)
        if (IS_AFFECTED2(f->follower, AFF_GROUP) &&
            (IN_ROOM(ch) == IN_ROOM(f->follower)) &&
	    !FIGHTING(f->follower) && (PRF_FLAGGED(f->follower, PRF_AUTOASS) ||
                                       IS_AFFECTED(f->follower, AFF_CHARM)) &&
            GET_HIT( f->follower ) > 1)
          do_assist(f->follower, GET_NAME(ch), 0, 0);
    }
  }
}

/* Routine for determining monk damage - SPM 5/26/95 */
int monk_damage(struct char_data *ch)
{
  int damage = 0;
  assert( ch != NULL );
  
  if (GET_LEVEL(ch) <= 5)                 /* Lvls 1 to 5  */
    damage = dice(2, 5);                 /* 2 - 10 pts   */
  else if (GET_LEVEL(ch) <= 14)           /* Lvls 6 to 14 */
    damage = dice(12, 2);                /* 12 - 24 pts  */
  else if (GET_LEVEL(ch) <= 24)           /* Lvls 15 to 24*/
    damage = dice(12, 3);                /* 12 - 36 pts  */
  else if (GET_LEVEL(ch) <= 34)           /* Lvls 25 to 34*/
    damage = dice(12, 4);                /* 12 - 48 pts  */
  else if (GET_LEVEL(ch) <= 44)           /* Lvls 35 to 44*/
    damage = dice(12, 5);                /* 12 - 60 pts  */
  else                                    /* Gods Only    */
    damage = dice(12, GET_LEVEL(ch));    /* 12 - 600 pts */
  
  return (damage);
}
