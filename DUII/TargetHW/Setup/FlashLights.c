#include "commondefs.h"
#include "DUII.h"
#include "LEDHandler.h"
#include "FlashLights.h"

#define INTER_LED_TIME	100

void FlashLights(void) {
	int lednum;
	U16 ledvalue;
	U16 centervalue;
	
	// led order from left to right
	int ledOrder[NUMBER_TACH_LEDS] = { 0, 1, 15, 3, 2, 4, 8, 9, 10, 11, 12, 13, 14 };

	TurnOffLEDs();
	SetLEDBrightness(FLASHLIGHT_LED_BRIGHTNESS);
	SuspendLEDUpdates(false);
	
	ledvalue = 0;

	// add lights toward center while cycling center lights
	for (lednum = 0; lednum < 5; lednum++) {
		ledvalue = ledvalue | (1 << ledOrder[lednum]) | (1 << ledOrder[NUMBER_TACH_LEDS-lednum-1]);

		centervalue = (1 << ledOrder[5+(lednum % 3)]);
		centervalue = centervalue | ledvalue;
		
		LEDShift(centervalue);
		OS_Delay(INTER_LED_TIME);

	} // for

	// subtract lights from center while cycling center lights
	for (lednum = 4; lednum >= 0; lednum--) {
		ledvalue = ledvalue & ~((1 << ledOrder[lednum]) | (1 << ledOrder[NUMBER_TACH_LEDS-lednum-1]));

		centervalue = (1 << ledOrder[5+((lednum+2) % 3)]);
		centervalue = centervalue | ledvalue;
		
		LEDShift(centervalue);
		OS_Delay(INTER_LED_TIME);

	} // for

	// turn all on
	WarningLEDOn(&LowerLED, 1);
	WarningLEDOn(&UpperLED, 1);
	ShiftLEDLevel(4);
		
	OS_Delay(INTER_LED_TIME);
	ResumeLEDUpdates();
	TurnOffLEDs();

} // FlashLights
	
