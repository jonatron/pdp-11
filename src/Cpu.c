#include "CoreMem.h"
#include "terminal.h"
#include "Cpu.h"

//Used for testing...
//#include <curses.h>
//extern WINDOW *iface;


#define PC reg[7]    //Program counter
#define SP reg[6]    //Stack pointer
#define SOURCE 07700
#define DEST 077
#define BYTE_BITS(x) (x & 0xFF)


Psw psw;    //Processor Status Word variable
unsigned short reg[8];    //General Register Array.
unsigned short IR;        // Instruction Register


//Prototypes for private use.
unsigned short setMem(short, short value, enum Type);
short getMem(short, enum Type);
void setPsw(unsigned short);
unsigned short cpuStatus(const char *, unsigned int, unsigned short);


//Tests
void testCpuStatus(void);
void testInterrupt(void);
void printBusReq(void);
void toBin(unsigned short); 
void testHello(void);
void testInstruction(void);


//initialise the cpu.
void initCpu() {
	SP = 0200;
	configureDevice(cpuStatus,0777776,0777776);

	//Test method calls.
	//testHello();
	//testInstruction();
}



//fetch/excecute cycle.
int fetchEx(void) {

	interruptCPU();      //Check for interrupts.

	IR = getWord(PC);    // Instruction Placed in the Instruction Register.

	PC+=2;    // Program counter is incremented by 2 to point to the next Word location.
	
	
	//Instruction Decoding:

	//halt
	if (IR == 0) {
		return 1;
	}

	// Branch on microtest (BUT).
	//Check to see if it is a double operand instruction.
	else if ((IR & 070000) != 0) {
		
		unsigned short op = IR >> 12;
		
		unsigned short dst = IR & 077;
		unsigned short src = (IR & SOURCE) >> 6;
	    
		signed short srcVal;
		signed char srcValByte;
		
		signed short result;
		signed char resultByte;
		
		signed short oldVal;
		
		signed short dstVal;
		signed char dstValByte;

		//printf("Double OP\n");
		//waddstr(iface,"\nIn Double operand\n");
		//wrefresh(iface);

		switch(op) {
			
		case 01:    //MOV
			//printf("%o\tMOV\n",PC);
			srcVal = getMem(src,WORD);
			setMem(dst,srcVal,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if (srcVal < 0) {
				psw.N = 1;
			}
			if (srcVal == 0) {
				psw.Z = 1;
			}
			break;

		case 011:    //MOV(B)
			//printf("%o\tMOV B",PC);
			srcValByte = getMem(src,BYTE);
			setMem(dst,srcValByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if (srcValByte < 0) {
				psw.N = 1;
			}
			if (srcValByte == 0) {
				psw.Z = 1;
			}
			break;

		case 012:    //CMP(B)
			srcValByte = getMem(src,BYTE);
			dstValByte = getMem(dst,BYTE);
			resultByte = srcValByte - dstValByte;
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			psw.C = 1;
			if (resultByte < 0)
				psw.N = 1;
			if (resultByte == 0)
				psw.Z = 1;
			if (((resultByte * dstValByte) > 0) && ((dstValByte * srcValByte) < 0))
				psw.V = 1;
			if ((unsigned char)srcValByte < (unsigned char)dstValByte)
				psw.C = 0;
			break;

		case 02:    //CMP
			srcVal = getMem(src,WORD);
			dstVal = getMem(dst,WORD);
			result = srcVal - dstVal;
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			psw.C = 1;
			if (result < 0)
				psw.N = 1;
			if (result == 0)
				psw.Z = 1;
			if (((result * dstVal) > 0) && ((dstVal * srcVal) < 0))
				psw.V = 1;
			if ((unsigned short)srcVal < (unsigned short)dstVal)
				psw.C = 0;
			break;

		case 06:    //ADD
			srcVal = getMem(src,WORD);
			result = srcVal + getMem(dst,WORD);
			oldVal = setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			psw.C = 0;
			if (result < 0)
				psw.N = 1;
			if (result == 0)
				psw.Z = 1;
			if ( oldVal * srcVal < 0)
				psw.V = 1;
			if (result < oldVal)
				psw.C = 1;
			break;

		case 016:    //SUB
			srcVal = getMem(src,WORD);
			dstVal = getMem(dst,WORD);
			result = dstVal - srcVal;
			oldVal = setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			psw.C = 1;
			if (result < 0)
				psw.N = 1;
			if (result == 0)
				psw.Z = 1;
			if (((result * dstVal) > 0) && ((dstVal * srcVal) < 0))
				psw.V = 1;
			if (result < oldVal)
				psw.C = 0;
			break;

			//Logical
		case 03:  //BIT
			dstVal = getMem(dst,WORD);
			srcVal = getMem(src,WORD);
			result = dstVal & srcVal;
			psw.V = 0;
			psw.N = 0;
			psw.Z = 0;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			break;

		case 013:  //BIT(B)
			dstValByte = getMem(dst,BYTE);
			srcValByte = getMem(src,BYTE);
			resultByte = dstValByte & srcValByte;
			psw.V = 0;
			psw.N = 0;
			psw.Z = 0;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			break;

		case 04:   //BIC
			srcVal = getMem(src,WORD);
			dstVal = getMem(dst,WORD);
			result = dstVal & (~srcVal);
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			break;

		case 014:   //BIC(B)
			srcValByte = getMem(src,BYTE);
			dstValByte = getMem(dst,BYTE);
			resultByte = dstValByte & (~srcValByte);
			setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			break;

			case 05:   //BIS
			srcVal = getMem(src,WORD);
			dstVal = getMem(dst,WORD);
			result = dstVal | srcVal;
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			break;

		case 015:   //BIS(B)
			srcValByte = getMem(src,BYTE);
			dstValByte = getMem(dst,BYTE);
			resultByte = dstValByte | srcValByte;
			setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			break;

			// Break if not a double operand.
		default:
			; //continue
		}
	}
        
	else if ((IR & 074000) == 074000){
		unsigned short op = IR >> 8;
		unsigned int regNum = (IR & 0700) >> 6;
		unsigned char dst = IR & 0xFF;
		signed short dstVal;
		unsigned int regVal;
		signed short result;

		//waddstr(iface,"\nIn XOR\n");
		//wrefresh(iface);

		switch(op) {
		case 0170:    //XOR
			dstVal = getMem(dst,WORD);
			regVal = reg[regNum];
			result = dstVal ^ regVal;
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			break;

		default:
			break; 
		}
	}


	//Branches
	else if ((IR & 0103400)) {
		unsigned short op = IR >> 8;
		char offset = IR & 0xFF;
		//printf("Branch\n");
		//waddstr(iface,"\nIn Branches\n");
		//wrefresh(iface);
				
		switch(op) {
			//Branches
		case 01:    //BR
			//printf("%o\tBR",PC);
			PC = (PC) + (2 * offset);
			//printf("\tnew PC: %o\n",PC);
			//printf("%c",reg[0]);
			break;

		case 03:    //BEQ	
			//printf("%o\tBEQ",PC);
			if (psw.Z == 1) {
				PC = (PC) + (2 * offset);
			}
			//printf("\tnew PC: %o\n",PC);
			break;

		case 0200:   //BPL
			//printf("%o\tBPL\tpsw.N: %o\n",PC,psw.N);
			if (psw.N == 1) {
				//printf("psw.N from within: %o\n",psw.N);
				PC = (PC) + (2 * offset);
			}
			//printf("new PC: %o\n",PC);
			break;
		
		default:
			break; //continue
		}
	}


	// Then check to see if it is a single operand instruction.
	else if ((IR & 0107700) && ((IR & 070000) == 0)) {
		unsigned short op = IR >> 6;
		unsigned short dst = IR & 077;
		
		signed short preV;
		signed char preVByte;

						
		signed short result;
		signed char resultByte;
 
		//printf("single op\n");
		//waddstr(iface,"\nIn Single Operand\n");
		//wrefresh(iface);
       
		switch(op) { 
			//General
		case 050: //CLR
			//printf("CLR\n");
			setMem(dst, 0, WORD);
			psw.N = 0;
			psw.V = 0;
			psw.C = 0;
			psw.Z = 1;
			break;

		case 01050: //CLR(B)
			//printf("CLR\(B\)\n");
			setMem(dst, 0, BYTE);
			psw.N = 0;
			psw.V = 0;
			psw.C = 0;
			psw.Z = 1;
			break;

		case 051: //COM
			//printf("COM\n");
			result = ~getMem(dst,WORD);
			setMem(dst,result,WORD);
			psw.V = 0;
			psw.Z = 0;
			psw.C = 1;
			psw.N = 0;
			if(result == 0)
				psw.Z = 1;
			if(result < 0)
				psw.N = 1;
			break;

		case 01051: //COM(B)
			resultByte = ~getMem(dst,BYTE);
			setMem(dst,resultByte,BYTE);
			psw.V = 0;
			psw.Z = 0;
			psw.C = 1;
			psw.N = 0;
			if(resultByte == 0)
				psw.Z = 1;
			if(resultByte < 0)
				psw.N = 1;
			break;

		case 052: //INC
			result = getMem(dst,WORD) + 1;
			preV = setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			if(preV == 077777)
				psw.V = 1;
			break;

		case 01052: //INC(B)
			resultByte = getMem(dst,BYTE) + 1;
			preVByte = setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			if(preVByte == 0177)
				psw.V = 1;
			break;

		case 053: //DEC
			result = getMem(dst,WORD) - 1;
			preV = setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			if(preV == 0100000)
				psw.V = 1;
			break;

		case 01053: //DEC(B)
			resultByte = getMem(dst,BYTE) - 1;
			preVByte = setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			if(preVByte == 0200)
				psw.V = 1;
			break;

		case 054: //NEG
			result = getMem(dst,WORD) * -1;
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			psw.C = 1;
			if(result < 0)
				psw.N = 1;
			if(result == 0) {
				psw.Z = 1;
				psw.C = 0;
			}
			if(result == 0100000)
				psw.V = 1;
			break;

		case 01054: //NEG(B)
			resultByte = getMem(dst,BYTE) * -1;
			setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			psw.C = 1;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0) {
				psw.Z = 1;
				psw.C = 0;
			}
			if(resultByte == 0200)
				psw.V = 1;
			break;

		case 057: //TST
			//printf("TST\n");
			result = getMem(dst,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			psw.C = 0;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			break;

		case 01057: //TST(B)
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			psw.C = 0;
			resultByte = getMem(dst,BYTE);
			if(resultByte < 0) {
				psw.N = 1;
			}
			if(resultByte == 0)
				psw.Z = 1;
			break;


			//Shift and Rotate
		case 062: //ASR
			preV = getMem(dst,WORD);
			result = preV >> 1;
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.C = preV & 1;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			psw.V = psw.N ^ psw.C;
			break;

		case 01062: //ASR(B)
			preVByte = getMem(dst,BYTE);
			resultByte = preVByte >> 1;
			setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			psw.C = preVByte & 1;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			psw.V = psw.N ^ psw.C;
			break;

		case 063:   //ASL
			preV = getMem(dst,WORD);
			result = preV << 1;
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.C = 0;
			if(preV < 0)
				psw.C = 1;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			psw.V = psw.N ^ psw.C;
			break;

		case 01063: //ASL(B)
			preVByte = getMem(dst,BYTE);
			resultByte = preVByte << 1;
			setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			psw.C = 0;
			if(preVByte < 0)
				psw.C = 1;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			psw.V = psw.N ^ psw.C;
			break;

		case 060: //ROR
			preV = getMem(dst,WORD);
			result = (unsigned short) preV / 2;
			if(psw.C == 1)
				result |= (1<<15);
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.C = preV & 1;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			psw.V = psw.N ^ psw.C;
			break;

		case 01060: //ROR(B)
			preVByte = getMem(dst,BYTE);
			resultByte = (unsigned char) preVByte / 2;
			if(psw.C == 1)
				resultByte |= (1<<7);
			setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			psw.C = preVByte & 1;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			psw.V = psw.N ^ psw.C;
			break;

		case 061: //ROL
			preV = getMem(dst,WORD);
			result = (unsigned short) preV << 1;
			if(psw.C == 1)
				result |= 1;
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			if(preV < 0)
				psw.C = 1;
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			psw.V = psw.N ^ psw.C;
			break;

		case 01061: //ROL(B)
			preVByte = getMem(dst,BYTE);
			resultByte = (unsigned char) preVByte << 1;
			if(psw.C == 1)
				resultByte |= 1;
			setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;
			if(preVByte < 0)
				psw.C = 1;
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			psw.V = psw.N ^ psw.C;
			break;

		case 03:  //SWAB
			preV = getMem(dst,WORD); 
			result = (unsigned short) (preV & 0xFF) << 8;
			result = (unsigned short) result + (((unsigned short) preV & 0xFF00) >> 8);
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;
			psw.V = 0;
			psw.C = 0;
			if((result & 0200) != 0)
				psw.N = 1;
			if((result & 0xFF) == 0)
				psw.Z = 1;
			break;


			//Multiple precision
		case 055: //ADC
			preV = getMem(dst,WORD);
			result = preV + psw.C;
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;			
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			if((preV == 077777) & (psw.C == 1)) {
				psw.V = 1;
			} else {
				psw.V = 0;
			}
			if(((unsigned short) preV == 0177777) & (psw.C == 1)) {
				psw.C = 1;
			} else {
				psw.C = 0;
			}
			break;

		case 01055: //ADC(B)
			preVByte = getMem(dst,BYTE);
			resultByte = preVByte + psw.C;
			setMem(dst,result,BYTE);
			psw.N = 0;
			psw.Z = 0;			
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			if((preVByte == 0177) & (psw.C == 1)) {
				psw.V = 1;
			} else {
				psw.V = 0;
			}
			if(((unsigned char) preVByte == 0377) & (psw.C == 1)) {
				psw.C = 1;
			} else {
				psw.C = 0;
			}
			break;

		case 056: //SBC
			preV = getMem(dst,WORD);
			result = preV - psw.C;
			setMem(dst,result,WORD);
			psw.N = 0;
			psw.Z = 0;			
			if(result < 0)
				psw.N = 1;
			if(result == 0)
				psw.Z = 1;
			if((unsigned short) preV == 0100000) {
				psw.V = 1;
			} else {
				psw.V = 0;
			}
			if(((unsigned short) preV == 0) & (psw.C == 1)) {
				psw.C = 1;
			} else {
				psw.C = 0;
			}
			break;

		case 01056: //SBC(B)
			preVByte = getMem(dst,BYTE);
			resultByte = preVByte - psw.C;
			setMem(dst,resultByte,BYTE);
			psw.N = 0;
			psw.Z = 0;			
			if(resultByte < 0)
				psw.N = 1;
			if(resultByte == 0)
				psw.Z = 1;
			if((unsigned char) preVByte == 0200) {
				psw.V = 1;
			} else {
				psw.V = 0;
			}
			if(((unsigned char) preVByte == 0) & (psw.C == 1)) {
				psw.C = 1;
			} else {
				psw.C = 0;
			}
			break;

		case 067: //SXT
			psw.V = 0;
			if(psw.N == 1) {
				setMem(dst,-1,WORD);
			}else {
				setMem(dst,0,WORD);
				psw.Z = 1;
			}
			break;

		default:
			break;
		}
	}
	//printf("test",reg[2], reg[4]);
	//toBin(pswToShort(psw));
	//printf("\n");

	//wprintw(iface, "\nreg2: %+ho\n", reg[2]);
	//wprintw(iface, "reg4: %+ho\n", reg[4]);
	//wrefresh(iface); //this is required to show changes

	return 0;
}


//Function to find and set a memory location based on addressing modes.
unsigned short setMem(short daf, short value, enum Type type){
    unsigned short regNumber = daf & 07;
    unsigned short mode = (daf >> 3) & 07;
	unsigned short addr;
  	unsigned short retVal;
	unsigned short x;

    switch(mode) {
        //Register
        case 0:
			retVal = reg[regNumber];
			if(type == WORD)
				reg[regNumber] = value;
			else
				reg[regNumber] = (reg[regNumber] & 0177400) + (value & 0377);
			return retVal;
			break;

        //Register Defered
        case 1:
            if(type == WORD) {
				retVal = getWord(reg[regNumber]);
                setWord(reg[regNumber], value);
			} else {
				retVal = getByte(reg[regNumber]);	
                setByte(reg[regNumber], value);
			}
			return retVal;
            break;

        //Autoincrement
        case 2:
            if((type == WORD) || (regNumber == 6) || (regNumber == 7)) {
                addr = reg[regNumber];
				retVal = getWord(addr);
  				reg[regNumber] += WORD;
				setWord(addr, value);
            } else{
                addr = reg[regNumber];
				retVal = getByte(addr);
                reg[regNumber] += WORD;
				setByte(addr, value);
            }
			return retVal;
            break;

        //Autoincrement Defered
        case 3:
            if((type == WORD) || (regNumber == 6) || (regNumber == 7)) {
				addr = getWord(reg[regNumber]);
				retVal = getWord(addr);
                reg[regNumber] = reg[regNumber] + 2;
				//printf("addr: %o\tvalue: %c\n",addr,value);
				setWord(addr,value);
            } else{
				addr = getWord(reg[regNumber]);
				retVal = getByte(addr);
                reg[regNumber] = reg[regNumber] + 1;
				setByte(addr,value);
            }
			return retVal;
            break;

        //Autodecrement
        case 4:
            reg[regNumber] -= type;
            if(type == WORD) {
                setWord(reg[regNumber], value);
				retVal = getWord(reg[regNumber]);
			} else {
                setByte(reg[regNumber], value);
				retVal = getByte(reg[regNumber]);
			}
			return retVal;
            break;

        //Autodecrement Defered
        case 5:
            reg[regNumber] -= type;
            if(type == WORD) {
                addr = getWord(reg[regNumber]);
				retVal = getWord(addr);
				setWord(addr,value);
			} else {
				addr = getWord(reg[regNumber]);
				retVal = getByte(addr);
				setByte(addr,value);
			}
			return retVal;
            break;

        //Index
        case 6:
            if(type == WORD) {
                x = getWord(PC);
				PC += 2;
				retVal = getWord(x + regNumber);
				setWord(x + regNumber, value);
			} else {
				x = getWord(PC);
				PC += 2;
				retVal = getByte(x + regNumber);
				setByte(x + regNumber, value);
			}
			return retVal;
            break;

        //Index Defered
        case 7:
            if(type == WORD){
				x = getWord(PC);
				PC += 2;
				addr = getWord(x + regNumber);
				retVal = getWord(addr);
				setWord(addr,value);
			} else {
				x = getWord(PC);
				PC += 2;
				addr = getWord(x + regNumber);
				retVal = getByte(addr);
				setByte(addr,value);
			}
			return retVal;
            break;

	default:
		return 0;
		break;
    }

}



//Function to find and get the memory location based on addressing modes.
short getMem(short daf, enum Type type) {
    unsigned short regNumber = daf & 07;
    unsigned short mode = (daf >> 3) & 07;
    unsigned short val;
	unsigned short addr;
	unsigned short op;

    switch(mode) {
        case 0:
			if(type == WORD)
				return reg[regNumber];
			else
				return (reg[regNumber] & 0xFF);
			break;
           
        //Register Defered
        case 1:
            if(type == WORD)
                return getWord(reg[regNumber]);
            else
                return getByte(reg[regNumber]);
            break;

        //Autoincrement
        case 2:
            if((type == WORD) || (regNumber == 6) || (regNumber == 7)) {
                val = getWord(reg[regNumber]);
                reg[regNumber] = reg[regNumber] + 2;
            } else{
                val = getByte(reg[regNumber]);
                reg[regNumber] = reg[regNumber] + 1;
            }
            return val;
            break;

        //Autoincrement Defered
        case 3:
            if((type == WORD) || (regNumber == 6) || (regNumber == 7)) {
                addr = getWord(reg[regNumber]);
                reg[regNumber] += 2;
				op = getWord(addr);
            } else {
				addr = getWord(reg[regNumber]);
                reg[regNumber] += 2;
				op = getByte(addr);
            }
            return op;
            break;

        //Autodecrement
        case 4:
            reg[regNumber] -= type;
            if(type == WORD)
                getWord(reg[regNumber]);
            else
                getByte(reg[regNumber]);

        //Autodecrement Defered
        case 5:
            reg[regNumber] -= 2;
            if(type == WORD)
                return getWord(getWord(reg[regNumber]));
            else
                return getByte(getWord(reg[regNumber]));
            break;

        //Index
        case 6:
            if(type == WORD)
                val = getWord(reg[regNumber] + PC);
            else
                val = getByte(reg[regNumber] + PC);
            PC += 2;
            return val;
            break;

        //Index Defered
        case 7:
            if(type == WORD)
                val = getWord(getWord(reg[regNumber] + PC));
            else
                val = getByte(getWord(reg[regNumber] + PC));
            PC += 2;
            return val;
            break;

        default:
	    return 0;
	    break;

    }
}



/*Recieve request for an interrupt and add it to corresponding priority queue.
Head is the star node for the priority queue. */
void busRequest(struct InterruptNode** headRef, unsigned short vAddress) {

	struct InterruptNode* newNode = malloc(sizeof(struct InterruptNode));

	newNode->vector = vAddress;
	newNode->next = (struct InterruptNode *) *headRef;
	*headRef = newNode;

}



/*Find the next waiting interrupt and make it the new PSW and PC if its
priority level are above the current CPU's. If it is suitable for the next
interrupt, it is removed from the list of waiting interrupts. */
void interruptCPU() {
	int i;
  	i = 8;

	for (i = 8; i >= psw.priority; i--) {
		struct InterruptNode *cur_ptr;

		switch(i) {
		case 8:
			cur_ptr = NPR;
			if(cur_ptr != NULL) {
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, (unsigned short)PC);         // put the PC on stack.
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, pswToShort(psw));            //put psw on stack.

				//get the new PSW and PC from the bus line.
				setPsw(getWord(cur_ptr->vector + 2));    //get the PSW from the vector + 1.
				PC = getWord(cur_ptr->vector);           //The new PC is the vector.
				
				//delete node.
				NPR = cur_ptr->next;
				free(cur_ptr);
				return;
			}
			break;
		
		case 7:
			cur_ptr = BR7;
			if(cur_ptr != NULL) {
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, (unsigned short)PC);         // put the PC on stack.
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, pswToShort(psw));            //put psw on stack.

				//get the new PSW and PC from the bus line.
				setPsw(getWord(cur_ptr->vector + 2));    //get the PSW from the vector + 1.
				PC = getWord(cur_ptr->vector);           //The new PC is the vector.
				
				//delete node.
				BR7 = cur_ptr->next;
				free(cur_ptr);
				return;
			}
			break;
		
		case 6:
			cur_ptr = BR6;
			if(cur_ptr != NULL) {
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, (unsigned short)PC);         // put the PC on stack.
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, pswToShort(psw));            //put psw on stack.

				//get the new PSW and PC from the bus line.
				setPsw(getWord(cur_ptr->vector + 2));    //get the PSW from the vector + 1.
				PC = getWord(cur_ptr->vector);           //The new PC is the vector.
				
				//delete node.
				BR6 = cur_ptr->next;
				free(cur_ptr);
				return;
			}	  
			break;
		
		case 5:
			cur_ptr = BR5;
			if(cur_ptr != NULL) {
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, (unsigned short)PC);         // put the PC on stack.
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, pswToShort(psw));            //put psw on stack.

				//get the new PSW and PC from the bus line.
				setPsw(getWord(cur_ptr->vector + 2));    //get the PSW from the vector + 1.
				PC = getWord(cur_ptr->vector);           //The new PC is the vector.
				
				//delete node.
				BR5 = cur_ptr->next;
				free(cur_ptr);
				return;
			}
			break;
		
		case 4:
			cur_ptr = BR4;
			if(cur_ptr != NULL) {
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, (unsigned short)PC);         // put the PC on stack.
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, pswToShort(psw));            //put psw on stack.

				//get the new PSW and PC from the bus line.
				setPsw(getWord(cur_ptr->vector + 2));    //get the PSW from the vector + 1.
				PC = getWord(cur_ptr->vector);           //The new PC is the vector.
				
				//delete node.
				BR4 = cur_ptr->next;
				free(cur_ptr);
				return;
			}
			break;
		
		case 3:
			cur_ptr = BR3;
			if(cur_ptr != NULL) {
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, (unsigned short)PC);         // put the PC on stack.
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, pswToShort(psw));            //put psw on stack.

				//get the new PSW and PC from the bus line.
				setPsw(getWord(cur_ptr->vector + 2));    //get the PSW from the vector + 1.
				PC = getWord(cur_ptr->vector);           //The new PC is the vector.
				
				//delete node.
				BR3 = cur_ptr->next;
				free(cur_ptr);
				return;
			}
			break;
		
		case 2:
			cur_ptr = BR2;
			if(cur_ptr != NULL) {
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, (unsigned short)PC);         // put the PC on stack.
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, pswToShort(psw));            //put psw on stack.

				//get the new PSW and PC from the bus line.
				setPsw(getWord(cur_ptr->vector + 2));    //get the PSW from the vector + 1.
				PC = getWord(cur_ptr->vector);           //The new PC is the vector.
				
				//delete node.
				BR2 = cur_ptr->next;
				free(cur_ptr);
				return;
			}
			break;
		
		case 1:
			cur_ptr = BR1;
			if(cur_ptr != NULL) {
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, (unsigned short)PC);         // put the PC on stack.
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, pswToShort(psw));            //put psw on stack.

				//get the new PSW and PC from the bus line.
				setPsw(getWord(cur_ptr->vector + 2));    //get the PSW from the vector + 1.
				PC = getWord(cur_ptr->vector);           //The new PC is the vector.
				
				//delete node.
				BR1 = cur_ptr->next;
				free(cur_ptr);
				return;
			}
			break;
		
		case 0:
			cur_ptr = BR0;
			if(cur_ptr != NULL) {
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, (unsigned short)PC);         // put the PC on stack.
				SP = SP - 2;                             //stack expands downwards.
				setWord(SP, pswToShort(psw));            //put psw on stack.

				//get the new PSW and PC from the bus line.
				setPsw(getWord(cur_ptr->vector + 2));    //get the PSW from the vector + 1.
				PC = getWord(cur_ptr->vector);           //The new PC is the vector.
				
				//delete node.
				BR0 = cur_ptr->next;
				free(cur_ptr);
				return;
			}
			break;
		
		default:
			break;
		}
	}		
}



//return an unsigned short based on the Psw.
unsigned short pswToShort(Psw status){
	unsigned short val;
	
	val = (status.currentMode << 14) +
		(status.previousMode << 12) +
		(status.regSet << 11) +
		(status.unused << 9) +
		(status.priority << 5) +
		(status.T << 4) +
		(status.N << 3) +
		(status.Z << 2) +
		(status.V << 1) +
		(status.C);

	return val;
}



//set the psw from an unsigned short.
void setPsw(unsigned short newPsw) {
	psw.C = newPsw & 1;
	psw.V = (newPsw & 2) >> 1;
	psw.Z = (newPsw & 4) >> 2;
	psw.N = (newPsw & 8) >> 3;
	psw.T = (newPsw & 16) >> 4;
	psw.priority = (newPsw & 224) >> 5;
	psw.unused = 0;
	psw.regSet = (newPsw & 2048) >> 11;
	psw.previousMode = (newPsw & 12288) >> 12;
	psw.currentMode = (newPsw & 49152) >> 14;
}


//Set up CPU Status address in memory.
unsigned short cpuStatus(const char * command, unsigned int addr, unsigned short value) {
	// Perform read operation
	if (strcmp(command, "read") == 0) {
		return pswToShort(psw);
	}
	// Perform write operation
	else if (strcmp(command, "write") == 0) {
		setPsw(value);
	}
	return 0;
}



//Tester method.
void testCpuStatus() {
	printf("Old PSW: ");
	toBin(pswToShort(psw));

	setWord(0777776,160);
	
	printf("\nNew PSW: ");
	toBin(pswToShort(psw));
	printf("\n");

}



//Another tester method.
void testHello() {

	PC = 01020;
	
/* 	reg[0] = 0xFFFF; */
// 	setWord(01000,06412);
/* 	setWord(01022,02000); */

    setWord(01000,042510);
	setWord(01002,046114);
	setWord(01004,020117);
	setWord(01006,047527);
	setWord(01010,046122);
	setWord(01012,020504);
	setWord(01014,005015);
	
	setWord(01016,000000);
	
	setWord(01020,0012701);
	setWord(01022,0001000);
	setWord(01024,0112100);
	setWord(01026,0001406);
	setWord(01030,0105737);
	setWord(01032,0177564);
	setWord(01034,0100375);
	setWord(01036,0110037);
	setWord(01040,0177566);
	setWord(01042,0000770);
	
   	setWord(01044,0000000);
}



//Test some instructions.
void testInstruction(){
	PC = 01000;
	reg[2] = 012340;
	reg[4] = 012345;
	psw.N = 0;
	psw.Z = 0;
	psw.V = 1;
	psw.C = 1;
	setWord(01000,05702);
}



//Test the linked list
void testInterrupt(){
	//start the stack at 0200 octal.
	SP = 0200;
	PC = 1;
	psw.priority = 4;
	
	//fake a bus error. with its psw at 006 set to priority 7
	setWord(004, 100);
	setWord(006, 224);

	//put some bus requests in. including the bus error.
	busRequest(&BR6, 004);
	busRequest(&BR5, 010);
	busRequest(&BR5, 020);


	printf("PC set to 1.\nPSW has a priority of 4.\nInterrupt on BR6 pointing to 100 and a PSW priority of 7\n\n");

	printf("\n--- BEFORE --- \nPSW: ");
	toBin(pswToShort(psw));
	printf(" PC: %d\nSTACK: ", PC);
	printMem("word",0170,0200);
	printBusReq();


	//interrupt!
	interruptCPU();	


	printf("\n--- AFTER --- \nNEW PSW: ");
	toBin(pswToShort(psw));
	printf(" PC: %d\nSTACK: ", PC);
	printMem("word",0170,0200);
	printBusReq();
}



//print the requests on the bus.
void printBusReq(){
	
 	struct InterruptNode *npr_ptr;
 	npr_ptr = NPR;

	struct InterruptNode *br7_ptr;
	br7_ptr = BR7;

	struct InterruptNode *br6_ptr;
	br6_ptr = BR6;
 	
	struct InterruptNode *br5_ptr;
	br5_ptr = BR5;

	struct InterruptNode *br4_ptr;
	br4_ptr = BR4;

	struct InterruptNode *br3_ptr;
	br3_ptr = BR3;

	struct InterruptNode *br2_ptr;
	br2_ptr = BR2;

	struct InterruptNode *br1_ptr;
	br1_ptr = BR1;

	struct InterruptNode *br0_ptr;
	br0_ptr = BR0;



	printf("\nBus Line Requests:\n");



 	while (npr_ptr != NULL) {
 		printf("NPR_%p --> ",npr_ptr);
 		npr_ptr = (struct InterruptNode *)npr_ptr->next;
 	}

	printf("\n");

	while (br7_ptr != NULL) {
 		printf("BR7_%p --> ",br7_ptr);
 		br7_ptr = (struct InterruptNode *)br7_ptr->next;
 	}
	
	printf("\n");

	while (br6_ptr != NULL) {
 		printf("BR6_%p --> ",br6_ptr);
 		br6_ptr = (struct InterruptNode *)br6_ptr->next;
 	}
	
	printf("\n");

	while (br5_ptr != NULL) {
 		printf("BR5_%p --> ",br5_ptr);
 		br5_ptr = (struct InterruptNode *)br5_ptr->next;
 	}
	
	printf("\n");

	while (br4_ptr != NULL) {
 		printf("BR4_%p --> ",br4_ptr);
 		br4_ptr = (struct InterruptNode *)br4_ptr->next;
 	}
	
	printf("\n");

	while (br3_ptr != NULL) {
 		printf("BR3_%p --> ",br3_ptr);
 		br3_ptr = (struct InterruptNode *)br3_ptr->next;
 	}
	
	printf("\n");

	while (br2_ptr != NULL) {
 		printf("BR2_%p --> ",br2_ptr);
 		br2_ptr = (struct InterruptNode *)br2_ptr->next;
 	}
	
	printf("\n");

	while (br1_ptr != NULL) {
 		printf("BR1_%p --> ",br1_ptr);
 		br1_ptr = (struct InterruptNode *)br1_ptr->next;
 	}
	
	printf("\n");

	while (br0_ptr != NULL) {
 		printf("BR0_%p --> ",br0_ptr);
 		br0_ptr = (struct InterruptNode *)br0_ptr->next;
 	}
	
	printf("\n");
}



//print to binary. stolen from jons terminal.c
void toBin(unsigned short n) {
	unsigned int i;
	i = 1<<(sizeof(n) * 8 - 1);
	while (i > 0) {
		if (n & i)
			;//waddstr(iface, "1");
		else
			;//waddstr(iface, "0");;
		i >>= 1;
	}
	//wrefresh(iface);
}
