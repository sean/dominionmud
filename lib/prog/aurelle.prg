>greet_prog 100~
if ispc($n)
  mpechoat $n The pale light gathers itself into the shape of a young woman, and a soft voice speaks inside your head.
  mpechoat $n "You followed my light.  I am sorry; it was the only way I had to call for help.  I am Aurelle, and my brother sleeps below."
endif
~
>speech_prog brother morvane~
if ispc($n)
  mpechoat $n "Morvane.  We were born beneath one star, of House Gemmar.  He feared death more than anything, and he found a way to cheat it: the souls of everyone the Fiend lures here feed him, through the trees."
  mpechoat $n "I tried to stop him.  He sealed my body away behind a white door and left my light to wander.  The door will open only to the name we shared."
endif
~
>speech_prog name door gemmar~
if ispc($n)
  mpechoat $n "Our house's name is carved on this crypt, if you know where to look.  Speak it at my door, and I will wake long enough to help you."
endif
~
>rand_prog 3~
mpecho The pale light flickers, and for a moment you hear a woman softly weeping.
~
|
