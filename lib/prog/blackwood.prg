>rand_prog 10~
   if level(ch) < 20   
     say If you are searching for the Heart of Evil, I can
     say assist you...for a price.
   else
     say Begone Fool!
~
>speech_prog price~
   grin
   say If you bring me a powerful ward from the High Tower
   say of Mages, I will give you the key to the Darkwall.
~
>speech_prog heart~
   grin
   say If you bring me a powerful ward from the High Tower
   say of Mages, I will give you the key to the Darkwall.
~
>give_prog ward major clear crystal~
   hold ward
   say Ha, pitiful fool!  Now you shall feel MY POWER!
   kill $n
   mpoload 7500
~
|
     
