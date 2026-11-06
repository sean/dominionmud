# The level curve: k(L) same-level kills to finish level L, e(L) exp a same-level mob gives.
def k(L):
    if L <= 50: return 8 + 0.4*L
    return 28 + 0.83*(L-50)**1.5
def e(L): return int(round(8*L*L + 12*L + 30))
def table():
    t=[0,0]; tot=0
    for L in range(1,100):
        tot += k(L)*e(L); t.append(int(round(tot,-2)))
    # immortal levels 101-108: unreachable by play
    for L in range(101,109): t.append(t[-1]*2 if L==101 else t[-1]+100000000)
    return t
if __name__=='__main__':
    t=table()
    print('entries',len(t),'L42',t[42],'L50',t[50],'L100',t[100],'L108',t[108])
    print('kills 1->42',round(sum(k(l) for l in range(1,42))),' 1->100',round(sum(k(l) for l in range(1,100))))
    for L in (1,2,5,10,20,30,42,50,60,75,90,99): print(L, 'kills',round(k(L)),'mob exp',e(L),'span',t[L+1]-t[L])
