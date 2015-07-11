>rand_prog 10~
   say Lemme know if you want to play or if you need help.
~
>rand_prog 25~
   mpecho $I counts his gold.
   mpat 0 drop all.coin
   mpat 0 get all.coin
~
>speech_prog help~
   say Currently, there is only one game that I have set up.
   say It's called The Gargoyle's Gambit.  You can either 
   say make a killing in cash or you'll walk away a poorer,
   say stupider person.  If you want help on Garoyle's Gambit 
   say just say, 'gambit'.
~
>speech_prog gambit~
   say Ok, the game works like this:  You GIVE me a 
   say predetermined amount of cash as it says on the 
   say SIGN...I shuffle the deck and then you pick one 
   say of the 20 cards.  There are only two different 
   say kinds of cards in this deck.  If it comes up 
   say "Treasure Chest", you win.  If it comes up with 
   say the Gargoyle you lose. 
   say Pretty simple, eh?  Want to try your luck?
~
>bribe_prog 40000~
   if rand(23)
     mpecho $I shuffles the deck and then cuts it down the middle.
     mpechoat $n You pick a card out of the deck....
     mpechoaround $n $n picks a card from the deck.
     mpechoat $n You pick a Treasure Card from the deck!
     give 49999 coin $n
   else
     mpecho $I shuffles the deck and then cuts it down the middle.
     mpechoat $n You pick a card out of the deck....
     mpechoaround $n $n picks a card from the deck.
     mpechoat $n You pick a Gargoyle Card from the deck!
     say Sorry, Please play again.
   endif
~   
>bribe_prog 30000~
   if rand(25)
     mpecho $I shuffles the deck and then cuts it down the middle.
     mpechoat $n You pick a card out of the deck....
     mpechoaround $n $n picks a card from the deck.
     mpechoat $n You pick a Treasure Card from the deck!
     give 39999 coin $n
   else
     mpecho $I shuffles the deck and then cuts it down the middle.
     mpechoat $n You pick a card out of the deck....
     mpechoaround $n $n picks a card from the deck.
     mpechoat $n You pick a Gargoyle Card from the deck!
     say Sorry, Please play again.
   endif
~   
>bribe_prog 25000~
 if rand(26)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 29999 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 20000~
 if rand(27)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 24999 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 15000~
 if rand(28)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 19999 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 10000~
 if rand(30)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 14999 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 7500~
 if rand(31)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 9999 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 5000~
 if rand(35)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 7499 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 3300~
 if rand(35)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 4999 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 2000~
 if rand(30)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 3299 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 1000~
 if rand(30)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 1999 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 550~
 if rand(30)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 999 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 200~
 if rand(30)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 549 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 100~
 if rand(30)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 199 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 50~
 if rand(40)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 99 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 25~
 if rand(50)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 49 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
>bribe_prog 5~
 if rand(55)
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Treasure Card from the deck!
   give 9 coin $n
 else
   mpecho $I shuffles the deck and then cuts it down the middle.
   mpechoat $n You pick a card out of the deck....
   mpechoaround $n $n picks a card from the deck.
   mpechoat $n You pick a Gargoyle Card from the deck!
   say Sorry, Please play again.
 endif
~   
|
