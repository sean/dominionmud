/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ACT_INFO.C                                   Based on CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#define __ACT_INFO_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
/* New for TD 5/24/95 - Orig 3/16/95 */
extern char *pc_race_types[];
extern char *pc_religion_types[];
extern struct material_type const material_list[];

extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
extern char *newbm;
extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *dirs[];
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern char *connected_types[];
extern char *room_bits[];
extern char *spells[];
/* TD 5/24/95 */
extern char *race_abbrev[];
long find_race_bitvector(char arg);
int  exp_needed(int level);
char *show_obj_condition(struct obj_data *obj);
ACMD(do_infobar);
ACMD(do_action);

#define IS_INVIS(o) IS_OBJ_STAT(o, ITEM_INVISIBLE)

void show_obj_to_char(struct obj_data * object, struct char_data * ch, int mode)
{
  bool found;

  *buf = '\0';
  if ((mode == 0) && object->description)
    str_cpy(buf, object->description);
  else if (object->short_description && ((mode == 1) || (mode == 2) ||
	    (mode == 3) || (mode == 4))) {
      if (mode == 1)
	 sprintf(buf, "%-30.30s", object->short_description);
      else
	 sprintf(buf, "%-40.40s", object->short_description);
      /* get the condition of the object */
      sprintf(buf, "%s\t%s", buf, show_obj_condition(object));
  } else if (mode == 5) {
    if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
      if (object->action_description) {
	str_cpy(buf, "There is something written upon it:\r\n\r\n");
	strcat(buf, object->action_description);
	page_string(ch->desc, buf, 1);
      } else
	act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
      return;
#if 0
    } else if (GET_OBJ_TYPE(object) != ITEM_DRINKCON) {
      str_cpy(buf, "You see nothing special..");
    } else                      /* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
      str_cpy(buf, "It looks like a drink container.");
#endif
    }
  }
  if ((mode != 3) && (mode != 1)) {
    found = FALSE;
    if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
      strcat(buf, " (invisible)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_BLESS) && IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
      strcat(buf, " ..It glows blue!");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
      strcat(buf, " ..It glows yellow!");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_GLOW)) {
      strcat(buf, " ..It has a soft glowing aura!");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_HUM)) {
      strcat(buf, " ..It emits a faint humming sound!");
      found = TRUE;
    }
  }
  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
}

/* new for TD */
char *show_obj_condition(struct obj_data *obj)
{
  int  condition = 100, tenth = 10;
  char *conditions[] = {
    " [perfect]",
    " [slightly used]",
    " [moderately used]",
    " [slightly damaged]",
    " [moderately damaged]",
    " [severely damaged]",
    " [broken]",
    " [destroyed]",            /* MAX for reg items 7 */
    " [fresh]",
    " [good]",
    " [spoiled]",              /* MAX for food 10 */
    " [bright]",
    " [dim]",
    " [guttering]",
    " [burnt out]"             /* MAX for lights 14 */
  };

  condition = GET_OBJ_VAL(obj, 9);
  tenth = (material_list[GET_OBJ_VAL(obj, 7)].sturdiness / 10);
 
  if (GET_OBJ_TYPE(obj) == ITEM_FOOD) {
     if (condition <= 48)
       return (conditions[8]);
     else if (condition <= 72)
       return (conditions[9]);
     else
       return (conditions[10]);
  } else if (GET_OBJ_TYPE(obj) == ITEM_LIGHT) {
     condition = GET_OBJ_VAL(obj, 2);
     if (condition == -1 || condition >= 12)
       return (conditions[11]);
     else if (condition >= 4)
       return (conditions[12]);
     else if (condition >= 1)
       return (conditions[13]);
     else
       return (conditions[14]);
  } else {
     if (material_list[GET_OBJ_VAL(obj, 7)].sturdiness == -1)
	return (conditions[0]);
     else if (condition < (tenth * 1))
	return (conditions[0]);
     else if (condition < (tenth * 2))
	return (conditions[1]);
     else if (condition < (tenth * 4))
	return (conditions[2]);
     else if (condition < (tenth * 6))
	return (conditions[3]);
     else if (condition < (tenth * 7))
	return (conditions[4]);
     else if (condition < (tenth * 8))
	return (conditions[5]);
     else if (condition < (tenth * 9))
	return (conditions[6]);
     else
	return (conditions[7]);
  }
}



void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode, bool show)
{
  struct obj_data *i, *vis_obj = NULL;
  bool found;
  int  num = 1;

  found = FALSE;
  /* New for item combine code - TD 5/24/95 */
  for (i = list; i; i = i->next_content) {
    while (1) {
      if (!i->next_content)
	break;
      if (i->item_number != i->next_content->item_number || \
	  strcmp(i->description, i->next_content->description) || \
	  IS_INVIS(i) != IS_INVIS(i->next_content))
	break;
      if (CAN_SEE_OBJ(ch, i)) {
        if (GET_OBJ_TYPE(i) != ITEM_TRAP && !IS_OBJ_STAT(i, ITEM_NOSHOW)) {
	  num++;
	  vis_obj = i;
        }
      }
      i = i->next_content;
    }
    if (num > 1) {
      sprintf(buf, "[%d] ", num);
      send_to_char(buf, ch);
      show_obj_to_char(vis_obj, ch, mode);
      num = 1;
      found = TRUE;
    } else {
      if (CAN_SEE_OBJ(ch, i)) {
        if (GET_OBJ_TYPE(i) != ITEM_TRAP && !IS_OBJ_STAT(i, ITEM_NOSHOW)) {
  	  show_obj_to_char(i, ch, mode);
	  found = TRUE;
        }
      }
    }
  }
  if (!found && show)
    send_to_char(" Nothing.\r\n", ch);
}


void diag_char_to_char(struct char_data * i, struct char_data * ch)
{
  int  percent;

  if (GET_MAX_HIT(i) > 0)
    percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  else
    percent = -1;               /* How could MAX_HIT be < 1?? */

  str_cpy(buf, (i != ch ? HSSH(i) : "You"));
  CAP(buf);

  if (percent >= 100)
    sprintf(buf, "%s %s to be in excellent condition.\r\n", buf,
	    (i == ch ? "appear" : "appears"));
  else if (percent >= 90)
    sprintf(buf, "%s %s a few scratches.\r\n", buf,
	    (i != ch ? "has" : "have"));
  else if (percent >= 75)
    sprintf(buf, "%s %s some small wounds and bruises.\r\n", buf,
	    (i != ch ? "has" : "have"));
  else if (percent >= 50)
    sprintf(buf, "%s %s quite a few wounds.\r\n", buf,
	    (i != ch ? "has" : "have"));
  else if (percent >= 30)
    sprintf(buf, "%s %s some big nasty wounds and scratches.\r\n", buf,
	    (i != ch ? "has" : "have"));
  else if (percent >= 15)
    sprintf(buf, "%s %s pretty hurt.\r\n", buf,
	    (i != ch ? "looks" : "look"));
  else if (percent >= 0)
    sprintf(buf, "%s %s in awful condition.\r\n", buf,
	    (i != ch ? "is" : "are"));
  else
    sprintf(buf, "%s %s bleeding awfully from big wounds.\r\n", buf,
	    (i != ch ? "is" : "are"));
  send_to_char(buf, ch);

  /* Now show wounds */
  if (GET_WOUNDS(i)) {
    str_cpy(buf, (i != ch ? HSSH(i) : "You"));
    CAP(buf);
    sprintf(buf, "%s %s %s.\r\n", buf, (i==ch ? "have" :"has"),
	    (GET_WOUNDS(i) <= 3 ? "some small wounds" :
	     (GET_WOUNDS(i)  <= 7 ? "several wounds" :
	      (GET_WOUNDS(i)  <= 10 ? "several large wounds" :
	       (GET_WOUNDS(i)  <= 14 ? "many wounds" :
		(GET_WOUNDS(i)  <= 18 ? "many gaping wounds" :
		 (GET_WOUNDS(i)  <= 24 ? "large bloody wounds" :
		  "quite a few large gaping wounds")))))));
    send_to_char(buf, ch);
  }
}


void look_at_char(struct char_data * i, struct char_data * ch)
{
  int    j, found;
  struct obj_data *tmp_obj;
  struct guild_type *gptr = NULL;
  char   look_race_buf[128], temp_buf[100];
  char   *heshebuf = NULL;
  extern struct guild_type *guild_info;
  extern char * locations[];

  if (i->player.description)
    send_to_char(i->player.description, ch);
  else
    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

  if (!IS_NPC(i) || !RACE_UNDEF(i)) {
    sprintf(temp_buf, "%s %s", (i != ch ? HSSH(i) : "you"),
	    (i != ch ? "is" : "are"));
    if (temp_buf[0] >= 'a' && temp_buf[0] <= 'z')
      temp_buf[0] = ((temp_buf[0] - 'a') + 'A');
    heshebuf = str_dup(temp_buf);

    /* TD 5/24/95 */
    sprintf(look_race_buf, "%s ", temp_buf);
    if (!IS_IMMORT(i)) {
      sprinttype(i->player.race, pc_race_types, temp_buf);
      sprintf(look_race_buf, "%s%s %s", look_race_buf, AN(temp_buf), temp_buf);
    } else sprintf(look_race_buf, "%san Immortal", look_race_buf);
    strcat(look_race_buf, ".\r\n");
    send_to_char(look_race_buf, ch);
  }
  /* now for height & weight */
  if (!IS_NPC(i) || (GET_HEIGHT(i) != 198 && GET_WEIGHT(i) != 200)) {
  sprintf(look_race_buf, "%s %s %d'%d\" tall and weigh%s approximately %d pound%s.\r\n", 
          (i != ch ? HSSH(i) : "You"), (i != ch ? "is" : "are"),
          (GET_HEIGHT(i)/12), (GET_HEIGHT(i)%12), (i != ch ? "s" : ""),
	  GET_WEIGHT(i), (GET_WEIGHT(i) > 1 ? "s" : ""));
  /* Capitalize the 1st letter */
  if (look_race_buf[0] >= 'a' && look_race_buf[0] <= 'z')
    look_race_buf[0] = ((look_race_buf[0] - 'a') + 'A');
  send_to_char(look_race_buf, ch);
  }
  if (GET_GUILD(i) != GUILD_NONE && GET_GUILD(i) != GUILD_UNDEFINED) {
    for (gptr = guild_info; gptr && gptr->number != GET_GUILD(i); gptr=gptr->next)
       ;
    if (gptr && gptr->member_look_str != NULL)
      act(gptr->member_look_str, TRUE, i, 0, ch, TO_VICT);
  }

  diag_char_to_char(i, ch);

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;

  sprintf(temp_buf, "%s %s", (i != ch ? HSSH(i) : "you"),
	    (i != ch ? "is" : "are"));
  if (temp_buf[0] >= 'a' && temp_buf[0] <= 'z')
    temp_buf[0] = ((temp_buf[0] - 'a') + 'A');
  free(heshebuf);
  heshebuf = str_dup(temp_buf);
  if (found) {
    sprintf(buf, "\r\n%s using:", heshebuf);
    act(buf, FALSE, i, 0, ch, TO_VICT);
    for (j = 0; j < NUM_WEARS; j++)
      if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) {
	send_to_char(CCCYN(ch, C_SPR), ch);
	if (j != 17  && j != 18)
	  send_to_char(where[j], ch);
	else {
	  if (j == 17)  /* wielded */
	    sprintf(buf, "<Wielded in %-5.5s hand>  ", (GET_HANDEDNESS(ch) ? "right" : "left"));
	  else /* held */
	    sprintf(buf, "<Held in %-5.5s hand>     ", (GET_HANDEDNESS(ch) ? "left" : "right"));
	  send_to_char(buf, ch);
	}
	send_to_char(CCNRM(ch, C_SPR), ch);
	show_obj_to_char(GET_EQ(i, j), ch, 1);
      }
  }
  send_to_char("\r\n", ch);
  /* Items stuck in the player */
  for (j = 0; j < 12; j++) {
    if (GET_ITEMS_STUCK(i, j) != NULL) {
      sprintf(buf, "$p is lodged in $s %s.", locations[j]);
      act(buf, FALSE, i, GET_ITEMS_STUCK(i, j), ch, TO_VICT);
    }
  }
  if (ch != i && (GET_SKILL(ch, SKILL_PEEK) >= 1 || GET_LEVEL(ch) >= LVL_IMMORT)) {
    found = FALSE;
    act("\r\nYou attempt to peek at $s inventory:", FALSE, i, 0, ch, TO_VICT);
    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 101) < GET_SKILL(ch, SKILL_PEEK))) {
	show_obj_to_char(tmp_obj, ch, 1);
	found = TRUE;
      }
    }

    if (!found)
      send_to_char("You can't see anything.\r\n", ch);
  }
}


void list_one_char(struct char_data * i, struct char_data * ch)
{
  char *positions[] = {
    " is lying here, dead.",
    " is lying here, mortally wounded.",
    " is lying here, incapacitated.",
    " is lying here, stunned.",
    " is sleeping here.",
    " is resting here.",
    " is sitting here.",
    "!FIGHTING!",
    " is standing here.",
    " is mounted on the back of ",
    " is here hovering just off the ground.",
    " is here, contemplating the universe."
  };
  /* Send the correct color to the player */
  sprintf(buf, "%s", !IS_NPC(i) ? CCYEL(ch, C_NRM) : CCGLD(ch, C_NRM));
  send_to_char(buf, ch);

  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i)) {
    str_cpy(buf, CCGLD(ch, C_NRM));
    if (IS_AFFECTED(i, AFF_INVISIBLE))
      strcat(buf, "*");

    if ((IS_AFFECTED(ch, AFF_DETECT_ALIGN)) || \
       (IS_ITEM_AFF(ch, AFFECTED_DETECT_ALIGN))) {
      if (IS_EVIL(i))
	strcat(buf, "(Red Aura) ");
      else if (IS_GOOD(i))
	strcat(buf, "(Blue Aura) ");
      else
	strcat(buf, "(Yellow Aura) ");
    }
    strcat(buf, i->player.long_descr);
    send_to_char(buf, ch);

    if ((IS_AFFECTED(i, AFF_SANCTUARY)) || \
	IS_ITEM_AFF(i, AFFECTED_SANCTUARY))
      act("...$e glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED2(i, AFF_BLIND))
      act("...$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED2(i, AFF_BLACKMANTLE))
      act("...$e is surrounded by a black aura!", FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED2(i, AFF_DISEASE))
      act("...$e is covered with puss filled sores!", FALSE, i, 0, ch, TO_VICT);
    return;
  }

  if (IS_NPC(i)) {
    str_cpy(buf, i->player.short_descr);
    CAP(buf);
  } else
    if (IS_AFFECTED2(i, AFF_FIREWALL))
      sprintf(buf, "Someone encased in a column of flame");
    else
      sprintf(buf, "%s", i->player.name);

  if ((GET_POS(i) != POS_FIGHTING) && 
       !(IS_AFFECTED2(i, AFF_FLYING) && GET_POS(i) == POS_STANDING)) {
    if (GET_POS(i) >= 0 && GET_POS(i) <= 11)
      strcat(buf, positions[(int) GET_POS(i)]);
    if (MOUNTED(i) != NULL) {
      strcat(buf, GET_NAME(MOUNTED(i)));
      strcat(buf, ".");
    }
  } else {
    if (IS_AFFECTED2(i, AFF_FLYING) && GET_POS(i) == POS_STANDING) {
       strcat(buf, " is here floating about.");
    } else {
       if (FIGHTING(i)) {
	 strcat(buf, " is here, fighting ");
	 if (FIGHTING(i) == ch)
	   strcat(buf, "YOU!");
	 else {
	   if (i->in_room == FIGHTING(i)->in_room)
	     strcat(buf, PERS(FIGHTING(i), ch));
	   else
	     strcat(buf, "someone who has already left");
	   strcat(buf, "!");
	 }
       } else                   /* NIL fighting pointer */
	 strcat(buf, " is here struggling with thin air.");
    }
  }

  if ((IS_AFFECTED(i, AFF_INVISIBLE)) || \
      IS_ITEM_AFF(i, AFFECTED_INVIS))
    strcat(buf, " (invisible)");
  if (IS_AFFECTED(i, AFF_HIDE))
    strcat(buf, " (hidden)");
  if (!IS_NPC(i) && !i->desc)
    strcat(buf, " (linkless)");
  if (PLR_FLAGGED(i, PLR_WRITING))
    strcat(buf, " (writing)");

  if ((IS_AFFECTED(ch, AFF_DETECT_ALIGN)) || \
      IS_ITEM_AFF(ch, AFFECTED_DETECT_ALIGN)) {
    if (IS_EVIL(i))
      strcat(buf, " (Red Aura)");
    else if (IS_GOOD(i))
      strcat(buf, " (Blue Aura)");
    else
      strcat(buf, " (Yellow Aura)");
  }
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  if ((IS_AFFECTED(i, AFF_SANCTUARY)) || \
      IS_ITEM_AFF(i, AFFECTED_SANCTUARY))
    act("...$e glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
  if (IS_AFFECTED2(i, AFF_BLACKMANTLE))
    act("...$e is surrounded by a black aura!", FALSE, i, 0, ch, TO_VICT);
  if (IS_AFFECTED2(i, AFF_DISEASE))
    act("...$e is covered with puss filled sores!", FALSE, i, 0, ch, TO_VICT);

  send_to_char(CCNRM(ch, C_NRM), ch);
}


void list_char_to_char(struct char_data * list, struct char_data * ch)
{
  struct char_data *i;
  int    j = 1;

  for (i = list; i; i = i->next_in_room)
    /* skip yourself and ridden animals */
    if (ch != i && !RIDDEN(i)) {
      if (CAN_SEE(ch, i) && (!SHADOWING(i) || GET_LEVEL(ch) >= LVL_IMMORT)) {
	 if (i->next_in_room && !strcmp(GET_NAME(i), GET_NAME(i->next_in_room)))
	    while (i->next_in_room != NULL && !strcmp(GET_NAME(i), GET_NAME(i->next_in_room))) {
	      j++;
	      i = i->next_in_room;
	    }
	 if (j > 1) {
	   sprintf(buf, "%s[%d] ", !IS_NPC(i) ? CCYEL(ch, C_NRM) : CCGLD(ch, C_NRM), j);
	   send_to_char(buf, ch);
	   j = 1;
	 }
	 list_one_char(i, ch);
      } else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) &&
		 IS_AFFECTED(i, AFF_INFRAVISION))
	send_to_char("You see a pair of glowing red eyes looking your way.\r\n", ch);
      else if (SHADOWING(i) && (GET_LEVEL(i) < LVL_IMMORT) &&
	       ((world[ch->in_room].sector_type == SECT_INSIDE) || \
	       (world[ch->in_room].sector_type == SECT_CITY)    || \
	       (world[ch->in_room].sector_type == SECT_FOREST)))
	send_to_char("You think you see someone slink back into the shadows.\r\n", ch);
    }
}


void do_auto_exits(struct char_data * ch)
{
  int door;
  char *dirname[] = {
     "N", "E", "S", "W", "U", "D", "NE", "NW", "SE", "SW"};
  *buf = '\0';

  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_SECRET))
      sprintf(buf, "%s%s ", buf, dirname[door]);

  sprintf(buf2, "%s[ Exits: %s]%s\r\n", CCCYN(ch, C_NRM),
	  *buf ? buf : "None! ", CCNRM(ch, C_NRM));

  send_to_char(buf2, ch);
}


ACMD(do_exits)
{
  int door;

  *buf = '\0';

  if (IS_AFFECTED2(ch, AFF_BLIND)) {
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
    return;
  }
  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      if (GET_LEVEL(ch) >= LVL_IMMORT)
	sprintf(buf2, "%-5s - [%ld] %s\r\n", dirs[door],
		world[EXIT(ch, door)->to_room].number,
		world[EXIT(ch, door)->to_room].name);
      else {
	sprintf(buf2, "%-5s - ", dirs[door]);
	if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
	  strcat(buf2, "Too dark to tell\r\n");
	else {
	  strcat(buf2, world[EXIT(ch, door)->to_room].name);
	  strcat(buf2, "\r\n");
	}
      }
      strcat(buf, CAP(buf2));
    }
  send_to_char("Obvious exits:\r\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char(" None.\r\n", ch);
}



void look_at_room(struct char_data * ch, int ignore_brief)
{
  /* BAD FIX: If ch is nowhere, move them to temple square */
  if (ch->in_room == NOWHERE)
    ch->in_room = real_room(TEMPLE_SQUARE_VNUM);

  if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    return;
  } else if (IS_AFFECTED2(ch, AFF_BLIND)) {
    send_to_char("You see nothing but infinite darkness...\r\n", ch);
    return;
  }
  send_to_char(CCCYN(ch, C_NRM), ch);
  if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
    sprintbit((long) ROOM_FLAGS(ch->in_room), room_bits, buf);
    sprintf(buf2, "[%ld] %s [ %s]", world[ch->in_room].number,
	    world[ch->in_room].name, buf);
    send_to_char(buf2, ch);
  } else
    send_to_char(world[ch->in_room].name, ch);

  send_to_char(CCNRM(ch, C_NRM), ch);
  send_to_char("\r\n", ch);

  if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief ||
      ROOM_FLAGGED(ch->in_room, ROOM_DEATH))
    send_to_char(world[ch->in_room].description, ch);

  /* autoexits */
  if (PRF_FLAGGED(ch, PRF_AUTOEXIT) && !PRF_FLAGGED(ch, PRF_INFOBAR))
    do_auto_exits(ch);

  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), ch);
  list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);
  send_to_char(CCYEL(ch, C_NRM), ch);
  list_char_to_char(world[ch->in_room].people, ch);
  send_to_char(CCNRM(ch, C_NRM), ch);
}



void look_in_direction(struct char_data * ch, int dir)
{
  if (EXIT(ch, dir)) {
    if (EXIT(ch, dir)->general_description)
      send_to_char(EXIT(ch, dir)->general_description, ch);
    else
      send_to_char("You see nothing special.\r\n", ch);

    if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED) && EXIT(ch, dir)->keyword
	&& !IS_SET(EXIT(ch, dir)->exit_info, EX_SECRET)) {
      sprintf(buf, "The %s is closed.\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf, ch);
    } else if (IS_SET(EXIT(ch, dir)->exit_info, EX_ISDOOR) && EXIT(ch, dir)->keyword
	&& !IS_SET(EXIT(ch, dir)->exit_info, EX_SECRET)) {
      sprintf(buf, "The %s is open.\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf, ch);
    }
  } else
    send_to_char("Nothing special there...\r\n", ch);
}



void look_in_obj(struct char_data * ch, char *arg)
{
  struct obj_data *obj = NULL;
  struct char_data *dummy = NULL;
  int amt, bits;

  if (!*arg)
    send_to_char("Look in what?\r\n", ch);
  else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
    sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	     (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
	     (GET_OBJ_TYPE(obj) != ITEM_CONTAINER)) {
     if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &dummy, &obj)))
	send_to_char("There's nothing inside that!\r\n", ch);
     else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	     (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
	     (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
	send_to_char("There's nothing inside that!\r\n", ch);
     else if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
	if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
	  send_to_char("It is closed.\r\n", ch);
	else {
	  send_to_char(fname(obj->name), ch);
	  switch (bits) {
	  case FIND_OBJ_INV:
	    send_to_char(" (carried): \r\n", ch);
	    break;
	  case FIND_OBJ_ROOM:
	    send_to_char(" (here): \r\n", ch);
	    break;
	 }
	 list_obj_to_char(obj->contains, ch, 2, TRUE);
       }
     }
  } else {
    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
      if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
	send_to_char("It is closed.\r\n", ch);
      else {
	send_to_char(fname(obj->name), ch);
	switch (bits) {
	case FIND_OBJ_INV:
	  send_to_char(" (carried): \r\n", ch);
	  break;
	case FIND_OBJ_ROOM:
	  send_to_char(" (here): \r\n", ch);
	  break;
	case FIND_OBJ_EQUIP:
	  send_to_char(" (used): \r\n", ch);
	  break;
	}

	list_obj_to_char(obj->contains, ch, 2, TRUE);
      }
    } else {            /* item must be a fountain or drink container */
      if (GET_OBJ_VAL(obj, 1) <= 0)
	send_to_char("It is empty.\r\n", ch);
      else {
	amt = ((GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0));
	sprintf(buf, "It's %sfull of a %s liquid.\r\n", fullness[amt],
		color_liquid[GET_OBJ_VAL(obj, 2)]);
	send_to_char(buf, ch);
      }
    }
  }
}



char *find_exdesc(char *word, struct extra_descr_data * list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word, i->keyword) || is_abbrev(word, i->keyword))
      return (i->description);

  return NULL;
}


/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */
void look_at_target(struct char_data * ch, char *arg, int subcmd)
{
  int    bits, found = 0, j;
  struct char_data *found_char = NULL;
  struct obj_data *obj = NULL, *found_obj = NULL;
  char   *desc;

  if (!*arg) {
    send_to_char("Look at what?\r\n", ch);
    return;
  }
  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

  /* Is the target a character? */
  if (found_char != NULL) {
    look_at_char(found_char, ch);
    if (ch != found_char && subcmd != SCMD_PEEK) {
      if (CAN_SEE(found_char, ch))
	act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
      act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
    }
    return;
  }
  /* Does the argument match an extra desc in the room? */
  if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL) {
    page_string(ch->desc, desc, 0);
    return;
  }
  /* Does the argument match an extra desc in the char's inventory? */
  for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  }
  /* Does the argument match an extra desc in the char's equipment? */
  for (j = 0; j < NUM_WEARS && !found; j++) {
    if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
      if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  }
  /* Does the argument match an extra desc of an object in the room? */
  for (obj = world[ch->in_room].contents; obj && !found; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  if (bits) {    /* If an object was found back in generic_find */
    if (!found)
      show_obj_to_char(found_obj, ch, 5);       /* Show no-description */
    else
      show_obj_to_char(found_obj, ch, 6);       /* Find hum, glow etc */
  } else if (!found)
    send_to_char("You do not see that here.\r\n", ch);
}


ACMD(do_look)
{
  static char arg2[MAX_INPUT_LENGTH];
  int look_type;

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POS_SLEEPING)
    send_to_char("You can't see anything but the inside of your eyelids!\r\n", ch);
  else if (IS_AFFECTED2(ch, AFF_BLIND))
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
  else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    list_char_to_char(world[ch->in_room].people, ch);   /* glowing red eyes */
  } else {
    half_chop(argument, arg, arg2);

    if (subcmd == SCMD_READ) {
      if (!*arg)
	send_to_char("Read what?\r\n", ch);
      else
	look_at_target(ch, arg, subcmd);
      return;
    }
    if (!*arg)                  /* "look" alone, without an argument at all */
      look_at_room(ch, 1);
    else if (is_abbrev(arg, "in"))
      look_in_obj(ch, arg2);
    /* did the char type 'look <direction>?' */
    else if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else if (is_abbrev(arg, "at"))
      look_at_target(ch, arg2, subcmd);
    else
      look_at_target(ch, arg, subcmd);
  }
}



ACMD(do_examine)
{
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Examine what?\r\n", ch);
    return;
  }
  look_at_target(ch, arg, subcmd);

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\r\n", ch);
      look_in_obj(ch, arg);
    }
  }
}



ACMD(do_gold)
{
  if (GET_GOLD(ch) == 0)
    send_to_char("You're broke!\r\n", ch);
  else if (GET_GOLD(ch) == 1)
    send_to_char("You have one miserable little gold piece.\r\n", ch);
  else {
    sprintf(buf, "You have %ld gold pieces in your purse, and %ld in the bank.\r\n",
	   GET_GOLD(ch), GET_BANK_GOLD(ch));
    send_to_char(buf, ch);
  }
}


void list_scanned_chars(struct char_data * list, struct char_data * ch, int distance, int door)
{
  const char *how_far[] = {
    "close by",
    "a ways off",
    "far off to the",
    "very far off to the"
  };
  struct char_data *i;
  int    count = 0;
  *buf = '\0';

  /* this loop is a quick, easy way to help make a grammatical sentence
     (i.e., "You see x, x, y, and z." with commas, "and", etc.)  */
  for (i = list; i; i = i->next_in_room)
    if (CAN_SEE(ch, i))
      count++;

  if (!count)
    return;

  for (i = list; i; i = i->next_in_room) {
    if (!CAN_SEE(ch, i))
      continue;
    if (!*buf)
      sprintf(buf, "You see %s", GET_NAME(i));
    else
      sprintf(buf, "%s%s", buf, GET_NAME(i));
    if (--count > 1)
      strcat(buf, ", ");
    else if (count == 1)
      strcat(buf, " and ");
    else {
      sprintf(buf2, " %s %s.\r\n", how_far[distance], dirs[door]);
      strcat(buf, buf2);
    }
  }
  send_to_char(buf, ch);
}

ACMD(do_scan)
{
  int door;
  extern struct zone_data *zone_table;
  *buf = '\0';

  if (IS_AFFECTED2(ch, AFF_BLIND)) {
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
    return;
  }
  /* may want to add more restrictions here, too */
  send_to_char("You quickly scan the area.\r\n", ch);
  act("$n peers about the room intently.\r\n", TRUE, ch, 0, 0, TO_ROOM);
  if (zone_table[(world[ch->in_room].zone)].weather_info.sky == SKY_BLIZZARD) {
    send_to_char("The blizzard makes it too difficult to see any farther away.\r\n", ch);
  }
  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      if (world[EXIT(ch, door)->to_room].people) {
	list_scanned_chars(world[EXIT(ch, door)->to_room].people, ch, 0, door);
      } else if (_2ND_EXIT(ch, door) && _2ND_EXIT(ch, door)->to_room !=
		 NOWHERE && !IS_SET(_2ND_EXIT(ch, door)->exit_info, EX_CLOSED)) {
	/* check the second room away */
	if (world[_2ND_EXIT(ch, door)->to_room].people) {
	  list_scanned_chars(world[_2ND_EXIT(ch, door)->to_room].people, ch, 1, door);
	} else if (_3RD_EXIT(ch, door) && _3RD_EXIT(ch, door)->to_room !=
		   NOWHERE && !IS_SET(_3RD_EXIT(ch, door)->exit_info, EX_CLOSED)) {
	  /* check the third room */
	  if (world[_3RD_EXIT(ch, door)->to_room].people) {
	    list_scanned_chars(world[_3RD_EXIT(ch, door)->to_room].people, ch, 2, door);
	  } else if (_4TH_EXIT(ch, door) && _4TH_EXIT(ch, door)->to_room !=
		   NOWHERE && !IS_SET(_4TH_EXIT(ch, door)->exit_info, EX_CLOSED)) {
	    /* check the third room */
	    if (world[_4TH_EXIT(ch, door)->to_room].people) {
	      list_scanned_chars(world[_4TH_EXIT(ch, door)->to_room].people, ch, 3, door);
	    }
	  }
	}
      }
    }
}


ACMD(do_score)
{
  char  temp_buf[100];
  int   i;
  float percent, curr, maxim;
  struct guild_type *gptr = NULL;
  extern struct guild_type *guild_info;
  struct time_info_data playing_time;
  struct time_info_data real_time_passed(time_t t2, time_t t1);
  extern char *locations[];

  sprintf(buf, "%sYou are %s%d%s%s years old.",
	       CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), GET_AGE(ch),
	       CCNRM(ch, C_SPR), CCMAG(ch, C_SPR));

  if ((age(ch).month == 0) && (age(ch).day == 0))
    strcat(buf, "  It's your birthday today.\r\n");
  else
    strcat(buf, "\r\n");

  curr    = (float) GET_HIT(ch);
  maxim   = MAX((float) 1, (float) GET_MAX_HIT(ch));
  percent = (curr/maxim)*100;
  sprintf(buf, "%s%sYou have %s%3.1f%%%s%s Hit, ", buf,
	       CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), percent,
	       CCNRM(ch, C_SPR), CCMAG(ch, C_SPR));
  curr    = (float) GET_MANA(ch);
  maxim   = MAX((float) 1, (float) GET_MAX_MANA(ch));
  percent = (curr/maxim)*100;
  sprintf(buf, "%s%s%3.1f%%%s%s Piety, and ", buf,
	       CCBMG(ch, C_SPR), percent, CCNRM(ch, C_SPR), CCMAG(ch, C_SPR));
  curr    = (float) GET_MOVE(ch);
  maxim   = MAX((float) 1, (float) GET_MAX_MOVE(ch));
  percent = (curr/maxim)*100;
  sprintf(buf, "%s%s%3.1f%%%s%s Move.%s\r\n", buf,
	       CCBMG(ch, C_SPR), percent, CCNRM(ch, C_SPR), CCMAG(ch, C_SPR),
	       CCNRM(ch, C_SPR));

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
  sprintf(buf, "%s%sAttributes: Str: %s%d/%d%s%s, Int: %s%d%s%s, Wis: "
	"%s%d%s%s, Dex: %s%d%s%s, Con: %s%d%s%s, Cha: %s%d%s%s, Will: "
	"%s%d%s%s.%s\r\n",
	  buf, CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), GET_STR(ch), GET_ADD(ch),
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), GET_INT(ch),
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), GET_WIS(ch),
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), GET_DEX(ch),
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), GET_CON(ch),
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), GET_CHA(ch),
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), GET_WILL(ch),
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR), CCNRM(ch, C_SPR));
  }
#if 0
  sprintf(buf, "%s%sYour armor ratings are: %sHead %3d%%, Body %3d%%, Shield %3d%%\r\n",
	  buf, CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), GET_AC(ch, ARMOR_HEAD),
	       GET_AC(ch, ARMOR_BODY), GET_AC(ch, ARMOR_SHIELD));
  sprintf(buf, "%s                        Arms (R/L) %3d%%/%3d%%, Legs (R/L) %3d%%/%3d%%%s\r\n",
	  buf, GET_AC(ch, ARMOR_ARM_R), GET_AC(ch, ARMOR_ARM_L),
	       GET_AC(ch, ARMOR_LEG_R), GET_AC(ch, ARMOR_LEG_L), CCNRM(ch, C_SPR));
#endif
  sprintf(buf, "%s%sYou are of %s%s%s%s alignment", buf, CCMAG(ch, C_SPR),
	       CCBMG(ch, C_SPR), (IS_EVIL(ch) ? "evil" : IS_GOOD(ch) ? "good" : "neutral"),
	       CCNRM(ch, C_SPR), CCMAG(ch, C_SPR));
  if (((GET_PERMALIGN(ch) == ALIGN_EVIL) && (!IS_EVIL(ch))) || \
      ((GET_PERMALIGN(ch) == ALIGN_GOOD) && (!IS_GOOD(ch))) || \
      ((GET_PERMALIGN(ch) == ALIGN_NEUT) && (IS_EVIL(ch) || IS_GOOD(ch))))
     sprintf(buf, "%s, but your soul is in turmoil.\r\n", buf);
  else
     strcat(buf, ".\r\n");

  sprintf(buf, "%sYou have scored %s%ld%s%s exp", buf,
	       CCBMG(ch, C_SPR), GET_EXP(ch), CCNRM(ch, C_SPR), CCMAG(ch, C_SPR));

  if (GET_LEVEL(ch) < MAX_MORT_LEVEL) {
	sprintf(buf, "%s, and need %s%ld%s%s exp to level.\r\n",
		buf, CCBMG(ch, C_SPR),
		(exp_needed(GET_LEVEL(ch) + 1) - GET_EXP(ch)),
		CCNRM(ch, C_SPR), CCMAG(ch, C_SPR));
  } else
      sprintf(buf, "%s, and you can no longer gain levels.\r\n", buf);

  /* They'll have to type gold to see what's in their purse from now on */
  if (!IS_NPC(ch)) {
     playing_time = real_time_passed((time(0) - ch->player.time.logon) +
				  ch->player.time.played, 0);
     sprintf(buf, "%sYou have been playing for %s%d%s%s"
		  " days, %s%d%s%s hours, %s%d%s%s minutes, "
		  "and %s%d%s%s seconds.\r\n",
	  buf, CCBMG(ch, C_SPR), playing_time.day,
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR),
	  CCBMG(ch, C_SPR), playing_time.hours,
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR),
	  CCBMG(ch, C_SPR), playing_time.minutes,
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR),
	  CCBMG(ch, C_SPR), playing_time.seconds,
	  CCNRM(ch, C_SPR), CCMAG(ch, C_SPR));

     sprintf(buf, "%sThis ranks you as %s%s the level %d ",
	     buf, CCYEL(ch, C_SPR), GET_NAME(ch), GET_LEVEL(ch));
     if (GET_MAX_LEVEL(ch) < LVL_IMMORT)
	sprinttype(ch->player.race, pc_race_types, temp_buf);
     else
	str_cpy(temp_buf, "Immortal");
     sprintf(buf, "%s%s%s%s.\r\n", buf, temp_buf, 
	     CCNRM(ch, C_SPR), CCMAG(ch, C_SPR));

     /*  Print the guild membership if any */
     if (GET_GUILD(ch) > 0) {
       for (gptr = guild_info; gptr && gptr->number != GET_GUILD(ch); gptr=gptr->next)
	 ;
       if (gptr && gptr->number == GET_GUILD(ch))
	 sprintf(buf,  "%s%sYou are a %s%s%s%s in %s%s%s%s.%s\r\n", buf, 
		CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), 
		(GET_GUILD_LEV(ch) == 5 ? gptr->xlvlname :
		 (GET_GUILD_LEV(ch) == 4 ? gptr->hlvlname :
		  (GET_GUILD_LEV(ch) == 1 ? gptr->blvlname :
		   gptr->mlvlname))), 
		CCNRM(ch, C_SPR), CCMAG(ch, C_SPR), CCBMG(ch, C_SPR), 
		gptr->name, CCNRM(ch, C_SPR), CCMAG(ch, C_SPR), 
		CCNRM(ch, C_SPR));
     }
  }
  switch (GET_POS(ch)) {
  case POS_DEAD:
    sprintf(buf, "%s%sYou are DEAD%s!\r\n", buf, CCRED(ch, C_SPR), CCNRM(ch, C_SPR));
    break;
  case POS_MORTALLYW:
    sprintf(buf, "%s%sYou are mortally wounded!  You should seek help!%s\r\n",
	    buf, CCBMG(ch, C_SPR), CCNRM(ch, C_SPR));
    break;
  case POS_INCAP:
    strcat(buf, "You are incapacitated, slowly fading away...\r\n");
    break;
  case POS_STUNNED:
    strcat(buf, "You are stunned!  You can't move!\r\n");
    break;
  case POS_SLEEPING:
    strcat(buf, "You are sleeping.\r\n");
    break;
  case POS_RESTING:
    strcat(buf, "You are resting.\r\n");
    break;
  case POS_SITTING:
    strcat(buf, "You are sitting.\r\n");
    break;
  case POS_FIGHTING:
    if (FIGHTING(ch))
      sprintf(buf, "%sYou are fighting %s.\r\n", buf, PERS(FIGHTING(ch), ch));
    else
      strcat(buf, "You are fighting thin air.\r\n");
    break;
  case POS_STANDING:
    strcat(buf, "You are standing.\r\n");
    break;
  case POS_MOUNTED:
    sprintf(buf, "%sYou are mounted upon %s.\r\n", buf, GET_NAME(MOUNTED(ch)));
    break;
  case POS_MEDITATING:
    strcat(buf, "You are contemplating your navel.\r\n");
    break;
  default:
    strcat(buf, "You are floating.\r\n");
    break;
  }

  for (i = 0; i < 12; i++) {
    if (GET_ITEMS_STUCK(ch,  i)) {
      strcat(buf, CCBRD(ch, C_SPR));
      sprintf(buf2, "%s is lodged in your %s.\r\n",
	      GET_ITEMS_STUCK(ch, i)->short_description ?
	      GET_ITEMS_STUCK(ch, i)->short_description : "Something",
	      locations[i]);
      CAP(buf2);
      strcat(buf, buf2);
    }
  }

  strcat(buf, CCBMG(ch, C_SPR));
  if (IS_AFFECTED2(ch, AFF_PETRIFIED))
    strcat(buf, "You are petrified, and unable to do anything!\r\n");

  if (IS_AFFECTED2(ch, AFF_BOUND))
    strcat(buf, "Your hands and feet are tied!\r\n");

  if (IS_AFFECTED2(ch, AFF_BLACKMANTLE))
    strcat(buf, "You are surrounded by a black aura!\r\n");

  if (GET_COND(ch, DRUNK) > 10)
    strcat(buf, "You are intoxicated.\r\n");

  if (GET_COND(ch, FULL) == 0)
    strcat(buf, "You are hungry.\r\n");

  if (GET_COND(ch, THIRST) == 0)
    strcat(buf, "You are thirsty.\r\n");

  if (GET_COND(ch, TIRED) <= 2 && GET_COND(ch, TIRED) >= 0)
    strcat(buf, "You are tired and in need of sleep.\r\n");

  if (IS_AFFECTED2(ch, AFF_BLIND))
    strcat(buf, "You have been blinded!\r\n");

  if ((IS_AFFECTED(ch, AFF_INVISIBLE)) || \
      IS_ITEM_AFF(ch, AFFECTED_INVIS))
    strcat(buf, "You are invisible.\r\n");

  if ((IS_AFFECTED(ch, AFF_DETECT_INVIS)) || \
      IS_ITEM_AFF(ch, AFFECTED_DETECT_INVIS))
    strcat(buf, "You are sensitive to the presence of invisible things.\r\n");

  if ((IS_AFFECTED(ch, AFF_SANCTUARY)) || \
      IS_ITEM_AFF(ch, AFFECTED_SANCTUARY))
    strcat(buf, "You are protected by Sanctuary.\r\n");

  if (IS_AFFECTED2(ch, AFF_POISON_I)  || \
      IS_AFFECTED2(ch, AFF_POISON_II) || \
      IS_AFFECTED2(ch, AFF_POISON_III))
    strcat(buf, "You are poisoned!\r\n");

  if (IS_AFFECTED2(ch, AFF_DISEASE))
    strcat(buf, "You are diseased!\r\n");

  if (IS_AFFECTED(ch, AFF_CHARM))
    strcat(buf, "You have been charmed!\r\n");

  if (IS_AFFECTED2(ch, AFF_SILENCE))
    strcat(buf, "You have been silenced!\r\n");

  if (affected_by_spell(ch, SPELL_ARMOR, FALSE))
    strcat(buf, "You feel protected.\r\n");

  if ((IS_AFFECTED(ch, AFF_INFRAVISION)) || \
      IS_ITEM_AFF(ch, AFFECTED_INFRA))
    strcat(buf, "Your eyes are glowing red.\r\n");

  if (COLOR_LEV(ch) != C_OFF)
     sprintf(buf, "%s%s", buf, CCNRM(ch, C_SPR));
  send_to_char(buf, ch);
}


/* We MUST get rid of inventory NOW!!!! */
ACMD(do_inventory)
{
  send_to_char("You are carrying:\r\n", ch);
  list_obj_to_char(ch->carrying, ch, 3, TRUE);
}


ACMD(do_equipment)
{
  int i, found = 0;

  send_to_char("You are using:\r\n", ch);
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (CAN_SEE_OBJ(ch, GET_EQ(ch, i))) {
	send_to_char(CCCYN(ch, C_SPR), ch);
	if (i != 17  && i != 18)
	  send_to_char(where[i], ch);
	else {
	  if (i == 17)  /* wielded */
	    sprintf(buf, "<Wielded in %-5.5s hand>  ", (GET_HANDEDNESS(ch) ? "right" : "left"));
	  else /* held */
	    sprintf(buf, "<Held in %-5.5s hand>     ", (GET_HANDEDNESS(ch) ? "left" : "right"));
	  send_to_char(buf, ch);
	}
	send_to_char(CCNRM(ch, C_SPR), ch);
	show_obj_to_char(GET_EQ(ch, i), ch, 1);
	found = TRUE;
      } else {
	send_to_char(CCCYN(ch, C_SPR), ch);
	if (i != 17  && i != 18)
	  send_to_char(where[i], ch);
	else {
	  if (i == 17)  /* wielded */
	    sprintf(buf, "<Wielded in %-5.5s hand>  ", (GET_HANDEDNESS(ch) ? "right" : "left"));
	  else /* held */
	    sprintf(buf, "<Held in %-5.5s hand>     ", (GET_HANDEDNESS(ch) ? "left" : "right"));
	  send_to_char(buf, ch);
	}
	send_to_char(CCNRM(ch, C_SPR), ch);
	send_to_char("Something.\r\n", ch);
	found = TRUE;
      }
    }
  }
  if (!found) {
    send_to_char(" Nothing.\r\n", ch);
  }
}


ACMD(do_time)
{
  char *suf;
  int weekday, day;
  extern struct time_info_data time_info;
  extern const char *weekdays[];
  extern const char *month_name[];
  extern ACMD(do_date);

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"));

  /* 35 days in a month */
  weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  day = time_info.day + 1;      /* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf, "The %d%s Day of the %s, Year %d.\r\n",
	  day, suf, month_name[(int) time_info.month], time_info.year);

  send_to_char(buf, ch);
  /* Now for the OOC stuff */
  send_to_char(CCBLU(ch, C_SPR), ch);
  do_date(ch, "", 0, SCMD_DATE);
  do_date(ch, "", 0, SCMD_UPTIME);
  send_to_char(CCNRM(ch, C_SPR), ch);
}


ACMD(do_weather)
{
  static char *sky_look[] = {
    "This heat wave is unbearable!",
    "It is very sunny and the temperature is rising.",
    "It is warm and sunny.",
    "The sky is clear and the weather is mild.",
    "The sky is partly cloudy.",
    "The cloud cover is thick.",
    "A light rain is falling.",
    "The rain is pouring down.",
    "The sky is lit by flashes of lightning, and thunder rumbles around you.",
    "The sky is filled with dark clouds and a twisting funnel touches down near by!",
    "Light snow falls from the sky.",
    "It is snowing heavily.",
    "You are blinded by the blizzard!"
  };

  if (OUTSIDE(ch)) {
    sprintf(buf, "%s\r\n", sky_look[(zone_table[(world[ch->in_room].zone)].weather_info.sky)]);
    send_to_char(buf, ch);
    if (GET_LEVEL(ch) > LVL_IMMORT) {
      sprintf(buf, "The humidity is at %d percent.\r\n", 
           zone_table[(world[ch->in_room].zone)].weather_info.humidity);
      send_to_char(buf, ch);
    }
  } else
    send_to_char("You have no feeling about the weather at all.\r\n", ch);
}


#ifdef OLD_HELP
struct help_index_element *build_help_index(FILE * fl, int *num)
{
  int nr = -1, issorted, i;
  struct help_index_element *list = 0, mem;
  char buf[128], tmp[128], *scan;
  long pos;
  int count_hash_records(FILE * fl);

  i = count_hash_records(fl) * 5;
  rewind(fl);
  CREATE(list, struct help_index_element, i);

  for (;;) {
    pos = ftell(fl);
    fgets(buf, 128, fl);
    *(buf + strlen(buf) - 1) = '\0';
    scan = buf;
    for (;;) {
      /* extract the keywords */
      scan = one_word(scan, tmp);

      if (!*tmp)
	break;

      nr++;

      list[nr].pos = pos;
      CREATE(list[nr].keyword, char, strlen(tmp) + 1);
      str_cpy(list[nr].keyword, tmp);
    }
    /* skip the text */
    do
      fgets(buf, 128, fl);
    while (*buf != '#');
    if (*(buf + 1) == '~')
      break;
  }
  /* we might as well sort the stuff */
  do {
    issorted = 1;
    for (i = 0; i < nr; i++)
      if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0) {
	mem = list[i];
	list[i] = list[i + 1];
	list[i + 1] = mem;
	issorted = 0;
      }
  } while (!issorted);

  *num = nr;
  return (list);
}
#endif

ACMD(do_help)
{
  extern int top_of_helpt;
  extern struct help_index_element *help_table;
  extern char *help;
#ifdef OLD_HELP
  extern FILE *help_fl;
  extern FILE *wizhelp_fl;
#endif

  extern int top_of_wizhelpt;
  extern struct help_index_element *wizhelp_table;

  int chk, bot, top, mid, minlen, found = 0;

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument) {
    page_string(ch->desc, help, 0);
    return;
  }
  if (!help_table) {
    send_to_char("No help available.\r\n", ch);
    return;
  }
  bot = 0;
  top = top_of_helpt;

  for (;;) {
    mid = (bot + top) >> 1;
    minlen = strlen(argument);

    if (!(chk = strn_cmp(argument, help_table[mid].keyword, minlen))) {

      /* trace backwards to find first matching entry. Thanks Jeff Fink! */
      while ((mid > 0) &&
	 (!(chk = strn_cmp(argument, help_table[mid - 1].keyword, minlen))))
	mid--;
#ifdef OLD_HELP
      fseek(help_fl, help_table[mid].pos, SEEK_SET);
      *buf2 = '\0';
      for (;;) {
	fgets(buf, 128, help_fl);
	if (*buf == '#')
	  break;
	buf[strlen(buf) - 1] = '\0';    /* cleave off the trailing \n */
	strcat(buf2, strcat(buf, "\r\n"));
      }
      page_string(ch->desc, buf2, 1);
#else
      page_string(ch->desc, help_table[mid].entry, 0);
#endif
      found = 1;
      return;
    } else if (bot >= top) {
      if (!found && GET_MAX_LEVEL(ch) >= LVL_IMMORT) {
	 if (!wizhelp_table) {
	    send_to_char("No wizhelp available.\r\n", ch);
	    return;
	 }
	 bot = 0;
	 top = top_of_wizhelpt;

	 for (;;) {
	     mid = (bot + top) >> 1;
	     minlen = strlen(argument);

	     if (!(chk = strn_cmp(argument, wizhelp_table[mid].keyword, minlen))) {
		/* trace backwards to find first matching entry. Thanks Jeff Fink! */
		while ((mid > 0) &&
		   (!(chk = strn_cmp(argument, wizhelp_table[mid - 1].keyword, minlen))))
		   mid--;
#ifdef OLD_HELP
		fseek(wizhelp_fl, wizhelp_table[mid].pos, SEEK_SET);
		*buf2 = '\0';
		for (;;) {
		   fgets(buf, 128, wizhelp_fl);
		   if (*buf == '#')
		      break;
		   buf[strlen(buf) - 1] = '\0'; /* cleave off the trailing \n */
		   strcat(buf2, strcat(buf, "\r\n"));
		}
		page_string(ch->desc, buf2, 1);
#else
		page_string(ch->desc, wizhelp_table[mid].entry, 0);
#endif
		return;
	     } else if (bot >= top) {
		send_to_char("There is no wizhelp on that word.\r\n", ch);
		return;
	     } else if (chk > 0)
		bot = ++mid;
	      else
		top = --mid;
	 }
      } else {
	 send_to_char("There is no help on that word.\r\n", ch);
	 return;
      }
    } else if (chk > 0)
      bot = ++mid;
    else
      top = --mid;
  }
}

/* old format:
 * "format: who [minlev[-maxlev]] [-n name] [-c classlist] [-s] [-o] [-q] [-r] [-z]\r\n"
 */
#define WHO_FORMAT \
"format: who [-n name] [-g gods] [-r race] [-R room]\r\n"


#define GOD_WHO_FORMAT \
"format: who [minlev[-maxlev]] [-n name] [-i gods] [-r race] [-g guild]\r\n" \
"            [-z zone] [-q quest] [-G God version] [-o outlaws]\r\n"


ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *tch;
  char name_search[MAX_INPUT_LENGTH];
  char mode;
  /* static int max_persons = 0; */
  int i, low = 0, high = LVL_CREATOR, localwho = 0, questwho = 0;
  int short_list = 0, outlaws = 0, num_can_see = 0;
  int who_room = 0, showrace = 0;

  skip_spaces(&argument);
  str_cpy(buf, argument);
  name_search[0] = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg) && (GET_LEVEL(ch) >= LVL_IMMORT)) {
      sscanf(arg, "%d-%d", &low, &high);
      str_cpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1);        /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	str_cpy(buf, buf1);
	break;
      case 'z':
	localwho = 1;
	str_cpy(buf, buf1);
	break;
      case 's':
	short_list = 1;
	str_cpy(buf, buf1);
	break;
      case 'q':
	questwho = 1;
	str_cpy(buf, buf1);
	break;
#if 0
      case 'l':
	if (GET_LEVEL(ch) >= LVL_IMMORT) {
	  half_chop(buf1, arg, buf);
	  sscanf(arg, "%d-%d", &low, &high);
	}
	break;
#endif
      case 'i':
	low  = LVL_IMMORT;
	break;
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'R':
	who_room = 1;
	str_cpy(buf, buf1);
	break;
      case 'r':
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	    showrace |= find_race_bitvector(arg[i]);
	break;
      default:
	if (GET_MAX_LEVEL(ch) < LVL_IMMORT)
	   send_to_char(WHO_FORMAT, ch);
	else
	   send_to_char(GOD_WHO_FORMAT, ch);
	return;
	break;
      }                         /* end of switch */

    } else {                    /* endif */
      send_to_char(WHO_FORMAT, ch);
      return;
    }
  }                             /* end while (parser) */

  send_to_char("\r\nThose currently in The Dominion\r\n"
		   "-------------------------------\r\n", ch);

  for (d = descriptor_list; d; d = d->next) {
    
    if ((d->connected) && !(d->connected == CON_OEDIT ||
	 d->connected == CON_REDIT || d->connected == CON_MEDIT ||
	 d->connected == CON_ZEDIT || d->connected == CON_SEDIT))
      continue;

    if (d->original)
      tch = d->original;
    else if (!(tch = d->character))
      continue;

    if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
	!strstr(GET_TITLE(tch), name_search))
      continue;
    if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
      continue;
    if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	!PLR_FLAGGED(tch, PLR_THIEF))
      continue;
    if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
      continue;
    if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
      continue;
    if (who_room && (tch->in_room != ch->in_room))
      continue;
    if (showrace && !(showrace & (1 << GET_RACE(tch))))
      continue;
    if (short_list) {
      sprintf(buf, "%s[ %s ] %-12.12s%s%s",
	      (GET_LEVEL(tch) >= LVL_IMMORT ? CCYEL(ch, C_SPR) : ""),
	      RACE_ABBR(tch), GET_NAME(tch),
	      (GET_LEVEL(tch) >= LVL_IMMORT ? CCNRM(ch, C_SPR) : ""),
	      ((!(++num_can_see % 4)) ? "\r\n" : ""));
      send_to_char(buf, ch);
    } else {
      num_can_see++;
      sprintf(buf, "%s[ %s ] %s %s",
	      (GET_LEVEL(tch) == LVL_IMMORT     ? CCYEL(ch, C_SPR) : \
	      (GET_LEVEL(tch) == LVL_BUILDER    ? CCBMG(ch, C_SPR) : \
	      (GET_LEVEL(tch) == LVL_DEITY      ? CCCYN(ch, C_SPR) : \
	      (GET_LEVEL(tch) == LVL_GOD        ? CCBCN(ch, C_SPR) : \
	      (GET_LEVEL(tch) == LVL_GRGOD      ? CCRED(ch, C_SPR) : \
	      (GET_LEVEL(tch) == LVL_SUP        ? CCGRN(ch, C_SPR) : \
	      (GET_LEVEL(tch) == LVL_IMPL       ? CCBBL(ch, C_SPR) : \
	      (GET_LEVEL(tch) == LVL_CREATOR    ? CCBBL(ch, C_SPR) : \
	       "")))))))),
	      (GET_LEVEL(tch) == LVL_IMMORT     ? "IMM" : \
	      (GET_LEVEL(tch) == LVL_BUILDER    ? "BLD" : \
	      (GET_LEVEL(tch) == LVL_DEITY      ? "DEI" : \
	      (GET_LEVEL(tch) == LVL_GOD        ? "GOD" : \
	      (GET_LEVEL(tch) == LVL_GRGOD      ? "GRG" : \
	      (GET_LEVEL(tch) == LVL_SUP        ? "SUP" : \
	      (GET_LEVEL(tch) == LVL_IMPL       ? "IMP" : \
	      (GET_LEVEL(tch) == LVL_CREATOR    ? "IMP" : \
	       RACE_ABBR(tch))))))))), GET_NAME(tch),
	       GET_TITLE(tch));

      if (GET_INVIS_LEV(tch))
	sprintf(buf, "%s (i%d)", buf, GET_INVIS_LEV(tch));
      else if (IS_AFFECTED(tch, AFF_INVISIBLE))
	strcat(buf, " (invis)");

      if (PLR_FLAGGED(tch, PLR_MAILING))
	strcat(buf, " (mailing)");
      else if (PLR_FLAGGED(tch, PLR_WRITING))
	strcat(buf, " (writing)");

      if (tch->desc != NULL &&
	  ((STATE(tch->desc) == CON_ZEDIT) ||
	  (STATE(tch->desc) == CON_REDIT) ||
	  (STATE(tch->desc) == CON_MEDIT) ||
	  (STATE(tch->desc) == CON_OEDIT) ||
	  (STATE(tch->desc) == CON_SEDIT)))
	strcat(buf, " (editing)");
      if (PRF_FLAGGED(tch, PRF_DEAF))
	strcat(buf, " (deaf)");
      if (PRF_FLAGGED(tch, PRF_NOTELL))
	strcat(buf, " (notell)");
      if (PRF_FLAGGED(tch, PRF_QUEST))
	strcat(buf, " (quest)");
      if (GET_LEVEL(tch) >= LVL_IMMORT)
	strcat(buf, CCNRM(ch, C_SPR));
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }                           /* endif shortlist */
  }                             /* end of for */
  if (short_list && (num_can_see % 4))
    send_to_char("\r\n", ch);
  if (num_can_see == 0)
    sprintf(buf, "\r\nNo-one at all!\r\n");
  else if (num_can_see == 1)
    sprintf(buf, "\r\nOne lonely character displayed.\r\n");
  else
    sprintf(buf, "\r\n%d characters displayed.\r\n", num_can_see);
  send_to_char(buf, ch);
}


#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-o] [-p] [-d]\r\n"

ACMD(do_users)
{
  extern char *connected_types[];
  char line[200], line2[220], idletime[10], levbuf[30], classname[20];
  char state[30], *timeptr, *format, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  struct char_data *tch;
  struct descriptor_data *d;
  int low = 0, high = LVL_CREATOR, num_can_see = 0;
  int outlaws = 0, playing = 0, deadweight = 0;

  host_search[0] = name_search[0] = '\0';

  str_cpy(buf, argument);
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (*arg == '-') {
      mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	playing = 1;
	str_cpy(buf, buf1);
	break;
      case 'p':
	playing = 1;
	str_cpy(buf, buf1);
	break;
      case 'd':
	deadweight = 1;
	str_cpy(buf, buf1);
	break;
      case 'l':
	playing = 1;
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	playing = 1;
	half_chop(buf1, name_search, buf);
	break;
      case 'h':
	playing = 1;
	half_chop(buf1, host_search, buf);
	break;
      default:
	send_to_char(USERS_FORMAT, ch);
	return;
	break;
      }                         /* end of switch */

    } else {                    /* endif */
      send_to_char(USERS_FORMAT, ch);
      return;
    }
  }                             /* end while (parser) */
  str_cpy(line,
	 "Num Level   Name         State          Idl Login@   Site\r\n");
  strcat(line,
	 "--- ------- ------------ -------------- --- -------- ------------------------\r\n");
  send_to_char(line, ch);

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected && playing)
      continue;
    if (!d->connected && deadweight)
      continue;
    if ((!d->connected) || (d->connected == CON_OEDIT ||
	 d->connected == CON_REDIT || d->connected == CON_MEDIT ||
	 d->connected == CON_ZEDIT || d->connected == CON_SEDIT)) {
      if (d->original)
	tch = d->original;
      else if (!(tch = d->character))
	continue;

      if (*host_search && !strstr(d->host, host_search))
	continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search))
	continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	  !PLR_FLAGGED(tch, PLR_THIEF))
	continue;
      if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
	continue;

      if (d->original) {
	sprintf(levbuf, " %2d", GET_LEVEL(d->original));
	sprintf(classname, "[ %3s ]",
		(GET_MAX_LEVEL(d->original) >= LVL_IMPL ? "IMP" : levbuf));
      } else {
	sprintf(levbuf, " %2d", GET_LEVEL(d->character));
	sprintf(classname, "[ %3s ]",
		(GET_MAX_LEVEL(d->character) >= LVL_IMPL ? "IMP" : levbuf));
      }
    } else
      str_cpy(classname, "   -   ");

    timeptr = asctime(localtime(&d->login_time));
    timeptr += 11;
    *(timeptr + 8) = '\0';

    if (!d->connected && d->original)
      str_cpy(state, "Switched");
    else
      str_cpy(state, connected_types[d->connected]);

    if (d->character && !d->connected && GET_LEVEL(d->character) < LVL_GOD)
      sprintf(idletime, "%3d", d->character->char_specials.timer *
	      SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
    else
      str_cpy(idletime, "");

    format = "%3d %-7s %-12s %-14s %-3s %-8s ";

    if (d->character && d->character->player.name) {
      if (d->original)
	sprintf(line, format, d->desc_num, classname,
		d->original->player.name, state, idletime, timeptr);
      else
	sprintf(line, format, d->desc_num, classname, d->character->player.name, state, idletime, timeptr);
    } else
      sprintf(line, format, d->desc_num, "   -   ", "UNDEFINED",
	      state, idletime, timeptr);

    if (d->host && *d->host)
      sprintf(line + strlen(line), "[%s]\r\n", d->host);
    else
      strcat(line, "[Hostname unknown]\r\n");

    if (d->connected) {
      sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
      str_cpy(line, line2);
    }
    if (d->connected || (!d->connected && CAN_SEE(ch, d->character))) {
      send_to_char(line, ch);
      num_can_see++;
    }
  }

  sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
  send_to_char(line, ch);
}


/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
  extern char circlemud_version[];

  switch (subcmd) {
  case SCMD_CREDITS:
    page_string(ch->desc, credits, 0);
    break;
  case SCMD_NEWS:
    page_string(ch->desc, news, 0);
    break;
  case SCMD_INFO:
    page_string(ch->desc, info, 0);
    break;
  case SCMD_WIZLIST:
    page_string(ch->desc, wizlist, 0);
    break;
  case SCMD_IMMLIST:
    page_string(ch->desc, immlist, 0);
    break;
  case SCMD_HANDBOOK:
    page_string(ch->desc, handbook, 0);
    break;
  case SCMD_POLICIES:
    page_string(ch->desc, policies, 0);
    break;
  case SCMD_MOTD:
    page_string(ch->desc, motd, 0);
    break;
  case SCMD_IMOTD:
    page_string(ch->desc, imotd, 0);
    break;
  case SCMD_NEWBM:
    page_string(ch->desc, newbm, 0);
    break;
  case SCMD_CLEAR:
    send_to_char("\033[H\033[J", ch);
    if (PRF_FLAGGED(ch, PRF_INFOBAR))
      do_infobar(ch, 0, 0, SCMDB_REDRAW);
    break;
  case SCMD_VERSION:
    send_to_char(circlemud_version, ch);
    break;
  case SCMD_WHOAMI:
    str_cpy(buf, GET_NAME(ch));
    send_to_char(strcat(buf, "\r\n"), ch);
    break;
  default:
    return;
    break;
  }
}


void perform_mortal_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct descriptor_data *d;

  if (!*arg) {
    send_to_char("Players in your Zone\r\n--------------------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE) &&
	    (world[ch->in_room].zone == world[i->in_room].zone)) {
	  sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room].name);
	  send_to_char(buf, ch);
	}
      }
  } else {                      /* print only FIRST char, not all. */
    for (i = character_list; i; i = i->next)
      if (world[i->in_room].zone == world[ch->in_room].zone && CAN_SEE(ch, i) &&
	  (i->in_room != NOWHERE) && isname(arg, i->player.name)) {
	sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room].name);
	send_to_char(buf, ch);
	return;
      }
    send_to_char("No-one around by that name.\r\n", ch);
  }
}


void print_object_location(int num, struct obj_data * obj, struct char_data * ch,
				int recur)
{
  if (num > 0)
    sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
  else
    sprintf(buf, "%33s", " - ");

  if (obj->in_room > NOWHERE) {
    sprintf(buf + strlen(buf), "[%ld] %s\n\r",
	    world[obj->in_room].number, world[obj->in_room].name);
    send_to_char(buf, ch);
  } else if (obj->carried_by) {
    sprintf(buf + strlen(buf), "carried by %s\n\r",
	    PERS(obj->carried_by, ch));
    send_to_char(buf, ch);
  } else if (obj->worn_by) {
    sprintf(buf + strlen(buf), "worn by %s\n\r",
	    PERS(obj->worn_by, ch));
    send_to_char(buf, ch);
  } else if (obj->in_obj) {
    sprintf(buf + strlen(buf), "inside %s%s\n\r",
	    obj->in_obj->short_description, (recur ? ", which is" : " "));
    send_to_char(buf, ch);
    if (recur)
      print_object_location(0, obj->in_obj, ch, recur);
  } else {
    sprintf(buf + strlen(buf), "in an unknown location\n\r");
    send_to_char(buf, ch);
  }
}



void perform_immort_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int num = 0, found = 0;

  if (!*arg) {
    send_to_char("Players\r\n-------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
	  if (d->original)
	    sprintf(buf, "%-20s - [%ld] %s (in %s)\r\n",
		    GET_NAME(i), world[d->character->in_room].number,
		 world[d->character->in_room].name, GET_NAME(d->character));
	  else
	    sprintf(buf, "%-20s - [%ld] %s\r\n", GET_NAME(i),
		    world[i->in_room].number, world[i->in_room].name);
	/* page_string(ch->desc, buf, 0); */
	send_to_char(buf, ch);
	}
      }
  } else {
    sprintf(buf, "Location of all '%s' in the game.\r\n"
                 "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n", arg);
    for (i = character_list; i; i = i->next)
      if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name)) {
	found = 1;
	sprintf(buf, "%sM%3d. %-25s - [%ld] %s\r\n", buf, ++num, GET_NAME(i),
		world[i->in_room].number, world[i->in_room].name);
      /* send_to_char(buf, ch);  */
      }
    page_string(ch->desc, buf, 1);
    for (num = 0, k = object_list; k; k = k->next)
      if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name)) {
	found = 1;
	print_object_location(++num, k, ch, TRUE);
      }
    if (!found)
      send_to_char("Couldn't find any such thing.\r\n", ch);
  }
}



ACMD(do_where)
{
  one_argument(argument, arg);

  if (GET_MAX_LEVEL(ch) >= LVL_IMMORT)
    perform_immort_where(ch, arg);
  else
    perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
  int i;

  if (IS_NPC(ch)) {
    send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
    return;
  }
  *buf = '\0';

  if (!*argument) {
     /* Changed for TD - 3/16/95 - SPM */
     for (i = 1; i < LVL_IMMORT; i++) {
	 sprintf(buf + strlen(buf), "[%2d] %8d-%-8d\r\n", i,
	    exp_needed(i), exp_needed(i+1));
     }
  } else
     sprintf(buf, "Don't add any arguments, there is only ONE experience table.\r\n");

  page_string(ch->desc, buf, 1);
}


ACMD(do_evaluate)
{
  struct obj_data *obj;
  extern char *item_types[];

  if (GET_SKILL(ch, SKILL_EVALUATE) < 1)
    return;

  one_argument(argument, buf);

  if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) {
    send_to_char("Evaluate what?\r\n", ch);
    return;
  }

  act("You study $p.", TRUE, ch, obj, NULL, TO_CHAR);
  act("$n examines $p carefully.", TRUE, ch, obj, NULL, TO_ROOM);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
    sprintf(buf, "%s appears to be %s ", obj->short_description,
            XANA(buf2));
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    sprintf(buf, "%s weighs %dlbs., approximate value is %d gold.\r\n",
	    obj->short_description, GET_OBJ_WEIGHT(obj), 
	    (int)(GET_OBJ_COST(obj)*0.025f));
    send_to_char(buf, ch);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_MISSILE:
    case ITEM_WEAPON:
      sprintf(buf, "The weapon's damage is approximately %.1f.\r\n", 
	      (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1)));
      send_to_char(buf, ch);
      break;
    case ITEM_ARMOR:
      sprintf(buf, "The armor will absorb %ld%% of the damage you receive\r\n",
       GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
      break;
    }
    sprintf(buf, "%s appears to be constructed from %s.\r\n",
	        obj->short_description,
		material_list[GET_OBJ_VAL(obj, 7)].name);
    send_to_char(buf, ch);
#if 0
    condition = GET_OBJ_VAL(obj, 9);
    tenth = (material_list[GET_OBJ_VAL(obj, 7)].sturdiness / 10);

    if ((GET_OBJ_TYPE(obj) != ITEM_FOOD) &&
        (GET_OBJ_TYPE(obj) != ITEM_LIGHT)) {
     if (material_list[GET_OBJ_VAL(obj, 7)].sturdiness == -1)
	sprintf(buf, "The item is in perfect condition.\r\n");
     else if (condition < (tenth * 1))
	return (conditions[0]);
     else if (condition < (tenth * 2))
	return (conditions[1]);
     else if (condition < (tenth * 4))
	return (conditions[2]);
     else if (condition < (tenth * 6))
	return (conditions[3]);
     else if (condition < (tenth * 7))
	return (conditions[4]);
     else if (condition < (tenth * 8))
	return (conditions[5]);
     else if (condition < (tenth * 9))
	return (conditions[6]);
     else
	sprintf(buf, "The item is beyond all hope, throw the shit away!\r\n");
  }
  send_to_char(buf, ch);
#endif
}

/* This needs to be redone based on the skills 'know <race>' */
ACMD(do_study)
{
  struct char_data *victim;
  int diff, percent;

  one_argument(argument, buf);

  if (!(victim = get_char_room_vis(ch, buf))) {
    send_to_char("Study who?\r\n", ch);
    return;
  }
  if (victim == ch) {
    send_to_char("You should know yourself by now!\r\n", ch);
    return;
  }

  act("You study $N.", TRUE, ch, 0, victim, TO_CHAR);
  act("$n looks at you intently.", TRUE, ch, 0, victim, TO_VICT);

  diff = (GET_LEVEL(victim) - GET_LEVEL(ch));

  if (diff <= -30) 
    act("$E seems to have less combat experience than yourself.", FALSE, 
	ch, 0, victim, TO_CHAR);
  else if (diff <= -20)
    act("$E seems inexperienced in the ways of combat.", FALSE, ch, 0, victim, TO_CHAR);
  else if (diff <= -10)
    act("$E seems qualified in the ways of combat.", FALSE, ch, 0, victim, TO_CHAR);
  else if (diff <= 0)
    act("$E seems fairly experienced in the ways of combat.", FALSE, ch, 0, victim, TO_CHAR);
  else if (diff <= 10)
    act("$E seems experienced in the ways of combat.", FALSE, ch, 0, victim, TO_CHAR);
  else if (diff <= 20)
    act("$E seems very experienced in the ways of combat.", FALSE, ch, 0, victim, TO_CHAR);
  else
    act("$E seems accomplished in the ways of combat.", FALSE, ch, 0, victim, TO_CHAR);
  /* Now check diff in movement */
  percent = (int)(((float)GET_MOVE(victim)/(float)GET_MAX_MOVE(victim)) * 100);

  if (percent < 25)
    act("$E looks exhausted.", FALSE, ch, 0, victim, TO_CHAR);
  else if (percent < 40)
    act("$E looks fatigued.", FALSE, ch, 0, victim, TO_CHAR);
  else if (percent < 65)
    act("$E looks tired.", FALSE, ch, 0, victim, TO_CHAR);
  else if (percent < 80)
    act("$E looks worn.", FALSE, ch, 0, victim, TO_CHAR);
  else if (percent < 95)
    act("$E looks rested.", FALSE, ch, 0, victim, TO_CHAR);
  else
    act("$E looks well rested.", FALSE, ch, 0, victim, TO_CHAR);

  percent = (int)(100 - (((float)GET_HIT(ch)/(float)GET_HIT(victim))*100));
  if (percent <= 0)
    act("$E looks like an easy kill.", FALSE, ch, 0, victim, TO_CHAR);
  else if (percent < 20)
    act("$E looks weak.", FALSE, ch, 0, victim, TO_CHAR);
  else if (percent < 40)
    act("$E looks healthy.", FALSE, ch, 0, victim, TO_CHAR);
  else if (percent < 60)
    act("$E looks very healthy.", FALSE, ch, 0, victim, TO_CHAR);
  else if (percent < 80)
    act("$E looks like a tough opponent.", FALSE, ch, 0, victim, TO_CHAR);
  else
    act("$E looks to be a well conditioned fighter.", FALSE, ch, 0, victim, TO_CHAR); 
}



ACMD(do_diagnose)
{
  struct char_data *vict;

  one_argument(argument, buf);

  if (*buf) {
    if (!(vict = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    } else
      diag_char_to_char(vict, ch);
  } else {
    if (FIGHTING(ch))
      diag_char_to_char(FIGHTING(ch), ch);
    else
      send_to_char("Diagnose who?\r\n", ch);
  }
}


static char *ctypes[] = {
"off", "sparse", "normal", "complete", "\n"};

ACMD(do_color)
{
  int tp;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "Your current color level is %s.\r\n", ctypes[COLOR_LEV(ch)]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
    send_to_char("Usage: color { Off | Sparse | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
  SET_BIT(PRF_FLAGS(ch), (PRF_COLOR_1 * (tp & 1)) | (PRF_COLOR_2 * (tp & 2) >> 1));

  sprintf(buf, "Your %s%sc%so%sl%so%sr%s is now %s.\r\n", CCFSH(ch, C_SPR),
	  CCRED(ch, C_SPR), CCBBL(ch, C_SPR), CCYEL(ch, C_SPR),
	  CCMAG(ch, C_SPR), CCGRN(ch, C_SPR), CCNRM(ch, C_OFF), ctypes[tp]);
  send_to_char(buf, ch);
}


ACMD(do_toggle)
{
  if (IS_NPC(ch))
    return;
  if (GET_WIMP_LEV(ch) == 0)
    str_cpy(buf2, "OFF");
  else
    sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

  sprintf(buf,
	  "Hit Pnt Display: %-3s    "
	  "   Mana Display: %-3s    "
	  "   Move Display: %-3s\r\n"

	  "Exp Pnt Display: %-3s    "
	  "     Brief Mode: %-3s    "
	  "   Compact Mode: %-3s\r\n"

	  " Auto Show Exit: %-3s    "
	  "    Auto Assist: %-3s    "
	  "      Auto Loot: %-3s\r\n"

	  "      Auto Gold: %-3s    "
	  "     Wimp Level: %-3s    "
	  "   Repeat Comm.: %-3s\r\n"

	  "           Deaf: %-3s    "
	  "         NoTell: %-3s    "
	  "    OOC Channel: %-3s\r\n"

	  " Gossip Channel: %-3s    "
	  "       On Quest: %-3s    "
	  "    Color Level: %s  \r\n"

	  "Fight Spam Prot: %-3s    "
	  "        InfoBar: %-3s\r\n",

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISEXP2LEV)),
	  ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
	  ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),

	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOASS)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOLOOT)),

	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOGOLD)),
	  buf2,
	  YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),

	  YESNO(PRF_FLAGGED(ch, PRF_DEAF)),
	  ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOOOC)),

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
	  YESNO(PRF_FLAGGED(ch, PRF_QUEST)),
	  ctypes[COLOR_LEV(ch)],

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOSPAM)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_INFOBAR)));

  send_to_char(buf, ch);
}


struct sort_struct {
  int sort_pos;
  byte is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;


void sort_commands(void)
{
  int a, b, tmp;

  num_of_cmds = 0;

  /*
   * first, count commands (num_of_commands is actually one greater than the
   * number of commands; it inclues the '\n'.
   */
  while (*cmd_info[num_of_cmds].command != '\n')
    num_of_cmds++;

  /* create data array */
  CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

  /* initialize it */
  for (a = 1; a < num_of_cmds; a++) {
    cmd_sort_info[a].sort_pos = a;
    cmd_sort_info[a].is_social = (cmd_info[a].command_pointer == do_action);
  }

  /* the infernal special case */
  cmd_sort_info[find_command("insult")].is_social = TRUE;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < num_of_cmds - 1; a++)
    for (b = a + 1; b < num_of_cmds; b++)
      if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
		 cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
	tmp = cmd_sort_info[a].sort_pos;
	cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
	cmd_sort_info[b].sort_pos = tmp;
      }
}



ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  struct char_data *vict;

  one_argument(argument, arg);

  if (*arg) {
    if (!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
      send_to_char("Who is that?\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("You can't see the commands of people above your level.\r\n", ch);
      return;
    }
  } else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;

  sprintf(buf, "The following %s%s are available to %s:\r\n",
	  wizhelp ? "privileged " : "",
	  socials ? "socials" : "commands",
	  vict == ch ? "you" : GET_NAME(vict));

  /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
    i = cmd_sort_info[cmd_num].sort_pos;
    if (cmd_info[i].minimum_level >= 0 &&
	GET_MAX_LEVEL(vict) >= cmd_info[i].minimum_level &&
	(cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
	(wizhelp || socials == cmd_sort_info[i].is_social)) {
      /* Don't print those pesky mp commands */
      if (!strstr(cmd_info[i].command, "mp")) {
	sprintf(buf + strlen(buf), "%-11s", cmd_info[i].command);
	if (!(no % 7))
	  strcat(buf, "\r\n");
	no++;
      }
    }
  }
  strcat(buf, "\r\n");
  send_to_char(buf, ch);
}



/*
 * infobar (everything below here is all infobar related)
 * BUGS: infobar doesn't seem to want to draw when its first used.  who knows.
 *       something about the bar seems to cause a seg fault right after connect
 */

/*
 * Produces strings for hit and damage bonuses.
 */
void bonstr(int x, char *s)
{
   if (x <= 1)
     str_cpy(s, "Poor");
   else if (x <= 10)
     str_cpy(s, "Average");
   else if (x <= 20)
     str_cpy(s, "Above Average");
   else if (x <= 30)
     str_cpy(s, "Good");
   else if (x <= 40)
     str_cpy(s, "Very Good");
   else if (x <= 50)
     str_cpy(s, "Excellent");
   else if (x <= 60)
     str_cpy(s, "Astounding");
   else
     str_cpy(s, "Amazing");
}

/*
 * Produces strings for AC.
 */
void acstr(int x, char *s)
{

   if (x <= 10)
     str_cpy(s, "You are extremely easy to hit.");
   else if (x <= 20)
     str_cpy(s, "You are easy to hit.");
   else if (x <= 40)
     str_cpy(s, "You are not difficult to hit.");
   else if (x <= 50)
     str_cpy(s, "Your are average.");
   else if (x <= 60)
     str_cpy(s, "You are not easy to hit.");
   else if (x <= 70)
     str_cpy(s, "You are difficult to hit.");
   else
     str_cpy(s, "You are very difficult to hit.");
}

/*
 * Produces a string describing your present hunger, thirst,
 * sobriety, and tiredness or lack thereof.
 */
void hungerstr(int f, int t, int d, int i, char *s)
{
   char full[40], thirst[40], drunk[40], tired[40];

   if ((f >= 0) && (f <= 4))
     str_cpy(full, "You are starving, ");
   else if ((f >= 0) && (f <= 8))
     str_cpy(full, "You are very hungry, ");
   else if ((f >= 0) && (f <= 12))
     str_cpy(full, "You are hungry, ");
   else if ((f >= 0) && (f <= 16))
     str_cpy(full, "You are a bit hungry, ");
   else if ((f >= 0) && (f <= 20))
     str_cpy(full, "You are not hungry, ");
   else if ((f >= 0) && (f <= 24))
     str_cpy(full, "You are well-fed, ");
   else
     str_cpy(full, "You are never hungry, ");

   if ((t >= 0) && (t <= 4))
     str_cpy(thirst, "dehydrated, ");
   else if ((t >= 0) && (t <= 8))
     str_cpy(thirst, "very thirsty, ");
   else if ((t >= 0) && (t <= 12))
     str_cpy(thirst, "thirsty, ");
   else if ((t >= 0) && (t <= 16))
     str_cpy(thirst, "parched, ");
   else if ((t >= 0) && (t <= 24))
     str_cpy(thirst, "not thirsty, ");
   else
     str_cpy(thirst, "never thirsty, ");

   if ((i >= 0) && (i <= 4))
     str_cpy(tired, "exhausted, ");
   else if ((i >= 0) && (i <= 8))
     str_cpy(tired, "very tired, ");
   else if ((i >= 0) && (i <= 12))
     str_cpy(tired, "tired, ");
   else if ((i >= 0) && (i <= 16))
     str_cpy(tired, "weary, ");
   else if ((i >= 0) && (i <= 24))
     str_cpy(tired, "well rested, ");
   else
     str_cpy(tired, "and never tired");

   if ((d >= 0) && (d <= 4))
     str_cpy(drunk, "and sober.");
   else if ((d >= 0) && (d <= 8))
     str_cpy(drunk, "and buzzing.");
   else if ((d >= 0) && (d <= 12))
     str_cpy(drunk, "and very tipsie.");
   else if ((d >= 0) && (d <= 16))
     str_cpy(drunk, "and intoxicated.");
   else if ((d >= 0) && (d <= 24))
     str_cpy(drunk, "and drunk.");
   else
     str_cpy(drunk, ".");

   sprintf(s,"%s%s%s%s", full, thirst, tired, drunk);
}


/*
 * Produces a string describing alignment.
 */
void naturestr(int x, char *s)
{
   if ((x <= -800) && (x >= -1000))
     sprintf(s, "Lawful Evil");
   else if ((x <= -500) && (x >= -799))
     sprintf(s, "Chaotic Evil");
   else if ((x <= -200) && (x >= -499))
     sprintf(s, "Neutral Evil");
   else if ((x <= -50) && (x >= -199))
     sprintf(s, "Neutral");
   else if ((x <= 49) && (x >= -49))
     sprintf(s, "True Neutral");
   else if ((x <= 199) && (x >= 50))
     sprintf(s, "Neutral");
   else if ((x <= 499) && (x >= 200))
     sprintf(s, "Neutral Good");
   else if ((x <= 799) && (x >= 500))
     sprintf(s, "Chaotic Good");
   else if ((x <= 1000) && (x >= 800))
     sprintf(s, "Lawful Good");
   else /* Unknown */
     sprintf(s, "Neutral");
}

/*
 *  Stuff to do with screen sizing.
 */

#define SIZE (ch->player_specials->screensize)

char *scrpos(int y, int x, struct char_data *ch)
{
    sprintf(specbuf, "\e[%d;%dH",(y+SIZE-24), x);
    return specbuf;
}

char *scrol(int y, int x, struct char_data *ch)
{
    sprintf(specbuf, "\e8\e[%d;%dH",(y+SIZE-24), x);
    return specbuf;
}

char *region(int y1, int y2, struct char_data *ch)
{
    sprintf(specbuf, "\e[?7h\e[%d;%dr", y1, y2+SIZE-24);
    return specbuf;
}

/* definitions for cursor locations and colors */
#define BORDER_COLOR                            CCBLD(ch, C_NRM)
#define TAG_COLOR                               CCBCN(ch, C_NRM)
#define VALUE_COLOR                             CCYEL(ch, C_NRM)
#define NORMAL_COLOR                            CCNRM(ch, C_NRM)

#define OUTPUT_LINE                             scrol(19,1,ch)
#define INPUT_LINE                              scrpos(24,1,ch)

#define ONE_BAR_SCROLL_REGION                   region(1,19,ch)
#define TWO_BAR_SCROLL_REGION                   region(7,19,ch)
#define SCORE_BAR_SCROLL_REGION                 region(7,23,ch)
#define FULL_SCROLL_REGION                      region(1,24,ch)

#define HIT_TAG_POS                             scrpos(21,4,ch)
#define MOV_TAG_POS                             scrpos(21,29,ch)
#define PIE_TAG_POS                             scrpos(21,54,ch)
#define EXITS_TAG_POS                           scrpos(22,4,ch)
#define EXITS_BRACKET_TAG_POS                   scrpos(22,26,ch)
#define ROOMDESC_TAG_POS                        scrpos(22,29,ch)
#define ROOMDESC_BRACKET_TAG_POS                scrpos(22,77,ch)

/* bottom values */
#define HIT_VAL_POS                             "\e[21;8H"
#define MOV_VAL_POS                             "\e[21;33H"
#define PIE_VAL_POS                             "\e[21;58H"
#define EXITS_VAL_POS                           "\e[22;12H"
#define ROOMDESC_VAL_POS                        "\e[22;39H"

#if 0
/* bottom bar tags */
#define HIT_TAG_POS                             "\e[21;4H"
#define MOV_TAG_POS                             "\e[21;29H"
#define PIE_TAG_POS                             "\e[21;54H"
#define EXITS_TAG_POS                           "\e[22;4H"
#define EXITS_BRACKET_TAG_POS                   "\e[22;26H"
#define ROOMDESC_TAG_POS                        "\e[22;29H"
#define ROOMDESC_BRACKET_TAG_POS                "\e[22;77H"

/* bottom values */
#define HIT_VAL_POS                             "\e[21;8H"
#define MOV_VAL_POS                             "\e[21;33H"
#define PIE_VAL_POS                             "\e[21;58H"
#define EXITS_VAL_POS                           "\e[22;12H"
#define ROOMDESC_VAL_POS                        "\e[22;39H"
#endif

/* top tags */
#define COMBAT_TAG_POS                          "\e[1;1H"
#define ALIGN_TAG_POS                           "\e[2;1H"
#define RACE_TAG_POS                            "\e[3;1H"
#define BANK_TAG_POS                            "\e[2;25H"
#define LEVEL_TAG_POS                           "\e[3;25H"
#define PRAC_TAG_POS                            "\e[3;36H"
#define HITBON_TAG_POS                          "\e[1;49H"
#define DAMBON_TAG_POS                          "\e[2;49H"
#define CARRYING_TAG_POS                        "\e[3;49H"
#define EXP_TAG_POS                             "\e[3;1H"

/* top values */
#define COMBAT_VAL_POS                          "\e[1;9H"
#define ALIGN_VAL_POS                           "\e[2;12H"
#define RACE_VAL_POS                            "\e[3;7H"
#define PRA_VAL_POS                             "\e[3;42H"
#define LEV_VAL_POS                             "\e[3;32H"
#define HUNGER_THIRST_VAL_POS                   "\e[4;1H"
#define HITBON_VAL_POS                          "\e[1;60H"
#define DAMBON_VAL_POS                          "\e[2;60H"
#define CARRYING_VAL_POS                        "\e[3;60H"
#define BANK_VAL_POS                            "\e[2;31H"
#define EXP_VAL_POS                             "\e[3;6H"

#define CHECK_SCORE             if (!PRF_FLAGGED(ch, PRF_SCOREBAR)) break

#define IBCH(field)             (ch->infobar.(field))

/*
 * This is the health_meter function that should be called while fighting.
 */
void health_meter(struct char_data *ch, struct char_data *vict)
{
   int a, b;

   if (ch == NULL)
     return; /* this may not be necessary, but just in case... */
   if (IS_NPC(ch) || !ch->desc)
     return;
   if (!PRF_FLAGGED(ch, PRF_METER) || !PRF_FLAGGED(ch, PRF_INFOBAR))
     return;

   ch->infobar.opphit = GET_HIT(vict);

   a = MAX( MIN(GET_MAX_HIT(vict), GET_HIT(vict)) , 0 );
   b = (int) (((float)a/(float)GET_MAX_HIT(vict) * (float)78.0));

   str_cpy(buf, KGRN);
   for (a = 0; a < b; a++)
      strcat(buf,"O"); /* ²"); */
   strcat(buf, KRED);
   for (a = 0; a < (78-b); a++)
      strcat(buf,"X"); /*°"); */
   strcat(buf, KNRM);
   sprintf(buf2, "%s[%s]",scrpos(23,1,ch), buf);
   sprintf(buf2, "%s%s", buf2, INPUT_LINE);

   write_to_output(buf2, ch->desc);
}

/*
 *  This is the big-daddy himself.
 */
ACMD(do_infobar)
{
   int door, n, i, acnum = 0;
   float curr, maxim, percent;
   char *dirname[] = {
     "N", "E", "S", "W", "U", "D", "Ne", "Nw", "Se", "Sw"};
   extern struct dex_app_type dex_app[];

   *buf = '\0';

   if (ch == NULL) {
     log("SYSERR: do_infobar: null character passed.");
     return;
   }
   if (IS_NPC(ch) || !ch->desc) {
     log("SYSERR: do_infobar: NPC passed or ch with NULL desc.");
     return;
   }

   if (!PRF_FLAGGED(ch, PRF_INFOBAR) && !PRF_FLAGGED(ch, PRF_SCOREBAR))
     return;

   switch (subcmd) {
     case SCMDB_CLEAR:
       sprintf(bigbuf,"%s%s",FULL_SCROLL_REGION, CLEAR_SCREEN);
       write_to_output(bigbuf, ch->desc);
       break;
     case SCMDB_REDRAW:
       do_infobar(ch, 0, 0, SCMDB_CLEAR);
         /* bottom bar border */
         sprintf(bigbuf,"%s%s",scrpos(20,1,ch),BORDER_COLOR);
         sprintf(bigbuf,"%s+------------------------------------------------------------------------------+",bigbuf);
         sprintf(bigbuf,"%s%s",bigbuf,scrpos(23,1,ch));
         sprintf(bigbuf,"%s+------------------------------------------------------------------------------+",bigbuf);
         sprintf(bigbuf,"%s%s|",bigbuf,scrpos(21,1,ch));
         sprintf(bigbuf,"%s%s|",bigbuf,scrpos(21,80,ch));
         sprintf(bigbuf,"%s%s|",bigbuf,scrpos(22,1,ch));
         sprintf(bigbuf,"%s%s|",bigbuf,scrpos(22,80,ch));
       /* top bar border */
       if (PRF_FLAGGED(ch, PRF_SCOREBAR)) {
	 sprintf(bigbuf,"%s\e[5;1H",bigbuf);
	 sprintf(bigbuf,"%s+------------------------------------------------------------------------------+",bigbuf);
       }
       write_to_output(bigbuf, ch->desc);
         /* bottom bar tags */
         sprintf(bigbuf,"%s",TAG_COLOR);
         sprintf(bigbuf,"%s%s%s",bigbuf,HIT_TAG_POS,"HP:");
         sprintf(bigbuf,"%s%s%s",bigbuf,MOV_TAG_POS,"MV:");
         sprintf(bigbuf,"%s%s%s",bigbuf,PIE_TAG_POS,"PP:");
         sprintf(bigbuf,"%s%s%s",bigbuf,EXITS_TAG_POS,"Exits: [");
         sprintf(bigbuf,"%s%s%s",bigbuf,EXITS_BRACKET_TAG_POS,"]");
         sprintf(bigbuf,"%s%s%s",bigbuf,ROOMDESC_TAG_POS,"In Room: [");
         sprintf(bigbuf,"%s%s%s",bigbuf,ROOMDESC_BRACKET_TAG_POS,"]");
       if (PRF_FLAGGED(ch, PRF_SCOREBAR)) {
         /* top bar tags */
	 sprintf(bigbuf,"%s%s%s",bigbuf,COMBAT_TAG_POS,"Combat:");
	 sprintf(bigbuf,"%s%s%s",bigbuf,HITBON_TAG_POS,"Hit Bonus:");
	 sprintf(bigbuf,"%s%s%s",bigbuf,ALIGN_TAG_POS, "Alignment:");
	 sprintf(bigbuf,"%s%s%s",bigbuf,BANK_TAG_POS,  "Gold:");
	 sprintf(bigbuf,"%s%s%s",bigbuf,DAMBON_TAG_POS,"Dam Bonus:");
  	 /* sprintf(bigbuf,"%s%s%s",bigbuf,RACE_TAG_POS,  "Race:"); */
	 sprintf(bigbuf,"%s%s%s",bigbuf,LEVEL_TAG_POS, "Level:");
	 sprintf(bigbuf,"%s%s%s",bigbuf,PRAC_TAG_POS,  "Prac:");
	 sprintf(bigbuf,"%s%s%s",bigbuf,CARRYING_TAG_POS,"Carrying :");
	 sprintf(bigbuf,"%s%s%s",bigbuf,EXP_TAG_POS,   "Exp:");
       }
       write_to_output(bigbuf, ch->desc);
       do_infobar(ch, 0, 0, SCMDB_EXITS);
       do_infobar(ch, 0, 0, SCMDB_ROOMDESC);
       do_infobar(ch, 0, 0, SCMDB_EXP);
       do_infobar(ch, 0, 0, SCMDB_AFF);
       do_infobar(ch, 0, 0, SCMDB_AC);
       do_infobar(ch, 0, 0, SCMDB_ALIGN);
       do_infobar(ch, 0, 0, SCMDB_HITBON);
       do_infobar(ch, 0, 0, SCMDB_DAMBON);
       do_infobar(ch, 0, 0, SCMDB_CONDITION);
       do_infobar(ch, 0, 0, SCMDB_RACE);
       do_infobar(ch, 0, 0, SCMDB_GOLD);
       do_infobar(ch, 0, 0, SCMDB_CARRYING);
       do_infobar(ch, 0, 0, SCMDB_HIT);
       do_infobar(ch, 0, 0, SCMDB_MOV);
       do_infobar(ch, 0, 0, SCMDB_MANA);
       do_infobar(ch, 0, 0, SCMDB_WEAR);
       do_infobar(ch, 0, 0, SCMDB_LEVEL);
       do_infobar(ch, 0, 0, SCMDB_PRA);
       do_infobar(ch, 0, 0, SCMDB_TITLE);
       if (PRF_FLAGGED(ch, PRF_SCOREBAR) && PRF_FLAGGED(ch, PRF_INFOBAR))
	 write_to_output(TWO_BAR_SCROLL_REGION, ch->desc);
       else
	 write_to_output(ONE_BAR_SCROLL_REGION, ch->desc);
       write_to_output(OUTPUT_LINE, ch->desc);
       write_to_output(CURSOR_POS_SAVE, ch->desc);
       sprintf(bigbuf, "%s", INPUT_LINE);
#if 0
       if (PRF_FLAGGED(ch, PRF_SCOREBAR))
	 sprintf(bigbuf,"%s", TWO_BAR_SCROLL_REGION);
       else
	 sprintf(bigbuf,"%s", ONE_BAR_SCROLL_REGION);
       sprintf(bigbuf,"%s%s%s%s",bigbuf,OUTPUT_LINE,CURSOR_POS_SAVE,INPUT_LINE);
#endif
       write_to_output(bigbuf, ch->desc);
       break;
     case SCMDB_EXITS:
       if (!PRF_FLAGGED(ch, PRF_INFOBAR))
         break;
       for (door = 0; door < NUM_OF_DIRS; door++) {
	 if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	     (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	      !IS_SET(EXIT(ch, door)->exit_info, EX_SECRET)))
	   sprintf(buf, "%s%s", buf, dirname[door]);
	 else if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	     (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	      !IS_SET(EXIT(ch, door)->exit_info, EX_SECRET)))
	   sprintf(buf, "%s%s%s%s%s", buf, KNRM, KGLD, dirname[door], VALUE_COLOR);
       }
       sprintf(buf2, "%s%s%-14.14s%s%s", VALUE_COLOR, EXITS_VAL_POS, buf, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf2, ch->desc);
       break;
     case SCMDB_ROOMDESC:
       if (!PRF_FLAGGED(ch, PRF_INFOBAR))
         break;
       if (IS_DARK(ch->in_room) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)
	   && !IS_AFFECTED(ch, AFF_INFRAVISION))
	 sprintf(buf,"Too dark too tell.");
       else if (IS_AFFECTED2(ch, AFF_BLIND))
	 sprintf(buf,"You are too BLIND to tell!");
       else
	 sprintf(buf, world[ch->in_room].name, ch);
       sprintf(buf2, "%s%s%-37.37s%s%s", VALUE_COLOR, ROOMDESC_VAL_POS, buf, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf2, ch->desc);
       ch->infobar.inroom = ch->in_room;
       break;
     case SCMDB_EXP:
       CHECK_SCORE;
       sprintf(buf, "%s%s%-12ld%s%s", VALUE_COLOR, EXP_VAL_POS, GET_EXP(ch), NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.exp = GET_EXP(ch);
       break;
     case SCMDB_AFF:
       /* AFFects go here when I get to it */
       break;
     case SCMDB_AC:
       CHECK_SCORE;
       for (i = 0; i < ARMOR_LIMIT; i++)
	  acnum += GET_AC(ch, i);
       acnum /= 100;
       acnum = (acnum + dex_app[GET_DEX(ch)].defensive) >> 1;
       acstr(acnum, buf1);
       sprintf(buf, "%s%s%-37s%s%s", VALUE_COLOR, COMBAT_VAL_POS, buf1, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.ac = acnum;
       break;
     case SCMDB_ALIGN:
       CHECK_SCORE;
       naturestr(GET_ALIGNMENT(ch), buf1);
       sprintf(buf, "%s%s%-11s%s%s", VALUE_COLOR, ALIGN_VAL_POS, buf1, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.align = GET_ALIGNMENT(ch);
       break;
     case SCMDB_HITBON:
       CHECK_SCORE;
       bonstr(GET_HITROLL(ch), buf1);
       sprintf(buf, "%s%s%-16s%s%s", VALUE_COLOR, HITBON_VAL_POS, buf1, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.hitbon = GET_HITROLL(ch);
       break;
     case SCMDB_DAMBON:
       CHECK_SCORE;
       bonstr(GET_DAMROLL(ch), buf1);
       sprintf(buf, "%s%s%-16s%s%s", VALUE_COLOR, DAMBON_VAL_POS, buf1, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.dambon = GET_DAMROLL(ch);
       break;
     case SCMDB_CONDITION:
       CHECK_SCORE;
       hungerstr(GET_COND(ch, FULL), GET_COND(ch, THIRST), GET_COND(ch, DRUNK), GET_COND(ch, TIRED), buf2);
       sprintf(buf, "%s%s%-78.78s%s%s", VALUE_COLOR, HUNGER_THIRST_VAL_POS, buf2, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.thirst = GET_COND(ch, THIRST);
       ch->infobar.hunger = GET_COND(ch, FULL);
       ch->infobar.drunk  = GET_COND(ch, DRUNK);
       ch->infobar.tired  = GET_COND(ch, TIRED);
       break;
     case SCMDB_RACE:
       /*  CHECK_SCORE;
       sprintf(buf2, "%s%s%-16s%s%s", VALUE_COLOR, RACE_VAL_POS, pc_race_types[(int)GET_RACE(ch)], NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf2, ch->desc); */
       ch->infobar.race = GET_RACE(ch);
       break;
     case SCMDB_TITLE:  /* this one is special!! */
       CHECK_SCORE;
       sprintf(buf2, "The Chronicle of %s %s.", GET_NAME(ch), GET_TITLE(ch));
       sprintf(bigbuf, "%s\e[5;%dH%s%s%s", VALUE_COLOR, (int)(40-strlen(buf2)/2), buf2, NORMAL_COLOR, INPUT_LINE);
       write_to_output(bigbuf, ch->desc);
       break;
     case SCMDB_GOLD:
       CHECK_SCORE;
       sprintf(buf, "%s%s%ld%s%s", VALUE_COLOR, BANK_VAL_POS, GET_GOLD(ch), NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.bank = GET_GOLD(ch);
       break;
     case SCMDB_CARRYING:
       CHECK_SCORE;
       sprintf(buf1, "%ld lbs.", IS_CARRYING_W(ch));
       sprintf(buf, "%s%s%-12s%s%s", VALUE_COLOR, CARRYING_VAL_POS, buf1, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.weight = IS_CARRYING_W(ch);
       break;
     case SCMDB_NEWROOM:
       if (!PRF_FLAGGED(ch, PRF_INFOBAR))
         break;
       do_infobar(ch, 0, 0, SCMDB_ROOMDESC);
       do_infobar(ch, 0, 0, SCMDB_EXITS);
       break;
     case SCMDB_GAINLEVEL:
       if (!PRF_FLAGGED(ch, PRF_INFOBAR))
         break;
       do_infobar(ch, 0, 0, SCMDB_HIT);
       do_infobar(ch, 0, 0, SCMDB_MOV);
       do_infobar(ch, 0, 0, SCMDB_MANA);
       do_infobar(ch, 0, 0, SCMDB_PRA);
       break;
     case SCMDB_HIT:
       if (!PRF_FLAGGED(ch, PRF_INFOBAR))
         break;
       *buf1 = '\0';
       curr  = (float) GET_HIT(ch);
       maxim = (float) GET_MAX_HIT(ch);
       percent = (curr/maxim)*100;
       acnum = (int)(percent/5);
       for (i = 0; i < acnum; i++)
	  strcat(buf1, "°");
       sprintf(buf, "%s%s%-20.20s%s%s", HIT_VAL_POS,
	       (acnum >= 10 ? KGRN : (acnum >= 5 ? KYEL : KRED)),
	       buf1, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.hit = GET_HIT(ch);
       break;
     case SCMDB_MOV:
       if (!PRF_FLAGGED(ch, PRF_INFOBAR))
         break;
       *buf1 = '\0';
       curr  = (float) GET_MOVE(ch);
       maxim = (float) GET_MAX_MOVE(ch);
       percent = (curr/maxim)*100;
       acnum = (int)(percent/5);
       for (i = 0; i < acnum; i++)
	  strcat(buf1, "°");
       sprintf(buf, "%s%s%-20.20s%s%s", MOV_VAL_POS,
	       (acnum >= 10 ? KGRN : (acnum >= 5 ? KYEL : KRED)),
	       buf1, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.end = GET_MOVE(ch);
       break;
     case SCMDB_MANA:
       if (!PRF_FLAGGED(ch, PRF_INFOBAR))
         break;
       *buf1 = '\0';
       curr  = (float) GET_MANA(ch);
       maxim = (float) GET_MAX_MANA(ch);
       percent = (curr/maxim)*100;
       acnum = (int)(percent/5);
       for (i = 0; i < acnum; i++)
	  strcat(buf1, "°");
       sprintf(buf, "%s%s%-20.20s%s%s", PIE_VAL_POS,
	       (acnum >= 10 ? KBCN : (acnum >= 5 ? KCYN : KMAG)),
	       buf1, NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.mana = GET_MANA(ch);
       break;
     case SCMDB_LEV:
     case SCMDB_LEVEL:
       CHECK_SCORE;
       sprintf(buf, "%s%s%-2d%s%s", VALUE_COLOR, LEV_VAL_POS, GET_LEVEL(ch), NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.level = GET_LEVEL(ch);
       break;
     case SCMDB_PRA:
       CHECK_SCORE;
       sprintf(buf, "%s%s%-2d%s%s", VALUE_COLOR, PRA_VAL_POS, GET_PRACTICES(ch), NORMAL_COLOR, INPUT_LINE);
       write_to_output(buf, ch->desc);
       ch->infobar.pra = GET_PRACTICES(ch);
       break;
     case SCMDB_WEAR:
       do_infobar(ch, 0, 0, SCMDB_AFF);
       do_infobar(ch, 0, 0, SCMDB_AC);
       do_infobar(ch, 0, 0, SCMDB_ALIGN);
       do_infobar(ch, 0, 0, SCMDB_HITBON);
       do_infobar(ch, 0, 0, SCMDB_DAMBON);
       do_infobar(ch, 0, 0, SCMDB_CARRYING);
       break;
     case SCMDB_GET:
       do_infobar(ch, 0, 0, SCMDB_CARRYING);
       do_infobar(ch, 0, 0, SCMDB_GOLD);
       break;
     case SCMDB_COMBAT:
       do_infobar(ch, 0, 0, SCMDB_HIT);
       do_infobar(ch, 0, 0, SCMDB_MOV);
       do_infobar(ch, 0, 0, SCMDB_MANA);
       break;
     case SCMDB_RESIZE:
       one_argument(argument, arg);
       if (!*arg) {
	 sprintf(buf, "Current screen size is %d.\r\n", SIZE);
	 send_to_char(buf, ch);
	 return;
       }
       n = atoi(arg);
       if (n <= 13)
	 return;
       SIZE = n;
       do_infobar(ch, 0, 0, SCMDB_REDRAW);
       sprintf(buf, "Display resized to %d lines per screen.\r\n", SIZE);
       send_to_char(buf, ch);
       break;
     case SCMDB_GENUPDATE:  /* the NEW update code. =) */
       for (i = 0; i < ARMOR_LIMIT; i++)
	  acnum += GET_AC(ch, i);
       acnum /= 100;
       acnum = (acnum + dex_app[GET_DEX(ch)].defensive) >> 1;
       if (ch->infobar.inroom != ch->in_room)  do_infobar(ch, 0, 0, SCMDB_NEWROOM);
       /* insert exits conditions here */
       if (ch->infobar.exp != GET_EXP(ch)) do_infobar(ch, 0, 0, SCMDB_EXP);
       /* insert affection conditions here*/
       if (ch->infobar.ac != acnum) do_infobar(ch, 0, 0, SCMDB_AC);
       if (ch->infobar.align != GET_ALIGNMENT(ch)) do_infobar(ch, 0, 0, SCMDB_ALIGN);
       /* defense would go here */
       if (ch->infobar.hitbon != GET_HITROLL(ch)) do_infobar(ch, 0, 0, SCMDB_HITBON);
       if (ch->infobar.dambon != GET_DAMROLL(ch)) do_infobar(ch, 0, 0, SCMDB_DAMBON);
       if ((ch->infobar.hunger != GET_COND(ch, FULL)) ||
	   (ch->infobar.thirst != GET_COND(ch, THIRST)) ||
	   (ch->infobar.drunk != GET_COND(ch, DRUNK)) ||
	   (ch->infobar.tired != GET_COND(ch, TIRED))) do_infobar(ch, 0, 0, SCMDB_CONDITION);
       if (ch->infobar.race != GET_RACE(ch)) do_infobar(ch, 0, 0, SCMDB_RACE);
       if (ch->infobar.bank != GET_GOLD(ch)) do_infobar(ch, 0, 0, SCMDB_GOLD);
       if (ch->infobar.weight != IS_CARRYING_W(ch)) do_infobar(ch, 0, 0, SCMDB_CARRYING);
       if (ch->infobar.hit != GET_HIT(ch)) do_infobar(ch, 0, 0, SCMDB_HIT);
       if (ch->infobar.end != GET_MOVE(ch)) do_infobar(ch, 0, 0, SCMDB_MOV);
       if (ch->infobar.mana != GET_MANA(ch)) do_infobar(ch, 0, 0, SCMDB_MANA);
       if (ch->infobar.level != GET_LEVEL(ch)) do_infobar(ch, 0, 0, SCMDB_LEVEL);
       if (ch->infobar.pra != GET_PRACTICES(ch)) do_infobar(ch, 0, 0, SCMDB_PRA);
       break;
     case SCMDB_ENDFIGHT:
       if (!PRF_FLAGGED(ch, PRF_INFOBAR))
         break;
       sprintf(bigbuf,"%s%s",bigbuf,scrpos(23,1,ch));
       sprintf(bigbuf,"%s+------------------------------------------------------------------------------+",bigbuf);
       write_to_output(bigbuf, ch->desc);
       break;
     default:
       break;
   } /* switch */
}


int mobs_nearby(struct char_data *ch)
{
  int door;
  struct char_data *tmp;

  for (tmp = world[ch->in_room].people; tmp; tmp = tmp->next_in_room) {
     if (IS_NPC(tmp))
       return 1;
  }
  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
     for (tmp = world[ch->in_room].people; tmp; tmp = tmp->next_in_room) {
       if (IS_NPC(tmp))
         return 1;
     }
    }
  }
  return 0;
}
