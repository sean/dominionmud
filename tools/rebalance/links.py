"""Room exits and reachability (breadth-first from the mortal start room)."""
import re, glob, collections
from paths import WORLD as W
def load():
    rooms={}
    for f in glob.glob(W+'/wld/*.wld'):
        t=open(f,encoding='latin-1').read()
        for blk in re.split(r'\n(?=#\d+\n)',t):
            m=re.match(r'#(\d+)\n(.*?)~\n',blk,re.S)
            if not m: continue
            ex={int(d.group(1)):int(d.group(2)) for d in re.finditer(r'^D(\d)\n.*?~\n.*?~\n\S+ \S+ (-?\d+)',blk,re.S|re.M)}
            rooms[int(m.group(1))]=(m.group(2),ex,f)
    return rooms
def bfs(rooms, start=3039):
    dist={start:0}; q=collections.deque([start])
    while q:
        r=q.popleft()
        for d,to in rooms[r][1].items():
            if to in rooms and to not in dist:
                dist[to]=dist[r]+1; q.append(to)
    return dist
