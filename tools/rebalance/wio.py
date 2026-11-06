"""World file reading for the rebalance: mobs, objects, zones, shops.

Mob and object records keep the file's lines, and the indexes of the lines
that hold the stats, so a rewrite touches only those lines."""
import re, glob, os, collections

from paths import LIB as ROOT, WORLD as W

def index(kind):
    out = []
    for l in open(f'{W}/{kind}/index', encoding='latin-1'):
        l = l.strip()
        if not l or l.startswith('$'):
            break
        out.append(l)
    return out

def flagbits(s):
    if s.lstrip('-').isdigit():
        return int(s)
    v = 0
    for c in s:
        if 'a' <= c <= 'z': v |= 1 << (ord(c) - 97)
        elif 'A' <= c <= 'Z': v |= 1 << (26 + ord(c) - 65)
    return v

def split_blocks(lines):
    """[(vnum, [line indexes])] for '#vnum' blocks; trailing '$' block excluded."""
    blocks, cur, start = [], None, None
    for i, l in enumerate(lines):
        if re.match(r'^#\d+\s*$', l):
            if cur is not None:
                blocks.append((cur, start, i))
            cur, start = int(l[1:]), i
        elif l.startswith('$'):
            if cur is not None:
                blocks.append((cur, start, i))
            cur = None
            break
    if cur is not None:
        blocks.append((cur, start, len(lines)))
    return blocks

def skip_strings(lines, i, n):
    """Skip n '~'-terminated strings starting at line i; return next line index."""
    for _ in range(n):
        while not lines[i].rstrip().endswith('~'):
            i += 1
        i += 1
    return i

STAT_RE = re.compile(r'^\s*(-?\d+)\s+(-?\d+)\s+(-?\d+)\s+(\d+)d(\d+)\+(-?\d+)\s+(\d+)d(\d+)\+(-?\d+)\s*$')

class Mob:
    pass

def read_mobs():
    mobs = {}
    files = {}
    for f in index('mob'):
        path = f'{W}/mob/{f}'
        lines = open(path, encoding='latin-1').read().split('\n')
        files[f] = lines
        for vnum, s, e in split_blocks(lines):
            m = Mob()
            m.vnum, m.file, m.start, m.end = vnum, f, s, e
            i = skip_strings(lines, s + 1, 4)
            fl = lines[i].split()
            m.flags_line = i
            m.flags, m.aff, m.align, m.type = fl[0], fl[1], int(fl[2]), fl[3]
            m.flagbits = flagbits(fl[0])
            names = lines[s + 1].rstrip('~')
            m.names = names
            j = s + 2
            m.short = lines[j].rstrip('~') if lines[j].endswith('~') else lines[j]
            m.stat_line = m.gold_line = None
            if m.type in 'SEL':
                st = STAT_RE.match(lines[i + 1])
                if not st:
                    raise SystemExit(f'mob {vnum} ({f}): bad stat line {lines[i+1]!r}')
                m.stat_line = i + 1
                t = [int(x) for x in st.groups()]
                m.level, m.hitroll, m.ac = t[0], t[1], t[2]
                m.hpd = t[3:6]; m.damd = t[6:9]
                m.gold_line = i + 2
                g = lines[i + 2].split()
                m.gold, m.exp = int(g[0]), int(g[1])
                m.espec = {}
                if m.type == 'E':
                    k = i + 4
                    while k < e and lines[k].strip() != 'E':
                        mm = re.match(r'^(\w+):\s*(-?\d+)', lines[k])
                        if mm: m.espec[mm.group(1)] = (k, int(mm.group(2)))
                        k += 1
            elif m.type == 'A':
                # A: "level race" line then others
                t = lines[i + 1].split()
                m.stat_line = i + 1
                m.level = int(t[0])
                m.hitroll = m.ac = 0; m.hpd = m.damd = None; m.gold = m.exp = 0; m.espec = {}
            m.aggressive = bool(m.flagbits & (1 << 5))
            m.sentinel = bool(m.flagbits & (1 << 1))
            mobs[vnum] = m
    return mobs, files

class Obj:
    pass

def read_objs():
    objs, files = {}, {}
    for f in index('obj'):
        path = f'{W}/obj/{f}'
        lines = open(path, encoding='latin-1').read().split('\n')
        files[f] = lines
        for vnum, s, e in split_blocks(lines):
            o = Obj()
            o.vnum, o.file, o.start, o.end = vnum, f, s, e
            i = skip_strings(lines, s + 1, 4)
            o.names = lines[s + 1].rstrip('~')
            o.short = lines[s + 2].rstrip('~')
            t = lines[i].split()
            o.type_line = i
            o.type, o.extra, o.wear = int(t[0]), t[1], t[2]
            o.val_line = i + 1
            o.vals = [int(x) for x in lines[i + 1].split()]
            o.wcr_line = i + 2
            w = lines[i + 2].split()
            o.weight, o.cost, o.rent = int(w[0]), int(w[1]), int(w[2])
            o.affects = []          # [(line index of values, loc, mod)]
            k = i + 3
            while k < e:
                if lines[k].strip() == 'A':
                    a = lines[k + 1].split()
                    o.affects.append((k + 1, int(a[0]), int(a[1])))
                    k += 2
                elif lines[k].strip() == 'E':
                    k = skip_strings(lines, k + 1, 2)
                else:
                    k += 1
            objs[vnum] = o
    return objs, files

def read_zones():
    zones = []
    for f in index('zon'):
        path = f'{W}/zon/{f}'
        lines = open(path, encoding='latin-1').read().split('\n')
        z = {'file': f, 'lines': lines}
        z['num'] = int(lines[0][1:])
        z['name'] = lines[1].rstrip('~')
        z['levelrec_line'] = 3
        z['levelrec'] = lines[3].rstrip('~')
        # top is the first all-numeric line of 3 fields after the header
        k = 4
        while not re.match(r'^\d+ \d+ \d+', lines[k]):
            k += 1
        z['top'] = int(lines[k].split()[0])
        cmds = []
        for l in lines[k + 1:]:
            s = l.split('*')[0].split()
            if not s or s[0] in ('S', '$'):
                if s and s[0] == 'S': break
                continue
            if s[0] in 'MOGEPDRL' and len(s) >= 4:
                try:
                    cmds.append((s[0], [int(x) for x in s[1:5] if re.match(r'^-?\d+$', x)]))
                except ValueError:
                    pass
        z['cmds'] = cmds
        zones.append(z)
    zones.sort(key=lambda z: z['num'])
    return zones

def zone_of(vnum, zones):
    for z in zones:
        if z['num'] * 100 <= vnum <= z['top']:
            return z['num']
    return None

def read_shops():
    shops = []
    for f in index('shp'):
        lines = open(f'{W}/shp/{f}', encoding='latin-1').read().split('\n')
        i = 0
        lines = lines + ['-1'] * 40
        while i < len(lines) - 40:
            m = re.match(r'^#(\d+)~', lines[i])
            if not m:
                i += 1; continue
            i += 1
            prods = []
            while lines[i].strip() != '-1' and '.' not in lines[i]:
                prods.append(int(lines[i])); i += 1
            if lines[i].strip() == '-1': i += 1
            i += 2  # profit buy, profit sell
            while lines[i].strip() != '-1':
                i += 1
            i += 1
            i = skip_strings(lines, i, 7)
            temper, bits, keeper, who = (int(lines[i + x]) for x in range(4))
            i += 4
            rooms = []
            while lines[i].strip() != '-1':
                rooms.append(int(lines[i])); i += 1
            if not (1 <= keeper): print('shop?', f, m.group(1), keeper)
            shops.append({'vnum': int(m.group(1)), 'prods': prods, 'keeper': keeper, 'rooms': rooms, 'file': f})
    return shops

def write_lines(kind, f, lines):
    open(f'{W}/{kind}/{f}', 'w', encoding='latin-1').write('\n'.join(lines))
