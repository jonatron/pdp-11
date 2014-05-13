#include "interface.h"
#include "console.h"

#include <panel.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


#include <wiringPi.h>
#include <mcp23017.h>


#define NLINES 35
#define NCOLS 70
#define PADLINES 100

//A lot of code stolen from "NCURSES Programming HOWTO"

WINDOW *my_wins[3];
WINDOW *debug_pad/*, *debug_subpad*/;
WINDOW *debug_subwindow, *tty_subwindow, *iface_subwindow;
PANEL  *my_panels[3];
PANEL  *top;

#define MAX_PAD_POS (PADLINES - NLINES + 3)
int pad_pos = MAX_PAD_POS;


pthread_mutex_t lock;


#define STATE_MENU 0
#define STATE_PROGRAM_SWITCH 1
char ifstate;
unsigned short current_switch;
unsigned char switch_bitcount;

void debug_print(char *str) {
	pthread_mutex_lock(&lock);
	waddstr(debug_pad, str);
	copywin(debug_pad, debug_subwindow, pad_pos, 0, 0, 0, NLINES - 5, NCOLS - 5, FALSE);
	if(top == my_panels[2]) {
		wrefresh(debug_subwindow);
	}
	pthread_mutex_unlock(&lock);
}

void tty_print(char *str) {
	pthread_mutex_lock(&lock);
	waddstr(tty_subwindow, str);
	touchwin(my_wins[0]);
	wrefresh(tty_subwindow);
	pthread_mutex_unlock(&lock);

}

void tty_printch(char ch) {
	pthread_mutex_lock(&lock);
	waddch(tty_subwindow, ch);
	touchwin(my_wins[0]);
	wrefresh(tty_subwindow);
	pthread_mutex_unlock(&lock);
}

void bin(WINDOW *win, unsigned int n) {
        unsigned int i;
	char counter;
        i = 1<<(sizeof(n) * 8 - 1);
	counter = 0;
        while (i > 0) {
                if (n & i)
                        waddstr(win, "1");
                else
                        waddstr(win, "0");
                i >>= 1;
		if(counter % 3 == 0)
			waddstr(win, " ");
		counter++;
        }
}

void updateregsdisplay() {
	if(top == my_panels[1]) {
		werase(iface_subwindow);
	}
        waddstr(iface_subwindow,   "Addr reg  :");
        unsigned long addrreg = getaddrreg();
        bin(iface_subwindow, addrreg);
        waddstr(iface_subwindow, "\nData reg  :");
        unsigned short datareg = getdatareg();
        bin(iface_subwindow, datareg);
        waddstr(iface_subwindow, "\nSwitch reg:");
        signed short switchreg = getswitchreg();
        bin(iface_subwindow, switchreg);
	if(top == my_panels[1]) {
	        wrefresh(iface_subwindow);
	}
	touchwin(my_wins[1]);

	/*
DATA
209 # 210 | 211 | 212 # 213 | 214 | 215 #  308 | 309 | 310 # 311 | 312 | 313 # 314 | 315 | 405

ADDRESS
408 | 409 | 410 # 411 | 412 | 413 # 414 | 415 | 307 # 306 | 305 | 304 # 303 | 302 | 301 # 300 | 407 | 406
*/
	digitalWrite(209, datareg >> 15 & 1);
	digitalWrite(210, datareg >> 14 & 1);
	digitalWrite(211, datareg >> 13 & 1);
	digitalWrite(212, datareg >> 12 & 1);
	digitalWrite(213, datareg >> 11 & 1);
	digitalWrite(214, datareg >> 10 & 1);
	digitalWrite(215, datareg >> 9 & 1);
	digitalWrite(308, datareg >> 8 & 1);
	digitalWrite(309, datareg >> 7 & 1);
	digitalWrite(310, datareg >> 6 & 1);
	digitalWrite(311, datareg >> 5 & 1);
	digitalWrite(312, datareg >> 4 & 1);
	digitalWrite(313, datareg >> 3 & 1);
	digitalWrite(314, datareg >> 2 & 1);
	digitalWrite(315, datareg >> 1 & 1);
	digitalWrite(405, datareg & 1);


        digitalWrite(408, addrreg >> 17 & 1);
        digitalWrite(409, addrreg >> 16 & 1);
        digitalWrite(410, addrreg >> 15 & 1);
        digitalWrite(411, addrreg >> 14 & 1);
        digitalWrite(412, addrreg >> 13 & 1);
        digitalWrite(413, addrreg >> 12 & 1);
        digitalWrite(414, addrreg >> 11 & 1);
        digitalWrite(415, addrreg >> 10 & 1);
        digitalWrite(307, addrreg >> 9 & 1);
        digitalWrite(306, addrreg >> 8 & 1);
        digitalWrite(305, addrreg >> 7 & 1);
        digitalWrite(304, addrreg >> 6 & 1);
        digitalWrite(303, addrreg >> 5 & 1);
        digitalWrite(302, addrreg >> 4 & 1);
        digitalWrite(301, addrreg >> 3 & 1);
        digitalWrite(300, addrreg >> 2 & 1);
        digitalWrite(407, addrreg >> 1 & 1);
        digitalWrite(406, addrreg & 1);



}



void setup_interface() {

	pthread_mutex_init(&lock, NULL);
	pthread_mutex_lock(&lock);

	/* Initialize curses */
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	timeout(0);

	/* Initialize all the colors */
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_BLUE, COLOR_BLACK);
	init_pair(4, COLOR_CYAN, COLOR_BLACK);

	init_wins(my_wins, 3);

	/* Attach a panel to each window */ 	/* Order is bottom up */
	my_panels[0] = new_panel(my_wins[0]); 	/* Push 0, order: stdscr-0 */
	my_panels[1] = new_panel(my_wins[1]); 	/* Push 1, order: stdscr-0-1 */
	my_panels[2] = new_panel(my_wins[2]); 	/* Push 2, order: stdscr-0-1-2 */

	/* Set up the user pointers to the next panel */
	set_panel_userptr(my_panels[0], my_panels[1]);
	set_panel_userptr(my_panels[1], my_panels[2]);
	set_panel_userptr(my_panels[2], my_panels[0]);

	/* Update the stacking order. 2nd panel will be on top */
	update_panels();

	/* Show it on the screen */
	attron(COLOR_PAIR(4));
	mvprintw(LINES - 2, 0, "Use tab to browse through the windows");
	attroff(COLOR_PAIR(4));
	doupdate();

	top = my_panels[2];

	pthread_mutex_unlock(&lock);

	iface_printmenu();


	/* gpio */
	mcp23017Setup(100, 0x20);
	mcp23017Setup(200, 0x21);
	mcp23017Setup(300, 0x22);
	mcp23017Setup(400, 0x23);
	mcp23017Setup(500, 0x24);

	int i;

	for(i = 100; i <= 115; i++) {
		pinMode(i, INPUT);
		pullUpDnControl (i, PUD_UP);
	}

	for(i = 200; i <= 208; i++) {
		pinMode(i, INPUT);
		pullUpDnControl (i, PUD_UP);
	}


	for(i = 209;i <= 215; i++) {
		pinMode(i, OUTPUT);
	}
	for(i = 300;i <= 315; i++) {
		pinMode(i, OUTPUT);
	}
	for(i = 400;i <= 415; i++) {
		pinMode(i, OUTPUT);
	}
	for(i = 500;i <= 501; i++) {
		pinMode(i, OUTPUT);
	}


}


void print_switches() {
	unsigned int switch_reg = 0;

	switch_reg = 0;
	switch_reg |= digitalRead(206);
	switch_reg |= digitalRead(207) << 1;
	switch_reg |= digitalRead(115) << 2;
	switch_reg |= digitalRead(114) << 3;
	switch_reg |= digitalRead(113) << 4;
	switch_reg |= digitalRead(112) << 5;
	switch_reg |= digitalRead(111) << 6;
	switch_reg |= digitalRead(110) << 7;
	switch_reg |= digitalRead(109) << 8;
	switch_reg |= digitalRead(108) << 9;
	switch_reg |= digitalRead(100) << 10;
	switch_reg |= digitalRead(101) << 11;
	switch_reg |= digitalRead(102) << 12;
	switch_reg |= digitalRead(103) << 13;
	switch_reg |= digitalRead(104) << 14;
	switch_reg |= digitalRead(105) << 15;
	switch_reg |= digitalRead(106) << 16;
	switch_reg |= digitalRead(107) << 17;

	switch_reg = ~switch_reg; //invert because i'm too lazy to turn switches round
	switch_reg = switch_reg & 0777777;

	set_switch_reg(switch_reg);

	bin(iface_subwindow, switch_reg);

	waddstr(iface_subwindow, "\n");
	wrefresh(iface_subwindow);
}

void iface_keypress(char ch) {
        if(ifstate == STATE_MENU) {
                switch(ch) {
                        case 'r':
                                updateregsdisplay();
                        break;
                        case 's':
                                ifstate = STATE_PROGRAM_SWITCH;
                                waddstr(iface_subwindow, "\n Enter switch register in binary and press enter\n");
                                switch_bitcount = 0;
                                current_switch = 0;
                        break;
                        case 'd':
                                dep_switch();
                                waddstr(iface_subwindow, "\n dep switch pressed \n");
                        break;
                        case 'e':
                                exam_switch();
                                waddstr(iface_subwindow, "\n exam switch pressed \n");
                        break;
                        case 'l':
                                load_adrs_switch();
                                waddstr(iface_subwindow, "\n load switch pressed \n");
                        break;
                        case 'a':
                                enable_switch();
                                waddstr(iface_subwindow, "\n enable switch pressed \n");
                        break;
                        case 'c':
                                cont_switch();
                                waddstr(iface_subwindow, "\n cont switch pressed \n");
                        break;
			case 'g':
				print_switches();
			break;
                }
        }
        else if(ifstate == STATE_PROGRAM_SWITCH) {
                if((ch == '0' || ch == '1') && switch_bitcount < 16) {
                        waddch(iface_subwindow, ch);
                        if(ch == '1')
                                current_switch |= 1 << (switch_bitcount);
			switch_bitcount++;
                }
                else if(ch == 10) { //enter
                        setWord(SWITCH, current_switch);
                        ifstate = STATE_MENU;
			updateregsdisplay();
                        iface_printmenu();
                }
        }
        wrefresh(iface_subwindow);
}

void iface_printmenu() {
        waddstr(iface_subwindow, "\nMenu: \n r: update address and data registers \n s: modify switch register"
        " \n d: press dep switch \n e: press exam switch \n l: press load switch"
        " \n a: press enable switch \n c: press cont switch");
        wrefresh(iface_subwindow);
}


void *interface_loop() {
	int ch;
	while(ch != KEY_F(1)) {
		pthread_mutex_lock(&lock);
		ch = getch();
		pthread_mutex_unlock(&lock);
		if(ch == ERR) {
			struct timespec req, rem;
			req.tv_sec = 0;
			req.tv_nsec = 50000;
			nanosleep(&req, &rem);
			continue;
		}
		if(ch == 9) { //tab
			top = (PANEL *)panel_userptr(top);
			pthread_mutex_lock(&lock);
			top_panel(top);
			pthread_mutex_unlock(&lock);
		}
		else {
			if(top == my_panels[2]) { //debug window
				if(ch == KEY_UP) {
					pthread_mutex_lock(&lock);
					if(pad_pos >= 1) {
						pad_pos--;
					}
					copywin(debug_pad, debug_subwindow, pad_pos, 0, 0, 0, NLINES - 5, NCOLS - 5, FALSE);
					wrefresh(debug_subwindow);
					pthread_mutex_unlock(&lock);
				}
				if(ch == KEY_DOWN) {
					pthread_mutex_lock(&lock);
					if(pad_pos < MAX_PAD_POS) {
						pad_pos++;
					}
					copywin(debug_pad, debug_subwindow, pad_pos, 0, 0, 0, NLINES - 5, NCOLS - 5, FALSE);
					wrefresh(debug_subwindow);
					pthread_mutex_unlock(&lock);
				}
			}
			if(top == my_panels[1]) { //iface window
				iface_keypress(ch);
				//if(ch == 'u') {
				//	updateregsdisplay();
				//}
				//wrefresh(iface_subwindow);
			}
			if(top == my_panels[0]) { //tty window
				on_keypress(ch);
			}
		}
		pthread_mutex_lock(&lock);
		update_panels();
		doupdate();
		pthread_mutex_unlock(&lock);
	}
	endwin();
	return 0;
}

/* Put all the windows */
void init_wins(WINDOW **wins, int n) {
	int x, y, i;
	char label[80];

	y = 2;
	x = 10;
	for(i = 0; i < n; ++i) {
		wins[i] = newwin(NLINES, NCOLS, y, x);
		switch(i) {
		case 0:
			sprintf(label, "PDP tty");
			tty_subwindow = derwin(wins[i], NLINES - 4, NCOLS - 4, 3, 2);
			scrollok(tty_subwindow, 1);
			break;
		case 1:
			sprintf(label, "Emulator Interface");
			iface_subwindow = derwin(wins[i], NLINES - 4, NCOLS - 4, 3, 2);
			break;
		case 2:
			sprintf(label, "Debug");
			debug_pad = newpad(100, NCOLS - 4);
			debug_subwindow = derwin(wins[i], NLINES - 4, NCOLS - 4, 3, 2);
			wmove(debug_pad, 99, 0);
			scrollok(debug_pad, 1);
			break;
		}
		win_show(wins[i], label, i + 1);
		y += 3;
		x += 7;
	}
}

/* Show the window with a border and a label */
void win_show(WINDOW *win, char *label, int label_color) {
	int startx, starty, height, width;

	getbegyx(win, starty, startx);
	getmaxyx(win, height, width);

	box(win, 0, 0);
	mvwaddch(win, 2, 0, ACS_LTEE);
	mvwhline(win, 2, 1, ACS_HLINE, width - 2);
	mvwaddch(win, 2, width - 1, ACS_RTEE);

	print_in_middle(win, 1, 0, width, label, COLOR_PAIR(label_color));
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color) {
	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}

