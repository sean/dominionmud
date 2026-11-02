>greet_prog 30~
if ispc($n)
  say Welcome, traveller.  Have you looked upon the obelisk at the crossing?  Few can read its runes any longer, but ask me of the runes and I will tell you what they say.
endif
~
>speech_prog runes~
if ispc($n)
  say Ah, the runes.  I am old enough to read them still.
  say They say: "Each spring I am born and each autumn I die, and the wood never mourns me.  Speak my name before this stone, and the heart of the wood will open to you."
  say What the stone wants, I leave to you to guess.
endif
~
|
