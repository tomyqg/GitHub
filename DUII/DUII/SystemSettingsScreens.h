
#ifndef	_SYSTEMSETTINGSSCREENS_H
#define	_SYSTEMSETTINGSSCREENS_H


#ifdef __cplusplus
extern "C" {
#endif

WM_HWIN GetSystemSettingsWindowHandle(void);
void SetSystemSettingsWindowHandle(WM_HWIN);
extern struct SCREEN_LIST SystemSettingsScreenList[];
extern void SystemSettingsScreensCallback(WM_MESSAGE *);
extern void SystemSettingsIgnoreNextKeyRelease(void);
extern char *GetLapMeterText(void);
extern int SystemSettingsPopUp(int screen, WM_HWIN owner);
extern void DeleteSystemSettingsPopUp(WM_HWIN hDialog);
extern void FormatSDCard(WM_HWIN);
extern void ClearNameStruct(void);
extern void nonModalPopupCallback(WM_MESSAGE * pMsg);
extern void DisplayPersonalizationScreen(WM_HWIN hPrev);
extern void ReturnToPersonalizationScreen(WM_HWIN hPrev);

extern int modalWindowUp;
extern GUI_HWIN hPersonalizationScreen;
extern int systemSettingsPopupScreen;
extern void PersonalizeScreenExit(void);
extern void DisplayRecordSpeed(WM_HWIN hParent);

#ifdef __cplusplus
}
#endif


#define	NUM_SYSTEM_SETTINGS_SCREENS	7

#define	DISPLAY_SELECTION		0
#define	UNITS_SELECTION			1
#define	TIME_SELECTION			2
#define	AUTOSTART_SELECTION		3
#define	STORAGE_SELECTION		4
#define	LAP_METER				5
#define	RECORD_SPEED			6

#define	ERASE_ALL_SESSIONS		1
#define	FACTORY_DEFAULTS		2
#define	FORMAT_SDCARD			3
#define	ALL_SESSIONS_ERASED		4
#define	RESTORED_DEFAULTS		5
#define	FORMATTING_SDCARD		6
#define	DELETE_SESSION			7
#define	INVALID_FILES			8
#define	FORMATTING_COMPLETE		9
#define	REAL_FORMATTING_SDCARD	10
#define	FORMATTING_FAILED		11
#define	ERASING_SESSIONS		12
#define	INVALID_SDCARD			13
#define	LOADING_SESSIONS		14
#define	GPS_SIMULATE_ON			15

#endif
