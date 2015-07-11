/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  OBJSAVE.C                                    Based on CircleMUD *
*  Usage: Loading/saving player objects for rent and crash-save           *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __OBJSAVE_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/* these factors should be unique integers */
#define RENT_FACTOR     1
#define CRYO_FACTOR     4

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int    top_of_p_table;
extern int    min_rent_cost;

struct obj_data * Obj_from_store_to(struct obj_file_elem object, int *loc);

/* Extern functions */
ACMD(do_tell);
SPECIAL(receptionist);
SPECIAL(cryogenicist);
ACMD(do_action);

struct obj_data *Obj_from_store(struct obj_file_elem object)
{
  int locate;

  return (Obj_from_store_to(object, &locate));
#if 0
  struct obj_data *obj;
  int j;

  if (real_object(object.item_number) > -1) {
    obj = read_object(object.item_number, VNUMBER);
    GET_OBJ_VAL(obj, 0) = object.value[0];
    GET_OBJ_VAL(obj, 1) = object.value[1];
    GET_OBJ_VAL(obj, 2) = object.value[2];
    GET_OBJ_VAL(obj, 3) = object.value[3];
    GET_OBJ_VAL(obj, 4) = object.value[4];
    GET_OBJ_VAL(obj, 5) = object.value[5];
    GET_OBJ_VAL(obj, 6) = object.value[6];
    GET_OBJ_VAL(obj, 7) = object.value[7];
    GET_OBJ_VAL(obj, 8) = object.value[8];
    GET_OBJ_VAL(obj, 9) = object.value[9];
    GET_OBJ_EXTRA(obj) = object.extra_flags;
    GET_OBJ_WEIGHT(obj) = object.weight;
    GET_OBJ_TIMER(obj) = object.timer;
    obj->obj_flags.bitvector = object.bitvector;

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
      obj->affected[j] = object.affected[j];

    return obj;
  } else
    return NULL;
#endif
}

struct obj_data *Obj_from_store_to(struct obj_file_elem object, int *locate)
{
  struct obj_data *obj;
  int j;

  if (real_object(object.item_number) > -1) {
    obj = read_object(object.item_number, VNUMBER);
    *locate = (int) object.locate;
    GET_OBJ_VAL(obj, 0) = object.value[0];
    GET_OBJ_VAL(obj, 1) = object.value[1];
    GET_OBJ_VAL(obj, 2) = object.value[2];
    GET_OBJ_VAL(obj, 3) = object.value[3];
    GET_OBJ_VAL(obj, 4) = object.value[4];
    GET_OBJ_VAL(obj, 5) = object.value[5];
    GET_OBJ_VAL(obj, 6) = object.value[6];
    GET_OBJ_VAL(obj, 7) = object.value[7];
    GET_OBJ_VAL(obj, 8) = object.value[8];
    GET_OBJ_VAL(obj, 9) = object.value[9];
    GET_OBJ_EXTRA(obj) = object.extra_flags;
    GET_OBJ_WEIGHT(obj) = object.weight;
    GET_OBJ_TIMER(obj) = object.timer;
    obj->obj_flags.bitvector = object.bitvector;

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
      obj->affected[j] = object.affected[j];

    return obj;
  } else
    return NULL;
}

int Obj_to_store_from(struct obj_data * obj, FILE * fl, int locate)
{
  int j;
  struct obj_file_elem object;

  object.item_number = GET_OBJ_VNUM(obj);
  object.locate = (sh_int) locate; /* where worn or inventory? */
  object.value[0] = GET_OBJ_VAL(obj, 0);
  object.value[1] = GET_OBJ_VAL(obj, 1);
  object.value[2] = GET_OBJ_VAL(obj, 2);
  object.value[3] = GET_OBJ_VAL(obj, 3);
  object.value[4] = GET_OBJ_VAL(obj, 4);
  object.value[5] = GET_OBJ_VAL(obj, 5);
  object.value[6] = GET_OBJ_VAL(obj, 6);
  object.value[7] = GET_OBJ_VAL(obj, 7);
  object.value[8] = GET_OBJ_VAL(obj, 8);
  object.value[9] = GET_OBJ_VAL(obj, 9);
  object.extra_flags = GET_OBJ_EXTRA(obj);
  object.weight = GET_OBJ_WEIGHT(obj);
  object.timer = GET_OBJ_TIMER(obj);
  object.bitvector = obj->obj_flags.bitvector;
  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    object.affected[j] = obj->affected[j];

  if (fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1) {
    perror("Error writing object in Obj_to_store");
    return 0;
  }
  return 1;
}


int Obj_to_store(struct obj_data * obj, FILE * fl)
{
  return (Obj_to_store_from(obj, fl, 0));
#if 0
  int j;
  struct obj_file_elem object;

  object.item_number = GET_OBJ_VNUM(obj);
  object.value[0] = GET_OBJ_VAL(obj, 0);
  object.value[1] = GET_OBJ_VAL(obj, 1);
  object.value[2] = GET_OBJ_VAL(obj, 2);
  object.value[3] = GET_OBJ_VAL(obj, 3);
  object.value[4] = GET_OBJ_VAL(obj, 4);
  object.value[5] = GET_OBJ_VAL(obj, 5);
  object.value[6] = GET_OBJ_VAL(obj, 6);
  object.value[7] = GET_OBJ_VAL(obj, 7);
  object.value[8] = GET_OBJ_VAL(obj, 8);
  object.value[9] = GET_OBJ_VAL(obj, 9);
  object.extra_flags = GET_OBJ_EXTRA(obj);
  object.weight = GET_OBJ_WEIGHT(obj);
  object.timer = GET_OBJ_TIMER(obj);
  object.bitvector = obj->obj_flags.bitvector;
  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    object.affected[j] = obj->affected[j];

  if (fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1) {
    perror("Error writing object in Obj_to_store");
    return 0;
  }
  return 1;
#endif
}


/* so this is gonna be the auto equip (hopefully) */
void auto_equip(struct char_data *ch, struct obj_data *obj, int locate)
{
  int j;

  if (locate > 0) { /* was worn */
    switch (j = locate-1) {
      case WEAR_FINGER_R:
      case WEAR_FINGER_L:
	if (!CAN_WEAR(obj,ITEM_WEAR_FINGER))
	  locate = 0;
	break;
      case WEAR_NECK_1:
      case WEAR_NECK_2:
	if (!CAN_WEAR(obj,ITEM_WEAR_NECK))
	  locate = 0;
	break;
      case WEAR_BODY:
	if (!CAN_WEAR(obj,ITEM_WEAR_BODY))
	  locate = 0;
	break;
      case WEAR_HEAD:
	if (!CAN_WEAR(obj,ITEM_WEAR_HEAD))
	  locate = 0;
	break;
      case WEAR_LEGS:
	if (!CAN_WEAR(obj,ITEM_WEAR_LEGS))
	  locate = 0;
	break;
      case WEAR_FOOT_R:
      case WEAR_FOOT_L:
	if (!CAN_WEAR(obj,ITEM_WEAR_FEET))
	  locate = 0;
	break;
      case WEAR_HAND_R:
      case WEAR_HAND_L:
	if (!CAN_WEAR(obj,ITEM_WEAR_HANDS))
	  locate = 0;
	break;
      case WEAR_ARM_R:
      case WEAR_ARM_L:
	if (!CAN_WEAR(obj,ITEM_WEAR_ARMS))
	  locate = 0;
	break;
      case WEAR_BACK:
	if (!CAN_WEAR(obj,ITEM_WEAR_BACK))
	  locate = 0;
	break;
      case WEAR_WAIST:
	if (!CAN_WEAR(obj,ITEM_WEAR_WAIST))
	  locate = 0;
	break;
      case WEAR_WRIST_R:
      case WEAR_WRIST_L:
	if (!CAN_WEAR(obj,ITEM_WEAR_WRIST))
	  locate = 0;
	break;
      case WEAR_WIELD:
      case WEAR_HOLD:
	if (!CAN_WEAR(obj, ITEM_WEAR_HOLD) &&
            !CAN_WEAR(obj, ITEM_WEAR_TAKE))
	  locate = 0;
	break;
      default:
	locate = 0;
    }
    if (locate > 0)
      if (!GET_EQ(ch,j)) {
/* check ch's alignment to prevent $M from being zapped through auto-equip */
	if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
	    (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
	    (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
	  locate = 0;
	else
	  equip_char(ch, obj, j);
	} else  /* oops - saved player with double equipment[j]? */
	  locate = 0;
   }
   if (locate <= 0)
     obj_to_char(obj, ch);
}


#define MAX_BAG_ROW 5
/*
 * should be enough - who would carry a bag in a bag in a bag in a
 *  bag in a bag in a bag ?!?
 */


int Crash_delete_file(char *name)
{
  char filename[50];
  FILE *fl;

  if (!get_filename(name, filename, CRASH_FILE))
    return 0;
  if (!(fl = fopen(filename, "rb"))) {
    if (errno != ENOENT) {      /* if it fails but NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (1)", filename);
      perror(buf1);
    }
    return 0;
  }
  fclose(fl);

  if (unlink(filename) < 0) {
    if (errno != ENOENT) {      /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (2)", filename);
      perror(buf1);
    }
  }
  return (1);
}


int Crash_delete_crashfile(struct char_data * ch)
{
  char fname[MAX_INPUT_LENGTH];
  struct rent_info rent;
  FILE *fl;

  if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
    return 0;
  if (!(fl = fopen(fname, "rb"))) {
    if (errno != ENOENT) {      /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: checking for crash file %s (3)", fname);
      perror(buf1);
    }
    return 0;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  fclose(fl);

  if (rent.rentcode == RENT_CRASH)
    Crash_delete_file(GET_NAME(ch));

  return 1;
}


int Crash_clean_file(char *name)
{
  char fname[MAX_STRING_LENGTH], filetype[20];
  struct rent_info rent;
  extern int rent_file_timeout, crash_file_timeout;
  FILE *fl;

  if (!get_filename(name, fname, CRASH_FILE))
    return 0;
  /*
   * open for write so that permission problems will be flagged now, at boot
   * time.
   */
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {      /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: OPENING OBJECT FILE %s (4)", fname);
      perror(buf1);
    }
    return 0;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  fclose(fl);

#define LIMITED_ITEMS 0

#if LIMITED_ITEMS
  for (j = 0; j < rent.nitems; j++)
      if (IS_OBJ_STAT(obj_elem.extra_flags, ITEM_LIMITED)) {
	 limited_objs[i] = obj_elem.item_number;
	 i++;
      }
#endif

  if ((rent.rentcode == RENT_CRASH) ||
      (rent.rentcode == RENT_FORCED) || (rent.rentcode == RENT_TIMEDOUT)) {
    if (rent.time < time(0) - (crash_file_timeout * SECS_PER_REAL_DAY)) {
      Crash_delete_file(name);
      switch (rent.rentcode) {
      case RENT_CRASH:
	strcpy(filetype, "crash");
	break;
      case RENT_FORCED:
	strcpy(filetype, "forced rent");
	break;
      case RENT_TIMEDOUT:
	strcpy(filetype, "idlesave");
	break;
      default:
	strcpy(filetype, "UNKNOWN!");
	break;
      }
      sprintf(buf, "    Deleting %s's %s file.", name, filetype);
      log(buf);
      return 1;
    }
    /* Must retrieve rented items w/in 30 days */
  } else if (rent.rentcode == RENT_RENTED)
    if (rent.time < time(0) - (rent_file_timeout * SECS_PER_REAL_DAY)) {
      Crash_delete_file(name);
      sprintf(buf, "    Deleting %s's rent file.", name);
      log(buf);
      return 1;
    }
  return (0);
}



void update_obj_file(void)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    Crash_clean_file((player_table + i)->name);
  return;
}



void Crash_listrent(struct char_data * ch, char *name)
{
  FILE *fl;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  struct obj_data *obj;
  struct rent_info rent;

  if (!get_filename(name, fname, CRASH_FILE))
    return;
  if (!(fl = fopen(fname, "rb"))) {
    sprintf(buf, "%s has no rent file.\r\n", name);
    send_to_char(buf, ch);
    return;
  }
  sprintf(buf, "%s\r\n", fname);
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  switch (rent.rentcode) {
  case RENT_RENTED:
    strcat(buf, "Rent\r\n");
    break;
  case RENT_CRASH:
    strcat(buf, "Crash\r\n");
    break;
  case RENT_CRYO:
    strcat(buf, "Cryo\r\n");
    break;
  case RENT_TIMEDOUT:
  case RENT_FORCED:
    strcat(buf, "TimedOut\r\n");
    break;
  default:
    strcat(buf, "Undef\r\n");
    break;
  }
  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      fclose(fl);
      return;
    }
    if (!feof(fl))
      if (real_object(object.item_number) > -1) {
	obj = read_object(object.item_number, VNUMBER);
	sprintf(buf, "%s [%ld] (%5dau) %-20s\r\n", buf,
		object.item_number, GET_OBJ_RENT(obj),
		obj->short_description);
	extract_obj(obj);
      }
  }
  send_to_char(buf, ch);
  fclose(fl);
}



int Crash_write_rentcode(struct char_data * ch, FILE * fl, struct rent_info * rent)
{
  if (fwrite(rent, sizeof(struct rent_info), 1, fl) < 1) {
    perror("Writing rent code.");
    return 0;
  }
  return 1;
}



int Crash_load(struct char_data * ch)
/* return values:
	0 - successful load, keep char in rent room.
	1 - load failure or load of crash items -- put char in temple.
	2 - rented equipment lost (no $)
*/
{
  void Crash_crashsave(struct char_data * ch);

  FILE *fl;
  char fname[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  struct rent_info rent;
  int    cost, orig_rent_code;
  float  num_of_days;
  struct obj_data *obj, *obj1;
  int    locate, j;
  struct obj_data *cont_row[MAX_BAG_ROW];

  if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
    return 1;
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {      /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: READING OBJECT FILE %s (5)", fname);
      perror(buf1);
      send_to_char("\r\n********************* NOTICE *********************\r\n"
		   "There was a problem loading your objects from disk.\r\n"
		   "Contact a God for assistance.\r\n", ch);
    }
    sprintf(buf, "%s entering game with no equipment.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    return 1;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  if (rent.rentcode == RENT_RENTED || rent.rentcode == RENT_TIMEDOUT) {
    num_of_days = (float) (time(0) - rent.time) / SECS_PER_REAL_DAY;
    cost = (int) (rent.net_cost_per_diem * num_of_days);
    if (cost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
      fclose(fl);
      sprintf(buf, "%s entering game, rented equipment lost (no $).",
	      GET_NAME(ch));
      mudlog(buf, BRF, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
      Crash_crashsave(ch);
      return 2;
    } else {
      GET_BANK_GOLD(ch) -= MAX((long)cost - GET_GOLD(ch), 0L);
      GET_GOLD(ch) = MAX((long)GET_GOLD(ch) - cost, 0L);
      save_char(ch, NOWHERE);
    }
  }
  switch (orig_rent_code = rent.rentcode) {
  case RENT_RENTED:
    sprintf(buf, "%s un-renting and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  case RENT_CRASH:
    sprintf(buf, "%s retrieving crash-saved items and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  case RENT_CRYO:
    sprintf(buf, "%s un-cryo'ing and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  case RENT_FORCED:
  case RENT_TIMEDOUT:
    sprintf(buf, "%s retrieving force-saved items and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  default:
    sprintf(buf, "WARNING: %s entering game with undefined rent code.", GET_NAME(ch));
    mudlog(buf, BRF, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    break;
  }

  for (j = 0;j < MAX_BAG_ROW;j++)
    cont_row[j] = NULL; /* empty all cont lists (you never know ...) */

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      perror("Reading crash file: Crash_load.");
      fclose(fl);
      return 1;
    }
    if (!feof(fl))
      if ((obj = Obj_from_store_to(object, &locate))) {
	auto_equip(ch, obj, locate);
	if (locate > 0) { /* item equipped */
	  for (j = MAX_BAG_ROW-1;  j > 0 ; --j) {
	    if (cont_row[j]) { /* no container -> back to ch's inventory */
	      for ( ; cont_row[j]; cont_row[j] = obj1) {
		obj1 = cont_row[j]->next_content;
		obj_to_char(cont_row[j], ch);
	      }
	      cont_row[j] = NULL;
	    }
	  }
	  if (cont_row[0]) { /* content list existing */
	    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
	      /* rem item ; fill ; equip again */
	      obj = unequip_char(ch, locate-1);
	      obj->contains = NULL; /* should be empty - but who knows */
	      for ( ; cont_row[0]; cont_row[0] = obj1) {
		obj1 = cont_row[0]->next_content;
		obj_to_obj(cont_row[0], obj);
	      }
	      equip_char(ch, obj, locate-1);
	    } else { /* object isn't container -> empty content list */
	      for ( ; cont_row[0]; cont_row[0] = obj1) {
		obj1 = cont_row[0]->next_content;
		obj_to_char(cont_row[0], ch);
	      }
	      cont_row[0] = NULL;
	    }
	  }
	} else { /* locate <= 0 */
	  for (j = MAX_BAG_ROW-1; j > -locate; j--) {
	    if (cont_row[j]) { /* no container -> back to ch's inventory */
	      for ( ; cont_row[j]; cont_row[j] = obj1) {
		obj1 = cont_row[j]->next_content;
		obj_to_char(cont_row[j], ch);
	      }
	      cont_row[j] = NULL;
	    }
	  }
	  if (j == -locate && cont_row[j]) { /* content list existing */
	    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
	      /* take item ; fill ; give to char again */
	      obj_from_char(obj);
	      obj->contains = NULL;
	      for (; cont_row[j]; cont_row[j] = obj1) {
		obj1 = cont_row[j]->next_content;
		obj_to_obj(cont_row[j], obj);
	      }
	      obj_to_char(obj, ch); /* add to inv first ... */
	    } else { /* object isn't container -> empty content list */
	      for ( ; cont_row[j]; cont_row[j] = obj1) {
		obj1 = cont_row[j]->next_content;
		obj_to_char(cont_row[j], ch);
	      }
	      cont_row[j] = NULL;
	    }
	  }
	  if (locate < 0 && locate >= -MAX_BAG_ROW) {
	    /*
	     * let obj be part of content list but put it at the list's
	     * end thus having the items in the same order as before
	     * renting.
	     */
	    obj_from_char(obj);
	    if ((obj1 = cont_row[-locate-1])) {
	      while (obj1->next_content)
		obj1 = obj1->next_content;
	      obj1->next_content = obj;
	    } else
	      cont_row[-locate-1] = obj;
	  }
	}
    }
#if 0
      obj_to_char(Obj_from_store(object), ch);
#endif
  }

  /* turn this into a crash file by re-writing the control block */
  rent.rentcode = RENT_CRASH;
  rent.time = time(0);
  rewind(fl);
  Crash_write_rentcode(ch, fl, &rent);

  fclose(fl);

  if ((orig_rent_code == RENT_RENTED) || (orig_rent_code == RENT_CRYO))
    return 0;
  else
    return 1;
}



int Crash_save(struct obj_data * obj, FILE * fp, int locate)
{
  struct obj_data *tmp;
  int result;

  if (obj) {
    Crash_save(obj->next_content, fp, locate);
    Crash_save(obj->contains, fp, MIN(0,locate)-1);
    result = Obj_to_store_from(obj, fp, locate);

    for (tmp = obj->in_obj; tmp; tmp = tmp->in_obj)
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

    if (!result)
      return 0;
  }
  return TRUE;
}


void Crash_restore_weight(struct obj_data * obj)
{
  if (obj) {
    Crash_restore_weight(obj->contains);
    Crash_restore_weight(obj->next_content);
    if (obj->in_obj)
      GET_OBJ_WEIGHT(obj->in_obj) += GET_OBJ_WEIGHT(obj);
  }
}



void Crash_extract_objs(struct obj_data * obj)
{
  if (obj) {
    Crash_extract_objs(obj->contains);
    Crash_extract_objs(obj->next_content);
    extract_obj(obj);
  }
}


int Crash_is_unrentable(struct obj_data * obj)
{
  if (!obj)
    return 0;

  if (IS_OBJ_STAT(obj, ITEM_NORENT) || GET_OBJ_RENT(obj) < 0 ||
      GET_OBJ_RNUM(obj) <= NOTHING || GET_OBJ_TYPE(obj) == ITEM_KEY)
    return 1;

  return 0;
}


void Crash_extract_norents(struct obj_data * obj)
{
  if (obj) {
    Crash_extract_norents(obj->contains);
    Crash_extract_norents(obj->next_content);
    if (Crash_is_unrentable(obj))
      extract_obj(obj);
  }
}

void Crash_extract_norents_from_equipped(struct char_data * ch)
{
  int j;

  for (j = 0;j < NUM_WEARS;j++) {
    if (GET_EQ(ch,j)) {
      if (IS_OBJ_STAT(GET_EQ(ch,j), ITEM_NORENT) ||
	  GET_OBJ_RENT(GET_EQ(ch,j)) < 0 ||
	  GET_OBJ_RNUM(GET_EQ(ch,j)) <= NOTHING ||
	  GET_OBJ_TYPE(GET_EQ(ch,j)) == ITEM_KEY)
	obj_to_char(unequip_char(ch,j),ch);
      else
	Crash_extract_norents(GET_EQ(ch,j));
    }
  }
}

void Crash_extract_expensive(struct obj_data * obj)
{
  struct obj_data *tobj, *max;

  max = obj;
  for (tobj = obj; tobj; tobj = tobj->next_content)
    if (GET_OBJ_RENT(tobj) > GET_OBJ_RENT(max))
      max = tobj;
  extract_obj(max);
}



void Crash_calculate_rent(struct obj_data * obj, int *cost)
{
  if (obj) {
    *cost += MAX(0, (int)GET_OBJ_RENT(obj));
    Crash_calculate_rent(obj->contains, cost);
    Crash_calculate_rent(obj->next_content, cost);
  }
}


void Crash_crashsave(struct char_data * ch)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  rent.rentcode = RENT_CRASH;
  rent.time = time(0);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  if (!Crash_save(ch->carrying, fp, 0)) {
    fclose(fp);
    return;
  }
  Crash_restore_weight(ch->carrying);

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j)) {
      if (!Crash_save(GET_EQ(ch, j), fp, j+1)) {
	fclose(fp);
	return;
      }
      Crash_restore_weight(GET_EQ(ch, j));
    }
  fclose(fp);
  REMOVE_BIT(PLR_FLAGS(ch), PLR_CRASH);
}


void Crash_idlesave(struct char_data * ch)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  int cost, cost_eq;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;
#if 0
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j))
      obj_to_char(unequip_char(ch, j), ch);
#endif
  Crash_extract_norents_from_equipped(ch);
  Crash_extract_norents(ch->carrying);

  cost_eq = 0;
  for (j = 0; j < NUM_WEARS; j++)
    Crash_calculate_rent(GET_EQ(ch,j), &cost_eq);
  cost = 0;
  Crash_calculate_rent(ch->carrying, &cost);
  cost <<= 1;                   /* forcerent cost is 2x normal rent */
  cost_eq <<= 1;
  if (cost+cost_eq > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
    for (j = 0; j < NUM_WEARS; j++) /* unequip player with low money */
      if (GET_EQ(ch,j))
	obj_to_char(unequip_char(ch, j), ch);
    cost += cost_eq;
    cost_eq = 0;
  }
  while ((cost+cost_eq > GET_GOLD(ch) + GET_BANK_GOLD(ch)) && ch->carrying) {
    Crash_extract_expensive(ch->carrying);
    cost = 0;
    Crash_calculate_rent(ch->carrying, &cost);
    cost <<= 1;
  }

  if (!ch->carrying) {
    for (j = 0; j < NUM_WEARS && !(GET_EQ(ch,j)); j++)
       ;
    if (j == NUM_WEARS) { /* no eq nor inv */
      fclose(fp);
      Crash_delete_file(GET_NAME(ch));
      return;
    }
  }
  rent.net_cost_per_diem = cost;

  rent.rentcode = RENT_TIMEDOUT;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  for (j = 0; j < NUM_WEARS; j++) {
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), fp, j+1)) {
	fclose(fp);
	return;
      }
      Crash_restore_weight(GET_EQ(ch,j));
      Crash_extract_objs(GET_EQ(ch,j));
    }
  }
  if (!Crash_save(ch->carrying, fp, 0)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
}


void Crash_rentsave(struct char_data * ch, int cost)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;
#if 0
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j))
      obj_to_char(unequip_char(ch, j), ch);
#endif
  Crash_extract_norents_from_equipped(ch);
  Crash_extract_norents(ch->carrying);

  rent.net_cost_per_diem = cost;
  rent.rentcode = RENT_RENTED;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), fp, j+1)) {
	fclose(fp);
	return;
      }
      Crash_restore_weight(GET_EQ(ch,j));
      Crash_extract_objs(GET_EQ(ch,j));
    }
  if (!Crash_save(ch->carrying, fp, 0)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
}


void Crash_cryosave(struct char_data * ch, int cost)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;
#if 0
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j))
      obj_to_char(unequip_char(ch, j), ch);
#endif
  Crash_extract_norents_from_equipped(ch);
  Crash_extract_norents(ch->carrying);

  GET_GOLD(ch) = MAX(0L, (long)GET_GOLD(ch) - cost);

  rent.rentcode = RENT_CRYO;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  rent.net_cost_per_diem = 0;
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), fp, j+1)) {
	fclose(fp);
	return;
      }
      Crash_restore_weight(GET_EQ(ch,j));
      Crash_extract_objs(GET_EQ(ch,j));
    }
  if (!Crash_save(ch->carrying, fp, 0)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
  SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
}


/* ************************************************************************
* Routines used for the receptionist                                      *
************************************************************************* */

void Crash_rent_deadline(struct char_data * ch, struct char_data * recep,
			      long cost)
{
  long rent_deadline;

  if (!cost)
    return;

  rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / cost);
  sprintf(buf,
      "%s You can rent for %ld day%s with the gold you have\r\n"
	  "on hand and in the bank.\r\n", GET_NAME(ch),
	  rent_deadline, (rent_deadline > 1) ? "s" : "");
  do_tell(recep, buf, 0, 0);
}

int Crash_report_unrentables(struct char_data * ch, struct char_data * recep,
				 struct obj_data * obj)
{
  char buf[128];
  int has_norents = 0;

  if (obj) {
    if (Crash_is_unrentable(obj)) {
      has_norents = 1;
      sprintf(buf, "%s You cannot store %s.", GET_NAME(ch), OBJS(obj, ch));
      do_tell(recep, buf, 0, 0);
    }
    has_norents += Crash_report_unrentables(ch, recep, obj->contains);
    has_norents += Crash_report_unrentables(ch, recep, obj->next_content);
  }
  return (has_norents);
}



void Crash_report_rent(struct char_data * ch, struct char_data * recep,
			    struct obj_data * obj, long *cost, long *nitems, int display, int factor)
{
  static char buf[256];

  if (obj) {
    if (!Crash_is_unrentable(obj)) {
      (*nitems)++;
      *cost += MAX(0, (int)(GET_OBJ_RENT(obj) * factor));
      if (display) {
	sprintf(buf, "%s %5d coins for %s..", GET_NAME(ch),
		(GET_OBJ_RENT(obj) * factor), OBJS(obj, ch));
	do_tell(recep, buf, 0, 0);
      }
    }
    Crash_report_rent(ch, recep, obj->contains, cost, nitems, display, factor);
    Crash_report_rent(ch, recep, obj->next_content, cost, nitems, display, factor);
  }
}



int Crash_offer_rent(struct char_data * ch, struct char_data * receptionist,
			 int display, int factor)
{
  extern int max_obj_save;      /* change in config.c */
  char buf[MAX_INPUT_LENGTH];
  int i;
  long totalcost = 0, numitems = 0, norent = 0;

  norent = Crash_report_unrentables(ch, receptionist, ch->carrying);
  for (i = 0; i < NUM_WEARS; i++)
    norent += Crash_report_unrentables(ch, receptionist, GET_EQ(ch, i));

  if (norent)
    return 0;

  totalcost = min_rent_cost * factor;

  Crash_report_rent(ch, receptionist, ch->carrying, &totalcost, &numitems, display, factor);

  for (i = 0; i < NUM_WEARS; i++)
    Crash_report_rent(ch, receptionist, GET_EQ(ch, i), &totalcost, &numitems, display, factor);

  if (!numitems) {
    sprintf(buf, "%s But you are not carrying anything!  Just quit!", GET_NAME(ch));
    do_tell(receptionist, buf, 0, 0);
    return (0);
  }
  if (numitems > max_obj_save) {
    sprintf(buf, "%s Sorry, but I cannot store more than %d items.",
		GET_NAME(ch), max_obj_save);
    do_tell(receptionist, buf, 0, 0);
    return (0);
  }
  if (display) {
    sprintf(buf, "%s Plus, my %d coin fee..", GET_NAME(ch),
	    min_rent_cost * factor);
    do_tell(receptionist, buf, 0, 0);
    sprintf(buf, "%s Totalling %ld, minus your rent credit of %ld...",
	    GET_NAME(ch), totalcost, (long)(GET_LEVEL(ch) * 800));
    do_tell(receptionist, buf, 0, 0);
    totalcost = MAX(0, (int)(totalcost - (GET_LEVEL(ch) * 800)));
    sprintf(buf, "%s That will be %ld coin%s%s.", GET_NAME(ch),
	    totalcost, (totalcost == 1 ? "" : "s"),
	    (factor == RENT_FACTOR ? " per day" : ""));
    do_tell(receptionist, buf, 0, 0);
    if (totalcost > GET_GOLD(ch)) {
      sprintf(buf, "%s ...which I see you can't afford.", GET_NAME(ch));
      do_tell(receptionist, buf, 0, 0);
      return (0);
    } else if (factor == RENT_FACTOR)
      Crash_rent_deadline(ch, receptionist, totalcost);
  }
  return (totalcost);
}



int gen_receptionist(struct char_data * ch, struct char_data * recep,
			 int cmd, char *arg, int mode)
{
  int cost = 0;
  extern int free_rent;
  sh_int save_room;
  char *action_table[] = {"smile", "dance", "sigh", "blush", "burp",
  "cough", "giggle", "twiddle", "yawn"};

  if (!ch->desc || IS_NPC(ch))
    return FALSE;

  if (!cmd && !number(0, 5)) {
    do_action(recep, "", find_command(action_table[number(0, 8)]), 0);
    return FALSE;
  }
  if (!CMD_IS("offer") && !CMD_IS("rent"))
    return FALSE;
  if (!AWAKE(recep)) {
    send_to_char("She is unable to talk to you...\r\n", ch);
    return TRUE;
  }
  if (!CAN_SEE(recep, ch)) {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
    return TRUE;
  }
  if (free_rent) {
    act("$n tells you, 'Rent is free here.  Just quit, and your objects will be saved!'",
	FALSE, recep, 0, ch, TO_VICT);
    return 1;
  }
  if (CMD_IS("rent")) {
    if (mode == RENT_FACTOR)
      sprintf(buf, "%s Rent will cost you %d gold coins per day.",
			GET_NAME(ch), cost);
    else if (mode == CRYO_FACTOR)
      sprintf(buf, "%s It will cost you %d gold coins to be frozen.",
			GET_NAME(ch), cost);
    do_tell(recep, buf, 0, 0);
    if (cost > GET_GOLD(ch)) {
      sprintf(buf, "%s ...which I see you can't afford.", GET_NAME(ch));
      do_tell(recep, buf, 0, 0);
      return TRUE;
    }
    if (mode == RENT_FACTOR)
      Crash_rent_deadline(ch, recep, cost);

    if (mode == RENT_FACTOR) {
      act("$n stores your belongings and helps you into your private chamber.",
	  FALSE, recep, 0, ch, TO_VICT);
      Crash_rentsave(ch, cost);
      sprintf(buf, "%s has rented (%d/day, %ld tot.)", GET_NAME(ch),
	      cost, GET_GOLD(ch) + GET_BANK_GOLD(ch));
    } else {                    /* cryo */
      act("$n stores your belongings and helps you into your private chamber.\r\n"
	  "A white mist appears in the room, chilling you to the bone...\r\n"
	  "You begin to lose consciousness...",
	  FALSE, recep, 0, ch, TO_VICT);
      Crash_cryosave(ch, cost);
      sprintf(buf, "%s has cryo-rented.", GET_NAME(ch));
      SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
    }

    mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(ch)), TRUE);
    act("$n helps $N into $S private chamber.", FALSE, recep, 0, ch, TO_NOTVICT);
    save_room = ch->in_room;
    extract_char(ch);
    save_char(ch, save_room);
  } else {
    Crash_offer_rent(ch, recep, TRUE, mode);
    act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
  }
  return TRUE;
}


SPECIAL(receptionist)
{
  return (gen_receptionist(ch, me, cmd, argument, RENT_FACTOR));
}


SPECIAL(cryogenicist)
{
  return (gen_receptionist(ch, me, cmd, argument, CRYO_FACTOR));
}


void Crash_save_all(void)
{
  struct descriptor_data *d;
  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLAYING) && !IS_NPC(d->character)) {
      if (PLR_FLAGGED(d->character, PLR_CRASH)) {
	Crash_crashsave(d->character);
	save_char(d->character, d->character->in_room);
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_CRASH);
      }
    }
  }
}
