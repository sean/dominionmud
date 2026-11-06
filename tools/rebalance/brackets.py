# Approved Nov 2026: zone -> (low, high, boss name pattern or None for "new boss")
B = {
 200:(1,6,'gelatinous cube'), 352:(1,6,None), 30:(2,8,None), 36:(3,8,'rat king'),
 326:(3,10,'craw wyrm'), 58:(5,12,'krommyon'), 190:(6,14,'dark figure'), 55:(6,14,None),
 64:(8,16,'grand master'), 56:(10,18,None), 18:(10,18,'treant'), 110:(12,20,'aglandiir'),
 410:(12,20,'toll guard'), 430:(14,22,'spinner|measurer|cutter'), 75:(16,24,'darktree'),
 39:(18,26,'king peringuard'), 320:(20,28,'the mirror'), 420:(22,30,'morvane'),
 350:(22,30,None), 353:(24,32,None), 250:(25,35,'jirdidan'), 50:(25,35,'lord of the black watch'),
 255:(26,34,'great druid'), 105:(30,40,'grand dragon'), 61:(32,40,'green dragon'),
 42:(34,42,None), 43:(34,42,'morrow blackwake'), 11:(35,45,None), 63:(36,44,'arachnos'),
 104:(38,46,'crypt keeper'), 383:(38,48,'the emperor'), 66:(40,55,'eternity dragon'),
 40:(42,50,None), 81:(45,52,'keeper of the torch'), 80:(46,54,'grem trimbleshield'),
 53:(48,58,'gladiator'), 328:(50,60,'kehrn the troll king'), 25:(50,60,None),
 27:(52,62,'king vrask'), 100:(55,65,'spine'), 106:(58,68,'iron golem'), 280:(62,70,'chieftain$'),
 117:(65,75,None), 21:(66,74,None), 37:(70,80,'grand mistress'), 47:(70,80,'wizard'),
 360:(75,85,'gate watcher'), 45:(78,88,None), 52:(80,92,'dracolich'), 330:(85,95,None),
 260:(88,100,'melkor'),
}
TOWNS = {30, 350, 11, 100, 50, 320, 26}
def party(low, high):
    m = (low + high) / 2
    if m <= 20: return 2
    if m <= 40: return 3
    if m <= 50: return 3
    if m <= 80: return 4
    return 5
