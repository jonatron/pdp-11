/*
 * File CoreMem.c
 *
 * Memory of the simulated machine
 *  Includes accessor and modifier functions
 */
 
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "coremem.h"
#include "interface.h"

/* Check this word is valid (on an even boundary) */
#define isWord(addr) (addr % 2 == 0) ? 1 : 0

/* Start and end of IO addresses */
#define IOSTART 0760000
#define IOEND 0777777
#define IOSTART16 0160000
#define IOEND16 0177777

/* Determine addresses in device range */
#define isDeviceAddr(addr) ((addr >= IOSTART16 && addr <= IOEND16) || (addr >= IOSTART && addr <= IOEND)) ? 1 : 0

/* Map an address in device range to a function */
#define getDeviceAddr(addr) ((addr - IOSTART) / 2)

/* 16-bit Machine Memory */
unsigned char memory[MEMSIZE];

/* Array of IO functions */
unsigned short (*currentFunc[4096]) (const char *, unsigned int, unsigned short);

/* Try a union*/
union memory {

  unsigned char byte[MEMSIZE]; /* Access one byte of memory */
  unsigned short word[MEMSIZE]; /* Access one word of memory */
} PDPMem;

/* Get the contents of memory location */
unsigned char inline getByte(unsigned int addr) {
  
  unsigned int deviceAddr = 0; /* 18-bit Device Address */
  unsigned short regValue = 0; /* Current register word value */
  
  /* Address in Device IO Space */
  if (isDeviceAddr(addr)) {
  
    /* Convert 16-bit address to 18-bit IO address adding extra bits*/
    deviceAddr = addr | 0600000;
  
    // Mapping of Device addresses to IO function addresses
    unsigned short ioAddr = getDeviceAddr(deviceAddr);
    
      // Perform IO request if function has been registered
      if (*currentFunc[ioAddr] != 0) {
  
        /* Read word from device using address specified */
  
        /* Device Address is odd, read high byte */
        if (deviceAddr % 2 != 0) {
          regValue = currentFunc[ioAddr]("read", deviceAddr - 1, 0);
          return regValue >> 8;
        }
        else {
          regValue = currentFunc[ioAddr]("read", deviceAddr, 0);
          return regValue;
        }
      }
      /* Device invalid - Interrupt Bus Error */
      else {
        busError();
        return 0;
      }
  }
  else {
    /* Normal Memory access */
    return memory[addr];
  }
}

/* Set the contents of memory location */
void inline setByte(unsigned int addr, unsigned char value) {

  unsigned int deviceAddr = 0; /* 18-bit Device Address */
  unsigned short word = 0; /* New Register Value */
  unsigned char lowByte = 0;
  unsigned char highByte = 0;
  
  /* Address in Device IO Space */
  if (isDeviceAddr(addr)) {
  
    /* Convert 16-bit address to 18-bit IO address adding extra bits*/
    deviceAddr = addr | 0600000;
  
    // Mapping of Device addresses to IO function addresses
    unsigned short ioAddr = getDeviceAddr(deviceAddr);
    
      // Perform IO request if function has been registered
      if (*currentFunc[ioAddr] != 0) {
  
        /* Generate a new 16 bit word using new byte value */
        
        /* Device Address is odd, set high byte */
        if (deviceAddr % 2 != 0) {
          highByte = value;
          lowByte = currentFunc[ioAddr]("read", deviceAddr - 1, 0);
          
          word = word | highByte;
          word = word << 8;
          word = word | lowByte;
          
          /* Write to device using address specified */
          currentFunc[ioAddr]("write", deviceAddr - 1, word);
        }
        else {
          /* Write to device using address specified */
          currentFunc[ioAddr]("write", deviceAddr, value);
        }
        
      }
      /* Device invalid - Interrupt Bus Error */
      else
        busError();
  }
  else {
  
    /* Normal memory access */
    memory[addr] = value;
  }
}

/* Get the contents of memory location */
unsigned short getWord(unsigned int addr) {

  /* need a word and a byte */
  unsigned short word = 0;
  unsigned char lowByte;
  unsigned char highByte;

  /* Check address of word we're getting is on an even boundary */
  if (isWord(addr)) {

    /* Address in IO Space */
    if (isDeviceAddr(addr)) {
    
      // Read word from device register
      return io("read", addr, 0);
    }
    else {
      /* Normal word memory access */
    
      /* Get low order byte and high order byte */
      lowByte = getByte(addr);
      highByte = getByte(addr + 1);

      /* Modify word to first put high byte into bottom of word */
      word = word | highByte;
  
      /* Shift left to move the high byte ready for the low byte */
      word = word << 8;
  
      /* Now put low byte into the free space */
      word = word | lowByte;
  
      return word;
    }
  }
  /* Invalid access - Bus Error */
  else {
    busError();
  }

  return 0;
}

/* Set the contents of memory location */
void setWord(unsigned int addr, unsigned short newWord) {

  /* need a word and a byte */
  unsigned short word = 0;
  unsigned char lowByte;
  unsigned char highByte;

  /* Check address of word we're setting is on an even boundary */
  if (isWord(addr)) {
  
    /* Address in IO Space */
    if (isDeviceAddr(addr)) {
    
      // Write new word to device register
      io("write", addr, newWord);
    }
    else {
    
      /* Normal Memory access */

      /* Grab just the low order byte from the word */
      lowByte = newWord;
  
      /* Shift word 8 places right to kick low byte off the end and take 
  	high byte out of the bottom */
      highByte = newWord >> 8;

      setByte(addr, lowByte);
      setByte(addr + 1, highByte);
    }
  }
  /* Invalid access - Bus Error */
  else {
    busError();
  }
}

/* Initialize array of Device IO function pointers */
void initializeDeviceIO(void) {

  int i = 0;
  
  for (i = 0; i < 4096; i++)
    currentFunc[i] = 0;
}

/* Configure the address range used by an IO device */
char configureDevice(unsigned short (*deviceFunction) (const char *, unsigned int, unsigned short), unsigned int startAddr, unsigned int endAddr) {

  // Ensure device range start and end are valid words
  if ((!isWord(startAddr)) || (!isWord(endAddr)))
    return 0;

  // Convert device access start address to an address in IO space
  startAddr = getDeviceAddr(startAddr);
  endAddr = getDeviceAddr(endAddr);
  
  // Loop through IO space addresses for this device to setup IO functions
  int i = 0;
  for (i = startAddr; i <= endAddr; i++)
    currentFunc[i] = deviceFunction;
    
  return 1;
}

/* Print the contents of memory locations in a set range as bytes or words */
void printMem(const char * type, unsigned short startAddr, unsigned short endAddr) {

  int i = 0;

  printf("Memory status:\n");
  printf("\n");

  // Select byte or word mode based on user input
  if (strcmp(type, "byte") == 0) {

    printf("Byte view\n");
    printf("-----------\n");

    //Loop through addresses from start to end address
    for (i = startAddr; i <= endAddr; i++) {

      printf("Address: %o", i);
      printf("\t[ %d ]\n", getByte(i));

    }
  }
  else if (strcmp(type, "word") == 0) {

    printf("Word view\n");
    printf("-----------\n");

    // Print column headings
    printf("\t\tHigh Byte  Low Byte\n");

    //Loop through addresses from start to end address
    for (i = startAddr; i <= endAddr; i += 2) {

      printf("Address: %o", i + 1);
      printf("\t[ %d ]", getByte(i + 1));
      printf("\t[ %d ]", getByte(i));
      printf("\t\tAddress: %o\n", i);

    }
  }

}

/* Memory Mapped IO */
unsigned short io(const char * command, unsigned int addr, unsigned short newWord) {

  unsigned int deviceAddr = 0;

  /* Convert 16-bit address to 18-bit IO address adding extra bits*/
  deviceAddr = addr | 0600000;

  // Mapping of Device addresses to IO function addresses
  unsigned short ioAddr = getDeviceAddr(deviceAddr);

  // Perform IO request if function has been registered
  if (*currentFunc[ioAddr] != 0) {

    if (strcmp(command, "read") == 0) {
      /* Read from device using address specified */
      return currentFunc[ioAddr]("read", deviceAddr, 0);
    }
    else if (strcmp(command, "write") == 0) {
      /* Write to device using address specified */
      currentFunc[ioAddr]("write", deviceAddr, newWord);
    }
  }
  /* Device invalid - Interrupt Bus Error */
  else
    busError();
    
  return 0;
}

/* Generate Bus Error interrupt */
void busError(void) {

    // Generate new processor status word
    struct Psw word;
    unsigned short newWord = 0;
    
    word.C = 0;
    word.V = 0;
    word.Z = 0;
    word.N = 0;
    word.T = 0;
    word.unused = 0;
    word.regSet = 0;
    // Unused on PDP11/03
    word.previousMode = 0;
    word.currentMode = 0;
    word.priority = 7;

    // Conver PSW to short to use its value
    newWord = pswToShort(word);

    // Store new PSW in location 0000006 (Interrupt Vector +2W)
    setWord(0000006, newWord);
    
    // Store Interrupt Service Routine location at Interrupt Vector address 0000004
    setWord(0000004, 0);

    // Request Bus and Interrupt CPU
    busRequest(&NPR, 0000004);
}
