>greet_prog 40~
if ispc($n)
  mpechoat $n The candles in the skull's eye sockets flare as you enter, and a dry whisper fills your mind: "Five sayings in these halls, one name beneath them all.  Speak it, and the Mute People will receive you."
endif
~
>speech_prog death~
if ispc($n)
  mpechoat $n The skull's jaw creaks open and the black candles gutter out.  The floor before the altar sinks away beneath your feet...
  mpechoaround $n The floor before the altar swallows $n, and the candles flicker back to life.
  mptransfer $n 10463
endif
~
|
