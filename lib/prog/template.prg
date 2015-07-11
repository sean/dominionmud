>greet_prog 30~
 mpechoat $n $I looks up from his paperwork and smiles at you.
 mpechoaround $n $I looks up from his paperwork and smiles at $n.
 if level($n) <= 2
   say How may I help you this fine day?
   say If you need any help, just ask.
 else 
   say What can I do for you today, $n?
 endif
~
>speech_prog help~
 say Well, I don't have much time but I'll answer to 
 say the best of my knowledge.
 smile
 say What do you wish to learn more about?  Trainers, Levels?
~
>speech_prog trainer train practice~
 say You must first practice one of the five basic 
 say spheres before you may acquire any skills.  Once 
 say you have done this, you must seek out the proper
 say trainer for the particular sub-sphere you are
 say interested in, so that he or she may train you.
 say There are several spheres: Thaumaturgy, Theomachy,
 say Thievery, Combat Proficiency and General Knowledge.
~
>speech_prog combat~
 say The captain of the guard, here in Elrhinn is wise 
 say in the ways of combat.  Seek him in the town barracks
 say where he works.
~
>speech_prog thaum thaumaturgy~
 say Ah, interested in the arcane arts, well then you would
 say be wise to seek out the great wizard in the Tower of Magus.
~
>speech_prog general knowledge~
 say Everyone needs to know the basics, don't they?
 say The Sage of Elrhinn is very wise and knows much
 say about everything, surely he can help you.
~
>speech_prog theomachy theo~
 say Theocracy, my personal speciality.
 smile $n
 say of course to get the proper training you should
 say seek out the priest in the town church, he would
 say be much more of an aid than I would.
~
>speech_prog thievery~
 eyebrow
 mpechoat $n $I gives you a suspicious glance.
 mpechoaround $n $I gives $n a suspicious look.
 say So, you're interested in the ways of murder and theft?
 say We have no use for the likes of you in Elrhinn.
 tell $n If you must know of their trecherous ways speak
 tell $n with the beggars, they should know more. But 
 tell $n considered yourself warned about the danger
 tell $n that lays before you if you choose that route.
~
>speech_prog level~
 say Just come back here when you feel you have gained new
 say insight into life and I will help guide your further
 say study.
~
>rand_prog 20~
  mpecho $I looks up from his paperwork.
  say just ask for help if you need it.
  smile
  mpecho $I turns his attention back to his work.
~
|
