>bribe_prog 5~
mpjunk coins
mpecho The clockwork fish swallows the coins with a clank, whirs, and sings in a tinny voice:
mpecho "Kobolds came by night from the mountain in the east,"
mpecho "they gnawed at the gate and they hammered and clawed;"
mpecho "the gnomes built it stronger with iron and steam,"
mpecho "but the catacombs wait where the old road is flawed."
mpecho The fish spits a stream of water into the air and falls silent.
~
>bribe_prog 1~
mpjunk coins
mpecho The clockwork fish gulps the coin, rattles, and coughs out a small puff of steam.  Perhaps it needs a few more.
~
>greet_prog 10~
if ispc($n)
  mpechoat $n A little brass plate on the fountain reads: "Feed the fish five coins for a song."
endif
~
|
