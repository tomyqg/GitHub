/*********************************************************************
*               SEGGER MICROCONTROLLER SYSTEME GmbH                  *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2008  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS version: 3.52e                                         *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : GUI_HelloWorld.c
Purpose : emWin and embOS demo application
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "GUI.h"
#include "commondefs.h"

#if	_WINDOWS
#include "SIMConf.h"
#endif


/*********************************************************************
*
*       main()
*/
void MainTask(void);
void MainTask(void)
{
	GUI_Init();
	
#ifdef _WINDOWS
	SIM_Init();
#endif

	GUI_SetFont(&GUI_Font32_ASCII);
        GUI_DispString("BEGIN COLOR TEST ");
         
	GUI_SetColor(GUI_BLUE); //          0x00FF0000
        GUI_DispString("BLUE ");
	GUI_SetColor(GUI_GREEN); //         0x0000FF00
        GUI_DispString("GREEN\n");
        
	GUI_SetColor(GUI_DARKBLUE); //      0x00800000
        GUI_DispString("DARKBLUE ");
	GUI_SetColor(GUI_DARKGREEN); //     0x00008000
        GUI_DispString("DARKGREEN ");
	GUI_SetColor(GUI_DARKRED); //       0x00000080
        GUI_DispString("DARKRED\n");
        
	GUI_SetColor(GUI_RED); //           0x000000FF
        GUI_DispString("RED ");
	GUI_SetColor(GUI_CYAN); //          0x00FFFF00
        GUI_DispString("CYAN ");
	GUI_SetColor(GUI_MAGENTA); //       0x00FF00FF
        GUI_DispString("MAGENTA ");
	GUI_SetColor(GUI_YELLOW); //        0x0000FFFF
        GUI_DispString("YELLOW\n");
        
	GUI_SetColor(GUI_LIGHTBLUE); //     0x00FF8080
        GUI_DispString("LIGHTBLUE ");
	GUI_SetColor(GUI_LIGHTGREEN); //    0x0080FF80
        GUI_DispString("LIGHTGREEN ");
	GUI_SetColor(GUI_LIGHTRED); //      0x008080FF
        GUI_DispString("LIGHTRED\n");
        
	GUI_SetColor(GUI_LIGHTCYAN); //     0x00FFFF80
        GUI_DispString("LIGHTCYAN ");
	GUI_SetColor(GUI_LIGHTMAGENTA); //  0x00FF80FF
        GUI_DispString("LIGHTMAGENTA ");
	GUI_SetColor(GUI_LIGHTYELLOW); //   0x0080FFFF
        GUI_DispString("LIGHTYELLOW\n");
        
	GUI_SetColor(GUI_DARKCYAN); //      0x00808000
        GUI_DispString("DARKCYAN ");
	GUI_SetColor(GUI_DARKMAGENTA); //   0x00800080
        GUI_DispString("DARKMAGENTA ");
 	GUI_SetColor(GUI_RED); //           0x000000FF
        GUI_DispString("RED\n");
       
	GUI_SetColor(GUI_DARKYELLOW); //    0x00008080
        GUI_DispString("DARKYELLOW ");
	GUI_SetColor(GUI_WHITE); //         0x00FFFFFF
        GUI_DispString("WHITE ");
	GUI_SetColor(GUI_LIGHTGRAY); //     0x00D3D3D3
        GUI_DispString("LIGHTGRAY\n");
        
	GUI_SetColor(GUI_GREEN); //          0x00808080
        GUI_DispString("GREEN ");
	GUI_SetColor(GUI_DARKGRAY); //      0x00404040
        GUI_DispString("DARKGRAY ");
	GUI_SetColor(GUI_YELLOW);
        GUI_DispString("YELLOW ");
	GUI_SetColor(GUI_BROWN); //         0x002A2AA5
        GUI_DispString("BROWN\n");
	// Main task loop
	while(1)
	{
		GUI_Delay(10);
#ifdef _WINDOWS
		if (SIM_Tick())
			return;
#endif
	}
}

/****** End of File *************************************************/
