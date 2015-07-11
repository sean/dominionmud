>greet_prog 30~
 if level($n) <= 7
   say Hey there, friend, this place is much too dangerous for you.
   say It'd be a good idea if you learned more and then came back.
 else
   say Take care in the swamps.  Few enter.  Even fewer return.
 endif
~
>rand_prog 25~
 if rand(50)
   mpecho $I swats at an insect that is nibbling on his neck.
 else
   mpecho $I makes a few half hearted thrusts with $l sword
   mpecho then leans back against the waypost wiping $l brow.
   say Gotta stay in shape ya know.
 endif
~
|
