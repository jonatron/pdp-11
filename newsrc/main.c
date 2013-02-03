#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <curses.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>

WINDOW *iface, *tt;

void *terminal() {
	//essential ncurses setup
        initscr();
        noecho();
	cbreak();

	// newwin(int height, int width, int starty, int startx)
	tt = newwin(0, 0, 0, 0); //create full screen window for teletype
	iface = newwin(LINES - 1, COLS - 1, 1, 1); //create full screen window for interface

        scrollok(iface, 1); //allow window to scroll
        scrollok(tt, 1);

	box(iface, 0, 0);

	wmove(iface, 1, 1);
	waddstr(iface, "iface window\n");
	refresh();
        touchwin(iface);
        wrefresh(iface);

        char ch;
        while(ch != 27) {
                ch = getch();
		if(ch == 'n') {
			waddstr(iface, "\n n \n");
			wrefresh(iface);
		}
		else {
			waddstr(iface, "\n key pressed \n");
			wrefresh(iface);
		}
	}
	pthread_exit(NULL);
}

int INSTRUCTIONS_PER_SEC = 300000;
int NSLEEP_EVERY = 3000;

int main(int argc, const char* argv[]) {
	pthread_t termthrd;
	int rc = pthread_create(&termthrd, NULL, terminal, NULL);
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
	while(1) {
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

	return (EXIT_SUCCESS);
}
