#include "console.h"
#include "coremem.h"
#include "interface.h"
#include "cpu.h"
#include <string.h>
#include <curses.h>
extern WINDOW *tt;


unsigned short datareg; //data register - contents of this is diplayed on console
unsigned long addrreg; //18 bit address register - contents of this is displayed on console
unsigned short switchreg;

char halt = 0;

unsigned short consoleio(const char * command, unsigned int addr, unsigned short value) {
	updateregsdisplay();
	if(!strcmp(command, "read") && addr == SWITCH) {
		return switchreg;
	}
	if(!strcmp(command, "write") && addr == SWITCH) {
		switchreg = value;
	}
	return 0;
}

/*
LOAD ADRS switch
Transfer contents of SWITCH register to ADDRESS register
*/
void load_adrs_switch() {
	addrreg = switchreg;
	updateregsdisplay();
}

/*
EXAM switch
DATA = memory contents at address ADDRESS
Then increment ADDRESS
*/
void exam_switch() {
	datareg = getWord(addrreg);
	addrreg++;
	updateregsdisplay();
}

/*
CONT switch
run one instruction when halted
*/
void cont_switch() {
	if(halt) {
		fetchEx();
		debug_print("CONT SWITCH PRESSED!");
	}
}

/*
ENABLE/HALT switch

*/
void enable_switch() {
	halt = !halt;
}

/*
START switch

*/
void start_switch() {
	
}

/*
DEP switch
Transfer contents of SWITCH register to the memory location at address ADDRESS
if odd address, use next even
*/
void dep_switch() {
	if(addrreg % 2 != 0) {
		addrreg--;
	}
	setWord(addrreg, switchreg);
	addrreg += 2;
	updateregsdisplay();
}

char gethalt() {
	return halt;
}

void sethalt(char hlt) {
	halt = hlt;
}

unsigned short getdatareg() {
	return datareg;
}

unsigned short getswitchreg() {
	return switchreg;
}

unsigned long getaddrreg() {
	return addrreg;
}
