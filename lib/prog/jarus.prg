>greet_prog 25~
 if isimmort ($n)
   mpechoat $n $I looks up from his work and quickly kneels before you. 
   mpechoaround $n $I looks up and quickly kneels before $n.
 else 
   mpecho $I instructs a few workers where to go, and then walks up to you.
   say Hello there, friend, what can I do for you?
 endif
~
>rand_prog 10~
   mpecho The dockworkers suddenly drop a heavy raft they are carrying.
   mpecho $I wheels to face them.
   say Do You All Know How Much That Just Cost ME?
   mpecho Jarus shakes his whip at the dockworkers.
~
>rand_prog 10~
   mpecho Jarus shouts at the workers who are moving too slow for him.
   mpecho You all are slower than a mules ass!
   mpecho You hear a CRACK as Jarus's whip catches them across the back.
~
|
