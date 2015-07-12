/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  DB.C                                         Based on CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __DB_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */
/* globals for mob_progs */
void mprog_read_programs(FILE * fp, struct index_data * pMobIndex);
char err_buf[MAX_STRING_LENGTH];

struct room_data *world = NULL; /* array of rooms                */
long   top_of_world = 0;        /* ref to top element of world   */

struct char_data *character_list = NULL;        /* global linked list of
						 * chars         */
struct index_data *mob_index;   /* index table for mobile file   */
struct char_data *mob_proto;    /* prototypes for mobs           */
long   top_of_mobt = 0;         /* top of mobile index table     */
long   num_of_mobs = 0;

struct obj_data *object_list = NULL;    /* global linked list of objs    */
struct index_data *obj_index;   /* index table for object file   */
struct obj_data *obj_proto;     /* prototypes for objs           */
long   top_of_objt = 0;         /* top of object index table     */

struct zone_data *zone_table;   /* zone table                    */
long   top_of_zone_table = 0;   /* top element of zone tab       */
struct message_list fight_messages[MAX_MESSAGES];       /* fighting messages     */

struct player_index_element *player_table = NULL;       /* index to plr file     */
FILE   *player_fl = NULL;         /* file desc of player file      */
int    top_of_p_table = 0;        /* ref to top of table           */
int    top_of_p_file = 0;         /* ref of size of p file         */
long   top_idnum = 0;             /* highest idnum in use          */

int no_mail = 0;                /* mail disabled?                */
int mini_mud = 0;               /* mini-mud mode?                */
int no_rent_check = 0;          /* skip rent check on boot?      */
time_t boot_time = 0;           /* time of mud boot              */
int game_restrict = 0;          /* level of game restriction     */
sh_int r_mortal_start_room;     /* rnum of mortal start room     */
sh_int r_immort_start_room;     /* rnum of immort start room     */
sh_int r_frozen_start_room;     /* rnum of frozen start room     */

char *credits = NULL;           /* game credits                  */
char *news = NULL;              /* mud news                      */
char *motd = NULL;              /* message of the day - mortals  */
char *imotd = NULL;             /* message of the day - immorts  */
char *newbm = NULL;             /* message for new characters    */
char *help = NULL;              /* help screen                   */
char *info = NULL;              /* info page                     */
char *wizlist = NULL;           /* list of higher gods           */
char *immlist = NULL;           /* list of peon gods             */
char *background = NULL;        /* background story              */
char *handbook = NULL;          /* handbook for new immortals    */
char *policies = NULL;          /* policies page                 */

struct help_index_element *help_table = 0;      /* the help table    */
struct help_index_element *wizhelp_table = 0;   /* the wizhelp table */
int    top_of_helpt = 0;           /* top of help index table       */
int    top_of_wizhelpt = 0;        /* top of help index table       */
FILE   *wizhelp_fl = NULL;
FILE   *help_fl = NULL;

struct time_info_data time_info;      /* the infomation about the time    */
struct player_special_data dummy_mob; /* dummy spec area for mobs      */
struct reset_q_type reset_q;          /* queue of zones to be reset    */
struct event_data *event_list;
int    event_top;

/* local functions */
void setup_dir(FILE * fl, long room, int dir);
void index_boot(int mode);
void discrete_load(FILE * fl, int mode);
void parse_room(FILE * fl, long vnumber_nr);
void parse_mobile(FILE * mob_f, unsigned long nr);
char *parse_object(FILE * obj_f, long nr);
void load_zones(FILE * fl, char *zonename);
void load_guilds(void);
#ifndef OLD_HELP
void load_help(FILE *fl, int mode);
#endif
void parse_experience( void );
void parse_attributes( void );
void parse_races( void );
void parse_religions( void );
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void assign_the_shopkeepers(void);
void build_player_index(void);
void char_to_store(struct char_data * ch, struct char_file_u * st);
void store_to_char(struct char_file_u * st, struct char_data * ch);
int is_empty(int zone_nr);
void reset_zone(int zone);
int file_to_string(char *name, char *buf);
int file_to_string_alloc(char *name, char **buf);
void check_start_rooms(void);
void renum_world(void);
void renum_zone_table(void);
void log_zone_error(long zone, int cmd_no, char *message);
void reset_time(void);
void clear_char(struct char_data * ch);
void set_bodyparts(struct char_data *ch);

/* external functions */
extern struct descriptor_data *descriptor_list;
void load_messages(void);
void weather_and_time(int mode);
void mag_assign_spells(void);
void boot_social_messages(void);
void update_obj_file(void);     /* In objsave.c */
void sort_commands(void);
void sort_spells(void);
void load_banned(void);
void Read_Invalid_List(void);
void boot_the_shops(FILE * shop_f, char *filename, int rec_count);
void weather_change(void);
#ifdef OLD_HELP
struct help_index_element *build_help_index(FILE * fl, int *num);
#else
int  hsort(const void *a, const void *b);
#endif
int  scan_file(void);
int  AddHatred(struct char_data *ch, int parm_type, int parm);
int  AddFears(struct char_data *ch, int parm_type, int parm);
void add_follower(struct char_data * ch, struct char_data * leader);
int  get_hometown(struct char_data *ch);
int  ungetc(int l, FILE *mob_f);
byte saving_throws( struct char_data * ch, int save );

SPECIAL(breath_any);
SPECIAL(breath_acid);
SPECIAL(breath_fire);
SPECIAL(breath_frost);
SPECIAL(breath_gas);
SPECIAL(breath_lightning);
SPECIAL(old_dragon);
SPECIAL(snake);
SPECIAL(fido);
SPECIAL(thief);

/* external vars */
extern int NUM_RACES;
extern struct race_data * races;
extern int no_specials;
long   exp_needed(int level);
extern struct material_type const material_list[];
extern char *color_list[];
#define READ_SIZE 256

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

/* this is necessary for the autowiz system */
void reboot_wizlists(void)
{
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
}


ACMD(do_reboot)
{
  int i;

  one_argument(argument, arg);

  if (!str_cmp(arg, "all") || *arg == '*') {
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
    file_to_string_alloc(IMMLIST_FILE, &immlist);
    file_to_string_alloc(NEWS_FILE, &news);
    file_to_string_alloc(CREDITS_FILE, &credits);
    file_to_string_alloc(MOTD_FILE, &motd);
    file_to_string_alloc(IMOTD_FILE, &imotd);
    file_to_string_alloc(NEWBM_FILE, &newbm);
    file_to_string_alloc(HELP_PAGE_FILE, &help);
    file_to_string_alloc(INFO_FILE, &info);
    file_to_string_alloc(POLICIES_FILE, &policies);
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
    file_to_string_alloc(BACKGROUND_FILE, &background);
    parse_experience( );
    parse_attributes( );
    parse_races( );
    parse_religions( );
  } else if (!str_cmp(arg, "wizlist"))
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
  else if (!str_cmp(arg, "immlist"))
    file_to_string_alloc(IMMLIST_FILE, &immlist);
  else if (!str_cmp(arg, "news"))
    file_to_string_alloc(NEWS_FILE, &news);
  else if (!str_cmp(arg, "credits"))
    file_to_string_alloc(CREDITS_FILE, &credits);
  else if (!str_cmp(arg, "motd"))
    file_to_string_alloc(MOTD_FILE, &motd);
  else if (!str_cmp(arg, "imotd"))
    file_to_string_alloc(IMOTD_FILE, &imotd);
  else if (!str_cmp(arg, "newbie"))
    file_to_string_alloc(NEWBM_FILE, &newbm);
  else if (!str_cmp(arg, "help"))
    file_to_string_alloc(HELP_PAGE_FILE, &help);
  else if (!str_cmp(arg, "info"))
    file_to_string_alloc(INFO_FILE, &info);
  else if (!str_cmp(arg, "policy"))
    file_to_string_alloc(POLICIES_FILE, &policies);
  else if (!str_cmp(arg, "handbook"))
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
  else if (!str_cmp(arg, "background"))
    file_to_string_alloc(BACKGROUND_FILE, &background);
  else if (!str_cmp(arg, "exp"))
    parse_experience( );
  else if (!str_cmp(arg, "attr"))
    parse_attributes( );
  else if (!str_cmp(arg, "races"))
    parse_races( );
  else if (!str_cmp(arg, "religions"))
    parse_religions( );
  else if (!str_cmp(arg, "xhelp")) {
#ifdef OLD_HELP
    if (help_fl)
      fclose(help_fl);
    if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
      return;
    else {
      for (i = 0; i < top_of_helpt; i++)
	free(help_table[i].keyword);
      free(help_table);
      help_table = build_help_index(help_fl, &top_of_helpt);
    }
#elif !defined OLD_HELP
    if (help_table) {
      for (i = 0; i < top_of_helpt; i++) {
	if (help_table[i].keyword)
	  free(help_table[i].keyword);
	if (help_table[i].entry && !help_table[i].duplicate)
	  free(help_table[i].entry);
      }
      free(help_table);
    }
    top_of_helpt = 0;
    index_boot(DB_BOOT_HLP);
#endif
  } else if (!str_cmp(arg, "wizhelp")) {
#ifdef OLD_HELP
    if (wizhelp_fl)
      fclose(wizhelp_fl);
    if (!(wizhelp_fl = fopen(WIZHELP_KWRD_FILE, "r")))
      return;
    else {
      for (i = 0; i < top_of_wizhelpt; i++)
	free(wizhelp_table[i].keyword);
      free(wizhelp_table);
      wizhelp_table = build_help_index(wizhelp_fl, &top_of_wizhelpt);
    }
#elif !defined OLD_HELP
    if (wizhelp_table) {
      for (i = 0; i < top_of_wizhelpt; i++) {
	if (wizhelp_table[i].keyword)
	  free(wizhelp_table[i].keyword);
	if (wizhelp_table[i].entry && !wizhelp_table[i].duplicate)
	  free(wizhelp_table[i].entry);
      }
      free(wizhelp_table);
    }
    top_of_wizhelpt = 0;
    index_boot(DB_BOOT_WIZHLP);
#endif
  } else {
    send_to_char( "Unknown reload option.\r\n"
                  "\tAvailable Options are:\r\n"
                  "\t\tAll | *    - Reload everything\r\n"
                  "\t\twizlist    - Reload the WizList\r\n"
                  "\t\timmlist    - Reload the ImmList\r\n"
                  "\t\tnews       - Reload the news\r\n"
                  "\t\tcredits    - Reload the credits\r\n"
                  "\t\tmotd       - Reload the Message of the Day\r\n"
                  "\t\timotd      - Reload the Immort Message of the Day\r\n"
                  "\t\tnewbie     - Reload the Newbie Info\r\n"
                  "\t\thelp       - Reload the Help File\r\n"
                  "\t\tinfo       - Reload the Info File\r\n"
                  "\t\tpolicy     - Reload the Policy File\r\n"
                  "\t\thandbook   - Reload the Immortal Handbook\r\n"
                  "\t\tbackground - Reload the Background Story\r\n"
                  "\t\texp        - Reload the Experience Table\r\n"
                  "\t\tattr       - Reload the Attribute Map\r\n"
                  "\t\traces      - Reload the Races\r\n"
                  "\t\treligions  - Reload the Religions\r\n"
                  "\t\txhelp      - Reload the Extra Help\r\n"
                  "\t\twizhelp    - Reload the WizHelp\r\n", ch);
    return;
  }

  send_to_char(OK, ch);
}


void boot_world(void)
{
  log("Loading zone table.");
  index_boot(DB_BOOT_ZON);

  log("Loading rooms.");
  index_boot(DB_BOOT_WLD);

  log("Renumbering rooms.");
  renum_world();

  log("Checking start rooms.");
  check_start_rooms();

  log("Loading mobs and generating index.");
  index_boot(DB_BOOT_MOB);

  log("Loading objs and generating index.");
  index_boot(DB_BOOT_OBJ);

  log("Renumbering zone table.");
  renum_zone_table();

  if (!no_specials) {
    log("Loading shops.");
    index_boot(DB_BOOT_SHP);
  }
}



/* body of the booting system */
void boot_db(void)
{
  void init_spell_spheres(void);
  long i;

  log("Boot db -- BEGIN.");

  log("Reading news, credits, help, bground, info & motds.");
  file_to_string_alloc(NEWS_FILE, &news);
  file_to_string_alloc(CREDITS_FILE, &credits);
  file_to_string_alloc(MOTD_FILE, &motd);
  file_to_string_alloc(IMOTD_FILE, &imotd);
  file_to_string_alloc(NEWBM_FILE, &newbm);
  file_to_string_alloc(HELP_PAGE_FILE, &help);
  file_to_string_alloc(INFO_FILE, &info);
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
  file_to_string_alloc(POLICIES_FILE, &policies);
  file_to_string_alloc(HANDBOOK_FILE, &handbook);
  file_to_string_alloc(BACKGROUND_FILE, &background);
  parse_experience( );
  parse_attributes( );
  parse_races( );
  parse_religions( );
  
#ifdef OLD_HELP
  log("Opening help file.");
  if (!(help_fl = fopen(HELP_KWRD_FILE, "r"))) {
    log("   Could not open help file.");
  } else
    help_table = build_help_index(help_fl, &top_of_helpt);
  log("Opening wizhelp file.");
  if (!(wizhelp_fl = fopen(WIZHELP_KWRD_FILE, "r"))) {
    log("   Could not open wizhelp file.");
  } else
    wizhelp_table = build_help_index(wizhelp_fl, &top_of_wizhelpt);
#else
  log("Loading help entries.");
  index_boot(DB_BOOT_HLP);
  log("Loading wizhelp entries.");
  index_boot(DB_BOOT_WIZHLP);
#endif

  boot_world();

  log("Generating player index.");
  build_player_index();

  log("Loading fight messages.");
  load_messages();

  log("Loading social messages.");
  boot_social_messages();

  log("Loading guilds.");
  load_guilds();

  log("Assigning function pointers:");

  if (!no_specials) {
    log("   Mobiles.");
    assign_mobiles();
    log("   Shopkeepers.");
    assign_the_shopkeepers();
    log("   Objects.");
    assign_objects();
    log("   Rooms.");
    assign_rooms();
  }
  log("   Spells.");
  mag_assign_spells();

  log("Assigning spell and skill levels.");
  init_spell_spheres();

  log("Sorting command list and spells.");
  sort_commands();
  sort_spells();

  log("Booting mail system.");
  if (!scan_file()) {
    log("    Mail boot failed -- Mail system disabled");
    no_mail = 1;
  }
  log("Reading banned site and invalid-name list.");
  load_banned();
  Read_Invalid_List();

  if (!no_rent_check) {
    log("Deleting timed-out crash and rent files:");
    update_obj_file();
    log("Done.");
  }
  for (i = 0; i <= top_of_zone_table; i++) {
    sprintf(buf2, "Resetting %s (rooms %ld-%ld).",
	    zone_table[i].name, (i ? (zone_table[i - 1].top + 1) : 0),
	    zone_table[i].top);
    log(buf2);
    reset_zone(i);
  }
  log("Restoring the game time:");
  reset_time();

  reset_q.head = reset_q.tail = NULL;

  if (!mini_mud) {
    log("Booting houses.");
    House_boot();
  }
  boot_time = time(0);

  log("Boot db -- DONE.");
}


/* reset the time in the game from file */
void reset_time(void)
{
  int i;
  long beginning_of_time = 650336715;
  struct time_info_data mud_time_passed(time_t t2, time_t t1);

  /* see if the MUD has ever run before, if so, pull the saved time
     from there, otherwise, start at the beginning of time */
  if ( restore_mud_time( &time_info ) != 0 )
    time_info = mud_time_passed(time(0), beginning_of_time);

  for (i = 0; i < top_of_zone_table; ++i) {
    if (time_info.hours <= 4)
      zone_table[i].weather_info.sunlight = SUN_DARK;
    else if (time_info.hours == 5)
      zone_table[i].weather_info.sunlight = SUN_RISE;
    else if (time_info.hours <= 20)
      zone_table[i].weather_info.sunlight = SUN_LIGHT;
    else if (time_info.hours == 21)
      zone_table[i].weather_info.sunlight = SUN_SET;
    else
      zone_table[i].weather_info.sunlight = SUN_DARK;

    zone_table[i].weather_info.humidity = 50;

    if ((time_info.month >= 1) && (time_info.month <= 7))
      zone_table[i].weather_info.humidity -= dice(1, 50);
    else
      zone_table[i].weather_info.humidity += dice(1, 50);

    zone_table[i].weather_info.change = 0;
    zone_table[i].weather_info.sky = SKY_SUNNY;
  }

  sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.",
          time_info.hours, time_info.day,
          time_info.month, time_info.year);
  log( buf );

  weather_change();
}



/* generate index table for the player file */
void build_player_index(void)
{
  int nr = -1, i;
  long size, recs;
  struct char_file_u dummy;

  if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
    if (errno != ENOENT) {
      perror("fatal error opening playerfile");
      exit(1);
    } else {
      log("No playerfile.  Creating a new one.");
      touch(PLAYER_FILE);
      if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
	perror("fatal error opening playerfile");
	exit(1);
      }
    }
  }

  fseek(player_fl, 0L, SEEK_END);
  size = ftell(player_fl);
  rewind(player_fl);
  if (size % sizeof(struct char_file_u))
    fprintf(stderr, "\aWARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
  recs = size / sizeof(struct char_file_u);
  if (recs) {
    sprintf(buf, "   %ld players in database.", recs);
    log(buf);
    CREATE(player_table, struct player_index_element, recs);
  } else {
    player_table = NULL;
    top_of_p_file = top_of_p_table = -1;
    return;
  }

  for (; !feof(player_fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
    if (!feof(player_fl)) {     /* new record */
      nr++;
      CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
      for (i = 0;
	   (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
      player_table[nr].id = dummy.char_specials_saved.idnum;
      top_idnum = MAX(top_idnum, dummy.char_specials_saved.idnum);
    }
  }

  top_of_p_file = top_of_p_table = nr;
}

/* function to count how many hash-mark delimited records exist in a file */
int count_hash_records(FILE * fl)
{
  char buf[128];
  int count = 0;

  while (fgets(buf, 128, fl))
    if (*buf == '#')
      count++;

  return count;
}

void index_boot(int mode)
{
  char *index_filename, *prefix = NULL;
  FILE *index, *db_file;
  int rec_count = 0;

  switch (mode) {
  case DB_BOOT_WLD:
    prefix = WLD_PREFIX;
    break;
  case DB_BOOT_MOB:
    prefix = MOB_PREFIX;
    break;
  case DB_BOOT_OBJ:
    prefix = OBJ_PREFIX;
    break;
  case DB_BOOT_ZON:
    prefix = ZON_PREFIX;
    break;
  case DB_BOOT_SHP:
    prefix = SHP_PREFIX;
    break;
#ifndef OLD_HELP
  case DB_BOOT_HLP:
    prefix = HLP_PREFIX;
    break;
  case DB_BOOT_WIZHLP:
    prefix = HLP_PREFIX;
    break;
#endif
  default:
    log("SYSERR: Unknown subcommand to index_boot!");
    exit(1);
    break;
  }

  if (mini_mud)
    index_filename = MINDEX_FILE;
  else
    index_filename = INDEX_FILE;

  sprintf(buf2, "%s/%s", prefix, index_filename);

  if (!(index = fopen(buf2, "r"))) {
    sprintf(buf1, "Error opening index file '%s'", buf2);
    perror(buf1);
    exit(1);
  }
  /* first, count the number of records in the file so we can malloc */
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      perror(buf2);
      log("file listed in index not found");
      exit(1);
    } else {
      if (mode == DB_BOOT_ZON)
	rec_count++;
      else
	rec_count += count_hash_records(db_file);
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }

  /* Exit if 0 records, unless this is shops */
  if (!rec_count) {
    if (mode == DB_BOOT_SHP)
       return;
    log("SYSERR: boot error - 0 records counted");
    exit(1);
  }
  rec_count++;

  switch (mode) {
  case DB_BOOT_WLD:
    CREATE(world, struct room_data, rec_count);
    break;
  case DB_BOOT_MOB:
    num_of_mobs = rec_count;
    CREATE(mob_proto, struct char_data, rec_count);
    CREATE(mob_index, struct index_data, rec_count);
    break;
  case DB_BOOT_OBJ:
    CREATE(obj_proto, struct obj_data, rec_count);
    CREATE(obj_index, struct index_data, rec_count);
    break;
  case DB_BOOT_ZON:
    CREATE(zone_table, struct zone_data, rec_count);
    break;
#ifndef OLD_HELP
  case DB_BOOT_HLP:
    CREATE(help_table, struct help_index_element, rec_count * 2);
    break;
  case DB_BOOT_WIZHLP:
    CREATE(wizhelp_table, struct help_index_element, rec_count * 2);
    break;
#endif
  }

  rewind(index);
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      perror(buf2);
      exit(1);
    }
    switch (mode) {
    case DB_BOOT_WLD:
    case DB_BOOT_OBJ:
    case DB_BOOT_MOB:
      discrete_load(db_file, mode);
      break;
    case DB_BOOT_ZON:
      load_zones(db_file, buf2);
      break;
#ifndef OLD_HELP
    case DB_BOOT_HLP:
      load_help(db_file, 0);
      break;
    case DB_BOOT_WIZHLP:
      load_help(db_file, 1);
      break;
#endif
    case DB_BOOT_SHP:
      boot_the_shops(db_file, buf2, rec_count);
      break;
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }
#ifndef OLD_HELP
  /* sort the help index */
  if (mode == DB_BOOT_HLP)
    qsort(help_table, top_of_helpt, sizeof(struct help_index_element), hsort);
  else if (mode == DB_BOOT_WIZHLP)
    qsort(wizhelp_table, top_of_wizhelpt, sizeof(struct help_index_element), hsort);
#endif
}


void discrete_load(FILE * fl, int mode)
{
  long nr = -1, last = 0;
  char line[256];

  char *modes[] = {"world", "mob", "obj"};

  for (;;) {
    /*
     * we have to do special processing with the obj files because they have
     * no end-of-record marker :(
     */
    if (mode != DB_BOOT_OBJ || nr < 0)
      if (!get_line(fl, line)) {
	fprintf(stderr, "Format error after %s #%ld\n", modes[mode], nr);
	exit(1);
      }
    if (*line == '$')
      return;

    if (*line == '#') {
      last = nr;
      if (sscanf(line, "#%ld", &nr) != 1) {
	fprintf(stderr, "Format error after %s #%ld\n", modes[mode], last);
	exit(1);
      }
      if (nr >= 9999999)
	return;
      else
	switch (mode) {
	case DB_BOOT_WLD:
	  parse_room(fl, nr);
	  break;
	case DB_BOOT_MOB:
	  parse_mobile(fl, nr);
	  break;
	case DB_BOOT_OBJ:
	  strcpy(line, parse_object(fl, nr));
	  break;
	}
    } else {
      fprintf(stderr, "Format error in %s file near %s #%ld\n",
	      modes[mode], modes[mode], nr);
      fprintf(stderr, "Offending line: '%s'\n", line);
      exit(1);
    }
  }
}


long asciiflag_conv(char *flag)
{
  long flags = 0;
  int is_number = 1;
  register char *p;

  for (p = flag; *p; p++) {
    if (islower((int)*p))
      flags |= 1 << (*p - 'a');
    else if (isupper((int)*p))
      flags |= 1 << (26 + (*p - 'A'));

    if (!isdigit((int)*p))
      is_number = 0;
  }

  if (is_number)
    flags = atol(flag);

  return flags;
}


/* load the rooms */
void parse_room(FILE * fl, long int vnumber_nr)
{
  static long int room_nr = 0, zone = 0;
  long   lt1, lt2;
  int t[10], i;
  char line[256], flags[128];
  struct extra_descr_data *new_descr;

  sprintf(buf2, "room #%ld", vnumber_nr);

  if (vnumber_nr <= (zone ? zone_table[zone - 1].top : -1)) {
    fprintf(stderr, "Room #%ld is below zone %ld.\n", vnumber_nr, zone);
    exit(1);
  }
  while (vnumber_nr > zone_table[zone].top)
    if (++zone > top_of_zone_table) {
      fprintf(stderr, "Room %ld is outside of any zone.\n", vnumber_nr);
      exit(1);
    }
  world[room_nr].zone = zone;
  world[room_nr].number = vnumber_nr;
  world[room_nr].name = fread_string(fl, buf2);
  world[room_nr].description = fread_string(fl, buf2);

  if (!get_line(fl, line) || sscanf(line, " %d %s %d %d %ld %ld %d", t,
       flags, t + 2, t + 3, &lt1, &lt2, t + 6) < 3) {
    fprintf(stderr, "Format error in room #%ld\n", vnumber_nr);
    exit(1);
  }
  /* t[0] is the zone number; ignored with the zone-file system */
  world[room_nr].room_flags = asciiflag_conv(flags);
  world[room_nr].sector_type = t[2];

  if (t[2] == -1) {
     world[room_nr].tele_time = t[3];
     world[room_nr].tele_targ = lt1;
     world[room_nr].tele_mask = lt2;
     world[room_nr].tele_cnt  = 0;
     world[room_nr].sector_type = t[6];
  } else {
     world[room_nr].tele_time = -1;
     world[room_nr].tele_targ = -1;
     world[room_nr].tele_mask = -1;
     world[room_nr].tele_cnt  = 0;
  }

  if (t[2] == SECT_WATER_NOSWIM || t[2] == SECT_UNDERWATER)  { /* river */
     /* read direction and rate of flow */
     world[room_nr].river_speed = t[3];
     world[room_nr].river_dir   = lt1;
  } else {
     world[room_nr].river_speed = -1;
     world[room_nr].river_dir   = -1;
  }

  world[room_nr].func     = NULL;
  world[room_nr].contents = NULL;
  world[room_nr].people   = NULL;
  world[room_nr].light    = 0;  /* Zero light sources */

  for (i = 0; i < NUM_OF_DIRS; i++)
    world[room_nr].dir_option[i] = NULL;

  world[room_nr].ex_description = NULL;

  sprintf(buf, "Format error in room #%ld (expecting D/E/S)", vnumber_nr);

  for (;;) {
    if (!get_line(fl, line)) {
      fprintf(stderr, "%s\n", buf);
      exit(1);
    }
    switch (*line) {
    case 'D':
      setup_dir(fl, room_nr, atoi(line + 1));
      break;
    case 'E':
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(fl, buf2);
      new_descr->description = fread_string(fl, buf2);
      new_descr->next = world[room_nr].ex_description;
      world[room_nr].ex_description = new_descr;
      break;
    case 'S':                   /* end of room */
      top_of_world = room_nr++;
      return;
      break;
    default:
      fprintf(stderr, "%s\n", buf);
      exit(1);
      break;
    }
  }
}



/* read direction data */
void setup_dir(FILE * fl, long room, int dir)
{
  long t[5];
  char line[256];

  sprintf(buf2, "room #%ld, direction D%d", world[room].number, dir);

  CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
  world[room].dir_option[dir]->general_description = fread_string(fl, buf2);
  world[room].dir_option[dir]->keyword = fread_string(fl, buf2);

  if (!get_line(fl, line)) {
    fprintf(stderr, "Format error, %s\n", buf2);
    exit(1);
  }
  if (sscanf(line, " %ld %ld %ld ", t, t + 1, t + 2) != 3) {
    fprintf(stderr, "Format error, %s\n", buf2);
    exit(1);
  }
  if (t[0] == 1)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR;
  else if (t[0] == 2)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
  else if (t[0] == 3)
     world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_SECRET;
  else if (t[0] == 4)
     world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF | EX_SECRET;
  else if (t[0] == 5)
     world[room].dir_option[dir]->exit_info = EX_CLIMB;
  else if (t[0] == 6)
     world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_CLIMB;
  else if (t[0] == 7)
     world[room].dir_option[dir]->exit_info = EX_CLIMB | EX_ISDOOR | EX_PICKPROOF;
  else
    world[room].dir_option[dir]->exit_info = 0;

  world[room].dir_option[dir]->key = t[1];
  world[room].dir_option[dir]->to_room = t[2];
}


/* make sure the start rooms exist & resolve their vnums to rnums */
void check_start_rooms(void)
{
  extern sh_int mortal_start_room;
  extern sh_int immort_start_room;
  extern sh_int frozen_start_room;

  if ((r_mortal_start_room = real_room(mortal_start_room)) < 0) {
    log("SYSERR:  Mortal start room does not exist.  Change in config.c.");
    exit(1);
  }
  if ((r_immort_start_room = real_room(immort_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Immort start room does not exist.  Change in config.c.");
    r_immort_start_room = r_mortal_start_room;
  }
  if ((r_frozen_start_room = real_room(frozen_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Frozen start room does not exist.  Change in config.c.");
    r_frozen_start_room = r_mortal_start_room;
  }
}


/* resolve all vnums into rnums in the world */
void renum_world(void)
{
  register long room, door;

  for (room = 0; room <= top_of_world; room++)
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (world[room].dir_option[door]) {
	world[room].dir_option[door]->to_room_vnum =
		  world[room].dir_option[door]->to_room;
	if (world[room].dir_option[door]->to_room != NOWHERE)
	  world[room].dir_option[door]->to_room =
	    real_room(world[room].dir_option[door]->to_room);
      }
}


#define ZCMD zone_table[zone].cmd[cmd_no]

/* resulve vnums into rnums in the zone reset tables */
void renum_zone_table(void)
{
  long zone, cmd_no, a, b;

  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
      a = b = 0;
      switch (ZCMD.command) {
      case 'M':
	a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
	b = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'O':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	if (ZCMD.arg3 != NOWHERE)
	  b = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'G':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	break;
      case 'E':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	break;
      case 'P':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	b = ZCMD.arg3 = real_object(ZCMD.arg3);
	break;
      case 'D':
	a = ZCMD.arg1 = real_room(ZCMD.arg1);
	break;
      case 'R': /* rem obj from room */
	a = ZCMD.arg1 = real_room(ZCMD.arg1);
	b = ZCMD.arg2 = real_object(ZCMD.arg2);
	break;
      }
      if (a < 0 || b < 0) {
	if (!mini_mud)
	  log_zone_error(zone, cmd_no, "Invalid vnum, cmd disabled");
	ZCMD.command = '*';
      }
    }
}



void parse_simple_mob(FILE *mob_f, unsigned long i, unsigned long nr)
{
  int j, t[10];
  char line[256];

  assert( mob_f != NULL );
  assert( i < num_of_mobs );

  mob_proto[i].real_abils.str   = 11;
  mob_proto[i].real_abils.intel = 11;
  mob_proto[i].real_abils.wis   = 11;
  mob_proto[i].real_abils.dex   = 11;
  mob_proto[i].real_abils.con   = 11;
  mob_proto[i].real_abils.cha   = 11;
  mob_proto[i].real_abils.will  = 11;

  get_line(mob_f, line);
  if (sscanf(line, " %d %d %d %dd%d+%d %dd%d+%d ",
             t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8) != 9) {
    fprintf(stderr, "Format error in mob #%ld, first line after S flag\n"
            "...expecting line of form '# # # #d#+# #d#+#'\n", nr);
    exit(1);
  }
  GET_LEVEL(mob_proto + i)    = MAX(0, MIN(LVL_IMPL, t[0]));
  if (t[1] >= (GET_LEVEL(mob_proto + i) * 2))
    mob_proto[i].points.hitroll = MIN(100, (GET_LEVEL(mob_proto +i)*2)+3);
  else
    mob_proto[i].points.hitroll = MIN(100, t[1]);
  
  for (j = 0; j < ARMOR_LIMIT; j++) {
    if (t[2] < 0 || t[2] >= 100)
      mob_proto[i].points.armor[j] = number(0, 100);
    else
      mob_proto[i].points.armor[j] = MAX((t[2]-number(0, 5)), 0);
  }
  
  /* max hit = 0 is a flag that H, M, V is xdy+z */
  mob_proto[i].points.max_hit = 0;
  mob_proto[i].points.hit     = t[3];
  mob_proto[i].points.mana    = t[4];
  mob_proto[i].points.move    = t[5];
  
  mob_proto[i].points.max_mana = 10;
  mob_proto[i].points.max_move = 50;
  
  mob_proto[i].mob_specials.damnodice   = t[6];
  mob_proto[i].mob_specials.damsizedice = t[7];
  mob_proto[i].points.damroll           = t[8];
  
  get_line(mob_f, line);
  sscanf(line, " %d %d ", t, t + 1);
  GET_GOLD(mob_proto + i) = t[0];
  GET_EXP(mob_proto + i) = t[1];
  
  get_line(mob_f, line);
  if (sscanf(line, " %d %d %d %d ", t, t + 1, t + 2, t + 3) < 3) {
    fprintf(stderr, "Format error in mob #%ld, second line after S flag\n"
            "...expecting line of form '# # #'\n", nr);
  }
  
  mob_proto[i].char_specials.position   = t[0];
  mob_proto[i].mob_specials.default_pos = t[1];
  if (mob_proto[i].mob_specials.default_pos <= POS_STUNNED)
    mob_proto[i].mob_specials.default_pos = POS_STANDING;
  if (mob_proto[i].char_specials.position <= POS_STUNNED)
    mob_proto[i].char_specials.position = POS_STANDING;
  
  mob_proto[i].player.sex               = t[2];
  
  mob_proto[i].player.race      = UNDEFINED_RACE;
  mob_proto[i].player.weight    = 200;
  mob_proto[i].player.height    = 72;
  
  for (j = 0; j < 4; j++)
    GET_COND(mob_proto + i, j) = -1;
  
  /*
   * these are now save applies; base save numbers for MOBs are now from
   * the warrior save table.
   */
  for (j = 0; j < 5; j++)
    GET_SAVE(mob_proto + i, j) = 0;
}


void parse_fuzzy_mob(FILE *mob_f, unsigned long i, unsigned long nr)
{
  int j, t[10];
  char line[256];

  assert( mob_f != NULL );
  assert( i < num_of_mobs );

  mob_proto[i].real_abils.str   = number(5, 20);
  mob_proto[i].real_abils.intel = number(5, 20);
  mob_proto[i].real_abils.wis   = number(5, 20);
  mob_proto[i].real_abils.dex   = number(5, 20);
  mob_proto[i].real_abils.con   = number(5, 20);
  mob_proto[i].real_abils.cha   = number(5, 20);
  mob_proto[i].real_abils.will  = number(5, 20);

  if (mob_proto[i].real_abils.str >= 18)
    mob_proto[i].real_abils.str_add = number(0, 99);

  get_line(mob_f, line);
  if (sscanf(line, " %d %d 0 0d0+0 0d0+0\n", t, t + 1) != 2) {
    fprintf(stderr, "Format error in mob #%ld, first line after A flag\n"
            "...expecting line of form '# # 0 0d0+0 0d0+0'\n", nr);
    exit(1);
  }

  GET_LEVEL(mob_proto + i)    = t[0];
  GET_RACE(mob_proto + i)     = t[1];

  mob_proto[i].points.hitroll = (number(5, 95));
  for (j = 0; j < ARMOR_LIMIT; j++)
    mob_proto[i].points.armor[j]   = (number(0, 90));

  /* max hit = 0 is a flag that H, M, V is xdy+z */
  mob_proto[i].points.max_hit = 0;

  mob_proto[i].points.hit =
    number( races[ i ].points.points.min_hp,
            races[ i ].points.points.max_hp ) +
    ( GET_LEVEL( mob_proto + i ) *
      number( races[ i ].points.points.min_lhp,
              races[ i ].points.points.max_lhp ) );
  mob_proto[i].points.mana =
    number( races[ i ].points.points.min_ma,
            races[ i ].points.points.max_ma ) +
    ( GET_LEVEL( mob_proto + i ) *
      number( races[ i ].points.points.min_lma,
              races[ i ].points.points.max_lma ) );
  mob_proto[i].points.move =
    number( races[ i ].points.points.min_mo,
            races[ i ].points.points.max_mo ) +
    ( GET_LEVEL( mob_proto + i ) *
      number( races[ i ].points.points.min_lmo,
              races[ i ].points.points.max_lmo ) );
  mob_proto[i].points.hitroll =
    number( races[ i ].points.points.min_hi,
            races[ i ].points.points.max_hi ) +
    ( GET_LEVEL( mob_proto + i ) *
      number( 1, races[ i ].points.points.min_hi ) );
  mob_proto[i].points.damroll =
    number( races[ i ].points.points.min_da,
            races[ i ].points.points.max_da ) +
    ( GET_LEVEL( mob_proto + i ) *
      number( 1, races[ i ].points.points.min_da ) );
  
  mob_proto[i].mob_specials.damnodice   = number(1, GET_LEVEL( mob_proto + i ));
  mob_proto[i].mob_specials.damsizedice = number(2, GET_LEVEL( mob_proto + i ));

  mob_proto[i].points.max_mana = mob_proto[i].points.mana;
  mob_proto[i].points.max_move = mob_proto[i].points.move;

  if (GET_LEVEL(mob_proto + i) <= 50)
    GET_EXP(mob_proto + i) = (exp_needed(GET_LEVEL(mob_proto + i))/6);
  else
    GET_EXP(mob_proto + i) = (exp_needed(40) + (exp_needed(25)*100000)/4);

  if (IS_HUMANOID(mob_proto + i))
    GET_GOLD(mob_proto + i) = (GET_LEVEL(mob_proto + i)*number(1, 50));
  else if (IS_DRAGON(mob_proto + i))
    GET_GOLD(mob_proto + i) = (GET_LEVEL(mob_proto + i)*number(100, 200));
  else
    GET_GOLD(mob_proto + i) = 0;

  get_line(mob_f, line);
  if (sscanf(line, " %d 0\n", t) != 1) 
    fprintf(stderr, "Format error in mob #%ld, second line after A flag\n"
            "...expecting line of form '# 0'\n", nr);
  mob_proto[i].mob_specials.attack_type = t[0];

  get_line(mob_f, line);
  if (sscanf(line, " %d %d %d\n", t, t + 1, t + 2) != 3) {
    fprintf(stderr, "Format error in mob #%ld, third line after A flag\n"
            "...expecting line of form '# # #'\n", nr);
  }

  mob_proto[i].char_specials.position   = t[0];
  mob_proto[i].mob_specials.default_pos = t[1];
  mob_proto[i].player.sex               = t[2];

  for (j = 0; j < 4; j++)
    GET_COND(mob_proto + i, j) = -1;
  
  for (j = 0; j < 5; j++)
    GET_SAVE(mob_proto + i, j) = saving_throws( mob_proto + i, j );

  mob_proto[i].mob_specials.sounds       = NULL;
  mob_proto[i].mob_specials.distant_snds = NULL;
}



void parse_loud_mob(FILE *mob_f, int i, long int nr)
{
  char line[256];

  parse_simple_mob(mob_f, i, nr);

  while (get_line(mob_f, line)) {
    if (!strcmp(line, "L"))     /* end of the loud section */
      return;
    else if (*line == '#') {    /* we've hit the next mob, maybe? */
      fprintf(stderr, "Unterminated L section in mob #%ld\n", nr);
      exit(1);
    } else {
      /* Read the sound strings */
      mob_proto[i].mob_specials.sounds       = str_dup(line);
      mob_proto[i].mob_specials.distant_snds = str_dup(line);
    }
  }
  
  fprintf(stderr, "Unexpected end of file reached after mob #%ld\n", nr);
  exit(1);
}


/*
 * interpret_espec is the function that takes espec keywords and values
 * and assigns the correct value to the mob as appropriate.  Adding new
 * e-specs is absurdly easy -- just add a new CASE statement to this
 * function!  No other changes need to be made anywhere in the code.
 */

#define CASE(test) if (!matched && !str_cmp(keyword, test) && (matched = 1))
#define RANGE(low, high) (num_arg = MAX((low), MIN((high), (num_arg))))

void interpret_espec(char *keyword, char *value, long i, long nr)
{
  int num_arg, matched = 0;

  num_arg = atoi(value);

  CASE("Race") {
    RANGE(0, NUM_RACES);
    mob_proto[i].player.race = num_arg;
  }

  CASE("BareHandAttack") {
    RANGE(0, 99);
    mob_proto[i].mob_specials.attack_type = num_arg;
  }

  CASE("Str") {
    RANGE(3, 25);
    mob_proto[i].real_abils.str = num_arg;
  }

  CASE("StrAdd") {
    RANGE(0, 100);
    mob_proto[i].real_abils.str_add = num_arg;    
  }

  CASE("Int") {
    RANGE(3, 25);
    mob_proto[i].real_abils.intel = num_arg;
  }

  CASE("Wis") {
    RANGE(3, 25);
    mob_proto[i].real_abils.wis = num_arg;
  }

  CASE("Dex") {
    RANGE(3, 25);
    mob_proto[i].real_abils.dex = num_arg;
  }

  CASE("Con") {
    RANGE(3, 25);
    mob_proto[i].real_abils.con = num_arg;
  }

  CASE("Cha") {
    RANGE(3, 25);
    mob_proto[i].real_abils.cha = num_arg;
  }

  CASE("Will") {
    RANGE(3, 25);
    mob_proto[i].real_abils.will = num_arg;
  }

  CASE("Height") {
    RANGE(1, 32000);
    mob_proto[i].player.height = num_arg;
  }

  CASE("Weight") {
    RANGE(1, 32000);
    mob_proto[i].player.weight = num_arg;
  }

  if (!matched) {
    fprintf(stderr, "Warning: unrecognized espec keyword %s in mob #%ld\n",
	    keyword, nr);
  }    
}

#undef CASE
#undef RANGE

void parse_espec(char *buf, long i, long nr)
{
  char *ptr;

  if ((ptr = strchr(buf, ':')) != NULL) {
    *(ptr++) = '\0';
    while (isspace((int)*ptr))
      ptr++;
  } else
    ptr = "";

  interpret_espec(buf, ptr, i, nr);
}


void parse_enhanced_mob(FILE *mob_f, long i, long nr)
{
  char line[256];

  parse_simple_mob(mob_f, i, nr);

  while (get_line(mob_f, line)) {
    if (!strcmp(line, "E"))     /* end of the ehanced section */
      return;
    else if (*line == '#') {    /* we've hit the next mob, maybe? */
      fprintf(stderr, "Unterminated E section in mob #%ld\n", nr);
      exit(1);
    } else
      parse_espec(line, i, nr);
  }

  fprintf(stderr, "Unexpected end of file reached after mob #%ld\n", nr);
  exit(1);
}

char fread_letter(FILE *fp)
{
   char c;

   do {
     c = getc(fp);
   } while (isspace((int)c));
   return c;
}

void parse_mobile(FILE * mob_f, unsigned long nr)
{
  static unsigned long i = 0;
  int j, t[10];
  char line[256], *tmpptr, letter;
  char f1[128], f2[128];

  assert( mob_f != NULL );
  assert( i < num_of_mobs );

  memset( mob_index + i, 0, sizeof(struct index_data *) );
  memset( mob_proto + i, 0, sizeof(struct char_data *) );

  mob_index[i].vnumber = nr;
  mob_index[i].number  = 0;
  mob_index[i].func    = NULL;

  clear_char(mob_proto + i);

  mob_proto[i].player_specials = &dummy_mob;
  sprintf(buf2, "mob vnum %ld", nr);

  /***** String data *** */
  if ((mob_proto[i].player.name = fread_string(mob_f, buf2)) == NULL) {
    fprintf(stderr, "Null mob name or format error at or near %s\n", buf2);
    exit(1);
  }
  tmpptr = mob_proto[i].player.short_descr = fread_string(mob_f, buf2);
  if (tmpptr && *tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);
  mob_proto[i].player.long_descr  = fread_string(mob_f, buf2);
  mob_proto[i].player.description = fread_string(mob_f, buf2);
  mob_proto[i].player.title       = NULL;

  /* *** Numeric data *** */
  get_line(mob_f, line);
  sscanf(line, "%s %s %d %c", f1, f2, t + 2, &letter);
  MOB_FLAGS(mob_proto + i) = asciiflag_conv(f1);
  SET_BIT(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
  AFF_FLAGS(mob_proto + i) = asciiflag_conv(f2);
  GET_ALIGNMENT(mob_proto + i) = t[2];

  mob_proto[i].mob_specials.mob_type = letter;

  switch (letter) {
  case 'S':     /* Simple monsters */
    parse_simple_mob(mob_f, i, nr);
    break;
  case 'E':     /* Circle3 Enhanced monsters */
    parse_enhanced_mob(mob_f, i, nr);
    break;
  case 'A':
    parse_fuzzy_mob(mob_f, i, nr);
    break;
  case 'L':
    parse_loud_mob(mob_f, i, nr);
    break;
  /* add new mob types here.. */
  default:
    fprintf(stderr, "Unsupported mob type '%c' in mob #%ld\n", letter, nr);
    exit(1);
    break;
  }

  mob_proto[i].aff_abils = mob_proto[i].real_abils;

  for (j = 0; j < NUM_WEARS; j++)
    mob_proto[i].equipment[j] = NULL;

  mob_proto[i].nr   = i;
  mob_proto[i].desc = NULL;

  letter = fread_letter(mob_f);
  if (letter == '>') {
     ungetc(letter, mob_f);
     (void) mprog_read_programs(mob_f, &mob_index[i]);
  } else
     ungetc(letter, mob_f);

  /* On the fly spec_procs */
#if 0
  switch (GET_RACE(mob_proto + i)) {
    case RACE_KENDER:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = thief;
      break;
    case RACE_ANIMAL:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = fido;
      break;
    case RACE_COPPER_DRAGON:
    case RACE_RED_DRAGON:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = breath_fire;
      break;
    case RACE_BRONZE_DRAGON:
    case RACE_BLUE_DRAGON:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = breath_lightning;
      break;
    case RACE_BLACK_DRAGON:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = breath_gas;
      break;
    case RACE_GREEN_DRAGON:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = breath_acid;
      break;
    case RACE_SILVER_DRAGON:
    case RACE_WHITE_DRAGON:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = breath_frost;
      break;
    case RACE_CHROMATIC:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = breath_any;
      break;
    case RACE_GOLD_DRAGON:
    case RACE_PLATINUM_DRAGON:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = old_dragon;
      break;
    case RACE_ARACHNID:
    case RACE_SERPENT:
      if (!IS_SET(MOB_FLAGS(mob_proto + i), MOB_SPEC))
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_SPEC);
      mob_index[i].func    = snake;
      break;
    default:
      break;
  }
#endif

  top_of_mobt = i++;
}


/* sets up the bonuses for material types */
void material_bonus(struct obj_data *obj, int k)
{
   switch (material_list[k].bonus_type) {     /* item type */
      case ITEM_LIGHT:
	GET_OBJ_VAL(obj, 2) = material_list[k].bonus_mod;
	break;
      case ITEM_SCROLL:
      case ITEM_WAND:
      case ITEM_STAFF:
      case ITEM_POTION:
	GET_OBJ_VAL(obj, 0) += material_list[k].bonus_mod;
	break;
      case ITEM_WEAPON:
      case ITEM_FIREWEAPON:
	GET_OBJ_VAL(obj,  1) += material_list[k].bonus_mod;
	break;
      case ITEM_MISSILE:
      case ITEM_TREASURE:
      case ITEM_WORN:
      case ITEM_OTHER:
      case ITEM_TRASH:
      case ITEM_TRAP:
      case ITEM_NOTE:
      case ITEM_KEY:
      case ITEM_PEN:
      case ITEM_BOAT:
      case ITEM_VEHICLE:
      default:
	/* do nothing */
	break;
      case ITEM_ARMOR:
	GET_OBJ_VAL(obj,  0) += material_list[k].bonus_mod;
	break;
      case ITEM_CONTAINER:
      case ITEM_DRINKCON:
      case ITEM_FOUNTAIN:
      case ITEM_FOOD:
      case ITEM_MONEY:
	GET_OBJ_VAL(obj,  0) += material_list[k].bonus_mod;
	break;
   }
   return;
}

/* randomizes the materials and/or color of the object */
void obj_strings(struct obj_data *obj)
{
   char  buf[MAX_STRING_LENGTH];
   const char *str;
   const char *i;
   char  *point;
   int   pass, j, k;

   char  c_str[50];
   char  m_str[50];

   j = number(0, MAX_COLOR_LIST-1);
   if (GET_OBJ_TYPE(obj) != ITEM_WEAPON)
      k = number(0, MAX_MATERIAL_LIST-1);
   else
      k = number(8, MAX_MATERIAL_LIST-1);

   sprintf(c_str, "%s", color_list[j] );
   sprintf(m_str, "%s", material_list[k].name);

   for (pass = 0; pass < 3; pass++ ) {
       switch( pass ) {
	 case 0:
	   if (obj->name)
	     str = str_dup(obj->name);
	   else str = str_dup("\0");
	   break;
	 case 1:
	   if (obj->short_description)
	     str = str_dup(obj->short_description);
	   else str = str_dup("\0");
	   break;
	 case 2:
	   if (obj->description)
	     str = str_dup(obj->description);
	   else str = str_dup("\0");
	   break;
	 default:
	   if (obj->short_description)
	     str = str_dup(obj->short_description);
	   else str = str_dup("\0");
	   sprintf(buf, "ERROR obj_strings: bad pass, obj: %ld.", GET_OBJ_VNUM(obj));
	   log(buf);
	   break;
       }

       point = buf;

       while (*str != '\0') {
	  if (*str != '$') {
	     *point++ = *str++;
	     continue;
	  }
	  ++str;
	  switch(*str) {
	     default:
	       i = " ";
	       break;
	     case 'c':
	       i = c_str;
	       break;
	     case 'm':
	       i = m_str;
	       if (pass == 2) {
		  GET_OBJ_VAL(obj, 7) = k;
		  GET_OBJ_COST(obj) *= material_list[k].value_mod;
		  material_bonus(obj, k);
	       }
	       break;
	  }
	  ++str;
	  while ((*point = *i) != '\0')
	     ++point, ++i;
       }
       *point = '\0';

       switch (pass) {
	 default:
	   break;
	 case 0:
	   obj->name        = str_dup( buf );
	   break;
	 case 1:
	   obj->short_description = str_dup( buf );
	   break;
	 case 2:
	   obj->description = str_dup( buf );
	   break;
       }
   }
   return;
}


/* read all objects from obj file; generate index and prototypes */
char *parse_object(FILE * obj_f, long nr)
{
  static long i = 0, retval;
  static char line[256];
  long t[10], j;
  char *tmpptr;
  char f1[256], f2[256];
  struct extra_descr_data *new_descr;

  obj_index[i].vnumber = nr;
  obj_index[i].number  = 0;
  obj_index[i].func    = NULL;

  clear_object(obj_proto + i);
  obj_proto[i].in_room     = NOWHERE;
  obj_proto[i].item_number = i;

  sprintf(buf2, "object #%ld", nr);

  /* *** string data *** */
  if ((obj_proto[i].name = fread_string(obj_f, buf2)) == NULL) {
    fprintf(stderr, "Null obj name or format error at or near %s\n", buf2);
    exit(1);
  }
  tmpptr = obj_proto[i].short_description = fread_string(obj_f, buf2);
  if (*tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);

  tmpptr = obj_proto[i].description = fread_string(obj_f, buf2);
  if (tmpptr && *tmpptr)
    *tmpptr = UPPER(*tmpptr);
  obj_proto[i].action_description = fread_string(obj_f, buf2);

  /* *** numeric data *** */
  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, " %ld %s %s", t, f1, f2)) != 3) {
    fprintf(stderr, "Format error in first numeric line (expecting 3 args, got %ld), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.type_flag   = t[0];
  obj_proto[i].obj_flags.extra_flags = asciiflag_conv(f1);
  obj_proto[i].obj_flags.wear_flags  = asciiflag_conv(f2);
  if (IS_SET(obj_proto[i].obj_flags.wear_flags, (1 << 13)))
    obj_proto[i].obj_flags.wear_flags -= (1 << 13);

  /* clears the t values for the obj */
  for (j = 0; j < 10; j++)
    t[j] = 0;

  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
                       t, t + 1, t + 2, t + 3, t + 4,
                       t + 5, t + 6, t + 7, t + 8)) < 4) {
    fprintf(stderr, "Format error in second numeric line " \
            "(expecting atleast 4 args, got %ld), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.value[0] = t[0];

  /* check if it's a fireweapon or ammo if the values are valid */
  if (obj_proto[i].obj_flags.type_flag == ITEM_FIREWEAPON ||
      obj_proto[i].obj_flags.type_flag == ITEM_MISSILE) {
    if (!(obj_proto[i].obj_flags.value[0] < NUM_AMMO_TYPES)) {
      fprintf(stderr,
              "Format error in second numeric line "\
              "(object is fireweapon/missle with invalid ammo type: %ld), %s\n",
              obj_proto[i].obj_flags.value[0], buf2);
      exit(1);
    }
  }

  if (obj_proto[i].obj_flags.type_flag == ITEM_POTION ||
      obj_proto[i].obj_flags.type_flag == ITEM_SCROLL) {
    obj_proto[i].obj_flags.value[1] = t[1] < 51 ? (t[1]+50) : t[1];
    obj_proto[i].obj_flags.value[2] = t[2] < 51 ? (t[2]+50) : t[2];
    obj_proto[i].obj_flags.value[3] = t[3] < 51 ? (t[3]+50) : t[3];
  } else {
    obj_proto[i].obj_flags.value[1] = t[1];
    obj_proto[i].obj_flags.value[2] = t[2];
    obj_proto[i].obj_flags.value[3] = t[3];
  }
  obj_proto[i].obj_flags.value[4] = t[4];
  obj_proto[i].obj_flags.value[5] = t[5];
  obj_proto[i].obj_flags.value[6] = t[6];
  obj_proto[i].obj_flags.value[7] = t[7];
  if (retval == 9)
    obj_proto[i].obj_flags.value[8] = t[8];
  else
    obj_proto[i].obj_flags.value[8] =
      ITEM_SIZE_SMALL | ITEM_SIZE_MEDIUM | ITEM_SIZE_LARGE;

  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%ld %ld %ld", t, t + 1, t + 2)) != 3) {
    fprintf(stderr,
            "Format error in third numeric line " \
            "(expecting 3 args, got %ld), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.weight = t[0];
  obj_proto[i].obj_flags.cost   = t[1];
  obj_proto[i].obj_flags.cost_per_day = t[2];

  /* check to make sure that weight of containers exceeds curr. quantity */
  if (obj_proto[i].obj_flags.type_flag == ITEM_DRINKCON ||
      obj_proto[i].obj_flags.type_flag == ITEM_FOUNTAIN) {
    if (obj_proto[i].obj_flags.weight < obj_proto[i].obj_flags.value[1])
      obj_proto[i].obj_flags.weight = obj_proto[i].obj_flags.value[1] + 5;
  }

  /* random object strings and adjustments */
  obj_strings(obj_proto + i);

  /* *** extra descriptions and affect fields *** */
  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    obj_proto[i].affected[j].location = APPLY_NONE;
    obj_proto[i].affected[j].modifier = 0;
  }

  strcat(buf2, ", after numeric constants (expecting E/A/#xxx)");
  j = 0;

  for (;;) {
    if (!get_line(obj_f, line)) {
      fprintf(stderr, "Format error in %s\n", buf2);
      exit(1);
    }
    switch (*line) {
    case 'E':
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(obj_f, buf2);
      new_descr->description = fread_string(obj_f, buf2);
      new_descr->next = obj_proto[i].ex_description;
      obj_proto[i].ex_description = new_descr;
      break;
    case 'A':
      if (j >= MAX_OBJ_AFFECT) {
	fprintf(stderr, "Too many A fields (%d max), %s\n", MAX_OBJ_AFFECT, buf2);
	exit(1);
      }
      get_line(obj_f, line);
      sscanf(line, " %ld %ld ", t, t + 1);
      obj_proto[i].affected[j].location = t[0];
      obj_proto[i].affected[j].modifier = t[1];
      j++;
      break;
    case '$':
    case '#':
      top_of_objt = i++;
      return line;
      break;
    default:
      fprintf(stderr, "Format error in %s\n", buf2);
      exit(1);
      break;
    }
  }
}


#define Z       zone_table[zone]

/* load the zone table and command tables */
void load_zones(FILE * fl, char *zonename)
{
  static long zone = 0;
  long cmd_no = 0, num_of_cmds = 0, line_num = 0, tmp, error;
  char *ptr, buf[256], zname[256];

  strcpy(zname, zonename);

  while (get_line(fl, buf))
    num_of_cmds++;              /* this should be correct within 3 or so */
  rewind(fl);

  if (num_of_cmds == 0) {
    fprintf(stderr, "%s is empty!\n", zname);
    exit(0);
  } else
    CREATE(Z.cmd, struct reset_com, num_of_cmds);

  line_num += get_line(fl, buf);

  if (sscanf(buf, "#%d", &Z.number) != 1) {
    fprintf(stderr, "Format error in %s, line %ld\n", zname, line_num);
    exit(0);
  }
  sprintf(buf2, "beginning of zone #%d", Z.number);

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
    *ptr = '\0';
  Z.name = str_dup(buf);

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
     *ptr = '\0';
  Z.builder = str_dup(buf);             /* The Builder of the Area */

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
     *ptr = '\0';
  Z.levelrec = str_dup(buf);             /* Level recommendations for area */

  line_num += get_line(fl, buf);
  if (sscanf(buf, " %ld %d ", &Z.recall_loc, &Z.pkill) != 2) {
     fprintf(stderr, "Format error in recall location (or pkill) of %s", zname);
     exit(0);
  }

  line_num += get_line(fl, buf);
  if (sscanf(buf, " %ld %d %d ", &Z.top, &Z.lifespan, &Z.reset_mode) != 3) {
    fprintf(stderr, "Format error in 3-constant line of %s", zname);
    exit(0);
  }
  cmd_no = 0;

  for (;;) {
    if ((tmp = get_line(fl, buf)) == 0) {
      fprintf(stderr, "Format error in %s - premature end of file\n", zname);
      exit(0);
    }
    line_num += tmp;
    ptr = buf;
    skip_spaces(&ptr);

    if ((ZCMD.command = *ptr) == '*')
      continue;

    ptr++;

    if (ZCMD.command == 'S' || ZCMD.command == '$') {
      ZCMD.command = 'S';
      break;
    }
    error = 0;
    if (strchr("MOEPD", ZCMD.command) == NULL) {        /* a 3-arg command */
      error = (sscanf(ptr, " %ld %ld %ld %d", &tmp, &ZCMD.arg1, &ZCMD.arg2, &ZCMD.chance));
      if (error != 4)
	ZCMD.chance = 100;
      if (error < 3)
	error = 1;
      else
        error = 0;
    } else {
      error = (sscanf(ptr, " %ld %ld %ld %ld %d ", &tmp, &ZCMD.arg1, &ZCMD.arg2,
		 &ZCMD.arg3, &ZCMD.chance));
      if (error != 5)
	ZCMD.chance = 100;
      if (error < 4)
	error = 1;
      else
        error = 0;
    }

    ZCMD.if_flag = tmp ? TRUE : FALSE;

    if (error) {
      fprintf(stderr, "Format error in %s, line %ld: '%s'\n", zname, line_num, buf);
      exit(0);
    }
    ZCMD.line = line_num;
    cmd_no++;
  }
  top_of_zone_table = zone++;
}

#undef Z

void get_one_line(FILE *fl, char *buf)
{
  if (fgets(buf, READ_SIZE, fl) == NULL) {
    // EOF?
    strcpy(buf, "$\n");
  }

  buf[strlen(buf) - 1] = '\0'; /* take off the trailing \n */
}

#ifndef OLD_HELP
/* 0 for HELP, 1 for WIZHELP */
void load_help(FILE *fl, int mode)
{
  char key[READ_SIZE+1], next_key[READ_SIZE+1], entry[32384];
  char line[READ_SIZE+1], *scan;
  struct help_index_element el;

  /* NOTE each help entry must be followed by a # and the file must be
     terminated with a $ -- no exceptions! */
  
  /* get the first keyword line */
  get_one_line(fl, key);
  while (*key != '$') {
    /* read in the corresponding help entry */
    strcpy(entry, strcat(key, "\r\n"));
    get_one_line(fl, line);
    while (*line != '#') {
      strcat(entry, strcat(line, "\r\n"));
      get_one_line(fl, line);
    }

    /* now, add the entry to the index with each keyword on the keyword line */
    el.duplicate = 0;
    el.entry = str_dup(entry);
    scan = one_word(key, next_key);
    while (*next_key) {
      el.keyword = str_dup(next_key);
      if (mode == 0)       /* help */
	 help_table[top_of_helpt++] = el;
      else if (mode == 1)  /* wizhelp */
	 wizhelp_table[top_of_wizhelpt++] = el;
      el.duplicate++;
      scan = one_word(scan, next_key);
    }

    /* get next keyword line (or $) */
    get_one_line(fl, key);
  }
}


int hsort(const void *a, const void *b)
{
  struct help_index_element *a1, *b1;

  a1 = (struct help_index_element *) a;
  b1 = (struct help_index_element *) b;

  return (str_cmp(a1->keyword, b1->keyword));
}
#endif

/*************************************************************************
*  procedures for resetting, both play-time and boot-time                *
*********************************************************************** */



long vnum_mobile(char *searchname, struct char_data * ch)
{
  long nr, found = 0;

  for (nr = 0; nr <= top_of_mobt; nr++) {
    if (isname(searchname, mob_proto[nr].player.name)) {
      sprintf(buf, "%4ld. [%ld] %s\r\n", ++found,
	      mob_index[nr].vnumber,
	      mob_proto[nr].player.short_descr);
      send_to_char(buf, ch);
    }
  }

  return (found);
}



long vnum_object(char *searchname, struct char_data * ch)
{
  long nr, found = 0;

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (isname(searchname, obj_proto[nr].name)) {
      sprintf(buf, "%4ld. [%ld] %s\r\n", ++found,
	      obj_index[nr].vnumber,
	      obj_proto[nr].short_description);
      send_to_char(buf, ch);
    }
  }
  return (found);
}


/* create a character, and add it to the char list */
struct char_data *create_char(void)
{
  struct char_data *ch;

  CREATE(ch, struct char_data, 1);
  clear_char(ch);
  ch->next = character_list;
  character_list = ch;

  return ch;
}


/* create a new mobile from a prototype */
struct char_data *read_mobile(long nr, int type)
{
  long i;
  struct char_data *mob;

  if (type == VNUMBER) {
    if ((i = real_mobile(nr)) < 0) {
      sprintf(buf, "Mobile (V) %ld does not exist in database.", nr);
      return (0);
    }
  } else
    i = nr;

  CREATE(mob, struct char_data, 1);
  clear_char(mob);
  *mob = mob_proto[i];
  mob->next = character_list;
  character_list = mob;

  if (!mob->points.max_hit) {
    mob->points.max_hit = dice(mob->points.hit, mob->points.mana) +
      mob->points.move;
  } else
    mob->points.max_hit = number(mob->points.hit, mob->points.mana);

  mob->points.hit = mob->points.max_hit;
  mob->points.mana = mob->points.max_mana;
  mob->points.move = mob->points.max_move;

  mob->player.time.birth = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon = time(0);

  mob_index[i].number++;

  return mob;
}


/* create an object, and add it to the object list */
struct obj_data *create_obj(void)
{
  struct obj_data *obj;

  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  obj->next = object_list;
  object_list = obj;

  return obj;
}


/* create a new object from a prototype */
struct obj_data *read_object(long nr, int type)
{
  struct obj_data *obj;
  long i;

  if (nr < 0) {
    log("SYSERR: trying to create obj with negative num!");
    return NULL;
  }
  if (type == VNUMBER) {
    if ((i = real_object(nr)) < 0) {
      sprintf(buf, "Object (V) %ld does not exist in database.", nr);
      return NULL;
    }
  } else
    i = nr;

  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  *obj = obj_proto[i];
  obj->next = object_list;
  object_list = obj;

  obj_index[i].number++;

  return obj;
}



#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
  long i;
  struct reset_q_element *update_u, *temp;
  static int timer = 0;
  char buf[128];
  char buf2[128];
  struct char_data *tch = NULL;
  unsigned long strike_loc = 0;
  int dam = 0;

  void die(struct char_data *ch);
  
  /* jelson 10/22/92 */
  if (((++timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60) {
    /* one minute has passed */
    /*
     * NOT accurate unless PULSE_ZONE is a multiple of PASSES_PER_SEC or a
     * factor of 60
     */

    timer = 0;

    /* since one minute has passed, increment zone ages */
    for (i = 0; i <= top_of_zone_table; i++) {
      if (zone_table[i].age < zone_table[i].lifespan &&
	  zone_table[i].reset_mode)
	(zone_table[i].age)++;

      if (zone_table[i].age >= zone_table[i].lifespan &&
	  zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
	/* enqueue zone */

	CREATE(update_u, struct reset_q_element, 1);

	update_u->zone_to_reset = i;
	update_u->next = 0;

	if (!reset_q.head)
	  reset_q.head = reset_q.tail = update_u;
	else {
	  reset_q.tail->next = update_u;
	  reset_q.tail = update_u;
	}

	zone_table[i].age = ZO_DEAD;
      }

      /* Perform weather checks */
      // the algorithm below is NOT right, I would rather pick a
      // random room in the zone, instead of picking a random room and
      // then determining if it is in the zone.
      strike_loc = number( 0, top_of_world );

      if ( world[ strike_loc ].zone == zone_table[i].number ) {
        struct weather_data weather_info =
          zone_table[i].weather_info;
      
        dam = 0;
        strcpy( buf, "" );
        strcpy( buf2, "" );

        // no funny weather indoors
        if (!ROOM_FLAGGED(strike_loc, ROOM_INDOORS)) {
          // find a character in that room
          tch = world[strike_loc].people;
          while ( tch != NULL ) {
            if (IS_IMMORTAL(tch))
              continue;

            if ( number( 1, 2 ) != 2 )
              break;

            tch = tch->next_in_room;
          }

          if ( tch == NULL )
            continue;
          
          switch ( weather_info.sky ) {
          case SKY_THUNDERSTORM:
            if ( number( 1, 10 ) != 10 ) {
              dam = dice(15, 15);
              sprintf( buf, "A lightning bolt strikes $n!" );
              sprintf( buf2, "You are struck by a lightning bolt!\r\n" );
            } else {
              sprintf( buf, "You jump as lightning strikes near by." );
              sprintf( buf2, "You jump as lightning strikes near by.\r\n" );
            }
            break;
          case SKY_TORNADO:
            if ( number( 1, 10 ) < 8 ) {
              dam = dice(6, 10);
              sprintf( buf, "A tornado touches down, throwing $n around!" );
              sprintf( buf2, "You are tossed around by a tornado!\r\n" );
            } else {
              sprintf( buf, "The wind picks up as a tornado passes overhead." );
              sprintf( buf2, "The wind picks up as a tornado passes overhead.\r\n" );
            }
            break;
          case SKY_BLIZZARD:
            if ( number( 1, 10 ) < 9 ) {
              dam = dice(8, 12);
              sprintf( buf, "A blizzard blows through the area covering $n with hail and snow!" );
              sprintf( buf2, "A blizzard rips right through you!\r\n" );
            } else {
              sprintf( buf, "You buckle down and hold your ground as a blizzard blows through." );
              sprintf( buf2, "You buckle down and hold your ground as a blizzard blows through.\r\n" );
            }
            break;
          default:
            // nothing happens
            break;
          }
          
          // tch is the character to get it,
          // buf is the message to the room,
          // buf2 is the message to the char,
          if ( tch != NULL &&
               strcmp( buf, "" ) &&
               strcmp( buf2, "" ) ) {
            assert( tch != NULL );
            
            if ( dam > 0 ) {
              GET_HIT(tch) -= dam;
              update_pos( tch );
            }
            act( buf, TRUE, tch, 0, 0, TO_ROOM );
            send_to_char( buf2, tch );
            // check if the character died
            if ( GET_POS(tch) == POS_DEAD )
              die( tch );
          }
        }
      }
    }
  }     /* end - one minute has passed */


  /* dequeue zones (if possible) and reset */
  /* this code is executed every 10 seconds (i.e. PULSE_ZONE) */
  for (update_u = reset_q.head; update_u; update_u = update_u->next)
    if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
	is_empty(update_u->zone_to_reset)) {
      reset_zone(update_u->zone_to_reset);
      sprintf(buf, "Auto zone reset: %s",
	      zone_table[update_u->zone_to_reset].name);
      mudlog(buf, CMP, LVL_GOD, FALSE);
      /* dequeue */
      if (update_u == reset_q.head)
	reset_q.head = reset_q.head->next;
      else {
	for (temp = reset_q.head; temp->next != update_u;
	     temp = temp->next);

	if (!update_u->next)
	  reset_q.tail = temp;

	temp->next = update_u->next;
      }

      free(update_u);
      break;
    }
}

void log_zone_error(long zone, int cmd_no, char *message)
{
  char buf[256];

  sprintf(buf, "SYSERR: error in zone file: %s", message);
  mudlog(buf, NRM, LVL_GOD, TRUE);

  sprintf(buf, "SYSERR: ...offending cmd: '%c' cmd in zone #%d, line %d",
	  ZCMD.command, zone_table[zone].number, ZCMD.line);
  mudlog(buf, NRM, LVL_GOD, TRUE);
}

#define ZONE_ERROR(message) \
	{ log_zone_error(zone, cmd_no, message); last_cmd = 0; }

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
  int    cmd_no, last_cmd = 0;
  struct char_data *mob = NULL, *master = NULL;
  struct obj_data *obj, *obj_to;

  for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {

    if (ZCMD.if_flag && !last_cmd)
      continue;

    /* If the number is >= than percentage of repop -- don't load */
    if (number(1, 100) >= ZCMD.chance)
      continue;

    switch (ZCMD.command) {
    case '*':                   /* ignore command */
      last_cmd = 0;
      break;

    case 'M':                   /* read a mobile */
      if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	mob = read_mobile(ZCMD.arg1, REAL);
	char_to_room(mob, ZCMD.arg3);
	master = mob;
	last_cmd = 1;
      } else
	last_cmd = 0;
      break;
	 case 'C': /* read a mobile.  Charm them to follow prev. */
	   if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	      mob = read_mobile(ZCMD.arg1, REAL);
	      if (master != NULL) {
		 char_to_room(mob, master->in_room);
		 add_follower(mob, master);
		 SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
	      } else {
		 extract_char(mob);
		 last_cmd = 0;
	      } last_cmd = 1;
	   } else
	      last_cmd = 0;
	   break;
	 case 'H': /* hatred to char */
	   if (AddHatred(mob, ZCMD.arg1, ZCMD.arg2))
	      last_cmd = 1;
	   else
	      last_cmd = 0;
	   break;
	 case 'F': /* fear to char */
	   if (AddFears(mob, ZCMD.arg1, ZCMD.arg2))
	      last_cmd = 1;
	   else
	      last_cmd = 0;
	   break;

    case 'O':                   /* read an object */
      if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	if (ZCMD.arg3 >= 0) {
	  obj = read_object(ZCMD.arg1, REAL);
	  obj_to_room(obj, ZCMD.arg3);
	  last_cmd = 1;
	} else {
	  obj = read_object(ZCMD.arg1, REAL);
	  obj->in_room = NOWHERE;
	  last_cmd = 1;
	}
      } else
	last_cmd = 0;
      break;

    case 'P':                   /* object to object */
      if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	obj = read_object(ZCMD.arg1, REAL);
	if (!(obj_to = get_obj_num(ZCMD.arg3))) {
	  ZONE_ERROR("target obj not found");
	  break;
	}
	obj_to_obj(obj, obj_to);
	last_cmd = 1;
      } else
	last_cmd = 0;
      break;

    case 'G':                   /* obj_to_char */
      if (!mob) {
	ZONE_ERROR("attempt to give obj to non-existant mob");
	break;
      }
      if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	obj = read_object(ZCMD.arg1, REAL);
	obj_to_char(obj, mob);
	last_cmd = 1;
      } else
	last_cmd = 0;
      break;

    case 'E':                   /* object to equipment list */
      if (!mob) {
	ZONE_ERROR("trying to equip non-existant mob");
	break;
      }
      if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	if (ZCMD.arg3 < 0 || ZCMD.arg3 >= NUM_WEARS) {
	  ZONE_ERROR("invalid equipment pos number");
	} else {
	  obj = read_object(ZCMD.arg1, REAL);
	  equip_char(mob, obj, ZCMD.arg3);
	  last_cmd = 1;
	}
      } else
	last_cmd = 0;
      break;

    case 'R': /* rem obj from room */
      if ((obj = get_obj_in_list_num(ZCMD.arg2, world[ZCMD.arg1].contents)) != NULL) {
	obj_from_room(obj);
	extract_obj(obj);
      }
      last_cmd = 1;
      break;


    case 'D':                   /* set state of door */
      if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS ||
	  (world[ZCMD.arg1].dir_option[ZCMD.arg2] == NULL)) {
	ZONE_ERROR("door does not exist");
      } else
	switch (ZCMD.arg3) {
	case 0:
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_CLOSED);
	  break;
	case 1:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  break;
	case 2:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_LOCKED);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  break;
	}
      last_cmd = 1;
      break;

    default:
      ZONE_ERROR("unknown cmd in reset table; cmd disabled");
      ZCMD.command = '*';
      break;
    }
  }

  zone_table[zone].age = 0;
}



/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
  struct descriptor_data *i;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected)
      if (world[i->character->in_room].zone == zone_nr)
	return 0;

  return 1;
}





/*************************************************************************
*  stuff related to the save/load player system                          *
*********************************************************************** */


long get_id_by_name(char *name)
{
  int i;

  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if ((!strcmp((player_table + i)->name, arg)) || 
        is_abbrev(arg, (player_table + i)->name))
      return ((player_table + i)->id);

  return -1;
}


char *get_name_by_id(long id)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if ((player_table + i)->id == id)
      return ((player_table + i)->name);

  return NULL;
}


/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_file_u * char_element)
{
  int player_i;

  int find_name(char *name);

  if ((player_i = find_name(name)) >= 0) {
    fseek(player_fl, (long) (player_i * sizeof(struct char_file_u)), SEEK_SET);
    fread(char_element, sizeof(struct char_file_u), 1, player_fl);
    return (player_i);
  } else
    return (-1);
}




/*
 * write the vital data of a player to the player file
 *
 * NOTE: load_room should be an *RNUM* now.  It is converted to a vnum here.
 */
void save_char(struct char_data * ch, ln_int load_room)
{
  struct char_file_u st;

  if (IS_NPC(ch) || !ch->desc || GET_PFILEPOS(ch) < 0)
    return;

  char_to_store(ch, &st);

  strncpy(st.host, ch->desc->host, HOST_LENGTH);
  st.host[HOST_LENGTH] = '\0';
  strncpy(st.email, ch->char_specials.email, MAX_TITLE_LENGTH);
  st.email[MAX_TITLE_LENGTH] = '\0';

  if (!PLR_FLAGGED(ch, PLR_LOADROOM)) {
    if (load_room == NOWHERE)
      st.player_specials_saved.load_room = NOWHERE;
    else
      st.player_specials_saved.load_room = world[load_room].number;
  }

  strcpy(st.pwd, GET_PASSWD(ch));

  fseek(player_fl, GET_PFILEPOS(ch) * sizeof(struct char_file_u), SEEK_SET);
  fwrite(&st, sizeof(struct char_file_u), 1, player_fl);
}

/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u * st, struct char_data * ch)
{
  int i;

  void set_height_weight(struct char_data *ch);
  /* to save memory, only PC's -- not MOB's -- have player_specials */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  GET_SEX(ch)         = st->sex;
  GET_RACE(ch)        = st->race;
  GET_REL(ch)         = st->assocs[0];
  GET_GUILD(ch)       = st->assocs[1];
  GET_GUILD_LEV(ch)   = st->assocs[2];
  GET_LEVEL(ch)       = st->level;
  GET_AGE_MOD(ch)     = st->race_mod;

  /* Reset a player's regen rates */
  GET_HIT_REGEN(ch) = GET_MANA_REGEN(ch) = GET_MOVE_REGEN(ch) = 0;

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

  if (GET_HEIGHT(ch) == 0 || GET_WEIGHT(ch) == 0)
    set_height_weight(ch);

  ch->real_abils             = st->abilities;
  ch->aff_abils              = st->abilities;
  ch->points                 = st->points;
  ch->char_specials.saved    = st->char_specials_saved;
  ch->player_specials->saved = st->player_specials_saved;
  POOFIN(ch)                 = str_dup(st->spoofin);
  POOFOUT(ch)                = str_dup(st->spoofout);

  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;
  /* Reset the ch's armor percentages */
  for (i = 0; i < ARMOR_LIMIT; i++)
    ch->points.armor[i] = 0;

  GET_HITROLL(ch) = st->points.hitroll;
  GET_DAMROLL(ch) = st->points.damroll;

  if (ch->player.name == NULL)
    CREATE(ch->player.name, char, strlen(st->name) + 1);
  strcpy(ch->player.name, st->name);
  strcpy(ch->player.passwd, st->pwd);

  /* Set number of followers to 0 */
  GET_FOLS(ch) = 0;

  /* Add all spell effects */
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type)
      affect_to_char(ch, &st->affected[i], FALSE);
  }
  /* Add all bad spell effects */
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected2[i].type)
      affect_to_char(ch, &st->affected2[i], TRUE);
  }
  for (i = 0; i < 12; i++)
    GET_ITEMS_STUCK(ch, i) = NULL;

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
    for (i = 0; i < 4; i++)
      GET_COND(ch, i) = 24;
    GET_COND(ch, DRUNK) = 0;
  }
  GET_POS(ch) = POS_STANDING;
}


/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data * ch, struct char_file_u * st)
{
  int i;
  struct affected_type *af;
  struct obj_data *char_eq[NUM_WEARS];

  /* Unaffect everything a character can be affected by */

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      char_eq[i] = unequip_char(ch, i);
    else
      char_eq[i] = NULL;
  }

  for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      st->affected[i] = *af;
      st->affected[i].next = 0;
      af = af->next;
    } else {
      st->affected[i].type = 0; /* Zero signifies not used */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].next = 0;
    }
  }
  /* and the bad affects */
  for (af = ch->affected2, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      st->affected2[i] = *af;
      st->affected2[i].next = 0;
      af = af->next;
    } else {
      st->affected2[i].type = 0; /* Zero signifies not used */
      st->affected2[i].duration = 0;
      st->affected2[i].modifier = 0;
      st->affected2[i].location = 0;
      st->affected2[i].bitvector = 0;
      st->affected2[i].next = 0;
    }
  }

  /*
   * remove the affections so that the raw values are stored; otherwise the
   * effects are doubled when the char logs back in.
   */

  while (ch->affected)
    affect_remove(ch, ch->affected, FALSE);
  while (ch->affected2)
    affect_remove(ch, ch->affected2, TRUE);

  if ((i >= MAX_AFFECT) && af && af->next)
    log("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

  ch->aff_abils  = ch->real_abils;

  st->birth      = ch->player.time.birth;
  st->played     = ch->player.time.played;
  st->played    += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);

  ch->player.time.played = st->played;
  ch->player.time.logon  = time(0);

  st->hometown       = ch->player.hometown;
  st->weight         = GET_WEIGHT(ch);
  st->height         = GET_HEIGHT(ch);
  st->sex            = GET_SEX(ch);
  st->race           = GET_RACE(ch);
  st->assocs[0]      = GET_REL(ch);
  st->assocs[1]      = GET_GUILD(ch);
  st->assocs[2]      = GET_GUILD_LEV(ch);
  st->level          = GET_LEVEL(ch);
  st->race_mod       = GET_AGE_MOD(ch);
  st->points.hitroll = GET_HITROLL(ch);
  st->points.damroll = GET_DAMROLL(ch);
  st->abilities      = ch->real_abils;
  st->points         = ch->points;
  st->char_specials_saved   = ch->char_specials.saved;
  st->player_specials_saved = ch->player_specials->saved;

  for (i = 0; i < ARMOR_LIMIT; i++)
     st->points.armor[i] = 0;

  if (GET_TITLE(ch))
    strcpy(st->title, GET_TITLE(ch));
  else
    *st->title = '\0';

  if (ch->player.description)
    strcpy(st->description, ch->player.description);
  else
    *st->description = '\0';

  strcpy(st->name, GET_NAME(ch));
  strncpy(st->email, ch->char_specials.email, MAX_TITLE_LENGTH);

  if (POOFIN(ch))
    strcpy(st->spoofin, POOFIN(ch));
  else
    *st->spoofin = '\0';

  if (POOFOUT(ch))
    strcpy(st->spoofout, POOFOUT(ch));
  else
    *st->spoofout = '\0';

  /* add spell and eq affections back in now */
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type)
      affect_to_char(ch, &st->affected[i], FALSE);
  }
  /* add spell and eq bad affections back in now */
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected2[i].type)
      affect_to_char(ch, &st->affected2[i], TRUE);
  }

  for (i = 0; i < NUM_WEARS; i++) {
    if (char_eq[i])
      equip_char(ch, char_eq[i], i);
  }
  if (GET_LEVEL(ch) > 5 && IS_SET(PLR_FLAGS(ch), PLR_NEWBIE))
     REMOVE_BIT(PLR_FLAGS(ch), PLR_NEWBIE);
/*   affect_total(ch); unnecessary, I think !?! */
}

void save_etext(struct char_data * ch)
{
/* this will be really cool soon */
/* Really? When? What is it supposed to do? */
}


/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
  int i;

  if (top_of_p_table == -1) {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  } else if (!(player_table = (struct player_index_element *)
	       realloc(player_table, sizeof(struct player_index_element) *
		       (++top_of_p_table + 1)))) {
    perror("create entry");
    exit(1);
  }
  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
       i++);

  return (top_of_p_table);
}



/************************************************************************
*  funcs of a (more or less) general utility nature                     *
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE * fl, char *error)
{
  char buf[MAX_STRING_LENGTH], tmp[512], *rslt;
  register char *point;
  int done = 0, length = 0, templength = 0;

  *buf = '\0';

  do {
    if (!fgets(tmp, 512, fl)) {
      fprintf(stderr, "SYSERR: fread_string: format error at or near %s\n",
	      error);
      exit(1);
    }
    /* If there is a '~', end the string; else put an "\r\n" over the '\n'. */
    if ((point = strchr(tmp, '~')) != NULL) {
      *point = '\0';
      done = 1;
    } else {
      point = tmp + strlen(tmp) - 1;
      *(point++) = '\r';
      *(point++) = '\n';
      *point = '\0';
    }
    /* Allow comments C++ style (// to EOL) */
    if (tmp[0] == '/' && tmp[1] == '/')
      continue;

    templength = strlen(tmp);

    if (length + templength >= MAX_STRING_LENGTH) {
      log("SYSERR: fread_string: string too large (db.c)");
      log(error);
      exit(1);
    } else {
      strcat(buf + length, tmp);
      length += templength;
    }
  } while (!done);

  /* allocate space for the new string and copy it */
  if (strlen(buf) > 0) {
    CREATE(rslt, char, length + 1);
    strcpy(rslt, buf);
  } else
    rslt = NULL;

  return rslt;
}


/* release memory allocated for a char struct */
void free_char(struct char_data * ch)
{
  int i;
  struct alias *a;

  void free_alias(struct alias * a);

  assert( ch != NULL );

  while ((a = GET_ALIASES(ch)) != NULL) {
    GET_ALIASES(ch) = (GET_ALIASES(ch))->next;
    free_alias(a);
  }

  if (ch->player_specials != NULL && ch->player_specials != &dummy_mob) {
    if (ch->player_specials->poofin)
      free(ch->player_specials->poofin);
    if (ch->player_specials->poofout)
      free(ch->player_specials->poofout);
    free(ch->player_specials);
    if (IS_NPC(ch))
      log("SYSERR: Mob had player_specials allocated!");
  }
  if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1)) {
    /* if this is a player, or a non-prototyped non-player, free all */
    if (GET_NAME(ch)) {
      free(GET_NAME(ch)); ch->player.name = NULL;
    }
    if (GET_RDESC(ch)) {
      free(GET_RDESC(ch)); ch->player.room_descr = NULL;
    }
    if (GET_KWDS(ch)) {
      free(GET_KWDS(ch)); ch->player.keywords = NULL;
    }
    if (GET_TITLE(ch)) {
      free(GET_TITLE(ch)); ch->player.title = NULL;
    }
    if (ch->player.short_descr) {
      free(ch->player.short_descr); ch->player.short_descr = NULL;
    }
    if (ch->player.long_descr) {
      free(ch->player.long_descr); ch->player.long_descr = NULL;
    }
    if (ch->player.description) {
      free(ch->player.description); ch->player.description = NULL;
    }
  } else if ((i = GET_MOB_RNUM(ch)) > -1) {
    /* otherwise, free strings only if the string is not pointing at proto */
    if (ch->player.name && ch->player.name !=
        mob_proto[i].player.name) {
      free(ch->player.name);  ch->player.name = NULL;
    }
    if (ch->player.title &&
        ch->player.title != mob_proto[i].player.title) {
      free(ch->player.title); ch->player.title = NULL;
    }
    if (ch->player.short_descr &&
        ch->player.short_descr != mob_proto[i].player.short_descr) {
      free(ch->player.short_descr); ch->player.short_descr = NULL;
    }
    if (ch->player.long_descr &&
        ch->player.long_descr != mob_proto[i].player.long_descr) {
      free(ch->player.long_descr); ch->player.long_descr = NULL;
    }
    if (ch->player.description &&
        ch->player.description != mob_proto[i].player.description) {
      free(ch->player.description); ch->player.description = NULL;
    }
  }
  while (ch->affected)
    affect_remove(ch, ch->affected, FALSE);
  while (ch->affected2)
    affect_remove(ch, ch->affected2, TRUE);

  /* remove the character from the event handler list if applicable */
  remove_event(ch);

  free(ch);
}

/* release memory allocated for a room struct */
void free_room(struct room_data *room)
{
  struct extra_descr_data *thisp, *next_one;
  int counter;

  if (room->name)
    free(room->name);
  if (room->description)
    free(room->description);
  for (counter = 0; counter < NUM_OF_DIRS; counter++)
    {
      if (room->dir_option[counter])
	{
	  if (room->dir_option[counter]->general_description)
	    free(room->dir_option[counter]->general_description);
	  if (room->dir_option[counter]->keyword)
	    free(room->dir_option[counter]->keyword);
	}
      free(room->dir_option[counter]);
    }
  if (room->ex_description)
    for (thisp = room->ex_description; thisp; thisp = next_one) {
      next_one = thisp->next;
      if (thisp->keyword)
	free(thisp->keyword);
      if (thisp->description)
	free(thisp->description);
      free(thisp);
    }
}


/* release memory allocated for an obj struct */
void free_obj(struct obj_data * obj)
{
  long nr;
  struct extra_descr_data *thisp, *next_one;

  if ((nr = GET_OBJ_RNUM(obj)) == -1) {
    if (obj->name)
      free(obj->name);
    if (obj->description)
      free(obj->description);
    if (obj->short_description)
      free(obj->short_description);
    if (obj->action_description)
      free(obj->action_description);
    if (obj->ex_description)
      for (thisp = obj->ex_description; thisp; thisp = next_one) {
	next_one = thisp->next;
	if (thisp->keyword)
	  free(thisp->keyword);
	if (thisp->description)
	  free(thisp->description);
	free(thisp);
      }
  } else {
    if (obj->name && obj->name != obj_proto[nr].name)
      free(obj->name);
    if (obj->description && obj->description != obj_proto[nr].description)
      free(obj->description);
    if (obj->short_description && obj->short_description != obj_proto[nr].short_description)
      free(obj->short_description);
    if (obj->action_description && obj->action_description != obj_proto[nr].action_description)
      free(obj->action_description);
    if (obj->ex_description && obj->ex_description != obj_proto[nr].ex_description)
      for (thisp = obj->ex_description; thisp; thisp = next_one) {
	next_one = thisp->next;
	if (thisp->keyword)
	  free(thisp->keyword);
	if (thisp->description)
	  free(thisp->description);
	free(thisp);
      }
  }
  free(obj);
}



/* read contets of a text file, alloc space, point buf to it */
int file_to_string_alloc(char *name, char **buf)
{
  char temp[MAX_STRING_LENGTH];

  if (*buf)
    free(*buf);

  if (file_to_string(name, temp) < 0) {
    *buf = "";
    return -1;
  } else {
    *buf = str_dup(temp);
    return 0;
  }
}


/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
  FILE *fl;
  char tmp[READ_SIZE+3];
  /* for OLC back-up file */
  char tname[80];

  *buf   = '\0';
  *tname = '\0';

  if (!(fl = fopen(name, "r"))) {
    sprintf(tmp, "Error reading %s", name);
    perror(tmp);
    return (-1);
  }
  /* OLC: If file is empty load the old one */
  if(feof(fl)) {
    fclose(fl);
    fprintf(stderr, "OLC: empty file %s, loading backup", name);
    sprintf(tname, "%s.back", name);
    remove(name);
    rename(tname, name);
    if (!(fl = fopen(name, "r"))) {
      sprintf(tmp, "Error reading %s", name);
      perror(tmp);
      return (-1);
    }
  }

  do {
    fgets(tmp, READ_SIZE, fl);
    tmp[strlen(tmp) - 1] = '\0'; /* take off the trailing \n */
    strcat(tmp, "\r\n");

    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 1 > MAX_STRING_LENGTH) {
	sprintf(buf, "SYSERR: %s: string too big (%d max)", name,
		MAX_STRING_LENGTH);
	log(buf);
	*buf = '\0';
	return -1;
      }
      strcat(buf, tmp);
    }
  } while (!feof(fl));

  fclose(fl);

  return (0);
}




/* clear some of the the working variables of a char */
void reset_char(struct char_data * ch)
{
  int i;

  for (i = 0; i < NUM_WEARS; i++)
    GET_EQ(ch, i) = NULL;

  ch->followers = NULL;
  ch->master    = NULL;
  ch->in_room   = NOWHERE;
  ch->carrying  = NULL;
  ch->next      = NULL;
  ch->next_fighting = NULL;
  ch->next_in_room  = NULL;
  FIGHTING(ch)  = NULL;
  ch->char_specials.position     = POS_STANDING;
  ch->mob_specials.default_pos   = POS_STANDING;
  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items  = 0;
  ch->player_specials->screensize = 24;

  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;
  if (GET_MAX_HIT(ch) <= 0)
    GET_MAX_HIT(ch) = 1;
  if (GET_MAX_MOVE(ch) <= 0)
    GET_MAX_MOVE(ch) = 1;
  if (GET_MAX_MANA(ch) <= 0)
    GET_MAX_MANA(ch) = 1;

  GET_LAST_TELL(ch) = NOBODY;
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
  GET_REL(ch)                  = REL_UNDEFINED;
  ch->mob_specials.default_pos = POS_STANDING;

  for (i=0; i < ARMOR_LIMIT; i++)
    ch->points.armor[i] = 0;        /* No Armor */
}


void clear_object(struct obj_data * obj)
{
  memset((char *) obj, 0, sizeof(struct obj_data));

  obj->item_number = NOTHING;
  obj->in_room = NOWHERE;
}




/* initialize a new character */
void init_char(struct char_data * ch)
{
  int i;

  /* create a player_special structure */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  /* *** if this is our first player --- he be God *** */

  /* Just to make sure no one is trusted ;-) */
  GET_TRUST(ch) = 0;

  if (top_of_p_table == 0) {
    GET_EXP(ch)   = 0;
    GET_LEVEL(ch) = LVL_CREATOR;

    ch->points.max_hit  = 25000;
    ch->points.max_mana = 25000;
    ch->points.max_move = 25000;
  }
  set_title(ch, NULL);

  ch->player.short_descr = NULL;
  ch->player.long_descr  = NULL;
  ch->player.description = NULL;

  ch->player.hometown    = get_hometown(ch);

  ch->player.time.birth  = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon  = time(0);

  for (i = 0; i < MAX_LANGUAGES; i++)
    GET_LANGUAGE(ch, i) = 0;

  ch->points.max_mana = number(80, 125);
  ch->points.mana     = GET_MAX_MANA(ch);
  ch->points.hit      = GET_MAX_HIT(ch);
  ch->points.max_move = GET_MAX_MOVE(ch);
  ch->points.move     = number(85, 115);
  for (i=0; i < ARMOR_LIMIT; i++)
      ch->points.armor[i] = 0;

  player_table[top_of_p_table].id = GET_IDNUM(ch) = ++top_idnum;

  for (i = 1; i <= MAX_SKILLS; i++) {
    if (GET_LEVEL(ch) < LVL_SUP)
      SET_SKILL(ch, i, 0)
    else
      SET_SKILL(ch, i, 100);
  }

  ch->char_specials.saved.affected_by = 0;
  ch->char_specials.saved.affected_by2 = 0;

  for (i = 0; i < 9; i++)
    GET_SAVE(ch, i) = 0;

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
     ch->real_abils.intel = 25;
     ch->real_abils.wis   = 25;
     ch->real_abils.dex   = 25;
     ch->real_abils.str   = 25;
     ch->real_abils.str_add = 100;
     ch->real_abils.con   = 25;
     ch->real_abils.cha   = 25;
     ch->real_abils.will  = 25;
  }

  for (i = 0; i < 4; i++)
    GET_COND(ch, i) = (GET_LEVEL(ch) >= LVL_IMMORT ? -1 : 24);

  GET_WOUNDS(ch)   = 0;
  set_bodyparts(ch);
  GET_LOADROOM(ch) = NOWHERE;
}

void set_bodyparts(struct char_data *ch)
{
/* will set the bits on all body parts for the ch so they are complete */
}

/* returns the real number of the room with given vnumber number */
long real_room(long vnumber)
{
  long bot, top, mid;

  bot = 0;
  top = top_of_world;

  /* perform binary search on world-table */
  for (;;) {
    mid = (bot + top) >> 1;

    if ((world + mid)->number == vnumber)
      return mid;
    if (bot >= top)
      return NOWHERE;
    if ((world + mid)->number > vnumber)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}



/* returns the real number of the monster with given vnumber number */
long real_mobile(long vnumber)
{
  long bot, top, mid;

  bot = 0;
  top = top_of_mobt;

  /* perform binary search on mob-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((mob_index + mid)->vnumber == vnumber)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((mob_index + mid)->vnumber > vnumber)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}



/* returns the real number of the object with given vnumber number */
long real_object(long vnumber)
{
  long bot, top, mid;

  bot = 0;
  top = top_of_objt;

  /* perform binary search on obj-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((obj_index + mid)->vnumber == vnumber)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((obj_index + mid)->vnumber > vnumber)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}


/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */
int mprog_name_to_type (char *name)
{
   if (!str_cmp(name, "in_file_prog"  ))    return IN_FILE_PROG;
   if (!str_cmp(name, "act_prog"      ))    return ACT_PROG;
   if (!str_cmp(name, "speech_prog"   ))    return SPEECH_PROG;
   if (!str_cmp(name, "rand_prog"     ))    return RAND_PROG;
   if (!str_cmp(name, "fight_prog"    ))    return FIGHT_PROG;
   if (!str_cmp(name, "hitprcnt_prog" ))    return HITPRCNT_PROG;
   if (!str_cmp(name, "death_prog"    ))    return DEATH_PROG;
   if (!str_cmp(name, "entry_prog"    ))    return ENTRY_PROG;
   if (!str_cmp(name, "greet_prog"    ))    return GREET_PROG;
   if (!str_cmp(name, "all_greet_prog"))    return ALL_GREET_PROG;
   if (!str_cmp(name, "give_prog"     ))    return GIVE_PROG;
   if (!str_cmp(name, "bribe_prog"    ))    return BRIBE_PROG;

   return(ERROR_PROG);
}

 /*
  * Read a number from a file.
  */
int fread_number(FILE *fp)
{
    int number;
    bool sign;
    char c;

    do {
       c = getc(fp);
    } while (isspace((int)c));

    number = 0;

    sign   = FALSE;
    if (c == '+') {
       c = getc(fp);
    } else if (c == '-') {
       sign = TRUE;
       c = getc(fp);
    }

    if (!isdigit((int)c)) {
       log("Fread_number: bad format.");
       exit(1);
    }

    while (isdigit((int)c)) {
       number = number * 10 + c - '0';
       c      = getc(fp);
    }

    if (sign)
       number = 0 - number;

    if (c == '|')
       number += fread_number(fp);
    else if (c != ' ')
       ungetc(c, fp);

    return number;
}

/*
  * Read a number from a file.
  */
long fread_long(FILE *fp)
{
  long number;
  bool sign;
  char c;
  
  do {
    c = getc(fp);
  } while (isspace((int)c));
  
  number = 0;

  sign   = FALSE;
  if (c == '+') {
    c = getc(fp);
  } else if (c == '-') {
    sign = TRUE;
    c = getc(fp);
  }
  
  if (!isdigit((int)c)) {
    log("Fread_long: bad format.");
    exit(1);
  }
  
  while (isdigit((int)c)) {
    number = number * 10 + c - '0';
    c      = getc(fp);
  }
  
  if (sign)
    number = 0 - number;
  
  if (c == '|')
    number += fread_long(fp);
  else if (c != ' ')
    ungetc(c, fp);
  
  return number;
}

 /*
  * Read to end of line (for comments).
  */
void fread_to_eol(FILE *fp)
{
  char c;
  
  do {
    c = getc(fp);
  } while (c != '\n' && c != '\r');
  
  do {
    c = getc(fp);
  } while (c == '\n' || c == '\r');
  
  ungetc(c, fp);
  return;
}

 /*
  * Read one word (into static buffer).
  */
char *fread_word(FILE *fp)
{
     static char word[MAX_INPUT_LENGTH];
     char *pword;
     char cEnd;

     do
     {
	 cEnd = getc(fp);
     }
     while (isspace((int)cEnd));

     if (cEnd == '\'' || cEnd == '"')
     {
	 pword   = word;
     }
     else
     {
	 word[0] = cEnd;
	 pword   = word+1;
	 cEnd    = ' ';
     }

     for (; pword < word + MAX_INPUT_LENGTH; pword++)
     {
	 *pword = getc(fp);
	 if (cEnd == ' ' ? isspace((int)*pword) || *pword == '~' : *pword == cEnd)
	 {
	     if (cEnd == ' ' || cEnd == '~')
		 ungetc(*pword, fp);
	     *pword = '\0';
	     return word;
	 }
     }

     log("SYSERR: Fread_word: word too long.");
     exit(1);
     return NULL;
}


/* This routine reads in scripts of MOBprograms from a file */
MPROG_DATA* mprog_file_read(char *f, MPROG_DATA *mprg, struct index_data *pMobIndex)
{
   char        MOBProgfile[ MAX_INPUT_LENGTH ];
   MPROG_DATA *mprg2;
   FILE       *progfile;
   char        letter;
   bool        done = FALSE;

   sprintf(MOBProgfile, "%s/%s", MOB_DIR, f);

   progfile = fopen(MOBProgfile, "r");
   if (!progfile) {
     sprintf(err_buf, "Mob: %ld couldnt open mobprog file", pMobIndex->vnumber);
     log(err_buf);
     exit(1);
   }

   mprg2 = mprg;
   switch (letter = fread_letter(progfile)) {
     case '>':
       break;
     case '|':
       log("empty mobprog file.");
       exit(1);
       break;
     default:
       log("in mobprog file syntax error.");
       exit(1);
       break;
   }

   while (!done) {
     mprg2->type = mprog_name_to_type(fread_word(progfile));
     switch (mprg2->type) {
       case ERROR_PROG:
	 log("mobprog file type error");
	 exit(1);
	 break;
       case IN_FILE_PROG:
	 log("mprog file contains a call to file.");
	 exit(1);
	 break;
       default:
	 sprintf(buf2, "Error in file %s", f);
	 pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
	 mprg2->arglist       = fread_string(progfile,buf2);
	 mprg2->comlist       = fread_string(progfile,buf2);

	 switch (letter = fread_letter(progfile)) {
	   case '>':
	     mprg2->next = (MPROG_DATA *)malloc(sizeof(MPROG_DATA));
	     mprg2       = mprg2->next;
	     mprg2->next = NULL;
	     break;
	   case '|':
	     done = TRUE;
	     break;
	   default:
	     sprintf(err_buf,"in mobprog file %s syntax error.", f);
	     log(err_buf);
	     exit(1);
	     break;
	 }
       break;
     }
   }
   fclose(progfile);
   return mprg2;
}

struct index_data *get_obj_index (long int vnum)
{
   long int nr;

   for(nr = 0; nr <= top_of_objt; nr++) {
      if(obj_index[nr].vnumber == vnum) return &obj_index[nr];
   }
   return NULL;
}

struct index_data *get_mob_index (long int vnum)
{
   long int nr;

   for(nr = 0; nr <= top_of_mobt; nr++) {
      if(mob_index[nr].vnumber == vnum) return &mob_index[nr];
   }
   return NULL;
}

/*
 * This procedure is responsible for reading any in_file MOBprograms.
 */
void mprog_read_programs(FILE *fp, struct index_data *pMobIndex)
{
   MPROG_DATA *mprg;
   char        letter;
   bool        done = FALSE;

   if ((letter = fread_letter(fp)) != '>')
   {
     sprintf(err_buf,"Load_mobiles: vnum %ld MOBPROG char", pMobIndex->vnumber);
     log(err_buf);
     exit(1);
   }
   pMobIndex->mobprogs = (MPROG_DATA *)malloc(sizeof(MPROG_DATA));
   mprg = pMobIndex->mobprogs;

   while (!done) {
     mprg->type = mprog_name_to_type(fread_word(fp));
     switch (mprg->type) {
       case ERROR_PROG:
	 sprintf(err_buf, "Load_mobiles: vnum %ld MOBPROG type.", pMobIndex->vnumber);
	 log(err_buf);
	 exit(1);
	 break;
       case IN_FILE_PROG:
	 sprintf(buf2, "Mobprog for mob #%ld", pMobIndex->vnumber);
	 mprg = mprog_file_read(str_dup(fread_word(fp)), mprg, pMobIndex);
	 fread_to_eol(fp);   /* need to strip off that silly ~*/
	 switch (letter = fread_letter(fp)) {
	   case '>':
	     mprg->next = (MPROG_DATA *)malloc(sizeof(MPROG_DATA));
	     mprg       = mprg->next;
	     mprg->next = NULL;
	     break;
	   case '|':
	     mprg->next = NULL;
	     fread_to_eol(fp);
	     done = TRUE;
	     break;
	   default:
	     sprintf(err_buf, "Load_mobiles: vnum %ld bad MOBPROG.", pMobIndex->vnumber);
	     log(err_buf);
	     exit(1);
	     break;
	 }
	 break;
       default:
	 sprintf(buf2, "Mobprog for mob #%ld", pMobIndex->vnumber);
	 pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
	 mprg->arglist        = fread_string(fp, buf2);
	 mprg->comlist        = fread_string(fp, buf2);
	 switch (letter = fread_letter(fp)) {
	   case '>':
	      mprg->next = (MPROG_DATA *)malloc(sizeof(MPROG_DATA));
	      mprg       = mprg->next;
	      mprg->next = NULL;
	    break;
	   case '|':
	      mprg->next = NULL;
	      fread_to_eol(fp);
	      done = TRUE;
	    break;
	   default:
	      sprintf(err_buf, "Load_mobiles: vnum %ld bad MOBPROG (%c).", pMobIndex->vnumber, letter);
	      log(err_buf);
	      exit(1);
	    break;
	 }
       break;
     }
   }

   return;
}


#define GET_NDD(mob)     ((mob)->mob_specials.damnodice)
#define GET_SDD(mob)     ((mob)->mob_specials.damsizedice)

void recalc_stats(struct char_data *mob)
{
  int i = 0;

  GET_EXP(mob)     = ((exp_needed(GET_LEVEL(mob))) >> 5);
  GET_HITROLL(mob) = (GET_LEVEL(mob) << 1) + dice(2, 10);
  GET_DAMROLL(mob) = (GET_LEVEL(mob) << 1) + dice(2, 10);
  /* Num of damage dice */
  GET_NDD(mob)     = GET_LEVEL(mob) >> 2;
  /* Size of damage dice */
  GET_SDD(mob)     = (GET_LEVEL(mob) >> 1) + dice(2, 10);
  /* Num of hit dice  - lvl /2 */
  GET_HIT(mob)     = GET_LEVEL(mob) >> 1;
  /* Size of hit dice - lvl x2 */
  GET_MANA(mob)    = GET_LEVEL(mob) << 1;
  /* Add hit points   - lvl x2 */
  GET_MOVE(mob)    = GET_LEVEL(mob) << 1;
  /* Avg. Armor %age */
  for (i = 0; i < ARMOR_LIMIT; i++)
    mob->points.armor[i] = MAX(0, MIN(100, (GET_LEVEL(mob) << 1) + dice(4, 5)));
  /* Gold */
  GET_GOLD(mob)    = IS_DRAGON(mob) ? (GET_LEVEL(mob) * 1000) : 
                     IS_HUMANOID(mob) ? (GET_LEVEL(mob) + dice(1, 50)) : 0;

  if (GET_LEVEL(mob) >= 15) {
    GET_STR(mob) = (GET_LEVEL(mob) >> 1) + dice(1, 4);
    GET_INT(mob) = (GET_LEVEL(mob) >> 1) + dice(1, 4);
    GET_WIS(mob) = (GET_LEVEL(mob) >> 1) + dice(1, 4);
    GET_DEX(mob) = (GET_LEVEL(mob) >> 1) + dice(1, 4);
    GET_CON(mob) = (GET_LEVEL(mob) >> 1) + dice(1, 4);
    GET_CHA(mob) = (GET_LEVEL(mob) >> 1) + dice(1, 4);
    GET_WILL(mob) = (GET_LEVEL(mob) >> 1) + dice(1, 4);
  }
}
