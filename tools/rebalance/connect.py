"""Connect the zones nobody could walk to (Nov 2026 rebalance), and take
out exits from them into Immortal Paradise (rooms 0 and 8)."""
import re, glob

import os
from paths import WORLD
W = os.path.join(WORLD, 'wld')
DIRS = ['north', 'east', 'south', 'west', 'up', 'down']

# (from room, dir, to room, what you see looking that way)
LINKS = [
    (6603, 2, 6602, 'The ancient path winds away south, back toward the foothills.'),
    (7543, 1, 7544, 'The trees thin to the east, where the forest edge weeps sap.'),
    (10077, 1, 28000, 'A gap in the boarded gate opens east on a trail into the hills.'),
    (28000, 3, 10077, 'The trail runs west to the boarded southern gate of Charing Cross.'),
    (10401, 2, 6121, 'A dead-end path south leads into the dark forest of Haon-Dor.'),
    (36000, 0, 36001, 'North, an onyx pathway glimmers through the smoke.'),
    (36001, 2, 36000, 'South, the onyx pathway ends in a dark, smoky chamber.'),
    (28013, 1, 28049, 'East, the hills open out onto the wide grass of the plains.'),
    (28049, 3, 28013, 'West, a thin trail climbs into the hills.'),
    (5614, 2, 1815, 'The forest runs on south toward the hollows of Darkweir.'),
    (1815, 0, 5614, 'North, the trees grow thick and a reddish smoke hangs among them.'),
    (10701, 1, 11700, 'A road of fitted stones leads east from the peak toward a distant tower.'),
    (11700, 3, 10701, 'The road climbs west to the peak of Rhaylinne.'),
    (32838, 4, 2100, 'The mountain trail climbs on, up toward the cold northern hills.'),
    (2100, 5, 32838, 'A trail drops away down the mountainside to the south.'),
    (8100, 2, 4700, 'South of the wastes the ground is churned to mud by marching feet.'),
    (4700, 0, 8100, 'North, the battlefield gives way to the grey wastelands.'),
    (2101, 0, 36000, 'A cleft in the hillside leads north into darkness and smoke.'),
    (36000, 2, 2101, 'South, a cleft in the rock opens on the hills of Nord.'),
    (5464, 3, 4500, 'West of the dunes a pass climbs into steep and silent hills.'),
    (4500, 1, 5464, 'The pass drops east into the Desert of Demise.'),
    (2719, 5, 5229, 'The tunnel floor gives way to a passage descending into the deep dark.'),
    (5229, 4, 2719, 'The passage climbs toward the kobolds\' winding tunnels.'),
    (3883, 5, 33000, 'Beneath the light, a stair spirals down into the walls of a maze.'),
    (33000, 4, 3883, 'A spiral stair climbs toward a brilliant light.'),
]
VOID = {0, 8}
UNVOID_ZONES = (52, 280, 330, 360)

def files_by_room():
    out = {}
    for f in glob.glob(W + '/*.wld'):
        for m in re.finditer(r'^#(\d+)$', open(f, encoding='latin-1').read(), re.M):
            out[int(m.group(1))] = f
    return out

def room_span(lines, vnum):
    s = lines.index(f'#{vnum}')
    e = s + 1
    while not re.match(r'^#\d+$', lines[e]) and not lines[e].startswith('$'):
        e += 1
    return s, e

def main():
    where = files_by_room()
    cache = {}
    def lines_of(f):
        if f not in cache:
            cache[f] = open(f, encoding='latin-1').read().split('\n')
        return cache[f]
    for frm, d, to, desc in LINKS:
        lines = lines_of(where[frm])
        s, e = room_span(lines, frm)
        if any(lines[i] == f'D{d}' for i in range(s, e)):
            raise SystemExit(f'room {frm} already has an exit {DIRS[d]}')
        last_s = max(i for i in range(s, e) if lines[i].strip() == 'S')
        lines[last_s:last_s] = [f'D{d}', desc, '~', '~', f'0 -1 {to}']
    # exits into the void, in the zones that are now reachable
    for f in glob.glob(W + '/*.wld'):
        zn = int(re.search(r'/(\d+)\.wld$', f).group(1)) if re.search(r'/(\d+)\.wld$', f) else None
        if zn not in UNVOID_ZONES:
            continue
        lines = lines_of(f)
        i = 0
        while i < len(lines):
            if re.match(r'^D[0-5]$', lines[i]):
                # D, desc~ (maybe several lines), keywords~, numbers
                j = i + 1
                while not lines[j].endswith('~'): j += 1
                j += 1
                while not lines[j].endswith('~'): j += 1
                j += 1
                nums = lines[j].split()
                if len(nums) == 3 and int(nums[2]) in VOID:
                    del lines[i:j + 1]
                    continue
            i += 1
    for f, lines in cache.items():
        open(f, 'w', encoding='latin-1').write('\n'.join(lines))
    print(len(LINKS), 'exits added')

if __name__ == '__main__':
    main()
