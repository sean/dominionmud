/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  PFILE.C                                                         *
*  Usage: Plain-text character records (see pfile.h)                     *
\*********************************************************************** */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "pfile.h"

#define LINE_SIZE  (4 * EXDSCR_LENGTH + 256)

/* ------------------------------------------------------------------ */
/* writing                                                             */

/* One line: key, a space, the string with \n and \\ escaped.  '\r' is
   dropped (the game keeps "\r\n" in memory; the reader puts it back). */
static void put_str(FILE *fp, const char *key, const char *s)
{
  fprintf(fp, "%s ", key);
  for (; *s; s++) {
    if (*s == '\r')
      continue;
    else if (*s == '\n')
      fputs("\\n", fp);
    else if (*s == '\\')
      fputs("\\\\", fp);
    else
      fputc(*s, fp);
  }
  fputc('\n', fp);
}

static void put_num(FILE *fp, const char *key, long long v)
{
  fprintf(fp, "%s %lld\n", key, v);
}

/* the list helpers take the array and its element type through a macro */
#define PUT_LIST(fp, key, arr, n) do {          \
    int i_;                                     \
    fprintf((fp), "%s", (key));                 \
    for (i_ = 0; i_ < (n); i_++)                \
      fprintf((fp), " %lld", (long long) (arr)[i_]); \
    fputc('\n', (fp));                          \
  } while (0)

static void put_affects(FILE *fp, const char *key, const struct affected_type *af)
{
  int i;

  for (i = 0; i < MAX_AFFECT; i++)
    if (af[i].type)
      fprintf(fp, "%s %d %d %d %d %lld\n", key, af[i].type, af[i].duration,
	      af[i].modifier, af[i].location, (long long) af[i].bitvector);
}

int pfile_write_record(FILE *fp, const struct char_file_u *st)
{
  const struct char_special_data_saved *cs = &st->char_specials_saved;
  const struct player_special_data_saved *ps = &st->player_specials_saved;
  const struct char_ability_data *ab = &st->abilities;
  const struct char_point_data *pt = &st->points;
  int i;

  put_num(fp, "Character", PFILE_VERSION);
  /* no Email or Password: the account file holds them for all its characters */
  put_str(fp, "Name", st->name);
  put_num(fp, "Id", cs->idnum);
  put_num(fp, "Level", st->level);
  put_num(fp, "Sex", st->sex);
  put_num(fp, "Race", st->race);
  put_num(fp, "RaceMod", st->race_mod);
  put_num(fp, "Hometown", st->hometown);
  put_num(fp, "Height", st->height);
  put_num(fp, "Weight", st->weight);
  put_num(fp, "Birth", (long long) st->birth);
  put_num(fp, "Played", st->played);
  put_num(fp, "LastLogon", (long long) st->last_logon);
  put_str(fp, "Host", st->host);
  put_str(fp, "Username", st->username);

  put_str(fp, "Title", st->title);
  put_str(fp, "Description", st->description);
  put_str(fp, "PolyDescr", st->poly_descr);
  put_str(fp, "ExtraDescr", st->xtra_descr);
  put_str(fp, "Keywords", st->keywords);
  put_str(fp, "RoomDescr", st->room_descr);
  put_str(fp, "PoofIn", st->spoofin);
  put_str(fp, "PoofOut", st->spoofout);

  PUT_LIST(fp, "Invitations", st->invitations, 3);
  PUT_LIST(fp, "Assocs", st->assocs, 5);
  PUT_LIST(fp, "Assocs2", st->assocs2, 5);
  for (i = 0; i < MAX_LANGUAGES; i++)
    if (st->speaks[i].basic_known || st->speaks[i].dialects_known)
      fprintf(fp, "Language %d %d %ld\n", i, st->speaks[i].basic_known,
	      st->speaks[i].dialects_known);

  put_num(fp, "Handedness", cs->handedness);
  put_num(fp, "Stance", cs->battle_stance);
  put_num(fp, "PermAlign", cs->perm_align);
  put_num(fp, "Alignment", cs->alignment);
  put_num(fp, "Act", cs->act);
  put_num(fp, "Act2", cs->act2);
  put_num(fp, "AffectedBy", cs->affected_by);
  put_num(fp, "AffectedBy2", cs->affected_by2);
  put_num(fp, "Disabilities", cs->disabilities);
  put_num(fp, "Abilities", cs->abilities);
  PUT_LIST(fp, "SaveMods", cs->apply_saving_throw, NUM_SAVES);

  PUT_LIST(fp, "Stats", ((long long []) { ab->str, ab->str_add, ab->intel,
	    ab->wis, ab->dex, ab->con, ab->cha, ab->will }), 8);
  fprintf(fp, "HitPoints %d %d\n", pt->hit, pt->max_hit);
  fprintf(fp, "Mana %d %d\n", pt->mana, pt->max_mana);
  fprintf(fp, "Moves %d %d\n", pt->move, pt->max_move);
  PUT_LIST(fp, "Armor", pt->armor, ARMOR_LIMIT);
  put_num(fp, "Gold", pt->gold);
  put_num(fp, "Bank", pt->bank_gold);
  PUT_LIST(fp, "Coins", pt->coins, 5);
  put_num(fp, "Exp", pt->exp);
  put_num(fp, "Hitroll", pt->hitroll);
  put_num(fp, "Damroll", pt->damroll);

  put_num(fp, "CurrLang", ps->curr_lang);
  put_num(fp, "Wimpy", ps->wimp_level);
  put_num(fp, "SpellsToLearn", ps->spells_to_learn);
  put_num(fp, "FreezeLevel", ps->freeze_level);
  put_num(fp, "InvisLevel", ps->invis_level);
  put_num(fp, "LoadRoom", ps->load_room);
  put_num(fp, "Pref", ps->pref);
  put_num(fp, "Pref2", ps->pref2);
  put_num(fp, "BadPasswords", ps->bad_pws);
  PUT_LIST(fp, "Conditions", ps->conditions, MAX_COND);
  put_num(fp, "FatePoints", ps->fate_pts);
  put_num(fp, "Wounds", ps->wounds);
  PUT_LIST(fp, "Addictions", ps->addictions, MAX_COND);
  PUT_LIST(fp, "BodyParts", ps->body_parts, 12);
  put_num(fp, "OlcZone", ps->olc_zone);
  for (i = 0; i < MAX_ACTIVE_QUESTS; i++)
    if (ps->quests.active[i].vnum > 0)
      fprintf(fp, "QuestActive %d %d %lld\n", ps->quests.active[i].vnum,
	      ps->quests.active[i].count, (long long) ps->quests.active[i].taken);
  for (i = 0; i < ps->quests.num_done && i < MAX_QUESTS_DONE; i++)
    fprintf(fp, "QuestDone %d %lld\n", ps->quests.done[i].vnum,
	    (long long) ps->quests.done[i].when);
  for (i = 0; i <= MAX_SKILLS; i++)
    if (ps->skills[i])
      fprintf(fp, "Skill %d %d\n", i, ps->skills[i]);

  put_affects(fp, "Affect", st->affected);
  put_affects(fp, "Affect2", st->affected2);
  fputs("End\n", fp);

  return ferror(fp) ? -1 : 0;
}

/* ------------------------------------------------------------------ */
/* reading                                                             */

/* Undo put_str's escapes into dst (size bytes), turning "\n" back into the
   "\r\n" the game uses in memory, and truncating if it doesn't fit. */
static void get_str(char *dst, size_t size, const char *s)
{
  size_t n = 0;

  for (; *s && n + 1 < size; s++) {
    if (*s == '\\' && s[1] == 'n') {
      if (n + 2 >= size)
	break;
      dst[n++] = '\r';
      dst[n++] = '\n';
      s++;
    } else if (*s == '\\' && s[1] == '\\') {
      dst[n++] = '\\';
      s++;
    } else
      dst[n++] = *s;
  }
  dst[n] = '\0';
}

/* Read up to n numbers from s into v (missing ones stay 0); returns the
   count read. */
static int get_nums(const char *s, long long *v, int n)
{
  char *end;
  int i;

  for (i = 0; i < n; i++) {
    v[i] = strtoll(s, &end, 10);
    if (end == s)
      break;
    s = end;
  }
  return i;
}

#define GET_LIST(arr, n, type) do {                     \
    long long v_[64] = { 0 };                           \
    int i_;                                             \
    get_nums(val, v_, (n));                             \
    for (i_ = 0; i_ < (n); i_++)                        \
      (arr)[i_] = (type) v_[i_];                        \
  } while (0)

static void get_affect(const char *val, struct affected_type *af)
{
  long long v[5] = { 0 };
  int i;

  get_nums(val, v, 5);
  for (i = 0; i < MAX_AFFECT; i++)
    if (!af[i].type) {
      af[i].type = (sh_int) v[0];
      af[i].duration = (sh_int) v[1];
      af[i].modifier = (sbyte) v[2];
      af[i].location = (byte) v[3];
      af[i].bitvector = (suprlong) v[4];
      af[i].next = NULL;
      return;
    }
}

int pfile_read_record(FILE *fp, struct char_file_u *st, int *unknown)
{
  struct char_special_data_saved *cs = &st->char_specials_saved;
  struct player_special_data_saved *ps = &st->player_specials_saved;
  struct char_ability_data *ab = &st->abilities;
  struct char_point_data *pt = &st->points;
  static char line[LINE_SIZE];
  char *key, *val, *p;
  long long v[8];
  long long n;

  memset(st, 0, sizeof(*st));
  if (unknown)
    *unknown = 0;

  while (fgets(line, sizeof(line), fp)) {
    if ((p = strchr(line, '\n')))
      *p = '\0';
    key = line;
    if (!*key || *key == '#')
      continue;
    if ((val = strchr(key, ' ')))
      *val++ = '\0';
    else
      val = key + strlen(key);
    n = strtoll(val, NULL, 10);

#define IS(k) (!strcmp(key, (k)))
#define STR(field) get_str((field), sizeof(field), val)
    if (IS("End"))		return 0;
    else if (IS("Name"))	STR(st->name);
    else if (IS("Id"))		cs->idnum = (long) n;
    else if (IS("Password"))	STR(st->pwd);
    else if (IS("Level"))	st->level = (int) n;
    else if (IS("Sex"))		st->sex = (int) n;
    else if (IS("Race"))	st->race = (int) n;
    else if (IS("RaceMod"))	st->race_mod = (int) n;
    else if (IS("Hometown"))	st->hometown = (int) n;
    else if (IS("Height"))	st->height = (int) n;
    else if (IS("Weight"))	st->weight = (int) n;
    else if (IS("Birth"))	st->birth = (time_t) n;
    else if (IS("Played"))	st->played = (long) n;
    else if (IS("LastLogon"))	st->last_logon = (time_t) n;
    else if (IS("Host"))	STR(st->host);
    else if (IS("Username"))	STR(st->username);
    else if (IS("Email"))	STR(st->email);
    else if (IS("Title"))	STR(st->title);
    else if (IS("Description"))	STR(st->description);
    else if (IS("PolyDescr"))	STR(st->poly_descr);
    else if (IS("ExtraDescr"))	STR(st->xtra_descr);
    else if (IS("Keywords"))	STR(st->keywords);
    else if (IS("RoomDescr"))	STR(st->room_descr);
    else if (IS("PoofIn"))	STR(st->spoofin);
    else if (IS("PoofOut"))	STR(st->spoofout);
    else if (IS("Invitations"))	GET_LIST(st->invitations, 3, int);
    else if (IS("Assocs"))	GET_LIST(st->assocs, 5, int);
    else if (IS("Assocs2"))	GET_LIST(st->assocs2, 5, int);
    else if (IS("Language")) {
      get_nums(val, v, 3);
      if (v[0] >= 0 && v[0] < MAX_LANGUAGES) {
	st->speaks[v[0]].basic_known = (sh_int) v[1];
	st->speaks[v[0]].dialects_known = (long) v[2];
      }
    }
    else if (IS("Handedness"))	cs->handedness = (sbyte) n;
    else if (IS("Stance"))	cs->battle_stance = (sbyte) n;
    else if (IS("PermAlign"))	cs->perm_align = (sbyte) n;
    else if (IS("Alignment"))	cs->alignment = (int) n;
    else if (IS("Act"))		cs->act = (suprlong) n;
    else if (IS("Act2"))	cs->act2 = (suprlong) n;
    else if (IS("AffectedBy"))	cs->affected_by = (suprlong) n;
    else if (IS("AffectedBy2"))	cs->affected_by2 = (suprlong) n;
    else if (IS("Disabilities"))	cs->disabilities = (long) n;
    else if (IS("Abilities"))	cs->abilities = (long) n;
    else if (IS("SaveMods"))	GET_LIST(cs->apply_saving_throw, NUM_SAVES, sh_int);
    else if (IS("Stats")) {
      get_nums(val, v, 8);
      ab->str = (sbyte) v[0];  ab->str_add = (sbyte) v[1];
      ab->intel = (sbyte) v[2]; ab->wis = (sbyte) v[3];
      ab->dex = (sbyte) v[4];  ab->con = (sbyte) v[5];
      ab->cha = (sbyte) v[6];  ab->will = (sbyte) v[7];
    }
    else if (IS("HitPoints")) { get_nums(val, v, 2); pt->hit = (int) v[0]; pt->max_hit = (int) v[1]; }
    else if (IS("Mana"))      { get_nums(val, v, 2); pt->mana = (int) v[0]; pt->max_mana = (int) v[1]; }
    else if (IS("Moves"))     { get_nums(val, v, 2); pt->move = (int) v[0]; pt->max_move = (int) v[1]; }
    else if (IS("Armor"))	GET_LIST(pt->armor, ARMOR_LIMIT, sh_int);
    else if (IS("Gold"))	pt->gold = (ln_int) n;
    else if (IS("Bank"))	pt->bank_gold = (ln_int) n;
    else if (IS("Coins"))	GET_LIST(pt->coins, 5, ln_int);
    else if (IS("Exp"))		pt->exp = (ln_int) n;
    else if (IS("Hitroll"))	pt->hitroll = (sbyte) n;
    else if (IS("Damroll"))	pt->damroll = (sbyte) n;
    else if (IS("CurrLang"))	ps->curr_lang = (byte) n;
    else if (IS("Wimpy"))	ps->wimp_level = (int) n;
    else if (IS("SpellsToLearn"))	ps->spells_to_learn = (int) n;
    else if (IS("FreezeLevel"))	ps->freeze_level = (byte) n;
    else if (IS("InvisLevel"))	ps->invis_level = (sh_int) n;
    else if (IS("LoadRoom"))	ps->load_room = (room_num) n;
    else if (IS("Pref"))	ps->pref = (suprlong) n;
    else if (IS("Pref2"))	ps->pref2 = (suprlong) n;
    else if (IS("BadPasswords"))	ps->bad_pws = (ubyte) n;
    else if (IS("Conditions"))	GET_LIST(ps->conditions, MAX_COND, sbyte);
    else if (IS("FatePoints"))	ps->fate_pts = (ubyte) n;
    else if (IS("Wounds"))	ps->wounds = (ubyte) n;
    else if (IS("Addictions"))	GET_LIST(ps->addictions, MAX_COND, ubyte);
    else if (IS("BodyParts"))	GET_LIST(ps->body_parts, 12, long);
    else if (IS("OlcZone"))	ps->olc_zone = (int) n;
    else if (IS("QuestActive")) {
      int q;
      get_nums(val, v, 3);
      for (q = 0; q < MAX_ACTIVE_QUESTS; q++)
	if (!ps->quests.active[q].vnum) {
	  ps->quests.active[q].vnum = (int) v[0];
	  ps->quests.active[q].count = (int) v[1];
	  ps->quests.active[q].taken = (time_t) v[2];
	  break;
	}
    }
    else if (IS("QuestDone")) {
      get_nums(val, v, 2);
      if (ps->quests.num_done < MAX_QUESTS_DONE) {
	ps->quests.done[ps->quests.num_done].vnum = (int) v[0];
	ps->quests.done[ps->quests.num_done].when = (time_t) v[1];
	ps->quests.num_done++;
      }
    }
    else if (IS("Skill")) {
      get_nums(val, v, 2);
      if (v[0] >= 0 && v[0] <= MAX_SKILLS)
	ps->skills[v[0]] = (byte) v[1];
    }
    else if (IS("Affect"))	get_affect(val, st->affected);
    else if (IS("Affect2"))	get_affect(val, st->affected2);
    else if (unknown)
      (*unknown)++;
#undef IS
#undef STR
  }
  return -1;                            /* the file ended inside the record */
}
