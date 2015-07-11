>greet_prog 100~
   say Welcome to my bakery $n, how may I help you?
   smile $n
 endif
~
>speech_prog help~
 if isnpc($n)
   smile $n
   nod $n
 else
   say What do you seek help with?  Spheres? Training?
   say Darkweir Hollow? Gaining Experience?
 endif
~
>speech_prog darkweir~
 if ispc($n)
   say Darkweir Hollow is an excellent place to begin your adventuring.
   say It is located south of Elhrinn, just outside of the South Gate.
  if level($n) <= 5
   say It would be wise to not go unaccompanied until you are
   say sufficiently  experienced enough to hold your own in the wilds.  
   say The Dominion has many dangerous places...you must be careful, $n.
  else
   say $n, you should know the rumors of Darkweir by now!
  endif
 endif
~
>speech_prog spheres~
 if ispc($n) 
   say Ah yes, spheres are basically associated groups of knowledge
   say that are geared towards specific professions.  You can practice
   say any sub-sphere that is visible from its main sphere, thus
   say specializing in a certain field in that particular sphere.
 else
   whisper $n *joke*
   mpforce $n nod
 endif
~
>speech_prog trainers~
 if ispc($n)
   say If you seek out the proper trainer you will be able to
   say increase your attributes beyond their current states.
   mpecho $I pauses and thinks for a moment.
   say I believe the Template of Elrhinn can tell you more.
   say Good luck, $n!
   smile
 endif
~
|
