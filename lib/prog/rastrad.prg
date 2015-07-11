>greet_prog 25~
   if isimmort ($n)
     mpechoat $n $I looks up from his work and quickly kneels before you. 
     mpechoaround $n $I looks up and quickly kneels before $n.
else 
     mpecho Hot steam hisses from a bucket as $I sticks a red hot sword in
     mpecho to cool it off.  He then turns his attention towards you.
     say Hello there, friend, what can I do for you?
   endif
~
>rand_prog 5~
   mpecho $I begins to hammer on a red, hot metal rod.
   mpecho Slowly, it begins to take the shape of a sword.
   mpecho After a while, Rastrad stops hammering and admires it.
   mpecho He then places it in a bucket of water and steam rises into the air. 
~
>rand_prog 5~
   mpecho Rastrad stands up and stretches his arms high in the air.
   mpecho You hear a loud CRACK as the bones in his back and neck unknit.
   mpecho Rastrad breathes out a heavy sigh of relief and satisfaction.
~
>rand_prog 5~
   mpecho Rastrad mutters something about 'shitty metal' and 'long days'.
~
>rand_prog 5~
   mpecho Rastrad stands up and walks over to a wall covered in weapons.
   mpecho He takes a deep breath and blows a cloud of dust off an old hammer.
   mpecho He then turns back to the counter and sits down in a chair.
~
>rand_prog 5~
   pon
   say interested in a little work?
~
>speech_prog work~
   say take this over to the Weaponshop in Gnishmire.  
   say The smith there will pay you.
   rem bar
   give bar $n
~
|
