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
File        : GUIDEMO_Intro.c
Purpose     : Introduction for emWin generic demo
              (This is also a good file to demo and explain basic
              emWin features by setting breakpoints)
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "GUIDEMO.h"
#include <string.h>

/*********************************************************************
*
*       GUIDEMO_Intro
*
**********************************************************************
*/
void GUIDEMO_Intro(void) {
  int xCenter;
  int y, yPos;
  char acText[50] = "Version of emWin: ";
  int lcd_xsize;
  int lcd_ysize;
  lcd_xsize = LCD_GetXSize();
  lcd_ysize = LCD_GetYSize();
  xCenter = lcd_xsize >> 1;
  strcat(acText, GUI_GetVersionString());
  GUIDEMO_SetBkColor(GUI_BLUE);
  GUIDEMO_SetColor(GUI_WHITE);
  GUI_Clear();
  GUI_SetFont(&GUI_FontComic18B_ASCII);
  GUI_DispStringHCenterAt("emWin", xCenter, y = 12);
  GUI_SetFont(&GUI_Font13H_ASCII);
  GUI_DispStringHCenterAt("Universal graphic software"
                          "\nfor embedded applications"
                          , xCenter, y += 23);
  GUI_SetColor(GUI_RED);
  GUI_SetFont(&GUI_Font16_ASCII);
  GUI_DispStringHCenterAt("Any CPU - Any LCD - Any Application"
                          , xCenter, y += 32);
  GUI_SetFont(&GUI_Font10S_ASCII);
  GUI_DispStringHCenterAt("Compiled " __DATE__ " "__TIME__, xCenter, y += 18);
  GUI_SetFont(&GUI_Font13HB_ASCII);
  GUI_SetColor(GUI_WHITE);
  GUI_DispStringHCenterAt(acText, xCenter, y += 15);
  GUI_SetFont(&GUI_FontComic18B_ASCII);
  GUI_SetColor(GUI_WHITE);
  GUI_DispStringHCenterAt("www.segger.com", lcd_xsize >> 1, lcd_ysize - 20);
  yPos  = y + 15 + ((lcd_ysize - y - 33) >> 1);
  yPos -= bmSeggerLogoBlue.YSize >> 1;
  if (yPos > 160) {
    yPos = 160;
  }
  GUI_DrawBitmap(&bmSeggerLogoBlue, (lcd_xsize - bmSeggerLogoBlue.XSize) >> 1, yPos);
  GUI_SetFont(&GUI_Font10S_ASCII);
  GUI_DispStringAt("GUI_OS: ", 0, lcd_ysize - 30); GUI_DispDecMin(GUI_OS);
  GUI_DispStringAt("GUI_ALLOC_SIZE: ",0, lcd_ysize - 20); GUI_DispDecMin(GUI_ALLOC_SIZE);
  GUI_DispStringAt("Compiler: "
  #ifdef _MSC_VER
    "Microsoft"
  #elif defined (NC308)
    "Mitsubishi NC308"
  #elif defined (NC30)
    "Mitsubishi NC30"
  #elif defined (__TID__)
    #if (((__TID__ >>8) &0x7f) == 48)            /* IAR MC80 */
      "IAR M32C"
    #elif (((__TID__ >>8) &0x7f) == 85)          /* IAR V850 */
      "IAR V850"
    #elif defined (__ICCARM__)                   /* IAR ARM  */
      "IAR ARM"
    #else                                        /* IAR MC16 */
      "IAR M32C"
    #endif
  #elif defined __WATCOMC__                      /* WATCOM */
      "WATCOM"
  #elif defined __GNUC__
      "GNU"
  #else
    "Unknown"
  #endif
    ,0, lcd_ysize - 10);
  GUIDEMO_Delay(5000);
  GUIDEMO_NotifyStartNext();
}
