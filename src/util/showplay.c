/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SHOWPLAY.C                                   Based on CircleMUD *
*  Usage: Printing the important parts of the pfile outside of the game   *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#include "../conf.h"
#include "../sysdep.h"
#include "../structs.h"
#include "../utils.h"

long memory_usage; 

#ifdef __cplusplus
template<class kind> 
kind MIN(kind a, kind b)
{
  return a < b ? a : b;
}

template<class kind> 
kind MAX(kind a, kind b)
{
  return a > b ? a : b;
}
#else
long MIN(long a, long b) { return a < b ? a : b; }
long MAX(long a, long b) { return a > b ? a : b; }
#endif

/* Create a duplicate of a string */
char *str_dup(const char *source)
{
  char *new;

  CREATE(new, char, strlen(source) + 1);
  return (strcpy(new, source));
}


/* clear ALL the working variables of a char; do NOT free any space alloc'ed */
void clear_char(struct char_data * ch)
{
  int i;

  memset((char *) ch, 0, sizeof(struct char_data));

  ch->in_room                  = NOWHERE;
  GET_PFILEPOS(ch)             = -1;
  GET_WAS_IN(ch)               = NOWHERE;
  GET_POS(ch)                  = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
}

/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u * st, struct char_data * ch)
{
  int i;

  /* to save memory, only PC's -- not MOB's -- have player_specials */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  GET_SEX(ch)         = st->sex;
  GET_RACE(ch)        = st->race;
  GET_REL(ch)         = st->assocs[0];
  GET_CLAN(ch)        = st->assocs[1];
  GET_GUILD_LEV(ch)   = st->assocs[2];
  GET_LEVEL(ch)       = st->level;
  GET_AGE_MOD(ch)     = st->race_mod;
  /*GET_INVITE(ch, 0)   = st->invitations[0];
  GET_INVITE(ch, 1)   = st->invitations[1];
  GET_INVITE(ch, 2)   = st->invitations[2];*/

  ch->player.short_descr = NULL;
  ch->player.long_descr  = NULL;
  ch->player.title       = str_dup(st->title);
  ch->player.description = str_dup(st->description);
  ch->char_specials.email = str_dup(st->email);

  ch->player.hometown = st->hometown;
  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->real_abils             = st->abilities;
  ch->aff_abils              = st->abilities;
  ch->points                 = st->points;
  ch->char_specials.saved    = st->char_specials_saved;
  ch->player_specials->saved = st->player_specials_saved;
  POOFIN(ch)                 = str_dup(st->spoofin);
  POOFOUT(ch)                = str_dup(st->spoofout);

  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;
  for (i = 0; i < ARMOR_LIMIT; i++)
      ch->points.armor[i] = 0;
  GET_HITROLL(ch) = MIN(100, (GET_LEVEL(ch) * 2) + 3);
  GET_HITROLL(ch) = MAX(3, GET_HITROLL(ch));
  GET_DAMROLL(ch) = MIN(100, (GET_LEVEL(ch) * 2));
  GET_DAMROLL(ch) = MAX(3, GET_DAMROLL(ch));

  if (ch->player.name == NULL)
    CREATE(ch->player.name, char, strlen(st->name) + 1);
  strcpy(ch->player.name, st->name);
  strcpy(ch->player.passwd, st->pwd);

  /* Set number of followers to 0 */
  GET_FOLS(ch) = 0;

  ch->in_room = GET_LOADROOM(ch);

  /*
   * If you're not poisioned and you've been away for more than an hour of
   * real time, we'll set your HMV back to full
   */

  if ((!IS_AFFECTED2(ch, AFF_POISON_I)   &&
       !IS_AFFECTED2(ch, AFF_POISON_II)  &&
       !IS_AFFECTED2(ch, AFF_POISON_III) &&
       !IS_AFFECTED2(ch, AFF_DISEASE))   &&
      (((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR)) {
    GET_HIT(ch)  = GET_MAX_HIT(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);
  }
}                               /* store_to_char */


/* release memory allocated for a char struct */
void free_char(struct char_data * ch)
{
  int i;
  struct alias *a;

  void free_alias(struct alias * a);

  if (ch == NULL) {
     abort();
  }

  if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1)) {
    /* if this is a player, or a non-prototyped non-player, free all */
    if (GET_NAME(ch))
      free(GET_NAME(ch));
    if (GET_RDESC(ch))
      free(GET_RDESC(ch));
    if (GET_KWDS(ch))
      free(GET_KWDS(ch));
    if (ch->player.title)
      free(ch->player.title);
    if (ch->player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description)
      free(ch->player.description);
  } else if ((i = GET_MOB_RNUM(ch)) > -1) {
    /* otherwise, free strings only if the string is not pointing at proto */
    if (ch->player.name)
      free(ch->player.name);
    if (ch->player.title)
      free(ch->player.title);
    if (ch->player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description)
      free(ch->player.description);
  }

  free(ch);
}


void show(char *filename)
{
  char sexname;
  char racename[20];
  FILE *fl;
  struct char_file_u player;
  struct char_data *victim;
  int num = 0;

  if (!(fl = fopen(filename, "r+"))) {
    perror("error opening playerfile");
    exit(1);
  }
  for (;;) {
    fread(&player, sizeof(struct char_file_u), 1, fl);
    if (feof(fl)) {
      fclose(fl);
      exit(1);
    }
    CREATE(victim, struct char_data, 1);
    clear_char(victim);
    store_to_char(&player, victim);

    switch (GET_RACE(victim)) {
    case RACE_HUMAN:
      strcpy(racename, "Hum");
      break;
    default:
      strcpy(racename, "Unk");
      break;
    }
    switch (GET_SEX(victim)) {
    case SEX_FEMALE:
      sexname = 'F';
      break;
    case SEX_MALE:
      sexname = 'M';
      break;
    case SEX_NEUTRAL:
      sexname = 'N';
      break;
    default:
      sexname = '-';
      break;
    }

    printf("%5d. ID: %5ld (%c) [%2d %s] %-16s %s\n", ++num,
	   player.char_specials_saved.idnum, sexname, player.level,
	   racename, player.name, victim->char_specials.email ?
	   victim->char_specials.email : "NONE!");

    free_char(victim);
  }
}


int main(int argc, char **argv)
{
  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else
    show(argv[1]);

  return 0;
}
