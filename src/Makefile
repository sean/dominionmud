# Generated automatically from Makefile.in by configure.
# CircleMUD Makefile.in - Makefile template used by 'configure'
#

# C compiler to use
CC = gcc

# Path to cxref utility
CXREF = cxref

# Any special flags you want to pass to the compiler
MYFLAGS = -Wall

#flags for profiling (see hacker.doc for more information)
PROFILE = 

##############################################################################
# Do Not Modify Anything Below This Line (unless you know what you're doing) #
##############################################################################

BINDIR = ../bin

CFLAGS = -g -O2 $(MYFLAGS) $(PROFILE)

LIBS =   

OBJFILES = act_comm.o act_info.o act_item.o act_move.o \
	act_off.o act_othr.o act_socl.o act_wizd.o alias.o ban.o \
	boards.o castle.o race.o comm.o config.o constant.o db.o fight.o \
	graph.o handler.o house.o interpre.o limits.o magic.o mail.o \
	mobact.o modify.o objsave.o olc.o random.o shop.o spec_ass.o \
	spec_pro.o spell_pa.o spells.o utils.o weather.o \
	bsd-snprintf.o mobcmd.o mobprog.o

CXREF_FILES = act_comm.c act_info.c act_item.c act_move.c \
	act_off.c act_othr.c act_socl.c act_wizd.c alias.c ban.c \
	boards.c castle.c race.c comm.c config.c constant.c db.c fight.c \
	graph.c handler.c house.c interpre.c limits.c magic.c mail.c \
	mobact.c modify.c objsave.c olc.c random.c shop.c spec_ass.c\
	spec_pro.c spell_pa.c spells.c utils.c weather.c \
	bsd-snprintf.c mobcmd.c mobprog.c

default: all

all: .accepted
	$(MAKE) $(BINDIR)/circle
	$(MAKE) utils

.accepted:
	@./licheck more

utils: .accepted
	(cd util; $(MAKE) all)
circle:
	$(MAKE) $(BINDIR)/circle

$(BINDIR)/circle : $(OBJFILES)
	$(CC) -o $(BINDIR)/circle $(PROFILE) $(OBJFILES) $(LIBS)

clean:
	rm -f *.o
ref:
#
# Create the cross reference files
# Note, this is not meant to be used unless you've installed cxref...
#
	@for file in $(CXREF_FILES) ; do \
	  echo Cross referencing $$file ; \
	  $(CXREF) -D__CXREF__ -xref -Odoc -Ncircle $$file ; \
	done
#
# Create the source files using cxref
#
	@for file in $(CXREF_FILES) ; do \
	   echo Documenting $$file ; \
	   ( cd . ; $(CXREF) -D__CXREF__ -warn-xref -xref -Odoc -Ncircle -html $$file ) ; \
	   rm -f $(DOCS) ; \
	done
#
# Create the index using cxref
#
	@echo Indexing
	@( cd . ; $(CXREF) -D__CXREF__ -index-all -Odoc -Ncircle -html )
	@rm -f $(DOCS)
#
# Make html files for the .h files
#
	@echo Creating .h.html files...
	@for file in *.h ; do \
	  echo $$file ; \
	  cat htmlh-head $$file htmlh-tail > doc/$$file.html ; \
	done
# Copy over to the html directory
	#cp doc/*.html $(HOME)/www/cxref
	#chmod 644 $(HOME)/www/cxref/*.html

# Dependencies for the object files (automagically generated with
# gcc -MM)

act_comm.o: act_comm.c conf.h sysdep.h structs.h utils.h comm.h interpre.h \
  handler.h db.h screen.h
	$(CC) -c $(CFLAGS) act_comm.c
act_info.o: act_info.c conf.h sysdep.h structs.h utils.h comm.h \
  interpre.h handler.h db.h spells.h screen.h constant.h
	$(CC) -c $(CFLAGS) act_info.c
act_item.o: act_item.c conf.h sysdep.h structs.h utils.h comm.h interpre.h \
  handler.h db.h spells.h constant.h
	$(CC) -c $(CFLAGS) act_item.c
act_move.o: act_move.c conf.h sysdep.h structs.h utils.h comm.h \
  interpre.h handler.h db.h spells.h house.h constant.h
	$(CC) -c $(CFLAGS) act_move.c
act_off.o: act_off.c conf.h sysdep.h structs.h utils.h comm.h \
  interpre.h handler.h db.h spells.h
	$(CC) -c $(CFLAGS) act_off.c
act_othr.o: act_othr.c conf.h sysdep.h structs.h utils.h comm.h interpre.h \
  handler.h db.h spells.h screen.h house.h constant.h
	$(CC) -c $(CFLAGS) act_othr.c
act_socl.o: act_socl.c conf.h sysdep.h structs.h utils.h comm.h \
  interpre.h handler.h db.h spells.h
	$(CC) -c $(CFLAGS) act_socl.c
act_wizd.o: act_wizd.c conf.h sysdep.h structs.h utils.h comm.h \
  interpre.h handler.h db.h spells.h house.h screen.h constant.h
	$(CC) -c $(CFLAGS) act_wizd.c
alias.o: alias.c conf.h sysdep.h structs.h utils.h interpre.h db.h
	$(CC) -c $(CFLAGS) alias.c
ban.o: ban.c conf.h sysdep.h structs.h utils.h comm.h interpre.h handler.h db.h
	$(CC) -c $(CFLAGS) ban.c
boards.o: boards.c conf.h sysdep.h structs.h utils.h comm.h db.h boards.h \
  interpre.h handler.h
	$(CC) -c $(CFLAGS) boards.c
castle.o: castle.c conf.h sysdep.h structs.h utils.h comm.h interpre.h \
  handler.h db.h spells.h
	$(CC) -c $(CFLAGS) castle.c
race.o: race.c conf.h sysdep.h structs.h db.h utils.h spells.h interpre.h \
  constant.h
	$(CC) -c $(CFLAGS) race.c
comm.o: comm.c conf.h sysdep.h structs.h utils.h comm.h interpre.h handler.h \
  db.h house.h
	$(CC) -c $(CFLAGS) comm.c
config.o: config.c conf.h sysdep.h structs.h interpre.h
	$(CC) -c $(CFLAGS) config.c
constant.o: constant.c conf.h sysdep.h structs.h interpre.h
	$(CC) -c $(CFLAGS) constant.c
db.o: db.c conf.h sysdep.h structs.h utils.h db.h comm.h handler.h spells.h mail.h \
  interpre.h house.h constant.h
	$(CC) -c $(CFLAGS) db.c
fight.o: fight.c conf.h sysdep.h structs.h utils.h comm.h handler.h interpre.h \
  db.h spells.h screen.h constant.h
	$(CC) -c $(CFLAGS) fight.c
graph.o: graph.c conf.h sysdep.h structs.h utils.h comm.h interpre.h handler.h \
  db.h spells.h
	$(CC) -c $(CFLAGS) graph.c
handler.o: handler.c conf.h sysdep.h structs.h utils.h comm.h db.h handler.h \
  interpre.h spells.h
	$(CC) -c $(CFLAGS) handler.c
house.o: house.c conf.h sysdep.h structs.h comm.h handler.h db.h interpre.h \
  utils.h house.h constant.h
	$(CC) -c $(CFLAGS) house.c
interpre.o: interpre.c conf.h sysdep.h structs.h comm.h interpre.h db.h \
  utils.h spells.h handler.h mail.h screen.h
	$(CC) -c $(CFLAGS) interpre.c
limits.o: limits.c conf.h sysdep.h structs.h utils.h spells.h comm.h db.h \
  handler.h
	$(CC) -c $(CFLAGS) limits.c
magic.o: magic.c conf.h sysdep.h structs.h utils.h comm.h spells.h handler.h db.h \
  constant.h
	$(CC) -c $(CFLAGS) magic.c
mail.o: mail.c conf.h sysdep.h structs.h utils.h comm.h db.h interpre.h \
  handler.h mail.h
	$(CC) -c $(CFLAGS) mail.c
mobact.o: mobact.c conf.h sysdep.h structs.h utils.h db.h comm.h interpre.h \
  handler.h spells.h constant.h
	$(CC) -c $(CFLAGS) mobact.c
mobcmd.o: mobcmd.c conf.h sysdep.h structs.h utils.h db.h comm.h interpre.h \
  handler.h spells.h constant.h
	$(CC) -c $(CFLAGS) mobcmd.c
mobprog.o: mobprog.c conf.h sysdep.h structs.h utils.h db.h comm.h interpre.h \
  handler.h spells.h constant.h
	$(CC) -c $(CFLAGS) mobprog.c
modify.o: modify.c conf.h sysdep.h structs.h utils.h interpre.h handler.h db.h \
  comm.h spells.h mail.h boards.h
	$(CC) -c $(CFLAGS) modify.c
objsave.o: objsave.c conf.h sysdep.h structs.h comm.h handler.h db.h \
  interpre.h utils.h spells.h
	$(CC) -c $(CFLAGS) objsave.c
olc.o: olc.c conf.h sysdep.h structs.h utils.h comm.h interpre.h handler.h db.h \
  olc.h
	$(CC) -c $(CFLAGS) olc.c
random.o: random.c utils.h
	$(CC) -c $(CFLAGS) random.c
shop.o: shop.c conf.h sysdep.h structs.h comm.h handler.h db.h interpre.h \
  utils.h shop.h constant.h
	$(CC) -c $(CFLAGS) shop.c
spec_ass.o: spec_ass.c conf.h sysdep.h structs.h db.h interpre.h \
  utils.h
	$(CC) -c $(CFLAGS) spec_ass.c
spec_pro.o: spec_pro.c conf.h sysdep.h structs.h utils.h comm.h \
  interpre.h handler.h db.h spells.h constant.h
	$(CC) -c $(CFLAGS) spec_pro.c
spell_pa.o: spell_pa.c conf.h sysdep.h structs.h utils.h interpre.h \
  spells.h handler.h comm.h db.h
	$(CC) -c $(CFLAGS) spell_pa.c
spells.o: spells.c conf.h sysdep.h structs.h utils.h comm.h spells.h handler.h \
  db.h constant.h interpre.h
	$(CC) -c $(CFLAGS) spells.c
utils.o: utils.c conf.h sysdep.h structs.h utils.h comm.h screen.h spells.h \
  handler.h db.h interpre.h
	$(CC) -c $(CFLAGS) utils.c
weather.o: weather.c conf.h sysdep.h structs.h utils.h comm.h handler.h \
  interpre.h db.h
	$(CC) -c $(CFLAGS) weather.c
bsd-snprintf.o: bsd-snprintf.c conf.h sysdep.h
	$(CC) -c $(CFLAGS) bsd-snprintf.c
