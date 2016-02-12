
#ifndef	_RUNNINGGAUGES_H
#define	_RUNNINGGAUGES_H

// Complete list of gauge screen IDs
#define	GMETER				0
#define	ANALOG_GAUGE		1
// inactive but leave as placeholders so user's screen don't shift when they upgrade
//#define	DUAL_GAUGES			2
//#define	DUAL_GAUGES2		3
#define	MULTI_ANALOG_GAUGES		4
#define	DRIVE_MODE_SCREEN	5
#define	RPM_SWEEP_GAUGE		6
#define	DIGITAL_GAUGE		7
#define	GPS_INFO_SCREEN		8
#define	TEST_INFO_SCREEN	9
#define	CHRONOGRAPH_SCREEN	11
#ifdef PROFILER
#define	PROFILER_SCREEN		10

#define	NUM_GAUGE_SCREENS		7
#define	NUM_DRIVE_SCREENS		8
#define	NUM_INSTRUMENTS_SCREENS	8

#else

#define	NUM_GAUGE_SCREENS		6
#define	NUM_DRIVE_SCREENS		7
#define	NUM_INSTRUMENTS_SCREENS	7

#endif

#define DEFAULT_DRIVE_MODE_SCREEN_INDEX				2
#define DEFAULT_INSTRUMENTS_MODE_SCREEN_INDEX		1
#define DEFAULT_RACE_MODE_SCREEN_INDEX				1


// Old screens - beyond the reach of the list (saved because we know they'll be added back eventually)
#define	ANALOG_GAUGE2		5

#define	BOX_PERSIST_TIME	5000

#define	TOP		1
#define	BOTTOM	2


#ifdef __cplusplus
extern "C" {
#endif

typedef struct GAUGE_SCREEN_LIST {
	int	screenID;
	WM_HWIN (*constructor)(WM_HWIN);
} GAUGESCREENLIST;

void RunningGauges(int screen, WM_HWIN hPrev);
WM_HWIN GetRunningGaugesWindowHandle(void);
void SetRunningGaugesWindowHandle(WM_HWIN);
extern struct GAUGE_SCREEN_LIST GaugesScreenList[];
extern struct GAUGE_SCREEN_LIST DriveScreenList[];
extern struct GAUGE_SCREEN_LIST InstrumentsScreenList[];
void rgDeleteScreen(int index);
void GaugesScreensCallback(WM_MESSAGE * pMsg);

#ifdef __cplusplus
}
#endif

#endif
