>rand_prog 15~
   mpecho You hear a strange voice whisper, 'Release me...Release me...'
~
>speech_prog release~
   mpecho The corpse opens blood soaked eyes which stare up at the sky.
   mpecho The corpse whispers, 'Only the silverleaf can release me.'
~
>speech_prog silverleaf~
   say I know not of what has become of my soul.
~
>give_prog silverleaf~
   mpecho A bright light pours forth from the rotten body of the corpse.
   mpecho It stands up and is suddenly garbed in armor with sword in hand.
   mpecho An elven warrior says, 'You have released me from my purgatory!'
   mpecho He continues, 'For this you shall be rewarded.'
   mpoload 5891
   give eye $n
   mpecho The bright light encompasses the warrior and then it disappears.
   mpgoto 6
~
|
