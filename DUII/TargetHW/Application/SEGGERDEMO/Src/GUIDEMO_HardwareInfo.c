/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2004  SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

***** emWin - Graphical user interface for embedded applications *****
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEMO_HardwareInfo.c
Purpose     : Displays hardware info
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "GUIDEMO.h"
#include "LCD_ConfDefaults.h"      /* valid LCD configuration */

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/

#if (LCD_CONTROLLER == 1375) | (LCD_CONTROLLER == 13705) | (LCD_CONTROLLER == 1386) | (LCD_CONTROLLER == 13806)
static void _Show(const char* s0, const char* s1) {
  GUIDEMO_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_Font13B_ASCII);
  GUI_DispString("\n");
  GUI_DispString(s0);
  GUIDEMO_Delay(500);
  GUI_GotoX(110);
  GUIDEMO_SetColor(GUI_GREEN);
  GUI_DispString(s1);
  GUIDEMO_Delay(1000);
}
#endif

/*********************************************************************
*
*       GUIDEMO_HardwareInfo
*
**********************************************************************
*/

void GUIDEMO_HardwareInfo(void) {
  #if (LCD_CONTROLLER == 1375) | (LCD_CONTROLLER == 13705) | (LCD_CONTROLLER == 1386) | (LCD_CONTROLLER == 13806)
    GUIDEMO_ShowIntro("Hardware info",   "" );
    GUI_Clear();
    GUI_SetFont(&GUI_FontComic24B_ASCII);
    GUIDEMO_SetColor(GUI_WHITE);
    #if (LCD_CONTROLLER == 1375) | (LCD_CONTROLLER == 13705)
      GUI_DispStringAt("EPSON 13705\n", 10, 65 );
      _Show("On chip VRAM",  "80 kb");
      _Show("Max. colors",   "4096 (4 bit per color)");
      _Show("Display types", "Mono / color / TFT");
      _Show("LUT",           "256 index colors");
    #elif (LCD_CONTROLLER == 1386) | (LCD_CONTROLLER == 13806)
      GUI_DispStringAt("EPSON 13806\n", 10, 65 );
      _Show("On chip VRAM",  "1200 kb");
      _Show("Max. colors",   "65536 (up to 16 bpp)");
      _Show("Display types", "Mono / color / TFT (analog / digital)");
      _Show("LUT",           "256 index colors");
    #endif
    GUIDEMO_Delay(1500);
  #endif
}
