#include "interface.h"
#include "terminal.h"
#include "coremem.h"
#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <curses.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>


int INSTRUCTIONS_PER_SEC = 300000;
int NSLEEP_EVERY = 3000;

int main(int argc, const char* argv[]) {
	pthread_t termthrd;
	setup_interface();
	int rc = pthread_create(&termthrd, NULL, interface_loop, NULL);
	if(rc) {
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	struct timespec start_time, end_time, req, rem;
	int instruction_count = 0;
	long ns_taken, ms_taken;
	long ns_to_sleep_per_sec;
	long ns_per_every = 0;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	//sleep(1);
	debug_print("about to initializeDeviceIO\n");
	tty_print("tty print test\n");
	int halt = 0;

	initializeDeviceIO();
	configureDevice(DL11io, RCSR, XBUF);
	initCpu();
	debug_print("running fetchex loop\n");
	debug_print("size of ");
	char buf[50];
	sprintf(buf, "char %zu short %zu int %zu long %zu", sizeof(char), sizeof(short), sizeof(int), sizeof(long));
	debug_print(buf);
	while(1) {
		{ //run at a certain number of instructions per seconds
		instruction_count++;
		if(instruction_count % INSTRUCTIONS_PER_SEC == 0 && ns_per_every == 0) {
			clock_gettime(CLOCK_MONOTONIC, &end_time);
			ns_taken = ((end_time.tv_sec - start_time.tv_sec) * 1000000000) + (end_time.tv_nsec - start_time.tv_nsec);
			ms_taken = ns_taken / 1000000;
			ns_to_sleep_per_sec = (1000000000 - ns_taken);
			if(ns_to_sleep_per_sec < 100) {
				ns_per_every = -1; //don't sleep - unlikely to happen
			}
			else {
				ns_per_every = ns_to_sleep_per_sec / (INSTRUCTIONS_PER_SEC / NSLEEP_EVERY);
				req.tv_sec = 0;
				req.tv_nsec = ns_per_every;
				//clock_gettime(CLOCK_MONOTONIC, &start_time);
			}
		}
		if(instruction_count % NSLEEP_EVERY == 0 && ns_per_every > 0) {
			nanosleep(&req, &rem);
		}
		}
		//debug_print("fet");
		//fetchex
		if(!halt) {
			//debug_print("fetchex");
                        halt = fetchEx();
                        //sethalt(halt);
                        //lineclock();
                }
                //halt = gethalt();

	}

	return (EXIT_SUCCESS);
}
