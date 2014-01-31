#include "config.h"
#include <msp430fr5739.h>

#define LH 0
#define ALLLH 0
#define ALLLL ~ALLLH

struct StenoStats {
	unsigned long volatile mc; // main entrances
	unsigned long volatile rchrd; // raw chord(s) overlayed
	unsigned long volatile ic; // interrupt entrances
	unsigned long volatile rc; // reconfigure entrances
	unsigned long volatile wk; // bad wakeup
};

typedef struct StenoStats StenoStats;

// 4kb/1kc log for testing
#define LOG_MAX 1024

struct StenoLog {
	unsigned int nc; // next chord location
	unsigned int lc; // last sent chord
	unsigned char i2nch; // last sent sub byte
	unsigned char flags; // log status
	unsigned char active_flags; // signs of life
	unsigned char resrv;
	unsigned int volatile log[LOG_MAX]; // bulk log
};

typedef struct StenoLog StenoLog;

struct StenoFeature {
	unsigned long pins;
	void (*onSetup)();
	void (*onFlagsWake)();
	void (*onInterrupt)();
	void *ivectors;
	unsigned char id;
	unsigned char flags;
	unsigned char sleep_bits;
	unsigned char pmm_bits;
	
};

typedef struct StenoFeature StenoFeature;

#define I2CP (BIT6|BIT7)
#define UART0 (BIT0|BIT1)
#define UART1 (BIT5|BIT6)

// exclude i2c for now
#define PADA_M (~I2CP)

// define PADA_M (~0x0)
// exclude uart 0 & 1 for now
#define PADB_M ~(BITF|BITE|BITD|BITC|BITB|BITA|UART0|UART1)


// ids
#define	CACHE		1
#define GPIOKEYS	2
#define GEMINIPR	4
#define I2CHID		8
#define DEBUG		16

// FLAGS
#define	CHREADY		1
#define	STRT		2
#define	R2NOTIFY	4
#define	I2CCSEND	8
// define	DEBUG		16
#define VALID		32 // valid (cache) entry

typedef enum {
#ifdef HAVE_GPIO
gpio,
#endif
#ifdef HAVE_GEMENI
gemeni,
#endif
end
} MNUM;


