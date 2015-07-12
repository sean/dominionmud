/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  MOBACT.C                                     Based on CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __MOBACT_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/* external structs */
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];
extern struct race_data *races;
extern int NUM_RACES;

#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)

ACMD(do_flee);
int is_empty(int zone_nr);
int is_same_align(struct char_data * ch, struct char_data * vict);
struct char_data *FindAHatee(struct char_data *ch); 
int Hates(struct char_data *ch, struct char_data *v);
int Fears(struct char_data *ch, struct char_data *v);
struct char_data *FindAFearee(struct char_data *ch); 
int RemHated(struct char_data *ch, struct char_data *pud);
int RemFeared(struct char_data *ch, struct char_data *pud); 
void mprog_random_trigger(struct char_data * mob);
void mprog_wordlist_check(char *arg, struct char_data * mob, struct char_data * actor,
			  struct obj_data * obj, void *vo, int type);
void hunt_victim(struct char_data *hunter);

ACMD(do_get);

void mobile_activity(void)
{
  register struct char_data *ch, *next_ch, *vict, *tmp_ch;
  struct obj_data *obj, *best_obj;
  int door, max, found;
  memory_rec *names;

  extern int no_specials;

  for (ch = character_list; ch; ch = next_ch) {
     next_ch = ch->next;

     if (!IS_MOB(ch) || FIGHTING(ch) || !AWAKE(ch))
       continue;
     /* Examine call for special procedure */
     if (MOB_FLAGGED(ch, MOB_SPEC) && !no_specials) {
       if (mob_index[GET_MOB_RNUM(ch)].func == NULL) {
	 sprintf(buf, "%s (#%ld): Attempting to call non-existing mob func",
		GET_NAME(ch), GET_MOB_VNUM(ch));
	 log(buf);
	 REMOVE_BIT(MOB_FLAGS(ch), MOB_SPEC);
       } else {
	 if ((mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, ""))
	   continue;             /* go to next char */
       }
     }
     /* rand_progs for mobs */
     if (!is_empty(world[ch->in_room].zone))
       mprog_random_trigger(ch);
     /* guardian, protects its master */
     if (MOB_FLAGGED(ch, MOB_GUARDIAN) && !FIGHTING(ch) && AWAKE(ch)) {
       found = FALSE;
       /* make sure  the mob is in the game and has a master under attack */
       if ((ch->in_room > -1) && (ch->master != NULL) &&
	   FIGHTING(ch->master)) {
	 if ((vict = FIGHTING(ch->master)) != NULL && CAN_SEE(ch, vict)) {
	   if (IS_HUMANOID(ch))
	     act("$n screams 'I must protect my master!'", FALSE, ch, 0, 0, TO_ROOM);
	   else
	     act("$n growls angrily!", FALSE, ch, 0, 0, TO_ROOM);
	   hit(ch, vict, TYPE_UNDEFINED);
	 }
       }
     } /* End of guardian mobs */
     /* hateful and fearing mobiles */
     if (MOB_FLAGGED(ch, MOB_HATEFUL) && (GET_HIT(ch) > (GET_MAX_HIT(ch) >> 1))) {
       if ((tmp_ch = FindAHatee(ch)) != NULL) {
	 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) && IS_HUMANOID(ch)) {
	   send_to_char("You ask your mortal enemy to step outside to settle matters.\n\r", ch);
	   act("$n growls '$N, would you care to step outside where we can settle this?'", TRUE, ch, 0, tmp_ch, TO_ROOM);
	 } else {
	   if (IS_HUMANOID(ch))
	     act("$n screams 'I'm gonna kill you!'", TRUE, ch, 0, 0, TO_ROOM);
	   else
	     act("$n growls at $N", TRUE, ch, 0, tmp_ch, TO_ROOM);
	   hit(ch, tmp_ch, TYPE_UNDEFINED);
	 }
	 if (Hates(ch, tmp_ch) && Fears(ch, tmp_ch)) {
	   RemHated(ch, tmp_ch);
	   RemFeared(ch, tmp_ch);
	 }
       }
     } /* End of hating mobs */
     /* Fearful mobs */
     if (MOB_FLAGGED(ch, MOB_AFRAID) && FIGHTING(ch)) {
       if ((tmp_ch = FindAFearee(ch)) != NULL)
	 do_flee(ch, "", 0, 0);
     } else if (MOB_FLAGGED(ch, MOB_AFRAID)) {
       if ((tmp_ch = FindAFearee(ch)) != NULL) {
	 act("$n looks at you, and screams, 'Aaaiiieeeehhhh!'", TRUE,
	      ch, 0, tmp_ch, TO_VICT);
	 act("$n looks at $n, and screams, 'Aaaiiieeeehhhh!'", TRUE,
	      ch, 0, tmp_ch, TO_ROOM);
	 do_flee(ch, "", 0, 0);
       }
     } /* End of fearful mobs */
     /* Scavenger (picking up objects) */
     if (MOB_FLAGGED(ch, MOB_SCAVENGER) && !FIGHTING(ch) && AWAKE(ch)) {
       if (world[ch->in_room].contents && !number(0, 10)) {
	 max = 1;
	 best_obj = NULL;
	 for (obj = world[ch->in_room].contents; obj; obj = obj->next_content) {
	   if (CAN_GET_OBJ(ch, obj) && GET_OBJ_COST(obj) > max) {
	     best_obj = obj;
	     max = GET_OBJ_COST(obj);
	   }
	 }
	 if (best_obj != NULL) {
	   obj_from_room(best_obj);
	   obj_to_char(best_obj, ch);
	   act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);
	 }
       }
     } /* End of scavengers */
     /* Mob Movement */
     if (!MOB_FLAGGED(ch, MOB_SENTINEL) && (GET_POS(ch) == POS_STANDING) &&
	 ((door = number(0, 18)) < NUM_OF_DIRS) && CAN_GO(ch, door) &&
         !IS_AFFECTED(ch, AFF_CHARM) &&
	 !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_NOMOB | ROOM_DEATH) &&
	 (!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
	 (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone))) {
       perform_move(ch, door, 1);
     } /* End of moving mobs */
     /* Aggressive Mobs */
     if (MOB_FLAGGED(ch, MOB_AGGRESSIVE | MOB_AGGR_TO_ALIGN)) {
       found = FALSE;
       for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
	 if (!CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE) || vict == ch)
	   continue;
	 if (MOB_FLAGGED(ch, MOB_WIMPY) && AWAKE(vict))
	   continue;
	 if (MOB_FLAGGED(ch, MOB_AGGRESSIVE) && IS_NPC(vict))
	   continue;  /* aggr_to_align will attack other mobs */
	 if (!MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN) ||
	     (MOB_FLAGGED(ch, MOB_AGGR_EVIL) && IS_EVIL(vict)) ||
	     (MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(vict)) ||
	     (MOB_FLAGGED(ch, MOB_AGGR_GOOD) && IS_GOOD(vict))) {
	   hit(ch, vict, TYPE_UNDEFINED);
	   found = TRUE;
	 }
       }
     } /* End of aggro mobs */
     /* Mob Memory */
     if (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch)) {
       found = FALSE;
       for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
	 if (IS_NPC(vict) || !CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE))
	   continue;
	 for (names = MEMORY(ch); names && !found; names = names->next) {
	   if (names->id == GET_IDNUM(vict)) {
	     found = TRUE;
	     if (IS_HUMANOID(ch)) {
	       if (IS_EVIL(ch))
		 act("$n says, 'I'm gonna kill you, $N!'", FALSE, ch, 0, vict, TO_ROOM);
	       else if (IS_NEUTRAL(ch) < 250)
		 act("$n says, 'Hey!  You're the fiend that attacked me!!!', exclaims $n.",
		     FALSE, ch, 0, 0, TO_ROOM);
	       else /* isgood */
		 act("$n says, 'You devilspawn, I'm going to repay your unkind act, $N!",
		     FALSE, ch, 0, vict, TO_ROOM);
	     } else {
	       act("$n growls at $N ferociously.", FALSE, ch, 0, vict, TO_NOTVICT);
	       act("$n growls at you ferociously.", FALSE, ch, 0, vict, TO_VICT);
	     }
	     hit(ch, vict, TYPE_UNDEFINED);
	   }
	 }
       }
     } /* End of mob memory */
     /* Helper Mobs */
     if (MOB_FLAGGED(ch, MOB_HELPER)) {
       found = FALSE;
       for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room)
	 if (ch != vict && FIGHTING(vict) &&
	     (ch != FIGHTING(vict) && !(!strcmp(GET_NAME(FIGHTING(vict)), GET_NAME(ch)))) &&
	     (IS_NPC(vict) || is_same_align(ch, vict))) {
	   /* changed so mobs help players too! */
	   act("$n jumps to the aid of $N!", FALSE, ch, 0, vict, TO_ROOM);
	   hit(ch, FIGHTING(vict), TYPE_UNDEFINED);
	   found = TRUE;
	 }
     } /* helper mobs */
     /* Hunter mobs */
     if (HUNTING(ch))
       hunt_victim(ch);
     /* Add new mobile actions here */
  }                             /* end for() */
}


/* used for helper mobs */
int is_same_align(struct char_data * ch, struct char_data * vict)
{
    if (IS_GOOD(ch) && IS_GOOD(vict))
      return 1;
    else if (IS_EVIL(ch) && IS_EVIL(vict))
      return 1;
    else if (IS_NEUTRAL(ch) && IS_NEUTRAL(vict))
      return 1;
    else
      return 0;
}

/* Mob Memory Routines */

/* make ch remember victim */
void remember(struct char_data * ch, struct char_data * victim)
{
  memory_rec *tmp;
  bool present = FALSE;

  if (!IS_NPC(ch) || IS_NPC(victim))
    return;

  for (tmp = MEMORY(ch); tmp && !present; tmp = tmp->next)
    if (tmp->id == GET_IDNUM(victim))
      present = TRUE;

  if (!present) {
    CREATE(tmp, memory_rec, 1);
    tmp->next = MEMORY(ch);
    tmp->id = GET_IDNUM(victim);
    MEMORY(ch) = tmp;
  }
}


/* make ch forget victim */
void forget(struct char_data * ch, struct char_data * victim)
{
  memory_rec *curr, *prev = NULL;

  if (!(curr = MEMORY(ch)))
    return;

  while (curr && curr->id != GET_IDNUM(victim)) {
    prev = curr;
    curr = curr->next;
  }

  if (!curr)
    return;                     /* person wasn't there at all. */

  if (curr == MEMORY(ch))
    MEMORY(ch) = curr->next;
  else
    prev->next = curr->next;

  free(curr);
}


/* erase ch's memory */
void clearMemory(struct char_data * ch)
{
  memory_rec *curr, *next;

  curr = MEMORY(ch);

  while (curr) {
    next = curr->next;
    free(curr);
    curr = next;
  }

  MEMORY(ch) = NULL;
}


/* Everything below here used to be in opinion.c */

void FreeHates(struct char_data *ch)
{
  struct char_list *k, *n;

  for (k = ch->hates.clist; k; k = n) {
      n = k->next;
      free(n);
  }
}


void FreeFears(struct char_data *ch)
{
  struct char_list *k, *n;

  for (k = ch->fears.clist; k; k = n) {
      n = k->next;
      free(n);
  }
}


int RemHated(struct char_data *ch, struct char_data *pud)
{
  struct char_list *oldpud, *t;

  if (pud) {
     for (oldpud = ch->hates.clist; oldpud; oldpud = oldpud->next) {
       if (!oldpud) return(FALSE);
       if (oldpud->op_ch) {
	if (oldpud->op_ch == pud) {
	  t = oldpud;
	  if (ch->hates.clist == t) {
	    ch->hates.clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = ch->hates.clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      } else {
	if (!strcmp(oldpud->name,GET_NAME(pud))) {
	  t = oldpud;
	  if (ch->hates.clist == t) {
	    ch->hates.clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = ch->hates.clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      }
     }
  }

  if (!ch->hates.clist) {
     REMOVE_BIT(ch->hatefield, HATE_CHAR);
  }
  if (!ch->hatefield)
     REMOVE_BIT(MOB_FLAGS(ch), MOB_HATEFUL);

  return( (pud) ? TRUE : FALSE);
}



int AddHated(struct char_data *ch, struct char_data *pud)
{

  struct char_list *newpud;

  if (ch == pud)
    return(FALSE);

  if (pud) {

    if (!CAN_SEE(ch, pud))
      return(FALSE);

    CREATE(newpud, struct char_list, 1);
    newpud->op_ch = pud;
    strcpy(newpud->name, GET_NAME(pud));
    newpud->next = ch->hates.clist;
    ch->hates.clist = newpud;
    if (!MOB_FLAGGED(ch, MOB_HATEFUL)) {
      SET_BIT(MOB_FLAGS(ch), MOB_HATEFUL);
    }
    if (!IS_SET(ch->hatefield, HATE_CHAR)) {
      SET_BIT(ch->hatefield, HATE_CHAR);
    }
    if (IS_IMMORTAL(pud))
       send_to_char("---Someone hates you.\n\r",pud);
  }

  return( (pud) ? TRUE : FALSE );
}


int AddHatred(struct char_data *ch, int parm_type, int parm)
{
  switch(parm_type) {
  case OP_SEX :
    if (!IS_SET(ch->hatefield, HATE_SEX))
      SET_BIT(ch->hatefield, HATE_SEX);
    ch->hates.sex = parm;
    break;
  case OP_RACE:
    if (!IS_SET(ch->hatefield, HATE_RACE))
      SET_BIT(ch->hatefield, HATE_RACE);
    ch->hates.race = parm;
    break;
  case OP_GOOD:
    if (!IS_SET(ch->hatefield, HATE_GOOD))
      SET_BIT(ch->hatefield, HATE_GOOD);
    ch->hates.good = parm;
    break;
  case OP_EVIL:
    if (!IS_SET(ch->hatefield, HATE_EVIL))
      SET_BIT(ch->hatefield, HATE_EVIL);
    ch->hates.evil = parm;
    break;
  case OP_VNUM:
    if (!IS_SET(ch->hatefield, HATE_VNUM))
      SET_BIT(ch->hatefield, HATE_VNUM);
    ch->hates.vnum = parm;
    break;
  }
  if (!MOB_FLAGGED(ch, MOB_HATEFUL)) {
    SET_BIT(MOB_FLAGS(ch), MOB_HATEFUL);
  }
  return (TRUE);
}


int RemHatred(struct char_data *ch, unsigned short bitv)
{
   REMOVE_BIT(ch->hatefield, bitv);

   if (!ch->hatefield)
      REMOVE_BIT(MOB_FLAGS(ch), MOB_HATEFUL);
   return (TRUE);
}

int Hates(struct char_data *ch, struct char_data *v)
{
  struct char_list *i;

  if (IS_AFFECTED(ch, AFF_PETRIFIED))
    return(FALSE);

  if (ch == v)
    return(FALSE);

  if (IS_SET(ch->hatefield, HATE_CHAR)) {
    if (ch->hates.clist) {
      for (i = ch->hates.clist; i; i = i->next) {
	if (i->op_ch) {
	  if ((i->op_ch == v) &&
	    (!strcmp(i->name, GET_NAME(v))))
	    return(TRUE);
	} else {
	  if (!strcmp(i->name, GET_NAME(v)))
	    return(TRUE);
	}
      }
    }
  }
  if (IS_SET(ch->hatefield, HATE_RACE)) {
    if (ch->hates.race != -1) {
      if (ch->hates.race == GET_RACE(v)) {
	char buf[256];
	sprintf(buf, "You hate %d", GET_RACE(v));
	send_to_char(buf, ch);
	return(TRUE);
      }
    }
  }

  if (IS_SET(ch->hatefield, HATE_SEX)) {
    if (ch->hates.sex == GET_SEX(v))
      return(TRUE);
  }
  if (IS_SET(ch->hatefield, HATE_GOOD)) {
    if (ch->hates.good < GET_ALIGNMENT(v))
      return(TRUE);
  }
  if (IS_SET(ch->hatefield, HATE_EVIL)) {
    if (ch->hates.evil > GET_ALIGNMENT(v))
      return(TRUE);
  }
  if (IS_SET(ch->hatefield, HATE_VNUM)) {
    if (ch->hates.vnum == mob_index[v->nr].vnumber)
      return(TRUE);
  }
  return(FALSE);
}


int RemFeared(struct char_data *ch, struct char_data *pud)
{

  struct char_list *oldpud, *t, *tmp;

  if (!MOB_FLAGGED(ch, MOB_AFRAID))
    return(FALSE);

  if (pud && (ch->fears.clist!=0)) {
    tmp = ch->fears.clist;
    for (oldpud = ch->fears.clist; (oldpud!=0); oldpud = tmp) {
      if (oldpud==0) return(FALSE);
      tmp = oldpud->next;
      if (oldpud->op_ch) {
	if (oldpud->op_ch == pud) {
	  t = oldpud;
	  if (ch->fears.clist == t) {
	    ch->fears.clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = ch->fears.clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      } else {
	if (!strcmp(oldpud->name,GET_NAME(pud))) {
	  t = oldpud;
	  if (ch->fears.clist == t) {
	    ch->fears.clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = ch->fears.clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      }
    }
  }
  if (!ch->fears.clist)
    REMOVE_BIT(ch->fearfield, FEAR_CHAR);
  if (!ch->fearfield)
    REMOVE_BIT(MOB_FLAGS(ch), MOB_AFRAID);
  return( (pud) ? TRUE : FALSE);
}


int Fears(struct char_data *ch, struct char_data *v)
{
  struct char_list *i;

  if (IS_AFFECTED(ch, AFF_PETRIFIED))
    return(FALSE);

  if (!MOB_FLAGGED(ch, MOB_AFRAID))
    return(FALSE);

  if (IS_SET(ch->fearfield, FEAR_CHAR)) {
    if (ch->fears.clist) {
      for (i = ch->fears.clist; i; i = i->next) {
	if (i) {
	  if (i->op_ch) {
	    if (i->name[0] != '\0') {
	      if ((i->op_ch == v) &&
		  (!strcmp(i->name, GET_NAME(v))))
		return(TRUE);
	    } else {
	      /* lets see if this clears the problem */
	      RemFeared(ch, i->op_ch);
	    }
	  } else {
	    if (i->name[0] != '\0') {
	      if (!strcmp(i->name, GET_NAME(v)))
		return(TRUE);
	    }
	  }
	}
      }
    }
  }
  if (IS_SET(ch->fearfield, FEAR_RACE)) {
    if (ch->fears.race != -1) {
      if (ch->fears.race == GET_RACE(v))
	return(TRUE);
    }
  }
  if (IS_SET(ch->fearfield, FEAR_SEX)) {
    if (ch->fears.sex == GET_SEX(v))
      return(TRUE);
  }
  if (IS_SET(ch->fearfield, FEAR_GOOD)) {
    if (ch->fears.good < GET_ALIGNMENT(v))
      return(TRUE);
  }
  if (IS_SET(ch->fearfield, FEAR_EVIL)) {
    if (ch->fears.evil > GET_ALIGNMENT(v))
      return(TRUE);
  }
  if (IS_SET(ch->fearfield, FEAR_VNUM)) {
    if (ch->fears.vnum == mob_index[v->nr].vnumber)
      return(TRUE);
  }
  return(FALSE);
}


int AddFeared(struct char_data *ch, struct char_data *pud)
{

  struct char_list *newpud;

  if (pud) {

    if (!CAN_SEE(ch, pud))
      return(FALSE);

    CREATE(newpud, struct char_list, 1);
    newpud->op_ch = pud;
    strcpy(newpud->name,GET_NAME(pud));
    newpud->next = ch->fears.clist;
    ch->fears.clist = newpud;
    
    if (!MOB_FLAGGED(ch, MOB_AFRAID)) {
      SET_BIT(MOB_FLAGS(ch), MOB_AFRAID);
    }
    if (!IS_SET(ch->fearfield, FEAR_CHAR)) {
      SET_BIT(ch->fearfield, FEAR_CHAR);
    }
    if (IS_IMMORTAL(pud))
      send_to_char("---Someone fears you (as well they should).\n\r",pud);
  } 
  
  return( (pud) ? TRUE : FALSE);
}


int AddFears(struct char_data *ch, int parm_type, int parm)
{
  switch(parm_type) {
  case OP_SEX :
    if (!IS_SET(ch->fearfield, FEAR_SEX))
      SET_BIT(ch->fearfield, FEAR_SEX);
    ch->fears.sex = parm;
    break;
  case OP_RACE:
    if (!IS_SET(ch->fearfield, FEAR_RACE))
      SET_BIT(ch->fearfield, FEAR_RACE);
    ch->fears.race = parm;
    break;
  case OP_GOOD:
    if (!IS_SET(ch->fearfield, FEAR_GOOD))
      SET_BIT(ch->fearfield, FEAR_GOOD);
    ch->fears.good = parm;
    break;
  case OP_EVIL:
    if (!IS_SET(ch->fearfield, FEAR_EVIL))
      SET_BIT(ch->fearfield, FEAR_EVIL);
    ch->fears.evil = parm;
    break;
  case OP_VNUM:
    if (!IS_SET(ch->fearfield, FEAR_VNUM))
      SET_BIT(ch->fearfield, FEAR_VNUM);
    ch->fears.vnum = parm;
    break;
  }
  if (!MOB_FLAGGED(ch, MOB_AFRAID)) {
     SET_BIT(MOB_FLAGS(ch), MOB_AFRAID);
   }
  return (TRUE);
}


struct char_data *FindAHatee(struct char_data *ch)
{
   struct char_data *tmp_ch;

   if (ch->in_room < 0)
     return(0);
  
   for (tmp_ch = world[ch->in_room].people; tmp_ch; 
	tmp_ch = tmp_ch->next_in_room) {
       if (Hates(ch, tmp_ch) && (CAN_SEE(ch, tmp_ch))) {
	  if (ch->in_room == tmp_ch->in_room) {
	    if (ch != tmp_ch) {
	       return(tmp_ch);
	     } else {
	       RemHated(ch,tmp_ch);
	       return(0);
	     }
	  }
	}
     }
     return(0);
}

struct char_data *FindAFearee(struct char_data *ch) 
{
   struct char_data *tmp_ch;

   if (ch->in_room < 0)
     return(0);

   for (tmp_ch = world[ch->in_room].people; tmp_ch; 
	tmp_ch = tmp_ch->next_in_room) {
       if (Fears(ch, tmp_ch) && (CAN_SEE(ch, tmp_ch))) {
	  if ((ch->in_room == tmp_ch->in_room) &&
	    (ch != tmp_ch)) {
	    return(tmp_ch);
	  }
	}
     }
  return(0);
}


/*
  these two procedures zero out the character pointer
  for quiting players, without removing names
  thus the monsters will still hate them
*/
void ZeroHatred(struct char_data *ch, struct char_data *v)
{

  struct char_list *oldpud;

  for (oldpud = ch->hates.clist; oldpud; oldpud = oldpud->next) {
    if (oldpud) {
      if (oldpud->op_ch) {
	if (oldpud->op_ch == v) {
	  oldpud->op_ch = 0;
	}
      }
    }
  }
}


void ZeroFeared(struct char_data *ch, struct char_data *v)
{

  struct char_list *oldpud;

  for (oldpud = ch->fears.clist; oldpud; oldpud = oldpud->next) {
    if (oldpud) {
      if (oldpud->op_ch) {
	if (oldpud->op_ch == v) {
	  oldpud->op_ch = 0;
	}
      }
    }
  }
}


/*
  these two are to make the monsters completely forget about them.
*/
void DeleteHatreds(struct char_data *ch)
{

  struct char_data *i;
  extern struct char_data *character_list;

  for (i = character_list; i; i = i->next) {
    if (Hates(i, ch))
      RemHated(i, ch);
  }

}


void DeleteFears(struct char_data *ch)
{
  struct char_data *i;
  extern struct char_data *character_list;

  
  for (i = character_list; i; i = i->next) {
    if (Fears(i, ch))
      RemFeared(i, ch);
  }
}


void create_thiefcatcher(struct char_data *keeper, struct char_data *thief)
{
    struct char_data *mob;
    int i;

    struct tc_data {
        int  zone_nr;
	long mob_num;
        long start_room;
        long weapon;
    } tc_info[] = {
        /* default entry */
        {30, 3100, 3039, 3021},
        /* add new entries after this line */

	{ -1, -1, -1, -1}
    };    

    for (i = 0; tc_info[i].zone_nr != -1; i++) {
      if (tc_info[i].zone_nr == world[keeper->in_room].zone)
        break; 
    }
    /* couldn't find entry in table, use default (Elrhinn) */
    if (tc_info[i].zone_nr == -1)
      i = 0;
    /* now load up the mobile */
    mob = read_mobile(tc_info[i].mob_num, VNUMBER);
    equip_char(mob, read_object(tc_info[i].weapon, VNUMBER), WEAR_WIELD);
    char_to_room(mob, real_room(tc_info[i].start_room));
    HUNTING(mob) = thief;
    hunt_victim(mob);
}
