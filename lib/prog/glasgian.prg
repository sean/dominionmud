>greet_prog 100~
 if level($n) >= 48
   bow $n
   say What's shaking Big Worm?
 else
   say Break yourself fool!
   mpforce $n flee
 endif
~
|
