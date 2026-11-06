"""sim.py [zone ...]: how fights go in each zone (all zones by default).

Monte Carlo fights using fight.c's rules (hit(): d100+hitroll vs defence,
damage x (1 - armour%)) between reference players (formulas.py) and the
actual mob prototypes in the files."""
import random, statistics, re, sys
import wio
from paths import ROLES
from formulas import (p_hp, p_hitroll, p_weapon, p_attacks, p_armour, DEX_DEF, STR_TODAM)

R = random.Random(7)

def dice(n, s):
    return sum(R.randint(1, s) for _ in range(n)) if n > 0 and s > 0 else 0

class P:
    def __init__(s, L):
        s.L = L; s.hp = s.max = int(p_hp(L)); s.hr = p_hitroll(L); s.ac = p_armour(L)
        s.att = p_attacks(L); s.defn = 60
    def swing(s, mob):
        n = int(s.att) + (1 if R.random() < s.att - int(s.att) else 0)
        dmg = 0
        for _ in range(n):
            if R.randint(1, 100) + s.hr < mob.defn:
                continue
            avg = p_weapon(s.L)
            d = R.randint(1, max(1, int(avg * 2 - 1))) + 2 + s.L // 16
            d = d // 2 if mob.sanct else d
            dmg += max(1, int(d - d * mob.ac / 100))
        return dmg

class M:
    def __init__(s, m):
        s.L = m.level
        hx, hy, hz = m.hpd
        s.hp = dice(hx, hy) + hz
        s.hr = min(100, 2 * m.level + 3) if m.hitroll >= 2 * m.level else m.hitroll
        s.ac = m.ac if 0 <= m.ac < 100 else 50
        s.dd = m.damd
        s.str = m.espec.get('Str', (0, 11))[1]
        s.defn = DEX_DEF.get(m.espec.get('Dex', (0, 11))[1], 40)
        aff = wio.flagbits(m.aff)
        s.sanct = bool(aff & (1 << 7))
        s.att = 2.2 if aff & (1 << 17) else 1.2
    def swing(s, p):
        n = int(s.att) + (1 if R.random() < s.att - int(s.att) else 0)
        tot = 0
        for _ in range(n):
            if R.randint(1, 100) + s.hr < p.defn:
                continue
            d = STR_TODAM.get(s.str, 0) + dice(s.dd[0], s.dd[1]) + s.dd[2]
            tot += max(1, int(d - d * p.ac / 100))
        return tot

def fight(m, levels, rounds=400):
    """players of the given levels against one mob; the mob hits the first."""
    ps = [P(L) for L in levels]
    mob = M(m)
    for r in range(1, rounds + 1):
        for p in ps:
            if p.hp > 0:
                mob.hp -= p.swing(mob)
        if mob.hp <= 0:
            return True, r, [1 - p.hp / p.max for p in ps]
        tank = next((p for p in ps if p.hp > 0), None)
        if tank is None:
            return False, r, [1.0] * len(ps)
        tank.hp -= mob.swing(tank)
        if all(p.hp <= 0 for p in ps):
            return False, r, [1.0] * len(ps)
    return False, rounds, [1 - p.hp / p.max for p in ps]

def report(m, levels, n=300):
    res = [fight(m, levels) for _ in range(n)]
    win = sum(1 for w, _, _ in res if w) / n
    rnd = statistics.mean(r for _, r, _ in res)
    lost = statistics.mean(l[0] for w, _, l in res if w) if win else 1.0
    return win, rnd, lost

if __name__ == '__main__':
    import json
    from brackets import B, party
    mobs, _ = wio.read_mobs()
    import os
    if os.path.exists(ROLES):          # from the last rebalance run
        roles = {int(k): v for k, v in json.load(open(ROLES)).items()}
    else:                              # from the files as they are: above the bracket is a boss
        zones = wio.read_zones()
        roles = {}
        for v, m in mobs.items():
            zn = wio.zone_of(v, zones)
            if zn in B and m.type in 'SE':
                lo, hi, _ = B[zn]
                if lo <= m.level <= hi:
                    roles[v] = ['normal', m.level, 1, zn]
                elif hi < m.level <= hi + 3:
                    roles[v] = ['boss', m.level, party(lo, hi), zn]
    only = [int(a) for a in sys.argv[1:]]
    for zn in sorted(B, key=lambda z: B[z][:2]):
        if only and zn not in only:
            continue
        lo, hi, _ = B[zn]
        normals = [mobs[v] for v, r in roles.items() if r[3] == zn and r[0] == 'normal' and mobs[v].type in 'SE']
        bosses = [mobs[v] for v, r in roles.items() if r[3] == zn and r[0] == 'boss' and mobs[v].type in 'SE']
        line = f"{zn:4d} {lo:3d}-{hi:<3d}"
        if normals:
            m = max(normals, key=lambda m: m.level)
            w, r, l = report(m, [m.level])
            line += f" | top normal L{m.level}: solo win {w:4.0%} {r:4.1f}rd -{l:3.0%}hp"
        for b in bosses[:1]:
            p = party(lo, hi)
            w1, r1, _ = report(b, [hi], 150)
            w10, r10, l10 = report(b, [min(100, hi + 10)], 150)
            wp, rp, lp = report(b, [hi] * p, 150)
            line += (f" | boss L{b.level}: solo@{hi} {w1:4.0%}  solo@{min(100,hi+10)} {w10:4.0%} "
                     f"party{p}@{hi} {wp:4.0%} {rp:4.1f}rd tank -{lp:3.0%}")
        print(line)
