#ifndef SENSOR_H
#define SENSOR_H

//	This is the Software Revision number in the format
//	software version * 100 (xxx.yy) 65535 = 655.35

#define SWREV	390		// added changing heading to gps simulation
						// changed index for buffer swaps from xor to subtractive
						// changed to support adesto version of dataflash, fixed timing loops, tweaked camera timing
						// eliminated chasecam support, reworked state machine, added usb camera control
						// added gps simulator
						// added replay xd camera
						// change compile options to all level 8
						// added pdrlanc.status to gpspulse, tested with du1, tested as basic
						// 3.22 gopro working and calibrated with traqdash
						// 3.15 3 axis freescale accel, tim-lc or lea5, su detecting, factory tests, smbus, 
						// autoon, global pullups, p3.1 open drain, improved baud rate change
						// reprogram digipot on micro flash update
						// smbus calibrated
						// send ublox5 V5.00 encoded max performance message
						// lanc interface added and debugged
						// lanc interface revisions 2
						// factory tests tweaked
						// lanc timing tweaks
						// tested with chasecam timing
						// added pdr status reporting in support of wait for cam screen on du
						// improved power saving on shutdown

#define HWREVSU1	100		// first SU hw revision
#define HWREVSU2	200		// new SU hw revision

#define MODELNUM	1	// SU is model number 1

// compilation controls
// DISPLAYATTACHED must be defined for SU/DU compilation
#define UBX

#include <tmtypes.h>
// Part of sensorunit.c, Traqmate product software.
// 11/10/2003
// Author: GAStephens
// Copyright 2003 - 2016 Track Systems Technologies, LLC
//
// This file contains the global definitions and function declarations unique to
// the Traqmate Sensor Unit.
//

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------
#define TSIPBAUD	9600L				// Baud rate of UART1 in bps
#define UBXLEABAUD	9600L				// Starting baud rate of UART1 in bps for LEA-5 Ublox GPS chip
#define UBXBAUD		57600L				// Baud rate of UART1 in bps
#define NUM_CHAN	8					// 5g x,y,z, power monitor, 4 analog inputs
#define BUTTONSECS	5					// number of seconds to wait for another button press
#define BUTTONTICKS (SAMPLE_RATE * BUTTONSECS)	// number of ticks to wait
#define FLASHTICKS	(SAMPLE_RATE / 10)	// flash LED for 1/10th second

// Sensor Parameters
#define ADC_RATE			2500000		// ADC conversion clock
#define RISING_EDGE			0x20		// mask for comparator rising edge flags
#define FALLING_EDGE		0x10		// mask for comparator falling edge flags
#define PULSE_KEEPOUT		100		
	// 100 rpm ticks = 100/2560000 ticks/second = 390 msec
#define RPM_AVERAGING		4			// number of samples to average to smooth RPM
#define PERIOD_WINDOW		4			// period must be different by this amount to change the value

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------

sbit LED0 = P3 ^ 3;						// upper left = power
sbit LED1 = P3 ^ 6;						// upper right = recording
sbit LED2 = P3 ^ 5;						// lower right = GPS
sbit LED3 = P3 ^ 4;						// lower left = COM

#define PWRLED	LED0
#define RECLED	LED1
#define GPSLED	LED2
#define COMLED	LED3

sbit DIGOUT0 = P4 ^ 0;					// External digital out 0
sbit DIGOUT1 = P4 ^ 1;					// External digital out 1
sbit DIGOUT2 = P4 ^ 2;					// External digital out 2
sbit DIGOUT3 = P4 ^ 3;					// External digital out 3
sbit DIGOUT4 = P4 ^ 4;					// External digital out 4
sbit DIGOUT5 = P4 ^ 5;					// External digital out 5
sbit DIGOUT6 = P4 ^ 6;					// External digital out 6

#define LANCMODE	DIGOUT0				// 0 = NORMAL, 1 = TEST MODE
#define LANCCMD		DIGOUT2				// 0 = REC STOP, 1 = RECORD
#define LANCNORMALMODE	1
#define LANCTESTMODE	1
#define LANCRECSTOP	0
#define LANCRECORD	1

sbit DIGIN0 = P5 ^ 0;					// External digital in 0
sbit DIGIN1 = P5 ^ 1;					// External digital in 1
sbit DIGIN2 = P5 ^ 2;					// External digital in 2
sbit DIGIN3 = P5 ^ 3;					// External digital in 3
sbit DIGIN4 = P5 ^ 4;					// External digital in 4
sbit DIGIN5 = P5 ^ 5;					// External digital in 5
sbit DIGIN6 = P5 ^ 6;					// External digital in 6

#define LANCNOTREADY	DIGIN4			// 0 = READY, 1 = NOT READY
#define LANCRECORDING	DIGIN3			// 0 = NOT RECORDING, 1 = RECORDING
#define GOPROREADY		DIGIN3			// 0 = NOT READY, 1 = READY
#define GOPRORECORDING	DIGIN2			// 0 = NOT RECORDING, 1 = RECORDING

sbit EN3VGPS = P7 ^ 4;					// 0 turns on GPS circuitry
sbit EN3VIF	 = P7 ^ 5;					// 1 turns on external interface
sbit EN5V    = P7 ^ 7;					// 1 keeps power on main unit

// sbit GPSRESET = P7 ^ 3;				// resets Ublox --- eliminated in SU2
sbit ENSMBPU = P7 ^ 3;					// enables pullups on SMBus I2C interface
sbit SMBSDA = P0 ^ 6;					// SMBus I2C data
sbit SMBSCL = P0 ^ 7;					// SMBus I2C clock

sbit ACCST   = P5 ^ 2;					// 1 turns on accelerometer selftest

sbit SW1 = P6 ^ 0;						// power / record switch
										// pressed = 0
#define SU2MASK		0xC0				// when P6.7 - P6.5 = 110, SU2.0 is present

#define GPSRXBUFCNT	3					// quadruple buffer incoming GPS messages

// NOTE: do not allow auto record at zero speed because you will lose control of unit
#define AUTORECORD ((0xFF != scratch.calibrate.turnonspeed) && (0 != scratch.calibrate.turnonspeed))	// Basic automatically enters record mode on startup
#define AUTORECORDSTOP ((0xFFFF != scratch.calibrate.turnofftime) && (0 != scratch.calibrate.turnofftime))		// Basic automatically stops recording when still
#define NOPOWEROFF (0 == (scratch.calibrate.usersettings & 0x01)) 	// Basic cannot be powered off with button
#define NORECORDOFF (0 == (scratch.calibrate.usersettings & 0x02))	// Basic, recording cannot be stopped with button
#endif

#define TEMPERATURE_OFFSET	50			// degrees F to subtract from die temp to approximate ambient
//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------

void PORT_Init_SU (void);
void ADC0_Init (void);
void ADC0_ISR (void);
void GPSPSS_ISR(void);
void Freq0_Init (void);
void DigiPot (BOOL);
void SMB_Init (void);
void SMB_Write ();
void SMB_Read ();
void New_Accessory_Setup();

#ifdef TSIP
void TsipInit(void);
void TsipSendPacket(u08	, int , u08 *);
void Parse_Tsip(u08 *, gpstype *);
#else
void UBXInit(void);
void UBXBaudChange(void);
void UBXSendPacket(u16 , u16);
void Parse_UBX(u08 *, gpstype *);
#endif

// smbus port definition
typedef struct {
	u08 target;						// address of SMBus device currently being addressed
    u08	sent_byte_counter;			// counts outgoing bytes
	u08 rec_byte_counter;			// counts incoming bytes
	u08 txbytes;					// number of bytes to transmit
	u08 rxbytes;					// number of bytes to receive
	u08 *txbuff;					// pointer to transmit buffer
	u08 *rxbuff;					// pointer to double or triple receive buffer
	BOOL data_ready;				// indicates data is ready on bus
} smbusporthandle;

typedef enum {
	ENABLED4USB,					// generic USB camera interface on D4 (Mobius)
	ENABLED4LANC,					// LANC interface using D4
	ENABLED4GOPRO,					// GoPro camera on D4
	ENABLED4REPLAY,					// Replay XD1080 camera on D4
	ENABLED4REPLAYUSB,				// Replay XD1080 camera on D4 (USB)
	ENABLED4MOBIUS,					// Mobius camera on D4 (USB)
	ENABLED4MINI,					// Replay Mini camera on D4 (USB)
	ENABLED4PRIME,					// Replay Prime X camera on D4 (USB)
	UNKNOWNCONFIG,					// have not determined configuration yet
	NOCAMLANC,						// no CAM or LANC configured
	NUMBER_CAMERAS					// used for array size
} camlancportstype;

typedef enum {
	BASIC,
	COMPLETE,
	NUMBER_TRAQMATE_TYPES
} traqmatetypes;

typedef enum {
	TEN,
	TWENTY,
	FORTY,
	NUMBER_SAMPLERATES
} traqmatespeeds;

typedef enum {
	PULSE,
	LEVEL,
	LANC,
	NUMBER_SIGNALTYPES
} camerasignaltypes;

typedef enum {
	SIGNALTYPE,
	DATADELAY,
	CAMDELAY,
	RECONTIME,
	RECOFFTIME,
	NUMBER_CAMERA_PARAMETERS
} cameraparameters;
	
typedef enum {
	NOEVENT,						// camera state machine events
	PULSEON,
	PULSEOFF,
	LANCTEST,
	LOCKOUT
} camlanceventtype;

typedef xdata struct {
	camlancportstype ports;			// which ports is chasecam enabled on
	camlanceventtype event;			// next event to execute
	camlancstatustype status;		// which state the CAM or LANC is in
	u16 eventtick;					// next clock tick to do something
} camlanctype;

typedef xdata enum {				// which accessories are connected
	NOACC,							// nothing connected to expansion port
	DIRECTACC,						// direct connect accessory (TraqTach, TraqSync, TraqData)
	DIRECTLANC,						// direct connect accessory with LANC interface
	SMBUSACC						// new style smbus accessory
} accessorytype;

// Global Variables
#ifdef SU_MAIN
xdata comporthandle com[NUMPORTS];		// handles to com ports
xdata smbusporthandle smbus;			// handle to smbus
xdata u08 msgrxbuff[MSGSIZE*2];			// inbound msg buffer
xdata u08 gpsrxbuff[GPSRXSIZE*GPSRXBUFCNT];		// inbound gps comm buffer	NOTE: Triple buffered. Qhead = rxptr, Qend = rxidx
xdata u08 gpstxbuff[GPSTXSIZE];			// outbound gps comm buffer
xdata u08 msgtxbuff[MSGSIZE];			// outbound msg buffer
xdata u08 smbustxbuff[SMBTXSIZE];		// smbus outbound msg buffer
xdata u08 smbusrxbuff[SMBRXSIZE];		// smbus inbound msg buffer
xdata u16 result[NUM_CHAN] = {0, 0, 0, 0, 0, 0, 0, 0};		// array to hold ADC samples    
//	0 place = this units id, 1 unit = PC = COM0 (via display), 2 unit = SU = self = -1, 3 unit = DISPLAY = COM0
code s08 msgrouting[NUMUNITS+1] = {SENSOR_UNIT, 0, -1, 0};	// what port to route messages to
xdata u08 CTS[NUMUNITS] = {0, 0, 0};		// non-zero when waiting for response, 0 when clear to send
xdata tmstatetype tm_state = WAITING;
xdata u08 samps_per_sec = 10;		// number of samples per second
xdata camlanctype camlanc;			// state machine controls for turning camera and lanc on/off
xdata u16 stoprecordtick;			// which second to stop recording if vehicle not in motion
xdata u08 session = 0;				// which session we are working on
int secondcnt = 0;					// how many seconds into sample period
int gcnt = 0;						// how many tenths into a second
u16 int_dec;					    // integrate/decimate counter
xdata u16 accumulator[NUM_CHAN] = { 0 }; // ** here's where we integrate ADC samples             
u08 channel = 0;					// which ADC channel are we sampling
xdata u08 adcresults = FALSE;		// TRUE when samples collected
xdata u16 period0;					// current period measurement for Freq0 input (RPM), in freq0ticks
xdata u16 periodbuff[RPM_AVERAGING];	// last several period measurements for averaging
xdata u16 negperiod0;				// current period measurement form a negative going pulse (anti-falsing)
xdata u08 gpspulse = FALSE;			// TRUE when GPS PPS signal received
xdata u16 gps_past_due = 0;			// on which clock tick GPS PPS will be overdue
xdata u08 txbufcnt = 0;				// used for UART0 prints
xdata u16 clocktick = 0;			// increments in 1/SAMPLE_RATE sec intervals
xdata u16 secondtick = 0;			// increments in one second intervals for timing slow events
xdata u16 nextsecond = SAMPLE_RATE;	// which clocktick the next second will appear
xdata u08 mode_change = FALSE;		// indicates change in state machine
xdata flashpagetype flashpage;		// structure to access one page of flash
xdata scratchpadtype scratch;		// structure to access scratchpad memory
xdata u08 gpslock = 0;				// 0 when no lock, non-zero when lock
xdata u16 dop = 0;					// gps dillution of precision * 100
xdata u16 pagesize;					// page size of dataflash
xdata u16 numpages;					// number of pages in dataflash
xdata BOOL resend = FALSE;				// true if necessary to resend last message
xdata BOOL calibrating = FALSE;		// true when in calibration mode
xdata tmstatetype new_mode = WAITING;	// holds mode change information
xdata u08 new_samp = 0;						// sample rate change
xdata u08 new_iocollect = 0;			// i/o point selection change
xdata u08 outstanding_msg[NUMUNITS];	// lists which messages are outstanding from each device
u16 pagecnt = 0;						// which page to write
u16 iopagecnt = 0;						// which io page to write
xdata u16 debounce = 0;				// counts up the button debounce period
xdata u08 iocollect = 0;			// which i/o point to collect
xdata u08 ioscaling[NUMANALOGS];	// max voltage range, number of bits to shift, 20V = 4, 10V = 3, 5V = 2
xdata u08 ubxfullset = 0;			// 7 when all 3 messages received
xdata gpstype previousgps[2];		// last two gps samples samples - used for dead reckoning
xdata u08 lastgps = 0;				// index into previousgps of last collected gps sample
xdata digouttype digout;			// used for quick access to digital output definitions during operation
xdata s08 currenttemp;				// current temperature reading
xdata u08 cyl;						// number of cylinders for currently selected car
xdata u16 revs;						// revwarning for currently selected car
xdata BOOL ubxdone = FALSE;			// TRUE when ubx has been initialized
xdata BOOL su2 = FALSE;				// TRUE when SU2.0 board is in operation (LEA5 GPS, 3 axis, AutoOn, I2C ...)
xdata accessorytype accessory = NOACC;	// which external expansion modules are connected
xdata BOOL duconnected = FALSE;		// TRUE if we have communicated with DU
xdata BOOL simgps = FALSE;			// TRUE if we are simulating GPS signal

#define GOPROCAMHOLDTIME	2100L										// number of ms to hold button on chasecam and gopro
#define GOPROCAMHOLDTICKS	(GOPROCAMHOLDTIME * SAMPLE_RATE / 1000L)	// number of ticks to hold button
#define REPLAYCAMHOLDTIME	750L										// number of ms to hold button on replay, 650 too short, 700 works, go with 750
#define REPLAYCAMHOLDTICKS	(REPLAYCAMHOLDTIME * SAMPLE_RATE / 1000L)	// number of ticks to hold button
#define GOPROOFFTIME		2800L										// number of ms to hold gopro button to turn off, 3000 works, 2800 good
#define GOPROHOLDTICKS	(GOPROOFFTIME * SAMPLE_RATE / 1000L)			// number of ticks to hold button

// parameters for each system type (basic, complete), speed (10,20,40) and camera at each sample rate
// cam type (0=pulse, 1=level), data delay (msec), gps delay (msec), rec on hold time (msec), rec off hold time (msec)

xdata int cameracontrol[NUMBER_CAMERAS][NUMBER_TRAQMATE_TYPES][NUMBER_SAMPLERATES][NUMBER_CAMERA_PARAMETERS] = {
// X = CALIBRATED
// if viewing screen of traqdash on video, video early = subtract from value in table
// if matching lateral g pulse to video using traqstudio, video early = add to value in table
//
// *** camera delay is not used at this time.
//
//		signal	data 	camera	rec on	rec off
//		type	delay	delay	pulse	pulse
	{{{	LEVEL,	7900,		50,		0,		0 },							// generic usb, basic, 10 -- should be same as mobius
	{	LEVEL,	7000,		50,		0,		0 },							// generic usb, basic, 20
	{	LEVEL,	6800,		50,		0,		0 }},							// generic usb, basic, 40
	{{	LEVEL,	7900,		50,		0,		0 },							// generic usb, complete, 10
	{	LEVEL,	7000,		50,		0,		0 },							// generic usb, complete, 20
	{	LEVEL,	6000,		50,		0,		0 }}},							// X generic usb, complete, 40

	{{{	LANC,	200,		50,		GOPROCAMHOLDTIME,	GOPROOFFTIME },		// lanc, basic, 10
	{	LANC,	600,		50,		GOPROCAMHOLDTIME,	GOPROOFFTIME },		// lanc, basic, 20
	{	LANC,	900,		50,		GOPROCAMHOLDTIME,	GOPROOFFTIME }},	// lanc, basic, 40
	{{	LANC,	200,		50,		GOPROCAMHOLDTIME,	GOPROOFFTIME },		// lanc, complete, 10
	{	LANC,	200,		50,		GOPROCAMHOLDTIME,	GOPROOFFTIME },		// lanc, complete, 20
	{	LANC,	100,		50,		GOPROCAMHOLDTIME,	GOPROOFFTIME }}},	// X lanc, complete, 40
			
	{{{	PULSE,	7325,		50, 	GOPROCAMHOLDTIME,	GOPROOFFTIME },		// gopro, basic, 10
	{	PULSE,	6325,		50, 	GOPROCAMHOLDTIME,	GOPROOFFTIME },		// gopro, basic, 20
	{	PULSE,	6125,		50, 	GOPROCAMHOLDTIME,	GOPROOFFTIME }},	// gopro, basic, 40
	{{	PULSE,	7325,		50, 	GOPROCAMHOLDTIME,	GOPROOFFTIME },		// gopro, complete, 10
	{	PULSE,	6325,		50, 	GOPROCAMHOLDTIME,	GOPROOFFTIME },		// gopro, complete, 20
	{	PULSE,	5325,		50, 	GOPROCAMHOLDTIME,	GOPROOFFTIME }}},	// gopro, complete, 40

	{{{	PULSE,	8500,		50, 	REPLAYCAMHOLDTIME,	GOPROOFFTIME },		// replayxd, basic, 10
	{	PULSE,	8500,		50, 	REPLAYCAMHOLDTIME,	GOPROOFFTIME },		// replayxd, basic, 20
	{	PULSE,	10300,		50, 	REPLAYCAMHOLDTIME,	GOPROOFFTIME }},	// replayxd, basic, 40
	{{	PULSE,	8500,		50, 	REPLAYCAMHOLDTIME,	GOPROOFFTIME },		// replayxd, complete, 10
	{	PULSE,	8500,		50, 	REPLAYCAMHOLDTIME,	GOPROOFFTIME },		// replayxd, complete, 20
	{	PULSE,	9500,		50, 	REPLAYCAMHOLDTIME,	GOPROOFFTIME }}},	// X replayxd, complete, 40

	{{{	LEVEL,	12000,		50,		0,		0 },							// replayxdusb, basic, 10
	{	LEVEL,	9500,		50,		0,		0 },							// replayxdusb, basic, 20
	{	LEVEL,	9200,		50,		0,		0 }},							// replayxdusb, basic, 40
	{{	LEVEL,	12000,		50,		0,		0 },							// replayxdusb, complete, 10
	{	LEVEL,	9500,		50,		0,		0 },							// replayxdusb, complete, 20
	{	LEVEL,	8400,		50,		0,		0 }}},							// replayxdusb, complete, 40

	{{{	LEVEL,	7900,		50,		0,		0 },							// mobius, basic, 10
	{	LEVEL,	7000,		50,		0,		0 },							// mobius, basic, 20
	{	LEVEL,	6800,		50,		0,		0 }},							// x mobius, basic, 40
	{{	LEVEL,	7900,		50,		0,		0 },							// X mobius, complete, 10
	{	LEVEL,	7000,		50,		0,		0 },							// X mobius, complete, 20
	{	LEVEL,	6000,		50,		0,		0 }}},							// X mobius, complete, 40
	
	{{{	LEVEL,	8100,		50,		0,		0 },							// replaymini, basic, 10
	{	LEVEL,	7300,		50,		0,		0 },							// replaymini, basic, 20
	{	LEVEL,	10100,		50,		0,		0 }},							// replaymini, basic, 40
	{{	LEVEL,	8100,		50,		0,		0 },							// X replaymini, complete, 10
	{	LEVEL,	7300,		50,		0,		0 },							// X replaymini, complete, 20
	{	LEVEL,	9300,		50,		0,		0 }}},							// X replaymini, complete, 40
	
	{{{	LEVEL,	8100,		50,		0,		0 },							// replayprimex, basic, 10
	{	LEVEL,	7300,		50,		0,		0 },							// replayprimex, basic, 20
	{	LEVEL,	9200,		50,		0,		0 }},							// replayprimex, basic, 40
	{{	LEVEL,	8100,		50,		0,		0 },							// replayprimex, complete, 10
	{	LEVEL,	7300,		50,		0,		0 },							// replayprimex, complete, 20
	{	LEVEL,	8400,		50,		0,		0 }}}							// replayprimex, complete, 40
}; // camera control matrix


#ifndef DISPLAYATTACHED
xdata u32 sessstarttime = 0xFFFFFFFF;	// these are the first and last good dates for a session
xdata u32 sessendtime = 0xFFFFFFFF;
xdata u16 sessstartweeks = 0xFFFF;
xdata u16 sessendweeks = 0xFFFF;
xdata u16 iobyte;					// next byte to write
#endif

#else
extern xdata comporthandle com[NUMPORTS];	// handles to com ports
extern xdata smbusporthandle smbus;			// handle to smbus
extern xdata u16 result[NUM_CHAN];			// array to hold ADC samples    
extern code s08 msgrouting[NUMUNITS+1];	// what port to route messages to
extern xdata u08 CTS[NUMUNITS];		// non-zero when waiting for response, 0 when clear to send
extern xdata tmstatetype tm_state;
extern xdata u08 samps_per_sec;		// number of samples per second
extern xdata camlanctype camlanc;	// state machine controls for turning camera and lanc on/off
extern xdata u16 stoprecordtick;	// which second to stop recording if vehicle not in motion
extern xdata u08 session;			// which session we are working on
extern u16 int_dec;				    // integrate/decimate counter
extern xdata u08 adcresults;		// TRUE when samples collected
extern xdata u16 period0;			// current period measurement for Freq0 input (RPM), in freq0ticks
extern xdata u16 periodbuff[RPM_AVERAGING];		// last several period measurements for averaging
extern xdata u16 negperiod0;		// current period measurement form a negative going pulse (anti-falsing)
extern xdata u08 gpspulse;			// TRUE when GPS PPS signal received
extern xdata u16 gps_past_due;		// on which clock tick GPS PPS will be overdue
extern xdata u08 txbufcnt;			// used for UART0 prints
extern xdata u16 clocktick;			// increments in 1/SAMPLE_RATE sec intervals
extern xdata u16 secondtick;				// increments in one second intervals for timing slow events
extern xdata u16 nextsecond;				// which clocktick the next second will appear
extern xdata u08 mode_change;		// indicates change in state machine
extern xdata flashpagetype flashpage;	// structure to access one page of flash
extern xdata scratchpadtype scratch;	// structure to access scratchpad memory
extern xdata u08 gpslock;				// 0 when no lock, non-zero when lock
extern xdata u16 dop;					// gps dillution of precision * 100
extern xdata u16 pagesize;			// page size of dataflash
extern xdata u16 numpages;			// number of pages in dataflash
extern xdata BOOL resend;			// true if necessary to resend last message
extern xdata BOOL calibrating;		// true when in calibration mode
extern xdata tmstatetype new_mode;	// holds mode change information
extern xdata u08 new_samp;			// sample rate change
extern xdata u08 new_iocollect;		// i/o point selection change
extern xdata u08 outstanding_msg[NUMUNITS];	// lists which messages are outstanding from each device
extern u16 pagecnt;					// which page to write
extern u16 iopagecnt;				// which io page to write
extern xdata u16 debounce;			// counts up the button debounce period
extern xdata u08 iocollect;			// which i/o point to collect
extern xdata u08 ioscaling[NUMANALOGS];	// max voltage range, number of bits to shift, 20V = 4, 10V = 3, 5V = 2
extern xdata u08 ubxfullset;			// 7 when all 3 messages received
extern xdata gpstype previousgps[2];	// last two gps samples samples - used for dead reckoning
extern xdata u08 lastgps;				// index into previousgps of last collected gps sample
extern xdata s08 currenttemp;			// current temperature reading
extern xdata u08 cyl;					// number of cylinders for currently selected car
extern xdata u16 revs;					// revwarning for currently selected car
extern xdata BOOL ubxdone;			// TRUE when ubx has been initialized
extern xdata BOOL su2;				// TRUE when SU2.0 board is in operation (LEA5 GPS, 3 axis, AutoOn, I2C ...)
extern xdata accessorytype accessory;	// which external expansion modules are connected
extern xdata BOOL duconnected;		// TRUE if we have communicated with DU
extern xdata BOOL simgps;			// TRUE if we are simulating GPS signal
extern int cameracontrol[NUMBER_CAMERAS][NUMBER_TRAQMATE_TYPES][NUMBER_SAMPLERATES][NUMBER_CAMERA_PARAMETERS];

#ifndef DISPLAYATTACHED
extern xdata u32 sessstarttime;		// these are the first and last good dates for a session
extern xdata u32 sessendtime;
extern xdata u16 sessstartweeks;
extern xdata u16 sessendweeks;
extern xdata u16 iobyte;			// next byte to write
#endif

#endif
