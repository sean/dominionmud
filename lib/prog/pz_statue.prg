>greet_prog 30~
if ispc($n)
  mpechoat $n For a moment the statue's marble eyes seem to turn toward you, searching your hands for something.
endif
~
>give_prog fragment scroll torn~
if ispc($n)
  mpjunk fragment
  mpechoat $n You fit the fragment against the torn scroll in the statue's hand.  The two pieces knit together, and light floods the statue's eyes.
  mpechoat $n A clear voice rings out: "At last my oath is whole.  Come, keeper of my words, and see what I guarded."
  mpechoaround $n $n places something in the statue's hand.  Light floods the statue's eyes, and the ground opens beneath $n.
  mptransfer $n 32241
endif
~
|
