# Rebalance tools

The scripts that rebalanced The Dominion for levels 1-100 in November 2026, kept so the work
can be repeated, checked, and extended to new zones. They need only Python 3 (standard library)
and a built `bin/dominion`. The model itself is described in AGENTS.md §5 ("Balance") and
mirrored in C by `mob_set_level()` in `src/levels.c`.

## For builders: balancing a new mob or zone

1. Give the zone a bracket: put `<low>-<high>~` on the "levels recommended" line of its zone
   file (the line after the builder's name), and add it to `brackets.py`. The farther from
   Elrhinn, the higher. AREAS shows it to players, and items in the zone get their levels from it.
2. Get each mob's stat lines from `statline.py`:

   ```
   python3 tools/rebalance/statline.py 46             # an ordinary level-46 mob
   python3 tools/rebalance/statline.py 52 boss 3      # the zone's boss, for a party of 3
   python3 tools/rebalance/statline.py 46 str 22 dex 19 sanct
   python3 tools/rebalance/statline.py table          # the reference numbers, levels 1-100
   ```

   It prints the `level hitroll ac hp damage` and `gold exp` lines of an `S` or `E` record. Keep
   ordinary mobs inside the bracket, and give every zone at least one boss at `high + 2`, built for
   a party of 2 (up to level 20), 3 (to 50), 4 (to 80) or 5. A mob's `+Z` damage is its damroll,
   which blows don't add, so damage is all in the dice. If the record sets Str, use the Str printed.
3. Check the zone: `python3 tools/rebalance/sim.py <zone>`. For the zone's strongest ordinary mob
   and its boss, it shows how often a player of the mob's level wins alone and how many rounds it
   takes. For the boss it also shows a solo player at the zone's top level and ten levels above,
   and a party at the top level. Aim for an ordinary mob to take about 6 rounds and 30% of a solo
   player's hit points. A boss should beat a solo player of the zone's top level, and the party
   should win nearly every time.
4. `bin/dominion -c` must load with no SYSERR. Items need nothing: their levels and the wear check
   follow from where they load (`compute_levels()` in `src/levels.c`).

## The scripts

| Script | What it does |
|---|---|
| `paths.py` | Where the repository is (two levels up, or `DOMINION_ROOT`), and the work directory (`REBALANCE_WORK`, default `<tmp>/dominion-rebalance`). |
| `wio.py` | Reads mob, object and zone files, keeping line positions so only stat lines are rewritten. |
| `brackets.py` | The approved zone brackets and boss names, the towns, and the party size for a bracket. |
| `formulas.py` | The balance model: reference player, mob stats by level and role, gold and experience. |
| `curve.py` | The experience curve (`python3 curve.py` prints it). It wrote `lib/data/exp_table`. |
| `links.py` | Room exits and reachability from the mortal start room. |
| `connect.py` | The 25 exits that connected the unreachable zones; it removes exits into Immortal Paradise. |
| `zone_levels.py` | Writes the brackets into the zone files. |
| `rescale_mobs.py` | Classifies every mob (ordinary, town staff, boss) and rescales it. Without `--write` it only reports. |
| `newmobs.py` | The residents and bosses added to empty zones, with their resets. |
| `items.py` | Rescales objects to the item levels the server computes (`DOMINION_LEVEL_DUMP`). |
| `quests.py` | Moves quest levels into their zones' brackets. |
| `sim.py` | Simulated fights against the mobs in the files, by zone. |
| `statline.py` | Stat lines for one mob. |
| `rebalance.sh` | Runs the whole pipeline. |

## Re-running the whole rebalance

`rescale_mobs.py` ranks each zone's mobs by their *original* levels, so the pipeline must start
from the pre-rebalance world, not the current one. Running it on an already rebalanced world
gives different (wrong) staff levels. To repeat it, for example after changing a formula:

```
git worktree add /tmp/old <the commit before the rebalance>
tools/rebalance/rebalance.sh /tmp/old/lib/world /tmp/old/lib/data/quests
```

The script overwrites `lib/world` and `lib/data/quests`; run it on a clean checkout. Anything built
in those files after the rebalance (such as the Arena's booth, ring, fighters and prizes in zone 53)
is lost and has to be merged back. Run on the
original files, it reproduces the delivered world byte for byte. It does not touch the C side,
which has its own copies of the formulas: `src/levels.c` (`mob_set_level`), `src/race.c`
(`mob_exp_for_level`), `src/magic.c` and `src/fight.c` (spell, heal and skill damage). Change both
sides together.
