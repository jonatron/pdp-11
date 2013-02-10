#define SWITCH 0777570 //console switch register memory location

unsigned short consoleio(const char * command, unsigned int addr, unsigned short value);

void load_adrs_switch();
void exam_switch();
void cont_switch();
void enable_switch();
void start_switch();
void dep_switch();
char gethalt();
void sethalt(char hlt);
void set_data_reg(unsigned short value);

unsigned short getdatareg();
unsigned short getswitchreg();
unsigned long getaddrreg();

