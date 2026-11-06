#!/usr/bin/env python3
"""Stat lines for new mobs, from the balance model (formulas.py).

  statline.py <level> [boss <party>] [str <n>] [dex <n>] [sanct] [haste]

prints the two numeric lines of an 'S' or 'E' mob record:
  <level> <hitroll> <ac> <XdY+Z hit points> <XdY+0 damage>
  <gold> <experience>
An ordinary mob of level L takes a player of level L about five rounds and
30% of his hit points; a boss built for a party of p has 2p times the hit
points and hits 15% harder.  Give its Str and Dex if its record sets them
(Str may be lowered: use the Str printed), and sanct / haste if it has
those affects.  With 'table', prints the reference numbers for levels 1-100."""
import sys
from formulas import stats, p_hp, player_dpr, mob_exp, mob_gold

def main(a):
    if not a or a[0] in ('-h', '--help'):
        print(__doc__); return
    if a[0] == 'table':
        print(' lvl  player-hp  player-dpr  mob-hp  boss3-hp  exp    gold')
        for L in (1, 5, 10, 15, 20, 30, 40, 50, 60, 70, 80, 90, 100):
            s, b = stats(L), stats(L, 'boss', 3)
            hp = lambda t: t['hp'][0] * (t['hp'][1] + 1) / 2 + t['hp'][2]
            print(f"{L:4d} {p_hp(L):9.0f} {player_dpr(L):11.1f} {hp(s):7.0f} {hp(b):9.0f} "
                  f"{mob_exp(L):6d} {mob_gold(L):6d}")
        return
    L, role, party, st, dx, sanct, haste = int(a[0]), 'normal', 1, 11, 11, False, False
    i = 1
    while i < len(a):
        if a[i] == 'boss': role, party, i = 'boss', int(a[i + 1]), i + 2
        elif a[i] == 'str': st, i = int(a[i + 1]), i + 2
        elif a[i] == 'dex': dx, i = int(a[i + 1]), i + 2
        elif a[i] == 'sanct': sanct, i = True, i + 1
        elif a[i] == 'haste': haste, i = True, i + 1
        else: raise SystemExit(f'what is {a[i]!r}?  (statline.py --help)')
    s = stats(L, role, party, st, dx, sanct, haste)
    hp, dm = s['hp'], s['dam']
    print(f"{L} {s['hitroll']} {s['ac']} {hp[0]}d{hp[1]}+{hp[2]} {dm[0]}d{dm[1]}+{dm[2]}")
    print(f"{s['gold']} {s['exp']}")
    if s['str'] != st:
        print(f"(Str: {s['str']})")

if __name__ == '__main__':
    main(sys.argv[1:])
