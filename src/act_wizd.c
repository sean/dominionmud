/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ACT_WIZ.C                                    Based on CircleMUD *
*  Usage: Player level god commands and other similar utilities           *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#define __ACT_WIZD_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/*   external vars  */
extern FILE *player_fl;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[];
extern struct wis_app_type wis_app[];
extern struct zone_data *zone_table;
extern int  top_of_zone_table;
extern int  game_restrict;
extern long top_of_world;
extern long top_of_mobt;
extern long top_of_objt;
extern int  top_of_p_table;
extern socket_t mother_desc;
extern sh_int port;
extern struct player_index_element *player_table;
extern struct guild_type *guild_info;
extern int num_of_guilds;

/* for objects */
extern char *item_types[];
extern char *item_size_bits[];
extern char *wear_bits[];
extern char *extra_bits[];
extern char *container_bits[];
extern char *drinks[];
extern char *trap_triggers[];
extern char *trap_dam_type[];

/* for rooms */
extern char *dirs[];
extern char *room_bits[];
extern char *exit_bits[];
extern char *sector_types[];

/* for chars */
extern char *spells[];
extern char *equipment_types[];
extern char *affected_bits[];
extern char *affected2_bits[];
extern char *apply_types[];
extern char *action_bits[];
extern char *player_bits[];
extern char *preference_bits[];
extern char *position_types[];
extern char *connected_types[];
/* New for TD 3/19/95 - SPM */
extern int NUM_RACES;
extern struct race_data * races;
extern int NUM_RELIGIONS;
extern struct material_type const material_list[];
int exp_needed(int level);
int parse_race(char arg);
void Crash_rentsave(struct char_data * ch, int cost);
void newbie_basket(struct char_data * ch);
void perform_remove(struct char_data * ch, int pos);
char* show_obj_condition(struct obj_data *obj);
void Dismount(struct char_data *ch, struct char_data *h, int pos);
void roll_real_abils(struct char_data * ch);
char *three_arguments(char *argument, char *first_arg, char *second_arg, char *third_arg);
void weather_and_time(int mode);
void affect_update(void);       /* In spells.c */
void show_guild_trainers(struct char_data * ch);
ACMD(do_tell);

ACMD(do_echo)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes.. but what?\r\n", ch);
  else {
    if (subcmd == SCMD_EMOTE)
      sprintf(buf, "$n %s", argument);
    else
      strcpy(buf, argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }
}


ACMD(do_send)
{
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Send what to who?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  send_to_char(buf, vict);
  send_to_char("\r\n", vict);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Sent.\r\n", ch);
  else {
    sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
    send_to_char(buf2, ch);
  }
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
sh_int find_target_room(struct char_data * ch, char *rawroomstr)
{
  int tmp;
  sh_int location;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  char roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char("You must supply a room number or name.\r\n", ch);
    return NOWHERE;
  }
  if (isdigit((int)*roomstr) && !strchr(roomstr, '.')) {
    tmp = atoi(roomstr);
    if ((location = real_room(tmp)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return NOWHERE;
    }
  } else if ((target_mob = get_char_vis(ch, roomstr)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, roomstr))) {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("That object is not available.\r\n", ch);
      return NOWHERE;
    }
  } else {
    send_to_char("No such creature or object around.\r\n", ch);
    return NOWHERE;
  }

  /* a location has been found -- if you're < GRGOD, check restrictions. */
  if (GET_MAX_LEVEL(ch) < LVL_GRGOD) {
    if (ROOM_FLAGGED(location, ROOM_GODROOM)) {
      send_to_char("You are not godly enough to use that room!\r\n", ch);
      return NOWHERE;
    }
    if (ROOM_FLAGGED(location, ROOM_PRIVATE) &&
	world[location].people && world[location].people->next_in_room) {
      send_to_char("There's a private conversation going on in that room.\r\n", ch);
      return NOWHERE;
    }
    if (ROOM_FLAGGED(location, ROOM_HOUSE) &&
	!House_can_enter(ch, world[location].number)) {
      send_to_char("That's private property -- no trespassing!\r\n", ch);
      return NOWHERE;
    }
  }
  return location;
}



ACMD(do_at)
{
  char command[MAX_INPUT_LENGTH];
  int location, original_loc;

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char("You must supply a room number or a name.\r\n", ch);
    return;
  }

  if (!*command) {
    send_to_char("What do you want to do there?\r\n", ch);
    return;
  }

  if ((location = find_target_room(ch, buf)) < 0)
    return;

  /* a location has been found. */
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the char is still there */
  if (ch->in_room == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}


ACMD(do_goto)
{
  ln_int location;
  struct char_data *mount = NULL;

  if ((location = find_target_room(ch, argument)) < 0)
    return;

  if (MOUNTED(ch))
     mount = MOUNTED(ch);

  if (!mount) {
     if (POOFOUT(ch))
	sprintf(buf, "%s", POOFOUT(ch));
     else
	strcpy(buf, "$n disappears in a puff of smoke.");

     act(buf, TRUE, ch, 0, 0, TO_ROOM);
     char_from_room(ch);
     char_to_room(ch, location);

     if (POOFIN(ch))
	sprintf(buf, "%s", POOFIN(ch));
     else
	strcpy(buf, "Your hear the loud crash of thunder!");

     act(buf, TRUE, ch, 0, 0, TO_ROOM);
  } else {  /* mounted */
     if (POOFOUT(ch))
	sprintf(buf, "%s", POOFOUT(ch));
     else
	sprintf(buf, "$n rips open a space-time rift and rides through on $N");

     act(buf, TRUE, ch, 0, mount, TO_ROOM);
     char_from_room(ch);
     char_from_room(mount);
     char_to_room(ch, location);
     char_to_room(mount, location);

     if (POOFIN(ch))
	sprintf(buf, "%s", POOFIN(ch));
     else
	sprintf(buf, "$n rips open a space-time rift and rides through on $N");

     act(buf, TRUE, ch, 0, mount, TO_ROOM);
  }
  look_at_room(ch, 0);
}



ACMD(do_trans)
{
  struct descriptor_data *i;
  struct char_data *victim;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to transfer?\r\n", ch);
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf)))
      send_to_char(NOPERSON, ch);
    else if (victim == ch)
      send_to_char("That doesn't make much sense, does it?\r\n", ch);
    else {
      if ((GET_MAX_LEVEL(ch) < GET_MAX_LEVEL(victim)) && !IS_NPC(victim)) {
	send_to_char("Go transfer someone your own size.\r\n", ch);
	return;
      }
      act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, ch->in_room);
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      look_at_room(victim, 0);
    }
  } else {                      /* Trans All */
    if (GET_MAX_LEVEL(ch) < LVL_GRGOD) {
      send_to_char("I think not.\r\n", ch);
      return;
    }

    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character && i->character != ch) {
	victim = i->character;
	if (GET_MAX_LEVEL(victim) >= GET_MAX_LEVEL(ch))
	  continue;
	act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	/* BAD FIX for a nasty bug :( */
	if (ch->in_room != NOWHERE)
	   char_from_room(victim);
	char_to_room(victim, ch->in_room);
	if (MOUNTED(victim))  Dismount(victim, MOUNTED(victim), POS_STANDING);
	act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	look_at_room(victim, 0);
      }
    send_to_char(OK, ch);
  }
}



ACMD(do_teleport)
{
  struct char_data *victim;
  sh_int target;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char("Whom do you wish to teleport?\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (victim == ch)
    send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char("Maybe you shouldn't do that.\r\n", ch);
  else if (!*buf2)
    send_to_char("Where do you wish to send this person?\r\n", ch);
  else if ((target = find_target_room(ch, buf2)) >= 0) {
    send_to_char(OK, ch);
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    if (ch->in_room != NOWHERE)
       char_from_room(victim);
    char_to_room(victim, target);
    if (MOUNTED(victim)) Dismount(victim, MOUNTED(victim), POS_STANDING);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    look_at_room(victim, 0);
  }
}

ACMD(do_tick)
{
    send_to_char("DO NOT ABUSE THIS COMMAND, YOU WILL BE DEMOTED!\r\n", ch);
    send_to_char("This makes 1 tick pass, or 1 hour of game time.\r\n", ch);

    sprintf(buf, "(GC) %s made a tick pass faster than normal.", GET_NAME(ch));
    mudlog(buf, BRF, LVL_GRGOD, TRUE);

    weather_and_time(1);
    affect_update();
    point_update();
    fflush(player_fl);

    send_to_char(OK, ch);
}


ACMD(do_vnum)
{
  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj"))) {
    send_to_char("Usage: vnum { obj | mob } <name>\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob"))
    if (!vnum_mobile(buf2, ch))
      send_to_char("No mobiles by that name.\r\n", ch);

  if (is_abbrev(buf, "obj"))
    if (!vnum_object(buf2, ch))
      send_to_char("No objects by that name.\r\n", ch);
}



void do_stat_room(struct char_data * ch)
{
  struct extra_descr_data *desc;
  struct room_data *rm = &world[ch->in_room];
  int i, found = 0;
  struct obj_data *j = 0;
  struct char_data *k = 0;

  sprintf(buf, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name,
	  CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprinttype(rm->sector_type, sector_types, buf2);
  sprintf(buf, "Zone: [%3d], VNum: [%s%8ld%s], RNum: [%8ld], Type: %s\r\n",
	  rm->zone, CCGRN(ch, C_NRM), rm->number, CCNRM(ch, C_NRM), ch->in_room, buf2);
  send_to_char(buf, ch);

  sprintbit((long) rm->room_flags, room_bits, buf2);
  sprintf(buf, "SpecProc: %s, Flags: %s\r\n",
	  (rm->func == NULL) ? "None" : "Exists", buf2);
  send_to_char(buf, ch);

  send_to_char("Description:\r\n", ch);
  if (rm->description)
    send_to_char(rm->description, ch);
  else
    send_to_char("  None.\r\n", ch);

  if (rm->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  sprintf(buf, "Chars present:%s", CCYEL(ch, C_NRM));
  for (found = 0, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;
    sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
	    (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (k->next_in_room)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);
  send_to_char(CCNRM(ch, C_NRM), ch);

  if (rm->contents) {
    sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
	continue;
      sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (rm->dir_option[i]) {
      if (rm->dir_option[i]->to_room == NOWHERE)
	sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
	sprintf(buf1, "%s%ld%s", CCCYN(ch, C_NRM),
		world[rm->dir_option[i]->to_room].number, CCNRM(ch, C_NRM));
      sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
      sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%ld], Keywrd: %s, Type: %s\r\n ",
	      CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
	   rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
	      buf2);
      send_to_char(buf, ch);
      if (rm->dir_option[i]->general_description)
	strcpy(buf, rm->dir_option[i]->general_description);
      else
	strcpy(buf, "  No exit description.\r\n");
      send_to_char(buf, ch);
    }
  }
}



void do_stat_object(struct char_data * ch, struct obj_data * j)
{
  int    i, found;
  long   vnumber;
  struct obj_data *j2;
  struct extra_descr_data *desc;
  extern const char *ammo_types[];

  vnumber = GET_OBJ_VNUM(j);
  sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
	  ((j->short_description) ? j->short_description : "<None>"),
	  CCNRM(ch, C_NRM), j->name);
  send_to_char(buf, ch);
  sprinttype(GET_OBJ_TYPE(j), item_types, buf1);
  if (GET_OBJ_RNUM(j) >= 0)
    strcpy(buf2, (obj_index[GET_OBJ_RNUM(j)].func ? "Exists" : "None"));
  else
    strcpy(buf2, "None");
  sprintf(buf, "VNum: [%s%8ld%s], RNum: [%8ld], Type: %s, SpecProc: %s\r\n",
   CCGRN(ch, C_NRM), vnumber, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), buf1, buf2);
  send_to_char(buf, ch);
  sprintf(buf, "L-Des: %s\r\n", ((j->description) ? j->description : "None"));
  send_to_char(buf, ch);

  if (j->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  send_to_char("Can be worn on: ", ch);
  sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Set char bits : ", ch);
  sprintbit(j->obj_flags.bitvector, affected_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Extra flags   : ", ch);
  sprintbit(GET_OBJ_EXTRA(j), extra_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\r\n",
     GET_OBJ_WEIGHT(j), GET_OBJ_COST(j), GET_OBJ_RENT(j), GET_OBJ_TIMER(j));
  send_to_char(buf, ch);

  strcpy(buf, "In room: ");
  if (j->in_room == NOWHERE)
    strcat(buf, "Nowhere");
  else {
    sprintf(buf2, "%ld", world[j->in_room].number);
    strcat(buf, buf2);
  }
  strcat(buf, ", In object: ");
  strcat(buf, j->in_obj ? j->in_obj->short_description : "None");
  strcat(buf, ", Carried by: ");
  strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
  strcat(buf, ", Worn by: ");
  strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Nobody");
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  switch (GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
       strcpy(buf, "Hours left: Infinite");
    else
       sprintf(buf, "Hours left: [%ld]", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    sprintf(buf, "Spells: (Level %ld) %s, %s, %s", GET_OBJ_VAL(j, 0),
	   skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
	   skill_name(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    sprintf(buf, "Spell: %s at level %ld, %ld (of %ld) charges remaining",
	    skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_FIREWEAPON:
    sprintf(buf, "Ammo Type: %s, Ammo: %ld, Missile: %ld",
	    ammo_types[GET_OBJ_VAL(j, 0)], GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2));
    break;
  case ITEM_WEAPON:
    sprintf(buf, "Todam: %ldd%ld, Message type: %ld Weapon type: %ld",
	    GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3), GET_OBJ_VAL(j, 4));
    break;
  case ITEM_MISSILE:
    sprintf(buf, "Ammo Type: %s, Ammo Amt: %ld, Todam: %ldd%ld",
	    ammo_types[GET_OBJ_VAL(j, 0)], GET_OBJ_VAL(j, 1),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
  case ITEM_ARMOR:
    sprintf(buf, "AC-apply: [%ld]", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_TRAP:
    sprintf(buf, "Trigger: %s, Dam Type: %s, Dam: %ldd%ld, Var: %s",
	    trap_triggers[GET_OBJ_VAL(j, 0)],
	    trap_dam_type[GET_OBJ_VAL(j, 1)],
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3),
	    GET_OBJ_VAL(j, 1) == TRAP_DAM_SPELL ? (spells[GET_OBJ_VAL(j, 4)]) :
	    "Unknown");
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf2);
    sprintf(buf, "Weight capacity: %ld, Lock Type: %s, Key Num: %ld, Corpse: %s, Quiver: %s",
	    GET_OBJ_VAL(j, 0), buf2, GET_OBJ_VAL(j, 2),
	    YESNO(GET_OBJ_VAL(j, 3)), YESNO(GET_OBJ_VAL(j, 4)));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf2);
    sprintf(buf, "Capacity: %ld, Contains: %ld, Poisoned: %s, Liquid: %s",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), YESNO(GET_OBJ_VAL(j, 3)),
	    buf2);
    break;
  case ITEM_NOTE:
    sprintf(buf, "Language: %ld", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_KEY:
    sprintf(buf, "Keytype: %ld", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_FOOD:
    sprintf(buf, "Makes full: %ld, Poisoned: %s, Condition: %s",
	    GET_OBJ_VAL(j, 0), YESNO(GET_OBJ_VAL(j, 3)),
	    show_obj_condition(j));
    break;
  case ITEM_MONEY:
    sprintf(buf, "Coins: %ld", GET_OBJ_VAL(j, 0));
    break;
  default:
    sprinttype(GET_OBJ_SIZE(j), item_size_bits, buf2);
    sprintf(buf, "Values 0-7: [%ld] [%ld] [%ld] [%ld] [%ld] [%ld] [%ld] Mat: %s Size: %s Cond: %s",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2),
	    GET_OBJ_VAL(j, 3), GET_OBJ_VAL(j, 4), GET_OBJ_VAL(j, 5),
	    GET_OBJ_VAL(j, 6), material_list[GET_OBJ_MATERIAL(j)].name, buf2,
	    show_obj_condition(j));
    break;
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  /*
   * I deleted the "equipment status" code from here because it seemed
   * more or less useless and just takes up valuable screen space.
   */

  if (j->contains) {
    sprintf(buf, "\r\nContents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j2->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  found = 0;
  send_to_char("Affections:", ch);
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf2);
      sprintf(buf, "%s %+d to %s", found++ ? "," : "",
	      j->affected[i].modifier, buf2);
      send_to_char(buf, ch);
    }
  if (!found)
    send_to_char(" None", ch);

  send_to_char("\r\n", ch);
}


void do_stat_character(struct char_data * ch, struct char_data * k)
{
  int    i, i2, found = 0, armor = 0;
  struct obj_data *j;
  struct follow_type *fol;
  struct affected_type *aff;
  struct guild_type *gptr = guild_info;
  extern struct attack_hit_type attack_hit_text[];
  extern struct str_app_type str_app[];

  switch (GET_SEX(k)) {
    case SEX_NEUTRAL:    strcpy(buf, "NEUTRAL-SEX");   break;
    case SEX_MALE:       strcpy(buf, "MALE");          break;
    case SEX_FEMALE:     strcpy(buf, "FEMALE");        break;
    default:             strcpy(buf, "ILLEGAL-SEX!!"); break;
  }

  sprintf(buf2, " %s '%s'  IDNum: [%5ld], In room [%8ld]\r\n",
	  (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	  GET_NAME(k), GET_IDNUM(k), world[k->in_room].number);
  send_to_char(strcat(buf, buf2), ch);
  if (IS_MOB(k)) {
    sprintf(buf, "Alias: %s, VNum: [%8ld], RNum: [%8ld]\r\n",
	    k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k));
    send_to_char(buf, ch);
  }

  if (!IS_NPC(k)) {
    sprintf(buf, "Email: %s\r\n", (k->char_specials.email ? k->char_specials.email : "<None>"));
    send_to_char(buf, ch);
    sprintf(buf, "Title: %s\r\n", (k->player.title ? k->player.title : "<None>"));
  } else 
    sprintf(buf, "LD: %s", k->player.long_descr);
  send_to_char(buf, ch);

  sprintf(buf, "Keywords: %s\r\n", GET_KWDS(k) ? GET_KWDS(k) : "<None>");
  sprintf(buf, "%sL-Des: %s %s\r\n", buf, 
	  (GET_RDESC(k) ? GET_RDESC(k) : "<None>"),
	  (GET_RDESC(k) ? "is standing here." : ""));
  send_to_char(buf, ch);

  sprintf( buf, "Race: %s Religion: %s",
           races[ k->player.race ].name,
           relg_name( k ) );

  while (gptr && gptr->number != GET_GUILD(k))
    gptr = gptr->next;
  if (gptr != NULL)
    sprintf(buf, "%s  Guild: %s  Guild Lev: %d", buf, 
		gptr->name ? gptr->name : "Unknown!", GET_GUILD_LEV(k));

  sprintf(buf2, "\r\nLev: [%s%2d%s], XP: [%s%ld%s], Align: [%4d], PAlign: %s\r\n",
	  CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
	  CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM),
	  GET_ALIGNMENT(k), (IS_PERMEVIL(k) ? "Evil" : (IS_PERMGOOD(k) ? "Good" : "Neutral" )));

  strcat(buf, buf2);
  send_to_char(buf, ch);
  buf[0] = '\0';

  if (!IS_NPC(k)) {
    strcpy(buf1, (char *) asctime(localtime(&(k->player.time.birth))));
    strcpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
    buf1[10] = buf2[10] = '\0';
    sprintf(buf, "Created: [%s], Last Logon: [%s], Played [%ldh %ldm], Age [%d]\r\n",
	    buf1, buf2, k->player.time.played / 3600,
	    ((k->player.time.played / 3600) % 60), age(k).year);
    send_to_char(buf, ch);

    sprintf(buf, "Home [%d], Pracs [%d], PerPrac [%d], Prac/Lev [%d], %s Handed, OLC [%d]\r\n",
	  GET_HOMETOWN(k), GET_PRACTICES(k), int_app[GET_INT(k)].learn,
	  wis_app[GET_WIS(k)].bonus, GET_HANDEDNESS(ch) ? "Right" : "Left", GET_OLC_ZONE(k) ? GET_OLC_ZONE(k) : -1);
  }
  sprintf(buf, "%sSt: [%s%d/%d%s], In: [%s%d%s], Wi: [%s%d%s], De: [%s%d%s], Co: [%s%d%s], Ch: [%s%d%s], Wl: [%s%d%s]\r\n",
	  buf, CCCYN(ch, C_NRM), GET_STR(k), GET_ADD(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_WILL(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "H:[%s%d/%d+%d%s] R:[%s%d%s]  M:[%s%d/%d+%d%s] R:[%s%d%s]  V:[%s%d/%d+%d%s] R:[%s%d%s]\r\n",
	  CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_HIT_REGEN(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), mana_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MANA_REGEN(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MOVE_REGEN(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  for (i = 0; i < ARMOR_LIMIT; i++)
    armor += GET_AC(k, i);
  armor /= ARMOR_LIMIT;
  sprintf(buf, "Coins: [%9ld], Bank: [%9ld] (Total: %ld)     Avg. AC: [%3d%%]\r\n",
	  GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k), armor);
  send_to_char(buf, ch);

  sprintf(buf, "Hitroll: [%3d%%], Damroll: [%3d%%], Saving throws: [%d/%d/%d/%d/%d]\r\n",
	  k->points.hitroll, k->points.damroll, GET_SAVE(k, 0),
	  GET_SAVE(k, 1), GET_SAVE(k, 2), GET_SAVE(k, 3), GET_SAVE(k, 4));
  send_to_char(buf, ch);

  sprinttype(GET_POS(k), position_types, buf2);
  sprintf(buf, "Pos: %s, Fighting: %s", buf2,
	  (FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody"));

  if (IS_NPC(k)) {
    strcat(buf, ", Attack type: ");
    strcat(buf, attack_hit_text[k->mob_specials.attack_type].singular);
  }
  if (k->desc) {
    sprinttype(k->desc->connected, connected_types, buf2);
    strcat(buf, ", Connected: ");
    strcat(buf, buf2);
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  strcpy(buf, "Default position: ");
  sprinttype((k->mob_specials.default_pos), position_types, buf2);
  strcat(buf, buf2);

  sprintf(buf2, ", Idle Timer (in tics) [%d]\r\n", k->char_specials.timer);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (IS_NPC(k)) {
    sprintbit(MOB_FLAGS(k), action_bits, buf2);
    sprintf(buf, "NPC flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  } else {
    sprintbit(PLR_FLAGS(k), player_bits, buf2);
    sprintf(buf, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbit(PRF_FLAGS(k), preference_bits, buf2);
    sprintf(buf, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  }

  if (IS_MOB(k)) {
    sprintf(buf, "Mob Spec-Proc: %s, NPC Bare Hand Dam: %dd%d\r\n",
	    (mob_index[GET_MOB_RNUM(k)].func ? "Exists" : "None"),
	    k->mob_specials.damnodice, k->mob_specials.damsizedice);
    send_to_char(buf, ch);
  }
  sprintf(buf, "Carried: weight: %ld/%d, items: %d/%ld; ",
	  IS_CARRYING_W(k), CAN_CARRY_W(k), IS_CARRYING_N(k), CAN_CARRY_N(k));

  for (i = 0, j = k->carrying; j; j = j->next_content, i++);
  sprintf(buf, "%sItems in: inventory: %d, ", buf, i);

  for (i = 0, i2 = 0; i < NUM_WEARS; i++)
    if (GET_EQ(k, i))
      i2++;
  sprintf(buf2, "eq: %d\r\n", i2);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  sprintf(buf, "Hunger: %d, Thirst: %d, Tired: %d, Drunk: %d\r\n",
	  GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, TIRED),
	  GET_COND(k, DRUNK));
  send_to_char(buf, ch);

  sprintf(buf, "Master is: %s, Followers are:",
	  ((k->master) ? GET_NAME(k->master) : "<none>"));

  for (fol = k->followers; fol; fol = fol->next) {
    sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (fol->next)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);

  /* Showing the bitvector */
  sprintbit(AFF_FLAGS(k), affected_bits, buf2);
  sprintf(buf, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
  /* Showing the bitvector */
  sprintbit(AFF2_FLAGS(k), affected2_bits, buf2);
  sprintf(buf, "AFF2: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    for (aff = k->affected; aff; aff = aff->next) {
      *buf2 = '\0';
      sprintf(buf, "SPL: (%3dhr) %s%-21s%s ", aff->duration + 1,
	      CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
      if (aff->modifier) {
	sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
	strcat(buf, buf2);
      }
      if (aff->bitvector) {
	if (*buf2)
	  strcat(buf, ", sets ");
	else
	  strcat(buf, "sets ");
	sprintbit(aff->bitvector, affected_bits, buf2);
	strcat(buf, buf2);
      }
      send_to_char(strcat(buf, "\r\n"), ch);
    }
  }
  if (k->affected2) {
    for (aff = k->affected2; aff; aff = aff->next) {
      *buf2 = '\0';
      if (!aff) abort();
      sprintf(buf, "SPL: (%3dhr) %s%-21s%s ", aff->duration + 1,
	      CCBRD(ch, C_NRM), spells[aff->type] ? spells[aff->type] : "ERROR!", CCNRM(ch, C_NRM));
      if (aff->modifier) {
	sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
	strcat(buf, buf2);
      }
      if (aff->bitvector) {
	if (*buf2)
	  strcat(buf, ", sets ");
	else
	  strcat(buf, "sets ");
	sprintbit(aff->bitvector, affected2_bits, buf2);
	strcat(buf, buf2);
      }
      send_to_char(strcat(buf, "\r\n"), ch);
    }
  }
}


ACMD(do_stat)
{
  struct char_data *victim = 0;
  struct obj_data *object = 0;
  struct char_file_u tmp_store;
  int tmp;

  half_chop(argument, buf1, buf2);

  if (!*buf1) {
    send_to_char("Stats on who or what?\r\n", ch);
    return;
  } else if (is_abbrev(buf1, "room")) {
    do_stat_room(ch);
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2)
      send_to_char("Stats on which mobile?\r\n", ch);
    else {
      if ((victim = get_char_vis(ch, buf2)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such mobile around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "player")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      if ((victim = get_player_vis(ch, buf2, 0)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such player around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
      if (load_char(buf2, &tmp_store) > -1) {
	store_to_char(&tmp_store, victim);
	if (GET_LEVEL(victim) > GET_LEVEL(ch))
	  send_to_char("Sorry, you can't do that.\r\n", ch);
	else
	  do_stat_character(ch, victim);
	free_char(victim);
      } else {
	send_to_char("There is no such player.\r\n", ch);
	free(victim);
      }
    }
  } else if (is_abbrev(buf1, "object")) {
    if (!*buf2)
      send_to_char("Stats on which object?\r\n", ch);
    else {
      if ((object = get_obj_vis(ch, buf2)))
	do_stat_object(ch, object);
      else
	send_to_char("No such object around.\r\n", ch);
    }
  } else {
    if ((object = get_object_in_equip_vis(ch, buf1, ch->equipment, &tmp)))
      do_stat_object(ch, object);
    else if ((object = get_obj_in_list_vis(ch, buf1, ch->carrying)))
      do_stat_object(ch, object);
    else if ((victim = get_char_room_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents)))
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, buf1)))
      do_stat_object(ch, object);
    else
      send_to_char("Nothing around by that name.\r\n", ch);
  }
}


ACMD(do_shutdown)
{
  extern int circle_shutdown, circle_reboot;

  if (subcmd != SCMD_SHUTDOWN) {
    send_to_char("If you want to shut something down, say so!\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Shutting down.\r\n");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "reboot")) {
    sprintf(buf, "(GC) Reboot by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Rebooting.. come back in a minute or two.\r\n");
    touch("../.fastboot");
    circle_shutdown = circle_reboot = 1;
  } else if (!str_cmp(arg, "die")) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Shutting down for maintenance.\r\n");
    touch("../.killscript");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "pause")) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Shutting down for maintenance.\r\n");
    touch("../pause");
    circle_shutdown = 1;
  } else
    send_to_char("Unknown shutdown option.\r\n", ch);
}


void stop_snooping(struct char_data * ch)
{
  if (!ch->desc->snooping)
    send_to_char("You aren't snooping anyone.\r\n", ch);
  else {
    send_to_char("You stop snooping.\r\n", ch);
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }
}


ACMD(do_snoop)
{
  struct char_data *victim, *tch;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg)
    stop_snooping(ch);
  else if (!(victim = get_char_vis(ch, arg)))
    send_to_char("No such person around.\r\n", ch);
  else if (!victim->desc)
    send_to_char("There's no link.. nothing to snoop.\r\n", ch);
  else if (victim == ch)
    stop_snooping(ch);
  else if (victim->desc->snoop_by)
    send_to_char("Busy already. \r\n", ch);
  else if (victim->desc->snooping == ch->desc)
    send_to_char("Don't be stupid.\r\n", ch);
  else {
    if (victim->desc->original)
      tch = victim->desc->original;
    else
      tch = victim;

    if (GET_MAX_LEVEL(tch) >= GET_MAX_LEVEL(ch)) {
      send_to_char("You can't.\r\n", ch);
      return;
    }
    send_to_char(OK, ch);

    if (ch->desc->snooping)
      ch->desc->snooping->snoop_by = NULL;

    ch->desc->snooping = victim->desc;
    victim->desc->snoop_by = ch->desc;
  }
}



ACMD(do_switch)
{
  struct char_data *victim;

  one_argument(argument, arg);

  if (ch->desc->original)
    send_to_char("You're already switched.\r\n", ch);
  else if (!*arg)
    send_to_char("Switch with who?\r\n", ch);
  else if (!(victim = get_char_vis(ch, arg)))
    send_to_char("No such character.\r\n", ch);
  else if (ch == victim)
    send_to_char("Hee hee... we are jolly funny today, eh?\r\n", ch);
  else if (victim->desc)
    send_to_char("You can't do that, the body is already in use!\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_SUP) && !IS_NPC(victim))
    send_to_char("You aren't holy enough to use a mortal's body.\r\n", ch);
  else if (GET_MAX_LEVEL(ch) <= GET_LEVEL(victim))  {
    sprintf(buf, "%s What the hell do you think you are doing with my body?",
		GET_NAME(victim));
    do_tell(victim, buf, 0, 0);
  } else {
    send_to_char(OK, ch);

    ch->desc->character = victim;
    ch->desc->original  = ch;

    victim->desc = ch->desc;
    ch->desc = NULL;
  }
}

/**
 * Added for TD by SPM 6/3/95 to allow immorts to remort.
 */
ACMD(do_remort)
{
   int  i, new_race = UNDEFINED_RACE;
   char name[20], race[20];
   struct obj_data *tobj;
   struct char_data *victim;

   char *REMORTMESS =
     "A bright light fills your head, you become dizzy as you feel\r\n"
     "the presence of others enter your mind.  They pull and tug at\r\n"
     "your being, tearing you apart to make you anew.\r\n"
     "\r\n\r\n"
     "When you awaken you have been reborn into ...\r\n\r\n"
     "  _____ _            ____                  _       _              \r\n"
     " |_   _| |__   ___  |  _ \\  ___  _ __ ___ (_)_ __ (_) ___  _ __   \r\n"
     "   | | | '_ \\ / _ \\ | | | |/ _ \\| '_ ` _ \\| | '_ \\| |/ _ \\| '_ \\  \r\n"
     "   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | | \r\n"
     "   |_| |_| |_|\\___| |____/ \\___/|_| |_| |_|_|_| |_|_|\\___/|_| |_| \r\n"
     "\r\n\r\n" ;

   two_arguments(argument, name, race);

   if (!*name) {
     send_to_char("Whom do you wish to remort?\r\n", ch);
     return;
   } else
     (victim = get_char_vis(ch, name));

   if (GET_LEVEL(victim) > GET_LEVEL(ch)) {
     send_to_char("You cannot remort an immortal higher than yourself.\r\n", ch);
     return;
   }
   if (GET_LEVEL(victim) < MAX_MORT_LEVEL) {
     send_to_char("Sorry, but that person cannot remort yet.\r\n", ch);
     return;
   }
   
   for ( i = 0; i < NUM_RACES; i++ ) {
     if ( !str_cmp( race, races[ i ].name ) ) {
       new_race = i;
       break;
     }
   }
   if ( new_race == UNDEFINED_RACE ) {
     send_to_char( "Sorry, but I don't know what race that is.\r\n",
                   ch );
     return;
   }
   GET_RACE( victim ) = new_race;
   
   sprintf(buf, "%s will be remorted as %s %s.\r\n",
           GET_NAME(victim),
           AN( races[ GET_RACE(victim) ].name ),
           races[ GET_RACE(victim) ].name );
   
   GET_LEVEL(victim)             = 1;
   GET_GOLD(victim)              = 250;
   GET_BANK_GOLD(victim)         = 250;
   GET_HITROLL(victim)           = 1;
   GET_DAMROLL(victim)           = 1;
   roll_real_abils(victim);      /* reroll and stuff */
   
   switch (number(0, 6)) {
     case 0: GET_CHA(victim)   += 1; break;
     case 1: GET_STR(victim)   += 1; break;
     case 2: GET_WIS(victim)   += 1; break;
     case 3: GET_INT(victim)   += 1; break;
     case 4: GET_DEX(victim)   += 1; break;
     case 5: GET_CON(victim)   += 1; break;
     case 6: GET_GOLD(victim) += number(150, 300); break;
     default: break;
   }
   /* Char's lose ALL of their eq */
   for (i = 0; i < NUM_WEARS; i++)
     perform_remove(victim, i);
   
   for (tobj = victim->carrying; tobj; tobj = tobj->next) {
     obj_from_char(tobj);
     extract_obj(tobj);
   }
   /* Give them the same shit newbies get */
   newbie_basket(victim);
   send_to_char("The remorting process is done, but if you made a mistake you will have to use the set command.\r\n",ch);
   send_to_char(REMORTMESS, victim);
}

/**
 */
ACMD(do_return)
{
  if (ch->desc && ch->desc->original) {
    send_to_char("You return to your original body.\r\n", ch);
    
    /* JE 2/22/95 */
    /* if someone switched into your original body, disconnect them */
    if (ch->desc->original->desc)
      close_socket(ch->desc->original->desc);
    
    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;
    
    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
  }
}

/**
 */
ACMD(do_create)
{
  struct char_data *mob;
  struct obj_data *obj;
  long   number, r_num;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit((int)*buf2)) {
    send_to_char("Usage: create { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	0, 0, TO_ROOM);
    act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}

/**
 */
ACMD(do_vstat)
{
  struct char_data *mob;
  struct obj_data *obj;
  long   number, r_num;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit((int)*buf2)) {
    send_to_char("Usage: vstat { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, 0);
    do_stat_character(ch, mob);
    extract_char(mob);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    do_stat_object(ch, obj);
    extract_obj(obj);
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}

/**
 * stop fighting - TD 4/6/95 - SPM
 */
ACMD(do_peace)
{
  struct char_data *vict, *next_v;
  
  act("With a gesture of $s hand, $n gives you a warm and fuzzy feeling.",
      FALSE, ch, 0, 0, TO_ROOM);
  send_to_room("Everything is quite peaceful now.\r\n", ch->in_room);
  send_to_char("You force everyone to solve their differences and stop fighting.\r\n",ch);
  
  for (vict = world[ch->in_room].people; vict; vict = next_v) {
    next_v = vict->next_in_room;

    if ((FIGHTING(vict)) && GET_LEVEL(vict) <= GET_LEVEL(ch)) {
      if (FIGHTING(FIGHTING(vict)) == vict)
        stop_fighting(FIGHTING(vict));
      stop_fighting(vict);
    }
  }
}

/**
 * clean a room of all mobiles and objects
 */
ACMD(do_purge)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  one_argument(argument, buf);

  if (*buf) {                   /* argument supplied. destroy single object
				 * or char */
    if ((vict = get_char_room_vis(ch, buf))) {
      if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
	send_to_char("Fuuuuuuuuu!\r\n", ch);
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (!IS_NPC(vict)) {
	sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
	mudlog(buf, BRF, LVL_GOD, TRUE);
	if (vict->desc) {
	  close_socket(vict->desc);
	  vict->desc = NULL;
	}
      }
      if (RIDDEN(vict)) {
	MOUNTED(RIDDEN(vict)) = NULL;
	RIDDEN(vict) = NULL; }
      if (MOUNTED(vict)) {
	RIDDEN(MOUNTED(vict)) = NULL;
	MOUNTED(vict) = NULL; }
      extract_char(vict);
    } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      send_to_char("Nothing here by that name.\r\n", ch);
      return;
    }

    send_to_char(OK, ch);
  } else {                      /* no argument. clean out the room */
    act("$n gestures... You are surrounded by scorching flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

    for (vict = world[ch->in_room].people; vict; vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict)) {
	if (RIDDEN(vict)) {
	  MOUNTED(RIDDEN(vict)) = NULL;
	  RIDDEN(vict) = NULL; }
	if (MOUNTED(vict)) {
	  RIDDEN(MOUNTED(vict)) = NULL;
	  MOUNTED(vict) = NULL; }
	extract_char(vict);
      }
    }

    for (obj = world[ch->in_room].contents; obj; obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
  }
}

/**
 */
ACMD(do_advance)
{
  struct char_data *victim;
  char   *name = arg, *level = buf2;
  int    newlevel = 0, oldlevel;
  void   do_start(struct char_data *ch);
  void   gain_exp(struct char_data * ch, int gain);

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name))) {
      send_to_char("That player is not here.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
    send_to_char("Maybe that's not such a great idea.\r\n", ch);
    return;
  }
  if (IS_NPC(victim)) {
    send_to_char("NO!  Not on NPC's.\r\n", ch);
    return;
  }
  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char("That's not a level!\r\n", ch);
    return;
  }
  if (newlevel > LVL_IMPL) {
    sprintf(buf, "%d is the highest possible level.\r\n", LVL_IMPL);
    send_to_char(buf, ch);
    return;
  }
  if (newlevel > GET_LEVEL(ch)) {
    send_to_char("Yeah, right.\r\n", ch);
    return;
  }
  if (newlevel == GET_LEVEL(victim)) {
    act("$N is already at that level!", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  oldlevel = GET_LEVEL(victim);
  if (newlevel < GET_LEVEL(victim)) {
    do_start(victim);
    GET_LEVEL(victim) = newlevel;
    send_to_char("You are momentarily enveloped by darkness!\r\n"
		 "You feel somewhat diminished.\r\n", victim);
  } else {
    act("$n makes some strange gestures.\r\n"
	"A strange feeling comes upon you,\r\n"
	"Like a giant hand, light comes down\r\n"
	"from above, grabbing your body, that\r\n"
	"begins to pulse with colored lights\r\n"
	"from inside.\r\n\r\n"
	"Your head seems to be filled with demons\r\n"
	"from another plane as your body dissolves\r\n"
	"to the elements of time and space itself.\r\n"
	"Suddenly a silent explosion of light\r\n"
	"snaps you back to reality.\r\n\r\n"
	"You feel slightly different.", FALSE, ch, 0, victim, TO_VICT);
  }
  send_to_char(OK, ch);

  sprintf(buf, "(GC) %s has advanced %s to level %d (from %d)",
	  GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);
  log(buf);
  gain_exp_regardless(victim, exp_needed(newlevel));
  save_char(victim, NOWHERE);
}

/**
 * restore one character to perfect condition.
 */
ACMD(do_restore)
{
  struct char_data *vict;
  int i;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to restore?\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else {
    GET_HIT(vict)  = GET_MAX_HIT(vict);
    GET_MANA(vict) = GET_MAX_MANA(vict);
    GET_MOVE(vict) = GET_MAX_MOVE(vict);
    GET_WOUNDS(vict) = 0;

    if ((GET_LEVEL(ch) >= LVL_GRGOD) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
      for (i = 1; i <= MAX_SKILLS; i++)
	SET_SKILL(vict, i, 100);

      if (GET_LEVEL(vict) >= LVL_BUILDER) {
	vict->real_abils.str_add = 100;
	vict->real_abils.intel = 25;
	vict->real_abils.wis   = 25;
	vict->real_abils.dex   = 25;
	vict->real_abils.str   = 25;
	vict->real_abils.con   = 25;
	vict->real_abils.cha   = 25;
	vict->real_abils.will  = 25;
      }
      vict->aff_abils = vict->real_abils;
    }
    update_pos(vict);
    send_to_char(OK, ch);
    act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);
  }
}

/**
 * helper function allow a wizinvis immort to become visible
 */
void perform_immort_vis(struct char_data *ch)
{
  void appear(struct char_data *ch);

  if (GET_INVIS_LEV(ch) == 0 && !IS_AFFECTED(ch, AFF_HIDE | AFF_INVISIBLE)) {
    send_to_char("You are already fully visible.\r\n", ch);
    return;
  }
   
  GET_INVIS_LEV(ch) = 0;
  appear(ch);
  send_to_char("You are now fully visible.\r\n", ch);
}

/**
 * helper function to allow an immort to become invisible
 */
void perform_immort_invis(struct char_data *ch, int level)
{
  struct char_data *tch;

  if (IS_NPC(ch))
    return;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;
    if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
      act("You blink and suddenly realize that $n is gone.", FALSE, ch, 0,
	  tch, TO_VICT);
    if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
      act("You suddenly realize that $n is standing beside you.", FALSE, ch, 0,
	  tch, TO_VICT);
  }

  GET_INVIS_LEV(ch) = level;
  sprintf(buf, "Your invisibility level is %d.\r\n", level);
  send_to_char(buf, ch);
}
  
/**
 * allow an immort to become invisble to all those of a particular
 * level or visible.
 */
ACMD(do_invis)
{
  int level;

  if (IS_NPC(ch)) {
    send_to_char("You can't do that!\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, GET_LEVEL(ch));
  } else {
    level = atoi(arg);
    if (level > GET_LEVEL(ch))
      send_to_char("You can't go invisible above your own level.\r\n", ch);
    else if (level < 1)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, level);
  }
}


ACMD(do_gecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);

  if (!*argument)
    send_to_char("That must be a mistake...\r\n", ch);
  else {
    sprintf(buf, "%s\r\n", argument);
    for (pt = descriptor_list; pt; pt = pt->next)
      if (!pt->connected && pt->character && pt->character != ch)
	send_to_char(buf, pt->character);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      send_to_char(buf, ch);
  }
}


ACMD(do_poofset)
{
  char **msg;

  switch (subcmd) {
  case SCMD_POOFIN:    msg = &(POOFIN(ch));    break;
  case SCMD_POOFOUT:   msg = &(POOFOUT(ch));   break;
  default:    return;    break;
  }

  skip_spaces(&argument);

  if (*msg)
    free(*msg);

  if (!*argument)
    *msg = NULL;
  else
    *msg = str_dup(argument);

  send_to_char(OK, ch);
}



ACMD(do_dc)
{
  struct descriptor_data *d;
  int num_to_dc;

  one_argument(argument, arg);
  if (!(num_to_dc = atoi(arg))) {
    send_to_char("Usage: DC <connection number> (type USERS for a list)\r\n", ch);
    return;
  }
  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    send_to_char("No such connection.\r\n", ch);
    return;
  }
  if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
    send_to_char("Umm.. maybe that's not such a good idea...\r\n", ch);
    return;
  }
  close_socket(d);
  sprintf(buf, "Connection #%d closed.\r\n", num_to_dc);
  send_to_char(buf, ch);
  sprintf(buf, "(GC) Connection closed by %s.", GET_NAME(ch));
  log(buf);
}



ACMD(do_wizlock)
{
  int value;
  char *when;

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > GET_LEVEL(ch)) {
      send_to_char("Invalid wizlock value.\r\n", ch);
      return;
    }
    game_restrict = value;
    when = "now";
  } else
    when = "currently";

  switch (game_restrict) {
  case 0:
    sprintf(buf, "The game is %s completely open.\r\n", when);
    break;
  case 1:
    sprintf(buf, "The game is %s closed to new players.\r\n", when);
    break;
  default:
    sprintf(buf, "Only level %d and above may enter the game %s.\r\n",
	    game_restrict, when);
    break;
  }
  send_to_char(buf, ch);
}


ACMD(do_date)
{
  char *tmstr;
  time_t mytime;
  int d, h, m;
  extern time_t boot_time;

  if (subcmd == SCMD_DATE)
    mytime = time(0);
  else
    mytime = boot_time;

  tmstr = (char *) asctime(localtime(&mytime));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (subcmd == SCMD_DATE)
    sprintf(buf, "Current machine time: %s\r\n", tmstr);
  else {
    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
	    ((d == 1) ? "" : "s"), h, m);
  }

  send_to_char(buf, ch);
}



ACMD(do_last)
{
  struct char_file_u chdata;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("For whom do you wish to search?\r\n", ch);
    return;
  }
  if (load_char(arg, &chdata) < 0) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }
  if (chdata.level > GET_LEVEL(ch)) {
    send_to_char("You are not sufficiently godly for that!\r\n", ch);
    return;
  }
  sprintf(buf, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
	  chdata.char_specials_saved.idnum, chdata.level,
          races[ chdata.race ].abbrev, chdata.name, chdata.host,
	  ctime(&chdata.last_logon));
  send_to_char(buf, ch);
}


ACMD(do_force)
{
  int notinroom = 0;
  struct descriptor_data *i, *next_desc;
  struct char_data *vict = NULL, *next_force;
  char to_force[MAX_INPUT_LENGTH + 2];

  half_chop(argument, arg, to_force);

  sprintf(buf1, "$n has forced you to '%s'.", to_force);

  if (!*arg || !*to_force)
    send_to_char("Whom do you wish to force do what?\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_GRGOD) || (str_cmp("all", arg) && str_cmp("room", arg))) {
    if (!(vict = get_char_room_vis(ch, arg)))
      notinroom = 1;
    if (notinroom && (!(vict = get_char_vis(ch, arg))))
      send_to_char(NOPERSON, ch);
    else if (GET_LEVEL(ch) <= GET_LEVEL(vict))
      send_to_char("No, no, no!\r\n", ch);
    else {
      send_to_char(OK, ch);
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
      mudlog(buf, NRM, MAX((int)LVL_GOD, (int)GET_INVIS_LEV(ch)), TRUE);
      command_interpreter(vict, to_force);
    }
  } else if (!str_cmp("room", arg)) {
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced room %ld to %s", GET_NAME(ch), 
	    world[ch->in_room].number, to_force);
    mudlog(buf, NRM, MAX((int)LVL_GOD, (int)GET_INVIS_LEV(ch)), TRUE);

    for (vict = world[ch->in_room].people; vict; vict = next_force) {
      next_force = vict->next_in_room;
      if (GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced all to %s", GET_NAME(ch), to_force);
    mudlog(buf, NRM, MAX((int)LVL_GOD, (int)GET_INVIS_LEV(ch)), TRUE);

    for (i = descriptor_list; i; i = next_desc) {
      next_desc = i->next;

      if (i->connected || !(vict = i->character) || GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  }
}



ACMD(do_wiznet)
{
  struct descriptor_data *d;
  char emote = FALSE;
  char any = FALSE;
  int level = LVL_IMMORT;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("Usage: wiznet <text> | #<level> <text> | *<emotetext> |\r\n "
		 "       wiznet @<level> *<emotetext> | wiz @\r\n", ch);
    return;
  }
  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), (int)LVL_IMMORT);
      if (level > GET_LEVEL(ch)) {
	send_to_char("You can't wizline above your own level.\r\n", ch);
	return;
      }
    } else if (emote)
      argument++;
    break;
  case '@':
    for (d = descriptor_list; d; d = d->next) {
      if (!d->connected && GET_LEVEL(d->character) >= LVL_IMMORT &&
	  !PRF_FLAGGED(d->character, PRF_NOWIZ) &&
	  (CAN_SEE(ch, d->character) || GET_LEVEL(ch) == LVL_IMPL)) {
	if (!any) {
	  sprintf(buf1, "Gods online:\r\n");
	  any = TRUE;
	}
	sprintf(buf1, "%s  %s", buf1, GET_NAME(d->character));
	if (PLR_FLAGGED(d->character, PLR_WRITING))
	  sprintf(buf1, "%s (Writing)\r\n", buf1);
	else if (PLR_FLAGGED(d->character, PLR_MAILING))
	  sprintf(buf1, "%s (Writing mail)\r\n", buf1);
	else
	  sprintf(buf1, "%s\r\n", buf1);

      }
    }
    any = FALSE;
    for (d = descriptor_list; d; d = d->next) {
      if (!d->connected && GET_LEVEL(d->character) >= LVL_IMMORT &&
	  PRF_FLAGGED(d->character, PRF_NOWIZ) &&
	  CAN_SEE(ch, d->character)) {
	if (!any) {
	  sprintf(buf1, "%sGods offline:\r\n", buf1);
	  any = TRUE;
	}
	sprintf(buf1, "%s  %s\r\n", buf1, GET_NAME(d->character));
      }
    }
    send_to_char(buf1, ch);
    return;
    break;
  case '\\':
    ++argument;
    break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    send_to_char("You are offline!\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Don't bother the gods like that!\r\n", ch);
    return;
  }
  if (level > LVL_IMMORT) {
    sprintf(buf1, "%s: <%d> %s%s\r\n", GET_NAME(ch), level,
	    emote ? "<--- " : "", argument);
    sprintf(buf2, "Someone: <%d> %s%s\r\n", level, emote ? "<--- " : "",
	    argument);
  } else {
    sprintf(buf1, "%s: %s%s\r\n", GET_NAME(ch), emote ? "<--- " : "",
	    argument);
    sprintf(buf2, "Someone: %s%s\r\n", emote ? "<--- " : "", argument);
  }

  for (d = descriptor_list; d; d = d->next) {
    if ((!d->connected) && (GET_LEVEL(d->character) >= level) &&
	(!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
	(!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING))
	&& (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
      send_to_char(CCCYN(d->character, C_NRM), d->character);
      if (CAN_SEE(d->character, ch))
	send_to_char(buf1, d->character);
      else
	send_to_char(buf2, d->character);
      send_to_char(CCNRM(d->character, C_NRM), d->character);
    }
  }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
}



ACMD(do_zreset)
{
  void reset_zone(int zone);

  int i, j;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("You must specify a zone.\r\n", ch);
    return;
  }
  if (*arg == '*') {
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i);
    send_to_char("Reset world.\r\n", ch);
    sprintf(buf, "(GC) %s reset entire world.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_GRGOD, (int)GET_INVIS_LEV(ch)), TRUE);
    return;
  } else if (*arg == '.')
    i = world[ch->in_room].zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
	break;
  }
  if (i >= 0 && i <= top_of_zone_table) {
    reset_zone(i);
    sprintf(buf, "Reset zone %d (#%d): %s.\r\n", i, zone_table[i].number,
	    zone_table[i].name);
    send_to_char(buf, ch);
    sprintf(buf, "(GC) %s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name);
    mudlog(buf, NRM, MAX((int)LVL_GRGOD, (int)GET_INVIS_LEV(ch)), TRUE);
  } else
    send_to_char("Invalid zone number.\r\n", ch);
}


ACMD(do_gen_vfile)
{
   char syscom[120], arg[50];
   FILE *pfp;
   char *filename = NULL;
   int  i = 20;

   int pclose(FILE *pfp);

   one_argument(argument, arg);
   if ((i = atoi(arg)) >= 1 && i <= 120)
     ;
   else
     i = 20;

   switch (subcmd) {
    case SCMD_V_TODO:
      filename = TODO_FILE;
      break;
    case SCMD_V_BUGS:
      filename = BUG_FILE;
      break;
    case SCMD_V_IDEAS:
      filename = IDEA_FILE;
      break;
    case SCMD_V_TYPOS:
      filename = TYPO_FILE;
      break;
    case SCMD_V_SYSLOG:
      filename = SYSLOG_FILE;
      break;
    case SCMD_V_SHOWPLRS:
      break;
    default:
      send_to_char("INVALID COMMAND!!: go_gen_vfile\r\n", ch);
      return;
      break;
   }
   if (subcmd != SCMD_V_SHOWPLRS)
     sprintf(syscom, "tail -%d %s", i, filename);
   else
     sprintf(syscom, "../showplrs");

   if ((pfp = popen(syscom, "r")) == NULL) {
      send_to_char("No entries found.\n\r",ch);
      return;
   }
   if (subcmd != SCMD_V_QUOTES && subcmd != SCMD_V_OQUOTE)
     strcpy(buf2, "Contents of file:\r\n");
   else
     sprintf(buf2, "%s\r\n\r\n", CLEAR_RESET);

   while (fgets(syscom, 120, pfp) != NULL){
      sprintf(buf2, "%s%s\r", buf2, syscom);
   }
   page_string(ch->desc, buf2, 0);
   pclose(pfp);
}



/*
 *  General fn for wizcommands of the sort: cmd <player>
 */

ACMD(do_wizutil)
{
  struct char_data *vict;
  long result;
  void roll_real_abils(struct char_data *ch);

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Yes, but for whom?!?\r\n", ch);
  else if (!(vict = get_char_vis(ch, arg)))
    send_to_char("There is no such player.\r\n", ch);
  else if (IS_NPC(vict))
    send_to_char("You can't do that to a mob!\r\n", ch);
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    send_to_char("Hmmm...you'd better not.\r\n", ch);
  else {
    switch (subcmd) {
    case SCMD_REROLL:
      send_to_char("Rerolled...\r\n", ch);
      roll_real_abils(vict);
      sprintf(buf, "(GC) %s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
      log(buf);
      sprintf(buf, "New stats: Str %d/%d, Int %d, Wis %d, Dex %d, Con %d, Cha %d, Will %d\r\n",
	      GET_STR(vict), GET_ADD(vict), GET_INT(vict), GET_WIS(vict),
	      GET_DEX(vict), GET_CON(vict), GET_CHA(vict), GET_WILL(vict));
      send_to_char(buf, ch);
      break;
    case SCMD_PARDON:
      if (!PLR_FLAGGED(vict, PLR_THIEF | PLR_KILLER)) {
	send_to_char("Your victim is not flagged.\r\n", ch);
	return;
      }
      REMOVE_BIT(PLR_FLAGS(vict), PLR_THIEF | PLR_KILLER);
      send_to_char("Pardoned.\r\n", ch);
      send_to_char("You have been pardoned by the Gods!\r\n", vict);
      sprintf(buf, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX((int)LVL_GOD, (int)GET_INVIS_LEV(ch)), TRUE);
      break;
    case SCMD_NOTITLE:
      result = PLR_TOG_CHK(vict, PLR_NOTITLE);
      sprintf(buf, "(GC) Notitle %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, NRM, MAX((int)LVL_GOD, (int)GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case SCMD_SQUELCH:
      result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
      sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX((int)LVL_GOD, (int)GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case SCMD_FREEZE:
      if (ch == vict) {
	send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
	return;
      }
      if (PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char("Your victim is already pretty cold.\r\n", ch);
	return;
      }
      SET_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      GET_FREEZE_LEV(vict) = GET_MAX_LEVEL(ch);
      send_to_char("A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n", vict);
      send_to_char("Frozen.\r\n", ch);
      act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
      sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX((int)LVL_GOD, (int)GET_INVIS_LEV(ch)), TRUE);
      break;
    case SCMD_THAW:
      if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
	return;
      }
      if (GET_FREEZE_LEV(vict) > GET_MAX_LEVEL(ch)) {
	sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
	   GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
	send_to_char(buf, ch);
	return;
      }
      sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX((int)LVL_GOD, (int)GET_INVIS_LEV(ch)), TRUE);
      REMOVE_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
      send_to_char("Thawed.\r\n", ch);
      act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
      break;
    case SCMD_UNAFFECT:
      if (vict->affected2) {
	while (vict->affected2)
	  affect_remove(vict, vict->affected2, TRUE);
	AFF2_FLAGS(ch) = 0;
	send_to_char("There is a brief flash of light!\r\n"
		     "You feel slightly different.\r\n", vict);
	send_to_char("All _bad_ spells removed.\r\n", ch);
        if (vict->affected) {
	  while (vict->affected)
	    affect_remove(vict, vict->affected, TRUE);
	  AFF_FLAGS(ch) = 0;
	  send_to_char("There is another brief flash of light!\r\n"
		     "You feel different.\r\n", vict);
        } 
      } else {
	send_to_char("Your victim does not have any affections!\r\n", ch);
	return;
      }
      break;
    default:
      log("SYSERR: Unknown subcmd passed to do_wizutil (act.wizard.c)");
      break;
    }
    save_char(vict, NOWHERE);
  }
}

/* Single Organization printing to show rels, clans, etc */
void show_orgs(struct char_data *ch, char *value, int type)
{
   int    i, j = -1;
   float  percent, total = 0, members = 0;
   char   org_list[MAX_STRING_LENGTH], tempbuf[MAX_INPUT_LENGTH];
   struct char_file_u player;

   sprintf(org_list, "\r\n");

   for (i = 0; i < top_of_p_table; i++) {
       if (load_char(player_table[i].name, &player) > -1) {
	  total++;

	  if (type == 1) {  /* Get guild */
#if 0 // sorry the code to parse_guild has been lost!            
	     if ((j = parse_guild(*value)) == GUILD_UNDEFINED) {
#endif               
		send_to_char("That is not a guild, valid guilds are:\r\n", ch);
                show_guild_trainers( ch );
		return;
#if 0                
	     }
	     else if (player.assocs[1] == j) {
		sprintf(org_list, "%s%s %s\r\n", org_list, player.name, player.title);
		members++;
	     }
#endif
	  } else {
	     if ((j >= NUM_RELIGIONS) || j < 0) {
		send_to_char("That is not a religion, valid religions are:\r\n", ch);
		return;
	     }
	     else if (player.assocs[0] == j) {
		sprintf(org_list, "%s%s %s\r\n", org_list, player.name, player.title);
		members++;
	     }
	  }
       }
   }
   if (type != 1)
     CAP(value);
   sprintf(tempbuf, "Members of %s%s %s.\r\n", type ? buf : value,
	   ((type == 0) ? "'s" : ""), ((type == 0) ? "Religion" : ""));
   sprintf(tempbuf, "%s~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n", tempbuf);
   send_to_char(tempbuf, ch);
   percent = (members/total)*100;
   sprintf(org_list, "%s\r\n%3.1f%% of the players in the game are in this %s.\r\n",
	  org_list, percent, (type == 0 ? "religion" : "guild"));
   page_string(ch->desc, org_list, 1);
   return;
}

void show_print_guilds(struct char_data *ch)
{
   int i;
   bool first = TRUE;
   struct guild_type *gptr = guild_info;

   strcpy(buf, "Guilds:\r\n");
   while (gptr != NULL) {
     sprintf(buf, "%s[%2d] %-40.40s\t\t-- (%ld) %s\r\n",
	     buf, gptr->number, gptr->name, 
	     gptr->guild_entr_room, gptr->leadersname);
     for (i = 0; i < MAX_PETITIONERS; i++) {
       if (gptr->petitioners[i] != NULL) {
         if (first) {
           first = FALSE;
           strcat(buf, "     Petitioners: ");
         }
	 sprintf(buf, "%s%s ", buf, gptr->petitioners[i]);
       }
     }
     if (!first)
       sprintf(buf, "%s\r\n", buf);
     
     gptr = gptr->next;
     first = TRUE;
   }
   strcat(buf, "\r\n");
   send_to_char(buf, ch);
}

/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void print_zone_to_buf(char *bufptr, int zone)
{
  sprintf(bufptr, "%s%3d %-30.30s Age: %3d; Reset: %3d (%1d); Top: %ld\r\n",
	  bufptr, zone_table[zone].number, zone_table[zone].name,
	  zone_table[zone].age, zone_table[zone].lifespan,
	  zone_table[zone].reset_mode, zone_table[zone].top);
}


ACMD(do_show)
{
  struct char_file_u vbuf;
  long i, j, k, l, con;
  char self = 0;
  struct char_data *vict;
  struct obj_data *obj;
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], birth[80];
  extern char *genders[];
  extern int buf_switches, buf_largecount, buf_overflows;
  void show_shops(struct char_data * ch, char *value);
  void hcontrol_list_houses(struct char_data *ch);

  struct show_struct {
    char *cmd;
    char level;
  } fields[] = {
    { "nothing",        0  },                           /* 0 */
    { "zones",          LVL_IMMORT },                   /* 1 */
    { "player",         LVL_GOD },
    { "rent",           LVL_GOD },
    { "stats",          LVL_IMMORT },
    { "errors",         LVL_IMPL },                     /* 5 */
    { "death",          LVL_GOD },
    { "godrooms",       LVL_GOD },
    { "shops",          LVL_IMMORT },
    { "houses",         LVL_GOD },
    { "religion",       LVL_GRGOD },                    /* 10 */
    { "guild",          LVL_GOD },
    { "trainers",       LVL_BUILDER },
    { "\n", 0 }
  };

  skip_spaces(&argument);

  if (!*argument) {
    strcpy(buf, "Show options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_MAX_LEVEL(ch))
	sprintf(buf, "%s%-15s%s", buf, fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_MAX_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  if (!strcmp(value, "."))
    self = 1;
  buf[0] = '\0';
  switch (l) {
  case 1:                       /* zone */
    /* tightened up by JE 4/6/93 */
    if (self)
      print_zone_to_buf(buf, world[ch->in_room].zone);
    else if (*value && is_number(value)) {
      for (j = atoi(value), i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
      if (i <= top_of_zone_table)
	print_zone_to_buf(buf, i);
      else {
	send_to_char("That is not a valid zone.\r\n", ch);
	return;
      }
    } else
      for (i = 0; i <= top_of_zone_table; i++)
	print_zone_to_buf(buf, i);
    page_string(ch->desc, buf, 1);
    break;
  case 2:                       /* player */
    if (!*value) {
      send_to_char("A name would help.\r\n", ch);
      return;
    }

    if (load_char(value, &vbuf) < 0) {
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
    sprintf(buf, "Player: %-12s (%s) [%2d %s]\r\n", vbuf.name,
            genders[(int) vbuf.sex], vbuf.level,
            races[ vbuf.race].abbrev );
    sprintf(buf,
	 "%sAu: %-ld  Bal: %-ld  Exp: %-ld  Align: %-5d  Lessons: %-3d\r\n",
	    buf, vbuf.points.gold, vbuf.points.bank_gold, vbuf.points.exp,
	    vbuf.char_specials_saved.alignment,
	    vbuf.player_specials_saved.spells_to_learn);
    strcpy(birth, ctime(&vbuf.birth));
    sprintf(buf,
	    "%sStarted: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
	    buf, birth, ctime(&vbuf.last_logon), (int) (vbuf.played / 3600),
	    (int) (vbuf.played / 60 % 60));
    send_to_char(buf, ch);
    break;
  case 3:
    Crash_listrent(ch, value);
    break;
  case 4:
    i = 0;
    j = 0;
    k = 0;
    con = 0;
    for (vict = character_list; vict; vict = vict->next) {
      if (IS_NPC(vict))
	j++;
      else if (CAN_SEE(ch, vict)) {
	i++;
	if (vict->desc)
	  con++;
      }
    }
    for (obj = object_list; obj; obj = obj->next)
      k++;
    sprintf(buf, "Current stats:\r\n");
    sprintf(buf, "%s  %5ld players in game  %5ld connected\r\n", buf, i, con);
    sprintf(buf, "%s  %5d registered\r\n", buf, top_of_p_table + 1);
    sprintf(buf, "%s  %5ld mobiles          %5ld prototypes\r\n",
	    buf, j, top_of_mobt + 1);
    sprintf(buf, "%s  %5ld objects          %5ld prototypes\r\n",
	    buf, k, top_of_objt + 1);
    sprintf(buf, "%s  %5ld rooms            %5d zones\r\n",
	    buf, top_of_world + 1, top_of_zone_table + 1);
    sprintf(buf, "%s  %5d large bufs\r\n", buf, buf_largecount);
    sprintf(buf, "%s  %5d buf switches     %5d overflows\r\n", buf,
	    buf_switches, buf_overflows);
    send_to_char(buf, ch);
    break;
  case 5:
    strcpy(buf, "Errant Rooms\r\n------------\r\n");
    for (i = 0, k = 0; i <= top_of_world; i++)
      for (j = 0; j < NUM_OF_DIRS; j++)
	if (world[i].dir_option[j] && world[i].dir_option[j]->to_room == 0)
	  sprintf(buf, "%s%2d: [%ld] %s\r\n", buf, (int) ++k, world[i].number,
		  world[i].name);
    send_to_char(buf, ch);
    break;
  case 6:
    strcpy(buf, "Death Traps\r\n-----------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (IS_SET(ROOM_FLAGS(i), ROOM_DEATH))
	sprintf(buf, "%s%2d: [%ld] %s\r\n", buf, (int) ++j,
		world[i].number, world[i].name);
    send_to_char(buf, ch);
    break;
  case 7:
#define GOD_ROOMS_ZONE 0
    strcpy(buf, "Godrooms\r\n--------------------------\r\n");
    for (i = 0, j = 0; i < top_of_world; i++) {
      if (world[i].zone == GOD_ROOMS_ZONE)
	sprintf(buf, "%s%2d: [%ld] %s\r\n", buf, (int) j++, world[i].number,
		world[i].name);
      else if (IS_SET(world[i].room_flags, ROOM_GODROOM))
	sprintf(buf, "%s%2d: [%ld] %s\r\n", buf, (int) j++, world[i].number,
		world[i].name);
    }
    send_to_char(buf, ch);
    break;
  case 8:
    show_shops(ch, value);
    break;
  case 9:
    hcontrol_list_houses(ch);
    break;
  case 10:
    if (!*value) {
       send_to_char("Please specify a god for the religion you'd like info on.\r\n", ch);
       return;
    }
    show_orgs(ch, value, 0);
    break;
  case 11:
    show_print_guilds(ch);
    break;
  case 12:
    show_guild_trainers(ch);
    break;

  default:
    send_to_char("Sorry, I don't understand that.\r\n", ch);
    break;
  }
}


#define PC   1
#define NPC  2
#define BOTH 3

#define MISC    0
#define BINARY  1
#define NUMBER  2

#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT(flagset, flags); \
	else if (off) REMOVE_BIT(flagset, flags); }

#define RANGE(low, high) (value = MAX((long)(low), MIN((long)(high), (long)(value))))

ACMD(do_set)
{
  int    i = 0, l, on = 0, off = 0, player_i = 0;
  struct char_data *vict = NULL, *cbuf = NULL;
  struct char_file_u tmp_store;
  struct guild_type *gptr = NULL;
  char   field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], val_arg[MAX_INPUT_LENGTH];
  long   value = 0;
  char   is_file = 0, is_mob = 0, is_player = 0;

  int parse_race(char arg);

  struct set_struct {
    char *cmd;
    char level;
    char pcnpc;
    char type;
  }          fields[] = {
   { "brief",           LVL_GOD,        PC,     BINARY },  /* 0 */
   { "invstart",        LVL_GOD,        PC,     BINARY },  /* 1 */
   { "title",           LVL_GOD,        PC,     MISC },
   { "maxhit",          LVL_GRGOD,      BOTH,   NUMBER },
   { "maxmana",         LVL_GRGOD,      BOTH,   NUMBER },
   { "maxmove",         LVL_GRGOD,      BOTH,   NUMBER },  /* 5 */
   { "hit",             LVL_GRGOD,      BOTH,   NUMBER },
   { "mana",            LVL_GRGOD,      BOTH,   NUMBER },
   { "move",            LVL_GRGOD,      BOTH,   NUMBER },
   { "align",           LVL_GOD,        BOTH,   NUMBER },
   { "palign",          LVL_IMPL,       BOTH,   NUMBER },  /* 10 */
   { "str",             LVL_GRGOD,      BOTH,   NUMBER },
   { "stradd",          LVL_GRGOD,      BOTH,   NUMBER },
   { "int",             LVL_GRGOD,      BOTH,   NUMBER },
   { "wis",             LVL_GRGOD,      BOTH,   NUMBER },
   { "dex",             LVL_GRGOD,      BOTH,   NUMBER },  /* 15 */
   { "con",             LVL_GRGOD,      BOTH,   NUMBER },
   { "cha",             LVL_GRGOD,      BOTH,   NUMBER },  /* 17 */
   { "will",            LVL_GRGOD,      BOTH,   NUMBER },  /* 18 */
   { "sex",             LVL_GRGOD,      BOTH,   MISC },    /* 19 */
   { "gold",            LVL_GOD,        BOTH,   NUMBER },  /* 20 */
   { "bank",            LVL_GOD,        PC,     NUMBER },
   { "exp",             LVL_GRGOD,      BOTH,   NUMBER },
   { "hitroll",         LVL_GRGOD,      BOTH,   NUMBER },
   { "damroll",         LVL_GRGOD,      BOTH,   NUMBER },
   { "invis",           LVL_SUP,        PC,     NUMBER },  /* 25 */
   { "nohassle",        LVL_GRGOD,      PC,     BINARY },  /* 26 */
   { "frozen",          LVL_FREEZE,     PC,     BINARY },
   { "practices",       LVL_GRGOD,      PC,     NUMBER },
   { "lessons",         LVL_GRGOD,      PC,     NUMBER },
   { "drunk",           LVL_GRGOD,      BOTH,   MISC },    /* 30 */
   { "hunger",          LVL_GRGOD,      BOTH,   MISC },    /* 31 */
   { "thirst",          LVL_GRGOD,      BOTH,   MISC },
   { "tired",           LVL_GRGOD,      BOTH,   MISC },
   { "killer",          LVL_GOD,        PC,     BINARY },  /* 34 */
   { "thief",           LVL_GOD,        PC,     BINARY },  /* 35 */
   { "level",           LVL_SUP,        BOTH,   NUMBER },
   { "room",            LVL_IMPL,       BOTH,   NUMBER },  /* 37 */
   { "roomflag",        LVL_GRGOD,      PC,     BINARY },  /* 38 */
   { "siteok",          LVL_GRGOD,      PC,     BINARY },
   { "deleted",         LVL_IMPL,       PC,     BINARY },
   { "olc",             LVL_SUP,        PC,     NUMBER },  /* 41 */
   { "nowizlist",       LVL_GOD,        PC,     BINARY },  /* 42 */
   { "quest",           LVL_GOD,        PC,     BINARY },
   { "loadroom",        LVL_GRGOD,      PC,     MISC },
   { "color",           LVL_GOD,        PC,     BINARY },  /* 45 */
   { "idnum",           LVL_CREATOR,    PC,     NUMBER },
   { "passwd",          LVL_CREATOR,    PC,     MISC },    /* 47 */
   { "nodelete",        LVL_GRGOD,      PC,     BINARY },
   { "race",            LVL_GOD,        BOTH,   MISC },
   { "religion",        LVL_GRGOD,      PC,     NUMBER },    /* 50 */
   { "guild",           LVL_SUP,        PC,     NUMBER },
   { "glevel",          LVL_SUP,        PC,     NUMBER },    /* 52 */
   { "height",          LVL_GOD,        PC,     NUMBER },
   { "weight",          LVL_GOD,        PC,     NUMBER },    /* 54 */
   { "\n", 0, BOTH, MISC }
  };

  half_chop(argument, name, buf);
  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob")) {
    is_mob = 1;
    half_chop(buf, name, buf);
  }
  half_chop(buf, field, buf);
  strcpy(val_arg, buf);

  if (!*name) {
    send_to_char("Usage: set <victim> <field> <value>\r\n", ch);
    return;
  }
  if (!*field) {
    send_to_char("Usage: set <victim> <field> <value>\r\n", ch);
    strcpy(buf, "Fielding being one of:\r\n");
    while (fields[i].cmd[0] != '\n')
      sprintf(buf, "%s%s", fields[i].cmd, ((++i) % 3) ? "\t" : "\r\n");
    return;
  }
  if (!is_file) {
    if (is_player) {
      if (!(vict = get_player_vis(ch, name, 0))) {
	send_to_char("There is no such player.\r\n", ch);
	return;
      }
    } else {
      if (!(vict = get_char_vis(ch, name))) {
	send_to_char("There is no such creature.\r\n", ch);
	return;
      }
    }
  } else if (is_file) {
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    if ((player_i = load_char(name, &tmp_store)) > -1) {
      store_to_char(&tmp_store, cbuf);
      if (GET_LEVEL(cbuf) >= GET_LEVEL(ch)) {
	free_char(cbuf);
	send_to_char("Sorry, you can't do that.\r\n", ch);
	return;
      }
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }
  if (GET_LEVEL(ch) < LVL_IMPL) {
    if (!IS_NPC(vict) && GET_MAX_LEVEL(ch) <= GET_MAX_LEVEL(vict) && vict != ch) {
      send_to_char("Maybe that's not such a great idea...\r\n", ch);
      return;
    }
  }
  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_MAX_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  if (IS_NPC(vict) && !(fields[l].pcnpc & NPC)) {
    send_to_char("You can't do that to a beast!\r\n", ch);
    return;
  } else if (!IS_NPC(vict) && !(fields[l].pcnpc & PC)) {
    send_to_char("That can only be done to a beast!\r\n", ch);
    return;
  }
  if (fields[l].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char("Value must be on or off.\r\n", ch);
      return;
    }
  } else if (fields[l].type == NUMBER) {
    value = atoi(val_arg);
  }

  strcpy(buf, "Okay.");  /* can't use OK macro here 'cause of \r\n */
  switch (l) {
  case 0:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
    break;
  case 1:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
    break;
  case 2:
    set_title(vict, val_arg);
    sprintf(buf, "%s's title is now: %s", GET_NAME(vict), GET_TITLE(vict));
    break;
  case 3:
    vict->points.max_hit = RANGE(1, 30000);
    affect_total(vict);
    break;
  case 4:
    vict->points.max_mana = RANGE(1, 20000);
    affect_total(vict);
    break;
  case 5:
    vict->points.max_move = RANGE(1, 20000);
    affect_total(vict);
    break;
  case 6:
    vict->points.hit = RANGE(-9, vict->points.max_hit);
    affect_total(vict);
    break;
  case 7:
    vict->points.mana = RANGE(0, vict->points.max_mana);
    affect_total(vict);
    break;
  case 8:
    vict->points.move = RANGE(0, vict->points.max_move);
    affect_total(vict);
    break;
  case 9:
    GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
    affect_total(vict);
    break;
  case 10:
    if (!str_cmp(val_arg, "good"))
      GET_PERMALIGN(vict) = ALIGN_GOOD;
    else if (!str_cmp(val_arg, "evil"))
      GET_PERMALIGN(vict) = ALIGN_EVIL;
    else if (!str_cmp(val_arg, "neutral"))
      GET_PERMALIGN(vict) = ALIGN_NEUTRAL;
    else if (!str_cmp(val_arg, "lawful"))
      GET_PERMALIGN(vict) = ALIGN_LAWFUL;
    else if (!str_cmp(val_arg, "chaotic"))
      GET_PERMALIGN(vict) = ALIGN_CHAOTIC;
    else {
      send_to_char("Must be 'good', 'evil', 'neutral', 'lawful' or 'chaotic'.\r\n", ch);
      return;
    }
    break;
  case 11:
    RANGE(3, 25);
    vict->real_abils.str = value;
    vict->real_abils.str_add = 0;
    affect_total(vict);
    break;
  case 12:
    vict->real_abils.str_add = RANGE(0, 100);
    if (value > 0)
      vict->real_abils.str = 18;
    affect_total(vict);
    break;
  case 13:
    RANGE(3, 25);
    vict->real_abils.intel = value;
    affect_total(vict);
    break;
  case 14:
    RANGE(3, 25);
    vict->real_abils.wis = value;
    affect_total(vict);
    break;
  case 15:
    RANGE(3, 25);
    vict->real_abils.dex = value;
    affect_total(vict);
    break;
  case 16:
    RANGE(3, 25);
    vict->real_abils.con = value;
    affect_total(vict);
    break;
  case 17:
    RANGE(3, 25);
    vict->real_abils.cha = value;
    affect_total(vict);
    break;
  case 18:
    RANGE(3, 25);
    vict->real_abils.will = value;
    affect_total(vict);
    break;
  case 19:
    if (!str_cmp(val_arg, "male"))
      vict->player.sex = SEX_MALE;
    else if (!str_cmp(val_arg, "female"))
      vict->player.sex = SEX_FEMALE;
    else if (!str_cmp(val_arg, "neutral"))
      vict->player.sex = SEX_NEUTRAL;
    else {
      send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
      return;
    }
    break;
  case 20:
    GET_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 21:
    GET_BANK_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 22:
    vict->points.exp = RANGE(0, 500000000);
    break;
  case 23:
    vict->points.hitroll = RANGE(0, 99);
    affect_total(vict);
    break;
  case 24:
    vict->points.damroll = RANGE(0, 99);
    affect_total(vict);
    break;
  case 25:
    if (GET_MAX_LEVEL(ch) < LVL_SUP && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return;
    }
    GET_INVIS_LEV(vict) = RANGE(0, MIN((int)GET_LEVEL(vict), (int)GET_LEVEL(ch)));
    break;
  case 26:
    if (GET_MAX_LEVEL(ch) < LVL_GRGOD && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return;
    }
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
    break;
  case 27:
    if (ch == vict) {
      send_to_char("Better not -- could be a long winter!\r\n", ch);
      return;
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
    break;
  case 28:
  case 29:
    GET_PRACTICES(vict) = RANGE(0, 255);
    break;
  case 30:
  case 31:
  case 32:
  case 33:
    if (!str_cmp(val_arg, "off")) {
      GET_COND(vict, (l - 30)) = (char) -1;
      sprintf(buf, "%s's %s now off.", GET_NAME(vict), fields[l].cmd);
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      RANGE(0, 24);
      GET_COND(vict, (l - 30)) = (char) value;
      sprintf(buf, "%s's %s set to %d.", GET_NAME(vict), fields[l].cmd,
	      (int) value);
    } else {
      send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
      return;
    }
    break;
  case 34:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER);
    break;
  case 35:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
    break;
  case 36:
    if (value > GET_MAX_LEVEL(ch) || value > LVL_IMPL) {
      send_to_char("You can't do that.\r\n", ch);
      return;
    }
    RANGE(0, LVL_IMPL);
    GET_LEVEL(vict) = (byte) value;
    break;
  case 37:
    if ((i = real_room(value)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return;
    }
    char_from_room(vict);
    char_to_room(vict, i);
    break;
  case 38:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
    break;
  case 39:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
    break;
  case 40:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
    break;
  case 41: /* set olc */
    GET_OLC_ZONE(vict) = value;
    break;
  case 42:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
    break;
  case 43:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
    break;
  case 44:
    if (!str_cmp(val_arg, "off"))
      REMOVE_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
    else if (is_number(val_arg)) {
      value = atol(val_arg);
      if (real_room(value) != NOWHERE) {
	SET_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
	GET_LOADROOM(vict) = value;
	sprintf(buf, "%s will enter at room #%ld.", GET_NAME(vict),
		GET_LOADROOM(vict));
      } else {
	sprintf(buf, "That room does not exist!");
      }
    } else {
      strcpy(buf, "Must be 'off' or a room's virtual number.\r\n");
    }
    break;
  case 45:
    SET_OR_REMOVE(PRF_FLAGS(vict), (PRF_COLOR_1 | PRF_COLOR_2));
    break;
  case 46:
    if (GET_IDNUM(ch) != 1 || !IS_NPC(vict))
      return;
    GET_IDNUM(vict) = value;
    break;
  case 47:
    if (!is_file)
      return;
    if (GET_IDNUM(ch) > 1) {
      send_to_char("Please don't use this command, yet.\r\n", ch);
      return;
    }
    if (GET_LEVEL(vict) >= LVL_GRGOD) {
      send_to_char("You cannot change that.\r\n", ch);
      return;
    }
    strncpy(tmp_store.pwd, CRYPT(val_arg, tmp_store.name), MAX_PWD_LENGTH);
    tmp_store.pwd[MAX_PWD_LENGTH] = '\0';
    sprintf(buf, "Password changed to '%s'.", val_arg);
    break;
  case 48:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
    break;
  case 49:
    if ((i = parse_race(*val_arg)) == UNDEFINED_RACE) {
      send_to_char("That is not a race.\r\n", ch);
      return;
    }
    GET_RACE(vict) = i;
    break;
  case 50:
    GET_REL(vict) = RANGE(0, NUM_RELIGIONS-1);
    break;
  case 51:
    if (value < 0 || value >= num_of_guilds) {
      send_to_char("That is not a valid guild number.\r\n", ch);
      strcpy(buf, "Valid guilds are:\r\n");
      for (i = 0, gptr = guild_info; gptr && i < num_of_guilds; gptr = gptr->next, i++) {
	sprintf(buf, "%s%d) %s%s", buf, i, gptr->name, (i+1) % 2 ? "\r\n" : "\t");
      }
      send_to_char(buf, ch);
      return;
    }
    GET_GUILD(vict) = RANGE(0, num_of_guilds-1);
    break;
  case 52:
    GET_GUILD_LEV(vict) = RANGE(0, 5);
    break;
  case 53:
    GET_HEIGHT(vict) = RANGE(1, 5000);
    break;
  case 54:
    GET_WEIGHT(vict) = RANGE(1, 25000);
    break;
  default:
    sprintf(buf, "Can't set that!");
    break;
  }

  if (fields[l].type == BINARY) {
    sprintf(buf, "%s %s for %s.\r\n", fields[l].cmd, ONOFF(on),
	    GET_NAME(vict));
    CAP(buf);
  } else if (fields[l].type == NUMBER) {
    sprintf(buf, "%s's %s set to %d.\r\n", GET_NAME(vict),
	    fields[l].cmd, (int) value);
  } else
    strcat(buf, "\r\n");
  send_to_char(CAP(buf), ch);

  if (!is_file && !IS_NPC(vict))
    save_char(vict, NOWHERE);

  if (is_file) {
    char_to_store(vict, &tmp_store);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free_char(cbuf);
    send_to_char("Saved in file.\r\n", ch);
  }
}


static char *logtypes[] = {
"off", "brief", "normal", "complete", "\n"};

ACMD(do_syslog)
{
  int tp;

  one_argument(argument, arg);

  if (!*arg) {
    tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
	  (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
    sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1 | PRF_LOG2);
  SET_BIT(PRF_FLAGS(ch), (PRF_LOG1 * (tp & 1)) | (PRF_LOG2 * (tp & 2) >> 1));

  sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
  send_to_char(buf, ch);
}


ACMD(do_olist)
{
   extern struct index_data *obj_index;
   extern struct obj_data *obj_proto;

   int  nr, start, end = 0, i, found = 0;
   bool f = FALSE;

   one_argument(argument, arg);

   start = (100 * atoi(arg));
   for (i = 0; i <= top_of_zone_table; i++)
     if (zone_table[i].number == atoi(arg)) {
       end = zone_table[i].top;
       break;
     }
   if (end == 0)
     end = start + 99;

   sprintf(buf, "Objects in Zone %4d\r\n"
		"~~~~~~~~~~~~~~~~~~~~\r\n", (start/100));

   for (nr = 0; nr <= top_of_objt; nr++) {
     if ((obj_index[nr].vnumber >= start) && (obj_index[nr].vnumber <= end))
     {
       f = TRUE;
       sprintf(buf, "%s%3d. [%ld] %s\r\n", buf, ++found,
	       obj_index[nr].vnumber,
	       obj_proto[nr].short_description);
     }
   }
   if (f == FALSE)
     send_to_char("Sorry, there are no objs in that zone.", ch);
   else {
     sprintf(buf, "%s** %d Objects found in Zone %d\r\n", buf, found, (start/100));
     page_string(ch->desc, buf, 0);
   }
}


ACMD(do_rlist)
{
   int i, ii, k;
   bool found = FALSE;

   one_argument(argument, arg);

   for (i = 0; i <= top_of_zone_table; i++)
     if (zone_table[i].number == atoi(arg))
       break;

   k = i;

   sprintf(buf, "Rooms in Zone %4d\r\n"
                "~~~~~~~~~~~~~~~~~~\r\n", (atoi(arg)));

   for (ii = 0; ii <= top_of_world; ii++)
     if (world[ii].zone == k) {
       found = TRUE;
       sprintf(buf, "%s[%ld] %s\r\n", buf, world[ii].number, world[ii].name);
     }

   if (found == FALSE)
     send_to_char("The desired zone does not exist.\r\n", ch);
   else
     page_string(ch->desc, buf, 0);
}


ACMD (do_mlist)
{
   extern struct index_data *mob_index;   /* index table for mobile file   */
   extern struct char_data *mob_proto;    /* prototypes for mobs           */

   int  nr, start, end = 0, found = 0, i;
   bool f = FALSE;

   one_argument(argument, arg);

   start = (100 * atoi(arg));
   for (i = 0; i <= top_of_zone_table; i++)
     if (zone_table[i].number == atoi(arg)) {
       end = zone_table[i].top;
       break;
     }
   if (end == 0)
     end = start + 99;

   sprintf(buf, "Mobiles in Zone %4d\r\n"
		"~~~~~~~~~~~~~~~~~~~~\r\n", (start/100));
   for (nr = 0; nr <= top_of_mobt; nr++) {
     if ((mob_index[nr].vnumber >= start) && (mob_index[nr].vnumber <= end)) {
       f = TRUE;
       sprintf(buf, "%s%3d. [%ld] %s\r\n", buf, ++found,
	       mob_index[nr].vnumber,
	       mob_proto[nr].player.short_descr);
     }
   }
   if (f == FALSE)
     send_to_char("Sorry, there are no mobs in that zone.", ch);
   else {
     sprintf(buf, "%s** %d Mobiles found in Zone %d\r\n", buf, found, (start/100));
     page_string(ch->desc, buf, 0);
   }
}

/* HOT REBOOT */
ACMD(do_copyover)
{
  FILE *fp;
  struct descriptor_data *d, *d_next;
  char buf [100], buf2[100];
  extern int mini_mud;
	
  if ( !(fp = fopen(COPYOVER_FILE, "w")) ) {
    send_to_char ("Hot reboot file not writeable, aborted.\n\r",ch);
    return;
  }
  
  /* 
   * Uncomment if you use OasisOLC2.0, this saves all OLC modules:
   save_all();
   *
   */
  sprintf(buf, "\n\r *** HOT REBOOT by %s - please remain seated! ***\n\r", GET_NAME(ch));
  
  /* For each playing descriptor, save its state */
  for (d = descriptor_list; d != NULL; d = d_next) {
    struct char_data * och = (d->original ? d->original : d->character);
    /* We delete from the list , so need to save this */
    d_next = d->next;
    
    /* drop those logging on */
    if (!d->character || d->connected > CON_PLAYING) {
      write_to_descriptor(d->descriptor, 
                          "\n\rSorry, we are rebooting. " 
                          "Come back in a few minutes.\n\r");
      close_socket(d); /* throw'em out */
    } else {
      fprintf (fp, "%d %s %s\n", d->descriptor, GET_NAME(och),
               d->host);
      write_to_descriptor( d->descriptor, buf );
      /* save och */
      Crash_rentsave(och,0);
      save_char(och, och->in_room);
    }
  }

  fprintf(fp, "-1\n");
  fclose(fp);

  /* Close reserve and other always-open files and release other resources */
  fclose(player_fl);
  
  /* exec - descriptors are inherited */
  sprintf(buf, "%d", port);
  sprintf(buf2, "-C%d", mother_desc);
  if ( mini_mud )
    sprintf( buf2, "%s -m", buf2 );
  
  /* Ugh, seems it is expected we are 1 step above lib - this may be dangerous! */
  chdir("..");
  
  execl(EXE_FILE, "dominion", buf2, buf, (char *)NULL);
  
  /* Failed - sucessful exec will not return */
  perror("do_copyover: execl");
  send_to_char("Hot Reboot FAILED!\n\r",ch);
  
  exit(1); /* too much trouble to try to recover! */
}
