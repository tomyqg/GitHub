/* This is a collection of test routines that are used to checkout the product at the factory

For Traqmate Sensor Unit 2 axis units this is the algorithm:
Sit level, allow to settle.
Adjust PWM values until both X and Y read zero.
Store PWM values in microprocessor flash.
Rotate through positions to get ranges for x and y.

For Traqmate Sensor Unit 3 axis units this is the algorithm:
Sit level, allow to settle.
Rotate through positions to get ranges for x, y, and z.

*/

#ifdef SENSOR
#include <sensor.h>
#endif
#ifdef DISPLAY
#include <display.h>
#endif

void Stabilize(void);
void ZeroOutXY(void);
void dfwait(void);

#define STABILITY	4					// range of values to allow
#define CALIBRATED	5					// max variation from midpoint to allow. 3 is too small.
#define PWMHI		255					// highest PWM calibration value
#define PWMLO		0					// lowest PWM calibration value
//#define STARTZERO	(PWMHI-PWMLO)/2		// middle of road value to start with
#define STARTZERO	160					// approximate PWM value for current hardware, speeds up cal
#define SETTLING	40					// number of samples to collect to see if settled
#define MINACCEL	0					// minimum accelerometer reading
#define MAXACCEL	4095				// maximum accelerometer reading
#define MIDACCEL	((MAXACCEL+1-MINACCEL)/2)	// midpoint of accelerometer range (12 bits)

#define TEXTDELAY	200					// number of ms to wait between text writes (150 just slightly too short)

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
	if (su2)
		scratch.calibrate.hwrev = HWREVSU2;
	else
		scratch.calibrate.hwrev = HWREVSU1;
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
	scratch.calibrate.xzero = MIDACCEL;
	scratch.calibrate.yzero = MIDACCEL;
	scratch.calibrate.owner[0] = '\0';
	scratch.calibrate.phone[0] = '\0';
	scratch.calibrate.note[0] = '\0';

	scratch.calibrate.calx = scratch.calibrate.caly = STARTZERO;

	debounce = 0;

	LED0 = 1; LED1 = 1; LED2 = 1; LED3 = 1;

	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Place unit flat on level surface.");
	DELAY_MS(TEXTDELAY);	// give the user some time

	Stabilize();

	if (su2) {
		scratch.calibrate.xzero = AVG(minx, maxx);
		scratch.calibrate.yzero = AVG(miny, maxy);
		scratch.calibrate.zplusg = AVG(minz, maxz);
	} // if
	else
		ZeroOutXY();

	if (su2) {
		LED0 = 0; LED2 = 0;
		
		// min z
		PRINTPC0( "Flip over. Face down.");
		DELAY_MS(TEXTDELAY);

		Stabilize();
		scratch.calibrate.zminusg = AVG(minz, maxz);
	} // if

	LED0 = 0; LED1 = 0; LED2 = 0; LED3 = 0;
	
	// max x, zero z
	PRINTPC0( "Hold vertical, GPS antenna jack up.");
	DELAY_MS(TEXTDELAY);

	Stabilize();
	scratch.calibrate.xplusg = AVG(minx, maxx);

	if (su2)
		scratch.calibrate.zzero = AVG(minz, maxz);

	LED0 = 1;

	// min x
	PRINTPC0( "Hold vertical, GPS antenna jack down.");
	DELAY_MS(TEXTDELAY);

	Stabilize();
	scratch.calibrate.xminusg = AVG(minx, maxx);
	
	LED1 = 1;

	// min y
	PRINTPC0( "Stand on right side.");
	DELAY_MS(TEXTDELAY);

	Stabilize();
	scratch.calibrate.yminusg = AVG(miny, maxy);
	
	LED2 = 1;

	// max y
	PRINTPC0( "Stand on left side.");
	DELAY_MS(TEXTDELAY);

	Stabilize();
	scratch.calibrate.yplusg = AVG(miny, maxy);
	
	LED3 = 1;

	if (scratch.calibrate.xplusg < scratch.calibrate.xminusg) {
		PRINTPC0("Inverting X Values");
		DELAY_MS(TEXTDELAY);
		temp = scratch.calibrate.xplusg;
		scratch.calibrate.xplusg = scratch.calibrate.xminusg;
		scratch.calibrate.xminusg = temp;
	}
	if (scratch.calibrate.yplusg < scratch.calibrate.yminusg) {
		PRINTPC0("Inverting Y Values");
		DELAY_MS(TEXTDELAY);		
		temp = scratch.calibrate.yplusg;
		scratch.calibrate.yplusg = scratch.calibrate.yminusg;
		scratch.calibrate.yminusg = temp;
	}

	if (su2) {
		if (scratch.calibrate.zplusg < scratch.calibrate.zminusg) {
			PRINTPC0("Inverting Z Values");
			DELAY_MS(TEXTDELAY);
			temp = scratch.calibrate.zplusg;
			scratch.calibrate.zplusg = scratch.calibrate.zminusg;
			scratch.calibrate.zminusg = temp;
		}
	} // if

	PRINTPC0( " ");
	DELAY_MS(TEXTDELAY);
//	PRINTPC2( "Calibration Values: %u, %u", scratch.calibrate.calx, scratch.calibrate.caly);
//	DELAY_MS(TEXTDELAY);	// wait for message to go out
	sprintf(textline, "Zero Points: X %u, Y %u, Z %u", scratch.calibrate.xzero, scratch.calibrate.yzero, scratch.calibrate.zzero);
	PRINTPC0(textline);
	DELAY_MS(TEXTDELAY);	// wait for message to go out	
	PRINTPC2( "X Range: %u, %u", scratch.calibrate.xminusg, scratch.calibrate.xplusg);
	DELAY_MS(TEXTDELAY);	// wait for message to go out
	PRINTPC2( "Y Range: %u, %u", scratch.calibrate.yminusg, scratch.calibrate.yplusg);
	DELAY_MS(TEXTDELAY);	// wait for message to go out
	PRINTPC2( "Z Range: %u, %u", scratch.calibrate.zminusg, scratch.calibrate.zplusg);
	DELAY_MS(TEXTDELAY);	// wait for message to go out
	PRINTPC0( "Storing Calibration Data in Flash.");

	EraseScratchpad();

	WriteScratchpad(scratch.scratchbuff, sizeof(caltype));

	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Calibration Complete.");

	LED0 = 1; LED1 = 0; LED2 = 0; LED3 = 0;

	calibrating = FALSE;

	// get the configuration data
	InitSUScratchpad();

	SFRPAGE = SFRPAGE_SAVE;
} // Calibrate

void Stabilize( void ) {
	int counter;

	do {		// sit here until values stabilize
		// initialize
		minx = miny = minz = MAXACCEL;
		maxx = maxy = maxz = MINACCEL;

		for (counter=0; counter < SETTLING; counter++) { // collect several values to compare
			// wait for the next valid sample
			while (!adcresults) ;

			minx = MIN(minx, result[0]);
			miny = MIN(miny, result[1]);
			minz = MIN(minz, result[3]);

			maxx = MAX(maxx, result[0]);
			maxy = MAX(maxy, result[1]);
			maxz = MAX(maxz, result[3]);

			adcresults = FALSE;
		} // for
	} while ((ABS(maxy - miny) > STABILITY) || (ABS(maxx - minx) > STABILITY));
	
} // Stabilize

void ZeroOutXY( void ) {

	unsigned char xdone = FALSE;
	unsigned char ydone = FALSE;
	xdata char textline[80];

	SFRPAGE = PCA0_PAGE;

	// put initial values in PWM
	PCA0CPL1 =	scratch.calibrate.caly;
	PCA0CPH1 =	scratch.calibrate.caly;
	PCA0CPL0 =	scratch.calibrate.calx;
	PCA0CPH0 =	scratch.calibrate.calx;

	PRINTPC0("Zeroing X, Y PWM");
	DELAY_MS(TEXTDELAY);

	do {		// zero in on x,y
		// throw out any old results
		while (!adcresults) ;
		adcresults = FALSE;
	
		// get a round of samples with new PWM settings
		while (!adcresults) ;
		
		// record the actual zero points
		if (!xdone) scratch.calibrate.xzero = result[0];
		if (!ydone) scratch.calibrate.yzero = result[1];
		adcresults = FALSE;

//		DELAY_MS(TEXTDELAY);
//		PRINTPC2( "Cal: %d,%d", scratch.calibrate.calx, scratch.calibrate.caly);
//		sprintf(textline, "Val: %u,%u", scratch.calibrate.xzero, scratch.calibrate.yzero);
//		DELAY_MS(TEXTDELAY);
//		PRINTPC0(textline);

		if (!xdone) xdone = (ABS(((s16) scratch.calibrate.xzero)-MIDACCEL) <= CALIBRATED);
		if (!xdone) {
			if (scratch.calibrate.xzero < MIDACCEL)
				if (PWMLO == scratch.calibrate.calx)
					// couldn't get to middle so go with what we got
					xdone = TRUE;
				else
					// drop the cal value and try again
					scratch.calibrate.calx -= 1;
			else
				if (PWMHI == scratch.calibrate.calx)
					// couldn't get to middle so go with what we got
					xdone = TRUE;
				else
					// raise the cal value and try again
					scratch.calibrate.calx += 1;

			// reprogram the PWM
			PCA0CPL0 =	scratch.calibrate.calx;
			PCA0CPH0 =	scratch.calibrate.calx;
		}

		if (!ydone) ydone = (ABS(((s16) scratch.calibrate.yzero)-MIDACCEL) <= CALIBRATED);
		if (!ydone) {
			if (scratch.calibrate.yzero < MIDACCEL)
				if (PWMLO == scratch.calibrate.caly)
					// couldn't get to middle so go with what we got
					ydone = TRUE;
				else
					// drop the cal value and try again
					scratch.calibrate.caly -= 1;
			else
				if (PWMHI == scratch.calibrate.caly)
					// couldn't get to middle so go with what we got
					ydone = TRUE;
				else
					// raise the cal value and try again
					scratch.calibrate.caly += 1;

			// reprogram the PWM
			PCA0CPL1 =	scratch.calibrate.caly;
			PCA0CPH1 =	scratch.calibrate.caly;
		} // if
	} while (!(xdone && ydone));

//	DELAY_MS(TEXTDELAY);
//	PRINTPC0( "X, Y Zeroed Out");

	Stabilize();
	// record the actual zero points
	scratch.calibrate.xzero = result[0];
	scratch.calibrate.yzero = result[1];

	PRINTPC2( "Final Cal: %u,%u", scratch.calibrate.calx, scratch.calibrate.caly);
	DELAY_MS(TEXTDELAY);
	sprintf(textline, "Final Val: %u,%u", scratch.calibrate.xzero, scratch.calibrate.yzero);
	PRINTPC0(textline);
	DELAY_MS(TEXTDELAY);

} // ZeroOutXY
#endif // SENSOR

void DataflashTest(u08 whichtest) {
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

	do {
		SFRPAGE = CONFIG_PAGE;
#ifdef SENSOR
		if (0 != SW1) {
			// see if it has just been released
			if (debounce >= DEBOUNCE_NORMAL) break;

			debounce = 0;
		} // if
#else
		if (SEL_SW) {
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
			PRINTPC2("** Flash FAILED **, page %d, byte%d", pagenum, i);
			break;
		} // if

		DELAY_MS(TEXTDELAY);
		PRINTPC1("Page %d ok", pagenum);

		// erase the page
		DataFlash_Erase_Page(pagenum);

		// at the end yet?
		if (pagenum >= numpages)
			pagenum = 0;
		else 
			// go to next page
			pagenum++;

	} while (whichtest == DATAFLASHTEST || pagenum <= 4);

#ifdef SENSOR
	debounce = 0;
#else
	debounce[SEL] = 0;
#endif

	DELAY_MS((TEXTDELAY));
	PRINTPC0( "Test Complete.");

	SFRPAGE = SFRPAGE_SAVE;
} // DataflashTest

#ifdef SENSOR
void InputTest(void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	xdata u08 mask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
	xdata char textline[80];

	calibrating = TRUE;

	// initialize
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Input Test");

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
			if (su2)
				sprintf(textline, "Z: %u, 5V: %u", result[3], result[2]);
			else
				sprintf(textline, "5V: %u, T: %u", result[2], result[3]);			
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

	LED0 = 1; LED1 = 0; LED2 = 0; LED3 = 0;

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
	PRINTPC0( "Output Test");
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

	LED0 = 1; LED1 = 0; LED2 = 0; LED3 = 0;

	SFRPAGE = SFRPAGE_SAVE;
} // OutputTest

void ADTest(void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	xdata char textline[80];
	BOOL testgood;
	BOOL pass = TRUE;
	u08 i;

	textline[0] = '\0';
	calibrating = TRUE;

	// initialize
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "A/D Test");
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Board flat. Test fixture attached.");
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "X  Y  5V Z  A0 A1 A2 A3");

	// wait for the next valid sample set
	while (!adcresults) ;
	
	for (i=0; i<8; i++) {		// step through all 8 channels
		switch (i) {
			case 0:		// x and y accelerometers
			case 1:
				testgood = (result[i] >= 1700 && result[i] <= 2300);
				break;
			case 2:		// 5v voltage monitor
				testgood = (result[i] >= 3716 && result[i] <= 4252);
				break;
			case 3:		// z channel or temperature
				if (su2)
					testgood = (result[i] >= 1450 && result[i] <= 1900);
				else	// no z channel so its a gimme
					testgood = 1;
				break;
			case 4:		// analog input 0
				testgood = (result[i] >= 1072 && result[i] <= 1376);
				break;		
			case 5:		// analog input 1
				testgood = (result[i] >= 714 && result[i] <= 918);
				break;		
			case 6:		// analog input 2
				testgood = (result[i] >= 357 && result[i] <= 459);
				break;		
			case 7:		// analog input 3
				testgood = (result[i] >= 831 && result[i] <= 1067);
				break;
		} // switch
		// one failure equals test fail
		if (!testgood)
			pass = FALSE;
		sprintf(textline+strlen(textline), testgood? "1   " : "0   ");
	} // for
	adcresults = FALSE;

	DELAY_MS(TEXTDELAY);
	PRINTPC0(textline);

	DELAY_MS(TEXTDELAY);
	if (pass)
		PRINTPC0( "ADTest PASS.");
	else
		PRINTPC0( "** ADTest FAIL **");

	LED0 = 1; LED1 = 0; LED2 = 0; LED3 = 0;

	calibrating = FALSE;

	SFRPAGE = SFRPAGE_SAVE;
} // ADTest

void DigIOTest(void) {					// this test is only valid for SU2
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	BOOL pass = TRUE;
	BOOL testgood;
	u08 testval = 0x00;					// initial value = 0
	u08 i;

	calibrating = TRUE;

	// configure Comparator 1
	SFRPAGE = CPT1_PAGE;
	CPT1CN = 0x80;						// 10000000, CPT1 enabled, no hysteresis
	SFRPAGE = SFRPAGE_SAVE;

	// initialize
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "Digital IO Test");
	
	if (!su2) {
		DELAY_MS(TEXTDELAY);
		PRINTPC0( "Only Valid for SU2 - Exiting.");
	} // if
	else {
		for (i=0; i <= 8; i++) {		// step through all 8 channels
			if (8 == i) {
				P4 = 0x00;
				ENSMBPU = 1;
			} // if
			else {
				P4 = testval;
				ENSMBPU = 0;
			} // else
	
			DELAY_MS(TEXTDELAY);
			PRINTPC1("DigIOTest Point %d", (int) i);
	
			// wait for the next valid sample set
			while (!adcresults) ;
			
			// comparing with A1 values. (reads x4)	
			switch (i) {
				case 0:	// testval == 0x00
					SFRPAGE = SFRPAGE_SAVE;
					testgood = ((P5 & 0x7F) == testval) && (result[5] > 714) && (1 == SMBSDA) && (1 == SMBSCL);
					SFRPAGE = CPT0_PAGE;
					testgood = testgood && (0 == CP0OUT);
					SFRPAGE = CPT1_PAGE;					
					testgood = testgood && (0 == CP1OUT);
					SFRPAGE = SFRPAGE_SAVE;
					break;
				case 1:	// testval == 0x01
					SFRPAGE = SFRPAGE_SAVE;
					testgood = ((P5 & 0x7F) == testval) && (result[5] < 350) && (0 == SMBSDA) && (1 == SMBSCL);
					SFRPAGE = CPT0_PAGE;
					testgood = testgood && (0 == CP0OUT);
					SFRPAGE = CPT1_PAGE;					
					testgood = testgood && (0 == CP1OUT);
					SFRPAGE = SFRPAGE_SAVE;
					break;
				case 2:	// testval == 0x02
					SFRPAGE = SFRPAGE_SAVE;
					testgood = ((P5 & 0x7F) == testval) && (result[5] < 350) && (1 == SMBSDA) && (0 == SMBSCL);
					SFRPAGE = CPT0_PAGE;
					testgood = testgood && (0 == CP0OUT);
					SFRPAGE = CPT1_PAGE;					
					testgood = testgood && (0 == CP1OUT);
					SFRPAGE = SFRPAGE_SAVE;
					break;
				case 3:	// testval == 0x04
					SFRPAGE = SFRPAGE_SAVE;
					testgood = ((P5 & 0x7F) == testval) && (result[5] < 350) && (1 == SMBSDA) && (1 == SMBSCL);
					SFRPAGE = CPT0_PAGE;
					testgood = testgood && (0 == CP0OUT);
					SFRPAGE = CPT1_PAGE;					
					testgood = testgood && (0 == CP1OUT);
					SFRPAGE = SFRPAGE_SAVE;
					break;
				case 4:	// testval == 0x08
					SFRPAGE = SFRPAGE_SAVE;
					testgood = ((P5 & 0x7F) == testval) && (result[5] < 350) && (1 == SMBSDA) && (1 == SMBSCL);
					SFRPAGE = CPT0_PAGE;
					testgood = testgood && (0 == CP0OUT);
					SFRPAGE = CPT1_PAGE;					
					testgood = testgood && (0 == CP1OUT);
					SFRPAGE = SFRPAGE_SAVE;
					break;
				case 5:	// testval == 0x10
					SFRPAGE = SFRPAGE_SAVE;
					testgood = ((P5 & 0x7F) == testval) && (result[5] < 350) && (1 == SMBSDA) && (1 == SMBSCL);
					SFRPAGE = CPT0_PAGE;
					testgood = testgood && (0 == CP0OUT);
					SFRPAGE = CPT1_PAGE;					
					testgood = testgood && (0 == CP1OUT);
					SFRPAGE = SFRPAGE_SAVE;
					break;
				case 6:	// testval == 0x20
					SFRPAGE = SFRPAGE_SAVE;
					testgood = ((P5 & 0x7F) == testval) && (result[5] < 350) && (1 == SMBSDA) && (1 == SMBSCL);
					SFRPAGE = CPT0_PAGE;
					testgood = testgood && (0 == CP0OUT);
					SFRPAGE = CPT1_PAGE;					
					testgood = testgood && (1 == CP1OUT);
					SFRPAGE = SFRPAGE_SAVE;
					break;
				case 7:	// testval == 0x40
					SFRPAGE = SFRPAGE_SAVE;
					testgood = ((P5 & 0x7F) == testval) && (result[5] < 350) && (1 == SMBSDA) && (1 == SMBSCL);
					SFRPAGE = CPT0_PAGE;
					testgood = testgood && (1 == CP0OUT);
					SFRPAGE = CPT1_PAGE;					
					testgood = testgood && (0 == CP1OUT);
					SFRPAGE = SFRPAGE_SAVE;
					break;
				case 8:	// special case
//	commented out waiting for hardware change to test rig, GAS 1/11/10
//					testgood = (0x03 == testval) && (0 == SMBSDA) && (0 == SMBSCL);
					testgood = TRUE;
					break;
			} // switch
			
			adcresults = FALSE;
					
			if (!testgood) {
				DELAY_MS(TEXTDELAY);
				PRINTPC0("**  FAIL **");
				pass = FALSE;
			} // if
	
			// get the next value
			if (i == 0)
				testval = 0x01;
			else
				testval = testval << 1;		  // walk a 1 up through the test value
		} // for
	
		DELAY_MS(TEXTDELAY);
		if (pass)
			PRINTPC0( "DigIOTest PASS.");
		else
			PRINTPC0( "** DigIOTest FAIL **");
	} // else

	// configure Comparator 1
	SFRPAGE = CPT1_PAGE;
	CPT1CN = 0x00;						// 10000000, CPT1 disabled, no hysteresis
	
	LED0 = 1; LED1 = 0; LED2 = 0; LED3 = 0;

	calibrating = FALSE;

	SFRPAGE = SFRPAGE_SAVE;
} // DigIoTest

void AutoOnTest(void) {					// only valid for su2
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	calibrating = TRUE;

	// initialize
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "AutoOn Test");

	if (!su2) {
		DELAY_MS(TEXTDELAY);
		PRINTPC0( "Only Valid for SU2 - Exiting.");
	} // if
	else {
		P4 = 0x20;		// disable the ON/OFF signal at DIN connector
		
		DELAY_MS(TEXTDELAY);
		PRINTPC0( "Enabling AutoOn...");
		DigiPot(TRUE);	// turn on AutoOn feature

		P4 = 0x60;		// disable ON/OFF signal at the DIN connector

		DELAY_MS(TEXTDELAY);
		PRINTPC0( "Dropping 5V Enable");
		EN5V = 0;
		DELAY_MS(1000);		// wait a cotton pickin second here
		
		EN5V = 1;
	
		DELAY_MS(TEXTDELAY);
		PRINTPC0( "Disabling AutoOn...");
		DigiPot(FALSE);	// turn off AutoOn feature

		P4 = 0x00;

		DELAY_MS(TEXTDELAY);
		PRINTPC0( "AutoOnTest PASS.");
	} // else

	LED0 = 1; LED1 = 0; LED2 = 0; LED3 = 0;

	calibrating = FALSE;

	SFRPAGE = SFRPAGE_SAVE;
} // AutoOnTest

void SMB_Master_Test(void) {		// only valid for su2

#define TEST_BYTES	8				// number of bytes to send/receive
#define NUM_CYCLES	50				// number of round-trip transfers in test

	u16 i, j;						// counters
	u16 timeoutctr;					// timeout counter
	BOOL xmit_error = FALSE;		// indicates transmission error
	bit savesmbpu = ENSMBPU;		// save state of SMBus Pullups
	
	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	calibrating = TRUE;

	// initialize
	DELAY_MS(TEXTDELAY);
	PRINTPC0( "SMBus Test - Master");

	if (!su2) {
		DELAY_MS(TEXTDELAY);
		PRINTPC0( "Only Valid for SU2 - Exiting.");
	} // if
	else {
		ENSMBPU = 0;			// enable pullups
		SFRPAGE = SMB0_PAGE;
		ENSMB = 1;				// Enable the bus
		DELAY_MS(TEXTDELAY);
		i = TEST_BYTES; j = NUM_CYCLES;
		PRINTPC2( "%u Bytes, %u Cycles", i, j);
#if 0
		// If slave is holding SMBSDA low because of an improper SMBus reset or error
		while(!SMBSDA) {
			// Provide clock pulses to allow the slave to advance out
			// of its current state. This will allow it to release SMBSDA.
			XBR1 = 0x40;							// Enable Crossbar
			SMBSCL = 0;								 // Drive the clock low
			for(i = 0; i < 255; i++);		  // Hold the clock low
			SMBSCL = 1;								 // Release the clock
			while(!SMBSCL);							// Wait for open-drain
														// clock output to rise
			for(i = 0; i < 10; i++);			// Hold the clock high
			XBR1 = 0x00;							// Disable Crossbar
		} // while
#endif
		// set up the LEDs for the test
		PWRLED = RECLED = 1;
		GPSLED = COMLED = 0;

		smbus.txbuff[0] = TEST_BYTES;			// put payload byte count in first byte
		for (i = 0; i < TEST_BYTES; i++)		// fill the buffer with sequential integers starting at 0x40
			smbus.txbuff[i+1] = i + 0x40;
		smbus.txbytes = TEST_BYTES + 1;			// number of bytes to send plus one for the count
		smbus.rxbytes = TEST_BYTES;				// number of bytes to receive
		smbus.target = SMBUS_SLAVE_1;			// address 4

		for (j=0; j < NUM_CYCLES; j++) {
			// indicate no receive data ready
			smbus.data_ready = FALSE;				// reset smbus ready flag		

			SMB_Write();							// initiate SMBus write to slave

			// toggle LED every 10 cycles
			if (j % 10) GPSLED = ~GPSLED;

			// if switch pressed, exit
			if (0 != SW1) {
				// see if it has just been released
				if (debounce >= DEBOUNCE_NORMAL) break;

				debounce = 0;
			} // if

			DELAY_MS(10);							// wait a bit

			SMB_Read();								// and read it back in

			timeoutctr = 0;
			// wait for a valid smbus response with approx 1 msec timeout
			do {} while (!smbus.data_ready && ++timeoutctr < 11500) ;

			// check for timeout
			if (!smbus.data_ready)
				xmit_error = TRUE;
			else { // transmission successful, look for errors				
				// Check transfer data
				for (i = 0; i < TEST_BYTES; i++)
					// Received data match transmit data?
					if(smbus.rxbuff[i] != smbus.txbuff[i+1])
						xmit_error = TRUE;				  // Indicate error
			} // else

			if (xmit_error) break;					// no reason to continue

			DELAY_MS(10);							// wait a bit and do it again
		} // for

		ENSMBPU = savesmbpu;		// restore pullups
		SFRPAGE = SFRPAGE_SAVE;
		ENSMB = 0;					// Disable the bus

		DELAY_MS(TEXTDELAY);
		if (xmit_error)
			PRINTPC0( "** SMBus Test FAIL **");
		else
			PRINTPC0( "SMBus Test Pass.");
	} // else
	
	PWRLED = 1; RECLED = GPSLED = COMLED = 0;
	calibrating = FALSE;

} // SMBus_Master_Test

void SMB_Slave_Test(void) {		// only valid for su2
	u16 i, j;						// counter
	bit savesmbpu = ENSMBPU;		// save state of SMBus Pullups
	
	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	calibrating = TRUE;
	smbus.txbytes = SMBTXSIZE;
	smbus.rxbytes = SMBRXSIZE;
	smbus.data_ready = FALSE;
	
	// initialize
	if (!su2) {
#ifndef SUSLAVETEST
		DELAY_MS(TEXTDELAY);
		PRINTPC0( "Only Valid for SU2 - Exiting.");
#endif
	} // if
	else {
#ifndef SUSLAVETEST
		DELAY_MS(TEXTDELAY);
		PRINTPC0( "SMBus Test - Slave");
#endif	
		ENSMBPU = 0;			// enable pullups
		i = TEST_BYTES; j = NUM_CYCLES;
#if 0
		// If slave is holding SMBSDA low because of an improper SMBus reset or error
		while(!SMBSDA) {
			// Provide clock pulses to allow the slave to advance out
			// of its current state. This will allow it to release SMBSDA.
			XBR1 = 0x40;							// Enable Crossbar
			SMBSCL = 0;								 // Drive the clock low
			for(i = 0; i < 255; i++);		  // Hold the clock low
			SMBSCL = 1;								 // Release the clock
			while(!SMBSCL);							// Wait for open-drain
														// clock output to rise
			for(i = 0; i < 10; i++);			// Hold the clock high
			XBR1 = 0x00;							// Disable Crossbar
		} // while
#endif
		// set up the LEDs for the test
		PWRLED = RECLED = 1;
		GPSLED = COMLED = 0;

		SFRPAGE = SMB0_PAGE;
		SMB0ADR = SMBUS_SLAVE_1;			// Set my slave address (4)
		SI = 0;
					
	   // Initialize the outgoing data array in case a read is done before a write
		for (i = 0; i < TEST_BYTES; i++) {
			smbus.txbuff[i] = 0xF0 + i;
		} // for
	
		while(1) {
			// check for new data received
			while(!smbus.data_ready) ;

			// if switch pressed, exit	
			if (0 != SW1) {
				// see if it has just been released
				if (debounce >= DEBOUNCE_NORMAL) break;
	
				debounce = 0;
			} // if
			
			smbus.data_ready = FALSE;
	
			// Copy the data from the input array to the output array
			for (i = 0; i < smbus.rxbytes; i++) {
				smbus.txbuff[i] = smbus.rxbuff[i];
			} // for
	
			// flash the GPS LED to indicate receive data
			GPSLED = ~GPSLED;
		} // while

		ENSMBPU = savesmbpu;		// restore pullups
		SFRPAGE = SFRPAGE_SAVE;
		ENSMB = 0;					// Disable the bus

	} // else
	
	PWRLED = 1; RECLED = GPSLED = COMLED = 0;

	calibrating = FALSE;

	SFRPAGE = SFRPAGE_SAVE;
} // SMBus_Slave_Test
#endif