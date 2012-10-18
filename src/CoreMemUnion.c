/*
 * File CoreMem.c
 *
 * Memory of the simulated machine
 *  Includes accessor and modifier functions
 */
 
#include <stdio.h>
#include <string.h>
#include "CoreMem.h"

/* Start simple char array */
unsigned char memory[MEMSIZE];

/* Try a union*/
union memory {

  unsigned char byte[MEMSIZE]; /* Access one byte of memory */
  unsigned short word[MEMSIZE]; /* Access one word of memory */
} PDPMem;

/* Get the contents of memory location */
unsigned char getByte(unsigned short addr) {

  unsigned char value = 0;

  if ((addr <= MEMSIZE) && (addr >= 0))
    value = memory[addr];

  return value;
}

/* Get the contents of memory location */
unsigned char setByte(unsigned short addr, unsigned char value) {

  if ((addr <= MEMSIZE) && (addr >= 0)) {
  	
  	memory[addr] = value;
  	return 1;
  }
  
  return 0;  
}

/* Get the contents of memory location */
unsigned short getWord(unsigned short addr) {

  /* need a word and a byte */
  unsigned short word = 0;	/* 0000000000000 */
  unsigned char lowByte;
  unsigned char highByte;

  /* Get low order byte and high order byte */
  lowByte = getByteU(addr);
  highByte = getByteU(addr + 1);

  /* Modify word to first put high byte into bottom of word */
  //word = word + highByte;
  word = word | highByte;
  
  /* Shift left to move the high byte ready for the low byte */
  word = word << 8;
  
  /* Equivalent of doing an OR operation on word and high byte, putting result in word */
  
  /* Now put low byte into the free space */
  word = word | lowByte;

  return word;
}

/* Set the contents of memory location */
unsigned char setWord(unsigned short addr, unsigned short newWord) {

  /* need a word and a byte */
  unsigned short word = 0;	/* 0000000000000 */
  unsigned char lowByte;
  unsigned char highByte;

  lowByte = newWord;
  
  highByte = newWord >> 8;
  
//  highByte = word;

  /* Grab just the low order byte from the word by OR operation on the new word, only the low bytes
  are taken out and stored in the low byte */
//  lowByte = word | newWord;
  
  /* Low byte is out now so we can set the word up
  word = */
  
  /* Shift word 8 places right to kick low byte off the end then we can take 
  	high byte out of the bottom */
  //highByte = newWord >> 8;

  if (setByteU(addr, lowByte) && setByteU(addr + 1, highByte)) {
  	return 1;
  }
  
  return 0;
}

/* Get the contents of memory location */
unsigned char getByteU(unsigned short addr) {

  unsigned char value = 0;

  if ((addr <= MEMSIZE) && (addr >= 0))
    value = PDPMem.byte[addr];

  return value;
}

/* Set the contents of memory location */
unsigned char setByteU(unsigned short addr, unsigned char value) {

  if ((addr <= MEMSIZE) && (addr >= 0)) {
  	
  	PDPMem.byte[addr] = value;
  	return 1;
  }
  
  return 0;  
}

/* Get the contents of memory location */
unsigned short getWordU(unsigned short addr) {

  unsigned short value = 0;

  if ((addr <= MEMSIZE) && (addr >= 0))
    value = PDPMem.word[addr];

  return value;
}

