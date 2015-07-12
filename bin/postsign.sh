#!/bin/sh

set PORT=8400
set SIGN=lib/text/sign.txt

if [ `ls | grep lib` != "lib" ]; then
    cd ..
fi;

# launch the sign application
bin/sign $PORT $SIGN
