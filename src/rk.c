/**
 * Simulate the RK11 disk controller with RK05 disks.
 * Disk image files are used as the simulated disks.
 *
 * @author bw88
 */



#include "rk.h"
#include "CoreMem.h"
#include "Cpu.h"

#define CONVERT_ENDIAN(v) ((v & 255) << 8) | ((v>>8) & 255)

/**
 * Fields
 */
short rkwc = 0;	//word count
u16 rkba = 0;	//bus address register
u16 rkda = 0;	//disk address
u16 rkdb = 0;
struct RKER rker = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
struct RKCS rkcs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
struct RKDS rkds = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
struct DRIVE drives[NO_OF_DRIVES];
u16 surface = 0;
u16 track = 0;
u16 sector = 0;
int ba;
int drive;
int BigEndian;
int position;
int func;

//PC from the CPU, so the disk can boot (and set PC itself)
extern u16 reg[8];
#define PC reg[7]

#if RK_TESTING
int rkcsgo = 1;//for testing, I don't always want a function to initiate when rkcs.GO is set
#endif


/**
*Open a drive image for binary reading/writing
*@param *f The file we want to open
*@param drive What drive no do we want to mount the file on (between 0 and 7)
*/
int mount(char *f, int driveNo) {
	if (drives[driveNo].file) {
		printf ("Cannot use drive %d. Already in use. Please dismount or select another drive\n", drive);
		return FALSE;
	}
	if (driveNo >= DRIVES)
		return FALSE;
	drives[driveNo].file = fopen(f, "r+b");
	if (!drives[drive].file) {
		printf("Unable to open file %s", f);
		return FALSE;
	}
	drives[drive].busy = FALSE;
	drives[driveNo].WPS = FALSE;
	return TRUE;
	//printf ("Mounted drive %d\n", drive);
}

/**
 * Called when the sim is shutting down.
 */
void unMount (int driveNo) {
	fclose(drives[driveNo].file);
	drives[driveNo].file = NULL;
	//printf ("Drive %d dismounted\n", drive);
}

/**
 * Read from the disk image
 */
void rk_read() {
	int i;
	//printf("Position %d\n", position);
	int seek = fseek (drives[drive].file, position * sizeof(u16), SEEK_SET);
	//printf("Seek %d\n", seek);
	short wc, buffSize;
	wc = buffSize = rkwc * -1;//turn rkwc from - to +
	if ((position + wc) >= SIZE) {
		wc = SIZE - position;//set the size of wc to the maximum that can be read
	}
	if (wc && !seek) {
		u16 buffer[buffSize];
		int read = fread (buffer, sizeof(u16), wc, drives[drive].file);
		for ( ; read < buffSize; read++)
			buffer[read] = 0;
		//if increment inhibited, transfer everything to one address (so only the last one is useful)
		if (rkcs.INH_BA) {
			if (BigEndian)
				buffer[wc-1] = CONVERT_ENDIAN(buffer[wc-1]);
			setWord(ba, buffer[wc-1]);
			return;
		}
		for (i = 0; i < buffSize; i++) {
			if (BigEndian)
				buffer[i] = CONVERT_ENDIAN(buffer[i]);
			setWord(ba, buffer[i]);
			//printf("Read: %d\n", buffer[i]);
			//printf("buffer %d\n", ba);
			ba += 2;
		}
	}
}

/**
 * Write to the disk image
 */
void rk_write() {
	//printf("Writing\n");
	int i;
	int seek = fseek (drives[drive].file, position * sizeof(u16), SEEK_SET);
	short wc = rkwc * -1;//turn rkwc from - to +
	if ((position + wc) >= SIZE) {
		wc = SIZE - position;//set the position to the maximum size we can write
	}
	if (wc && !seek) {
		u16 buffer[wc];
		for (i = 0; i < wc; i++) {
			u16 word = getWord(ba);
			if (BigEndian) {
				word = CONVERT_ENDIAN(word);
			}
			buffer[i] = word;
			if (!rkcs.INH_BA) {
				ba += 2;
			}
		}
		int wl = fwrite (buffer, sizeof(u16), wc, drives[drive].file);
		if (wl != wc) {
			//not written properly, set some errors
			return;
		}

		//fill the rest of the buffer with 0's. This is in case we
		//wc overflows before the end of a sector
		if ((wc % WORDS) != 0) {
			int need = WORDS - (wc % WORDS);
			u16 fill[WORDS] = { 0 };//we're only ever going to need 255 at most, and need a constant, so WORDS
			wl = fwrite (fill, sizeof(u16), need, drives[drive].file);
		}
		

		//if (wl != (wc * sizeof(u16))) {
			//not written properly, set some errors
		//	return;
		//}
	}
}

/**
 * Perform a write check
 */
void writeCheck() {
	int i;
	int seek = fseek (drives[drive].file, position * sizeof(u16), SEEK_SET);
	short wc = rkwc * -1;
	if ((position + wc) >= SIZE) {
		wc = SIZE - position;
	}
	if (wc && !seek) {
		u16 buffer[wc];
		int re = fread (buffer, sizeof(u16), wc, drives[drive].file);
		if (re != (wc * sizeof(u16))) {
			//not read properly, set some errors
			return;
		}
		for (i = 0; i < wc; i++) {
			if (BigEndian) {
				buffer[i] = CONVERT_ENDIAN(buffer[i]);
			}
			if (getWord(ba) == buffer[i]) {
				if (!rkcs.INH_BA)
					ba += 2;
			}
			else {
				rker.WCE = TRUE;
				if (rkcs.SSE)
					return;
			}
		}
	}
}

/**
 * Do a read format. We copy wc header word to memory.
 * Given that the images do not contain header words, only
 * data, the header work (cylinder number) is calculated from
 * the start position, and transfered to memory accordingly.
 * No disk IO actually takes place.
 */
void readFMT() {
	short wc = rkwc * -1;
	int i;
	for (i = 0; i < wc; i++) {
		setWord(ba, track);
		//printf("Track is %d, sector is %d\n", track, sector);
		if (!rkcs.INH_BA)
			ba+=2;
		if (sector == (SECTORS-1)) {
			sector = 0;
			if (track == (TRACKSS-1) && surface == 0) {
				surface = 1;
				track = 0;
			}
			else if (track == (TRACKSS-1) && surface == 1) {//we've overrun, set the error, and stop
				rker.OVR = TRUE;
				return;
			}
			else {
				track++;
			}
			continue;
		}
		sector++;
	}
}

/**
 * Set All error bits to FALSE
 */
void clearErrors() {
	rker.WCE = FALSE;
	rker.CSE = FALSE;
	rker.NXS = FALSE;
	rker.NXC = FALSE;
	rker.NXD = FALSE;
	rker.TE = FALSE;
	rker.DLT = FALSE;
	rker.NXM = FALSE;
	rker.PGE = FALSE;
	rker.SKE = FALSE;
	rker.WLO = FALSE;
	rker.OVR = FALSE;
	rker.DRE = FALSE;
}

/**
 * Set all bit in RKCS to FALSE
 */
void clearCont() {
	rkcs.GO = FALSE;
	rkcs.FUN = FALSE;
	rkcs.MEX = FALSE;
	rkcs.IDE = FALSE;
	rkcs.RDY = FALSE;
	rkcs.SSE = FALSE;
	rkcs.FMT = FALSE;
	rkcs.INH_BA = FALSE;
	rkcs.SCP = FALSE;
	rkcs.HE = FALSE;
	rkcs.ERROR = FALSE;
}

/**
 * Set all bits in RKDS to FALSE
 */
void rkdsReset() {
	rkds.SC = FALSE;
	rkds.SCSA = FALSE;
	rkds.WPS = FALSE;
	rkds.RWSR = FALSE;
	rkds.DRY = FALSE;
	rkds.SOK = FALSE;
	rkds.SIN = FALSE;
	rkds.DRU = FALSE;
	rkds.RK05 = FALSE;
}

/**
 * Reset all the registers, except RKDS, and set bit 7 of RKCS.
 * Would normally stop a function in progress, however, we are
 * not introducing concurrency for the disk, so it doesn't matter
 */
void controlReset() {
	rkda = FALSE;
	rkba = FALSE;
	clearErrors();
	clearCont();
	rkdsReset();
	rkcs.RDY = TRUE;
}

/**
 * Let's do a command
 */
void go() {
	func = rkcs.FUN;
	//are we resetting?
	if (func == CONTROL_RESET) {
		//printf("Reset\n");
		controlReset();
		return;
	}
	
	//if not, clear some bits
	rker.WCE = rker.CSE = FALSE; //clear soft errors
	if (readRKER() == 0)//if not errors
		rkcs.ERROR = FALSE;//make sure rkcs error bit is cleare
	rkcs.SCP = FALSE;//clear search complete
	rkcs.RDY = FALSE;//clear ready
	
	//are we formatting while not reading o writing? We shouldn't
	if (rkcs.FMT && func != WRITE && func != READ) {
		//printf("PGE Error\n");
		rker.PGE = TRUE;
		done();
		return;
	}
	
	//find what drive we want
	drive = GET_DRIVE(rkda);
	//make sure the drive is there, and it's not above drive 8
	if (drives[drive].file == NULL) {
		rker.NXD = TRUE;
		done();
		return;
	}
	
	//is the drive busy. No reason why it shuld be, but oh well
	if (drives[drive].busy) {
		rker.DRE = TRUE;
		done();
		return;
	}
	
	//are we writing to a protected drive
	if (func == WRITE && drives[drive].WPS) {
		rker.WLO = TRUE;
		done();
		return;
	}

	//do we want to write lock the drive?
	if (func == WRITE_LOCK) {
		drives[drive].WPS = TRUE;
		done();
		if (rkcs.IDE) {
			busRequest(&IntLevel, IntVector);
		}
		return;
	}

	//are we resetting the drive? If so, set track and sect to FALSE, and perform a seek, else get from RKDA
	if (func == DRIVE_RESET) {
		drives[drive].WPS = FALSE;
		track = sector = surface =  FALSE;
		func = SEEK;
	}
	else {
		track = GET_TRACK(rkda);
		sector = GET_SECT(rkda);
		surface = GET_SUR(rkda);
		//printf("surface %d\n", surface);
	}

	//is the sector valid?
	if (sector >= SECTORS) {
		rker.NXS = TRUE;
		done();
		return;
	}

	//is the track valid?
	if (track >= TRACKSS) {
		rker.NXC = TRUE;
		done();
		return;
	}

	//are we seeking? If so, we do nothing (nothing to move)
	if (func == SEEK) {
		rkds.SCSA = TRUE;
		done();
	}//may be more to do for seek

	//assume there are now no problems with the disk, so do a function
	
	ba = (rkcs.MEX << sizeof(unsigned short)*8) + rkba;//get the 18 bit address
	
	position = SEEK (track, sector, surface);//'seek' to position on the disk we want
	//printf("check2\n");
	//printf("func: %d", func);
	drives[drive].busy = TRUE;
	switch (func) {
		case WRITE:
			if (rkcs.FMT) {
				//set format done TODO........
			}
			else {
				//printf("Write\n");
				rk_write();
			}
			break;
		case READ:
			if (rkcs.FMT) {
				readFMT();
			}
			else {
				rk_read();
			}
			break;
		case WRITE_CHECK://write and write format do the same thing, with the exception of 
			writeCheck();
			break;
		case READ_CHECK:
			break;//going to assume a read check is useless, can't see a way of implementing using the disk image.
	}

	rkba = ba & 0177777;//set rkba
	rkwc = 0;//rolls xover anyway
	drives[drive].busy = FALSE;
	done();
}

/**
 * Called when we're finished. Set the done bits, in RKCS
 * check if there are errors, and set the error bit in RKCS
 * and generate an interrupt if the interrupt bit is set
 */
void done() {
	rkcs.RDY = TRUE;
	rkcs.SCP = TRUE;
	rkds.SC = REVERSE_SECT(rkda);
	rkds.RWSR = TRUE;
	rkds.DRY = TRUE;
	rkds.SOK = TRUE;
	if (readRKER()) {
		rkcs.ERROR = TRUE;
	}
	if (readRKER() & RKER_HARD) {
		rkcs.HE = TRUE;
	}
	if (rkcs.IDE) {
	if (func == SEEK || func == CONTROL_RESET) {
		rkcs.SCP = TRUE;
		rkds.ID = drive;
	}
	 	busRequest(&IntLevel, IntVector);
	}
	
}

/**
 * Convert the RKDS bitfield into an unsigned short, and return
 */
u16 readRKDS() {
	u16 tmp = 0;
	tmp |= rkds.SC;
	tmp |= (rkds.SCSA << 4);
	tmp |= (rkds.WPS << 5);
	tmp |= (rkds.RWSR << 6);
	tmp |= (rkds.DRY << 7);
	tmp |= (rkds.SOK << 8);
	tmp |= (rkds.SIN << 9);
	tmp |= (rkds.DRU << 10);
	tmp |= (rkds.RK05 << 11);
	tmp |= (rkds.DPL << 12);
	tmp |= (rkds.ID << 13);
	return tmp;
}

/**
 * Convert the RKER bitfield into an unsigned short, and return
 */
u16 readRKER() {
	u16 tmp = 0;
	tmp |= rker.WCE;
	tmp |= (rker.CSE << 1);
	tmp |= (rker.NXS << 5);
	tmp |= (rker.NXC << 6);
	tmp |= (rker.NXD << 7);
	tmp |= (rker.TE << 8);
	tmp |= (rker.DLT << 9);
	tmp |= (rker.NXM << 10);
	tmp |= (rker.PGE << 11);
	tmp |= (rker.SKE << 12);
	tmp |= (rker.WLO << 13);
	tmp |= (rker.OVR << 14);
	tmp |= (rker.DRE << 15);
	return tmp;
}

/**
 * Convert the RKCS bitfield into an unsigned short, and return
 */
u16 readRKCS() {
	u16 tmp = 0;
	tmp |= rkcs.FUN << 1;
	tmp |= rkcs.MEX << 4;
	tmp |= rkcs.IDE << 6;
	tmp |= rkcs.RDY << 7;
	tmp |= rkcs.SSE << 8;
	tmp |= rkcs.FMT << 10;
	tmp |= rkcs.INH_BA << 11;
	tmp |= rkcs.SCP << 13;
	tmp |= rkcs.HE << 14;
	tmp |= rkcs.ERROR << 15;
	return tmp;
}

/**
 * Take the new value for RKCS, and set the bits that are writeable
 */
void writeRKCS(u16 value) {
	rkcs.GO = value & 1;
	rkcs.FUN = (value >> 1) & 7;
	rkcs.MEX = (value >> 4) & 3;
	rkcs.IDE = (value >> 6) & 1;
	rkcs.SSE = (value >> 8) & 1;
	rkcs.FMT = (value >> 10) & 1;
	rkcs.INH_BA = (value >> 11) & 1;
}

/**
 * Give the memory access to the registers for this device
 *	
 * If RKCS is written, and bit 0 is set, this function will call go to begin a new opertion
 */
u16 rkIO(const char * command, unsigned int addr, u16 value) {
	switch (addr) {
		//rkds read only
		case RKDSA:
			return readRKDS();
			
		//rker read only
		case RKERA:
			return readRKER();
			
		//rkcs read and write
		case RKCSA:
			if (strcmp(command, "read") == 0) {
				//printf("RKCS read\n");
				return readRKCS();
			}
			else if (strcmp(command, "write") == 0) {
				writeRKCS(value&017777);
				#if RK_TESTING
					if (rkcsgo) {
						if (rkcs.GO) {
							go();
						}
					}
				#else
					if (rkcs.GO) {
						go();
					}
				#endif
			}
			break;
			
		//rkwc write and write (or write only, depending on source)
		case RKWCA:
			if (strcmp(command, "write") == 0) {
				rkwc = value;
			}
			else {
				return rkwc;
			}
			break;
			
		//rkba read and write (or write only, depending on source)
		case RKBAA:
			if (strcmp(command, "write") == 0) {
				rkba = value;
			}
			else {
				return rkba;
			}
			break;
			
		//rkda write only
		case RKDAA:
			rkda = value;
			break;
	}
	return 0;
}

/**
 * Used in conjunction with the hello.dsk image to boot the hello world
 * program from disk, rather than hard coded in CPU
 */
void rk_boot_hello() {
	PC = 01020;
	setWord (RKWCA, -25);//set word count
	setWord (RKBAA, 512);//set bus address
	setWord (RKDAA, 0);//start at the beggining
	setWord (RKCSA, 05);
}

/**
 * The main bootstrap.
 * Set the CSR to read, and go, and rkwc to -256
 */
void bootStrap() {
	PC = 000000;
	
	setWord (RKWCA, -256);//set word count
	setWord (RKCSA, 05);//set read instruction, and set go bit to initiate a function
	//printMem("word", 00000, 01000);
}

/**
* Register the device with the Memory's IO space
*/
void rk_init() {
	configureDevice( rkIO, RKDSA, RKDBA);
	BigEndian = isBigEndian();
	rkcs.RDY = TRUE;
}

/**
 * Check if the machine we are working on is a big-endian one.
 * If it is, set a global variable so we knwo to convert when reading or writing
 */
int isBigEndian() {
	int no = 1;
	char *chk = (char *)&no;

	if (chk[0] == 1) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

