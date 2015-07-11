>greet_prog 45~
   say Well now, this is sure interesting....
   mpecho The old elf peers at you from beneath wire-rimmed spectacles.
   say Haven't had any visitors around here since...well, i dunno...
~
>greet_prog 45~
   say Say there, you wouldn't want to do me a favor would ye?
~
>speech_prog favor what~
   say Well, i need to deliver this letter to an old friend 
   say of mine but i'm no aventurer anymore but you look as 
   say if you might be the kind that would help an old elf out.
   mpecho $I looks at you with hopeful eyes.
   say will you help me out?
~
>speech_prog yes~
   smile
   say allrighty then, just take it to the ol dwarf at the 
   say Pelorinne Trading Post.  Then come back here with his 
   say seal and I'll give give ya a little reward.
   mpoload 32037
   give letter $n
~
>give_prog andar coin seal~
   say Well, I guess the old bastard IS still alive...good to know
   say I suppose I owe you something in return for the favor...
   give key $n
   say I forget what that goes to...something important though...
   say Good luck in your journeys, $n.
~
>rand_prog 15~
   mpecho $I stands up and stokes the fire in his fireplace.
~
>rand_prog 15~
   mpecho $I walks over to a bookshelf and begins to thumb through a book.
~
>speech_prog silverleaf~
   say I once heard of an elf that wandered into the swamps long ago...
   say Heard he was killed too...
   say Poor bastard.
   say They say he was attacked and had his heart ripped out somewhere
   say near the southwestern section of the swamp.
~
|
