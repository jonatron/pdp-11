#include <curses.h>
#include <string.h>
#include "coremem.h"
#include "terminal.h"
#include "interface.h"
#include "cpu.h"

unsigned short rbuf, rcsr, xcsr;
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
	debug_print("on_keypress\n");

	//if receiver interrupt enable bit (6) is set in RCSR, fire interrupt
	if((rcsr & (1 << 6)) != 0) {
		busRequest(&BR4, 060);
	}
}


uint16_t DL11io(const char * command, uint32_t addr, uint16_t value) {
	if(!strcmp(command, "read") && addr == RBUF) {
		debug_print("read rbuf");
		//clear receiver done bit (7) in RCSR
		rcsr &= ~(1 << 7);

		//clear overrun and error bit in RBUF (14 and 15)
		rbuf &= ~(1 << 14 | 1 << 15);
		return rbuf;
	} else if(!strcmp(command, "write") && addr == XBUF) {
		//clear transmitter ready bit (7) in XCSR
		xcsr &= ~(1 << 7);
		//output - may be to socket later
		if(value != 13) {
			//debug_print("printing to tty\n");
			tty_printch((char) value);
			//waddch(tt, (unsigned char) value);
			//wprintw(iface, "%d %c \n", value, value);
		}
		//set transmitter ready bit (7) in XCSR
		xcsr |= 1 << 7;
		//busRequest(&BR4, 064); - breaks hello world at the moment
	} else if(!strcmp(command, "read") && addr == RCSR) {
		debug_print("read rcsr\n");
		return rcsr;
	} else if(!strcmp(command, "read") && addr == XCSR) {
		debug_print("read xcsr\n");
			char buf[50];
                        sprintf(buf, "xcsr: %d\n", xcsr);
                        debug_print(buf);
		return xcsr;
	}else if(!strcmp(command, "write") && addr == XCSR) {
		xcsr = value;
	}else if(!strcmp(command, "write") && addr == RCSR) {
		rcsr = value;
	}
	return 0;
}

