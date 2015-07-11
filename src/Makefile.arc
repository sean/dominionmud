# CircleMUD Makefile/arc - manually created (G. Duncan 13 June 98)
#

# C compiler to use
CC = gcc
LINK = drlink

# Any special flags you want to pass to the compiler
MYFLAGS = -O2 -Iunix:Sockets.Include -Irun:
LIBS = 	unix:Sockets.Libs.o.socklib unix:Sockets.Libs.o.inetlib \
	gcc:o.libgcc unix:o.UnixLib -rescan

#flags for profiling (see hacker.doc for more information)
PROFILE = 

##############################################################################
# Do Not Modify Anything Below This Line (unless you know what you're doing) #
##############################################################################

BINDIR = ^.bin

CFLAGS = $(MYFLAGS) $(PROFILE)

OBJFILES = o.comm act.o.comm act.o.informative act.o.movement act.o.item \
	act.o.offensive act.o.other act.o.social act.o.wizard o.ban o.boards \
	o.castle o.race o.config o.constant o.db o.fight o.graph o.handler \
	o.house o.interpre o.limits o.magic o.mail o.mobact o.modify \
	o.objsave o.olc o.random o.shop o.spec_ass o.spec_pro \
	o.spell_pa o.spells o.utils o.weather o.alias

default:	all

all:	$(BINDIR).circle

$(BINDIR).circle: $(OBJFILES)
	$(LINK) -o $(BINDIR).circle $(PROFILE) $(OBJFILES) $(LIBS)

clean:
	wipe o.* ~V~CF

# Dependencies for the object files (automagically generated with
# gcc -MM)

o.alias: c.alias h.conf h.sysdep h.structs h.utils h.interpre h.db
        $(CC) -c $(CFLAGS) c.alias -o o.alias
act.o.comm: act.c.comm h.conf h.sysdep h.structs \
  h.utils h.comm h.interpre h.handler \
  h.db h.screen
	$(CC) -c $(CFLAGS) act.c.comm -o act.o.comm
act.o.informative: act.c.informative h.conf h.sysdep \
  h.structs h.utils h.comm h.interpre \
  h.handler h.db h.spells h.screen h.constant
	$(CC) -c $(CFLAGS) act.c.informative -o act.o.informative
act.o.item: act.c.item h.conf h.sysdep h.structs \
  h.utils h.comm h.interpre h.handler \
  h.db h.spells
	$(CC) -c $(CFLAGS) act.c.item -o act.o.item
act.o.movement: act.c.movement h.conf h.sysdep \
  h.structs h.utils h.comm h.interpre \
  h.handler h.db h.spells h.house h.constant
	$(CC) -c $(CFLAGS) act.c.movement -o act.o.movement
act.o.offensive: act.c.offensive h.conf h.sysdep \
  h.structs h.utils h.comm h.interpre \
  h.handler h.db h.spells
	$(CC) -c $(CFLAGS) act.c.offensive -o act.o.offensive
act.o.other: act.c.other h.conf h.sysdep h.structs \
  h.utils h.comm h.interpre h.handler \
  h.db h.spells h.screen h.house
	$(CC) -c $(CFLAGS) act.c.other -o act.o.other
act.o.social: act.c.social h.conf h.sysdep h.structs \
  h.utils h.comm h.interpre h.handler \
  h.db h.spells 
	$(CC) -c $(CFLAGS) act.c.social -o act.o.social
act.o.wizard: act.c.wizard h.conf h.sysdep h.structs \
  h.utils h.comm h.interpre h.handler \
  h.db h.spells h.house h.screen h.constant
	$(CC) -c $(CFLAGS) act.c.wizard -o act.o.wizard
o.ban: c.ban h.conf h.sysdep h.structs h.utils h.comm h.interpre h.handler h.db
	$(CC) -c $(CFLAGS) c.ban
o.boards: c.boards h.conf h.sysdep h.structs h.utils h.comm h.db h.boards \
  h.interpre h.handler
	$(CC) -c $(CFLAGS) c.boards
o.castle: c.castle h.conf h.sysdep h.structs h.utils h.comm h.interpre \
  h.handler h.db h.spells
	$(CC) -c $(CFLAGS) c.castle
o.race: c.race h.conf h.sysdep h.structs h.db h.utils h.spells h.interpre
	$(CC) -c $(CFLAGS) c.race
o.comm: c.comm h.conf h.sysdep h.structs h.utils h.comm h.interpre h.handler \
  h.db h.house
	$(CC) -c $(CFLAGS) c.comm
o.config: c.config h.conf h.sysdep h.structs
	$(CC) -c $(CFLAGS) c.config
o.constant: c.constant h.conf h.sysdep h.structs
	$(CC) -c $(CFLAGS) c.constant
o.db: c.db h.conf h.sysdep h.structs h.utils h.db h.comm h.handler h.spells h.mail \
  h.interpre h.house
	$(CC) -c $(CFLAGS) c.db
o.fight: c.fight h.conf h.sysdep h.structs h.utils h.comm h.handler h.interpre \
  h.db h.spells h.screen
	$(CC) -c $(CFLAGS) c.fight
o.graph: c.graph h.conf h.sysdep h.structs h.utils h.comm h.interpre h.handler \
  h.db h.spells
	$(CC) -c $(CFLAGS) c.graph
o.handler: c.handler h.conf h.sysdep h.structs h.utils h.comm h.db h.handler \
  h.interpre h.spells
	$(CC) -c $(CFLAGS) c.handler
o.house: c.house h.conf h.sysdep h.structs h.comm h.handler h.db h.interpre \
  h.utils h.house h.constant
	$(CC) -c $(CFLAGS) c.house
o.interpre: c.interpre h.conf h.sysdep h.structs h.comm h.interpre h.db \
  h.utils h.spells h.handler h.mail h.screen
	$(CC) -c $(CFLAGS) c.interpre
o.limits: c.limits h.conf h.sysdep h.structs h.utils h.spells h.comm h.db \
  h.handler
	$(CC) -c $(CFLAGS) c.limits
o.magic: c.magic h.conf h.sysdep h.structs h.utils h.comm h.spells h.handler h.db
	$(CC) -c $(CFLAGS) c.magic
o.mail: c.mail h.conf h.sysdep h.structs h.utils h.comm h.db h.interpre \
  h.handler h.mail
	$(CC) -c $(CFLAGS) c.mail
o.mobact: c.mobact h.conf h.sysdep h.structs h.utils h.db h.comm h.interpre \
  h.handler h.spells
	$(CC) -c $(CFLAGS) c.mobact
o.modify: c.modify h.conf h.sysdep h.structs h.utils h.interpre h.handler h.db \
  h.comm h.spells h.mail h.boards
	$(CC) -c $(CFLAGS) c.modify
o.objsave: c.objsave h.conf h.sysdep h.structs h.comm h.handler h.db \
  h.interpre h.utils h.spells
	$(CC) -c $(CFLAGS) c.objsave
o.olc: c.olc h.conf h.sysdep h.structs h.utils h.comm h.interpre h.handler h.db \
  h.olc
	$(CC) -c $(CFLAGS) c.olc
o.random: c.random h.utils
	$(CC) -c $(CFLAGS) c.random
o.shop: c.shop h.conf h.sysdep h.structs h.comm h.handler h.db h.interpre \
  h.utils h.shop
	$(CC) -c $(CFLAGS) c.shop
o.spec_ass: c.spec_ass h.conf h.sysdep h.structs h.db h.interpre \
  h.utils
	$(CC) -c $(CFLAGS) c.spec_ass
o.spec_pro: c.spec_pro h.conf h.sysdep h.structs h.utils h.comm \
  h.interpre h.handler h.db h.spells
	$(CC) -c $(CFLAGS) c.spec_pro
o.spell_pa: c.spell_pa h.conf h.sysdep h.structs h.utils h.interpre \
  h.spells h.handler h.comm h.db
	$(CC) -c $(CFLAGS) c.spell_pa
o.spells: c.spells h.conf h.sysdep h.structs h.utils h.comm h.spells h.handler \
  h.db h.constant
	$(CC) -c $(CFLAGS) c.spells
o.utils: c.utils h.conf h.sysdep h.structs h.utils h.comm h.screen h.spells \
  h.handler h.db
	$(CC) -c $(CFLAGS) c.utils
o.weather: c.weather h.conf h.sysdep h.structs h.utils h.comm h.handler \
  h.interpre h.db
	$(CC) -c $(CFLAGS) c.weather
