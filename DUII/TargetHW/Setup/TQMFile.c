#define TQMFILE_MAIN

#ifdef PROFILER
#include "profiler.h"
#define THIS_TASK	TQMWRITETASK_NUM
#endif

#include <stdio.h>
#include <string.h>
#include "WM.h"
#include "commondefs.h"
#include "DUII.h"
#include "tmtypes.h"
#include "DataHandler.h"
#include "TQMFile.h"
#include "CommTask.h"
#include "FlashReadWrite.h"
#include "dataflash.h"
#include "dbgu.h"
#include "datadecode.h"

extern const char *GetCameraName(void);		// camera text name function from VehicleSettingsScreens.c


#define TQM	tqmheader.tqmheaderstruct

// creates tqm file header
// if start is true, it creates the header with all the information needed by traqview
// if start is false, it writes the ending date and time
//
void CreateTqmFileHeader( char *sessionname, BOOL start) {
	int i;
	char *pChar;
	int heading;

	if (start) {
		strncpy(TQM.format.fileid, "TQM008", 6);
		TQM.format.offset = TQM_FILE_DATA_OFFSET;
		if (start) {
			TQM.info.StartDateTime = T_DATE_TO_GPS_DATE + (GetValue(GPS_WEEKS).uVal * SECONDS_IN_A_WEEK) + (GetValue(GPS_TIME).uVal / 1000);
			TQM.info.EndDateTime = TQM.info.StartDateTime;				// will get changed later
		} // if
	
		strncpy(TQM.info.DriverName, sysData.driver, NAMELEN);
		strncpy(TQM.info.CarName, sysData.vehicle, NAMELEN);
		strncpy(TQM.info.TrackStart.TrackName, sysData.track, NAMELEN);
		
		TQM.info.CalInfo = scratchpad;				// 128 bytes
		
		// swap the bytes and fill in some critical data
		strncpy(TQM.info.CalInfo.calibrate.owner, unsavedSysData.unitInfo.owner, OWNER_LEN);
		strncpy(TQM.info.CalInfo.calibrate.phone, unsavedSysData.unitInfo.phone, PHONE_LEN);
		strncpy(TQM.info.CalInfo.calibrate.wifiName, unsavedSysData.unitInfo.wifiName, WIFI_LEN);
		strncpy(TQM.info.CalInfo.calibrate.wifiPass, unsavedSysData.unitInfo.wifiPass, WIFI_LEN);
		TQM.info.CalInfo.calibrate.timecode = sysData.timeZone;
		TQM.info.CalInfo.calibrate.model = 3;
		TQM.info.CalInfo.calibrate.hwrev = unsavedSysData.duHwRev;
		
		TQM.info.CalInfo.calibrate.serno = SWAP32(unsavedSysData.unitInfo.serno);
		TQM.info.CalInfo.calibrate.week = (u08) unsavedSysData.unitInfo.week;
		TQM.info.CalInfo.calibrate.year = (u08) unsavedSysData.unitInfo.year;
		
		// flip the bytes in the accelerometer calibration info
		TQM.info.CalInfo.calibrate.xzero = SWAP16(TQM.info.CalInfo.calibrate.xzero);
		TQM.info.CalInfo.calibrate.yzero = SWAP16(TQM.info.CalInfo.calibrate.yzero);
		TQM.info.CalInfo.calibrate.xminusg = SWAP16(TQM.info.CalInfo.calibrate.xminusg);
		TQM.info.CalInfo.calibrate.xplusg = SWAP16(TQM.info.CalInfo.calibrate.xplusg);
		TQM.info.CalInfo.calibrate.yminusg = SWAP16(TQM.info.CalInfo.calibrate.yminusg);
		TQM.info.CalInfo.calibrate.yplusg = SWAP16(TQM.info.CalInfo.calibrate.yplusg);
		TQM.info.CalInfo.calibrate.calx = SWAP16(TQM.info.CalInfo.calibrate.calx);
		TQM.info.CalInfo.calibrate.caly = SWAP16(TQM.info.CalInfo.calibrate.caly);
		TQM.info.CalInfo.calibrate.zzero = SWAP16(TQM.info.CalInfo.calibrate.zzero);
		TQM.info.CalInfo.calibrate.zminusg = SWAP16(TQM.info.CalInfo.calibrate.zminusg);
		TQM.info.CalInfo.calibrate.zplusg = SWAP16(TQM.info.CalInfo.calibrate.zplusg);
	
		TQM.info.CalInfo.calibrate.swrev = unsavedSysData.duSwRev;
		TQM.info.CalInfo.calibrate.datarev = unsavedSysData.duDataRev;
	
		TQM.info.ModeSamp = getModeSamp();
	
		if (16 == vehicleData.engineCylinders)
			i = 15;		// can't use 16 since there are only 4 bits
		else
			i = vehicleData.engineCylinders;

		TQM.info.CarSetup.cylandweight = (vehicleData.weight / 2) << 4 | i;
		TQM.info.CarSetup.revwarning = vehicleData.tach.upperYellowStart;
		TQM.info.CarSetup.revlimit =  vehicleData.tach.upperRedStart;
		
		TQM.info.Gears.diffratio = (u16) (vehicleData.diffRatio * 100.0);
		TQM.info.Gears.wheelcircumference = (u16) (vehicleData.tireCircumference * 100.0);
		for (i=0; i < NUMGEARS; i++) {
			TQM.info.Gears.inchesperenginerev[i] = (u16) (TQM.info.Gears.wheelcircumference / vehicleData.diffRatio / vehicleData.gearRatios[i]);
		} // for
		TQM.info.IOData = getIoData();
	
		for (i=0; i < NUMANALOGS; i++) {
			strncpy((char *) TQM.info.Analogs[i].pointname, vehicleData.analogInput[i].Name, IONAMELEN);
			strncpy((char *) TQM.info.Analogs[i].unitname, vehicleData.analogInput[i].Units, IONAMELEN);
		
			TQM.info.Analogs[i].maxscale = vehicleData.analogInput[i].inputRange;
			TQM.info.Analogs[i].lowval = vehicleData.analogInput[i].lowValuePoint;
			TQM.info.Analogs[i].highval = vehicleData.analogInput[i].highValuePoint;
			TQM.info.Analogs[i].lowreading = (u08) ((float) VOLTFULLSCALE * vehicleData.analogInput[i].lowVoltagePoint / vehicleData.analogInput[i].inputRange);
			TQM.info.Analogs[i].highreading = (u08) ((float) VOLTFULLSCALE * vehicleData.analogInput[i].highVoltagePoint / vehicleData.analogInput[i].inputRange);

			// Traqview only supports one alarm zone so decide which way to go with alarm, upper range has priority
			switch (vehicleData.analogInput[i].upperAlarmTriggerComparison) {
				case ABOVE:
					TQM.info.Analogs[i].alarmdirection = 1;
					TQM.info.Analogs[i].alarmval = vehicleData.analogInput[i].upperAlarmTriggerLevel;
					break;
				case BELOW:
				case OFF:
				case EQUAL_TO:
					// above value not valid so use lower value if valid
					switch (vehicleData.analogInput[i].lowerAlarmTriggerComparison) {
						case BELOW:
							TQM.info.Analogs[i].alarmdirection = -1;
							TQM.info.Analogs[i].alarmval = vehicleData.analogInput[i].lowerAlarmTriggerLevel;
							break;
						case ABOVE:
						case OFF:
						case EQUAL_TO:
							TQM.info.Analogs[i].alarmdirection = 0;
							TQM.info.Analogs[i].alarmval = vehicleData.analogInput[i].upperAlarmTriggerLevel;
							break;
					} // switch		
					break;
			} // switch			
		} // for
	
		for (i=0; i < NUMDIGITALS; i++) {		// 7 digitals in tqm file, only 2 in unit, digital 4 and 5
			if (i == 4 || i == 5) {
				strncpy((char *) TQM.info.Digitals[i].pointname, vehicleData.digitalInput[i-4].Name, IONAMELEN);
				strncpy((char *) TQM.info.Digitals[i].onname, vehicleData.digitalInput[i-4].HighName, IONAMELEN);
				strncpy((char *) TQM.info.Digitals[i].offname, vehicleData.digitalInput[i-4].LowName, IONAMELEN);
			}
			else {
				strcpy((char *) TQM.info.Digitals[i].pointname, "");
				strcpy((char *) TQM.info.Digitals[i].onname, "");
				strcpy((char *) TQM.info.Digitals[i].offname, "");
			} // else
		} // for
	
		for (i=0; i < NUMFREQS; i++) {
			TQM.info.Frequency[i].rpmwarning = vehicleData.tach.upperYellowStart;
			TQM.info.Frequency[i].rpmlimit = vehicleData.tach.upperRedStart;
			TQM.info.Frequency[i].divider = vehicleData.engineCylinders / 2;
		} // for
		
		strncpy (TQM.SessionDesc, sessionname + strlen(MY_SESSIONS_PATH) + 1, 10);	// copy date string, skipping over foldername
		pChar = TQM.SessionDesc + 10;
		*pChar++ = '\0';

		formattime (SHORT12, pChar);									// write out time of day
		pChar += strlen(pChar) + 1;
		
		strncpy(pChar, sysData.track, NAMELEN);							// copy track name
		pChar += strlen(pChar) + 1;

		strncpy(pChar, sysData.driver, NAMELEN);
		pChar += strlen(pChar) + 1;
				
		strncpy(pChar, sysData.vehicle, NAMELEN);
		pChar += strlen(pChar) + 1;

		pChar = TQM.SessionDesc + 10;
		*pChar++ = '\0';

		// blank fill lap info
		TQM.lapInfo.numberOfLaps = TQM.lapInfo.bestLapNumber = TQM.lapInfo.bestLapTime = 0;

		// put in lap time ascii header
		strcpy (TQM.UserFieldDescLaptimes, "LAPTIMES");
		
		// write out the current configuration
		pChar = TQM.UserFieldDescConfig;

		// analog configuration
		for (i=0; i<NUMBER_OF_ANALOGS; i++) {
			pChar += sprintf(pChar, "A%d: %s: %s\n", i, vehicleData.analogInput[i].Name, vehicleData.analogInput[i].enabled? "enabled" : "disabled");
		} // for

		// digital configuration
		for (i=0; i<NUMBER_OF_DIGITALS; i++) {
			pChar += sprintf(pChar, "\nD%d: %s: %s", i+4, vehicleData.digitalInput[i].Name, vehicleData.digitalInput[i].enabled? "enabled" : "disabled");
		} // for

		// camera configuration
		pChar += sprintf(pChar, "\n\nSU Cam: %s\nUSB Cam: ", GetCameraName());
		if (0x80 & vehicleData.usbCameraDelay)
			sprintf(pChar, "Enabled, %1.1fs", (float) ((unsigned int) (0x7F & vehicleData.usbCameraDelay)) / 10.0 );
		else
			sprintf(pChar, "None");

		// blank fill lap area
		for (i=0; i < NUMLAPS; i++) {
			TQM.lapInfo.lapTime[i] = 0;				// no laps
		} // if
	
		TQM.info.nUserDefFields = TQM_USER_FIELDS;				// number of descriptive fields in tqm file header
	
		} // of
	else { // closing file
		int charswritten;
		int hrs, min, sec;
		
		charswritten = DataValues[ELAPSED_TIME].uVal / 1000;		// seconds
		hrs = charswritten / 3600;
		min = (charswritten / 60) - (hrs * 60);
		sec = charswritten % 60;
		
		TQM.info.EndDateTime = T_DATE_TO_GPS_DATE + (GetValue(GPS_WEEKS).uVal * SECONDS_IN_A_WEEK) + (GetValue(GPS_TIME).uVal / 1000);

		// general information about session
		sprintf(TQM.UserFieldDescGeneral, "Duration: %02d:%02d:%02d\nDistance: %0.2f %s\nSatellites %u - %u\nDOP %1.2f - %1.2f\nElevation %d - %d %s",
			hrs, min, sec, (sysData.units == STD ? (GetValue(TOTAL_DISTANCE).fVal / 5280.0) : (GetValue(TOTAL_DISTANCE).fVal / 1000.0)),
			(sysData.units == STD ? "miles" : "km"), DataValues[MIN_SATS].uVal, DataValues[MAX_SATS].uVal, DataValues[MIN_DOP].fVal, DataValues[MAX_DOP].fVal,
			(int) GetValue(MIN_ALTITUDE).fVal, (int) GetValue(MAX_ALTITUDE).fVal, (sysData.units == STD ? "ft" : "m"));

		// input information
		pChar = TQM.UserFieldDescInputs;
		pChar += sprintf(pChar, "High Revs: %u\nOver Revs: %u\nMax RPM: %u\n\n",
			DataValues[TOTAL_HIGHREVS].uVal, DataValues[TOTAL_OVERREVS].uVal, DataValues[MAX_RPM].uVal);

		// print out the analogs
		for (i=0; i<NUMBER_OF_ANALOGS; i++) {
			if (vehicleData.analogInput[i].enabled)
				pChar += sprintf(pChar, "A%d: %s\n%1.2f - %1.2f %s\n\n",
					i, vehicleData.analogInput[i].Name,
					GetValue((GlobalDataIndexType) ((int) MIN_A_0 + i)).fVal,
					GetValue((GlobalDataIndexType) ((int) MAX_A_0 + i)).fVal,
					vehicleData.analogInput[i].Units);
		} // for

		charswritten = 0;

		// performance information from session
		if (0 == DataValues[SIXTY_FOOT].uVal) { // skip these if drag race mode
			charswritten += sprintf(TQM.UserFieldDescPerformance + charswritten, "Max Speed: %0.1f%s\n", GetValue(MAX_SPEED).fVal, (sysData.units == STD ? "mph" : "kph"));
			if (0 == DataValues[ELAPSED_TIME].uVal)
				charswritten += sprintf(TQM.UserFieldDescPerformance + charswritten, "Avg Speed: 0.0\n");
			else
				charswritten += sprintf(TQM.UserFieldDescPerformance + charswritten, "Avg Speed: %0.1f%s\n",
					(sysData.units == STD ? (GetValue(TOTAL_DISTANCE).fVal / 5280.0) : (GetValue(TOTAL_DISTANCE).fVal / 1000.0)) /
					(((float) DataValues[ELAPSED_TIME].uVal) / 1000.0 / 3600.0), (sysData.units == STD ? "mph" : "kph"));
			charswritten += sprintf(TQM.UserFieldDescPerformance + charswritten, "Max Lateral:\nL%1.2f G, R%1.2f G\n", -DataValues[MIN_Y_G].fVal, DataValues[MAX_Y_G].fVal);
			charswritten += sprintf(TQM.UserFieldDescPerformance + charswritten, "Max Brake: %1.2f G\n", DataValues[MAX_X_G].fVal);
		} // if
		
		charswritten += sprintf(TQM.UserFieldDescPerformance + charswritten, "Max Accel: %1.2f G\n", -DataValues[MIN_X_G].fVal);

		if (0 != DataValues[ZERO_SIXTY].uVal)
			if (0 != DataValues[ZERO_HUNDRED].uVal) {
				charswritten += sprintf((TQM.UserFieldDescPerformance + charswritten), "0-60,100mph: %0.1fs, %0.1fs\n",
					(float) DataValues[ZERO_SIXTY].uVal / 1000.0, (float) DataValues[ZERO_HUNDRED].uVal / 1000.0);
			} // if
			else {		// didn't reach 100mph
				charswritten += sprintf((TQM.UserFieldDescPerformance + charswritten), "0-60mph: %0.1fs\n", (float) DataValues[ZERO_SIXTY].uVal / 1000.0);	
			} // else
		
		if (0 != DataValues[SIXTY_FOOT].uVal)
			charswritten += sprintf((TQM.UserFieldDescPerformance + charswritten), "60': %0.1fs, %0.1f%s\n",
				(float) DataValues[SIXTY_FOOT].uVal / 1000.0, GetValue(SIXTY_TRAP).fVal, (sysData.units == STD ? "mph" : "kph"));
		if (0 != DataValues[THREE_THIRTY_FOOT].uVal)
			charswritten += sprintf((TQM.UserFieldDescPerformance + charswritten), "330': %0.1fs, %0.1f%s\n",
				(float) DataValues[THREE_THIRTY_FOOT].uVal / 1000.0, GetValue(THREE_THIRTY_TRAP).fVal, (sysData.units == STD ? "mph" : "kph"));
		if (0 != DataValues[EIGHTH_MILE].uVal)
			charswritten += sprintf((TQM.UserFieldDescPerformance + charswritten), "1/8mi: %0.1fs, %0.1f%s\n",
				(float) DataValues[EIGHTH_MILE].uVal / 1000.0, GetValue(EIGHTH_TRAP).fVal, (sysData.units == STD ? "mph" : "kph"));
		if (0 != DataValues[THOUSAND_FOOT].uVal)
			charswritten += sprintf((TQM.UserFieldDescPerformance + charswritten), "1000': %0.1fs, %0.1f%s\n",
				(float) DataValues[THOUSAND_FOOT].uVal / 1000.0, GetValue(THOUSAND_TRAP).fVal, (sysData.units == STD ? "mph" : "kph"));
		if (0 != DataValues[QUARTER_MILE].uVal)
			charswritten += sprintf((TQM.UserFieldDescPerformance + charswritten), "1/4mi: %0.1fs, %0.1f%s\n",
				(float) DataValues[QUARTER_MILE].uVal / 1000.0, GetValue(QUARTER_TRAP).fVal, (sysData.units == STD ? "mph" : "kph"));

		// put in horsepower and torque when available
	} // else

	// do this at start and end in case it changed
	TQM.info.TrackStart.startlat = SWAP32((s32) (unsavedSysData.workingStartLineLat / 180.0 * GPSINTEGERTOFLOAT));
	TQM.info.TrackStart.startlon = SWAP32((s32) (unsavedSysData.workingStartLineLong / 180.0 * GPSINTEGERTOFLOAT));
	TQM.info.TrackStart.startalt = SWAP16((s16) unsavedSysData.workingStartLineAlt);
	// convert from compass (0=N) to radian heading (90=N)
	heading = 360 - (((int) unsavedSysData.workingStartLineHeading + 270) % 360);
	TQM.info.TrackStart.starthead = SWAP16((u16) heading);

	TQM.info.TrackFinish.finishlat = SWAP32((s32) (unsavedSysData.workingFinishLineLat / 180.0 * GPSINTEGERTOFLOAT));
	TQM.info.TrackFinish.finishlon = SWAP32((s32) (unsavedSysData.workingFinishLineLong / 180.0 * GPSINTEGERTOFLOAT));
	TQM.info.TrackFinish.finishalt = SWAP16((s16) unsavedSysData.workingFinishLineAlt);
	// convert from compass (0=N) to radian heading (90=N)
	heading = 360 - (((int) unsavedSysData.workingFinishLineHeading + 270) % 360);
	TQM.info.TrackFinish.finishhead = SWAP16((u16) heading);
	
#undef TQM
	
} // WriteTqmFileHeader


// this task allows file writes in low priority background task
//
void TQMWriteTask ( void ) {
	void *pData;						// pointer to data to write to tqm file
	int size;
	char which_event;										 // bitmask of events that occured
			
	while (1) {
		// wait for something to happen
		which_event = OS_WaitEvent( (unsigned char) ALL_TQM_EVENTS );
		
		if (which_event & (1 << DU2_POWER_LOSS)) {	// close files quickly
			if (NULL != sessionFile) {
				FS_FClose(sessionFile);
				FS_Sync("");							// flush cache, update headers
				sessionFile = NULL;
			} // if
			OS_Yield();								// job is done here
		} // if

		if (which_event & (1 << DU2_SDCARD_EJECT)) {	// stop saving and discard queue
			if (NULL != sessionFile) {
				FS_FClose(sessionFile);				// ????
				sessionFile = NULL;
			} // if

			OS_Q_Clear( &Queue_tqm );				// throw away unwritten data (shouldn't be much)
			OS_Yield();								// job is done here
		} // if
		
		if (which_event & (1 << DU2_TQM_OPEN)) {			// open file and create tqm header
			size = OS_Q_GetPtr( &Queue_tqm, &pData );		// wait until filename arrives
				
			sessionFile = FS_FOpen(pData, "wb");
			FS_ConfigFileBufferFlags(sessionFile, FS_FILE_BUFFER_WRITE);
					
#ifdef BUILD_DEBUG_PORT
			if (NULL == sessionFile) {
				DBGU_Print("Could not open session file.\n\r");
			} // if
			DBGU_Print("session: ");
			DBGU_Print(pData);					
			DBGU_Print(" \n\r");
#endif
			// chop off .tqm for session name
			*((char *) pData + size - 4) = '\0';

			// create and write file header
			CreateTqmFileHeader( (char *) pData, true );			// send filename string for inclusion in tqm file
			
			OS_Q_Purge ( &Queue_tqm );

			// write the tqm file header
			if (TQM_FILE_DATA_OFFSET != FS_Write( sessionFile, (u08 *) &tqmheader, TQM_FILE_DATA_OFFSET)) {
#ifdef BUILD_DEBUG_PORT
				DBGU_Print("Error writing session file data.\n\r");
#endif
			} // if	
		} // if
		
		if (which_event & (1 << DU2_TQM_WRITE)) {	// need to write data so get it from the queue
			if (0 != OS_Q_GetMessageCnt( &Queue_tqm ))	// don't get blocked
				do {	// process all the messages
					size = OS_Q_GetPtr ( &Queue_tqm, &pData );
					if (NULL != sessionFile) {	
						if (size != FS_Write( sessionFile, (u08 *) pData, size)) {
#ifdef BUILD_DEBUG_PORT
						DBGU_Print("Error writing session file data.\n\r");
#endif
						} // if
					} // if
					OS_Q_Purge ( &Queue_tqm );
				} while (0 != OS_Q_GetMessageCnt( &Queue_tqm ));
		} // if

		if (which_event & (1 << DU2_TQM_CLOSE)) {	// need to write end time and close data file

			if (NULL != sessionFile) {
#ifdef BUILD_DEBUG_PORT
				DBGU_Print("closing session file \n\r");
#endif
				CreateTqmFileHeader( NULL, false );			// skip over date string

				// write out tqm header created elsewhere
				FS_FSeek( sessionFile, 0, FS_SEEK_SET);
				FS_Write( sessionFile, &tqmheader, TQM_FILE_DATA_OFFSET);
				FS_FClose( sessionFile );
				FS_Sync("");					// flush cache, update headers
				OS_Q_Clear( &Queue_tqm );		// throw away unwritten data (shouldn't be much)
				sessionFile = NULL;
			} // if
		} // if
			
	} // while
} // TQMWrite

