/* ***********************************************************************\
*    _____ _            ____                  _       _                   *
*   |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __        *
*     | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \       *
*     | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |      *
*     |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|      *
*                                                                         *
*  File:  CREATION.C                                                      *
*  Usage: Source file for On-Line-Creation Utilities                      *
*  Programmer(s): Ideas gleaned from the Creators of SillyMUD             *
*                 and the creator of redit and iedit for CircleMUD        *
*                 All code by Sean Mountcastle (Glasgian)                 *
*                 and Harvey Gilpin (Copyright 1996)                      *
\*********************************************************************** */

#define __OLC_C__

#include "conf.h"
#include "sysdep.h"
#include "protos.h"

extern struct zone_data *zone_table;            /*. db.c        .*/
extern struct room_data *world;                 /*. db.c        .*/
extern int    top_of_zone_table;                /*. db.c        .*/
extern struct char_data *mob_proto;             /*. db.c        .*/
extern struct index_data *mob_index;            /*. db.c        .*/
extern struct obj_data *obj_proto;              /*. db.c        .*/
extern struct index_data *obj_index;            /*. db.c        .*/
extern char   *equipment_types[];               /*. constants.c .*/
extern char   *dirs[];                          /*. constants.c .*/
extern long   top_of_world;
extern struct room_data *world;
extern struct obj_data *obj_proto;
extern struct char_data *mob_proto;
extern char   *room_bits[];
extern char   *sector_types[];
extern char   *exit_bits[];
extern struct zone_data *zone_table;
extern sh_int r_mortal_start_room;
extern sh_int r_immort_start_room;
extern sh_int r_frozen_start_room;
extern sh_int mortal_start_room;
extern sh_int immort_start_room;
extern sh_int frozen_start_room;
extern struct index_data *mob_index;                    /*. db.c        .*/
extern struct char_data *mob_proto;                     /*. db.c        .*/
extern struct char_data *character_list;                /*. db.c        .*/
extern long   top_of_mobt;                              /*. db.c        .*/
extern struct zone_data *zone_table;                    /*. db.c        .*/
extern struct player_special_data dummy_mob;            /*. db.c        .*/
extern struct attack_hit_type attack_hit_text[];        /*. fight.c     .*/
extern char   *action_bits[];                           /*. constants.c .*/
extern char   *affected_bits[];                         /*. constants.c .*/
extern char   *position_types[];                        /*. constants.c .*/
extern char   *genders[];                               /*. constants.c .*/
extern char   *pc_race_types[];                         /*. racerel.c   .*/
extern long   top_shop;                                 /*. shop.c      .*/
extern struct shop_data *shop_index;                    /*. shop.c      .*/
extern struct descriptor_data *descriptor_list;         /*. comm.c      .*/
extern struct obj_data *obj_proto;
extern struct index_data *obj_index;
extern struct obj_data *object_list;
extern long   top_of_objt;
extern struct attack_hit_type attack_hit_text[];        /*. fight.c     .*/
extern char   *item_types[];
extern char   *wear_bits[];
extern char   *extra_bits[];
extern char   *drinks[];
extern char   *apply_types[];
extern char   *container_bits[];
extern char   *spells[];
extern struct board_info_type board_info[];
extern struct descriptor_data *descriptor_list; /*. comm.c      .*/
extern struct char_data *mob_proto;             /*. db.c        .*/
extern struct room_data *world;                 /*. db.c        .*/
extern struct zone_data *zone_table;            /*. db.c        .*/
extern struct index_data *mob_index;            /*. db.c        .*/
extern struct index_data *obj_index;            /*. db.c        .*/
extern char   *trade_letters[];                 /*. shop.h      .*/
extern char   *shop_bits[];                     /*. shop.h      .*/
extern char   *item_types[];                    /*. constants.c .*/
extern struct guild_type *guild_info;

/* FUNCTION PROTOS in this file */
void zedit_disp_menu(struct descriptor_data * d);
void zedit_setup(struct descriptor_data *d, int room_num);
void add_cmd_to_list(struct reset_com **list, struct reset_com *newcmd, int pos);
void remove_cmd_from_list(struct reset_com **list, int pos);
void delete_command(struct descriptor_data *d, int pos);
int  new_command(struct descriptor_data *d, int pos);
int  start_change_command(struct descriptor_data *d, int pos);
void zedit_disp_comtype(struct descriptor_data *d);
void zedit_disp_arg1(struct descriptor_data *d);
void zedit_disp_arg2(struct descriptor_data *d);
void zedit_disp_arg3(struct descriptor_data *d);
void zedit_save_internally(struct descriptor_data *d);
void zedit_save_to_disk(struct descriptor_data *d);
void zedit_create_index(int znum, char *type);
void zedit_new_zone(struct char_data *ch, int vzone_num);

void redit_disp_extradesc_menu(struct descriptor_data * d);
void redit_disp_exit_menu(struct descriptor_data * d);
void redit_disp_exit_flag_menu(struct descriptor_data * d);
void redit_disp_flag_menu(struct descriptor_data * d);
void redit_disp_sector_menu(struct descriptor_data * d);
void redit_disp_menu(struct descriptor_data * d);
void redit_disp_tele_menu(struct descriptor_data * d);
void redit_disp_river_menu(struct descriptor_data * d);
void redit_parse(struct descriptor_data * d, char *arg);
void redit_setup_new(struct descriptor_data *d);
void redit_setup_existing(struct descriptor_data *d, long real_num);
void redit_save_to_disk(struct descriptor_data *d);
void redit_save_internally(struct descriptor_data *d);
void redit_free_room(struct room_data *room);

void medit_parse(struct descriptor_data * d, char *arg);
void medit_disp_menu(struct descriptor_data * d);
void medit_setup_new(struct descriptor_data *d);
void medit_setup_existing(struct descriptor_data *d, long rmob_num);
void medit_save_internally(struct descriptor_data *d);
void medit_save_to_disk(struct descriptor_data *d);
void init_mobile(struct char_data *mob);
void copy_mobile(struct char_data *tmob, struct char_data *fmob);
void medit_disp_positions(struct descriptor_data *d);
void medit_disp_mob_flags(struct descriptor_data *d);
void medit_disp_aff_flags(struct descriptor_data *d);
void medit_disp_attack_types(struct descriptor_data *d);

void oedit_disp_container_flags_menu(struct descriptor_data * d);
void oedit_disp_extradesc_menu(struct descriptor_data * d);
void oedit_disp_weapon_menu(struct descriptor_data * d);
void oedit_disp_traptype_menu(struct descriptor_data * d);
void oedit_disp_traptrig_menu(struct descriptor_data * d);
void oedit_disp_val1_menu(struct descriptor_data * d);
void oedit_disp_val2_menu(struct descriptor_data * d);
void oedit_disp_val3_menu(struct descriptor_data * d);
void oedit_disp_val4_menu(struct descriptor_data * d);
void oedit_disp_type_menu(struct descriptor_data * d);
void oedit_disp_extra_menu(struct descriptor_data * d);
void oedit_disp_wear_menu(struct descriptor_data * d);
void oedit_disp_materials(struct descriptor_data * d);
void oedit_disp_sizes(struct descriptor_data * d);
void oedit_disp_menu(struct descriptor_data * d);
void oedit_parse(struct descriptor_data * d, char *arg);
void oedit_disp_spells_menu(struct descriptor_data * d);
void oedit_liquid_type(struct descriptor_data * d);
void oedit_setup_new(struct descriptor_data *d);
void oedit_setup_existing(struct descriptor_data *d, long real_num);
void oedit_save_to_disk(struct descriptor_data *d);
void oedit_save_internally(struct descriptor_data *d);

long real_shop(long vshop_num);
void sedit_setup_new(struct descriptor_data *d);
void sedit_setup_existing(struct descriptor_data *d, long rmob_num);
void sedit_parse(struct descriptor_data * d, char *arg);
void sedit_disp_menu(struct descriptor_data * d);
void sedit_namelist_menu(struct descriptor_data *d);
void sedit_types_menu(struct descriptor_data *d);
void sedit_products_menu(struct descriptor_data *d);
void sedit_rooms_menu(struct descriptor_data *d);
void sedit_compact_rooms_menu(struct descriptor_data *d);
void sedit_shop_flags_menu(struct descriptor_data *d);
void sedit_no_trade_menu(struct descriptor_data *d);
void sedit_save_internally(struct descriptor_data *d);
void sedit_save_to_disk(struct descriptor_data *d);
void copy_shop(struct shop_data *tshop, struct shop_data *fshop);
void copy_list(long **tlist, long *flist);
void copy_type_list(struct shop_buy_data **tlist, struct shop_buy_data *flist);
void sedit_add_to_type_list(struct shop_buy_data **list, struct shop_buy_data *newp);
void sedit_remove_from_type_list(struct shop_buy_data **list, int num);
void free_shop_strings(struct shop_data *shop);
void free_type_list(struct shop_buy_data **list);
void free_shop(struct shop_data *shop);
void sedit_modify_string(char **str, char *newp);

void gedit_disp_menu(struct descriptor_data * d);
void gedit_disp_restricts_menu(struct descriptor_data * d);
void gedit_disp_dir_menu(struct descriptor_data * d);
void gedit_free_guild(struct guild_type *gptr);
void gedit_setup_new(struct descriptor_data *d);
void gedit_parse(struct descriptor_data *d, char *arg);

int  real_zone(long number);
void olc_saveinfo(struct char_data *ch);

/*. External .*/
SPECIAL(shop_keeper);


/*. Internal data .*/
struct olc_scmd_data {
  char *text;
  int  con_type;
};

struct olc_scmd_data olc_scmd_info[5] =
{
  {"room",   CON_REDIT},
  {"object", CON_OEDIT},
  {"room",   CON_ZEDIT},
  {"mobile", CON_MEDIT},
  {"shop",   CON_SEDIT}
};

void sprintbits(long vektor, char *outstring)
{
   int i = 0;
   char flags[53] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

   strcpy(outstring, "");
   for (i = 0; i < 32; ++i) {
     if (vektor & 1) {
       *outstring = flags[i];
       outstring++;
     }
     vektor >>= 1;
   }
   *outstring = 0;
}

/* MACROS to cut down on writing */
#define ZCMD                    zone_table[zone].cmd[cmd_no]
#define MYCMD                   OLC_ZONE(d)->cmd[cmd_no]
#define OLC_CMD(d)              (OLC_ZONE(d)->cmd[OLC_VAL(d)])
#define W_EXIT(room, num)       (world[(room)].dir_option[(num)])
#define GET_NDD(mob)            ((mob)->mob_specials.damnodice)
#define GET_SDD(mob)            ((mob)->mob_specials.damsizedice)
#define GET_ALIAS(mob)          ((mob)->player.name)
#define GET_SDESC(mob)          ((mob)->player.short_descr)
#define GET_LDESC(mob)          ((mob)->player.long_descr)
#define GET_DDESC(mob)          ((mob)->player.description)
#define GET_ATTACK(mob)         ((mob)->mob_specials.attack_type)
#define S_KEEPER(shop)          ((shop)->keeper)
#define S_PRODUCT(s, i)         ((s)->producing[(i)])
#define S_NUM(i)                ((i)->vnumber)
#define S_OPEN1(i)              ((i)->open1)
#define S_CLOSE1(i)             ((i)->close1)
#define S_OPEN2(i)              ((i)->open2)
#define S_CLOSE2(i)             ((i)->close2)
#define S_BANK(i)               ((i)->bankAccount)
#define S_BROKE_TEMPER(i)       ((i)->temper1)
#define S_BITVECTOR(i)          ((i)->bitvector)
#define S_NOTRADE(i)            ((i)->with_who)
#define S_SORT(i)               ((i)->lastsort)
#define S_BUYPROFIT(i)          ((i)->profit_buy)
#define S_SELLPROFIT(i)         ((i)->profit_sell)
#define S_FUNC(i)               ((i)->func)

#define S_ROOMS(i)              ((i)->in_room)
#define S_PRODUCTS(i)           ((i)->producing)
#define S_NAMELISTS(i)          ((i)->type)
#define S_ROOM(i, num)          ((i)->in_room[(num)])
#define S_BUYTYPE(i, num)       (BUY_TYPE((i)->type[(num)]))
#define S_BUYWORD(i, num)       (BUY_WORD((i)->type[(num)]))

#define S_NOITEM1(i)            ((i)->no_such_item1)
#define S_NOITEM2(i)            ((i)->no_such_item2)
#define S_NOCASH1(i)            ((i)->missing_cash1)
#define S_NOCASH2(i)            ((i)->missing_cash2)
#define S_NOBUY(i)              ((i)->do_not_buy)
#define S_BUY(i)                ((i)->message_buy)
#define S_SELL(i)               ((i)->message_sell)


/*
 * This is the main interface into On-Line-Creation
 *  it deals with all of the general olc stuff and then passes
 *  control onto the parsers for each olc
 */
ACMD(do_olc)
{
  int    save = 0;
  long   real_num, number = -1;
  struct descriptor_data *d;
  struct guild_type *gptr = NULL;
  extern struct guild_type *guild_info;

  if (IS_NPC(ch))
    /*. No screwing around .*/
    return;

  if (subcmd == SCMD_OLC_SAVEINFO) {
    olc_saveinfo(ch);
    return;
  }

  /*. Parse any arguments .*/
  two_arguments(argument, buf1, buf2);

  /* No argument given .*/
  if (!*buf1) {
    switch(subcmd) {
      case SCMD_OLC_ZEDIT:
      case SCMD_OLC_REDIT:
	number = world[IN_ROOM(ch)].number;
	break;
      case SCMD_OLC_OEDIT:
      case SCMD_OLC_MEDIT:
      case SCMD_OLC_SEDIT:
	sprintf(buf, "Specify a %s VNUM to edit.\r\n", olc_scmd_info[subcmd].text);
	send_to_char (buf, ch);
	return;
      case SCMD_OLC_GEDIT:
	number = 0;
	break;
    }
  } else if (!isdigit (*buf1)) {
    if (strncmp("save", buf1, 4) == 0) {
      if (!*buf2) {
	send_to_char("Save which zone?\r\n", ch);
	return;
      } else {
	save = 1;
	number = atoi(buf2) * 100;
      }
    } else if (subcmd == SCMD_OLC_ZEDIT && GET_LEVEL(ch) >= LVL_IMPL) {
      if ((strncmp("new", buf1, 3) == 0) && *buf2)
	zedit_new_zone(ch, atoi(buf2));
      else
	send_to_char("Specify a new zone number.\r\n", ch);
      return;
    } else {
      send_to_char ("Yikes!  Stop that, someone will get hurt!\r\n", ch);
      return;
    }
  }

  /*. If a numeric argument was given, get it .*/
  if (number == -1)
    number = atoi(buf1);

  /*. Check whatever it is isn't already being edited .*/
  for (d = descriptor_list; d; d = d->next)
    if (d->connected == olc_scmd_info[subcmd].con_type)
      if (d->olc && OLC_NUM(d) == number) {
	sprintf(buf, "That %s is currently being edited by %s.\r\n",
		olc_scmd_info[subcmd].text, GET_NAME(d->character));
	send_to_char(buf, ch);
	return;
      }

  d = ch->desc;

  /*. Give descriptor an OLC struct .*/
  CREATE(d->olc, struct olc_data, 1);

  /*. Find the zone .*/
  OLC_ZNUM(d) = real_zone(number);

  if (OLC_ZNUM(d) == -1) {
    send_to_char ("Sorry, there is no zone for that number!\r\n", ch);
    free(d->olc);
    return;
  }

  /*. Everyone but IMPLs can only edit zones they have been assigned .*/
  if (((GET_LEVEL(ch) < LVL_IMPL) &&
      (zone_table[OLC_ZNUM(d)].number != GET_OLC_ZONE(ch))) &&
      (GET_OLC_ZONE(ch) != -128)) {
    send_to_char("You do not have permission to edit this zone.\r\n", ch);
    free(d->olc);
    return;
  }

  if (save) {
    switch(subcmd) {
      case SCMD_OLC_REDIT:
	send_to_char("Saving all rooms in zone.\r\n", ch);
	sprintf(buf, "OLC: %s saves rooms for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
	mudlog(buf, CMP, LVL_GOD, TRUE);
	redit_save_to_disk(d);
	break;
      case SCMD_OLC_ZEDIT:
	send_to_char("Saving all zone information.\r\n", ch);
	sprintf(buf, "OLC: %s saves zone info for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
	mudlog(buf, CMP, LVL_GOD, TRUE);
	zedit_save_to_disk(d);
	break;
      case SCMD_OLC_OEDIT:
	send_to_char("Saving all objects in zone.\r\n", ch);
	sprintf(buf, "OLC: %s saves objects for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
	mudlog(buf, CMP, LVL_GOD, TRUE);
	oedit_save_to_disk(d);
	break;
      case SCMD_OLC_MEDIT:
	send_to_char("Saving all mobiles in zone.\r\n", ch);
	sprintf(buf, "OLC: %s saves mobs for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
	mudlog(buf, CMP, LVL_GOD, TRUE);
	medit_save_to_disk(d);
	break;
      case SCMD_OLC_SEDIT:
	send_to_char("Saving all shops in zone.\r\n", ch);
	sprintf(buf, "OLC: %s saves shops for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
	mudlog(buf, CMP, LVL_GOD, TRUE);
	sedit_save_to_disk(d);
	break;
    }
    free(d->olc);
    return;
  }

  OLC_NUM(d) = number;

  /*. Steal players descriptor start up subcommands .*/
  switch (subcmd) {
    case SCMD_OLC_REDIT:
      real_num = real_room(number);
      if (real_num >= 0)
	redit_setup_existing(d, real_num);
      else
	redit_setup_new(d);
      STATE(d) = CON_REDIT;
      break;
    case SCMD_OLC_ZEDIT:
      real_num = real_room(number);
      if (real_num < 0) {
	 send_to_char("That room does not exist.\r\n", ch);
	 free(d->olc);
	 return;
      }
      zedit_setup(d, real_num);
      STATE(d) = CON_ZEDIT;
      break;
    case SCMD_OLC_MEDIT:
      real_num = real_mobile(number);
      if (real_num < 0)
	medit_setup_new(d);
      else
	medit_setup_existing(d, real_num);
      STATE(d) = CON_MEDIT;
      break;
    case SCMD_OLC_OEDIT:
      real_num = real_object(number);
      if (real_num >= 0)
	oedit_setup_existing(d, real_num);
      else
	oedit_setup_new(d);
      STATE(d) = CON_OEDIT;
      break;
    case SCMD_OLC_SEDIT:
      real_num = real_shop(number);
      if (real_num >= 0)
	sedit_setup_existing(d, real_num);
      else
	sedit_setup_new(d);
      STATE(d) = CON_SEDIT;
      break;
    case SCMD_OLC_GEDIT:
      if (number == 0)
	gedit_setup_new(ch->desc);
      else {
	for (gptr = guild_info; gptr && gptr->number != number; gptr=gptr->next)
	  ;
	if (gptr && (gptr->number == number)) {
	  OLC_GUILD(d) = gptr;
	  gedit_disp_menu(d);
	} else {
	  send_to_char("Invalid guild number!\r\n", d->character);
	  return;
	}
      }
      STATE(d) = CON_GEDIT;
      break;
  }
  act("$n starts using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
  SET_BIT(PLR_FLAGS (ch), PLR_WRITING);
}

/* This function shows which portions of the database need to be saved to disk */
void olc_saveinfo(struct char_data *ch)
{
   struct olc_save_info *entry;
   static char *save_info_msg[5] = { "Rooms", "Objects", "Zone info", "Mobiles", "Shops" };

   if (olc_save_list)
     send_to_char("The following OLC components need saving:-\r\n", ch);
   else
     send_to_char("The database is up to date.\r\n", ch);

   for (entry = olc_save_list; entry; entry = entry->next) {
     sprintf(buf, " - %s for zone %d.\r\n",
	     save_info_msg[(int)entry->type], entry->zone);
     send_to_char(buf, ch);
   }
}


/* This just returns the zone number of a room */
int real_zone(long number)
{
  int counter;

  for (counter = 0; counter <= top_of_zone_table; counter++)
    if ((number >= (zone_table[counter].number * 100)) &&
	(number <= (zone_table[counter].top)))
      return counter;

  return -1;
}


/*. Add an entry to the 'to be saved' list .*/
void olc_add_to_save_list(int zone, byte type)
{
  struct olc_save_info *newp;

  /*. Return if it's already in the list .*/
  for (newp = olc_save_list; newp; newp = newp->next)
    if ((newp->zone == zone) && (newp->type == type))
      return;

  CREATE(newp, struct olc_save_info, 1);
  newp->zone = zone;
  newp->type = type;
  newp->next = olc_save_list;
  olc_save_list = newp;
}

/*. Remove an entry from the 'to be saved' list .*/
void olc_remove_from_save_list(int zone, byte type)
{
  struct olc_save_info **entry;
  struct olc_save_info *temp;

  for (entry = &olc_save_list; *entry; entry = &(*entry)->next)
    if (((*entry)->zone == zone) && ((*entry)->type == type)) {
      temp = *entry;
      *entry = temp->next;
      free(temp);
      return;
    }
}

/* Set-up the colors for the OLC system */
void get_char_cols(struct char_data *ch)
{
   nrm = CCNRM(ch, C_NRM);
   grn = CCGRN(ch, C_NRM);
   cyn = CCCYN(ch, C_NRM);
   yel = CCYEL(ch, C_NRM);
}


/*
 * This procedure removes the '\r\n' from a string so that it may be
 *   saved to a file.  Use it only on buffers, not on the oringinal
 *   strings.
 */

void strip_string(char *buffer)
{
  char  *pointer;

  pointer = buffer;
  while ((pointer = strchr(pointer, '\r')))
    /*. Hardly elegant, but it does the job .*/
    strcpy(pointer, pointer+1);
}


/*
 * This procdure frees up the strings and/or the structures
 *   attatched to a descriptor, sets all flags back to how they
 *   should be.
 */
void cleanup_olc(struct descriptor_data *d, byte cleanup_type)
{
  if (d->olc) {
    /*. Check for room .*/
    if (OLC_ROOM(d)) {
      /*. free_room performs no sanity checks, must be careful here .*/
      switch(cleanup_type) {
	case CLEANUP_ALL:
	  redit_free_room(OLC_ROOM(d));
	  break;
	case CLEANUP_STRUCTS:
	  free(OLC_ROOM(d));
	  break;
	default:
	  /*. Caller has screwed up .*/
	  break;
      }
    }

    if (OLC_GUILD(d)) {
      /*. free_room performs no sanity checks, must be careful here .*/
      switch(cleanup_type) {
	case CLEANUP_ALL:
	  gedit_free_guild(OLC_GUILD(d));
	  break;
	case CLEANUP_STRUCTS:
	  break;
	default:
	  /*. Caller has screwed up .*/
	  break;
      }
      OLC_GUILD(d) = NULL;
    }

    /*. Check for object .*/
    if (OLC_OBJ(d)) {
      /*. free_obj checks strings aren't part of proto .*/
      free_obj(OLC_OBJ(d));
    }

    /*. Check for mob .*/
    if (OLC_MOB(d)) {
      /*. free_char checks strings aren't part of proto .*/
      free_char(OLC_MOB(d));
    }

    /*. Check for zone .*/
    if (OLC_ZONE(d)) {
      /*. cleanup_type is irrelivent here, free everything .*/
      free(OLC_ZONE(d)->name);
      free(OLC_ZONE(d)->builder);
      free(OLC_ZONE(d)->levelrec);
      free(OLC_ZONE(d)->cmd);
      free(OLC_ZONE(d));
    }

    /*. Check for shop .*/
    if (OLC_SHOP(d)) {
      /*. free_shop performs no sanity checks, must be carefull here .*/
      switch(cleanup_type) {
	case CLEANUP_ALL:
	  free_shop(OLC_SHOP(d));
	  break;
	case CLEANUP_STRUCTS:
	  free(OLC_SHOP(d));
	  break;
	default:
	  /*. Caller has screwed up .*/
	  break;
      }
    }

    /*. Restore desciptor playing status .*/
    if (d->character) {
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
      STATE(d) = CON_PLAYING;
      act("$n stops using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
    }
    free(d->olc);
  }
}



void zedit_setup(struct  descriptor_data *d, int room_num)
{
  struct zone_data *zonep;
  int    zone, cmd_no = 0, count = 0, cmd_room = -1;

  /*. Alloc some zone shaped space .*/
  CREATE(zonep, struct zone_data, 1);

  zone = OLC_ZNUM(d);

  /*. Copy in zone header info .*/
  zonep->name       = str_dup(zone_table[zone].name);
  zonep->builder    = str_dup(zone_table[zone].builder);
  zonep->levelrec   = str_dup(zone_table[zone].levelrec);
  zonep->pkill      = zone_table[zone].pkill;
  zonep->recall_loc = zone_table[zone].recall_loc;
  zonep->lifespan   = zone_table[zone].lifespan;
  zonep->top        = zone_table[zone].top;
  zonep->reset_mode = zone_table[zone].reset_mode;
  /*. The remaining fields are used as a 'has been modified' flag .*/
  zonep->number = 0;    /*. Header info has changed .*/
  zonep->age = 0;       /*. Commands have changed   .*/

  /*. Start the reset command list with a terminator .*/
  CREATE(zonep->cmd, struct reset_com, 1);
  zonep->cmd[0].command = 'S';

  /*. Add all entried in zone_table that relate to this room .*/
  while(ZCMD.command != 'S') {
    switch(ZCMD.command) {
      case 'M':
      case 'O':
	cmd_room = ZCMD.arg3;
	break;
      case 'D':
      case 'R':
	cmd_room = ZCMD.arg1;
	break;
      default:
	break;
    }
    if (cmd_room == room_num) {
      add_cmd_to_list(&(zonep->cmd), &ZCMD, count);
      count++;
    }
    cmd_no++;
  }
  OLC_ZONE(d) = zonep;
  /*. Display main menu .*/
  zedit_disp_menu(d);
}


/*-------------------------------------------------------------------*/
/*. Create a new zone .*/

void zedit_new_zone(struct char_data *ch, int vzone_num)
{
  FILE   *fp;
  struct zone_data *new_table;
  int    i, room, found = 0;

  if (vzone_num > 326) {
    send_to_char("326 is the highest zone allowed.\r\n", ch);
    return;
  }
  sprintf(buf, "%s/%i.zon", ZON_PREFIX, vzone_num);

  /*. Check zone does not exist .*/
  room = vzone_num * 100;
  for (i = 0; i <= top_of_zone_table; i++)
    if ((zone_table[i].number * 100 <= room) &&
	(zone_table[i].top >= room)) {
      send_to_char("A zone already covers that area.\r\n", ch);
      return;
    }

  /*. Create Zone file .*/
  if (!(fp = fopen(buf, "w+"))) {
    mudlog("SYSERR: OLC: Can't write new zone file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp,
	"#%d\n"
	"New Zone~\n"
	"%d 30 2\n"
	"S\n"
	"$\n",
	vzone_num,
	(vzone_num * 100) + 99
  );
  fclose(fp);

  /*. Create Rooms file .*/
  sprintf(buf, "%s/%d.wld", WLD_PREFIX, vzone_num);
  if (!(fp = fopen(buf, "w+"))) {
    mudlog("SYSERR: OLC: Can't write new world file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp,
	"#%d\n"
	"The Begining~\n"
	"Not much here.\n"
	"~\n"
	"%d 0 0\n"
	"S\n"
	"$\n",
	vzone_num * 100,
	vzone_num
  );
  fclose(fp);

  /*. Create Mobiles file .*/
  sprintf(buf, "%s/%i.mob", MOB_PREFIX, vzone_num);
  if (!(fp = fopen(buf, "w+"))) {
    mudlog("SYSERR: OLC: Can't write new mob file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp, "$\n");
  fclose(fp);

  /*. Create Objects file .*/
  sprintf(buf, "%s/%i.obj", OBJ_PREFIX, vzone_num);
  if (!(fp = fopen(buf, "w+"))) {
    mudlog("SYSERR: OLC: Can't write new obj file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp, "$\n");
  fclose(fp);

  /*. Create Shops file .*/
  sprintf(buf, "%s/%i.shp", SHP_PREFIX, vzone_num);
  if (!(fp = fopen(buf, "w+"))) {
    mudlog("SYSERR: OLC: Can't write new shop file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp, "$~\n");
  fclose(fp);

  /*. Update index files .*/
  zedit_create_index(vzone_num, "zon");
  zedit_create_index(vzone_num, "wld");
  zedit_create_index(vzone_num, "mob");
  zedit_create_index(vzone_num, "obj");
  zedit_create_index(vzone_num, "shp");

  /*. Make a new zone in memory.*/
  CREATE(new_table, struct zone_data, top_of_zone_table);
  /* CHECK THIS OUT THIS COULD BE A BIG PROBLEM!!! */
  new_table[top_of_zone_table + 1].number = 32000;

  for (i = 0; i <= top_of_zone_table + 1; i++) {
    if (!found) {
      if (i > top_of_zone_table || zone_table[i].number > vzone_num) {
	found = 1;
	new_table[i].name       = str_dup("New Zone");
	new_table[i].builder    = str_dup(GET_NAME(ch));
	new_table[i].levelrec   = str_dup("BUILDING");
	new_table[i].pkill      = 0;
	new_table[i].recall_loc = 3039;
	new_table[i].number     = vzone_num;
	new_table[i].top        = (vzone_num * 100) + 99;
	new_table[i].lifespan   = 30;
	new_table[i].age        = 0;
	new_table[i].reset_mode = 2;
	CREATE(new_table[i].cmd, struct reset_com, 1);
	new_table[i].cmd[0].command = 'S';
	if (i <= top_of_zone_table)
	  new_table[i+1] = zone_table[i];
      } else
	new_table[i] = zone_table[i];
    } else
      new_table[i+1] = zone_table[i];
  }
  free(zone_table);
  zone_table = new_table;
  top_of_zone_table++;

  sprintf(buf, "OLC: %s creates new zone #%d", GET_NAME(ch), vzone_num);
  mudlog(buf, BRF, LVL_GOD, TRUE);
  send_to_char("Zone created.\r\n", ch);
  return;
}


void zedit_create_index(int znum, char *type)
{
  FILE *newfile, *oldfile;
  char new_name[32], old_name[32], *prefix;
  int  num, found = FALSE;

  switch (*type) {
    case 'z':
      prefix = ZON_PREFIX;
      break;
    case 'w':
      prefix = WLD_PREFIX;
      break;
    case 'o':
      prefix = OBJ_PREFIX;
      break;
    case 'm':
      prefix = MOB_PREFIX;
      break;
    case 's':
      prefix = SHP_PREFIX;
      break;
    default:
      /*. Caller messed up .*/
      return;
  }

  sprintf(old_name, "%s/index", prefix);
  sprintf(new_name, "%s/newindex", prefix);

  if (!(oldfile = fopen(old_name, "r"))) {
    sprintf(buf, "SYSERR: OLC: Failed to open %s", buf);
    mudlog(buf, BRF, LVL_IMPL, TRUE);
    return;
  }
  if (!(newfile = fopen(new_name, "w+"))) {
    sprintf(buf, "SYSERR: OLC: Failed to open %s", buf);
    mudlog(buf, BRF, LVL_IMPL, TRUE);
    return;
  }

  /*
   * Index contents must be in order: search through the old file for
   *   the right place, insert the new file, then copy the rest over.
   */

  sprintf(buf1, "%d.%s", znum, type);
  while (get_line(oldfile, buf)) {
    if (*buf == '$') {
      if (!found)
	fprintf(newfile, "%s\n", buf1);
      fprintf(newfile, "$\n");
      break;
    }
    if (!found) {
      sscanf(buf, "%d", &num);
      if (num > znum) {
	found = TRUE;
	fprintf(newfile, "%s\n", buf1);
      }
    }
    fprintf(newfile, "%s\n", buf);
  }

  fclose(newfile);
  fclose(oldfile);
  /*. Out with the old, in with the new .*/
  remove(old_name);
  rename(new_name, old_name);
}

/*
 * Save all the information on the players descriptor back into
 *   the zone table.
 */
void zedit_save_internally(struct descriptor_data *d)
{
  int cmd_no = 0, cmd_room = -2, room_num, zone = OLC_ZNUM(d);

  room_num = real_room(OLC_NUM(d));

  /*. Zap all entried in zone_table that relate to this room .*/
  while (ZCMD.command != 'S') {
    switch (ZCMD.command) {
      case 'M':
      case 'O':
	cmd_room = ZCMD.arg3;
	break;
      case 'D':
      case 'R':
	cmd_room = ZCMD.arg1;
	break;
      default:
	break;
    }
    if (cmd_room == room_num)
      remove_cmd_from_list(&(zone_table[OLC_ZNUM(d)].cmd), cmd_no);
    else
      cmd_no++;
  }

  /*. Now add all the entries in the players descriptor list .*/
  cmd_no = 0;
  while (MYCMD.command != 'S') {
    add_cmd_to_list(&(zone_table[OLC_ZNUM(d)].cmd), &MYCMD, cmd_no);
    cmd_no++;
  }

  /*. Finally, if zone headers have been changed, copy over .*/
  if (OLC_ZONE(d)->number) {
    free(zone_table[OLC_ZNUM(d)].name);
    free(zone_table[OLC_ZNUM(d)].builder);
    free(zone_table[OLC_ZNUM(d)].levelrec);
    zone_table[OLC_ZNUM(d)].name        = str_dup(OLC_ZONE(d)->name);
    zone_table[OLC_ZNUM(d)].builder     = str_dup(OLC_ZONE(d)->builder);
    zone_table[OLC_ZNUM(d)].levelrec    = str_dup(OLC_ZONE(d)->levelrec);
    zone_table[OLC_ZNUM(d)].pkill       = OLC_ZONE(d)->pkill;
    zone_table[OLC_ZNUM(d)].recall_loc  = OLC_ZONE(d)->recall_loc;
    zone_table[OLC_ZNUM(d)].top         = OLC_ZONE(d)->top;
    zone_table[OLC_ZNUM(d)].reset_mode  = OLC_ZONE(d)->reset_mode;
    zone_table[OLC_ZNUM(d)].lifespan    = OLC_ZONE(d)->lifespan;
  }
  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ZONE);
}


/*
 * Save all the zone_table for this zone to disk.  Yes, this automatically
 *   comments what it saves to disk -- pretty cool eh?
 */

void zedit_save_to_disk(struct descriptor_data *d)
{
  int  cmd_no = 0, zone = OLC_ZNUM(d);
  long arg1 = -1, arg2 = -1, arg3 = -1;
  char fname[64], backname[72];
  FILE *zfile;
  char *time_and_date;
  time_t ct;

  /* Used to record the time of the last update */
  ct = time(0);
  time_and_date = asctime(localtime(&ct));

  sprintf(fname, "%s/%d.zon", ZON_PREFIX,
	   zone_table[OLC_ZNUM(d)].number);
  sprintf(backname, "%s/%d.zon.back", ZON_PREFIX, zone_table[OLC_ZNUM(d)].number);
  if (rename(fname, backname) == 0) {
     sprintf(buf, "Backup Copy saved of %d.zon.", zone_table[OLC_ZNUM(d)].number);
     mudlog(buf, CMP, LVL_BUILDER, TRUE);
  } else {
     if (!(zfile = fopen(fname, "a+"))) {
       sprintf(buf, "ERROR: Could not backup %d.zon, new zon not written to disk!", zone_table[OLC_ZNUM(d)].number);
       mudlog(buf, CMP, LVL_BUILDER, TRUE);
       return;
     }
  }
  if (!(zfile = fopen(fname, "w"))) {
    sprintf(buf, "SYSERR: OLC: zedit_save_to_disk:  Can't write zone %d.",
	    zone_table[OLC_ZNUM(d)].number);
    mudlog(buf, BRF, LVL_GOD, TRUE);
    return;
  }

  /*. Print zone header to file .*/
  sprintf(buf,
	"#%d\n"
	"%s~\n"
	"%s~\n"
	"%s~\n"
	"%ld %d\n"
	"%ld %d %d\n"
	"*\n* Last Updated: %-20.20s\n*           By: %s\n*\n",
	zone_table[OLC_ZNUM(d)].number,
	zone_table[OLC_ZNUM(d)].name,
	zone_table[OLC_ZNUM(d)].builder,
	zone_table[OLC_ZNUM(d)].levelrec,
	zone_table[OLC_ZNUM(d)].recall_loc,
	zone_table[OLC_ZNUM(d)].pkill,
	zone_table[OLC_ZNUM(d)].top,
	zone_table[OLC_ZNUM(d)].lifespan,
	zone_table[OLC_ZNUM(d)].reset_mode,
	time_and_date, GET_NAME(d->character)
  );
  fprintf(zfile, buf);

  for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
    switch (ZCMD.command) {
      case 'M':
	arg1 = mob_index[ZCMD.arg1].vnumber;
	arg2 = ZCMD.arg2;
	arg3 = world[ZCMD.arg3].number;
	sprintf(buf2, "\t* %s at %s. (Max: %ld)",
	    mob_proto[ZCMD.arg1].player.short_descr,
	    world[ZCMD.arg3].name, ZCMD.arg2);
	break;
      case 'C':
	arg1 = mob_index[ZCMD.arg1].vnumber;
	arg2 = ZCMD.arg2;
	arg3 = -1;
	sprintf(buf2, "\t\t\t* And charm %s to follow it.",
	    mob_proto[ZCMD.arg1].player.short_descr);
	break;
      case 'O':
	arg1 = obj_index[ZCMD.arg1].vnumber;
	arg2 = ZCMD.arg2;
	arg3 = world[ZCMD.arg3].number;
	sprintf(buf2, "\t* %s at %s. (Max: %ld)",
	    obj_proto[ZCMD.arg1].short_description,
	    world[ZCMD.arg3].name, ZCMD.arg2);
	break;
      case 'G':
	arg1 = obj_index[ZCMD.arg1].vnumber;
	arg2 = ZCMD.arg2;
	arg3 = -1;
	sprintf(buf2, "\t\t\t* And give it %s. (Max: %ld)",
	    obj_proto[ZCMD.arg1].short_description,
	    ZCMD.arg2);
	break;
      case 'E':
	arg1 = obj_index[ZCMD.arg1].vnumber;
	arg2 = ZCMD.arg2;
	arg3 = ZCMD.arg3;
	sprintf(buf2, "\t\t\t* And equip it with %s. (Max: %ld)",
	    obj_proto[ZCMD.arg1].short_description,
	    ZCMD.arg2);
	break;
      case 'P':
	arg1 = obj_index[ZCMD.arg1].vnumber;
	arg2 = ZCMD.arg2;
	arg3 = obj_index[ZCMD.arg3].vnumber;
	sprintf(buf2, "\t* Put %s in %s. (Max: %ld)",
	    obj_proto[ZCMD.arg1].short_description,
	    obj_proto[ZCMD.arg3].short_description, ZCMD.arg2);
	break;
      case 'D':
	arg1 = world[ZCMD.arg1].number;
	arg2 = ZCMD.arg2;
	arg3 = ZCMD.arg3;
	sprintf(buf2, "\t\t* %s %s door at %s",
	    (ZCMD.arg3 ? (ZCMD.arg3 == 2 ? "Lock and Close" : "Close") : "Open"),
	    (ZCMD.arg2 ? (ZCMD.arg3 > 1 ? (ZCMD.arg3 > 2 ? (ZCMD.arg3 > 3 ?
	     (ZCMD.arg3 > 4 ? (ZCMD.arg3 > 5 ? (ZCMD.arg3 > 6 ? (ZCMD.arg3 > 7 ?
	      (ZCMD.arg3 > 8 ? "Southwestern" : "Southeastern") : "Northwestern") :
	       "Northeastern") : "Lower") : "Upper") : "Western") : "Southern") :
		"Eastern") : "Northern"),
	    world[ZCMD.arg1].name);
	break;
      case 'R':
	arg1 = world[ZCMD.arg1].number;
	arg2 = obj_index[ZCMD.arg2].vnumber;
	arg3 = -1;
	sprintf(buf2, "\t\t* Remove %s from %s.",
	    obj_proto[ZCMD.arg2].short_description,
	    world[ZCMD.arg1].name);
	break;
      case '*':
	/*. Invalid commands are replaced with '*' - Ignore them .*/
	continue;
      default:
	sprintf(buf, "SYSERR: OLC: z_save_to_disk(): Unknown cmd '%c' - NOT saving", ZCMD.command);
	mudlog(buf, BRF, LVL_GOD, TRUE);
	continue;
    }
    sprintf(buf, "%ld", arg3);
    fprintf(zfile, "%c %d %ld %ld %s %d %s\n",
	ZCMD.command, ZCMD.if_flag, arg1, arg2,
	(arg3 == -1 ? "" : buf),
	(((ZCMD.chance < 101) && (ZCMD.chance > 0)) ? ZCMD.chance : 100),
	buf2);
  }
  fprintf(zfile, "S\n$~\n");
  fclose(zfile);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ZONE);
}

/*
 * Adds a new reset command into a list.  Takes a pointer to the list
 *   so that it may play with the memory locations.
 */

void add_cmd_to_list(struct reset_com **list, struct reset_com *newcmd, int pos)
{
   int    count = 0, i, l;
   struct reset_com *newlist;

   /*. Count number of commands (not including terminator) .*/
   while ((*list)[count].command != 'S')
     count++;

   CREATE(newlist, struct reset_com, count + 2);

   /*. Tight loop to copy old list and insert new command .*/
   l = 0;
   for (i = 0; i <= count; i++) {
     if (i == pos)
       newlist[i] = *newcmd;
     else
       newlist[i] = (*list)[l++];
   }
   /*. Add terminator then insert new list .*/
   newlist[count+1].command = 'S';
   free(*list);
   *list = newlist;
}

/*
 * Remove a reset command from a list.  Takes a pointer to the list
 *   so that it may play with the memory locations.
 */
void remove_cmd_from_list(struct reset_com **list, int pos)
{
   int count = 0, i, l;
   struct reset_com *newlist;

   /*. Count number of commands (not including terminator) .*/
   while ((*list)[count].command != 'S')
     count++;

   CREATE(newlist, struct reset_com, count);

   /*. Tight loop to copy old list and skip unwanted command .*/
   l = 0;
   for (i = 0; i < count; i++) {
     if (i == pos)
       continue;
     else
       newlist[l++] = (*list)[i];
   }
   /*. Add terminator then insert new list .*/
   newlist[count-1].command = 'S';
   free(*list);
   *list = newlist;
}

/*
 * Error check user input and then add new (blank) command.
 */
int new_command(struct descriptor_data *d, int pos)
{
   int    cmd_no = 0;
   struct reset_com *new_com;

   /*. Error check to ensure users hasn't given too large an index .*/
   while (MYCMD.command != 'S')
     cmd_no++;

   if ((pos > cmd_no) || (pos < 0))
     return 0;

   /*. Ok, let's add a new (blank) command.*/
   CREATE(new_com, struct reset_com, 1);
   new_com->command = 'N';
   add_cmd_to_list(&OLC_ZONE(d)->cmd, new_com, pos);
   return 1;
}


/*
 * Error check user input and then remove command.
 */
void delete_command(struct descriptor_data *d, int pos)
{
   int cmd_no = 0;

   /*. Error check to ensure users hasn't given too large an index .*/
   while (MYCMD.command != 'S')
     cmd_no++;

   if ((pos >= cmd_no) || (pos < 0))
     return;

   /*. Ok, let's zap it .*/
   remove_cmd_from_list(&OLC_ZONE(d)->cmd, pos);
}

/*
 * Error check user input and then setup change.
 */
int start_change_command(struct descriptor_data *d, int pos)
{
   int cmd_no = 0;

   /*. Error check to ensure users hasn't given too large an index .*/
   while (MYCMD.command != 'S')
     cmd_no++;

   if ((pos >= cmd_no) || (pos < 0))
     return 0;

   /*. Ok, let's get editing .*/
   OLC_VAL(d) = pos;
   return 1;
}

/**************************************************************************
 Menu functions
 **************************************************************************/

/* the main menu */
void zedit_disp_menu(struct descriptor_data * d)
{
   int cmd_no = 0;
   long room, counter = 0;

   get_char_cols(d->character);
   room = real_room(OLC_NUM(d));

   /*. Menu header .*/
   sprintf(buf,
	"[H[J"
	"Room number: %s%ld%s		Room zone: %s%d\r\n"
	"%sZ%s) Zone name   : %s%s\r\n"
	"%sB%s) Zone Builder: %s%s\r\n"
	"%sC%s) Zone Lvl Rec: %s%s\r\n"
	"%sP%s) Pkill       : %s%s\r\n"
	"%sO%s) Recall Room : %s%ld\r\n"
	"%sL%s) Lifespan    : %s%d minutes\r\n"
	"%sT%s) Top of zone : %s%ld\r\n"
	"%sR%s) Reset Mode  : %s%s%s\r\n"
	"[Command list]\r\n",

	cyn, OLC_NUM(d), nrm,
	cyn, zone_table[OLC_ZNUM(d)].number,
	grn, nrm, yel, OLC_ZONE(d)->name,
	grn, nrm, yel, OLC_ZONE(d)->builder,
	grn, nrm, yel, OLC_ZONE(d)->levelrec,
	grn, nrm, yel, (OLC_ZONE(d)->pkill == 1 ? "Yes" : "No"),
	grn, nrm, yel, (OLC_ZONE(d)->recall_loc),
	grn, nrm, yel, OLC_ZONE(d)->lifespan,
	grn, nrm, yel, OLC_ZONE(d)->top,
	grn, nrm, yel, OLC_ZONE(d)->reset_mode ?
	  ((OLC_ZONE(d)->reset_mode == 1) ?
	  "Reset when no players are in zone." :
	  "Normal reset.") :
	  "Never reset", nrm);

  /*. Print the commands for this room into display buffer .*/
  while (MYCMD.command != 'S') {
    /*. Translate what the command means .*/
    switch (MYCMD.command) {
      case'M':
	sprintf(buf2, "%sLoad %s [%s%ld%s], Max : %ld",
		MYCMD.if_flag ? " then " : "",
		mob_proto[MYCMD.arg1].player.short_descr,
		cyn, mob_index[MYCMD.arg1].vnumber, yel,
		MYCMD.arg2
	);
	break;
      case'G':
	sprintf(buf2, "%sGive it %s [%s%ld%s], Max : %ld",
		MYCMD.if_flag ? " then " : "",
		obj_proto[MYCMD.arg1].short_description,
		cyn, obj_index[MYCMD.arg1].vnumber, yel,
		MYCMD.arg2
	);
	break;
      case'O':
	sprintf(buf2, "%sLoad %s [%s%ld%s], Max : %ld",
		MYCMD.if_flag ? " then " : "",
		obj_proto[MYCMD.arg1].short_description,
		cyn, obj_index[MYCMD.arg1].vnumber, yel,
		MYCMD.arg2
	);
	break;
      case'E':
	sprintf(buf2, "%sEquip with %s [%s%ld%s], %s, Max : %ld",
		MYCMD.if_flag ? " then " : "",
		obj_proto[MYCMD.arg1].short_description,
		cyn, obj_index[MYCMD.arg1].vnumber, yel,
		equipment_types[MYCMD.arg3],
		MYCMD.arg2
	);
	break;
      case'P':
	sprintf(buf2, "%sPut %s [%s%ld%s] in %s [%s%ld%s], Max : %ld",
		MYCMD.if_flag ? " then " : "",
		obj_proto[MYCMD.arg1].short_description,
		cyn, obj_index[MYCMD.arg1].vnumber, yel,
		obj_proto[MYCMD.arg3].short_description,
		cyn, obj_index[MYCMD.arg3].vnumber, yel,
		MYCMD.arg2
	);
	break;
      case'R':
	sprintf(buf2, "%sRemove %s [%s%ld%s] from room.",
		MYCMD.if_flag ? " then " : "",
		obj_proto[MYCMD.arg2].short_description,
		cyn, obj_index[MYCMD.arg2].vnumber, yel
	);
	break;
      case'D':
	sprintf(buf2, "%sSet door %s as %s.",
		MYCMD.if_flag ? " then " : "",
		dirs[MYCMD.arg2],
		MYCMD.arg3 ? ((MYCMD.arg3 == 1) ?
		  "closed" : "locked") : "open"
	);
	break;
      default:
	strcpy(buf2, "<Unknown Command>");
	break;
    }
    /*. Build the display buffer for this command .*/
    sprintf(buf1, "%s%ld - %s%s\r\n",
	nrm, counter++, yel,
	buf2
    );
    strcat(buf, buf1);
    cmd_no++;
  }
  /*. Finish off menu .*/
  sprintf(buf1,
		"%s%ld - <END OF LIST>\r\n"
		"%sN%s) New command.\r\n"
		"%sE%s) Edit a command.\r\n"
		"%sD%s) Delete a command.\r\n"
		"%sQ%s) Quit\r\nEnter your choice : ",
		nrm, counter,
		grn, nrm, grn, nrm, grn, nrm, grn, nrm
  );

  strcat(buf, buf1);
  send_to_char(buf, d->character);

  OLC_MODE(d) = ZEDIT_MAIN_MENU;
}


/*
 * Print the command type menu and setup response catch.
 */
void zedit_disp_comtype(struct descriptor_data *d)
{
   get_char_cols(d->character);
   sprintf(buf, "[H[J"
    "%sM%s) Load Mobile to room             %sO%s) Load Object to room\r\n"
    "%sE%s) Equip mobile with object        %sG%s) Give an object to a mobile\r\n"
    "%sP%s) Put object in another object    %sD%s) Open/Close/Lock a Door\r\n"
    "%sR%s) Remove an object from the room\r\n"
    "What sort of command will this be?  ",
    grn, nrm, grn, nrm, grn, nrm, grn, nrm, grn, nrm,
    grn, nrm, grn, nrm
  );
  send_to_char(buf, d->character);
  OLC_MODE(d) = ZEDIT_COMMAND_TYPE;
}


/*
 * Print the appropriate message for the command type for arg1 and set
 *   up the input catch clause.
 */
void zedit_disp_arg1(struct descriptor_data *d)
{
  switch(OLC_CMD(d).command) {
    case 'M':
      send_to_char("Input mob's vnum : ", d->character);
      OLC_MODE(d) = ZEDIT_ARG1;
      break;
    case 'O':
    case 'E':
    case 'P':
    case 'G':
      send_to_char("Input object vnum : ", d->character);
      OLC_MODE(d) = ZEDIT_ARG1;
      break;
    case 'D':
    case 'R':
      /*. Arg1 for these is the room number, skip to arg2 .*/
      OLC_CMD(d).arg1 = real_room(OLC_NUM(d));
      zedit_disp_arg2(d);
      break;
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog("SYSERR: OLC: zedit_disp_arg1(): Help!", BRF, LVL_GOD, TRUE);
      return;
  }
}



/*
 * Print the appropriate message for the command type for arg2 and set
 *   up the input catch clause.
 */
void zedit_disp_arg2(struct descriptor_data *d)
{
   int i = 0;

   switch (OLC_CMD(d).command) {
     case 'M':
     case 'O':
     case 'E':
     case 'P':
     case 'G':
       send_to_char("Input the maximum number that can exist in this room : ", d->character);
       break;
     case 'D':
       while (*dirs[i] != '\n') {
	 sprintf(buf, "%d) Exit %s.\r\n", i, dirs[i]);
	 send_to_char(buf, d->character);
	 i++;
       }
       send_to_char("Enter exit number for door : ", d->character);
       break;
     case 'R':
       send_to_char("Input object's vnum : ", d->character);
       break;
     default:
       /*. We should never get here .*/
       cleanup_olc(d, CLEANUP_ALL);
       mudlog("SYSERR: OLC: zedit_disp_arg2(): Help!", BRF, LVL_GOD, TRUE);
       return;
   }
   OLC_MODE(d) = ZEDIT_ARG2;
}


/*
 * Print the appropriate message for the command type for arg3 and set
 *   up the input catch clause.
 */
void zedit_disp_arg3(struct descriptor_data *d)
{
   int i = 0;

   switch (OLC_CMD(d).command) {
     case 'E':
       while(*equipment_types[i] !=  '\n') {
	 sprintf(buf, "%2d) %26.26s %2d) %26.26s\r\n", i,
	 equipment_types[i], i+1, (*equipment_types[i+1] != '\n') ?
	 equipment_types[i+1] : "");
	 send_to_char(buf, d->character);
	 if (*equipment_types[i+1] != '\n')
	   i += 2;
	 else
	   break;
       }
       send_to_char("Input location to equip : ", d->character);
       break;
     case 'P':
       send_to_char("Input the vnum of the containter : ", d->character);
       break;
     case 'D':
       send_to_char("0)  Door open\r\n"
		    "1)  Door closed\r\n"
		    "2)  Door locked\r\n"
		    "Enter state of the door : ", d->character);
       break;
     case 'M':
     case 'O':
     case 'R':
     case 'G':
       send_to_char("Input the percent chance of loading : ", d->character);
       break;
     default:
       /*. We should never get here .*/
       cleanup_olc(d, CLEANUP_ALL);
       mudlog("SYSERR: OLC: zedit_disp_arg3(): Help!", BRF, LVL_GOD, TRUE);
       return;
   }
   OLC_MODE(d) = ZEDIT_ARG3;
}

void zedit_disp_arg4(struct descriptor_data *d)
{
   switch (OLC_CMD(d).command) {
     case 'E':
     case 'P':
     case 'D':
       send_to_char("Input the percent chance of loading : ", d->character);
       break;
     default:
       /*. We should never get here .*/
       cleanup_olc(d, CLEANUP_ALL);
       mudlog("SYSERR: OLC: zedit_disp_arg4(): Help!", BRF, LVL_GOD, TRUE);
       return;
   }
   OLC_MODE(d) = ZEDIT_ARG4;
}

/**************************************************************************
 *                   The GARGANTAUN event handler                         *
 **************************************************************************/

void zedit_parse(struct descriptor_data * d, char *arg)
{
   int  i = 0;
   long pos;

   switch (OLC_MODE(d)) {
     case ZEDIT_CONFIRM_SAVESTRING:
       switch (*arg) {
	 case 'y':
	 case 'Y':
	   /*. Save the zone in memory .*/
	   send_to_char("Saving zone info in memory.\r\n", d->character);
	   zedit_save_internally(d);
	   sprintf(buf, "OLC: %s edits zone info for room %ld",
		   GET_NAME(d->character), OLC_NUM(d));
	   mudlog(buf, CMP, LVL_GOD, TRUE);
	   cleanup_olc(d, CLEANUP_ALL);
	   break;
	 case 'n':
	 case 'N':
	   cleanup_olc(d, CLEANUP_ALL);
	   break;
	 default:
	   send_to_char("Invalid choice!\r\n", d->character);
	   send_to_char("Do you wish to save the zone info? : ", d->character);
	   break;
       }
       break; /* end of ZEDIT_CONFIRM_SAVESTRING */

     case ZEDIT_MAIN_MENU:
       switch (*arg) {
	 case 'q':
	 case 'Q':
	   if (OLC_ZONE(d)->age || OLC_ZONE(d)->number) {
	     send_to_char("Do you wish to save the changes to the zone info? (y/n) : ", d->character);
	     OLC_MODE(d) = ZEDIT_CONFIRM_SAVESTRING;
	   } else {
	     send_to_char("No changes made.\r\n", d->character);
	     cleanup_olc(d, CLEANUP_ALL);
	   }
	   break;
	 case 'n':
	 case 'N':
	   /*. New entry .*/
	   send_to_char("What number in the list should the new command be? : ", d->character);
	   OLC_MODE(d) = ZEDIT_NEW_ENTRY;
	   break;
	 case 'e':
	 case 'E':
	   /*. Change an entry .*/
	   send_to_char("Which command do you wish to change? : ", d->character);
	   OLC_MODE(d) = ZEDIT_CHANGE_ENTRY;
	   break;
	 case 'd':
	 case 'D':
	   /*. Delete an entry .*/
	   send_to_char("Which command do you wish to delete? : ", d->character);
	   OLC_MODE(d) = ZEDIT_DELETE_ENTRY;
	   break;
	 case 'z':
	 case 'Z':
	   /*. Edit zone name .*/
	   send_to_char("Enter new zone name : ", d->character);
	   OLC_MODE(d) = ZEDIT_ZONE_NAME;
	   break;
	 case 'b':
	 case 'B':
	   /*. Edit zone builder .*/
	   send_to_char("Enter new zone builder's name : ", d->character);
	   OLC_MODE(d) = ZEDIT_ZONE_BUILDER;
	   break;
	 case 'c':
	 case 'C':
	   /*. Edit zone level recommend .*/
	   send_to_char("Enter new level recommendations : ", d->character);
	   OLC_MODE(d) = ZEDIT_ZONE_LEVELREC;
	   break;
	 case 'p':
	 case 'P':
	   /*. Edit zone pkill.*/
	   send_to_char("Can people in this zone killl players: ", d->character);
	   OLC_MODE(d) = ZEDIT_ZONE_PKILL;
	   break;
	 case 'o':
	 case 'O':
	   /*. Edit zone recall .*/
	   send_to_char("Enter zone recall room : ", d->character);
	   OLC_MODE(d) = ZEDIT_ZONE_RECALL;
	   break;
	 case 't':
	 case 'T':
	   /*. Edit zone top .*/
	   if (GET_LEVEL(d->character) < LVL_IMPL)
	     zedit_disp_menu(d);
	   else {
	     send_to_char("Enter new top of zone : ", d->character);
	     OLC_MODE(d) = ZEDIT_ZONE_TOP;
	   }
	   break;
	 case 'l':
	 case 'L':
	   /*. Edit zone lifespan .*/
	   send_to_char("Enter new zone lifespan : ", d->character);
	   OLC_MODE(d) = ZEDIT_ZONE_LIFE;
	   break;
	 case 'r':
	 case 'R':
	   /*. Edit zone reset mode .*/
	   send_to_char("\r\n"
			"0) Never reset\r\n"
			"1) Reset only when no players in zone\r\n"
			"2) Normal reset\r\n"
			"Enter new zone reset type : ", d->character);
	   OLC_MODE(d) = ZEDIT_ZONE_RESET;
	   break;
	 default:
	   zedit_disp_menu(d);
	   break;
       }
       break; /*. End ZEDIT_MAIN_MENU .*/

     case ZEDIT_NEW_ENTRY:
       /*. Get the line number and insert the new line .*/
       pos = atoi(arg);
       if (isdigit(*arg) && new_command(d, pos)) {
	 if (start_change_command(d, pos)) {
	   zedit_disp_comtype(d);
	   OLC_ZONE(d)->age = 1;
	 }
       } else
	 zedit_disp_menu(d);
       break;

     case ZEDIT_DELETE_ENTRY:
       /*. Get the line number and delete the line .*/
       pos = atoi(arg);
       if (isdigit(*arg)) {
	 delete_command(d, pos);
	 OLC_ZONE(d)->age = 1;
       }
       zedit_disp_menu(d);
       break;

     case ZEDIT_CHANGE_ENTRY:
       /*. Parse the input for which line to edit, and goto next quiz .*/
       pos = atoi(arg);
       if (isdigit(*arg) && start_change_command(d, pos)) {
	 zedit_disp_comtype(d);
	 OLC_ZONE(d)->age = 1;
       } else
	 zedit_disp_menu(d);
       break;

     case ZEDIT_COMMAND_TYPE:
       /*
	* Parse the input for which type of command this is,
	* and goto next quiz.
	*/
       OLC_CMD(d).command = toupper(*arg);
       if (strchr("MOPEDGR", OLC_CMD(d).command) == NULL) {
	 send_to_char("Invalid choice, try again : ", d->character);
       } else {
	 if (OLC_VAL(d)) {
	   /*. If there was a previous command .*/
	   send_to_char("Is this command dependent on the success of the previous one? (y/n)\r\n", d->character);
	   OLC_MODE(d) = ZEDIT_IF_FLAG;
	 } else {
	   /*. 'if-flag' not appropriate .*/
	   OLC_CMD(d).if_flag = FALSE;
	   zedit_disp_arg1(d);
	 }
       }
       break;

     case ZEDIT_IF_FLAG:
       /*. Parse the input for the if flag, and goto next quiz .*/
       switch (*arg) {
	 case 'y':
	 case 'Y':
	   OLC_CMD(d).if_flag = TRUE;
	   break;
	 case 'n':
	 case 'N':
	   OLC_CMD(d).if_flag = FALSE;
	   break;
	 default:
	   send_to_char("Try again : ", d->character);
	   return;
       }
       zedit_disp_arg1(d);
       break;

     case ZEDIT_ARG1:
       /*. Parse the input for arg1, and goto next quiz .*/
       if (!isdigit(*arg)) {
	 send_to_char("Must be a numeric value, try again : ", d->character);
	 return;
       }
       switch (OLC_CMD(d).command) {
	 case 'M':
	   pos = real_mobile(atol(arg));
	   if (pos >= 0) {
	     OLC_CMD(d).arg1 = pos;
	     zedit_disp_arg2(d);
	   } else
	     send_to_char("That mobile does not exist, try again : ", d->character);
	   break;
	 case 'O':
	 case 'P':
	 case 'E':
	 case 'G':
	   pos = real_object(atol(arg));
	   if (pos >= 0) {
	     OLC_CMD(d).arg1 = pos;
	     zedit_disp_arg2(d);
	   } else
	     send_to_char("That object does not exist, try again : ", d->character);
	   break;
	 case 'D':
	 case 'R':
	 default:
	   /*. We should never get here .*/
	   cleanup_olc(d, CLEANUP_ALL);
	   mudlog("SYSERR: OLC: zedit_parse(): case ARG1: Ack!", BRF, LVL_GOD, TRUE);
	   break;
       }
       break;

     case ZEDIT_ARG2:
       /*. Parse the input for arg2, and goto next quiz .*/
       if (!isdigit(*arg)) {
	 send_to_char("Must be a numeric value, try again : ", d->character);
	 return;
       }
       switch (OLC_CMD(d).command) {
	 case 'M':
	 case 'O':
	   OLC_CMD(d).arg2 = atol(arg);
	   OLC_CMD(d).arg3 = real_room(OLC_NUM(d));
	   zedit_disp_arg3(d);
	   break;
	 case 'G':
	   OLC_CMD(d).arg2 = atol(arg);
	   zedit_disp_arg3(d);
	   break;
	 case 'P':
	 case 'E':
	   OLC_CMD(d).arg2 = atol(arg);
	   zedit_disp_arg3(d);
	   break;
	 case 'D':
	   pos = atoi(arg);
	   /*. Count dirs .*/
	   while (*dirs[i] != '\n')
	     i++;
	   if ((pos < 0) || (pos > i))
	     send_to_char("Try again : ", d->character);
	   else {
	     OLC_CMD(d).arg2 = pos;
	     zedit_disp_arg3(d);
	   }
	   break;
	 case 'R':
	   pos = real_object(atol(arg));
	   if (pos >= 0) {
	     OLC_CMD(d).arg2 = pos;
	     zedit_disp_arg3(d);
	   } else
	     send_to_char("That object does not exist, try again : ", d->character);
	   break;
	 default:
	   /*. We should never get here .*/
	   cleanup_olc(d, CLEANUP_ALL);
	   mudlog("SYSERR: OLC: zedit_parse(): case ARG2: Ack!", BRF, LVL_GOD, TRUE);
	   break;
       }
       break;

     case ZEDIT_ARG3:
       /*. Parse the input for arg3, and go back to main menu.*/
       if (!isdigit(*arg)) {
	 send_to_char("Must be a numeric value, try again : ", d->character);
	 return;
       }
       switch (OLC_CMD(d).command) {
	 case 'E':
	   pos = atoi(arg);
	   /*
	    * Count number of wear positions (could use NUM_WEARS,
	    *  this is more reliable).
	    */
	   while (*equipment_types[i] != '\n')
	     i++;
	   if ((pos < 0) || (pos > i))
	     send_to_char("Try again : ", d->character);
	   else {
	     OLC_CMD(d).arg3 = pos;
	     zedit_disp_arg4(d);
	   }
	   break;
	 case 'P':
	   pos = real_object(atol(arg));
	   if (pos >= 0) {
	     OLC_CMD(d).arg3 = pos;
	     zedit_disp_arg4(d);
	   } else
	     send_to_char("That object does not exist, try again : ", d->character);
	   break;
	 case 'D':
	   pos = atoi(arg);
	   if ((pos < 0) || (pos > 2))
	     send_to_char("Try again : ", d->character);
	   else {
	     OLC_CMD(d).arg3 = pos;
	     zedit_disp_arg4(d);
	   }
	   break;
	 case 'M':
	 case 'O':
	 case 'G':
	 case 'R':
	   pos = atoi(arg);
	   if ((pos < 0) || (pos > 100))
	     send_to_char("A percentage is from 0 to 100, try again : ", d->character);
	   else {
	     OLC_CMD(d).chance = pos;
	     zedit_disp_menu(d);
	   }
	   break;
	 default:
	   /*. We should never get here .*/
	   cleanup_olc(d, CLEANUP_ALL);
	   mudlog("SYSERR: OLC: zedit_parse(): case ARG3: Ack!", BRF, LVL_GOD, TRUE);
	   break;
       }
       break;

     case ZEDIT_ARG4:
       if (!isdigit(*arg)) {
	 send_to_char("Must be a numeric value, try again : ", d->character);
	 return;
       }
       switch (OLC_CMD(d).command) {
	 case 'E':
	 case 'P':
	 case 'D':
	   pos = atoi(arg);
	   if ((pos < 0) || (pos > 100))
	     send_to_char("A percentage is from 0 to 100, try again : ", d->character);
	   else {
	     OLC_CMD(d).chance = pos;
	     zedit_disp_menu(d);
	   }
	   break;
	 case 'M':
	 case 'O':
	 case 'G':
	 case 'R':
	 default:
	   /*. We should never get here .*/
	   cleanup_olc(d, CLEANUP_ALL);
	   mudlog("SYSERR: OLC: zedit_parse(): case ARG4: Ack!", BRF, LVL_GOD, TRUE);
	   break;
       }
       break;

     case ZEDIT_ZONE_NAME:
       /*. Add new name and return to main menu .*/
       free(OLC_ZONE(d)->name);
       OLC_ZONE(d)->name = str_dup(arg);
       OLC_ZONE(d)->number = 1;
       zedit_disp_menu(d);
       break;

     case ZEDIT_ZONE_BUILDER:
       /*. Add new builder and return to main menu .*/
       free(OLC_ZONE(d)->builder);
       OLC_ZONE(d)->builder = str_dup(arg);
       OLC_ZONE(d)->number = 1;
       zedit_disp_menu(d);
       break;

     case ZEDIT_ZONE_LEVELREC:
       /*. Add new level recommendation and return to main menu .*/
       free(OLC_ZONE(d)->levelrec);
       OLC_ZONE(d)->levelrec = str_dup(arg);
       OLC_ZONE(d)->number = 1;
       zedit_disp_menu(d);
       break;

     case ZEDIT_ZONE_PKILL:
       /*. Change pkill value. */
       switch (*arg) {
	 case 'N':
	 case 'n':
	   OLC_ZONE(d)->pkill = 0;
	   OLC_ZONE(d)->number = 1;
	   zedit_disp_menu(d);
	   break;
	 case 'Y':
	 case 'y':
	   OLC_ZONE(d)->pkill  = 1;
	   OLC_ZONE(d)->number = 1;
	   zedit_disp_menu(d);
	   break;
	 default:
	   send_to_char("Try again (Yes/No): ", d->character);
	   break;
       }
       break;

     case ZEDIT_ZONE_RECALL:
       /*. Parse and add new recall location and return to main menu .*/
       pos = real_room(atol(arg));
       if (pos >= 0) {
	 OLC_ZONE(d)->recall_loc = atol(arg);
	 OLC_ZONE(d)->number = 1;
	 zedit_disp_menu(d);
       } else
	 send_to_char("That room does not exist, try again : ", d->character);
       break;

     case ZEDIT_ZONE_RESET:
       /*. Parse and add new reset_mode and return to main menu .*/
       pos = atoi(arg);
       if (!isdigit(*arg) || (pos <  0) || (pos > 2))
	 send_to_char("Try again (0-2) : ", d->character);
       else {
	 OLC_ZONE(d)->reset_mode = pos;
	 OLC_ZONE(d)->number = 1;
	 zedit_disp_menu(d);
       }
       break;

     case ZEDIT_ZONE_LIFE:
       /*. Parse and add new lifespan and return to main menu .*/
       pos = atoi(arg);
       if (!isdigit(*arg) || (pos <  0) || (pos > 240))
	 send_to_char("Try again (0-240) : ", d->character);
       else {
	 OLC_ZONE(d)->lifespan = pos;
	 OLC_ZONE(d)->number = 1;
	 zedit_disp_menu(d);
       }
       break;

     case ZEDIT_ZONE_TOP:
       /*. Parse and add new top room in zone and return to main menu .*/
       if (OLC_ZNUM(d) == top_of_zone_table)
	 OLC_ZONE(d)->top = MAX((long)(OLC_ZNUM(d) * 100), atol(arg));
       else
	 OLC_ZONE(d)->top = MAX((long)(OLC_ZNUM(d) * 100), (long)MIN((long)(zone_table[OLC_ZNUM(d) +1].number * 100), atol(arg)));
       zedit_disp_menu(d);
       break;

     default:
       /*. We should never get here .*/
       cleanup_olc(d, CLEANUP_ALL);
       mudlog("SYSERR: OLC: zedit_parse(): Reached default case!",BRF,LVL_GOD,TRUE);
       break;
   }
}


/*
 * Create a new room with some default strings.
 */
void redit_setup_new(struct descriptor_data *d)
{
   CREATE(OLC_ROOM(d), struct room_data, 1);

   OLC_ROOM(d)->name = str_dup("An unfinished room");
   OLC_ROOM(d)->description = str_dup("You are in an unfinished room.\r\n");
   redit_disp_menu(d);
   OLC_VAL(d) = 0;
}

/*
 * Copy over an existing room to work with.
 */
void redit_setup_existing(struct descriptor_data *d, long real_num)
{
   struct room_data *room;
   int    counter;

   /*. Build a copy of the room .*/
   CREATE (room, struct room_data, 1);
   *room = world[real_num];
   /* allocate space for all strings  */
   if (world[real_num].name)
     room->name = str_dup (world[real_num].name);
   if (world[real_num].description)
     room->description = str_dup (world[real_num].description);

   /* exits - alloc only if necessary */
   for (counter = 0; counter < NUM_OF_DIRS; counter++) {
     if (world[real_num].dir_option[counter]) {
       CREATE(room->dir_option[counter], struct room_direction_data, 1);
       /* copy numbers over */
       *room->dir_option[counter] = *world[real_num].dir_option[counter];
       /* malloc strings */
       if (world[real_num].dir_option[counter]->general_description)
	 room->dir_option[counter]->general_description =
	    str_dup(world[real_num].dir_option[counter]->general_description);
       if (world[real_num].dir_option[counter]->keyword)
	 room->dir_option[counter]->keyword =
	    str_dup(world[real_num].dir_option[counter]->keyword);
     }
   }

   /*. Extra descriptions if necessary .*/
   if (world[real_num].ex_description) {
     struct extra_descr_data *thisp, *temp, *temp2;
     CREATE (temp, struct extra_descr_data, 1);
     room->ex_description = temp;
     for (thisp = world[real_num].ex_description; thisp; thisp = thisp->next) {
       if (thisp->keyword)
	 temp->keyword = str_dup (thisp->keyword);
       if (thisp->description)
	 temp->description = str_dup (thisp->description);
       if (thisp->next) {
	 CREATE (temp2, struct extra_descr_data, 1);
	 temp->next = temp2;
	 temp = temp2;
       } else
	 temp->next = NULL;
     }
   }

   /*. Attatch room copy to players descriptor .*/
   OLC_ROOM(d) = room;
   OLC_VAL(d) = 0;
   redit_disp_menu(d);
}

/*
 * Save the room to memory
 */
void redit_save_internally(struct descriptor_data *d)
{
   int    j, found = 0, zone, cmd_no;
   long   i, room_num;
   struct room_data *new_world;
   struct char_data *temp_ch;
   struct obj_data *temp_obj;

   room_num = real_room(OLC_NUM(d));
   if (room_num > 0) {
     /*. Room exits: move contents over then free and replace it .*/
     OLC_ROOM(d)->contents = world[room_num].contents;
     OLC_ROOM(d)->people = world[room_num].people;
     redit_free_room(world + room_num);
     world[room_num] = *OLC_ROOM(d);
   } else {
     /*. Room doesn't exist, hafta add it .*/
     CREATE(new_world, struct room_data, top_of_world + 2);
     /* count thru world tables */
     for (i = 0; i <= top_of_world + 1; i++) {
       if (!found) {
	 /*. Is this the place? .*/
	 if ((i > top_of_world) || (world[i].number > OLC_NUM(d))) {
	   found = 1;

	   new_world[i] = *(OLC_ROOM(d));
	   new_world[i].number = OLC_NUM(d);
	   new_world[i].func = NULL;
	   room_num  = i;

	   /* copy from world to new_world + 1 */
	   if (i <= top_of_world) {
	     new_world[i + 1] = world[i];
	     /* people in this room must have their numbers moved */
	     for (temp_ch = world[i].people; temp_ch; temp_ch = temp_ch->next_in_room) {
	       if (temp_ch->in_room != -1)
		 temp_ch->in_room = i + 1;
	     }
	     /* move objects */
	     for (temp_obj = world[i].contents; temp_obj; temp_obj = temp_obj->next_content) {
	       if (temp_obj != NULL && temp_obj->in_room != -1)
		 temp_obj->in_room = i + 1;
	     }
	   }
	 } else {
	   /*. Not yet placed, copy straight over .*/
	   new_world[i] = world[i];
	 }
       } else {
	 /*. Already been found  .*/

	 /* people in this room must have their in_rooms moved */
	 for (temp_ch = world[i].people; temp_ch; temp_ch = temp_ch->next_in_room) {
	   if (temp_ch->in_room != -1)
	     temp_ch->in_room = i + 1;
	 }
	 /* move objects */
	 for (temp_obj = world[i].contents; temp_obj; temp_obj = temp_obj->next_content)  {
	   if (temp_obj != NULL && temp_obj->in_room != -1)
	     temp_obj->in_room = i + 1;
	 }
	 new_world[i + 1] = world[i];
       }
     }
     /* copy world table over */
     free(world);
     world = new_world;
     top_of_world++;

     /*. Update zone table .*/
     for (zone = 0; zone <= top_of_zone_table; zone++) {
       for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
	 switch (ZCMD.command) {
	   case 'M':
	   case 'O':
	     if (ZCMD.arg3 >= room_num)
	       ZCMD.arg3++;
	     break;
	   case 'D':
	   case 'R':
	     if (ZCMD.arg1 >= room_num)
	       ZCMD.arg1++;
	   case 'G':
	   case 'P':
	   case 'E':
	   case '*':
	     break;
	   default:
	     mudlog("SYSERR: OLC: redit_save_internally: Unknown comand", BRF, LVL_GOD, TRUE);
	 }
       }
     }
     /* update load rooms, to fix creeping load room problem */
     if (room_num <= r_mortal_start_room)
       r_mortal_start_room++;
     if (room_num <= r_immort_start_room)
       r_immort_start_room++;
     if (room_num <= r_frozen_start_room)
       r_frozen_start_room++;

     /*. Update world exits .*/
     for (i = 0; i < top_of_world + 1; i++) {
       for (j = 0; j < NUM_OF_DIRS; j++)  {
	 if (W_EXIT(i, j))  {
	   if (W_EXIT(i, j)->to_room >= room_num)
	     W_EXIT(i, j)->to_room++;
	 }
       }
     }
   }
   olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ROOM);
}


/*
 * Save the world file to disk.
 */
void redit_save_to_disk(struct descriptor_data *d)
{
   long counter, counter2, realcounter;
   FILE *fp;
   char fname[64], backname[72], roomflags[64], telebuf[80];
   struct room_data *room;
   struct extra_descr_data *ex_desc;

   sprintf(fname, "%s/%d.wld", WLD_PREFIX, zone_table[OLC_ZNUM(d)].number);
   sprintf(backname, "%s/%d.wld.back", WLD_PREFIX, zone_table[OLC_ZNUM(d)].number);
   /* Save a copy of the world file in case anything goes wrong */
   if (rename(fname, backname) == 0) {
     sprintf(buf, "Backup Copy saved of %d.wld.", zone_table[OLC_ZNUM(d)].number);
     mudlog(buf, CMP, LVL_BUILDER, TRUE);
   } else {
     /* Unable to back it up, does it exist? */
     if (!(fp = fopen(fname, "a+"))) {
       sprintf(buf, "ERROR: Could not backup %d.wld, new wld not written to disk!", zone_table[OLC_ZNUM(d)].number);
       mudlog(buf, CMP, LVL_BUILDER, TRUE);
       return;
     }
  }
  if (!(fp = fopen(fname, "w+"))) {
    mudlog("SYSERR: OLC: Cannot open room file!", BRF, LVL_GOD, TRUE);
    return;
  }

  for (counter = zone_table[OLC_ZNUM(d)].number * 100;
       counter <= zone_table[OLC_ZNUM(d)].top;
       counter++) {
    realcounter = real_room(counter);
    if (realcounter >= 0) {
      room = (world + realcounter);
      /*. Remove the '\r\n' sequences from description .*/
      strcpy(buf1, room->description ? room->description : "\0");
      strip_string(buf1);

      sprintbits(room->room_flags, roomflags);
      if (roomflags[0] == '\0')
	strcpy(roomflags, "0");
      /*. Build a buffer ready to save .*/
      if (room->tele_targ > 1)
	sprintf(telebuf, "-1 %d %ld %ld %d", room->tele_time,
	    room->tele_targ, room->tele_mask, room->sector_type);
      else if (room->sector_type == SECT_WATER_NOSWIM ||
		 room->sector_type == SECT_UNDERWATER)
	sprintf(telebuf, "%d %d %d", room->sector_type,
	    room->river_speed, room->river_dir);
      else
	sprintf(telebuf, "%d", room->sector_type);
      /* Now save the proper info to the file */
      fprintf(fp, "#%ld\n%s~\n%s~\n%d %s %s\n",
		counter, room->name, buf1,
		zone_table[room->zone].number,
		roomflags, telebuf);
      /*. Handle exits .*/
      for (counter2 = 0; counter2 < NUM_OF_DIRS; counter2++) {
	if (room->dir_option[counter2]) {
	  int temp_door_flag;

	  /*. Again, strip out the crap .*/
	  if (room->dir_option[counter2]->general_description) {
	    strcpy(buf1, room->dir_option[counter2]->general_description);
	    strip_string(buf1);
	  } else
	    *buf1 = 0;

	  /*. Figure out door flag .*/
	  if (IS_SET(room->dir_option[counter2]->exit_info, EX_ISDOOR))
	    temp_door_flag = 1;
	  else if (IS_SET(room->dir_option[counter2]->exit_info, EX_PICKPROOF))
	    temp_door_flag = 2;
	  else if (IS_SET(room->dir_option[counter2]->exit_info, EX_SECRET))
	    temp_door_flag = 3;
	  else if (IS_SET(room->dir_option[counter2]->exit_info, EX_SECRET | EX_ISDOOR))
	    temp_door_flag = 4;
	  else if (IS_SET(room->dir_option[counter2]->exit_info, EX_SECRET | EX_PICKPROOF | EX_ISDOOR))
	    temp_door_flag = 5;
	  else if (IS_SET(room->dir_option[counter2]->exit_info, EX_CLIMB))
	    temp_door_flag = 6;
	  else if (IS_SET(room->dir_option[counter2]->exit_info, EX_CLIMB | EX_ISDOOR))
	    temp_door_flag = 7;
	  else
	    temp_door_flag = 0;

	  /*. Check for keywords .*/
	  if (room->dir_option[counter2]->keyword)
	    strcpy(buf2, room->dir_option[counter2]->keyword);
	  else
	    *buf2 = 0;

	  /*. Ok, now build a buffer to output to file .*/
	  fprintf(fp, "D%ld\n%s~\n%s~\n%d %ld %ld\n",
			counter2, buf1, buf2, temp_door_flag,
			room->dir_option[counter2]->key,
			world[room->dir_option[counter2]->to_room].number);
	}
      }
      /* See if we have any extra descrs in this room */
      if (room->ex_description) {
	for (ex_desc = room->ex_description; ex_desc; ex_desc = ex_desc->next) {
	  /*. Home straight, just deal with extras descriptions..*/
	  strcpy(buf1, ex_desc->description);
	  strip_string(buf1);
	  fprintf(fp, "E\n%s~\n%s~\n", ex_desc->keyword,buf1);
	}
      }
      fprintf(fp, "S\n");
    }
  }
  /* write final line and close */
  fprintf(fp, "$~\n");
  fclose(fp);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ROOM);
}

/*
 * Frees the memory used by a room structure
 */
void redit_free_room(struct room_data *room)
{
   int i;
   struct extra_descr_data *thisp, *next;

   if (room->name)
     free(room->name);
   if (room->description)
     free(room->description);

   /*. Free exits .*/
   for (i = 0; i < NUM_OF_DIRS; i++) {
     if (room->dir_option[i]) {
       if (room->dir_option[i]->general_description)
	 free(room->dir_option[i]->general_description);
       if (room->dir_option[i]->keyword)
	 free(room->dir_option[i]->keyword);
     }
     free(room->dir_option[i]);
   }

   /*. Free extra descriptions .*/
   for (thisp = room->ex_description; thisp; thisp = next) {
     next = thisp->next;
     if (thisp->keyword)
       free(thisp->keyword);
     if (thisp->description)
       free(thisp->description);
     free(thisp);
   }
}


/**************************************************************************
 Menu functions
 **************************************************************************/

/* For extra descriptions */
void redit_disp_extradesc_menu(struct descriptor_data * d)
{
  struct extra_descr_data *extra_desc = OLC_DESC(d);

  sprintf(buf, "[H[J"
	"%s1%s) Keyword: %s%s\r\n"
	"%s2%s) Description:\r\n%s%s\r\n"
	"%s3%s) Goto next description: ",
	grn, nrm, yel, extra_desc->keyword ? extra_desc->keyword : "<NONE>",
	grn, nrm, yel, extra_desc->description ?  extra_desc->description : "<NONE>",
	grn, nrm
  );

  if (!extra_desc->next)
    strcat(buf, "<NOT SET>\r\n");
  else
    strcat(buf, "Set.\r\n");
  strcat(buf, "Enter choice (0 to quit) : ");
  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_EXTRADESC_MENU;
}

/* For exits */
void redit_disp_exit_menu(struct descriptor_data * d)
{
  /* if exit doesn't exist, alloc/create it */
  if (!OLC_EXIT(d))
    CREATE(OLC_EXIT(d), struct room_direction_data, 1);

  /* weird door handling! */
  if (IS_SET(OLC_EXIT(d)->exit_info, EX_ISDOOR)) {
    if (IS_SET(OLC_EXIT(d)->exit_info, EX_PICKPROOF))
      strcpy(buf2, "Pickproof");
    else
      strcpy(buf2, "Is a door");
  } else
    strcpy(buf2, "No door");

  get_char_cols(d->character);
  sprintf(buf, "[H[J"
	"%s1%s) Exit to     : %s%ld\r\n"
	"%s2%s) Description : %s%s\r\n"
	"%s3%s) Door name   : %s%s\r\n"
	"%s4%s) Key         : %s%ld\r\n"
	"%s5%s) Door flags  : %s%s\r\n"
	"%s6%s) Purge exit.\r\n"
	"Enter choice, 0 to quit : ",

	grn, nrm, cyn, world[OLC_EXIT(d)->to_room].number,
	grn, nrm, yel, OLC_EXIT(d)->general_description ? OLC_EXIT(d)->general_description : "<NONE>",
	grn, nrm, yel, OLC_EXIT(d)->keyword ? OLC_EXIT(d)->keyword : "<NONE>",
	grn, nrm, cyn, OLC_EXIT(d)->key,
	grn, nrm, cyn, buf2, grn, nrm
  );

  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_EXIT_MENU;
}

/* For exit flags */
void redit_disp_exit_flag_menu(struct descriptor_data * d)
{
  get_char_cols(d->character);
  sprintf(buf,  "%s0%s) No door\r\n"
		"%s1%s) Closeable door\r\n"
		"%s2%s) Pickproof\r\n"
		"%s3%s) Secret\r\n"
		"%s4%s) Climbable\r\n"
		"Enter choice : ",
		grn, nrm, grn, nrm, grn, nrm, grn, nrm, grn, nrm);
  send_to_char(buf, d->character);
}

/* For room flags */
void redit_disp_flag_menu(struct descriptor_data * d)
{
   int counter, columns = 0;

   get_char_cols(d->character);
   send_to_char("[H[J", d->character);
   for (counter = 0; counter < NUM_ROOM_FLAGS; counter++) {
     sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter + 1, nrm, room_bits[counter]);
     if (!(++columns % 2))
       strcat(buf, "\r\n");
     send_to_char(buf, d->character);
   }
   sprintbit(OLC_ROOM(d)->room_flags, room_bits, buf1);
   sprintf(buf,
	"\r\nRoom flags: %s%s%s\r\n"
	"Enter room flags, 0 to quit : ",
	cyn, buf1, nrm
   );
   send_to_char(buf, d->character);
   OLC_MODE(d) = REDIT_FLAGS;
}

/* for sector type */
void redit_disp_sector_menu(struct descriptor_data * d)
{
   int counter, columns = 0;

   send_to_char("[H[J", d->character);
   for (counter = 0; counter < NUM_ROOM_SECTORS; counter++) {
     sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter, nrm, sector_types[counter]);
     if (!(++columns % 2))
       strcat(buf, "\r\n");
     send_to_char(buf, d->character);
   }
   send_to_char("(-1 to set Teleport Room)\r\n", d->character);
   send_to_char("\r\nEnter sector type : ", d->character);
   OLC_MODE(d) = REDIT_SECTOR;
}

/* for tele type */
void redit_disp_tele_menu(struct descriptor_data * d)
{
   send_to_char("[H[J", d->character);
   sprintf(buf, "%s 1%s) Teleport Destination Room : %ld\r\n"
		"%s 2%s) Teleport Timer            : %d\r\n"
		"%s 3%s) Teleport Masking          : %ld\r\n\r\n",
	       grn, nrm, OLC_ROOM(d)->tele_targ,
	       grn, nrm, OLC_ROOM(d)->tele_time,
	       grn, nrm, OLC_ROOM(d)->tele_mask);
  strcat(buf, "Enter Choice : ");
  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_TELE_STUFF;
}

void redit_disp_tmask_menu(struct descriptor_data * d)
{
   send_to_char("[H[J", d->character);
   sprintf(buf, "%s 1%s) Force Look on Teleport : %s\r\n\r\n",
	       grn, nrm, IS_SET(OLC_ROOM(d)->tele_mask, TELE_LOOK) ? "YES" : "NO");
   strcat(buf, "Enter Choice : ");
   send_to_char(buf, d->character);
   OLC_MODE(d) = REDIT_TELE_MASK;
}

/* the main menu */
void redit_disp_menu(struct descriptor_data * d)
{
   struct room_data *room;

   get_char_cols(d->character);
   room = OLC_ROOM(d);

   sprintbit((long) room->room_flags, room_bits, buf1);
   sprinttype(room->sector_type, sector_types, buf2);
   sprintf(buf,
	"[H[J"
	"-- Room number : [%s%ld%s]  	Room zone: [%s%d%s]\r\n"
	"%s1%s) Name        : %s%s\r\n"
	"%s2%s) Description :\r\n%s%s"
	"%s3%s) Room flags  : %s%s\r\n"
	"%s4%s) Sector type : %s%s\r\n"
	"%s5%s) Exit north  : %s%ld\r\n"
	"%s6%s) Exit east   : %s%ld\r\n"
	"%s7%s) Exit south  : %s%ld\r\n"
	"%s8%s) Exit west   : %s%ld\r\n"
	"%s9%s) Exit up     : %s%ld\r\n"
	"%sA%s) Exit down   : %s%ld\r\n"
	"%sB%s) Exit northeast  : %s%ld\r\n"
	"%sC%s) Exit northwest  : %s%ld\r\n"
	"%sD%s) Exit southeast  : %s%ld\r\n"
	"%sE%s) Exit southwest  : %s%ld\r\n"
	"%sF%s) Extra descriptions menu\r\n"
	"%sQ%s) Quit\r\n"
	"Enter choice : ",

	cyn, OLC_NUM(d), nrm,
	cyn, zone_table[OLC_ZNUM(d)].number, nrm,
	grn, nrm, yel, room->name,
	grn, nrm, yel, room->description,
	grn, nrm, cyn, buf1,
	grn, nrm, cyn, buf2,
	grn, nrm, cyn, room->dir_option[NORTH] ?
	  world[room->dir_option[NORTH]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[EAST] ?
	  world[room->dir_option[EAST]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[SOUTH] ?
	  world[room->dir_option[SOUTH]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[WEST] ?
	  world[room->dir_option[WEST]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[UP] ?
	  world[room->dir_option[UP]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[DOWN] ?
	  world[room->dir_option[DOWN]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[NORTHEAST] ?
	  world[room->dir_option[NORTHEAST]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[NORTHWEST] ?
	  world[room->dir_option[NORTHWEST]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[SOUTHEAST] ?
	  world[room->dir_option[SOUTHEAST]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[SOUTHWEST] ?
	  world[room->dir_option[SOUTHWEST]->to_room].number : -1,
	grn, nrm, grn, nrm
  );
  send_to_char(buf, d->character);

  OLC_MODE(d) = REDIT_MAIN_MENU;
}


/**************************************************************************
  The main loop
 **************************************************************************/

void redit_parse(struct descriptor_data * d, char *arg)
{
   long number;

   switch (OLC_MODE(d)) {
     case REDIT_CONFIRM_SAVESTRING:
       switch (*arg) {
	 case 'y':
	 case 'Y':
	   redit_save_internally(d);
	   sprintf(buf, "OLC: %s edits room %ld", GET_NAME(d->character), OLC_NUM(d));
	   mudlog(buf, CMP, LVL_GOD, TRUE);
	   /*. Do NOT free strings! just the room structure .*/
	   cleanup_olc(d, CLEANUP_STRUCTS);
	   send_to_char("Room saved to memory.\r\n", d->character);
	   break;
	 case 'n':
	 case 'N':
	   /* free everything up, including strings etc */
	   cleanup_olc(d, CLEANUP_ALL);
	   break;
	 default:
	   send_to_char("Invalid choice!\r\n", d->character);
	   send_to_char("Do you wish to save this room internally? : ", d->character);
	   break;
       }
       return;

     case REDIT_MAIN_MENU:
       switch (*arg) {
	 case 'q':
	 case 'Q':
	   if (OLC_VAL(d)) {
	     /*. Something has been modified .*/
	     send_to_char("Do you wish to save this room internally? : ", d->character);
	     OLC_MODE(d) = REDIT_CONFIRM_SAVESTRING;
	   } else
	     cleanup_olc(d, CLEANUP_ALL);
	   return;
	 case '1':
	   send_to_char("Enter room name:-\r\n| ", d->character);
	   OLC_MODE(d) = REDIT_NAME;
	   break;
	 case '2':
	   send_to_char("Enter room description:-\r\n", d->character);
	   OLC_MODE(d) = REDIT_DESC;
	   d->str = (char **) malloc(sizeof(char *));
	   *(d->str) = NULL;
	   d->max_str = MAX_ROOM_DESC;
	   d->mail_to = 0;
	   OLC_VAL(d) = REDIT_DESC;
	   break;
	 case '3':
	   redit_disp_flag_menu(d);
	   break;
	 case '4':
	   redit_disp_sector_menu(d);
	   break;
	 case '5':
	   OLC_VAL(d) = NORTH;
	   redit_disp_exit_menu(d);
	   break;
	 case '6':
	   OLC_VAL(d) = EAST;
	   redit_disp_exit_menu(d);
	   break;
	 case '7':
	   OLC_VAL(d) = SOUTH;
	   redit_disp_exit_menu(d);
	   break;
	 case '8':
	   OLC_VAL(d) = WEST;
	   redit_disp_exit_menu(d);
	   break;
	 case '9':
	   OLC_VAL(d) = UP;
	   redit_disp_exit_menu(d);
	   break;
	 case 'a':
	 case 'A':
	   OLC_VAL(d) = DOWN;
	   redit_disp_exit_menu(d);
	   break;
	 case 'b':
	 case 'B':
	   OLC_VAL(d) = NORTHEAST;
	   redit_disp_exit_menu(d);
	   break;
	 case 'c':
	 case 'C':
	   OLC_VAL(d) = NORTHWEST;
	   redit_disp_exit_menu(d);
	   break;
	 case 'd':
	 case 'D':
	   OLC_VAL(d) = SOUTHEAST;
	   redit_disp_exit_menu(d);
	   break;
	 case 'e':
	 case 'E':
	   OLC_VAL(d) = SOUTHWEST;
	   redit_disp_exit_menu(d);
	   break;
	 case 'f':
	 case 'F':
	   /* if extra desc doesn't exist . */
	   if (!OLC_ROOM(d)->ex_description) {
	     CREATE(OLC_ROOM(d)->ex_description, struct extra_descr_data, 1);
	     OLC_ROOM(d)->ex_description->next = NULL;
	   }
	   OLC_DESC(d) = OLC_ROOM(d)->ex_description;
	   redit_disp_extradesc_menu(d);
	   break;
	 default:
	   send_to_char("Invalid choice!", d->character);
	   redit_disp_menu(d);
	   break;
       }
       return;

     case REDIT_NAME:
       if (OLC_ROOM(d)->name)
	 free(OLC_ROOM(d)->name);
       if (strlen(arg) > MAX_ROOM_NAME)
	 arg[MAX_ROOM_NAME -1] = 0;
       OLC_ROOM(d)->name = str_dup(arg);
       break;

     case REDIT_DESC:
       /* we will NEVER get here */
       mudlog("SYSERR: Reached REDIT_DESC case in parse_redit",BRF,LVL_GOD,TRUE);
       break;

     case REDIT_FLAGS:
       number = atoi(arg);
       if ((number < 0) || (number > NUM_ROOM_FLAGS)) {
	 send_to_char("That's not a valid choice!\r\n", d->character);
	 redit_disp_flag_menu(d);
       } else {
	 if (number == 0)
	   break;
	 else {
	   /* toggle bits */
	   if (IS_SET(OLC_ROOM(d)->room_flags, 1 << (number - 1)))
	     REMOVE_BIT(OLC_ROOM(d)->room_flags, 1 << (number - 1));
	   else
	     SET_BIT(OLC_ROOM(d)->room_flags, 1 << (number - 1));
	   redit_disp_flag_menu(d);
	 }
       }
       return;

     case REDIT_SECTOR:
       number = atoi(arg);
       if (number < -1 || number >= NUM_ROOM_SECTORS) {
	 send_to_char("Invalid choice!", d->character);
	 redit_disp_sector_menu(d);
	 return;
       } else if (number == -1) {
	 redit_disp_tele_menu(d);
	 return;
#if 0
       } else if (number == SECT_WATER_NOSWIM || number == SECT_UNDERWATER) {
	 redit_disp_river_menu(d);
	 return;
#endif
       } else
	 OLC_ROOM(d)->sector_type = number;
       break;

     case REDIT_TELE_STUFF:
       switch (*arg) {
	 case 1:
	   OLC_MODE(d) = REDIT_TELE_DEST;
	   send_to_char("Teleport to room number : ", d->character);
	   return;
	 case 2:
	   OLC_MODE(d) = REDIT_TELE_TIME;
	   send_to_char("Teleport Time : ", d->character);
	   return;
	 case 3:
	   redit_disp_tmask_menu(d);
	   return;
	 case 0:
	   redit_disp_sector_menu(d);
	   return;
	 default:
	   send_to_char("Invalid choice!", d->character);
	   redit_disp_tele_menu(d);
	   return;
       }
       break;

     case REDIT_TELE_DEST:
       if (real_room(atol(arg)) == NOWHERE)  {
	 send_to_char("Destination room doesn't exist!", d->character);
	 redit_disp_tele_menu(d);
	 return;
       } else
	 OLC_ROOM(d)->tele_targ = atol(arg);
       break;

     case REDIT_TELE_TIME:
       number = atoi(arg);
       if (number < 0 || number > 256) {
	 send_to_char("Value out of range!", d->character);
	 redit_disp_tele_menu(d);
	 return;
       } else
	 OLC_ROOM(d)->tele_time = number;
       break;

     case REDIT_TELE_MASK:
       number = atoi(arg);
       if ((number < 0) || (number > 1)) {
	 send_to_char("That's not a valid choice!\r\n", d->character);
	 redit_disp_tmask_menu(d);
       } else {
	 if (number == 0) {
	   redit_disp_tele_menu(d);
	   return;
	 } else {
	   /* toggle bits */
	   if (IS_SET(OLC_ROOM(d)->tele_mask, 1 << (number - 1)))
	     REMOVE_BIT(OLC_ROOM(d)->tele_mask, 1 << (number - 1));
	   else
	     SET_BIT(OLC_ROOM(d)->tele_mask, 1 << (number - 1));
	   redit_disp_tmask_menu(d);
	 }
       }
       break;

     case REDIT_EXIT_MENU:
       switch (*arg) {
	 case '0':
	   break;
	 case '1':
	   OLC_MODE(d) = REDIT_EXIT_NUMBER;
	   send_to_char("Exit to room number : ", d->character);
	   return;
	 case '2':
	   OLC_MODE(d) = REDIT_EXIT_DESCRIPTION;
	   d->str = (char **) malloc(sizeof(char *));
	   *(d->str) = NULL;
	   d->max_str = MAX_EXIT_DESC;
	   d->mail_to = 0;
	   send_to_char("Enter exit description:-\r\n", d->character);
	   return;
	 case '3':
	   OLC_MODE(d) = REDIT_EXIT_KEYWORD;
	   send_to_char("Enter keywords : ", d->character);
	   return;
	 case '4':
	   OLC_MODE(d) = REDIT_EXIT_KEY;
	   send_to_char("Enter key number : ", d->character);
	   return;
	 case '5':
	   redit_disp_exit_flag_menu(d);
	   OLC_MODE(d) = REDIT_EXIT_DOORFLAGS;
	   return;
	 case '6':
	   /* delete exit */
	   if (OLC_EXIT(d)->keyword)
	     free(OLC_EXIT(d)->keyword);
	   if (OLC_EXIT(d)->general_description)
	     free(OLC_EXIT(d)->general_description);
	   free(OLC_EXIT(d));
	   OLC_EXIT(d) = NULL;
	   break;
	 default:
	   send_to_char("Try again : ", d->character);
	   return;
       }
       break;

     case REDIT_EXIT_NUMBER:
       number = real_room(atol(arg));
       if (number < 0)
	 send_to_char("That room does not exist, try again : ", d->character);
       else {
	 OLC_EXIT(d)->to_room = number;
	 redit_disp_exit_menu(d);
       }
       return;

     case REDIT_EXIT_DESCRIPTION:
       /* we should NEVER get here */
       mudlog("SYSERR: Reached REDIT_EXIT_DESC case in parse_redit",BRF,LVL_GOD,TRUE);
       break;

     case REDIT_EXIT_KEYWORD:
       if (OLC_EXIT(d)->keyword)
	 free(OLC_EXIT(d)->keyword);
       OLC_EXIT(d)->keyword = str_dup(arg);
       redit_disp_exit_menu(d);
       return;

     case REDIT_EXIT_KEY:
       number = atol(arg);
       OLC_EXIT(d)->key = number;
       redit_disp_exit_menu(d);
       return;

     case REDIT_EXIT_DOORFLAGS:
       number = atoi(arg);
       if ((number < 0) || (number > 7)) {
	 send_to_char("That's not a valid choice!\r\n", d->character);
	 redit_disp_exit_flag_menu(d);
       } else {
	 /* doors are a bit idiotic, don't you think? :) */
	 if (number == 0)
	   OLC_EXIT(d)->exit_info = 0;
	 else if (number == 1)
	   OLC_EXIT(d)->exit_info = EX_ISDOOR;
	 else if (number == 2)
	   OLC_EXIT(d)->exit_info = EX_ISDOOR | EX_PICKPROOF;
	 else if (number == 3)
	   OLC_EXIT(d)->exit_info = EX_SECRET;
	 else if (number == 4)
	   OLC_EXIT(d)->exit_info = EX_SECRET | EX_ISDOOR;
	 else if (number == 5)
	   OLC_EXIT(d)->exit_info = EX_SECRET | EX_PICKPROOF | EX_ISDOOR;
	 else if (number == 6)
	   OLC_EXIT(d)->exit_info = EX_CLIMB;
	 else if (number == 7)
	   OLC_EXIT(d)->exit_info = EX_CLIMB | EX_ISDOOR;
	 else
	   OLC_EXIT(d)->exit_info = 0;
	 /* jump out to menu */
	 redit_disp_exit_menu(d);
       }
       return;

     case REDIT_EXTRADESC_KEY:
       OLC_DESC(d)->keyword = str_dup(arg);
       redit_disp_extradesc_menu(d);
       return;

     case REDIT_EXTRADESC_MENU:
       number = atoi(arg);
       switch (number) {
	 case 0:
	   {
	   /*
	    * If something got left out, delete the extra desc
	    *  when backing out to menu.
	    */
	    if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) {
	      struct extra_descr_data **tmp_desc;

	      if (OLC_DESC(d)->keyword)
		free(OLC_DESC(d)->keyword);
	      if (OLC_DESC(d)->description)
		free(OLC_DESC(d)->description);

	      /*. Clean up pointers .*/
	      for (tmp_desc = &(OLC_ROOM(d)->ex_description);
		   *tmp_desc;
		   tmp_desc = &((*tmp_desc)->next)) {
		if (*tmp_desc == OLC_DESC(d)) {
		  *tmp_desc = NULL;
		  break;
		}
	      }
	      free(OLC_DESC(d));
	    }
	   }
	   break;
	 case 1:
	   OLC_MODE(d) = REDIT_EXTRADESC_KEY;
	   send_to_char("Enter keywords, separated by spaces : ", d->character);
	   return;
	 case 2:
	   OLC_MODE(d) = REDIT_EXTRADESC_DESCRIPTION;
	   send_to_char("Enter extra description:-\r\n", d->character);
	   /* send out to modify.c */
	   d->str = (char **) malloc(sizeof(char *));
	   *(d->str) = NULL;
	   d->max_str = MAX_EXTRA_DESC;
	   d->mail_to = 0;
	   return;
	 case 3:
	   if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) {
	     send_to_char("You can't edit the next extra desc without completing this one.\r\n", d->character);
	     redit_disp_extradesc_menu(d);
	   } else {
	     struct extra_descr_data *new_extra;

	     if (OLC_DESC(d)->next)
	       OLC_DESC(d) = OLC_DESC(d)->next;
	     else {
	       /* make new extra, attach at end */
	       CREATE(new_extra, struct extra_descr_data, 1);
	       OLC_DESC(d)->next = new_extra;
	       OLC_DESC(d) = new_extra;
	     }
	     redit_disp_extradesc_menu(d);
	   }
	   return;
       }
       break;

     default:
       /* we should never get here */
       mudlog("SYSERR: Reached default case in parse_redit",BRF,LVL_GOD,TRUE);
       break;
   }
   /*. If we get this far, something has be changed .*/
   OLC_VAL(d) = 1;
   redit_disp_menu(d);
}

/*
 * Creates a new mob with some default values.
 */
void medit_setup_new(struct descriptor_data *d)
{
  struct char_data *mob;

  /*. Alloc some mob shaped space .*/
  CREATE(mob, struct char_data, 1);
  init_mobile(mob);

  GET_MOB_RNUM(mob) = -1;
  /*. default strings .*/
  GET_ALIAS(mob) = str_dup("mob unfinished");
  GET_SDESC(mob) = str_dup("the unfinished mob");
  GET_LDESC(mob) = str_dup("An unfinished mob stands here.\r\n");
  GET_DDESC(mob) = str_dup("It looks, err, unfinished.\r\n");

  OLC_MOB(d) = mob;
  OLC_VAL(d) = 0;   /*. Has changed flag .*/
  medit_disp_menu(d);
}


/*
 *  Alloc some space for the mob struct then cal copy_mobile.
 */
void medit_setup_existing(struct descriptor_data *d, long rmob_num)
{
   struct char_data *mob;

   /*. Alloc some mob shaped space .*/
   CREATE(mob, struct char_data, 1);
   copy_mobile(mob, mob_proto + rmob_num);
   OLC_MOB(d) = mob;
   medit_disp_menu(d);
}

/*
 * Copy one mob struct to another.
 */
void copy_mobile(struct char_data *tmob, struct char_data *fmob)
{
   /*. Free up any used strings .*/
   if (GET_ALIAS(tmob))
     free(GET_ALIAS(tmob));
   if (GET_SDESC(tmob))
     free(GET_SDESC(tmob));
   if (GET_LDESC(tmob))
     free(GET_LDESC(tmob));
   if (GET_DDESC(tmob))
     free(GET_DDESC(tmob));

   /*.Copy mob .*/
   *tmob = *fmob;

   /*. Realloc strings .*/
   if (GET_ALIAS(fmob))
     GET_ALIAS(tmob) = str_dup(GET_ALIAS(fmob));

   if (GET_SDESC(fmob))
     GET_SDESC(tmob) = str_dup(GET_SDESC(fmob));

   if (GET_LDESC(fmob))
     GET_LDESC(tmob) = str_dup(GET_LDESC(fmob));

   if (GET_DDESC(fmob))
     GET_DDESC(tmob) = str_dup(GET_DDESC(fmob));
}


/*
 * This initializes all of the working variables of a mobile.
 */
void init_mobile(struct char_data *mob)
{
   clear_char(mob);

   GET_HIT(mob)      = 1;
   GET_MANA(mob)     = 1;
   GET_MAX_MANA(mob) = 100;
   GET_MAX_MOVE(mob) = 100;
   GET_NDD(mob)      = 1;
   GET_SDD(mob)      = 1;
   GET_WEIGHT(mob)   = 200;
   GET_HEIGHT(mob)   = 198;
   GET_RACE(mob)     = RACE_UNDEFINED;
   GET_MOB_TYPE(mob) = 'S';

   mob->real_abils.str   = 12;
   mob->real_abils.intel = 12;
   mob->real_abils.wis   = 12;
   mob->real_abils.dex   = 12;
   mob->real_abils.con   = 12;
   mob->real_abils.cha   = 12;
   mob->real_abils.will  = 12;
   mob->aff_abils        = mob->real_abils;

   SET_BIT(MOB_FLAGS(mob), MOB_ISNPC);
   mob->player_specials  = &dummy_mob;
}

/*
 * Save new/edited mob to memory.
 */
void medit_save_internally(struct descriptor_data *d)
{
   long   rmob_num, found = 0, new_mob_num = 0, zone, cmd_no, shop;
   struct char_data *new_proto;
   struct index_data *new_index;
   struct char_data *live_mob;
   struct descriptor_data *dsc;

   rmob_num = real_mobile(OLC_NUM(d));

   /*. Mob exists? Just update it .*/
   if (rmob_num != -1) {
     copy_mobile((mob_proto + rmob_num), OLC_MOB(d));
     /*. Update live mobiles .*/
     for (live_mob = character_list; live_mob; live_mob = live_mob->next)
       if (IS_MOB(live_mob) && (GET_MOB_RNUM(live_mob) == rmob_num)) {
	 /*. Only really need update the strings, since these can cause
	     protection faults.  The rest can wait till a reset/reboot .*/
	 GET_ALIAS(live_mob) = GET_ALIAS(mob_proto + rmob_num);
	 GET_SDESC(live_mob) = GET_SDESC(mob_proto + rmob_num);
	 GET_LDESC(live_mob) = GET_LDESC(mob_proto + rmob_num);
	 GET_DDESC(live_mob) = GET_DDESC(mob_proto + rmob_num);
       }
   } else {
     /*. Mob does not exist, hafta add it .*/
     CREATE(new_proto, struct char_data, top_of_mobt + 2);
     CREATE(new_index, struct index_data, top_of_mobt + 2);

     for (rmob_num = 0; rmob_num <= top_of_mobt + 1; rmob_num++) {
       if (!found) {
	 /*. Is this the place?  .*/
	 if ((rmob_num > top_of_mobt) ||
	     (mob_index[rmob_num].vnumber > OLC_NUM(d))) {
	   /*. Yep, stick it here .*/
	   found = 1;
	   new_index[rmob_num].vnumber = OLC_NUM(d);
	   new_index[rmob_num].number = 0;
	   new_index[rmob_num].func = NULL;
	   new_mob_num = rmob_num;
	   GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
	   copy_mobile((new_proto + rmob_num), OLC_MOB(d));
	   /*. if there's an entry that should go here, put it in .*/
	   if (rmob_num <= top_of_mobt) {
	     new_index[rmob_num + 1] = mob_index[rmob_num];
	     new_proto[rmob_num + 1] = mob_proto[rmob_num];
	     GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
	   }
	 } else {
	   /*. Nope, copy over as normal.*/
	   new_index[rmob_num] = mob_index[rmob_num];
	   new_proto[rmob_num] = mob_proto[rmob_num];
	 }
       } else {
	 /*. We've already found it, copy the rest over .*/
	 new_index[rmob_num + 1] = mob_index[rmob_num];
	 new_proto[rmob_num + 1] = mob_proto[rmob_num];
	 GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
       }
     }
     /*. Replace tables .*/
     free(mob_index);
     free(mob_proto);
     mob_index = new_index;
     mob_proto = new_proto;
     top_of_mobt++;

     /*. Update live mobile rnums .*/
     for (live_mob = character_list; live_mob; live_mob = live_mob->next) {
       if (GET_MOB_RNUM(live_mob) > new_mob_num)
	 GET_MOB_RNUM(live_mob)++;
     }
     /*. Update zone table .*/
     for (zone = 0; zone <= top_of_zone_table; zone++) {
       for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
	 if (ZCMD.command == 'M')
	   if (ZCMD.arg1 > new_mob_num)
	     ZCMD.arg1++;
       }
     }
     /*. Update shop keepers .*/
     for (shop = 0; shop <= top_shop; shop++) {
       if (SHOP_KEEPER(shop) > new_mob_num)
	 SHOP_KEEPER(shop)++;
     }
     /*. Update keepers in shops being edited .*/
     for (dsc = descriptor_list; dsc; dsc = dsc->next) {
       if (dsc->connected == CON_SEDIT)
	 if (S_KEEPER(OLC_SHOP(dsc)) >= new_mob_num)
	   S_KEEPER(OLC_SHOP(dsc))++;
     }
   }
   olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}


/*
 * Save ALL mobiles for a zone to their .mob file, mobs are all
 *   saved in Extended format, regardless of whether they have any
 *   extended fields.  Thanks to Samedi for ideas on this bit of code.
 */
void medit_save_to_disk(struct descriptor_data *d)
{
   long   i, rmob_num, top;
   int    armor = 0, zone, j = 0;
   FILE   *mob_file;
   char   fname[64], backname[72], mobflags[64], affflags[64];
   struct char_data *mob;
   struct mob_prog_data *pstMProg;
#if 0
   char   *pcSrc;
   char   *pcDst;
#endif
   extern struct index_data *mob_index;

   zone = zone_table[OLC_ZNUM(d)].number;
   top  = zone_table[OLC_ZNUM(d)].top;

   sprintf(fname, "%s/%i.mob", MOB_PREFIX, zone);
   sprintf(backname, "%s/%d.mob.back", MOB_PREFIX, zone);
   /* Make a backup copy in case anything goes wrong */
   if (rename(fname, backname) == 0) {
     sprintf(buf, "Backup Copy saved of %d.mob.", zone);
     mudlog(buf, CMP, LVL_BUILDER, TRUE);
   } else {
     if (!(mob_file = fopen(fname, "a+"))) {
       sprintf(buf, "ERROR: Could not backup %d.mob, new mob not written to disk!", zone);
       mudlog(buf, CMP, LVL_BUILDER, TRUE);
       return;
     } else if (rename(fname, backname) == 0) {
       sprintf(buf, "Backup saved of %d.mob.", zone);
       mudlog(buf, CMP, LVL_BUILDER, TRUE);
     }
   }
   if (!(mob_file = fopen(fname, "w"))) {
     mudlog("SYSERR: OLC: Cannot open mob file!", BRF, LVL_GOD, TRUE);
     return;
   }

   /*. Seach database for mobs in this zone and save em .*/
   for (i = (zone * 100); i <= top; i++) {
     rmob_num = real_mobile(i);

     if (rmob_num != -1) {
       if (fprintf(mob_file, "#%ld\n", i) < 0) {
	 mudlog("SYSERR: OLC: Cannot write mob file!\r\n", BRF, LVL_GOD, TRUE);
	 fclose(mob_file);
	 return;
       }
       mob = (mob_proto + rmob_num);
       /*. Clean up strings .*/
       strcpy(buf1, GET_LDESC(mob));
       strip_string(buf1);
       if (GET_DDESC(mob))
	 strcpy(buf2, GET_DDESC(mob));
       else
	 strcpy(buf2, "You see nothing special.");
       strip_string(buf2);

       sprintbits(MOB_FLAGS(mob), mobflags);
       if (mobflags[0] == '\0')
	 strcpy(mobflags, "0");
       sprintbits(AFF_FLAGS(mob), affflags);
       if (affflags[0] == '\0')
	 strcpy(affflags, "0");

       for (j = 0; j < ARMOR_LIMIT; ++j)
	 armor += mob->points.armor[j];
       armor /= ARMOR_LIMIT;

       fprintf(mob_file,
	"%s~\n"
	"%s~\n"
	"%s~\n"
	"%s~\n"
	"%s %s %i %c\n"
	"%d %d %i %dd%d+%d %dd%d+%d\n"
	"%ld %ld\n"
	"%d %d %d\n",
	GET_ALIAS(mob),
	GET_SDESC(mob),
	buf1,
	buf2,
	mobflags,
	affflags,
	GET_ALIGNMENT(mob),
	GET_MOB_TYPE(mob),
	GET_LEVEL(mob),
	GET_MOB_TYPE(mob) == 'A' ? GET_RACE(mob) : GET_HITROLL(mob),
	GET_MOB_TYPE(mob) == 'A' ? 0 : armor,
	GET_MOB_TYPE(mob) == 'A' ? 0 : GET_HIT(mob),
	GET_MOB_TYPE(mob) == 'A' ? 0 : GET_MANA(mob),
	GET_MOB_TYPE(mob) == 'A' ? 0 : GET_MOVE(mob),
	GET_MOB_TYPE(mob) == 'A' ? 0 : GET_NDD(mob),
	GET_MOB_TYPE(mob) == 'A' ? 0 : GET_SDD(mob),
	GET_MOB_TYPE(mob) == 'A' ? 0 : GET_DAMROLL(mob),
	GET_MOB_TYPE(mob) == 'A' ? GET_ATTACK_TYPE(mob) : GET_GOLD(mob),
	GET_MOB_TYPE(mob) == 'A' ? 0 : GET_EXP(mob),
	GET_POS(mob),
	GET_DEFAULT_POS(mob),
	GET_SEX(mob));

      if (GET_MOB_TYPE(mob) == 'E') {
	/*. Deal with Extra stats in case they are there .*/
	if (GET_ATTACK(mob) != 0)
	  fprintf(mob_file, "BareHandAttack: %d\n", GET_ATTACK_TYPE(mob));
	if (GET_STR(mob) != 11)
	  fprintf(mob_file, "Str: %d\n", GET_STR(mob));
	if (GET_ADD(mob) != 0)
	  fprintf(mob_file, "StrAdd: %d\n", GET_ADD(mob));
	if (GET_DEX(mob) != 11)
	  fprintf(mob_file, "Dex: %d\n", GET_DEX(mob));
	if (GET_INT(mob) != 11)
	  fprintf(mob_file, "Int: %d\n", GET_INT(mob));
	if (GET_WIS(mob) != 11)
	  fprintf(mob_file, "Wis: %d\n", GET_WIS(mob));
	if (GET_CON(mob) != 11)
	  fprintf(mob_file, "Con: %d\n", GET_CON(mob));
	if (GET_CHA(mob) != 11)
	  fprintf(mob_file, "Cha: %d\n", GET_CHA(mob));
	if (GET_RACE(mob) != RACE_UNDEFINED)
	  fprintf(mob_file, "Race: %d\n", GET_RACE(mob));
        if (GET_HEIGHT(mob) != 198)
          fprintf(mob_file, "Height: %d\n", GET_HEIGHT(mob));
        if (GET_WEIGHT(mob) != 200)
          fprintf(mob_file, "Weight: %d\n", GET_WEIGHT(mob));
#if 0
	if (mob_index[GET_MOB_RNUM(mob)].func != NULL)
	  fprintf(mob_file, "Special: %d\n", ??);
#endif
	/*. Add E-mob handlers here .*/
	fprintf(mob_file, "E\n");
      }
      /* save the MOBProg information if applicable */

      pstMProg = mob_index[rmob_num].mobprogs;
      while (pstMProg != NULL) {
#if 0
	pcSrc = pstMProg->comlist;
	pcDst = buf;
	while (*pcSrc != '\0') {
	  if (*pcSrc != '\r') {
	    *pcDst = *pcSrc;
	    pcDst++;
	  }
	  pcSrc++;
	}
	*pcDst = '\0';
#endif
	/* Print out only in_file_prog's */
	if (pstMProg->type == IN_FILE_PROG)
	  fprintf(mob_file, ">in_file_prog %s~\n", pstMProg->arglist);

	pstMProg = pstMProg->next;
      }
      if (mob_index[rmob_num].mobprogs != NULL)
	fprintf(mob_file, "|\n");
    }
  }
  fprintf(mob_file, "$~\n");
  fclose(mob_file);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}

/**************************************************************************
 Menu functions
 **************************************************************************/
/*
 * Display poistions (sitting, standing etc).
 */
void medit_disp_positions(struct descriptor_data *d)
{
  int i;

  get_char_cols(d->character);

  send_to_char("[H[J", d->character);
  for (i = 0; *position_types[i] != '\n'; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, position_types[i]);
    send_to_char(buf, d->character);
  }
  send_to_char("Enter position number : ", d->character);
}

/*
 * Display sex.
 */
void medit_disp_sex(struct descriptor_data *d)
{
  int i;

  get_char_cols(d->character);

  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_GENDERS; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, genders[i]);
    send_to_char(buf, d->character);
  }
  send_to_char("Enter gender number : ", d->character);
}

/*
 * Display attack types menu.
 */
void medit_disp_attack_types(struct descriptor_data *d)
{
  int i;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_ATTACK_TYPES; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n",
	    grn, i, nrm, attack_hit_text[i].singular);
    send_to_char(buf, d->character);
  }
  send_to_char("Enter attack type : ", d->character);
}


/* 
 * Display attrib menu
 */

void medit_disp_attribs(struct descriptor_data *d)
{
  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  sprintf(buf, "Attributes:\r\n"
               "\t[%sS%s]trength     %-2d\t\t[%sD%s]exterity %-2d\r\n"
	       "\t[%sI%s]ntelligence %-2d\t\t[%sW%s]isdom    %-2d\r\n"
	       "\t[%sC%s]onstitution %-2d\t\tC[%sH%s]arisma  %-2d\r\n"
	       "\tWi[%sL%s]lpower    %-2d\t\t[%sQ%s]uit\r\n\r\nYour Choice? ",
          grn, nrm, GET_STR(OLC_MOB(d)), grn, nrm, GET_DEX(OLC_MOB(d)),
          grn, nrm, GET_INT(OLC_MOB(d)), grn, nrm, GET_WIS(OLC_MOB(d)),
          grn, nrm, GET_CON(OLC_MOB(d)), grn, nrm, GET_CHA(OLC_MOB(d)),
          grn, nrm, GET_WILL(OLC_MOB(d)), grn, nrm);
   send_to_char(buf, d->character);
}
 
/*
 * Display race types menu.
 */
void medit_disp_race(struct descriptor_data *d)
{
  int i, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < TOT_RACES; i++)  {
    sprintf(buf, "%s%-2d%s) %-18.18s",
	grn, i, nrm, pc_race_types[i] ? pc_race_types[i] : "Unused");
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  send_to_char("Enter new race : ", d->character);
}

/*
 * Display mob-flags menu.
 */
void medit_disp_mob_flags(struct descriptor_data *d)
{
  int i, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_MOB_FLAGS; i++) {
    sprintf(buf, "%s%2d%s) %-20.20s  ",
	    grn, i+1, nrm, action_bits[i]);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintbit(MOB_FLAGS(OLC_MOB(d)), action_bits, buf1);
  sprintf(buf, "\r\n"
	"Current flags : %s%s%s\r\n"
	"Enter mob flags (0 to quit) : ",
	cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

/*
 * Display aff-flags menu.
 */
void medit_disp_aff_flags(struct descriptor_data *d)
{
  int i, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_AFF_FLAGS; i++) {
    sprintf(buf, "%s%2d%s) %-20.20s  ",
	    grn, i+1, nrm, affected_bits[i]);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintbit(AFF_FLAGS(OLC_MOB(d)), affected_bits, buf1);
  sprintf(buf, "\r\n"
	"Current flags   : %s%s%s\r\n"
	"Enter aff flags (0 to quit) : ",
	cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

/*
 * Display main menu.
 */
void medit_disp_menu(struct descriptor_data * d)
{
  struct char_data *mob;
  int    armor = 0, i;

  mob = OLC_MOB(d);
  get_char_cols(d->character);

  for (i = 0; i < ARMOR_LIMIT; i++)
    armor += mob->points.armor[i];
  armor /= ARMOR_LIMIT;

  sprintf(buf, "[H[J"
	"-- Mob Number:  [%s%ld%s]\r\n"
	"%s1%s) Sex: %s%-7.7s%s	         %s2%s) Alias: %s%s\r\n"
	"%s3%s) S-Desc: %s%s\r\n"
	"%s4%s) L-Desc:-\r\n%s%s"
	"%s5%s) D-Desc:-\r\n%s%s"
	"%s6%s) Level:       [%s%-5d%s],  %s7%s) Alignment:    [%s%4d%s]\r\n"
	"%s8%s) Hitroll:     [%s%-5d%s],  %s9%s) Damroll:      [%s%4d%s]\r\n"
	"%sA%s) NumDamDice:  [%s%-5d%s],  %sB%s) SizeDamDice:  [%s%4d%s]\r\n"
	"%sC%s) Num HP Dice: [%s%-5d%s],  %sD%s) Size HP Dice: [%s%4d%s]\r\n"
	"%sE%s) HP Bonus:    [%s%-5d%s],  %sF%s) Armor Class:  [%s%4d%s]\r\n"
	"%sG%s) Exp:      [%s%-8ld%s],  %sH%s) Gold:     [%s%-8ld%s]\r\n"
	"%sI%s) Mob Type:        [%s%c%s]\r\n",

	cyn, OLC_NUM(d), nrm,
	grn, nrm, yel, genders[(int)GET_SEX(mob)], nrm,
	grn, nrm, yel, GET_ALIAS(mob),
	grn, nrm, yel, GET_SDESC(mob),
	grn, nrm, yel, GET_LDESC(mob),
	grn, nrm, yel, GET_DDESC(mob),
	grn, nrm, cyn, GET_LEVEL(mob), nrm,
	grn, nrm, cyn, GET_ALIGNMENT(mob), nrm,
	grn, nrm, cyn, GET_HITROLL(mob), nrm,
	grn, nrm, cyn, GET_DAMROLL(mob), nrm,
	grn, nrm, cyn, GET_NDD(mob), nrm,
	grn, nrm, cyn, GET_SDD(mob), nrm,
	grn, nrm, cyn, GET_HIT(mob), nrm,
	grn, nrm, cyn, GET_MANA(mob), nrm,
	grn, nrm, cyn, GET_MOVE(mob), nrm,
	grn, nrm, cyn, armor, nrm,
	/*. Gold & Exp are longs in my mud, ignore any warnings .*/
	grn, nrm, cyn, GET_EXP(mob), nrm,
	grn, nrm, cyn, GET_GOLD(mob), nrm,
	grn, nrm, cyn, GET_MOB_TYPE(mob), nrm
  );
  send_to_char(buf, d->character);

  sprintbit(MOB_FLAGS(mob), action_bits, buf1);
  sprintbit(AFF_FLAGS(mob), affected_bits, buf2);
  sprintf(buf,
	"%sJ%s) Position  : %s%s\r\n"
	"%sK%s) Default   : %s%s\r\n"
	"%sL%s) Attack    : %s%s\r\n"
	"%sM%s) NPC Flags : %s%s\r\n"
	"%sN%s) AFF Flags : %s%s\r\n"
	"%sO%s) Race      : %s%s\r\n"
        "%sP%s) Height    : %s%d'%d\"\r\n"
        "%sR%s) Weight    : %s%d lbs\r\n"
	"%sS%s) Attributes: %sSt %d, In %d, Wi %d, De %d, Co %d, Ch %d, Wil %d\r\n"
	"%sQ%s) Quit\r\n"
	"Enter choice : ",

	grn, nrm, yel, position_types[(int)GET_POS(mob)],
	grn, nrm, yel, position_types[(int)GET_DEFAULT_POS(mob)],
	grn, nrm, yel, attack_hit_text[GET_ATTACK(mob)].singular,
	grn, nrm, cyn, buf1,
	grn, nrm, cyn, buf2,
	grn, nrm, yel, pc_race_types[(int) GET_RACE(mob)],
        grn, nrm, yel, (GET_HEIGHT(mob) / 12), (GET_HEIGHT(mob) % 12),
        grn, nrm, yel, GET_WEIGHT(mob),
	grn, nrm, yel, GET_STR(mob), GET_INT(mob), GET_WIS(mob), GET_DEX(mob), GET_CON(mob), GET_CHA(mob), GET_WILL(mob),
	grn, nrm
  );
  send_to_char(buf, d->character);

  OLC_MODE(d) = MEDIT_MAIN_MENU;
}


/**************************************************************************
 *                   The GARGANTAUN event handler                         *
 **************************************************************************/
void medit_parse(struct descriptor_data * d, char *arg)
{
   int i, armor = 0;
   void recalc_stats(struct char_data *ch);

   if (OLC_MODE(d) > MEDIT_NUMERICAL_RESPONSE) {
     if (!isdigit(arg[0]) && ((*arg == '-') && (!isdigit(arg[1])))) {
       send_to_char("Field must be numerical, try again : ", d->character);
       return;
     }
   }

   switch (OLC_MODE(d)) {
     case MEDIT_CONFIRM_SAVESTRING:
       /*. Ensure mob has MOB_ISNPC set or things will go pair shaped .*/
       SET_BIT(MOB_FLAGS(OLC_MOB(d)), MOB_ISNPC);
       switch (*arg) {
	 case 'y':
	 case 'Y':
	   /*. Save the mob in memory and to disk  .*/
	   send_to_char("Saving mobile to memory.\r\n", d->character);
	   medit_save_internally(d);
	   sprintf(buf, "OLC: %s edits mob %ld", GET_NAME(d->character),
		   OLC_NUM(d));
	   mudlog(buf, CMP, LVL_GOD, TRUE);
	   cleanup_olc(d, CLEANUP_ALL);
	   return;
	 case 'n':
	 case 'N':
	   cleanup_olc(d, CLEANUP_ALL);
	   return;
	 default:
	   send_to_char("Invalid choice!\r\n", d->character);
	   send_to_char("Do you wish to save the mobile? : ", d->character);
	   return;
       }
       break;

     case MEDIT_MAIN_MENU:
       i = 0;
       switch (*arg) {
	 case 'q':
	 case 'Q':
	   /*. Anything been changed? .*/
	   if (OLC_VAL(d)) {
	     send_to_char("Do you wish to save the changes to the mobile? (y/n) : ", d->character);
	     OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
	   } else
	     cleanup_olc(d, CLEANUP_ALL);
	   return;
	 case '1':
	   OLC_MODE(d) = MEDIT_SEX;
	   medit_disp_sex(d);
	   return;
	 case '2':
	   OLC_MODE(d) = MEDIT_ALIAS;
	   i--;
	   break;
	 case '3':
	   OLC_MODE(d) = MEDIT_S_DESC;
	   i--;
	   break;
	 case '4':
	   OLC_MODE(d) = MEDIT_L_DESC;
	   i--;
	   break;
	 case '5':
	   send_to_char("Enter new mob description:\r\n", d->character);
	   /*. Pass control to modify.c .*/
	   OLC_MODE(d) = MEDIT_D_DESC;
	   d->str = (char **) malloc(sizeof(char *));
	   *(d->str) = NULL;
	   d->max_str = MAX_MOB_DESC;
	   d->mail_to = 0;
	   return;
	 case '6':
	   OLC_MODE(d) = MEDIT_LEVEL;
	   i++;
	   break;
	 case '7':
	   OLC_MODE(d) = MEDIT_ALIGNMENT;
	   i++;
	   break;
	 case '8':
	   OLC_MODE(d) = MEDIT_HITROLL;
	   i++;
	   break;
	 case '9':
	   OLC_MODE(d) = MEDIT_DAMROLL;
	   i++;
	   break;
	 case 'a':
	 case 'A':
	   OLC_MODE(d) = MEDIT_NDD;
	   i++;
	   break;
	 case 'b':
	 case 'B':
	   OLC_MODE(d) = MEDIT_SDD;
	   i++;
	   break;
	 case 'c':
	 case 'C':
	   OLC_MODE(d) = MEDIT_NUM_HP_DICE;
	   i++;
	   break;
	 case 'd':
	 case 'D':
	   OLC_MODE(d) = MEDIT_SIZE_HP_DICE;
	   i++;
	   break;
	 case 'e':
	 case 'E':
	   OLC_MODE(d) = MEDIT_ADD_HP;
	   i++;
	   break;
	 case 'f':
	 case 'F':
	   OLC_MODE(d) = MEDIT_AC;
	   i++;
	   break;
	 case 'g':
	 case 'G':
	   OLC_MODE(d) = MEDIT_EXP;
	   i++;
	   break;
	 case 'h':
	 case 'H':
	   OLC_MODE(d) = MEDIT_GOLD;
	   i++;
	   break;
	 case 'i':
	 case 'I':
	   OLC_MODE(d) = MEDIT_TYPE;
	   i++;
	   send_to_char("\r\nMob Type:\r\n"
			"\t[S]imple\r\n"
			"\t[E]nhanced\r\n"
			"\t[A]utomatic\r\n", d->character);
	   break;
	 case 'j':
	 case 'J':
	   OLC_MODE(d) = MEDIT_POS;
	   medit_disp_positions(d);
	   return;
	 case 'k':
	 case 'K':
	   OLC_MODE(d) = MEDIT_DEFAULT_POS;
	   medit_disp_positions(d);
	   return;
	 case 'l':
	 case 'L':
	   OLC_MODE(d) = MEDIT_ATTACK;
	   medit_disp_attack_types(d);
	   return;
	 case 'm':
	 case 'M':
	   OLC_MODE(d) = MEDIT_NPC_FLAGS;
	   medit_disp_mob_flags(d);
	   return;
	 case 'n':
	 case 'N':
	   OLC_MODE(d) = MEDIT_AFF_FLAGS;
	   medit_disp_aff_flags(d);
	   return;
	 case 'o':
	 case 'O':
	   OLC_MODE(d) = MEDIT_RACE;
	   medit_disp_race(d);
	   return;
         case 'p':
         case 'P':
	   OLC_MODE(d) = MEDIT_HEIGHT;
	   i++;
	   break;
         case 'r':
         case 'R':
	   OLC_MODE(d) = MEDIT_WEIGHT;
	   i++;
	   break;
	 case 's':
	 case 'S':
	   OLC_MODE(d) = MEDIT_ATTRIBUTES;
           medit_disp_attribs(d);
	   send_to_char("Attributes:\r\n"
			"\t[S]trength     %-2d\t\t[D]exterity %-2d\r\n"
			"\t[I]ntelligence %-2d\t\t[W]isdom    %-2d\r\n"
			"\t[C]onstitution %-2d\t\tC[H]arisma  %-2d\r\n"
			"\tWi[L]lpower    %-2d\t\t[Q]uit\r\n\r\nYour Choice? ",
			d->character);
	   return;
	 default:
	   medit_disp_menu(d);
	   return;
       }
       if (i == 1) {
	 send_to_char("\r\nEnter new value : ", d->character);
	 return;
       }
       if (i == -1) {
	 send_to_char("\r\nEnter new text :\r\n> ", d->character);
	 return;
       }
       break;

     case MEDIT_ALIAS:
       if (GET_ALIAS(OLC_MOB(d)))
	 free(GET_ALIAS(OLC_MOB(d)));
       GET_ALIAS(OLC_MOB(d)) = str_dup(arg);
       break;

     case MEDIT_S_DESC:
       if (GET_SDESC(OLC_MOB(d)))
	 free(GET_SDESC(OLC_MOB(d)));
       GET_SDESC(OLC_MOB(d)) = str_dup(arg);
       break;

     case MEDIT_L_DESC:
       if (GET_LDESC(OLC_MOB(d)))
	 free(GET_LDESC(OLC_MOB(d)));
       strcpy(buf, arg);
       strcat(buf, "\r\n");
       GET_LDESC(OLC_MOB(d)) = str_dup(buf);
       break;

     case MEDIT_D_DESC:
       /*. We should never get here .*/
       cleanup_olc(d, CLEANUP_ALL);
       mudlog("SYSERR: OLC: medit_parse(): Reached D_DESC case!",BRF,LVL_GOD,TRUE);
       break;

     case MEDIT_NPC_FLAGS:
       i = atoi(arg);
       if (i == 0)
	 break;
       if (!((i < 0) || (i > NUM_MOB_FLAGS))) {
	 i = 1 << (i - 1);
	 if (IS_SET(MOB_FLAGS(OLC_MOB(d)), i))
	   REMOVE_BIT(MOB_FLAGS(OLC_MOB(d)), i);
	 else
	   SET_BIT(MOB_FLAGS(OLC_MOB(d)), i);
       }
       medit_disp_mob_flags(d);
       return;

     case MEDIT_AFF_FLAGS:
       i = atoi(arg);
       if (i == 0)
	 break;
       if (!((i < 0) || (i > NUM_AFF_FLAGS))) {
	 i = 1 << (i - 1);
	 if (IS_SET(AFF_FLAGS(OLC_MOB(d)), i))
	   REMOVE_BIT(AFF_FLAGS(OLC_MOB(d)), i);
	 else
	   SET_BIT(AFF_FLAGS(OLC_MOB(d)), i);
       }
       medit_disp_aff_flags(d);
       return;

     /*. Numerical responses .*/
     case MEDIT_SEX:
       GET_SEX(OLC_MOB(d)) = MAX(0, MIN(NUM_GENDERS -1, atoi(arg)));
       break;

     case MEDIT_HITROLL:
       GET_HITROLL(OLC_MOB(d)) = MAX(0, MIN(99, atoi(arg)));
       break;

     case MEDIT_DAMROLL:
       GET_DAMROLL(OLC_MOB(d)) = MAX(0, MIN(99, atoi(arg)));
       break;

     case MEDIT_NDD:
       GET_NDD(OLC_MOB(d)) = MAX(0, MIN(100, atoi(arg)));
       break;

     case MEDIT_SDD:
       GET_SDD(OLC_MOB(d)) = MAX(0, MIN(127, atoi(arg)));
       break;

     case MEDIT_NUM_HP_DICE:
       GET_HIT(OLC_MOB(d)) = MAX(0, MIN(100, atoi(arg)));
       break;

     case MEDIT_SIZE_HP_DICE:
       GET_MANA(OLC_MOB(d)) = MAX(0, MIN(1000, atoi(arg)));
       break;

     case MEDIT_ADD_HP:
       GET_MOVE(OLC_MOB(d)) = MAX(0, MIN(30000, atoi(arg)));
       break;

    case MEDIT_AC:
      armor = MAX(0, MIN(99, atoi(arg)));
      for (i = 0; i < ARMOR_LIMIT; i++)
	OLC_MOB(d)->points.armor[i] = armor;
      break;

    case MEDIT_EXP:
      GET_EXP(OLC_MOB(d)) = MAX(0L, atol(arg));
      break;

    case MEDIT_GOLD:
      GET_GOLD(OLC_MOB(d)) = MAX(0L, atol(arg));
      break;

    case MEDIT_TYPE:
      switch (*arg) {
	case 's':
	case 'S':
	  GET_MOB_TYPE(OLC_MOB(d)) = 'S';
	  send_to_char("This mob type saves:\r\n"
		       "NPC/AFF Flags, Sex, Positions\r\n"
		       "Lvl, Hitroll, Damroll, Armor, Dice, Gold, and Exp.\r\n",
		       d->character);
	  break;
	case 'e':
	case 'E':
	  GET_MOB_TYPE(OLC_MOB(d)) = 'E';
	  send_to_char("This mob type saves:\r\n"
		       "NPC/AFF Flags, Sex, Positions\r\n"
		       "Lvl, Hit/Damrolls, Armor, Dice, Gold, Exp.\r\n"
		       "Attack Type, Race, and Attributes.\r\n", d->character);
	  break;
	case 'a':
	case 'A':
	  GET_MOB_TYPE(OLC_MOB(d)) = 'A';
	  send_to_char("This mob type is the easiest and only saves:\r\n"
		       "NPC/AFF Flags, Sex, Positions\r\n"
		       "Level, Race, and Attack Type.\r\n", d->character);
	  break;
	default:
	  send_to_char("That is not a valid mob type!\r\n", d->character);
	  OLC_MODE(d) = MEDIT_TYPE;
	  send_to_char("\r\nMob Type:\r\n"
		       "\t[S]imple\r\n"
		       "\t[E]nhanced\r\n"
		       "\t[A]utomatic\r\n"
		       "\r\nYour Choice? ", d->character);
	  return;
       }
       break;

     case MEDIT_POS:
       GET_POS(OLC_MOB(d)) = MAX(0, MIN(NUM_POSITIONS-1, atoi(arg)));
       break;

     case MEDIT_DEFAULT_POS:
       GET_DEFAULT_POS(OLC_MOB(d)) = MAX(0, MIN(NUM_POSITIONS-1, atoi(arg)));
       break;

     case MEDIT_ATTACK:
       GET_ATTACK(OLC_MOB(d)) = MAX(0, MIN(NUM_ATTACK_TYPES-1, atoi(arg)));
       GET_MOB_TYPE(OLC_MOB(d)) = 'E';
       break;

     case MEDIT_RACE:
       GET_RACE(OLC_MOB(d)) = MAX(0, MIN(TOT_RACES-1, atoi(arg)));
       GET_MOB_TYPE(OLC_MOB(d)) = 'E';
       break;

     case MEDIT_LEVEL:
       GET_LEVEL(OLC_MOB(d)) = MAX(1, MIN(100, atoi(arg)));
       recalc_stats(OLC_MOB(d));
       break;

     case MEDIT_ALIGNMENT:
       GET_ALIGNMENT(OLC_MOB(d)) = MAX(-1000, MIN(1000, atoi(arg)));
       break;

     case MEDIT_HEIGHT:
       i = atoi(arg);
       GET_HEIGHT(OLC_MOB(d)) = MAX(1, i);
       GET_MOB_TYPE(OLC_MOB(d)) = 'E';
       break;

     case MEDIT_WEIGHT:
       i = atoi(arg);
       GET_WEIGHT(OLC_MOB(d)) = MAX((int)(1.5 * GET_HEIGHT(OLC_MOB(d))), i);
       GET_MOB_TYPE(OLC_MOB(d)) = 'E';
       break;

     case MEDIT_ATTRIBUTES:
       switch (*arg) {
	 case 's':
	 case 'S':
	   OLC_MODE(d) = MEDIT_STR;
	   ++i;
	   send_to_char("Enter new value: ", d->character);
	   break;
	 case 'd':
	 case 'D':
	   OLC_MODE(d) = MEDIT_DEX;
	   ++i;
	   send_to_char("Enter new value: ", d->character);
	   break;
	 case 'i':
	 case 'I':
	   OLC_MODE(d) = MEDIT_INT;
	   ++i;
	   send_to_char("Enter new value: ", d->character);
	   break;
	 case 'w':
	 case 'W':
	   OLC_MODE(d) = MEDIT_WIS;
	   ++i;
	   send_to_char("Enter new value: ", d->character);
	   break;
	 case 'c':
	 case 'C':
	   OLC_MODE(d) = MEDIT_CON;
	   ++i;
	   send_to_char("Enter new value: ", d->character);
	   break;
	 case 'h':
	 case 'H':
	   OLC_MODE(d) = MEDIT_CHA;
	   ++i;
	   send_to_char("Enter new value: ", d->character);
	   break;
	 case 'l':
	 case 'L':
	   OLC_MODE(d) = MEDIT_WIL;
	   ++i;
	   send_to_char("Enter new value: ", d->character);
	   break;
	 case 'q':
	 case 'Q':
	 default:
	   OLC_MODE(d) = MEDIT_MAIN_MENU;
	   medit_disp_menu(d);
	   break;
       }
       GET_MOB_TYPE(OLC_MOB(d)) = 'E';
       break;

     case MEDIT_STR:
       GET_STR(OLC_MOB(d)) = MAX(1, MIN(24, atoi(arg)));
       OLC_MODE(d) = MEDIT_ATTRIBUTES;
       if (GET_STR(OLC_MOB(d)) == 18)
	 GET_ADD(OLC_MOB(d)) = number(0, 99);
       return;

     case MEDIT_DEX:
       GET_DEX(OLC_MOB(d)) = MAX(1, MIN(24, atoi(arg)));
       OLC_MODE(d) = MEDIT_ATTRIBUTES;
       return;

     case MEDIT_INT:
       GET_INT(OLC_MOB(d)) = MAX(1, MIN(24, atoi(arg)));
       OLC_MODE(d) = MEDIT_ATTRIBUTES;
       return;

     case MEDIT_WIS:
       GET_DEX(OLC_MOB(d)) = MAX(1, MIN(24, atoi(arg)));
       OLC_MODE(d) = MEDIT_ATTRIBUTES;
       return;

     case MEDIT_CON:
       GET_DEX(OLC_MOB(d)) = MAX(1, MIN(24, atoi(arg)));
       OLC_MODE(d) = MEDIT_ATTRIBUTES;
       return;

     case MEDIT_CHA:
       GET_DEX(OLC_MOB(d)) = MAX(1, MIN(24, atoi(arg)));
       OLC_MODE(d) = MEDIT_ATTRIBUTES;
       return;

     case MEDIT_WIL:
       GET_WILL(OLC_MOB(d)) = MAX(1, MIN(24, atoi(arg)));
       OLC_MODE(d) = MEDIT_ATTRIBUTES;
       return;

     default:
       /*. We should never get here .*/
       cleanup_olc(d, CLEANUP_ALL);
       mudlog("SYSERR: OLC: medit_parse(): Reached default case!",BRF,LVL_GOD,TRUE);
       break;
   }
   /*
    * If we get here, we have probably changed something, and now want to
    *   return to main menu.  Use OLC_VAL as a 'has changed' flag.
    */
   OLC_VAL(d) = 1;
   free(arg);
   *arg = NULL;
   medit_disp_menu(d);
}


/*
 * Creates a new object with some default strings
 */
void oedit_setup_new(struct descriptor_data *d)
{
   CREATE (OLC_OBJ(d), struct obj_data, 1);
   clear_object(OLC_OBJ(d));
   OLC_OBJ(d)->name = str_dup("unfinished object");
   OLC_OBJ(d)->description = str_dup("An unfinished object is lying here.");
   OLC_OBJ(d)->short_description = str_dup("an unfinished object");
   GET_OBJ_WEAR(OLC_OBJ(d)) = ITEM_WEAR_TAKE;
   OLC_VAL(d) = 0;
   oedit_disp_menu(d);
}

/*
 * Copies over an existing object so that it may be edited.
 */
void oedit_setup_existing(struct descriptor_data *d, long real_num)
{
  struct extra_descr_data *thisp, *temp, *temp2;
  struct obj_data *obj;

  /* allocate object */
  CREATE (obj, struct obj_data, 1);
  clear_object (obj);
  *obj = obj_proto[real_num];

  /* copy all strings over */
  if (obj_proto[real_num].name)
    obj->name = str_dup (obj_proto[real_num].name);
  if (obj_proto[real_num].short_description)
    obj->short_description = str_dup (obj_proto[real_num].short_description);
  if (obj_proto[real_num].description)
    obj->description = str_dup (obj_proto[real_num].description);
  if (obj_proto[real_num].action_description)
    obj->action_description = str_dup (obj_proto[real_num].action_description);

  /*. Extra descriptions if necessary .*/
  if (obj_proto[real_num].ex_description) {
    /* temp is for obj being edited */
    CREATE (temp, struct extra_descr_data, 1);
    obj->ex_description = temp;
    for (thisp = obj_proto[real_num].ex_description; thisp; thisp = thisp->next) {
      if (thisp->keyword)
	temp->keyword = str_dup (thisp->keyword);
      if (thisp->description)
	temp->description = str_dup (thisp->description);
      if (thisp->next) {
	CREATE (temp2, struct extra_descr_data, 1);
	temp->next = temp2;
	temp = temp2;
      } else
	temp->next = NULL;
    }
  }

  /*. Attatch new obj to players descriptor .*/
  OLC_OBJ(d) = obj;
  OLC_VAL(d) = 0;
  oedit_disp_menu(d);
}

/*
 * Save the changed object to memory.
 */
void oedit_save_internally(struct descriptor_data *d)
{
  long   i, shop, robj_num, found = FALSE, zone, cmd_no;
  struct extra_descr_data *thisp, *next_one;
  struct obj_data *obj, *swap, *new_obj_proto;
  struct index_data *new_obj_index;
  struct descriptor_data *dsc;

  /* write to internal tables */
  robj_num = real_object(OLC_NUM(d));

  if (robj_num > 0) {
    /* we need to run through each and every object currently in the
     * game to see which ones are pointing to this prototype */

    /* if object is pointing to this prototype, then we need to replace
     * with the new one */
    CREATE(swap, struct obj_data, 1);
    for (obj = object_list; obj; obj = obj->next) {
      if (obj->item_number == robj_num) {
	*swap = *obj;
	*obj = *OLC_OBJ(d);
	/* copy game-time dependent vars over */
	obj->in_room = swap->in_room;
	obj->item_number = robj_num;
	obj->carried_by = swap->carried_by;
	obj->worn_by = swap->worn_by;
	obj->worn_on = swap->worn_on;
	obj->in_obj = swap->in_obj;
	obj->contains = swap->contains;
	obj->next_content = swap->next_content;
	obj->next = swap->next;
      }
    }
    free_obj(swap);
    /* now safe to free old proto and write over */
    if (obj_proto[robj_num].name)
      free(obj_proto[robj_num].name);
    if (obj_proto[robj_num].description)
      free(obj_proto[robj_num].description);
    if (obj_proto[robj_num].short_description)
      free(obj_proto[robj_num].short_description);
    if (obj_proto[robj_num].action_description)
      free(obj_proto[robj_num].action_description);
    if (obj_proto[robj_num].ex_description) {
      for (thisp = obj_proto[robj_num].ex_description;
	   thisp; thisp = next_one) {
	next_one = thisp->next;
	if (thisp->keyword)
	  free(thisp->keyword);
	if (thisp->description)
	  free(thisp->description);
	free(thisp);
      }
    }
    obj_proto[robj_num] = *OLC_OBJ(d);
    obj_proto[robj_num].item_number = robj_num;
  } else {
    /*. It's a new object, we must build new tables to contain it .*/
    CREATE(new_obj_index, struct index_data, top_of_objt + 2);
    CREATE(new_obj_proto, struct obj_data, top_of_objt + 2);
    /* start counting through both tables */
    for (i = 0; i <= top_of_objt + 1; i++) {
      /* if we haven't found it */
      if (!found) {
	/* check if current vnumber is bigger than our vnumber */
	if ((i > top_of_objt) ||
	    (obj_index[i].vnumber > OLC_NUM(d))) {
	  found = TRUE;
	  robj_num = i;
	  OLC_OBJ(d)->item_number = robj_num;
	  new_obj_index[robj_num].vnumber = OLC_NUM(d);
	  new_obj_index[robj_num].number = 0;
	  new_obj_index[robj_num].func = NULL;
	  new_obj_proto[robj_num] = *(OLC_OBJ(d));
	  new_obj_proto[robj_num].in_room = NOWHERE;
	  if (robj_num <= top_of_objt) {
	    new_obj_index[robj_num + 1] = obj_index[robj_num];
	    new_obj_proto[robj_num + 1] = obj_proto[robj_num];
	    new_obj_proto[robj_num + 1].item_number = robj_num + 1;
	  }
	} else {
	  /* just copy from old to new, no num change */
	  new_obj_proto[i] = obj_proto[i];
	  new_obj_index[i] = obj_index[i];
	}
      } else {
	/* we HAVE already found it.. therefore copy to object + 1 */
	new_obj_index[i + 1] = obj_index[i];
	new_obj_proto[i + 1] = obj_proto[i];
	new_obj_proto[i + 1].item_number = i + 1;
      }
    }
    /* free and replace old tables */
    free (obj_proto);
    free (obj_index);
    obj_proto = new_obj_proto;
    obj_index = new_obj_index;
    top_of_objt++;

    /*. Renumber live objects .*/
    for (obj = object_list; obj; obj = obj->next) {
      if (GET_OBJ_RNUM (obj) >= robj_num)
	GET_OBJ_RNUM (obj)++;
    }
    /*. Renumber zone table .*/
    for (zone = 0; zone <= top_of_zone_table; zone++) {
      for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
	switch (ZCMD.command) {
	  case 'P':
	    if (ZCMD.arg3 >= robj_num)
	      ZCMD.arg3++;
	    /*. No break here - drop into next case .*/
	  case 'O':
	  case 'G':
	  case 'E':
	    if(ZCMD.arg1 >= robj_num)
	      ZCMD.arg1++;
	    break;
	  case 'R':
	    if(ZCMD.arg2 >= robj_num)
	      ZCMD.arg2++;
	    break;
	}
      }
    }
    /*. Renumber notice boards */
    for (i = 0; i < NUM_OF_BOARDS; i++) {
      if (BOARD_RNUM(i) >= robj_num)
	BOARD_RNUM(i) = BOARD_RNUM(i) + 1;
    }
    /*. Renumber shop produce .*/
    for (shop = 0; shop < top_shop; shop++) {
      for (i = 0; SHOP_PRODUCT(shop, i) != -1; i++) {
	if (SHOP_PRODUCT(shop, i) >= robj_num)
	  SHOP_PRODUCT(shop, i)++;
      }
    }
    /*. Renumber produce in shops being edited .*/
    for (dsc = descriptor_list; dsc; dsc = dsc->next) {
      if (dsc->connected == CON_SEDIT)
	for (i = 0; S_PRODUCT(OLC_SHOP(dsc), i) != -1; i++) {
	  if (S_PRODUCT(OLC_SHOP(dsc), i) >= robj_num)
	    S_PRODUCT(OLC_SHOP(dsc), i)++;
	}
    }
  }
  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_OBJ);
}

/*
 * Save the object file to disk.
 */
void oedit_save_to_disk(struct descriptor_data *d)
{
  int    zone = zone_table[OLC_ZNUM(d)].number;
  long   counter, counter2, realcounter;
  FILE   *fp;
  char   fname[64], backname[72], extra_buf[60], wear_buf[60];
  struct obj_data *obj;
  struct extra_descr_data *ex_desc;

  sprintf(fname, "%s/%d.obj", OBJ_PREFIX, zone);
  sprintf(backname, "%s/%d.obj.back", OBJ_PREFIX, zone);
  /* Make a backup copy just to be safe */
  if (rename(fname, backname) == 0) {
    sprintf(buf, "Backup Copy saved of %d.obj.", zone);
    mudlog(buf, CMP, LVL_BUILDER, TRUE);
  } else {
     if (!(fp = fopen(fname, "a+"))) {
       sprintf(buf, "ERROR: Could not backup %d.obj, new obj not written to disk!", zone);
       mudlog(buf, CMP, LVL_BUILDER, TRUE);
       return;
     } else if (rename(fname, backname) == 0) {
       sprintf(buf, "Backup saved of %d.mob.", zone);
       mudlog(buf, CMP, LVL_BUILDER, TRUE);
     }
  }
  if (!(fp = fopen(fname, "w+"))) {
    mudlog("SYSERR: OLC: Cannot open objects file!", BRF, LVL_GOD, TRUE);
    return;
  }

  /* start running through all objects in this zone */
  for (counter = zone_table[OLC_ZNUM(d)].number * 100;
       counter <= zone_table[OLC_ZNUM(d)].top;
       counter++) {
    /* write object to disk */
    realcounter = real_object(counter);
    if (realcounter >= 0) {
      obj = (obj_proto + realcounter);
      if (obj->action_description) {
	strcpy(buf1, obj->action_description);
	strip_string(buf1);
      } else
	*buf1 = 0;

      sprintbits(GET_OBJ_EXTRA(obj), extra_buf);
      if (extra_buf[0] == '\0')
	strcpy(extra_buf, "0");
      sprintbits(GET_OBJ_WEAR(obj), wear_buf);
      if (wear_buf[0] == '\0')
	strcpy(wear_buf, "0");

      fprintf(fp,
	"#%ld\n"
	"%s~\n"
	"%s~\n"
	"%s~\n"
	"%s~\n"
	"%d %s %s\n"
	"%ld %ld %ld %ld %ld %ld %ld %ld %ld\n"
	"%d %d %d\n",

	GET_OBJ_VNUM(obj),
	obj->name,
	obj->short_description,
	obj->description,
	buf1,
	GET_OBJ_TYPE(obj),
	extra_buf,
	wear_buf,
	GET_OBJ_VAL(obj, 0),
	GET_OBJ_VAL(obj, 1),
	GET_OBJ_VAL(obj, 2),
	GET_OBJ_VAL(obj, 3),
	GET_OBJ_VAL(obj, 4),
	GET_OBJ_VAL(obj, 5),
	GET_OBJ_VAL(obj, 6),
	GET_OBJ_MATERIAL(obj),
	GET_OBJ_SIZE(obj),               /* condition is not printed to file */
	GET_OBJ_WEIGHT(obj),
	GET_OBJ_COST(obj),
	GET_OBJ_RENT(obj)
      );

      /* Do we have extra descriptions? */
      if (obj->ex_description) {
	/*. Yep, save them too .*/
	for (ex_desc = obj->ex_description; ex_desc; ex_desc = ex_desc->next) {
	  /*. Sanity check to prevent nasty protection faults .*/
	  if (!*ex_desc->keyword || !*ex_desc->description) {
	    mudlog("SYSERR: OLC: oedit_save_to_disk: Corrupt ex_desc!", BRF, LVL_GOD, TRUE);
	    continue;
	  }
	  strcpy(buf1, ex_desc->description);
	  strip_string(buf1);
	  fprintf(fp,   "E\n"
			"%s~\n"
			"%s~\n",
			ex_desc->keyword,
			buf1
	  );
	}
      }

      /* Do we have affects? */
      for (counter2 = 0; counter2 < MAX_OBJ_AFFECT; counter2++) {
	if (obj->affected[counter2].modifier)
	  fprintf(fp,   "A\n"
			"%d %d\n",
			obj->affected[counter2].location,
			obj->affected[counter2].modifier);
      }
    }
  }
  /* write final line, close */
  fprintf(fp, "$~\n");
  fclose(fp);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_OBJ);
}


/**************************************************************************
 Menu functions
 **************************************************************************/

/* For container flags */
void oedit_disp_container_flags_menu(struct descriptor_data * d)
{
  get_char_cols(d->character);
  sprintbit(GET_OBJ_VAL(OLC_OBJ(d), 1), container_bits, buf1);
  send_to_char("[H[J", d->character);
  sprintf(buf,
	"%s1%s) CLOSEABLE\r\n"
	"%s2%s) PICKPROOF\r\n"
	"%s3%s) CLOSED\r\n"
	"%s4%s) LOCKED\r\n"
	"Container flags: %s%s%s\r\n"
	"Enter flag, 0 to quit : ",
	grn, nrm, grn, nrm, grn, nrm, grn, nrm, cyn, buf1, nrm
  );
  send_to_char(buf, d->character);
}

/* For extra descriptions */
void oedit_disp_extradesc_menu(struct descriptor_data * d)
{
  struct extra_descr_data *extra_desc = OLC_DESC(d);

  if (!extra_desc->next)
    strcpy(buf1, "<Not set>\r\n");
  else
    strcpy(buf1, "Set.");

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  sprintf(buf,
	"Extra desc menu\r\n"
	"%s1%s) Keyword: %s%s\r\n"
	"%s2%s) Description:\r\n%s%s\r\n"
	"%s3%s) Goto next description: %s\r\n"
	"%s0%s) Quit\r\n"
	"Enter choice : ",

	grn, nrm, yel, extra_desc->keyword ? extra_desc->keyword : "<NONE>",
	grn, nrm, yel, extra_desc->description ? extra_desc->description : "<NONE>",
	grn, nrm, buf1,
	grn, nrm
  );
  send_to_char(buf, d->character);
  OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
}

/* Ask for *which* apply to edit */
void oedit_disp_prompt_apply_menu(struct descriptor_data * d)
{
  int counter;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
    if (OLC_OBJ(d)->affected[counter].modifier) {
      sprinttype(OLC_OBJ(d)->affected[counter].location, apply_types, buf2);
      sprintf(buf, " %s%d%s) %+d to %s\r\n",
		grn, counter + 1, nrm,
		OLC_OBJ(d)->affected[counter].modifier, buf2
      );
      send_to_char(buf, d->character);
    } else {
      sprintf(buf, " %s%d%s) None.\r\n", grn, counter + 1, nrm);
      send_to_char(buf, d->character);
    }
  }
  send_to_char("\r\nEnter affection to modify (0 to quit) : ", d->character);
  OLC_MODE(d) = OEDIT_PROMPT_APPLY;
}

/*. Ask for liquid type .*/
void oedit_liquid_type(struct descriptor_data * d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 0; counter < NUM_LIQ_TYPES; counter++) {
    sprintf(buf, " %s%2d%s) %s%-20.20s ",
		grn, counter, nrm, yel,
		drinks[counter]);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n%sEnter drink type : ", nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = OEDIT_VALUE_3;
}

/* The actual apply to set */
void oedit_disp_apply_menu(struct descriptor_data * d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 0; counter < NUM_APPLIES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter, nrm, apply_types[counter]);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter apply type (0 is no apply) : ", d->character);
  OLC_MODE(d) = OEDIT_APPLY;
}


/* weapon type */
void oedit_disp_weapon_menu(struct descriptor_data * d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 0; counter < NUM_ATTACK_TYPES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter, nrm, attack_hit_text[counter].singular);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter weapon type : ", d->character);
}

/* trap type */
void oedit_disp_traptype_menu(struct descriptor_data * d)
{
  int counter;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 1; counter < NUM_TRAP_TYPES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter, nrm, trap_dam_type[counter]);
    if (!(counter % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter trap type : ", d->character);
}

/* trap trigger type */
void oedit_disp_traptrig_menu(struct descriptor_data * d)
{
  int counter;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 1; counter < NUM_TRIGGERS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter, nrm, trap_triggers[counter]);
    if (!(counter % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter trigger type : ", d->character);
}

/* spell type */
void oedit_disp_spells_menu(struct descriptor_data * d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = MAX_SPHERES; counter < NUM_SPELLS; counter++) {
    if (strcmp(spells[counter], "!UNUSED!")) {
      sprintf(buf, "%s%3d%s) %s%-20.20s ",
	    grn, counter, nrm, yel, spells[counter]);
      if (!(++columns % 3))
	strcat(buf, "\r\n");
      send_to_char(buf, d->character);
    }
  }
  sprintf(buf, "\r\n%sEnter spell choice (0 for none) : ", nrm);
  send_to_char(buf, d->character);
}

/* material type */
void oedit_disp_materials(struct descriptor_data * d)
{
  extern struct material_type material_list[];
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J"
	       " Num  Material       Sturdiness\r\n", d->character);
  for (counter = 0; counter < MAX_MATERIALS; counter++) {
    sprintf(buf, "%s%2d%s) %s%-20.20s %s%-4d ",
	    grn, counter, nrm, yel, material_list[counter].name,
	    cyn, material_list[counter].sturdiness);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n%sEnter material type : ", nrm);
  send_to_char(buf, d->character);
}

void oedit_disp_sizes(struct descriptor_data * d)
{
  extern char   *item_size_bits[];
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 0; counter < NUM_SIZES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter + 1, nrm, item_size_bits[counter]);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintbit(GET_OBJ_SIZE(OLC_OBJ(d)), item_size_bits, buf1);
  sprintf(buf,  "\r\nSize flags: %s%s%s\r\n"
		"Enter size flag, 0 to quit : ",
		cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

/* object value 1 */
void oedit_disp_val1_menu(struct descriptor_data * d)
{
  OLC_MODE(d) = OEDIT_VALUE_1;

  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_LIGHT:
      /* values 0 and 1 are unused.. jump to 2 */
      oedit_disp_val3_menu(d);
      break;
    case ITEM_SCROLL:
    case ITEM_WAND:
    case ITEM_STAFF:
    case ITEM_POTION:
      send_to_char("Spell level : ", d->character);
      break;
    case ITEM_FIREWEAPON:
    case ITEM_MISSILE:
      send_to_char("Ammo Type : ", d->character);
      break;
    case ITEM_WEAPON:
      /* this seems to be a circleism.. not part of normal diku? */
      send_to_char("Modifier to Hitroll : ", d->character);
      break;
    case ITEM_ARMOR:
      send_to_char("Apply to AC : ", d->character);
      break;
    case ITEM_CONTAINER:
      send_to_char("Max weight to contain : ", d->character);
      break;
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
      send_to_char("Max drink units : ", d->character);
      break;
    case ITEM_FOOD:
      send_to_char("Hours to fill stomach : ", d->character);
      break;
    case ITEM_MONEY:
      send_to_char("Number of gold coins : ", d->character);
      break;
    case ITEM_NOTE:
      /* this is supposed to be language, but it's unused */
      break;
    case ITEM_TRAP:
      /* trigger type */
      oedit_disp_traptrig_menu(d);
      break;
    default:
      oedit_disp_menu(d);
  }
}

/* object value 2 */
void oedit_disp_val2_menu(struct descriptor_data * d)
{
  OLC_MODE(d) = OEDIT_VALUE_2;

  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      oedit_disp_spells_menu(d);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      send_to_char("Max number of charges : ", d->character);
      break;
    case ITEM_MISSILE:
      send_to_char("Number of Ammunition in obj : ", d->character);
      break;
    case ITEM_FIREWEAPON:
      GET_OBJ_VAL(OLC_OBJ(d), 1) = 0;
      oedit_disp_val3_menu(d);
      break;
    case ITEM_WEAPON:
      send_to_char("Number of damage dice : ", d->character);
      break;
    case ITEM_FOOD:
      /* values 2 and 3 are unused, jump to 4. how odd */
      oedit_disp_val4_menu(d);
      break;
    case ITEM_CONTAINER:
      /* these are flags, needs a bit of special handling */
      oedit_disp_container_flags_menu(d);
      break;
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
      send_to_char("Initial drink units : ", d->character);
      break;
    case ITEM_TRAP:
      oedit_disp_traptype_menu(d);
      break;
    default:
      oedit_disp_menu(d);
  }
}

/* object value 3 */
void oedit_disp_val3_menu(struct descriptor_data * d)
{
  OLC_MODE(d) = OEDIT_VALUE_3;

  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_LIGHT:
      send_to_char("Number of hours (0 = burnt, -1 is infinite) : ", d->character);
      break;
    case ITEM_SCROLL:
    case ITEM_POTION:
      oedit_disp_spells_menu(d);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      send_to_char("Number of charges remaining : ", d->character);
      break;
    case ITEM_MISSILE:
      send_to_char("Enter number of damage dice : ", d->character);
      break;
    case ITEM_FIREWEAPON:
      send_to_char("Enter max ammo weapon can hold : ", d->character);
      break;
    case ITEM_WEAPON:
      send_to_char("Size of damage dice : ", d->character);
      break;
    case ITEM_TRAP:
      if (GET_OBJ_VAL(OLC_OBJ(d), 1) == TRAP_DAM_TELEPORT)
	send_to_char("Destination room : ", d->character);
      else if (GET_OBJ_VAL(OLC_OBJ(d), 1) == TRAP_DAM_SPELL)
	oedit_disp_spells_menu(d);
      else
	send_to_char("Number of damage dice : ", d->character);
      break;
    case ITEM_CONTAINER:
      send_to_char("Vnum of key to open container (-1 for no key) : ", d->character);
      break;
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
      oedit_liquid_type(d);
      break;
    default:
      oedit_disp_menu(d);
  }
}

/* object value 4 */
void oedit_disp_val4_menu(struct descriptor_data * d)
{
  OLC_MODE(d) = OEDIT_VALUE_4;

  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_WAND:
    case ITEM_STAFF:
      oedit_disp_spells_menu(d);
      break;
    case ITEM_MISSILE:
      send_to_char("Enter size of damage dice : ", d->character);
      break;
    case ITEM_WEAPON:
      oedit_disp_weapon_menu(d);
      break;
    case ITEM_TRAP:
      if (GET_OBJ_VAL(OLC_OBJ(d), 1) == TRAP_DAM_TELEPORT)
	oedit_disp_menu(d);
      else if (GET_OBJ_VAL(OLC_OBJ(d), 1) == TRAP_DAM_SPELL)
	send_to_char("Spell level : ", d->character);
      else
	send_to_char("Size of damage dice : ", d->character);
      break;
    case ITEM_CONTAINER: /* Quiver */
      send_to_char("Vnum of missile(s) it holds : ", d->character);
      break;
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
    case ITEM_FOOD:
      send_to_char("Poisoned (0 = not poison) : ", d->character);
      break;
    default:
      oedit_disp_menu(d);
  }
}

/* object type */
void oedit_disp_type_menu(struct descriptor_data * d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 0; counter < NUM_ITEM_TYPES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter, nrm, item_types[counter]);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter object type : ", d->character);
}

/* object extra flags */
void oedit_disp_extra_menu(struct descriptor_data * d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 0; counter < NUM_EXTRA_BITS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter + 1, nrm, extra_bits[counter]);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintbit(GET_OBJ_EXTRA(OLC_OBJ(d)), extra_bits, buf1);
  sprintf(buf,  "\r\nObject flags: %s%s%s\r\n"
		"Enter object extra flag (0 to quit) : ",
		cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

/* object wear flags */
void oedit_disp_wear_menu(struct descriptor_data * d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (counter = 0; counter < NUM_WEAR_BITS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s ",
	    grn, counter + 1, nrm, wear_bits[counter]);
    if (!(++columns % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintbit(GET_OBJ_WEAR(OLC_OBJ(d)), wear_bits, buf1);
  sprintf(buf,  "\r\nWear flags: %s%s%s\r\n"
		"Enter wear flag, 0 to quit : ",
		cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

/* display main menu */
void oedit_disp_menu(struct descriptor_data * d)
{
  struct obj_data *obj;

  extern struct material_type material_list[];
  extern char   *item_size_bits[];

  obj = OLC_OBJ(d);
  get_char_cols(d->character);

  /*. Build buffers for first part of menu .*/
  sprinttype(GET_OBJ_TYPE(obj), item_types, buf1);
  sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf2);

  /*. Build first hallf of menu .*/
  sprintf(buf, "[H[J"
	"-- Item number : [%s%ld%s]\r\n"
	"%s1%s) Namelist : %s%s\r\n"
	"%s2%s) S-Desc   : %s%s\r\n"
	"%s3%s) L-Desc   :-\r\n%s%s\r\n"
	"%s4%s) A-Desc   :-\r\n%s%s"
	"%s5%s) Type        : %s%s\r\n"
	"%s6%s) Extra flags : %s%s\r\n",

	cyn, OLC_NUM(d), nrm,
	grn, nrm, yel, obj->name,
	grn, nrm, yel, obj->short_description,
	grn, nrm, yel, obj->description,
	grn, nrm, yel, obj->action_description ?  obj->action_description : "<not set>\r\n",
	grn, nrm, cyn, buf1,
	grn, nrm, cyn, buf2
  );
  /*. Send first half .*/
  send_to_char(buf, d->character);

  /*. Build second half of menu .*/
  sprintbit(GET_OBJ_WEAR(obj), wear_bits, buf1);
  sprintbit(GET_OBJ_SIZE(obj), item_size_bits, buf2);
  sprintf(buf,
	"%s7%s) Wear flags  : %s%s\r\n"
	"%s8%s) Weight      : %s%d\r\n"
	"%s9%s) Cost        : %s%d\r\n"
	"%sA%s) Cost/Day    : %s%d\r\n"
	"%sB%s) Timer       : %s%d\r\n"
	"%sC%s) Material    : %s%s\r\n"
	"%sD%s) Size        : %s%s\r\n"
	"%sE%s) Values      : %s%ld %ld %ld %ld\r\n"
	"%sF%s) Applies menu\r\n"
	"%sG%s) Extra descriptions menu\r\n"
	"%sQ%s) Quit\r\n"
	"Enter choice : ",

	grn, nrm, cyn, buf1,
	grn, nrm, cyn, GET_OBJ_WEIGHT(obj),
	grn, nrm, cyn, GET_OBJ_COST(obj),
	grn, nrm, cyn, GET_OBJ_RENT(obj),
	grn, nrm, cyn, GET_OBJ_TIMER(obj),
	grn, nrm, cyn, material_list[GET_OBJ_MATERIAL(obj)].name,
	grn, nrm, cyn, buf2,        /* Item size */
	grn, nrm, cyn, GET_OBJ_VAL(obj, 0),
			GET_OBJ_VAL(obj, 1),
			GET_OBJ_VAL(obj, 2),
			GET_OBJ_VAL(obj, 3),
	grn, nrm, grn, nrm, grn, nrm
  );
  send_to_char(buf, d->character);
  OLC_MODE(d) = OEDIT_MAIN_MENU;
}

/***************************************************************************
 main loop (of sorts).. basically interpreter throws all input to here
 ***************************************************************************/


void oedit_parse(struct descriptor_data * d, char *arg)
{
   long number, max_val = 128000, min_val = -128000;

   switch (OLC_MODE(d)) {
     case OEDIT_CONFIRM_SAVESTRING:
       switch (*arg) {
	 case 'y':
	 case 'Y':
	   send_to_char("Saving object to memory.\r\n", d->character);
	   oedit_save_internally(d);
	   sprintf(buf, "OLC: %s edits obj %ld", GET_NAME(d->character), OLC_NUM(d));
	   mudlog(buf, CMP, LVL_GOD, TRUE);
	   cleanup_olc(d, CLEANUP_STRUCTS);
	   return;
	 case 'n':
	 case 'N':
	   /*. Cleanup all .*/
	   cleanup_olc(d, CLEANUP_ALL);
	   return;
	 default:
	   send_to_char("Invalid choice!\r\n", d->character);
	   send_to_char("Do you wish to save this object internally?\r\n", d->character);
	   return;
       }
       break;

     case OEDIT_MAIN_MENU:
       /* throw us out to whichever edit mode based on user input */
       switch (*arg) {
	 case 'q':
	 case 'Q':
	   if (OLC_VAL(d)) {
	     /*. Something has been modified .*/
	     send_to_char("Do you wish to save this object internally? : ", d->character);
	     OLC_MODE(d) = OEDIT_CONFIRM_SAVESTRING;
	   } else
	     cleanup_olc(d, CLEANUP_ALL);
	   return;
	 case '1':
	   send_to_char("Enter namelist : ", d->character);
	   OLC_MODE(d) = OEDIT_EDIT_NAMELIST;
	   break;
	 case '2':
	   send_to_char("Enter short desc : ", d->character);
	   OLC_MODE(d) = OEDIT_SHORTDESC;
	   break;
	 case '3':
	   send_to_char("Enter long desc :-\r\n| ", d->character);
	   OLC_MODE(d) = OEDIT_LONGDESC;
	   break;
	 case '4':
	   /* let's go out to modify.c */
	   send_to_char("Enter action desc :-\r\n", d->character);
	   OLC_MODE(d) = OEDIT_ACTDESC;
	   d->str = (char **) malloc(sizeof(char *));
	   *(d->str) = NULL;
	   d->max_str = MAX_OBJ_DESC;
	   d->mail_to = 0;
	   break;
	 case '5':
	   oedit_disp_type_menu(d);
	   OLC_MODE(d) = OEDIT_TYPE;
	   break;
	 case '6':
	   oedit_disp_extra_menu(d);
	   OLC_MODE(d) = OEDIT_EXTRAS;
	   break;
	 case '7':
	   oedit_disp_wear_menu(d);
	   OLC_MODE(d) = OEDIT_WEAR;
	   break;
	 case '8':
	   send_to_char("Enter weight : ", d->character);
	   OLC_MODE(d) = OEDIT_WEIGHT;
	   break;
	 case '9':
	   send_to_char("Enter cost : ", d->character);
	   OLC_MODE(d) = OEDIT_COST;
	   break;
	 case 'a':
	 case 'A':
	   send_to_char("Enter cost per day : ", d->character);
	   OLC_MODE(d) = OEDIT_COSTPERDAY;
	   break;
	 case 'b':
	 case 'B':
	   send_to_char("Enter timer : ", d->character);
	   OLC_MODE(d) = OEDIT_TIMER;
	   break;
	 /* new TD stuff starts here */
	 case 'c':
	 case 'C':
	   oedit_disp_materials(d);
	   OLC_MODE(d) = OEDIT_MATERIAL;
	   break;
	 case 'd':
	 case 'D':
	   oedit_disp_sizes(d);
	   OLC_MODE(d) = OEDIT_SIZE;
	   break;
	 case 'e':
	 case 'E':
	   oedit_disp_val1_menu(d);
	   break;
	 case 'f':
	 case 'F':
	   oedit_disp_prompt_apply_menu(d);
	   break;
	 case 'g':
	 case 'G':
	   /* if extra desc doesn't exist . */
	   if (!OLC_OBJ(d)->ex_description) {
	     CREATE(OLC_OBJ(d)->ex_description, struct extra_descr_data, 1);
	     OLC_OBJ(d)->ex_description->next = NULL;
	   }
	   OLC_DESC(d) = OLC_OBJ(d)->ex_description;
	   oedit_disp_extradesc_menu(d);
	   break;
	 default:
	   oedit_disp_menu(d);
	   break;
       }
       return;                     /* end of OEDIT_MAIN_MENU */

     case OEDIT_EDIT_NAMELIST:
       if (OLC_OBJ(d)->name)
	 free(OLC_OBJ(d)->name);
       OLC_OBJ(d)->name = str_dup(arg);
       break;

     case OEDIT_SHORTDESC:
       if (OLC_OBJ(d)->short_description)
	 free(OLC_OBJ(d)->short_description);
       OLC_OBJ(d)->short_description = str_dup(arg);
       break;

     case OEDIT_LONGDESC:
       if (OLC_OBJ(d)->description)
	 free(OLC_OBJ(d)->description);
       OLC_OBJ(d)->description = str_dup(arg);
       break;

     case OEDIT_TYPE:
       number = atoi(arg);
       if ((number < 1) || (number >= NUM_ITEM_TYPES)) {
	 send_to_char("Invalid choice, try again : ", d->character);
	 return;
       } else {
	 GET_OBJ_TYPE(OLC_OBJ(d)) = number;
       }
       break;

     case OEDIT_EXTRAS:
       number = atoi(arg);
       if ((number < 0) || (number > NUM_EXTRA_BITS)) {
	 oedit_disp_extra_menu(d);
	 return;
       } else {
	 /* if 0, quit */
	 if (number == 0)
	   break;
	 else {
	   /* if already set.. remove */
	   if (IS_SET(GET_OBJ_EXTRA(OLC_OBJ(d)), 1 << (number - 1)))
	     REMOVE_BIT(GET_OBJ_EXTRA(OLC_OBJ(d)), 1 << (number - 1));
	   else /* set */
	     SET_BIT(GET_OBJ_EXTRA(OLC_OBJ(d)), 1 << (number - 1));
	   oedit_disp_extra_menu(d);
	   return;
	 }
       }
       break;

     case OEDIT_WEAR:
       number = atoi(arg);
       if ((number < 0) || (number > NUM_WEAR_BITS)) {
	 send_to_char("That's not a valid choice!\r\n", d->character);
	 oedit_disp_wear_menu(d);
	 return;
       } else {
	 /* if 0, quit */
	 if (number == 0)
	   break;
	 else {
	   /* if already set.. remove */
	   if (IS_SET(GET_OBJ_WEAR(OLC_OBJ(d)), 1 << (number - 1)))
	     REMOVE_BIT(GET_OBJ_WEAR(OLC_OBJ(d)), 1 << (number - 1));
	   else
	     SET_BIT(GET_OBJ_WEAR(OLC_OBJ(d)), 1 << (number - 1));
	   oedit_disp_wear_menu(d);
	   return;
	 }
       }
       break;

     case OEDIT_WEIGHT:
       number = atoi(arg);
       GET_OBJ_WEIGHT(OLC_OBJ(d)) = number;
       break;

     case OEDIT_COST:
       number = atoi(arg);
       GET_OBJ_COST(OLC_OBJ(d)) = number;
       break;

    case OEDIT_COSTPERDAY:
      number = atoi(arg);
      GET_OBJ_RENT(OLC_OBJ(d)) = number;
      break;

    case OEDIT_TIMER:
      number = atoi(arg);
      GET_OBJ_TIMER(OLC_OBJ(d)) = number;
      break;

    case OEDIT_MATERIAL:
      number = atoi(arg);
      if (number < 0 || number > MAX_MATERIALS) {
	oedit_disp_materials(d);
	send_to_char("That's not a valid material type!\r\n", d->character);
	send_to_char("Enter material type : ", d->character);
	return;
      } else
	GET_OBJ_MATERIAL(OLC_OBJ(d)) = number;
      break;

    case OEDIT_SIZE:
       number = atoi(arg);
       if ((number < 0) || (number > NUM_SIZES)) {
	 send_to_char("That's not a valid choice!\r\n", d->character);
	 oedit_disp_sizes(d);
	 return;
       } else {
	 /* if 0, quit */
	 if (number == 0)
	   break;
	 else {
	   /* if already set.. remove */
	   if (IS_SET(GET_OBJ_SIZE(OLC_OBJ(d)), 1 << (number - 1)))
	     REMOVE_BIT(GET_OBJ_SIZE(OLC_OBJ(d)), 1 << (number - 1));
	   else
	     SET_BIT(GET_OBJ_SIZE(OLC_OBJ(d)), 1 << (number - 1));
	   oedit_disp_sizes(d);
	   return;
	 }
       }
       break;

     case OEDIT_VALUE_1:
       number = atoi(arg);
       switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
	case ITEM_FIREWEAPON:
	case ITEM_MISSILE:
          /* Ammo type */
	  min_val = 0;
	  max_val = NUM_AMMO_TYPES-1;
	  GET_OBJ_VAL(OLC_OBJ(d), 0) = MAX(min_val, MIN(number, max_val));
	  break;
	case ITEM_TRAP:
	  min_val = 1;
	  max_val = NUM_TRIGGERS-1;
	  GET_OBJ_VAL(OLC_OBJ(d), 0) = MAX(min_val, MIN(number, max_val));
	  break;
	default:
	  GET_OBJ_VAL(OLC_OBJ(d), 0) = number;
	  break;
       }
       /* proceed to menu 2 */
       oedit_disp_val2_menu(d);
       return;

     case OEDIT_VALUE_2:
       /* here, I do need to check for outofrange values */
       number = atoi(arg);
       switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
	 case ITEM_MISSILE:  /* ammo amount */
	   min_val = 1;
	   max_val = 50;
	   GET_OBJ_VAL(OLC_OBJ(d), 1) = MAX(min_val, MIN(number, max_val));
	   oedit_disp_val3_menu(d);
	   break;
	 case ITEM_TRAP:
	   min_val = 1;
	   max_val = NUM_TRAP_TYPES-1;
	   GET_OBJ_VAL(OLC_OBJ(d), 1) = MAX(min_val, MIN(number, max_val));
	   oedit_disp_val3_menu(d);
	   break;
	 case ITEM_SCROLL:
	 case ITEM_POTION:
           if (number == -1 || number == 0) {
             GET_OBJ_VAL(OLC_OBJ(d), 1) = -1;
             GET_OBJ_VAL(OLC_OBJ(d), 2) = -1;
             GET_OBJ_VAL(OLC_OBJ(d), 3) = -1;
             oedit_disp_menu(d);
             return;
           } else if (number < 51 || number >= NUM_SPELLS)
	     oedit_disp_val2_menu(d);
	   else {
	     GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
	     oedit_disp_val3_menu(d);
	   }
	   break;
	 case ITEM_CONTAINER:
	   /* needs some special handling since we are dealing with flag values
	    * here */
	   number = atoi(arg);
	   if (number < 0 || number > 4)
	     oedit_disp_container_flags_menu(d);
	   else {
	     /* if 0, quit */
	     if (number != 0) {
	       number = 1 << (number - 1);
	       if (IS_SET(GET_OBJ_VAL(OLC_OBJ(d), 1), number))
		 REMOVE_BIT(GET_OBJ_VAL(OLC_OBJ(d), 1), number);
	       else
		 SET_BIT(GET_OBJ_VAL(OLC_OBJ(d), 1), number);
	       oedit_disp_val2_menu(d);
	     } else
	       oedit_disp_val3_menu(d);
	   }
	   break;
	 default:
	   GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
	   oedit_disp_val3_menu(d);
       }
       return;

    case OEDIT_VALUE_3:
      number = atoi(arg);
      /*. Quick'n'easy error checking .*/
      switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
	case ITEM_TRAP:
	  if (GET_OBJ_VAL(OLC_OBJ(d), 1) == TRAP_DAM_SPELL) {
	    min_val = 51;
	    max_val = NUM_SPELLS -1;
	  } else if (GET_OBJ_VAL(OLC_OBJ(d), 1) == TRAP_DAM_TELEPORT) {
	    if (real_room(number) <= 0)
	      number = -1;
	    else {
	      min_val = number;
	      max_val = number;
	    }
	  } else {
	    min_val = 1;
	    max_val = 50;
	  }
	  break;
	case ITEM_SCROLL:
	case ITEM_POTION:
          if (number == -1 || number == 0) {
            GET_OBJ_VAL(OLC_OBJ(d), 2) = -1;
            GET_OBJ_VAL(OLC_OBJ(d), 3) = -1;
            oedit_disp_menu(d);
	    return;
	  } else if (number < 51 || number >= NUM_SPELLS)
	    oedit_disp_val3_menu(d);
	  else {
	    min_val = 51;
	    max_val = NUM_SPELLS -1;
	  }
	  break;
        case ITEM_FIREWEAPON: /* max ammo amt */
          min_val = 1;
          max_val = 128;
          break;
	case ITEM_MISSILE: /* num dice */
	case ITEM_WEAPON:
	  min_val = 1;
	  max_val = 50;
          break;
	case ITEM_WAND:
	case ITEM_STAFF:
	  min_val = 0;
	  max_val = 20;
	  break;
	case ITEM_DRINKCON:
	case ITEM_FOUNTAIN:
	  min_val = 0;
	  max_val = NUM_LIQ_TYPES -1;
	  break;
	default:
	  min_val = -32000;
	  max_val = 32000;
      }
      GET_OBJ_VAL(OLC_OBJ(d), 2) = MAX(min_val, MIN(number, max_val));
      oedit_disp_val4_menu(d);
      return;

    case OEDIT_VALUE_4:
      number = atoi(arg);
      switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
	case ITEM_TRAP:
	  min_val = 1;
	  max_val = 255;
	  break;
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_WAND:
	case ITEM_STAFF:
	  min_val = -1;
	  max_val = NUM_SPELLS -1;
	  break;
	case ITEM_MISSILE: /* size dice */
	  min_val = 2;
	  max_val = 128;
	  break;
        case ITEM_FIREWEAPON:
          min_val = -1;
          max_val = -1;
          break;
	case ITEM_WEAPON:
	  min_val = 0;
	  max_val = NUM_ATTACK_TYPES -1;
	  break;
	default:
	  min_val = -31999;
	  max_val =  31999;
	  break;
       }
       GET_OBJ_VAL(OLC_OBJ(d), 3) = MAX(min_val, MIN(number, max_val));
       break;

     case OEDIT_PROMPT_APPLY:
       number = atoi(arg);
       if (number == 0)
	 break;
       else if (number < 0 || number > MAX_OBJ_AFFECT) {
	 oedit_disp_prompt_apply_menu(d);
	 return;
       }
       OLC_VAL(d) = number - 1;
       OLC_MODE(d) = OEDIT_APPLY;
       oedit_disp_apply_menu(d);
       return;

     case OEDIT_APPLY:
       number = atoi(arg);
       if (number == 0) {
	 OLC_OBJ(d)->affected[OLC_VAL(d)].location = 0;
	 OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = 0;
	 oedit_disp_prompt_apply_menu(d);
       } else if (number < 0 || number >= NUM_APPLIES)
	 oedit_disp_apply_menu(d);
       else {
	 OLC_OBJ(d)->affected[OLC_VAL(d)].location = number;
	 send_to_char("Modifier : ", d->character);
	 OLC_MODE(d) = OEDIT_APPLYMOD;
       }
       return;

     case OEDIT_APPLYMOD:
       number = atoi(arg);
       OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = number;
       oedit_disp_prompt_apply_menu(d);
       return;

     case OEDIT_EXTRADESC_KEY:
       if (OLC_DESC(d)->keyword)
	 free(OLC_DESC(d)->keyword);
       OLC_DESC(d)->keyword = str_dup(arg);
       oedit_disp_extradesc_menu(d);
       return;

     case OEDIT_EXTRADESC_MENU:
       number = atoi(arg);
       switch (number) {
	 case 0:
	 { /* if something got left out */
	   if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) {
	     struct extra_descr_data **tmp_desc;

	     if (OLC_DESC(d)->keyword)
	       free(OLC_DESC(d)->keyword);
	     if (OLC_DESC(d)->description)
	       free(OLC_DESC(d)->description);
	     /*. Clean up pointers .*/
	     for (tmp_desc = &(OLC_OBJ(d)->ex_description); *tmp_desc;
		  tmp_desc = &((*tmp_desc)->next)) {
	       if (*tmp_desc == OLC_DESC(d)) {
		 *tmp_desc = NULL;
		 break;
	       }
	     }
	     free(OLC_DESC(d));
	   }
	 }
	 break;

	 case 1:
	   OLC_MODE(d) = OEDIT_EXTRADESC_KEY;
	   send_to_char("Enter keywords, separated by spaces :-\r\n| ", d->character);
	   return;

	 case 2:
	   OLC_MODE(d) = OEDIT_EXTRADESC_DESCRIPTION;
	   send_to_char("Enter description :-\r\n", d->character);
	   /* send out to modify.c */
	   d->str = (char **) malloc(sizeof(char *));
	   *(d->str) = NULL;
	   d->max_str = MAX_OBJ_DESC;
	   d->mail_to = 0;
	   OLC_VAL(d) = 1;
	   return;

	 case 3:
	   /*. Only go to the next descr if this one is finished .*/
	   if (OLC_DESC(d)->keyword && OLC_DESC(d)->description) {
	     struct extra_descr_data *new_extra;

	     if (OLC_DESC(d)->next)
	       OLC_DESC(d) = OLC_DESC(d)->next;
	     else {
	       /* make new extra, attach at end */
	       CREATE(new_extra, struct extra_descr_data, 1);

	       OLC_DESC(d)->next = new_extra;
	       OLC_DESC(d) = OLC_DESC(d)->next;
	     }
	   }
	   /*. No break - drop into default case .*/
	 default:
	   oedit_disp_extradesc_menu(d);
	   return;
       }
       break;

     default:
       mudlog("SYSERR: OLC: Reached default case in oedit_parse()!", BRF, LVL_GOD, TRUE);
       break;
   }

   /*. If we get here, we have changed something .*/
   OLC_VAL(d) = 1; /*. Has changed flag .*/
   oedit_disp_menu(d);
}

/*
 * Creates a new shop struct with some default values.
 */
void sedit_setup_new(struct descriptor_data *d)
{
  struct shop_data *shop;

  /*. Alloc some shop shaped space .*/
  CREATE(shop, struct shop_data, 1);

  /*. Some default values .*/
  S_KEEPER(shop)     = -1;
  S_CLOSE1(shop)     = 28;
  S_BUYPROFIT(shop)  = 1.0;
  S_SELLPROFIT(shop) = 1.0;
  /*. Some default strings .*/
  S_NOITEM1(shop) = str_dup("%s Sorry, I don't stock that item.");
  S_NOITEM2(shop) = str_dup("%s You don't seem to have that.");
  S_NOCASH1(shop) = str_dup("%s I can't afford that!");
  S_NOCASH2(shop) = str_dup("%s You are too poor!");
  S_NOBUY(shop)   = str_dup("%s I don't trade in such items.");
  S_BUY(shop)     = str_dup("%s That'll be %d coins, thanks.");
  S_SELL(shop)    = str_dup("%s I'll give you %d coins for that.");
  /*. Init the lists .*/
  CREATE(S_PRODUCTS(shop), long, 1);
  S_PRODUCT(shop, 0) = -1;
  CREATE(S_ROOMS(shop), long, 1);
  S_ROOM(shop, 0) = -1;
  CREATE(S_NAMELISTS(shop), struct shop_buy_data, 1);
  S_BUYTYPE(shop, 0) = -1;

  OLC_SHOP(d) = shop;
  sedit_disp_menu(d);
}

/*
 * Copy over a shop to new space so it can be edited.
 */
void sedit_setup_existing(struct descriptor_data *d, long rshop_num)
{
  /*. Alloc some shop shaped space .*/
  CREATE(OLC_SHOP(d), struct shop_data, 1);
  copy_shop(OLC_SHOP(d), shop_index + rshop_num);
  sedit_disp_menu(d);
}


/*
 * Copy the internals of the shop.
 */
void copy_shop(struct shop_data *tshop, struct shop_data *fshop)
{
  /*. Copy basic info over .*/
  S_NUM(tshop)          = S_NUM(fshop);
  S_KEEPER(tshop)       = S_KEEPER(fshop);
  S_OPEN1(tshop)        = S_OPEN1(fshop);
  S_CLOSE1(tshop)       = S_CLOSE1(fshop);
  S_OPEN2(tshop)        = S_OPEN2(fshop);
  S_CLOSE2(tshop)       = S_CLOSE2(fshop);
  S_BANK(tshop)         = S_BANK(fshop);
  S_BROKE_TEMPER(tshop) = S_BROKE_TEMPER(fshop);
  S_BITVECTOR(tshop)    = S_BITVECTOR(fshop);
  S_NOTRADE(tshop)      = S_NOTRADE(fshop);
  S_SORT(tshop)         = S_SORT(fshop);
  S_BUYPROFIT(tshop)    = S_BUYPROFIT(fshop);
  S_SELLPROFIT(tshop)   = S_SELLPROFIT(fshop);
  S_FUNC(tshop)         = S_FUNC(fshop);

  /*. Copy lists over .*/
  copy_list(&(S_ROOMS(tshop)), S_ROOMS(fshop));
  copy_list(&(S_PRODUCTS(tshop)), S_PRODUCTS(fshop));
  copy_type_list(&(tshop->type), fshop->type);

  /*. Copy notification strings over .*/
  free_shop_strings(tshop);
  S_NOITEM1(tshop) = str_dup(S_NOITEM1(fshop));
  S_NOITEM2(tshop) = str_dup(S_NOITEM2(fshop));
  S_NOCASH1(tshop) = str_dup(S_NOCASH1(fshop));
  S_NOCASH2(tshop) = str_dup(S_NOCASH2(fshop));
  S_NOBUY(tshop)   = str_dup(S_NOBUY(fshop));
  S_BUY(tshop)     = str_dup(S_BUY(fshop));
  S_SELL(tshop)    = str_dup(S_SELL(fshop));
}

/*
 * Copy a -1 terminated integer array list.
 */
void copy_list(long **tlist, long *flist)
{
  int num_items, i;

  if (*tlist)
    free(*tlist);

  /*. Count number of entries .*/
  for (i = 0; flist[i] != -1; i++)
    /* The loop does all the work */;
  num_items = i + 1;

  /*. Make space for entries .*/
  CREATE(*tlist, long, num_items);

  /*. Copy entries over .*/
  i = 0;
  do {
    (*tlist)[i] = flist[i];
    i++;
  } while(i < num_items);
}


/*
 * Copy a -1 terminated (in the type field) shop_buy_data
 *   array list.
 */
void copy_type_list(struct shop_buy_data **tlist, struct shop_buy_data *flist)
{
  int num_items, i;

  if (*tlist)
    free_type_list(tlist);

  /*. Count number of entries .*/
  for (i=0; BUY_TYPE(flist[i]) != -1; i++)
    /* The loop does all the work */;
  num_items = i + 1;

  /*. Make space for entries .*/
  CREATE(*tlist, struct shop_buy_data, num_items);

  /*. Copy entries over .*/
  i = 0;
  do {
    (*tlist)[i].type = flist[i].type;
    if (BUY_WORD(flist[i]))
      BUY_WORD((*tlist)[i]) = str_dup(BUY_WORD(flist[i]));
    i++;
  } while(i < num_items);
}


/*
 * Removes an item from the buy list.
 */
void sedit_remove_from_type_list(struct shop_buy_data **list, int num)
{
  int i, num_items;
  struct shop_buy_data *nlist;

  /*. Count number of entries .*/
  for (i=0; (*list)[i].type != -1; i++)
    /* The loop does all the work */;

  if (num >= i || num < 0)
    return;
  num_items = i;

  CREATE(nlist, struct shop_buy_data, num_items);

  for (i = 0; i < num_items; i++) {
    if (i < num)
      nlist[i] = (*list)[i];
    else
      nlist[i] = (*list)[i+1];
  }
  free(BUY_WORD((*list)[num]));
  free(*list);
  *list = nlist;
}


/*
 * Adds an item to the buy list.
 */
void sedit_add_to_type_list(struct shop_buy_data **list, struct shop_buy_data *newp) 
{
  int i, num_items;
  struct shop_buy_data *nlist;

  /*. Count number of entries .*/
  for (i=0; (*list)[i].type != -1; i++)
    /* The loop does all the work */;
  num_items = i;

  /*. make a new list and slot in the new entry .*/
  CREATE(nlist, struct shop_buy_data, num_items + 2);
  for (i = 0; i < num_items; i++)
    nlist[i] = (*list)[i];
  nlist[num_items] = *newp;
  nlist[num_items+1].type = -1;

  /*. Out with the old, in with the new .*/
  free(*list);
  *list = nlist;
}


/*
 * Adds an item to an long list.
 */
void sedit_add_to_int_list(long **list, long newp)
{
  int  i, num_items;
  long *nlist;

  /*. Count number of entries .*/
  for (i=0; (*list)[i] != -1; i++)
    /* The loop does all the work */;
  num_items = i;

  /*. make a new list and slot in the new entry .*/
  CREATE(nlist, long, num_items + 2);
  for (i = 0; i < num_items; i++)
    nlist[i] = (*list)[i];
  nlist[num_items] = newp;
  nlist[num_items+1] = -1;

  /*. Out with the old, in with the new .*/
  free(*list);
  *list = nlist;
}

/*-------------------------------------------------------------------*/

void sedit_remove_from_int_list(long **list, long num)
{
  int  i, num_items;
  long *nlist;

  /*. Count number of entries .*/
  for (i=0; (*list)[i] != -1; i++)
    /* The loop does all the work */;

  if (num >= i || num < 0)
    return;
  num_items = i;

  CREATE(nlist, long, num_items);

  for (i = 0; i < num_items; i++) {
    if (i < num)
      nlist[i] = (*list)[i];
    else
      nlist[i] = (*list)[i+1];
  }
  free(*list);
  *list = nlist;
}

/*
 * Free all the notice character strings in a shop structure.
 */
void free_shop_strings(struct shop_data *shop)
{
  if (S_NOITEM1(shop)) {
    free(S_NOITEM1(shop));
    S_NOITEM1(shop) = NULL;
  }
  if (S_NOITEM2(shop)) {
    free(S_NOITEM2(shop));
    S_NOITEM2(shop) = NULL;
  }
  if (S_NOCASH1(shop)) {
    free(S_NOCASH1(shop));
    S_NOCASH1(shop) = NULL;
  }
  if (S_NOCASH2(shop)) {
    free(S_NOCASH2(shop));
    S_NOCASH2(shop) = NULL;
  }
  if (S_NOBUY(shop)) {
    free(S_NOBUY(shop));
    S_NOBUY(shop) = NULL;
  }
  if (S_BUY(shop)) {
    free(S_BUY(shop));
    S_BUY(shop) = NULL;
  }
  if (S_SELL(shop)) {
    free(S_SELL(shop));
    S_SELL(shop) = NULL;
  }
}

/*
 * Free a type list and all the strings it contains.
 */
void free_type_list(struct shop_buy_data **list)
{
  int i;

  for (i = 0; (*list)[i].type != -1; i++) {
    if (BUY_WORD((*list)[i]))
      free(BUY_WORD((*list)[i]));
  }
  free(*list);
  *list = NULL;
}

/*
 * Free up the whole shop structure and it's content.
 */
void free_shop(struct shop_data *shop)
{
  free_shop_strings(shop);
  free_type_list(&(S_NAMELISTS(shop)));
  free(S_ROOMS(shop));
  free(S_PRODUCTS(shop));
  free(shop);
}


/*
 * Get the real number of a shop from it's vnum.
 */
long real_shop(long vshop_num)
{
  long rshop_num;

  for (rshop_num = 0; rshop_num < top_shop; rshop_num++) {
    if (SHOP_NUM(rshop_num) == vshop_num)
      return rshop_num;
  }
  return -1;
}

/*
 * Generic string modifyer for shop keeper messages.
 */
void sedit_modify_string(char **str, char *newp)
{
   char *pointer;

   /*. Check the '%s' is present, if not, add it .*/
   if (*newp != '%') {
     strcpy(buf, "%s ");
     strcat(buf, newp);
     pointer = buf;
   } else
     pointer = newp;

   if (*str)
     free(*str);
   *str = str_dup(pointer);
}

/*
 * Save the shop to memory.
 */
void sedit_save_internally(struct descriptor_data *d)
{
  int    found = 0;
  long   rshop;
  struct shop_data *shop;
  struct shop_data *new_index;

  rshop = real_shop(OLC_NUM(d));
  shop  = OLC_SHOP(d);

  if (rshop > -1) {
    /*. The shop already exists, just update it.*/
    copy_shop((shop_index + rshop), shop);
  } else {
    /*. Doesn't exist - hafta insert it .*/
    CREATE(new_index, struct shop_data, top_shop + 1);
    for (rshop = 0; rshop <= top_shop; rshop++) {
      if (!found) {
	/*. Is this the place ?.*/
	if (rshop == top_shop || SHOP_NUM(rshop) > OLC_NUM(d)) {
	  /*. Yep, stick it in here .*/
	  found = 1;
	  S_NUM(shop) = OLC_NUM(d);
	  copy_shop(&(new_index[rshop]), shop);
	  /*. Move the entry that used to go here up a place .*/
	  if (rshop < top_shop)
	    new_index[rshop+1] = shop_index[rshop];
	} else {
	  /*. This isn't the place, copy over info .*/
	  new_index[rshop] = shop_index[rshop];
	}
      } else {
	/*. Shop's already inserted, copy rest over .*/
	new_index[rshop+1] = shop_index[rshop];
      }
    }
    /*. Switch index in .*/
    free(shop_index);
    shop_index = new_index;
    top_shop++;
  }
  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_SHOP);
}


/*
 * Save shop-file to disk.
 */
void sedit_save_to_disk(struct descriptor_data *d)
{
  long   i, j, rshop, top;
  int    zone;
  FILE   *shop_file;
  char   fname[64], backname[72];
  struct shop_data *shop;

  zone = zone_table[OLC_ZNUM(d)].number;
  top  = zone_table[OLC_ZNUM(d)].top;

  sprintf(fname, "%s/%i.shp", SHP_PREFIX, zone);
  sprintf(backname, "%s/%d.shp.back", SHP_PREFIX, zone);
  /* Make  a  backup copy just in case */
  if (rename(fname, backname) == 0) {
     sprintf(buf, "Backup Copy saved of %d.shp.", zone);
     mudlog(buf, CMP, LVL_BUILDER, TRUE);
  } else {
     if (!(shop_file = fopen(fname, "a+"))) {
       sprintf(buf, "ERROR: Could not backup %d.shp, new shp not written to disk!", zone);
       mudlog(buf, CMP, LVL_BUILDER, TRUE);
       return;
     } else if (rename(fname, backname) == 0) {
       sprintf(buf, "Backup Copy saved of %d.shp.", zone);
       mudlog(buf, CMP, LVL_BUILDER, TRUE);
     }
  }
  if (!(shop_file = fopen(fname, "w"))) {
    mudlog("SYSERR: OLC: Cannot open shop file!", BRF, LVL_GOD, TRUE);
    return;
  }
  if (fprintf(shop_file, "CircleMUD v3.0 Shop File~\n") < 0) {
    mudlog("SYSERR: OLC: Cannot write to shop file!", BRF, LVL_GOD, TRUE);
    return;
  }

  /*. Search database for shops in this zone .*/
  for (i = (zone*100); i <= top; i++) {
    rshop = real_shop(i);
    if (rshop != -1) {
      fprintf(shop_file, "#%ld~\n", i);
      shop = shop_index + rshop;

      /*. Save products .*/
      for (j = 0; S_PRODUCT(shop, j) != -1; j++)
	fprintf(shop_file, "%ld\n", obj_index[S_PRODUCT(shop, j)].vnumber);

      /*. Save rates .*/
      fprintf(shop_file, "-1\n%1.2f\n%1.2f\n",
	      S_BUYPROFIT(shop), S_SELLPROFIT(shop));

      /*. Save buy types and namelists .*/
      j = -1;
      do {
	j++;
	fprintf(shop_file, "%d%s\n",
	  S_BUYTYPE(shop, j),
	  S_BUYWORD(shop, j) ? S_BUYWORD(shop, j) : "");
      } while(S_BUYTYPE(shop, j) != -1);

      /*. Save messages'n'stuff .*/
      fprintf(shop_file,
	"%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n"
	"%d\n%d\n%ld\n%d\n",
	S_NOITEM1(shop),
	S_NOITEM2(shop),
	S_NOBUY(shop),
	S_NOCASH1(shop),
	S_NOCASH2(shop),
	S_BUY(shop),
	S_SELL(shop),
	S_BROKE_TEMPER(shop),
	S_BITVECTOR(shop),
	mob_index[S_KEEPER(shop)].vnumber,
	S_NOTRADE(shop)
      );

      /*. Save rooms .*/
      j = -1;
      do {
	j++;
	fprintf(shop_file, "%ld\n", S_ROOM(shop, j));
      } while(S_ROOM(shop, j) != -1);

      /*. Save open/closing times.*/
      fprintf(shop_file, "%d\n%d\n%d\n%d\n",
	S_OPEN1(shop),
	S_CLOSE1(shop),
	S_OPEN2(shop),
	S_CLOSE2(shop)
      );
    }
  }
  fprintf(shop_file, "$~\n");
  fclose(shop_file);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_SHOP);
}


/**************************************************************************
 Menu functions
 **************************************************************************/

void sedit_products_menu(struct descriptor_data *d)
{
  struct shop_data *shop;
  long   i;

  shop = OLC_SHOP(d);
  get_char_cols(d->character);

  send_to_char("[H[J##     VNUM     Product\r\n", d->character);
  for (i = 0; S_PRODUCT(shop, i) != -1; i++) {
    sprintf(buf,
	"%2ld - [%s%ld%s] - %s%s%s\r\n",
	i,
	cyn, obj_index[S_PRODUCT(shop, i)].vnumber, nrm,
	yel, obj_proto[S_PRODUCT(shop, i)].short_description, nrm
    );
    send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n"
	"%sA%s) Add a new product.\r\n"
	"%sD%s) Delete a product.\r\n"
	"%sQ%s) Quit\r\n"
	"Enter choice : ",
	grn, nrm, grn, nrm, grn, nrm);
  send_to_char(buf, d->character);

  OLC_MODE(d) = SEDIT_PRODUCTS_MENU;
}



void sedit_compact_rooms_menu(struct descriptor_data *d)
{
  struct shop_data *shop;
  int    count = 0;
  long   i;

  shop = OLC_SHOP(d);
  get_char_cols(d->character);

  send_to_char("[H[J", d->character);
  for (i = 0; S_ROOM(shop, i) != -1; i++) {
    sprintf(buf,
	"%2ld - [%s%ld%s]  | ",
	i, cyn, S_ROOM(shop, i), nrm
    );
    if (!(++count % 5))
      strcpy((buf + strlen(buf) -3), "\r\n");
    send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n"
	"%sA%s) Add a new room.\r\n"
	"%sD%s) Delete a room.\r\n"
	"%sQ%s) Quit\r\n"
	"Enter choice : ",
	grn, nrm, grn, nrm, grn, nrm);
  send_to_char(buf, d->character);

  OLC_MODE(d) = SEDIT_ROOMS_MENU;
}



void sedit_rooms_menu(struct descriptor_data *d)
{
  struct shop_data *shop;
  long   i;

  shop = OLC_SHOP(d);
  get_char_cols(d->character);

  send_to_char("[H[J##     VNUM     Room\r\n\r\n", d->character);
  for (i = 0; S_ROOM(shop, i) != -1; i++) {
    sprintf(buf,
	"%2ld - [%s%ld%s] - %s%s%s\r\n",
	i, cyn, S_ROOM(shop, i), nrm,
	yel, world[real_room(S_ROOM(shop, i))].name, nrm
    );
    send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n"
	"%sA%s) Add a new room.\r\n"
	"%sD%s) Delete a room.\r\n"
	"%sC%s) Compact Display.\r\n"
	"%sQ%s) Quit\r\n"
	"Enter choice : ",
	grn, nrm, grn, nrm, grn, nrm, grn, nrm);
  send_to_char(buf, d->character);

  OLC_MODE(d) = SEDIT_ROOMS_MENU;
}


void sedit_namelist_menu(struct descriptor_data *d)
{
  struct shop_data *shop;
  int i;

  shop = OLC_SHOP(d);
  get_char_cols(d->character);

  send_to_char("[H[J##         Type   Namelist\r\n\r\n", d->character);
  for (i = 0; S_BUYTYPE(shop, i) != -1; i++) {
    sprintf(buf,
	"%2d - %s%10s%s - %s%s%s\r\n",
	i, cyn, item_types[S_BUYTYPE(shop, i)], nrm,
	yel, S_BUYWORD(shop, i) ? S_BUYWORD(shop, i) : "<None>", nrm
    );
    send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n"
	"%sA%s) Add a new entry.\r\n"
	"%sD%s) Delete an entry.\r\n"
	"%sQ%s) Quit\r\n"
	"Enter choice : ",
	grn, nrm, grn, nrm, grn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = SEDIT_NAMELIST_MENU;
}


void sedit_shop_flags_menu(struct descriptor_data *d)
{
  int i, count =0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for(i = 0; i < NUM_SHOP_FLAGS; i++)
  { sprintf(buf,
	"%s%2d%s) %-20.20s   ",
	grn, i+1, nrm, shop_bits[i] ? shop_bits[i] : "Unused"
    );
    if (!(++count % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintbit(S_BITVECTOR(OLC_SHOP(d)), shop_bits, buf1);
  sprintf(buf,
	"\r\nCurrent Shop Flags : %s%s%s\r\n"
	"Enter choice : ",
	cyn, buf1, nrm
  );
  send_to_char(buf, d->character);
  OLC_MODE(d) = SEDIT_SHOP_FLAGS;
}


void sedit_no_trade_menu(struct descriptor_data *d)
{
  int i, count =0;

  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_TRADERS; i++) {
    sprintf(buf,
	"%s%2d%s) %-20.20s   ",
	grn, i+1, nrm, trade_letters[i] ? trade_letters[i] : "Unused"
    );
    if (!(++count % 2))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintbit(S_NOTRADE(OLC_SHOP(d)), trade_letters, buf1);
  sprintf(buf,
	"\r\nCurrently won't trade with: %s%s%s\r\n"
	"Enter choice : ",
	cyn, buf1, nrm
  );
  send_to_char(buf, d->character);
  OLC_MODE(d) = SEDIT_NOTRADE;
}


void sedit_types_menu(struct descriptor_data *d)
{
  struct shop_data *shop;
  int i, count = 0;

  shop = OLC_SHOP(d);
  get_char_cols(d->character);

  send_to_char("[H[J", d->character);
  for (i = 0;  i < NUM_ITEM_TYPES; i++) {
    sprintf(buf,
	"%s%2d%s) %s%-20s%s  ",
	grn, i, nrm,
	cyn, item_types[i] ? item_types[i] : "Unused", nrm
    );
    if(!(++count % 3))
      strcat(buf, "\r\n");
    send_to_char(buf, d->character);
  }
  sprintf(buf, "%sEnter choice : ", nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = SEDIT_TYPE_MENU;
}


/*
 *  Display main menu.
 */
void sedit_disp_menu(struct descriptor_data *d)
{
  struct shop_data *shop;

  shop = OLC_SHOP(d);
  get_char_cols(d->character);

  sprintbit(S_NOTRADE(shop), trade_letters, buf1);
  sprintbit(S_BITVECTOR(shop), shop_bits, buf2);
  sprintf(buf, "[H[J"
	"-- Shop Number : [%s%ld%s]\r\n"
	"%s0%s) Keeper      : [%s%ld%s] %s%s\r\n"
	"%s1%s) Open 1      : %s%4d%s          %s2%s) Close 1     : %s%4d\r\n"
	"%s3%s) Open 2      : %s%4d%s          %s4%s) Close 2     : %s%4d\r\n"
	"%s5%s) Sell rate   : %s%1.2f%s          %s6%s) Buy rate    : %s%1.2f\r\n"
	"%s7%s) Keeper no item : %s%s\r\n"
	"%s8%s) Player no item : %s%s\r\n"
	"%s9%s) Keeper no cash : %s%s\r\n"
	"%sA%s) Player no cash : %s%s\r\n"
	"%sB%s) Keeper no buy  : %s%s\r\n"
	"%sC%s) Buy sucess     : %s%s\r\n"
	"%sD%s) Sell sucess    : %s%s\r\n"
	"%sE%s) No Trade With  : %s%s\r\n"
	"%sF%s) Shop flags     : %s%s\r\n"
	"%sR%s) Rooms Menu\r\n"
	"%sP%s) Products Menu\r\n"
	"%sT%s) Accept Types Menu\r\n"
	"%sQ%s) Quit\r\n"
	"Enter Choice : ",

	cyn, OLC_NUM(d), nrm,
	grn, nrm, cyn, S_KEEPER(shop) == -1 ?
		       -1 : mob_index[S_KEEPER(shop)].vnumber, nrm,
		  yel, S_KEEPER(shop) == -1 ?
		       "None" : mob_proto[S_KEEPER(shop)].player.short_descr,
	grn, nrm, cyn, S_OPEN1(shop), nrm,
	grn, nrm, cyn, S_CLOSE1(shop),
	grn, nrm, cyn, S_OPEN2(shop), nrm,
	grn, nrm, cyn, S_CLOSE2(shop),
	grn, nrm, cyn, S_BUYPROFIT(shop), nrm,
	grn, nrm, cyn, S_SELLPROFIT(shop),
	grn, nrm, yel, S_NOITEM1(shop),
	grn, nrm, yel, S_NOITEM2(shop),
	grn, nrm, yel, S_NOCASH1(shop),
	grn, nrm, yel, S_NOCASH2(shop),
	grn, nrm, yel, S_NOBUY(shop),
	grn, nrm, yel, S_BUY(shop),
	grn, nrm, yel, S_SELL(shop),
	grn, nrm, cyn, buf1,
	grn, nrm, cyn, buf2,
	grn, nrm, grn, nrm, grn, nrm, grn, nrm
  );
  send_to_char(buf, d->character);

  OLC_MODE(d) = SEDIT_MAIN_MENU;
}

/**************************************************************************
 *                   The GARGANTUAN event handler                         *
 **************************************************************************/

void sedit_parse(struct descriptor_data * d, char *arg)
{
  long i;

   if (OLC_MODE(d) > SEDIT_NUMERICAL_RESPONSE) {
     if (!isdigit(arg[0]) && ((*arg == '-') && (!isdigit(arg[1])))) {
       send_to_char("Field must be numerical, try again : ", d->character);
       return;
     }
   }

   switch (OLC_MODE(d)) {
     case SEDIT_CONFIRM_SAVESTRING:
       switch (*arg) {
	 case 'y':
	 case 'Y':
	   send_to_char("Saving shop to memory.\r\n", d->character);
	   sedit_save_internally(d);
	   sprintf(buf, "OLC: %s edits shop %ld", GET_NAME(d->character),
		   OLC_NUM(d));
	   mudlog(buf, CMP, LVL_GOD, TRUE);
	   cleanup_olc(d, CLEANUP_STRUCTS);
	   return;
	 case 'n':
	 case 'N':
	   cleanup_olc(d, CLEANUP_ALL);
	   return;
	 default:
	   send_to_char("Invalid choice!\r\n", d->character);
	   send_to_char("Do you wish to save the shop? : ", d->character);
	   return;
       }
       break;

     case SEDIT_MAIN_MENU:
       i = 0;
       switch (*arg) {
	 case 'q':
	 case 'Q':
	   /*. Anything been changed? .*/
	   if (OLC_VAL(d)) {
	     send_to_char("Do you wish to save the changes to the shop? (y/n) : ", d->character);
	     OLC_MODE(d) = SEDIT_CONFIRM_SAVESTRING;
	   } else
	     cleanup_olc(d, CLEANUP_ALL);
	   return;
	 case '0':
	   OLC_MODE(d) = SEDIT_KEEPER;
	   send_to_char("Enter vnumber number of shop keeper : ", d->character);
	   return;
	 case '1':
	   OLC_MODE(d) = SEDIT_OPEN1;
	   i++;
	   break;
	 case '2':
	   OLC_MODE(d) = SEDIT_CLOSE1;
	   i++;
	   break;
	 case '3':
	   OLC_MODE(d) = SEDIT_OPEN2;
	   i++;
	   break;
	 case '4':
	   OLC_MODE(d) = SEDIT_CLOSE2;
	   i++;
	   break;
	 case '5':
	   OLC_MODE(d) = SEDIT_BUY_PROFIT;
	   i++;
	   break;
	 case '6':
	   OLC_MODE(d) = SEDIT_SELL_PROFIT;
	   i++;
	   break;
	 case '7':
	   OLC_MODE(d) = SEDIT_NOITEM1;
	   i--;
	   break;
	 case '8':
	   OLC_MODE(d) = SEDIT_NOITEM2;
	   i--;
	   break;
	 case '9':
	   OLC_MODE(d) = SEDIT_NOCASH1;
	   i--;
	   break;
	 case 'a':
	 case 'A':
	   OLC_MODE(d) = SEDIT_NOCASH2;
	   i--;
	   break;
	 case 'b':
	 case 'B':
	   OLC_MODE(d) = SEDIT_NOBUY;
	   i--;
	   break;
	 case 'c':
	 case 'C':
	   OLC_MODE(d) = SEDIT_BUY;
	   i--;
	   break;
	 case 'd':
	 case 'D':
	   OLC_MODE(d) = SEDIT_SELL;
	   i--;
	   break;
	 case 'e':
	 case 'E':
	   sedit_no_trade_menu(d);
	   return;
	 case 'f':
	 case 'F':
	   sedit_shop_flags_menu(d);
	   return;
	 case 'r':
	 case 'R':
	   sedit_rooms_menu(d);
	   return;
	 case 'p':
	 case 'P':
	   sedit_products_menu(d);
	   return;
	 case 't':
	 case 'T':
	   sedit_namelist_menu(d);
	   return;
	 default:
	   sedit_disp_menu(d);
	   return;
       }

       if (i == 1) {
	 send_to_char("\r\nEnter new value : ", d->character);
	 return;
       }
       if (i == -1) {
	 send_to_char("\r\nEnter new text :\r\n| ", d->character);
	 return;
       }
       break;

     case SEDIT_NAMELIST_MENU:
       switch (*arg) {
	 case 'a':
	 case 'A':
	   sedit_types_menu(d);
	   return;
	 case 'd':
	 case 'D':
	   send_to_char("\r\nDelete which entry? : ", d->character);
	   OLC_MODE(d) = SEDIT_DELETE_TYPE;
	   return;
	 case 'q':
	 case 'Q':
	   break;
       }
       break;

     case SEDIT_PRODUCTS_MENU:
       switch (*arg) {
	 case 'a':
	 case 'A':
	   send_to_char("\r\nEnter new product vnumber number : ", d->character);
	   OLC_MODE(d) = SEDIT_NEW_PRODUCT;
	   return;
	 case 'd':
	 case 'D':
	   send_to_char("\r\nDelete which product? : ", d->character);
	   OLC_MODE(d) = SEDIT_DELETE_PRODUCT;
	   return;
	 case 'q':
	 case 'Q':
	   break;
       }
       break;

     case SEDIT_ROOMS_MENU:
       switch (*arg) {
	 case 'a':
	 case 'A':
	   send_to_char("\r\nEnter new room vnumber number : ", d->character);
	   OLC_MODE(d) = SEDIT_NEW_ROOM;
	   return;
	 case 'c':
	 case 'C':
	   sedit_compact_rooms_menu(d);
	   return;
	 case 'd':
	 case 'D':
	   send_to_char("\r\nDelete which room? : ", d->character);
	   OLC_MODE(d) = SEDIT_DELETE_ROOM;
	   return;
	 case 'q':
	 case 'Q':
	   break;
       }
       break;

     /*. String edits .*/
     case SEDIT_NOITEM1:
       sedit_modify_string(&S_NOITEM1(OLC_SHOP(d)), arg);
       break;
     case SEDIT_NOITEM2:
       sedit_modify_string(&S_NOITEM2(OLC_SHOP(d)), arg);
       break;
     case SEDIT_NOCASH1:
       sedit_modify_string(&S_NOCASH1(OLC_SHOP(d)), arg);
       break;
     case SEDIT_NOCASH2:
       sedit_modify_string(&S_NOCASH2(OLC_SHOP(d)), arg);
       break;
     case SEDIT_NOBUY:
       sedit_modify_string(&S_NOBUY(OLC_SHOP(d)), arg);
       break;
     case SEDIT_BUY:
       sedit_modify_string(&S_BUY(OLC_SHOP(d)), arg);
       break;
     case SEDIT_SELL:
       sedit_modify_string(&S_SELL(OLC_SHOP(d)), arg);
       break;
     case SEDIT_NAMELIST:
       { struct shop_buy_data new_entry;
	 BUY_TYPE(new_entry) = OLC_VAL(d);
	 if (*arg)
	   BUY_WORD(new_entry) = str_dup(arg);
	 else
	   BUY_WORD(new_entry) = NULL;
	 sedit_add_to_type_list(&(S_NAMELISTS(OLC_SHOP(d))), &new_entry);
       }
       sedit_namelist_menu(d);
       return;

     /*. Numerical responses .*/
     case SEDIT_KEEPER:
       i = real_mobile(atoi(arg));
       if (i < 0) {
	 send_to_char("That mobile does not exist, try again : ", d->character);
	 return;
       }
       S_KEEPER(OLC_SHOP(d)) = i;
       /*. Fiddle with special procs .*/
       S_FUNC(OLC_SHOP(d)) = mob_index[i].func;
       mob_index[i].func = shop_keeper;
       break;

     case SEDIT_OPEN1:
       S_OPEN1(OLC_SHOP(d)) = MAX(0, MIN(28, atoi(arg)));
       break;
     case SEDIT_OPEN2:
       S_OPEN2(OLC_SHOP(d)) = MAX(0, MIN(28, atoi(arg)));
       break;
     case SEDIT_CLOSE1:
       S_CLOSE1(OLC_SHOP(d)) = MAX(0, MIN(28, atoi(arg)));
       break;
     case SEDIT_CLOSE2:
       S_CLOSE2(OLC_SHOP(d)) = MAX(0, MIN(28, atoi(arg)));
       break;

     case SEDIT_BUY_PROFIT:
       sscanf(arg, "%f", &S_BUYPROFIT(OLC_SHOP(d)));
       break;
     case SEDIT_SELL_PROFIT:
       sscanf(arg, "%f", &S_SELLPROFIT(OLC_SHOP(d)));
       break;

     case SEDIT_TYPE_MENU:
       OLC_VAL(d) = MAX(0, MIN(NUM_ITEM_TYPES - 1, atoi(arg)));
       send_to_char("Enter namelist (return for none) :-\r\n| ", d->character);
       OLC_MODE(d) = SEDIT_NAMELIST;
       return;
     case SEDIT_DELETE_TYPE:
       sedit_remove_from_type_list(&(S_NAMELISTS(OLC_SHOP(d))), atoi(arg));
       sedit_namelist_menu(d);
       return;
     case SEDIT_NEW_PRODUCT:
       i = real_object(atol(arg));
       if (i < 0) {
	 send_to_char("That object does not exist, try again : ", d->character);
	 return;
       }
       sedit_add_to_int_list(&(S_PRODUCTS(OLC_SHOP(d))), i);
       sedit_products_menu(d);
       return;
     case SEDIT_DELETE_PRODUCT:
       sedit_remove_from_int_list(&(S_PRODUCTS(OLC_SHOP(d))), atoi(arg));
       sedit_products_menu(d);
       return;
     case SEDIT_NEW_ROOM:
       i = real_room(atol(arg));
       if (i < 0) {
	 send_to_char("That room does not exist, try again : ", d->character);
	 return;
       }
       sedit_add_to_int_list(&(S_ROOMS(OLC_SHOP(d))), atoi(arg));
       sedit_rooms_menu(d);
       return;
     case SEDIT_DELETE_ROOM:
       sedit_remove_from_int_list(&(S_ROOMS(OLC_SHOP(d))), atoi(arg));
       sedit_rooms_menu(d);
       return;
     case SEDIT_SHOP_FLAGS:
       i = MAX(0, MIN(NUM_SHOP_FLAGS, atoi(arg)));
       if (i > 0) {
	 /*. Toggle bit .*/
	 i = 1 << (i-1);
	 if (IS_SET(S_BITVECTOR(OLC_SHOP(d)), i))
	   REMOVE_BIT(S_BITVECTOR(OLC_SHOP(d)), i);
	 else
	   SET_BIT(S_BITVECTOR(OLC_SHOP(d)), i);
	 sedit_shop_flags_menu(d);
	 return;
       }
       break;
     case SEDIT_NOTRADE:
       i = MAX(0, MIN(NUM_TRADERS, atoi(arg)));
       if (i > 0) {
	 /*. Toggle bit .*/
	 i = 1 << (i-1);
	 if (IS_SET(S_NOTRADE(OLC_SHOP(d)), i))
	   REMOVE_BIT(S_NOTRADE(OLC_SHOP(d)), i);
	 else
	   SET_BIT(S_NOTRADE(OLC_SHOP(d)), i);
	 sedit_no_trade_menu(d);
	 return;
       }
       break;

     default:
       /*. We should never get here .*/
       cleanup_olc(d, CLEANUP_ALL);
       mudlog("SYSERR: OLC: sedit_parse(): Reached default case!",BRF,LVL_GOD,TRUE);
       break;
   }
   /*
    * If we get here, we have probably changed something, and now want to
    *    return to main menu.  Use OLC_VAL as a 'has changed' flag.
    */
   OLC_VAL(d) = 1;
   sedit_disp_menu(d);
}


/*. Stuff for guilds starts here .*/
void gedit_disp_menu(struct descriptor_data * d)
{
   extern char *restr_bits[];

   if (!OLC_GUILD(d))
     gedit_setup_new(d);

   get_char_cols(d->character);

   sprintbit((long) OLC_GUILD(d)->restrictions, restr_bits, buf1);

   sprintf(buf,
	"[H[J"
	"-- Guild number : [%s%d%s]\r\n"
	"%s1%s) Name        : %s%s\r\n"
	"%s2%s) Leader      : %s%s\r\n"
	"%s3%s) Restrictions: %s%s\r\n"
	"%s4%s) Ldr's Title : %s%s\r\n"
	"%s5%s) Hig's Title : %s%s\r\n"
	"%s6%s) Med's Title : %s%s\r\n"
	"%s7%s) Low's Title : %s%s\r\n"
	"%s8%s) Look String : \r\n%s%s\r\n"
	"%s9%s) Entrance    : %s[%s%ld%s] %s%s\r\n"
	"%sA%s) Guard       : %s[%s%ld%s] %s%s\r\n"
	"%sB%s) Direction   : %s%s\r\n"
	"%sP%s) Purge this Guild\r\n"
	"%sQ%s) Quit\r\n"
	"Enter choice : ",

	cyn, OLC_GUILD(d)->number, nrm,
	grn, nrm, yel, OLC_GUILD(d)->name,
	grn, nrm, yel, OLC_GUILD(d)->leadersname,
	grn, nrm, cyn, buf1,
	grn, nrm, cyn, OLC_GUILD(d)->xlvlname,
	grn, nrm, cyn, OLC_GUILD(d)->hlvlname,
	grn, nrm, cyn, OLC_GUILD(d)->mlvlname,
	grn, nrm, cyn, OLC_GUILD(d)->blvlname,
	grn, nrm, cyn, OLC_GUILD(d)->member_look_str ?
	   OLC_GUILD(d)->member_look_str : "None",
	grn, nrm, yel, cyn, real_room(OLC_GUILD(d)->guild_entr_room) == -1 ?
	   -1 : OLC_GUILD(d)->guild_entr_room, yel,
	cyn, real_room(OLC_GUILD(d)->guild_entr_room) == -1 ?
	   "None" : world[real_room(OLC_GUILD(d)->guild_entr_room)].name,
	grn, nrm, yel, cyn, real_mobile(OLC_GUILD(d)->guardian_mob_vn) == -1 ?
	   -1 : mob_index[real_mobile(OLC_GUILD(d)->guardian_mob_vn)].vnumber,
	yel, cyn, real_mobile(OLC_GUILD(d)->guardian_mob_vn) == -1 ?
	   "None" : mob_proto[real_mobile(OLC_GUILD(d)->guardian_mob_vn)].player.short_descr,
	grn, nrm, cyn, OLC_GUILD(d)->direction <= 0 ? "None" :  dirs[OLC_GUILD(d)->direction-1],
	grn, nrm,
	grn, nrm
  );
  send_to_char(buf, d->character);

  OLC_MODE(d) = GEDIT_MAIN_MENU;
}

void gedit_disp_restricts_menu(struct descriptor_data * d)
{
}

void gedit_disp_dir_menu(struct descriptor_data * d)
{
}

void gedit_free_guild(struct guild_type *gptr)
{
   void dequeue_guild(int gnum);
   void free_guild(struct guild_type *gptr);

   dequeue_guild(gptr->number);
   free_guild(gptr);
}

void gedit_parse(struct descriptor_data *d, char *arg)
{
   void save_guilds(void);
   long number;

   switch (OLC_MODE(d)) {
     case GEDIT_CONFIRM_SAVE:
       switch (*arg) {
	 case 'y':
	 case 'Y':
	   save_guilds();
	   sprintf(buf, "OLC: %s edits guild %d", GET_NAME(d->character), OLC_GUILD(d)->number);
	   mudlog(buf, CMP, LVL_GOD, TRUE);
	   send_to_char("Guild saved to disk and memory.\r\n", d->character);
	   cleanup_olc(d, CLEANUP_STRUCTS);
	   break;
	 case 'n':
	 case 'N':
	   /* free everything up, including strings etc */
	   cleanup_olc(d, CLEANUP_STRUCTS);
	   break;
	 default:
	   send_to_char("Invalid choice!\r\n", d->character);
	   send_to_char("Do you wish to save this guild? : ", d->character);
	   break;
       }
       return;

     case GEDIT_MAIN_MENU:
       switch (*arg) {
	 case 'q':
	 case 'Q':
	   if (OLC_VAL(d)) {
	     /*. Something has been modified .*/
	     send_to_char("Do you wish to save this guild? : ", d->character);
	     OLC_MODE(d) = GEDIT_CONFIRM_SAVE;
	   } else
	     cleanup_olc(d, CLEANUP_STRUCTS);
	   return;
	 case '1':
	   send_to_char("Enter guild name:-\r\n| ", d->character);
	   OLC_MODE(d) = GEDIT_NAME;
	   break;
	 case '2':
	   send_to_char("Enter guild leader's name: ", d->character);
	   OLC_MODE(d) = GEDIT_LEADERSNAME;
	   break;
	 case '3':
	   gedit_disp_restricts_menu(d);
	   break;
	 case '4':
	   send_to_char("Enter guild leader's title: ", d->character);
	   OLC_MODE(d) = GEDIT_XLVLNAME;
	   break;
	 case '5':
	   send_to_char("Enter guild high member's title: ", d->character);
	   OLC_MODE(d) = GEDIT_HLVLNAME;
	   break;
	 case '6':
	   send_to_char("Enter guild medium member's title: ", d->character);
	   OLC_MODE(d) = GEDIT_MLVLNAME;
	   break;
	 case '7':
	   send_to_char("Enter guild low member's title: ", d->character);
	   OLC_MODE(d) = GEDIT_BLVLNAME;
	   break;
	 case '8':
	   send_to_char("Enter guild member look string:-\r\n| ", d->character);
	   OLC_MODE(d) = GEDIT_MBR_LOOK_STR;
	   break;
	 case '9':
	   send_to_char("Enter guild entrance vnum: ", d->character);
	   OLC_MODE(d) = GEDIT_ENTR_ROOM;
	   break;
	 case 'a':
	 case 'A':
	   send_to_char("Enter guild guardian vnum: ", d->character);
	   OLC_MODE(d) = GEDIT_GUARD;
	   break;
	 case 'b':
	 case 'B':
	   send_to_char("Enter guardian direction block: ", d->character);
	   OLC_MODE(d) = GEDIT_GUARD;
	   break;
	 case 'p':
	 case 'P':
	   if (GET_LEVEL(d->character) >= LVL_IMPL) {
	     /* free everything up, including strings etc */
	     cleanup_olc(d, CLEANUP_ALL);
	   } else {
	     send_to_char("Sorry you are not allowed to do that at this time.\r\n",d->character);
	     gedit_disp_menu(d);
	   }
	   return;
	 default:
	   send_to_char("Invalid choice!", d->character);
	   gedit_disp_menu(d);
	   break;
       }
       return;

     case GEDIT_NAME:
       if (OLC_GUILD(d)->name)
	 free(OLC_GUILD(d)->name);
       OLC_GUILD(d)->name = str_dup(arg);
       break;

     case GEDIT_LEADERSNAME:
       if (OLC_GUILD(d)->leadersname)
	 free(OLC_GUILD(d)->leadersname);
       OLC_GUILD(d)->leadersname = str_dup(arg);
       break;

     case GEDIT_XLVLNAME:
       if (OLC_GUILD(d)->xlvlname)
	 free(OLC_GUILD(d)->xlvlname);
       OLC_GUILD(d)->xlvlname = str_dup(arg);
       break;

     case GEDIT_HLVLNAME:
       if (OLC_GUILD(d)->hlvlname)
	 free(OLC_GUILD(d)->hlvlname);
       OLC_GUILD(d)->hlvlname = str_dup(arg);
       break;

     case GEDIT_MLVLNAME:
       if (OLC_GUILD(d)->mlvlname)
	 free(OLC_GUILD(d)->mlvlname);
       OLC_GUILD(d)->mlvlname = str_dup(arg);
       break;

     case GEDIT_BLVLNAME:
       if (OLC_GUILD(d)->blvlname)
	 free(OLC_GUILD(d)->blvlname);
       OLC_GUILD(d)->blvlname = str_dup(arg);
       break;

     case GEDIT_MBR_LOOK_STR:
       if (OLC_GUILD(d)->member_look_str)
	 free(OLC_GUILD(d)->member_look_str);
       OLC_GUILD(d)->member_look_str = str_dup(arg);
       break;

     case GEDIT_ENTR_ROOM:
       number = real_room(atol(arg));
       if (number < 0)
	 send_to_char("That room does not exist, try again : ", d->character);
       else {
	 OLC_GUILD(d)->guild_entr_room = number;
	 gedit_disp_menu(d);
       }
       return;

     case GEDIT_GUARD:
       number = real_mobile(atol(arg));
       if (number < 0)
	 send_to_char("That mobile does not exist, try again : ", d->character);
       else {
	 OLC_GUILD(d)->guardian_mob_vn = number;
	 gedit_disp_menu(d);
       }
       return;

     case GEDIT_DIRECTION:
       number = atoi(arg);
       if (number < 0 || number > NUM_OF_DIRS)
	 send_to_char("Invalid direction, try again : ", d->character);
       else {
	 OLC_GUILD(d)->direction = number;
	 gedit_disp_menu(d);
       }
       return;

     case GEDIT_RESTRICTS:
       OLC_GUILD(d)->restrictions = 0;
       gedit_disp_menu(d);
       return;

     default:
       /* we should never get here */
       mudlog("SYSERR: Reached default case in gedit_parse",BRF,LVL_GOD,TRUE);
       break;
   }
   /*. If we get this far, something has be changed .*/
   OLC_VAL(d) = 1;
   gedit_disp_menu(d);
}

/*
 * Create a new guild with some default strings.
 */
void gedit_setup_new(struct descriptor_data *d)
{
   struct guild_type *enqueue_guild(void);
   extern int gnum;

   if ((OLC_GUILD(d) = enqueue_guild()) != NULL) {
     OLC_GUILD(d)->name = str_dup("Unfinished Guild");
     OLC_GUILD(d)->number = gnum++;
     OLC_GUILD(d)->restrictions = 0;
     OLC_GUILD(d)->leadersname = str_dup("NoOne");
     OLC_GUILD(d)->xlvlname = str_dup("Leader");
     OLC_GUILD(d)->hlvlname = str_dup("Champion");
     OLC_GUILD(d)->mlvlname = str_dup("Member");
     OLC_GUILD(d)->blvlname = str_dup("Initiate");
     OLC_GUILD(d)->member_look_str = NULL;
     OLC_GUILD(d)->guild_entr_room = 0;
     OLC_GUILD(d)->guardian_mob_vn = 0;
     OLC_GUILD(d)->direction = 0;
   } else
     fprintf(stderr, "SYSERR: Unable to create new guild!\r\n");
   gedit_disp_menu(d);
   OLC_VAL(d) = 0;
}
