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
unsigned long switchreg;

char halt = 1;

char last_halt = 1;
char last_cont = 1;
char last_exam = 1;
char last_load = 1;
char last_dep = 1;

void check_switches() {
	char cur_start = digitalRead(200) ^ 1;
	char cur_halt = digitalRead(202) ^ 1;
	char cur_cont = digitalRead(203) ^ 1;
	char cur_exam = digitalRead(204) ^ 1;
	char cur_load = digitalRead(205) ^ 1;
	char cur_dep = digitalRead(208) ^ 1;
	char buf[50];

	if(cur_halt != last_halt) {
		sethalt(cur_halt);
		last_halt = cur_halt;
	}
	if(cur_dep != last_dep) {
		dep_switch();
		last_dep = cur_dep;
	}
	if(cur_load != last_load) {
		load_adrs_switch();
		last_load = cur_load;
	}

	unsigned long switch_reg = 0;

	switch_reg = 0;
	switch_reg |= digitalRead(206);
	switch_reg |= digitalRead(207) << 1;
	switch_reg |= digitalRead(115) << 2;
	switch_reg |= digitalRead(114) << 3;
	switch_reg |= digitalRead(113) << 4;
	switch_reg |= digitalRead(101) << 5;
	switch_reg |= digitalRead(111) << 6;
	switch_reg |= digitalRead(110) << 7;
	switch_reg |= digitalRead(109) << 8;
	switch_reg |= digitalRead(108) << 9;
	switch_reg |= digitalRead(100) << 10;
	switch_reg |= digitalRead(112) << 11;
	switch_reg |= digitalRead(102) << 12;
	switch_reg |= digitalRead(103) << 13;
	switch_reg |= digitalRead(104) << 14;
	switch_reg |= digitalRead(105) << 15;
	switch_reg |= digitalRead(106) << 16;
	switch_reg |= digitalRead(107) << 17;

	switch_reg = ~switch_reg; //invert because i'm too lazy to turn switches round
	switch_reg = switch_reg & 0777777;

	set_switch_reg(switch_reg);


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

void set_switch_reg(unsigned long swr) {
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
	datareg = switchreg;
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

unsigned long getswitchreg() {
	return switchreg;
}

unsigned long getaddrreg() {
	return addrreg;
}
