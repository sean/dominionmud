/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  ACT_COMM.C                                   Based on CircleMUD *
*  Usage: Player-level communication commands                             *
*  Programmer(s): Original work by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
*                 For The Dominion.                                       *
\*********************************************************************** */

#define __ACT_COMM_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
/* New for TD 5/24/95 */
extern struct race_data * races;
extern struct religion_data * religions;
extern void mprog_speech_trigger(char *txt, struct char_data *mob);
extern int  top_of_p_table;
extern struct player_index_element *player_table;
extern int NUM_RACES;

char *RandomWord(void)
{
  static char *string[50] = {
    "argle",
    "bargle",
    "glop",
    "glyph",
    "hussamah",  /* 5 */
    "rodina",
    "mustafah",
    "angina",
    "the",
    "fribble",  /* 10 */
    "fnort",
    "frobozz",
    "zarp",
    "ripple",
    "yrk",    /* 15 */
    "yid",
    "yerf",
    "oork",
    "grapple",
    "red",   /* 20 */
    "blue",
    "you",
    "me",
    "ftagn",
    "hastur",   /* 25 */
    "brob",
    "gnort",
    "lram",
    "truck",
    "kill",    /* 30 */
    "cthulhu",
    "huzzah",
    "acetacytacylic",
    "hydrooxypropyl",
    "summah",     /* 35 */
    "hummah",
    "cookies",
    "stan",
    "will",
    "wadapatang",   /* 40 */
    "pterodactyl",
    "frob",
    "yuma",
    "gumma",
    "lo-pan",   /* 45 */
    "sushi",
    "yaya",
    "yoyodine",
    "phooey",
    "tralala"   /* 50 */
  };
  return(string[number(0,49)]);
}


ACMD(do_sign)
{
  struct char_data *targ;
  int last = 0, i = 0;
  char string[256];

  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes, but WHAT do you want to sign?\r\n", ch);
  else if (!IS_HUMANOID(ch))
    send_to_char("Yeah, right with what hands?\r\n", ch);
  else {
    /* mess up the string a bit depending on the signer */
    while (argument[last]) {
      if (GET_SKILL(ch, SKILL_SIGN) > number(1, 101)) {
	for (i = last; argument[i] != ' ' && argument[i] != '\0' && i < 256; i++)
	   string[i] = argument[i];
	i++;
	string[i++] = ' ';
	last = i;
       } else {
	for (i = last; argument[i] != ' ' && argument[i] != '\0' && i < 256; i++) ;
	strcat(string, RandomWord());
	strcat(string, " ");
	last = ++i;
       }
    }
    string[i] = '\0';
    for (targ = world[ch->in_room].people; targ; targ = targ->next_in_room)
       if (targ != ch) {
	 if (GET_SKILL(targ, SKILL_SIGN) < number(10, 20))
	   act("$n makes funny motions with $s hands.", TRUE, ch, 0, targ, TO_VICT);
	 else {
	   sprintf(buf, "$n signs, '%s'", string);
	   act(buf, FALSE, ch, 0, targ, TO_VICT);
	 }
       }
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      /* Heh the signer thinks he/she signed it right ;) */
      sprintf(buf, "You sign, '%s'", argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    }
  }
}


ACMD(do_say)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
  else if (IS_AFFECTED2(ch, AFF_SILENCE))
    send_to_char("Your throat is too tight to get the words out.\r\n", ch);
  else {
    sprintf(buf, "$n says, '%s'", argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You say, '%s'", argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    }
  /* this was if YOU say it... */
  mprog_speech_trigger(argument, ch);
  }
}


ACMD(do_gsay)
{
  struct char_data *k;
  struct follow_type *f;

  skip_spaces(&argument);

  if (!IS_AFFECTED2(ch, AFF_GROUP)) {
    send_to_char("But you are not the member of a group!\r\n", ch);
    return;
  }
  if (!*argument)
    send_to_char("Yes, but WHAT do you want to group-say?\r\n", ch);
  else if (IS_AFFECTED2(ch, AFF_SILENCE))
    send_to_char("Your throat is too tight to get the words out.\r\n", ch);
  else {
    if (ch->master)
      k = ch->master;
    else
      k = ch;

    sprintf(buf, "$n tells the group, '%s'", argument);

    if (IS_AFFECTED2(k, AFF_GROUP) && (k != ch))
      act(buf, FALSE, ch, 0, k, TO_VICT | TO_SLEEP);
    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED2(f->follower, AFF_GROUP) && (f->follower != ch))
	act(buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP);

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You tell the group, '%s'", argument);
      act(buf, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
    }
  mprog_speech_trigger(argument, ch);
  }
}


void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
  send_to_char(CCBRD(vict, C_NRM), vict);
  sprintf(buf, "$n tells you, '%s'", arg);
  act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(vict, C_NRM), vict);

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    send_to_char(CCBRD(ch, C_CMP), ch);
    sprintf(buf, "You tell $N, '%s'", arg);
    act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    send_to_char(CCNRM(ch, C_CMP), ch);
  }
  GET_LAST_TELL(vict) = GET_IDNUM(ch);
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
  struct char_data *vict;

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char("Who do you wish to tell what??\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf)) && !IS_NPC(ch))
    send_to_char(NOPERSON, ch);
  else if (vict == NULL)
    send_to_char(NOPERSON, ch);
  else if (ch == vict)
    send_to_char("You try to tell yourself something.\r\n", ch);
  else if (PRF_FLAGGED(ch, PRF_NOTELL))
    send_to_char("You can't tell other people while you have notell on.\r\n", ch);
  else if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
    send_to_char("The walls seem to absorb your words.\r\n", ch);
  else if (!IS_NPC(vict) && !vict->desc)        /* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now; try again later.",
	FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PRF_FLAGGED(vict, PRF_NOTELL) || ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF))
    act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (IS_AFFECTED2(ch, AFF_SILENCE))
    send_to_char("Your throat is too tight to get the words out.\r\n", ch);
  else
    perform_tell(ch, vict, buf2);
}


ACMD(do_reply)
{
  struct char_data *tch = character_list;

  skip_spaces(&argument);

  if (GET_LAST_TELL(ch) == NOBODY)
    send_to_char("You have no-one to reply to!\r\n", ch);
  else if (!*argument)
    send_to_char("What is your reply?\r\n", ch);
  else if (IS_AFFECTED2(ch, AFF_SILENCE))
    send_to_char("Your throat is too tight to get the words out.\r\n", ch);
  else {
    /*
     * Make sure the person you're replying to is still playing by searching
     * for them.  Note, now last tell is stored as player IDnum instead of
     * a pointer, which is much better because it's safer, plus will still
     * work if someone logs out and back in again.
     */

    while (tch != NULL && GET_IDNUM(tch) != GET_LAST_TELL(ch))
      tch = tch->next;

    if (tch == NULL)
      send_to_char("They are no longer playing.\r\n", ch);
    else
      perform_tell(ch, tch, argument);
  }
}


ACMD(do_spec_comm)
{
  struct char_data *vict;
  char *action_sing, *action_plur, *action_others;

  if (subcmd == SCMD_WHISPER) {
    action_sing = "whisper to";
    action_plur = "whispers to";
    action_others = "$n whispers something to $N.";
  } else {
    action_sing = "ask";
    action_plur = "asks";
    action_others = "$n asks $N a question.";
  }

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2) {
    sprintf(buf, "Whom do you want to %s.. and what??\r\n", action_sing);
    send_to_char(buf, ch);
  } else if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (vict == ch)
    send_to_char("You can't get your mouth close enough to your ear...\r\n", ch);
  else if (IS_AFFECTED2(ch, AFF_SILENCE))
    send_to_char("Your throat is too tight to get the words out.\r\n", ch);
  else {
    sprintf(buf, "$n %s you, '%s'", action_plur, buf2);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    if (IS_NPC(vict))
      mprog_speech_trigger(buf2, vict);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You %s %s, '%s'\r\n", action_sing, GET_NAME(vict), buf2);
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
    act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);
  }
}


#define MAX_NOTE_LENGTH 1000    /* arbitrary */

ACMD(do_write)
{
  struct obj_data *paper = 0, *pen = 0;
  char *papername, *penname;

  papername = buf1;
  penname = buf2;

  two_arguments(argument, papername, penname);

  if (!ch->desc)
    return;

  if (!*papername) {            /* nothing was delivered */
    send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
    return;
  }
  if (*penname) {               /* there were two arguments */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", penname);
      send_to_char(buf, ch);
      return;
    }
  } else {              /* there was one arg.. let's see what we can find */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "There is no %s in your inventory.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(paper) == ITEM_PEN) {      /* oops, a pen.. */
      pen = paper;
      paper = 0;
    } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
      send_to_char("That thing has nothing to do with writing.\r\n", ch);
      return;
    }
    /* One object was found.. now for the other one. */
    if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_HOLD)) {
      sprintf(buf, "You can't write with %s %s alone.\r\n", AN(papername),
	      papername);
      send_to_char(buf, ch);
      return;
    }
    if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_WIELD)) &&
	!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD))) {
      send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
      return;
    }
    if (pen) {
      if (GET_EQ(ch, WEAR_HOLD) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_NOTE)
	paper = GET_EQ(ch, WEAR_HOLD);
      else
	paper = GET_EQ(ch, WEAR_WIELD);
    } else {
      if (GET_EQ(ch, WEAR_HOLD) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_PEN)
	pen = GET_EQ(ch, WEAR_HOLD);
      else
	pen = GET_EQ(ch, WEAR_WIELD);
    }
  }

  /* ok.. now let's see what kind of stuff we've found */
  if (GET_OBJ_TYPE(pen) != ITEM_PEN)
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  else if (paper->action_description)
    send_to_char("There's something written on it already.\r\n", ch);
  else {
    /* we can write - hooray! */
    send_to_char("Write your note.  End with '@' on a new line.\r\n", ch);
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}



ACMD(do_page)
{
  struct descriptor_data *d;
  struct char_data *vict;

  half_chop(argument, arg, buf2);

  if (IS_NPC(ch))
    send_to_char("Monsters can't page.. go away.\r\n", ch);
  else if (!*arg)
    send_to_char("Whom do you wish to page?\r\n", ch);
  else {
    sprintf(buf, "*%s*\007\007 %s\007\007\r\n", GET_NAME(ch), buf2);
    if (!str_cmp(arg, "all")) {
      if (GET_LEVEL(ch) > LVL_GOD) {
	for (d = descriptor_list; d; d = d->next)
	  if (!d->connected && d->character)
	    act(buf, FALSE, ch, 0, d->character, TO_VICT);
      } else
	send_to_char("You will never be godly enough to do that!\r\n", ch);
      return;
    }
    if ((vict = get_char_vis(ch, arg)) != NULL) {
      act(buf, FALSE, ch, 0, vict, TO_VICT);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
	send_to_char(OK, ch);
      else
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
      return;
    } else
      send_to_char("There is no such person in the game!\r\n", ch);
  }
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm)
{
  extern int level_can_shout;
  extern int holler_move_cost;
  struct descriptor_data *i;
  char   color_on[24];

  /* Array of flags which must _not_ be set in order for comm to be heard */
  static int channels[] = {
    0,
    PRF_DEAF,
    PRF_NOGOSS,
    0,
    PRF_NOOOC,
    0
  };

  /*
   * com_msgs: [0] Message if you can't perform the action because of noshout
   *           [1] name of the action
   *           [2] message if you're not on the channel
   *           [3] a color string.
   */
  static char *com_msgs[][4] = {
    {"You cannot holler!!\r\n",
      "holler",
      "",
    KYEL},

    {"You cannot shout!!\r\n",
      "shout",
      "Turn off your noshout flag first!\r\n",
    KYEL},

    {"You cannot gossip!!\r\n",
      "gossip",
      "You aren't even on the channel!\r\n",
    KCYN},

    {"You cannot wish!!\r\n",
     "wish",
     "You aren't even on that channel!\r\n",
    KMAG},

    {"You cannot OOC Chat!\r\n",
      "OOC",
      "You aren't even on the channel!\r\n",
    KGRN}
  };

  /* to keep pets, etc from being ordered to shout */
  if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    return;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char(com_msgs[subcmd][0], ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF)) {
    send_to_char("The walls seem to absorb your words.\r\n", ch);
    return;
  }
  if (IS_AFFECTED2(ch, AFF_SILENCE)) {
    send_to_char("Your throat is too tight to get the words out.\r\n", ch);
    return;
  }
  if (FIGHTING(ch) && subcmd != SCMD_SHOUT) {
    sprintf(buf, "You can't %s during combat, it's too distracting!\r\n",
		com_msgs[subcmd][2]);
    send_to_char(buf, ch);
    return;
  }
  /* level_can_shout defined in config.c */
  if ((GET_LEVEL(ch) < level_can_shout) && !IS_NPC(ch)) {
    sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
	    level_can_shout, com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }
  /* make sure the char is on the channel */
  if (PRF_FLAGGED(ch, channels[subcmd])) {
    send_to_char(com_msgs[subcmd][2], ch);
    return;
  }
  /* skip leading spaces */
  skip_spaces(&argument);

  /* make sure that there is something there to say! */
  if (!*argument) {
    sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT???\r\n",
	    com_msgs[subcmd][1], com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }
  if (subcmd == SCMD_HOLLER) {
    if (GET_MOVE(ch) < holler_move_cost) {
      send_to_char("You're too exhausted to holler.\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= holler_move_cost;
  }
  /* set up the color on code */
  strcpy(color_on, com_msgs[subcmd][3]);

  /* first, set up strings to be given to the communicator */
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    if (COLOR_LEV(ch) >= C_CMP)
      sprintf(buf1, "%sYou %s, '%s'%s", color_on, com_msgs[subcmd][1],
	      argument, KNRM);
    else
      sprintf(buf1, "You %s, '%s'", com_msgs[subcmd][1], argument);
    act(buf1, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
  }

  sprintf(buf, "$n %ss, '%s'", com_msgs[subcmd][1], argument);

  /* now send all the strings out */
  for (i = descriptor_list; i; i = i->next) {
    if (!i->connected && i != ch->desc && i->character &&
	!PRF_FLAGGED(i->character, channels[subcmd]) &&
	!PLR_FLAGGED(i->character, PLR_WRITING) &&
	!ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {

      if ((subcmd == SCMD_SHOUT || (subcmd == SCMD_GOSSIP && GET_MAX_LEVEL(ch) < LVL_IMMORT)) &&
	  ((world[ch->in_room].zone != world[i->character->in_room].zone) ||
	   GET_POS(i->character) < POS_RESTING))
	continue;

      if ((subcmd == SCMD_WISH) && (GET_MAX_LEVEL(i->character) < LVL_IMMORT))
	 continue;

      if (COLOR_LEV(i->character) >= C_NRM)
	send_to_char(color_on, i->character);
      act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      if (COLOR_LEV(i->character) >= C_NRM)
	send_to_char(KNRM, i->character);
      if (subcmd == SCMD_OOC && !IS_IMMORT(ch))
	WAIT_STATE(ch, 5 * PULSE_VIOLENCE);
    }
  }
}


ACMD(do_qcomm)
{
  struct descriptor_data *i;

  if (!PRF_FLAGGED(ch, PRF_QUEST)) {
    send_to_char("You aren't even part of the quest!\r\n", ch);
    return;
  }
  if (IS_AFFECTED2(ch, AFF_SILENCE)) {
    send_to_char("Your throat is too tight to get the words out.\r\n", ch);
    return;
  }

  skip_spaces(&argument);

  if (!*argument) {
    sprintf(buf, "%s?  Yes, fine, %s we must, but WHAT??\r\n", CMD_NAME,
	    CMD_NAME);
    CAP(buf);
    send_to_char(buf, ch);
  } else {
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      if (subcmd == SCMD_QSAY)
	sprintf(buf, "You quest-say, '%s'", argument);
      else
	strcpy(buf, argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    }

    if (subcmd == SCMD_QSAY)
      sprintf(buf, "$n quest-says, '%s'", argument);
    else
      strcpy(buf, argument);

    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i != ch->desc &&
	  PRF_FLAGGED(i->character, PRF_QUEST))
	act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
  }
}

