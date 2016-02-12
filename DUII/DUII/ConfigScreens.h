
#ifndef	_CONFIGSCREENS_H
#define	_CONFIGSCREENS_H

#include "InterTask.h"

// screen_scrolling values
#define	INACTIVE			0
#define	ACTIVE				1
#define	HORIZONTAL_ONLY		2

#define	FINGER_WIDTH		35

#ifdef __cplusplus
extern "C" {
#endif

enum GUI_RACE_MODES {
	ROADRACE_MODE,
	AUTOXRALLY_MODE,
	INSTRUMENTS_MODE,
	DRIVE_MODE,
	DRAG_MODE,
	NO_CHANGE_MODE
};

enum RACE_MODES {
	RM_INITIAL,
	RM_FOUND_SATELLITES,
	RM_CANCELLED_WFS,
	RM_SELECTED_TRACK,
	RM_CANCELLED_SELECT_TRACK,
	RM_CANCELLED_WAITING_FOR_CAMERA,
	RM_CAMERA_READY,
	RM_CAMERA_RECORDING,
	RM_HAVE_SF,
	RM_STAGED,
	RM_LAUNCH_DETECTED,
	RM_RACING
};

void SetLastScreen(enum SCREEN_TYPE screen_type, int screen);
void SetScreenType(enum SCREEN_TYPE screentype);
enum SCREEN_TYPE GetScreenType(void);
int GetPrevIndex(void);
int GetNextIndex(void);
void DisplayNewScreenList(enum SCREEN_TYPE, int, WM_HWIN);
int CS_IsVisible(int screen);
void ProcessScreenMovement(void);
int CSSetYOrigin(int y);
extern void _cbMainCallback(WM_MESSAGE *);
extern WM_HWIN GetConfigWindowHandle(void);
int GetCurrentScreen(void);
int GetLastScreen(enum SCREEN_TYPE screenType);
void ConfigScreensIgnoreNextKeyRelease(void);
void popupCallback(WM_MESSAGE * pMsg);
void SetUserMode(DU2_STATE_TYPE mode);
void StartPressed(WM_HWIN hLastScreen);
void GoRacingStateMachine(WM_HWIN hParent);
int StartupGoToRaceMode(void);
int CheckForInfoKey(int hasHelp);
void CheckSF_ThenGoRacing(enum GUI_RACE_MODES mode, WM_HWIN hParent);
void GoRacingStateMachine(WM_HWIN hParent);
void GaugeScreensIgnoreNextKeyRelease(void);
char TouchPosIsBackKey(void);
void ExitRecord(char saveData);
int NotSupportedPopup(WM_HWIN owner);
char AnyGaugeScreen(enum SCREEN_TYPE type);
void IncrementSessionType(void);
char CheckSlideoutTimer(WM_HWIN hWin);
void ValidateRaceScreens(void);
int DetermineAnalogScreenSelection(void);

#ifdef	INCLUDE_RACE_HELP
void StartRaceHelpScreenTimer(void);
void CheckRaceHelpScreen(int screenID);

extern int raceStartTime;
extern int raceTimerActive;
#endif

extern int screen_scrolling;
extern int pressingbutton_semaphore;
extern int selectedSessionUnsorted;
extern long hScreenTimer;
extern char processingScreenTransition;
extern enum RACE_MODES raceModeState;
extern int glbBlackoutFlag;
extern enum GUI_RACE_MODES selectedRaceMode;
extern char SlideOutDeployed(void);
extern char SlideOutVisible(void);

#define PauseScreenScrolling()	(screen_scrolling = INACTIVE)
#define ResumeScreenScrolling()	(screen_scrolling = ACTIVE)
#define	SetScreenScrollingHOnly()	(screen_scrolling = HORIZONTAL_ONLY)

#ifdef __cplusplus
}
#endif

#define	GO_RACING				0
#define	RACE_SETUP				1
#define	REVIEW_SESSIONS			2
#define	SYSTEM_SETUP			3

#define	NUM_CONFIG_SCREENS		4


typedef struct SCREEN_LIST {
	int	screenID;
	void (*constructor)(WM_HWIN);
	void (*paint)(WM_HWIN);
} SCREENLIST;

#endif
