COMPILER = gcc
CCFLAGS = -Wall -g
LINKFLAGS = -lcurses -lpthread -lrt

all: main

main: main.c
	${COMPILER} ${CCFLAGS} ${LINKFLAGS} -o pdp main.c

clean:
	@rm -f pdp *.o
