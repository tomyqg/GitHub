#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "commondefs.h"
#include "GUI.h"
#include "InterTask.h"
#include "DUII.h"
#include "LEDHandler.h"
#include "FactoryCheckout.h"

extern void reset( void );

void FactoryCheckout (void) {
	
	GUI_Clear();

	GUI_SetFont(&GUI_Font32_ASCII);

	GUI_SetColor(GUI_WHITE); //         0x00FFFFFF
       GUI_DispString("WHITE ");
	GUI_SetColor(GUI_RED); //           0x000000FF
        GUI_DispString("RED ");
	GUI_SetColor(GUI_BLUE); //          0x00FF0000
        GUI_DispString("BLUE ");
	GUI_SetColor(GUI_GREEN); //         0x0000FF00
        GUI_DispString("GREEN ");
	GUI_SetColor(GUI_YELLOW); //        0x0000FFFF
        GUI_DispString("YELLOW\n");

	GUI_SetColor(GUI_DARKBLUE); //      0x00800000
        GUI_DispString("DARKBLUE ");
	GUI_SetColor(GUI_DARKGREEN); //     0x00008000
        GUI_DispString("DARKGREEN ");
	GUI_SetColor(GUI_DARKRED); //       0x00000080
        GUI_DispString("DARKRED\n");

	GUI_SetColor(GUI_ORANGE); //        0x000080FF
        GUI_DispString("ORANGE ");
	GUI_SetColor(GUI_CYAN); //          0x00FFFF00
        GUI_DispString("CYAN ");
	GUI_SetColor(GUI_BROWN); //         0x002A2AA5
        GUI_DispString("BROWN ");
	GUI_SetColor(GUI_MAGENTA); //       0x00FF00FF
        GUI_DispString("MAGENTA\n");

	GUI_SetColor(GUI_LIGHTBLUE); //     0x00FF8080
        GUI_DispString("LIGHTBLUE ");
	GUI_SetColor(GUI_LIGHTGREEN); //    0x0080FF80
        GUI_DispString("LIGHTGREEN ");
	GUI_SetColor(GUI_LIGHTRED); //      0x008080FF
        GUI_DispString("LIGHTRED\n");

	GUI_SetColor(GUI_DARKGRAY); //      0x00404040
        GUI_DispString("DARKGRAY ");
	GUI_SetColor(GUI_DARKCYAN); //      0x00808000
        GUI_DispString("DARKCYAN ");
	GUI_SetColor(GUI_LIGHTGRAY); //     0x00D3D3D3
       GUI_DispString("LIGHTGRAY\n");
		
	GUI_SetColor(GUI_LIGHTCYAN); //     0x00FFFF80
        GUI_DispString("LIGHTCYAN ");
	GUI_SetColor(GUI_GRAY);
        GUI_DispString("GRAY ");
	GUI_SetColor(GUI_DARKMAGENTA); //   0x00800080
        GUI_DispString("DARKMAGENTA\n");

	GUI_SetColor(GUI_DARKYELLOW); //    0x00008080
        GUI_DispString("DARKYELLOW ");
	GUI_SetColor(GUI_LIGHTMAGENTA); //  0x00FF80FF
        GUI_DispString("LIGHTMAGENTA\n");
		
	GUI_SetColor(GUI_LIGHTYELLOW); //   0x0080FFFF
        GUI_DispString("LIGHTYELLOW ");
	GUI_SetColor(GUI_DARKBROWN);
        GUI_DispString("DARKBROWN\n");

	// led order from left to right
	int ledOrder[NUMBER_TACH_LEDS] = { 0, 1, 15, 3, 2, 4, 8, 9, 10, 11, 12, 13, 14 };

	SetLEDBrightness(DEFAULT_LED_BRIGHTNESS);
	SuspendLEDUpdates(false);
	
	while (1) {
		int lednum;
		
		TurnOffLEDs();
		WarningLEDOn(&LowerLED, 1);
		
		OS_Delay(500);

		TurnOffLEDs();
		WarningLEDOn(&UpperLED, 1);

		OS_Delay(500);

		TurnOffLEDs();

		for (lednum = 0; lednum < NUMBER_TACH_LEDS; lednum++) {
			LEDShift((U16) (1 << ledOrder[lednum]));
			OS_Delay(500);
		} // for
		
		// turn all on
		WarningLEDOn(&LowerLED, 1);
		WarningLEDOn(&UpperLED, 1);
		ShiftLEDLevel(4);
		
		OS_Delay(500);
		
	} // while
} // FactoryCheckout
	
