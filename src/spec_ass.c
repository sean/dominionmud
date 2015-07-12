/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SPEC_ASS.C                                   Based on CircleMUD *
*  Usage: Assigning the special procedures to the mobs                    *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All Modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "protos.h"

extern struct room_data *world;
extern long   top_of_world;
extern int    mini_mud;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
  SPECIAL(postmaster);
  SPECIAL(cityguard);
  SPECIAL(receptionist);
  SPECIAL(cryogenicist);
  SPECIAL(puff);
  SPECIAL(fido);
  SPECIAL(janitor);
  SPECIAL(mayor);
  SPECIAL(snake);
  SPECIAL(thief);
  SPECIAL(magic_user);
  SPECIAL(breath_acid);
  SPECIAL(breath_any);
  SPECIAL(old_dragon);
  SPECIAL(breath_fire);
  SPECIAL(breath_frost);
  SPECIAL(breath_gas);
  SPECIAL(breath_lightning);
  SPECIAL(arena_entrance);
  SPECIAL(temple_guard);
  SPECIAL(cleric);
  SPECIAL(trainer);
  SPECIAL(smithy);
  SPECIAL(guild);
  SPECIAL(bank);
  SPECIAL(gen_board);
  SPECIAL(immortal_scroll);
  SPECIAL(dump);
  SPECIAL(pet_shops);
  SPECIAL(pray_for_items);
  SPECIAL(tardis);
  SPECIAL(tardis2);
  SPECIAL(lose_mist);
  SPECIAL(frost_shatter);
  SPECIAL(temple);

/* functions to perform assignments */

void ASSIGNMOB(long mob, SPECIAL(fname))
{
  if (real_mobile(mob) >= 0)
    mob_index[real_mobile(mob)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant mob #%ld",
	    mob);
    log(buf);
  }
}

void ASSIGNOBJ(long obj, SPECIAL(fname))
{
  if (real_object(obj) >= 0)
    obj_index[real_object(obj)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant obj #%ld",
	    obj);
    log(buf);
  }
}

void ASSIGNROOM(long room, SPECIAL(fname))
{
  if (real_room(room) >= 0)
    world[real_room(room)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant rm. #%ld",
	    room);
    log(buf);
  }
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
#if 0
  void assign_kings_castle(void);
  assign_kings_castle();
#endif
  /* Immortal Zone */
  ASSIGNMOB(2,  receptionist);
  ASSIGNMOB(3,  postmaster);
  ASSIGNMOB(4,  janitor);

  /* Trainers */
  ASSIGNMOB(3090,  trainer);
  ASSIGNMOB(6607,  trainer);
  ASSIGNMOB(25501, trainer);
  ASSIGNMOB(1803,  trainer);
  ASSIGNMOB(1810,  trainer);
  ASSIGNMOB(3020,  trainer);
  ASSIGNMOB(3021,  trainer);
  ASSIGNMOB(3022,  trainer);
  ASSIGNMOB(3023,  trainer);
  ASSIGNMOB(3099,  trainer); 
  ASSIGNMOB(6419,  trainer);
  ASSIGNMOB(32003, trainer);
  ASSIGNMOB(19017, trainer);
  ASSIGNMOB(3092,  trainer);
  ASSIGNMOB(3041,  trainer);
  ASSIGNMOB(3042,  trainer);
  ASSIGNMOB(3043,  trainer);
  ASSIGNMOB(1813,  trainer);
  ASSIGNMOB(8009,  trainer);
  ASSIGNMOB(1115,  trainer);
   /* Elrhinn */
  ASSIGNMOB(3097,  guild);
  ASSIGNMOB(35005, receptionist);
  ASSIGNMOB(3005,  receptionist);
  ASSIGNMOB(11009, receptionist);
  ASSIGNMOB(19009, receptionist);
  ASSIGNMOB(3010,  postmaster);
  ASSIGNMOB(35015, postmaster);
  ASSIGNMOB(3061,  janitor);
  ASSIGNMOB(3068,  janitor);
  ASSIGNMOB(8017,  janitor);
  ASSIGNMOB(35004, janitor);
/*  ASSIGNMOB(3095,  cryogenicist); */
  ASSIGNMOB(3070,  smithy);
  ASSIGNMOB(35014,  smithy);
#if 0  
  /* High Tower Of Sorcery */
  ASSIGNMOB(2501, cleric); /* should likely be cleric */
  ASSIGNMOB(2504, magic_user);
  ASSIGNMOB(2507, magic_user);
  ASSIGNMOB(2508, magic_user);
  ASSIGNMOB(2510, magic_user);
  ASSIGNMOB(2511, thief);
  ASSIGNMOB(2514, magic_user);
  ASSIGNMOB(2515, magic_user);
  ASSIGNMOB(2516, magic_user);
  ASSIGNMOB(2517, magic_user);
  ASSIGNMOB(2518, magic_user);
  ASSIGNMOB(2520, magic_user);
  ASSIGNMOB(2521, magic_user);
  ASSIGNMOB(2522, magic_user);
  ASSIGNMOB(2523, magic_user);
  ASSIGNMOB(2524, magic_user);
  ASSIGNMOB(2525, magic_user);
  ASSIGNMOB(2526, magic_user);
  ASSIGNMOB(2527, magic_user);
  ASSIGNMOB(2528, magic_user);
  ASSIGNMOB(2529, magic_user);
  ASSIGNMOB(2530, magic_user);
  ASSIGNMOB(2531, magic_user);
  ASSIGNMOB(2532, magic_user);
  ASSIGNMOB(2533, magic_user);
  ASSIGNMOB(2534, magic_user);
  ASSIGNMOB(2536, magic_user);
  ASSIGNMOB(2537, magic_user);
  ASSIGNMOB(2538, magic_user);
  ASSIGNMOB(2540, magic_user);
  ASSIGNMOB(2541, magic_user);
  ASSIGNMOB(2548, magic_user);
  ASSIGNMOB(2549, magic_user);
  ASSIGNMOB(2552, magic_user);
  ASSIGNMOB(2553, magic_user);
  ASSIGNMOB(2554, magic_user);
  ASSIGNMOB(2556, magic_user);
  ASSIGNMOB(2557, magic_user);
  ASSIGNMOB(2559, magic_user);
  ASSIGNMOB(2560, magic_user);
  ASSIGNMOB(2562, magic_user);
  ASSIGNMOB(2564, magic_user);

  /* SEWERS */
  ASSIGNMOB(7006, snake);
  ASSIGNMOB(7009, magic_user);
  ASSIGNMOB(7200, magic_user);
  ASSIGNMOB(7201, magic_user);
  ASSIGNMOB(7202, magic_user);
#endif
  /* FOREST */
  ASSIGNMOB(6113, snake);
  ASSIGNMOB(6114, magic_user);
  ASSIGNMOB(6115, magic_user);
  ASSIGNMOB(6116, cleric); /* should be a cleric */
  ASSIGNMOB(6117, magic_user);

  /* ARACHNOS */
  ASSIGNMOB(6302, magic_user);
  ASSIGNMOB(6309, magic_user);
  ASSIGNMOB(6312, magic_user);
  ASSIGNMOB(6314, magic_user);
  ASSIGNMOB(6315, magic_user);

#if 0
  /* Desert */
  ASSIGNMOB(5004, magic_user);
  ASSIGNMOB(5010, magic_user);
  ASSIGNMOB(5014, magic_user);

  /* Drow City */
  ASSIGNMOB(5103, magic_user);
  ASSIGNMOB(5104, magic_user);
  ASSIGNMOB(5107, magic_user);
  ASSIGNMOB(5108, magic_user);

  /* Old Thalos */
  ASSIGNMOB(5200, magic_user);
  ASSIGNMOB(5201, magic_user);
  ASSIGNMOB(5209, magic_user);

  /* New Thalos */
/* 5481 - Cleric (or Mage... but he IS a high priest... *shrug*) */
  ASSIGNMOB(5404, receptionist);
  ASSIGNMOB(5421, magic_user);
  ASSIGNMOB(5422, magic_user);
  ASSIGNMOB(5423, magic_user);
  ASSIGNMOB(5424, magic_user);
  ASSIGNMOB(5425, magic_user);
  ASSIGNMOB(5426, magic_user);
  ASSIGNMOB(5427, magic_user);
  ASSIGNMOB(5428, magic_user);
  ASSIGNMOB(5434, cityguard);
  ASSIGNMOB(5440, magic_user);
  ASSIGNMOB(5455, magic_user);
  ASSIGNMOB(5461, cityguard);
  ASSIGNMOB(5462, cityguard);
  ASSIGNMOB(5463, cityguard);
  ASSIGNMOB(5482, cityguard);
/*
5400 - Guildmaster (Mage)
5401 - Guildmaster (Cleric)
5402 - Guildmaster (Warrior)
5403 - Guildmaster (Thief)
5456 - Guildguard (Mage)
5457 - Guildguard (Cleric)
5458 - Guildguard (Warrior)
5459 - Guildguard (Thief)
*/
#endif
  ASSIGNMOB(16081, thief);
  ASSIGNMOB(5214,  magic_user);
  /* CHARING CROSS */
  ASSIGNMOB(10012, trainer);
  ASSIGNMOB(10019, trainer);
  ASSIGNMOB(10013, thief);
  ASSIGNMOB(10020, thief);
  ASSIGNMOB(10017, thief);
  ASSIGNMOB(10023, temple_guard);

#if 0
  /* Astral Area */
  ASSIGNMOB(10302, magic_user);
  ASSIGNMOB(10312, magic_user);
  ASSIGNMOB(10305, thief);
  ASSIGNMOB(10313, magic_user);  /*Should be undead!*/
  ASSIGNMOB(10314, magic_user);
  ASSIGNMOB(10317, magic_user);
  ASSIGNMOB(10318, magic_user);
#endif
  /* Ancalador */
  ASSIGNMOB(10403, magic_user);
  ASSIGNMOB(10404, magic_user);
  ASSIGNMOB(10407, magic_user);
  ASSIGNMOB(10499, magic_user);
  ASSIGNMOB(10402, snake);
  ASSIGNMOB(10411, snake);

  /* Vice Island */
  ASSIGNMOB(10516, breath_gas);

  /* Somewhere */
  ASSIGNMOB(11005, old_dragon);
#if 0
  /* ROME */
  ASSIGNMOB(12009, magic_user);
  ASSIGNMOB(12018, cityguard);
  ASSIGNMOB(12020, magic_user);
  ASSIGNMOB(12021, cityguard);
  ASSIGNMOB(12025, magic_user);
  ASSIGNMOB(12030, magic_user);
  ASSIGNMOB(12031, magic_user);
  ASSIGNMOB(12032, magic_user);

  /* King Welmar's Castle (not covered in castle.c) */
  ASSIGNMOB(15015, thief);      /* Ergan... have a better idea? */
  ASSIGNMOB(15032, magic_user); /* Pit Fiend, have something better?  Use it */

  /* DWARVEN KINGDOM */
  ASSIGNMOB(6500, cityguard);
  ASSIGNMOB(6502, magic_user);
  ASSIGNMOB(6509, magic_user);
  ASSIGNMOB(6516, magic_user);
#endif
  /* Western Mountain Areas */
  ASSIGNMOB(25001, cityguard);
  ASSIGNMOB(25002, cityguard);
  ASSIGNMOB(25011, cityguard);
  ASSIGNMOB(25012, cityguard);
  ASSIGNMOB(25013, breath_fire);
#if 0
  ASSIGNMOB(25020, cityguard);
  ASSIGNMOB(25025, fido);
  ASSIGNMOB(25026, fido);
  ASSIGNMOB(25024, magic_user);
  ASSIGNMOB(25027, breath_lightning);
  ASSIGNMOB(25028, magic_user);
#endif
  ASSIGNMOB(25007, breath_lightning);

  /* Druid's Glade */
  ASSIGNMOB(25509, temple_guard);

  /* Thangorodom */
  ASSIGNMOB(26016, cleric);

  /* Monk Monestary */ 
  ASSIGNMOB(6419, trainer);

#if 0
/* Specials for The Keep of Mahn-tor */
  ASSIGNMOB(2200, magic_user);
  ASSIGNMOB(2202, breath_acid);
  ASSIGNMOB(2206, snake);
  ASSIGNMOB(2213, breath_frost);
  ASSIGNMOB(2214, breath_acid);
  ASSIGNMOB(2224, magic_user);
  ASSIGNMOB(2226, magic_user);
  ASSIGNMOB(2228, magic_user);
  ASSIGNMOB(2233, magic_user);

/* Specials for Oceania */
  ASSIGNMOB(2301, breath_fire);
  ASSIGNMOB(2303, breath_any);

/* Specials for Dragon Spyre */
  ASSIGNMOB(2401, breath_frost);
  ASSIGNMOB(2403, old_dragon);
  ASSIGNMOB(2404, breath_frost);
  ASSIGNMOB(2405, magic_user);
  ASSIGNMOB(2407, old_dragon);
  ASSIGNMOB(2409, magic_user);
#endif
}



/* assign special procedures to objects */
void assign_objects(void)
{
  ASSIGNOBJ(3096, gen_board);   /* social board */
  ASSIGNOBJ(3097, gen_board);   /* freeze board */
  ASSIGNOBJ(3098, gen_board);   /* immortal board */
  ASSIGNOBJ(3099, gen_board);   /* mortal board */
  ASSIGNOBJ(3092, gen_board);   /* Glas's Board */
  ASSIGNOBJ(3093, gen_board);   /* Kira's Board */
  ASSIGNOBJ(3094, gen_board);   /* Sand's Board */
  ASSIGNOBJ(3095, gen_board);   /* Garou's Board */

  ASSIGNOBJ(32026, immortal_scroll);   /* scroll to advance people */

  ASSIGNOBJ(3034, bank);        /* money exchng mbr */
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
  extern int dts_are_dumps;
  int i;

  ASSIGNROOM(3031,  pet_shops);
  ASSIGNROOM(3066,  tardis);
  ASSIGNROOM(20,    temple);         /* Glas    */
#if 0
  ASSIGNROOM(XX,    temple);         /* Landrin */
#endif
  ASSIGNROOM(10110, temple);         /* Kirahn  */
  ASSIGNROOM(25567, temple);         /* Garou   */
  ASSIGNROOM(25570, temple);         /* Malachi */
  ASSIGNROOM(25579, temple);         /* Cair    */
  ASSIGNROOM(25574, temple);         /* Sb      */
  ASSIGNROOM(25583, temple);         /* Gaeya   */

#if 0
/* Specials for The Keep of Mahn-tor */
  ASSIGNROOM(2232, lose_mist);
  ASSIGNROOM(2233, lose_mist);
  ASSIGNROOM(2234, lose_mist);
  ASSIGNROOM(2235, lose_mist);
  ASSIGNROOM(2236, lose_mist);
  ASSIGNROOM(2237, lose_mist);

  ASSIGNROOM(2251, frost_shatter);
  ASSIGNROOM(2252, frost_shatter);
  ASSIGNROOM(2254, frost_shatter);
  ASSIGNROOM(2255, frost_shatter);
  ASSIGNROOM(2256, frost_shatter);
  ASSIGNROOM(2257, frost_shatter);
  ASSIGNROOM(2258, frost_shatter);
  ASSIGNROOM(2259, frost_shatter);
#endif

  if (dts_are_dumps)
    for (i = 0; i < top_of_world; i++)
      if (IS_SET(ROOM_FLAGS(i), ROOM_DEATH))
	world[i].func = dump;
}
