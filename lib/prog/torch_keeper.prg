>greet_prog 60~
if ispc($n)
  mpechoat $n The keeper of the Torch turns its hooded head toward you and raises its green flame higher.
  mpechoat $n A dry voice whispers: "Another one through the mirror.  Walk carefully.  The Overseer's eyes are everywhere in this mist."
endif
~
>rand_prog 4~
mpecho The keeper's torch gutters green, and for a moment the impaled shapes on the posts seem to turn their heads.
~
>speech_prog overseer~
if ispc($n)
  say The Overseer was mighty once, and good, they say.  Then he changed, and this land changed with him.  The crows remember.
endif
~
>speech_prog mirror~
if ispc($n)
  say The mirror opens only one way.  The road home lies to the northeast, if you live long enough to walk it.
endif
~
|
