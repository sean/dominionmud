>fight_prog 100~
   if level($n) >= 20
      if isfight($i)
	say You dare trifle with me?!?
	mpecho The old woman changes shape before your eyes.
	mpecho Suddenly, you are staring at a fierce, black dragon!
	mpmload 8019
        mpoload 8018
	give cloak lich
        mpgoto 1
      endif
   endif
~
|
