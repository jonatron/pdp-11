int mount(char *f, int drive);
void unMount (int drive);
void rk_init(void);
void rk_boot_hello(void);
void bootStrap(void);


#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define u16 unsigned short

#define TRUE 1
#define FALSE 0

//#define RK_TESTING TRUE
#define RK_TESTING FALSE

/**
 * RK05 specs
 */
enum RK05 {
WORDS = 256,					//words/sector
SECTORS = 12,					//sectors/track
SURFACES = 2,					//surface/drive
TRACKSS = 203,					//tracks/surface
//TRACKSD = (TRACKSS * SURFACES),			//tracks/drive
DRIVES = 8,					//drives/controller
SIZE = (TRACKSS * SURFACES * SECTORS * WORDS),  	//words/drive
MAXSIZE = ((int) (SIZE / sizeof (short))),	//words/memory
IntVector = 220
};

#define IntLevel BR5

/**
 * Bot field declarations
 */
struct RKCS {
	u16 GO:1;//bit 0
	u16 FUN:3;//bit 3-1
	u16 MEX:2;//bit 4-5
	u16 IDE:1;//bit 6
	u16 RDY:1;//bit 7
	u16 SSE:1;//bit 8
	u16 FMT:1;//bit 10
	u16 INH_BA:1;//bit11
	u16 SCP:1;//bit 13
	u16 HE:1;//bit 14
	u16 ERROR:1;//bit 15
};

struct RKER {
	u16 WCE:1;//bit 0
	u16 CSE:1;//1
	u16 NXS:1;//2
	u16 NXC:1;
	u16 NXD:1;
	u16 TE:1;
	u16 DLT:1;
	u16 NXM:1;
	u16 PGE:1;
	u16 SKE:1;
	u16 WLO:1;
	u16 OVR:1;
	u16 DRE:1;
};

struct RKDS {
	u16 SC:4;//bits 0-3
	u16 SCSA:1;//bi 4
	u16 WPS:1;//bit 5
	u16 RWSR:1;//bit 6
	u16 DRY:1;//bit 7
	u16 SOK:1;//bit 8
	u16 SIN:1;//bit 9
	u16 DRU:1;//bit 10
	u16 RK05:1;//bit 11
	u16 DPL:1;//bit 12
	u16 ID:3;//bits 13-15
};


/**
 * Register Addresses
 */
enum RegisterAddresses {
RKDSA = 0777400,
RKERA = 0777402,
RKCSA = 0777404,
RKWCA = 0777406,
RKBAA = 0777410,
RKDAA = 0777412,
RKDBA = 0777416
};

/**
 * RKDA register shifting (how to find parts)
 */
enum RKDA {
RDKA_SSECT = 0, //how far do we shift to find the sector
RKDA_ASECT = 017, //& for sector
RKDA_SSUR = 4, //shift for sur 
RKDA_ASUR = 01,
RKDA_STRAC = 5,
RKDA_ATRAC = 0377,
RKDA_SDRV = 13,
RKDA_ADRV = 07
};

/**
*RKCS Functions
*Speak for themselves
*/
enum RKCSF {
CONTROL_RESET = 0,
WRITE = 1,
READ = 2,
WRITE_CHECK = 3,
SEEK = 4,
READ_CHECK = 5,
DRIVE_RESET = 6,
WRITE_LOCK = 7
};

/**
 * macro definitions
 */
#define GET_SECT(x)	(((x) >> RDKA_SSECT) & RKDA_ASECT)	//what sector are we reading from
#define GET_SUR(x) (((x) >> RKDA_SSUR) & RKDA_ASUR)		//which surface
#define GET_TRACK(x) (((x) >> RKDA_STRAC) & RKDA_ATRAC)		//which track
//#define GET_FUNC(x)	(((x) >> 1) & RKCS_FUN)		//what are we doing (read, write etc.)?
#define GET_DRIVE(x) (((x) >> RKDA_SDRV) & RKDA_ADRV)
#define SURFACESIZE (TRACKSS * SECTORS * WORDS)			//how big is one surface?

#define FWORD(t, s) (((t * SECTORS) + s) * WORDS)		//calculate where to read if on surface 0
#define FWORD1(t, s) (FWORD(t, s) + SURFACESIZE)			//calculate where to read if on surface 1
#define SEEK(t, s, su) ((su) == 0 ? FWORD(t, s) : FWORD1(t, s))
#define RKER_HARD 0177740
#define CONTROLLER 1
#define NO_OF_DRIVES 8

#define REVERSE_SECT(x) ((x/WORDS) % TRACKSS)

/**
 * Drive strucy
 */
struct DRIVE {
	FILE *file;
	u16 busy;
	u16 WPS;
};

/**
 * Function prototypes
 */
void controlReset();
void rk_read();
void rk_write();
void writeCheck();
void clearErrors();
void clearCont();
void controlReset();
void go();
u16 readRKDS();
u16 readRKER();
u16 readRKCS();
void writeRKCS(u16);
u16 rkIO(const char * command, unsigned int addr, u16 value);
int isBigEndian();
u16 convertEndian(u16);
void done();
void bootStrap(void);
void readFMT(void);

