>greet_prog 25~
 if isgood($n)
    shake $n
   say Good day to you.
 else
   mpechoat $n $I gives you a long, cold stare.
   mpechoaround $n $I gives $n a long, cold stare. 
   say Move along, $n.
 endif
~
|
>rand_prog 20~
  mpecho $I taps his sword against his palm.
~
>rand_prog 55~
  if level($n) == 1
    say Hello, $n.  Just ask if you need assistance.
  endif
~
>rand_prog 20~
    mpecho $I quickly scans the area.
~
>speech_prog help assistance~
   tell $n What do you need help with?  I don't really know all that
   tell $n much, but you may want to speak with Jynnah.
   tell $n She is the baker up near the inn on Dalkyst Road.
~
>fight_prog 24~
 if rand(20)  if hitprcnt <= 25 
   shout My comrades, I am in need of thine aid!
   mpecho A member of the Red Guard has arrived.
   mpmload 3060
   mpforce 2.guard assist guard
   say We shall be victorious!
  endif
 endif
~
>act_prog p tried to steal something from you~
 say We don't take kindly to thieves here!
 shout Guards!  I've caught a thief!!
 mpkill $n
~
|
