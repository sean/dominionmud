/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  OPINION.C                                    Based on CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
*                                                                         *
*  Original Diku Mud copyright (c) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Circle Mud improvements copyright (c) 1994 by Jeremy "Ras" Elson.      *
*                                                                         *
*  Dominion Mud improvements copyright (c) 1994, 1995, 1996, 1997,        *
*  2000, 2001 by Sean Mountcastle, John Olvey and Kevin Huff              *
\*********************************************************************** */

#define __OPINION_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

#define GET_HATES(ch)       ((ch)->mob->hates)
#define GET_FEARS(ch)       ((ch)->mob->fears)
#define GET_HATEFIELD(ch)   ((ch)->mob->hatefield)
#define GET_FEARFIELD(ch)   ((ch)->mob->fearfield)

/* globals */
extern struct index_data *mob_index;
extern struct room_data *world;

/**
 * @param ch the mob to free all of the hates for
 */
void FreeHates(struct char_data *ch)
{
  struct char_list *k, *n;

  for (k = GET_HATES(ch).clist; k; k = n) {
      n = k->next;
      free(n);
  }
}

/**
 * @param ch the mob to free all of the fears for
 */
void FreeFears(struct char_data *ch)
{
  struct char_list *k, *n;

  for (k = GET_FEARS(ch).clist; k; k = n) {
      n = k->next;
      free(n);
  }
}


int RemHated(struct char_data *ch, struct char_data *pud)
{
  struct char_list *oldpud, *t;

  if (pud) {
     for (oldpud = GET_HATES(ch).clist; oldpud; oldpud = oldpud->next) {
       if (!oldpud) return(FALSE);
       if (oldpud->op_ch) {
	if (oldpud->op_ch == pud) {
	  t = oldpud;
	  if (GET_HATES(ch).clist == t) {
	    GET_HATES(ch).clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = GET_HATES(ch).clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      } else {
	if (!strcmp(oldpud->name,GET_NAME(pud))) {
	  t = oldpud;
	  if (GET_HATES(ch).clist == t) {
	    GET_HATES(ch).clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = GET_HATES(ch).clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      }
     }
  }

  if (!GET_HATES(ch).clist) {
     REMOVE_BIT(GET_HATEFIELD(ch), HATE_CHAR);
  }
  if (!GET_HATEFIELD(ch))
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
    newpud->next = GET_HATES(ch).clist;
    GET_HATES(ch).clist = newpud;
    if (!MOB_FLAGGED(ch, MOB_HATEFUL)) {
      SET_BIT(MOB_FLAGS(ch), MOB_HATEFUL);
    }
    if (!IS_SET(GET_HATEFIELD(ch), HATE_CHAR)) {
      SET_BIT(GET_HATEFIELD(ch), HATE_CHAR);
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
    if (!IS_SET(GET_HATEFIELD(ch), HATE_SEX))
      SET_BIT(GET_HATEFIELD(ch), HATE_SEX);
    GET_HATES(ch).sex = parm;
    break;
  case OP_RACE:
    if (!IS_SET(GET_HATEFIELD(ch), HATE_RACE))
      SET_BIT(GET_HATEFIELD(ch), HATE_RACE);
    GET_HATES(ch).race = parm;
    break;
  case OP_GOOD:
    if (!IS_SET(GET_HATEFIELD(ch), HATE_GOOD))
      SET_BIT(GET_HATEFIELD(ch), HATE_GOOD);
    GET_HATES(ch).good = parm;
    break;
  case OP_EVIL:
    if (!IS_SET(GET_HATEFIELD(ch), HATE_EVIL))
      SET_BIT(GET_HATEFIELD(ch), HATE_EVIL);
    GET_HATES(ch).evil = parm;
    break;
  case OP_VNUM:
    if (!IS_SET(GET_HATEFIELD(ch), HATE_VNUM))
      SET_BIT(GET_HATEFIELD(ch), HATE_VNUM);
    GET_HATES(ch).vnum = parm;
    break;
  }
  if (!MOB_FLAGGED(ch, MOB_HATEFUL)) {
    SET_BIT(MOB_FLAGS(ch), MOB_HATEFUL);
  }
  return (TRUE);
}


int RemHatred(struct char_data *ch, unsigned short bitv)
{
   REMOVE_BIT(GET_HATEFIELD(ch), bitv);

   if (!GET_HATEFIELD(ch))
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

  if (IS_SET(GET_HATEFIELD(ch), HATE_CHAR)) {
    if (GET_HATES(ch).clist) {
      for (i = GET_HATES(ch).clist; i; i = i->next) {
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
  if (IS_SET(GET_HATEFIELD(ch), HATE_RACE)) {
    if (GET_HATES(ch).race != -1) {
      if (GET_HATES(ch).race == GET_RACE(v)) {
	char buf[256];
	sprintf(buf, "You hate %d", GET_RACE(v));
	send_to_char(buf, ch);
	return(TRUE);
      }
    }
  }

  if (IS_SET(GET_HATEFIELD(ch), HATE_SEX)) {
    if (GET_HATES(ch).sex == GET_SEX(v))
      return(TRUE);
  }
  if (IS_SET(GET_HATEFIELD(ch), HATE_GOOD)) {
    if (GET_HATES(ch).good < GET_ALIGNMENT(v))
      return(TRUE);
  }
  if (IS_SET(GET_HATEFIELD(ch), HATE_EVIL)) {
    if (GET_HATES(ch).evil > GET_ALIGNMENT(v))
      return(TRUE);
  }
  if (IS_SET(GET_HATEFIELD(ch), HATE_VNUM)) {
    if (GET_HATES(ch).vnum == mob_index[v->mob->nr].vnumber)
      return(TRUE);
  }
  return(FALSE);
}


int RemFeared(struct char_data *ch, struct char_data *pud)
{

  struct char_list *oldpud, *t, *tmp;

  if (!MOB_FLAGGED(ch, MOB_AFRAID))
    return(FALSE);

  if (pud && (GET_FEARS(ch).clist!=0)) {
    tmp = GET_FEARS(ch).clist;
    for (oldpud = GET_FEARS(ch).clist; (oldpud!=0); oldpud = tmp) {
      if (oldpud==0) return(FALSE);
      tmp = oldpud->next;
      if (oldpud->op_ch) {
	if (oldpud->op_ch == pud) {
	  t = oldpud;
	  if (GET_FEARS(ch).clist == t) {
	    GET_FEARS(ch).clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = GET_FEARS(ch).clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      } else {
	if (!strcmp(oldpud->name,GET_NAME(pud))) {
	  t = oldpud;
	  if (GET_FEARS(ch).clist == t) {
	    GET_FEARS(ch).clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = GET_FEARS(ch).clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      }
    }
  }
  if (!GET_FEARS(ch).clist)
    REMOVE_BIT(GET_FEARFIELD(ch), FEAR_CHAR);
  if (!GET_FEARFIELD(ch))
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

  if (IS_SET(GET_FEARFIELD(ch), FEAR_CHAR)) {
    if (GET_FEARS(ch).clist) {
      for (i = GET_FEARS(ch).clist; i; i = i->next) {
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
  if (IS_SET(GET_FEARFIELD(ch), FEAR_RACE)) {
    if (GET_FEARS(ch).race != -1) {
      if (GET_FEARS(ch).race == GET_RACE(v))
	return(TRUE);
    }
  }
  if (IS_SET(GET_FEARFIELD(ch), FEAR_SEX)) {
    if (GET_FEARS(ch).sex == GET_SEX(v))
      return(TRUE);
  }
  if (IS_SET(GET_FEARFIELD(ch), FEAR_GOOD)) {
    if (GET_FEARS(ch).good < GET_ALIGNMENT(v))
      return(TRUE);
  }
  if (IS_SET(GET_FEARFIELD(ch), FEAR_EVIL)) {
    if (GET_FEARS(ch).evil > GET_ALIGNMENT(v))
      return(TRUE);
  }
  if (IS_SET(GET_FEARFIELD(ch), FEAR_VNUM)) {
    if (GET_FEARS(ch).vnum == mob_index[v->mob->nr].vnumber)
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
    newpud->next = GET_FEARS(ch).clist;
    GET_FEARS(ch).clist = newpud;
    
    if (!MOB_FLAGGED(ch, MOB_AFRAID)) {
      SET_BIT(MOB_FLAGS(ch), MOB_AFRAID);
    }
    if (!IS_SET(GET_FEARFIELD(ch), FEAR_CHAR)) {
      SET_BIT(GET_FEARFIELD(ch), FEAR_CHAR);
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
    if (!IS_SET(GET_FEARFIELD(ch), FEAR_SEX))
      SET_BIT(GET_FEARFIELD(ch), FEAR_SEX);
    GET_FEARS(ch).sex = parm;
    break;
  case OP_RACE:
    if (!IS_SET(GET_FEARFIELD(ch), FEAR_RACE))
      SET_BIT(GET_FEARFIELD(ch), FEAR_RACE);
    GET_FEARS(ch).race = parm;
    break;
  case OP_GOOD:
    if (!IS_SET(GET_FEARFIELD(ch), FEAR_GOOD))
      SET_BIT(GET_FEARFIELD(ch), FEAR_GOOD);
    GET_FEARS(ch).good = parm;
    break;
  case OP_EVIL:
    if (!IS_SET(GET_FEARFIELD(ch), FEAR_EVIL))
      SET_BIT(GET_FEARFIELD(ch), FEAR_EVIL);
    GET_FEARS(ch).evil = parm;
    break;
  case OP_VNUM:
    if (!IS_SET(GET_FEARFIELD(ch), FEAR_VNUM))
      SET_BIT(GET_FEARFIELD(ch), FEAR_VNUM);
    GET_FEARS(ch).vnum = parm;
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

  for (oldpud = GET_HATES(ch).clist; oldpud; oldpud = oldpud->next) {
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

  for (oldpud = GET_FEARS(ch).clist; oldpud; oldpud = oldpud->next) {
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
