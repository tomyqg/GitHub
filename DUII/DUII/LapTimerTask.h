
#ifndef	LAPTIMERTASK_H
#define	LAPTIMERTASK_H

void LapTimerTask( void );

#define SIXTY_FEET_IN_METERS			18.29			// 60 feet = 18.28800 meters
#define SIXTY_MPH_IN_KPH				96.56			// 60 mph = 96.56064 kph
#define HUNDRED_MPH_IN_KPH				160.93			// 100 mph = 160.9344 kph
#define THREETHIRTY_FEET_IN_METERS		100.58			// 330 feet = 100.58400 meters
#define EIGHTH_MILE_IN_METERS			201.17			// .125 miles = 201.16800 meters
#define THOUSAND_FEET_IN_METERS			304.8			// 1000 feet = 304.8 meters
#define QUARTER_MILE_IN_METERS			402.34			// .25 miles = 402.33600 meters
#define ONE_G_IN_MET_PER_SEC2			9.80665			// 32 ft/sec/sec = 9.80665 met/sec/sec
#define MET_PER_SEC_TO_KPH				(3600.0 / 1000.0)
#define KPH_TO_MET_PER_SEC				(1000.0 / 3600.0)

#define DRAG_LAUNCH_TIME_CORRECTION		0				// in ms, add this to all drag times to correct for launch error
#define DRAG_LAUNCH_DIST_CORRECTION		6				// extra distance in meters to record for each drag distance

// settings for Top Gear USA
// note they are launching behind the actual start line so not correct
// #define AUTOX_LAUNCH_TIME_CORRECTION	-600			// in ms, add this to autocross times to correct for launch error

#define AUTOX_LAUNCH_TIME_CORRECTION	-300			// in ms, add this to autocross times to correct for launch error
#endif

