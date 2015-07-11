>greet_prog 50~
  if level($n) <= 3
    say How the hell did you get down here?
    mpecho $I flips a knife and catches it while glaring at you intensly.
    say Oh...you must be one of the new recruits...just ask if I can
    say help you with anything.  Gonna cost you though.
  endif
~
>speech_prog help~
  mpecho $I turns his head and spits.
  say Well, what the hell do you want to know about?  Spheres? 
~  
>speech_prog sphere spheres~
  say Here's what you do, Little One...you practice 
  say Thievery with me...then go find Anas or that 
  say drunken slob, Durik.. and they'll train you in 
  say either Looting, Deception, or Murder, all three 
  say of which are the sub-sphere's of thievery.
  think
  say Don't ask me where the hell they are...
  say because I don't know.
~
>speech_prog advice~
  say You want some advice, $n?
  say Don't let yourself get attached to anything you're
  say not willing to walk out on in 30 seconds flat if
  say you hear the city guardsmen coming around the corner.
~
|
