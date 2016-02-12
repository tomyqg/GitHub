// Part of displayunit.c
// 8/3/2004
// Author: GAStephens
//
// formats and uploads the contents of the dataflash
// part of traqmate.c

#include <math.h>

#include "DUII.h"
#include "commondefs.h"
#include "WM.h"
#include "DataHandler.h"
#include "tmtypes.h"
#include "datadecode.h"
#include "CommTask.h"

extern OS_RSEMA SEMA_GlobalData;			// resource semaphore around global data

void unpack_velocities( u08 *velptr, s16 *eastvel, s16 *northvel, s16 *vertvel) {
	
	*eastvel = (s16) ((((u16) (velptr[0] & 0xE0)) << 3) + (u16) velptr[1]);
	// sign extend
	if (*eastvel & 0x0400) *eastvel |= 0xF800;

	*northvel = (s16) ((((u16) (velptr[0] & 0x1C)) << 6) + (u16) velptr[2]);
	// sign extend
	if (*northvel & 0x0400) *northvel |= 0xF800;

	*vertvel = (s16) ((((u16) (velptr[0] & 0x03)) << 8) + (u16) velptr[3]);
	// sign extend
	if (*vertvel & 0x0200) *vertvel |= 0xFC00;
} // unpack_velocities

float compute_heading (s16 eastvel, s16 northvel) {
	float tempfloat;
	s16 tempint;

	// compute heading = vector sum of east/west vel and inverse of north/south vel (due east is 0)
	tempfloat = atan2((float) northvel, (float) eastvel);

	tempfloat = (tempfloat * (180.0 / PI));
	tempint = (s16) tempfloat;
	
	// save the fraction
	tempfloat = tempfloat - (float) tempint;
	
	// convert from trig coordinate to compass heading and make sure it is 0-360
	tempint = 360 - ((tempint + 270) % 360);

	// add the fraction to the integer
	tempfloat += (float) tempint;
								
	return (tempfloat);

} // compute_heading

// returns C or F calculated degrees from recorded value
float dotemp(s08 scaledtemp, char corf) {
	float temperature;

	temperature = (float) scaledtemp;
	if (corf == 'F' || corf == 'f')
		temperature = temperature / 2.0 * 9.0 / 5.0 + 32.0;	// in fahrenheit
	else
		temperature = temperature / 2.0;	// in celsius
	
	return (temperature);
}

// gets GPS time from global memory and breaks it out into individual numbers in FS_FILETIME structure
// returns 0 if GPS time is not valid
//
#define MAXTIME		(7*24*60*60*1000)		// 604,800,000 maximum time value

unsigned char breakOutTime ( FS_FILETIME *FileTime ) {
	U32 time;
	U16 weeks;
	U32 secs;

	// request use of global data
	OS_Use(&SEMA_GlobalData);

	// update global data
	time = (u32) GetValue(GPS_TIME).uVal;
	weeks = (u16) GetValue(GPS_WEEKS).uVal;

	// relinquish global data
	OS_Unuse(&SEMA_GlobalData);	

	if (0 == weeks)		// bad GPS time
		return 0;
	else {
		s32 newtime;
		int days, dow;

		// add on offset in milliseconds
		newtime = (s32) time + ((s32) sysData.timeZone * (60 * 60 * 1000));

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

		// get seconds since Sunday midnight
		secs = time / 1000;
		dow = (secs / 60 / 60 / 24);
		FileTime->Hour = (secs / 3600) % 24;
		FileTime->Minute = (secs / 60) % 60;
		FileTime->Second = secs % 60;

		// 8030 = number days from GPS start Jan 6, 1980 to Jan 1, 2002
		days = (weeks * 7) + dow - 8030; // days since Jan 1, 2002

		dodate (&(FileTime->Day), &((*FileTime).Month), &((*FileTime).Year), days);
	} // else

	return 1;
} // breakOutTime
	
// time and weeks are gps format
// textline is a buffer large enough to hold the expected formatted string
//
char *formattime (timeforms whattodo, char *textline) {
	FS_FILETIME FileTime;

	if (0 == breakOutTime(&FileTime)) {
		strcpy (textline, "--:--");
	} // if
	else {
		switch (whattodo) {
			case SHORTDANDT:
				// date
				sprintf(textline, "%02d/%02d", FileTime.Month, FileTime.Day);
				// time
				sprintf(textline + strlen(textline), " %02d:%02d:%02d", FileTime.Hour, FileTime.Minute, FileTime.Second);
				break;
			case LONGDANDT:
			case SHORTDATE:
				// date
				sprintf(textline, "%04d-%02d-%02d", FileTime.Year, FileTime.Month, FileTime.Day);
				if (SHORTDATE == whattodo) break;		// leave early
				// time
				sprintf(textline + strlen(textline), " %02d:%02d:%02d", FileTime.Hour, FileTime.Minute, FileTime.Second);
				break;
			case SHORTTIME:
				// time
				sprintf(textline, "%02d:%02d", FileTime.Hour, FileTime.Minute);
				break;
			case SHORT12: {
				int pm = (FileTime.Hour >= 12);
				// time
				if (FileTime.Hour == 0)
					FileTime.Hour = 12;
				if (FileTime.Hour > 12)
					FileTime.Hour = FileTime.Hour % 12;
				sprintf(textline, "%2d:%02d%s", FileTime.Hour, FileTime.Minute, (pm? "pm": "am"));
				break;
			} // case
		} // switch
	} // else
	
	return textline;
} // formattime

// returns a file timestamp that can be used to set file time
// returns 0 if GPS time has not yet been set
//
U32 getTimeStamp ( void ) {
	FS_FILETIME FileTime;
	U32 TimeStamp;

	if (0 == breakOutTime(&FileTime))
		return 0;
	FS_FileTimeToTimeStamp (&FileTime, &TimeStamp);
	return TimeStamp;

} // getTimeStamp

// day 1 = Jan 1, 2002
// returns day of month, month (jan = 1), and year

void dodate(U16 *day, U16 *mon, U16 *year, U16 days) {

	u08 calendar[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

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

// converts a timestamp into seconds since midnight jan 1, 1980
// NOTE: This routine is approximate for comparing dates. It is not exact to the day.
//
U32 FileTimeToSeconds(FS_FILETIME *pFileTime) {
	U32 seconds = 0;

	seconds += pFileTime->Second;
	seconds += pFileTime->Minute * 60;
	seconds += pFileTime->Hour * 60 * 60;
	seconds += pFileTime->Day * 24 * 60 * 60;
	seconds += pFileTime->Month * 30 * 24 * 60 * 60;		// not accurate (months vary)
	seconds += pFileTime->Year * 365 * 24 * 60 * 60;		// not accurate (leap years)
	
	return (seconds);
	
} // TimeToSeconds

