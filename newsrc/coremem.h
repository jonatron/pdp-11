#include <stdint.h>
/* Functions for Core Memory */
//size of char 1 short 2 int 4 long 8
//             8       16    32     64


#define MEMSIZE 0177777

#define GETBYTE(addr) (((addr <= MEMSIZE) && (addr >= 0)) ? 0 : memory[addr])

uint8_t inline getByte(uint32_t addr);

void inline setByte(uint32_t addr, uint8_t value);

uint16_t getWord(uint32_t addr);

void setWord(uint32_t addr, uint16_t value);

/* Union functions */
uint8_t getByteU(uint16_t addr);

uint8_t setByteU(uint16_t addr, uint8_t value);

/* Debugging function */
void printMem(const char * type, uint16_t startAddr, uint16_t endAddr);

/* Memory Mapped IO functions */

/* Initialize device function pointers to zero */
void initializeDeviceIO(void);

int8_t configureDevice(uint32_t (*deviceFunction) (const char *, uint32_t, uint32_t), uint32_t startAddr, uint32_t endAddr);

uint16_t io(const char * command, uint32_t addr, uint16_t newWord);

/* Interrupts */

void busError(void);

/* Functions related to the CPU - Temporary until interrupts are complete */

/* Interrupt vectors */
#define BUS_ERROR 4


