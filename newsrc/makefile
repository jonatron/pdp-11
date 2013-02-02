COMPILER = gcc
CCFLAGS = -Wall -g
LINKFLAGS = -lcurses -lpthread

all: main

main: main.c
	${COMPILER} ${CCFLAGS} ${LINKFLAGS} -o pdp main.c

clean:
	@rm -f pdp *.o
