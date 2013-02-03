COMPILER = gcc
CCFLAGS = -Wall -g
LINKFLAGS = -lpanel -lcurses -lpthread -lrt

all: main

main: interface.o
	${COMPILER} ${CCFLAGS} ${LINKFLAGS} -o pdp main.c interface.c

interface.o:
	${COMPILER} ${CCFLAGS} -c interface.c


clean:
	@rm -f pdp *.o
