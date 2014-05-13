#include "console.h"
#include "coremem.h"
#include "interface.h"
#include "cpu.h"
#include <string.h>
#include <curses.h>

#include <wiringPi.h>
#include <mcp23017.h>

extern WINDOW *tt;


unsigned short datareg; //data register - contents of this is diplayed on console
unsigned long addrreg; //18 bit address register - contents of this is displayed on console
unsigned int switchreg;

char halt = 0;

void check_switches() {
	char start = digitalRead(200) ^ 1;
	char halt = digitalRead(202) ^ 1;
	char cont = digitalRead(203) ^ 1;
	char exam = digitalRead(204) ^ 1;
	char load = digitalRead(205) ^ 1;
	char dep = digitalRead(208) ^ 1;
	char buf[50];

	/*
	sprintf(buf, "cont switch in octal: %o\n", cont);
	debug_print(buf);
	sprintf(buf, "start switch in octal: %o\n", start);
	debug_print(buf);
	sprintf(buf, "load switch in octal: %o\n", load);
	debug_print(buf);
	sprintf(buf, "exam switch in octal: %o\n", exam);
	debug_print(buf);
	sprintf(buf, "halt switch in octal: %o\n", halt);
	debug_print(buf);
	sprintf(buf, "dep switch in octal: %o\n", dep);
	debug_print(buf);

	print_switches();*/
}

unsigned short consoleio(const char * command, unsigned int addr, unsigned int value) {
	updateregsdisplay();
	if(!strcmp(command, "read") && addr == SWITCH) {
		return switchreg;
	}
	if(!strcmp(command, "write") && addr == SWITCH) {
		switchreg = value;
	}
	return 0;
}

void set_switch_reg(unsigned int swr) {
	switchreg = swr;
}

/*
LOAD ADRS switch
Transfer contents of SWITCH register to ADDRESS register
*/
void load_adrs_switch() {
	addrreg = switchreg;
	updateregsdisplay();
}

void set_data_reg(unsigned short value) {
	datareg = value;
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

unsigned int getswitchreg() {
	return switchreg;
}

unsigned long getaddrreg() {
	return addrreg;
}
