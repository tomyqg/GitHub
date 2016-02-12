
#ifndef	_ANALOGINPUTSETTINGS_H
#define	_ANALOGINPUTSETTINGS_H


WM_HWIN GetAnalogInputSettingsWindowHandle(void);
void SetAnalogInputSettingsWindowHandle(WM_HWIN);
extern struct SCREEN_LIST AnalogInputSettingsScreenList[];
extern void AnalogInputSettingsCallback(WM_MESSAGE *);
void AnalogInputSettingsIgnoreNextKeyRelease(void);
void DeleteAnalogKeyboards(void);
void AISKillKeyboard(void);
char ValidateAnalogData(void);

#define	NUM_ANALOG_INPUT_SCREENS	4

#define	SCALE				0
#define	ALARM				1
#define	RENAME_INPUT	2
#define	RENAME_UNITS	3


#endif
