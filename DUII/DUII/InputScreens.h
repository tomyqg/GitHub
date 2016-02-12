
#ifndef	_INPUTSSCREENS_H
#define	_INPUTSSCREENS_H

// RAW == "HIGH", "LOW" for digital or volts for analog
// USER == User-defined names, default is "ON", "OFF" for digital, scaled input reading in user units for Analog
enum RAW_OR_USER { RAW, USER };
enum UPPER_LOWER { LOWER, UPPER };

WM_HWIN GetInputsWindowHandle(void);
void SetInputsWindowHandle(WM_HWIN);
void InputsIgnoreNextKeyRelease(void);
float GetAnalogInput(enum RAW_OR_USER type, int input);
char *GetDigitalText(enum RAW_OR_USER type, int input);

extern struct SCREEN_LIST InputsScreenList[];
extern void InputsScreensCallback(WM_MESSAGE *);
extern char globalInput;
extern char *GetAlarmTriggerText(char input, enum UPPER_LOWER upperLower);
void InputCleanUpKeyboards(void);


#define	NUM_INPUTS_SCREENS	6

#define	ANALOG0			0
#define	ANALOG1			1
#define	ANALOG2			2
#define	ANALOG3			3
#define	DIGITAL4			4
#define	DIGITAL5			5


#endif
