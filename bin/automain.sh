#!/bin/sh
#
# CircleMUD 3.0 automaint script
# Originally by Fred C. Merkel
# Copyright (c) 1993 The Trustees of The Johns Hopkins University
# All Rights Reserved
# See license.doc for more information

set DATE=(`date`);
set ETC=../lib/etc
set PURGEPLAY=../../bin/purgeplay

cd $ETC
echo "automaint cleaning pfile: $DATE" >> DELETED

$PURGEPLAY players >> DELETED 
 
mv players players.old
mv players.new players
