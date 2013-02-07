#include <stdint.h>

//A word is two bytes...
enum Type {
    BYTE = 1,
    WORD = 2
};



// Struct to make a Processor status word.
typedef struct Psw {
	uint16_t C:1;
	uint16_t V:1;
	uint16_t Z:1;
	uint16_t N:1;
	uint16_t T:1;
	uint16_t priority:3;
	uint16_t unused:3;
	uint16_t regSet:1;
	uint16_t previousMode:2;
	uint16_t currentMode:2;
}Psw;



/*Linked list struct. Daisy-chain the interrupt requests.
Note: PDP-11/20 Doesn't use any priority less than BR4. Those are for the
larger machines*/
struct InterruptNode {
	uint16_t vector;
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
Second parameter is the vector address. Just an uint16_t.
example:
    busRequest( &BR7, address );*/
void busRequest(struct InterruptNode**, uint16_t);

/*Find the next waiting interrupt and make it the new PSW and PC if its
priority level are above the current CPU's. If it is suitable for the next
interrupt, it is removed from the list of waiting interrupts. */
void interruptCPU(void);

//convert the psw struct to uint16_t
uint16_t pswToShort(struct Psw);
