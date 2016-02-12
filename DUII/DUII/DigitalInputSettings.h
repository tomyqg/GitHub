
#ifndef	_DIGITALINPUTSETTINGS_H
#define	_DIGITALINPUTSETTINGS_H


WM_HWIN GetDigitalInputSettingsWindowHandle(void);
void SetDigitalInputSettingsWindowHandle(WM_HWIN);
extern struct SCREEN_LIST DigitalInputSettingsScreenList[];
extern void DigitalInputSettingsCallback(WM_MESSAGE *);
void DigitalInputSettingsIgnoreNextKeyRelease(void);
void DeleteDigitalKeyboards(void);

#define	NUM_DIGITAL_INPUT_SCREENS	4

#define	DIG_ALARM_STATE		0
#define	DIG_RENAME_INPUT		1
#define	DIG_RENAME_HIGH		2
#define	DIG_RENAME_LOW			3


#endif
