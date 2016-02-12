
#ifndef	_TRACKSETTINGSSCREENS_H
#define	_TRACKSETTINGSSCREENS_H


#ifdef __cplusplus
extern "C" {
#endif

enum TRACK_TYPES {
	ROAD_COURSE,
	OVAL,
	AUTOXRALLY,
	DRAG_1_8,
	DRAG_1000,
	DRAG,
	HILL_CLIMB,
	DRIVE_COURSE
};

WM_HWIN GetTrackSettingsWindowHandle(void);
void SetTrackSettingsWindowHandle(WM_HWIN);
extern struct SCREEN_LIST TrackSettingsScreenList[];
extern void TrackSettingsScreensCallback(WM_MESSAGE *);
extern void TrackSettingsIgnoreNextKeyRelease(void);

void Display_TrackSettingMainScreen(WM_HWIN hPrev);
WM_HWIN GetTrackSettingMainWindowHandle(void);
void SetTrackSettingMainWindowHandle(WM_HWIN);

extern char alteredTrackData;

#ifdef __cplusplus
}
#endif


#define	NUM_TRACK_SETTINGS_SCREENS	4

#define	TRACKTYPE_SELECTION			0
#define	HOLDTIME_SELECTION			1
#define	STARTFINISH_SELECTION		2
#define	RESETPL_SELECTION			3


#endif
