COMPILER = gcc
CCFLAGS = -Wall -g
LINKFLAGS = -lcurses -lpthread

all: main

main: Main.c CoreMem.o Cpu.o terminal.o interface.o console.o rk.o lineclock.o
	${COMPILER} ${CCFLAGS} ${LINKFLAGS} -o pdp Main.c terminal.c Cpu.o CoreMem.o interface.o console.o rk.o lineclock.o

terminal.o: terminal.c terminal.h
	${COMPILER} ${CCFLAGS} -c terminal.c

rk.o: rk.c rk.h
	${COMPILER} ${CCFLAGS} -c rk.c

CoreMem.o: CoreMem.c CoreMem.h
	${COMPILER} ${CCFLAGS} -c CoreMem.c 

interface.o: interface.c interface.h
	${COMPILER} ${CCFLAGS} -c interface.c

console.o: console.c console.h
	${COMPILER} ${CCFLAGS} -c console.c

Cpu.o: Cpu.c Cpu.h
	${COMPILER} ${CCFLAGS} -c Cpu.c

lineclock.o: lineclock.c lineclock.h
	${COMPILER} ${CCFLAGS} -c lineclock.c

rktest.o: rktest.c rktest.h
	${COMPILER} ${CCFLAGS} -c rktest.c

testdsk: rkdiskgen.c
	${COMPILER} -o gen rkdiskgen.c
	./gen
	rm gen

rktest: Main.c CoreMem.o Cpu.o terminal.o interface.o console.o rk.o lineclock.o rktest.o
	${COMPILER} ${CCFLAGS} ${LINKFLAGS} -o rktest Main.c terminal.c Cpu.o CoreMem.o interface.o console.o rk.o lineclock.o rktest.o

test: cpu_tests.c cut.h Cpu.o CoreMem.o
	cutgen -o testcpu.c cpu_tests.c
	${COMPILER} ${CCFLAGS} -o tests	Cpu.o CoreMem.o cpu_tests.c testcpu.c


install: pdp
	@mv pdp /usr/local/bin/
	
uninstall:
	@rm /usr/local/bin/pdp

clean:
	@rm -f pdp CoreMem.o Cpu.o test testcpu.c *.o
