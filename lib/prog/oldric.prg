>greet_prog 100~
if ispc($n)
  if level($n) <= 5
    say Another young one!  Good.  If you want to learn how to stay alive out there, say 'trial', and I will send you into the Proving Halls.
  else
    say You have the look of someone who has already learned the hard way.  Still, the Proving Halls are there if you want them.  Just say 'trial'.
  endif
endif
~
>speech_prog trial ready~
if ispc($n)
  say Very well.  Read the walls, mind your lessons, and come out the other side.
  mpechoat $n Master Oldric raps his cane against the shimmering bricks, and the north wall seems to swallow you...
  mpechoaround $n Master Oldric raps his cane against the bricks, and $n steps into the shimmer and is gone.
  mptransfer $n 5802
endif
~
>speech_prog academy~
if ispc($n)
  say The Academy trained half the heroes on those walls, once.  Now there is only me, and the Halls.  They still work.  Mostly.
endif
~
>rand_prog 3~
mpecho Master Oldric taps his wooden leg with his cane and stares at the paintings of old friends.
~
|
