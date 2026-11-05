/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  WEATHER.C                                    Based on CircleMUD *
*  Usage: Funcs handling the time and weather                             *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All Modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct time_info_data time_info;
extern struct descriptor_data *descriptor_list;
extern const int rev_dir[];
extern char *dirs[];
extern struct room_data *world;

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);
int  is_empty(int znr);
static void weather_hour(void);
static void load_climates(void);
int  has_boat(struct char_data *ch);
void look_at_room(struct char_data *ch, int mode);

void weather_and_time(int mode)
{
  another_hour(mode);
  if (mode)
    weather_change();
}


void another_hour(int mode)
{
  int i;
  extern long top_of_zone_table;

  time_info.hours++;

  for (i = 0;  i <= top_of_zone_table; ++i) {
  if (mode) {
    switch (time_info.hours) {
    case 5:
      zone_table[i].weather_info.sunlight = SUN_RISE;
      send_to_zone("The sun rises in the east.\r\n", i);
      break;
    case 6:
      zone_table[i].weather_info.sunlight = SUN_LIGHT;
      send_to_zone("The day has begun.\r\n", i);
      break;
    case 20:
      zone_table[i].weather_info.sunlight = SUN_SET;
      send_to_zone("The sun slowly disappears in the west.\r\n", i);
      break;
    /* I want to change this one to moon rise */
    case 21:
      zone_table[i].weather_info.sunlight = SUN_DARK;
      send_to_zone("The night has begun.\r\n", i);
      break;
    default:
      break;
    }
  }
  }
  if (time_info.hours > 23) {   /* Changed by HHS due to bug ??? */
    time_info.hours -= 24;
    time_info.day++;

    if (time_info.day > 34) {
      time_info.day = 0;
      time_info.month++;

      if (time_info.month > 16) {
	time_info.month = 0;
	time_info.year++;
      }
    }
  }

  store_mud_time( &time_info );

  weather_hour();               /* temperature, floods, storms (below) */
  routines_hour();              /* NPC daily routines (routine.c) */
  quest_hour();                 /* time limits, lost escorts (quest.c) */
  theater_hour();               /* the Lyceum's performances (theater.c) */
}


void weather_change(void)
{
  int diff, change = 0, i;

  extern long top_of_zone_table;

  for (i = 0; i <= top_of_zone_table; i++) {
    /* Why waste cycles with an empty zone? */
    if (is_empty(i))
      continue;

    if (zone_table[i].weather_info.humidity >= 50)
      diff = -1;
    else
      diff = 1;

    zone_table[i].weather_info.change +=
	 (dice(1, 5) * diff + dice(1, 5) - dice(1, 5));

    zone_table[i].weather_info.humidity +=
	 zone_table[i].weather_info.change;

    zone_table[i].weather_info.humidity =
	 MAX(zone_table[i].weather_info.humidity, 0);

    zone_table[i].weather_info.humidity =
	 MIN(zone_table[i].weather_info.humidity, 100);
    /* winter -- colder temperatures */
    if ((time_info.month >= 15) || (time_info.month <= 2))
      zone_table[i].weather_info.temperature -= (dice(2, 5) - dice(1, 5));
    /* spring -- warmer perhaps */
    else if ((time_info.month >= 3) && (time_info.month <= 7))
      zone_table[i].weather_info.temperature += (dice(2, 5) - dice(2, 5));
    /* summer -- warmer temperatures */
    else if ((time_info.month >= 8) && (time_info.month <= 11))
      zone_table[i].weather_info.temperature += (dice(2, 5) - dice(1, 5));
    else /* fall 12 to 14 -- colder temperatures perhaps */
      zone_table[i].weather_info.temperature -= (dice(2, 5) - dice(2, 5));

    /* winter season */
    if ((time_info.month >= 0) && (time_info.month <= 4)) {
      switch (zone_table[i].weather_info.sky) {
	case SKY_SUNNY:
	  if (zone_table[i].weather_info.humidity > 10)
	    change = 2;
	  break;
	case SKY_CLEAR:
	  if (zone_table[i].weather_info.humidity >40)
	    change = 4;
	  if (zone_table[i].weather_info.humidity <= 10)
	    change = 1;
	  break;
	case SKY_PARTCLOUD:
	  if (zone_table[i].weather_info.humidity > 50)
	    change = 6;
	  if (zone_table[i].weather_info.humidity <= 30)
	    change = 3;
	  break;
	case SKY_CLOUDY:
	  if (zone_table[i].weather_info.humidity > 60)
	    change = 8;
	  if (zone_table[i].weather_info.humidity <= 40)
	    change = 5;
	  break;
	case SKY_SNOWING:
	  if (zone_table[i].weather_info.humidity > 85)
	    change = 10;
	  if (zone_table[i].weather_info.humidity <= 50)
	    change = 7;
	  break;
	case SKY_SNOWSTORM:
	  if (zone_table[i].weather_info.humidity > 95)
	    change = 12;
	  if (zone_table[i].weather_info.humidity <= 75)
	    change = 9;
	  break;
	case SKY_BLIZZARD:
	  if (zone_table[i].weather_info.humidity <= 90)
	    change = 11;
	  break;
	default:
	  change = 0;
	  zone_table[i].weather_info.sky = SKY_PARTCLOUD;
	  break;
       }       /* spring */
     } else if ((time_info.month >= 5) && (time_info.month <= 8)) {
       switch (zone_table[i].weather_info.sky) {
	 case SKY_HEATWAVE:
	   if (zone_table[i].weather_info.humidity > 5)
	     change = 2;
	   break;
	 case SKY_SIZZLING:
	   if (zone_table[i].weather_info.humidity > 20)
	     change = 4;
	   if (zone_table[i].weather_info.humidity <= 5)
	     change = 1;
	   break;
	 case SKY_SUNNY:
	   if (zone_table[i].weather_info.humidity > 40)
	     change = 6;
	   if (zone_table[i].weather_info.humidity <= 10)
	     change = 3;
	   break;
	 case SKY_CLEAR:
	   if (zone_table[i].weather_info.humidity > 50)
	     change = 8;
	   if (zone_table[i].weather_info.humidity <= 30)
	     change = 5;
	   break;
	 case SKY_PARTCLOUD:
	   if (zone_table[i].weather_info.humidity > 60)
	     change = 10;
	   if (zone_table[i].weather_info.humidity <= 40)
	     change = 7;
	   break;
	 case SKY_CLOUDY:
	   if (zone_table[i].weather_info.humidity > 65)
	     change = 12;
	   if (zone_table[i].weather_info.humidity <= 50)
	     change = 9;
	   break;
	 case SKY_RAINING:
	   if (zone_table[i].weather_info.humidity > 85)
	     change = 14;
	   if (zone_table[i].weather_info.humidity <= 55)
	     change = 11;
	   break;
	 case SKY_POURING:
	   if (zone_table[i].weather_info.humidity > 95)
	     change = 16;
	   if (zone_table[i].weather_info.humidity <= 75)
	     change = 13;
	   break;
	 case SKY_THUNDERSTORM:
	   if (zone_table[i].weather_info.humidity > 99)
	     change = 18;
	   if (zone_table[i].weather_info.humidity <= 90)
	     change = 15;
	   break;
	 case SKY_TORNADO:
	   if (zone_table[i].weather_info.humidity <= 95)
	     change = 17;
	   break;
	 default:
	   change = 0;
	   zone_table[i].weather_info.sky = SKY_CLEAR;
	   break;
       }   /* summer */
     } else if ((time_info.month >= 9) && (time_info.month <= 12)) {
       switch (zone_table[i].weather_info.sky) {
	 case SKY_HEATWAVE:
	   if (zone_table[i].weather_info.humidity > 10)
	     change = 2;
	   break;
	 case SKY_SIZZLING:
	   if (zone_table[i].weather_info.humidity > 35)
	     change = 4;
	   if (zone_table[i].weather_info.humidity <= 10)
	     change = 1;
	   break;
	 case SKY_SUNNY:
	   if (zone_table[i].weather_info.humidity >70)
	     change = 6;
	   if (zone_table[i].weather_info.humidity <= 25)
	     change = 3;
	   break;
	 case SKY_CLEAR:
	   if (zone_table[i].weather_info.humidity > 80)
	     change = 8;
	   if (zone_table[i].weather_info.humidity <= 60)
	     change = 5;
	   break;
	 case SKY_PARTCLOUD:
	   if (zone_table[i].weather_info.humidity > 85)
	     change = 10;
	   if (zone_table[i].weather_info.humidity <= 70)
	     change = 7;
	   break;
	 case SKY_CLOUDY:
	   if (zone_table[i].weather_info.humidity > 90)
	     change = 12;
	   if (zone_table[i].weather_info.humidity <= 75)
	     change = 9;
	   break;
	 case SKY_RAINING:
	   if (zone_table[i].weather_info.humidity > 95)
	     change = 14;
	   if (zone_table[i].weather_info.humidity <= 85)
	     change = 11;
	   break;
	 case SKY_POURING:
	   if (zone_table[i].weather_info.humidity > 99)
	     change = 16;
	   if (zone_table[i].weather_info.humidity <= 90)
	     change = 13;
	   break;
	 case SKY_THUNDERSTORM:
	   if (zone_table[i].weather_info.humidity > 99)
	     change = 18;
	   if (zone_table[i].weather_info.humidity <= 95)
	     change = 15;
	   break;
	 case SKY_TORNADO:
	   if (zone_table[i].weather_info.humidity <= 99)
	     change = 17;
	   break;
	 default:
	   change = 0;
	   zone_table[i].weather_info.sky = SKY_CLEAR;
	   break;
       }   /* fall */
     } else if ((time_info.month >= 13) && (time_info.month <= 16)) {
       switch (zone_table[i].weather_info.sky) {
	 case SKY_SUNNY:
	   if (zone_table[i].weather_info.humidity > 15)
	     change = 2;
	   break;
	 case SKY_CLEAR:
	   if (zone_table[i].weather_info.humidity > 50)
	     change = 4;
	   if (zone_table[i].weather_info.humidity <= 15)
	     change = 1;
	   break;
	 case SKY_PARTCLOUD:
	   if (zone_table[i].weather_info.humidity > 65)
	     change = 6;
	   if (zone_table[i].weather_info.humidity <= 40)
	     change = 3;
	   break;
	 case SKY_CLOUDY:
	   if (zone_table[i].weather_info.humidity > 85)
	     change = 8;
	   if (zone_table[i].weather_info.humidity <= 55)
	     change = 5;
	   break;
	 case SKY_SNOWING:
	   if (zone_table[i].weather_info.humidity > 95)
	     change = 10;
	   if (zone_table[i].weather_info.humidity <= 75)
	     change = 7;
	   break;
	 case SKY_SNOWSTORM:
	   if (zone_table[i].weather_info.humidity > 99)
	     change = 12;
	   if (zone_table[i].weather_info.humidity <= 90)
	     change = 9;
	   break;
	 case SKY_BLIZZARD:
	   if (zone_table[i].weather_info.humidity <= 95)
	     change = 11;
	   break;
	 default:
	   change = 0;
	   zone_table[i].weather_info.sky = SKY_PARTCLOUD;
	   break;
       }
     }
     /* the warm-weather table for spring and summer, as chosen above */
     if (weather_season() == SEASON_SPRING || weather_season() == SEASON_SUMMER) {
       switch (change) {
	 case 0:
	   break;
	 case 1:
	   send_to_zone("Perspiration pours down your forehead from the tremendous heat wave.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_HEATWAVE;
	   break;
	 case 2:
	   send_to_zone("The air cools a bit as the heat wave passes.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_SIZZLING;
	   break;
	 case 3:
	   send_to_zone("The temperature starts to get significantly hotter.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_SIZZLING;
	   break;
	 case 4:
	   send_to_zone("A cool breeze signals the end of another unbearable heat wave.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_SUNNY;
	   break;
	 case 5:
	   send_to_zone("The sun beats down warmly upon you.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_SUNNY;
	   break;
	 case 6:
	   send_to_zone("It starts to get cooler, but the sky remains clear.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_CLEAR;
	   break;
	 case 7:
	   send_to_zone("The sky clears as the clouds continue on their journey.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_CLEAR;
	   break;
	 case 8:
	   send_to_zone("Clouds starts to move in overhead.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_PARTCLOUD;
	   break;
	 case 9:
	   send_to_zone("The clouds slowly begin to dissipate.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_PARTCLOUD;
	   break;
	 case 10:
	   send_to_zone("The cloud cover overhead gets thicker.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_CLOUDY;
	   break;
	 case 11:
	   send_to_zone("The rain stops.\r\n", i);
	   zone_table[i].weather_info.sky = SKY_CLOUDY;
	   break;
	 case 12:
	   send_to_zone("You see the first raindrops start to sprinkle around you.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_RAINING;
	   break;
	 case 13:
	   send_to_zone("The rain starts to thin out.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_RAINING;
	   break;
	 case 14:
	   send_to_zone("The rain picks up and you are drenched by the torrential down-pour.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_POURING;
	   break;
	 case 15:
	   send_to_zone("The dull booms of thunder start to subside.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_POURING;
	   break;
	 case 16:
	   send_to_zone("You jump in surprise as you see a surge of lightning strike not too far away.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_THUNDERSTORM;
	   break;
	 case 17:
	   send_to_zone("The dark clouds begin to pass, but the rain continues.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_THUNDERSTORM;
	   break;
	 case 18:
	   send_to_zone("The clouds darken and the funnel of a tornado appears not to far away!\r\n",i);
	   zone_table[i].weather_info.sky = SKY_TORNADO;
	   break;
	 default:
	   break;
       }
     } else {
       switch (change) {
	 case 0:
	   break;
	 case 1:
	   send_to_zone("The chill in the air is replaced by a warm sky.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_SUNNY;
	   break;
	 case 2:
	   send_to_zone("The cool air moves in, replacing the warmth.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_CLEAR;
	   break;
	 case 3:
	   send_to_zone("The clouds blow off and you see the clear sky.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_CLEAR;
	   break;
	 case 4:
	   send_to_zone("Clouds start to move in overhead.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_PARTCLOUD;
	   break;
	 case 5:
	   send_to_zone("The clouds slowly begin to dissapate.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_PARTCLOUD;
	   break;
	 case 6:
	   send_to_zone("The cloud cover overhead gets thicker with a cold chill.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_CLOUDY;
	   break;
	 case 7:
	   send_to_zone("The light snow flurries stop.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_CLOUDY;
	   break;
	 case 8:
	   send_to_zone("A light snow begins to fall.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_SNOWING;
	   break;
	 case 9:
	   send_to_zone("The thick snow slows to a light snow flurry.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_SNOWING;
	   break;
	 case 10:
	   send_to_zone("The flurries get worse and the snow falls heavier.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_SNOWSTORM;
	   break;
	 case 11:
	   send_to_zone("The blizzard lessens, and dwindles to a heavy snowfall.\r\n",i);
	   zone_table[i].weather_info.sky = SKY_SNOWSTORM;
	   break;
	 case 12:
	   send_to_zone("You are blinded by a sudden blizzard!\r\n",i);
	   zone_table[i].weather_info.sky = SKY_BLIZZARD;
	   break;
	 default:
	   break;
       }
     }
   }
}


/* ====================================================================
 * Weather that matters (November 2026)
 *
 *   Temperature  every zone has a climate, worked out at boot from the
 *                sectors of its outdoor rooms, and a temperature (degrees
 *                Celsius) recomputed every hour from the season, the time
 *                of day, the sky and the climate.  See weather_season().
 *   Cold         a mortal outdoors at or below freezing who isn't dressed
 *                for it (a cloak, or fur, wool or leather) pays one extra
 *                movement point per room and regains movement at half
 *                the usual rate.
 *   Storms       gutter unshielded flames (torches, candles) held outdoors,
 *                and blow arrows and thrown weapons off course.
 *   Floods       the exits listed in lib/data/floods (fords, creekbeds,
 *                river landings) close while their zone is in flood: a
 *                spring of heavy rain raises the water, dry weather lets
 *                it fall.
 *   Heat         a mortal outdoors in sweltering heat (over 35 degrees)
 *                drinks from any water he carries, a swig an hour; with
 *                none left he pays one extra movement point per room and
 *                regains movement at half the usual rate.  (Hunger and
 *                thirst are switched off in this game, see gain_condition(),
 *                so heat works through movement like the cold does.)
 *   Ice          the still water listed in lib/data/ice (lakes and pools)
 *                freezes after some hours at -5 degrees or below and thaws
 *                above freezing.  Frozen, it can be walked on without a
 *                boat; when it thaws, anyone standing on it without a boat
 *                scrambles back to the nearest shore.
 * Fishing and foraging (act_othr.c) use the season and the sky too.
 * ==================================================================== */

#define SECT_COUNT       (SECT_JUNGLE + 1)   /* NUM_ROOM_SECTORS is out of date */
#define FLOOD_FILE       "data/floods"
#define ICE_FILE         "data/ice"
#define CLIMATE_FILE     "data/climates"
#define ICE_MAX          6
#define ICE_FROZEN       4              /* thickness at which it bears weight */
#define HEAT_SWELTERING  36             /* degrees: heat starts to tell */
#define FLOOD_MAX        12
#define FLOOD_RISEN      6              /* level at which the zone floods */
#define FLOOD_FALLEN     2              /* level at which it recedes      */

struct flood_exit {
  long room;                            /* real room number */
  int  dir;
};

static struct flood_exit *flood_exits = NULL;
static int num_flood_exits = 0;


/* 0 winter (months 0-4), 1 spring (5-8), 2 summer (9-12), 3 fall (13-16):
   the same seasons the sky changes in weather_change() use. */
int weather_season(void)
{
  if (time_info.month <= 4)
    return SEASON_WINTER;
  if (time_info.month <= 8)
    return SEASON_SPRING;
  if (time_info.month <= 12)
    return SEASON_SUMMER;
  return SEASON_FALL;
}


const char *season_name(int season)
{
  static const char *names[] = { "winter", "spring", "summer", "fall" };

  return names[season & 3];
}


/* Works out each zone's climate from the sectors of its outdoor rooms. */
void weather_init_climates(void)
{
  extern long top_of_world;
  extern long top_of_zone_table;
  long r, z;
  int *count, total, i;

  CREATE(count, int, (top_of_zone_table + 1) * SECT_COUNT);
  for (r = 0; r <= top_of_world; r++) {
    if (ROOM_FLAGGED(r, ROOM_INDOORS) || world[r].sector_type < 0 ||
	world[r].sector_type >= SECT_COUNT ||
	world[r].sector_type == SECT_INSIDE)
      continue;
    count[world[r].zone * SECT_COUNT + world[r].sector_type]++;
  }
  for (z = 0; z <= top_of_zone_table; z++) {
    int *c = count + z * SECT_COUNT;

    for (total = 0, i = 0; i < SECT_COUNT; i++)
      total += c[i];
    if (!total)
      zone_table[z].weather_info.climate = CLIMATE_TEMPERATE;
    else if (c[SECT_GLACIER] * 4 >= total)
      zone_table[z].weather_info.climate = CLIMATE_ARCTIC;
    else if (c[SECT_MOUNTAIN] * 10 >= total * 3)
      zone_table[z].weather_info.climate = CLIMATE_COLD;
    else if (c[SECT_DESERT] * 10 >= total * 3)
      zone_table[z].weather_info.climate = CLIMATE_DESERT;
    else if (c[SECT_JUNGLE] * 10 >= total * 3)
      zone_table[z].weather_info.climate = CLIMATE_TROPICAL;
    else
      zone_table[z].weather_info.climate = CLIMATE_TEMPERATE;
  }
  free(count);

  load_climates();
  for (z = 0; z <= top_of_zone_table; z++)
    update_temperature(z);
}


/* Reads lib/data/climates: "<zone number> <climate>" overrides. */
static void load_climates(void)
{
  extern long top_of_zone_table;
  static const char *names[] = { "temperate", "cold", "arctic", "desert", "tropical", "\n" };
  FILE *fl;
  char line[256], name[64];
  int znum, c, lineno = 0, count = 0;
  long z;

  if (!(fl = fopen(CLIMATE_FILE, "r")))
    return;
  while (fgets(line, sizeof(line), fl)) {
    lineno++;
    if (*line == ';' || *line == '#' || *line == '\n' || *line == '\r')
      continue;
    if (*line == '$')
      break;
    if (sscanf(line, "%d %63s", &znum, name) != 2 ||
	(c = search_block(name, (char **) names, TRUE)) < 0) {
      sprintf(buf, "SYSERR: %s line %d: expected '<zone> <temperate|cold|arctic|desert|tropical>'",
	      CLIMATE_FILE, lineno);
      log(buf);
      continue;
    }
    for (z = 0; z <= top_of_zone_table; z++)
      if (zone_table[z].number == znum)
	break;
    if (z > top_of_zone_table) {
      sprintf(buf, "SYSERR: %s line %d: no zone %d", CLIMATE_FILE, lineno, znum);
      log(buf);
      continue;
    }
    zone_table[z].weather_info.climate = c;
    count++;
  }
  fclose(fl);
  sprintf(buf, "Loaded %d zone climates.", count);
  log(buf);
}


/* Recomputes a zone's temperature for the current hour. */
void update_temperature(int z)
{
  static const int season_base[4] = { 0, 12, 24, 7 };
  struct weather_data *w = &zone_table[z].weather_info;
  int t, h = time_info.hours, night = (h >= 21 || h <= 5);

  t = season_base[weather_season()];

  /* time of day */
  if (night)
    t -= 5;
  else if (h >= 11 && h <= 16)
    t += 4;

  switch (w->climate) {
  case CLIMATE_COLD:     t -= 8;                 break;
  case CLIMATE_ARCTIC:   t -= 18;                break;
  case CLIMATE_DESERT:   t += night ? -6 : 12;   break;
  case CLIMATE_TROPICAL: t += 8;                 break;
  default:                                       break;
  }

  switch (w->sky) {
  case SKY_HEATWAVE:     t += 8;  break;
  case SKY_SIZZLING:     t += 4;  break;
  case SKY_SUNNY:        t += 2;  break;
  case SKY_RAINING:      t -= 2;  break;
  case SKY_POURING:
  case SKY_THUNDERSTORM: t -= 3;  break;
  case SKY_SNOWING:      t -= 3;  break;
  case SKY_SNOWSTORM:    t -= 6;  break;
  case SKY_BLIZZARD:     t -= 10; break;
  default:                        break;
  }

  w->temperature = t + number(-2, 2);
}


const char *temperature_word(int t)
{
  if (t <= -10) return "bitterly cold";
  if (t <= 0)   return "freezing";
  if (t <= 8)   return "cold";
  if (t <= 15)  return "cool";
  if (t <= 22)  return "mild";
  if (t <= 29)  return "warm";
  if (t <= 35)  return "hot";
  return "sweltering";
}


/* TRUE if ch is out in the open (where weather reaches). */
int is_outdoors(struct char_data *ch)
{
  return room_outdoors(ch->in_room);
}


/* TRUE if the room is out in the open. */
int room_outdoors(long room)
{
  if (room == NOWHERE || ROOM_FLAGGED(room, ROOM_INDOORS))
    return FALSE;
  return (SECT(room) != SECT_INSIDE && SECT(room) != SECT_UNDERWATER);
}


/* Weather nobody goes out in if he can help it: 0, or the sky. */
int weather_downpour(int zone)
{
  switch (zone_table[zone].weather_info.sky) {
  case SKY_POURING:
  case SKY_THUNDERSTORM:
  case SKY_TORNADO:
  case SKY_SNOWSTORM:
  case SKY_BLIZZARD:
    return TRUE;
  }
  return FALSE;
}


/* "rain" or "snow", for messages about a downpour in the zone. */
const char *downpour_word(int zone)
{
  int sky = zone_table[zone].weather_info.sky;

  return (sky == SKY_SNOWSTORM || sky == SKY_BLIZZARD) ? "snow" : "rain";
}


int room_temperature(long room)
{
  return zone_table[world[room].zone].weather_info.temperature;
}


/* A cloak, or anything worn that is made of fur, wool or leather. */
int is_warmly_dressed(struct char_data *ch)
{
  struct obj_data *o;
  int i;

  if (GET_EQ(ch, WEAR_BACK))
    return TRUE;
  for (i = 0; i < NUM_WEARS; i++) {
    if (!(o = GET_EQ(ch, i)) || i == WEAR_WIELD || i == WEAR_HOLD)
      continue;
    if (GET_OBJ_TYPE(o) != ITEM_ARMOR && GET_OBJ_TYPE(o) != ITEM_WORN)
      continue;
    if (GET_OBJ_MATERIAL(o) == 5 || GET_OBJ_MATERIAL(o) == 8 ||   /* wool, leather */
	isname("fur", o->name) || isname("pelt", o->name) ||
	isname("wool", o->name) || isname("cloak", o->name) ||
	isname("coat", o->name) || isname("cape", o->name))
      return TRUE;
  }
  return FALSE;
}


/* A mortal player out in freezing weather without warm clothes. */
int feels_cold(struct char_data *ch)
{
  if (IS_NPC(ch) || GET_LEVEL(ch) >= LVL_IMMORT || !is_outdoors(ch))
    return FALSE;
  return (room_temperature(ch->in_room) <= 0 && !is_warmly_dressed(ch));
}


/* Something to drink: a drink container with something in it. */
static struct obj_data *carried_water(struct char_data *ch)
{
  struct obj_data *o;

  if ((o = GET_EQ(ch, WEAR_HOLD)) && GET_OBJ_TYPE(o) == ITEM_DRINKCON &&
      GET_OBJ_VAL(o, 1) != 0)
    return o;
  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_TYPE(o) == ITEM_DRINKCON && GET_OBJ_VAL(o, 1) != 0)
      return o;
  return NULL;
}


/* A mortal player out in sweltering heat with nothing left to drink. */
int feels_hot(struct char_data *ch)
{
  if (IS_NPC(ch) || GET_LEVEL(ch) >= LVL_IMMORT || !is_outdoors(ch))
    return FALSE;
  return (room_temperature(ch->in_room) >= HEAT_SWELTERING && !carried_water(ch));
}


static int sky_is_gale(int sky)
{
  return (sky == SKY_THUNDERSTORM || sky == SKY_TORNADO ||
	  sky == SKY_SNOWSTORM || sky == SKY_BLIZZARD);
}


static int sky_is_storm(int sky)
{
  return (sky_is_gale(sky) || sky == SKY_POURING);
}


/* Percent taken off a shot or throw from ch at vict by the wind. */
int weather_ranged_penalty(struct char_data *ch, struct char_data *vict)
{
  int sky = -1;

  if (is_outdoors(ch))
    sky = WEATHER(ch->in_room);
  else if (vict && is_outdoors(vict))
    sky = WEATHER(vict->in_room);
  if (sky < 0)
    return 0;
  if (sky_is_gale(sky))
    return 15;
  if (sky == SKY_POURING)
    return 5;
  return 0;
}


/* An open flame: not magical, not burning forever, and not in a lantern
   or lamp (or any metal or glass housing). */
static int light_is_shielded(struct obj_data *light)
{
  int m = GET_OBJ_MATERIAL(light);

  if (GET_OBJ_VAL(light, 2) < 0 || IS_OBJ_STAT(light, ITEM_MAGIC))
    return TRUE;
  if (isname("lantern", light->name) || isname("lamp", light->name))
    return TRUE;
  return ((m >= 11 && m <= 21) && m != 20);   /* brass ... adamantium, glass */
}


/* An hour in sweltering heat: a swig of water if there is any. */
static void heat_hour(struct char_data *ch)
{
  void name_from_drinkcon(struct obj_data *obj);
  void weight_change_object(struct obj_data *obj, int weight);
  struct obj_data *o;

  if (!(o = carried_water(ch))) {
    if (AWAKE(ch))
      send_to_char(number(0, 1) ?
		   "The heat is fierce, and your mouth is as dry as dust.  You need water.\r\n" :
		   "Sweat stings your eyes.  You would give a lot for a drink of water.\r\n", ch);
    return;
  }
  if (!AWAKE(ch))
    return;
  act("You take a long swig from $p in the blazing heat.", FALSE, ch, o, 0, TO_CHAR);
  act("$n takes a long swig from $p.", TRUE, ch, o, 0, TO_ROOM);
  if (GET_OBJ_VAL(o, 1) < 0)            /* never runs dry */
    return;
  if (GET_OBJ_WEIGHT(o) > 1)
    weight_change_object(o, -1);
  if (--GET_OBJ_VAL(o, 1) <= 0) {
    GET_OBJ_VAL(o, 1) = 0;
    GET_OBJ_VAL(o, 2) = 0;
    GET_OBJ_VAL(o, 3) = 0;
    name_from_drinkcon(o);
    act("$p is empty.", FALSE, ch, o, 0, TO_CHAR);
  }
}


/* Hourly weather effects on players. */
static void weather_hour_effects(void)
{
  struct descriptor_data *d;
  struct char_data *ch;
  struct obj_data *light;
  int sky;
  static char *shiver[] = {
    "You shiver in the freezing air.  Something warm to wear would help.\r\n",
    "The cold bites at your fingers and ears.\r\n",
    "Your breath steams in the bitter air, and your teeth chatter.\r\n"
  };

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) != CON_PLAYING || !(ch = d->character) || ch->in_room == NOWHERE)
      continue;

    if (feels_cold(ch) && AWAKE(ch))
      send_to_char(shiver[number(0, 2)], ch);
    else if (!IS_NPC(ch) && GET_LEVEL(ch) < LVL_IMMORT && is_outdoors(ch) &&
	     room_temperature(ch->in_room) >= HEAT_SWELTERING)
      heat_hour(ch);

    if (!is_outdoors(ch))
      continue;
    sky = WEATHER(ch->in_room);
    if (!sky_is_storm(sky) || !(light = GET_EQ(ch, WEAR_HOLD)) ||
	GET_OBJ_TYPE(light) != ITEM_LIGHT || GET_OBJ_VAL(light, 2) <= 0 ||
	light_is_shielded(light) || number(1, 8) != 1)
      continue;

    GET_OBJ_VAL(light, 2) = MAX(0, GET_OBJ_VAL(light, 2) - 2);
    if (GET_OBJ_VAL(light, 2) > 0) {
      act(sky == SKY_POURING ? "The rain hisses on $p, and it gutters." :
	  "A gust of wind makes $p gutter and smoke.", FALSE, ch, light, 0, TO_CHAR);
      act("$n's $o gutters in the storm.", TRUE, ch, light, 0, TO_ROOM);
    } else {
      act("The storm blows out $p.", FALSE, ch, light, 0, TO_CHAR);
      act("The storm blows out $n's $o.", TRUE, ch, light, 0, TO_ROOM);
      world[ch->in_room].light--;
    }
  }
}


/* ------------------------------------------------------------------ */
/* Floods                                                             */
/* ------------------------------------------------------------------ */

/* Reads lib/data/floods: "<room vnum> <direction>" per line.  The exit
   (and the exit back, if there is one) floods with its zone. */
void load_floods(void)
{
  FILE *fl;
  char line[256], dirname[64];
  long vnum, r, back;
  int dir, lineno = 0, max = 0;
  struct room_direction_data *ex;

  if (!(fl = fopen(FLOOD_FILE, "r"))) {
    log("No floods file (" FLOOD_FILE "); no exits flood.");
    return;
  }
  while (fgets(line, sizeof(line), fl)) {
    lineno++;
    if (*line == ';' || *line == '#' || *line == '\n' || *line == '\r')
      continue;
    if (*line == '$')
      break;
    if (sscanf(line, "%ld %63s", &vnum, dirname) != 2 ||
	(dir = search_block(dirname, dirs, FALSE)) < 0 ||
	(r = real_room(vnum)) == NOWHERE || !(ex = world[r].dir_option[dir]) ||
	ex->to_room == NOWHERE) {
      sprintf(buf, "SYSERR: %s line %d: expected '<room vnum> <direction>' "
	      "naming an existing exit", FLOOD_FILE, lineno);
      log(buf);
      continue;
    }
    if (num_flood_exits + 2 > max) {
      max = max ? max * 2 : 32;
      RECREATE(flood_exits, struct flood_exit, max);
    }
    SET_BIT(ex->exit_info, EX_FLOODS);
    flood_exits[num_flood_exits].room = r;
    flood_exits[num_flood_exits++].dir = dir;
    back = ex->to_room;
    if ((ex = world[back].dir_option[rev_dir[dir]]) && ex->to_room == r) {
      SET_BIT(ex->exit_info, EX_FLOODS);
      flood_exits[num_flood_exits].room = back;
      flood_exits[num_flood_exits++].dir = rev_dir[dir];
    }
  }
  fclose(fl);
  sprintf(buf, "Loaded %d flooding exits.", num_flood_exits);
  log(buf);
}


/* TRUE if the exit from room in direction dir is under water just now. */
int exit_flooded(long room, int dir)
{
  struct room_direction_data *ex;

  if (room == NOWHERE || dir < 0 || dir >= NUM_OF_DIRS ||
      !(ex = world[room].dir_option[dir]) || !IS_SET(ex->exit_info, EX_FLOODS))
    return FALSE;
  return zone_table[world[room].zone].weather_info.flooded;
}


static void flood_message(int z, const char *msg)
{
  int i;
  long last = NOWHERE;

  for (i = 0; i < num_flood_exits; i++)
    if (world[flood_exits[i].room].zone == z && flood_exits[i].room != last) {
      send_to_room((char *) msg, flood_exits[i].room);
      last = flood_exits[i].room;
    }
}


/* Raises or lowers the water in zone z for the hour. */
static void update_flood(int z)
{
  struct weather_data *w = &zone_table[z].weather_info;
  int i, has = FALSE;

  for (i = 0; i < num_flood_exits && !has; i++)
    if (world[flood_exits[i].room].zone == z)
      has = TRUE;
  if (!has)
    return;

  if (weather_season() == SEASON_SPRING) {
    switch (w->sky) {
    case SKY_RAINING:      w->flood += 1; break;
    case SKY_POURING:      w->flood += 2; break;
    case SKY_THUNDERSTORM:
    case SKY_TORNADO:      w->flood += 3; break;
    default:               w->flood -= 1; break;
    }
  } else
    w->flood -= 2;
  w->flood = MAX(0, MIN(FLOOD_MAX, w->flood));

  if (!w->flooded && w->flood >= FLOOD_RISEN) {
    w->flooded = TRUE;
    flood_message(z, "With a roar, the swollen water bursts its banks!  The way across is flooded.\r\n");
  } else if (w->flooded && w->flood <= FLOOD_FALLEN) {
    w->flooded = FALSE;
    flood_message(z, "The floodwater has gone down, leaving mud and broken branches behind.\r\n");
  }
}


/* ------------------------------------------------------------------ */
/* Ice                                                                */
/* ------------------------------------------------------------------ */

static bool *room_freezes = NULL;       /* indexed by real room number */


static int is_water_room(long r)
{
  return (SECT(r) == SECT_WATER_SWIM || SECT(r) == SECT_WATER_NOSWIM);
}


/* Reads lib/data/ice: one "<vnum>" or "<first vnum>-<last vnum>" per line.
   Only outdoor water rooms in those ranges freeze. */
void load_ice(void)
{
  extern long top_of_world;
  FILE *fl;
  char line[256];
  long from, to, r;
  int lineno = 0, count = 0;

  if (room_freezes)
    free(room_freezes);
  CREATE(room_freezes, bool, top_of_world + 1);

  if (!(fl = fopen(ICE_FILE, "r"))) {
    log("No ice file (" ICE_FILE "); no lakes freeze.");
    return;
  }
  while (fgets(line, sizeof(line), fl)) {
    lineno++;
    if (*line == ';' || *line == '#' || *line == '\n' || *line == '\r')
      continue;
    if (*line == '$')
      break;
    if (sscanf(line, "%ld-%ld", &from, &to) == 2)
      ;
    else if (sscanf(line, "%ld", &from) == 1)
      to = from;
    else {
      sprintf(buf, "SYSERR: %s line %d: expected '<vnum>' or '<vnum>-<vnum>'",
	      ICE_FILE, lineno);
      log(buf);
      continue;
    }
    for (r = 0; r <= top_of_world; r++)
      if (world[r].number >= from && world[r].number <= to &&
	  is_water_room(r) && !ROOM_FLAGGED(r, ROOM_INDOORS) && !room_freezes[r]) {
	room_freezes[r] = TRUE;
	count++;
      }
  }
  fclose(fl);
  sprintf(buf, "Loaded %d freezing water rooms.", count);
  log(buf);
}


/* TRUE if the water in room is frozen hard enough to walk on. */
int room_frozen(long room)
{
  return (room_freezes && room != NOWHERE && room_freezes[room] &&
	  zone_table[world[room].zone].weather_info.iced);
}


/* Moves ch from thawing ice to the nearest room that isn't open water. */
static void scramble_ashore(struct char_data *ch)
{
  extern long top_of_world;
  long *queue, *from, r, to, start = ch->in_room, dest = NOWHERE;
  int head = 0, tail = 0, dir;

  CREATE(queue, long, top_of_world + 1);
  CREATE(from, long, top_of_world + 1);
  for (r = 0; r <= top_of_world; r++)
    from[r] = -2;
  from[start] = -1;
  queue[tail++] = start;
  while (head < tail && dest == NOWHERE) {
    r = queue[head++];
    for (dir = 0; dir < NUM_OF_DIRS && dest == NOWHERE; dir++) {
      if (!world[r].dir_option[dir] ||
	  (to = world[r].dir_option[dir]->to_room) == NOWHERE ||
	  to < 0 || to > top_of_world || from[to] != -2)
	continue;
      if (IS_SET(world[r].dir_option[dir]->exit_info, EX_CLOSED) ||
	  ROOM_FLAGGED(to, ROOM_DEATH))
	continue;
      from[to] = r;
      if (SECT(to) != SECT_WATER_NOSWIM && SECT(to) != SECT_UNDERWATER)
	dest = to;
      else if (room_freezes[to] && tail < 64)
	queue[tail++] = to;               /* keep looking across the lake */
    }
  }
  free(queue);
  free(from);

  if (dest == NOWHERE) {
    send_to_char("The ice heaves and cracks all around you!  You cling to a floating slab.\r\n", ch);
    return;
  }
  send_to_char("The ice groans and splits beneath you!  You slip, splash and scramble\r\n"
	       "across the breaking floes until you reach solid ground, soaked and shaking.\r\n", ch);
  act("$n splashes and scrambles across the breaking ice.", TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, dest);
  act("$n hauls $mself out of the icy water, soaked to the skin.", TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
}


static void ice_message(int z, const char *msg)
{
  extern long top_of_world;
  long r;

  for (r = 0; r <= top_of_world; r++)
    if (room_freezes[r] && world[r].zone == z && world[r].people)
      send_to_room((char *) msg, r);
}


/* Thickens or thins the ice in zone z for the hour. */
static void update_ice(int z)
{
  extern long top_of_world;
  struct weather_data *w = &zone_table[z].weather_info;
  long r;
  int has = FALSE;

  if (!room_freezes)
    return;
  for (r = 0; r <= top_of_world && !has; r++)
    if (room_freezes[r] && world[r].zone == z)
      has = TRUE;
  if (!has)
    return;

  if (w->temperature <= -5)
    weather_set_ice(z, w->ice + 1);
  else if (w->temperature > 0)
    weather_set_ice(z, w->ice - 2);
}


/* Sets the thickness of the ice in zone z, freezing or breaking up the
   water as it crosses the thresholds (also used by "weather ice <n>"). */
void weather_set_ice(int z, int level)
{
  extern long top_of_world;
  struct weather_data *w = &zone_table[z].weather_info;
  struct char_data *ch, *next;
  long r;

  w->ice = MAX(0, MIN(ICE_MAX, level));
  if (!room_freezes)
    return;

  if (!w->iced && w->ice >= ICE_FROZEN) {
    w->iced = TRUE;
    ice_message(z, "The water has frozen hard.  You could walk across the ice now.\r\n");
  } else if (w->iced && w->ice <= 0) {
    ice_message(z, "With a long groan, the ice begins to break up.\r\n");
    w->iced = FALSE;
    for (r = 0; r <= top_of_world; r++) {
      if (!room_freezes[r] || world[r].zone != z || SECT(r) != SECT_WATER_NOSWIM)
	continue;
      for (ch = world[r].people; ch; ch = next) {
	next = ch->next_in_room;
	if (!has_boat(ch) && GET_LEVEL(ch) < LVL_IMMORT)
	  scramble_ashore(ch);
      }
    }
  }
}


/* Called by another_hour() once the clock has moved on. */
static void weather_hour(void)
{
  extern long top_of_zone_table;
  int z;

  for (z = 0; z <= top_of_zone_table; z++) {
    update_temperature(z);
    update_flood(z);
    update_ice(z);
  }
  weather_hour_effects();
}
