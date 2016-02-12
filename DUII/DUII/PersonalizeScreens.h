
#ifndef	_PERSONALIZE_H
#define	_PERSONALIZE_H


enum PERSONALIZE_SCREEN_ID {
	OWNER_SCREEN,
	CONTACTINFO_SCREEN,
	WIFI_NAME_SCREEN,
	WIFI_PASSWORD_SCREEN,
	SERIALNUMBER_SCREEN,
	BUILDDATE_SCREEN,
	TEMP_SCREEN,
	READING_SCREEN
};

WM_HWIN GetPersonalizeEntryWindowHandle(void);
void SetPersonalizeEntryWindowHandle(WM_HWIN);
extern void PersonalizeEntryCB(WM_MESSAGE *);
void PersonalizeIgnoreNextKeyRelease(void);
void DeletePersonalizeKeyboards(void);
extern void trim(char *s);
extern void DisplayNewKeyboardScreen(enum PERSONALIZE_SCREEN_ID id, GUI_HWIN hPrev);

#define	NUM_GEAR_RATIOS_SCREENS	8

#ifdef _WINDOWS
void LoadPersonalizationData(void);
#endif


#endif
