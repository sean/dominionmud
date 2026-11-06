"""Rescale objects to their item level (computed by the server from where
they load: src/levels.c compute_levels(), dumped by DOMINION_LEVEL_DUMP).

  weapons   damage dice averaging 3 + 0.3L, times the weapon's own
            strength within its zone (0.8-1.25, from its old dice)
  armour    absorbs 0.8 x (3 + 0.4L) percent where it is worn, times its
            own strength within its zone; armour that had 0 keeps 0
  affects   capped by level (hitroll and damroll 1 + L/12, max hp 3 + L/4,
            max mana and moves 5 + L/2, stats 1 + L/35, saves 1 + L/15,
            regeneration 5 + L/5); armour class bonuses flipped to the
            sign this engine uses (+ absorbs more) and capped at 1 + L/20
  magic     potions, scrolls, wands and staves cast at the item's level
  value     weapons 10 + 2L + 0.3L^2 (armour 60%), times its strength;
            treasure between 10 and ten times a level-L mob's gold
Items of the unbracketed zones (Immortal Paradise, the Guilds) that load
nowhere else are left alone."""
import re, statistics, collections
import wio
from paths import level_dump
DUMP = None
from brackets import B
from formulas import mob_gold

SKIP_ZONES = {0, 26}
WEAPON, ARMOR, POTION, SCROLL, WAND, STAFF, TREASURE = 5, 9, 10, 2, 3, 4, 8

def levels():
    lv = {}
    for l in open(DUMP):
        p = l.split()
        if p[0] == 'obj':
            lv[int(p[1])] = int(p[2])
    return lv

def loaded_somewhere():
    out = set()
    for z in wio.read_zones():
        for c, a in z['cmds']:
            if c in 'OGEP' and len(a) >= 2:
                out.add(a[1])
    for l in open(DUMP):
        if l.startswith('shop'):
            out.update(int(x) for x in l.split()[7:])
    return out

def clamp(x, a, b):
    return max(a, min(b, x))

CAPS = {1: lambda L: 1 + L // 35, 2: lambda L: 1 + L // 35, 3: lambda L: 1 + L // 35,
        4: lambda L: 1 + L // 35, 5: lambda L: 1 + L // 35, 6: lambda L: 1 + L // 35,
        7: lambda L: 1 + L // 35, 12: lambda L: 5 + L // 2, 13: lambda L: 3 + L // 4,
        14: lambda L: 5 + L // 2, 18: lambda L: 1 + L // 12, 19: lambda L: 1 + L // 12,
        30: lambda L: 5 + L // 5, 31: lambda L: 5 + L // 5, 32: lambda L: 5 + L // 5}
for s in range(20, 29):
    CAPS[s] = lambda L: 1 + L // 15

def main():
    global DUMP
    DUMP = level_dump('levels_items')   # item levels after the mobs changed
    objs, files = wio.read_objs()
    zones = wio.read_zones()
    lv = levels()
    used = loaded_somewhere()
    zone = {v: wio.zone_of(v, zones) for v in objs}

    def wavg(o): return o.vals[1] * (o.vals[2] + 1) / 2 if o.vals[1] > 0 and o.vals[2] > 0 else 0
    med_w = collections.defaultdict(list); med_a = collections.defaultdict(list)
    for o in objs.values():
        if o.type == WEAPON and wavg(o): med_w[zone[o.vnum]].append(wavg(o))
        if o.type == ARMOR and o.vals[0] > 0: med_a[zone[o.vnum]].append(o.vals[0])
    med_w = {z: statistics.median(v) for z, v in med_w.items()}
    med_a = {z: statistics.median(v) for z, v in med_a.items()}

    changed = collections.Counter()
    for v, o in objs.items():
        if zone[v] in SKIP_ZONES and v not in used:
            continue
        if zone[v] not in B and zone[v] not in SKIP_ZONES and v not in used:
            continue
        L = lv.get(v, 1)
        lines = files[o.file]
        vals = list(o.vals)
        cost = o.cost
        r = 1.0
        if o.type == WEAPON and wavg(o):
            r = clamp((wavg(o) / med_w.get(zone[v], wavg(o))) ** 0.5, 0.8, 1.25)
            target = (3 + 0.3 * L) * r
            n = 1 + L // 15
            size = max(2, int(round(2 * target / n - 1)))
            vals[1], vals[2] = n, size
            cost = int(round((10 + 2 * L + 0.3 * L * L) * r))
        elif o.type == ARMOR:
            if vals[0] > 0:
                r = clamp((vals[0] / med_a.get(zone[v], vals[0])) ** 0.5, 0.75, 1.25)
                vals[0] = int(clamp(round(0.8 * (3 + 0.4 * L) * r), 1, 60))
            cost = int(round((10 + 2 * L + 0.3 * L * L) * 0.6 * r))
        elif o.type in (POTION, SCROLL, WAND, STAFF):
            vals[0] = L
            cost = int(round(20 + 3 * L + 0.1 * L * L))
        elif o.type == TREASURE:
            cost = int(clamp(cost, 10, max(50, 10 * mob_gold(L))))
        if vals != o.vals:
            lines[o.val_line] = ' '.join(str(x) for x in vals)
        if cost != o.cost:
            w = lines[o.wcr_line].split()
            w[1] = str(cost)
            w[2] = str(min(int(w[2]), max(0, cost // 10)))
            lines[o.wcr_line] = ' '.join(w)
        for k, loc, mod in o.affects:
            new = mod
            if loc == 17:                          # armour class: + absorbs more here
                new = clamp(abs(mod), 0, 1 + L // 20) if mod else 0
            elif loc in CAPS:
                cap = CAPS[loc](L)
                if 20 <= loc <= 28:                # saves: negative is good
                    new = clamp(mod, -cap, cap)
                else:
                    new = clamp(mod, -cap, cap)
            if new != mod:
                parts = lines[k].split()
                parts[1] = str(new)
                lines[k] = ' '.join(parts)
        changed[o.file] += 1
    for f in changed:
        wio.write_lines('obj', f, files[f])
    print('objects touched', sum(changed.values()))

if __name__ == '__main__':
    main()
