"""New mobs for zones that had none worth fighting, and the bosses the table
calls 'new'.  Each is written with its final stats (formulas.stats) and
reset in its zone.  Run after rescale_mobs.py --write."""
import random, re, collections
import wio
from paths import ROLES
from formulas import stats
from brackets import B, party

from links import load as _load, bfs as _bfs
ROOMS = REACH = RZ = None
def rooms_of(zn, pat='.'):
    """Rooms of zone zn that players can reach, whose names match pat."""
    global ROOMS, REACH, RZ
    if ROOMS is None:
        ROOMS = _load()
        REACH = set(_bfs(ROOMS))
        zones = wio.read_zones()
        RZ = {r: wio.zone_of(r, zones) for r in ROOMS}
    return [r for r in sorted(ROOMS) if RZ[r] == zn and r in REACH and re.search(pat, ROOMS[r][0].lower())]

# (zone, key, keywords, short, long, description, level, role, flags, count, room pattern | [rooms], sex)
# level: absolute; role 'boss' uses party(lo, hi).  flags: f aggressive, b sentinel (g stay-zone always)
N = []
def add(zone, keywords, short, long, desc, level, role='normal', agg=False, sentinel=False,
        count=1, rooms='.', sex=0):
    N.append(dict(zone=zone, keywords=keywords, short=short, long=long, desc=desc, level=level,
                  role=role, agg=agg, sentinel=sentinel, count=count, rooms=rooms, sex=sex))

# --- Thallin Plains (1-6)
add(352, 'coyote plains', 'a plains coyote', 'A scrawny plains coyote slinks through the grass.',
    'A rangy, yellow-grey coyote with a torn ear, all ribs and teeth.  It watches you\nsideways and keeps its distance - for now.', 3, count=4, rooms='thallin plains', sex=1)
add(352, 'bear great plains grizzly', 'a great plains bear',
    'A great humped plains bear rears up out of the grass, huffing!',
    'A grizzled old bear the size of a hay wagon, with claws like sickles and a\nhide scarred by every hunter who ever tried for it.  Hunters from the\nmonastery go after it in twos and threes, and not all of them come back.',
    8, 'boss', rooms='thallin plains', sex=1)
# --- Elrhinn (2-8): the Atharian smugglers under the Grog and Tankard
add(30, 'smuggler atharian', 'an Atharian smuggler', 'An Atharian smuggler counts crates by lamplight.',
    'A wiry man in a dark coat, with a cudgel at his belt and ink-stained fingers.\nThe Atharians move whatever pays, and they do not like visitors.',
    5, agg=True, count=2, rooms=[3079, 3080], sex=1)
add(30, 'dace black ringleader atharian', 'Black Dace, the Atharian ringleader',
    'Black Dace, the Atharian ringleader, sits on a crate with a long knife across his knees.',
    'A lean, pockmarked man with a black beard and a gold tooth, ringleader of the\nAtharian smugglers.  Half the contraband in Elrhinn passes under his hands,\nand the town watch pretends not to know where he keeps it.',
    10, 'boss', sentinel=True, rooms=[3080], sex=1)
# --- Elrhinn Hills (6-14)
add(55, 'wolf hill grey', 'a hill wolf', 'A grey hill wolf pads through the heather.',
    'A lean grey wolf with yellow eyes, one of a pack that follows the flocks.', 8, count=4,
    rooms='hills|valley', sex=1)
add(55, 'brigand bandit', 'a hill brigand', 'A hill brigand lurks beside the trail, hand on his sword.',
    'A weathered man in mismatched leathers and a stolen cloak, one of Red\nGarreth\'s band who rob the drovers on the hill roads.', 10, agg=True, count=3,
    rooms='trail|path|bridge', sex=1)
add(55, 'garreth red bandit captain', 'Red Garreth, the bandit captain',
    'Red Garreth, the bandit captain, leans on his sword by the rope bridge, grinning.',
    'A big red-bearded man with a broken nose and a sword as long as a man is\ntall.  He has held the rope bridge for three summers, and he takes a toll\nin gold or in blood.',
    16, 'boss', sentinel=True, rooms='before the rope bridge', sex=1)
# --- Gnishmire Town (22-30)
add(350, 'golem clockwork runaway', 'a runaway clockwork golem',
    'A runaway clockwork golem clanks along the beach, gears shrieking!',
    'A hulking gnomish machine of brass plates and pistons, twice the height of a\nman, built to haul cargo and long since gone wrong.  Steam hisses from its\njoints and it smashes anything that moves.  The gnomes would very much like\nsomebody to switch it off.',
    32, 'boss', agg=True, rooms='beach of gnishmire')
# --- Gnishmire Forests (24-32)
add(353, 'boar spiddel wild', 'a Spiddel boar', 'A bristling Spiddel boar roots among the leaves.',
    'A black, bristling boar with tusks as long as daggers.', 26, count=3, rooms='forest|game path', sex=1)
add(353, 'spider forest giant', 'a giant forest spider', 'A giant forest spider hangs in its web between the trees.',
    'A spider the size of a pony, glossy and black, with a red hourglass on its\nback.', 28, agg=True, count=3, rooms='deep in spiddel')
add(353, 'lurker creek', 'a creek lurker', 'Something long and scaly lies half-hidden in the creek.',
    'A long, flat, scaly thing like a crocodile crossed with a newt, with too many\nteeth.', 30, agg=True, count=2, rooms='creekbed')
add(353, 'tusker old boar monstrous', 'the Old Tusker',
    'The Old Tusker, a monstrous boar, glares at you with tiny red eyes.',
    'A boar the size of a bull, grey with age, its hide bristling with the broken\nshafts of a dozen hunters\' arrows.  The gnomish foresters say it has killed\nmore of them than any wolf.',
    34, 'boss', rooms='deep in spiddel', sex=1)
# --- Mytherandil (35-45)
add(11, 'cutpurse thief', 'a Mytherandil cutpurse', 'A cutpurse loiters in a doorway, eyeing your belt.',
    'A narrow-faced youth in grey with quick hands and a quicker knife.', 38, count=3,
    rooms='darkmin|alley', sex=1)
add(11, 'vesk knife master thieves', 'Vesk the Knife',
    'Vesk the Knife, master of the Mytherandil thieves, lounges here, juggling daggers.',
    'A slender, smiling man in black silk, master of the Mytherandil thieves.  He\nnever stops juggling three daggers, and he never drops one.',
    47, 'boss', sentinel=True, rooms='empty house', sex=1)
# --- Hills (42-50)
add(40, 'bat cave giant', 'a giant cave bat', 'A giant cave bat hangs from the roof, wings folded.',
    'A bat with a wingspan wider than a man is tall, and a face like a skinned dog.',
    43, agg=True, count=3, rooms='tunnel|hole')
add(40, 'giant hill', 'a hill giant', 'A hill giant squats here, gnawing a sheep.',
    'A shaggy, stooped giant twice the height of a man, wrapped in hides.',
    46, count=3, rooms='foothills|the hills', sex=1)
add(40, 'troll cave', 'a cave troll', 'A cave troll lurches out of the dark.',
    'A grey-green troll with a hide like wet stone and arms down to its knees.', 48, agg=True,
    count=3, rooms='cave|maze', sex=1)
add(40, 'grukk chief hill giant', 'Grukk, chief of the hill giants',
    'Grukk, chief of the hill giants, sits on a heap of bones in the cave.',
    'The biggest hill giant you have ever seen, with a tree trunk for a club and a\nnecklace of dwarven helmets.', 52, 'boss', sentinel=True, rooms='the large cave', sex=1)
# --- Dark Tower (50-60): the Bloodlord's order
add(25, 'knight bloodlord', 'a knight of the Bloodlord', 'A knight of the Bloodlord drills with a black sword.',
    'A knight in blood-red plate, his face hidden behind a skull-faced helm.', 55, agg=True, count=3,
    rooms='knights|hallway', sex=1)
add(25, 'wizard bloodlord', 'a wizard of the Bloodlord', 'A wizard of the Bloodlord mutters over a bowl of blood.',
    'A gaunt wizard in red robes, his fingers stained to the wrist.', 56, count=2,
    rooms='wizards|alcove', sex=1)
add(25, 'rogue bloodlord', 'a rogue of the Bloodlord', 'A rogue of the Bloodlord watches you from the shadows.',
    'A silent figure in dark leathers with a curved knife.', 54, agg=True, count=2, rooms='rogue|alcove', sex=1)
add(25, 'priest bloodlord', 'a priest of the Bloodlord', 'A priest of the Bloodlord reads from a book bound in skin.',
    'An old priest with a shaven, tattooed head and eyes like a dead fish.', 57, count=1,
    rooms='library', sex=1)
add(25, 'bloodlord lord blood', 'the Bloodlord',
    'The Bloodlord stands before his altar, and the air tastes of iron.',
    'A towering figure in armour the colour of old blood, with a crown of iron\nthorns.  His order worships him as a god, and he does nothing to dissuade\nthem.', 62, 'boss', sentinel=True, rooms='temple', sex=1)
# --- Rhaylinne Tower (65-75)
add(117, 'guardian stone', 'a stone guardian', 'A stone guardian stands motionless by the wall.',
    'A statue of a robed warrior, carved from grey granite, that turns its head\nto follow you.', 68, count=3, rooms='hallway|entrance|corner')
add(117, 'armour suit animated', 'an animated suit of armour', 'An empty suit of armour clanks along the corridor.',
    'A suit of old plate armour, quite empty, that walks and fights by itself.', 70, agg=True,
    count=3, rooms='room #|hallway')
add(117, 'apprentice tower', 'a tower apprentice', 'A tower apprentice hurries past with an armful of scrolls.',
    'A pale young scholar in the grey robes of Rhaylinne.', 66, count=2, rooms='room #', sex=2)
add(117, 'elemental air bound', 'a bound air elemental', 'A bound air elemental howls in a circle of runes.',
    'A whirling column of wind and dust with two burning eyes, straining against\nits bindings.', 72, agg=True, count=2, rooms='room #')
add(117, 'ysolde archmage', 'Archmage Ysolde of Rhaylinne',
    'Archmage Ysolde of Rhaylinne studies a floating sphere of light.',
    'A tall, silver-haired woman in midnight-blue robes, mistress of the tower.\nPower crackles at her fingertips.', 77, 'boss', sentinel=True, rooms='room #117[6-8]', sex=2)
# --- Hills of Nord (66-74)
add(21, 'wolf frost', 'a frost wolf', 'A frost wolf, white as snow, watches you from the grass.',
    'A huge white wolf with ice in its fur and breath that steams.', 67, agg=True, count=4,
    rooms='field|grass|bushes|room #', sex=1)
add(21, 'raider nordman', 'a Nordman raider', 'A Nordman raider sharpens his axe.',
    'A tall, fair-bearded warrior in mail and furs, with a round shield.', 68, count=4,
    rooms='road|hut|mound|path', sex=1)
add(21, 'berserker nordman', 'a Nordman berserker', 'A Nordman berserker howls and bites his shield!',
    'A huge man in a bearskin, eyes rolling, foam on his lips.', 72, agg=True, count=2,
    rooms='hut|center', sex=1)
add(21, 'hrothgar jarl frostborn', 'Jarl Hrothgar the Frostborn',
    'Jarl Hrothgar the Frostborn sits on a carved chair, his great axe across his knees.',
    'The jarl of the Nordmen, grey-bearded and broad as a door, his mail rimed with\nfrost.  They say he was born in a snowdrift and has never been warm since.',
    76, 'boss', sentinel=True, rooms='center|hut', sex=1)
# --- Tomb of Haqim-Taar (78-88): fills the 'unfinished mob' slots 4501-
add(45, 'wraith sand', 'a sand wraith', 'A sand wraith whirls up out of the dust of the pass.',
    'A shape of whirling sand and old bones, with two pits of darkness for eyes.',
    80, agg=True, count=3, rooms='pass|trail|hills')
add(45, 'legionnaire skeletal', 'a skeletal legionnaire', 'A skeletal legionnaire stands guard, spear grounded.',
    'The bones of a soldier of an empire long dead, still in its rusted harness.',
    84, agg=True, count=4, rooms='tunnel')
add(45, 'mummy guardian tomb', 'a tomb guardian', 'A tomb guardian shambles out of a niche, trailing wrappings.',
    'A mummified warrior wrapped in brown linen, its eyes two gleaming gems.',
    82, agg=True, count=4, rooms='room #|chamber')
add(45, 'priest haqim', 'a priest of Haqim-Taar', 'A priest of Haqim-Taar chants over a bowl of smoking incense.',
    'A withered priest, more than half a corpse, who has served his king for a\nthousand years.', 86, count=2, rooms='room #|monument')
add(45, 'haqim taar undying king', 'Haqim-Taar, the Undying King',
    'Haqim-Taar, the Undying King, sits on a throne of bones, and his eyes burn.',
    'A towering mummy in a golden mask and a mantle of ancient purple, who ruled\nthese hills before Elrhinn was a village and means to rule them again.',
    90, 'boss', sentinel=True, rooms='chamber beneath', sex=1)
# --- The Labyrinth (85-95)
add(330, 'minotaur', 'a minotaur', 'A minotaur paws the floor and lowers its horns.',
    'A bull-headed giant with a double axe and a ring through its nose.', 88, agg=True, count=5,
    rooms='red', sex=1)
add(330, 'stalker labyrinth', 'a labyrinth stalker', 'Something invisible breathes close by.',
    'A thing of shifting colours, nearly invisible against the labyrinth walls.', 90, agg=True,
    count=4, rooms='green')
add(330, 'horror chromatic', 'a chromatic horror', 'A chromatic horror pulses in every colour at once.',
    'A mass of shifting colour that hurts to look at, with too many mouths.', 92, agg=True, count=2,
    rooms='color|black')
add(330, 'asterion lord labyrinth', 'Asterion, Lord of the Labyrinth',
    'Asterion, Lord of the Labyrinth, waits in the centre of the carousel.',
    'The father of all minotaurs, black as night, with gold-capped horns and an axe\nno man could lift.  The Labyrinth was built to hold him, and it has.',
    97, 'boss', sentinel=True, rooms='carousel', sex=1)


def free_vnums(zn, mobs, want, prefer_unfinished=False):
    lo = zn * 100
    top = [z for z in wio.read_zones() if z['num'] == zn][0]['top']
    out = []
    if prefer_unfinished:
        out = [v for v in sorted(mobs) if lo <= v <= top and mobs[v].short == 'an unfinished mob']
    v = lo
    while len(out) < want and v <= top:
        if v not in mobs and v not in out:
            out.append(v)
        v += 1
    return out[:want]


def mob_text(v, d):
    lo, hi = B[d['zone']][:2]
    s = stats(d['level'], d['role'], party(lo, hi) if d['role'] == 'boss' else 1)
    flags = 'd' + ('b' if d['sentinel'] else '') + ('f' if d['agg'] else '') + 'g'
    flags = ''.join(sorted(flags))
    hp, dm = s['hp'], s['dam']
    return (f"#{v}\n{d['keywords']}~\n{d['short']}~\n{d['long']}\n~\n{d['desc']}\n~\n"
            f"{flags} 0 {-350 if d['agg'] else 0} E\n"
            f"{d['level']} {s['hitroll']} {s['ac']} {hp[0]}d{hp[1]}+{hp[2]} {dm[0]}d{dm[1]}+{dm[2]}\n"
            f"{s['gold']} {s['exp']}\n8 8 {d['sex']}\nE")


def run():
    mobs, files = wio.read_mobs()
    rng = random.Random(1106)
    byzone = collections.defaultdict(list)
    for d in N:
        byzone[d['zone']].append(d)
    added = {}
    for zn, ds in byzone.items():
        vn = free_vnums(zn, mobs, len(ds), prefer_unfinished=(zn == 45))
        mobfile = f'{zn}.mob'
        lines = files[mobfile]
        resets = []
        for d, v in zip(ds, vn):
            text = mob_text(v, d)
            if v in mobs:       # an 'unfinished mob' slot: replace the block
                m = mobs[v]
                lines[m.start:m.end] = text.split('\n')
                mobs, files = None, None
                wio.write_lines('mob', mobfile, lines)
                mobs, files = wio.read_mobs()
                lines = files[mobfile]
            else:
                # keep vnums in ascending order: before the first block above v
                after = [mobs[x].start for x in mobs if x > v and mobs[x].file == mobfile]
                at = min(after) if after else max(i for i, l in enumerate(lines) if l.startswith('$'))
                lines[at:at] = text.split('\n')
                wio.write_lines('mob', mobfile, lines)
                mobs, files = wio.read_mobs()
                lines = files[mobfile]
            rs = d['rooms'] if isinstance(d['rooms'], list) else rooms_of(zn, d['rooms'])
            if not rs:
                rs = rooms_of(zn)          # pattern matched nowhere reachable: anywhere reachable
            if not rs:
                raise SystemExit(f"no rooms for {d['short']} in zone {zn} ({d['rooms']})")
            picks = [rs[0]] if d['sentinel'] and len(rs) == 1 else rng.sample(rs, min(d['count'], len(rs)))
            while len(picks) < d['count']:
                picks.append(rng.choice(rs))
            for r in picks:
                resets.append(f"M 0 {v} {d['count']} {r} 100 \t* {d['short']} ({'group boss' if d['role']=='boss' else 'level %d' % d['level']})")
            added[v] = d
        # zone resets: before the final S
        zf = f'{wio.W}/zon/{zn}.zon'
        zl = open(zf, encoding='latin-1').read().split('\n')
        si = max(i for i, l in enumerate(zl) if l.strip() == 'S')
        zl[si:si] = ['* Nov 2026 rebalance: residents'] + resets
        open(zf, 'w', encoding='latin-1').write('\n'.join(zl))
    return added

if __name__ == '__main__':
    a = run()
    import json
    roles = json.load(open(ROLES))
    for v, d in a.items():
        roles[str(v)] = [d['role'], d['level'], party(*B[d['zone']][:2]) if d['role'] == 'boss' else 1, d['zone']]
    json.dump(roles, open(ROLES, 'w'))
    print(len(a), 'new mobs')
    for v, d in sorted(a.items()):
        print(v, d['short'], d['level'], d['role'])
