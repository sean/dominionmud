/************************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SPEC_PRO.C                                   Based on CircleMUD *
*  Usage: Implementation of special procedures for mobiles/objects/rooms  *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All Modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern char *pc_race_types[];

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
int  exp_needed(int level);
void clone_char(struct char_data *, struct char_data *);
  ACMD(do_tell);
  ACMD(do_gen_door);
  ACMD(do_say);
  ACMD(do_drop);
  char *fname(char *namelist);

struct social_type {
  char *cmd;
  int next_line;
};


/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

void LowerString(char *string)
{  /* Convert string to all lower case */
   int i;

   for (i = 0; string[i] != '\0'; i++)
      if (string[i] >= 'A' && string[i] <= 'Z')
	string[i] = (string[i] - 'A') + 'a';
}


int spell_sort_info[MAX_SKILLS+1];

extern char *spells[];

void sort_spells(void)
{
  int a, b, tmp;

  /* initialize array */
  for (a = 1; a < MAX_SKILLS; a++)
    spell_sort_info[a] = a;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < MAX_SKILLS - 1; a++)
    for (b = a + 1; b < MAX_SKILLS; b++)
      if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0) {
	tmp = spell_sort_info[a];
	spell_sort_info[a] = spell_sort_info[b];
	spell_sort_info[b] = tmp;
      }
}


char *how_good(int percent)
{
  static char buf[256];

  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 10)
    strcpy(buf, " (awful)");
  else if (percent <= 20)
    strcpy(buf, " (bad)");
  else if (percent <= 40)
    strcpy(buf, " (poor)");
  else if (percent <= 55)
    strcpy(buf, " (average)");
  else if (percent <= 70)
    strcpy(buf, " (fair)");
  else if (percent <= 80)
    strcpy(buf, " (good)");
  else if (percent <= 85)
    strcpy(buf, " (very good)");
  else
    strcpy(buf, " (superb)");

  return (buf);
}

char *prac_types[] = {
  "spell",
  "skill"
};


int max_learn(int skill, struct char_data *ch)
{
   int i = 0;
   extern struct spell_info_type spell_info[];

   if (spell_info[skill].sphere == SPHERE_PRAYER ||
       skill == SPHERE_PRAYER ||
       spell_info[skill].sphere == SPHERE_ACTS_OF_DEITIES ||
       spell_info[skill].sphere == SPHERE_AFFLICTIONS ||
       spell_info[skill].sphere == SPHERE_CURES ||
       spell_info[skill].sphere == SPHERE_HAND_OF_DEITIES ||
       spell_info[skill].sphere == SPHERE_FAITH)
      i = ((GET_WIS(ch) + GET_WILL(ch)) / 2);
   else if (skill == SPHERE_ARCANE ||
       spell_info[skill].sphere == SPHERE_ARCANE ||
       spell_info[skill].sphere == SPHERE_CONJURATION ||
       spell_info[skill].sphere == SPHERE_DIVINATION ||
       spell_info[skill].sphere == SPHERE_ENCHANTMENT ||
       spell_info[skill].sphere == SPHERE_ENTROPIC ||
       spell_info[skill].sphere == SPHERE_INVOCATION ||
       spell_info[skill].sphere == SPHERE_PHANTASMIC)
      i = ((GET_INT(ch) + GET_WIS(ch)) / 2);
   else if (skill == SPHERE_GTHIEVERY ||
       spell_info[skill].sphere == SPHERE_GTHIEVERY ||
       spell_info[skill].sphere == SPHERE_MURDER ||
       spell_info[skill].sphere == SPHERE_LOOTING ||
       spell_info[skill].sphere == SPHERE_THIEVING)
      i = ((GET_DEX(ch) + GET_INT(ch)) / 2);
   else if (skill == SPHERE_COMBAT ||
       spell_info[skill].sphere == SPHERE_COMBAT ||
       spell_info[skill].sphere == SPHERE_WEAPONS ||
       spell_info[skill].sphere == SPHERE_HAND_TO_HAND ||
       spell_info[skill].sphere == SPHERE_MARTIAL_ARTS)
      i = ((GET_STR(ch) + GET_DEX(ch)) / 2);
   else
      i = ((GET_WIS(ch) + GET_DEX(ch)) / 2);

   if (i >= 20)
     i = 100;
   else if (i >= 18)
     i = 95;
   else if (i >= 17)
     i = 90;
   else if (i >= 16)
     i = 85;
   else if (i >= 14)
     i = 80;
   else if (i >= 12)
     i = 75;
   else if (i >= 10)
     i = 65;
   else if (i >= 8)
     i = 55;
   else
     i = 50;

   if (skill <= 50)
     return i;
   else
     return (MIN(i, (int)GET_SKILL(ch, (int)(spell_info[(int)skill].sphere)))); 
}

void list_skills(struct char_data * ch, int sphere)
{
  extern char   *spells[];
  extern struct spell_info_type spell_info[];
  int i, sortpos, group;

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf, "%sYou know of the following disciplines:\r\n\r\n", buf);
  strcpy(buf2, buf);

  /* if the player sends us a spell as the sphere get its sphere */
  if (sphere > MAX_SPHERES)
    sphere = spell_info[sphere].sphere;

  if (sphere == -1) {
    for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++)  {
      i = spell_sort_info[sortpos];
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
	strcat(buf2, "**OVERFLOW**\r\n");
	break;
      }
      if ((i > 0 && i <= MAIN_SPHERES) && !(!strcmp(spells[i], "!UNUSED!"))) {
	sprintf(buf, "%24s : (Learnedness: %3d)\t\tMax Learnedness: %-3d\r\n",
		spells[i], GET_SKILL(ch, i), max_learn(i, ch));
	strcat(buf2, buf);
      }
    }
  } else if ((sphere == 0) && IS_IMMORTAL(ch)) { /* gods only */
    sphere = SPHERE_NONE;
    for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++)  {
      i = spell_sort_info[sortpos];
      /* only print the skills in the unused sphere */
      if (spell_info[i].sphere != sphere)
	continue;
      if (i <= MAX_SPHERES)
	continue;
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
	strcat(buf2, "**OVERFLOW**\r\n");
	break;
      }
      if ((spell_info[i].sphere == sphere) && !(!strcmp(spells[i], "!UNUSED!"))) {
	sprintf(buf, "%24s : (Learnedness: %3d)\r\n",
		spells[i], GET_SKILL(ch, i));
	strcat(buf2, buf);
      }
    }
  } else
    for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
      i = spell_sort_info[sortpos];

      /* only print the skills in the sphere asked for */
      if (spell_info[i].sphere != sphere)
	continue;

      if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
	strcat(buf2, "**OVERFLOW**\r\n");
	break;
      }
      /* find out which sphere this skill belongs to */
      group = spell_info[i].sphere;
      if ((GET_SKILL(ch, group) >= spell_info[i].percent) &&
	  (spell_info[i].sphere == group)) {
	if (!(!strcmp(spells[i], "!UNUSED!"))) {
	  if (i > MAX_SPHERES)
	    sprintf(buf, "%-40s %s\r\n", spells[i], how_good(GET_SKILL(ch, i)));
	  else
	    sprintf(buf, "%24s : (Learnedness: %3d)\r\n", spells[i], GET_SKILL(ch, i));
	  strcat(buf2, buf);
	}
      }
    }
  /* end of else */
  page_string(ch->desc, buf2, 1);
}


SPECIAL(guild)
{
  int num_lvls = 0, immortal = FALSE;
  struct char_data *gm = (struct char_data *) me;

  if (IS_NPC(ch))
    return 0;

  skip_spaces(&argument);

  if (!CMD_IS("gain"))
    return FALSE;
  else {
    if ((GET_EXP(ch) >= exp_needed(GET_LEVEL(ch) + 1)) &&
	 (GET_LEVEL(ch) < MAX_MORT_LEVEL)) {
      while ((GET_EXP(ch) >= exp_needed(GET_LEVEL(ch) + 1)) &&
	       (GET_LEVEL(ch) < MAX_MORT_LEVEL)) {
	num_lvls++;
	GET_LEVEL(ch) += 1;
	if (GET_LEVEL(ch) >= LVL_IMMORT)
	  immortal = TRUE;
	advance_level(ch);
      }
      if ((num_lvls > 1) && !immortal)
	sprintf(buf, "%sYou rise %s%d%s levels!%s\r\n",  CCBMG(ch, C_SPR),
		   CCYEL(ch, C_SPR), num_lvls, CCBMG(ch, C_SPR), CCNRM(ch, C_SPR));
      else if (immortal)
	sprintf(buf, "%sYou rise %s%d%s level%s, and are now an %s%sIMMORTAL%s!%s\r\n",
		   CCBMG(ch, C_SPR), CCYEL(ch, C_SPR), num_lvls, CCBMG(ch, C_SPR),
		   (num_lvls > 1 ? "s" : ""), CCFSH(ch, C_SPR), CCYEL(ch, C_SPR),
		   CCBMG(ch, C_SPR), CCNRM(ch, C_SPR));
      else
	sprintf(buf, "%sYou rise a level!%s\r\n", CCBMG(ch, C_SPR), CCNRM(ch, C_SPR));
      send_to_char(buf, ch);
    } else { /* Cannot level at this time */
      sprintf(buf2, "Sorry, %s, you are unable to become more experienced at this time.", GET_NAME(ch));
      do_say(gm, buf2, 0, 0);
    }
    return TRUE;
  }
  return FALSE;
}


/* return a number from 1 to 4 based on how well the PC can learn the skill */
int learn_amount(int skill, struct char_data *ch)
{
   int i;
   extern struct spell_info_type spell_info[];

   if (IS_PRAYER(skill))
     i = ((GET_WIS(ch) + GET_WILL(ch)) >> 1);
   else if (IS_ARCANE(skill))
     i = ((GET_INT(ch) + GET_WIS(ch)) >> 1);
   else if (IS_THIEVERY(skill))
     i = ((GET_DEX(ch) + GET_INT(ch)) >> 1);
   else if (IS_COMBAT(skill))
     i = ((GET_STR(ch) + GET_DEX(ch)) >> 1);
   else
     i = ((GET_WIS(ch) + GET_DEX(ch)) >> 1);

   if (i >= 18)
     return 6;
   else if (i >= 16)
     return 5;
   else if (i >= 14)
     return 4;
   else if (i >= 12)
     return 3;
   else if (i >= 10)
     return 2;
   else
     return 1;
}

struct training_info {
     mob_num  my_num;
     int      sphere;
} trainers[] = {
     {3023,  SPHERE_COMBAT},
     {3020,  SPHERE_ARCANE},
     {3021,  SPHERE_PRAYER},
     {3022,  SPHERE_GTHIEVERY},
     {3099,  SPHERE_GENERAL},
     {10019, SPHERE_MURDER},
     {10012, SPHERE_THIEVING},
     {3090,  SPHERE_CURES},
     {32003, SPHERE_CONJURATION},
     {6419,  SPHERE_MARTIAL_ARTS},
     {19017, SPHERE_WEAPONS},
     {1810,  SPHERE_HAND_TO_HAND},
     {25501, SPHERE_HAND_OF_DEITIES},
     {6607,  SPHERE_ACTS_OF_DEITIES},
     {1803,  SPHERE_ENCHANTMENT},
     {3092,  SPHERE_DIVINATION},
     {3041,  SPHERE_ENTROPIC},
     {3042,  SPHERE_INVOCATION},
     {3043,  SPHERE_PHANTASMIC},
     {1813,  SPHERE_AFFLICTIONS},  
     {8009,  SPHERE_MURDER},
     {1115,  SPHERE_LOOTING},
     {-1, -1}
  };

/* Prints the trainer's vnum and what he/she trains in */
void show_guilds(struct char_data *ch)
{
   int i = 0;
   rnum r_num;
   struct char_data *mob;
   extern char *spells[];

   strcpy(buf, "  Vnum    Name                     Sphere\r\n");
   sprintf(buf, "%s~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n", buf);
   while (trainers[i].my_num != -1) {
      if ((r_num = real_mobile(trainers[i].my_num)) < 0) {
	sprintf(buf, "%s[%-7ld] -- %s\r\n", buf, trainers[i].my_num, 
		spells[(trainers[i].sphere)]);
      } else {
	mob = read_mobile(r_num, REAL);
	char_to_room(mob, ch->in_room);
	sprintf(buf, "%s[%-7ld] %-21.21s -- %s\r\n", buf, 
	      trainers[i].my_num, GET_NAME(mob),
	      spells[(trainers[i].sphere)]);
	extract_char(mob);
      }
      ++i;
   }
   page_string(ch->desc, buf, 1);
}

/* Check to see if the player learns a little in the sub skills */
void increase_sub_skills(struct char_data *ch, int skill_num)
{
  extern struct spell_info_type spell_info[];
  int i, sortpos;

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++)  {
     i = spell_sort_info[sortpos];
     if (spell_info[i].sphere == skill_num)
       if (GET_SKILL(ch, skill_num) - spell_info[i].percent > 25)
	 GET_SKILL(ch, i) = MIN(100, GET_SKILL(ch, i) + number(1, 2));
	 /* PC has learned a bit in a sub-skill */
  }
  /* now I just need to make a function so that players can learn thru use */
}

#define LEARNED(ch, skill)  (MIN(100, max_learn(skill, ch)))

/* Trainer - Each trainer can only do ONE sphere, and it's sub-spheres */
SPECIAL(trainer)
{
  int    i = 0;
  struct char_data *tm = (struct char_data *) me;
  void practice(struct char_data *ch, struct char_data *gm, char *argument, int teaches);

  if (IS_NPC(ch) || !CMD_IS("practice"))
    return 0;

  for (i = 0; trainers[i].my_num != -1; i++)
    if (trainers[i].my_num == GET_MOB_VNUM(tm))
      break;

  if (CMD_IS("practice")) {
    if (!CAN_SEE(tm, ch))
      act("$N says, 'I don't deal with people I can't see!'", FALSE, ch, NULL, tm, TO_CHAR);
    else
      practice(ch, tm, argument, trainers[i].sphere);
    return TRUE;
  }
  return FALSE;
}


char *god_sexes[] = {
   "None",
   "His",         /* Glas    */
   "His",         /* Malachi */
   "Her",         /* Gaeya   */
   "His",         /* Garou   */
   "His",         /* Landrin */
   "His",         /* sb      */
   "Her",         /* Cair    */
   "His",         /* Kirahn  */
   "His",         /* Duvel   */
   "\n"
};

struct {
  long room;
  int  religion;
  char *godname;
  int  spell1;
  int  spell2;
  int  spell3;
} temple_array[] = {
/*
  {20, REL_GLASGIAN, "glasgian", SPELL_SANCTUARY, SPELL_ARMOR, SPELL_SOUL_SEARCH},
  {10110, REL_KIRAHN, "kirahn",  SPELL_STRENGTH, SPELL_MIN_GLOBE, SPELL_SOUL_SEARCH},
  {25567, REL_GAROU, "garou",    SPELL_STRENGTH, SPELL_HASTE, SPELL_INFRAVISION},
*/
  {-1, -1, NULL, -1, -1, -1}
};


SPECIAL(temple)
{
  int i = 0;
  struct affected_type af;
  extern char *pc_religion_types[];

  if (!CMD_IS("worship") || !*argument)
     return FALSE;
  else {
  /* All of the worship code fits snugly in here ;-) */
     skip_spaces(&argument);

     for (i = 0; temple_array[i].room != -1; ++i)
	if (ch->in_room == temple_array[i].room)
	  break;
     /* Guess the ch isn't in the right room ... */
     if (temple_array[i].room == -1)
       return FALSE;
     /* We got the room right, let's boogy! */
     LowerString(argument);
     if (strstr(argument, temple_array[i].godname)) {
       sprintf(buf, "You kneel down and begin your prayer to %s.\r\n",
	       pc_religion_types[temple_array[i].religion]);
       send_to_char(buf, ch);
       if (GET_REL(ch) != temple_array[i].religion) {
	  sprintf(buf, "$n kneels down and utters a prayer to %s.",
		 pc_religion_types[temple_array[i].religion]);
	  act(buf, TRUE, ch, 0, 0, TO_ROOM);
	  return TRUE;
       }
       sprintf(buf, "$n kneels down and utters a prayer to $s god, %s.",
	       pc_religion_types[temple_array[i].religion]);
       act(buf, TRUE, ch, 0, 0, TO_ROOM);
       if (IS_AFFECTED(ch, AFF_RELIGED) && (GET_REL(ch) == temple_array[i].religion)) {
	 sprintf(buf, "%s has already shone %s thanks upon you already this day.",
		 pc_religion_types[temple_array[i].religion], god_sexes[temple_array[i].religion]);
	 send_to_char(buf, ch);
       } else {
	 if (temple_array[i].spell1 != -1)
	   call_magic(ch, ch, 0, temple_array[i].spell1, 50, CAST_SPELL);
	 if (temple_array[i].spell2 != -1)
	   call_magic(ch, ch, 0, temple_array[i].spell2, 50, CAST_SPELL);
	 if (temple_array[i].spell3 != -1)
	   call_magic(ch, ch, 0, temple_array[i].spell3, 50, CAST_SPELL);
	 af.type      = SPELL_BLESS;
	 af.duration  = 24;
	 af.location  = APPLY_HITROLL;
	 af.modifier  = 1;
	 af.bitvector = AFF_RELIGED;
	 affect_join(ch, &af, TRUE, FALSE, FALSE, FALSE, TRUE);
	 sprintf(buf, "%s has smiled upon you this day.",
		 pc_religion_types[temple_array[i].religion]);
	 send_to_char(buf, ch);
       }
       return TRUE;
     }
     return FALSE;
  }
}



SPECIAL(lose_mist)
{
  struct obj_data *k;
  int value = 0;

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p is lost in the mist!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }

  if (!CMD_IS("drop"))
    return 0;

  do_drop(ch, argument, cmd, 0);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p is lost in the mist!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  return 1;
}


SPECIAL(frost_shatter)
{
  struct obj_data *k;
  int value = 0;

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p shatters as it hits the ground!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }

  if (!CMD_IS("drop"))
    return 0;

  do_drop(ch, argument, cmd, 0);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("It is so cold that $p shatters as it hits the ground!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  return 1;
}



SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }

  if (!CMD_IS("drop"))
    return 0;

  do_drop(ch, argument, cmd, 0);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  if (value) {
    act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

    if (GET_LEVEL(ch) < 3)
      gain_exp(ch, value);
    else
      GET_GOLD(ch) += value;
  }
  return 1;
}

struct {
   long vnum;
   char *word;
   char *mesg;
   long loaditem;
} say_specials[] = {
   {32215, "telchontar", "The fire flares brightly, illuminating every corner of the room.\r\n", 32025},
   {-1, NULL, NULL, -1}
};


SPECIAL(say_special_word)
{
   int i;
   struct obj_data *obj;
   char *compstr;

   skip_spaces(&argument);

   if ((!CMD_IS("say") && !CMD_IS("'")) || !*argument)
      return FALSE;
   else {
      compstr = str_dup(argument);
      LowerString(compstr);
      /* find the correct room */
      for (i = 0; say_specials[i].vnum != -1; i++)
	  if (say_specials[i].vnum == world[ch->in_room].number)
	     break;
      if (say_specials[i].vnum == -1)
	 return (FALSE);
      /* we should have the right room now */
      if (strstr(compstr, say_specials[i].word)) {
	 act("$n whispers some words of magic.", FALSE, ch, 0, 0, TO_ROOM);
	 sprintf(buf, "You say, '%s'\r\n", argument);
	 send_to_char(buf, ch);
	 send_to_room(say_specials[i].mesg, ch->in_room);
	 if (say_specials[i].loaditem != -1) {
	    obj = read_object(say_specials[i].loaditem, VNUMBER);
	    obj_to_room(obj, ch->in_room);
	 }
	 /* dont forget to free the memory! */
	 free(compstr);
	 return (TRUE);
      }
      return (FALSE);
   }
   return (FALSE);
}


SPECIAL(smithy)
{
  int    price = 0, i;
  struct char_data *smith = (struct char_data *) me;
  struct obj_data  *obj;
  extern struct material_type const material_list[];

  if (!CMD_IS("repair") && !CMD_IS("appraise") && !CMD_IS("sell"))
    return FALSE;
  else {
    if (!CAN_SEE(smith, ch)) {
      send_to_char("The smith says, 'I only deal with people I can see!'\r\n", ch);
      return TRUE;
    }
    if (!*argument)
      return FALSE;
    skip_spaces(&argument);
    if ((obj = get_obj_in_list_vis(ch, argument, ch->carrying)) == NULL) {
      for (i = 0; !obj && i < NUM_WEARS; i++) {
	if (GET_EQ(ch, i) && !str_cmp(argument, GET_EQ(ch, i)->name)) 
	  obj = GET_EQ(ch, i);
      }
    }

    if (!obj) {
      sprintf(buf2, "You don't seem to have %s %s.", 
	    XANA(argument), argument);
      do_say(smith, buf2, 0, 0);
      return TRUE;
    }
    if (obj)
      price = (GET_OBJ_VAL(obj, 9) * material_list[GET_OBJ_VAL(obj, 7)].value_mod);

    if (obj && (GET_OBJ_TYPE(obj) != ITEM_WEAPON &&
		GET_OBJ_TYPE(obj) != ITEM_ARMOR)) {
      sprintf(buf2, "I only repair weapons and armor, take that piece of trash elsewhere.");
      do_say(smith, buf2, 0, 0);
      return TRUE;
    }

    if (obj && ((material_list[GET_OBJ_VAL(obj, 7)].sturdiness == -1) ||
		GET_OBJ_VAL(obj, 9) <= 0)) {
      sprintf(buf2, "I'm a blacksmith, I fix things and only buy raw materials, what would I want with that?");
      do_say(smith, buf2, 0, 0);
      return TRUE;
    }

    if (obj && CMD_IS("repair")) {
      if (GET_OBJ_VAL(obj, 9) > material_list[GET_OBJ_VAL(obj, 7)].sturdiness) {
	sprintf(buf2, "What IS that thing?  There is no way I can repair it.");
	do_say(smith, buf2, 0, 0);
	sprintf(buf2, "But I may be interested in buying it for scrap.");
	do_say(smith, buf2, 0, 0);
	return TRUE;
      }

      sprintf(buf2, "Okay, that will be %d gold pieces.",
	      price);
      do_say(smith, buf2, 0, 0);
      if (GET_GOLD(ch) < price)  {
	sprintf(buf2, "Which I see you can't afford.");
	do_say(smith, buf2, 0, 0);
	return TRUE;
      } else {
	act("$n takes $o and begins repairing it.", FALSE, smith, obj, NULL, TO_ROOM);
	GET_GOLD(ch) -= price;
	GET_OBJ_VAL(obj, 9) = 0;
	sprintf(buf2, "Thank you, and here is your %s back, good as new.",
	        obj->name);
	do_say(smith, buf2, 0, 0);
        if (IS_OBJ_STAT(obj, ITEM_BROKEN))
          REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_BROKEN);
	return TRUE;
      }
    } else if (obj && CMD_IS("appraise")) {
      if (GET_OBJ_VAL(obj, 9) > material_list[GET_OBJ_VAL(obj, 7)].sturdiness) {
	sprintf(buf2, "What IS that thing?  There is no way I can repair it.");
	do_say(smith, buf2, 0, 0);
	sprintf(buf2, "But I'll give you %d gp for the scraps.",
		(material_list[GET_OBJ_VAL(obj, 7)].value_mod >> 1));
	do_say(smith, buf2, 0, 0);
	return TRUE;
      }
      sprintf(buf2, "I can fix that for %d gold pieces.",
	      price);
      do_say(smith, buf2, 0, 0);
    } else if (obj && CMD_IS("sell")) {
      sprintf(buf2, "Thanks.  Here's %d gold pieces for it.",
	      (material_list[GET_OBJ_VAL(obj,  7)].value_mod >> 1));
      do_say(smith, buf2, 0, 0);
      obj_from_char(obj);
      extract_obj(obj);
      GET_GOLD(ch) += (material_list[GET_OBJ_VAL(obj, 7)].value_mod >> 1);
    } else {
      if (!obj)
      send_to_char("The smith says, 'Which item did you want me to look at?'\r\n", ch);
      else
      send_to_char("The smith says, 'Did you want me to appraise, repair, or buy something?'\r\n", ch);
    }
    return TRUE;
  }
  return FALSE;
}

/* This and the other tardis should be made into one special with an array *
 * to figure out what each one does */
SPECIAL(tardis)
{
  char buf[MAX_STRING_LENGTH];
  int i;
  struct room_data *tardis_room = &world[ch->in_room];
  struct char_data *who;

  for(who = tardis_room->people; who; who = who->next_in_room) {
      if(GET_MOB_VNUM(who) == CHRONO_VNUM)
       break;
  }

  if (CMD_IS("list")) {
      if(!who) {
       send_to_char("The Chronomancer appears to be out.\r\n", ch);
       return(TRUE);
      }
      send_to_char("Available Temporal Zones:\r\n", ch);
      for(i = 0; i < NUM_OF_DIRS; i++) {
       if (!EXIT(who, i) || EXIT(who, i)->to_room == NOWHERE)
	continue;
       if (IS_SET(EXIT(who,i)->exit_info, EX_ISDOOR))
	continue;
       if (!IS_SET(EXIT(who,i)->exit_info, EX_LOCKED))
	continue;
       sprintf(buf, "%5d gp - %s\r\n", 5000, EXIT(who,i)->keyword);
       send_to_char(buf, ch);
      }
      return (TRUE);
  } else if (CMD_IS("buy")) {
      if(!who) {
       send_to_char("The Chronomancer appears to be out.\r\n", ch);
       return(TRUE);
      }
      argument = one_argument(argument, buf);

      for(i = 0; i < NUM_OF_DIRS; i++) {
       if (!EXIT(who, i) || EXIT(who, i)->to_room == NOWHERE)
	continue;
       if (IS_SET(EXIT(who,i)->exit_info, EX_ISDOOR))
	continue;
       if (!IS_SET(EXIT(who,i)->exit_info, EX_LOCKED))
	continue;
       if(is_abbrev(buf, EXIT(who, i)->keyword))
	break;
      }
      if(i == NUM_OF_DIRS) {
       send_to_char("The Chronomancer arches a brow: '*That* is not for sale here.'\r\n", ch);
       return (TRUE);
      }

      if (GET_GOLD(ch) < 5000) {
       send_to_char("The Chronomancer tells you: 'Come back when you have 5,000 gp.'\r\n", ch);
       return (TRUE);
      }
      GET_GOLD(ch) -= 5000;

      send_to_char("The Chronomancer accepts your gold and ushers you out...\r\n", ch);
      act("The Chronomancer accepts $N's gold and ushers $M out.",
       TRUE, who, 0, ch, TO_NOTVICT);
      char_from_room(ch);
      char_to_room(ch, EXIT(who,i)->to_room);
      look_at_room(ch, 0);
      act("$n enters the room from a shimmering portal.",
       TRUE, ch, 0, ch, TO_NOTVICT);

      return (TRUE);
  }
  /* All commands except list and buy */
  return (FALSE);
}

SPECIAL(tardis2)
{
  char buf[MAX_STRING_LENGTH];
  int i;
  struct room_data *tardis_room = &world[ch->in_room];
  struct char_data *who;

  for(who = tardis_room->people; who; who = who->next_in_room) {
      if(GET_MOB_VNUM(who) == CHRONO_VNUM2)
       break;
  }

  if (CMD_IS("list")) {
      if(!who) {
       send_to_char("The Chronomancer appears to be out.\r\n", ch);
       return(TRUE);
      }
      send_to_char("Available Temporal Zones:\r\n", ch);
      for(i = 0; i < NUM_OF_DIRS; i++) {
       if (!EXIT(who, i) || EXIT(who, i)->to_room == NOWHERE)
	continue;
       if (IS_SET(EXIT(who,i)->exit_info, EX_ISDOOR))
	continue;
       if (!IS_SET(EXIT(who,i)->exit_info, EX_LOCKED))
	continue;
       sprintf(buf, "%2d pp - %s\r\n", 20, EXIT(who,i)->keyword);
       send_to_char(buf, ch);
      }
      return (TRUE);
  } else if (CMD_IS("buy")) {
      if(!who) {
       send_to_char("The Chronomancer appears to be out.\r\n", ch);
       return(TRUE);
      }
      argument = one_argument(argument, buf);

      for(i = 0; i < NUM_OF_DIRS; i++) {
       if (!EXIT(who, i) || EXIT(who, i)->to_room == NOWHERE)
	continue;
       if (IS_SET(EXIT(who,i)->exit_info, EX_ISDOOR))
	continue;
       if (!IS_SET(EXIT(who,i)->exit_info, EX_LOCKED))
	continue;
       if (is_abbrev(buf, EXIT(who, i)->keyword))
	break;
      }
      if(i == NUM_OF_DIRS) {
       send_to_char("The Chronomancer arches a brow: '*That* is not for sale here.'\r\n", ch);
       return (TRUE);
      }

      if (GET_GOLD(ch) < 20000) {
       send_to_char("The Chronomancer tells you: 'Come back when you have 20,000 gp.'\r\n",ch);
       return (TRUE);
      }
      GET_GOLD(ch) -= 20000;

      send_to_char("The Chronomancer accepts your gold and ushers you out...\r\n", ch);
      act("The Chronomancer accepts $N's gold and ushers $M out.",
       TRUE, who, 0, ch, TO_NOTVICT);
      char_from_room(ch);
      char_to_room(ch, EXIT(who,i)->to_room);
      look_at_room(ch, 0);
      act("$n enters the room from a shimmering portal.",
       TRUE, ch, 0, ch, TO_NOTVICT);

      return (TRUE);
  }
  /* All commands except list and buy */
  return (FALSE);
}


#define GAME_NONE          -1
#define GAME_SLOTS          0
#define GAME_HIGH_DICE      1
#define GAME_TRIPLES        2
#define GAME_SEVEN          3
#define GAME_CRAPS          4
#define GAME_BLACKJACK      5

#define DEALER_HIGH_DICE_VNUM   3089
#define DEALER_TRIPLES_VNUM     3089
#define DEALER_SEVEN_VNUM       3089
#define DEALER_CRAPS_VNUM       3089
#define DEALER_BLACKJACK_VNUM   3089

#define SLOT_MACHINE_VNUM       1899

char *WHAT_GAME =
     "Valid games are:\r\n"
     "      Slots     (Max bet = 1)\r\n"
     "      High Dice (Max bet = 5000)\r\n"
     "      Triples   (Max bet = 5000)\r\n"
     "      Seven     (Max bet = 5000)\r\n"
     "      Craps     (Max bet = 10000)\r\n"
     "      Blackjack (Max bet = 25000)\r\n"
     "\r\n";

#if 0
/* This needs to be fixed up alot and new games added! Check out the *
 * casino games on BarrenRealms MUD thats where I got the idea from  */
SPECIAL(casino)
{
    struct char_data *dealer = NULL, *who;
    struct room_data *casino = &world[ch->in_room];
    struct obj_data  *obj;
    char   game_name[MAX_STRING_LENGTH], bet_name[MAX_STRING_LENGTH],
	   guess[MAX_STRING_LENGTH];
    int    game_type = -1, bet = 1;
    bool   found = FALSE;

    /* func prototypes from gamble.c */
    void play_high_dice(struct char_data *ch, struct char_data *dealer, int bet);
    void play_triples(struct char_data *ch, struct char_data *dealer, char *guess, int bet);
    void play_seven(struct char_data *ch, struct char_data *dealer, char *guess, int bet);
    void play_craps(struct char_data *ch, struct char_data *dealer, int bet);
    void play_blackjack(struct char_data *ch, struct char_data *dealer, char *guess);
    void play_slots(struct char_data *ch); /* bet always = 1 */

    /* let all other commands than play slip by */
    if (!CMD_IS("play"))
       return (FALSE);
    else {
       argument = three_arguments(argument, game_name, guess, bet_name);

       if (!*game_name) {
	  send_to_char(WHAT_GAME, ch);
	  return (TRUE);
       } else if (is_abbrev(game_name, "slots"))
	  game_type = GAME_SLOTS;
       else if (is_abbrev(game_name, "high dice"))
	  game_type = GAME_HIGH_DICE;
       else if (is_abbrev(game_name, "triples"))
	  game_type = GAME_TRIPLES;
       else if (is_abbrev(game_name, "seven"))
	  game_type = GAME_SEVEN;
       else if (is_abbrev(game_name, "craps"))
	  game_type = GAME_CRAPS;
       else if (is_abbrev(game_name, "blackjack"))
	  game_type = GAME_BLACKJACK;
       else {
	  for (who = casino->people; who; who = who->next_in_room) {
	      if (MOB_FLAGGED(who, MOB_DEALER))
		 break;
	  }
	  act("$n says, 'Sorry $N, but I don't know how to play that game.",
	     FALSE, who, 0, ch, TO_ROOM);
	  act("$n tells you, 'You may want to bring it up with the manager.",
	     FALSE, who, 0, ch, TO_VICT);
	  return (TRUE);
       }
       if (game_type != GAME_SLOTS) {
	  for (who = casino->people; who; who = who->next_in_room) {
	      if (MOB_FLAGGED(who, MOB_DEALER))
		 switch (game_type) {
		    case GAME_HIGH_DICE:
		      if (GET_MOB_VNUM(who) == DEALER_HIGH_DICE_VNUM)
			 found = TRUE;
		      if (!*guess) {
			 send_to_char("To play high dice type: play high dice <# from 1 to 5000>\r\n", ch);
			 return (FALSE);
		      } else
			 bet = atoi(guess);
		      break;
		    case GAME_TRIPLES:
		      if (GET_MOB_VNUM(who) == DEALER_TRIPLES_VNUM)
			 found = TRUE;
		      if (!*bet_name || !*guess) {
			 send_to_char("To play triples type: play triples <upper/lower/triple> <# from 1 to 5000>\r\n", ch);
			 return (FALSE);
		      } else
			 bet = atoi(bet_name);
		      break;
		    case GAME_SEVEN:
		      if (GET_MOB_VNUM(who) == DEALER_SEVEN_VNUM)
			 found = TRUE;
		      if (!*bet_name || !*guess) {
			 send_to_char("To play seven type: play seven <under/over/seven> <# from 1 to 5000>\r\n", ch);
			 return (FALSE);
		      } else
			 bet = atoi(bet_name);
		      break;
		    case GAME_CRAPS:
		      if (GET_MOB_VNUM(who) == DEALER_CRAPS_VNUM)
			 found = TRUE;
		      if (!*guess) {
			 send_to_char("To play craps type: play craps <# from 1 to 10000>\r\n", ch);
			 return (FALSE);
		      } else
			 bet = atoi(guess);
		      break;
		    case GAME_BLACKJACK:
		      if (GET_MOB_VNUM(who) == DEALER_BLACKJACK_VNUM)
			 found = TRUE;
		      if (!*guess) {
			 send_to_char("To play blackjack type: play blackjack <# from 1 to 25000>\r\n", ch);
			 return (FALSE);
		      }
		      break;
		    default:
		      found = FALSE;
		      break;
		 }
		 if (found) {
		    dealer = who;
		    break;    /* we found our dealer now save him */
		 }
	  }
	  /* Now call the proper function from gamble.c */
	  switch (game_type) {
	     case GAME_HIGH_DICE:
	       play_high_dice(ch, dealer, bet);
	       break;
	     case GAME_TRIPLES:
	       play_triples(ch, dealer, guess, bet);
	       break;
	     case GAME_SEVEN:
	       play_seven(ch, dealer, guess, bet);
	       break;
	     case GAME_CRAPS:
	       play_craps(ch, dealer, bet);
	       break;
	     case GAME_BLACKJACK:
	       play_blackjack(ch, dealer, guess);
	       break;
	     default:
	       break;
	  }
       /* we must be playing slots - bet always = 1 */
       } else {
	 found = FALSE;
	 for (obj = casino->contents; obj; obj = obj->next)
	     if (GET_OBJ_VNUM(obj) == SLOT_MACHINE_VNUM) {
		found = TRUE;
		play_slots(ch);
		break;
	     }
	 if (!found)
	    send_to_char("There doesn't seem to be a slot machine around for you to play.\r\n", ch);
       }
    }
    return (TRUE);
}
#endif


/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */




SPECIAL(mayor)
{
  static char open_path[] =
  "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
  "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;
  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  switch (path[index]) {
  case '0':
  case '1':
  case '2':
  case '3':
    perform_move(ch, path[index] - '0', 1);
    break;

  case 'W':
    GET_POS(ch) = POS_STANDING;
    act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POS_SLEEPING;
    act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'a':
    act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'b':
    act("$n says 'What a view!  I must get something done about that dump!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'c':
    act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'd':
    act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'e':
    act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'E':
    act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'O':
    do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
    do_gen_door(ch, "gate", 0, SCMD_OPEN);
    break;

  case 'C':
    do_gen_door(ch, "gate", 0, SCMD_CLOSE);
    do_gen_door(ch, "gate", 0, SCMD_LOCK);
    break;

  case '.':
    move = FALSE;
    break;

  }

  index++;
  return FALSE;
}


void npc_steal(struct char_data * ch, struct char_data * victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LVL_IMMORT)
    return;

  if (AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  } /* make it so that they steal eq as well */
}


SPECIAL(snake)
{
  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, 42 - GET_LEVEL(ch)) == 0)) {
    act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
    return TRUE;
  }
  return FALSE;
}


SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_STANDING)
    return FALSE;

  for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && (!number(0, 4))) {
      npc_steal(ch, cons);
      return TRUE;
    }
  return FALSE;
}

/* I dont know if this works, just coded it */
SPECIAL(shapeshifter)
{
  bool change = TRUE;
  struct char_data *vict;
  struct char_data *shapeshifter = (struct char_data *) me;

  if (cmd)
     return FALSE;

  for (vict = world[shapeshifter->in_room].people; vict; vict = vict->next_in_room)
      if ((GET_MAX_LEVEL(vict) > GET_MAX_LEVEL(shapeshifter)) && !IS_NPC(vict))
	 if (change && !FIGHTING(shapeshifter)) {
	    clone_char(shapeshifter, vict);
	    change = FALSE;
	    return TRUE;
	 }

  return FALSE;
}

void clone_char(struct char_data *targ, struct char_data *vict)
{
}

SPECIAL(magic_user)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL)
    vict = FIGHTING(ch);

  if ((GET_LEVEL(ch) > 13) && (number(0, 10) == 0))
    cast_spell(ch, vict, NULL, SPELL_SLEEP);

  if ((GET_LEVEL(ch) > 7) && (number(0, 8) == 0))
    cast_spell(ch, vict, NULL, SPELL_BLIND);

  if ((GET_LEVEL(ch) > 12) && (number(0, 12) == 0)) {
    if (IS_EVIL(ch))
      cast_spell(ch, vict, NULL, SPELL_ENERGY_DRAIN);
    else if (IS_GOOD(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
  }
  if (number(0, 4))
    return TRUE;

  switch (GET_LEVEL(ch)) {
  case 4:
  case 5:
    cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
    break;
  case 6:
  case 7:
    cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
    break;
  case 8:
  case 9:
    cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
    break;
  case 10:
  case 11:
    cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP);
    break;
  case 12:
  case 13:
    cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
    break;
  case 14:
  case 15:
  case 16:
  case 17:
    cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY);
    break;
  default:
    cast_spell(ch, vict, NULL, SPELL_FIREBALL);
    break;
  }
  return TRUE;

}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

SPECIAL(temple_guard)
{
  int i;
  extern int religion_info[][3];
  char *buf = "The guard humiliates you, and blocks your way into the temple.\r\n";
  char *buf2 = "The guard humiliates $n, and blocks $s way into the temple.";

  if (!IS_MOVE(cmd) || GET_LEVEL(ch) >= LVL_IMMORT)
    return FALSE;

  for (i = 0; religion_info[i][0] != -1; i++) {
    if ((IS_NPC(ch) || (GET_REL(ch) != religion_info[i][0])) &&
     world[ch->in_room].number == religion_info[i][1] &&
     cmd == religion_info[i][2]) {
      send_to_char(buf, ch);
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
    }
  }
  return FALSE;
}


SPECIAL(guild_guard)
{
  bool   found = FALSE;
  struct char_data *guard = (struct char_data *) me;
  struct guild_type *gptr = NULL;
  extern struct guild_type *guild_info;

  char *buf  = "The guard steps in front of you, and blocks your way.\r\n";
  char *buf2 = "The guard steps in front of $n, and blocks $s way.";

  if (!IS_MOVE(cmd) || IS_AFFECTED2(guard, AFF_BLIND))
    return FALSE;

  /* Immorts can pass freely too */
  if (GET_LEVEL(ch) >= LVL_IMMORT)
    return FALSE;

  for (gptr = guild_info; gptr; gptr = gptr->next) {
    if (gptr->guild_entr_room == world[ch->in_room].number) {
      found = TRUE;
      break;
    }
  }
  if (!found)
    return FALSE;
  else {
    if ((IS_NPC(ch) || (GET_GUILD(ch) != gptr->number)) &&
        world[ch->in_room].number == gptr->guild_entr_room &&
        cmd == gptr->direction) {
      send_to_char(buf, ch);
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
    }
  }
  return FALSE;
}


int immortal_info[][4] = {
    {32008,  MAX_MORT_LEVEL,   TEMPLE_SQUARE_VNUM,   SCMD_SOUTH},
    {-1, -1, -1, -1}
};

SPECIAL(immortal_guard)
{
  int i;
  extern int immortal_info[][4];
  struct char_data *guard = (struct char_data *) me;

  if (!IS_MOVE(cmd))
     return FALSE;

  for (i = 0; immortal_info[i][0] != -1; i++) {
      if (immortal_info[i][0] == GET_MOB_VNUM(guard)) {
	 if (((IS_NPC(ch) || GET_MAX_LEVEL(ch) <= immortal_info[i][1]) &&
	     world[ch->in_room].number == immortal_info[i][2]) &&
	     cmd == immortal_info[i][3]) {
	    send_to_char("Two disembodied eyes appear out of the darkness in front of you.\r\n", ch);
	    send_to_char("The eyes say, 'You are not worthy to pass through this portal yet.'\r\n", ch);
	    act("Two disembodied eyes appear in front of $n.", FALSE, ch, 0, 0, TO_ROOM);
	    act("The eyes say, 'You are not worthy to pass through this portal yet, $n.'",
		FALSE, ch, 0, 0, TO_ROOM);
	    return TRUE;
	 }
      }
  }

  return FALSE;
}


SPECIAL(puff)
{
  if (cmd)
    return (0);

  switch (number(0, 60)) {
  case 0:
    do_say(ch, "My god!  It's full of stars!", 0, 0);
    return (1);
  case 1:
    do_say(ch, "How'd all those fish get up here?", 0, 0);
    return (1);
  case 2:
    do_say(ch, "I'm a very female dragon.", 0, 0);
    return (1);
  case 3:
    do_say(ch, "I've got a peaceful, easy feeling.", 0, 0);
    return (1);
  default:
    return (0);
  }
}


#if 0
SPECIAL(mybius) /* FUNKYTALK */
{
 ACMD(do_action);
 struct char_data *guard = (struct char_data *) me;
 ACMD(do_say);
 char arg1[50], tmpbuf[80];
 char *compstr;
 int i = 0;
 int cmdnum = 0;

 /* Well I made it an array of structs, but instead of making it char   *
  * char int, I made it char char char so we could just type in the cmd *
  * we want, and we only have to use up one int space this way - Jason  */

 struct funky {
     char *saytext;
     char *msgtext;
     char *command;
 } funkytalk[] = {
 {"hello",    "Hello there, and welcome to the township of Elrhinn.", "bow"},
 {"help",     "What is it that I may help thee with?",              "smile"},
 {"fuck you", "Hey! Fuck you too, pal!",                             "slap"},
 {"guard",    "Yes?",                                                  NULL},
  /* add more in between here */
 {NULL, NULL, NULL}
 };

  skip_spaces(&argument);

  if (((!CMD_IS("say") && !CMD_IS("'")) && !CMD_IS("peer")) || !*argument)
    return FALSE;

  /* extract what the player said */
  one_argument(argument, arg1);

  if (CMD_IS("say") || CMD_IS("'")) {
     compstr = str_dup(argument);
     LowerString(compstr);

     for (i = 0; funkytalk[i].saytext != NULL; i++)
	if (strstr(compstr, funkytalk[i].saytext) != NULL)
	  break;

     if (funkytalk[i].saytext == NULL)
       return FALSE;

     /* make sure the PC talks first - Great job Jason!!! */
     do_say(ch, argument, 0, 0);
     do_say(guard, funkytalk[i].msgtext, 0, 0);
     /* have the mobile perform a social */
     /* If we made funkytalk an array of structs {char *, char *, int} *
      * we could incorporate the cmd# into the array too .....         */
     if (funkytalk[i].command != NULL) {
	cmdnum = find_command(funkytalk[i].command);
	do_action(guard, GET_NAME(ch), cmdnum, 0);
     }
     /* don't forget to free the memory!! */
     free(compstr);
     return TRUE;
  } else if (CMD_IS("peer") && !IS_SAME_RACE(ch, guard)) {
     if (strstr(arg1, "guard")) {
       do_action(ch, "guard", find_command("peer"), 0);
       sprintf(tmpbuf, "Stop staring at me you stupid %s!",
	       pc_race_types[(int)GET_RACE(ch)]);
       do_say(guard, tmpbuf, 0, 0);
       return TRUE;
     }
  }
  return FALSE;
}
#endif


SPECIAL(fido)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3)) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
	next_obj = temp->next_content;
	obj_from_obj(temp);
	obj_to_room(temp, ch->in_room);
      }
      extract_obj(i);
      return (TRUE);
    }
  }
  return (FALSE);
}



SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
      continue;
    if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
      continue;
    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
    obj_from_room(i);
    obj_to_char(i, ch);
    return TRUE;
  }

  return FALSE;
}


SPECIAL(cityguard)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch) || FIGHTING(ch))
    return FALSE;

  max_evil = 1000;
  evil = 0;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_KILLER)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER KILLERS!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_THIEF)){
      act("$n screams 'HEY!!!  You're one of those PLAYER THIEVES!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (CAN_SEE(ch, tch) && FIGHTING(tch)) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}

SPECIAL(cleric)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL)
    vict = FIGHTING(ch);

  if ((GET_LEVEL(ch) > 20) && (number(0, 20) <= 1))
     cast_spell(ch, vict, NULL, SPELL_ICE_SHOWER);

  if ((GET_LEVEL(ch) > 13) && (number(0, 10) <= 1))
    cast_spell(ch, ch, NULL, SPELL_HEAL);

  if ((GET_LEVEL(ch) > 7) && (number(0, 8) == 0))
    cast_spell(ch, vict, NULL, SPELL_BLIND);

  if ((GET_LEVEL(ch) > 12) && (number(0, 12) <= 2)) {
    if (IS_EVIL(ch))
      cast_spell(ch, vict, NULL, SPELL_HARM);
    else if (IS_GOOD(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
  }
  if (number(0, 4))
    return TRUE;

  switch (GET_LEVEL(ch)) {
  case 4:
  case 5:
    cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
    break;
  case 6:
  case 7:
    cast_spell(ch, vict, NULL, SPELL_POISON);
    break;
  case 8:
  case 9:
  case 10:
  case 11:
    cast_spell(ch, vict, NULL, SPELL_EARTHQUAKE);
    break;
  case 12:
  case 13:
    cast_spell(ch, vict, NULL, SPELL_FIRE_STORM);
    break;
  case 14:
  case 15:
  case 16:
  case 17:
    cast_spell(ch, vict, NULL, SPELL_ICE_SHOWER);
    break;
  default:
    cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
    break;
  }
  return TRUE;

}



SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data *pet;

  pet_room = ch->in_room + 1;

  if (CMD_IS("list")) {
    send_to_char("Available pets are:\r\n", ch);
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%ld - %s\r\n", 3 * GET_EXP(pet), GET_NAME(pet));
      send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {

    argument = one_argument(argument, buf);
    argument = one_argument(argument, pet_name);

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\r\n", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < (GET_EXP(pet) * 3)) {
      send_to_char("You don't have enough gold!\r\n", ch);
      return (TRUE);
    }
    GET_GOLD(ch) -= GET_EXP(pet) * 3;

    pet = read_mobile(GET_MOB_RNUM(pet), REAL);
    GET_EXP(pet) = 0;
    SET_BIT(AFF_FLAGS(pet), AFF_CHARM);

    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      /* free(pet->player.name); don't free the prototype! */
      pet->player.name = str_dup(buf);

      sprintf(buf, "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
	      pet->player.description, pet_name);
      /* free(pet->player.description); don't free the prototype! */
      pet->player.description = str_dup(buf);
    }
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);

    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_WEIGHT(pet) = 1000;
    IS_CARRYING_N(pet) = 100;

    send_to_char("May you enjoy your pet.\r\n", ch);
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return 1;
  }
  /* All commands except list and buy */
  return 0;
}

int dragon(struct char_data *ch, int spellnum)
{
  struct char_data *vict;
  /* char *cp; */

  if (number(0,4) < 3)
    return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL)
    vict = FIGHTING(ch);

  switch( spellnum ) {
  case SPELL_FIRE_BREATH:
  case SPELL_GAS_BREATH:
  case SPELL_FROST_BREATH:
  case SPELL_ACID_BREATH:
  case SPELL_LIGHTNING_BREATH:
      call_magic(ch, vict, NULL, spellnum, GET_LEVEL(ch), CAST_BREATH);
      return TRUE;
  default:
      /* the dragon's asthama is evidently acting up */
      act("$n snorts and hocks a loogie!", FALSE, ch, 0, 0, TO_ROOM);
      break;

  }
  return TRUE;
}

SPECIAL(old_dragon)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL)
    vict = FIGHTING(ch);


  switch (number(3,18)) {
  case 4:return dragon(ch, SPELL_FIRE_BREATH);
  case 5:
    return dragon(ch, SPELL_LIGHTNING_BREATH);
    cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
    break;
  case 6:return dragon(ch, SPELL_FIRE_BREATH);
  case 7:
    return dragon(ch, SPELL_FROST_BREATH);
    cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
    break;
  case 8: return dragon(ch, SPELL_LIGHTNING_BREATH);
  case 9:
    cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
    break;
  case 10: return dragon(ch, SPELL_ACID_BREATH);
  case 11:
  case 12: return dragon(ch, SPELL_ACID_BREATH);
  case 13:
    return dragon(ch, SPELL_LIGHTNING_BREATH);
    break;
  case 14: return dragon(ch, SPELL_GAS_BREATH);
  case 15: return dragon(ch, SPELL_FROST_BREATH);
  case 16: return dragon(ch, SPELL_FIRE_BREATH);
  case 17:
    return dragon(ch, SPELL_GAS_BREATH);
    break;
  default:
    return dragon(ch, SPELL_ACID_BREATH);
    break;
  }
  return TRUE;

}


SPECIAL(breath_any)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  switch( number(0,7) ) {
  case 0: return dragon(ch, SPELL_FIRE_BREATH);
  case 1:
  case 2: return dragon(ch, SPELL_LIGHTNING_BREATH);
  case 3: return dragon(ch, SPELL_GAS_BREATH);
  case 4: return dragon(ch, SPELL_ACID_BREATH);
  case 5:
  case 6:
  case 7: return dragon(ch, SPELL_FROST_BREATH);
  }
  return FALSE;
}

SPECIAL(breath_acid)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (number(0, 9) < 8)
    return FALSE;

  return dragon(ch, SPELL_ACID_BREATH);
}

SPECIAL(breath_fire)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (number(0, 9) < 8)
    return FALSE;

  return dragon(ch, SPELL_FIRE_BREATH);
}

SPECIAL(breath_gas)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (number(0, 9) < 8)
    return FALSE;

  return dragon(ch, SPELL_GAS_BREATH);
}

SPECIAL(breath_lightning)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (number(0, 9) < 8)
    return FALSE;

  return dragon(ch, SPELL_LIGHTNING_BREATH);
}

SPECIAL(breath_frost)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (number(0, 9) < 8)
    return FALSE;

  return dragon(ch, SPELL_FROST_BREATH);
}



/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */

/* this needs to be fixed now that there are no avatars or saints */
#define IMMORT_SCROLL_VNUM    32026

SPECIAL(immortal_scroll)
{
   struct obj_data *obj;

   if (!CMD_IS("recite"))
      return (FALSE);
   if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_HOLD))
      return (FALSE);
   if (!(!strcmp(argument, "scroll")))
      return (FALSE);
   else {
      if (GET_EQ(ch, WEAR_WIELD) &&
	  GET_OBJ_VNUM(GET_EQ(ch, WEAR_WIELD)) == IMMORT_SCROLL_VNUM)
	 obj = GET_EQ(ch, WEAR_WIELD);
      else if (GET_EQ(ch, WEAR_HOLD) &&
	  GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == IMMORT_SCROLL_VNUM)
	 obj = GET_EQ(ch, WEAR_HOLD);
      else
	 return (FALSE);

      if ((obj != NULL) &&
	  GET_MAX_LEVEL(ch) == MAX_MORT_LEVEL) {
	 obj_from_char(obj);
	 extract_obj(obj);
	 GET_LEVEL(ch) = LVL_IMMORT;
	 send_to_char("A bright light flashes as the scroll bursts into flame.\r\n", ch);
	 send_to_char("The knowledge of the mighty gods has been conferred upon you.\r\n", ch);
	 send_to_char("You are now an immortal of The Dominion!\r\n", ch);
	 return (TRUE);
      } else
	 return (FALSE);
   }
   return (FALSE);
}


SPECIAL(bank)
{
  int amount;

  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      sprintf(buf, "Your current balance is %ld coins.\r\n",
	      GET_BANK_GOLD(ch));
    else
      sprintf(buf, "You currently have no money deposited.\r\n");
    send_to_char(buf, ch);
    return 1;
  } else if (CMD_IS("deposit")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to deposit?\r\n", ch);
      return 1;
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) -= amount;
    GET_BANK_GOLD(ch) += amount;
    sprintf(buf, "You deposit %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, 0, TO_ROOM);
    return 1;
  } else if (CMD_IS("withdraw")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to withdraw?\r\n", ch);
      return 1;
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins deposited!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) += amount;
    GET_BANK_GOLD(ch) -= amount;
    sprintf(buf, "You withdraw %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, 0, TO_ROOM);
    return 1;
  } else
    return 0;
}

