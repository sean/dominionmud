/* ***********************************************************************\
*    _____ _            ____                  _       _                   *
*   |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __        *
*     | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \       *
*     | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |      *
*     |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|      *
*                                                                         *
*  File:  OLC.C                                                           *
*  Usage: Source file for On-Line-Creation Utilities                      *
*  Programmer(s): All code by Sean Mountcastle (Glasgian)                 *
\*********************************************************************** */

#define __OLC_C__

#include "conf.h"
#include "sysdep.h"
#include "protos.h"

void olc_disp_menu(struct descriptor_data * d);
void olc_default_parse( struct descriptor_data * d,
                        char * arg );
void zedit_parse(struct descriptor_data *d, char *arg);
void redit_parse(struct descriptor_data *d, char *arg);
void oedit_parse(struct descriptor_data *d, char *arg);
void medit_parse(struct descriptor_data *d, char *arg);
void sedit_parse(struct descriptor_data *d, char *arg);
void gedit_parse(struct descriptor_data *d, char *arg);
void reledit_parse(struct descriptor_data *d, char *arg);
void racedit_parse(struct descriptor_data *d, char *arg);
void skedit_parse(struct descriptor_data *d, char *arg);
void atedit_parse(struct descriptor_data *d, char *arg);
ACMD(do_olc_master);

/* top level OLC entry point */
ACMD(do_olc)
{
  STATE(ch->desc) = CON_EDITTING;
  SUBSTATE(ch->desc) = OLC_DEFAULT;
  /*. Give descriptor an OLC struct .*/
  CREATE(ch->desc->olc, struct olc_data, 1);
  olc_disp_menu(ch->desc);
}

void olc_parse( struct descriptor_data * d,
                char * arg )
{
  switch ( SUBSTATE( d ) ) {
  case OLC_ZONE_EDIT:
    zedit_parse(d, arg);
    break;
  case OLC_ROOM_EDIT:
    redit_parse(d, arg);
    break;
  case OLC_OBJ_EDIT:
    oedit_parse(d, arg);
    break;
  case OLC_MOB_EDIT:
    medit_parse(d, arg);
    break;
  case OLC_SHOP_EDIT:
    sedit_parse(d, arg);
    break;
  case OLC_GUILD_EDIT:
    gedit_parse(d, arg);
    break;
  case OLC_REL_EDIT:
    reledit_parse(d, arg);
    break;
  case OLC_RACE_EDIT:
    racedit_parse(d, arg);
    break;
  case OLC_SKILL_EDIT:
    skedit_parse(d, arg);
    break;
  case OLC_ATTRIB_EDIT:
    atedit_parse(d, arg);
    break;
  case OLC_DEFAULT:
  default:
    olc_default_parse(d, arg);
    break;
  }
}

void olc_default_parse( struct descriptor_data * d,
                        char * arg )
{
  switch (OLC_MODE(d)) {
    case OLC_MAIN_MENU:
      /* throw us out to whichever edit mode based on user input */
      switch (*arg) {
        case 'q':
        case 'Q':
          cleanup_olc(d, CLEANUP_ALL);
          return;
        case 's':
        case 'S':
          /* SAVE all outstanding zones */
          do_olc_master( d->character, arg, 0, SCMD_OLC_SAVEINFO );
          return;
        case 'h':
        case 'H':
          send_to_char( "Enter the name of the Help Entry to edit : ",
                        d->character );
          OLC_MODE(d) = OLC_HEDIT_SELECT;
          break;
        case '1':
        case 'z':
        case 'Z':
          send_to_char( "Enter the Zone Number to edit (or Enter to use this zone): ",
                        d->character );
          OLC_MODE(d) = OLC_ZEDIT_SELECT;
          break;
        case '2':
        case 'r':
        case 'R':
          send_to_char( "Enter the Room Number to edit (or Enter to use this room): ",
                        d->character );
          OLC_MODE(d) = OLC_REDIT_SELECT;
          break;
        case '3':
        case 'o':
        case 'O':
          send_to_char( "Enter the Object Number to edit : ",
                        d->character );
          OLC_MODE(d) = OLC_OEDIT_SELECT;
          break;
        case '4':
        case 'm':
        case 'M':
          send_to_char( "Enter the Mob Number to edit : ",
                        d->character );
          OLC_MODE(d) = OLC_MEDIT_SELECT;
          break;
        case '5':
          send_to_char( "Enter the Shop Number to edit : ",
                        d->character );
          OLC_MODE(d) = OLC_SEDIT_SELECT;
          break;
        case '6':
        case 'g':
        case 'G':
          send_to_char( "Enter the Guild Number to edit (or Enter to create one): ",
                        d->character );
          OLC_MODE(d) = OLC_GEDIT_SELECT;
          break;
        case '7':
          send_to_char( "Enter the Religion Number to edit (or Enter to create one): ",
                        d->character );
          OLC_MODE(d) = OLC_RELEDIT_SELECT;
          break;
        case '8':
          send_to_char( "Enter the Race Number to edit (or Enter to create one): ",
                        d->character );
          OLC_MODE(d) = OLC_RACEDIT_SELECT;
          break;
        case '9':
          send_to_char( "Enter the Skill/Spell Number to edit (or Enter to create one): ",
                        d->character );
          OLC_MODE(d) = OLC_SKEDIT_SELECT;
          break;
        default:
          olc_disp_menu(d);
          break;
      }
      return;
    case OLC_ZEDIT_SELECT:
      // int zone_num = atoi(arg);
      do_olc_master( d->character, arg, 0, SCMD_OLC_ZEDIT );
      break;
    case OLC_REDIT_SELECT:
      // int room_num = atol(arg);
      do_olc_master( d->character, arg, 0, SCMD_OLC_REDIT );
      break;
    case OLC_OEDIT_SELECT:
      // int obj_num = atol(arg);
      do_olc_master( d->character, arg, 0, SCMD_OLC_OEDIT );
      break;
    case OLC_MEDIT_SELECT:
      // int mob_num = atol(arg);
      do_olc_master( d->character, arg, 0, SCMD_OLC_MEDIT );
      break;
    case OLC_SEDIT_SELECT:
      // int shop_num = atol(arg);
      do_olc_master( d->character, arg, 0, SCMD_OLC_SEDIT );
      break;
    case OLC_GEDIT_SELECT:
      do_olc_master( d->character, arg, 0, SCMD_OLC_GEDIT );
      break;
    case OLC_HEDIT_SELECT:
      do_olc_master( d->character, arg, 0, SCMD_OLC_HEDIT );
      break;
    default:
      olc_disp_menu(d);
      mudlog("SYSERR: OLC: Reached default case in olc_default_parse()!",
             BRF, LVL_GOD, TRUE);
      break;
  }
}

void olc_disp_menu(struct descriptor_data * d)
{
  struct char_data * ch = d->character;
  /*. Build the menu .*/
  sprintf(buf, "[H[J"
          "     %s-- OLC MENU --%s\r\n"
          "%s1%s) %sZone Editor%s\r\n"
          "%s2%s) %sRoom Editor%s\r\n"
          "%s3%s) %sObject Editor%s\r\n"
          "%s4%s) %sMob Editor%s\r\n"
          "%s5%s) %sShop Editor%s\r\n"
          "%s6%s) %sGuild Editor%s\r\n"
          "%s7%s) %sReligion Editor%s\r\n"
          "%s8%s) %sRace Editor%s\r\n"
          "%s9%s) %sSkill/Spell Editor%s\r\n"
          "%sH%s) %sHelp Editor%s\r\n"
          "%sS%s) %sSave%s\r\n"
          "%sQ%s) %sQuit%s\r\n"
          "Enter choice : ",
          CCCYN(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCYEL(ch, C_NRM), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), CCCYN(ch, C_NRM), CCNRM(ch, C_NRM) );
  /*. Send the menu .*/
  send_to_char(buf, ch);

  OLC_MODE(d) = OLC_MAIN_MENU;
}

/* Handle the editting of religions */
void reledit_parse(struct descriptor_data *d, char *arg)
{
}

/* Handle the editting of races */
void racedit_parse(struct descriptor_data *d, char *arg)
{
}

/* Handle the editting of skills/spells */
void skedit_parse(struct descriptor_data *d, char *arg)
{
}

/* Handle the editting of attributes */
void atedit_parse(struct descriptor_data *d, char *arg)
{
}
