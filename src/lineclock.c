#include "lineclock.h"
#include "Cpu.h"
#include <time.h>
#include <string.h>

clock_t endclock;

unsigned short lineclockio(const char * command, unsigned int addr, unsigned short value) {
	if(!strcmp(command, "read")) {
		return 1;
	}
	return 1;
}

void startlineclock() {
	endclock = clock() + (CLOCKS_PER_SEC / LINE_CLOCK_HZ);
}

void lineclock() {
	if(clock() >= endclock) {
		endclock = clock() + (CLOCKS_PER_SEC / LINE_CLOCK_HZ);
		busRequest(&BR6, 0100);
	}
}
