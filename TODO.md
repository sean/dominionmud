# TODO

Legend: - Not Done; / Started; * Complete

## Migration to plain-text data

* Move to an ASCII (plain-text) file to manage races.
* Move to an ASCII (plain-text) file to manage religions.
* Move to an ASCII (plain-text) file to manage guilds.
* Move to an ASCII (plain-text) file to manage the experience table.
* Change to account system where the account is used to manage multiple
  players (lib/accounts; up to 5 characters each).
* Move to an ASCII (plain-text) character save file system (one file per account in lib/accounts).
- Move to an ASCII (plain-text) file to manage skills/spells.

## Known issues

These were section 8 of AGENTS.md (verified October-November 2026; good starter tasks).

### Security

/ Simultaneous connections per IP address are capped (`max_conns_per_site` in `config.c`, default 5,
  0 = off). The *rate* of new connections isn't limited, though. A client can connect, try 3 passwords
  (about 0.3 s of game-loop time) and reconnect in a loop. A per-site rate limit or failed-login
  back-off would close that gap, and so would moving password hashing off the game loop.
- Every new connection does a blocking reverse-DNS lookup (`gethostbyaddr`) unless `slowns` is on.
- The server is IPv4-only (`AF_INET` sockets), and so is the per-site limit.
- Some `fscanf("%s")` calls have no field width (e.g. `ban.c`, copyover recovery in `comm.c`, the index
  readers in `db.c`, `race.c`). The input files are local, but widths should be added.

### Crashes and undefined behavior

- Zone `temperature` (`weather.c`) is updated every hour but never read and never clamped. Nothing in
  the game uses it yet.
- Mob hit regeneration (`hit_gain()` via the shared `dummy_mob` `player_specials`) differs between `-O0`
  and `-O2` builds: `stat <mob>` shows `+0` at `-O2`. Something reads uninitialised or shared state.
- `FreeHates`/`FreeFears` (`mobact.c`) are never called, so a mob's hate/fear lists leak when it is extracted.
* Fixed November 2026 (found with an AddressSanitizer build, `make CFLAGS="-g -O1 -fsanitize=address"
  LIBS=-fsanitize=address`, worth repeating after big changes): the clone spell freed the doppleganger
  prototype's strings (now gives the clone its own copies, scaled to the victim's level); `stat <mob>` on a
  mob with no race read `races[-1]` and crashed; `relg_name()` allowed an index one past the end; shops whose
  keeper mob doesn't exist wrote to `mob_index[-1]` at boot (now logged as "Shop #N disabled" and skipped;
  10 such shops, in zones 65, 120 and 150); `get_number()` ("2.guard") and three other places used
  overlapping `strcpy`; so did `half_chop()` (`set`). `stat file` read `world[]` with a load-room vnum.
  Loops over `player_table` stopped one player short (`<` instead of `<=`). `do_flee()` used a fleeing
  mob after the hunter it provoked had killed and freed it (mob-vs-mob fights, at random).

### Housekeeping

- `opinion.c` isn't built. Its functions were partly moved into `mobact.c`, and its copies of
  `FreeHates`/`FreeFears` still have the old free-the-wrong-node bug.
- `src/util/Makefile` (generated from `src/util/Makefile.in`) is a separate legacy Makefile that the normal
  build doesn't use. The utilities are built by `src/Makefile`. It still has hand-listed header
  dependencies and targets for utilities that don't compile.
- Generated `src/Makefile`/`conf.h` are committed and macOS-flavoured. The legacy non-Unix Makefiles and
  `conf.h.*` are unmaintained.
/ About 1,000 rooms are still unnamed placeholders ("Room NNNN", "This lovely room has yet to be written")
  in unfinished or unconnected areas (down from about 1,400; see the finished zones below).
- `lib/data/README` and the 1990s player reports in `lib/misc/{bugs,ideas,typos,todo}` are the historical
  backlog; nobody has gone through them yet.

### Done (for history)

* `parse_fuzzy_mob()` used the prototype index instead of the race, giving fuzzy mobs garbage HP. `-c` mode
  didn't load the race tables and crashed. `FreeHates`/`FreeFears` freed the wrong node.
* All ~520 gcc warnings were cleared. Real bugs fixed along the way: overlapping `sprintf`s that wiped
  `score`/`stat`/`show` output; the `set` field list was never shown; good mobs used the neutral line
  (`IS_NEUTRAL(ch) < 250`); summon success messages used the failure table, which was also missing a comma;
  OLC wrote zone headers with a user-controlled format string and logged the wrong file name on open
  failure; `rent_adjust()` read an uninitialised struct; mob prototypes were `memset` with the pointer size;
  `save.c` stored pointers in `int`; `alias.c` read file-controlled lengths into a 127-byte buffer; boot
  hung forever on an index file with no `$` or a data file with no final newline; copyover recovery could
  spin on a truncated file; a short rent-file read could delete a player's crash file; and
  `file_to_string()`/`get_line()`/the ban list wrote before their buffers on a failed read.
* clang-only warnings, now also 0. **Gameplay change:** `IS_CARRYING_W` lacked parentheses, so carried
  weight (items, plus gold/100 for mortals) was always 0 and carry limits were never enforced. They are
  enforced now, so a mortal holding lots of gold can be over the limit. Winter in `weather.c` used `&&`
  where `||` was meant; this has no visible effect yet because nothing reads temperature.
* `src/Makefile` hand-listed only `conf.h sysdep.h protos.h` per object, so header edits didn't trigger
  rebuilds. It now uses compiler-generated dependencies (`-MMD -MP`).
* Passwords were stored in plaintext: `CRYPT()` was a no-op because `configure` defined `CIRCLE_CRYPT`
  but `utils.h` tested `HAVE_CRYPT`, and the 12-character field couldn't hold a real hash anyway. They
  are now PBKDF2 hashes (see §7), and old plaintext entries are upgraded at the next login. This
  changed the player-file record size. `util/mudpasswd` builds again (missing include path) and writes
  hashes.
* Per-site connection limit added. `new_descriptor()` (`comm.c`) counts live connections from the
  peer's IP (`descriptor_data.ip_addr`, also restored after copyover) before the DNS lookup and refuses
  extras with a message. Refusals are logged at most once a minute, with a count.
* "You last logged on from ..." printed garbage for **every** player, because it read `nanny()`'s local
  `tmp_store` on a later call than the one that loaded it. The previous host and time are now kept on the
  descriptor (`last_host`, `last_logon`). The line is shown only to existing characters, and the time is
  that of the last save, i.e. when the player last left.
* Zone resets: `reset_zone()` skipped a 100%-chance command 1 time in 100 (`>=` instead of `>`), and a
  skipped command left `last_cmd` set, so the E/G/P commands that depend on it acted on the previously
  loaded mob. That caused the random `Char is already equipped` SYSERRs. A lost roll now counts as a
  failed command. `obj/54.obj` repeated 95 records from `53.obj` (vnums 5400-5496) in an old format,
  which also broke the sorted object index; it now holds only its own objects (5570-5582).
* World text (Nov 2026): about 1,060 misspellings and 100 grammar slips fixed in rooms, mobs, objects
  and shops; stray carriage returns removed; ~600 short or empty room descriptions written or expanded
  (`Room NNNN` / "yet to be written" placeholders in unconnected areas were left alone), and the whole of
  Elrhinn (zone 30) rewritten. Data fixes: swapped diagonal exits in rooms 3039 and 2711, a corrupted
  extra description in 32194, `(null)` keywords/names/messages in rooms 5896, 25574, 38350 and shop 11,
  and two vandalised extra descriptions in Elrhinn.
* Unfinished zones finished (Nov 2026): Immortal Paradise (0: illusion mobs 29/30, blank mobs removed),
  The Guilds (26: eleven faction halls made real guilds 5-15 with guardians and entrances), Ganin's
  World (27: the kobold war cavern below the Kobold Catacombs, stair at 20065), Gemini's Zone (420:
  the Weeping Wood, graveyard and catacombs of House Gemmar, east of Blackwood room 7544), Valley of Fate
  (430: through the ancient archway in Pelarinne, 32621), and The Jungle (58: the Academy's Proving Halls,
  and the Isle of Karras, reached by boat east from Sea of Tears room 20029). Leftover placeholder rooms,
  mobs and objects in 58, 420 and 430 were removed (the vnum ranges stay reserved). The Fiend (42001) was
  cut from ~6,500 hp/137 dmg to ~1,200 hp/36 dmg, and the Teeth of Gwahlur (42001, a 10000d50 weapon with
  +100s) became a 4d6 dagger with +2/+2/+20 hp. Guild bugs: `save_guilds()` wrote a count line that
  `load_guilds()` can't read (the first petition, accept or reject stopped the next boot); guilds were
  `malloc`ed, so unused petitioner slots were garbage and `petition` crashed; the Bleak Cabal's guard
  blocked north instead of east (direction 1, now 2).
* Strength table (Nov 2026): `STRENGTH_APPLY_INDEX` maps 18/01-18/100 strength to rows 26-30, but
  `str_app[]` had only 26 rows, so those characters read past the end of the array and got garbage
  to-hit, to-damage, wield and carry limits (a new 18/56 character could carry nothing). `str_app[]` now has
  `NUM_STR_APPLY` (31) rows, and `lib/data/attributes` has five new `Str` rows (26-30) between the
  values for 18 and 19.
  `affect_total()` also rerolled exceptional strength at random (18/01-18/100) on every affect change, so
  those limits jumped around; it now keeps the character's own `real_abils.str_add` (and zeroes it
  when strength isn't exactly 18).
* `unaffect` (Nov 2026) removed nothing unless the victim also had a harmful affect; when it did, it
  called `affect_remove(..., TRUE)` on the helpful list, which returns without removing anything, so the
  loop never ended and the server hung. It also cleared the *immortal's* AFF flags. It now removes both
  lists and leaves other AFF bits alone.
* Castle of Llyndyr (Nov 2026): `castle.c` (CircleMUD's King's Castle procedures) is built again and drives
  zone 383. King Welmar became the Emperor, with a new daily route (see AGENTS.md); the rest of the
  household got new mobs. Fixed on the way: the King's moves used `path[i] - 0` instead of `- '0'` (no
  move ever worked); a missing mob wrote `mob_index[-1]`; the door guards blocked the staff too and
  checked whether the *player* was awake; the butler only ever looked at the first object in a room; the
  cleaners picked up corpses. The zone was rebuilt: the written rooms kept and polished, a bedchamber, inn,
  armory, storeroom, barracks, battlements and training hall added, an overland road from Gnishmire
  (35106 north), the unfinished grids turned into the SaberBeast's warren (38340-38356, reached through
  the flagstone in 38323 or the tunnels), four shops (shp/383.shp), and the treasure and items rebalanced
  (the Shadow Blade was 200d50 with +100 STR/AC/mana; the treasury held 360,000 coins). The King's
  Castle leftovers (`obj/150.obj`, `shp/150.shp`, the zone-150 `ASSIGNMOB`s) were removed.
  Also: `do_flee()` made the hunter chase (and possibly kill) the fleeing mob at once, from inside the
  `damage()` call that was still using it (a use-after-free that crashed an ASan build within minutes of
  boot); the hunter now gives chase on its next turn. `hunt_victim()` called `remember()` on a prey that
  had already been freed. Objects of glass or paper always showed as `[destroyed]`.
* `shoot` and `throw` (Nov 2026) share their damage code now (`ranged_damage()` in `act_off.c`). Both used a
  mob after killing it (to make it hunt the attacker), and neither told the room about a hit (the message
  had no room to go to, and logged a SYSERR). `shoot` emptied the bow after every shot whatever ammunition
  was left. `throw` destroyed the thrown object and made a fresh copy from its prototype (crashing on an
  object with no prototype); it now throws the object itself. Both now respect shopkeeper and immortal
  protection and flag player killers like melee does, and a mob that wants revenge does so last, since
  it may kill the attacker. An unarmed mob that dodges a thrown weapon picks it up and wields it (this
  was meant to happen before, but never worked).
