#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//A word is two bytes... 
enum Type {
    BYTE = 1,
    WORD = 2
};



// Struct to make a Processor status word.
typedef struct Psw {
	unsigned short C:1;
	unsigned short V:1;
	unsigned short Z:1;
	unsigned short N:1;
	unsigned short T:1;
	unsigned short priority:3;
	unsigned short unused:3;
	unsigned short regSet:1;
	unsigned short previousMode:2;
	unsigned short currentMode:2;
}Psw;



/*Linked list struct. Daisy-chain the interrupt requests.
Note: PDP-11/20 Doesn't use any priority less than BR4. Those are for the 
larger machines*/
struct InterruptNode {
	unsigned short vector;
	struct InterruptNode *next;
} *NPR, *BR7, *BR6, *BR5, *BR4, *BR3, *BR2, *BR1, *BR0;



//Prototypes for public use....

//initialise the cpu.
void initCpu(void);

//fetch PC instruction and execute it.
int fetchEx(void);                   

/*Adds the interrupt to a queue of waiting interrupts.
First parameter is the address to the pointer to the head of 
that priority queue.
Second parameter is the vector address. Just an unsigned short.
example:
    busRequest( &BR7, address );*/
void busRequest(struct InterruptNode**, unsigned short); 

/*Find the next waiting interrupt and make it the new PSW and PC if its
priority level are above the current CPU's. If it is suitable for the next
interrupt, it is removed from the list of waiting interrupts. */
void interruptCPU(void);

//convert the psw struct to unsigned short
unsigned short pswToShort(struct Psw);
