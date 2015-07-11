>greet_prog 50~
  if level($n) >= 10
    tell $n Are you searching for the Eye of Sul-Katr as well?
  endif
~
>speech_prog sul-katr eye~
  say Back in the Dark Years, a great magician ruled
  say the land.  He supposedly carried the Eye of Sul-Katr
  say which allowed him to see anything anywhere in the realm!
~
>speech_prog magician~
  say Well supposedly, the mage was killed by a
  say strange beast...It was half bird and half man... 
  say A Hippogriff.
~
>speech_prog hippogriff~
  ponder
  say You can probably find him in the trees somewhere
  say in the southern reaches of the Hallows.
  say Good luck $n!
~
>rand_prog 5~
  mpecho $I claps and one of his slaves brings him a cup of water.
~
>rand_prog 8~
  mpecho $I draws a strange, eye-shaped symbol on the ground.
  ponder
~
|
