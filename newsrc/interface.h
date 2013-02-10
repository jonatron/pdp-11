#include <curses.h>

void debug_print(char *str);
void tty_print(char *str);
void tty_printch(char ch);
void setup_interface();
void *interface_loop();
void updateregsdisplay();
void init_wins(WINDOW **wins, int n);
void win_show(WINDOW *win, char *label, int label_color);
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
