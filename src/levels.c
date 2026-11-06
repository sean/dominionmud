/* ***********************************************************************\
*  File:  LEVELS.C                                                        *
*  Usage: Zone level brackets and item levels                             *
*  Programmer(s): Written for The Dominion, November 2026                 *
\*********************************************************************** */

/*
 * Every zone's recommended levels are in the "levels recommended" line of
 * its zone file (zone_data.levelrec), as "<low>-<high>", e.g. "12-20".
 * Zones without one (the Guilds, Immortal Paradise) have no bracket.
 *
 * Items have no level in their files: each one's level is worked out at
 * boot from where it loads, the way the Nov 2026 rebalance sized its
 * stats:
 *
 *   carried or worn by a mob (G, E)    the mob's level
 *   left in a room (O)                 the middle of the zone's bracket
 *   put in a container (P)             the container's level
 *   sold in a shop                     the low end of the shop's zone
 *   loaded nowhere                     the low end of its own zone
 *
 * taking the lowest where it loads in several places.  A player may wear,
 * wield or hold an item from five levels below its level (lights aside).
 * AREAS lists the zones by level for players.
 */

#define __LEVELS_C__

#include "conf.h"
#include "sysdep.h"

#include "protos.h"
#include "shop.h"

extern struct zone_data *zone_table;
extern long top_of_zone_table;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct char_data *mob_proto;
extern struct obj_data *obj_proto;
extern long top_of_objt;
extern struct shop_data *shop_index;
extern long top_shop;

#define LEVEL_GRACE   5                 /* wear items this far above you */

static int *zone_low = NULL, *zone_high = NULL;
static int *obj_levels = NULL;


/* "12-20" -> 12 and 20; anything else is no bracket (0, 0). */
static void parse_bracket(const char *rec, int *low, int *high)
{
  int a = 0, b = 0;

  *low = *high = 0;
  if (rec && sscanf(rec, " %d - %d", &a, &b) == 2 && a >= 1 && b >= a &&
      b <= LVL_IMMORT - 1) {
    *low = a;
    *high = b;
  }
}


int zone_level_low(int znum)
{
  return (zone_low && znum >= 0 && znum <= top_of_zone_table) ? zone_low[znum] : 0;
}


int zone_level_high(int znum)
{
  return (zone_high && znum >= 0 && znum <= top_of_zone_table) ? zone_high[znum] : 0;
}


/* The zone rnum that vnum (a room, mob or object number) belongs to. */
static int zone_of_vnum(long vnum)
{
  int z;

  for (z = 0; z <= top_of_zone_table; z++)
    if (vnum >= zone_table[z].number * 100 && vnum <= zone_table[z].top)
      return z;
  return -1;
}


static void set_level(long rnum, int level)
{
  if (rnum < 0 || rnum > top_of_objt || level < 1)
    return;
  level = MIN(level, LVL_IMMORT - 1);
  if (!obj_levels[rnum] || level < obj_levels[rnum])
    obj_levels[rnum] = level;
}


void compute_levels(void)
{
  int z, c, i, j, mid, lastlvl = 0;
  long lastobj = -1, r;
  struct reset_com *cmd;

  if (zone_low) {
    free(zone_low);
    free(zone_high);
    free(obj_levels);
  }
  CREATE(zone_low, int, top_of_zone_table + 1);
  CREATE(zone_high, int, top_of_zone_table + 1);
  CREATE(obj_levels, int, top_of_objt + 1);

  for (z = 0; z <= top_of_zone_table; z++)
    parse_bracket(zone_table[z].levelrec, &zone_low[z], &zone_high[z]);

  /* where things load */
  for (z = 0; z <= top_of_zone_table; z++) {
    mid = zone_low[z] ? (zone_low[z] + zone_high[z]) / 2 : 0;
    for (c = 0; (cmd = &zone_table[z].cmd[c]) && cmd->command != 'S'; c++) {
      switch (cmd->command) {
      case 'M':
	lastlvl = (cmd->arg1 >= 0) ? GET_LEVEL(mob_proto + cmd->arg1) : 0;
	break;
      case 'G':
      case 'E':
	set_level(cmd->arg1, lastlvl);
	lastobj = cmd->arg1;
	break;
      case 'O':
	set_level(cmd->arg1, mid);
	lastobj = cmd->arg1;
	break;
      case 'P':
	if (cmd->arg3 >= 0 && cmd->arg3 <= top_of_objt)
	  set_level(cmd->arg1, obj_levels[cmd->arg3] ? obj_levels[cmd->arg3] : mid);
	lastobj = cmd->arg1;
	break;
      default:
	break;
      }
    }
  }
  (void) lastobj;

  /* shops: the low end of the shop's zone */
  for (i = 0; i < top_shop; i++) {
    if (!shop_index[i].producing || !shop_index[i].in_room)
      continue;
    z = zone_of_vnum(SHOP_ROOM(i, 0));
    for (j = 0; (r = SHOP_PRODUCT(i, j)) != NOTHING; j++)   /* rnums */
      set_level(r, (z >= 0 && zone_low[z]) ? zone_low[z] : 1);
  }

  /* everything else: the low end of its own zone */
  for (r = 0; r <= top_of_objt; r++)
    if (!obj_levels[r]) {
      z = zone_of_vnum(obj_index[r].vnumber);
      obj_levels[r] = (z >= 0 && zone_low[z]) ? zone_low[z] : 1;
    }

  /* For balancing tools: DOMINION_LEVEL_DUMP=<file> writes the item */
  /* levels and the shopkeepers and their shops' rooms and goods.     */
  if (getenv("DOMINION_LEVEL_DUMP")) {
    FILE *fp = fopen(getenv("DOMINION_LEVEL_DUMP"), "w");

    if (fp) {
      for (r = 0; r <= top_of_objt; r++)
	fprintf(fp, "obj %ld %d\n", obj_index[r].vnumber, obj_levels[r]);
      for (i = 0; i < top_shop; i++) {
	fprintf(fp, "shop %ld keeper %ld room %ld goods", shop_index[i].vnumber,
		SHOP_KEEPER(i) >= 0 ? mob_index[SHOP_KEEPER(i)].vnumber : -1L,
		shop_index[i].in_room ? SHOP_ROOM(i, 0) : -1L);
	for (j = 0; shop_index[i].producing && SHOP_PRODUCT(i, j) != NOTHING; j++)
	  if (SHOP_PRODUCT(i, j) >= 0 && SHOP_PRODUCT(i, j) <= top_of_objt)
	    fprintf(fp, " %ld", obj_index[SHOP_PRODUCT(i, j)].vnumber);
	fprintf(fp, "\n");
      }
      fclose(fp);
    }
  }
}


/* For data files that place items themselves (the Arena's prizes). */
void obj_level_set(long rnum, int level)
{
  if (obj_levels && rnum >= 0 && rnum <= top_of_objt && level >= 1)
    obj_levels[rnum] = MIN(level, LVL_IMMORT - 1);
}


int obj_level(struct obj_data *obj)
{
  if (!obj || !obj_levels || GET_OBJ_RNUM(obj) < 0 || GET_OBJ_RNUM(obj) > top_of_objt)
    return 1;
  return obj_levels[GET_OBJ_RNUM(obj)];
}


/* TRUE if ch may wear, wield or hold obj; tells him if not. */
int level_can_use(struct char_data *ch, struct obj_data *obj)
{
  int lvl = obj_level(obj);

  if (IS_NPC(ch) || IS_IMMORT(ch) || GET_OBJ_TYPE(obj) == ITEM_LIGHT ||
      GET_LEVEL(ch) + LEVEL_GRACE >= lvl)
    return TRUE;
  sprintf(buf, "You are not experienced enough to use $p (level %d).", lvl);
  act(buf, FALSE, ch, obj, 0, TO_CHAR);
  return FALSE;
}


/* AREAS: the zones by level. */
ACMD(do_areas)
{
  int order[1024], n = 0, i, j, t, lvl = GET_LEVEL(ch);
  char line[256];

  if (!zone_low)
    return;
  for (i = 0; i <= top_of_zone_table && n < 1024; i++)
    if (zone_low[i])
      order[n++] = i;
  for (i = 1; i < n; i++)              /* by low end, then high end */
    for (j = i; j > 0 && (zone_low[order[j]] < zone_low[order[j - 1]] ||
			  (zone_low[order[j]] == zone_low[order[j - 1]] &&
			   zone_high[order[j]] < zone_high[order[j - 1]])); j--) {
      t = order[j];
      order[j] = order[j - 1];
      order[j - 1] = t;
    }
  strcpy(buf, "Levels   Area\r\n-------  --------------------------------------\r\n");
  for (i = 0; i < n; i++) {
    t = order[i];
    sprintf(line, "%s%3d-%-3d%s  %s\r\n",
	    (lvl >= zone_low[t] && lvl <= zone_high[t]) ? CCGRN(ch, C_NRM) :
	    (lvl < zone_low[t] ? CCRED(ch, C_NRM) : ""),
	    zone_low[t], zone_high[t], CCNRM(ch, C_NRM), zone_table[t].name);
    strcat(buf, line);
  }
  strcat(buf, "\r\nAreas in green suit your level; red ones are beyond you.  Every area "
	 "has at\r\nleast one foe that calls for a group.\r\n");
  page_string(ch->desc, buf, 1);
}


/* ------------------------------------------------------------------ */
/* The balance formulas, for mobs made or changed at run time.  They  */
/* mirror the rebalance tool that wrote the mob files (Nov 2026):     */
/* an ordinary mob of level L takes a player of level L in level gear */
/* about five rounds to kill and costs him about 30% of his hit       */
/* points; a boss built for a party of p has 2p times the hit points  */
/* and hits 15% harder.                                               */
/* ------------------------------------------------------------------ */

static double hit_chance(double defence, double hitroll)
{
  double c = (101.0 - defence + hitroll) / 100.0;

  /* (not MAX/MIN: they are for longs) */
  return c < 0.05 ? 0.05 : (c > 1.0 ? 1.0 : c);
}

void mob_set_level(struct char_data *mob, int level, int party)
{
  extern struct dex_app_type dex_app[];
  double L, per, dpr, hp, hit, defence;
  int i, n, size;

  if (!mob || !IS_NPC(mob))
    return;
  level = MAX(1, MIN(level, LVL_IMPL));
  L = level;
  defence = dex_app[GET_DEX(mob)].defensive;
  per = 3 + 0.3 * L + 2 + L / 16;                      /* the player's blow    */
  dpr = (1.2 + 0.006 * L) * hit_chance(defence, 0.5 * L + 2 + L / 8) * per *
        (1 - (5 + 0.25 * L) / 100);
  hp = 5 * dpr < 4.0 ? 4.0 : 5 * dpr;
  hit = 0.06 * (15 + 3.5 * L) /
        (1.2 * hit_chance(60, 10 + 0.5 * L) * (1 - (3 + 0.4 * L) / 100));
  if (party > 1) {
    hp *= 2 * party;
    hit *= 1.15;
  }
  if (IS_AFFECTED(mob, AFF_SANCTUARY))
    hp /= 2;

  GET_LEVEL(mob) = level;
  GET_HITROLL(mob) = (int) (10 + 0.5 * L);
  GET_DAMROLL(mob) = 0;
  for (i = 0; i < ARMOR_LIMIT; i++)
    mob->points.armor[i] = (int) (5 + 0.25 * L);
  GET_MAX_HIT(mob) = GET_HIT(mob) = (int) hp;
  n = MAX(1, MIN(1 + level / 12, (int) hit));
  size = MAX(1, (int) (2 * hit / n - 1 + 0.5));
  mob->mob_specials.damnodice = n;
  mob->mob_specials.damsizedice = size;
  GET_EXP(mob) = mob_exp_for_level(level) * MAX(1, 2 * party);
  GET_GOLD(mob) = (long) (2 * L + 0.15 * L * L) * MAX(1, 3 * party);
}
