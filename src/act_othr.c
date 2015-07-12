/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ACT_OTH.C                                    Based on CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#define __ACT_OTHR_C__

#include "conf.h"
#include "sysdep.h"

#include <sys/stat.h>
#include "protos.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern char   *class_abbrevs[];
/* For TD 3/19/95 */
extern struct race_data *races;
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern int NUM_RACES;

/* extern procedures */
SPECIAL(shop_keeper);
ACMD(do_track);
ACMD(do_follow);
ACMD(do_infobar);
ACMD(do_gen_comm);
ACMD(do_tell);
/* extern prototypes */
void write_aliases(struct char_data *ch); /* TD used for saving aliases */
void Dismount(struct char_data *ch, struct char_data *h, int pos);
void FallOffMount(struct char_data *ch, struct char_data *h);
int  RideCheck(struct char_data *ch);
int  MountEgoCheck(struct char_data *ch, struct char_data *horse);
void add_follower(struct char_data * ch, struct char_data * leader);
void LowerString(char *s);

ACMD(do_quit)
{
  void die(struct char_data * ch);
  void Crash_rentsave(struct char_data * ch, int cost);
  int mobs_nearby(struct char_data *ch);
  extern int free_rent;
  int save_room;
  struct descriptor_data *d, *next_d;

  if (IS_NPC(ch) || !ch->desc)
    return;

  if (subcmd != SCMD_QUIT && GET_LEVEL(ch) < LVL_IMMORT) {
    sprintf(buf, "If you really want to quit, type %squit!%s.\r\n", 
	CCBCN(ch, C_SPR), CCNRM(ch, C_SPR));
    send_to_char(buf, ch);
  } else if (GET_POS(ch) == POS_FIGHTING)
    send_to_char("No way!  You're fighting for your life!\r\n", ch);
  else if (mobs_nearby(ch))
    send_to_char("No way!  It's too dangerous to quit around here!\r\n", ch);
  else if ((GET_POS(ch) < POS_STUNNED) && GET_HIT(ch) <= 0) {
    send_to_char("You die before your time...\r\n", ch);
    die(ch);
  } else {
    if (!GET_INVIS_LEV(ch))
      act("$n has left the realm.", TRUE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "%s has left the realm.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    send_to_char("Goodbye, friend.. Come back soon!\r\n", ch);

    /*
     * kill off all sockets connected to the same player as the one who is
     * trying to quit.  Helps to maintain sanity as well as prevent duping.
     */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (d == ch->desc)
	continue;
      if (d->character && (GET_IDNUM(d->character) == GET_IDNUM(ch)))
	close_socket(d);
    }

   save_room = ch->in_room;
   if (free_rent)
      Crash_rentsave(ch, 0);
    extract_char(ch);           /* Char is saved in extract char */

    /* If someone is quitting in their house, let them load back here */
    if (ROOM_FLAGGED(save_room, ROOM_HOUSE))
      save_char(ch, save_room);
  }
}



ACMD(do_save)
{
  if (IS_NPC(ch) || !ch->desc)
    return;

  if (cmd) {
    sprintf(buf, "Saving %s.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
  }
  write_aliases(ch);      /* Save the players aliases - TD */
  save_char(ch, ch->in_room);
  // THIS ALLOWS FOR ASCII saving
  save_char_obj( ch );
  Crash_crashsave(ch);
  if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
    House_crashsave(world[ch->in_room].number);
}


/* generic function for commands which are normally overridden by
   special procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
  send_to_char("Sorry, but you cannot do that here!\r\n", ch);
}


ACMD(do_feign_death)
{
  int percent = number(1, 101);
  struct char_data *vict;
  void death_cry(struct char_data *ch);

  if (!FIGHTING(ch)) {
     send_to_char("Why do you want to play dead?  You're not fighting anyone.\r\n", ch);
     return;
  } else if (GET_SKILL(ch, SKILL_FEIGN_DEATH) > percent) {
     vict = FIGHTING(ch);
     if (FIGHTING(vict) == ch)
	stop_fighting(vict);
     stop_fighting(ch);
     act("$n is dead!  R.I.P.", FALSE, ch, 0, 0, TO_ROOM);
     send_to_char("You receive one lousy experience point.\r\n", vict);
     send_to_char("Everyone thinks your dead! Hehehehehe\r\n", ch);
     death_cry(ch);
     SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
  } else
     WAIT_STATE(ch, PULSE_VIOLENCE * 4);
}


ACMD(do_sneak)
{
  struct affected_type af;
  byte percent;

  send_to_char("Okay, you'll try to move silently for a while.\r\n", ch);
  if (IS_AFFECTED(ch, AFF_SNEAK))
    affect_from_char(ch, SKILL_SNEAK, FALSE);

  percent = number(1, 101);     /* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_SNEAK) + dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af, FALSE);
}



ACMD(do_hide)
{
  byte percent;

  send_to_char("You attempt to hide yourself.\r\n", ch);

  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);

  percent = number(1, 101);     /* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide)
    return;

  SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
}




ACMD(do_steal)
{
  struct char_data *vict;
  struct obj_data *obj;
  char vict_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
  int percent, gold, eq_pos, pcsteal = 0, ohoh = 0;
  extern int pt_allowed;

  argument = one_argument(argument, obj_name);
  one_argument(argument, vict_name);

  if (!(vict = get_char_room_vis(ch, vict_name))) {
    send_to_char("Steal what from who?\r\n", ch);
    return;
  } else if (vict == ch) {
    send_to_char("Come on now, that's rather stupid!\r\n", ch);
    return;
  } else if (GET_EQ(ch, WEAR_WIELD) && GET_EQ(ch, WEAR_HOLD)) {
    send_to_char("How do you propose to steal from somone with your hands full?\r\n", ch);
    return;
  }
  if (!pt_allowed) {
    if (!IS_NPC(vict) && !PLR_FLAGGED(vict, PLR_THIEF) &&
	!PLR_FLAGGED(vict, PLR_KILLER) && !PLR_FLAGGED(ch, PLR_THIEF)) {
      pcsteal = 1;
    }
  }
  /* 101% is a complete failure */
  percent = number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;

  if (GET_POS(vict) <= POS_SLEEPING)
    percent = -1;               /* ALWAYS SUCCESS */

  /* NO NO With Imp's and Shopkeepers! */
  if ((GET_LEVEL(vict) >= LVL_IMMORT) || pcsteal ||
      GET_MOB_SPEC(vict) == shop_keeper)
    percent = 101;              /* Failure */

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {

    if (!(obj = get_obj_in_list_vis(vict, obj_name, vict->carrying))) {

      for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
	if (GET_EQ(vict, eq_pos) &&
	    (isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
	    CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos))) {
	  obj = GET_EQ(vict, eq_pos);
	  break;
	}
      if (!obj) {
	act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR);
	return;
      } else {                  /* It is equipment */
	if ((GET_POS(vict) > POS_SLEEPING)) {
	  send_to_char("Steal the equipment now?  Impossible!\r\n", ch);
	  return;
	} else {
	  act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
	  act("$n steals $p from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
	  obj_to_char(unequip_char(vict, eq_pos), ch);
	}
      }
    } else {                    /* obj found in inventory */

      percent += GET_OBJ_WEIGHT(obj);   /* Make heavy harder */

      if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
	ohoh = TRUE;
	act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
      } else {                  /* Steal the item */
	if ((IS_CARRYING_N(ch) + 1) < CAN_CARRY_N(ch)) {
	  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    send_to_char("Got it!\r\n", ch);
	  }
	} else
	  send_to_char("You cannot carry that much.\r\n", ch);
      }
    }
  } else {                      /* Steal some coins */
    if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
      ohoh = TRUE;
      act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
      act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, vict, TO_VICT);
      act("$n tries to steal gold from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(vict) * number(1, 10)) / 100);
      gold = MIN(1782, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(vict) -= gold;
	sprintf(buf, "Bingo!  You got %d gold coins.\r\n", gold);
	send_to_char(buf, ch);
      } else {
	send_to_char("You couldn't get any gold...\r\n", ch);
      }
    }
  }

  if (ohoh) {
     if (!pt_allowed || (zone_table[(world[ch->in_room].zone)].pkill != 1)) {
	if (IS_HUMANOID(vict) && !PLR_FLAGGED(vict, PLR_THIEF) &&
	    !PLR_FLAGGED(vict, PLR_KILLER) && !PLR_FLAGGED(ch, PLR_THIEF)) {
          SET_BIT(PLR_FLAGS(ch), PLR_THIEF);
          send_to_char("You should be more careful who you take from, and where.\r\n", ch);
          send_to_char("You are now a THIEF!\r\n", ch);
          sprintf(buf, "PC Thief bit set on %s", GET_NAME(ch));
          log(buf);
	}
     }
     if (IS_NPC(vict) && AWAKE(vict))
	hit(vict, ch, TYPE_UNDEFINED);
  }
}


ACMD(do_skills)
{
   int i, sortpos;
   extern char * spells[];
   int spell_sort_info[MAX_SKILLS+1];
   char *how_good(int skill);

   strcpy(buf2, "You know the follwing skills:\r\n");
   for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
      i = spell_sort_info[sortpos];
      if (i > MAX_SKILLS)
	continue;
      if ((i <= MAX_SPHERES) || (!strcmp(spells[i], "!UNUSED!")))
	continue;
      if (GET_SKILL(ch, i) <= 1)
	continue;
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
	 strcat(buf2, "**OVERFLOW**\r\n");
	 break;
      }
      if ((i > MAX_SPHERES) && !(!strcmp(spells[i], "!UNUSED!"))) {
	 sprintf(buf, "%24s (%s)\t%s", spells[i], how_good(GET_SKILL(ch, i)),
		 (i % 2 ? "\r\n" : "\t"));
	 strcat(buf2, buf);
      }
   }
   page_string(ch->desc, buf2, 1);
}


ACMD(do_spheres)
{
  void list_skills(struct char_data * ch, int sphere);
  int  find_skill_num(char *arg);

  skip_spaces(&argument);

  if (!*argument)
     list_skills(ch, -1);
  else if (!str_cmp(argument, "unused"))
     list_skills(ch, 0);
  else
     list_skills(ch, find_skill_num(argument));
}

/* this function enhances a PC's skill by a random factor */
void learn_from_mistake(struct char_data *ch, int skill)
{
    /* this code allows this func to be called EVERYTIME the PC fails */
    if (GET_SKILL(ch, skill) < 98) {
      GET_SKILL(ch, skill) += number(1, 2);
      send_to_char("You learn from your mistake.\r\n", ch);
    }
}


ACMD(do_visible)
{
  void appear(struct char_data * ch);
  void perform_immort_vis(struct char_data *ch);

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    perform_immort_vis(ch);
    return;
  }

  if IS_ITEM_AFF(ch, AFFECTED_INVIS) {
     send_to_char("You must remove the item that is making you invisibile.\r\n", ch);
     return;
  }

  if IS_AFFECTED(ch, AFF_INVISIBLE) {
    appear(ch);
    send_to_char("You break the spell of invisibility.\r\n", ch);
  } else
    send_to_char("You are already visible.\r\n", ch);
}



ACMD(do_title)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (IS_NPC(ch))
    send_to_char("Your title is fine... go away.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_NOTITLE))
    send_to_char("You can't title yourself -- you shouldn't have abused it!\r\n", ch);
  else if (strstr(argument, "(") || strstr(argument, ")"))
    send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
  else if (strlen(argument) > MAX_TITLE_LENGTH) {
    sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
	    MAX_TITLE_LENGTH);
    send_to_char(buf, ch);
  } else {
    set_title(ch, argument);
    sprintf(buf, "Okay, you're now %s %s.\r\n", GET_NAME(ch), GET_TITLE(ch));
    send_to_char(buf, ch);
  }
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_INFOBAR))
    do_infobar(ch, 0, 0, SCMDB_TITLE);
}


int perform_group(struct char_data *ch, struct char_data *vict)
{
  if (IS_AFFECTED2(vict, AFF_GROUP) || !CAN_SEE(ch, vict))
    return 0;

  if (SHADOWING(vict) == ch)
    return 0;

  SET_BIT(AFF2_FLAGS(vict), AFF_GROUP);
  if (ch != vict)
    act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
  act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
  act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
  return 1;
}


char *print_cond(struct char_data *ch, int mode)
{
   extern char *const  percent_tired [];
   extern char *const  percent_hit   [];

   if (mode == 0)        /* print the hit string */
      return ((char *) (percent_hit[(((GET_HIT(ch)*100)/GET_MAX_HIT(ch))/10)]));
   else if (mode == 1)   /* print the tired string */
      return ((char *) (percent_tired[(((GET_MOVE(ch)*100)/GET_MAX_MOVE(ch))/10)]));
   else                  /* do nothing */
      return NULL;
}

void print_group(struct char_data *ch)
{
  struct char_data *k;
  struct follow_type *f;

  if (!IS_AFFECTED2(ch, AFF_GROUP))
    send_to_char("But you are not the member of a group!\r\n", ch);
  else {
    send_to_char("Your group consists of:\r\n", ch);

    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED2(k, AFF_GROUP)) {
      sprintf(buf, "     $N is %s and %s. (Head of group)",
	      print_cond(k, 0), print_cond(k, 1));
      act(buf, FALSE, ch, 0, k, TO_CHAR);
    }

    for (f = k->followers; f; f = f->next) {
      if (!IS_AFFECTED2(f->follower, AFF_GROUP))
	continue;

      sprintf(buf, "     $N is %s and %s.",
	      print_cond(f->follower, 0), print_cond(f->follower, 1));
      act(buf, FALSE, ch, 0, f->follower, TO_CHAR);
    }
  }
}



ACMD(do_group)
{
  struct char_data *vict;
  struct follow_type *f;
  int found;

  one_argument(argument, buf);

  if (!*buf) {
    print_group(ch);
    return;
  }

  if (ch->master) {
    act("You can not enroll group members without being head of a group.",
	FALSE, ch, 0, 0, TO_CHAR);
    return;
  }

  if (!str_cmp(buf, "all")) {
    perform_group(ch, ch);
    for (found = 0, f = ch->followers; f; f = f->next)
      found += perform_group(ch, f->follower);
    if (!found)
      send_to_char("Everyone following you is already in your group.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if ((vict->master != ch) && (vict != ch))
    act("$N must follow you to enter your group.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!IS_AFFECTED2(vict, AFF_GROUP))
      perform_group(ch, vict);
    else {
      if (ch != vict)
	act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
      act("You have been kicked out of $n's group!", FALSE, ch, 0, vict, TO_VICT);
      act("$N has been kicked out of $n's group!", FALSE, ch, 0, vict, TO_NOTVICT);
      REMOVE_BIT(AFF2_FLAGS(vict), AFF_GROUP);
    }
  }
}



ACMD(do_ungroup)
{
  struct follow_type *f, *next_fol;
  struct char_data *tch;
  void stop_follower(struct char_data * ch);

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->master || !(IS_AFFECTED2(ch, AFF_GROUP))) {
      send_to_char("But you lead no group!\r\n", ch);
      return;
    }
    sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
    for (f = ch->followers; f; f = next_fol) {
      next_fol = f->next;
      if (IS_AFFECTED2(f->follower, AFF_GROUP)) {
	REMOVE_BIT(AFF2_FLAGS(f->follower), AFF_GROUP);
	send_to_char(buf2, f->follower);
	if (!IS_AFFECTED(f->follower, AFF_CHARM))
	  stop_follower(f->follower);
      }
    }

    REMOVE_BIT(AFF2_FLAGS(ch), AFF_GROUP);
    send_to_char("You disband the group.\r\n", ch);
    return;
  }
  if (!(tch = get_char_room_vis(ch, buf))) {
    send_to_char("There is no such person!\r\n", ch);
    return;
  }
  if (tch->master != ch) {
    send_to_char("That person is not following you!\r\n", ch);
    return;
  }

  if (!IS_AFFECTED2(tch, AFF_GROUP)) {
    send_to_char("That person isn't in your group.\r\n", ch);
    return;
  }

  REMOVE_BIT(AFF2_FLAGS(tch), AFF_GROUP);

  act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
  act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
  act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);

  if (!IS_AFFECTED(tch, AFF_CHARM))
    stop_follower(tch);
}




ACMD(do_report)
{
  if (!IS_AFFECTED2(ch, AFF_GROUP)) {
    send_to_char("But you are not a member of any group!\r\n", ch);
    return;
  }

  sprintf(buf, "$n reports, 'I am %s and %s.'",
	  print_cond(ch, 0), print_cond(ch, 1));
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "You report, 'I am %s and %s.'",
	  print_cond(ch, 0), print_cond(ch, 1));
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
}



ACMD(do_split)
{
  int amount, num, share;
  struct char_data *k;
  struct follow_type *f;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (is_number(buf)) {
    amount = atoi(buf);
    if (amount <= 0) {
      send_to_char("Sorry, you can't do that.\r\n", ch);
      return;
    }
    if (amount > GET_GOLD(ch)) {
      send_to_char("You don't seem to have that much gold to split.\r\n", ch);
      return;
    }
    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED2(k, AFF_GROUP) && (k->in_room == ch->in_room))
      num = 1;
    else
      num = 0;

    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED2(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room))
	num++;

    if (num && IS_AFFECTED2(ch, AFF_GROUP))
      share = amount / num;
    else {
      send_to_char("With whom do you wish to share your gold?\r\n", ch);
      return;
    }

    GET_GOLD(ch) -= share * (num - 1);

    if (IS_AFFECTED2(k, AFF_GROUP) && (k->in_room == ch->in_room)
	&& !(IS_NPC(k)) && k != ch) {
      GET_GOLD(k) += share;
      sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
	      amount, share);
      send_to_char(buf, k);
    }
    for (f = k->followers; f; f = f->next) {
      if (IS_AFFECTED2(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room) &&
	  f->follower != ch) {
	GET_GOLD(f->follower) += share;
	sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
		amount, share);
	send_to_char(buf, f->follower);
      }
    }
    sprintf(buf, "You split %d coins among %d members -- %d coins each.\r\n",
	    amount, num, share);
    send_to_char(buf, ch);
  } else {
    send_to_char("How many coins do you wish to split with your group?\r\n", ch);
    return;
  }
}



ACMD(do_use)
{
  struct char_data *victim;
  struct obj_data *mag_item;
  int    equipped = 1;

  half_chop(argument, arg, buf);
  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return;
  }
  mag_item = GET_EQ(ch, WEAR_HOLD);
  /* Guess the ch wasn't holding it after all */
  if (!mag_item || !isname(arg, mag_item->name)) 
    mag_item = GET_EQ(ch, WEAR_WIELD);

  if (!mag_item || !isname(arg, mag_item->name)) {
    switch (subcmd) {
    case SCMD_RECITE:
    case SCMD_QUAFF:
      equipped = 0;
      if (!(mag_item = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf2, ch);
	return;
      }
      break;
    case SCMD_USE:
      sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
      send_to_char(buf2, ch);
      return;
      break;
    default:
      log("SYSERR: Unknown subcmd passed to do_use");
      return;
      break;
    }
  }
  switch (subcmd) {
  case SCMD_QUAFF:
    if (GET_OBJ_TYPE(mag_item) != ITEM_POTION) {
      send_to_char("You can only quaff potions.", ch);
      return;
    }
    break;
  case SCMD_RECITE:
    if (GET_OBJ_TYPE(mag_item) != ITEM_SCROLL) {
      send_to_char("You can only recite scrolls.", ch);
      return;
    }
    break;
  case SCMD_USE:
    if ((GET_OBJ_TYPE(mag_item) != ITEM_WAND) &&
	(GET_OBJ_TYPE(mag_item) != ITEM_STAFF) &&
	(GET_OBJ_TYPE(mag_item) != ITEM_WEAPON)) {
      send_to_char("You can't seem to figure out how to use it.\r\n", ch);
      return;
    }
    break;
  }
  /* For special weapon affects */
  if (subcmd == SCMD_USE && GET_OBJ_TYPE(mag_item) == ITEM_WEAPON) {
    if (*buf) {
      if ((victim = get_char_room_vis(ch, buf)) != NULL) {
	if (IS_OBJ_STAT(mag_item, ITEM_FROSTY))
	  call_magic(ch, victim, mag_item, SPELL_FROST_BREATH, GET_LEVEL(ch), CAST_BREATH);
	if (IS_OBJ_STAT(mag_item, ITEM_SPARKING))
	  call_magic(ch, victim, mag_item, SPELL_LIGHTNING_BREATH, GET_LEVEL(ch), CAST_BREATH);
	if (IS_OBJ_STAT(mag_item, ITEM_FLAMING))
	  call_magic(ch, victim, mag_item, SPELL_FIRE_BREATH, GET_LEVEL(ch), CAST_BREATH);
	if (IS_OBJ_STAT(mag_item, ITEM_ACIDIC))
	  call_magic(ch, victim, mag_item, SPELL_ACID_BREATH, GET_LEVEL(ch), CAST_BREATH);
	if (IS_OBJ_STAT(mag_item, ITEM_NOXIOUS))
	  call_magic(ch, victim, mag_item, SPELL_GAS_BREATH, GET_LEVEL(ch), CAST_BREATH);
      } else
	send_to_char("Cannot seem to find your target.\r\n", ch);
    }
    return;
  } else if (subcmd == SCMD_RECITE && GET_OBJ_TYPE(mag_item) == ITEM_SCROLL) {
    if (GET_SKILL(ch, SKILL_READ_MAGIC) < number(2, 99)) {
      send_to_char("As you recite the unfamiliar words the scroll bursts into flame!", ch);
      act("As $n begins reading from the scroll it bursts into flame!", TRUE, ch, 0, 0, TO_ROOM);
      extract_obj(mag_item);
      GET_HIT(ch)--;
      return;
    }
  } else if (FIGHTING(ch) && subcmd == SCMD_QUAFF) {
    if (number(0, 10) >= 6) {
      extract_obj(mag_item);
      send_to_char("The potion breaks and you are cut by the glass!\r\n", ch);
      GET_HIT(ch) -= 2;
      act("$n fumbles with $s potion and it shatters.", FALSE, ch, 0, 0, TO_ROOM);
      return;
    }
  }
  /* If we get this far do it! */
  mag_objectmagic(ch, mag_item, buf);
}



ACMD(do_wimpy)
{
  int wimp_lev;

  one_argument(argument, arg);

  if (!*arg) {
    if (GET_WIMP_LEV(ch)) {
      wimp_lev = (GET_WIMP_LEV(ch) * 100)/GET_MAX_HIT(ch);
      sprintf(buf, "Your current wimp level is %d%% of your hit points.\r\n",
	      wimp_lev);
      send_to_char(buf, ch);
      return;
    } else {
      send_to_char("At the moment, you're not a wimp.  (sure, sure...)\r\n", ch);
      return;
    }
  }
  if (isdigit((int)*arg)) {
    if ((wimp_lev = atoi(arg))) {
      if ((wimp_lev < 0) || (wimp_lev > 50))
	send_to_char("Your wimpiness can only be from 0 to 50% of your hit points.\r\n", ch);
      else {
	sprintf(buf, "Okay, you'll wimp out if you drop below %d%% of your hit points.\r\n",
		wimp_lev);
	send_to_char(buf, ch);
	GET_WIMP_LEV(ch) = ((GET_MAX_HIT(ch) * wimp_lev) / 100);
      }
    } else {
      send_to_char("Okay, you'll now tough out fights to the bitter end.\r\n", ch);
      GET_WIMP_LEV(ch) = 0;
    }
  } else
    send_to_char("Specify at the percentage of hit points you want to wimp out at.  (0 to disable)\r\n", ch);

  return;
}


ACMD(do_display)
{
  int i, off = 0;

  if (IS_NPC(ch)) {
    send_to_char("Mosters don't need displays.  Go away.\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Usage: prompt { H | M | V | X | T | E | all | none }\r\n", ch);
    return;
  }
  if ((!str_cmp(argument, "on")) || (!str_cmp(argument, "all")))
    SET_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE | PRF_DISEXP2LEV | PRF_DISPTANK | PRF_DISPENEMY);
  else {
    REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE | PRF_DISEXP2LEV | PRF_DISPTANK | PRF_DISPENEMY);

    for (i = 0; i < strlen(argument); i++) {
      switch (LOWER(argument[i])) {
      case 'h':
	if (!off)
	   SET_BIT(PRF_FLAGS(ch), PRF_DISPHP);
	else
	   REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPHP);
	off = 0;
	break;
      case 'm':
	if (!off)
	   SET_BIT(PRF_FLAGS(ch), PRF_DISPMANA);
	else
	   REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPMANA);
	off = 0;
	break;
      case 'v':
	if (!off)
	   SET_BIT(PRF_FLAGS(ch), PRF_DISPMOVE);
	else
	   REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPMOVE);
	off = 0;
	break;
      case 'x':
	if (!off)
	   SET_BIT(PRF_FLAGS(ch), PRF_DISEXP2LEV);
	else
	   REMOVE_BIT(PRF_FLAGS(ch), PRF_DISEXP2LEV);
	off = 0;
	break;
      case 't':
	if (!off)
	   SET_BIT(PRF_FLAGS(ch), PRF_DISPTANK);
	else
	   REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPTANK);
	off = 0;
	break;
      case 'e':
	if (!off)
	   SET_BIT(PRF_FLAGS(ch), PRF_DISPENEMY);
	else
	   REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPENEMY);
	off = 0;
	break;
      case '-':
	off = 1;
	break;
      }
    }
  }

  send_to_char(OK, ch);
}



ACMD(do_gen_write)
{
  FILE *fl;
  char *tmp, *filename, buf[MAX_STRING_LENGTH];
  struct stat fbuf;
  extern int max_filesize;
  time_t ct;

  switch (subcmd) {
  case SCMD_BUG:
    filename = BUG_FILE;
    break;
  case SCMD_TYPO:
    filename = TYPO_FILE;
    break;
  case SCMD_IDEA:
    filename = IDEA_FILE;
    break;
  case SCMD_TODO:
    filename = TODO_FILE;
    break;
  default:
    return;
  }

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (IS_NPC(ch)) {
    send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("That must be a mistake...\r\n", ch);
    return;
  }
  sprintf(buf, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);
  mudlog(buf, CMP, LVL_IMMORT, FALSE);

  if (stat(filename, &fbuf) < 0) {
    perror("Error statting file");
    return;
  }
  if (fbuf.st_size >= max_filesize) {
    send_to_char("Sorry, the file is full right now.. try again later.\r\n", ch);
    return;
  }
  if (!(fl = fopen(filename, "a"))) {
    perror("do_gen_write");
    send_to_char("Could not open the file.  Sorry.\r\n", ch);
    return;
  }
  fprintf(fl, "%-8s (%6.6s) [%ld] %s\n", GET_NAME(ch), (tmp + 4),
	  world[ch->in_room].number, argument);
  fclose(fl);
  send_to_char("Okay.  Thanks!\r\n", ch);
}



#define TOG_OFF 0
#define TOG_ON  1

#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))

ACMD(do_gen_tog)
{
  long result;
  extern int nameserver_is_slow;

  char *tog_messages[][2] = {
    {"Nohassle disabled.\r\n",
    "Nohassle enabled.\r\n"},
    {"Brief mode off.\r\n",
    "Brief mode on.\r\n"},
    {"Compact mode off.\r\n",
    "Compact mode on.\r\n"},
    {"You can now hear tells.\r\n",
    "You are now deaf to tells.\r\n"},
    {"You can now hear wishes.\r\n",
    "You are now deaf to wishes.\r\n"},
    {"You can now hear shouts.\r\n",
    "You are now deaf to shouts.\r\n"},
    {"You can now hear gossip.\r\n",
    "You are now deaf to gossip.\r\n"},
    {"You can now hear the OOC channel.\r\n",
    "You are now deaf to the OOC channel.\r\n"},
    {"You can now hear music.\r\n",
    "You are now deaf to music.\r\n"},
    {"You can now hear the Wiz-channel.\r\n",
    "You are now deaf to the Wiz-channel.\r\n"},
    {"You are no longer part of the Quest.\r\n",
    "Okay, you are part of the Quest!\r\n"},
    {"You will no longer see the room flags.\r\n",
    "You will now see the room flags.\r\n"},
    {"You will now have your communication repeated.\r\n",
    "You will no longer have your communication repeated.\r\n"},
    {"HolyLight mode off.\r\n",
    "HolyLight mode on.\r\n"},
    {"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
    "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
    {"Autoexits disabled.\r\n",
    "Autoexits enabled.\r\n"},
    {"Autoassist disabled.\r\n",
    "Autoassist enabled.\r\n"},
    {"Pkill protection disabled.\r\n",
    "Pkill protection enabled.\r\n"},
    {"Autoloot disabled.\r\n",
    "Autoloot enabled.\r\n"},
    {"Autogold disabled.\r\n",
    "Autogold enabled.\r\n"},
    {"Fight spam ensabled.\r\n",
    "Fight spam disabled.\r\n"},    /* opposite because of wording */
    { "Infobar disabled.\n\r",
    "Infobar enabled.\n\r"},
    { "Scorebar disabled.\n\r",
    "Scorebar enabled when infobar is enabled.\n\r"},
    { "Opponent health meter disabled.\n\r",
    "Opponent health meter enabled.\n\r"},
    { "Welcome back!! You are no longer AFK.\n\r",
    "You are now away from the keyboard! Come back soon :-)\n\r"},
  };


  if (IS_NPC(ch))
    return;

  switch (subcmd) {
  case SCMD_NOHASSLE:
    result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
    break;
  case SCMD_BRIEF:
    result = PRF_TOG_CHK(ch, PRF_BRIEF);
    break;
  case SCMD_COMPACT:
    result = PRF_TOG_CHK(ch, PRF_COMPACT);
    break;
  case SCMD_NOTELL:
    result = PRF_TOG_CHK(ch, PRF_NOTELL);
    break;
  case SCMD_DEAF:
    result = PRF_TOG_CHK(ch, PRF_DEAF);
    break;
  case SCMD_NOGOSSIP:
    result = PRF_TOG_CHK(ch, PRF_NOGOSS);
    break;
  case SCMD_NOOOC:
    result = PRF_TOG_CHK(ch, PRF_NOOOC);
    break;
  case SCMD_NOWIZ:
    result = PRF_TOG_CHK(ch, PRF_NOWIZ);
    break;
  case SCMD_QUEST:
    result = PRF_TOG_CHK(ch, PRF_QUEST);
    break;
  case SCMD_ROOMFLAGS:
    result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS);
    break;
  case SCMD_NOREPEAT:
    result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
    break;
  case SCMD_HOLYLIGHT:
    result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
    break;
  case SCMD_SLOWNS:
    result = (nameserver_is_slow = !nameserver_is_slow);
    break;
  case SCMD_AUTOEXIT:
    result = PRF_TOG_CHK(ch, PRF_AUTOEXIT);
    break;
  case SCMD_AUTOASS:
    result = PRF_TOG_CHK(ch, PRF_AUTOASS);
    break;
  case SCMD_PKILLPROT:
    if (GET_MAX_LEVEL(ch) <= 5)
      result = PRF_TOG_CHK(ch, PLR_NEWBIE);
    else
      result = 0;
    break;
  case SCMD_AUTOLOOT:
    result = PRF_TOG_CHK(ch, PRF_AUTOLOOT);
    break;
  case SCMD_AUTOGOLD:
    result = PRF_TOG_CHK(ch, PRF_AUTOGOLD);
    break;
  case SCMD_NOSPAM:
    result = PRF_TOG_CHK(ch, PRF_NOSPAM);
    break;
  case SCMD_INFOBAR:
    if (PRF_FLAGGED(ch, PRF_INFOBAR)) {
      do_infobar(ch, 0, 0, SCMDB_CLEAR);
      result = PRF_TOG_CHK(ch, PRF_INFOBAR);
    } else {
      result = PRF_TOG_CHK(ch, PRF_INFOBAR);
      do_infobar(ch, 0, 0, SCMDB_REDRAW);
    }
    break;
  case SCMD_SCOREBAR:
    result = PRF_TOG_CHK(ch, PRF_SCOREBAR);
    do_infobar(ch, 0, 0, SCMDB_REDRAW);
    break;
  case SCMD_METER:
    result = PRF_TOG_CHK(ch, PRF_METER);
    break;
  case SCMD_AFK:
    result = PRF_TOG_CHK(ch, PRF_AFK);
    break;
  default:
    log("SYSERR: Unknown subcmd in do_gen_toggle");
    return;
    break;
  }

  if (result)
    send_to_char(tog_messages[subcmd][TOG_ON], ch);
  else
    send_to_char(tog_messages[subcmd][TOG_OFF], ch);

  return;
}

/* Below here is all new stuff for TD */
ACMD(do_meditate)
{
    if (FIGHTING(ch)) {
       send_to_char("How the hell do you expect to meditate while fighting?\r\n", ch);
       return;
    }

    if (GET_POS(ch) < POS_FIGHTING) {
       send_to_char("You begin to meditate.....\r\n", ch);
       act("$n begins to meditate.", TRUE, ch, 0, 0, TO_ROOM);
    } else {
       send_to_char("You sit down and begin to meditate.\r\n", ch);
       act("$n sits down and begins to meditate.", TRUE, ch, 0, 0, TO_ROOM);
    }

    GET_POS(ch) = POS_MEDITATING;
}


ACMD(do_hunt)
{
  struct char_data *vict;

  argument = one_argument(argument, buf);

  if (!*buf)
    send_to_char("Whom do you wish to hunt?\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (ch == vict)
    send_to_char("You have already tracked yourself down!\r\n", ch);
  else {
    sprintf(buf, "%s", GET_NAME(vict));
    if (GET_SKILL(ch, SKILL_HUNT) < 5) {
       do_track(ch, buf, 0, 0);
    } else {
       act("You begin to hunt $N.", FALSE, ch, 0, vict, TO_CHAR);
       HUNTING(ch) = vict;
       do_track(ch, buf, 0, 0);
    }
  }
}



ACMD(do_mount)
{
  char buf[256];
  char name[112];
  int check;
  struct char_data *horse;

  if (CMD_IS("mount") || CMD_IS("ride")) {
     one_argument(argument, name);

  if ((!(horse = get_char_room_vis(ch, name))) || ((ch->in_room) != (horse->in_room))) {
     send_to_char("Mount what?\n\r", ch);
     return;
  }

  if (!IS_NPC(horse)) {
     sprintf(buf, "%s just tried to mount you! How rude.\r\n", GET_NAME(ch));
     send_to_char(buf, horse);
     act("You try to mount $N, but $E tells you to get off of $M", FALSE, ch, 0, horse, TO_CHAR);
     act("$n tries to mount $N, but $E throws $m off!", FALSE, ch, 0, horse, TO_NOTVICT);
     return;
  }

  if (IS_NPC(horse)) {
     if (IS_HUMANOID(horse)) {
	act("$N doesn't appreciate you hopping on $S back.", FALSE, ch, 0, horse, TO_CHAR);
	act("$n tries to mount $N, but $E throws $m off!", FALSE, ch, 0, horse, TO_ROOM);
	return;
     }

     if (GET_POS(horse) < POS_STANDING) {
	send_to_char("Your mount must be standing\n\r", ch);
	return;
     }

     if (RIDDEN(horse)) {
	sprintf(buf, "But %s is already being ridden!\r\n", GET_NAME(horse));
	send_to_char(buf, ch);
	return;
     } else if (MOUNTED(ch)) {
	send_to_char("But you are already riding something!\n\r", ch);
	return;
     }

     if (IS_DRAGON(horse)) {
	check = number(1, 110);
	if (GET_SKILL(ch, SKILL_DRAGON_RIDE) < check) {
	   act("$N snarls and attacks!", FALSE, ch, 0, horse, TO_CHAR);
	   act("As $n tries to mount $N, $N attacks $n!", FALSE, ch, 0, horse, TO_NOTVICT);
	   WAIT_STATE(ch, PULSE_VIOLENCE*10);
	   hit(horse, ch, TYPE_UNDEFINED);
	   return;
	}
     }

     /* If it's a dragon the draogn gets two chances to protect itself */
     check = MountEgoCheck(ch, horse);
     if (check > 5) {
	act("$N snarls and attacks!",
	    FALSE, ch, 0, horse, TO_CHAR);
	act("As $n tries to mount $N, $N attacks $n!",
	    FALSE, ch, 0, horse, TO_NOTVICT);
	WAIT_STATE(ch, PULSE_VIOLENCE*2);
	hit(horse, ch, TYPE_UNDEFINED);
	return;
     } else if (check == -1) {
	act("$N moves out of the way, you fall on your butt",
	    FALSE, ch, 0, horse, TO_CHAR);
	act("as $n tries to mount $N, $N moves out of the way",
	    FALSE, ch, 0, horse, TO_NOTVICT);
	WAIT_STATE(ch, PULSE_VIOLENCE);
	GET_POS(ch) = POS_SITTING;
	return;
     }

     if (RideCheck(ch)) {
	act("You hop on $N's back", FALSE, ch, 0, horse, TO_CHAR);
	act("$n hops on $N's back", FALSE, ch, 0, horse, TO_NOTVICT);
	act("$n hops on your back!", FALSE, ch, 0, horse, TO_VICT);
	MOUNTED(ch)   = horse;
	RIDDEN(horse) = ch;
	GET_POS(ch)   = POS_MOUNTED;
	REMOVE_BIT(AFF_FLAGS(ch), AFF_SNEAK | AFF_HIDE);
	add_follower(horse, ch);
     } else {
	act("You try to ride $N, but you fall on your butt",
	    FALSE, ch, 0, horse, TO_CHAR);
	act("$n tries to ride $N, but falls on $s butt",
	    FALSE, ch, 0, horse, TO_NOTVICT);
	act("$n tries to ride you, but falls on $s butt",
	    FALSE, ch, 0, horse, TO_VICT);
	send_to_char("You learn from your mistake.\r\n", ch);
	GET_POS(ch) = POS_SITTING;
	WAIT_STATE(ch, PULSE_VIOLENCE*2);
     }
    } else {
      send_to_char("You can't ride that!\n\r", ch);
      return;
    }
  } else if (CMD_IS("dismount")) {
      horse = MOUNTED(ch);
      if ((GET_POS(ch) != POS_MOUNTED) || (!MOUNTED(ch))) {
         MOUNTED(ch) = NULL;
         GET_POS(ch) = POS_STANDING;
	 send_to_char("Okay you get off your invisible horse.\r\n",ch);
	 return;
      }

      if ((ch->in_room) != (horse->in_room)) {
	Dismount(ch, MOUNTED(ch), POS_STANDING);
	return;
      }
      act("You dismount from $N", FALSE, ch, 0, horse, TO_CHAR);
      act("$n dismounts from $N", FALSE, ch, 0, horse, TO_NOTVICT);
      act("$n dismounts from you", FALSE, ch, 0, horse, TO_VICT);
      Dismount(ch, MOUNTED(ch), POS_STANDING);
      return;
  }
}

void MountThrow(struct char_data *ch, struct char_data *h, int pos)
{
}

void MountTrap(struct char_data *ch, struct char_data *h, int pos)
{
}

void Dismount(struct char_data *ch, struct char_data *h, int pos)
{
  if (h == NULL) {
     MOUNTED(ch) = NULL;
     GET_POS(ch) = pos;
  } else {
     MOUNTED(ch) = NULL;
     RIDDEN(h)   = NULL;
     GET_POS(ch) = pos;
     if (h->master)
       stop_follower(h);
  }
}

/**
 */
int MountEgoCheck(struct char_data *ch, struct char_data *horse)
{
  int prob, percent, check;

  prob = GET_SKILL(ch, SKILL_RIDE);
  percent = number(1, 101);

  if ( IS_DRAGON(horse) )
     check = number(5, 10);

  if (MOB_FLAGGED(horse, MOB_AGGRESSIVE) || \
     (MOB_FLAGGED(horse, MOB_AGGR_EVIL) && \
      (GET_ALIGNMENT(ch) < -250)) || \
     ((MOB_FLAGGED(horse, MOB_AGGR_NEUTRAL) && \
      ((GET_ALIGNMENT(ch) < -350) || (GET_ALIGNMENT(ch) > 350))) || \
     ((MOB_FLAGGED(horse, MOB_AGGR_GOOD) && \
      (GET_ALIGNMENT(ch) > 250)))))
       check = 10;

  else if (MOB_FLAGGED(horse, MOB_NOCHARM))
       check = 10;

  else if (GET_LEVEL(horse) > GET_LEVEL(ch))
      check = (GET_LEVEL(horse));

  else if (GET_SKILL(ch, SKILL_RIDE) == 0)
      check = -1;

  else if ((GET_LEVEL(horse) < GET_LEVEL(ch)) && \
      (prob > percent))
      check = 1;

  else  check = -1;

  return (check);
}


int RideCheck(struct char_data *ch)
{
    if (GET_SKILL(ch, SKILL_RIDE) > number(1,101)) {
       return(TRUE);
    } else {
	  if (GET_SKILL(ch, SKILL_RIDE) < 90) {
/*          send_to_char("You learn from your mistake.\n\r", ch);*/
	    GET_SKILL(ch, SKILL_RIDE) += 2;
	  }
       return(FALSE);
    }
    return(FALSE);
}


void FallOffMount(struct char_data *ch, struct char_data *h)
{
  act("$n loses control and falls off of $N", FALSE, ch, 0, h, TO_NOTVICT);
  act("$n loses control and falls off of you", FALSE, ch, 0, h, TO_VICT);
  act("You lose control and fall off of $N", FALSE, ch, 0, h, TO_CHAR);
  Dismount(ch, MOUNTED(ch), POS_SITTING);
}


ACMD(do_shadow)
{
  struct char_data *vict;
  char name[200];
  one_argument(argument, name);

  if (SHADOWING(ch) && !*name) {
      SHADOWED(SHADOWING(ch)) = NULL;
      SHADOWING(ch) = NULL;      
      return;
  }
  if (!(vict = get_char_room_vis(ch, name))) {
      send_to_char("Shadow who?\r\n", ch);
      return;
  }

  if (ch->master) {
     send_to_char("You cannot shadow anyone while grouped!\r\n", ch);
     return;
  }

  if (GET_SKILL(ch, SKILL_SHADOW) < (GET_LEVEL(vict) * number(1, 2))) {
     do_follow(ch, GET_NAME(vict), 0, 0);
     act("It looks like $N noticed you.", FALSE, ch, 0, vict, TO_CHAR);
     return;
  }

  if (SHADOWING(ch)) {
     sprintf(buf, "You stop shadowing %s, and start shadowing %s.\r\n",
	    GET_NAME(SHADOWING(ch)), GET_NAME(vict));
     SHADOWING(ch) = vict;
     SHADOWED(vict) = ch;
     add_follower(ch, vict);
     SET_BIT(AFF_FLAGS(ch), AFF_SNEAK);
     return;
  }

  else {
     SHADOWING(ch) = vict;
     SHADOWED(vict) = ch;
     sprintf(buf, "You begin shadowing %s.\r\n", GET_NAME(vict));
     add_follower(ch, vict);
     SET_BIT(AFF_FLAGS(ch), AFF_SNEAK);
     return;
  }

}


ACMD(do_world)
{
    int i;

    sprintf(buf, "Num   Name                          Lvl Rec     Builder       PK/PT\r\n");
    sprintf(buf, "%s-------------------------------------------------------------------\r\n", buf);

    for (i = 0; i <= top_of_zone_table; i++)
	sprintf(buf, "%s%3d   %-30.30s %-12.12s %-12.12s %s\r\n",
		buf, zone_table[i].number, zone_table[i].name,
		zone_table[i].levelrec, zone_table[i].builder,
		((zone_table[i].pkill == 1) ? "Yes" : "No"));

    page_string(ch->desc, buf, 1);
}


ACMD(do_lm)
{
   void charmode_on(struct descriptor_data *d);
   void charmode_off(struct descriptor_data *d);

   if (subcmd) {
     charmode_on(ch->desc);
     send_to_char( "You are now in character mode ...\r\n", ch );
   } else {
     charmode_off(ch->desc);
     send_to_char( "You are now in line mode ...\r\n", ch );
   }
}

/* return a number from 1 to 4 based on how well the PC can learn the skill */
int learn_amount(int skill, struct char_data * ch)
{
  // base percent is based on your intelligence
  int percent = 0;
  extern struct int_app_type int_app[ ];

  assert( ch != NULL );
  percent = int_app[ GET_INT(ch) ].learn;
  
  if (IS_PRAYER(skill))
    percent += ((GET_WIS(ch) + GET_WILL(ch)) >> 1);
  else if (IS_ARCANE(skill))
    percent += ((GET_INT(ch) + GET_WIS(ch)) >> 1);
  else if (IS_THIEVERY(skill))
    percent += ((GET_DEX(ch) + GET_INT(ch)) >> 1);
  else if (IS_COMBAT(skill))
    percent += ((GET_STR(ch) + GET_DEX(ch)) >> 1);

  return percent;
}

void practice(struct char_data *ch,
              struct char_data *gm,
              char *argument, int teaches)
{
   int skill_num, sphere, numtimes = 1, i, j = 51, percent = 0, oldskill;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   void increase_sub_skills(struct char_data *ch, int skill);
   int  learn_amount(int skill, struct char_data *ch);
   int  max_learn(int skill, struct char_data *ch);
   extern char *spells[];

   assert( ch != NULL );
   assert( gm != NULL );
   assert( argument != NULL );
   
   skip_spaces(&argument);

   if (!*argument) {
     sprintf(buf2, "$n says, 'I teach the discipline of %s.'\r\n"
		    "Type 'practice %s <# of pracs>' to learn this discipline.",
	     spells[teaches], spells[teaches]);
     act(buf2, FALSE, gm, 0, 0, TO_ROOM);
     return;
   }
   if (GET_PRACTICES(ch) <= 0) {
     sprintf(buf2, "%s Come back when you are ready to learn from me.", GET_NAME(ch));
     do_tell(gm, buf2, 0, 0);
     return;
   }
   argument = any_two_args(argument, arg1, arg2);

   while (*arg2 && !isdigit((int)*arg2)) {
     strcat(arg1, " ");
     strcat(arg1, arg2);
     if (*argument)
       argument = any_one_arg(argument, arg2);
     else
       *arg2 ='\0';
   }
   skill_num = find_skill_num(arg1);
   sphere = spell_info[skill_num].sphere;

   /* the number of pracs this person wants to spend on skill_num */
   if (*arg2)
     numtimes = atoi(arg2);

   if ((skill_num == teaches) ||
       (spell_info[skill_num].sphere == teaches)) {
     /* this guy can teach 'em, so let's start lernin'! */
     if ((skill_num <= MAIN_SPHERES &&
          GET_SKILL(ch, skill_num) < max_learn(skill_num, ch)) ||
	 ((GET_SKILL(ch, sphere) >= spell_info[skill_num].percent) &&
	  (GET_SKILL(ch, skill_num) < max_learn(skill_num, ch)))) {
       for (i = 0; i < numtimes && GET_PRACTICES(ch); ++i) {
	 /* the player can learn this skill */
         percent = learn_amount( skill_num, ch );
	 /* now increase the skill */
	 oldskill = GET_SKILL(ch, skill_num);
	 GET_SKILL(ch, skill_num) =
           MIN(max_learn(skill_num, ch), GET_SKILL(ch, skill_num) + percent);
	 increase_sub_skills(ch, skill_num);
	 send_to_char("You practice for awhile . . .", ch);
	 GET_PRACTICES(ch)--;
	 strcpy(buf, "\r\n");
	 if (skill_num <= MAX_SPHERES && j >= START_SUBSPHERES) {
	   for (j = (MAX_SPHERES+1); j < MAX_SKILLS; ++j) {
	     if (spell_info[j].sphere != skill_num)
	       continue;
	     if (spell_info[j].percent > oldskill &&
		 spell_info[j].percent <= GET_SKILL(ch, skill_num))
	       sprintf(buf, "%sYou learn about %s.\r\n", buf, spells[j]);
	   }
	 }
	 sprintf(buf, "%sYou now have %d practices left.\r\n", buf, GET_PRACTICES(ch));
	 send_to_char(buf, ch);
       }
     } else if ((GET_SKILL(ch, sphere) >= spell_info[skill_num].percent) &&
		(GET_SKILL(ch, skill_num) >= max_learn(skill_num, ch))) {
       sprintf(buf2, "%s I am unable to teach you anymore about %s.",
               GET_NAME(ch), spells[skill_num]);
       do_tell(gm, buf2, 0, 0);
       return;
     } else if (GET_SKILL(ch, skill_num) >= max_learn(skill_num, ch) ||
		GET_SKILL(ch, skill_num) == 100) {
       sprintf(buf2, "%s I have taught you all that I know of %s.",
               GET_NAME(ch), spells[skill_num]);
       do_tell(gm, buf2, 0, 0);
       return;
     } else {
       sprintf(buf2, "%s You would not understand %s without prior training.",
               GET_NAME(ch), spells[skill_num]);
       do_tell(gm, buf2, 0, 0);
       return;
     }
   } else {
     sprintf(buf2, "%s I do not know of %s.", GET_NAME(ch), arg1);
     do_tell(gm, buf2, 0, 0);
     return;
   }
   return;
}
