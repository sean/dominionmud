>greet_prog 40~
 if level($n) < 3
   if rand(30)
     say Hi, $n, if you need any help, seek out
     say Jynnah or a member of the Red Guard.
   else
     smile $n
   endif
 else
   if rand(50)
    if ispc($n) 
     mpechoat $n $I tips $l hat at you.
     mpechoaround $n $I tips $l hat at $n.
    endif
   endif
 endif
~
>entry_prog 5~
 look
 if level($n) <= 5
   say Hello there, Elrhinn sure is looking good.
   say Hail to the Template!
 else
   bow $n
 endif
~
>fight_prog 40~
 shout Help! Help! $n is trying to kill me!
 if level($n) > 3
   mpecho A member of the Red Guard runs into the room.
   mpmload 3060
   mpecho The guard leaps to the aid of the poor citizen.
   mpforce guard hit $n
   flee
 endif
~
>act_prog p tried to steal something from you~
  shout Guards! Guards! $n is a thief!!
  flee
~
>act_prog p pokes you in the~
  say Ow! Quit it, $n.
~
|
