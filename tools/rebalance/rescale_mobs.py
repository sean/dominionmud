"""Rescale every mob prototype to its zone's bracket (see brackets.py).

  staff   shopkeepers, trainers, receptionists, postmasters, guards with a
          guard spec, quest givers, NPCs with daily routines, and in towns
          any non-aggressive mob of old level 25+: kept well above the zone
          (old level 43+ -> 105, beyond mortals; else hi + (100-hi)*old/42)
  boss    the zone's group boss: hi + 2, built for party(lo, hi) players
  normal  everything else: old levels ranked within the zone and spread
          over lo..hi
Usage: rescale_mobs.py [--write]"""
import sys, re, os, glob, collections
import wio
from paths import SRC, ROLES, level_dump
from brackets import B, TOWNS, party
from formulas import stats, max_dex

STAFF_SPECS = {'trainer', 'cityguard', 'receptionist', 'postmaster', 'temple_guard', 'smithy',
               'guild', 'cryogenicist', 'janitor'}
# bosses that the name search can't find in the vnum's own zone, or that I chose
BOSS_VNUM = {326: 3101, 190: 5680, 81: 19018, 56: 5608, 42: 4212, 350: None}
# zones whose own mobs are left alone (no bracket)
SKIP = {0, 26}

def spec_assignments():
    out = {}
    src = SRC
    for f in sorted(glob.glob(os.path.join(src, '*.c'))):
        for m in re.finditer(r'ASSIGNMOB\((\d+),\s*([a-z_0-9]+)\)', open(f, encoding='latin-1').read()):
            out[int(m.group(1))] = m.group(2)
    return out

def quest_givers():
    g = set()
    for l in open(wio.ROOT + '/data/quests', encoding='latin-1'):
        m = re.match(r'^giver\s+(\d+)', l)
        if m: g.add(int(m.group(1)))
        m = re.match(r'^(?:deliver|escort)\s+(\d+)', l)
        if m: g.add(int(m.group(1)))      # recipients and escortees: keep them alive
    return g

def routine_mobs():
    r = set()
    for l in open(wio.ROOT + '/data/routines', encoding='latin-1'):
        m = re.match(r'^#(\d+)', l)
        if m: r.add(int(m.group(1)))
    return r

def shopkeepers():
    k = set()
    for l in open(level_dump('levels_shops')):
        if l.startswith('shop'):
            k.add(int(l.split()[3]))
    return k

def plan():
    mobs, files = wio.read_mobs()
    zones = wio.read_zones()
    loads = collections.defaultdict(collections.Counter)
    for z in zones:
        for c, a in z['cmds']:
            if c == 'M' and len(a) >= 2:
                loads[a[1]][z['num']] += 1
    specs = spec_assignments()
    givers, routines, keepers = quest_givers(), routine_mobs(), shopkeepers()

    def home(v):
        own = wio.zone_of(v, zones)
        if loads[v] and own not in loads[v]:
            return loads[v].most_common(1)[0][0]
        return own

    byzone = collections.defaultdict(list)
    for v, m in mobs.items():
        m.zone = home(v)
        byzone[m.zone].append(m)

    out = {}
    for zn, ms in byzone.items():
        if zn in SKIP or zn not in B:
            continue
        lo, hi, pat = B[zn]
        p = party(lo, hi)
        # boss
        boss = BOSS_VNUM.get(zn, 'search')
        if boss == 'search':
            boss = None
            if pat:
                cand = [m for m in ms if re.search(pat, m.short.lower()) or re.search(pat, m.names.lower())]
                cand.sort(key=lambda m: (-sum(loads[m.vnum].values()) > 0, -m.level))
                boss = cand[0].vnum if cand else None
        bosses = {boss} if boss else set()
        if zn == 430:   # the three Fates fight together: each a third of the boss
            bosses = {m.vnum for m in ms if re.search(pat, m.short.lower())}
        staff, normal = [], []
        for m in ms:
            if m.vnum in bosses:
                continue
            sp = specs.get(m.vnum)
            is_staff = (m.vnum in keepers or sp in STAFF_SPECS or m.vnum in givers or
                        m.vnum in routines or
                        (zn in TOWNS and not m.aggressive and m.level >= 25))
            (staff if is_staff else normal).append(m)
        # normal: rank old levels and spread them over lo..hi
        lv = sorted({m.level for m in normal})
        for m in normal:
            r = lv.index(m.level) / max(1, len(lv) - 1) if len(lv) > 1 else 0.5
            out[m.vnum] = ('normal', round(lo + (hi - lo) * r), 1)
        for m in staff:
            nl = 105 if m.level >= 43 else min(100, round(hi + (100 - hi) * max(m.level, 1) / 42))
            out[m.vnum] = ('staff', max(nl, hi + 5), 1)
        for v in bosses:
            out[v] = ('boss', min(107, hi + 2), p)
    return mobs, files, zones, out

def apply(mobs, files, out):
    changed = collections.Counter()
    for v, (role, L, p) in out.items():
        m = mobs[v]
        lines = files[m.file]
        if m.type == 'A':
            # 'fuzzy' mobs roll random stats at boot: make them enhanced (E)
            # mobs with the formula's stats, keeping their race
            race = int(lines[m.stat_line].split()[1])
            aff = wio.flagbits(m.aff)
            s = stats(L, 'boss' if role == 'boss' else 'normal', p,
                      sanct=bool(aff & (1 << 7)), haste=bool(aff & (1 << 17)))
            hp, dm = s['hp'], s['dam']
            fl = lines[m.flags_line].split()
            fl[3] = 'E'
            lines[m.flags_line] = ' '.join(fl)
            lines[m.stat_line] = (f"{L} {s['hitroll']} {s['ac']} {hp[0]}d{hp[1]}+{hp[2]} "
                                  f"{dm[0]}d{dm[1]}+{dm[2]}")
            lines[m.stat_line + 1] = f"{s['gold']} {s['exp']}"
            # the A record's third line is "pos defpos sex"; E adds specs then E
            lines[m.stat_line + 2] = lines[m.stat_line + 2] + f"\nRace: {race}\nE"
            changed[m.file] += 1
            continue
        strength = m.espec.get('Str', (None, 11))[1]
        dex = m.espec.get('Dex', (None, 11))[1]
        if 'Dex' in m.espec and dex > max_dex(L):
            k = m.espec['Dex'][0]
            lines[k] = re.sub(r'-?\d+', str(max_dex(L)), lines[k], count=1)
            dex = max_dex(L)
        aff = wio.flagbits(m.aff)
        s = stats(L, 'boss' if role == 'boss' else 'normal', p, strength, dex,
                  sanct=bool(aff & (1 << 7)), haste=bool(aff & (1 << 17)))
        if 'Str' in m.espec and s['str'] != strength:
            k = m.espec['Str'][0]
            lines[k] = re.sub(r'-?\d+', str(s['str']), lines[k], count=1)
        hp, dm = s['hp'], s['dam']
        lines[m.stat_line] = (f"{L} {s['hitroll']} {s['ac']} {hp[0]}d{hp[1]}+{hp[2]} "
                              f"{dm[0]}d{dm[1]}+{dm[2]}")
        g = lines[m.gold_line].split()
        g[0], g[1] = str(s['gold']), str(s['exp'])
        lines[m.gold_line] = ' '.join(g)
        changed[m.file] += 1
    for f in changed:
        wio.write_lines('mob', f, files[f])
    return changed

if __name__ == '__main__':
    mobs, files, zones, out = plan()
    roles = collections.Counter(r for r, _, _ in out.values())
    print('planned', len(out), 'of', len(mobs), dict(roles))
    bz = collections.defaultdict(list)
    for v, (r, L, p) in out.items():
        bz[mobs[v].zone].append((r, L, v, mobs[v].level, mobs[v].short[:26], p))
    for zn in sorted(bz, key=lambda z: B[z][:2]):
        rows = bz[zn]
        b = [x for x in rows if x[0] == 'boss']
        n = [x[1] for x in rows if x[0] == 'normal']
        s = [x for x in rows if x[0] == 'staff']
        print(f"{zn:4d} {B[zn][0]:3d}-{B[zn][1]:<3d} normal {len(n):3d} [{min(n) if n else '-'}-{max(n) if n else '-'}] "
              f"staff {len(s):2d} boss {[(x[4], x[3], '->', x[1], 'p', x[5]) for x in b]}")
    if '--write' in sys.argv:
        import json
        json.dump({v: [r, L, p, mobs[v].zone] for v, (r, L, p) in out.items()},
                  open(ROLES, 'w'))
        print(apply(mobs, files, out))
