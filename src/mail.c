/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  MAIL.C                                       Based on CircleMUD *
*  Usage: Internal funcs and player spec_procs of mud-mail system         *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

/******* MUD MAIL SYSTEM MAIN FILE ***************************************

Written by Jeremy Elson (jelson@cs.jhu.edu)

*************************************************************************/

#define  __MAIL_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

#define VNUM_OBJ_MAIL 97

void postmaster_send_mail(struct char_data * ch, struct char_data *mailman,
			  int cmd, char *arg);
void postmaster_check_mail(struct char_data * ch, struct char_data *mailman,
			  int cmd, char *arg);
void postmaster_receive_mail(struct char_data * ch, struct char_data *mailman,
			  int cmd, char *arg);
ACMD(do_tell);
extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct index_data *mob_index;
extern int no_mail;
extern struct room_data *world;
int find_name(char *name);

mail_index_type *mail_index = 0;/* list of recs in the mail file  */
position_list_type *free_list = 0;      /* list of free positions in file */
long file_end_pos = 0;          /* length of file */


void push_free_list(long pos)
{
  position_list_type *new_pos;

  CREATE(new_pos, position_list_type, 1);
  new_pos->position = pos;
  new_pos->next = free_list;
  free_list = new_pos;
}



long pop_free_list(void)
{
  position_list_type *old_pos;
  long return_value;

  if ((old_pos = free_list) != 0) {
    return_value = free_list->position;
    free_list = old_pos->next;
    free(old_pos);
    return return_value;
  } else
    return file_end_pos;
}



mail_index_type *find_char_in_index(long searchee)
{
  mail_index_type *tmp;

  if (searchee < 0) {
    log("SYSERR: Mail system -- non fatal error #1.");
    return 0;
  }
  for (tmp = mail_index; (tmp && tmp->recipient != searchee); tmp = tmp->next);

  return tmp;
}



void write_to_file(void *buf, int size, long filepos)
{
  FILE *mail_file;

  mail_file = fopen(MAIL_FILE, "r+b");

  if (filepos % BLOCK_SIZE) {
    log("SYSERR: Mail system -- fatal error #2!!!");
    no_mail = 1;
    return;
  }
  fseek(mail_file, filepos, SEEK_SET);
  fwrite(buf, size, 1, mail_file);

  /* find end of file */
  fseek(mail_file, 0L, SEEK_END);
  file_end_pos = ftell(mail_file);
  fclose(mail_file);
  return;
}


void read_from_file(void *buf, int size, long filepos)
{
  FILE *mail_file;

  mail_file = fopen(MAIL_FILE, "r+b");

  if (filepos % BLOCK_SIZE) {
    log("SYSERR: Mail system -- fatal error #3!!!");
    no_mail = 1;
    return;
  }
  fseek(mail_file, filepos, SEEK_SET);
  fread(buf, size, 1, mail_file);
  fclose(mail_file);
  return;
}




void index_mail(long id_to_index, long pos)
{
  mail_index_type *new_index;
  position_list_type *new_position;

  if (id_to_index < 0) {
    log("SYSERR: Mail system -- non-fatal error #4.");
    return;
  }
  if (!(new_index = find_char_in_index(id_to_index))) {
    /* name not already in index.. add it */
    CREATE(new_index, mail_index_type, 1);
    new_index->recipient = id_to_index;
    new_index->list_start = NULL;

    /* add to front of list */
    new_index->next = mail_index;
    mail_index = new_index;
  }
  /* now, add this position to front of position list */
  CREATE(new_position, position_list_type, 1);
  new_position->position = pos;
  new_position->next = new_index->list_start;
  new_index->list_start = new_position;
}


/* SCAN_FILE */
/* scan_file is called once during boot-up.  It scans through the mail file
   and indexes all entries currently in the mail file. */
int scan_file(void)
{
  FILE *mail_file;
  header_block_type next_block;
  int total_messages = 0, block_num = 0;
  char buf[100];

  if (!(mail_file = fopen(MAIL_FILE, "r"))) {
    log("Mail file non-existant... creating new file.");
    mail_file = fopen(MAIL_FILE, "w");
    fclose(mail_file);
    return 1;
  }
  while (fread(&next_block, sizeof(header_block_type), 1, mail_file)) {
    if (next_block.block_type == HEADER_BLOCK) {
      index_mail(next_block.header_data.to, block_num * BLOCK_SIZE);
      total_messages++;
    } else if (next_block.block_type == DELETED_BLOCK)
      push_free_list(block_num * BLOCK_SIZE);
    block_num++;
  }

  file_end_pos = ftell(mail_file);
  fclose(mail_file);
  sprintf(buf, "   %ld bytes read.", file_end_pos);
  log(buf);
  if (file_end_pos % BLOCK_SIZE) {
    log("SYSERR: Error booting mail system -- Mail file corrupt!");
    log("SYSERR: Mail disabled!");
    return 0;
  }
  sprintf(buf, "   Mail file read -- %d messages.", total_messages);
  log(buf);
  return 1;
}                               /* end of scan_file */


/* HAS_MAIL */
/* a simple little function which tells you if the guy has mail or not */
int has_mail(long recipient)
{
  if (find_char_in_index(recipient))
    return 1;
  return 0;
}



/* STORE_MAIL  */
/* call store_mail to store mail.  (hard, huh? :-) )  Pass 3 arguments:
   who the mail is to (long), who it's from (long), and a pointer to the
   actual message text (char *).
*/

void store_mail(long to, long from, char *message_pointer)
{
  header_block_type header;
  data_block_type data;
  long last_address, target_address;
  char *msg_txt = message_pointer;
  int bytes_written = 0;
  int total_length = strlen(message_pointer);

  assert(sizeof(header_block_type) == sizeof(data_block_type));
  assert(sizeof(header_block_type) == BLOCK_SIZE);

  if (from < 0 || to < 0 || !*message_pointer) {
    log("SYSERR: Mail system -- non-fatal error #5.");
    return;
  }
  memset((char *) &header, 0, sizeof(header));  /* clear the record */
  header.block_type = HEADER_BLOCK;
  header.header_data.next_block = LAST_BLOCK;
  header.header_data.from = from;
  header.header_data.to = to;
  header.header_data.mail_time = time(0);
  strncpy(header.txt, msg_txt, HEADER_BLOCK_DATASIZE);
  header.txt[HEADER_BLOCK_DATASIZE] = '\0';

  target_address = pop_free_list();     /* find next free block */
  index_mail(to, target_address);       /* add it to mail index in memory */
  write_to_file(&header, BLOCK_SIZE, target_address);

  if (strlen(msg_txt) <= HEADER_BLOCK_DATASIZE)
    return;                     /* that was the whole message */

  bytes_written = HEADER_BLOCK_DATASIZE;
  msg_txt += HEADER_BLOCK_DATASIZE;     /* move pointer to next bit of text */

  /*
   * find the next block address, then rewrite the header to reflect where
   * the next block is.
   */
  last_address = target_address;
  target_address = pop_free_list();
  header.header_data.next_block = target_address;
  write_to_file(&header, BLOCK_SIZE, last_address);

  /* now write the current data block */
  memset((char *) &data, 0, sizeof(data));      /* clear the record */
  data.block_type = LAST_BLOCK;
  strncpy(data.txt, msg_txt, DATA_BLOCK_DATASIZE);
  data.txt[DATA_BLOCK_DATASIZE] = '\0';
  write_to_file(&data, BLOCK_SIZE, target_address);
  bytes_written += strlen(data.txt);
  msg_txt += strlen(data.txt);

  /*
   * if, after 1 header block and 1 data block there is STILL part of the
   * message left to write to the file, keep writing the new data blocks and
   * rewriting the old data blocks to reflect where the next block is.  Yes,
   * this is kind of a hack, but if the block size is big enough it won't
   * matter anyway.  Hopefully, MUD players won't pour their life stories out
   * into the Mud Mail System anyway.
   * 
   * Note that the block_type data field in data blocks is either a number >=0,
   * meaning a link to the next block, or LAST_BLOCK flag (-2) meaning the
   * last block in the current message.  This works much like DOS' FAT.
   */

  while (bytes_written < total_length) {
    last_address = target_address;
    target_address = pop_free_list();

    /* rewrite the previous block to link it to the next */
    data.block_type = target_address;
    write_to_file(&data, BLOCK_SIZE, last_address);

    /* now write the next block, assuming it's the last.  */
    data.block_type = LAST_BLOCK;
    strncpy(data.txt, msg_txt, DATA_BLOCK_DATASIZE);
    data.txt[DATA_BLOCK_DATASIZE] = '\0';
    write_to_file(&data, BLOCK_SIZE, target_address);

    bytes_written += strlen(data.txt);
    msg_txt += strlen(data.txt);
  }
}                               /* store mail */




/* READ_DELETE */
/* read_delete takes 1 char pointer to the name of the person whose mail
you're retrieving.  It returns to you a char pointer to the message text.
The mail is then discarded from the file and the mail index. */

char *read_delete(long recipient, int  mail_zone)
/* recipient is the name as it appears in the index.
   recipient_formatted is the name as it should appear on the mail
   header (i.e. the text handed to the player) */
{
  header_block_type header;
  data_block_type data;
  mail_index_type *mail_pointer, *prev_mail;
  position_list_type *position_pointer;
  long mail_address, following_block;
  char *message, *tmstr, buf[200];
  size_t string_size;

  if (recipient < 0) {
    log("SYSERR: Mail system -- non-fatal error #6.");
    return 0;
  }
  if (!(mail_pointer = find_char_in_index(recipient))) {
    log("SYSERR: Mail system -- post office spec_proc error?  Error #7.");
    return 0;
  }
  if (!(position_pointer = mail_pointer->list_start)) {
    log("SYSERR: Mail system -- non-fatal error #8.");
    return 0;
  }
  if (!(position_pointer->next)) {      /* just 1 entry in list. */
    mail_address = position_pointer->position;
    free(position_pointer);

    /* now free up the actual name entry */
    if (mail_index == mail_pointer) {   /* name is 1st in list */
      mail_index = mail_pointer->next;
      free(mail_pointer);
    } else {
      /* find entry before the one we're going to del */
      for (prev_mail = mail_index;
	   prev_mail->next != mail_pointer;
	   prev_mail = prev_mail->next);
      prev_mail->next = mail_pointer->next;
      free(mail_pointer);
    }
  } else {
    /* move to next-to-last record */
    while (position_pointer->next->next)
      position_pointer = position_pointer->next;
    mail_address = position_pointer->next->position;
    free(position_pointer->next);
    position_pointer->next = 0;
  }

  /* ok, now lets do some readin'! */
  read_from_file(&header, BLOCK_SIZE, mail_address);

  if (header.block_type != HEADER_BLOCK) {
    log("SYSERR: Oh dear.");
    no_mail = 1;
    log("SYSERR: Mail system disabled!  -- Error #9.");
    return 0;
  }
  tmstr = asctime(localtime(&header.header_data.mail_time));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  sprintf(buf, " * * * * %s Mail System * * * *\r\n"
	  "Date: %s\r\n"
	  "  To: %s\r\n"
	  "From: %s\r\n\r\n", 
          (zone_table[mail_zone].name ? zone_table[mail_zone].name: "Dominion"),
          tmstr, CAP(get_name_by_id(recipient)),
	  CAP(get_name_by_id(header.header_data.from)));

  string_size = (sizeof(char) * (strlen(buf) + strlen(header.txt) + 1));
  CREATE(message, char, string_size);
  strcpy(message, buf);
  strcat(message, header.txt);
  message[string_size - 1] = '\0';
  following_block = header.header_data.next_block;

  /* mark the block as deleted */
  header.block_type = DELETED_BLOCK;
  write_to_file(&header, BLOCK_SIZE, mail_address);
  push_free_list(mail_address);

  while (following_block != LAST_BLOCK) {
    read_from_file(&data, BLOCK_SIZE, following_block);

    string_size = (sizeof(char) * (strlen(message) + strlen(data.txt) + 1));
    RECREATE(message, char, string_size);
    strcat(message, data.txt);
    message[string_size - 1] = '\0';
    mail_address = following_block;
    following_block = data.block_type;
    data.block_type = DELETED_BLOCK;
    write_to_file(&data, BLOCK_SIZE, mail_address);
    push_free_list(mail_address);
  }

  return message;
}


/*****************************************************************
** Below is the spec_proc for a postmaster using the above       **
** routines.  Written by Jeremy Elson (jelson@server.cs.jhu.edu) **
*****************************************************************/

SPECIAL(postmaster)
{
  if (!ch->desc || IS_NPC(ch))
    return 0;                   /* so mobs don't get caught here */

  if (!(CMD_IS("mail") || CMD_IS("check") || CMD_IS("receive")))
    return 0;

  if (no_mail) {
    send_to_char("Sorry, the mail system is having technical difficulties.\r\n", ch);
    return 0;
  }

  if (CMD_IS("mail")) {
    postmaster_send_mail(ch, me, cmd, argument);
    return 1;
  } else if (CMD_IS("check")) {
    postmaster_check_mail(ch, me, cmd, argument);
    return 1;
  } else if (CMD_IS("receive")) {
    postmaster_receive_mail(ch, me, cmd, argument);
    return 1;
  } else
    return 0;
}


void postmaster_send_mail(struct char_data * ch, struct char_data *mailman,
			  int cmd, char *arg)
{
  long recipient;
  char buf[256];

  if (!CAN_SEE(mailman, ch)) {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, mailman, 0, ch, TO_VICT);
    return;
  }
  if (GET_LEVEL(ch) < MIN_MAIL_LEVEL) {
    sprintf(buf, "%s Sorry, you have to be level %d to send mail!", 
		GET_NAME(ch), MIN_MAIL_LEVEL);
    do_tell(mailman, buf, 0, 0);
    return;
  }
  one_argument(arg, buf);

  if (!*buf) {                  /* you'll get no argument from me! */
    sprintf(buf, "%s You need to specify an addressee!", GET_NAME(ch));
    do_tell(mailman, buf, 0, 0);
    return;
  }
  if (GET_GOLD(ch) < STAMP_PRICE) {
    sprintf(buf, "%s A stamp costs %d coins ...which I see you can't afford.",
	     GET_NAME(ch), STAMP_PRICE);
    do_tell(mailman, buf, 0, 0);
    return;
  }
  if ((recipient = get_id_by_name(buf)) < 0) {
    sprintf(buf, "%s No one by that name is registered here!", GET_NAME(ch));
    do_tell(mailman, buf, 0, 0);
    return;
  }
  act("$n starts to write some mail.", TRUE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "%s I'll take %d coins for the stamp.", GET_NAME(ch), 
	STAMP_PRICE);
  do_tell(mailman, buf, 0, 0);
  sprintf(buf, "%s Write your message to %s, use @ on a new line when done.",
	   GET_NAME(ch), CAP(get_name_by_id(recipient)));
  do_tell(mailman, buf, 0, 0);

  GET_GOLD(ch) -= STAMP_PRICE;
  SET_BIT(PLR_FLAGS(ch), PLR_MAILING | PLR_WRITING);

  ch->desc->mail_to = recipient;
  ch->desc->str = (char **) malloc(sizeof(char *));
  *(ch->desc->str) = NULL;
  ch->desc->max_str = MAX_MAIL_SIZE;
}


void postmaster_check_mail(struct char_data * ch, struct char_data *mailman,
			  int cmd, char *arg)
{
  char buf[256];

  if (!CAN_SEE(mailman, ch)) {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, mailman, 0, ch, TO_VICT);
    return;
  }
  if (has_mail(GET_IDNUM(ch)))
    sprintf(buf, "%s You have mail waiting.", GET_NAME(ch));
  else 
    sprintf(buf, "%s Sorry, you don't have any mail waiting.", GET_NAME(ch));
  do_tell(mailman, buf, 0, 0);
}


void postmaster_receive_mail(struct char_data * ch, struct char_data *mailman,
			  int cmd, char *arg)
{
  char buf[256];
  struct obj_data *obj;

  if (!CAN_SEE(mailman, ch)) {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, mailman, 0, ch, TO_VICT);
    return;
  }
  if (!has_mail(GET_IDNUM(ch))) {
    sprintf(buf, "%s Sorry you don't have any mail waiting.", GET_NAME(ch));
    do_tell(mailman, buf, 0, 0);
    return;
  }
  while (has_mail(GET_IDNUM(ch))) {
    obj = create_obj();
    obj->item_number = real_object(VNUM_OBJ_MAIL);
    obj->name = str_dup("mail paper letter");
    obj->short_description = str_dup("a piece of mail");
    obj->description = str_dup("Someone has left a piece of mail here.");

    GET_OBJ_TYPE(obj) = ITEM_NOTE;
    GET_OBJ_WEAR(obj) = ITEM_WEAR_TAKE | ITEM_WEAR_HOLD;
    GET_OBJ_WEIGHT(obj) = 1;
    GET_OBJ_COST(obj) = 30;
    GET_OBJ_RENT(obj) = 10;
    obj->action_description = read_delete(GET_IDNUM(ch), IN_ZONE(mailman));

    if (obj->action_description == NULL)
      obj->action_description =
	str_dup("Mail system error - please report.  Error #11.\r\n");

    obj_to_char(obj, ch);

    act("$n gives you a piece of mail.", FALSE, mailman, 0, ch, TO_VICT);
    act("$N gives $n a piece of mail.", FALSE, ch, 0, mailman, TO_ROOM);
  }
}
