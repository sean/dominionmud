/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  COMM.C                                       Based on CircleMUD *
*  Usage: Communication, socket handling, main(), central game loop       *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __COMM_C__

#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/telnet.h>
#include <netdb.h>
#include <signal.h>

#include "conf.h"
#include "sysdep.h"
#include "protos.h"

int getpagesize(void);
int getrusage(int, struct rusage *);

#define TELOPT_SGA        3
#define TELOPT_NAWS       31
#define TELOPT_TSPEED     32
#define TELOPT_LFLOW      33
#define TELOPT_LINEMODE   34

/* externs */
extern int game_restrict;
extern int mini_mud;
extern int no_rent_check;
extern FILE *player_fl;
extern int DFLT_PORT;
extern char *DFLT_DIR;
extern int MAX_PLAYERS;
extern int MAX_DESCRIPTORS_AVAILABLE;

extern struct room_data *world; /* In db.c */
extern long top_of_world;       /* In db.c */
extern struct time_info_data time_info;         /* In db.c */
extern char help[];
extern const char *dirs[];
extern const char *percent_hit[];

/* local globals */
struct descriptor_data *descriptor_list = NULL;         /* master desc list */
struct txt_block *bufpool = 0;  /* pool of large output buffers */
int buf_largecount = 0;         /* # of large buffers which exist */
int buf_overflows = 0;          /* # of overflows of output */
int buf_switches = 0;           /* # of switches from small to large buf */
int circle_shutdown = 0;        /* clean shutdown */
int circle_reboot = 0;          /* reboot the game after a shutdown */
int no_specials = 0;            /* Suppress ass. of special routines */
int max_players = 0;            /* max descriptors available */
int tics = 0;                   /* for extern checkpointing */
int scheck = 0;                 /* for syntax checking mode */
bool MOBTrigger = TRUE;         /* For MOBProgs */
extern int nameserver_is_slow;  /* see config.c */
extern int auto_save;           /* see config.c */
extern int autosave_time;       /* see config.c */
struct timeval null_time;       /* zero-valued time structure */
extern struct index_data *mob_index;
static bool fCopyOver;
sh_int port;
socket_t mother_desc;

FILE * fpReserve;

/* functions in this file */
int  get_from_q(struct txt_q *queue, char *dest, int *aliased);
void init_game(int port);
void signal_setup(void);
void game_loop(int mother_desc);
int  init_socket(int port);
int  new_descriptor(int s);
int  get_max_players(void);
int  process_output(struct descriptor_data *t);
int  process_input(struct descriptor_data *t);
void close_socket(struct descriptor_data *d);
struct timeval timediff(struct timeval *a, struct timeval *b);
void flush_queues(struct descriptor_data *d);
void nonblock(int s);
int  perform_subst(struct descriptor_data *t, char *orig, char *subst);
int  perform_alias(struct descriptor_data *d, char *orig);
void record_usage(void);
void make_prompt(struct descriptor_data *point);
void check_idle_passwords(void);
void init_descriptor(struct descriptor_data *newd, int desc);

/* extern fcnts */
void boot_db(void);
void boot_world(void);
void zone_update(void);
void affect_update(void);       /* In spells.c */
void point_update(void);        /* In limits.c */
void mobile_activity(void);
void event_activity(void);
void string_add(struct descriptor_data *d, char *str);
void perform_violence(void);
void show_string(struct descriptor_data *d, char *input);
int isbanned(char *hostname);
void weather_and_time(int mode);
void perform_teleport_pulse(void);
void perform_spc_update(void);
void perform_current_sweep(void);
void die(struct char_data *ch);
long exp_needed(int level);
void free_room(struct room_data *room);
void free_zone(struct zone_data *zone);
void free_shop(struct shop_data *shop);
ACMD(do_track);
ACMD(do_infobar);
char *scrpos(int y, int x, struct char_data *ch);
void health_meter(struct char_data *c, struct char_data *v);
void mprog_wordlist_check(char *arg, struct char_data*mob, struct char_data *ch, struct obj_data  *o, void *vo, int prog_type);

/* *********************************************************************
*  main game loop and related stuff                                    *
********************************************************************* */

int main(int argc, char **argv)
{
  char buf[512];
  int pos = 1;
  char *dir;

  port = DFLT_PORT;
  dir = DFLT_DIR;

  while ((pos < argc) && (*(argv[pos]) == '-')) {
    switch (*(argv[pos] + 1)) {
    case 'd':
      if (*(argv[pos] + 2))
	dir = argv[pos] + 2;
      else if (++pos < argc)
	dir = argv[pos];
      else {
	log("Directory arg expected after option -d.");
	exit(1);
      }
      break;
    case 'm':
      mini_mud = 1;
      no_rent_check = 1;
      log("Running in minimized mode & with no rent check.");
      break;
    case 'c':
      scheck = 1;
      log("Syntax check mode enabled.");
      break;
    case 'q':
      no_rent_check = 1;
      log("Quick boot mode -- rent check supressed.");
      break;
    case 'r':
      if (*(argv[pos] + 2)) {
	      game_restrict = atoi(argv[pos] + 2);
      } else if (++pos < argc) {
	      game_restrict = atoi(argv[pos]);
      } else {
	      game_restrict = 1;
      }
      sprintf(buf, "Restricting game -- no new players allowed above level %d.", game_restrict);
      log(buf);
      break;
    case 's':
      no_specials = 1;
      log("Suppressing assignment of special routines.");
      break;
    case 'C':
      fCopyOver = TRUE;
      mother_desc = atoi(argv[pos]+2);
      break;
    default:
      sprintf(buf, "SYSERR: Unknown option -%c in argument string.", *(argv[pos] + 1));
      log(buf);
      break;
    }
    pos++;
  }

  if (pos < argc) {
    if (!isdigit((int)*argv[pos])) {
      fprintf(stderr, 
	      "Usage: %s [-c] [-m] [-q] [-r #] [-s] [-d pathname] [port #]\n", argv[0]);
      exit(1);
    } else if ((port = atoi(argv[pos])) <= 1024) {
      fprintf(stderr, "Illegal port number.\n");
      exit(1);
    }
  }
  if (chdir(dir) < 0) {
    perror("Fatal error changing to data directory");
    exit(1);
  }
  sprintf(buf, "Using %s as data directory.", dir);
  log(buf);

  // reserve a file descriptor
  if ( !(fpReserve = fopen( NULL_FILE, "r" )) ) {
    perror( NULL_FILE );
    exit( 1 );
  }

  if (scheck) {
    boot_world();
    log("Done.");
    exit(0);
  } else {
    sprintf(buf, "Running game on port %d.", port);
    log(buf);
    init_game(port);
  }

  return 0;
}

int enter_player_game(struct descriptor_data *d);

/* Reload players after a copyover */
void copyover_recover( )
{
  struct descriptor_data *d;
  FILE *fp;
  char name[MAX_INPUT_LENGTH];
  char host[1024];
  struct char_file_u tmp_store;
  int desc, player_i;
  bool fOld;
	
  log ("Hot reboot recovery initiated");

  /* there are some descriptors open which will hang forever then ? */
  if ( !(fp = fopen (COPYOVER_FILE, "r")) ) {
    perror ("copyover_recover:fopen");
    log ("Hot reboot file not found. Exitting.\r\n");
    exit (1);
  }
  
  /* In case something crashes - doesn't prevent reading	*/
  unlink(COPYOVER_FILE);
  
  for (;;) {
    fOld = TRUE;
    fscanf(fp, "%d %s %s\n", &desc, name, host);

    if (desc == -1)
      break;
    
    /* Write something, and check if it goes error-free */		
    if (write_to_descriptor(desc, "\n\rRestoring from hot reboot ...\n\r") < 0) {
      plog( "Unable to write to descriptor, closing connection to %s from %s.\n", name, host );
      close(desc); /* nope */
      continue;
    }
    
    /* create a new descriptor */
    CREATE (d, struct descriptor_data, 1);
    memset ((char *) d, 0, sizeof (struct descriptor_data));
    init_descriptor (d,desc); /* set up various stuff */
    
    strcpy(d->host, host);
    d->next = descriptor_list;
    descriptor_list = d;
    
    d->connected = CON_CLOSE;
    
    /* Now, find the pfile */
    CREATE(d->character, struct char_data, 1);
    clear_char(d->character);
    CREATE(d->character->player_specials, struct player_special_data, 1);
    d->character->desc = d;
    
    if ((player_i = load_char(name, &tmp_store)) >= 0) {
      store_to_char(&tmp_store, d->character);
      GET_PFILEPOS(d->character) = player_i;
      if (!PLR_FLAGGED(d->character, PLR_DELETED))
	REMOVE_BIT(PLR_FLAGS(d->character),PLR_WRITING | PLR_MAILING | PLR_CRYO);
      else
	fOld = FALSE;
    } else
      fOld = FALSE;
    
    /* Player file not found?! */
    if (!fOld) {
      write_to_descriptor(desc, 
			  "\n\rSomehow, your character was lost in the reboot. Sorry.\n\r");
      close_socket (d);			
    } else {
      enter_player_game(d);
      d->connected = CON_PLAYING;
      write_to_descriptor(desc, "\n\rReboot recovery complete.\n\r");
      look_at_room(d->character, 0);
    }
  }
  fclose (fp);
}


/* Init sockets, run game, and cleanup sockets */
void init_game(int port)
{
  srandom(time(0));

  /* If copyover mother_desc is already set up */
  if (!fCopyOver) {
    log("Opening mother connection.");
    mother_desc = init_socket(port);
  }

  max_players = get_max_players();

  boot_db();

  log("Signal trapping.");
  signal_setup();

  if (fCopyOver) /* reload players */
  copyover_recover( );

  log("Entering game loop.");
  game_loop(mother_desc);

  log("Closing all sockets.");
  while (descriptor_list)
    close_socket(descriptor_list);

  close(mother_desc);
  fclose(player_fl);

  if (circle_reboot) {
    log("Rebooting.");
    exit(52);                   /* what's so great about HHGTTG, anyhow? */
  }

  /* save the current day/month/year etc. */
  store_mud_time( &time_info );
  
  log("Normal termination of game.");
}



/*
 * init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens.
 */
int init_socket(int port)
{
  int s, opt;
  struct sockaddr_in sa;

  /*
   * Should the first argument to socket() be AF_INET or PF_INET?  I don't
   * know, take your pick.  PF_INET seems to be more widely adopted, and
   * Comer (_Internetworking with TCP/IP_) even makes a point to say that
   * people erroneously use AF_INET with socket() when they should be using
   * PF_INET.  However, the man pages of some systems indicate that AF_INET
   * is correct; some such as ConvexOS even say that you can use either one.
   * All implementations I've seen define AF_INET and PF_INET to be the same
   * number anyway, so ths point is (hopefully) moot.
   */

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Create socket");
    exit(1);
  }
#if defined(SO_SNDBUF)
  opt = LARGE_BUFSIZE + GARBAGE_SPACE;
  if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt SNDBUF");
    exit(1);
  }
#endif

#if defined(SO_REUSEADDR)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEADDR");
    exit(1);
  }
#endif

#if defined(SO_REUSEPORT)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEPORT");
    exit(1);
  }
#endif

#if defined(SO_LINGER)
  {
    struct linger ld;

    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0) {
      perror("setsockopt LINGER");
      exit(1);
    }
  }
#endif

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    perror("bind");
    close(s);
    exit(1);
  }
  nonblock(s);
  listen(s, 5);
  return s;
}


int get_max_players(void)
{
  int max_descs = 0;
  char *method;

/*
 * First, we'll try using getrlimit/setrlimit.  This will probably work
 * on most systems.
 */
#if defined (RLIMIT_NOFILE) || defined (RLIMIT_OFILE)
#if !defined(RLIMIT_NOFILE)
#define RLIMIT_NOFILE RLIMIT_OFILE
#endif
  {
    struct rlimit limit;

    method = "rlimit";
    if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("calling getrlimit");
      exit(1);
    }

#ifndef __APPLE__
    /* set the current to the maximum */
    limit.rlim_cur = limit.rlim_max;
    if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("calling setrlimit");
      exit(1);
    }
#endif

#ifdef RLIM_INFINITY
    if (limit.rlim_max == RLIM_INFINITY)
      max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
    else
      max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);
#else
      max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);
#endif
  }

#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if !defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif
  method = "OPEN_MAX";
  max_descs = OPEN_MAX;         /* Uh oh.. rlimit didn't work, but we have
				 * OPEN_MAX */
#elif defined (POSIX)
  /*
   * Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
   * use the POSIX sysconf() function.  (See Stevens' _Advanced Programming
   * in the UNIX Environment_).
   */
  method = "POSIX sysconf";
  errno = 0;
  if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0) {
    if (errno == 0)
      max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
    else {
      perror("Error calling sysconf");
      exit(1);
    }
  }
#endif

  /* now calculate max _players_ based on max descs */
  max_descs = MIN(MAX_PLAYERS, max_descs - NUM_RESERVED_DESCS);

  if (max_descs <= 0) {
    sprintf(buf, "Non-positive max player limit!  (Set at %d using %s).",
	    max_descs, method);
    log(buf);
    exit(1);
  }

  sprintf(buf, "Setting player limit to %d using %s.", max_descs, method);
  log(buf);
  return max_descs;
}



/*
 * game_loop contains the main loop which drives the entire MUD.  It
 * cycles once every 0.10 seconds and is responsible for accepting new
 * new connections, polling existing connections for input, dequeueing
 * output and sending it out to players, and calling "heartbeat" functions
 * such as mobile_activity().
 */
void game_loop(int mother_desc)
{
  fd_set input_set, output_set, exc_set;
  struct timeval last_time, now, timespent, timeout, opt_time;
  char   comm[MAX_INPUT_LENGTH];
  struct descriptor_data *d, *next_d;
  int  pulse = 0, mins_since_crashsave = 0, maxdesc, aliased;

  /* initialize various time values */
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  opt_time.tv_usec = OPT_USEC;
  opt_time.tv_sec = 0;
  gettimeofday(&last_time, (struct timezone *) 0);

  /* The Main Loop.  The Big Cheese.  The Top Dog.  The Head Honcho.  The.. */
  while (!circle_shutdown) {
    /* Sleep if we don't have any connections */
    if (descriptor_list == NULL) {
      log("No connections.  Going to sleep.");
      FD_ZERO(&input_set);
      FD_SET(mother_desc, &input_set);
      if (select(mother_desc + 1, &input_set,
                 (fd_set *) 0, (fd_set *) 0, NULL) < 0) {
	if (errno == EINTR) {
	  log("Waking up to process signal.");
	} else
	  perror("Select coma");
      } else {
	log("New connection.  Waking up.");
      }
      gettimeofday(&last_time, (struct timezone *) 0);
    }
    /* Set up the input, output, and exception sets for select(). */
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(mother_desc, &input_set);
    maxdesc = mother_desc;
    for (d = descriptor_list; d; d = d->next) {
      if (d->descriptor > maxdesc)
	maxdesc = d->descriptor;
      FD_SET(d->descriptor, &input_set);
      FD_SET(d->descriptor, &output_set);
      FD_SET(d->descriptor, &exc_set);
    }

    /*
     * At this point, the original Diku code set up a signal mask to block
     * all signals from being delivered.  I believe this was done in order
     * to prevent the MUD from dying with an "interrupted system call"
     * error in the event that a signal be received while the MUD is sleeping
     * in between ticks.  However, there is no easy way to *portably* block
     * signals reliably, so I think it is easier to check for an EINTR error
     * return from this select() call than to block and unblock signals.
     */
    do {
      errno = 0;                /* clear error condition */
      /* figure out for how long we have to sleep */
      gettimeofday(&now, (struct timezone *) 0);
      timespent = timediff(&now, &last_time);
      timeout = timediff(&opt_time, &timespent);

      /* sleep until the next 0.1 second tick */
      if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0)
	if (errno != EINTR) {
	  perror("Select sleep");
	  exit(1);
	}
    } while (errno);

    /* record the time for the next pass */
    gettimeofday(&last_time, (struct timezone *) 0);

    /* poll (without blocking) for new input, output, and exceptions */
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      perror("Select poll");
      return;
    }
    /* If there are new connections waiting, accept them. */
    if (FD_ISSET(mother_desc, &input_set)) {
      new_descriptor(mother_desc);
    }

    /* kick out the freaky folks in the exception set */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &exc_set)) {
	FD_CLR(d->descriptor, &input_set);
	FD_CLR(d->descriptor, &output_set);
	close_socket(d);
      }
    }

    /* process descriptors with input pending */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &input_set))
	if (process_input(d) < 0)
	  close_socket(d);
    }

    /* process commands we just read from process_input */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;

      if (d->character && IS_IMMORT(d->character))
        d->wait = 0;

      if ((--(d->wait) <= 0) && get_from_q(&d->input, comm, &aliased)) {
	if (d->character) {
	  /* reset the idle timer & pull char back from void if necessary */
	  d->character->char_specials.timer = 0;
	  if (!d->connected && GET_WAS_IN(d->character) != NOWHERE) {
	    if (d->character->in_room != NOWHERE)
	      char_from_room(d->character);
	    char_to_room(d->character, GET_WAS_IN(d->character));
	    GET_WAS_IN(d->character) = NOWHERE;
	    act("$n has returned.", TRUE, d->character, 0, 0, TO_ROOM);
	  }
	}
	d->wait = 1;
	d->prompt_mode = 1;

	if (d->str)             /* writing boards, mail, etc.     */
	  string_add(d, comm);
	else if (d->showstr_count)      /* reading something w/ pager     */
	  show_string(d, comm);
	else if (d->connected != CON_PLAYING)   /* in menus, etc. */
	  nanny(d, comm);
	else {                  /* else: we're playing normally */
	  if (aliased)          /* to prevent recursive aliases */
	    d->prompt_mode = 0;
	  else {
	    if (perform_alias(d, comm))   /* run it through aliasing system */
	      get_from_q(&d->input, comm, &aliased);
	  }
	  command_interpreter(d->character, comm);  /* send it to interpreter */
	}
      }
    }

    /* send queued output out to the operating system (ultimately to user) */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &output_set) && *(d->output)) {
	if (process_output(d) < 0)
	  close_socket(d);
	else
	  d->prompt_mode = 1;
      }
    }

    /* kick out folks in the CON_CLOSE state */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (STATE(d) == CON_CLOSE)
	close_socket(d);
    }

    /* give each descriptor an appropriate prompt */
    for (d = descriptor_list; d; d = d->next) {
      if (d->prompt_mode) {
	make_prompt(d);
	d->prompt_mode = 0;
      }
    }

    /* handle heartbeat stuff */
    /* Note: pulse now changes every 0.10 seconds  */

    pulse++;

    if (!(pulse % PULSE_ZONE))
      zone_update();

    if (!(pulse % PULSE_MOBILE)) {
      check_idle_passwords();
      mobile_activity();
    }

    if (!(pulse % PULSE_VIOLENCE))
      perform_violence();
    /* new for TD */
    if (!(pulse % PULSE_TELEPORT))
      perform_teleport_pulse();
    if (!(pulse % PULSE_SPC_UPDATE))
      perform_spc_update();
    if (!(pulse % PULSE_EVENTS))
      event_activity();
    if (!(pulse % PULSE_RIVER))
      perform_current_sweep();

    if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC))) {
      /* Update weather every 3 mins */
      if (!(pulse % (number(180, 600) * PASSES_PER_SEC)))
	weather_and_time(1);
      else
	weather_and_time(0);
      affect_update();
      point_update();
      fflush(player_fl);
    }

    if (auto_save)
      if (!(pulse % (60 * PASSES_PER_SEC)))     /* 1 minute */
	if (++mins_since_crashsave >= autosave_time) {
	  mins_since_crashsave = 0;
	  Crash_save_all();
	  House_save_all();
	}
    if (!(pulse % (300 * PASSES_PER_SEC)))      /* 5 minutes */
      record_usage();

    if (pulse >= (30 * 60 * PASSES_PER_SEC)) {  /* 30 minutes */
      pulse = 0;
    }
    tics++;                     /* tics since last checkpoint signal */
  }
}



/* ******************************************************************
*  general utility stuff (for local use)                            *
****************************************************************** */

/*
 *  new code to calculate time differences, which works on systems
 *  for which tv_usec is unsigned (and thus comparisons for something
 *  being < 0 fail).  Based on code submitted by ss@sirocco.cup.hp.com.
 */

/*
 * code to return the time difference between a and b (a-b).
 * always returns a nonnegative value (floors at 0).
 */
struct timeval timediff(struct timeval *a, struct timeval *b)
{
  struct timeval rslt;

  if (a->tv_sec < b->tv_sec)
    return null_time;
  else if (a->tv_sec == b->tv_sec) {
    if (a->tv_usec < b->tv_usec)
      return null_time;
    else {
      rslt.tv_sec = 0;
      rslt.tv_usec = a->tv_usec - b->tv_usec;
      return rslt;
    }
  } else {                      /* a->tv_sec > b->tv_sec */
    rslt.tv_sec = a->tv_sec - b->tv_sec;
    if (a->tv_usec < b->tv_usec) {
      rslt.tv_usec = a->tv_usec + 1000000 - b->tv_usec;
      rslt.tv_sec--;
    } else
      rslt.tv_usec = a->tv_usec - b->tv_usec;
    return rslt;
  }
}

#define RUSAGE

void record_usage(void)
{
#ifdef RUSAGE
  int sockets_connected = 0, sockets_playing = 0;
  struct descriptor_data *d;
  char buf[256];

  for (d = descriptor_list; d; d = d->next) {
    sockets_connected++;
    if (!d->connected)
      sockets_playing++;
  }

  sprintf(buf, "nusage: %-3d sockets connected, %-3d sockets playing",
	  sockets_connected, sockets_playing);
  log(buf);

  {
    int pagesize, kbytes;
    struct rusage ru;

    getrusage(0, &ru);
    if (((pagesize = getpagesize()) < 1) || ((ru.ru_maxrss * pagesize / 1024) < 1)) {
       sprintf(buf, "rusage: user time: %ld sec, system time: %ld sec, max res size: %ld",
	    ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, ru.ru_maxrss);
    } else {
       kbytes = (ru.ru_maxrss * pagesize) / 1024;
       sprintf(buf, "rusage: user time: %ld sec, system time: %ld sec, mem usage: %dk",
	    ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, kbytes);
    }
    log(buf);
  }
#endif

}



/*
 * Turn off echoing (specific to telnet client)
 */
void echo_off(struct descriptor_data *d)
{
  char off_string[] =
  {
    (char) IAC,
    (char) WILL,
    (char) TELOPT_ECHO,
    (char) 0,
  };

  SEND_TO_Q(off_string, d);
}


/*
 * Turn on echoing (specific to telnet client)
 */
void echo_on(struct descriptor_data *d)
{
  char on_string[] =
  {
    (char) IAC,
    (char) WONT,
    (char) TELOPT_ECHO,
    (char) TELOPT_NAOFFD,
    (char) TELOPT_NAOCRD,
    (char) 0,
  };

  SEND_TO_Q(on_string, d);
}


/*
 * Turn off linemode (specific to telnet client)
 */
void charmode_on(struct descriptor_data *d)
{
  char on_string[] =
  {
    (char) IAC,
    (char) WILL,
    (char) TELOPT_SGA,
    (char) 0,
  };

  SEND_TO_Q(on_string, d);
}

void charmode_off(struct descriptor_data *d)
{
  char off_string[] =
  {
    (char) IAC,
    (char) WONT,
    (char) TELOPT_SGA,
    (char) 0,
  };

  SEND_TO_Q(off_string, d);
}

void make_prompt(struct descriptor_data *d)
{
  char  prompt[MAX_INPUT_LENGTH];
  float percent = 0, curr, maxim;
  int   num, dir;

  if (d->str)
    write_to_descriptor(d->descriptor, "] ");
  else if (d->showstr_count) {
    sprintf(prompt,
"\r[ Return to continue, (q)uit, (r)efresh, (b)ack, or page number (%d/%d) ]",
	    d->showstr_page, d->showstr_count);
    write_to_descriptor(d->descriptor, prompt);
  }
  else if (!d->connected) {
    char prompt[MAX_INPUT_LENGTH];

    *prompt = '\0';

    if (HUNTING(d->character)) {
       num = number(0, 101);
       if (GET_SKILL(d->character, SKILL_HUNT) < num) {
	  do {
	     dir = number(0, NUM_OF_DIRS - 1);
	  } while (!CAN_GO(d->character, dir));
	  sprintf(prompt, "You sense your prey is to the %s.\r\n", dirs[dir]);
       } else {
	  sprintf(buf, "%s", GET_NAME(HUNTING(d->character)));
	  do_track(d->character, buf, 0, 0);
       }
    }
    if (GET_INVIS_LEV(d->character))
       sprintf(prompt, "%s%si%d%s ", prompt, CCBBL(d->character, C_NRM),
	       GET_INVIS_LEV(d->character), CCNRM(d->character, C_NRM));

    if (PRF_FLAGGED(d->character, PRF_DISPHP)) {
       curr    = (float) GET_HIT(d->character);
       maxim   = (float) GET_MAX_HIT(d->character);
       if (maxim >= 1)
	  percent = (curr/maxim)*100;
       else percent = 0;
       if (percent == 100)
       sprintf(prompt, "%s%s%3.0f%%H%s ", prompt, CCGRN(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
       else if (percent > 50)
       sprintf(prompt, "%s%s%2.1f%%H%s ", prompt, CCGRN(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
       else if (percent > 25)
       sprintf(prompt, "%s%s%2.1f%%H%s ", prompt, CCYEL(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
       else
       sprintf(prompt, "%s%s%2.1f%%H%s ", prompt, CCRED(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
    }

    if (PRF_FLAGGED(d->character, PRF_DISPMANA)) {
       curr    = (float) GET_MANA(d->character);
       maxim   = (float) GET_MAX_MANA(d->character);
       if (maxim >= 1)
	 percent = (curr/maxim)*100;
       else percent = 0;
       if (percent == 100)
       sprintf(prompt, "%s%s%3.0f%%M%s ", prompt, CCGRN(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
       else if (percent > 50)
       sprintf(prompt, "%s%s%2.1f%%M%s ", prompt, CCGRN(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
       else if (percent > 25)
       sprintf(prompt, "%s%s%2.1f%%M%s ", prompt, CCYEL(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
       else
       sprintf(prompt, "%s%s%2.1f%%M%s ", prompt, CCRED(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
    }

    if (PRF_FLAGGED(d->character, PRF_DISPMOVE)) {
       curr  = (float) GET_MOVE(d->character);
       maxim = (float) GET_MAX_MOVE(d->character);
       if (maxim >= 1)
	 percent = (curr/maxim)*100;
       else percent = 0;
       if (percent == 100)
       sprintf(prompt, "%s%s%3.0f%%V%s ", prompt, CCGRN(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
       else if (percent > 50)
       sprintf(prompt, "%s%s%2.1f%%V%s ", prompt, CCGRN(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
       else if (percent > 25)
       sprintf(prompt, "%s%s%2.1f%%V%s ", prompt, CCYEL(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
       else
       sprintf(prompt, "%s%s%2.1f%%V%s ", prompt, CCRED(d->character, C_NRM),
	      percent, CCNRM(d->character, C_NRM));
    }

    if (PRF_FLAGGED(d->character, PRF_DISEXP2LEV)) {
       curr  = (float) GET_EXP(d->character);
       maxim = (float) exp_needed(GET_LEVEL(d->character)+1);
       if (maxim != 0)
	 percent = (curr/maxim)*100;
       else
	 percent = 0;
       percent = (100 - percent);
       if (percent == 0)
       sprintf(prompt, "%s%s%1.0f%%E%s ", prompt, CCMAG(d->character, C_NRM),
	       percent, CCNRM(d->character, C_NRM));
       else if (percent < 100)
       sprintf(prompt, "%s%s%2.1f%%E%s ", prompt, CCMAG(d->character, C_NRM),
	       percent, CCNRM(d->character, C_NRM));
       else
       sprintf(prompt, "%s%s%3.1f%%E%s ", prompt, CCBMG(d->character, C_NRM),
	       percent, CCNRM(d->character, C_NRM));
    }
#if 0
    if (PRF_FLAGGED(d->character, PRF_DISPTANK) &&
	(FIGHTING(d->character) && FIGHTING(FIGHTING(d->character)) != d->character)) {
      if (GET_MAX_HIT(FIGHTING(FIGHTING(d->character))) > 0)
	percent = (int)((float)GET_HIT(FIGHTING(FIGHTING(d->character)))/(float)GET_MAX_HIT(FIGHTING(FIGHTING(d->character))))*10;
      else percent = 0;
      sprintf(prompt, "%s [Tank: %s]", prompt, percent_hit[(int) percent]);
    }

    if (PRF_FLAGGED(d->character, PRF_DISPENEMY) &&
	FIGHTING(d->character)) {
      if (GET_MAX_HIT(FIGHTING(d->character)) > 0)
	percent = (int)((float)GET_HIT(FIGHTING(d->character))/(float)GET_MAX_HIT(FIGHTING(d->character)))*10;
      else percent = 0;
      sprintf(prompt, "%s [Enemy: %s]", prompt, percent_hit[(int) percent]);
    }
#endif
    if (IS_NPC(d->character))
      sprintf(prompt, "%s%s", prompt, GET_NAME(d->character));

    strcat(prompt, "> ");

    if (PRF_FLAGGED(d->character, PRF_INFOBAR)) {
      strcpy(prompt, "> ");
      do_infobar(d->character, 0, 0, SCMDB_GENUPDATE);
      if (FIGHTING(d->character))
	if (d->character->infobar.opphit != GET_HIT(FIGHTING(d->character)))
	  health_meter(d->character, FIGHTING(d->character));
    }
    write_to_descriptor(d->descriptor, prompt);
  }
}


void write_to_q(char *txt, struct txt_q *queue, int aliased)
{
  struct txt_block *newp;

  CREATE(newp, struct txt_block, 1);
  CREATE(newp->text, char, strlen(txt) + 1);
  strcpy(newp->text, txt);
  newp->aliased = aliased;

  /* queue empty? */
  if (!queue->head) {
    newp->next = NULL;
    queue->head = queue->tail = newp;
  } else {
    queue->tail->next = newp;
    queue->tail = newp;
    newp->next = NULL;
  }
}



int get_from_q(struct txt_q *queue, char *dest, int *aliased)
{
  struct txt_block *tmp;

  /* queue empty? */
  if (!queue->head)
    return 0;

  tmp = queue->head;
  strcpy(dest, queue->head->text);
  *aliased = queue->head->aliased;
  queue->head = queue->head->next;

  free(tmp->text);
  free(tmp);

  return 1;
}



/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
  int dummy;

  if (d->large_outbuf) {
    d->large_outbuf->next = bufpool;
    bufpool = d->large_outbuf;
  }
  while (get_from_q(&d->input, buf2, &dummy));
}


/* Add a new string to a player's output queue */
void write_to_output(const char *txt, struct descriptor_data *t)
{
  int size;

  if (t == NULL)
    return;
  size = strlen(txt);

  /* if we're in the overflow state already, ignore this new output */
  if (t->bufptr < 0)
    return;

  /* if we have enough space, just write to buffer and that's it! */
  if (t->bufspace >= size) {
    strcpy(t->output + t->bufptr, txt);
    t->bufspace -= size;
    t->bufptr += size;
    return;
  }

  /*
   * If we're already using the large buffer, or if even the large buffer
   * is too small to handle this new text, chuck the text and switch to the
   * overflow state.
   */
  if (t->large_outbuf || ((size + strlen(t->output)) > LARGE_BUFSIZE)) {
    t->bufptr = -1;
    buf_overflows++;
    return;
  }

  buf_switches++;

  /* if the pool has a buffer in it, grab it */
  if (bufpool != NULL) {
    t->large_outbuf = bufpool;
    bufpool = bufpool->next;
  } else {                      /* else create a new one */
    CREATE(t->large_outbuf, struct txt_block, 1);
    CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
    buf_largecount++;
  }

  strcpy(t->large_outbuf->text, t->output);     /* copy to big buffer */
  t->output = t->large_outbuf->text;    /* make big buffer primary */
  strcat(t->output, txt);       /* now add new text */

  /* calculate how much space is left in the buffer */
  t->bufspace = LARGE_BUFSIZE - 1 - strlen(t->output);

  /* set the pointer for the next write */
  t->bufptr = strlen(t->output);
}


/*
 * infobar1 (send_to_q function)
 */
void send_to_q(const char *txt, struct descriptor_data * d)
{
   char tempbuf[80];

   if (d == 0)
     return;

   /* restore cursor position in output window -- i.e. 1 or 7 */
   if (d != NULL && d->character != NULL) {
     if (!IS_NPC(d->character) &&
         PRF_FLAGGED(d->character, PRF_INFOBAR)) {
       if (d->connected == CON_PLAYING) {
         if (PRF_FLAGGED(d->character, PRF_SCOREBAR))
           write_to_output("\033[7;1H", d);
         else
           write_to_output("\033[1;1H", d); // CURSOR_POS_RESTORE
       }
     }
   }
   /* write output text */
   write_to_output(txt, d);

   /* save position in output window and return to input line */
   if (d != NULL && d->character != NULL)
     if (!IS_NPC(d->character) && PRF_FLAGGED(d->character, PRF_INFOBAR))
       if (d->connected == CON_PLAYING) {
    	 write_to_output(CURSOR_POS_SAVE, d);
	 sprintf(tempbuf, "%s%s%s%s", CURSOR_POS_SAVE, scrpos(24,1,d->character), KNRM, CLEAR_CURSOR_TO_END);
	 write_to_output(tempbuf, d);
       }
}

/* ******************************************************************
 *  socket handling                                                 *
 ****************************************************************** */

/* Initialize a descriptor */
void init_descriptor (struct descriptor_data *newd, int desc)
{
  static int last_desc = 0;	/* last descriptor number */
    
  newd->descriptor = desc;
  newd->idle_tics = 0;
  newd->output = newd->small_outbuf;
  newd->bufspace = SMALL_BUFSIZE - 1;
  newd->login_time = time(0);
  *newd->output = '\0';
  newd->bufptr = 0;
  // newd->has_prompt = 1;  /* prompt is part of greetings */
  STATE(newd) = CON_NEW_CONN;
  // CREATE(newd->history, char *, HISTORY_SIZE);
  if (++last_desc == 1000)
    last_desc = 1;
  newd->desc_num = last_desc;
}

int new_descriptor(int s)
{
  int desc, sockets_connected = 0;
  unsigned long addr;
  int i;
  struct descriptor_data *newd;
  struct sockaddr_in peer;
  struct hostent *from;

  /* accept the new connection */
  i = sizeof(peer);
  if ((desc = accept(s, (struct sockaddr *) &peer, &i)) < 0) {
    perror("accept");
    return -1;
  }
  /* keep it from blocking */
  nonblock(desc);

  /* make sure we have room for it */
  for (newd = descriptor_list; newd; newd = newd->next)
    sockets_connected++;

  if (sockets_connected >= max_players) {
    write_to_descriptor(desc,
                        "Sorry, The Dominion is full right now... please try again later!\r\n");
    close(desc);
    return 0;
  }
  /* create a new descriptor */
  CREATE(newd, struct descriptor_data, 1);
  memset((char *) newd, 0, sizeof(struct descriptor_data));

  /* find the sitename */
  if (nameserver_is_slow || !(from = gethostbyaddr((char *) &peer.sin_addr,
				      sizeof(peer.sin_addr), AF_INET))) {
    if (!nameserver_is_slow)
      perror("gethostbyaddr");
    addr = ntohl(peer.sin_addr.s_addr);
    sprintf(newd->host, "%03u.%03u.%03u.%03u", (int) ((addr & 0xFF000000) >> 24),
	    (int) ((addr & 0x00FF0000) >> 16), (int) ((addr & 0x0000FF00) >> 8),
	    (int) ((addr & 0x000000FF)));
  } else {
    strncpy(newd->host, from->h_name, HOST_LENGTH);
    *(newd->host + HOST_LENGTH) = '\0';
  }

  /* determine if the site is banned */
  if (isbanned(newd->host) == BAN_ALL) {
    close(desc);
    sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
    mudlog(buf2, CMP, LVL_GOD, TRUE);
    free(newd);
    return 0;
  }
  /* Log new connections - probably unnecessary, but you may want it */
  sprintf(buf2, "New connection from [%s]", newd->host);
  mudlog(buf2, CMP, LVL_GOD, FALSE);

  /* initialize descriptor data */
  init_descriptor(newd, desc);

  /* prepend to list */
  newd->next = descriptor_list;
  descriptor_list = newd;

  /* usual junk to comply with the license */
  sprintf( buf, "\r\n" \
	   "                     The Dominion\r\n\r\n" \
           "          A derivative of DikuMUD (GAMMA 0.0),\r\n" \
           "    created by Hans-Henrik Staerfeldt, Katja Nyboe,\r\n" \
           "   Tom Madsen, Michael Seifert, and Sebastian Hammer\r\n\r\n\r\n" );
  /* now ask the user if they can see colors, to find out if their term is ANSI compatible */
  switch (number(0, 5)) {
    case 0:
      sprintf(buf, "%s\r\n%sIs this text in color (specifically red)?%s ", 
	      buf, KBRD, KNRM);
      break;
    case 1:
      sprintf(buf, "%s\r\n%sIs this text in color (specifically yellow)?%s ", 
	      buf, KYEL, KNRM);
      break;
    case 2:
      sprintf(buf, "%s\r\n%sIs this text in color (specifically green)?%s ", 
	      buf, KBGR, KNRM);
      break;
    case 3:
      sprintf(buf, "%s\r\n%sIs this text in color (specifically magenta)?%s ", 
	      buf, KBMG, KNRM);
      break;
    case 4:
      sprintf(buf, "%s\r\n%sIs this text in color (specifically cyan)?%s ", 
	      buf, KBCN, KNRM);
      break;
    case 5: 
    default:
      sprintf(buf, "%s\r\n%sIs this text in color (specifically blue)?%s ", 
	      buf, KBBL, KNRM);
      break;
  }
  SEND_TO_Q(buf, newd);

  return 0;
}



int process_output(struct descriptor_data *t)
{
  static char i[LARGE_BUFSIZE + GARBAGE_SPACE];
  static int  result;

  /* we may need this \r\n for later -- see below */
  strcpy(i, "\r\n");

  /* now, append the 'real' output */
  strcpy(i + 2, t->output);

  /* if we're in the overflow state, notify the user */
  if (t->bufptr < 0)
    strcat(i, "**OVERFLOW**");

  /* add the extra CRLF if the person isn't in compact mode */
  if (!t->connected && t->character && !PRF_FLAGGED(t->character, PRF_COMPACT)) {
    if (!PRF_FLAGGED(t->character, PRF_INFOBAR))
      strcat(i + 2, "\r\n");
    else
      sprintf(i,"%s\e8\r\n\e7%s",i,scrpos(24,1,t->character));
  }

  if (!t->connected && t->character && PRF_FLAGGED(t->character, PRF_COMPACT) &&
      PRF_FLAGGED(t->character, PRF_INFOBAR))
    sprintf(i,"%s%s",i,scrpos(24, 1, t->character));

  /*
   * now, send the output.  If this is an 'interruption', use the prepended
   * CRLF, otherwise send the straight output sans CRLF.
   */
  if (!t->prompt_mode)          /* && !t->connected) */
    result = write_to_descriptor(t->descriptor, i);
  else
    result = write_to_descriptor(t->descriptor, i + 2);

  /* handle snooping: prepend "% " and send to snooper */
  if (t->snoop_by) {
    SEND_TO_Q("% ", t->snoop_by);
    SEND_TO_Q(t->output, t->snoop_by);
    SEND_TO_Q("%%", t->snoop_by);
  }
  /*
   * if we were using a large buffer, put the large buffer on the buffer pool
   * and switch back to the small one
   */
  if (t->large_outbuf) {
    t->large_outbuf->next = bufpool;
    bufpool = t->large_outbuf;
    t->large_outbuf = NULL;
    t->output = t->small_outbuf;
  }
  /* reset total bufspace back to that of a small buffer */
  t->bufspace = SMALL_BUFSIZE - 1;
  t->bufptr = 0;
  *(t->output) = '\0';

  return result;
}



int write_to_descriptor(int desc, char *txt)
{
  int total, bytes_written;

  total = strlen(txt);

  do {
    if ((bytes_written = write(desc, txt, total)) < 0) {
#ifdef EWOULDBLOCK
      if (errno == EWOULDBLOCK)
	errno = EAGAIN;
#endif
      if (errno == EAGAIN) {
	log("process_output: socket write would block, about to close");
      } else
	perror("Write to socket");
      return -1;
    } else {
      txt += bytes_written;
      total -= bytes_written;
    }
  } while (total > 0);

  return 0;
}


/*
 * ASSUMPTION: There will be no newlines in the raw input buffer when this
 * function is called.  We must maintain that before returning.
 */
int process_input(struct descriptor_data *t)
{
  int buf_length, bytes_read, space_left, failed_subst;
  char *ptr, *read_point, *write_point, *nl_pos = NULL;
  char tmp[MAX_INPUT_LENGTH + 8];

  /* first, find the point where we left off reading data */
  buf_length = strlen(t->inbuf);
  read_point = t->inbuf + buf_length;
  space_left = MAX_RAW_INPUT_LENGTH - buf_length - 1;

  do {
    if (space_left <= 0) {
      log("process_input: about to close connection: input overflow");
      return -1;
    }
    if ((bytes_read = read(t->descriptor, read_point, space_left)) < 0) {

#ifdef EWOULDBLOCK
      if (errno == EWOULDBLOCK)
	errno = EAGAIN;
#endif
      if (errno != EAGAIN) {
	perror("process_input: about to lose connection");
	return -1;              /* some error condition was encountered on
				 * read */
      } else
	return 0;               /* the read would have blocked: just means no
				 * data there but everything's okay */
    } else if (bytes_read == 0) {
      log("EOF on socket read (connection broken by peer)");
      return -1;
    }
    /* at this point, we know we got some data from the read */

    *(read_point + bytes_read) = '\0';  /* terminate the string */

    /* search for a newline in the data we just read */
    for (ptr = read_point; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
	nl_pos = ptr;

    read_point += bytes_read;
    space_left -= bytes_read;

/*
 * on some systems such as AIX, POSIX-standard nonblocking I/O is broken,
 * causing the MUD to hang when it encounters input not terminated by a
 * newline.  This was causing hangs at the Password: prompt, for example.
 * I attempt to compensate by always returning after the _first_ read, instead
 * of looping forever until a read returns -1.  This simulates non-blocking
 * I/O because the result is we never call read unless we know from select()
 * that data is ready (process_input is only called if select indicates that
 * this descriptor is in the read set).  JE 2/23/95.
 */
#if !defined(POSIX_NONBLOCK_BROKEN)
  } while ((nl_pos == NULL)); // && (STATE(t) != CON_PICO));
#else
  } while (0);

  if ((nl_pos == NULL)) // && (STATE(t) != CON_PICO))
    return 0;
#endif

  /*
   * okay, at this point we have at least one newline in the string; now we
   * can copy the formatted data to a new array for further processing.
   */
  read_point = t->inbuf;

  /*
  if (STATE(t) == CON_PICO)
    for (ptr = read_point; *ptr && !nl_pos; ptr++)
      if (*(ptr+1) == '\0')
	nl_pos = ptr;
  */
  while (nl_pos != NULL) {
    write_point = tmp;
    space_left = MAX_INPUT_LENGTH - 1;

    for (ptr = read_point; (space_left > 0) && (ptr < nl_pos); ptr++) {
      if (*ptr == '\b') {       /* handle backspacing */
	if (write_point > tmp) {
	  if (*(--write_point) == '$') {
	    write_point--;
	    space_left += 2;
	  } else
	    space_left++;
	}
      } else if (isascii((int)*ptr) && isprint((int)*ptr)) {
	if ((*(write_point++) = *ptr) == '$') {         /* copy one character */
	  *(write_point++) = '$';       /* if it's a $, double it */
	  space_left -= 2;
	} else
	  space_left--;
      }
    }

    *write_point = '\0';

    if ((space_left <= 0) && (ptr < nl_pos)) {
      char buffer[MAX_INPUT_LENGTH + 64];

      sprintf(buffer, "Line too long.  Truncated to:\r\n%s\r\n", tmp);
      if (write_to_descriptor(t->descriptor, buffer) < 0)
	return -1;
    }
    if (t->snoop_by) {
      SEND_TO_Q("% ", t->snoop_by);
      SEND_TO_Q(tmp, t->snoop_by);
      SEND_TO_Q("\r\n", t->snoop_by);
    }
    failed_subst = 0;

    if (*tmp == '!')
      strcpy(tmp, t->last_input);
    else if (*tmp == '^') {
      if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
	strcpy(t->last_input, tmp);
    } else
      strcpy(t->last_input, tmp);

    if (!failed_subst)
      write_to_q(tmp, &t->input, 0);

    /* find the end of this line */
    while (ISNEWL(*nl_pos))
      nl_pos++;

    /* see if there's another newline in the input buffer */
    read_point = ptr = nl_pos;
    for (nl_pos = NULL; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
	nl_pos = ptr;
  }

  /* now move the rest of the buffer up to the beginning for the next pass */
  write_point = t->inbuf;
  while (*read_point)
    *(write_point++) = *(read_point++);
  *write_point = '\0';

  return 1;
}



/*
 * perform substitution for the '^..^' csh-esque syntax
 * orig is the orig string (i.e. the one being modified.
 * subst contains the substition string, i.e. "^telm^tell"
 */
int perform_subst(struct descriptor_data *t, char *orig, char *subst)
{
  char newp[MAX_INPUT_LENGTH + 5];

  char *first, *second, *strpos;

  /*
   * first is the position of the beginning of the first string (the one
   * to be replaced
   */
  first = subst + 1;

  /* now find the second '^' */
  if (!(second = strchr(first, '^'))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);
    return 1;
  }
  /* terminate "first" at the position of the '^' and make 'second' point
   * to the beginning of the second string */
  *(second++) = '\0';

  /* now, see if the contents of the first string appear in the original */
  if (!(strpos = strstr(orig, first))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);
    return 1;
  }
  /* now, we construct the new string for output. */

  /* first, everything in the original, up to the string to be replaced */
  strncpy(newp, orig, (strpos - orig));
  newp[(strpos - orig)] = '\0';

  /* now, the replacement string */
  strncat(newp, second, (MAX_INPUT_LENGTH - strlen(newp) - 1));

  /* now, if there's anything left in the original after the string to
   * replaced, copy that too. */
  if (((strpos - orig) + strlen(first)) < strlen(orig))
    strncat(newp, strpos + strlen(first), (MAX_INPUT_LENGTH - strlen(newp) - 1));

  /* terminate the string in case of an overflow from strncat */
  newp[MAX_INPUT_LENGTH - 1] = '\0';
  strcpy(subst, newp);

  return 0;
}



void close_socket(struct descriptor_data *d)
{
  char buf[128];
  struct descriptor_data *temp;
  long target_idnum = -1;

  close(d->descriptor);
  flush_queues(d);

  /* Forget snooping */
  if (d->snooping)
    d->snooping->snoop_by = NULL;

  if (d->snoop_by) {
    SEND_TO_Q("Your victim is no longer among us.\r\n", d->snoop_by);
    d->snoop_by->snooping = NULL;
  }
  /*. Kill any OLC stuff .*/
  switch (d->connected) {
     case CON_EDITTING:
       cleanup_olc(d, CLEANUP_ALL);
     default:
       break;
  }
  if (d->character) {
    target_idnum = GET_IDNUM(d->character);
    if (d->connected == CON_PLAYING) {
      save_char(d->character, NOWHERE);
      act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
      sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
      mudlog(buf, NRM, MAX((int)LVL_IMMORT, (int)GET_INVIS_LEV(d->character)), TRUE);
      d->character->desc = NULL;
    } else {
      sprintf(buf, "Losing player: %s.",
	      GET_NAME(d->character) ? GET_NAME(d->character) : "<null>");
      mudlog(buf, CMP, LVL_IMMORT, TRUE);
      free_char(d->character);
    }
  } else
    mudlog("Losing descriptor without char.", CMP, LVL_IMMORT, TRUE);

  /* JE 2/22/95 -- part of my unending quest to make switch stable */
  if (d->original && d->original->desc)
    d->original->desc = NULL;

  REMOVE_FROM_LIST(d, descriptor_list, next);

  if (d->showstr_head)
    free(d->showstr_head);
  if (d->showstr_count)
    free(d->showstr_vector);

  free(d);
}



void check_idle_passwords(void)
{
  struct descriptor_data *d, *next_d;

  for (d = descriptor_list; d; d = next_d) {
    next_d = d->next;
    if (STATE(d) != CON_PASSWORD)
      continue;
    if (!d->idle_tics) {
      d->idle_tics++;
      continue;
    } else {
      echo_on(d);
      SEND_TO_Q("\r\nTimed out... goodbye.\r\n", d);
      STATE(d) = CON_CLOSE;
    }
  }
}



/*
 * I tried to universally convert Circle over to POSIX compliance, but
 * alas, some systems are still straggling behind and don't have all the
 * appropriate defines.  In particular, NeXT 2.x defines O_NDELAY but not
 * O_NONBLOCK.  Krusty old NeXT machines!  (Thanks to Michael Jones for
 * this and various other NeXT fixes.)
 */
#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void nonblock(int s)
{
  int flags;

  flags = fcntl(s, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (fcntl(s, F_SETFL, flags) < 0) {
    perror("Fatal error executing nonblock (comm.c)");
    exit(1);
  }
}


/* ******************************************************************
*  signal-handling functions (formerly signals.c)                   *
****************************************************************** */


RETSIGTYPE checkpointing(void)
{
  if (!tics) {
    log("WARNING! WARNING! *** Infinite Loop Hit ***");
    log("SYSERR: CHECKPOINT shutdown: tics not updated");
    abort();
  } else
    tics = 0;
}


RETSIGTYPE reread_wizlists(void)
{
  void reboot_wizlists(void);

  mudlog("Signal received - rereading wizlists.", CMP, LVL_IMMORT, TRUE);
  reboot_wizlists();
}


RETSIGTYPE unrestrict_game(void)
{
  extern struct ban_list_element *ban_list;
  extern int num_invalid;

  mudlog("Received SIGUSR2 - completely unrestricting game (emergent)",
	 BRF, LVL_IMMORT, TRUE);
  ban_list = NULL;
  game_restrict = 0;
  num_invalid = 0;
}


RETSIGTYPE hupsig(void)
{
  /* we should really save stuff here or something */
  log("Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
  exit(0);                      /* perhaps something more elegant should
				 * substituted */
}


/*
 * This is an implementation of signal() using sigaction() for portability.
 * (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
 * Programming in the UNIX Environment_.  We are specifying that all system
 * calls _not_ be automatically restarted for uniformity, because BSD systems
 * do not restart select(), even if SA_RESTART is used.
 *
 * Note that NeXT 2.x is not POSIX and does not have sigaction; therefore,
 * I just define it to be the old signal.  If your system doesn't have
 * sigaction either, you can use the same fix.
 *
 * SunOS Release 4.0.2 (sun386) needs this too, according to Tim Aldric.
 */

#ifndef POSIX
#define my_signal(signo, func) signal(signo, func)
#else
#ifdef ultrix
sigfunc *my_signal(int signo, sigfunc * func)
#else
void (*my_signal(int signo, void (*func)()) )()
#endif /* ultrix */
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
#ifdef SA_INTERRUPT
  act.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif

  if (sigaction(signo, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}
#endif                          /* NeXT */


void signal_setup(void)
{
  struct itimerval itime;
  struct timeval interval;

  /* user signal 1: reread wizlists.  Used by autowiz system. */
  my_signal(SIGUSR1, reread_wizlists);

  /*
   * user signal 2: unrestrict game.  Used for emergencies if you lock
   * yourself out of the MUD somehow.  (Duh...)
   */
  my_signal(SIGUSR2, unrestrict_game);

  /*
   * set up the deadlock-protection so that the MUD aborts itself if it gets
   * caught in an infinite loop for more than 3 minutes
   */
  interval.tv_sec = 180;
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, NULL);
  my_signal(SIGVTALRM, checkpointing);

  /* just to be on the safe side: */
  my_signal(SIGHUP,  hupsig);
  my_signal(SIGINT,  hupsig);
  my_signal(SIGTERM, hupsig);
  my_signal(SIGPIPE, SIG_IGN);
  my_signal(SIGALRM, SIG_IGN);
}



/* ****************************************************************
*       Public routines for system-to-player-communication        *
**************************************************************** */

void send_to_char(char *messg, struct char_data *ch)
{
  if (ch->desc && messg)
    SEND_TO_Q(messg, ch->desc);
}


void send_to_all(char *messg)
{
  struct descriptor_data *i;

  if (messg)
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
	SEND_TO_Q(messg, i);
}


void send_to_outdoor(char *messg)
{
  struct descriptor_data *i;

  if (!messg || !*messg)
    return;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character && AWAKE(i->character) &&
	OUTSIDE(i->character) && !PLR_FLAGGED(i->character, PLR_WRITING))
      SEND_TO_Q(messg, i);
}

void send_to_zone(char *messg, int zone)
{
  struct descriptor_data *i;

  if (!messg || !*messg)
    return;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character && AWAKE(i->character) &&
	OUTSIDE(i->character) && !PLR_FLAGGED(i->character, PLR_WRITING))
      if (world[i->character->in_room].zone == zone)
	SEND_TO_Q(messg, i);
}


void send_to_room(char *messg, int room)
{
  struct char_data *i;

  if (messg)
    for (i = world[room].people; i; i = i->next_in_room)
      if (i->desc)
	SEND_TO_Q(messg, i->desc);
}



char *ACTNULL = "<NULL>";

#define CHECK_NULL(pointer, expression) \
  if ((pointer) == NULL) i = ACTNULL; else i = (expression);


/* higher-level communication: the act() function */
void perform_act(char *orig, struct char_data *ch, struct obj_data *obj,
		 void *vict_obj, struct char_data *to)
{
  register char *i = NULL, *buf;
  static char lbuf[MAX_STRING_LENGTH];

  buf = lbuf;

  for (;;) {
    if (*orig == '$') {
      switch (*(++orig)) {
      case 'n':
	i = PERS(ch, to);
	break;
      case 'N':
	CHECK_NULL(vict_obj, PERS((struct char_data *) vict_obj, to));
	break;
      case 'm':
	i = HMHR(ch);
	break;
      case 'M':
	CHECK_NULL(vict_obj, HMHR((struct char_data *) vict_obj));
	break;
      case 's':
	i = HSHR(ch);
	break;
      case 'S':
	CHECK_NULL(vict_obj, HSHR((struct char_data *) vict_obj));
	break;
      case 'e':
	i = HSSH(ch);
	break;
      case 'E':
	CHECK_NULL(vict_obj, HSSH((struct char_data *) vict_obj));
	break;
      case 'o':
	CHECK_NULL(obj, OBJN(obj, to));
	break;
      case 'O':
	CHECK_NULL(vict_obj, OBJN((struct obj_data *) vict_obj, to));
	break;
      case 'p':
	CHECK_NULL(obj, OBJS(obj, to));
	break;
      case 'P':
	CHECK_NULL(vict_obj, OBJS((struct obj_data *) vict_obj, to));
	break;
      case 'a':
	CHECK_NULL(obj, SANA(obj));
	break;
      case 'A':
	CHECK_NULL(vict_obj, SANA((struct obj_data *) vict_obj));
	break;
      case 'T':
	CHECK_NULL(vict_obj, (char *) vict_obj);
	break;
      case 'F':
	CHECK_NULL(vict_obj, fname((char *) vict_obj));
	break;
      case '%':
	if (ch == to)
	  i = "";
	else
	  i = "s";
	break;
      case '$':
	i = "$";
	break;
      default:
	log("SYSERR: Illegal $-code to act():");
	strcpy(buf1, "SYSERR: ");
	strcat(buf1, orig);
	log(buf1);
	break;
      }
      while ((*buf = *(i++)))
	buf++;
      orig++;
    } else if (!(*(buf++) = *(orig++)))
      break;
  }

  *(--buf) = '\r';
  *(++buf) = '\n';
  *(++buf) = '\0';

  if (to->desc != NULL)
    send_to_q(CAP(lbuf), to->desc);
  else if (IS_NPC(to))
    mprog_wordlist_check(lbuf, to, ch, NULL, NULL, ACT_PROG);
}

#if 0
#define SENDOK(ch) ((ch)->desc && (AWAKE(ch) || sleep) && \
		    !PLR_FLAGGED((ch), PLR_WRITING))
#endif
#define SENDOK(ch) ((AWAKE(ch) || sleep) && \
		    !PLR_FLAGGED((ch), PLR_WRITING))

void act(char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type)
{
  struct char_data *to = NULL;
  static int sleep;

  if (!str || !*str)
    return;

  /*
   * Warning: the following TO_SLEEP code is a hack.
   * 
   * I wanted to be able to tell act to deliver a message regardless of sleep
   * without adding an additional argument.  TO_SLEEP is 128 (a single bit
   * high up).  It's ONLY legal to combine TO_SLEEP with one other TO_x
   * command.  It's not legal to combine TO_x's with each other otherwise.
   */

  /* check if TO_SLEEP is there, and remove it if it is. */
  if ((sleep = (type & TO_SLEEP)))
    type &= ~TO_SLEEP;

  if (type == TO_CHAR) {
    if (ch && SENDOK(ch))
      perform_act(str, ch, obj, vict_obj, ch);
    return;
  }
  if (type == TO_VICT) {
    if ((to = (struct char_data *) vict_obj) && SENDOK(to))
      perform_act(str, ch, obj, vict_obj, to);
    return;
  }
  /* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */

  if (ch && ch->in_room != NOWHERE)
    to = world[ch->in_room].people;
  else if (obj && obj->in_room != NOWHERE)
    to = world[obj->in_room].people;
#if 0  
  else if (vict_obj && vict_obj->in_room != NOWHERE)
    to = world[vict_obj->in_room].people;
#endif
  else {
    log("SYSERR: no valid target to act()!");
    return;
  }

  for (; to; to = to->next_in_room)
    if (SENDOK(to) && !(hide_invisible && ch && !CAN_SEE(to, ch)) &&
	(to != ch) && (type == TO_ROOM || (to != vict_obj)))
      perform_act(str, ch, obj, vict_obj, to);

}
