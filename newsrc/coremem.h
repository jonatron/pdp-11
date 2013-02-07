#include <stdint.h>
/* Functions for Core Memory */
//size of char 1 short 2 int 4 long 8
//             8       16    32     64


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

int8_t configureDevice(uint16_t (*deviceFunction) (const char *, uint32_t, uint16_t), uint32_t startAddr, uint32_t endAddr);

unsigned short io(const char * command, unsigned int addr, unsigned short newWord);

/* Interrupts */

void busError(void);

/* Functions related to the CPU - Temporary until interrupts are complete */

/* Interrupt vectors */
#define BUS_ERROR 4


