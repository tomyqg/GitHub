/* This is a collection of test routines that are used to checkout the product at the factory

For Traqmate Sensor Unit 2 axis units this is the algorithm:
Sit level, allow to settle.
Adjust PWM values until both X and Y read zero.
Store PWM values in microprocessor flash.
Rotate through positions to get ranges for x and y.

For Traqmate Sensor Unit 3 axis units this is the algorithm:
Sit level, allow to settle.
Rotate through positions to get ranges for x and y.

*/

#ifdef SENSOR
#include <sensor.h>
#endif
#ifdef DISPLAY
#include <display.h>
#endif

void Stabilize(void);
void ZeroOutXY(void);

#define STABILITY	4					// range of values to allow
#define CALIBRATED	20					// max variation from midpoint to allow
#define PWMHI		255					// highest PWM calibration value
#define PWMLO		0					// lowest PWM calibration value
//#define STARTZERO	(PWMHI-PWMLO)/2		// middle of road value to start with
#define STARTZERO	160					// approximate PWM value for current hardware
#define SETTLING	20					// number of samples to collect to see if settled
#define MIDPOINT	2048				// midpoint of accelerometer range (12 bits)

#define TEXTDELAY	150					// number of ms to wait between text writes

#ifdef SENSOR
// globals
xdata u16 minx, maxx, miny, maxy, minz, maxz, mintemp, maxtemp;

void Calibrate(void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	u16 temp;
	xdata char textline[80];

	calibrating = TRUE;
	SFRPAGE = CONFIG_PAGE;

	// get current values
	ReadScratchpad(scratch.scratchbuff);

	// initialize
	scratch.calibrate.timecode = (s08) 0;
	scratch.calibrate.model = msgrouting[0];		// what are we?
	scratch.calibrate.hwrev = HWREV;
	scratch.calibrate.swrev = SWREV;
	scratch.calibrate.datarev = DATAREV;
	if (0xFFFFFFFF == scratch.calibrate.serno)
		scratch.calibrate.serno = 0;
	if (0xFF == scratch.calibrate.week)
		scratch.calibrate.week = 1;
	if (0xFF == scratch.calibrate.year)
		scratch.calibrate.year = CURRENTYEAR;
	scratch.calibrate.contrast = 0xFF;
	scratch.calibrate.hwconfig = 0xFF;
	scratch.calibrate.xzero = MIDPOINT;
	scratch.calibrate.yzero = MIDPOINT;
	scratch.calibrate.owner[0] = '\0';
	scratch.calibrate.phone[0] = '\0';
	scratch.calibrate.note[0] = '\0';

	scratch.calibrate.calx = scratch.calibrate.caly = STARTZERO;

	debounce = 0;

	HI(LED0); HI(LED1); HI(LED2); HI(LED3);

	// zero x, y, get max z
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Place unit flat on level surface.");

	Stabilize();
//	while (!adcresults) ;
//	scratch.calibrate.xzero = result[0];
//	scratch.calibrate.yzero = result[1];
//	scratch.calibrate.zplusg = result[3];
	scratch.calibrate.xzero = AVG(minx, maxx);
	scratch.calibrate.yzero = AVG(miny, maxy);
	scratch.calibrate.zplusg = AVG(minz, maxz);
	adcresults = FALSE;

	LO(LED0); LO(LED2);
	
	// min z
	DELAY_MS(TEXTDELAY);	
	PRINTPC0( "Flip over. Face down.");

	Stabilize();
//	while (!adcresults) ;
//	scratch.calibrate.zminusg = result[3];
	scratch.calibrate.zminusg = AVG(minz, maxz);
	adcresults = FALSE;

	LO(LED0); LO(LED1); LO(LED2); LO(LED3);
	
	// max x, zero z
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Hold vertical, GPS antenna jack up.");

	Stabilize();
//	while (!adcresults) ;
//	scratch.calibrate.xplusg = result[0];
//	scratch.calibrate.zzero = result[3];
	scratch.calibrate.xplusg = AVG(minx, maxx);
	scratch.calibrate.zzero = AVG(minz, maxz);
	adcresults = FALSE;

	HI(LED0);

	// min x
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Hold vertical, GPS antenna jack down.");

	Stabilize();
//	while (!adcresults) ;
//	scratch.calibrate.xminusg = result[0];
	scratch.calibrate.xminusg = AVG(minx, maxx);
	adcresults = FALSE;
	
	HI(LED1);

	// min y
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Stand on right side.");

	Stabilize();
//	while (!adcresults) ;
//	scratch.calibrate.yminusg = result[1];
	scratch.calibrate.yminusg = AVG(miny, maxy);
	adcresults = FALSE;
	
	HI(LED2);

	// max y
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Stand on left side.");

	Stabilize();
//	while (!adcresults) ;
//	scratch.calibrate.yplusg = result[1];
	scratch.calibrate.yplusg = AVG(miny, maxy);
	adcresults = FALSE;
	
	HI(LED3);

	if (scratch.calibrate.xplusg < scratch.calibrate.xminusg) {
		DELAY_MS(TEXTDELAY);
		PRINTPC0("Inverting X Values");
		temp = scratch.calibrate.xplusg;
		scratch.calibrate.xplusg = scratch.calibrate.xminusg;
		scratch.calibrate.xminusg = temp;
	}
	if (scratch.calibrate.yplusg < scratch.calibrate.yminusg) {
		DELAY_MS(TEXTDELAY);
		PRINTPC0("Inverting Y Values");
		temp = scratch.calibrate.yplusg;
		scratch.calibrate.yplusg = scratch.calibrate.yminusg;
		scratch.calibrate.yminusg = temp;
	}
	if (scratch.calibrate.zplusg < scratch.calibrate.zminusg) {
		DELAY_MS(TEXTDELAY);
		PRINTPC0("Inverting Z Values");
		temp = scratch.calibrate.zplusg;
		scratch.calibrate.zplusg = scratch.calibrate.zminusg;
		scratch.calibrate.zminusg = temp;
	}

	DELAY_MS(TEXTDELAY);
	PRINTPC0( " ");
	DELAY_MS(TEXTDELAY);
//	PRINTPC2( "Calibration Values: %u, %u", scratch.calibrate.calx, scratch.calibrate.caly);
//	DELAY_MS(TEXTDELAY);	// wait for message to go out
	sprintf(textline, "Zero Points: X %u, Y %u, Z %u", scratch.calibrate.xzero, scratch.calibrate.yzero, scratch.calibrate.zzero);
	PRINTPC0(textline);
	DELAY_MS(TEXTDELAY);	// wait for message to go out	
	PRINTPC2( "X Range: %d, %d", scratch.calibrate.xminusg, scratch.calibrate.xplusg);
	DELAY_MS(TEXTDELAY);	// wait for message to go out
	PRINTPC2( "Y Range: %d, %d", scratch.calibrate.yminusg, scratch.calibrate.yplusg);
	DELAY_MS(TEXTDELAY);	// wait for message to go out
	PRINTPC2( "Z Range: %d, %d", scratch.calibrate.zminusg, scratch.calibrate.zplusg);
	DELAY_MS(TEXTDELAY);	// wait for message to go out
	PRINTPC0( "Storing Calibration Data in Flash.");

	EraseScratchpad();

	WriteScratchpad(scratch.scratchbuff, sizeof(caltype));

	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Calibration Complete.");

	HI(LED0); LO(LED1); LO(LED2); LO(LED3);

	calibrating = FALSE;

	// get the configuration data
	InitSUScratchpad();

	SFRPAGE = SFRPAGE_SAVE;
} // Calibrate

void InputTest(void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	xdata u08 mask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
	xdata char textline[80];

	calibrating = TRUE;

	// initialize
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Traqmate Input Test");

	debounce = 0;
	while (1) {
		u08 port5;
	   	int i;

		SFRPAGE = CONFIG_PAGE;
		if (0 != SW1) {
			// see if it has just been released
			if (debounce >= DEBOUNCE_NORMAL) break;

			debounce = 0;
		} // if

		if (adcresults) {
			adcresults = FALSE;

			port5 = P5;

			DELAY_MS(1000);
			PRINTPC0(" ");

			DELAY_MS(TEXTDELAY);
			sprintf(textline, "DIGIN: ");
			for (i = 6; i >= 0; i--)
				sprintf(textline+strlen(textline), (port5 & mask[i])? "1" : "0");
			PRINTPC0(textline);

			DELAY_MS(TEXTDELAY);
			sprintf(textline, "X: %u, Y: %u", result[0], result[1]);
			PRINTPC0(textline);

			DELAY_MS(TEXTDELAY);
			sprintf(textline, "5V: %u, Z: %u", result[2], result[3]);
			PRINTPC0(textline);

			DELAY_MS(TEXTDELAY);
			sprintf(textline, "A0: %u, A1: %u", result[4], result[5]);
			PRINTPC0(textline);

			DELAY_MS(TEXTDELAY);
			sprintf(textline, "A2: %u, A3: %u", result[6], result[7]);
			PRINTPC0(textline);
		} // if

	} // while
	debounce = 0;

	DELAY_MS(TEXTDELAY);
	PRINTPC0( "InputTest Complete.");

	HI(LED0); LO(LED1); LO(LED2); LO(LED3);

	calibrating = FALSE;

	SFRPAGE = SFRPAGE_SAVE;
} // InputTest

void OutputTest(void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	u16 clocksave;			// saves the clock for elapsed time calculations
	int bitnum = 0;
	xdata u08 mask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

#define BIT_CHANGE_RATE	1	// once per second

	// initialize
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Traqmate Output Test");
	DELAY_MS(TEXTDELAY);

	debounce = 0;
	clocksave = clocktick + (SAMPLE_RATE*BIT_CHANGE_RATE);	// change bit every 1 seconds

	while (1) {

		SFRPAGE = CONFIG_PAGE;
		if (0 != SW1) {
			// see if it has just been released
			if (debounce >= DEBOUNCE_NORMAL) break;

			debounce = 0;
		} // if

		if (clocksave == clocktick) {
			// get next tickover
			clocksave = clocktick + (SAMPLE_RATE*BIT_CHANGE_RATE);	// change bit every 1 seconds

			PRINTPC1("bit %d", bitnum);

			P4 = mask[bitnum];
			bitnum = ++bitnum % 7;
		} // if

	} // while
	debounce = 0;

	DELAY_MS(TEXTDELAY);
	PRINTPC0( "OutputTest Complete.");

	HI(LED0); LO(LED1); LO(LED2); LO(LED3);

	SFRPAGE = SFRPAGE_SAVE;
} // OutputTest

void Stabilize( void ) {
	int counter;
//	xdata char textline[80];

	do {		// sit here until values stabilize
		// initialize
		minx = miny = minz = 4095;
		maxx = maxy = maxz = 0;

		adcresults = FALSE;

		for (counter=0; counter < SETTLING; counter++) { // collect several values to compare
			// wait for the next valid sample
			while (!adcresults) ;
//				if (!(counter % 20)) {
//					DELAY_MS(TEXTDELAY);
//					sprintf(textline, "%u,%u", result[0], result[1]);
//					PRINTPC0(textline);
//				}
			minx = MIN(minx, result[0]);
			miny = MIN(miny, result[1]);
			minz = MIN(minz, result[3]);

			maxx = MAX(maxx, result[0]);
			maxy = MAX(maxy, result[1]);
			maxz = MAX(maxz, result[3]);

			adcresults = FALSE;
		} // for
	} while ((ABS(maxy - miny) > STABILITY) || (ABS(maxx - minx) > STABILITY));
	
//	DELAY_MS(TEXTDELAY);
//	PRINTPC0("Stabilized");
} // Stabilize

#if 0
void ZeroOutXY( void ) {

	unsigned char xdone = FALSE;
	unsigned char ydone = FALSE;
	xdata char textline[80];

	// wait for a fresh sample
	while (!adcresults) ;
	adcresults = FALSE;

	// record the actual zero points for x,y and z max point
	scratch.calibrate.xzero = result[0];
	scratch.calibrate.yzero = result[1];
	scratch.calibrate.zmax = result[3];

	DELAY_MS(TEXTDELAY);
	sprintf(textline, "Final Vals: %u,%u,%u", scratch.calibrate.xzero, scratch.calibrate.yzero, scratch.calibrate.zmax);
	PRINTPC0(textline);

} // ZeroOutXY
#endif
#endif // SENSOR

void DataflashTest(void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	int pagenum = 0;
	int i;
	BOOL flashfail = FALSE;

	// initialize
	PRINTPC0( "Dataflash Test");

#ifdef SENSOR
	debounce = 0;
#else
	debounce[SEL] = 0;
#endif

	// initialize page to write
	for (i=0; i<pagesize; i++) flashpage.bigbuff[i] = i % 256;

	while (1) {

		SFRPAGE = CONFIG_PAGE;
#ifdef SENSOR
		if (0 != SW1) {
			// see if it has just been released
			if (debounce >= DEBOUNCE_NORMAL) break;

			debounce = 0;
		} // if
#else
		if (ISHI(SEL_SW)) {
			// see if it has just been released
			if (debounce[SEL] >= DEBOUNCE_NORMAL) break;

			debounce[SEL] = 0;
		} // if
#endif

		// write buffer to flash
		DataFlash_Page_Write_Erase(0, pagesize, flashpage.bigbuff, pagenum);

		// read back and verify
		DataFlash_Page_Read(pagesize, flashpage.bigbuff, pagenum);

		// initialize page to write
		for (i=0; i<pagesize; i++) {
			if (flashpage.bigbuff[i] != (i % 256)) {
				flashfail = TRUE;
				break;
			} // if
		} // for

		if (flashfail) {
			PRINTPC2("****** Flash Write FAILED ******, page %d, byte%d", pagenum, i);
			break;
		} // if

		DELAY_MS(TEXTDELAY);
		PRINTPC1("Flash ok page %d", pagenum);

		// erase the page
		DataFlash_Erase_Page(pagenum);

		// at the end yet?
		if (pagenum >= numpages)
			pagenum = 0;
		else 
			// go to next page
			pagenum++;

	} // while
#ifdef SENSOR
	debounce = 0;
#else
	debounce[SEL] = 0;
#endif

	DELAY_MS(TEXTDELAY);
	PRINTPC0( "DataflashTest Complete.");

	SFRPAGE = SFRPAGE_SAVE;
} // DataflashTest


