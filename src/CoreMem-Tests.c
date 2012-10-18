/* Tests for the CoreMem module */
#include <stdio.h>
#include <string.h>
#include "cut.h"
#include "CoreMem.h"

/* IO Testing variables */
unsigned short TR1;
unsigned short TR2;

/* IO Testing function */
unsigned short testIO(const char *, unsigned int, unsigned short);

void __CUT_BRINGUP__Explode( void ) {

  printf("Running Tests for Module CoreMem.c\n");

  initializeDeviceIO();

  setByte(0000372, 230);
  setWord(0000374, 540);
  setWord(0000375, 674);
  
  printf("Running IO Tests for Module CoreMem.c\n");
  
  /* Configure two test devices - (0160002 - 0160006) and (0160012 - 0160014) */
  configureDevice(testIO, 0760002, 0760006);
  configureDevice(testIO, 0760012, 0760014);
}

/* Memory related Tests */

void __CUT__TestA( void ) {
  ASSERT(getByte(0000372) == 230, "Error storing a byte in memory location 0000372.");
}

void __CUT__TestB( void ) {
  ASSERT(getWord(0000374) == 540, "Error storing a word in memory location 0000374.");
}

void __CUT__TestC( void ) {
  ASSERT(getWord(0000375) != 674, "Storing a word in an invalid memory location should fail.");
}

/* IO Device configuration Tests */

void __CUT__TestD( void ) {
  /* Attempt to configure a device with an invalid address range (Invalid word boundary) */
  unsigned char testResult = configureDevice(testIO, 0760001, 0760002);
  ASSERT(testResult == 0, "Using an invalid word address to configure a device should fail.");
}

/* IO Reading and Writing Tests */
void __CUT__TestE( void ) {
  /* Attempt to write to a device that has not been configured */
  setWord(0160000, 255);
  ASSERT(getWord(0160000) != 255, "Writing to a device that has not been configured should fail.");
}
void __CUT__TestF( void ) {
  /* Attempt to write a byte to a device that has been configured */
  setByte(0160003, 255);
  ASSERT(getByte(0160003) == 255, "Error writing a byte to a device register at address 0160003.");
}
void __CUT__TestG( void ) {
  /* Attempt to write to a device that has been configured */
  setWord(0160002, 255);
  ASSERT(getWord(0160002) == 255, "Error writing to a device register at address 0160002.");
}
void __CUT__TestH( void ) {
  /* Attempt to write to a device that has been configured using 18-bit addressing */
  setWord(0760012, 255);
  ASSERT(getWord(0760012) == 255, "Error writing to a device register using 18-bit addressing.");
}
void __CUT__TestI( void ) {
  /* Attempt to write an invalid byte value to a device that has been configured */
  setByte(0160004, 256);
  ASSERT(getByte(0160004) != 256, "Writing a byte value larger than 8 bits to a device should fail.");
}
void __CUT__TestJ( void ) {
  /* Attempt to write an invalid value to a device that has been configured */
  setWord(0160004, 65536);
  ASSERT(getWord(0160004) != 65536, "Writing a value larger than 16 bits to a device should fail.");
}

void __CUT_TAKEDOWN__Explode( void ) {
  printf("");
}

/* Additional functions defined for Testing */

unsigned short testIO(const char * command, unsigned int addr, unsigned short value) {
  
  // Perform read operation
  if (strcmp(command, "read") == 0) {
    
    return TR1;
  }
  // Perform write operation
  else if (strcmp(command, "write") == 0) {
  
    // Write to a dummy IO device
    TR1 = value;
    
    return NULL;
  }
  
  return NULL;
}
