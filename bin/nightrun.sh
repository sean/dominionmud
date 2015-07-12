#!/bin/csh
#
# CircleMUD 2.0 autorun script
# Originally by Fred C. Merkel
# Copyright (c) 1993 The Trustees of The Johns Hopkins University
# All Rights Reserved
# See LICENSE.md for more information

# If .fastboot exists, the script will sleep for only 5 seconds between reboot
# attempts.  If .killscript exists, the script commit suicide (and remove
# .killscript).  If pause exists, the script will repeatedly sleep for
# 60 seconds and will not restart the mud until pause is removed.

set PORT=8400
set FLAGS='-m'

set SERVER=./bin/dominion
set LOGDIR=./lib/logs
set SYSLOG="$LOGDIR/syslog"

while (1)

  # ensure we are in the correct directory, unfortunately we can only
  # look upwards.
  set DIR=`pwd`
  while ( `basename $DIR` != "DominionMUD" )
    cd ..
    set DIR=`pwd`
  end

  rm -f $SYSLOG
  rm -f $LOGDIR/usage
  set DATE = (`date`);
  echo "autoscript starting game $DATE" >> $SYSLOG

  $SERVER $PORT >>& $SYSLOG

  # At this point we should check for a core and if so copy the syslog
  # and the core file to the debug directory.
  if ( `ls lib | grep core` == "core" ) then
    echo "Moving core file to debug ..."
    mv lib/core debug/core.$DATE
    cp $SYSLOG debug/"$SYSLOG".$DATE
  endif

  fgrep "self-delete" $SYSLOG >> $LOGDIR/delete
  fgrep "death trap" $SYSLOG >> $LOGDIR/dts
  fgrep "killed" $SYSLOG >> $LOGDIR/rip
  fgrep "Running" $SYSLOG >> $LOGDIR/restarts
  fgrep "advanced" $SYSLOG >> $LOGDIR/levels
  fgrep "equipment lost" $SYSLOG >> $LOGDIR/rentgone
  fgrep "usage" $SYSLOG >> $LOGDIR/usage
  fgrep "new player" $SYSLOG >> $LOGDIR/newplayers
  fgrep "SYSERR" $SYSLOG >> $LOGDIR/errors
  fgrep "(GC)" $SYSLOG >> $LOGDIR/godcmds
  fgrep "Bad PW" $SYSLOG >> $LOGDIR/badpws
  fgrep "OLC" $SYSLOG >> $LOGDIR/olccmds

  rm -f $LOGDIR/syslog.1
  mv -f $LOGDIR/syslog.2 $LOGDIR/syslog.1
  mv -f $LOGDIR/syslog.3 $LOGDIR/syslog.2
  mv -f $LOGDIR/syslog.4 $LOGDIR/syslog.3
  mv -f $LOGDIR/syslog.5 $LOGDIR/syslog.4
  mv -f $LOGDIR/syslog.6 $LOGDIR/syslog.5
  mv -f syslog       $LOGDIR/syslog.6
  touch $SYSLOG
  chmod uog+rw $SYSLOG

  if (! -r .fastboot) then
    sleep 10
  else
    rm -f .fastboot
    sleep 5
  endif

  if (-r .killscript) then
    set DATE = (`date`);
    echo "autoscript killed $DATE"  >> $SYSLOG
    rm -f .killscript
    exit
  endif 

  while (-r pause) 
    sleep 60
  end

end
