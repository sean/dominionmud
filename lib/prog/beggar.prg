>bribe_prog 5~
 if ispc($n)
   say Thank you $n, you are very kind.
   if level($n) <= 3
     if rand(50)
       tell $n If you seek to hone your thieving skills take the
       tell $n trapdoor down from the Grog and Tankard.
     endif
   else
     smile $n
   endif
 endif
~
>rand_prog 30~
 if ispc($n)
   mpecho $I tugs at your sleeve.
   say Can you spare some change, friend?
 endif
~
|

