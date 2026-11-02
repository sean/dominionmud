>greet_prog 40~
if ispc($n)
  mpechoat $n The guardsman looks you over carefully, then touches the golden sun on his breastplate in greeting.
  mpechoaround $n The guardsman looks $n over carefully, then touches the golden sun on his breastplate.
endif
~
>rand_prog 3~
mpecho The guardsman straightens as the fire in the hearth pops, then settles back into his watch.
~
>speech_prog law laws~
if ispc($n)
  say The law is the wall that keeps the darkness out.  The Watchers stand upon that wall.
endif
~
|
