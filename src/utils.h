/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  UTILS.H                                      Based on CircleMUD *
*  Usage: Header: Utility macros and protos of utility functions          *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All Modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#ifndef _UTILS_H
#define _UTILS_H

/* external declarations and prototypes **********************************/

/* public functions in utils.c */
char    *str_dup(const char *source);
void    str_cpy(char *s, char *t);
int     str_cmp(char *arg1, char *arg2);
int     strn_cmp(char *arg1, char *arg2, int n);
int     touch(char *path);
void    mudlog(char *str, char type, int level, byte file);
void    log_death_trap(struct char_data *ch);
long    number(long from, long to);
int     dice(int number, int size);
void    sprintbit(long vektor, char *names[], char *result);
void    sprinttype(int type, char *names[], char *result);
int     get_line(FILE *fl, char *buf);
int     get_filename(char *orig_name, char *filename, int mode);
struct  time_info_data age(struct char_data *ch);
int     num_pc_in_room(struct room_data *room);
const char *relg_name(struct char_data  *ch);
void    ulog( const char * str, const char * file, const int line );
void    remove_trailing_spaces( char * str );

/* if you prefer to not log the filename and line number comment out
   the following line and uncomment the line beneath it */
// #define log( str )  ulog( str, __FILE__, __LINE__ );
void mud_log( const char * str );
void plog( char * format, ... );

#define log mud_log

int store_mud_time( const struct time_info_data * );
int restore_mud_time( struct time_info_data * );

/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

#ifdef __cplusplus
template<class kind>
kind MAX(kind a, kind b);

template<class kind> 
kind MIN(kind a, kind b);
#else
long MAX(long a, long b);
long MIN(long a, long b);
#endif

/* in magic.c */
bool    circle_follow(struct char_data *ch, struct char_data * victim);

/* in act.informative.c */
void    look_at_room(struct char_data *ch, int mode);

/* in act.movmement.c */
int     do_simple_move(struct char_data *ch, int dir, int following);
int     perform_move(struct char_data *ch, int dir, int following);

/* in limits.c */
int     mana_limit(struct char_data *ch);
int     hit_limit(struct char_data *ch);
int     move_limit(struct char_data *ch);
int     mana_gain(struct char_data *ch);
int     hit_gain(struct char_data *ch);
int     move_gain(struct char_data *ch);
void    advance_level(struct char_data *ch);
void    set_title(struct char_data *ch, char *title);
void    gain_exp(struct char_data *ch, int gain);
void    gain_exp_regardless(struct char_data *ch, int gain);
void    gain_condition(struct char_data *ch, int condition, int value);
void    check_idling(struct char_data *ch);
void    point_update(void);
void    update_pos(struct char_data *victim);


/* various constants *****************************************************/


/* defines for mudlog() */
#define OFF     0
#define BRF     1
#define NRM     2
#define CMP     3

/* get_filename() */
#define CRASH_FILE      0
#define ETEXT_FILE      1
#define ALIAS_FILE      2

/* breadth-first searching */
#define BFS_ERROR               -1
#define BFS_ALREADY_THERE       -2
#define BFS_NO_PATH             -3

/* mud-life time */
#define SECS_PER_MUD_HOUR       75
#define SECS_PER_MUD_DAY        (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH      (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR       (17*SECS_PER_MUD_MONTH)

/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_SEC       1
#define SECS_PER_REAL_MIN       60
#define SECS_PER_REAL_HOUR      (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY       (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR      (365*SECS_PER_REAL_DAY)


/* string utils **********************************************************/


#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")


/* memory utils **********************************************************/


#define CREATE(result, type, number)  do {\
	   if (!((result) = (type *) calloc ((number), sizeof(type)))) { \
	      perror("malloc failure: buy more memory for your machine!"); \
	      abort(); \
	   } else \
              memset( result, 0, sizeof( type ) * number ); \
        } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
  { perror("realloc failure: buy more memory for your machine!"); abort(); } } while(0)

/*
 * the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */
#define REMOVE_FROM_LIST(item, head, next)      \
   if ((item) && (head)) {              \
     if ((item) == (head))                \
       head = (item)->next;               \
     else {                               \
       temp = head;                       \
       while (temp && (temp->next != (item))) \
	 temp = temp->next;              \
       if (temp)                         \
	 temp->next = (item)->next;      \
     }                                   \
   }                                     \


/* basic bitvector utils *************************************************/


#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))

#define MOB_FLAGS(ch)       ((ch)->char_specials.saved.act)
#define PLR_FLAGS(ch)       ((ch)->char_specials.saved.act)
#define PLR2_FLAGS(ch)      ((ch)->char_specials.saved.act2)
#define PRF_FLAGS(ch)       ((ch)->player_specials->saved.pref)
#define PRF2_FLAGS(ch)      ((ch)->player_specials->saved.pref2)
#define AFF_FLAGS(ch)       ((ch)->char_specials.saved.affected_by)
#define AFF2_FLAGS(ch)      ((ch)->char_specials.saved.affected_by2)
#define BODY_PARTS(ch)      ((ch)->player_specials->saved.body_parts)
#define ITEM_AFF(ch)        ((ch)->char_specials.item_aff_by)
#define ROOM_FLAGS(loc)     (world[(loc)].room_flags)
#define DISABIL_FLAGS(ch)   ((ch)->char_specials.saved.disabilities)
#define ABIL_FLAGS(ch)      ((ch)->char_specials.saved.abilities)

#define IS_NPC(ch)  (IS_SET(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_MOB(ch)  (IS_NPC(ch) && ((ch)->nr >-1))

#define MOB_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET(PLR_FLAGS(ch), (flag)))
#define AFF_FLAGGED(ch, flag) (IS_SET(AFF_FLAGS(ch), (flag)))
#define AFF2_FLAGGED(ch, flag)  (IS_SET(AFF2_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET(PRF_FLAGS(ch), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET(ROOM_FLAGS(loc), (flag)))
#define HAS_BPART(ch, part)     (IS_SET(BODY_PARTS(ch), (part)))
#define ITEM_AFF_FLAGGED(ch, flag) (IS_SET(ITEM_AFF(ch), (flag)))
#define DISABIL_FLAGGED(ch, flag)  (IS_SET(DISABIL_FLAGS(ch), (flag)))
#define ABIL_FLAGGED(ch, flag)     (IS_SET(ABIL_FLAGS(ch), (flag)))

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)))
#define IS_AFFECTED2(ch, skill) (AFF2_FLAGGED((ch), (skill)))
#define IS_ITEM_AFF(ch, affect) (ITEM_AFF_FLAGGED((ch), (affect)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR_FLAGS(ch), (flag))) & (flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))


/* room utils ************************************************************/


#define SECT(room)      (world[(room)].sector_type)

#define WEATHER(room)   (zone_table[(world[(room)].zone)].weather_info.sky)
#define SUNLIGHT(room)  (zone_table[(world[(room)].zone)].weather_info.sunlight)
#define IS_DARK(room)   (!world[room].light && \
			 (ROOM_FLAGGED(room, ROOM_DARK) || \
			  ( ( SECT(room) != SECT_INSIDE && \
			      SECT(room) != SECT_CITY ) && \
			    (SUNLIGHT(room) == SUN_SET || \
			     SUNLIGHT(room) == SUN_DARK))))

#define IS_LIGHT(room)  (!IS_DARK(room))

#define GET_ROOM_SPEC(room) ((room) >= 0 ? world[(room)].func : NULL)

/* char utils ************************************************************/
#define IN_ROOM(ch)     ((ch)->in_room)
#define IN_ZONE(ch)     (world[(ch)->in_room].zone)
#define GET_WAS_IN(ch)  ((ch)->was_in_room)
#define GET_AGE(ch)     (age(ch).year)

#define IS_SAME_RACE(ch, vict) (GET_RACE(ch) == GET_RACE(vict))

#define RACE_UNDEF(ch)   ( (GET_RACE(ch) == UNDEFINED_RACE) || \
                           !(GET_RACE(ch) < NUM_RACES) )

#define IS_VERYOLD(ch)   ( !RACE_UNDEF(ch) && \
                           GET_AGE( ch ) > races[ GET_RACE(ch) ].points.points.old_age - 10)

#define IS_ELDERLY(ch)   ( !RACE_UNDEF(ch) && \
                           GET_AGE( ch ) > races[ GET_RACE(ch) ].points.points.old_age + 10)

#define IS_ANCIENT(ch)   ( !RACE_UNDEF(ch) && \
                           GET_AGE( ch ) > races[ GET_RACE(ch) ].points.points.old_age + 20)

#define GET_NAME(ch)    (IS_NPC(ch) ? \
			 (ch)->player.short_descr : (ch)->player.name)
#define GET_KWDS(ch)    (IS_NPC(ch) ? (ch)->player.short_descr : \
			 (ch)->player.keywords)
#define GET_RDESC(ch)   (IS_NPC(ch) ? (ch)->player.long_descr : \
			 (ch)->player.room_descr)
#define GET_SHORT_DESC(ch) ((ch)->player.short_descr)
#define GET_LONG_DESC(ch)  ((ch)->player.long_descr)
#define GET_DESCRIPTION(ch) ((ch)->player.description)
#define GET_TITLE(ch)      ((ch)->player.title)
#define GET_TRUST(ch)      ((ch)->player.trust)
#define GET_LEVEL(ch)      ((ch)->player.level)
#define GET_MAX_LEVEL(ch)  ((ch)->player.level)
#define GET_AGE_MOD(ch)    ((ch)->player.race_mod)
#define GET_PASSWD(ch)     ((ch)->player.passwd)
#define GET_PFILEPOS(ch)   ((ch)->pfilepos)
#define IS_IMMORTAL(ch)    (GET_LEVEL(ch) >= LVL_IMMORT)

/*
 * I wonder if this definition of GET_REAL_LEVEL should be the definition
 * of GET_LEVEL?  JE
 */
#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))
#define GET_EMAIL(ch)     ((ch)->desc->email);
#define GET_RACE(ch)      ((ch)->player.race)
#define GET_REL(ch)       ((ch)->player.assocs[0])
#define HAS_RELIGION(ch)  (strcmp( relg_name(ch), "None" ))
#define GET_CLAN(ch)      ((ch)->player.assocs[1])
#define GET_GUILD(ch)     ((ch)->player.assocs[1])
#define GET_GUILD_LEV(ch) ((ch)->player.assocs[2])
#define GET_HIT_REGEN(ch) ((ch)->player.regens[0])
#define GET_MANA_REGEN(ch) ((ch)->player.regens[1])
#define GET_MOVE_REGEN(ch) ((ch)->player.regens[2])
#define GET_HOME(ch)      (pc_homes[(ch)->player.hometown])
#define GET_HOMETOWN(ch)  ((ch)->player.hometown)
#define GET_HEIGHT(ch)    ((ch)->player.height)
#define GET_WEIGHT(ch)    ((ch)->player.weight)
#define GET_SEX(ch)       ((ch)->player.sex)
#define IS_MALE(ch)       (GET_SEX(ch) == SEX_MALE)
#define GET_LANGUAGE(ch, i) ((ch)->player.speaks[(i)].basic_known)
#define GET_DIALECT(ch, i)  ((ch)->player.speaks[(i)].dialects_known)
#define GET_CURR_LANG(ch)   ((ch)->player_specials->saved.curr_lang)
#define GET_HANDEDNESS(ch)  ((ch)->char_specials.saved.handedness)
#define GET_BATTLE_POS(ch)  ((ch)->char_specials.saved.battle_stance)

#define GET_STR(ch)     ((ch)->aff_abils.str)
#define GET_ADD(ch)     ((ch)->aff_abils.str_add)
#define GET_DEX(ch)     ((ch)->aff_abils.dex)
#define GET_INT(ch)     ((ch)->aff_abils.intel)
#define GET_WIS(ch)     ((ch)->aff_abils.wis)
#define GET_CON(ch)     ((ch)->aff_abils.con)
#define GET_CHA(ch)     ((ch)->aff_abils.cha)
#define GET_WILL(ch)    ((ch)->aff_abils.will)

#define CAN_LEV(ch)     (exp_needed(GET_LEVEL(ch)+1) <= GET_EXP(ch))
#define GET_EXP(ch)       ((ch)->points.exp)
#define GET_AC(ch, i)     ((ch)->points.armor[i] <= 100 && (ch)->points.armor[i] >= 0 ? \
			   (ch)->points.armor[i] : ((ch)->points.armor[i] = 0))
#define GET_HIT(ch)       ((ch)->points.hit)
#define GET_MAX_HIT(ch)   ((ch)->points.max_hit)
#define GET_MOVE(ch)      ((ch)->points.move)
#define GET_MAX_MOVE(ch)  ((ch)->points.max_move)
#define GET_MANA(ch)      ((ch)->points.mana)
#define GET_MAX_MANA(ch)  ((ch)->points.max_mana)
#define GET_GOLD(ch)      ((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)
#define GET_HITROLL(ch)   ((ch)->points.hitroll)
#define GET_DAMROLL(ch)   ((ch)->points.damroll)
/* Edit mode for OLC */
#define EDIT_MODE(d)      ((d)->edit_mode)

#define GET_POS(ch)       ((ch)->char_specials.position)
#define GET_IDNUM(ch)     ((ch)->char_specials.saved.idnum)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight + \
                           IS_IMMORT(ch) ? 0 : (GET_GOLD(ch)/100))
#define IS_CARRYING_WEIGHT(ch) ((ch)->char_specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)      ((ch)->char_specials.fighting)
#define HUNTING(ch)       ((ch)->char_specials.hunting)
#define MOUNTED(ch)       ((ch)->char_specials.mounted_on)
#define RIDDEN(ch)        ((ch)->char_specials.ridden_by)
#define SHADOWING(ch)     ((ch)->char_specials.shadowing)
#define SHADOWED(ch)      ((ch)->char_specials.shadowed_by)
#define FURNITURE(ch)     ((ch)->char_specials.using)
#define GET_ITEMS_STUCK(ch, loc)  ((ch)->char_specials.lodged[(loc)])
#define GET_FOLS(ch)      ((ch)->char_specials.num_fols)
#define GET_SAVE(ch, i)   ((ch)->char_specials.saved.apply_saving_throw[i])
#define GET_ALIGNMENT(ch) ((ch)->char_specials.saved.alignment)
#define GET_PERMALIGN(ch) ((ch)->char_specials.saved.perm_align)
#define GET_PALIGN(ch)    ((ch)->char_specials.saved.perm_align)
#define GET_HANDEDNESS(ch) ((ch)->char_specials.saved.handedness)
#define GET_DEATH_COUNTER(ch)   ((ch)->player_specials->saved.death_counter)
#define GET_WOUNDS(ch)     ((ch)->player_specials->saved.wounds)
#define GET_FATE_PTS(ch)   ((ch)->player_specials->saved.fate_pts)

#define GET_COND(ch, i)         ((ch)->player_specials->saved.conditions[(i)])
#define GET_ADDICT(ch, i)       ((ch)->player_specials->saved.addictions[(i)])
#define GET_LOADROOM(ch)        ((ch)->player_specials->saved.load_room)
#define GET_PRACTICES(ch)       ((ch)->player_specials->saved.spells_to_learn)
#define GET_INVIS_LEV(ch)       ((ch)->player_specials->saved.invis_level)
#define GET_WIMP_LEV(ch)        ((ch)->player_specials->saved.wimp_level)
#define GET_FREEZE_LEV(ch)      ((ch)->player_specials->saved.freeze_level)
#define GET_BAD_PWS(ch)         ((ch)->player_specials->saved.bad_pws)
#define POOFIN(ch)              ((ch)->player_specials->poofin)
#define POOFOUT(ch)             ((ch)->player_specials->poofout)
#define GET_LAST_OLC_TARG(ch)   ((ch)->player_specials->last_olc_targ)
#define GET_LAST_OLC_MODE(ch)   ((ch)->player_specials->last_olc_mode)
#define GET_ALIASES(ch)         ((ch)->player_specials->aliases)
#define GET_LAST_TELL(ch)       ((ch)->player_specials->last_tell)
#define GET_REGEN_GAIN(ch)      (regen_gain[(int) GET_RACE((ch))])
#define GET_SESS_EXP(ch)        ((ch)->player_specials->exp_gained)
#define GET_SESS_MKA(ch)        ((ch)->player_specials->killed_mobs[0])
#define GET_SESS_MKG(ch)        ((ch)->player_specials->killed_mobs[1])
#define GET_SESS_RELC(ch)       ((ch)->player_specials->rel_contrib)

#define GET_SKILL(ch, i)        ((ch)->player_specials->saved.skills[i])
#define SET_SKILL(ch, i, pct)   { (ch)->player_specials->saved.skills[i] = pct; }

#define GET_EQ(ch, i)           ((ch)->equipment[i])

#define GET_MOB_SPEC(ch) (IS_MOB(ch) ? (mob_index[(ch->nr)].func) : NULL)
#define GET_MOB_RNUM(mob)       ((mob)->nr)
#define GET_MOB_VNUM(mob)       (IS_MOB(mob) ? \
				 mob_index[GET_MOB_RNUM(mob)].vnumber : -1)

#define GET_MOB_WAIT(ch)        ((ch)->mob_specials.wait_state)
#define GET_DEFAULT_POS(ch)     ((ch)->mob_specials.default_pos)
#define GET_MOB_TYPE(mob)       ((mob)->mob_specials.mob_type)
#define GET_ATTACK_TYPE(mob)    ((mob)->mob_specials.attack_type)
#define MEMORY(ch)              ((ch)->mob_specials.memory)

#define STRENGTH_APPLY_INDEX(ch) \
	( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
	  (GET_ADD(ch) <= 50) ? 26 :( \
	  (GET_ADD(ch) <= 75) ? 27 :( \
	  (GET_ADD(ch) <= 90) ? 28 :( \
	  (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
	)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)
#define CAN_CARRY_N(ch) (MAX(8, GET_LEVEL(ch) >> 1))

#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)
#define CAN_SEE_IN_DARK(ch) \
	(AFF_FLAGGED(ch, AFF_INFRAVISION) || PRF_FLAGGED(ch, PRF_HOLYLIGHT) || \
	 HAS_LIGHT(ch) || IS_ITEM_AFF(ch, AFFECTED_INFRA))


#define HAS_LIGHT(ch)  \
  ((GET_EQ(ch, WEAR_HAND_L) && (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HAND_L)) == ITEM_LIGHT && \
    GET_OBJ_VAL(GET_EQ(ch, WEAR_HAND_L), 2) != 0)) || \
   (GET_EQ(ch, WEAR_HAND_R) && (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HAND_R)) == ITEM_LIGHT && \
    GET_OBJ_VAL(GET_EQ(ch, WEAR_HAND_R), 2) != 0)))

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))
/* these definitions are wrong, so please fix them */
#define IS_LAWFUL(ch)  (IS_GOOD(ch))
#define IS_CHAOTIC(ch) (IS_EVIL(ch))

#define IS_PERMGOOD(ch) (GET_PERMALIGN(ch) == ALIGN_GOOD)
#define IS_PERMEVIL(ch) (GET_PERMALIGN(ch) == ALIGN_EVIL)
#define IS_PERMNEUT(ch) (GET_PERMALIGN(ch) == ALIGN_NEUTRAL)

#define IS_PRAYER(spell)  ((spell_info[(spell)].sphere == SPHERE_PRAYER) || \
			   (spell_info[(spell)].sphere == SPHERE_ACTS_OF_DEITIES) || \
			   (spell_info[(spell)].sphere == SPHERE_AFFLICTIONS) || \
			   (spell_info[(spell)].sphere == SPHERE_CURES) || \
			   (spell_info[(spell)].sphere == SPHERE_HAND_OF_DEITIES) || \
			   (spell_info[(spell)].sphere == SPHERE_FAITH))

#define IS_ARCANE(spell)  ((spell_info[(spell)].sphere == SPHERE_ARCANE) || \
			   (spell_info[(spell)].sphere == SPHERE_CONJURATION) || \
			   (spell_info[(spell)].sphere == SPHERE_DIVINATION) || \
			   (spell_info[(spell)].sphere == SPHERE_ENCHANTMENT) || \
			   (spell_info[(spell)].sphere == SPHERE_ENTROPIC) || \
			   (spell_info[(spell)].sphere == SPHERE_INVOCATION) || \
			   (spell_info[(spell)].sphere == SPHERE_PHANTASMIC))

#define IS_THIEVERY(spell) ((spell_info[(spell)].sphere == SPHERE_LOOTING) || \
			    (spell_info[(spell)].sphere == SPHERE_MURDER) || \
			    (spell_info[(spell)].sphere == SPHERE_THIEVING))

#define IS_COMBAT(spell)  ((spell_info[(spell)].sphere == SPHERE_WEAPONS) || \
			   (spell_info[(spell)].sphere == SPHERE_HAND_TO_HAND) || \
			   (spell_info[(spell)].sphere == SPHERE_MARTIAL_ARTS))

/* descriptor-based utils ************************************************/


#define WAIT_STATE(ch, cycle) { \
	if ((ch)->desc) (ch)->desc->wait = (cycle); \
	else if (IS_NPC(ch)) GET_MOB_WAIT(ch) = (cycle); }

#define CHECK_WAIT(ch)  (((ch)->desc) ? ((ch)->desc->wait > 1) : 0)
#define STATE(d)        ((d)->connected)
#define SUBSTATE(d)     ((d)->sub_state)
#define GET_ANSI(d)     ((d)->ansi)

/* object utils **********************************************************/


#define GET_OBJ_TYPE(obj)       ((obj)->obj_flags.type_flag)
#define GET_OBJ_COST(obj)       ((obj)->obj_flags.cost)
#define GET_OBJ_RENT(obj)       ((obj)->obj_flags.cost_per_day)
#define GET_OBJ_EXTRA(obj)      ((obj)->obj_flags.extra_flags)
#define GET_OBJ_WEAR(obj)       ((obj)->obj_flags.wear_flags)
#define GET_OBJ_VAL(obj, val)   ((obj)->obj_flags.value[(val)])
#define GET_OBJ_MATERIAL(obj)   ((obj)->obj_flags.value[7])
#define GET_OBJ_SIZE(obj)       ((obj)->obj_flags.value[8])
#define GET_OBJ_COND(obj)       ((obj)->obj_flags.value[9])
#define GET_OBJ_WEIGHT(obj)     ((obj)->obj_flags.weight)
#define GET_OBJ_TIMER(obj)      ((obj)->obj_flags.timer)
#define GET_OBJ_RNUM(obj)       ((obj)->item_number)
#define GET_OBJ_VNUM(obj)       (GET_OBJ_RNUM(obj) >= 0 ? \
				 obj_index[GET_OBJ_RNUM(obj)].vnumber : -1)
#define IS_OBJ_STAT(obj,stat)   (IS_SET((obj)->obj_flags.extra_flags,stat))
#define IS_OBJ_SIZE(obj, size)  (IS_SET((obj)->obj_flags.value[8], size))

#define IS_STAB_OBJ(obj)        (GET_OBJ_VAL((obj), 3) == TYPE_PIERCE - TYPE_HIT)
#define IS_CUDGEL_OBJ(obj)      ((GET_OBJ_VAL((obj), 3) == TYPE_BLUDGEON - TYPE_HIT) || \
				 (GET_OBJ_VAL((obj), 3) == TYPE_CRUSH - TYPE_HIT) || \
				 (GET_OBJ_VAL((obj), 3) == TYPE_POUND - TYPE_HIT) || \
				 (GET_OBJ_VAL((obj), 3) == TYPE_SMASH - TYPE_HIT))
#define IS_PIERCE_OBJ(obj)      (GET_OBJ_VAL((obj), 3) == TYPE_PIERCE - TYPE_HIT)
#define IS_BLUNT_OBJ(obj)       (IS_CUDGEL_OBJ(obj))
#define IS_SWORDS_OBJ(obj)      ((GET_OBJ_VAL((obj), 3) == TYPE_SLASH - TYPE_HIT) || \
				 (GET_OBJ_VAL((obj), 3) == TYPE_PIERCE - TYPE_HIT))
#define IS_STAB(wtype)          ((wtype) == TYPE_PIERCE - TYPE_HIT)
#define IS_CUDGEL(wtype)        (((wtype) == TYPE_BLUDGEON - TYPE_HIT) || \
				 ((wtype) == TYPE_CRUSH - TYPE_HIT) || \
				 ((wtype) == TYPE_POUND - TYPE_HIT) || \
				 ((wtype) == TYPE_SMASH - TYPE_HIT))
#define IS_PIERCE(wtype)        ((wtype) == TYPE_PIERCE - TYPE_HIT)
#define IS_BLUNT(wtype)         (IS_CUDGEL(wtype))
#define IS_SWORD(wtype)         (((wtype) == TYPE_SLASH - TYPE_HIT) || \
				  ((wtype) == TYPE_PIERCE - TYPE_HIT))

#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
	(obj_index[(obj)->item_number].func) : NULL)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags, (part)))

#define IS_MAGITEM(obj)     (GET_OBJ_TYPE(obj) == ITEM_WAND || \
			     GET_OBJ_TYPE(obj) == ITEM_STAFF || \
			     GET_OBJ_TYPE(obj) == ITEM_POTION || \
			     GET_OBJ_TYPE(obj) == ITEM_SCROLL || \
			     IS_OBJ_STAT(obj, ITEM_FROSTY)    || \
			     IS_OBJ_STAT(obj, ITEM_FLAMING)   || \
			     IS_OBJ_STAT(obj, ITEM_SPARKING)  || \
			     IS_OBJ_STAT(obj, ITEM_ACIDIC)    || \
			     IS_OBJ_STAT(obj, ITEM_NOXIOUS))

/* compound utilities and other macros **********************************/

#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define HISHER(i) (i == SEX_MALE ? "his" : i == SEX_FEMALE ? "her" : "its")
#define HESHE(i)  (i == SEX_MALE ? "he"  : i == SEX_FEMALE ? "she" : "it")
#define HIMHER(i) (i == SEX_MALE ? "him" : i == SEX_FEMALE ? "her" : "it")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")
#define XANA(name) (strchr("aeiouyAEIOUY", *(name)) ? "an" : "a")

/* Various macros building up to CAN_SEE */

#define LIGHT_OK(sub)   (!IS_AFFECTED2(sub, AFF_BLIND) && \
   (IS_LIGHT((sub)->in_room) || IS_AFFECTED((sub), AFF_INFRAVISION)))

#define INVIS_OK(sub, obj) \
 ((!IS_AFFECTED((obj),AFF_INVISIBLE) || IS_AFFECTED(sub,AFF_DETECT_INVIS)) && \
 (!IS_AFFECTED((obj), AFF_HIDE) || IS_AFFECTED(sub, AFF_SENSE_LIFE)))

#define MORT_CAN_SEE(sub, obj) (LIGHT_OK(sub) && INVIS_OK(sub, obj))

#define IMM_CAN_SEE(sub, obj) \
   (MORT_CAN_SEE(sub, obj) || PRF_FLAGGED(sub, PRF_HOLYLIGHT))

#define SELF(sub, obj)  ((sub) == (obj))

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
   ((GET_REAL_LEVEL(sub) >= GET_INVIS_LEV(obj)) && IMM_CAN_SEE(sub, obj)))

/* End of CAN_SEE */


#define INVIS_OK_OBJ(sub, obj) \
  (!IS_OBJ_STAT((obj), ITEM_INVISIBLE) || IS_AFFECTED((sub), AFF_DETECT_INVIS))

#define MORT_CAN_SEE_OBJ(sub, obj) (LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj))

#define CAN_SEE_OBJ(sub, obj) \
   ((MORT_CAN_SEE_OBJ(sub, obj) || PRF_FLAGGED((sub), PRF_HOLYLIGHT)))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
    CAN_SEE_OBJ((ch),(obj)))


#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : "someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
			  ((obj)->short_description ? (obj)->short_description : \
			  "something") : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")


#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])
#define _2ND_EXIT(ch, door) (world[EXIT(ch, door)->to_room].dir_option[door])
#define _3RD_EXIT(ch, door) (world[_2ND_EXIT(ch, door)->to_room].dir_option[door])
#define _4TH_EXIT(ch, door) (world[_3RD_EXIT(ch, door)->to_room].dir_option[door])
#define _5TH_EXIT(ch, door) (world[_4TH_EXIT(ch, door)->to_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
/* CAN_GO */		 (EXIT(ch,door)->to_room != NOWHERE) && \
/* CAN_GO */		  !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))


#define CLASS_ABBR(ch) (IS_NPC(ch) ? "--" : class_abbrevs[(int)GET_CLASS(ch)])

#define SPEC_ABBR(ch)  (IS_NPC(ch) ? "--" : spec_class_abbrevs[(int) GET_SPEC_CLASS(ch)])

#define RACE_ABBR(ch)  ( RACE_UNDEF(ch) ? "--" : \
                         races[ GET_RACE(ch) ].abbrev )

#define GUILD_ABBR(ch)  (IS_NPC(ch) ? "--" : guild_abbrev[(int) GET_GUILD(ch)])

#define IS_WIZARD(ch)  (GET_SKILL(ch, SPHERE_ARCANE) > 1)
#define IS_PRIEST(ch)  (GET_SKILL(ch, SPHERE_PRAYER) > 1)
#define IS_ROGUE(ch)   (GET_SKILL(ch, SPHERE_GTHIEVERY) > 1)
#define IS_WARRIOR(ch) (GET_SKILL(ch, SPHERE_COMBAT) > 1)

#define CAN_SPEAK(ch)        ( !RACE_UNDEF(ch) && \
                               races[ GET_RACE(ch) ].canSpeak )

#define IS_REGENER(ch)       ( (!RACE_UNDEF(ch) && \
                                races[ GET_RACE(ch) ].points.regen != 0) || \
			       IS_SET(ABIL_FLAGS(ch), ABIL_REGENERATION))

#define IS_HUMANOID(ch)      ( !RACE_UNDEF(ch) && races[ GET_RACE(ch) ].isHumanoid )

#define IS_VAMPIRE(ch)       ( AFF2_FLAGGED(ch, AFF_VAMPIRE) )

#define IS_LYCANTHROPE(ch)   ( AFF2_FLAGGED(ch, AFF_LYCANTHROPE) )

#define IS_UNDEAD(ch)        ( AFF2_FLAGGED(ch, AFF_UNDEAD) )

#define IS_MINOTAUR(ch)      (!RACE_UNDEF(ch) && \
                              !strcmp( races[ GET_RACE(ch) ].name, "Minotaur" ))

#define IS_DRAGON(ch)        (!RACE_UNDEF(ch) && \
                              strstr( races[ GET_RACE(ch) ].name, "Dragon" ))

/* This needs to be fixed */
#define IS_ANIMAL(ch)        (FALSE)

/* This needs to be fixed */
#define IS_ILLUSION(ch)      (FALSE)

/* update as new races are added, perhaps the race_data should be
   modified to keep a bitvec of perm affects */
#define RACE_HAS_INFRA(ch)   ( !RACE_UNDEF(ch) && \
                               (!strcmp( races[ GET_RACE(ch) ].name, "Gnome" ) || \
                                !strcmp( races[ GET_RACE(ch) ].name, "Dwarf" )) )
                               
#define IS_TINY(ch)          ( !RACE_UNDEF(ch) && \
                               races[ GET_RACE(ch) ].points.rsize == TINY)

#define IS_SMALL(ch)         ( !RACE_UNDEF(ch) && \
                               races[ GET_RACE(ch) ].points.rsize == SMALL)

#define IS_MEDIUM(ch)        ( !RACE_UNDEF(ch) && \
                               races[ GET_RACE(ch) ].points.rsize == MEDIUM)

#define IS_LARGE(ch)         ( !RACE_UNDEF(ch) && \
                               races[ GET_RACE(ch) ].points.rsize == LARGE)

#define IS_HUGE(ch)          ( !RACE_UNDEF(ch) && \
                               races[ GET_RACE(ch) ].points.rsize == HUGE)

#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS))


/* OS compatibility ******************************************************/


/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0L
#endif

#if !defined(__cplusplus)
#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
#endif

#if defined(NOCRYPT) || !defined(HAVE_CRYPT)
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

#endif
