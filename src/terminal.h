#define RCSR 0777560 //receiver status register address
#define RBUF 0777562 //receiver data buffer register address
#define XCSR 0777564 //Transmitter status register address
#define XBUF 0777566 //Transmitter data buffer register

void printregs();
void on_keypress(unsigned char key);
unsigned short DL11io(const char * command, unsigned int addr, unsigned short value);
