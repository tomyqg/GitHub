// Part of displayunit.c
// 8/3/2004
// Author: GAStephens
//
// formats and uploads the contents of the dataflash
// part of traqmate.c

#include <display.h>
#include <math.h>

void dodate(int *, int *, int *, int);

// gps is a structure with all the relevant gps data
// msec is the tenth of second number
// textline is a buffer large enough to hold the expected formatted string
// linenum is 0 if all data is required or 1,2,3 for respective 20 char lines
//
void formatgps (gpstype *gps, int msec, char *textline, u08 linenum) {
	xdata float lat, lon;
	float tempfloat;

	lat = (float) gps->lat * (180.0 / 2147483647.0);
	lon = (float) gps->lon * (180.0 / 2147483647.0);

	if (linenum < 2)
		formattime(gps->time, gps->weeks, LONGDANDT, textline);

	if (linenum == 0) textline += strlen(textline);
	if (linenum == 0 || linenum == 2) {

		// location - MUST BE BROKEN INTO 2 SPRINTFS
		sprintf(textline, "%3.3f,%3.3f", lat, lon);
		textline += strlen(textline);

		// do altitude in proper units
		if (scratch.calibrate.usersettings & 0x01)	// english
			sprintf(textline, ",%d", (s16)(((long) gps->alt * 39L)/12L));
		else	// metric
			sprintf(textline, ",%d", (s16) gps->alt);			
	}
	if (linenum == 0) textline += strlen(textline);
	if (linenum == 0 || linenum == 3) {
		s16 eastvel, northvel, vertvel;
		u16 speed;
		u16 heading;
		xdata char temptype;

		// unpack the velocities
		eastvel = (s16) ((((u16) (gps->velpack[0] & 0xE0)) << 3) | (u16) gps->velpack[1]);
		// sign extend
		if (eastvel & 0x0400) eastvel = (u16) eastvel | 0xF800;

		northvel = (s16) ((((u16) (gps->velpack[0] & 0x1C)) << 6) | (u16) gps->velpack[2]);
		if (northvel & 0x0400) northvel = (u16) northvel | 0xF800;

		vertvel = (s16) ((((u16) (gps->velpack[0] & 0x03)) << 8) | (u16) gps->velpack[3]);
		// sign extend
		if (vertvel & 0x0200) vertvel = (u16) vertvel | 0xFC00;

		// do not write heading if we are not moving
		if (eastvel != 0 || northvel != 0) {
			// compute heading = vector sum of east/west vel and north/south vel (due east is 0)
			heading = compute_heading(eastvel, northvel);

			// convert from trig coordinate to compass heading
			heading = (360 - (heading + 270) % 360) % 360;

			// heading
			sprintf(textline, "H %03d, ", heading);
		}
		else { // no heading
			// heading
			sprintf(textline, "H ---, ", heading);
		} // else

		textline += strlen(textline);

		// calculate speed in meters/sec plus fudge factor
		tempfloat = GPSVELSCALE * sqrt((float) ((double) eastvel * (double) eastvel + (double) northvel * (double) northvel));

		if (scratch.calibrate.usersettings & 0x01) {  // english
			// convert to miles / hour
			speed = (u16) (tempfloat * (39.0/12.0/5280.0*60.0*60.0));

			// temperature
			temptype = 'F';
		} // if
		else { // metric
 			// convert to km / hour
			speed = (u16) (tempfloat * (3600.0 / 1000.0));

			// temperature
			temptype = 'C';			
		} // else
						
		// velocities
		sprintf(textline, "V %02d, ", speed);
	
		textline += strlen(textline);

		if (su_hw >= 200) {		// only do this if we are connected to 3 axis SU2
			tempfloat = (float) (u08) gps->temp - (float) (scratch.calibrate.zzero >> 4);
			if (tempfloat < 0.0) {
				tempfloat = tempfloat / ((float) (scratch.calibrate.zzero >> 4) - (float) (scratch.calibrate.zminusg >> 4));
			} // if
			else  {
				tempfloat = tempfloat / ((float) (scratch.calibrate.zplusg >> 4) - (float) (scratch.calibrate.zzero >> 4));
			} // else
			sprintf(textline, "Z%+1.1f", tempfloat);
		} // if
		else {					// original 2 axis SU1
			tempfloat = dotemp(gps->temp, temptype);
			sprintf(textline, "T %2.0f%c", tempfloat, temptype);
		} // else
		
		textline += strlen(textline);

		// number of satellites
		sprintf(textline, "%2d ", (int) gpslock);
	}
} // formatgps

// gps is a structure with all the relevant gps data
//
void writerpm (gpstype *gps) {
	float tempfloat;
	u16 rpm, period;
	xdata s16 eastvel, northvel, vertvel;
	xdata char tempstring[21];
	xdata char warning;

	// get period
	period = ((u16) allinputs[5]) << 8 | allinputs[6];

	// check for valid data
	if (period == 0)
		rpm = 0;
	else {
		u16 cyl;

		// figure out rpm
		// hz
		tempfloat = (float) FREQ_SAMPLE_RATE / (float) period;

		// rpm
		if (0 == (cyl = currentcar.cylandweight & 0x0F)) // kart
			rpm = (u16) (tempfloat * 60 * 4);
		else {
			if (15 == cyl) cyl = 16;
			rpm = (u16) (tempfloat * (60 * 2 / (cyl)));
		} // else
	} // else

//	Write_Screen(rpmbckgnd, (warning = (rpm > currentcar.revwarning)? '~' : '-'));

	warning = (rpm > currentcar.revwarning)? '~' : '-';
	if (warning != '~')
		Clear_Screen();		
	else
		Full_Screen((u08) 0xFF);

	// put up a line bar
	Write_Icon(0, 0, NUMBER_OF_COLUMNS, 8, &(menubar[0]), warning);

	// write menu centered name
	Write_Text(0, 22, "Tach & Speedo", warning);
	// write undertitles
	Write_Text(3, 22, "RPM", warning);
	if (scratch.calibrate.usersettings & 0x01) // english
		Write_Text(3, 93, "MPH", warning);
	else // metric
		Write_Text(3, 93, "KPH", warning);

	if (0 == iocollect)		// no connect
	 	Write_Big_Text(1, 2, " N/C", warning);
	else {
		// put rpm numbers on display
		sprintf(tempstring, "%5u", rpm);
	 	Write_Big_Text(1, 2, tempstring, warning);
	} // else

#ifdef DEMOBOX
	if (rpm < 900)
		rpm = 0;
	else
		rpm = (rpm-900) / 120;
	sprintf(tempstring, "%3d", rpm);
	// put speed on display
	Write_Big_Text(1, 83, tempstring, warning);
#else
	if (gpslock >= GPS_GOOD) {
		u16 speed;

		// decode velocities
		unpack_velocities (&(gps->velpack[0]), &eastvel, &northvel, &vertvel);
	
		// calculate speed in meters/sec plus fudge factor
//		tempfloat = GPSVELSCALE * sqrt((float) eastvel * (float) eastvel + (float) northvel * (float) northvel);
		tempfloat = GPSVELSCALE * sqrt((float) ((double) eastvel * (double) eastvel + (double) northvel * (double) northvel));
	
		if (scratch.calibrate.usersettings & 0x01) // english
			// convert to miles / hour
			speed = (u16) (tempfloat * (39.0/12.0/5280.0*60.0*60.0));
		else // metric
 			// convert to km / hour
			speed = (u16) (tempfloat * (3600.0 / 1000.0));

		sprintf(tempstring, "%3d", speed);
		// put speed on display
		Write_Big_Text(1, 83, tempstring, warning);
	}
	else
		Write_Big_Text(1, 70, "NoGPS", warning);	
#endif	
} // writerpm

// writes the status of analog and digital inputs on screen
//
void writeinputs (void) {
	xdata char tempstring[21];
	xdata float tempfloat;
	xdata s08 j;
	u16 rpm, period;
	
	for (j=0; j<4; j++) {
		tempfloat = (float) analogscaling[j].maxscale * (float) (allinputs[4-j]) / (float) (VOLTFULLSCALE);
		sprintf(tempstring, "A%d %4.2fV ", (int) j, tempfloat);
		Write_Text(1+j/2, (j & 0x01)? 67: 6, tempstring, '-');
	} // for

	// hardware checkout mode - show all digitals if d0 and d1 = 0
	if (!(allinputs[0] & ~0x03)) {
		tempstring[0] = 'D';
		tempstring[1] = ':';
		tempstring[2] = ' ';
		// check each bit
		for (j = 7; j >= 0; j--)
			tempstring[10-j] = (((allinputs[0] >> j) & 0x01)? '1' : '0');
		tempstring[11] = '\0';
		Write_Text(3, 1, tempstring, '-');
 	} // if
 	else {
		// normal mode - just show D4 and D5
		sprintf(tempstring, "D4: %d D5: %d", (int)((allinputs[0] & 0x10)? 1:0), (int)((allinputs[0] & 0x20)? 1:0));
		Write_Text(3, 1, tempstring, '-');
	} // else

	// get period
	period = ((u16) allinputs[5]) << 8 | allinputs[6];

	// check for valid data
	if (period == 0)
		rpm = 0;
	else {
		u16 cyl;

		// figure out rpm
		// hz
		tempfloat = (float) FREQ_SAMPLE_RATE / (float) period;

		// rpm
		if (0 == (cyl = currentcar.cylandweight & 0x0F)) // kart
			rpm = (u16) (tempfloat * 60 * 4);
		else {
			if (15 == cyl) cyl = 16;
			rpm = (u16) (tempfloat * (60 * 2 / (cyl)));
		} // else
	} // else

	sprintf(tempstring, "R:%5u", rpm);
	Write_Text(3, 75, tempstring, '-');
	
} // writeinputs

// writes the status of a particular analog or the digital inputs on screen
//
void formatgauges (tmstatetype state) {
	xdata char tempstring[24];
	xdata float tempfloat;
	xdata u08 temp;

	if (GAUGED4 == state || GAUGED5 == state) {
		u08 j = state - GAUGED4;
		char *nameptr;

		Clear_Screen();

		// put up a line bar
		Write_Icon(0, 0, NUMBER_OF_COLUMNS, 8, &(menubar[0]), '-');

		digitalscaling[j].pointname[11] = '\0';

		temp = 6*(20-strlen(digitalscaling[j].pointname))/2;
		// write menu centered name
		Write_Text(0, temp, digitalscaling[j].pointname, '-');

		// create message
		if (GAUGED4 == state)
			if (allinputs[0] & 0x10)
				nameptr = digitalscaling[0].onname;
			else
				nameptr = digitalscaling[0].offname; 
		else
			if (allinputs[0] & 0x20)
				nameptr = digitalscaling[1].onname;
			else
				nameptr = digitalscaling[1].offname; 

		// put value on display
		Write_Big_Text(2, ((13-strlen(nameptr))*9)/2, nameptr, '-');

	} // if
	else { // analogs
		u08 j = state - GAUGEA0;
		u08 warning;

		// snap values to rails
		if (allinputs[4-j] < analogscaling[j].lowreading)
			tempfloat = 0.0;
		else if (allinputs[4-j] > analogscaling[j].highreading)
			tempfloat = 1.0;
//		else if (analogscaling[4-j].highreading == analogscaling[4-j].lowreading)	// check for divide by zero
//			tempfloat = 0.0;
		else // scale as percent of usable range
			tempfloat = (float) (allinputs[4-j] - analogscaling[j].lowreading) /
				(float) (analogscaling[j].highreading - analogscaling[j].lowreading);

		// scale as actual units to be measured
		tempfloat = tempfloat * (analogscaling[j].highval - analogscaling[j].lowval) + analogscaling[j].lowval;
	
		// check for alarm condition
		switch (analogscaling[j].alarmdirection  ) {
			case 1:
				warning = (tempfloat > analogscaling[j].alarmval)? '~' : '-';
				break;
			case 0:
				warning = (tempfloat == analogscaling[j].alarmval)? '~' : '-';
				break;
			case -1:
				warning = (tempfloat < analogscaling[j].alarmval)? '~' : '-';
				break;
		} // switch
	
		if (warning != '~')
			Clear_Screen();		
		else
			Full_Screen((u08) 0xFF);

		// put up a line bar
		Write_Icon(0, 0, NUMBER_OF_COLUMNS, 8, &(menubar[0]), warning);

		analogscaling[j].pointname[11] = '\0';	
		temp = 6*(20-strlen(analogscaling[j].pointname))/2;
		// write menu centered name
		Write_Text(0, temp, analogscaling[j].pointname, warning);
	
		// create message
		if (FLOATABS(tempfloat) > 10.0)	// use one decimal place
			sprintf(tempstring, "%4.1f %s ", tempfloat, analogscaling[j].unitname); 
		else // use two decimal places
			sprintf(tempstring, "%4.2f %s ", tempfloat, analogscaling[j].unitname); 
	
	 	// truncate
		tempstring[12] = '\0';
		// put value on display
		Write_Big_Text(2, ((13-strlen(tempstring))*9)/2, tempstring, warning);

	} // else
} // formatgauges

// gps is a structure with all the relevant gps data
// textline is a buffer large enough to hold the expected formatted string
//
void writecompass (gpstype *gps) {
	float tempfloat;
	xdata u16 speed;
	xdata s16 eastvel, northvel, vertvel;
	xdata u16 heading;
	xdata char tempstring[21];

	// decode velocities
	unpack_velocities (&(gps->velpack[0]), &eastvel, &northvel, &vertvel);

	// do not write heading if we are not moving
	if (eastvel != 0 || northvel != 0) {
		// compute heading = vector sum of east/west vel and north/south vel (due east is 0)
		heading = compute_heading(eastvel, northvel);

		// convert from trig coordinate to compass heading
		heading = (360 - (heading + 270) % 360) % 360;

		// figure out our quadrant
		if (heading > 327 || heading < 23 )
			sprintf(tempstring, "N ");
		else if (heading < 68 )
			sprintf(tempstring, "NE");
		else if (heading > 295 )
			sprintf(tempstring, "NW");
		else if (heading < 113 )
			sprintf(tempstring, "E ");
		else if (heading > 247 )
			sprintf(tempstring, "W ");
		else if (heading < 158 )
			sprintf(tempstring, "SE");
		else if (heading > 202 )
			sprintf(tempstring, "SW");
		else
			sprintf(tempstring, "S ");

		// put compass direction on display
		Write_Big_Text(1, 2, tempstring, '-');
	} // if

	// get the time to display
	formattime(gps->time, gps->weeks, SHORTTIME, tempstring);

	// put time on display
	Write_Big_Text(1, 34, tempstring, '-');

	// calculate speed in meters/sec plus fudge factor
	tempfloat = GPSVELSCALE * sqrt((float) ((double) eastvel * (double) eastvel + (double) northvel * (double) northvel));

	if (scratch.calibrate.usersettings & 0x01) // english
		// convert to miles / hour
//		speed = (u16) (tempfloat * (39.0/12.0/5280.0*60.0*60.0));
		speed = (u16) (0.5 + (tempfloat * 2.21591));
	else // metric
		// convert to km / hour
		speed = (u16) (tempfloat * (3600.0 / 1000.0));

	sprintf(tempstring, "%3d", speed);

	// put speed on display
	Write_Big_Text(1, 92, tempstring, '-');

	// temperature
//	temperature = dotemp(gps->temp, 'F');
//	sprintf(textline, "%3.1f", temperature);

} // writecompass

#ifdef GMETER_ENABLED
// writes graph of g forces on screen
//
// these define must match up with the gmeter background screen
// text
#define LEFTCOL		1
#define LEFTROW		11
#define RIGHTCOL	98
#define RIGHTROW	11
#define BRAKECOL	29
#define BRAKEROW	0
#define ACCELCOL	69
#define ACCELROW	22
// graphs
#define LEFTSTART	54
#define LEFTEND		27
#define RIGHTSTART	67
#define RIGHTEND	94
#define BRAKESTART	15
#define BRAKEEND	1
#define ACCELSTART	17
#define ACCELEND	31

void writegmeter (BOOL init, float xg, float yg) {
	static xdata float accel, brake, left, right;
	xdata float scale = 2.0;		// full scale deflection of graph
	xdata int g;
	xdata char tempstring[6];

	if (init) {
		Reset_Top_Line();
		Write_Screen(gmeterbckgnd);
		accel = brake = left = right = 0.0;
	} // if
	else {
		if (xg < 0.0) {
			accel = MIN(accel, xg);

			// scale the g's
			g = (int) (-xg/scale * (float) (ACCELEND - ACCELSTART + 1));

			// draw the lower half
			Draw_Rect(LEFTSTART+2, ACCELSTART, RIGHTSTART-2, ACCELSTART+g, '-');	// dark
			Draw_Rect(LEFTSTART+2, ACCELSTART+g+1, RIGHTSTART-2, ACCELEND, '~');	// light
			// clear the upper half
			Draw_Rect(LEFTSTART+2, BRAKEEND, RIGHTSTART-2, BRAKESTART, '~');	// light

			// write the max accel numbers
			g = (int) (ABS(accel) * 100);
			// write the hundredths
			Write_Icon(ACCELCOL+18, ACCELROW, 6, 10, &(meddigits[g%10][0][0]), '-');
			g /= 10;
			// write the tenths
			Write_Icon(ACCELCOL+10, ACCELROW, 6, 10, &(meddigits[g%10][0][0]), '-');
			g /= 10;
			// write the integer
			Write_Icon(ACCELCOL, ACCELROW, 6, 10, &(meddigits[g][0][0]), '-');
		} // if
		else {
			brake = MAX(brake, xg);

			// scale the g's
			g = (int) (xg/scale * (float) (BRAKESTART - BRAKEEND + 1));

			// draw the upper half
			Draw_Rect(LEFTSTART+2, BRAKESTART-g, RIGHTSTART-2, BRAKESTART, '-');	// dark
			Draw_Rect(LEFTSTART+2, BRAKEEND, RIGHTSTART-2, BRAKESTART-g-1, '~');	// light
			// clear the lower half
			Draw_Rect(LEFTSTART+2, ACCELSTART, RIGHTSTART-2, ACCELEND, '~');	// light

			// write the max brake numbers
			g = (int) (ABS(brake) * 100);
			// write the hundredths
			Write_Icon(BRAKECOL+18, BRAKEROW, 6, 10, &(meddigits[g%10][0][0]), '-');
			g /= 10;
			// write the tenths
			Write_Icon(BRAKECOL+10, BRAKEROW, 6, 10, &(meddigits[g%10][0][0]), '-');
			g /= 10;
			// write the integer
			Write_Icon(BRAKECOL, BRAKEROW, 6, 10, &(meddigits[g][0][0]), '-');

		} // else
		if (yg < 0.0) {
			left = MIN(left, yg);

			// scale the g's
			g = (int) (-yg/scale * (float) (LEFTSTART - LEFTEND + 1));

			// draw the left half
			Draw_Rect(LEFTSTART-g, BRAKESTART-3, LEFTSTART, ACCELSTART+2, '-');	// dark
			Draw_Rect(LEFTEND, BRAKESTART-3, LEFTSTART-g-1, ACCELSTART+2, '~');	// light
			// clear the right half
			Draw_Rect(RIGHTSTART, BRAKESTART-3, RIGHTEND, ACCELSTART+2, '~');	// light

			// write the max left numbers
			// write the integer
			Write_Icon(LEFTCOL, LEFTROW, 6, 10, &(meddigits[(int) left][0][0]), '-');
			// write the fraction
			sprintf(tempstring, "%02f", -left - floor(-left));
			Write_Med_Text((u08) (LEFTCOL+10), (u08) LEFTROW, tempstring, '-');
		} // if
		else {
			right = MAX(right, yg);

			// scale the g's
			g = (int) (yg/scale * (float) (RIGHTEND - RIGHTSTART + 1));

			// draw the right half
			Draw_Rect(RIGHTSTART, BRAKESTART-3, RIGHTSTART+g, ACCELSTART+2, '-');	// dark
			Draw_Rect(RIGHTSTART+g+1, BRAKESTART-3, RIGHTEND, ACCELSTART+2, '~');	// light
			// clear the left half
			Draw_Rect(LEFTEND, BRAKESTART-3, LEFTSTART, ACCELSTART+2, '~');	// light

			// write the max right numbers
			// write the integer
			Write_Icon(RIGHTCOL, RIGHTROW, 6, 10, &(meddigits[(int) right][0][0]), '-');
			// write the fraction
			sprintf(tempstring, "%02f", right - floor(right));
			Write_Med_Text((u08) (RIGHTCOL+10), (u08) RIGHTROW, tempstring, '-');
		} // else
	} // else
} // writegmeter
#endif

// time and weeks are gps format
// textline is a buffer large enough to hold the expected formatted string
//
void formattime (u32 time, u16 weeks, timeforms whattodo, char *textline) {
	xdata int day, mon, dow;
	xdata int year, days;
	xdata int hour, min, sec;
	xdata u32 secs;

#define MAXTIME		(7L*24L*60L*60L*1000L)		// 604,800,000 maximum time value

	if (0 == weeks)		// bad GPS time
		day = mon = year = hour = min = sec = 0;
	else {
		// if timecode is programmed, use it
		if (0xFF != (u08) scratch.calibrate.timecode) {
			s32 newtime;

			// add on offset in milliseconds
			newtime = (s32) time +
					((s32) scratch.calibrate.timecode * (60L * 60L * 1000L));

			// handle week boundaries
			if (newtime < 0) {		// moved into last week
				weeks--;
				newtime += MAXTIME;
			}
			else if (newtime > MAXTIME) { // moved into next week
				weeks++;
				newtime -= MAXTIME;
			}
			time = (u32) newtime;
		} // if
	
		// get seconds since Sunday midnight
		secs = time / 1000;
		dow = (secs / 60 / 60 / 24);
		hour = (secs / 3600) % 24;
		min = (secs / 60) % 60;
		sec = secs % 60;

		// 8030 = number days from GPS start Jan 6, 1980 to Jan 1, 2002
		days = (weeks * 7) + dow - 8030; // days since Jan 1, 2002

		dodate (&day, &mon, &year, days);
	} // else

	switch (whattodo) {
		case SHORTDANDT:
			// date
			sprintf(textline, "%02d/%02d", mon, day);
			// time
			sprintf(textline + strlen(textline), " %02d:%02d:%02d", hour, min, sec);
			break;
		case LONGDANDT:
			// date
			sprintf(textline, "%04d-%02d-%02d", year, mon, day);
			// time
			sprintf(textline + strlen(textline), " %02d:%02d:%02d", hour, min, sec);
			break;
		case SHORTTIME:
			// time
			sprintf(textline, "%02d:%02d", hour, min);
			break;
	} // switch
} // formattime

// returns C or F calculated degrees from recorded value
float dotemp(s08 scaledtemp, char corf) {
	xdata float temperature;

	temperature = (float) scaledtemp;
	if (corf == 'F' || corf == 'f')
		temperature = temperature / 2.0 * 9.0 / 5.0 + 32.0;	// in fahrenheit
	else
		temperature = temperature / 2.0;	// in celcius
	
	return (temperature);
}

// day 1 = Jan 1, 2002
// returns day of month, month (jan = 1), and year

void dodate(int *day, int *mon, int *year, int days) {

	xdata u08 calendar[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	*year = 2002;	
	while (days > 365 ) {
		// fix leap year
		if (!(*year%4) && days == 366) break;
		days -= ((*year % 4)? 365 : 366);
		++*year;
	}

	if (!(*year%4)) calendar[1] = 29; 		// fix leap year

	*mon = 0;
	while (days > calendar[*mon]) {
		days -= calendar[*mon];
		++*mon;
	}
	++*mon;

	*day = days;
}