"""Write each zone's bracket into its zone file's 'levels recommended'
line (zone_data.levelrec) as "<low>-<high>"; src/levels.c reads it."""
import wio
from brackets import B
n = 0
for z in wio.read_zones():
    if z['num'] in B:
        lo, hi, _ = B[z['num']]
        lines = z['lines']
        lines[z['levelrec_line']] = f'{lo}-{hi}~'
        open(f"{wio.W}/zon/{z['file']}", 'w', encoding='latin-1').write('\n'.join(lines))
        n += 1
print(n, 'zones')
