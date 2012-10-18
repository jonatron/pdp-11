#include "Cpu.h"
#include "CoreMem.h"
#include "terminal.h"
#include "interface.h"
#include "console.h"
#include "rk.h"
#include "lineclock.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <curses.h>
#include <unistd.h>
#include "rktest.h"

extern WINDOW *tt;

int main(int argc, const char* argv[]) {
	
	//for testing to stop the machine.
	int halt = 0;

	pthread_t termthrd;
	initializeDeviceIO();
	configureDevice(DL11io, RCSR, XBUF);
	configureDevice(consoleio, SWITCH, SWITCH);
	//configureDevice(lineclockio, KW11L, KW11L);
	initCpu();
	rk_init();
	
	#if RK_TESTING
	if (argc != 2)
		printf("Usage is \"pdp blah.dsk\"\n");
	else
		if (!mount((char *)argv[1], 0)) {
			printf("Cannot mount drive\n");
			return 1;
		}
	int rk = rk_run_tests();
	unMount(0);
	if (!rk)
		return 0;
	else
		return 1;
	#endif
	
	if (!mount("hello.dsk", 0)) {
		printf("Cannot mount drive\n");
		return 1;
	}
	/**
	 * to use the hello.dsk image; comment out testHello() in initCpu()
	 * in Cpu.c and uncomment rk_boot_hello() below  
	 */
	
	rk_boot_hello(); //see above for how to use this
	//bootStrap();
	setupwindow();	
	int rc = pthread_create(&termthrd, NULL, terminal, NULL);
	if (rc){
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}
	startlineclock();
	while (1) {
		if(!halt) {
			halt = fetchEx();
			sethalt(halt);
			lineclock();
		}
		else
			sleep(1);

 		halt = gethalt();
		//just for testing. remove if you need the machine to keep running after one cycle.
		//halt = 1;
	}
	endwin();
	unMount(0);
	
	return (EXIT_SUCCESS);
}
