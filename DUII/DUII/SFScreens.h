
#ifndef	_SFSCREENS_H
#define	_SFSCREENS_H

#ifdef __cplusplus
extern "C" {
#endif

enum WFSTYPE {
	GO_RACING_TYPE,
	DISPLAY_TRACKLIST_TYPE
};

enum SELECTED_GAUGE {
	SG_SWEEPTACH,
	SG_ANALOG,
	SG_DIGITAL
};


extern void SFScreensCallback(WM_MESSAGE *);
void SFScreensIgnoreNextKeyRelease(void);
void DeleteSFScreensKeyboards(void);
void DisplayTapAtSFScreen(WM_HWIN hPrev);
void DisplayWaitingForSatellites(enum WFSTYPE type, enum GUI_RACE_MODES mode, WM_HWIN hPrev);
int HaveSatelliteReception(void);
void KillWaitingForSatelliteScreen(char foundSatellites);
void DisplayWaitingForCameraScreen(WM_HWIN hPrev);
char ValidateSFCoordinates(void);
WM_HWIN GetSFWindowHandle(void);
void DisplayAutoXStageScreen(WM_HWIN hPrev);
void DisplayDragStageScreen(WM_HWIN hPrev);
void DisplayLaunchScreen(WM_HWIN hPrev);
WM_HWIN GetStageWindowHandle(void);
WM_HWIN GetLaunchScreenWindowHandle(void);
void DragAutoXGauges(WM_HWIN hPrev);
WM_HWIN GetDragAutoXGaugesWindowHandle(void);
void DisplaySFKeyboard(int screenID, WM_HWIN hPrev);


extern WM_HWIN hWaitingForCamera;

#ifdef __cplusplus
}
#endif

#define	NUM_SF_SCREENS				9

#define	LONGITUDE_SCREEN			0
#define	LATITUDE_SCREEN				1
#define	HEADING_SCREEN				2
#define	START_LONGITUDE_SCREEN		3
#define	START_LATITUDE_SCREEN		4
#define	START_HEADING_SCREEN		5
#define	FINISH_LONGITUDE_SCREEN		6
#define	FINISH_LATITUDE_SCREEN		7
#define	FINISH_HEADING_SCREEN		8


#endif
