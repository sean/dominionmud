>greet_prog 15~
 if isgood($n)
   mpechoat $r $I turns and looks at you.
   mpechoaround $r $I turns and looks at $n.
   say Will you save us from the Usurper?
 else
   cower
 endif
~
>speech_prog usurper~
 if isgood($n)
  say Someday someone will conquer him and bring peace to the city.
  mpecho $I looks to the sky and utters a prayer of hope.
  say Ages ago, this city was a haven to all who had kindness
  say within their soul.  Now it is a den of evil.
  say You can thank Spine for that.  Destroyed the entire city.
  say No one even dares to try and stand up against him....
 else
  say Do not say his name!
  mpecho $I suddenly turns sheet white and cowers in fear.    
~
|
