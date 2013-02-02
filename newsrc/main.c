#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <curses.h>
#include <unistd.h>
#include <time.h>

WINDOW *iface, *tt;

void *terminal() {
        initscr();
	tt = newwin(0, 0, 0, 0); //create full screen window for teletype
        noecho();
	cbreak();
	iface = newwin(0, 0, 0, 0); //create full screen window for interface
        scrollok(iface, 1); //allow window to scroll
        scrollok(tt, 1);
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
}


int main(int argc, const char* argv[]) {
	//terminal();
	pthread_t termthrd;
	int rc = pthread_create(&termthrd, NULL, terminal, NULL);
	while(1) {
		//sleep(1);
	}

	return (EXIT_SUCCESS);
}
