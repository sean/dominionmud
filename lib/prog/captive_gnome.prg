>greet_prog 50~
if ispc($n)
  mpechoat $n Pibb looks up from his spinning pebble, and hope flickers in his eyes.
  mpechoaround $n Pibb looks up at $n hopefully.
  say A visitor!  Not a kobold!  You do not happen to have a file on you?  No?  Pity.
endif
~
>speech_prog treasure hoard rubble~
if ispc($n)
  say Oh, the hoard.  Everything they ever stole from Gnishmire, my tools included.
  say The King had it walled up behind the rubble at the western end of the old river.  He thinks nobody knows.
  say But I hear the diggers going in and out.  They say his name to the stones, and the stones let them through.  "Vrask!  Vrask!"  All day long, as if the rocks cared.
endif
~
>speech_prog war invasion plan~
if ispc($n)
  say The trolls are coming from the east, once the bridge over the chasm is finished.  Then the kobolds climb the staircase and pour out into the mines and on to Gnishmire.
  say Somebody ought to warn the Watchers.  Somebody who is not chained to a wall.
endif
~
>rand_prog 4~
mpecho Pibb spins his little wire top and watches it wobble to a stop.
~
|
