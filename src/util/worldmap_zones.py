#!/usr/bin/env python3
"""Read DominionMUD's zone, room and mob files and summarise each zone.

    python3 src/util/worldmap_zones.py [-l lib]

Prints each zone's room count, sector mix, links to other zones and its
strongest mobs.  worldmap.py imports load() from here for the roads.
"""
import re, sys, collections, argparse


def load(lib="lib"):
    W = lib.rstrip("/") + "/world"
    zones=[]
    for f in open(W+'/zon/index').read().split():
        if f=='$':break
        L=open(W+'/zon/'+f,errors='replace').read().split('\n')
        num=int(L[0][1:]);name=L[1].rstrip('~')
        top=int(L[5].split()[0])
        resets=[l for l in L if l.startswith('M ')]
        zones.append(dict(num=num,name=name,top=top,mresets=[int(l.split()[2]) for l in resets]))
    zones.sort(key=lambda z:z['top']);b=0
    for z in zones: z['bot']=b; b=z['top']+1
    def zof(v):
        for z in zones:
            if z['bot']<=v<=z['top']: return z['num']
    rooms={}
    for f in open(W+'/wld/index').read().split():
        if f=='$':break
        txt=open(W+'/wld/'+f,errors='replace').read()
        for blk in re.split(r'\n(?=#\d+\n)','\n'+txt):
            m=re.match(r'\s*#(\d+)\n',blk)
            if not m:continue
            v=int(m.group(1));parts=blk.split('~')
            name=parts[0].split('\n',2)[-1].strip() if len(parts)>1 else ''
            # flags line after desc~
            rest=parts[2] if len(parts)>2 else ''
            fl=rest.strip().split('\n')[0].split()
            sect=int(fl[2]) if len(fl)>2 and fl[2].lstrip('-').isdigit() else 0
            ex=[(int(d),int(t)) for d,t in re.findall(r'\nD(\d)\n[^~]*~\n[^~]*~\n\s*-?\d+\s+-?\d+\s+(-?\d+)',blk)]
            rooms[v]=dict(name=name,sect=sect,ex=ex)
    mobs={}
    for f in open(W+'/mob/index').read().split():
        if f=='$':break
        txt=open(W+'/mob/'+f,errors='replace').read()
        for blk in re.split(r'\n(?=#\d+\n)','\n'+txt):
            m=re.match(r'\s*#(\d+)\n',blk)
            if not m:continue
            p=blk.split('~')
            if len(p)<5:continue
            short=p[1].strip(); after=p[4].strip().split('\n')
            try: lvl=int(after[1].split()[0])
            except: continue
            mobs[int(m.group(1))]=dict(short=short,level=lvl,flags=after[0].split()[0])
    out=[]
    for z in zones:
        rs=[v for v in rooms if z['bot']<=v<=z['top']]
        sec=collections.Counter(rooms[v]['sect'] for v in rs)
        links=collections.Counter()
        for v in rs:
            for d,t in rooms[v]['ex']:
                if t in rooms:
                    tz=zof(t)
                    if tz!=z['num']: links[tz]+=1
        ms=sorted({m for m in z['mresets'] if m in mobs},key=lambda m:-mobs[m]['level'])
        out.append(dict(num=z['num'],name=z['name'],rooms=len(rs),sect=dict(sec),links=dict(links),
            topmobs=[(m,mobs[m]['short'],mobs[m]['level']) for m in ms[:4]]))
    return out


SECTORS = "IN CITY FIELD FOREST HILLS MTN SWIM WATER UNDER FLY GLAC DESERT JUNGLE".split()

if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-l", "--lib", default="lib", help="path to the lib directory (default: lib)")
    a = ap.parse_args()
    S = SECTORS
    for z in load(a.lib):
        print(z['num'], z['name'], z['rooms'], {S[k] if k < len(S) else k: v for k, v in sorted(z['sect'].items(), key=lambda x: -x[1])}, '->', z['links'])
        print('    ', [(n, l) for _, n, l in z['topmobs']])
