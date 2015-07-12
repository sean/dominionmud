/***************************************************************************
 * MOBProgram ported for CircleMUD 3.0 by Mattias Larsson                  *
 * Traveller@AnotherWorld (ml@eniac.campus.luth.se 4000)                   *
 **************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy........    N'Atas-Ha *
 ***************************************************************************/

#define __MOBCMD_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct index_data *mob_index;
extern struct room_data *world;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct spell_info_type spell_info[];

extern struct index_data *get_mob_index(int vnum);
extern struct index_data *get_obj_index(int vnum);

extern sh_int find_target_room(struct char_data * ch, char *rawroomstr);
ACMD(do_trans);

#define bug(x, y) { sprintf(buf2, (x), (y)); log(buf2); }

/*
 * Local functions.
 */

char *                  mprog_type_to_name      (int type);

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. It allows the words to show up in mpstat to
 *  make it just a hair bit easier to see what a mob should be doing.
 */

char *mprog_type_to_name(int type)
{
    switch (type)
    {
    case IN_FILE_PROG:          return "in_file_prog";
    case ACT_PROG:              return "act_prog";
    case SPEECH_PROG:           return "speech_prog";
    case RAND_PROG:             return "rand_prog";
    case FIGHT_PROG:            return "fight_prog";
    case HITPRCNT_PROG:         return "hitprcnt_prog";
    case DEATH_PROG:            return "death_prog";
    case ENTRY_PROG:            return "entry_prog";
    case GREET_PROG:            return "greet_prog";
    case ALL_GREET_PROG:        return "all_greet_prog";
    case GIVE_PROG:             return "give_prog";
    case BRIBE_PROG:            return "bribe_prog";
    default:                    return "ERROR_PROG";
    }
}

/* string prefix routine */

bool str_prefix(const char *astr, const char *bstr)
{
  if (!astr) {
    log("Strn_cmp: null astr.");
    return TRUE;
  }
  if (!bstr) {
    log("Strn_cmp: null astr.");
    return TRUE;
  }
  for(; *astr; astr++, bstr++) {
    if(LOWER(*astr) != LOWER(*bstr)) return TRUE;
  }
  return FALSE;
}

/* prints the argument to all the rooms aroud the mobile */

ACMD(do_mpasound)
{

  room_num was_in_room;
  int              door;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	bug("Mpasound - No argument: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }
    one_argument(argument, arg);

    was_in_room = ch->in_room;
    for (door = 0; door <= 5; door++)
    {
      struct room_direction_data       *pexit;
      
      if ((pexit = world[was_in_room].dir_option[door]) != NULL
	  && pexit->to_room != NOWHERE
	  && pexit->to_room != was_in_room)
      {
	ch->in_room = pexit->to_room;
	MOBTrigger  = FALSE;
	act(arg, FALSE, ch, NULL, NULL, TO_ROOM);
      }
    }

  ch->in_room = was_in_room;
  return;

}

#define SINFO  spell_info[spellnum]

/* lets the mobile cast spells w/o using mana */
ACMD(do_mpcast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t, tmpbuf[1024];
  int spellnum, i, target = 0;

  if (!IS_NPC(ch))
    return;

  if (isdigit((int)*argument)) {
    /* MOBProg uses spellnum instead of name */
    one_argument(tmpbuf, argument);
    spellnum = atoi(tmpbuf);
  } else {
    /* get: blank, spell name, target name */
    s = strtok(argument, "'");

    if (s == NULL) {
      sprintf(buf, "SYSERR: (MPCAST) %s's MOBProg doesn't have a valid spell!", GET_NAME(ch));
      mudlog(buf, NRM, LVL_GRGOD, TRUE);
      return;
    }
    s = strtok(NULL, "'");
    if (s == NULL) {
      sprintf(buf, "SYSERR: (MPCAST) %s's spell isn't enclosed in '!", GET_NAME(ch));
      mudlog(buf, NRM, LVL_GRGOD, TRUE);
      return;
    }
    t = strtok(NULL, "\0");

    /* spellnum = search_block(s, spells, 0); */
    spellnum = find_skill_num(s);
  }
  /* see if we have a valid spellnum */
  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    sprintf(buf, "SYSERR: (MPCAST) %s has an invalid spell number!", GET_NAME(ch));
    mudlog(buf, NRM, LVL_GRGOD, TRUE);
    return;
  }

  /* Find the target */
  if (t != NULL) {
    one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
  }
  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_room_vis(ch, t)) != NULL)
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	 if (GET_EQ(ch, i) && !str_cmp(t, GET_EQ(ch, i)->name)) {
	   tobj = GET_EQ(ch, i);
	   target = TRUE;
	 }
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t)))
	target = TRUE;
  } else {                      /* if target string is empty */
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the spell isn't violent, default to self */
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
	!SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "SYSERR: (MPCAST) %s's MOBProg has NO TARGET!", GET_NAME(ch));
      mudlog(buf, NRM, LVL_GRGOD, TRUE);
      return;
    }
  }
  if (target && (tch == ch) && SINFO.violent) {
    sprintf(buf, "SYSERR: (MPCAST) %s's MOBProg casts killer spell on ITSELF!", GET_NAME(ch));
    mudlog(buf, NRM, LVL_GRGOD, TRUE);
    return;
  }
  if (!target) {
    sprintf(buf, "SYSERR: (MPCAST) %s's MOBProg has NO TARGET!", GET_NAME(ch));
    mudlog(buf, NRM, LVL_GRGOD, TRUE);
    return;
  }
  if (number(1, 100) < dice(10, 10)) {
    if (!tch || !skill_message(0, ch, tch, spellnum))
      send_to_char("You lost your concentration!\r\n", ch);
    if (SINFO.violent && tch && IS_NPC(tch))
      hit(tch, ch, TYPE_UNDEFINED);
  } else /* cast spell returns 1 on success; subtract mana & set waitstate */
    cast_spell(ch, tch, tobj, spellnum);
  WAIT_STATE(ch, PULSE_VIOLENCE);
}



/* lets the mobile kill any player or mobile without murder*/

ACMD(do_mpkill)
{
    char      arg[ MAX_INPUT_LENGTH ];
    struct char_data *victim;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	bug("MpKill - no argument: vnum %ld.",
		mob_index[ch->nr].vnumber);
	return;
    }

    if ((victim = get_char_room_vis(ch, arg)) == NULL)
    {
	bug("MpKill - Victim not in room: vnum %ld.",
	    mob_index[ch->nr].vnumber);
	return;
    }

    if (victim == ch)
    {
	bug("MpKill - Bad victim to attack: vnum %ld.",
	    mob_index[ch->nr].vnumber);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
    {
	bug("MpKill - Charmed mob attacking master: vnum %ld.",
	    mob_index[ch->nr].vnumber);
	return;
    }

    if (ch->char_specials.position == POS_FIGHTING)
    {
	bug("MpKill - Already fighting: vnum %ld",
	    mob_index[ch->nr].vnumber);
	return;
    }

    hit(ch, victim, -1);
    return;
}


/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy
   items using all.xxxxx or just plain all of them */

ACMD(do_mpjunk)
{
    char      arg[ MAX_INPUT_LENGTH ];
    int pos;
    struct obj_data *obj;
    struct obj_data *obj_next;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	bug("Mpjunk - No argument: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    if (str_cmp(arg, "all") && str_prefix("all.", arg)) {
      if ((obj=get_object_in_equip_vis(ch,arg,ch->equipment,&pos))!= NULL) {
	unequip_char(ch, pos);
	extract_obj(obj);
	return;
      }
      if ((obj = get_obj_in_list_vis(ch, arg, ch->carrying)) != NULL)
	extract_obj(obj);
      return;
    } else {
      for (obj = ch->carrying; obj != NULL; obj = obj_next) {
	obj_next = obj->next_content;
	if (arg[3] == '\0' || isname(arg+4, obj->name)) {
	  extract_obj(obj);
	}
      }
      while((obj=get_object_in_equip_vis(ch,arg,ch->equipment,&pos))!=NULL){
	  unequip_char(ch, pos);
	  extract_obj(obj);
      }   
    }
    return;
}

/* prints the message to everyone in the room other than the mob and victim */

ACMD(do_mpechoaround)
{
  char       arg[ MAX_INPUT_LENGTH ];
  struct char_data *victim;
  char *p;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
       send_to_char("Huh?\n\r", ch);
       return;
    }

    p=one_argument(argument, arg);
    while(isspace((int)*p)) p++; /* skip over leading space */

    if (arg[0] == '\0')
    {
       bug("Mpechoaround - No argument:  vnum %ld.", mob_index[ch->nr].vnumber);
       return;
    }

    if (!(victim=get_char_room_vis(ch, arg)))
    {
	bug("Mpechoaround - victim does not exist: vnum %ld.",
	    mob_index[ch->nr].vnumber);
	return;
    }

    act(p, FALSE, ch, NULL, victim, TO_NOTVICT);
    return;
}

/* prints the message to only the victim */

ACMD(do_mpechoat)
{
  char       arg[ MAX_INPUT_LENGTH ];
  struct char_data *victim;
  char *p;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
       send_to_char("Huh?\n\r", ch);
       return;
    }

    p = one_argument(argument, arg);
    while(isspace((int)*p)) p++; /* skip over leading space */

    if (arg[0] == '\0')
    {
       bug("Mpechoat - No argument:  vnum %ld.",
	   mob_index[ch->nr].vnumber);
       return;
    }

    if (!(victim = get_char_room_vis(ch, arg)))
    {
	bug("Mpechoat - victim does not exist: vnum %ld.",
	    mob_index[ch->nr].vnumber);
	return;
    }

    act(p,FALSE,  ch, NULL, victim, TO_VICT);
    return;
}

/* prints the message to the room at large */

ACMD(do_mpecho)
{
    char *p;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	bug("Mpecho - called w/o argument: vnum %ld.",
	    mob_index[ch->nr].vnumber);
	return;
    }
    p = argument;
    while (isspace((int)*p)) p++;

    act(p,FALSE,  ch, NULL, NULL, TO_ROOM);
    return;

}

/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */

ACMD(do_mpmload)
{
    char            arg[ MAX_INPUT_LENGTH ];
    struct index_data *pMobIndex;
    struct char_data      *victim;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0' || !is_number(arg))
    {
	bug("Mpmload - Bad vnum as arg: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    if ((pMobIndex = get_mob_index(atoi(arg))) == NULL)
    {
	bug("Mpmload - Bad mob vnum: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    victim = read_mobile(atoi(arg), VNUMBER);
    char_to_room(victim, ch->in_room);
    return;
}

ACMD(do_mpoload)
{
    char arg1[ MAX_INPUT_LENGTH ];
    struct index_data *pObjIndex;
    struct obj_data       *obj;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg1);
 
    if (arg1[0] == '\0' || !is_number(arg1))
    {
	bug("Mpoload - Bad syntax: vnum %ld.",
	    mob_index[ch->nr].vnumber);
	return;
    }

    if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL)
    {
	bug("Mpoload - Bad vnum arg: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    obj = read_object(atoi(arg1), VNUMBER);
    if (obj == NULL)
       return;
    if (CAN_WEAR(obj, ITEM_WEAR_TAKE)) {
	obj_to_char(obj, ch);
    } else {
	obj_to_room(obj, ch->in_room);
    }

    return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MOBprogram
   otherwise ugly stuff will happen */

ACMD(do_mppurge)
{
    struct char_data *victim;
    struct obj_data  *obj;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	/* 'purge' */
	struct char_data *vnext;
	struct obj_data  *obj_next;

	for (victim = world[ch->in_room].people; victim != NULL; victim = vnext)
	{
	  vnext = victim->next_in_room;
	  if (IS_NPC(victim) && victim != ch)
	    extract_char(victim);
	}

	for (obj = world[ch->in_room].contents; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  extract_obj(obj);
	}

	return;
    }

    if (!(victim = get_char_room_vis(ch, arg)))
    {
	if ((obj = get_obj_vis(ch, arg))) {
	    extract_obj(obj);
	} else {
	    bug("Mppurge - Bad argument: vnum %ld.",mob_index[ch->nr].vnumber);
	}
	return;
    }

    if (!IS_NPC(victim))
    {
	bug("Mppurge - Purging a PC: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    extract_char(victim);
    return;
}


/* lets the mobile goto any location it wishes that is not private */

ACMD(do_mpgoto)
{
    char             arg[ MAX_INPUT_LENGTH ];
    sh_int location;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
	bug("Mpgoto - No argument: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    if ((location = find_target_room(ch, arg)) < 0)
    {
	bug("Mpgoto - No such location: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    if (FIGHTING(ch) != NULL)
	stop_fighting(ch);

    char_from_room(ch);
    char_to_room(ch, location);

    return;
}

/* lets the mobile do a command at another location. Very useful */

ACMD(do_mpat)
{
    char             arg[ MAX_INPUT_LENGTH ];
    sh_int location;
    sh_int original;
/*    struct char_data       *wch; */

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	bug("Mpat - Bad argument: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    if ((location = find_target_room(ch, arg)) < 0)
    {
	bug("Mpat - No such location: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    original = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    command_interpreter(ch, argument);

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    if(ch->in_room == location) {
      char_from_room(ch);
      char_to_room(ch, original);
    }

    return;
}
 
/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location */

ACMD(do_mptransfer)
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    sh_int location;
    struct descriptor_data *d;
    struct char_data       *victim;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
	bug("Mptransfer - Bad syntax: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    if (!str_cmp(arg1, "all"))
    {
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NOWHERE
	    &&   CAN_SEE(ch, d->character))
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "%s %s", d->character->player.name, arg2);
		do_trans(ch, buf, cmd, 0);
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if (arg2[0] == '\0')
    {
	location = ch->in_room;
    }
    else
    {
	if ((location = find_target_room(ch, arg2)) < 0)
	{
	    bug("Mptransfer - No such location: vnum %ld.",
		mob_index[ch->nr].vnumber);
	    return;
	}

	if (IS_SET(world[location].room_flags, ROOM_PRIVATE))
	{
	    bug("Mptransfer - Private room: vnum %ld.",
		mob_index[ch->nr].vnumber);
	    return;
	}
    }

    if ((victim = get_char_vis(ch, arg1)) == NULL)
    {
	bug("Mptransfer - No such person: vnum %ld.",
	    mob_index[ch->nr].vnumber);
	return;
    }

    if (victim->in_room == 0)
    {
	bug("Mptransfer - Victim in Limbo: vnum %ld.",
	    mob_index[ch->nr].vnumber);
	return;
    }

    if (FIGHTING(victim) != NULL)
	stop_fighting(victim);

    char_from_room(victim);
    char_to_room(victim, location);

    return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */

ACMD(do_mpforce)
{
    char arg[ MAX_INPUT_LENGTH ];

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	bug("Mpforce - Bad syntax: vnum %ld.", mob_index[ch->nr].vnumber);
	return;
    }

    if (!str_cmp(arg, "all")) {
	struct descriptor_data *i;
	struct char_data *vch;

	for (i = descriptor_list; i ; i = i->next) {
	  if(i->character != ch && !i->connected &&
	     i->character->in_room == ch->in_room) {
	    vch = i->character;
	    if(GET_LEVEL(vch) < GET_LEVEL(ch) && CAN_SEE(ch, vch)) {
		command_interpreter(vch, argument);
	    }
	  }
	}
    } else {
	struct char_data *victim;

	if ((victim = get_char_room_vis(ch, arg)) == NULL) {
	    bug("Mpforce - No such victim: vnum %ld.",
		mob_index[ch->nr].vnumber);
	    return;
	}

	if (victim == ch) {
	    bug("Mpforce - Forcing oneself: vnum %ld.",
		mob_index[ch->nr].vnumber);
	    return;
	}

	command_interpreter(victim, argument);
    }

    return;
}
