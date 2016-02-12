
#ifndef	_ANALOGINPUTSSCALE_H
#define	_ANALOGINPUTSSCALE_H


WM_HWIN GetAnalogInputsScaleWindowHandle(void);
void SetAnalogInputsScaleWindowHandle(WM_HWIN);
extern struct SCREEN_LIST AnalogInputsScaleScreenList[];
extern void AnalogInputsScaleCallback(WM_MESSAGE *);
void AnalogInputsScaleIgnoreNextKeyRelease(void);
void DeleteAnalogScaleKeyboards(void);

#define	NUM_ANALOG_SCALE_SCREENS	4

#define	VOLTS_LOW_SCREEN		0
#define	VOLTS_HIGH_SCREEN		1
#define	VALUE_LOW_SCREEN		2
#define	VALUE_HIGH_SCREEN		3


#endif
