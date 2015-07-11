>rand_prog 50~
 if isgood($n) 
   if rand(35)
     say Someone needs to teach you people a lesson.
     growl
   else
     if goldamt($n) >= 5000 
       mpechoat $n $I takes a long look at you.
       mpechoaround $n $I takes a long look at $n.
       say That's a purty, little purse ye gots, my friend.
       say Looks so heavy I may have to take it off yer hands.
       steal gold $n
       grin 
     endif
   endif
 endif
~
>greet_prog 50~
 if sex($n) == 2
   wolfwhistle
   mpechoat $n $I looks you up and down.
   mpechoaround $n $I looks $n up and down greedily.
   say Hey there, little lady.  I got what you need.
   steal gold $n
   grin
 endif
~
>entry_prog 40~
 mpecho $I scans the area and looks over $l shoulder several times.
~
>rand_prog 15~
 mpecho $I tosses $l dagger up and down casually.
~
>rand_prog 10~
 mpecho $I pulls $l cloak about $kself and scans the area.
~
|	


