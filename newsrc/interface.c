#include "interface.h"

#include <panel.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


#define NLINES 15
#define NCOLS 70

//A lot of code stolen from "NCURSES Programming HOWTO"

WINDOW *my_wins[3];
WINDOW *debug_subwindow, *tty_subwindow;
PANEL  *my_panels[3];
PANEL  *top;

pthread_mutex_t lock;

void debug_print(char *str) {
	pthread_mutex_lock(&lock);
	waddstr(debug_subwindow, str);
	touchwin(my_wins[2]);
	wrefresh(debug_subwindow);
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
		switch(ch) {
		case 9:
			top = (PANEL *)panel_userptr(top);
			pthread_mutex_lock(&lock);
			top_panel(top);
			pthread_mutex_unlock(&lock);
			break;
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
			break;
		case 2:
			sprintf(label, "Debug");
			debug_subwindow = derwin(wins[i], NLINES - 4, NCOLS - 4, 3, 2);
			scrollok(debug_subwindow, 1);
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

