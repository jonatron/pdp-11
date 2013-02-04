COMPILER = gcc
CCFLAGS = -Wall -g
LINKFLAGS = -lpanel -lcurses -lpthread -lrt

all: main

main: interface.o cpu.o coremem.o terminal.o 
	${COMPILER} ${CCFLAGS} ${LINKFLAGS} -o pdp main.c interface.c cpu.c coremem.c terminal.c

interface.o:
	${COMPILER} ${CCFLAGS} -c interface.c

cpu.o:
	${COMPILER} ${CCFLAGS} -c cpu.c

coremem.o:
	${COMPILER} ${CCFLAGS} -c coremem.c

terminal.o:
	${COMPILER} ${CCFLAGS} -c terminal.c


clean:
	@rm -f pdp *.o
