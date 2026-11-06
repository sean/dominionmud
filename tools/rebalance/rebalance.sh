#!/bin/bash
# Re-run the Nov 2026 rebalance from a pre-rebalance world.
#
#   tools/rebalance/rebalance.sh <old lib/world directory> <old lib/data/quests>
#
# The rescale ranks each zone's mobs by their ORIGINAL levels, so it must
# start from the world as it was before the rebalance (e.g. from git:
#   git worktree add /tmp/old <commit before the rebalance>
#   tools/rebalance/rebalance.sh /tmp/old/lib/world /tmp/old/lib/data/quests).
# It OVERWRITES lib/world and lib/data/quests, then checks the result with
# bin/dominion -c.  Build the server first (it is used to compute item levels).
set -e
here="$(cd "$(dirname "$0")" && pwd)"
repo="$(cd "$here/../.." && pwd)"
[ $# -eq 2 ] && [ -d "$1" ] && [ -f "$2" ] || { sed -n '2,13p' "$0"; exit 1; }
old_world="$(cd "$1" && pwd)"; old_quests="$(cd "$(dirname "$2")" && pwd)/$(basename "$2")"
[ "$old_world" != "$repo/lib/world" ] || { echo "the old world must be a separate copy"; exit 1; }

rm -rf "$repo/lib/world" && cp -R "$old_world" "$repo/lib/world"
cp "$old_quests" "$repo/lib/data/quests"
cd "$here"
python3 connect.py          # links to the zones nobody could reach
python3 zone_levels.py      # brackets into the zone files' levels-recommended lines
python3 rescale_mobs.py --write > "${REBALANCE_WORK:-${TMPDIR:-/tmp}/dominion-rebalance}/rescale.txt"
python3 newmobs.py          # residents and bosses for the empty zones
python3 items.py            # items to their levels
python3 quests.py           # quest levels into the brackets
cd "$repo"
if bin/dominion -c 2>&1 | grep SYSERR; then echo "SYSERRs above"; exit 1; fi
echo "Rebalanced.  Check fights with: python3 tools/rebalance/sim.py [zone ...]"
