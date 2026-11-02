>greet_prog 60~
if ispc($n)
  say Welcome to the Valley of Fate, traveller.  Walk softly, and harm nothing that grows here.
endif
~
>speech_prog fate fates valley~
if ispc($n)
  say Three sisters keep this valley.  The Spinner begins every thread, the Measurer decides its length, and the Cutter ends it.
  say Their roads meet north of the pool, at the heart of the valley.  Many come hoping to change their fate.  Few understand what they find.
endif
~
>speech_prog change~
if ispc($n)
  say Change it?  You might as well ask the brook to run uphill.  But the sisters do not weave the cloth, they only tend the threads.  Ask the eldest of them who weaves.
endif
~
>rand_prog 3~
mpecho Corrin shades his eyes and gazes out across the valley, as he has done for a thousand years.
~
|
