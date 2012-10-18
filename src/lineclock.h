#define LINE_CLOCK_HZ 50
#define KW11L 0777546

unsigned short lineclockio(const char * command, unsigned int addr, unsigned short value);
void startlineclock();
void lineclock();
