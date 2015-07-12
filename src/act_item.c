/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ACT_ITEM.C                                   Based on CircleMUD *
*  Usage: Player level commands, used in the manipulation of objects      *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#define __ACT_ITEM_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern char *drinks[];
extern int drink_aff[][3];
/* For TD 3/25/95 */
extern struct race_data * races;
void break_obj(struct obj_data *o);
void mprog_give_trigger(struct char_data * mob, struct char_data * ch, struct obj_data * obj);
void mprog_bribe_trigger(struct char_data * mob, struct char_data * ch, int amount);
int  check_trigger_trap(struct char_data *vict, void *vo, int trigger_type);
struct obj_data *find_trap(struct char_data *ch, int trigger);
int trap_trig(struct obj_data *trap);
int trap_damage(struct char_data *vict, struct obj_data *trap);
extern int NUM_RACES;
ACMD(do_infobar);

void perform_put(struct char_data * ch, struct obj_data * obj, struct obj_data * cont)
{
  if (GET_OBJ_WEIGHT(cont) + GET_OBJ_WEIGHT(obj) > GET_OBJ_VAL(cont, 0))
    act("$p won't fit in $P.", FALSE, ch, obj, cont, TO_CHAR);
  else {
    obj_from_char(obj);
    obj_to_obj(obj, cont);
    act("You put $p in $P.", FALSE, ch, obj, cont, TO_CHAR);
    act("$n puts $p in $P.", TRUE, ch, obj, cont, TO_ROOM);
  }
}


/* The following put modes are supported by the code below:

	1) put <object> <container>
	2) put all.<object> <container>
	3) put all <container>

	<container> must be in inventory or on ground.
	all objects to be put into container must be in inventory.
*/

ACMD(do_put)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj, *cont;
  struct char_data *tmp_char;
  int obj_dotmode, cont_dotmode, found = 0;

  two_arguments(argument, arg1, arg2);
  obj_dotmode = find_all_dots(arg1);
  cont_dotmode = find_all_dots(arg2);

  if (!*arg1)
    send_to_char("Put what in what?\r\n", ch);
  else if (cont_dotmode != FIND_INDIV)
    send_to_char("You can only put things into one container at a time.\r\n", ch);
  else if (!*arg2) {
    sprintf(buf, "What do you want to put %s in?\r\n",
	    ((obj_dotmode == FIND_INDIV) ? "it" : "them"));
    send_to_char(buf, ch);
  } else {
    generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
    if (!cont) {
      sprintf(buf, "You don't see %s %s here.\r\n", AN(arg2), arg2);
      send_to_char(buf, ch);
    } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
      act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
    else if (IS_SET(GET_OBJ_VAL(cont, 1), CONT_CLOSED))
      send_to_char("You'd better open it first!\r\n", ch);
    else {
      if (obj_dotmode == FIND_INDIV) {  /* put <obj> <container> */
	if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	  sprintf(buf, "You aren't carrying %s %s.\r\n", AN(arg1), arg1);
	  send_to_char(buf, ch);
	} else if (obj == cont)
	  send_to_char("You attempt to fold it into itself, but fail.\r\n", ch);
	else
	  perform_put(ch, obj, cont);
      } else {
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (obj != cont && CAN_SEE_OBJ(ch, obj) &&
	      (obj_dotmode == FIND_ALL || isname(arg1, obj->name))) {
	    found = 1;
	    perform_put(ch, obj, cont);
	  }
	}
	if (!found) {
	  if (obj_dotmode == FIND_ALL)
	    send_to_char("You don't seem to have anything to put in it.\r\n", ch);
	  else {
	    sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
	    send_to_char(buf, ch);
	  }
	}
      }
    }
  }
}



int can_take_obj(struct char_data * ch, struct obj_data * obj)
{
  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
    act("$p: you can't carry that many items.", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
    act("$p: you can't carry that much weight.", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if (!(CAN_WEAR(obj, ITEM_WEAR_TAKE))) {
    act("$p: you can't take that!", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  }
  return 1;
}


void get_check_money(struct char_data * ch, struct obj_data * obj)
{
  if ((GET_OBJ_TYPE(obj) == ITEM_MONEY) && (GET_OBJ_VAL(obj, 0) > 0)) {
    obj_from_char(obj);
    if (GET_OBJ_VAL(obj, 0) > 1) {
      sprintf(buf, "There were %ld coins.\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    GET_GOLD(ch) += GET_OBJ_VAL(obj, 0);
    extract_obj(obj);
  }
}


void perform_get_from_container(struct char_data * ch, struct obj_data * obj,
				     struct obj_data * cont, int mode)
{
  if (mode == FIND_OBJ_INV || can_take_obj(ch, obj)) {
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
      act("$p: you can't hold any more items.", FALSE, ch, obj, 0, TO_CHAR);
    else {
      obj_from_obj(obj);
      obj_to_char(obj, ch);
      act("You get $p from $P.", FALSE, ch, obj, cont, TO_CHAR);
      act("$n gets $p from $P.", TRUE, ch, obj, cont, TO_ROOM);
      get_check_money(ch, obj);
    }
  }
}


void get_from_container(struct char_data * ch, struct obj_data * cont,
			     char *arg, int mode)
{
  struct obj_data *obj, *next_obj;
  int obj_dotmode, found = 0;

  obj_dotmode = find_all_dots(arg);

  if (IS_SET(GET_OBJ_VAL(cont, 1), CONT_CLOSED))
    act("$p is closed.", FALSE, ch, cont, 0, TO_CHAR);
  else if (obj_dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, cont->contains))) {
      sprintf(buf, "There doesn't seem to be %s %s in $p.", AN(arg), arg);
      act(buf, FALSE, ch, cont, 0, TO_CHAR);
    } else
      perform_get_from_container(ch, obj, cont, mode);
  } else {
    if (obj_dotmode == FIND_ALLDOT && !*arg) {
      send_to_char("Get all of what?\r\n", ch);
      return;
    }
    for (obj = cont->contains; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) &&
	  (obj_dotmode == FIND_ALL || isname(arg, obj->name))) {
	found = 1;
	perform_get_from_container(ch, obj, cont, mode);
      }
    }
    if (!found) {
      if (obj_dotmode == FIND_ALL)
	act("$p seems to be empty.", FALSE, ch, cont, 0, TO_CHAR);
      else {
	sprintf(buf, "You can't seem to find any %ss in $p.", arg);
	act(buf, FALSE, ch, cont, 0, TO_CHAR);
      }
    }
  }
}


int perform_get_from_room(struct char_data * ch, struct obj_data * obj)
{
  if (GET_OBJ_TYPE(obj) == ITEM_TRAP)
    if (check_trigger_trap(ch, obj, TRIG_GET_OBJ))
      return 0;
  if (can_take_obj(ch, obj)) {
    obj_from_room(obj);
    obj_to_char(obj, ch);
    act("You get $p.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n gets $p.", TRUE, ch, obj, 0, TO_ROOM);
    get_check_money(ch, obj);
    return 1;
  }
  return 0;
}


void get_from_room(struct char_data * ch, char *arg)
{
  struct obj_data *obj, *next_obj;
  int dotmode, found = 0;

  dotmode = find_all_dots(arg);

  if (dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
      sprintf(buf, "You don't see %s %s here.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
    } else
      perform_get_from_room(ch, obj);
  } else {
    if (dotmode == FIND_ALLDOT && !*arg) {
      send_to_char("Get all of what?\r\n", ch);
      return;
    }
    for (obj = world[ch->in_room].contents; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) &&
	  (dotmode == FIND_ALL || isname(arg, obj->name))) {
	found = 1;
	perform_get_from_room(ch, obj);
      }
    }
    if (!found) {
      if (dotmode == FIND_ALL)
	send_to_char("There doesn't seem to be anything here.\r\n", ch);
      else {
	sprintf(buf, "You don't see any %ss here.\r\n", arg);
	send_to_char(buf, ch);
      }
    }
  }
}



ACMD(do_get)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  int    cont_dotmode, found = 0, mode;
  struct obj_data *cont;
  struct char_data *tmp_char;

  two_arguments(argument, arg1, arg2);

  if ((FIGHTING(ch) != NULL) && number(0, 1))
    send_to_char("You are unable to break away from combat to get it!\r\n", ch);
  else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    send_to_char("Your arms are already full!\r\n", ch);
  else if (!*arg1)
    send_to_char("Get what?\r\n", ch);
  else if (!*arg2)
    get_from_room(ch, arg1);
  else {
    cont_dotmode = find_all_dots(arg2);
    if (cont_dotmode == FIND_INDIV) {
      mode = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
      if (!cont) {
	sprintf(buf, "You don't have %s %s.\r\n", AN(arg2), arg2);
	send_to_char(buf, ch);
      } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
	act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
      else
	get_from_container(ch, cont, arg1, mode);
    } else {
      if (cont_dotmode == FIND_ALLDOT && !*arg2) {
	send_to_char("Get from all of what?\r\n", ch);
	return;
      }
      for (cont = ch->carrying; cont; cont = cont->next_content) {
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
	    found = 1;
	    get_from_container(ch, cont, arg1, FIND_OBJ_INV);
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    found = 1;
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	  }
        }
      }
      for (cont = world[ch->in_room].contents; cont;
           cont = cont->next_content) {
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
	    get_from_container(ch, cont, arg1, FIND_OBJ_ROOM);
	    found = 1;
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	    found = 1;
	  }
        }
      }
      if (!found) {
	if (cont_dotmode == FIND_ALL)
	  send_to_char("You can't seem to find any containers.\r\n", ch);
	else {
	  sprintf(buf, "You can't seem to find any %ss here.\r\n", arg2);
	  send_to_char(buf, ch);
	}
      }
    }
  }
}


void perform_drop_gold(struct char_data * ch, int amount,
			    byte mode, sh_int RDR)
{
  struct obj_data *obj;

  if (amount <= 0)
    send_to_char("Heh heh heh.. we are jolly funny today, eh?\r\n", ch);
  else if (GET_GOLD(ch) < amount)
    send_to_char("You don't have that many coins!\r\n", ch);
  else {
    if (mode != SCMD_JUNK) {
      WAIT_STATE(ch, PULSE_VIOLENCE);   /* to prevent coin-bombing */
      obj = create_money(amount);
      if (mode == SCMD_DONATE) {
	send_to_char("You throw some gold into the air where it disappears in a puff of smoke!\r\n", ch);
	act("$n throws some gold into the air where it disappears in a puff of smoke!",
	    FALSE, ch, 0, 0, TO_ROOM);
	obj_to_room(obj, RDR);
	act("$p suddenly appears in a puff of orange smoke!", 0, 0, obj, 0, TO_ROOM);
      } else {
	send_to_char("You drop some gold.\r\n", ch);
	sprintf(buf, "$n drops %s.", money_desc(amount));
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	obj_to_room(obj, ch->in_room);
      }
    } else {
      sprintf(buf, "$n drops %s which disappears in a puff of smoke!",
	      money_desc(amount));
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You drop some gold which disappears in a puff of smoke!\r\n", ch);
    }
    GET_GOLD(ch) -= amount;
  }
}


#define VANISH(mode) ((mode == SCMD_DONATE || mode == SCMD_JUNK) ? \
		      "  It vanishes in a puff of smoke!" : "")

int perform_drop(struct char_data * ch, struct obj_data * obj,
		     byte mode, char *sname, sh_int RDR)
{
  int value;

  if (GET_OBJ_TYPE(obj) == ITEM_TRAP)
    if (check_trigger_trap(ch, obj, TRIG_DROP_OBJ))
      return 0;

  if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
    sprintf(buf, "You can't %s $p, it must be CURSED!", sname);
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  }
  sprintf(buf, "You %s $p.%s", sname, VANISH(mode));
  act(buf, FALSE, ch, obj, 0, TO_CHAR);
  sprintf(buf, "$n %ss $p.%s", sname, VANISH(mode));
  act(buf, TRUE, ch, obj, 0, TO_ROOM);
  obj_from_char(obj);

  if ((mode == SCMD_DONATE) && IS_OBJ_STAT(obj, ITEM_NODONATE))
    mode = SCMD_JUNK;

  switch (mode) {
  case SCMD_DROP:
    obj_to_room(obj, ch->in_room);
    return 0;
    break;
  case SCMD_DONATE:
    obj_to_room(obj, RDR);
    act("$p suddenly appears in a puff a smoke!", FALSE, 0, obj, 0, TO_ROOM);
    return 0;
    break;
  case SCMD_JUNK:
    value = MAX(1, MIN(200, GET_OBJ_COST(obj) >> 4));
    extract_obj(obj);
    return value;
    break;
  default:
    log("SYSERR: Incorrect argument passed to perform_drop");
    break;
  }

  return 0;
}



ACMD(do_drop)
{
  extern rnum donation_room_1;
#if 0
  extern rnum donation_room_2;  /* uncomment if needed! */
  extern rnum donation_room_3;  /* uncomment if needed! */
  extern rnum donation_room_4;
  extern rnum donation_room_5;
  extern rnum donation_room_6;
#endif
  struct obj_data *obj, *next_obj;
  rnum RDR = 0;
  byte mode = SCMD_DROP;
  int dotmode, amount = 0;
  char *sname;

  switch (subcmd) {
  case SCMD_JUNK:
    sname = "junk";
    mode = SCMD_JUNK;
    break;
  case SCMD_DONATE:
    sname = "donate";
    mode = SCMD_DONATE;
    switch (number(0, 2)) {
    case 0:
      mode = SCMD_JUNK;
      break;
    case 1:
    case 2:
      RDR = real_room(donation_room_1);
      break;
/*    case 3: RDR = real_room(donation_room_2); break;
      case 4: RDR = real_room(donation_room_3); break;
*/
    }
    if (RDR == NOWHERE) {
      send_to_char("Sorry, you can't donate anything right now.\r\n", ch);
      return;
    }
    break;
  default:
    sname = "drop";
    break;
  }

  argument = one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "What do you want to %s?\r\n", sname);
    send_to_char(buf, ch);
    return;
  } else if (is_number(arg)) {
    amount = atoi(arg);
    argument = one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg))
      perform_drop_gold(ch, amount, mode, RDR);
    else {
      /* code to drop multiple items.  anyone want to write it? -je */
      send_to_char("Sorry, you can't do that to more than one item at a time.\r\n", ch);
    }
    return;
  } else {
    dotmode = find_all_dots(arg);

    /* Can't junk or donate all */
    if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE)) {
      if (subcmd == SCMD_JUNK)
	send_to_char("Go to the dump if you want to junk EVERYTHING!\r\n", ch);
      else
	send_to_char("Go do the donation room if you want to donate EVERYTHING!\r\n", ch);
      return;
    }
    if (dotmode == FIND_ALL) {
      if (!ch->carrying)
	send_to_char("You don't seem to be carrying anything.\r\n", ch);
      else
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  amount += perform_drop(ch, obj, mode, sname, RDR);
	}
    } else if (dotmode == FIND_ALLDOT) {
      if (!*arg) {
	sprintf(buf, "What do you want to %s all of?\r\n", sname);
	send_to_char(buf, ch);
	return;
      }
      if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf, "You don't seem to have any %ss.\r\n", arg);
	send_to_char(buf, ch);
      }
      while (obj) {
	next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
	amount += perform_drop(ch, obj, mode, sname, RDR);
	obj = next_obj;
      }
    } else {
      if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf, ch);
      } else
	amount += perform_drop(ch, obj, mode, sname, RDR);
    }
  }

  if (amount && (subcmd == SCMD_JUNK)) {
    send_to_char("You have been rewarded by the gods!\r\n", ch);
    act("$n has been rewarded by the gods!", TRUE, ch, 0, 0, TO_ROOM);
    GET_GOLD(ch) += amount;
  }
}


void perform_give(struct char_data * ch, struct char_data * vict,
		       struct obj_data * obj)
{
  if (IS_OBJ_STAT(obj, ITEM_NODROP) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    act("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0, TO_CHAR);
    return;
  }
  if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict)) {
    act("$N seems to have $S hands full.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if ((GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(vict)) > CAN_CARRY_W(vict)) {
    act("$E can't carry that much weight.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  obj_from_char(obj);
  obj_to_char(obj, vict);
  act("You give $p to $N.", FALSE, ch, obj, vict, TO_CHAR);
  act("$n gives you $p.", FALSE, ch, obj, vict, TO_VICT);
  act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_NOTVICT);
  /* Used for mob_prog give stuff */
  mprog_give_trigger(vict, ch, obj);
}

/* utility function for give */
struct char_data *give_find_vict(struct char_data * ch, char *arg)
{
  struct char_data *vict;

  if (!*arg) {
    send_to_char("To who?\r\n", ch);
    return NULL;
  } else if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return NULL;
  } else if (vict == ch) {
    send_to_char("What's the point of that?\r\n", ch);
    return NULL;
  } else
    return vict;
}


void perform_give_gold(struct char_data * ch, struct char_data * vict,
			    int amount)
{
  if (amount <= 0) {
    send_to_char("Heh heh heh ... we are jolly funny today, eh?\r\n", ch);
    return;
  }
  if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_GOD))) {
    send_to_char("You don't have that many coins!\r\n", ch);
    return;
  }
  send_to_char(OK, ch);
  sprintf(buf, "$n gives you %d gold coins.", amount);
  act(buf, FALSE, ch, 0, vict, TO_VICT);
  sprintf(buf, "$n gives %s to $N.", money_desc(amount));
  act(buf, TRUE, ch, 0, vict, TO_NOTVICT);
  if (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_GOD))
    GET_GOLD(ch) -= amount;
  GET_GOLD(vict) += amount;
  mprog_bribe_trigger(vict, ch, amount);
}


ACMD(do_give)
{
  int amount, dotmode;
  struct char_data *vict;
  struct obj_data *obj, *next_obj;

  argument = one_argument(argument, arg);

  if (!*arg)
    send_to_char("Give what to who?\r\n", ch);
  else if (is_number(arg)) {
    amount = atoi(arg);
    argument = one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg)) {
      argument = one_argument(argument, arg);
      if ((vict = give_find_vict(ch, arg)))
	perform_give_gold(ch, vict, amount);
      return;
    } else {
      /* code to give multiple items.  anyone want to write it? -je */
      send_to_char("You can't give more than one item at a time.\r\n", ch);
      return;
    }
  } else {
    one_argument(argument, buf1);
    if (!(vict = give_find_vict(ch, buf1)))
      return;
    dotmode = find_all_dots(arg);
    if (dotmode == FIND_INDIV) {
      if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf, ch);
      } else
	perform_give(ch, vict, obj);
    } else {
      if (dotmode == FIND_ALLDOT && !*arg) {
	send_to_char("All of what?\r\n", ch);
	return;
      }
      if (!ch->carrying)
	send_to_char("You don't seem to be holding anything.\r\n", ch);
      else
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (CAN_SEE_OBJ(ch, obj) &&
	      ((dotmode == FIND_ALL || isname(arg, obj->name))))
	    perform_give(ch, vict, obj);
	}
    }
  }
}


/* Everything from here down is what was formerly act.obj2.c */


void weight_change_object(struct obj_data * obj, int weight)
{
  struct obj_data *tmp_obj;
  struct char_data *tmp_ch;

  if (obj->in_room != NOWHERE) {
    GET_OBJ_WEIGHT(obj) += weight;
  } else if ((tmp_ch = obj->carried_by)) {
    obj_from_char(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_char(obj, tmp_ch);
  } else if ((tmp_obj = obj->in_obj)) {
    obj_from_obj(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_obj(obj, tmp_obj);
  } else {
    log("SYSERR: Unknown attempt to subtract weight from an object.");
  }
}



void name_from_drinkcon(struct obj_data * obj)
{
  int i;
  char *new_name;
  extern struct obj_data *obj_proto;

  for (i = 0; (*((obj->name) + i) != ' ') && (*((obj->name) + i) != '\0'); i++);

  if (*((obj->name) + i) == ' ') {
    new_name = str_dup((obj->name) + i + 1);
    if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
      free(obj->name);
    obj->name = new_name;
  }
}



void name_to_drinkcon(struct obj_data * obj, int type)
{
  char *new_name;
  extern struct obj_data *obj_proto;
  extern char *drinknames[];

  CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
  sprintf(new_name, "%s %s", drinknames[type], obj->name);
  if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
    free(obj->name);
  obj->name = new_name;
}



ACMD(do_drink)
{
  struct obj_data *temp;
  struct affected_type af;
  int amount, weight;
  int on_ground = 0;

  one_argument(argument, arg);

  if (GET_POS(ch) == POS_FIGHTING) {
    send_to_char("Yeah right, in the middle of combat?!?!?\r\n", ch);
    return;
  }
  if (!*arg) {
    send_to_char("Drink from what?\r\n", ch);
    return;
  }
  if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    if (!(temp = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
      act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    } else
      on_ground = 1;
  }
  if ((GET_OBJ_TYPE(temp) != ITEM_DRINKCON) &&
      (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)) {
    send_to_char("You can't drink from that!\r\n", ch);
    return;
  }
  if (on_ground && (GET_OBJ_TYPE(temp) == ITEM_DRINKCON)) {
    send_to_char("You have to be holding that to drink from it.\r\n", ch);
    return;
  }
  if ((GET_COND(ch, DRUNK) > 10) && (GET_COND(ch, THIRST) > 0)) {
    /* The pig is drunk */
    send_to_char("You can't seem to get close enough to your mouth.\r\n", ch);
    act("$n tries to drink but misses $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
  if ((GET_COND(ch, FULL) > 20) && (GET_COND(ch, THIRST) > 0)) {
    send_to_char("Your stomach can't contain anymore!\r\n", ch);
    return;
  }
  if (!GET_OBJ_VAL(temp, 1)) {
    send_to_char("It's empty.\r\n", ch);
    return;
  }
  if (subcmd == SCMD_DRINK) {
    sprintf(buf, "$n drinks %s from $p.", drinks[GET_OBJ_VAL(temp, 2)]);
    act(buf, TRUE, ch, temp, 0, TO_ROOM);

    sprintf(buf, "You drink the %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
    send_to_char(buf, ch);

    if (drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] > 0)
      amount = (25 - GET_COND(ch, THIRST)) / drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK];
    else
      amount = number(3, 10);

  } else {
    act("$n sips from $p.", TRUE, ch, temp, 0, TO_ROOM);
    sprintf(buf, "It tastes like %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
    send_to_char(buf, ch);
    amount = 1;
  }

  amount = MIN((int)amount, (int)GET_OBJ_VAL(temp, 1));

  /* You can't subtract more than the object weighs */
  weight = MIN((int)amount, (int)GET_OBJ_WEIGHT(temp));

  weight_change_object(temp, -weight);  /* Subtract amount */

  gain_condition(ch, DRUNK,
	 (int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] * amount) / 4);

  gain_condition(ch, FULL,
	  (int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][FULL] * amount) / 4);

  gain_condition(ch, THIRST,
	(int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][THIRST] * amount) / 4);

  if (GET_COND(ch, DRUNK) > 10)
    send_to_char("You feel drunk.\r\n", ch);

  if (GET_COND(ch, THIRST) > 20)
    send_to_char("You don't feel thirsty any more.\r\n", ch);

  if (GET_COND(ch, FULL) > 20)
    send_to_char("You are full.\r\n", ch);

  /* The shit was poisoned ! */
  if (GET_OBJ_VAL(temp, 3) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("Oops, it tasted rather strange!\r\n", ch);
    act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);
    
    switch (GET_OBJ_VAL(temp, 3)) {
       case 2:
	 af.type      = SPELL_POISON;
	 af.duration  = amount * 3;
	 af.modifier  = 0;
	 af.location  = APPLY_NONE;
	 af.bitvector = AFF_POISON_II;
	 break;
       case 3:
	 af.type      = SPELL_POISON;
	 af.duration  = amount * 2;
	 af.modifier  = 0;
	 af.location  = APPLY_NONE;
	 af.bitvector = AFF_POISON_III;
	 break;
       case 4:
	 af.type      = SPELL_DISEASE;
	 af.duration  = -1;          /* Permanent! */
	 af.modifier  = 0;
	 af.location  = APPLY_NONE;
	 af.bitvector = AFF_DISEASE;
	 break;
       case 1:
       default:
	 af.type      = SPELL_POISON;
	 af.duration  = amount * 4;
	 af.modifier  = 0;
	 af.location  = APPLY_NONE;
	 af.bitvector = AFF_POISON_I;
	 break;
    }
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE, TRUE);
  }
  /* empty the container, and no longer poison. */
  GET_OBJ_VAL(temp, 1) -= amount;
  if (!GET_OBJ_VAL(temp, 1)) {  /* The last bit */
    GET_OBJ_VAL(temp, 2) = 0;
    GET_OBJ_VAL(temp, 3) = 0;
#if 0
    name_from_drinkcon(temp);
#endif
  }
  return;
}



ACMD(do_eat)
{
  struct obj_data *food;
  struct affected_type af;
  int amount;

  one_argument(argument, arg);

  if (GET_POS(ch) == POS_FIGHTING) {
    send_to_char("Yeah right, in the middle of combat?!?!?\r\n", ch);
    return;
  }
  if (!*arg) {
    send_to_char("Eat what?\r\n", ch);
    return;
  }
  if (!(food = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
    return;
  }
  if (subcmd == SCMD_TASTE && ((GET_OBJ_TYPE(food) == ITEM_DRINKCON) ||
			       (GET_OBJ_TYPE(food) == ITEM_FOUNTAIN))) {
    do_drink(ch, argument, 0, SCMD_SIP);
    return;
  }
  if ((GET_OBJ_TYPE(food) != ITEM_FOOD) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("You can't eat THAT!\r\n", ch);
    return;
  }
  if (GET_COND(ch, FULL) > 20) {/* Stomach full */
    act("You are too full to eat more!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (subcmd == SCMD_EAT) {
    act("You eat the $o.", FALSE, ch, food, 0, TO_CHAR);
    act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
  } else {
    act("You nibble a little bit of the $o.", FALSE, ch, food, 0, TO_CHAR);
    act("$n tastes a little bit of $p.", TRUE, ch, food, 0, TO_ROOM);
  }

  amount = (subcmd == SCMD_EAT ? GET_OBJ_VAL(food, 0) : 1);

  gain_condition(ch, FULL, amount);

  if (GET_COND(ch, FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  if ((GET_OBJ_VAL(food, 3) || (GET_OBJ_VAL(food, 9) >= 128))
      && (GET_LEVEL(ch) < LVL_IMMORT)) {
    /* The shit was poisoned ! */
    send_to_char("Oops, that tasted rather strange!\r\n", ch);
    act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);
    
    if (GET_OBJ_VAL(food, 9) >= 72) {
      GET_OBJ_VAL(food, 3) = number(4, 6);
      amount = 1;
    }
    switch (GET_OBJ_VAL(food, 3)) {
       case 2:
	 af.type      = SPELL_POISON;
	 af.duration  = amount * 3;
	 af.modifier  = 0;
	 af.location  = APPLY_NONE;
	 af.bitvector = AFF_POISON_II;
	 break;
       case 3:
	 af.type      = SPELL_POISON;
	 af.duration  = amount * 2;
	 af.modifier  = 0;
	 af.location  = APPLY_NONE;
	 af.bitvector = AFF_POISON_III;
	 break;
       case 4:
	 af.type      = SPELL_DISEASE;
	 af.duration  = amount * 24;          /* Permanent! */
	 af.modifier  = 0;
	 af.location  = APPLY_NONE;
	 af.bitvector = AFF_DISEASE;
	 break;
       case 1:
       default:
	 af.type      = SPELL_POISON;
	 af.duration  = amount * dice(1, 4);
	 af.modifier  = 0;
	 af.location  = APPLY_NONE;
	 af.bitvector = AFF_POISON_I;
	 break;
    }
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE, TRUE);
  }
  if (subcmd == SCMD_EAT)
    extract_obj(food);
  else {
    if (!(--GET_OBJ_VAL(food, 0))) {
      send_to_char("There's nothing left now.\r\n", ch);
      extract_obj(food);
    }
  }
}


ACMD(do_pour)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *from_obj = NULL, *to_obj = NULL;
  int amount;

  two_arguments(argument, arg1, arg2);

  if (subcmd == SCMD_POUR) {
    if (!*arg1) {               /* No arguments */
      act("From what do you want to pour?", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_DRINKCON) {
      act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  }
  if (subcmd == SCMD_FILL) {
    if (!*arg1) {               /* no arguments */
      send_to_char("What do you want to fill?  And what are you filling it from?\r\n", ch);
      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      send_to_char("You can't find it!", ch);
      return;
    }
    if (GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) {
      act("You can't fill $p!", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!*arg2) {               /* no 2nd argument */
      act("What do you want to fill $p from?", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg2, world[ch->in_room].contents))) {
      sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg2), arg2);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN) {
      act("You can't fill something from $p.", FALSE, ch, from_obj, 0, TO_CHAR);
      return;
    }
  }
  if (GET_OBJ_VAL(from_obj, 1) == 0) {
    act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
    return;
  }
  if (subcmd == SCMD_POUR) {    /* pour */
    if (!*arg2) {
      act("Where do you want it?  Out or in what?", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if (!str_cmp(arg2, "out")) {
      act("$n empties $p.", TRUE, ch, from_obj, 0, TO_ROOM);
      act("You empty $p.", FALSE, ch, from_obj, 0, TO_CHAR);

      weight_change_object(from_obj, -GET_OBJ_VAL(from_obj, 1)); /* Empty */

      GET_OBJ_VAL(from_obj, 1) = 0;
      GET_OBJ_VAL(from_obj, 2) = 0;
      GET_OBJ_VAL(from_obj, 3) = 0;
#if 0
      name_from_drinkcon(from_obj);
#endif
      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
      act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if ((GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) &&
	(GET_OBJ_TYPE(to_obj) != ITEM_FOUNTAIN)) {
      act("You can't pour anything into that.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  }
  if (to_obj == from_obj) {
    act("A most unproductive effort.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if ((GET_OBJ_VAL(to_obj, 1) != 0) &&
      (GET_OBJ_VAL(to_obj, 2) != GET_OBJ_VAL(from_obj, 2))) {
    act("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (!(GET_OBJ_VAL(to_obj, 1) < GET_OBJ_VAL(to_obj, 0))) {
    act("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (subcmd == SCMD_POUR) {
    sprintf(buf, "You pour the %s into the %s.",
	    drinks[GET_OBJ_VAL(from_obj, 2)], arg2);
    send_to_char(buf, ch);
  }
  if (subcmd == SCMD_FILL) {
    act("You gently fill $p from $P.", FALSE, ch, to_obj, from_obj, TO_CHAR);
    act("$n gently fills $p from $P.", TRUE, ch, to_obj, from_obj, TO_ROOM);
  }
  /* New alias */
#if 0
  if (GET_OBJ_VAL(to_obj, 1) == 0)
    name_to_drinkcon(to_obj, GET_OBJ_VAL(from_obj, 2));
#endif

  /* First same type liq. */
  GET_OBJ_VAL(to_obj, 2) = GET_OBJ_VAL(from_obj, 2);

  /* Then how much to pour */
  GET_OBJ_VAL(from_obj, 1) -= (amount =
			 (GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1)));

  GET_OBJ_VAL(to_obj, 1) = GET_OBJ_VAL(to_obj, 0);

  if (GET_OBJ_VAL(from_obj, 1) < 0) {   /* There was too little */
    GET_OBJ_VAL(to_obj, 1) += GET_OBJ_VAL(from_obj, 1);
    amount += GET_OBJ_VAL(from_obj, 1);
    GET_OBJ_VAL(from_obj, 1) = 0;
    GET_OBJ_VAL(from_obj, 2) = 0;
    GET_OBJ_VAL(from_obj, 3) = 0;
#if 0
    name_from_drinkcon(from_obj);
#endif
  }
  /* Then the poison boogie */
  GET_OBJ_VAL(to_obj, 3) =
    (GET_OBJ_VAL(to_obj, 3) || GET_OBJ_VAL(from_obj, 3));

  /* And the weight boogie */
  weight_change_object(from_obj, -amount);
  weight_change_object(to_obj, amount); /* Add weight */

  return;
}



void wear_message(struct char_data * ch, struct obj_data * obj, int where)
{
  char *wear_messages[][2] = {
    {"$n slides $p on to $s right ring finger.",
    "You slide $p on to your right ring finger."},

    {"$n slides $p on to $s left ring finger.",
    "You slide $p on to your left ring finger."},

    {"$n wears $p around $s neck.",
    "You wear $p around your neck."},

    {"$n wears $p around $s neck.",
    "You wear $p around your neck."},

    {"$n wears $p on $s body.",
    "You wear $p on your body.",},

    {"$n wears $p on $s head.",
    "You wear $p on your head."},

    {"$n puts $p on $s legs.",
    "You put $p on your legs."},

    {"$n wears $p on $s right foot.",
    "You wear $p on your right foot."},

    {"$n wears $p on $s left foot.",
    "You wear $p on your left foot."},

    {"$n puts $p on $s right hand.",
    "You put $p on your right hand."},

    {"$n puts $p on $s left hand.",
    "You put $p on your left hand."},

    {"$n wears $p on $s right arm.",
    "You wear $p on your right arm."},

    {"$n wears $p on $s left arm.",
    "You wear $p on your left arm."},

    {"$n wears $p on $s back.",
    "You wear $p on your back."},

    {"$n wears $p around $s waist.",
    "You wear $p around your waist."},

    {"$n puts $p on around $s right wrist.",
    "You put $p on around your right wrist."},

    {"$n puts $p on around $s left wrist.",
    "You put $p on around your left wrist."},

    {"$n holds $p in $s right hand.",
    "You hold $p in your right hand."},

    {"$n holds $p in $s left hand.",
    "You hold $p in your left hand."}
  };

  act(wear_messages[where][0], TRUE, ch, obj, 0, TO_ROOM);
  act(wear_messages[where][1], FALSE, ch, obj, 0, TO_CHAR);
}

/* checks to see if obj is right size for char */
int right_size(struct char_data *ch, struct obj_data * obj)
{
    if (IS_OBJ_SIZE(obj, ITEM_SIZE_TINY) && IS_TINY(ch))
       return 1;
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_SMALL) && IS_SMALL(ch))
       return 1;
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_MEDIUM) && IS_MEDIUM(ch))
       return 1;
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_LARGE) && IS_LARGE(ch))
       return 1;
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_HUGE) && IS_HUGE(ch))
       return 1;
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_TINY) && !IS_TINY(ch))
       return 2;      /* you are too big and break it! */
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_SMALL) && IS_TINY(ch))
       return 3;      /* it is too big for you and falls off */
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_SMALL) && !IS_SMALL(ch))
       return 2;      /* you are too big and break it! */
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_MEDIUM) && (IS_SMALL(ch) || IS_TINY(ch)))
       return 3;      /* it is too big for you and falls off */
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_MEDIUM) && !IS_MEDIUM(ch))
       return 2;      /* you are too big and break it! */
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_LARGE) &&
	     (IS_SMALL(ch) || IS_TINY(ch) || IS_MEDIUM(ch)))
       return 3;      /* it is too big for you and falls off */
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_LARGE) && !IS_LARGE(ch))
       return 2;      /* you are too big and break it! */
    else if (IS_OBJ_SIZE(obj, ITEM_SIZE_HUGE) && !IS_HUGE(ch))
       return 3;      /* it is too big for you and falls off */
    return 1;
}

/* scraps the object or just lays it on the ground */
void scrap_obj(struct obj_data *obj, struct char_data *ch, int mode)
{
   int location = 0;

   /* make some scraps */
   if (mode != 3) {
     if (obj->name)
       free(obj->name);
     obj->name = str_dup("scrap pile crap");
     if (obj->description)
       free(obj->description);
     obj->description = str_dup("A pile of useless scrap is lying here.");
     if (obj->short_description)
       free(obj->short_description);
     obj->short_description = str_dup("useless scrap");
     GET_OBJ_TYPE(obj)   = ITEM_TRASH;
     GET_OBJ_WEAR(obj)   = ITEM_WEAR_TAKE;
     GET_OBJ_EXTRA(obj)  = ITEM_NODONATE;
     GET_OBJ_VAL(obj, 9) = 99;
   }
   /* now send the appriopriate mesg */
   if (mode == 1) {
     act("$n frantically discards $p, due to the extreme damage it has absorbed.", TRUE, ch, obj, NULL, TO_ROOM);
     act("You frantically discard $p, due to the extreme damage it has absorbed.",  FALSE, ch, obj, NULL, TO_CHAR);
   } else if (mode == 2) {
     act("$n tries to wear $p, but it is too small for $m and $e breaks it.",
	TRUE, ch, obj, NULL, TO_ROOM);
     act("You try to wear $p, but it is too small for you and it breaks!",
	FALSE, ch, obj, NULL, TO_CHAR);
   } else if (mode == 3) {
     act("$n tries to wear $p, but it is too big for $m and falls off.",
	TRUE, ch, obj, NULL, TO_ROOM);
     act("You try to wear $p, but it is too big for you and falls off.",
	FALSE, ch, obj, NULL, TO_CHAR);
   }
   if (mode != 1)
     obj_from_char(obj);
   else {
     for (location = 0; location < NUM_WEARS; location++)
       if (GET_EQ(ch, location) == obj)
	 obj_from_char(unequip_char(ch, location));
   }
   obj_to_room(obj, ch->in_room);
}

void perform_wear(struct char_data * ch, struct obj_data * obj, int where)
{
  int i;
  /*
   * ITEM_WEAR_TAKE is used for objects that do not require special bits
   * to be put into that position (e.g. you can hold any object, not just
   * an object with a HOLD bit.)
   */

  int wear_bitvectors[] = {
    /* No lights */ ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
    ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
    ITEM_WEAR_FEET, ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_HANDS,
    ITEM_WEAR_ARMS, ITEM_WEAR_ARMS, ITEM_WEAR_BACK, ITEM_WEAR_WAIST,
    ITEM_WEAR_WRIST, ITEM_WEAR_WRIST, ITEM_WEAR_TAKE, ITEM_WEAR_TAKE};

  char *already_wearing[] = {
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both of your ring fingers.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You can't wear anything else around your neck.\r\n",
    "You're already wearing something on your body.\r\n",
    "You're already wearing something on your head.\r\n",
    "You're already wearing something on your legs.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both of your feet.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both of your hands.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both your arms.\r\n",
    "You're already wearing something on your back.\r\n",
    "You already have something around your waist.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something around both of your wrists.\r\n",
    "You're already wielding something in your hand.\r\n",
    "You're already holding something in your hand.\r\n"
  };

  /* first, make sure the object is usable */
  if (IS_OBJ_STAT(obj, ITEM_BROKEN)) {
     send_to_char("You cannot use a broken item!\r\n", ch);
     return;
  }

  /* second, make sure that the wear position is valid. */
  if (!CAN_WEAR(obj, wear_bitvectors[where])) {
    act("You can't wear $p there.", FALSE, ch, obj, 0, TO_CHAR);
    return;
  }

  /* for finger, neck, wrist, foot, hand, arm, and hold try pos 2 if pos 1 is already full */
  if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) ||
      (where == WEAR_WRIST_R) || (where == WEAR_FOOT_R) ||
      (where == WEAR_HAND_R) || (where == WEAR_ARM_R))
     if (GET_EQ(ch, where))
	where++;

  if (GET_EQ(ch, where)) {
    send_to_char(already_wearing[where], ch);
    return;
  }

  /* added so that there are sizes to equipment - TD 3/14/96 */
  if ((i = right_size(ch, obj)) != 1)
    scrap_obj(obj, ch, i);

  wear_message(ch, obj, where);
  obj_from_char(obj);
  equip_char(ch, obj, where);
}


int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg)
{
  int where = -1;

  static char *keywords[] = {
    "finger",
    "!RESERVED!",
    "neck",
    "!RESERVED!",
    "body",
    "head",
    "legs",
    "foot",
    "!RESERVED!",
    "hand",
    "!RESERVED!",
    "arm",
    "!RESERVED!",
    "back",
    "waist",
    "wrist",
    "!RESERVED!",
    "!RESERVED!",
    "!RESERVED!",
    "\n"
  };

  if (!arg || !*arg) {
    if (CAN_WEAR(obj, ITEM_WEAR_FINGER))      where = WEAR_FINGER_R;
    if (CAN_WEAR(obj, ITEM_WEAR_NECK))        where = WEAR_NECK_1;
    if (CAN_WEAR(obj, ITEM_WEAR_BODY))        where = WEAR_BODY;
    if (CAN_WEAR(obj, ITEM_WEAR_HEAD))        where = WEAR_HEAD;
    if (CAN_WEAR(obj, ITEM_WEAR_LEGS))        where = WEAR_LEGS;
    if (CAN_WEAR(obj, ITEM_WEAR_FEET))        where = WEAR_FOOT_R;
    if (CAN_WEAR(obj, ITEM_WEAR_HANDS))       where = WEAR_HAND_R;
    if (CAN_WEAR(obj, ITEM_WEAR_ARMS))        where = WEAR_ARM_R;
    if (CAN_WEAR(obj, ITEM_WEAR_BACK))        where = WEAR_BACK;
    if (CAN_WEAR(obj, ITEM_WEAR_WAIST))       where = WEAR_WAIST;
    if (CAN_WEAR(obj, ITEM_WEAR_WRIST))       where = WEAR_WRIST_R;
    if (CAN_WEAR(obj, ITEM_WEAR_HOLD))
      if (GET_OBJ_WEIGHT(obj) < str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
	where = WEAR_WIELD;
  } else {
    if ((where = search_block(arg, keywords, FALSE)) < 0) {
      sprintf(buf, "'%s'?  What part of your body is THAT?\r\n", arg);
      send_to_char(buf, ch);
    }
  }

  return where;
}



ACMD(do_wear)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj;
  int where = -1, dotmode, items_worn = 0;

  two_arguments(argument, arg1, arg2);

  if (!*arg1) {
    send_to_char("Wear what?\r\n", ch);
    return;
  }
  dotmode = find_all_dots(arg1);

  if (*arg2 && (dotmode != FIND_INDIV)) {
    send_to_char("You can't specify the same body location for more than one item!\r\n", ch);
    return;
  }
  if (dotmode == FIND_ALL) {
    for (obj = ch->carrying; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) && (where = find_eq_pos(ch, obj, 0)) >= 0) {
	items_worn++;
	perform_wear(ch, obj, where);
      }
    }
    if (!items_worn)
      send_to_char("You don't seem to have anything wearable.\r\n", ch);
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg1) {
      send_to_char("Wear all of what?\r\n", ch);
      return;
    }
    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
      send_to_char(buf, ch);
    } else
      while (obj) {
	next_obj = get_obj_in_list_vis(ch, arg1, obj->next_content);
	if ((where = find_eq_pos(ch, obj, 0)) >= 0)
	  perform_wear(ch, obj, where);
	else
	  act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
	obj = next_obj;
      }
  } else {
    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
      send_to_char(buf, ch);
    } else {
      if ((where = find_eq_pos(ch, obj, arg2)) >= 0)
	perform_wear(ch, obj, where);
      else if (!*arg2)
	act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_INFOBAR))
    do_infobar(ch, 0, 0, SCMDB_WEAR);
}


ACMD(do_wield)
{
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Wield what?\r\n", ch);
  else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else {
    if (!CAN_WEAR(obj, ITEM_WEAR_HOLD))
      send_to_char("You can't wield that.\r\n", ch);
    else if (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
      send_to_char("It's too heavy for you to use.\r\n", ch);
    else
      perform_wear(ch, obj, WEAR_WIELD);
  }
}



ACMD(do_grab)
{
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*argument || !*arg)
    send_to_char("Hold what?\r\n", ch);
  else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else {
    if (GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      perform_wear(ch, obj, WEAR_HOLD);
    else {
      if (!can_take_obj(ch, obj))
	send_to_char("You can't hold that.\r\n", ch);
      else
	perform_wear(ch, obj, WEAR_HOLD);
    }
  }
}


void perform_remove(struct char_data * ch, int pos)
{
  struct obj_data *obj;

  if (!(obj = GET_EQ(ch, pos))) {
    log("Error in perform_remove: bad pos passed.");
    return;
  }
  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    act("$p: you can't carry that many items!", FALSE, ch, obj, 0, TO_CHAR);
  else {
    obj_to_char(unequip_char(ch, pos), ch);
    act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM);
  }
}



ACMD(do_remove)
{
  struct obj_data *obj;
  int i, dotmode, found;
  void unlodge_obj_in_char(struct char_data *ch, struct obj_data *obj, int i);
  extern char *locations[];

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Remove what?\r\n", ch);
    return;
  }
  dotmode = find_all_dots(arg);

  if (dotmode == FIND_ALL) {
    found = 0;
    for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i)) {
	perform_remove(ch, i);
	found = 1;
      }
    if (!found)
      send_to_char("You're not using anything.\r\n", ch);
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg)
      send_to_char("Remove all of what?\r\n", ch);
    else {
      found = 0;
      for (i = 0; i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) &&
	    isname(arg, GET_EQ(ch, i)->name)) {
	  perform_remove(ch, i);
	  found = 1;
	}
      if (!found) {
	sprintf(buf, "You don't seem to be using any %ss.\r\n", arg);
	send_to_char(buf, ch);
      }
    }
  } else {
    if (!(obj = get_object_in_equip_vis(ch, arg, ch->equipment, &i))) {
      if ((i = find_lodged_obj(ch, arg)) != -1) {
        obj = GET_ITEMS_STUCK(ch, i);
	sprintf(buf, "You remove $p from your %s.", locations[i]);
	act(buf, FALSE, ch, obj, 0, TO_CHAR);
	sprintf(buf, "$n removes $p from $s %s.", locations[i]);
	act(buf, FALSE, ch, obj, 0, TO_ROOM);
	unlodge_obj_in_char(ch, obj, i);
        if (number(1, 5) > 4)
          break_obj(obj);
      } else {
	sprintf(buf, "You don't seem to be using %s %s.\r\n", AN(arg), arg);
	send_to_char(buf, ch);
      }
    } else
      perform_remove(ch, i);
  }
}

ACMD(do_poison_weapon)
{
  struct obj_data *obj;
  int poison_type = number(1, 4), i = 0;

  if (GET_POS(ch) == POS_FIGHTING) {
    send_to_char("You have no time to poison weapons now!\r\n", ch);
    return;
  }
  if (GET_SKILL(ch, SKILL_POISON_WEAPON) <= 1) {
    send_to_char("You had better leave that to the professionals.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Poison which weapon?\r\n", ch);
    return;
  }
  if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    for (i = 0; !obj && i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i) && !str_cmp(arg, GET_EQ(ch, i)->name)) 
        obj = GET_EQ(ch, i);
    }
  }
  if (!obj) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
    return;
  }
  /* Okay now we have the object to poison */
  if (GET_OBJ_TYPE(obj) != ITEM_WEAPON) {
    send_to_char("You can only poison weapons.\r\n", ch);
    return;
  }
  if ((GET_OBJ_VAL(obj, 3) != TYPE_SLASH - TYPE_HIT) &&
      (GET_OBJ_VAL(obj, 3) != TYPE_PIERCE - TYPE_HIT)) {
    send_to_char("You can only poison bladed weapons.\r\n", ch);
    return;
  }
  /* Now check for success, poison, etc. */

  if (number(1, 101) < GET_SKILL(ch, SKILL_POISON_WEAPON)) {
    if (!GET_OBJ_VAL(obj, 4)) 
      GET_OBJ_VAL(obj, 4) = poison_type;
      sprintf(buf, "You coat the blade of %s with poison.\r\n",
	    obj->short_description);
      send_to_char(buf, ch);

  } else {
    send_to_char("Oops, you seem to have poisoned yourself!\r\n", ch);
    mag_affects(dice(10, 5), ch, ch, SPELL_POISON, 0);
  }
}

/* to load ranged weapons */
ACMD(do_load)
{
/*
 * Fire Weapons:
 *   v[0] = Ammo Type
 *   v[1] = Current # Ammo
 *   v[2] = Max Allow # Ammo
 *   v[3] = missile vnum (if loaded)
 * Missile/Ammo:
 *   v[0] = Ammo Type
 *   v[1] = Ammo Amount
 *   v[2] = Num Dam Dice
 *   v[3] = Size Dam Dice
 */
  int    i;
  char   arg1[MAX_INPUT_LENGTH];
  char   arg2[MAX_INPUT_LENGTH];
  struct obj_data *source, *target;
  extern struct index_data *obj_index;

  two_arguments(argument, arg1, arg2);

  if (!*arg1) {
    send_to_char("What do you want to load?\r\n", ch);
    return;
  }
  if (!(target = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
    for (i = 0; !target && i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i) && (isname(arg1, GET_EQ(ch, i)->name) ||
	  is_abbrev(arg1, GET_EQ(ch, i)->name)))
	target = GET_EQ(ch, i);
    }
  }
  if (!target) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", XANA(arg1), arg1);
    send_to_char(buf, ch);
    return;
  }
  if (!*arg2) {
    act("What do you want to load $p with?", FALSE, ch, target, 0, TO_CHAR);
    return;
  }
  if (!(source = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
    for (i = 0; !source && i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i) && (isname(arg2, GET_EQ(ch, i)->name) ||
	  is_abbrev(arg2, GET_EQ(ch, i)->name)))
	source = GET_EQ(ch, i);
    }
  }
  if (!source) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", XANA(arg2), arg2);
    send_to_char(buf, ch);
    return;
  }
  if (GET_OBJ_TYPE(target) != ITEM_FIREWEAPON) {
    act("$p is not a ranged weapon!", FALSE, ch, target, 0, TO_CHAR);
    return;
  }
  if ((GET_OBJ_TYPE(source) != ITEM_MISSILE) && 
      (GET_OBJ_TYPE(source) != ITEM_CONTAINER)) {
      act("$p is not a projectile!", FALSE, ch, source, 0, TO_CHAR);
    return;
  }
  if (GET_OBJ_TYPE(source) == ITEM_CONTAINER) {
    if (GET_OBJ_VAL(source, 4)) {
      act("$p is not a quiver!", FALSE, ch, source, 0, TO_CHAR);
      return;
    } else
      source = read_object(GET_OBJ_VAL(source, 4), VNUMBER);
  }
  if (GET_OBJ_VAL(target, 1) > 0) {
    act("But $p is already loaded!!", FALSE, ch, target, 0, TO_CHAR);
    return;
  }
  /* Need to be the same type */
  if (GET_OBJ_VAL(target, 0) != GET_OBJ_VAL(source, 0)) {
    sprintf(buf, "You cannot load $p with %s %s!\r\n", XANA(OBJN(source, ch)), OBJN(source, ch));
    act(buf, FALSE, ch, target, NULL, TO_CHAR);
    return;
  }
  if (GET_OBJ_VAL(target, 1) >= GET_OBJ_VAL(target,  2)) {
    sprintf(buf, "You can only load $p with a maximum of %ld %ss", GET_OBJ_VAL(target, 2), OBJN(source, ch));
    act(buf, FALSE, ch, target, 0, TO_CHAR);
    return;
  }
  /* Okay at this point we should have everything ready */
  GET_OBJ_VAL(target, 1) += MIN(GET_OBJ_VAL(target,2), GET_OBJ_VAL(source, 1));  /* Amount of ammo */
  GET_OBJ_VAL(source, 1) -= MIN(GET_OBJ_VAL(target,2), GET_OBJ_VAL(source,1));
  GET_OBJ_VAL(target, 3) = GET_OBJ_VNUM(source);       /* missile vnum */
  sprintf(buf, "You load %s with %s.\r\n", target->short_description, source->short_description);
  send_to_char(buf, ch);
  sprintf(buf, "$n loads %s with %s.\r\n", target->short_description, source->short_description);
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  if (!GET_OBJ_VAL(source, 1))
    extract_obj(source);
  if (!IS_IMMORT(ch))
    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);  
}

int check_trigger_trap(struct char_data *vict, void *vo, int trigger_type)
{
   struct obj_data *trap;
   if (vict == NULL) {
     log("SYSERR: NULL vict passed to check_trigger_trap()");
     abort();
   }
   if ((trigger_type == TRIG_GET_OBJ || trigger_type == TRIG_DROP_OBJ) &&
       vo != NULL) {
     trap = (struct obj_data *) vo;
     if ((GET_OBJ_TYPE(trap) == ITEM_TRAP) && (trap_trig(trap) == trigger_type))
       return (trap_damage(vict, trap));
     else
       return 0;
   } else if ((trap = find_trap(vict, trigger_type)) != NULL)
     return (trap_damage(vict, trap));
   else
     return 0;
}

struct obj_data *find_trap(struct char_data *ch, int trigger)
{
   struct obj_data *i;

   for (i = world[ch->in_room].contents; i; i = i->next_content) {
     if (GET_OBJ_TYPE(i) == ITEM_TRAP)
       if (trap_trig(i) == trigger)
	 return i;
   }
   return NULL;
}

int trap_trig(struct obj_data *trap)
{
   if ((GET_OBJ_TYPE(trap) == ITEM_TRAP) && GET_OBJ_VAL(trap, 0) < NUM_TRIGGERS)
     return GET_OBJ_VAL(trap, 0);
   else
     return TRIG_UNDEFINED;
}

int trap_damage(struct char_data *vict, struct obj_data *trap)
{
   int    dam = 0;
   char   desc[40];
   struct affected_type af;
   struct char_data *tch;

   if (vict == NULL || trap == NULL) {
     log("SYSERR: NULL vict passed to check_trigger_trap()");
     abort();
   }
   dam = dice(GET_OBJ_VAL(trap, 2), GET_OBJ_VAL(trap, 3));
   if (IS_AFFECTED(vict, AFF_SANCTUARY))
     dam >>= 1;
   else if (IS_AFFECTED2(vict, AFF_BLACKMANTLE))
     dam <<= 1;

   if (number(1, 35) < GET_DEX(vict))
     dam = 0;

   switch (GET_OBJ_VAL(trap, 1)) {
     case TRAP_DAM_PIERCE:
       strcpy(desc, "pierced");
       break;
     case TRAP_DAM_SLASH:
       strcpy(desc, "sliced");
       break;
     case TRAP_DAM_BLUNT:
       strcpy(desc, "pounded");
       break;
     case TRAP_DAM_FIRE:
       strcpy(desc, "seared");
       break;
     case TRAP_DAM_COLD:
       strcpy(desc, "frozen");
       break;
     case TRAP_DAM_ACID:
       strcpy(desc, "corroded");
       break;
     case TRAP_DAM_ELEC:
       strcpy(desc, "electrocuted");
       break;
     case TRAP_DAM_SLEEP:
       strcpy(desc, "knocked out");
       break;
     case TRAP_DAM_TELEPORT:
       strcpy(desc, "teleported away");
       break;
     case TRAP_DAM_POISON:
       strcpy(desc, "poisoned");
       break;
     case TRAP_DAM_DISEASE:
       strcpy(desc, "diseased");
       break;
     default:
       strcpy(desc, "damaged");
       break;
   }
   if (GET_SKILL(vict, SKILL_DETECT_TRAP) > number(5, 101)) {
     act("You notice that $p is trapped!", FALSE, vict, trap, NULL, TO_CHAR);
     return 0;
   } else if ((dam > 0) &&
       (GET_OBJ_VAL(trap, 1) != TRAP_DAM_SLEEP) &&
       (GET_OBJ_VAL(trap, 1) != TRAP_DAM_POISON) &&
       (GET_OBJ_VAL(trap, 1) != TRAP_DAM_DISEASE) &&
       (GET_OBJ_VAL(trap, 1) != TRAP_DAM_TELEPORT)) {
     if (trap_trig(trap) == TRIG_GET_OBJ || trap_trig(trap) == TRIG_DROP_OBJ) {
       sprintf(buf, "$n is %s by $p!", desc);
       act(buf, TRUE, vict, trap, NULL, TO_ROOM);
       sprintf(buf, "You are %s by $p!", desc);
       act(buf, TRUE, vict, trap, NULL, TO_CHAR);
       GET_HIT(vict) -= dam;
     } else {
       sprintf(buf, "Everyone is %s by $p!", desc);
       act(buf, TRUE, NULL, trap, NULL, TO_ROOM);
       for (tch = world[vict->in_room].people; tch; tch = tch->next_in_room) {
	 sprintf(buf, "You are %s by $p!", desc);
	 act(buf, TRUE, tch, trap, NULL, TO_CHAR);
	 GET_HIT(tch) -= dam;
       }
     }
     return 1;
   } else if (dam <= 0) {
     if (trap_trig(trap) == TRIG_GET_OBJ || trap_trig(trap) == TRIG_DROP_OBJ) {
       sprintf(buf, "$n is almost %s by $p!", desc);
       act(buf, TRUE, vict, trap, NULL, TO_ROOM);
       sprintf(buf, "You are almost %s by $p!", desc);
       act(buf, TRUE, vict, trap, NULL, TO_CHAR);
     } else {
       sprintf(buf, "Everyone is almost %s by $p!", desc);
       act(buf, TRUE, NULL, trap, NULL, TO_ROOM);
       for (tch = world[vict->in_room].people; tch; tch = tch->next_in_room) {
	 sprintf(buf, "You are almost %s by $p!", desc);
	 act(buf, TRUE, tch, trap, NULL, TO_CHAR);
       }
     }
     return 1;
   } else if (GET_OBJ_VAL(trap, 1) == TRAP_DAM_SLEEP) {
     af.type      = SPELL_SLEEP;
     af.duration  = dam;
     af.modifier  = 0;
     af.location  = APPLY_NONE;
     af.bitvector = AFF_SLEEP;
     affect_join(vict, &af, FALSE, FALSE, FALSE, FALSE, TRUE);
     if (GET_POS(vict) > POS_SLEEPING) {
       act("You feel very sleepy ..... zzzzzz",FALSE,vict,0,0,TO_CHAR);
       act("$p knocks $n out.",TRUE,vict,trap,0,TO_ROOM);
       GET_POS(vict) = POS_SLEEPING;
     }
     return 1;
   } else if (GET_OBJ_VAL(trap, 1) == TRAP_DAM_POISON) {
     af.type      = SPELL_POISON;
     af.duration  = dam >> 1;
     af.modifier  = 0;
     af.location  = APPLY_NONE;
     af.bitvector = (dam > 25 ? AFF_POISON_III : dam > 15 ? AFF_POISON_II : AFF_POISON_I);
     affect_join(vict, &af, FALSE, FALSE, FALSE, FALSE, TRUE);
     return 1;
   } else if (GET_OBJ_VAL(trap, 1) == TRAP_DAM_DISEASE) {
     af.type      = SPELL_DISEASE;
     af.duration  = dam;
     af.modifier  = 0;
     af.location  = APPLY_NONE;
     af.bitvector = AFF_DISEASE;
     affect_join(vict, &af, FALSE, FALSE, FALSE, FALSE, TRUE);
     return 1;
   } else if (GET_OBJ_VAL(trap, 1) == TRAP_DAM_TELEPORT) {
     if (real_room(GET_OBJ_VAL(trap, 2)) > 0) {
       act("$n slowly fades out of existence.", FALSE, vict, trap, NULL, TO_ROOM);
       char_from_room(vict);
       char_to_room(vict, real_room(GET_OBJ_VAL(trap, 4)));
       act("$n slowly fades into existence.", FALSE, vict, trap, NULL, TO_ROOM);
       return 1;
     }
     return 0;
   } else if (GET_OBJ_VAL(trap, 1) == TRAP_DAM_SPELL) {
     call_magic(vict, vict, 0, GET_OBJ_VAL(trap, 2), GET_OBJ_VAL(trap, 3), CAST_SPELL);
     return 1;
   } else
     return 0;
}

void break_obj(struct obj_data *obj)
{
   char temp[80];

   strcpy(temp, obj->name);
   obj->item_number = NOTHING;
   sprintf(buf2, "broken %s", temp);
   obj->name        = str_dup(buf2);
   sprintf(buf2, "A broken %s has been left here.", temp);
   obj->description = str_dup(buf2);
   sprintf(buf2, "a broken %s", temp);
   obj->short_description = str_dup(buf2);
 
   GET_OBJ_TYPE(obj)   = ITEM_TRASH;
   GET_OBJ_RENT(obj)   = 0;
}
