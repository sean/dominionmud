>greet_prog 60~
if ispc($n)
  mpechoat $n King Vrask jabs his scepter in your direction.
  mpechoaround $n King Vrask jabs his scepter in $n's direction.
  say Who let this one in?  Guards!  Somebody!  Anybody!
endif
~
>rand_prog 5~
mpecho King Vrask taps the war plan with his scepter and mutters, "Trolls first.  Then gnomes.  Then everybody."
~
>rand_prog 4~
say Dig faster!  The surface will not conquer itself!
~
>speech_prog troll trolls gorrug~
if ispc($n)
  say The trolls are our friends.  Our big, strong, very hungry friends.  Once the bridge is done, they come, and then nobody laughs at kobolds again.
endif
~
>fight_prog 20~
say You will be the first name on my list of the conquered!
~
|
