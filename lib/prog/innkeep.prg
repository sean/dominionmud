>greet_prog 100~
 if isimmort ($n)
   mpechoat $n $I looks up from his work and quickly kneels before you. 
   mpechoaround $n $I looks up and quickly kneels before $n.
 else 
   mpecho $I looks up from their book and scowls.
   say Well...what can I do for you?
 endif
~
>rand_prog 10~
 mpecho $I sighs and turns the page in his book.
~
>rand_prog 10~
 mpecho $I sets down his book and mumbles something under his breath.
~
>rand_prog 10~
 mpecho $I turns a page in his book.
~
>rand_prog 5~
 mpechoat $I sets down his book and beckons to you.
 mpechoaround $I sets down his book and beckons $n.
 whisper $n You know...I hear the Template has a new 'mistress'...
 whisper $n but you didn't hear it from me.
 mpecho $I glances about the room suspiciously.
 mpecho $I picks up his book and begins to read again.
~
>act_prog p has arrived riding on~
 say $n, get that god damned thing out of my inn!
 mpforce $n down
~
|
