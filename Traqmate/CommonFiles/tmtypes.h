#ifndef TMTYPE_H
#define TMTYPE_H

// This is the data format revision number in the format 
#define DATAREV		8		// sixth release - with gear ratios and stored predictive laps
#define CURRENTYEAR	15		// 2015

// compilation controls
#define PRINTDISPLAY

// Part of sensorunit.c and displayunit.c, Traqmate product software.
// 11/10/2003
// Author: GAStephens
// Revised: 15 July 2004 - changed data storage format to handle user, config data
//			15 January 2005 - changed data storage format for compression, laptimes, io data collection
//			15 November 2005 - added io stuff
//
// This file contains the global definitions and function declarations.
//
// Flash memory map
//		N = number of memory pages, each 528 bytes in length, currently = 4096
//		S = number of data collection sessions allowed, currently = 21
//		D = number of pages of GPS and Accelerometer data starting at memory bottom
//		I = number of pages of i/o data collected starting at top of memory
//
// Page 0				= Index Page, contains headers for all data collection sessions
// Page 1				= User Page, contains information and names for drivers and cars
// Page 2				= Track Page, contains information and names for tracks
// Page 3				= IO Page, contains information and names for io points
// Page 4 to (4+D-1)	= Data Pages, format changes based on sampling rate
// Page (N-S-I) to (N-S)= I/O Data Pages, format changes based on which i/o points are enabled
// Page (N-S+1) to N	= Lap Time Pages, one for each session
//
// Note that D grows upward in memory and I grows downward. Must always check to ensure no collision.

// universal data elements
typedef unsigned char u08;
typedef unsigned int u16;
typedef unsigned long u32;
typedef char s08;
typedef int s16;
typedef long s32;
typedef union {unsigned int i; unsigned char c[2];} WORD;		// used for access to each byte

//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F02x
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <boolean.h>
#include <c8051f120.h>
#include <dataflash.h>
#include <message.h>
#include <opcode.h>

sfr16 DP       = 0x82;                 // data pointer
sfr16 ADC0     = 0xbe;                 // ADC0 data
sfr16 ADC0GT   = 0xc4;                 // ADC0 greater than window
sfr16 ADC0LT   = 0xc6;                 // ADC0 less than window
sfr16 RCAP2    = 0xca;                 // Timer2 capture/reload
sfr16 RCAP3    = 0xca;                 // Timer3 capture/reload
sfr16 RCAP4    = 0xca;                 // Timer4 capture/reload
sfr16 TMR2     = 0xcc;                 // Timer2
sfr16 TMR3     = 0xcc;                 // Timer3
sfr16 TMR4     = 0xcc;                 // Timer4
sfr16 DAC0     = 0xd2;                 // DAC0 data
sfr16 DAC1     = 0xd2;                 // DAC1 data
sfr16 PCA0CP5  = 0xe1;                 // PCA0 Module 5 capture
sfr16 PCA0CP2  = 0xe9;                 // PCA0 Module 2 capture
sfr16 PCA0CP3  = 0xeb;                 // PCA0 Module 3 capture
sfr16 PCA0CP4  = 0xed;                 // PCA0 Module 4 capture
sfr16 PCA0     = 0xf9;                 // PCA0 counter
sfr16 PCA0CP0  = 0xfb;                 // PCA0 Module 0 capture
sfr16 PCA0CP1  = 0xfd;                 // PCA0 Module 1 capture

//-----------------------------------------------------------------------------
// Software Downloading Definitions
//-----------------------------------------------------------------------------

#define CODE_PAGE_SIZE		1024		// x132 microprocessor has 1024 byte flash pages
#define CODE_DOWNLOAD_SIZE	512			// code is downloaded 512 bytes at a time

#define BOOTSTRAP_ADDR	0xFE00			// where bootstrap is located in code space
#define LAST_CODE_PAGE	0xFC00			// address of start of last 1024 flash page
//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------
#define GPS_PULSE_WID	10L				// GPS pulse width in microseconds

#define GPS_RATE		4				// GPS reporting rate in Hertz
#define GPS_MS			(1000/GPS_RATE)	// number of ms between GPS reports
#define GPS_GOOD		3				// must have 3 satellites to be "in coverage"

#define SAMPLE_RATE	160					// THIS AFFECTS COMM TIMEOUT!!
										// ADC record rate in hz (must be 10x INT_DEC)

#define FREQ_SAMPLE_RATE	256000		// counting freq for RPM samples

// data input definitions				// upper two bits of digitals define the type of interface
										// these definitions are the actual polarity measured at the port pin
#define TRAQDATALITE_ENABLED	0x01	// Bit 0 = HIGH, Bit 1 = LOW   	// 2 digital, 4 analog, 1 rpm
#define TRAQTACH_ENABLED		0x03	// Bit 0 = HIGH, Bit 1 = HIGH	// 1 digital, 1 rpm
#define TRAQUSB_ENABLED			0x02	// Bit 0 = LOW, Bit 1 = HIGH	// 1 digital, 1 rpm, (formerly ChaseCam) TraqData USB or LANC
#define TACHORDATA				0x03	// used as a mask to determine if either interface is connected
#define TRAQDATA_PRO_ENABLED	0x03	// Bit 0 = HIGH, Bit 1 = HIGH  	// 2 digital, 4 analog, 1 rpm. Pro if D3 also HIGH

#define MINLANC			0x01			// lowest number for a LANC config
//#define DEFAULTLANC		0x05			// default to new-style Sony HD camcorder
#define DEFAULTLANC		0x01			// default to automatic (for viewing in Traqview) V3.70
#define MAXLANC			0x06			// highest number for a LANC config

// synchronization values, min value = 1
// note, if in traqstudio, video is ahead of data, increase this number causing data to start recording later
// video playing early in traqstudio is because video recording overall started too late
//
#define SKIPCYCLES				4		// number of GPS pulses to skip at beginning of session before recording

// button debounce parameters
#define DEBOUNCE_NORMAL_MSEC	60		// debounce value of normal buttons in msec
#define DEBOUNCE_ONOFF_MSEC		1500	// 1.5 seconds
#define DEBOUNCE_NORMAL		((u16) ((long) DEBOUNCE_NORMAL_MSEC * (long) SAMPLE_RATE / 1000L))	// clock ticks to debounce for record mode
#define DEBOUNCE_ONOFF		((u16) ((long) DEBOUNCE_ONOFF_MSEC * (long) SAMPLE_RATE / 1000L))	// clock ticks to debounce for turn off

// ubx buffer sizes
#define	UBXTXSIZE			64		// max length of UBX transmit packet
#define UBXRXSIZE			220		// max length of UBX receive packet NAV-SVINFO

#define GPSRXSIZE		UBXRXSIZE
#define GPSTXSIZE		UBXTXSIZE

// smbus buffer sizes
#define SMBTXSIZE			16			// max length of a transmit packet over smbus
#define SMBRXSIZE			16			// max length of a receive packet over smbus

#define PCMSGBAUD		115200L			// Baud rate to talk to PC through UART
#define PCUSBMSGBAUD	230400L			// Baud rate to talk to PC through USB
#define DUMSGBAUD		230400L			// Baud rate between SU and DU
#define CMDLINEBAUD		9600L			// Baud rate for command line processing

#define CRYSTALFREQ		24576000L			// crystal frequency

#ifdef EXTCLK1
#define SYSCLK			(4L*22118400L)		// 22 MHz x 4
#endif
#ifdef EXTCLK2
#define SYSCLK			(5L*20000000L)		// 20 MHz crystal x 5 = 100 MHz
#endif
#ifdef EXTCLK4
#define SYSCLK			(CRYSTALFREQ)		// 24.576 Native clock speed
#endif
#ifdef EXTCLK5
#define SYSCLK			(2L*CRYSTALFREQ)	// 50 MHz
#endif
#ifdef EXTCLK6
#define SYSCLK			(3L*CRYSTALFREQ)	// 75 MHz
#endif
#ifdef EXTCLK3
#define SYSCLK			(4L*CRYSTALFREQ)	// 100 Mhz
#endif
#ifdef INTCLK
#define SYSCLK			(4L*24500000L)		// Internal Oscillator x 4
#endif

// Dataflash constants
#define MSGDATA			530				// max size data transmitted in a message
#define MSGSIZE			(MSGDATA + MSG_WRAPPER + 2)	// each packet is MSGSIZE bytes + wrapper + CRC
#define NAMELEN			16				// length of text string for driver and track
#define NUMSESSIONS		21				// 528 / 19 = 21 sessions
#define NUMDRIVERS		12				// 12 individual drivers allowed
#define NUMCARS			12				// 12 individual cars allowed
#define NUMTRACKS		16				// 16 tracks allowed
#define NUMGEARS		8				// 8 gears per car
#define IONAMELEN		12				// length of text string for input name
#define NUMDIGITALS		7				// 7 digital inputs
#define NUMANALOGS		4				// 4 analog inputs
#define NUMFREQS		2				// 2 frequency inputs
#define NUMDIGOUTS		4				// 4 digital outputs defined
#define NUMINPUTBYTES	(1+NUMANALOGS+NUMFREQS)		// number of bytes needed to store the inputs

// definitions for TraqData breakout box
#define NUMTDDIGITALS	2				// 2 digital inputs
#define NUMTDANALOGS	NUMANALOGS		// 4 analog inputs
#define NUMTDFREQS		1				// 2 rpm only
#define NUMTDINPUTBYTES	(1+NUMTDANALOGS+1+NUMTDFREQS)	// number of bytes needed to store the inputs

#define INDEXPAGE		0				// session index segment
#define USERPAGE		1				// user data segment
#define TRACKPAGE		2				// track data segment
#define IODEFPAGE		3				// definitions for data inputs
#define TRACKFINPAGE	4				// finish lines for the tracks and gear ratio information for the cars
#define FIRSTDATAPAGE	5				// first data segment

#define MAX_PAGE_SIZE				MAX_PAGE_32MBIT	// change to 64 bit if using bigger part
#define MAX_NUM_PAGES				NUM_PAGE_32MBIT // used by fdl.c for erasing flash

// Dataflash buffer assignments
#define IOBUFFER	1					// holds the i/o data
#define GENBUFFER	2					// used for general dataflash read/writes

//Cygnal 8051 SPI Definitions
#define	SPICFG_MODE_MASK			0xC0
#define	SPICFG_BITCNT_MASK			0x38
#define	SPICFG_FRMSIZ_MASK			0x07
#define SCRATCHLEN					128		// size of scratchpad in flash
#define FLASHLEN					MAX_PAGE_SIZE		// size of a page in flash = 528
#define NUMPORTS					2 		// number of UARTS on MCU

//#define SPI_CLOCK_DIVIDE			(SYSCLK/12500000L/2)		// approx 9 - 12.5MHz
#define	SPI_CLOCK_DIVIDE			4		// 10MHz when set to 100Mhz
#define	SPI_CONFIG_VALUE			0x70	//  01110000

// voltage monitored by SU, displayed by DU
#define POWER_THRESHOLD	(4*750)			// minimum ADC value to operate
										// value range: aggressive 2400 - 3200 conservative
										// 4V (min V) / 4.86V (scale) * 1024 * 4
// analog voltage hardware range
#define VOLTFULLRANGE	20.56			// 20V = 1992 a-d reading (out of 2048), so 20.56 = 2048
#define VOLTFULLSCALE	248				// 20V approx = 248 in 8 bits

// GPS velocity scaling
#define	GPSVELSCALE		1.02			// Multiply Ublox GPS velocity by this number to get actual

// DU Memory Map
//
// From top of memory
// 1 or more pages of scratch buffer for predictive lap timing
// lap timing pages, 1 for each session
// io data starting at top of memory and running toward bottom
//
// From bottom of memory
// Session index segment
// User data segment
// Track data segment
// Data IO definition segment
// Track Finish lines segemnt
// Recorded sessions going up in memory

// SU Memory Map
//
// From top of memory
// io data starting at top of memory and running toward bottom
//
// From bottom of memory
// Session index segment
// User data segment
// Track data segment
// Data IO definition segment
// Track Finish lines segemnt
// Recorded sessions going up in memory

// upper memory layout
#ifdef DISPLAY
#define NUMLAPPAGES		NUMSESSIONS		// in DU, each session has its own page of laptimes
#define NUMPREDPAGES	4				// in DU, four pages of predictive lap intermediate sector times
#else
#define NUMLAPPAGES		0				// no laps recorded in SU
#define NUMPREDPAGES	0				// no predictive lap in SU
#endif
#define PREDPAGESTART	(numpages - NUMPREDPAGES)		// last page is predictive lap scratchpad
#define LAPPAGESTART	(PREDPAGESTART - NUMLAPPAGES)	// first lap storage page
#define LASTDATAPAGE	(LAPPAGESTART - 1)	// last place to write data
#define LASTLAPPAGE		(PREDPAGESTART - 1)		// last place to write laps
#define NUMLAPS			(MAX_PAGE_SIZE/4)	// number of laps stored per session
#define IOPAGESTART		LASTDATAPAGE	// first place to store i/o data

#define NUMSECTORS					32		// number of intermediate points for predictive lap timing
									
#define OWNER_LEN					20		// number of characters in owner name
#define PHONE_LEN					20		// number of characters in owner phone number
#define NOTE_LEN					40		// number of characters in owner note

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------

// send a message to PC for display on screen
#define PRINTPC1(FORMAT, ARG1) do { sprintf(com[msgrouting[PC]].txbuff+DATA_START, FORMAT, ARG1); \
SendMessage(msgrouting[0], (u08) PC, (u08) WRITE_TEXT, 1+strlen(com[msgrouting[PC]].txbuff+DATA_START)); } while (0)
#define PRINTPC2(FORMAT, ARG1, ARG2) do { sprintf(com[msgrouting[PC]].txbuff+DATA_START, FORMAT, ARG1, ARG2); \
SendMessage(msgrouting[0], (u08) PC, (u08) WRITE_TEXT, 1+strlen(com[msgrouting[PC]].txbuff+DATA_START)); } while (0)
// send a message to display unit
#define PRINTDU1(LINENUM, FORMAT, ARG1) do { *(com[msgrouting[PC]].txbuff+DATA_START) = LINENUM; \
sprintf(com[msgrouting[msgptr[PC]]].txbuff+DATA_START+1, FORMAT, ARG1); \
SendMessage(msgrouting[0], (u08) DISPLAY_UNIT, (u08) WRITE_DISPLAY_TEXT, 2+strlen(com[msgrouting[PC]].txbuff+DATA_START+1)); } while (0)
// no formatting, just the text ma'am
#define PRINTPC0(TEXT) PRINTPC1("%s",TEXT)
#define PRINTDU0(TEXT) PRINTDU("%s",TEXT)

#ifdef SAVEMEM
#define Info_Screen(x) do { Clear_Screen(); Write_Big_Text(1, 7, x, '-'); } while(0)
#endif

// Useful Macros
#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))? (x):(y))
#define ABS(a) ((a >= 0)? (a) : -(a))
#define FLOATABS(a) ((a >= 0.0)? (a) : -(a))
#define AVG(q,r) (((q)+(r))/2)
//#define HI(x) ((P##x) |= (1<<(B##x)))
//#define LO(x) ((P##x) &= ~(1<<(B##x)))
//#define XOR(x) ((P##x) ^= (1<<(B##x)))
//#define ISHI(x) ((P##x) & (1<<(B##x)))
//#define ISLO(x) (!(ISHI(x)))
#define tohex(c) (c<10? (c+'0'):(c+'A'-10))
#define DELAY_MS(x) do {xdata long zzz; for (zzz=((SYSCLK/CRYSTALFREQ*150L) * x); zzz>0; zzz--); } while(0)
#define SWAP16(x) ( (((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00) )
#define SWAP32(x) ( (((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | \
				    (((x) << 8) & 0x00FF0000) | (((x) << 24) & 0xFF000000) )

// DataFlash port pins
sbit DF_WP = P7 ^ 0;					// DataFlash chip write protect
sbit DF_CS1 = P7 ^ 1;					// DataFlash 1 chip select
sbit DF_RST = P7 ^ 2;					// DataFlash chip reset
#ifdef DISPLAY
sbit DF_CS2 = P7 ^ 3;					// DataFlash 2 chip select
#endif

#if 0
#define PDF_RST	P7						// DataFlash chip reset
#define BDF_RST	2
#define PDF_WP	P7						// DataFlash chip write protect
#define BDF_WP	0
#define PDF_CS1	P7						// DataFlash 1 chip select
#define BDF_CS1	1
#ifdef DISPLAY
#define PDF_CS2	P7						// DataFlash 2 chip select
#define BDF_CS2	3
#endif
#endif

#define DATAFLASHMASK	0x10			// mask of bit 4 in cygnalflash hwconfig for dataflash type

//types of com ports
typedef enum {
	MESSAGING,
	TSIPGPS,
	UBXGPS,
	ASCII
} comusagetype;

typedef enum {
	OPEN,
	CLOSE
} comaction;

typedef struct {
	u08 port;						// port number
	comusagetype use;				// what port is used for
	u16 txbytes;					// number of bytes left to xmit on port
	u08 *rxptr;						// NULL when no rx'd msg, else point to rx'd msg in double buffer
	u08 *txptr;						// points to current byte in output buffer
	char parity;					// which parity is in use
	u08 rxnextidx;					// index to rx buffer = where to write next message (Q end)
#ifdef SENSOR
	u08 rxfirstidx;					// index to rx buffer = where to read next message (Q start)
#endif
	u08 *txbuff;					// pointer to transmit buffer
	u08 *rxbuff;					// pointer to double or triple receive buffer
} comporthandle;

typedef enum {
	UNKNOWNSTATUS,					// have not determined camera configuration state yet
	CAMLANCNOTCONNECTED = 2,		// no hardware for camera or LANC is connected
	CAMOFF,							// camera is turned off
	CAMLANCREADY,					// camera is ready, determined by digital input
	CAMLANCNOTREADY,				// camera is not ready, determined digital input
	CAMLANCRECORD,					// digital input indicates camera or lanc is recording
	CAMLANCONPULSE,					// in middle of pulse to turn camera on
	CAMLANCOFFPULSE,				// in middle of pulse to turn camera off
} camlancstatustype;				// camera or lanc status through direct digital inputs

typedef struct {
		u32 time;					// 4 in milliseconds since midnight on Sunday
		u16 weeks;					// 2 gps number of weeks, week 0 started Jan 6, 1980

		s32 lat;					// 4 divide by 2^31 and multiply by 180 for degrees
		s32 lon;					// 4

		s16 alt;					// 2 altitude in meter (- = below sea level)
		u08 velpack[4];				// 4 byte 0: 7-5 upper 3 bits of signed east-west
									//           4-2 upper 3 bits of signed north-south
									//           1-0 upper 2 bits of signed alt
									//   byte 1: east-west velocity (met/10msec) east +, west -
									//   byte 2: north-south velocity met/10msec. north + south -
									//   byte 3: vertical velocity met/10msec up + down -
		s08 temp;					// 1 -128 to 128 (half degrees celcius)
} gpstype;							// total 21

// use these structures to access the individual sample groups within a flash page
typedef u08 acceltype[3];		  			// byte 0: bit 7-4 upper nibble of x, bit 3-0 upper nibble of y,
											// byte 1: lower byte of x, byte 2: lower byte of y

typedef struct {			// ***** DO NOT CHANGE STRUCTURE. CODE DEPENDENT!! *****
	gpstype gps;							// 21
	acceltype accel[10];					// 3*10 = 30
} samptype10;								// 51 total bytes
typedef struct {			// ***** DO NOT CHANGE STRUCTURE. CODE DEPENDENT!! *****
	gpstype gps;							// 21
	acceltype accel[5];						// 3*5 = 15
} samptype5;								// 36 total bytes

#define SAMP10S_PER_PAGE			(FLASHLEN / sizeof(samptype10))	// 10 number of sample groups (gps&10) per storage page
#define SAMP5S_PER_PAGE				(FLASHLEN / sizeof(samptype5))	// 14 number of sample groups (gps&5) per storage page

// information about a data acquisition session
typedef struct {
	u16 startseg;		// 2 which flash page the session starts on
	u16 endseg;			// 2 which flash page the session ends on
	u32 starttime;		// 4 in milliseconds since midnight on Sunday
	u16 startweeks;		// 2 gps number of weeks, week 0 started Jan 6, 1980
	u32 endtime;		// 4 in milliseconds since midnight on Sunday
	u16 endweeks;		// 2 gps number of weeks, week 0 started Jan 6, 1980
	u08 modesample;		// 1 upper nibble is collection mode
						//   0 = lapping
						//   1 = autocross
						//   2 = quarter mile
						//   3 = eighth mile
						//   4 = hillclimb (segment timing with separate start and finish points)
						//   5 = drive
						//   lower nibble is sampling rate, mix of accel & i/o and gps rates
						//   0 = 10 ACC & 1 GPS per second (samptype10)
						//	 1 = 10 ACC & 2 GPS per second (samptype5)
						//	 2 = 20 ACC & 2 GPS per second (samptype10)
						//   3 = 20 ACC & 4 GPS per second (samptype5)
						//	 4 = 40 ACC & 4 GPS per second (samptype10)
	u08 iodata;			// 1 which i/o data points are turned on/off
						//   bit 7 = 16bit Freq, 6 = all digitals, 5 = Anlg3, 4 = Anlg2
						//   bit 3 = Anlg1, 2 = Anlg0, 1 = Freq1, 0 = Freq0
	u16 iostartseg;		// 2 flash page where this session's io starts and goes backwards
	u16 ioendseg;		// 2 flash page where this session's io ends (endseg < startseg)
	u08 driver;			// 1 index into driver table
	u08 car;			// 1 index into car table
	u08 track;			// 1 index into track table
} sessiontype;			// 25 total

typedef struct {
	u16 cylandweight;						// 2 upper 12 bits = weight in pounds/2
											//	 lower 4 bits = cylinders. 0 = one cyl, two stroke,
											//		 15 = 16 cylinders
	u16 revwarning;							// 2 1 to 32000 = caution zone
	u16 revlimit;							// 2 1 to 32000 = maximum rev	
} cartype;									// 6 total

// setup data downloaded from Traqview
typedef struct {			// ***** DO NOT CHANGE STRUCTURE. CODE DEPENDENT!! *****
	u08 selecteddriver;						// 1 driver selected via menu or traqview
	u08 selectedtrack;						// 1 track selected via menu or traqview
	u08 selectedcar;						// 1 car selected via menu or traqview
	u08 modesample;							// 1 current configuration for sampling rate and collection mode
											// 1 upper nibble is collection mode
													//   0 = lapping
													//   1 = autocross
													//   2 = quarter mile
													//   3 = eighth mile
													//   4 = rally
													//   5 = drive
													//   lower nibble is sampling rate, mix of accel & i/o and gps rates
													//   0 = 10 ACC & 1 GPS per second (samptype10)
													//	 1 = 10 ACC & 2 GPS per second (samptype5)
													//	 2 = 20 ACC & 2 GPS per second (samptype10)
													//   3 = 20 ACC & 4 GPS per second (samptype5)
													//	 4 = 40 ACC & 4 GPS per second (samptype10)
	u08 iodata;								// 1 which i/o data points are turned on/off
											//   bit 7 = 16bit tach, bit 6 = all digitals, 5 = Anlg3, 4 = Anlg2
											//   bit 3 = Anlg1, 2 = Anlg0, 1 = Freq1, 0 = Freq0
	u08 drivername[NUMDRIVERS][NAMELEN];	// 192 = 16*12 string of driver name (null term if < 16 char)
	struct {
		u08 name[NAMELEN];					// 16 string of car name (null term if < 16 char)
		cartype carspecs;					// 6
	} car[NUMCARS];							// 264 = 22*12
} usertype;									// 462 TOTAL

typedef struct {
		u08 pointname[IONAMELEN];			// 12 name of this data input
		u08 unitname[IONAMELEN];			// 12 units that describe value of this input
		u08 maxscale;						// 1 5, 10, 20
		float lowval;						// 4 value represented at low end of sensor range
		float highval;						// 4 value represented at high end of sensor range
		float alarmval;						// 4 value that if exceeded in the alarmdirection, triggers alarm
		s08 alarmdirection;					// 1 -1 = less than, 0 = equal to, +1 = greater than
		u08 lowreading;						// 1 voltage reading at low end of sensor range
		u08 highreading;					// 1 voltage reading at high end of sensor range
} analogdefs;								// 40x4 = 160

typedef struct {
	u08 pointname[IONAMELEN];			// 12 name of this data input
	u08 onname[IONAMELEN];				// 12 name of the digital '1' condition
	u08 offname[IONAMELEN];				// 12 name of the digital '0' condition
} digitaldefs;							// 36x7 = 252

typedef struct {
	u16 rpmwarning;						// 2 caution zone
	u16 rpmlimit;						// 2 maximum rpm
	u08 divider;						// 1 freq divided by this number to give rpm
} freqdefs; 							// 5x2 = 10

typedef struct {			// ***** DO NOT CHANGE STRUCTURE. CODE DEPENDENT!! *****	
	analogdefs analogs[NUMANALOGS];			// 40x4 = 160
	digitaldefs digitals[NUMDIGITALS];		// 36x7 = 252
	freqdefs frequency[NUMFREQS]; 			// 5x2 = 10
	u08 digouts[NUMDIGOUTS/2];				// 2 data output configurations - 4 bits per output for D4 - D7
											// if non-zero, overrides digital input setting
												// digout[0] = LSN D4, MSN D5
												// digout[1] = LSN D6, MSN D7
													// 0000 = digital input (if enabled in iodata)
													// 0001 = rpm warning
													// 0010 = gauge warning
													// 0011 = any camera
													// 0100 = any analog warning (alarm rollup)
													// 0110 = Replay XD w/ 4 wire interface
													// 0111 = GoPro HERO1,2 w/ 4 wire interface
													// 0110 = Replay XD w/ 4 wire interface
													// 0111 = GoPro HERO1,2 w/ 4 wire interface
													// 1000 = ReplayXD USB
													// 1001 = Replay Mini USB
													// 1010 = Replay Prime X USB
													// 1011 = Mobius USB
													// values 12-15 undefined at this time
	u08 lancconfig;							// 1 only valid if digout[0] is LANC
													// 0x1	Automatic (Hardpower, partial Softpower support) MINLANC
													// 0x2	Camcorder mode (Softpower or Hardpower)
													// 0x3	VCR (Hardpower)
													// 0x4	VCR HC (Softpower)
													// 0x5	HD (new style) SONY camcorder DEFAULTLANC
													// 0x6	ChaseCam PDR w/ LANC interface	MAXLANC
													// 0x7 - 0xF	TBD
} iotype;									// 425 total

typedef struct {
	struct {
		u08 name[NAMELEN];					// 16 string of track name (null term if < 16 char)
		s32 startlat;						// 4 divide by 2^31 and multiply by 180 for degrees
		s32 startlon;						// 4
		s16 startalt;						// 2 altitude in meter (- = below sea level)
		u16 starthead;						// 2 heading in integer degrees of start position
	} track[NUMTRACKS];						// 448 = 28 * 16
} tracktype;								// 448 TOTAL

typedef struct {
	u16 diffratio;							// 2 x100 0.5 - 8.0 typical or 50 - 1000
	u16 wheelcircumference;					// 2 x100 4 - 30 inches typical or 400 - 3000
	u16 inchesperenginerev[NUMGEARS];		// 16 = 2*8 x100 5 - 20 inches per rev typical or 500 - 2000
} geartype;									// 20 bytes

// typedef	geartype gearinfo[NUMCARS][NUMGEARS];	// 192 = 12 * 8 * 6

typedef struct {
	struct {
		s32 finishlat;						// 4 divide by 2^31 and multiply by 180 for degrees
		s32 finishlon;						// 4
		s16 finishalt;						// 2 altitude in meter (- = below sea level)
		u16 finishhead;						// 2 heading in integer degrees of finish position
	} trackfinish[NUMTRACKS];				// 192 = 12 * 16
	geartype gears[NUMCARS];				// 240 = 20 * 12 gear information page
} trackfinishandgeartype;					// 432 TOTAL

typedef enum {
	A0,
	A1,
	A2,
	A3
} analoglist;

typedef enum {
	D4,
	D5,
	D6,
	D7
} digitallist;

typedef u32 laplisttype[NUMLAPS];	// format of lap time flash storage

typedef struct {				// structure to hold intermediate and current information for predictive laptimes
	s32 reference;				// reference lap timestamp for a sector boundary in msec since s/f
	s32 current;				// current lap timestamp for a sector boundary in msec since s/f
} sectortype;

typedef struct {
	s32 bestsectortimes[NUMSECTORS];	// 128 = 4 * 32 all intermediate sector times for a given track in msec
	s32 bestlapever;					// 4            lap time associated with those sectors in msec
} storedtimestype;						// 132

				// stored best sector and lap times in msec for one fourth of tracks. need 4 of these.
typedef storedtimestype storedsectortype[NUMTRACKS/4];	// 528 = 132 * 4

typedef enum {							// data output definitions
	DISABLED,							// 0000 = digital input (if enabled in iodata)
	RPMWARN,							// 0001 = rpm warning
	ANALOGALARM,						// 0010 = gauge warning
	CAMERA,								// 0011 = trigger camera recording
	ALARMROLLUP,						// 0100 = any analog warning (alarm rollup)
	REPLAYCAMERA = 6,					// 0110 = start recording (used for replay 4 wire)
	GOPROCAMERA	= 7,					// 0111 = start recording (used for gopro 4 wire)
	REPLAYXDUSB = 8,					// 1000 = start recording (used for replay xd1080 on usb)
	REPLAYMINI = 9,						// 1001 = start recording (used for replay mini on usb)
	REPLAYPRIME = 10,					// 1010 = start recording (used for replay prime on use)
	MOBIUS = 11,						// 1011 = start recording (used for mobius on usb
	SONYLANC = 12						// 1100 = start recording (sony camera on LANC)
} digoutstatetype;

typedef struct {						// analog alarm definitions for use in digital output alarms
	u08 alarmnum;						// 8 bit quantity where alarm occurs
	s08 alarmdirection;					// 1 -1 = less than, 0 = equal to, +1 = greater than
} analogwarningtype;

typedef struct {
	digoutstatetype digoutstate[NUMDIGOUTS];		// how is this digital port configured
	u16 minperiod;									// minimum period (maximum rev) to declare warning
	analogwarningtype analogwarning[NUMANALOGS]; 	// where to declare warnings for each analog input
} digouttype;

// session headers
typedef struct {			// ***** DO NOT CHANGE STRUCTURE. CODE DEPENDENT!! *****
	sessiontype session[NUMSESSIONS];	// 19 * 26
} indextype;					// indextype grand total 494 bytes

typedef union {			// ***** DO NOT CHANGE STRUCTURE. CODE DEPENDENT!! *****
	indextype index;									// easy access to index (page 0)
	usertype user;										// easy access to driver and car names & data (page 1)
	tracktype circuit;									// easy access to track names & data (page 2)
	iotype io;											// easy access to io names & scaling info (page 3)
	trackfinishandgeartype finishandgear;				// easy access to timetrial finish lines and gear info (page 4)
	samptype10 secsamp10[SAMP10S_PER_PAGE];				// easy access to data 10 accel/gps (pages 6 to LASTSESSIONPAGE)
	samptype5 secsamp5[SAMP5S_PER_PAGE];				// easy access to data 5 accel/gps (pages 6 to LASTSESSIONPAGE)
	u08 sampbuff10[SAMP10S_PER_PAGE][sizeof (samptype10)]; 	// bytewise access into structure
	u08 sampbuff5[SAMP5S_PER_PAGE][sizeof (samptype5)]; 	// bytewise access into structure
	laplisttype laps;										// easy access to lap times (above recorded data)
	storedsectortype storedsectors;						// easy access to stored intermediate times for pred lap timing	(above laps)
	u08 bigbuff[MAX_PAGE_SIZE];							// to establish overall buffer size
} flashpagetype;

// this is the format for the Cygnal microprocessor scratchpad flash memory
typedef struct {			// ***** DO NOT CHANGE STRUCTURE. CODE DEPENDENT!! *****
	char owner[OWNER_LEN];		// 20 name of Traqmate owner (entered by Traqview for security)
	char phone[PHONE_LEN];		// 20 contact number for owner, entered by Traqview
	char note[NOTE_LEN];		// 40 address or anything the user wants to put here, entered by Traqview
	s08 timecode;				// 1 hours +/- from GMT for display, entered by Traqview, changed by program
	u08 model;					// 1 model number, entered by Factory Cal
	u08 hwrev;					// 1 hardware revision * 100 (237 = 2.37), entered by Factory Cal
	u32 serno;					// 4 serial number, entered by Factory Cal
	u08 week;					// 1 week of year of manufacture, entered by Factory Cal
	u08 year;					// 1 year of manufacture, entered by Factory Cal
	u16 xzero;					// 2 zero offset value for x, entered by Factory Cal
	u16 yzero;					// 2 zero offset value for y, entered by Factory Cal
	u16 xminusg;				// 2 max deflection negative at 1 G, entered by Factory Cal
	u16 xplusg;					// 2 max deflection positive at 1 G, entered by Factory Cal
	u16 yminusg;				// 2 max deflection negative at 1 G, entered by Factory Cal
	u16 yplusg;					// 2 max deflection positive at 1 G, entered by Factory Cal
	u16 calx;					// 2 zero offset value for PWM, entered by Factory Cal
	u16 caly;					// 2 zero offset value for PWM, entered by Factory Cal
	u16 swrev;					// 2 software version * 100 (xxx.yy) 65535 = 655.35, entered by SW
	u16 datarev;				// 2 data format version, entered by SW
	u08 contrast;				// 1 LCD contrast setting, initial entered by Factory Cal, changed by program
	u08 hwconfig;				// 1 bit 7-5: reserved, all entered by Factory Cal
								//	 bit 4 - 0 if 16Mbit Dataflash, 1 if 32Mbit Dataflash (set by unit)
								//	 bit 3 - 0 if oldstyle LCD installed, 1 if new style (diff contrast)
								//	 bit 2 - 0 if 2 dataflash chips installed
								//	 bit 1 - 0 if 7 switches installed
								//	 bit 0 - 0 if bling LEDs are installed
	u08 lapholdtime;			// 1 on DU, 0 to 240 seconds, 0xFF = uninitialized
	u08 usersettings;			// 1
								// on DU various user selections, default = 0x73
								//   bits 7-5 eight possible states for graph scale
								//		000 = +-0.1 sec, 001 = +-0.2 sec, 010 = +-0.5 sec, 011 = +-1 sec,
								//		100 = +-2 sec, 101 = +-5 sec, 110 = +-10 sec, 111 = +-20 sec
								//	 bit 4, 1 = AutoOn disabled, 0 AutoOn enabled
								//   bits 3-1 eight possible states for track size
								//		000 = small, 001 = med, 010 = large, others unused
								//   bit 0 - units on DU. 1 if english units, 0 if metric units
								// on SU, controls AutoOn (su2), AutoStart, Poweroff
								//	  bits 7-5, 3,2 reserved
								//	  bit 4, 1 = AutoOn disabled, 0 AutoOn enabled
								//    bit 1, 1 = button stops recording, 0 = no recording stop
								//	  bit 0, 1 = button poweroff, holding red button shuts off, 0 = nopoweroff
	u08 turnonspeed;			// 1 on SU, speed in mph to exceed to start recording includes auto-record, 0xFF = disabled, button record start
	u16 turnofftime;			// 2 on SU, time in seconds at zero speed required to end session, 0xFFFF = disabled
	u16 zzero;					// 2 zero offset value for z, entered by Factory Cal
	u16 zminusg;				// 2 max deflection negative at 1 G, entered by Factory Cal
	u16 zplusg;					// 2 max deflection positive at 1 G, entered by Factory Cal
	u08 menustart;				// 1 which menu to execute on startup
								//	 0xFF = main menu, 0x01 = Laps, 0x02 = Qual, 0x03 = Drive, 0x04 = Tach, 0x05 = Gauges
								//	 0x06 = GPS Compass, 0x07 = GPS Location
} caltype;						// 123 total out of 128

#define AUTOONMASK	0x10		// bit mask for AutoOn bit in usersettings
#define AUTOONENABLED ((scratch.calibrate.usersettings & AUTOONMASK)? 0 : 1)

typedef union {				// ***** DO NOT CHANGE STRUCTURE. CODE DEPENDENT!! *****
	caltype calibrate;
	u08 scratchbuff[SCRATCHLEN];
} scratchpadtype;

// SU and DU Modes
// NOTE: SOME CODE DEPENDS ON ORDER OF THESE ITEMS
typedef enum {
	WAITING,		// idle state
	WAITING_AUTOX,	// temp state for menu access
	WAITING_LAP,	// temp state for menu access
	WAITING_HILL,	// temp state for menu access
	WAITING_DRAG,	// temp state for menu access
	TIMING,			// recording - timing started
	DRIVE,			// recording point to point drive
	AUTOXGETFINISH,	// recording at an autocross, user presses select at finish
	HILLCLIMB,		// recording at a hillclimb
	LAPS,			// recording lap timing or qualifying
	GPSREC,			// 10 recording GPS information screen
	GPSRPM,			// recording RPM screen
	INPUTS,			// recording inputs screen
	GAUGEA0,		// gauge screens for individual inputs
	GAUGEA1,
	GAUGEA2,
	GAUGEA3,
	GAUGED4,
	GAUGED5,
	GFORCES,
	RALLY,			// 20 recording point to point rally with stages
	DRAGREADY,		// recording, waiting to launch at drag start
	AUTOXREADY,		// recording, waiting to launch at autocross start
	HILLCLIMBWAITSTART,	// recording, waiting to cross start/finish 1st time
	HILLCLIMBGETFINISH,	// recording, waiting for user to hit SEL at finish
	HILLCLIMBSTART,		// recording, looking for hillclimb start line
	HILLCLIMBFINISH,	// recording, looking for hillclimb finish line
	AUTOXFINISH,	// recording, looking for autocross finish line
	AUTOXHOLDTIME,	// showing finish time, still recording
	DRAGFINISH,		// looking for end of strip
	DRAGHOLDTIME,	// 30 showing finish time, still recording
	DRAGHOLDNUMS,	// showing dragrace numbers, still recording
	WAITSTART,		// recording - waiting to cross start/finish
	AUTOXSTAGE,		// non-recording waiting to stage at autocross start
	DRAGSTAGE,		// non-recording waiting to stage at dragrace start
	GPSINFO,		// non-recording gps information screen
	INSTRUMENT,		// non-recording tach and speedo screen
	COMPASS,		// non-recording digital compass
	CHKINPUTS,		// non-recording inputs screen
	REVIEW,			// 39 reviewing lap times

					// standalone sensor unit modes
	WAIT_COLLECT = 0xE0,	// 224 waiting for event to start collecting data
	COLLECTING,		// collecting and storing data
	ERASING,		// erasing memory
	WAIT4BACK,		// waiting for user to acknowledge message

					// software download modes
	START_DOWNLOAD = 0xF0,	// 240 causes SU or DU to clear flash and wait for software download packets
	DOWNLOAD_COMPLETE,		// causes SU or DU to verify sw received and ACK or NAK
							// after ACKing this mode change, SU or DU should wait for SW_DOWNLOAD command
	CANCEL_DOWNLOAD,			// cancels sw download and resume normal operation
					// startup and shutdown modes
	STARTUP = 0xFE,
	SHUTDOWN = 0xFF
} tmstatetype;

#if defined (SENSOR) && !defined (DISPLAYATTACHED)
#define RECORDING(x) ((x) == COLLECTING)
#define MONITORING(x) (((x) == WAIT_COLLECT) || ((x) == COLLECTING))
#else
#define RECORDING(x) ((x) >= TIMING && (x) <= WAITSTART)
#define MONITORING(x) ((x) >= TIMING && (x) <= CHKINPUTS)
#define DRIVING(x) (((x) == DRIVE) || ((x) == GPSRPM) || ((x) == GPSREC))
#endif

#define OCCUPIED(x) (RECORDING(x) || (ERASING == (x)))
#define REPROGRAMMING(x) (((x) == START_DOWNLOAD) || ((x) == DOWNLOAD_COMPLETE) || ((x) == CANCEL_DOWNLOAD))

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------

void SYSCLK_Init (void);
void PPS_Init (void);
void UART_Init (comporthandle *, u32 );
void XmitUART(comporthandle *, u16 );
BOOL ComPort(comporthandle *, comaction );
void Timer3_Init (u16);
void Timer3_ISR (void);

void SPIO_Init (void);
void XRAM_Init(void);
void DataFlash_Init(u16 *, u16 *);
void DataFlash_Erase_Page(u16);
void DataFlash_Erase_Block(u16);
u08 DataFlash_Read_Status(void);
void DataFlash_Page_Write_Erase(int, int , char *, int );
void DataFlash_Page_Read(int , char *, int );
void DataFlash_Erase_Range( u16, u16, BOOL);
void DataFlash_Buffer_Write(int, int, u16, char *);
void DataFlash_Page_Write_From_Buffer_Erase(int, int );
void DataFlash_Buffer_Clear(int, int );
BOOL DataFlash_Page_Erased(int , int );
unsigned char xferSPI (unsigned char);

void EraseScratchpad( void );
void WriteScratchpad( unsigned char *, int );
void ReadScratchpad( unsigned char *);
BOOL InitDUScratchpad(BOOL );
void InitSUScratchpad(void);

void Init_Session(u16 *, u16 *);
u08 Start_Session(u16 *, u16 *);
void End_Session(u08 , u16, u16 );
void Erase_Session(u08 );
void Session_Data(u08 , sessiontype *);
void Repair_Sessions( void );
u08 Session_Usage(u16 *);
void Init_Tables( void );
void Pack_Sessions( BOOL);

void SaveDownload(u08 *, u08 *);
void CopytoCodeSpace(u16);

unsigned int crc16(u08 *, u16);

#endif
