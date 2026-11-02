>greet_prog 25~
if ispc($n)
  mpechoat $n As you pass the black monolith, words rise to the surface of the stone like ripples on dark water: "Two pans, one beam, no heart; I weigh the guilty and the true alike.  Name me, and Sand's door will open."
endif
~
>speech_prog scales~
if ispc($n)
  mpechoat $n The black stone ripples like water.  Before you can step back, it draws you in...
  mpechoaround $n The black monolith ripples like water and swallows $n whole.
  mptransfer $n 1296
endif
~
|
