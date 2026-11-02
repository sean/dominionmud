>greet_prog 100~
if ispc($n)
  mpechoat $n The Cutter looks up at you with her clear gray eyes, and you feel suddenly very small and very mortal.
  say Not yet, dear.  Not yet.  Sit down; you look tired.
endif
~
>speech_prog loom weave weaver weaves change~
if ispc($n)
  say My sisters and I only tend the threads.  We do not weave the cloth.  That is our mother's work: Ananke, who weaves at the great Loom beneath the pool.
  say Speak her name to the pool, if you would see her work.  But do not expect her to unpick a single stitch for you.
endif
~
>speech_prog thread fate death die~
if ispc($n)
  say Everyone comes to me in the end.  It is not cruelty; a thread without an end is only a tangle.
endif
~
|
