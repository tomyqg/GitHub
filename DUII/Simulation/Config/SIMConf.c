/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2009  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.00 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : SIMConf.c
Purpose     : Windows Simulator configuration
---------------------------END-OF-HEADER------------------------------
*/

#ifdef WIN32

#include "LCD_SIM.h"
#include "SIM.h"
#include "SIMConf.h"
#include "FlashReadWrite.h"
//
static int exit = 0;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       SIM_X_Config
*
* Purpose:
*   Called during the initialization process in order to configure
*   the simulator.
*/
void SIM_X_Config() {
  // KMC
	SIM_GUI_UseCustomBitmaps();
	SIM_GUI_SetLCDPos(87, 127);				// Position of upper left corner of screen
	SIM_GUI_SetTransColor(0x0000FF);		// Blue

}

#else

void SIMConf_C(void); // Avoid empty object files
void SIMConf_C(void) {}

#endif

void SIM_Powerkey_CB(int KeyIndex, int State)
{
	if (KeyIndex == POWER_KEY)
	{
		// Power key
		if (State == 1)		// key pressed
		{
//			WriteDataStructures(); // This should now save track data - user data saved as we go
		}
		if (State == 0)		// key released
			exit = 1;
	}
}

//	Configure simulation keys - be sure the power key is the correct key
void SIM_Init(void)
{
	SIM_HARDKEY_SetCallback(POWER_KEY, *SIM_Powerkey_CB);
}

int SIM_Tick(void)
{
	return exit;
}

/*************************** End of file ****************************/
