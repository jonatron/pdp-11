#include "rk.h"
#include <stdio.h>
#include "CoreMem.h"
#include "Cpu.h"

extern short rkwc;	//word count
extern u16 rkba;	//bus address register
extern u16 rkda;	//disk address
extern u16 rkdb;
extern struct RKER rker;
extern struct RKCS rkcs;
extern struct RKDS rkds;
//struct DRIVE drives[NO_OF_DRIVES];
extern u16 surface;
extern u16 track;
extern u16 sector;
extern int ba;
extern int drive;
extern struct DRIVE drives[NO_OF_DRIVES];


extern int rkcsgo;

/**
 * Test the macro definitions
 */
int macroTests() {

	u16 value = 4121;
	
	
	if (GET_SUR(value) != 1) return FALSE;
	
	if (GET_SECT(value) != 9) return FALSE;
	
	if (GET_TRACK(value) != 128) return FALSE;
	
	if (SEEK(GET_TRACK(value), GET_SECT(value), GET_SUR(value)) != 1019136) return FALSE;

	if (GET_DRIVE(value) != 0) return FALSE;

	return TRUE;
}

/**
 * check that the clearCont function works, getting ready for the next
 */
int test_clearCont() {
	if (rkcs.RDY != TRUE) return FALSE;//should start at true
	
	//might as well test clear con't while here, and rkcs needs to be 0
	clearCont();
	if (getWord(RKCSA) != 0) return FALSE;
	
	return TRUE;
}

/**
 * Test the setting of the flags in the RKCS bitfield
 */
int test_rkcsSet() {
	//rkcs.GO
	rkcsgo = 0;
	setWord(RKCSA, 01);
	if (rkcs.GO != TRUE) return FALSE;
		
	if (rkcs.FUN != CONTROL_RESET) return FALSE; //should be 0 (control reset) at start
	setWord(RKCSA, 02);
	if (rkcs.FUN != WRITE) return FALSE;
	setWord(RKCSA, 04);
	if (rkcs.FUN != READ) return FALSE;
	setWord(RKCSA, 06);
	if (rkcs.FUN != WRITE_CHECK) return FALSE;
	setWord(RKCSA, 010);
	if (rkcs.FUN != SEEK) return FALSE;
	setWord(RKCSA, 012);
	if (rkcs.FUN != READ_CHECK) return FALSE;
	setWord(RKCSA, 014);
	if (rkcs.FUN != DRIVE_RESET) return FALSE;
	setWord(RKCSA, 016);
	if (rkcs.FUN != WRITE_LOCK) return FALSE;
	
	setWord(RKCSA, 020);
	if (rkcs.MEX != 01) return FALSE;
	setWord(RKCSA, 060);
	if (rkcs.MEX != 03) return FALSE;
	
	setWord(RKCSA, 0100);
	if (rkcs.IDE != TRUE) return FALSE;
	
	setWord(RKCSA, 0200);//should be read only
	if (rkcs.RDY == TRUE) return FALSE;
	
	setWord(RKCSA, 0400);
	if (rkcs.SSE != TRUE) return FALSE;
	
	setWord(RKCSA, 02000);
	if (rkcs.FMT != TRUE) return FALSE;

	setWord(RKCSA, 04000);
	if (rkcs.INH_BA != TRUE) return FALSE;
	
	//should not be set by setword, so values should not be TRUE
	setWord(RKCSA, 020000);
	if (rkcs.SCP == TRUE) return FALSE;
	
	setWord(RKCSA, 040000);
	if (rkcs.HE == TRUE) return FALSE;
	
	setWord(RKCSA, 0100000);
	if (rkcs.ERROR == TRUE) return FALSE;
	
	
	return TRUE;
}

/**
 * Test the retrieval of the RKCS flags
 */
int test_rkcsGet() {
	clearCont();
	
	//Go should be write only
	rkcs.GO = TRUE;
	if (getWord(RKCSA) == 01) return FALSE;
	rkcs.GO = FALSE;
	
	rkcs.FUN = CONTROL_RESET;
	if (getWord(RKCSA) != 0) return FALSE;
	rkcs.FUN = WRITE;
	if (getWord(RKCSA) != 02) return FALSE;
	rkcs.FUN = READ;
	if (getWord(RKCSA) != 04) return FALSE;
	rkcs.FUN = WRITE_CHECK;
	if (getWord(RKCSA) != 06) return FALSE;
	rkcs.FUN = SEEK;
	if (getWord(RKCSA) != 010) return FALSE;
	rkcs.FUN = READ_CHECK;
	if (getWord(RKCSA) != 012) return FALSE;
	rkcs.FUN = DRIVE_RESET;
	if (getWord(RKCSA) != 014) return FALSE;
	rkcs.FUN = WRITE_LOCK;
	if (getWord(RKCSA) != 016) return FALSE;
	
	rkcs.FUN = 0;//clear fun

	rkcs.MEX = 01;
	if (getWord(RKCSA) != 020) return FALSE;
	rkcs.MEX = 02;
	if (getWord(RKCSA) != 040) return FALSE;
	rkcs.MEX = 03;
	if (getWord(RKCSA) != 060) return FALSE;
	rkcs.MEX = 0;
	
	rkcs.IDE = TRUE;
	if (getWord(RKCSA) != 0100) return FALSE;
	rkcs.IDE = FALSE;
	
	rkcs.RDY = TRUE;
	if (getWord(RKCSA) != 0200) return FALSE;
	rkcs.RDY = FALSE;
	
	rkcs.SSE = TRUE;
	if (getWord(RKCSA) != 0400) return FALSE;
	rkcs.SSE = FALSE;
	
	rkcs.FMT = TRUE;
	if (getWord(RKCSA) != 02000) return FALSE;
	rkcs.FMT = FALSE;
	
	rkcs.INH_BA = TRUE;
	if (getWord(RKCSA) != 04000) return FALSE;
	rkcs.INH_BA = FALSE;
	
	rkcs.SCP = TRUE;
	if (getWord(RKCSA) != 020000) return FALSE;
	rkcs.SCP = FALSE;
	
	rkcs.HE = TRUE;
	if (getWord(RKCSA) != 040000) return FALSE;
	rkcs.HE = FALSE;
	
	rkcs.ERROR = TRUE;
	if (getWord(RKCSA) != 0100000) return FALSE;
	rkcs.ERROR = FALSE;
	
	//set RKCS back to it's initial state (RDY true)
	rkcs.RDY = TRUE;
				
	return TRUE;
}

/**
 * Make sure we can write to the RKWC register with a positive value,
 * and that this is converted to the right 2's complement value
 */
int RKWCtest() {
	u16 value;
	value = -128;
	setWord (RKWCA, value);
	if (rkwc != -128) {
		printf("wc 1 failed\n");
		return FALSE;
	}
	
	value = -1024;
	setWord (RKWCA, value);
	if (rkwc != -1024) {
		printf("wc 2 failed\n");
		return FALSE;
	}
	
	value = -21760;
	setWord (RKWCA, value);
	if (rkwc != -21760) {
		printf("wc 3 failed\n");
		return FALSE;
	}
	return TRUE;
}

/**
 * Test the retrieval of the RKER flags
 */
int RKERtest() {
	rker.WCE = TRUE;
	if (getWord(RKERA) != 01) return FALSE;
	rker.WCE = FALSE;
	
	rker.CSE = TRUE;
	if (getWord(RKERA) != 02) return FALSE;
	rker.CSE = FALSE;
	
	rker.NXS = TRUE;
	if (getWord(RKERA) != 040) return FALSE;
	rker.NXS = FALSE;
	
	rker.NXC = TRUE;
	if (getWord(RKERA) != 0100) return FALSE;
	rker.NXC = FALSE;
	
	rker.NXD = TRUE;
	if (getWord(RKERA) != 0200) return FALSE;
	rker.NXD = FALSE;
	
	rker.TE = TRUE;
	if (getWord(RKERA) != 0400) return FALSE;
	rker.TE = FALSE;
	
	rker.DLT = TRUE;
	if (getWord(RKERA) != 01000) return FALSE;
	rker.DLT = FALSE;
	
	rker.NXM = TRUE;
	if (getWord(RKERA) != 02000) return FALSE;
	rker.NXM = FALSE;
	
	rker.PGE = TRUE;
	if (getWord(RKERA) != 04000) return FALSE;
	rker.PGE = FALSE;
	
	rker.SKE = TRUE;
	if (getWord(RKERA) != 010000) return FALSE;
	rker.SKE = FALSE;
	
	rker.WLO = TRUE;
	if (getWord(RKERA) != 020000) return FALSE;
	rker.WLO = FALSE;
	
	rker.OVR = TRUE;
	if (getWord(RKERA) != 040000) return FALSE;
	rker.OVR = FALSE;
	
	rker.DRE = TRUE;
	if (getWord(RKERA) != 0100000) return FALSE;
	rker.DRE = FALSE;
	
	return TRUE;
}

/**
 * Test to make sure that if we try to do a format, when not reading or writing, the correct error flag is set
 */
int test_programError1() {
	rkcsgo = TRUE;
	//just need to set rkcs, we should fail before anything else is needed
	setWord(RKCSA, 02013);//format and read check
	if (!rker.PGE || !rkcs.ERROR || !rkcs.HE) {
		//printf("PGE: %d, ERROR: %d, HE: %d\n", rker.PGE, rkcs.ERROR, rkcs.HE);
		return FALSE;
	}
	//printf("PGE: %d, ERROR: %d, HE: %d\n", rker.PGE, rkcs.ERROR, rkcs.HE);
	controlReset();//clear all the erors
	return TRUE;

}

/**
 * See if a drive exists (has a file been opened for it?)
 */
int test_driveCheck() {
	rkcsgo = TRUE;
	setWord(RKDAA, 0160000);//try reading drive 7 (does not exist, only 0 is used for testing)
	setWord (RKCSA, 013);//start a read check (should stop as disk not present)
	
	if (!rker.NXD || !rkcs.ERROR || !rkcs.HE) {
		return FALSE;
	}
	controlReset();
	return TRUE;
}

/**
 * set write protection
 */
int test_writeLock() {
	setWord(RKDAA, 0);
	setWord(RKCSA, 017);
	if (!drives[0].WPS) {
		return FALSE;
	}
	return TRUE;//don't clear write protect, we'll try a wrute next
}


//write to a drive that is write protected
int test_writeLockWrite() {
	setWord(RKCSA, 03);
	if (!rker.WLO || !rkcs.ERROR || !rkcs.HE) {
		return FALSE;
	}
	controlReset();//clear the error
	return TRUE;
}

//since the drive is locked, might as well test drive reset to clear the lockout
int test_driveReset() {
	setWord(RKCSA, 015);
	if (drives[0].WPS || track != 0 || sector != 0 || surface != 0) {
		return FALSE;
	}
	return TRUE;
}

//try to access a sector over 11
int test_highSector() {
	setWord(RKDAA, 014);//access sector 12
	setWord(RKCSA, 03);//try a write
	if (!rker.NXS || !rkcs.ERROR || !rkcs.HE) {
		return FALSE;
	}
	controlReset();
	return TRUE;
}

int test_highTrack() {
	setWord(RKDAA, 6496);//access track 203
	setWord(RKCSA, 03);//try a write
	if (!rker.NXC || !rkcs.ERROR || !rkcs.HE) {
		return FALSE;
	}
	controlReset();
	return TRUE;
}

//test a readfmt on s0, cy3, sector 10
int testReadFMT1() {
	rkcsgo = TRUE;
	u16 ba = 01000;
	u16 headers[10] = {3, 3, 4, 4, 4, 4, 4, 4, 4, 4};
	setWord(RKWCA, -10);
	setWord(RKBAA, ba);
	setWord(RKDAA, 0152);
	
	setWord(RKCSA, 02005);
	int i;
	for (i = 0; i < 10; i++) {
		if (getWord(ba) != headers[i]) {
			return FALSE;
		}
		ba+=2;
	}
	return TRUE;
}

//test readfmt at s0, cyl 202, sec 7(expect to roll over to surface 1)
int testReadFMT2() {
	rkcsgo = TRUE;
	u16 ba = 01000;
	u16 headers[10] = {202, 202, 202, 202, 202, 0, 0, 0, 0, 0};
	setWord(RKWCA, -10);
	setWord(RKBAA, ba);
	setWord(RKDAA, 014507);
	
	setWord(RKCSA, 02005);
	int i;
	for (i = 0; i < 10; i++) {
		if (getWord(ba) != headers[i]) {
			return FALSE;
		}
		ba+=2;
	}
	if (surface != 1)
		return FALSE;
	return TRUE;
}

//test readfmt at s1, cyl 202, sec 7(expect to get overflow, so need to check errors)
int testReadFMT3() {
	rkcsgo = TRUE;
	u16 ba = 01000;
	u16 headers[10] = {202, 202, 202, 202, 202, 0, 0, 0, 0, 0};
	setWord(RKWCA, -10);
	setWord(RKBAA, ba);
	setWord(RKDAA, 014517);
	
	setWord(RKCSA, 02005);
	int i;
	for (i = 0; i < 10; i++) {
		if (getWord(ba) != headers[i]) {
			return FALSE;
		}
		ba+=2;
	}
	if (!rker.OVR && !rkcs.ERROR && !rkcs.HE)//check the right errors have been set
		return FALSE;
	return TRUE;
}

//test read at start
int testRead1() {
	rkcsgo = TRUE;
	u16 ba = 02000;
	u16 words[256];
	words[0] = 0;
	int i;
	for (i = 0; i < 255; i++) {
		words[i+1] = i;
	}
	setWord(RKWCA, -256);
	setWord(RKBAA, ba);
	setWord(RKDAA, 0);
	setWord(RKCSA, 05);
	for (i = 0; i < 256; i++) {
		//printf("Mem: %d, expected %d\n", getWord(ba), words[i]);
		if (getWord(ba) != words[i])
			return FALSE;
		ba += 2;
		
	}
	return TRUE;
}

//test read at end (overflow)
int testRead2() {
	u16 words[260];
	words[0] = 4871;
	int i;
	for (i = 1; i < 260; i++) {
		if (i >=256) {
			words[i] = 0;
			continue;
		}
		words[i] = i-1;
	}

	int ba = 01000;
	setWord(RKBAA, ba);
	setWord(RKWCA, -512);
	setWord(RKDAA, 014533);
	setWord(RKCSA, 05);
	//printMem("word", 01000, 02000);
	for (i = 0; i < 260; i++) {
		if (getWord(ba) != words[i])
			return FALSE;
		ba += 2;
	}
	if (!rker.OVR && !rkcs.ERROR && !rkcs.HE)//check the right errors have been set
		return FALSE;
	return TRUE;
}

//test writing 256 words at beggining
int testWrite1() {
	setWord(RKCSA, 01);
	int wba = 01000;
	setWord(RKWCA, -256);
	setWord(RKBAA, wba);
	setWord(RKDAA, 01);
	
	int i;
	for (i = 0; i < 256; i++) {
		setWord (wba, 10);
		wba += 2;
	}
	wba = 01000;
	
	setWord(RKCSA, 03);//write and go
	
	//read back to check
	
	int rba = 02000;
	setWord(RKWCA, -256);
	setWord(RKDAA, 01);//disk back to beggining
	setWord(RKBAA, rba);
	setWord(RKCSA, 05);//read and go
	
	//check the words were written
	for (i = 0; i < 256; i++) {
		if (getWord(wba) != getWord(rba))
			return FALSE;
		wba += 2;
		rba += 2;
	}
	return TRUE;
}


//test writing <256 words (at beggining), so should write in 0s
//this function assumes the read function works okay
int testWrite2() {
	setWord(RKCSA, 01);
	int wba = 01000;
	setWord(RKDAA, 02);
	setWord(RKWCA, -100);
	setWord(RKBAA, wba);
	
	int i;
	for (i = 0; i < 100; i++) {
		setWord (wba, 10);
		wba += 2;
	}
	wba = 01000;
	
	setWord(RKCSA, 03);//write and go
	
	//read back to check
	
	int rba = 02000;
	setWord(RKWCA, -256);
	setWord(RKDAA, 02);//disk back to beggining
	setWord(RKBAA, rba);
	setWord(RKCSA, 05);//read and go
	
	//check the words were written
	for (i = 0; i < 100; i++) {
		if (getWord(wba) != getWord(rba))
			return FALSE;
		wba += 2;
		rba += 2;
	}
	//check that the rest of what was read is 0
	for (; i < 256; i++) {
		if (getWord(rba) != 0)
			return FALSE;
	}
	
	return TRUE;
}



int rkdsCheck() {
	rkds.SC = 11;//secotr 11
	if (getWord(RKDSA) != 013) return FALSE;
	rkds.SC = 0;
	
	rkds.SCSA = TRUE;
	if (getWord(RKDSA) != 040) return FALSE;
	rkds.SCSA = FALSE;
	
	rkds.WPS = TRUE;
	if (getWord(RKDSA) != 0100) return FALSE;
	rkds.WPS = FALSE;
	
	rkds.RWSR = TRUE;
	if (getWord(RKDSA) != 0200) return FALSE;
	rkds.RWSR = FALSE;
	
	rkds.DRY = TRUE;
	if (getWord(RKDSA) != 0400) return FALSE;
	rkds.DRY = FALSE;

	rkds.SOK = TRUE;
	if (getWord(RKDSA) != 0400) return FALSE;
	rkds.SOK = FALSE;
	
	rkds.SIN = TRUE;
	if (getWord(RKDSA) != 01000) return FALSE;
	rkds.SIN = FALSE;
	
	rkds.DRU = TRUE;
	if (getWord(RKDSA) != 02000) return FALSE;
	rkds.DRU = FALSE;
	
	rkds.RK05 = TRUE;
	if (getWord(RKDSA) != 04000) return FALSE;
	rkds.RK05 = FALSE;
	
	rkds.DPL = TRUE;
	if (getWord(RKDSA) != 010000) return FALSE;
	rkds.DPL = FALSE;
	
	rkds.ID = TRUE;
	if (getWord(RKDSA) != 070000) return FALSE;
	rkds.ID = FALSE;
	
	return TRUE;
}

int rk_run_tests() {
	
	if (!macroTests()) {
		printf("Macro Testing failed\n");
		return FALSE;
	}
	if (!test_clearCont()) {
		printf("RKCS clearCont failed\n");
		return FALSE;
	}
	if (!test_rkcsSet()) {
		printf("rkcs set failed\n");
		return FALSE;
	}
	if (!test_rkcsGet()) {
		printf("rkcs get failed\n");
		return FALSE;
	}
	if (!RKWCtest()) {
		printf("RKWCtest failed\n");
		return FALSE;
	}
	if (!RKERtest()) {
		printf("RKERtest failed\n");
		return FALSE;
	}
	if (!RKDSCheck()) {
		printf("RKDStest failed\n");
		return FALSE;
	}
	
	if (!test_programError1()) {
		printf("PGE test failed");
		return FALSE;
	}
	
	if (!test_driveCheck()) {
		printf("drive check test failed");
		return FALSE;
	}
	if (!test_writeLock()) {
		printf("write lock test failed");
		return FALSE;
	}
	if (!test_writeLockWrite()) {
		printf("write lock write test failed");
		return FALSE;
	}
	if (!test_driveReset()) {
		printf("drive reset test failed");
		return FALSE;
	}
	if (!test_highSector()) {
		printf("high sector test failed");
		return FALSE;
	}
	if (!test_highTrack()) {
		printf("high track test failed");
		return FALSE;
	}
	
	
	if (!testReadFMT1()) {
		printf("testFMT 1 failed\n");
		return FALSE;
	}
	if (!testReadFMT2()) {
		printf("Test read fmt 2 failed");
		return FALSE;
	}
	if (!testReadFMT3()) {
		printf("Test read fmt 3 failed");
		return FALSE;
	}
	if (!testRead1()) {
		printf("Test Read 1 failed\n");
		return FALSE;
	}
	if (!testRead2()) {
		printf("Test read 2 failed\n");
		return FALSE;
	}
	if (!testWrite1()) {
		printf("Test write 1 failed\n");
		return FALSE;
	}
	
	
	if (!testWrite2()) {
		printf("Test write 2 failed\n");
		return FALSE;
	}
	
	
	
	printf ("All tests passed\n");
	return TRUE;
}

