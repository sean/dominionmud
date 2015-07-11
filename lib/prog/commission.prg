>rand_prog 10~
  say So much to do...so little time...
  sigh
~
>rand_prog 10~
  say Now where in the Heavens is that damned servant?
  mpecho The commissioner begins to fret and swear about his tardy servant.
~
>greet_prog 75~
  if level($n) >= 5
    say Here for a commission today, $n?
  else 
    say Just ask for help if you need any.
  endif
~
>speech_prog help~
  say Well, I'm in the business of giving commissions
  say for those who help with what needs to be done.
  say The commission could be for something as simple
  say as gathering valuable metals, or it could be
  say something dangerous like hunting a known criminal.
~
>speech_prog commissions commission~
   if level($n) <= 5
     say Well, let me see what I got here...
     mpecho The commissioner begins to rumage through his papers.
     say Elrhinn has need for dark-blue wood...
     say most commonly found in Darkweir Hallows.  
     say We will pay a fine sum for whatever you retrieve.  
     say Also you should stop in periodically,
     say to see if there are any quests in progress.
     smile
   else
     say Well, I can't give a full explanation of each...
     say but here's what we will pay for
     say in return for the retrieval:
     mpecho 1. Emerald Gems from Serdim-ir-Lesk    (900 gp)
     mpecho 2. 'Krisahla' from Bryn Mawr           (4,000 gp)
     mpecho 3. Any raw mithril (no weapons)        (9,000 gp)
     mpecho 4. Any raw adamantite (no weapons)     (11,000 gp)
     mpecho $I pauses a moment.
     say If you find of the above, bring it here and
     say give it to me, each item one at a time,
     say and I will pay you.  Good day.
    endif
~
>speech_prog darkweir~
   say Darkweir is an ancient forest, south of the city.  
   say Not much can be told about it...They say it is the
   say product of magic rather than the grace of
   say The Kinnslayer...but who knows?
   say No one around here has the time to be a historian.
   mpecho He finishes, 'You can also talk to Jynnah for more information.'
~
>speech_prog jynnah~
   say Jynnah is the town's baker.
   say Look for her near the inn on Dalkyst Road.
~
>give_prog wood dark dark-blue blue~
     say Ah! Just what we needed.
     mpecho $I unlocks and opens a safe underneath his desk.
     say And in return for your deed here is this:
     give 90 coin $n
     mpecho $I closes and locks the safe.
     mpjunk wood dark darkblue-blue blue
~
>give_prog gem emerald~
     say Perfect!
     mpecho $I unlocks and opens a safe underneath his desk.
     say I hope that this will be a good compensation fee.
     give 900 coin $n
     mpjunk gem emerald
~
>give_prog bar krisahla~
     say Your reward for this will be sufficient I hope.
     mpecho $I unlocks and opens a safe beneath his desk.
     give 4000 coin $n
     mpjunk bar krisahla
~
>give_prog bar mithril~
     say You will surely be paid in full for this.
     mpecho $I unlocks and opens a safe beneath his desk.
     give 9000 coin $n
     mpjunk bar mithril
~
>give_prog bar adamantite~
     say What a great find and the reward will be equally great!
     em unlocks and opens a safe beneath his desk.
     give 11000 coin $n
     say Thank you very much, $n.
     mpjunk bar adamantite
~
>give_prog scale green~
     say You have done a great deed, $n!
     say For this you will be rewarded well.
     give 10000 coin $n
     say Thank you again for ridding us of that horrible beast.
     mpat 1 drop scale
~
>rand_prog 10~
     if level($n) = 9
       say By the way, I have a SPECIAL quest if you are worthy enough.
     endif
~
>speech_prog special quest~
     say Well, there is some strange beast which has 
     say wandered out from the Light-Knows-Where
     say and has been roaming the Pelarinne Forest.
     sigh
     say The thing tore two of Elrhinn's finest soldiers
     say limb from limb and devoured most of their corpses!
     shake
     say I will pay you dearly if you kill this beast,
     say and bring back one of its scales as proof.
~
>speech_prog pelarinne~
     peer $n
     say Just go out the Western Gate and walk down the road
     say a ways.
~
|
