#define LAPTIMER_MAIN

#ifdef PROFILER
#include "profiler.h"
#define THIS_TASK	LAPTIMERTASK_NUM
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "WM.h"
#include "commondefs.h"
#include "tmtypes.h"
#include "DUII.h"
#include "LapTimerTask.h"
#include "DataHandler.h"
#include "FlashReadWrite.h"

#ifndef _WINDOWS
#include "RTOS.h"
#include "FS.h"
#include "CommTask.h"
#endif

// external functions
void SetUserMode(DU2_STATE_TYPE mode);

// local functions
void ProcessFinishLine( s32 );

static void InitLapTimerVariables(void) {

	DataValues[PREDICTIVE_LAP].fVal = 0.0;
	DataValues[LAP_TIME].iVal = 0;
	DataValues[LAST_LAP_TIME].iVal = 0;
	DataValues[ELAPSED_TIME].iVal = 0;
	DataValues[ELAPSED_RACE_TIME].iVal = 0;
	DataValues[LAP_NUMBER].iVal = 0;
	DataValues[LAP_DISTANCE].fVal = 0.0;
	DataValues[TOTAL_DISTANCE].fVal = 0.0;
	DataValues[MIN_DOP].fVal = 10.0;
	DataValues[MIN_SATS].uVal = 4;
	DataValues[MAX_DOP].fVal = 0.0;
	DataValues[MAX_SATS].uVal = 0;
	DataValues[MAX_SPEED].fVal = 0.0;
	DataValues[VMIN].fVal = 0.0;
	DataValues[VMAX].fVal = 0.0;
	DataValues[SPEED_PEAKS].fVal = 0.0;
	DataValues[MAX_X_G].fVal = 0.0;
	DataValues[MAX_Y_G].fVal = 0.0;
	DataValues[MAX_Z_G].fVal = 0.0;
	DataValues[MIN_X_G].fVal = 0.0;
	DataValues[MIN_Y_G].fVal = 0.0;
	DataValues[MIN_Z_G].fVal = 0.0;
	DataValues[TOTAL_HIGHREVS].uVal = 0;
	DataValues[TOTAL_OVERREVS].uVal = 0;
	DataValues[MAX_RPM].uVal = 0;
	DataValues[MIN_ALTITUDE].fVal = 0.0;
	DataValues[MAX_ALTITUDE].fVal = 0.0;
	DataValues[SIXTY_FOOT].uVal = 0;
	DataValues[SIXTY_TRAP].fVal = 0;
	DataValues[ZERO_SIXTY].uVal = 0;
	DataValues[ZERO_HUNDRED].uVal = 0;
	DataValues[THREE_THIRTY_FOOT].uVal = 0;
	DataValues[THREE_THIRTY_TRAP].fVal = 0.0;
	DataValues[EIGHTH_MILE].uVal = 0;
	DataValues[EIGHTH_TRAP].fVal = 0.0;
	DataValues[THOUSAND_FOOT].uVal = 0;
	DataValues[THOUSAND_TRAP].fVal = 0.0;
	DataValues[QUARTER_MILE].uVal = 0;
	DataValues[QUARTER_TRAP].fVal = 0.0;
} // InitLapTimerVariables

// 4/8/2005 - changed lap_dist from 30 to 35 & lap_head from 20 to 30 based on VIR test
// 10/12/2005 - changed lap_dist from 35 to 40, lap_alt from 80 to 100 based on customer feedback
// 3/28/2006 - changed lap_dist from 40 to 45
//
// Lap Timer Parameters
// good for skidpad #define LAP_DISTRANGE	10		// 45 in du1 matching error for start/finish distance for lat and lon in meters
#define LAP_DISTRANGE	35		// 45 in du1 matching error for start/finish distance for lat and lon in meters
#define LAP_ALTRANGE	100		// matching error for start/finish altitude in meters -- not used
#define LAP_HEADRANGE	30		// 40 in du1, matching error for start/finish heading in degrees

#define AUTOXSETHDGTIME	2000	// 2 second = number of ms to delay from launch before recording heading
#define AUTOX_DISTRANGE	10		// matching error for finish line distance for lat and lon in meters
#define AUTOX_HEADRANGE	30		// matching error for finish heading in degrees

#define TIMER_RESOLUTION	50	// 50 msec = 1/20th of second

typedef struct {
	double lat;					// in decimal degrees
	double lon;					// in decimal degrees
	float heading;				// in degrees
} gpspostype;

typedef struct {
	s32 besttime;				// best lap time in this session
	gpspostype currentpos;		// where we are now
	gpspostype previouspos;		// position of last sample
	s32 timernminus1;			// lap time at last gps sample in msec
} laptype;

// globals
	char LapInitialize = true;				// semaphore between CommTask and LapTimerTask indicating when to initialize lap data

// local globals
	static laptype lap;						// timing of lap
	static sectortype sector[NUMBER_OF_SECTORS];	// intermediate values for predictive lap timing
	static int refsectornum;				// which sector are we comparing to
	static int cursectornum;				// which sector are we in
	static double seeklat;					// what position are we watching for. could be start or finish
	static double seeklon;
	static float seekhdg;
	static double speed;					// in meters/sec

void LapTimerTask( void ) {
	static u32 lastTimerValue;				// last reading of system clock tick
	static char inrange = FALSE;			// TRUE when we are near Start or Finish
	static float lastdist;					// previous samples distance measurement for predictive lap timing
	static char writeTracks = FALSE;

	u32 currentTimerValue;					// current reading of system clock tick
	u32 elapsedTime;						// time since last task event in msec
	double deltad;							// in meters
	int predictiveDistance;					// distance between breadcrumbs
		
	while (1) {
		unsigned char which_event;		 	// bitmask of events that occured
		
		// wait for something to happen. wake up in 1/20 second to check where we are
		which_event = OS_WaitSingleEventTimed( (unsigned char) (1 << DU2_NEW_GPS) , TIMER_RESOLUTION );
	
		if (which_event)		// if got gps, add on the du2_start or du2_finish if it occurred
			which_event = which_event | OS_ClearEvents(NULL);

		// if we are reprogramming just stop
		if (DU2_REPROGRAMMING == unsavedSysData.systemMode || DU2_REPROGRAMMING_SU == unsavedSysData.systemMode) {	// don't do anything if we are reflashing unit
			OS_Delay(600000);				// go to sleep for a long time. unit will reset and wake us up
		} // if

		// before doing anything else, check to see if we are fresh into a recording mode and initialize
		if (LapInitialize) {
			LapInitialize = false;
			
			InitLapTimerVariables();
			lastTimerValue = 0;
			lastdist = 0.0;
			lap.timernminus1 = 0;
			elapsedTime = 0;
			lastTimerValue = OS_GetTime32();

			// must do in case the user changed tracks
			seeklat = trackData.startLineLat;
			seeklon = trackData.startLineLong;
			seekhdg = trackData.startLineHeading;
			
			// initialize sector times if they don't exist or if we are in Lap/Race and in qualifying mode
			if ((0 == trackData.bestLapEver) || ((DU2_LAPS_WAITSTART == unsavedSysData.systemMode) && (false == sysData.lappingMode))) {
				// new sf so initialize sectors
				for (cursectornum = 0; cursectornum < NUMBER_OF_SECTORS; cursectornum++)
					sector[cursectornum].reference =
					sector[cursectornum].current =
					trackData.sectorTimes[cursectornum] = 0;
				trackData.numberOfSectors = 0;
				lap.besttime = 0;
			} // if
			else {		// best lap recorded so load the reference sector data and clear the current data
				for (cursectornum = 0; cursectornum < trackData.numberOfSectors; cursectornum++)
					sector[cursectornum].reference = trackData.sectorTimes[cursectornum];
				lap.besttime = trackData.bestLapEver;
				for (cursectornum = 0; cursectornum < NUMBER_OF_SECTORS; cursectornum++)
					sector[cursectornum].current = 0;
			} // else
			cursectornum = 0;
			refsectornum = 0;
			lap.previouspos.lat = lap.currentpos.lat = GetValue(LATITUDE).dVal;
			lap.previouspos.lon = lap.currentpos.lon = GetValue(LONGITUDE).dVal;
			lap.previouspos.heading = lap.currentpos.heading = GetValue(HEADING).fVal;
				
		} // if LapInitialize

		if ((DU2_IDLE == unsavedSysData.systemMode) && (writeTracks == TRUE)) {		// not timing any more so save the track SF
			writeTracks = FALSE;
			if (unsavedSysData.sdCardInserted) {		// only write file if we have an SD card
				WriteDataStructures(TRACK_DATA);
				FS_Sync("");			// Flush cache to be sure all files written to disk
			} // if
		} // if
		else if (DU2_IDLE != unsavedSysData.systemMode &&
			DU2_AUTOX_WAITSTAGE != unsavedSysData.systemMode &&
			DU2_DRAG_WAITSTAGE  != unsavedSysData.systemMode &&
			DU2_CAMERA_TEST != unsavedSysData.systemMode) {			// don't do anything if not in a timing mode or waiting to stage
			// is this a launchable mode?
			if (DU2_AUTOX_WAITLAUNCH == unsavedSysData.systemMode || DU2_DRAG_WAITLAUNCH  == unsavedSysData.systemMode) {	
				// check for launch -- note that launch g is negative
				if (DataValues[X_G].fVal < (((float) sysData.launchGForce) / -10.0)) {
					// save the start location. we'll grab the heading later
					unsavedSysData.workingStartLineLat = trackData.startLineLat = GetValue(LATITUDE).dVal;
					unsavedSysData.workingStartLineLong = trackData.startLineLong = GetValue(LONGITUDE).dVal;
					unsavedSysData.workingStartLineHeading = 0.0;	// will be filled in when we get moving
	
					// clear any distance that was racked up wandering up to stage
					DataValues[LAP_DISTANCE].fVal = 0.0;
					DataValues[TOTAL_DISTANCE].fVal = 0.0;
					lastdist = 0.0;
					
					// prime the timer
					elapsedTime = 0;
					lap.timernminus1 = 0;
					lastTimerValue = OS_GetTime32();
					DataValues[LAP_NUMBER].iVal = 1;
					predictiveDistance = SHORT_PREDICTIVE_DISTANCE;
					
					if (DU2_DRAG_WAITLAUNCH == unsavedSysData.systemMode)
						SetUserMode(DU2_DRAG);
					else {		// autox
						if (0.0 == trackData.finishLineLat)
							SetUserMode(DU2_AUTOX_WAITFINISH);
						else { // got a finish line so find it
							seeklat = trackData.finishLineLat;
							seeklon = trackData.finishLineLong;
							seekhdg = trackData.finishLineHeading;
							SetUserMode(DU2_AUTOX);
						} // else
					} // else
				} // if
			} // if
			else {		// not waiting to launch so we are in a real timing mode
				if (DU2_LAPS_WAITSTART == unsavedSysData.systemMode || DU2_HILLCLIMB_WAITSTART == unsavedSysData.systemMode) {
					// we are driving so count toward operating time
					sysData.totalTimeEver += ((currentTimerValue = OS_GetTime32()) - lastTimerValue) / 1000.0;		// all time counter in seconds
					lastTimerValue = currentTimerValue;
				} // if
				// increment the time
				else if (DU2_LAPS == unsavedSysData.systemMode ||
					DU2_DRIVE == unsavedSysData.systemMode ||
					DU2_INSTRUMENT == unsavedSysData.systemMode ||
					DU2_AUTOX == unsavedSysData.systemMode ||
					DU2_AUTOX_WAITFINISH == unsavedSysData.systemMode ||
					DU2_HILLCLIMB == unsavedSysData.systemMode ||
					DU2_HILLCLIMB_WAITFINISH == unsavedSysData.systemMode ||
					DU2_DRAG == unsavedSysData.systemMode) {

					predictiveDistance = LAP_PREDICTIVE_DISTANCE;
					
					// timed out so up the times
					elapsedTime = (currentTimerValue = OS_GetTime32()) - lastTimerValue;
					lastTimerValue = currentTimerValue;
		
					switch (unsavedSysData.systemMode) {
						case DU2_DRIVE:
							writeTracks = TRUE;							// save the start line at end of run
							// fall through
						case DU2_INSTRUMENT:
							sysData.driveTripTime += elapsedTime;		// drive time counter
					} // switch
		
					sysData.totalTimeEver += elapsedTime / 1000.0;		// all time counter in seconds
		
					// request use of global data
					OS_Use(&SEMA_GlobalData);
		
					DataValues[LAP_TIME].iVal += elapsedTime;
					DataValues[ELAPSED_TIME].iVal += elapsedTime;
					DataValues[ELAPSED_RACE_TIME].iVal += elapsedTime;
		
					// relinquish global data
					OS_Unuse(&SEMA_GlobalData);		
				} // if

				// check to see if we have a new gps position
				if (which_event & (1 << DU2_NEW_GPS)) {			// comm received a new GPS position		
					// new gps update so get speed in kph and convert to meters / sec
					speed = GetRawValue(SPEED).fVal * KPH_TO_MET_PER_SEC;
				
				  // update the positions
					lap.previouspos = lap.currentpos;
					lap.currentpos.lat = GetValue(LATITUDE).dVal;
					lap.currentpos.lon = GetValue(LONGITUDE).dVal;
					lap.currentpos.heading = GetValue(HEADING).fVal;
					
					// don't get an incorrect distance if we just acquired GPS
					if (0.0 == lap.previouspos.lat)
						lap.previouspos = lap.currentpos;
					
					// process start and finish lines
					if (DU2_LAPS_WAITNOSF == unsavedSysData.systemMode || DU2_HILLCLIMB_WAITNOSF == unsavedSysData.systemMode ||
						DU2_AUTOX_WAITFINISH == unsavedSysData.systemMode || DU2_HILLCLIMB_WAITFINISH == unsavedSysData.systemMode) {
						if (which_event & (1 << DU2_START_SET)) {	// user set the start or start/finish and we have new gps
		
							// we are in range
							inrange = TRUE;
		
							// prime the timer
							elapsedTime = 0;
							lastTimerValue = OS_GetTime32();
							DataValues[LAP_NUMBER].iVal = 1;
							
							lap.besttime = trackData.bestLapEver = 0;
							DataValues[LAP_DISTANCE].fVal = 0.0;
							lastdist = 0.0;
						
							// new sf so initialize sectors
							for (cursectornum = 0; cursectornum < NUMBER_OF_SECTORS; cursectornum++)
								sector[cursectornum].reference =
								sector[cursectornum].current =
								trackData.sectorTimes[cursectornum] = 0;
							cursectornum = 0;
							refsectornum = 0;
							trackData.numberOfSectors = 0;
				
							// save starting position for this track and make it available for tqm file writes
							seeklat = unsavedSysData.workingStartLineLat = trackData.startLineLat = lap.currentpos.lat;
							seeklon = unsavedSysData.workingStartLineLong = trackData.startLineLong = lap.currentpos.lon;
							seekhdg = unsavedSysData.workingStartLineHeading = trackData.startLineHeading = lap.currentpos.heading;
							trackData.startLineAlt = unsavedSysData.workingStartLineAlt = GetRawValue(ALTITUDE).fVal;
							writeTracks = TRUE;
							
							// got start so go to the next mode
							switch (unsavedSysData.systemMode) {
								case DU2_LAPS_WAITNOSF:
									SetUserMode(DU2_LAPS);
									break;
								case DU2_HILLCLIMB_WAITNOSF:
									SetUserMode(DU2_HILLCLIMB_WAITFINISH);
									break;
							} // switch
						} // if
						else if (which_event & (1 << DU2_FINISH_SET)) {	// user set the finish line position
							
							// user set the finish line so remember it
							unsavedSysData.workingFinishLineLat = trackData.finishLineLat = GetValue(LATITUDE).dVal;
							unsavedSysData.workingFinishLineLong = trackData.finishLineLong = GetValue(LONGITUDE).dVal;	
							unsavedSysData.workingFinishLineHeading = trackData.finishLineHeading = GetValue(HEADING).fVal;
							writeTracks = TRUE;
							
							// for hillclimb, start looking for the start again
							if (DU2_HILLCLIMB_WAITFINISH == unsavedSysData.systemMode) {
								
								// restart
								lastTimerValue = OS_GetTime32();
											
								seeklat = trackData.startLineLat;
								seeklon = trackData.startLineLong;
								seekhdg = trackData.startLineHeading;
			
								ProcessFinishLine( 0 );
								SetUserMode(DU2_HILLCLIMB);
							} // if
							else {
								ProcessFinishLine( 0 );
								SetUserMode(DU2_AUTOX_COMPLETE);
							} // else
						} // else if finish
					} // if no start finish mode
					
					// process the position information. skip if we are not moving to eliminate divide by zero
					if (speed > 0.01) {
						double fdeltalat, fdeltalon;
						float tempfloat;					
						int headrange;
						int distrange;

						if (DU2_AUTOX == unsavedSysData.systemMode) {
							headrange = AUTOX_HEADRANGE;
							distrange = AUTOX_DISTRANGE;
						} // if
						else {
							headrange = LAP_HEADRANGE;
							distrange = LAP_DISTRANGE;
						} // else
						
						// calculate lap distance
						// find difference in meters of lat, lon
						fdeltalat = (lap.currentpos.lat - lap.previouspos.lat) * LATMET;
						fdeltalon = (lap.currentpos.lon - lap.previouspos.lon) * LONMET(lap.currentpos.lat);
		
						// remember the distance at the last sample point
						lastdist = DataValues[LAP_DISTANCE].fVal;
	
						// add on distance traveled in this sample
						deltad = sqrt(fdeltalat * fdeltalat + fdeltalon * fdeltalon);
	
						if (DU2_DRIVE == unsavedSysData.systemMode || DU2_INSTRUMENT == unsavedSysData.systemMode) {
							sysData.driveTripDistance += (float) deltad;
							DataValues[TOTAL_DISTANCE].fVal += (float) deltad;
						} // if
						else if (DU2_AUTOX_COMPLETE != unsavedSysData.systemMode && DU2_DRAG_COMPLETE != unsavedSysData.systemMode) {
							DataValues[LAP_DISTANCE].fVal += (float) deltad;
							DataValues[TOTAL_DISTANCE].fVal += (float) deltad;
						} // else if
	
						sysData.totalDistanceEver += (float) deltad;
	
						// now that we are moving, save the heading shortly after launch
						// limit to small window just in case we are headed due East
						if ((DU2_AUTOX == unsavedSysData.systemMode || DU2_AUTOX_WAITFINISH == unsavedSysData.systemMode || DU2_DRAG == unsavedSysData.systemMode)) {
							if ((0.0 == unsavedSysData.workingStartLineHeading) && (DataValues[LAP_TIME].iVal >= AUTOXSETHDGTIME) && (DataValues[LAP_TIME].iVal <= (1000 + AUTOXSETHDGTIME))) {
								// save starting heading for this track
								unsavedSysData.workingStartLineHeading = trackData.startLineHeading = GetValue(HEADING).fVal;
								writeTracks = TRUE;
							} // if
						} // if
	
						// predictive lap timing
						if (DataValues[LAP_TIME].iVal > 0 && lastdist > 0.0) {	// only process after passing S/F and collecting one more sample
							int lapdist = ((int) DataValues[LAP_DISTANCE].fVal) % predictiveDistance;  // integer lap distance
		
							// get just the remainder in this sector
							lastdist = (float) (((int) lastdist) % predictiveDistance);
	
							// process sectors if we are in a race mode
							if (DU2_LAPS == unsavedSysData.systemMode ||
								DU2_AUTOX == unsavedSysData.systemMode ||
//								DU2_AUTOX_WAITFINISH == unsavedSysData.systemMode ||
								DU2_HILLCLIMB == unsavedSysData.systemMode ||
//								DU2_HILLCLIMB_WAITFINISH == unsavedSysData.systemMode ||
								DU2_DRAG == unsavedSysData.systemMode) {
								
								if (lapdist < lastdist) {	// crossed sector boundary
									float sectoradd;		// bit to add to last timing measurement to get to sector bounday
									// interpolate and save the time
									sectoradd = ((float) predictiveDistance - lastdist) /
											(lapdist + (float) predictiveDistance - lastdist); // ratio between points
									sectoradd = sectoradd * (float) (DataValues[LAP_TIME].iVal - lap.timernminus1); // time in ms to add to previous
																	
									// if no best time then reference sectors all zero. otherwise check them
									if (0 != lap.besttime) {
										// are we out of comparison sectors?
										if (0 == sector[refsectornum].reference && refsectornum > 0)
											// if so, back up a sector
											refsectornum--;
									} // if
		
									// store interpolated sector time as current for comparison
									sector[cursectornum].current = lap.timernminus1 + (s32) sectoradd;
			
									// only update graph until we run out of sectors
									if (cursectornum < (NUMBER_OF_SECTORS-1)) {
										// if sector data loaded then compare progress and display graph						
										if (0 != lap.besttime) {
											// determine how much ahead / behind. + is ahead, - is behind
											DataValues[PREDICTIVE_LAP].fVal = (float) (sector[cursectornum].current - sector[refsectornum].reference) / 1000.0;
										} // if
										cursectornum++;
									} // if
									if (refsectornum < (NUMBER_OF_SECTORS-1))
										refsectornum++;
								} // if
							} // if
						} // if process predictive

						// save the current timer for future interpolation
						lap.timernminus1 = DataValues[LAP_TIME].iVal;
	
						tempfloat = lap.currentpos.heading - seekhdg;
						tempfloat = FLOATABS(tempfloat);
	
						// if we are waiting for user action, don't bother going any further
						if (DU2_HILLCLIMB_WAITFINISH != unsavedSysData.systemMode && DU2_AUTOX_WAITFINISH != unsavedSysData.systemMode) {
	
							if (DU2_DRAG == unsavedSysData.systemMode) {
								int dragcomplete = false;
								float extraDistance;			// in meters
								float extraTime;				// in met/sec
								float extraSpeed;				// in seconds
								
								// look for distances and record them
								// all values interpolated using the speed, distance, and acceleration when the boundary was crossed
								
								if (0 == DataValues[SIXTY_FOOT].uVal && DataValues[LAP_DISTANCE].fVal > SIXTY_FEET_IN_METERS) {
									extraDistance = (DataValues[LAP_DISTANCE].fVal - SIXTY_FEET_IN_METERS);
									extraTime = extraDistance / speed;
									extraSpeed = extraTime * -DataValues[X_G].fVal * ONE_G_IN_MET_PER_SEC2;
									DataValues[SIXTY_FOOT].iVal = DataValues[LAP_TIME].iVal - (int) (extraTime * 1000.0) + DRAG_LAUNCH_TIME_CORRECTION;
									DataValues[SIXTY_TRAP].fVal = DataValues[SPEED].fVal - (extraSpeed * MET_PER_SEC_TO_KPH);		// record speed in kph
								} // if
			
								if (0 == DataValues[ZERO_SIXTY].uVal && DataValues[SPEED].fVal > SIXTY_MPH_IN_KPH) {
									extraSpeed = DataValues[SPEED].fVal - SIXTY_MPH_IN_KPH;
									extraTime = extraSpeed / (-DataValues[X_G].fVal * ONE_G_IN_MET_PER_SEC2);
									DataValues[ZERO_SIXTY].iVal = DataValues[LAP_TIME].iVal - (int) (extraTime * 1000.0) + DRAG_LAUNCH_TIME_CORRECTION;
								} // if
									
								if (0 == DataValues[ZERO_HUNDRED].uVal && DataValues[SPEED].fVal > HUNDRED_MPH_IN_KPH) {
									extraSpeed = DataValues[SPEED].fVal - HUNDRED_MPH_IN_KPH;
									extraTime = extraSpeed / (-DataValues[X_G].fVal * ONE_G_IN_MET_PER_SEC2);
									DataValues[ZERO_HUNDRED].iVal = DataValues[LAP_TIME].iVal - (int) (extraTime * 1000.0) + DRAG_LAUNCH_TIME_CORRECTION;
								} // if
			
								if (0 == DataValues[THREE_THIRTY_FOOT].uVal && DataValues[LAP_DISTANCE].fVal > THREETHIRTY_FEET_IN_METERS) {
									extraDistance = (DataValues[LAP_DISTANCE].fVal - THREETHIRTY_FEET_IN_METERS);
									extraTime = extraDistance / speed;
									extraSpeed = extraTime * -DataValues[X_G].fVal * ONE_G_IN_MET_PER_SEC2;
									DataValues[THREE_THIRTY_FOOT].iVal = DataValues[LAP_TIME].iVal - (int) (extraTime * 1000.0) + DRAG_LAUNCH_TIME_CORRECTION;
									DataValues[THREE_THIRTY_TRAP].fVal = DataValues[SPEED].fVal - (extraSpeed * MET_PER_SEC_TO_KPH);		// record speed in kph
								} // if
			
								if (0 == DataValues[EIGHTH_MILE].uVal && DataValues[LAP_DISTANCE].fVal > (EIGHTH_MILE_IN_METERS + DRAG_LAUNCH_DIST_CORRECTION)) {
									extraDistance = (DataValues[LAP_DISTANCE].fVal - EIGHTH_MILE_IN_METERS);
									extraTime = extraDistance / speed;
									extraSpeed = extraTime * -DataValues[X_G].fVal * ONE_G_IN_MET_PER_SEC2;
									DataValues[EIGHTH_MILE].iVal = DataValues[LAP_TIME].iVal - (int) (extraTime * 1000.0) + DRAG_LAUNCH_TIME_CORRECTION;
									DataValues[EIGHTH_TRAP].fVal = DataValues[SPEED].fVal - (extraSpeed * MET_PER_SEC_TO_KPH);		// record speed in kph
			
									// 1/8 mile run so stop here
									if (DD_1_8 == sysData.dragDistance) {
										dragcomplete = true;
										DataValues[LAP_DISTANCE].fVal = EIGHTH_MILE_IN_METERS;		// fix up lap length
										DataValues[LAP_TIME].uVal = DataValues[EIGHTH_MILE].uVal;	// fix up the time
									} // if
								} // if
	
								if (0 == DataValues[THOUSAND_FOOT].uVal && DataValues[LAP_DISTANCE].fVal > (THOUSAND_FEET_IN_METERS + DRAG_LAUNCH_DIST_CORRECTION)) {
									extraDistance = (DataValues[LAP_DISTANCE].fVal - THOUSAND_FEET_IN_METERS);
									extraTime = extraDistance / speed;
									extraSpeed = extraTime * -DataValues[X_G].fVal * ONE_G_IN_MET_PER_SEC2;
									DataValues[THOUSAND_FOOT].iVal = DataValues[LAP_TIME].iVal - (int) (extraTime * 1000.0) + DRAG_LAUNCH_TIME_CORRECTION;
									DataValues[THOUSAND_TRAP].fVal = DataValues[SPEED].fVal - (extraSpeed * MET_PER_SEC_TO_KPH);		// record speed in kph
			
									// 1000' run so stop here
									if (DD_1000 == sysData.dragDistance) {
										dragcomplete = true;
										DataValues[LAP_DISTANCE].fVal = THOUSAND_FEET_IN_METERS;		// fix up lap length
										DataValues[LAP_TIME].uVal = DataValues[THOUSAND_FOOT].uVal;		// fix up the time
									} // if
								} // if
								
								if (0 == DataValues[QUARTER_MILE].uVal && DataValues[LAP_DISTANCE].fVal > (QUARTER_MILE_IN_METERS + DRAG_LAUNCH_DIST_CORRECTION)) {
									extraDistance = (DataValues[LAP_DISTANCE].fVal - QUARTER_MILE_IN_METERS);
									extraTime = extraDistance / speed;
									extraSpeed = extraTime * -DataValues[X_G].fVal * ONE_G_IN_MET_PER_SEC2;
									DataValues[QUARTER_MILE].iVal = DataValues[LAP_TIME].iVal - (int) (extraTime * 1000.0) + DRAG_LAUNCH_TIME_CORRECTION;
									DataValues[QUARTER_TRAP].fVal = DataValues[SPEED].fVal - (extraSpeed * MET_PER_SEC_TO_KPH);		// record speed in kph
									
									// 1/4 mile run so stop here
									if (DD_1_4 == sysData.dragDistance) {
										dragcomplete = true;
										DataValues[LAP_DISTANCE].fVal = QUARTER_MILE_IN_METERS;		// fix up lap length
										DataValues[LAP_TIME].uVal = DataValues[QUARTER_MILE].uVal;	// fix up the time
									} // if
								} // if
	
								if (dragcomplete) {
									// save the finish information
									unsavedSysData.workingFinishLineLat = trackData.finishLineLat = lap.currentpos.lat;
									unsavedSysData.workingFinishLineLong = trackData.finishLineLong = lap.currentpos.lon;
									unsavedSysData.workingFinishLineHeading = trackData.finishLineHeading = lap.currentpos.heading;
									writeTracks = TRUE;
									
									ProcessFinishLine( 0 );
									SetUserMode(DU2_DRAG_COMPLETE);
								} // if
	
							} // if dragfinish

							// if current heading matches the start/finish heading then check for position
							// while allowing for headings near 0 (due east)
							else if (tempfloat < headrange || tempfloat > (360.0 - headrange)) {
		
								// find difference in meters of lat, lon
								fdeltalat = (lap.currentpos.lat - seeklat) * LATMET;
								fdeltalon = (lap.currentpos.lon - seeklon) * LONMET(seeklat);
			
								if (FLOATABS(fdeltalat) < distrange && FLOATABS(fdeltalon) < distrange) {
									if (DU2_AUTOX != unsavedSysData.systemMode || DataValues[LAP_DISTANCE].fVal > (AUTOX_DISTRANGE*8.0)) {
										// found either the Start or the Finish
										if (!inrange) {
											float distance;
											s32 offset;
											BOOL dofirststart = FALSE;
											BOOL dostart = FALSE;
											BOOL dofinish = FALSE;
				
											// now we are in range
											inrange = TRUE;
				
											// find the actual distance to start/finish in meters
											distance = sqrt(fdeltalat * fdeltalat + fdeltalon * fdeltalon);
				
											// fix up the timer and lap distance based on our speed, rounding up the hundredth
											offset = (s32) (50.0 + (1000.0 * distance) / speed);
				
											// process the incoming states							
											switch (unsavedSysData.systemMode) {
												case DU2_LAPS:
													dostart = TRUE;
													dofinish = TRUE;
													break;
												case DU2_LAPS_WAITSTART:
													dofirststart = TRUE;
													dofinish = FALSE;
													break;
												case DU2_HILLCLIMB_WAITNOSF:
													dofirststart = TRUE;
													break;
												case DU2_HILLCLIMB_WAITSTART:
													dostart = TRUE;
													break;
												case DU2_HILLCLIMB_FINISH:
													dofinish = TRUE;
													break;
												case DU2_AUTOX:
													dofinish = TRUE;
													offset += AUTOX_LAUNCH_TIME_CORRECTION;
													break;
											} // switch
								
											// general finish line stuff
											if (dofinish) {
												ProcessFinishLine( offset );
												if (DU2_AUTOX == unsavedSysData.systemMode)
													SetUserMode(DU2_AUTOX_COMPLETE);		// stop timing at finish line
											} // if
			
											// general start line stuff
											if (dostart || dofirststart) {
												// initialize to where start will be
												DataValues[LAP_TIME].iVal = -offset;
												lastdist = DataValues[LAP_DISTANCE].fVal = -distance;
												lap.timernminus1 = 0;

												// clear the current sector data
												for (cursectornum = 0; cursectornum < NUMBER_OF_SECTORS; cursectornum++)
													sector[cursectornum].current = 0;
												cursectornum = 0;
												refsectornum = 0;
			
												elapsedTime = 0;
												lastTimerValue = OS_GetTime32();
												
												// next lap please
												(DataValues[LAP_NUMBER].iVal)++;
											} // if
											
											switch (unsavedSysData.systemMode) {
												case DU2_LAPS_WAITSTART:
													SetUserMode(DU2_LAPS);
													break;
											} // switch
										} // if !inrange
									} // if !autox or distance over 50
								} // if
								else // no longer in range
									inrange = FALSE;
							} // else if check for heading in range
						} // if not hillclimb waitfinish
					} // if speed > 0
				} // if new gps position
			} // else not autox, drag startup modes
		} // if
	} // while
} // LapTimerTask

void ProcessFinishLine( s32 offset ) {
	int i;

	// fudge to where time would be when we cross finish line, rounding up
	DataValues[LAST_LAP_TIME].iVal = (DataValues[LAP_TIME].iVal += offset);

	// store the lap plus a fudge based on our speed
	if (DataValues[LAP_NUMBER].iVal <= NUMLAPS) {
		tqmheader.tqmheaderstruct.lapInfo.lapTime[DataValues[LAP_NUMBER].iVal-1] = DataValues[LAST_LAP_TIME].uVal;
		tqmheader.tqmheaderstruct.lapInfo.numberOfLaps = DataValues[LAP_NUMBER].uVal;
	} // if

	// check for best lap ever
	if ((0 == lap.besttime) || (DataValues[LAST_LAP_TIME].uVal < lap.besttime)) {
		int copybestlap = ((0 == trackData.bestLapEver) || (DataValues[LAST_LAP_TIME].uVal < trackData.bestLapEver));
		
		DataValues[BEST_LAP].uVal = BEST_LAP_EVER;
		lap.besttime = DataValues[LAST_LAP_TIME].uVal;
		if (copybestlap) {
			trackData.bestLapEver = lap.besttime;
			trackData.numberOfSectors = cursectornum;
		} // if

		// make current lap the reference lap
		for (i = 0; i < cursectornum; i++) {
			sector[i].reference = sector[i].current;
			if (copybestlap)
				trackData.sectorTimes[i] = sector[i].current;
		} // for

		tqmheader.tqmheaderstruct.lapInfo.bestLapTime = DataValues[LAST_LAP_TIME].uVal;
		tqmheader.tqmheaderstruct.lapInfo.bestLapNumber = DataValues[LAP_NUMBER].uVal;
	} // if								
	// check for best lap of this session
	else if ((0 == tqmheader.tqmheaderstruct.lapInfo.bestLapTime) || (DataValues[LAST_LAP_TIME].uVal < tqmheader.tqmheaderstruct.lapInfo.bestLapTime)) {
		DataValues[BEST_SESSION_LAP].uVal = DataValues[LAST_LAP_TIME].uVal;	
		DataValues[BEST_LAP].uVal = BEST_LAP_OF_SESSION;
		tqmheader.tqmheaderstruct.lapInfo.bestLapTime = DataValues[LAST_LAP_TIME].uVal;
		tqmheader.tqmheaderstruct.lapInfo.bestLapNumber = DataValues[LAP_NUMBER].uVal;
	} // if
	else
		DataValues[BEST_LAP].uVal = NOT_BEST_LAP;

} // ProcessFinishLine