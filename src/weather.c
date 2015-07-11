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

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);
int  is_empty(int znr);

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

  for (i = 0;  i < top_of_zone_table; ++i) {
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
}


void weather_change(void)
{
  int diff, change = 0, i;

  extern long top_of_zone_table;

  for (i = 0; i <= top_of_zone_table; i++) {
    /* Why waste cycles with an empty zone? */
#if 0
    if (is_empty(i))
      continue;
#endif
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
#if 0
    /* winter -- colder temperatures */
    if ((time_info.month >= 15) && (time_info.month <= 2))
      zone_table[i].weather_info.temperature -= (dice(2, 5) - dice(1, 5));
    /* spring -- warmer perhaps */
    else if ((time_info.month >= 3) && (time_info.month <= 7))
      zone_table[i].weather_info.temperature += (dice(2, 5) - dice(2, 5));
    /* summer -- warmer temperatures */
    else if ((time_info.month >= 8) && (time_info.month <= 11))
      zone_table[i].weather_info.temperature += (dice(2, 5) - dice(1, 5));
    else /* fall 12 to 14 -- colder temperatures perhaps */
      zone_table[i].weather_info.temperature -= (dice(2, 5) - dice(2, 5));
#endif
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
     if ((time_info.month >= 6) && (time_info.month <= 13)) {
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
