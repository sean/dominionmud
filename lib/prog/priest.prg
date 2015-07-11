>greet_prog 25~
 if isgood(ch)
   say Welcome, $n, the light is strong within you.
 else
   say Welcome, $n.
 endif
~
>rand_prog 15~
 say If you seek help, just ask, friend.
~
>rand_prog 15~
 say Would you give alms for the poor, friend?
 say I will reward you for your graciousness.
~
>speech_prog reward~
 say Well, if you give say, 20000 coins, I will heal your wounds.
 say If 10000, I will cure any critical wounds you have.
 say For 5000 gold, I will gladly cure you of your ills.
 say For a blessing, a donation of 500 coins will be sufficient.
~
>bribe_prog 20000~
  mpcast 'heal' $n
  say A gift for you, in return for your gift to us.
~
>bribe_prog 10000~
  mpcast 'cure critic' $n
  say Your soul is truly in the hands of our lord.
~
>bribe_prog 5000~
  mpcast 'cure poison' $n
  mpcast 'cure blind' $n
  mpcast 'cure disease' $n
  say May all those you meet become as gracious as yourself.
~
>bribe_prog 500~
  mpcast 'bless' $n
  say May Glasgian bless you, and keep your soul in everlasting light.
~
>speech_prog help~
   say What do you seek help with?  Sphere's or General help?
~
>speech_prog sphere spheres~
   say A sphere is a blessing which the gods have given us.  There
   say five main spheres from which we may all learn from - but
   say Theomachy is what I know best.
   smile
~
>speech_prog theomachy theo~
   say Ahh...you seek to know more about Theomachy, you are wise, $n.
   say Well, if you practice Theomachy to its fullest degree, you will 
   say become aware of The Lord's blessing - he has given you 
   say sub-sphere's in which you may train further and learn certain 
   say prayers which will aid you in your quests.  There are 5
   say different sub-spheres which you may practice - Cures,
   say Afflictions, Acts of the Deities, Hand of the Deities, and
   say Faith.  
~
>speech_prog acts of the deities~
   say One who has been enlightened in the sphere, Acts of the Deities,
   say can call forth powers from the gods to aid them.  You must
   say seek out the Great Eternity Dragon in the Dragonspire Mtns.
   say to learn more.
~
>speech_prog hand of the deities~
   say This is a powerful field for the righteous Clerics who spread
   say the word of their Lord across the lands.  To learn more
   say into this field, seek ye the Great Heirophant of Druids
   say in the Druid's Glade south of Darkweir.
~
>speech_prog cure cures~
  say This is the most common sphere learned by priests.  It involves
  say the mastery of the body and mind and controlling them to
  say serve your will.  My acolyte in the the chapel north of
  say will aid you in learning more in this sphere.
~
>speech_prog afflict afflictions affliction~
  mpechoat $n $I gives you a pitiful look.
  mpechoaround $n $I gives $n a pained look.
  say This cruel sphere is only sought out by the Dark Clerics. 
  say I believe the Black Druid, Drerimin, can help you, heathen.
~
>speech_prog faith~
  say this is a very simple sphere in which you may learn prayers
  say which will help protect you from the evils of the Dominion.
  say at the moment I am unsure whether or not the Gods have bestown
  say this power upon any teacher.
~
|
