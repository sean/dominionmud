>greet_prog 80~
if ispc($n)
  mpechoat $n Krommyon looks you up and down and spits a splinter of bone at your feet.
  mpechoaround $n Krommyon looks $n up and down and spits a splinter of bone on the floor.
  say A visitor.  Have the book-readers sent another envoy?  I ate the last one.
endif
~
>speech_prog byterian byterians skeiron kerkyon~
if ispc($n)
  say Book-readers and candle-sniffers.  Karras gave us axes, not quills.  One day I will take the white tower and use their library for kindling.
endif
~
>fight_prog 20~
say Now THIS is a conversation!
~
|
