#include <curses.h>
#include <string.h>
#include "CoreMem.h"
#include "terminal.h"
#include "interface.h"
#include "Cpu.h"

unsigned short rbuf, rcsr, xcsr;
extern WINDOW *tt;
extern WINDOW *iface;
/*
function to be called when a key is pressed
*/
void on_keypress(unsigned char key) {
	//transfer character to RBUF
	rbuf = key;
	
	//if receiver done bit (7) is set, set overrun and error bit in RBUF (14 and 15)
	if((rcsr & (1 << 7)) != 0) {
		rbuf |= 1 << 14 | 1 << 15;
	}
	//set receiver done bit (7) in RCSR
	rcsr |= (1 << 7);

	//if receiver interrupt enable bit (6) is set in RCSR, fire interrupt
	if((rcsr & (1 << 6)) != 0) {
		busRequest(&BR4, 060);
	}
}


unsigned short DL11io(const char * command, unsigned int addr, unsigned short value) {
	if(!strcmp(command, "read") && addr == RBUF) {
		//clear receiver done bit (7) in RCSR
		rcsr &= ~(1 << 7);

		//clear overrun and error bit in RBUF (14 and 15)
		rbuf &= ~(1 << 14 | 1 << 15);
		return rbuf;
	}
	else if(!strcmp(command, "write") && addr == XBUF) {
		//clear transmitter ready bit (7) in XCSR
		xcsr &= ~(1 << 7);
		//output - may be to socket later
		if(value != 13) {
			waddch(tt, (unsigned char) value);
			//wprintw(iface, "%d %c \n", value, value);
		}
		wrefresh(tt);
		//set transmitter ready bit (7) in XCSR
		xcsr |= 1 << 7;
		//busRequest(&BR4, 064); - breaks hello world at the moment
	}
	else if(!strcmp(command, "read") && addr == RCSR) {
		return rcsr;
	}
	else if(!strcmp(command, "read") && addr == XCSR) {
		return xcsr;
	}
	return 0;
}

