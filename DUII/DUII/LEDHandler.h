
#ifndef	LEDHANDLER_H
#define	LEDHANDLER_H

#define	UPPER_WARNING_LED		1
#define	LOWER_WARNING_LED		2
#define BOTH_WARNING_LED		3

#define NUMBER_TACH_LEDS		13

#define	MINIMUM_BRIGHTNESS	0			// 100 scale
#define MAXIMUM_BRIGHTNESS	99

#define	DEFAULT_LIGHT_LEVEL_WHEN_LED_SET		50
#define	DEFAULT_BACKLIGHT_BRIGHTNESS			50
#define	DEFAULT_LED_BRIGHTNESS					50
#define FLASHLIGHT_LED_BRIGHTNESS				5

void SetTachLEDs(int rpm);
void SetWarningLED(int type, float value);
void SuspendLEDUpdates(char lightsOn);
void ResumeLEDUpdates(void);
void TurnOffLEDs(void);
void RecordAmbientLight(void);
void SuspendEnvironmentalUpdates(void);
void ResumeEnvironmentalUpdates(void);
void AutoAdjustOn(char);
void LEDSystemShutdown(void);
int InAlarm(int);
void WarningPopupInit(void);
void DecreaseLEDBrightness(void);
void IncreaseLEDBrightness(void);
void DecreaseBacklightBrightness(void);
void IncreaseBacklightBrightness(void);
unsigned int GetNormalizedLightIntensity(void);
void InitializeLEDs(void);
void InitializeLEDStructures(void);

extern struct LED_DATA UpperLED, LowerLED;

extern void WarningLEDOff(struct LED_DATA *pLED, int override);
extern void WarningLEDOn(struct LED_DATA *pLED, int override);


#ifdef __cplusplus
extern "C" {
#endif

void CheckWarningPopups(void);

#ifdef __cplusplus
}
#endif
  

#ifndef _WINDOWS
void SetLEDBrightness(int brightness);
void SetBacklightBrightness(int brightness);
void LEDHandlerTask(void);
void LEDShift(U16);
void ShiftLEDLevel(char);
#endif

#endif
