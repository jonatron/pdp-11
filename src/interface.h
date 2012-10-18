#define TT 0
#define IFACE 1
#define STATE_MENU 0
#define STATE_PROGRAM_SWITCH 1

void setupwindow();
void updateregsdisplay();
void *terminal();
void iface_keypress(char ch);
void iface_printmenu();


