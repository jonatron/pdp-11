#include <stdio.h>
#include <string.h>
#include "CoreMem.h"

#define TRUE 1
#define FALSE 0

// test Registers
unsigned short LP1 = 0;
unsigned short LP2 = 0;

// test Device to test IO code
unsigned short testDevice(const char * command, unsigned int addr, unsigned short value) {

  // Perform read operation
  if (strcmp(command, "read") == 0) {
    
    return LP1;
  }
  // Perform write operation
  else if (strcmp(command, "write") == 0) {
  
    // Write to a dummy IO device
    LP1 = value;
    
    return NULL;
  }
  
  return NULL;
}

//Main test code
int main (int argc, const char * argv[]) {

	printf("");
	printf("Test Device IO\n");

	// Initialize Array of IO functions to zero
	initializeDeviceIO();
	
	if (!configureDevice(testDevice, 0777570, 0777570))
	  return 0;

	/* Set mem location 1 to 180 (10) in octal */
	setByte(0000000, 0000264);
	/* Set mem location 2 to 99 (10) in octal 143 (8) */
	setByte(0000001, 0000143);
	setWord(0000003, 0000317);
	/* Invalid device access */
	setByte(0160000, 0);
	setWord(0177570, 0000022);

/*	unsigned char testValue = getByte(0000000);
	printf("Byte 1 is: %d\n", testValue);
	unsigned char testValue2 = getByte(0000001);
	printf("Byte 2 is: %d\n", testValue2);
	printf("");
	unsigned short testValue3 = getWord(0000003);
	printf("Word value is: %d\n", testValue3);

*/

	unsigned short testValue4 = getWord(0177570);
	printf("Test Device 0777570: %d\n", testValue4);

	printf("\n");
	printMem("word", 0000000, 0000003);

        /* Test Union code */
	/* setByteU(0000000, 0000264);
	setByteU(0000001, 0000143);
	unsigned char testValue4 = getByteU(0000000);
	printf("Union - Byte value is: %d\n", testValue4);
	unsigned char testValue5 = getByteU(0000001);
	printf("Union - Byte value is: %d\n", testValue5); */

    return 0;
}

