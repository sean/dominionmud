>greet_prog 30~
if ispc($n)
  mpechoat $n The markings on the archway pulse slowly, like a heartbeat, and the bloody verse on the stones seems to wait for someone to finish it aloud.
endif
~
>speech_prog p never die~
if ispc($n)
  mpechoat $n The markings on the archway flare red.  The stones beneath you groan apart, and you tumble down into darkness...
  mpechoaround $n The markings on the archway flare red, and the stones swallow $n.
  mptransfer $n 1898
endif
~
|
