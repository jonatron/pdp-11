/* Functions for Core Memory */

#define MEMSIZE 0177777

#define GETBYTE(addr) (((addr <= MEMSIZE) && (addr >= 0)) ? 0 : memory[addr])

unsigned char inline getByte(unsigned int addr);

void inline setByte(unsigned int addr, unsigned char value);

unsigned short getWord(unsigned int addr);

void setWord(unsigned int addr, unsigned short value);

/* Union functions */
unsigned char getByteU(unsigned short addr);

unsigned char setByteU(unsigned short addr, unsigned char value);

/* Debugging function */
void printMem(const char * type, unsigned short startAddr, unsigned short endAddr);

/* Memory Mapped IO functions */

/* Initialize device function pointers to zero */
void initializeDeviceIO(void);

char configureDevice(unsigned short (*deviceFunction) (const char *, unsigned int, unsigned short), unsigned int startAddr, unsigned int endAddr);

unsigned short io(const char * command, unsigned int addr, unsigned short newWord);

/* Interrupts */

void busError(void);

/* Functions related to the CPU - Temporary until interrupts are complete */

/* Interrupt vectors */
#define BUS_ERROR 4


