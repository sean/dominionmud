/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SPELL_PA.C                                   Based on CircleMUD *
*  Usage: Top-level magic routines; outside points of entry to magic sys. *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All Modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */


#include "conf.h"
#include "sysdep.h"

#include "protos.h"

struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];

#define SINFO spell_info[spellnum]

extern struct room_data *world;

/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, this should provide
 * ample slots for skills.
 */

/*
 * The names of the spheres, spells and skills, indexed by number, read
 * from lib/data/spells by load_spells().  Unused numbers are "!UNUSED!",
 * and the list ends with "\n" just after the highest number in use.
 */
char *spells[TOP_SPELL_DEFINE + 2];


struct syllable {
  char *org;
  char *newp;
};


struct syllable syls[] = {
  {" ", " "},
  {"ar", "abra"},
  {"ate", "i"},
  {"cau", "kada"},
  {"blind", "nose"},
  {"bur", "mosa"},
  {"cu", "judi"},
  {"de", "oculo"},
  {"dis", "mar"},
  {"ect", "kamina"},
  {"en", "uns"},
  {"gro", "cra"},
  {"light", "dies"},
  {"lo", "hi"},
  {"magi", "kari"},
  {"mon", "bar"},
  {"mor", "zak"},
  {"move", "sido"},
  {"ness", "lacri"},
  {"ning", "illa"},
  {"per", "duda"},
  {"ra", "gru"},
  {"re", "candus"},
  {"son", "sabru"},
  {"tect", "infra"},
  {"tri", "cula"},
  {"ven", "nofo"},
  {"word of", "inset"},
  {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"}, {"g", "t"},
  {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"}, {"n", "b"},
  {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "e"},
  {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}
};

int mag_manacost(struct char_data * ch, int spellnum)
{
  int mana;

  mana = MAX((int)SINFO.mana_max - (SINFO.mana_change *
		(GET_SKILL(ch, spellnum) - (SINFO.percent/10))),
		(int)SINFO.mana_min);

  return mana;
}


/* say_spell erodes buf, buf1, buf2 */
void say_spell(struct char_data * ch, int spellnum, struct char_data * tch,
		    struct obj_data * tobj)
{
  char lbuf[256];

  struct char_data *i;
  int j, ofs = 0;

  *buf = '\0';
  strcpy(lbuf, spells[spellnum]);

  while (*(lbuf + ofs)) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].newp);
	ofs += strlen(syls[j].org);
      }
    }
  }

  if (tch != NULL && tch->in_room == ch->in_room) {
    if (tch == ch)
      sprintf(lbuf, "$n closes $s eyes and utters the words, '%%s'.");
    else
      sprintf(lbuf, "$n stares at $N and utters the words, '%%s'.");
  } else if (tobj != NULL &&
	     ((tobj->in_room == ch->in_room) || (tobj->carried_by == ch)))
    sprintf(lbuf, "$n stares at $p and utters the words, '%%s'.");
  else
    sprintf(lbuf, "$n utters the words, '%%s'.");

  sprintf(buf1, lbuf, spells[spellnum]);
  sprintf(buf2, lbuf, buf);

  for (i = world[ch->in_room].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i))
      continue;
    if (GET_SKILL(i, spellnum) > 0)
       perform_act(buf1, ch, tobj, tch, i);
    else
       perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && tch->in_room == ch->in_room) {
    sprintf(buf1, "$n stares at you and utters the words, '%s'.",
	    GET_SKILL(tch, spellnum) > 0 ? spells[spellnum] : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}

int skill_lookup( const char * name )
{
  int i;
  for ( i = 0; *spells[i] != '\n'; i++ )
    if ( !strcmp( name, spells[i] ) )
      return i;
  // should return index for UNDEFINED
  return -1;
}

char *skill_name(int num)
{
  int i = 0;

  if (num <= 0)
    return "UNDEFINED";

  while (num && *spells[i] != '\n') {
    num--;
    i++;
  }

  return spells[i];
}


int find_skill_num(char *name)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*spells[++index] != '\n') {
    if (is_abbrev(name, spells[index]) || isname(name, spells[index]))
      return index;

    ok = 1;
    temp  = any_one_arg(spells[index], first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
	ok = 0;
      temp  = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return index;
  }

  return -1;
}



/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(struct char_data * caster, struct char_data * cvict,
	     struct obj_data * ovict, int spellnum, int level, int casttype)
{
  int savetype;

  if (spellnum < 51 || spellnum > TOP_SPELL_DEFINE)
    return 0;

  if (ROOM_FLAGGED(caster->in_room, ROOM_NOMAGIC)) {
    send_to_char("Your magic fizzles out and dies.\r\n", caster);
    act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
    return 0;
  }
  if (IS_SET(ROOM_FLAGS(caster->in_room), ROOM_PEACEFUL) &&
      (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))) {
    send_to_char("A flash of white light fills the room, dispelling your "
		 "violent magic!\r\n", caster);
    act("White light from no particular source suddenly fills the room, "
	"then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
    return 0;
  }
  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }

  if (IS_SET(SINFO.routines, MAG_DAMAGE))
    mag_damage(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AFFECTS))
    mag_affects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_MANUAL))
    switch (spellnum) {
    case SPELL_CHARM:           MANUAL_SPELL(spell_charm);          break;
    case SPELL_HYPNOSIS:        MANUAL_SPELL(spell_charm);          break;
    case SPELL_CREATE_WATER:    MANUAL_SPELL(spell_create_water);   break;
    case SPELL_DETECT_POISON:   MANUAL_SPELL(spell_detect_poison);  break;
    case SPELL_ENCHANT_WEAPON:  MANUAL_SPELL(spell_enchant_weapon); break;
    case SPELL_IDENTIFY:        MANUAL_SPELL(spell_identify);       break;
    case SPELL_LOCATE_OBJECT:   MANUAL_SPELL(spell_locate_object);  break;
    case SPELL_SUMMON:          MANUAL_SPELL(spell_summon);         break;
    case SPELL_WORD_OF_RECALL:  MANUAL_SPELL(spell_recall); 	    break;
    case SPELL_PORTAL:          MANUAL_SPELL(spell_portal);         break;
    case SPELL_ASTRAL_WALK:     MANUAL_SPELL(spell_astral_walk);    break;
    case SPELL_DISPEL_MAGIC:    MANUAL_SPELL(spell_dispel_magic);   break;
    case SPELL_ENCHANT_ARMOR:   MANUAL_SPELL(spell_enchant_armor);  break;
    case SPELL_CURSE_ARMOR:     MANUAL_SPELL(spell_curse_armor);    break;
    case SPELL_CURSE_WEAPON:    MANUAL_SPELL(spell_curse_weapon);   break;
    case SPELL_WIZARD_EYE:      MANUAL_SPELL(spell_wizard_eye);     break;
    case SPELL_TELEPORT:        MANUAL_SPELL(spell_teleport);       break;
    case SPELL_CONTROL_WEATHER: MANUAL_SPELL(spell_control_weather); break;
    case SPELL_PURIFY:          MANUAL_SPELL(spell_purify);         break;
    case SPELL_CORRUPT:         MANUAL_SPELL(spell_corrupt);        break;
    case SPELL_BALANCE:         MANUAL_SPELL(spell_balance);        break;
    case SPELL_BLADEBARRIER:    MANUAL_SPELL(spell_blade_barrier);  break;
    case SPELL_CALM:            MANUAL_SPELL(spell_calm);           break;
    }
  return 1;
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.
 *
 * staff  - [0] level   [1] max charges [2] num charges [3] spell num
 * wand   - [0] level   [1] max charges [2] num charges [3] spell num
 * scroll - [0] level   [1] spell num   [2] spell num   [3] spell num
 * potion - [0] level   [1] spell num   [2] spell num   [3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified.
 */

void mag_objectmagic(struct char_data * ch, struct obj_data * obj,
			  char *argument)
{
  int i, k;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_STAFF:
    act("You raise $p aloft and feel power rush through you.", FALSE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
    else
      act("$n raises $p aloft and begins to glow with power.", FALSE, ch, obj, 0, TO_ROOM);

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      GET_OBJ_VAL(obj, 2)--;
      WAIT_STATE(ch, PULSE_VIOLENCE);
      for (tch = world[ch->in_room].people; tch; tch = next_tch) {
	next_tch = tch->next_in_room;
	if (ch == tch)
	  continue;
	if (GET_OBJ_VAL(obj, 0))
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
		     GET_OBJ_VAL(obj, 0), CAST_STAFF);
	else
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
		     DEFAULT_STAFF_LVL, CAST_STAFF);
      }
    }
    break;
  case ITEM_WAND:
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
      } else {
	act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
	if (obj->action_description != NULL)
	  act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
	else
	  act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
      }
    } else if (tobj != NULL) {
      act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
      if (obj->action_description != NULL)
	act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      else
	act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
    } else {
      act("At what should $p be pointed?", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
      return;
    }
    GET_OBJ_VAL(obj, 2)--;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (GET_OBJ_VAL(obj, 0))
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 GET_OBJ_VAL(obj, 0), CAST_WAND);
    else
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 DEFAULT_WAND_LVL, CAST_WAND);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
	act("There is nothing to here to affect with $p.", FALSE,
	    ch, obj, NULL, TO_CHAR);
	return;
      }
    } else
      tch = ch;

    act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_SCROLL)))
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  case ITEM_POTION:
    tch = ch;
    act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_POTION)))
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  default:
    log("SYSERR: Unknown object_type in mag_objectmagic");
    break;
  }
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */

int cast_spell(struct char_data * ch, struct char_data * tch,
		   struct obj_data * tobj, int spellnum)
{
  if (GET_POS(ch) < SINFO.min_position) {
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
      send_to_char("You dream about great magical powers.\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("You cannot concentrate while resting.\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("You can't do this sitting!\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
      break;
    default:
      send_to_char("You can't do much of anything like this!\r\n", ch);
      break;
    }
    return 0;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tch)) {
    send_to_char("You are afraid you might hurt your master!\r\n", ch);
    return 0;
  }
  if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
    send_to_char("You can only cast this spell upon yourself!\r\n", ch);
    return 0;
  }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {
    send_to_char("You cannot cast this spell upon yourself!\r\n", ch);
    return 0;
  }
  if (IS_SET(SINFO.routines, MAG_GROUPS) && !IS_AFFECTED2(ch, AFF_GROUP)) {
    send_to_char("You can't cast this spell if you're not in a group!\r\n",ch);
    return 0;
  }
  send_to_char(OK, ch);
  say_spell(ch, spellnum, tch, tobj);

  return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL));
}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */
ACMD(do_cast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, spellnum, i, target = 0, sphere;

  bool religion_out_of_sync( struct char_data * ch );
  
  if (IS_NPC(ch))
    return;

  if (IS_AFFECTED2(ch, AFF_SILENCE)) {
    send_to_char("You are unable to conjure the words!\r\n", ch);
    return;
  }
  /* get: blank, spell name, target name */
  s = strtok(argument, "'");

  if (s == NULL) {
    send_to_char("Cast what where?\r\n", ch);
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Spell names must be enclosed in the Holy Magic Symbols: '\r\n", ch);
    return;
  }
  t = strtok(NULL, "\0");

  /* spellnum = search_block(s, spells, 0); */
  spellnum = find_skill_num(s);

  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    send_to_char("Cast what?!?\r\n", ch);
    return;
  }

  /* check if it is a prayer or spell */
  if (IS_PRAYER(spellnum)) {
    if (!CMD_IS("pray")) {
      send_to_char("Priestly orders do not cast spells!  They pray to their diety!\r\n", ch);
      return;
    }

    if (GET_REL(ch) == REL_UNDEFINED) {
      send_to_char("But you have no god to call upon for aid!\r\n", ch);
      return;
    }

    if ( religion_out_of_sync( ch ) ) {
      sprintf(buf, "You have strayed from the path of %s's religion.\r\n",
	      relg_name(ch));
      send_to_char(buf, ch);
      send_to_char("Your prayer fails!\r\n",ch);
      return;
    }
  }
  sphere = SINFO.sphere;

  if (GET_SKILL(ch, sphere) < SINFO.percent) {
    if (IS_PRAYER(spellnum))
      send_to_char("You do not know that prayer!\r\n", ch);
    else
      send_to_char("You do not know that spell!\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, spellnum) <= 0) {
    if (IS_PRAYER(spellnum))
      send_to_char("You are unfamiliar with that prayer.\r\n", ch);
    else
      send_to_char("You are unfamiliar with that spell.\r\n", ch);
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
      sprintf(buf, "Upon %s should the spell be cast?\r\n",
	 IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
	 "what" : "who");
      send_to_char(buf, ch);
      return;
    }
  }

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char("You shouldn't cast that on yourself -- could be bad for your health!\r\n", ch);
    return;
  }
  if (!target) {
    send_to_char("Cannot find the target of your spell!\r\n", ch);
    return;
  }
  mana = mag_manacost(ch, spellnum);
  /* DEBUGGING
  fprintf(stderr, "Spell: %s mana cost %d, ch: %s.\r\n", spells[spellnum],
		mana, GET_NAME(ch));
  */
  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("You haven't the energy to cast that spell!\r\n", ch);
    return;
  }

  /* You throw the dice and you takes your chances.. 101% is total failure */
  if (number(0, 101) > GET_SKILL(ch, spellnum)) {
    if (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))
      WAIT_STATE(ch, PULSE_VIOLENCE * (SINFO.percent/10));

    if (!tch || !skill_message(0, ch, tch, spellnum))
      send_to_char("You lost your concentration!\r\n", ch);
    if (mana > 0)
      GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana >> 1)));
    if (SINFO.violent && tch && IS_NPC(tch))
      hit(tch, ch, TYPE_UNDEFINED);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (cast_spell(ch, tch, tobj, spellnum)) {
      if (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))
	WAIT_STATE(ch, PULSE_VIOLENCE * (SINFO.percent/10));

      if (mana > 0)
	GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
    }
  }
}


/*
 * Spells, skills and spheres: lib/data/spells (see its header for the
 * format).  The numbers are fixed by spells.h and by the skills saved
 * with every character; the file gives each its name, sphere, cost,
 * position, targets and magic routines.  Without the file the game can't
 * run, so a missing file, or one that says nothing, is fatal.
 */

static const char *spell_positions[] = {
  "dead", "mortally", "incap", "stunned", "sleeping", "resting", "sitting",
  "fighting", "standing", "mounted", "flying", "meditating", "\n"
};

static const char *spell_targets[] = {
  "ignore", "char_room", "char_world", "fight_self", "fight_vict",
  "self_only", "not_self", "obj_inv", "obj_room", "obj_world", "obj_equip",
  "door", "must_wield", "\n"
};

static const char *spell_routines[] = {
  "damage", "affects", "unaffects", "points", "alter_objs", "groups",
  "masses", "areas", "summons", "creations", "manual", "\n"
};


static void spell_error(int line, int num, const char *msg)
{
  sprintf(buf, "SYSERR: " SPELL_FILE " line %d (#%d): %s", line, num, msg);
  log(buf);
}


/* The index of word in list, or -1. */
static int spell_word(const char *list[], const char *word)
{
  int i;

  for (i = 0; *list[i] != '\n'; i++)
    if (!str_cmp((char *) list[i], (char *) word))
      return i;
  return -1;
}


/* Bit flags named in text ("char_room fight_vict"); -1 if one is unknown. */
static int spell_flags(const char *list[], char *text)
{
  char word[MAX_INPUT_LENGTH];
  int bits = 0, i;

  for (text = one_argument(text, word); *word; text = one_argument(text, word)) {
    if ((i = spell_word(list, word)) < 0)
      return -1;
    bits |= (1 << i);
  }
  return bits;
}


void load_spells(void)
{
  FILE *fl;
  char line[MAX_STRING_LENGTH], word[MAX_INPUT_LENGTH], *p, *e;
  int num = -1, lineno = 0, i, top = 0, count = 0, a, b, c;

  for (i = 0; i <= TOP_SPELL_DEFINE; i++) {
    if (i) {
      spell_info[i].sphere       = SPHERE_NONE;
      spell_info[i].percent      = 100;
      spell_info[i].mana_max     = 0;
      spell_info[i].mana_min     = 0;
      spell_info[i].mana_change  = 0;
      spell_info[i].min_position = 0;
      spell_info[i].targets      = 0;
      spell_info[i].violent      = 0;
      spell_info[i].routines     = 0;
      spell_info[i].pretime      = 0;
    }
    spells[i] = "!UNUSED!";
  }
  spells[0] = "!RESERVED!";

  if (!(fl = fopen(SPELL_FILE, "r"))) {
    perror("SYSERR: opening " SPELL_FILE);
    exit(1);
  }
  while (fgets(line, sizeof(line), fl)) {
    lineno++;
    if ((p = strchr(line, '\n')))
      *p = '\0';
    if ((p = strchr(line, '\r')))
      *p = '\0';
    p = line;
    skip_spaces(&p);
    if (!*p || *p == ';')
      continue;
    if (*p == '$')
      break;

    if (*p == '#') {
      num = (int) strtol(p + 1, &e, 10);
      if (e == p + 1 || num < 1 || num > TOP_SPELL_DEFINE) {
	spell_error(lineno, num, "bad number");
	num = -1;
	continue;
      }
      skip_spaces(&e);
      for (p = e + strlen(e); p > e && isspace((unsigned char) p[-1]); p--)
	*(p - 1) = '\0';
      if (!*e) {
	spell_error(lineno, num, "no name");
	num = -1;
	continue;
      }
      if (strcmp(spells[num], "!UNUSED!"))
	spell_error(lineno, num, "number used twice");
      spells[num] = str_dup(e);
      top = MAX(top, num);
      count++;
      continue;
    }
    if (num < 0)
      continue;                         /* inside a bad entry */

    p = one_argument(p, word);
    skip_spaces(&p);
    if (!str_cmp(word, "sphere")) {
      for (i = 1; i <= MAX_SPHERES; i++)
	if (!str_cmp(spells[i], p))
	  break;
      if (i > MAX_SPHERES)
	spell_error(lineno, num, "no such sphere (spheres must come first)");
      else
	spell_info[num].sphere = i;
    } else if (!str_cmp(word, "percent"))
      spell_info[num].percent = atoi(p);
    else if (!str_cmp(word, "pretime"))
      spell_info[num].pretime = atoi(p);
    else if (!str_cmp(word, "mana")) {
      if (sscanf(p, "%d %d %d", &a, &b, &c) != 3)
	spell_error(lineno, num, "mana needs max, min and change");
      else {
	spell_info[num].mana_max = a;
	spell_info[num].mana_min = b;
	spell_info[num].mana_change = c;
      }
    } else if (!str_cmp(word, "position")) {
      if ((i = spell_word(spell_positions, p)) < 0)
	spell_error(lineno, num, "unknown position");
      else
	spell_info[num].min_position = i;
    } else if (!str_cmp(word, "targets")) {
      if ((i = spell_flags(spell_targets, p)) < 0)
	spell_error(lineno, num, "unknown target");
      else
	spell_info[num].targets = i;
    } else if (!str_cmp(word, "routines")) {
      if ((i = spell_flags(spell_routines, p)) < 0)
	spell_error(lineno, num, "unknown routine");
      else
	spell_info[num].routines = i;
    } else if (!str_cmp(word, "violent"))
      spell_info[num].violent = TRUE;
    else
      spell_error(lineno, num, "unknown line");
  }
  fclose(fl);

  if (!count) {
    log("SYSERR: " SPELL_FILE " defines no spells or skills.");
    exit(1);
  }
  spells[top + 1] = "\n";
  sprintf(buf, "Loaded %d spells, skills and spheres.", count);
  log(buf);
}

