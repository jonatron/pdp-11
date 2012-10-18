#include "interface.h"
#include "terminal.h"
#include "CoreMem.h"
#include "console.h"
#include <curses.h>

char ifstate, currentwindow;
unsigned short current_switch;
unsigned char switch_bitcount;

WINDOW *iface, *tt, *iface2; //tt = teletype terminal

#define STATE_MENU 0
#define STATE_PROGRAM_SWITCH 1

void bin(unsigned short n) {
	unsigned int i;
	i = 1<<(sizeof(n) * 8 - 1);
	while (i > 0) {
		if (n & i)
			waddstr(iface2, "1");
		else
			waddstr(iface2, "0");
		i >>= 1;
	}
}

void updateregsdisplay() {
	werase(iface2);
	waddstr(iface2, "Addr reg:");
	unsigned long addrreg = getaddrreg();
	bin(addrreg);
	waddstr(iface2, " Data reg:");
	unsigned short datareg = getdatareg();
	bin(datareg);
	waddstr(iface2, " Switch reg:");
	signed short switchreg = getswitchreg();
	bin(switchreg);
	if(currentwindow == IFACE)
		wrefresh(iface2);

}

void iface_keypress(char ch) {
	if(ifstate == STATE_MENU) {
		switch(ch) {
			case 'r':
				updateregsdisplay();
			break;
			case 's':
				ifstate = STATE_PROGRAM_SWITCH;
				waddstr(iface, "\n Enter switch register in binary and press enter \n");
				switch_bitcount = 0;
				current_switch = 0;
			break;
			case 'd':
				dep_switch();
				waddstr(iface, "\n dep switch pressed \n");
			break;
			case 'e':
				exam_switch();
				waddstr(iface, "\n exam switch pressed \n");
			break;
			case 'l':
				load_adrs_switch();
				waddstr(iface, "\n load switch pressed \n");
			break;
			case 'a':
				enable_switch();
				waddstr(iface, "\n enable switch pressed \n");
			break;
			case 'c':
				cont_switch();
				waddstr(iface, "\n cont switch pressed \n");
			break;
		}
	}
	else if(ifstate == STATE_PROGRAM_SWITCH) {
		if((ch == '0' || ch == '1') && switch_bitcount < 16) {
			waddch(iface, ch);
			switch_bitcount++;
			if(ch == '1')
				current_switch |= 1 << (16 - switch_bitcount);
		}
		else if(ch == 10) { //enter
			setWord(SWITCH, current_switch);
			ifstate = STATE_MENU;
			iface_printmenu();
		}
	}
	wrefresh(iface);
}

void iface_printmenu() {
	waddstr(iface, "\nMenu: \n r: update address and data registers \n s: modify switch register"
	" \n d: press dep switch \n e: press exam switch \n l: press load switch"
	" \n a: press enable switch \n c: press cont switch");
	wrefresh(iface);
}

void setupwindow() {
        initscr();
        noecho();
        iface = newwin(0, 0, 1, 0);
        tt = newwin(0, 0, 0, 0);
	iface2 = newwin(1, 0, 0, 0);
        scrollok(iface, 1);
        scrollok(tt, 1);
        addstr("PDP11 Emulator Started");
        waddstr(tt, "tt window\n");
        waddstr(iface, "iface window\n");
	waddstr(iface2, "iface2 here\n");
        refresh();
        touchwin(tt);
        wrefresh(tt);
}

void *terminal() {
        char ch;
	currentwindow = TT;
        while(ch != 27) {
                ch = getch();
                if(ch == ERR)
			continue;
		if(ch == 14) {
			currentwindow = !currentwindow;
			touchwin(iface);
			touchwin(iface2);
			touchwin(tt);
			if(currentwindow == IFACE) {
				ifstate = STATE_MENU;
				iface_printmenu();
				wrefresh(iface);
				wrefresh(iface2);
			}
			else {
				wrefresh(tt);
			}
		}
		else if(currentwindow == TT) {
	                on_keypress(ch);
                	//setWord((unsigned short) XBUF, (unsigned short) ch); //to be removed, here for testing
			//wrefresh(tt); 
		}
		else if(currentwindow == IFACE) {
			iface_keypress(ch);
		}
                
        }
	return 0;
}

