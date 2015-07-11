>rand_prog 5~
   mpecho The passed out drunk in the corner of the bar vomits quietly.
   mpecho The bartender calls, 'You sick piece of shit, clean that up!'
~
>speech_prog elf silverleaf~
   growl
   say I don't know what the hell you're talking about...
   mpechoat $n You see Curdine whip a knife out from behind his back!
   mpechoaround $n You see Curdine pull out a knife and advance on $n.
   mpkill $n
~
>speech_prog curdine~
   mpecho The passed out man in the corner of the bar raises his head...
   say who da hell said dat?
~
>fight_prog 100~
  if hitprcnt($i) <= 10
   mpechoat $n $I falls down and puts his hand up in defense.
   mpechoaround $n $I falls to his knees and begs $n for mercy.
   say p-p-p-lease don't killl meeeee....I'll 
   say t-t-tell you anything...
   mpmload 5891
   mpgoto 1
  endif
~
|
