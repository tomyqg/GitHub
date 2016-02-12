
#ifndef	_TACHSETUPSCREENS_H
#define	_TACHSETUPSCREENS_H


#ifdef __cplusplus
extern "C" {
#endif

WM_HWIN GetTachSetupWindowHandle(void);
void SetTachSetupWindowHandle(WM_HWIN);
extern struct SCREEN_LIST TachSetupScreenList[];
extern void TachSetupScreensCallback(WM_MESSAGE *);
void TachSetupIgnoreNextKeyRelease(void);
char *GetCylindersText(void);
char IndexToCylinders(char index);
char CylindersToIndex(char cyl);

#ifdef __cplusplus
}
#endif


#define	NUM_TACH_SETUP_SCREENS	5

#define	WARNING_RPM_SETUP		0
#define	REDLINE_RPM_SETUP		1
#define	ENGINE_CYLINDERS_SETUP	2
#define	POWERBANDEND_RPM_SETUP	3
#define	MAX_RPM_SETUP			4

#define	TACH_SLIDER_STEP_SIZE	500

#endif
