"""Move every quest's level into its zone's bracket (lib/data/quests).

  kill quests       a level or two under the target's new level (a group
                    boss: the middle of the bracket), never outside it
  daily errands     in towns: open from level 1
  everything else   keeps its place among its zone's quests, spread over
                    the bracket
An explicit maximum keeps its distance from the minimum.  The Pirate King
(4201) is offered from level 35."""
import re, json, collections
import wio
from paths import ROLES
from brackets import B, TOWNS

Q = wio.ROOT + '/data/quests'
SPECIAL = {4201: 35}

def main():
    lines = open(Q, encoding='latin-1').read().split('\n')
    roles = {int(k): v for k, v in json.load(open(ROLES)).items()}
    mobs, _ = wio.read_mobs()
    quests = []                     # dicts with line indexes
    cur = None
    for i, l in enumerate(lines):
        m = re.match(r'^#(\d+)\s', l)
        if m:
            cur = {'vnum': int(m.group(1)), 'level_line': None, 'kill': None, 'daily': False}
            quests.append(cur)
        elif cur is not None:
            m = re.match(r'^level\s+(\d+)(?:\s+(\d+))?', l)
            if m:
                cur['level_line'] = i
                cur['min'] = int(m.group(1))
                cur['max'] = int(m.group(2)) if m.group(2) else None
            m = re.match(r'^kill\s+(\d+)', l)
            if m:
                cur['kill'] = int(m.group(1))
            if l.strip() == 'daily':
                cur['daily'] = True
    byzone = collections.defaultdict(list)
    for q in quests:
        if q['level_line'] is not None:
            byzone[q['vnum'] // 100].append(q)
    changed = 0
    for zn, qs in byzone.items():
        if zn not in B:
            continue
        lo, hi, _ = B[zn]
        mins = sorted({q['min'] for q in qs})
        for q in qs:
            if q['vnum'] in SPECIAL:
                new = SPECIAL[q['vnum']]
            elif q['kill'] and q['kill'] in mobs:
                r = roles.get(q['kill'])
                tl = mobs[q['kill']].level
                new = (lo + hi) // 2 if r and r[0] == 'boss' else tl - 2
            elif q['daily'] and zn in TOWNS:
                new = 1
            else:
                rel = mins.index(q['min']) / max(1, len(mins) - 1) if len(mins) > 1 else 0
                new = round(lo + (hi - lo) * rel)
            if not (q['daily'] and zn in TOWNS) and q['vnum'] not in SPECIAL:
                new = max(lo, min(hi, new))
            new = max(1, new)
            text = f"level {new}"
            if q['max'] is not None:
                text += f" {new + max(0, q['max'] - q['min'])}"
            if lines[q['level_line']] != text:
                lines[q['level_line']] = text
                changed += 1
    open(Q, 'w', encoding='latin-1').write('\n'.join(lines))
    print('quests re-levelled', changed)

if __name__ == '__main__':
    main()
