# 
# DominionMUD Makefile
#

default: all

all:
	cd src && $(MAKE)

clean:
	cd src && $(MAKE) clean
