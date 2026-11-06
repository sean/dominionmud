"""The Nov 2026 balance model.  A 'reference player' of level L wearing gear
of his level, against a mob of level L:

  player HP          15 + 3.5L                (race data, unchanged)
  player hitroll     0.5L (levels) + 2 (Str 16) + L/8 (gear)
  player weapon avg  3 + 0.3L  (+2 Str, +L/16 weapon skill)
  attacks / round    1 + 0.006L (double / triple attack)
  player armour      3 + 0.4L % absorbed (full gear of his level)
  player defence     60 (Dex 15)

  mob armour         5 + 0.25L % absorbed
  mob hitroll        10 + 0.5L
  mob defence        40 (Dex 11) unless its Dex says otherwise

An ordinary mob takes about 5 rounds for the player to kill, and costs him
about 30% of his hit points.  A group boss built for p players has 2p times
the hit points and hits 15% harder: p players take about 13 rounds and the
one it fights loses most of his hit points; one alone takes p times as long
and dies, unless he is ten or more levels above it in the lower brackets.  These functions are mirrored in
src/levels.c (mob_set_level) for mobs made at run time.
"""

STR_TODAM = {0:-5,1:-4,2:-2,3:-1,4:-1,5:-1,6:0,7:0,8:0,9:0,10:0,11:0,12:0,13:0,14:1,15:1,
             16:2,17:3,18:5,19:7,20:8,21:9,22:10,23:11,24:12,25:14}
DEX_DEF = {0:0,1:2,2:4,3:6,4:8,5:10,6:15,7:20,8:25,9:30,10:35,11:40,12:45,13:50,14:55,15:60,
           16:65,17:70,18:75,19:80,20:85,21:90,22:95,23:100,24:105,25:110}

def p_hp(L):      return 15 + 3.5 * L
def p_hitroll(L): return 0.5 * L + 2 + L / 8
def p_weapon(L):  return 3 + 0.3 * L
def p_attacks(L): return 1.2 + 0.006 * L   # +20%: the chance of a fourth attack
def p_armour(L):  return 3 + 0.4 * L
def m_armour(L):  return 5 + 0.25 * L
def m_hitroll(L): return 10 + 0.5 * L

def hit_chance(defence, hitroll):
    return max(0.05, min(1.0, (101 - defence + hitroll) / 100))

def player_dpr(L, mob_def=40):
    per = p_weapon(L) + 2 + L / 16
    return p_attacks(L) * hit_chance(mob_def, p_hitroll(L)) * per * (1 - m_armour(L) / 100)

def mob_hp(L, mob_def=40, rounds=5.0):
    return max(4, rounds * player_dpr(L, mob_def))

def mob_hit_avg(L, share=0.06, attacks=1.2):
    """Average damage of one mob hit (before Str), so that it takes `share`
    of a same-level player's hit points per round.  Mobs swing 1.2 times a
    round (the chance of a fourth attack), 2.2 when hasted."""
    per_round = share * p_hp(L)
    return per_round / (attacks * hit_chance(60, m_hitroll(L)) * (1 - p_armour(L) / 100))

def max_str(L, hit):
    """Highest Str whose damage bonus is at most 40% of a hit."""
    return max([s for s, v in STR_TODAM.items() if v <= 0.4 * hit] or [11])

def mob_gold(L):  return int(round(2 * L + 0.15 * L * L))
def mob_exp(L):   return int(round(8 * L * L + 12 * L + 30))

def max_dex(L):
    """Highest Dex a level-L mob keeps (defence <= 40 + 0.6L)."""
    cap = 40 + 0.6 * L
    return max(d for d, v in DEX_DEF.items() if v <= cap)

def dice_for(avg, ndice):
    """XdY+0 with about the given average.  (A mob's +Z is its damroll,
    which hit() does not add to the blow, so all the damage is in dice.)"""
    avg = max(1.0, avg)
    n = max(1, min(ndice, int(avg)))
    size = max(1, int(round(2 * avg / n - 1)))
    return n, size, 0

def stats(L, role='normal', party=1, strength=11, dex=11, sanct=False, haste=False):
    """New stat-line values for a mob: (hitroll, ac, hp(X,Y,Z), dam(X,Y,Z), gold, exp, str)."""
    mdef = DEX_DEF.get(min(dex, max_dex(L)), 40)
    hp = mob_hp(L, mdef) / (2 if sanct else 1)
    hit = mob_hit_avg(L, attacks=2.2 if haste else 1.2)
    gold, exp = mob_gold(L), mob_exp(L)
    if role == 'boss':
        hp *= 2 * party
        hit *= 1.15
        exp = int(exp * 2 * party)
        gold = int(gold * 3 * party)
    if role == 'boss':
        pass
    strength = min(strength, max_str(L, hit))
    hit -= STR_TODAM.get(strength, 0)
    hx = max(1, L // 3)
    hy = 6
    hz = int(round(hp - hx * 3.5))
    while hz < 0 and hx > 1:
        hx -= 1
        hz = int(round(hp - hx * 3.5))
    hz = max(0, hz)
    dmg = dice_for(hit, 1 + L // 12)
    return dict(hitroll=int(round(m_hitroll(L))), ac=int(round(m_armour(L))),
                hp=(hx, hy, hz), dam=dmg, gold=gold, exp=exp, str=strength)

if __name__ == '__main__':
    for L in (1, 5, 10, 20, 30, 42, 50, 60, 75, 90, 100):
        s = stats(L)
        b = stats(L, 'boss', 3)
        print(f"L{L:3d} player hp {p_hp(L):5.0f} dpr {player_dpr(L):5.1f} | mob hp {s['hp']} "
              f"dam {s['dam']} hr {s['hitroll']} ac {s['ac']} gold {s['gold']} exp {s['exp']} "
              f"| boss3 hp {b['hp']} dam {b['dam']}")
