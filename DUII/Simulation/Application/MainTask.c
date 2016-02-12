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
File        : MainTask.C
Purpose     : Main program, called from after main after initialisation
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       External data
*
**********************************************************************
*/
extern const GUI_BITMAP bmSeggerLogoBlue;
extern const GUI_BITMAP bmSeggerLogo_1bpp;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {
  int i, yPos, Len, xSize, ySize;
  const GUI_BITMAP * pBitmap;

  GUI_Init();
  GUI_SetBkColor(GUI_RED);
  GUI_Clear();
  GUI_Delay(1000);
  GUI_SetBkColor(GUI_BLUE);
  GUI_Clear();
  GUI_Delay(1000);
  GUI_SetColor(GUI_WHITE);
  for (i = 0; i < 1000; i += 10) {
    GUI_DrawHLine(i, 0, 100);
    GUI_DispStringAt("Line ", 0, i);
    GUI_DispDecMin(i);
  }
  GUI_Delay(1000);
  GUI_SetColor(GUI_BLACK);
  GUI_SetBkColor(GUI_WHITE);
  for (i = 0; i < 160; i++) {
    Len = (i < 80) ? i : 160 - i;
    GUI_DrawHLine(i, 20, Len + 20);
  }
  GUI_Delay(1000);
  GUI_Clear();
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  if (ySize > (100 + bmSeggerLogo_1bpp.YSize)) {
    pBitmap = &bmSeggerLogoBlue;
  } else {
    GUI_SetColor(GUI_BLUE);
    pBitmap = &bmSeggerLogo_1bpp;
  }
  yPos = 20 + pBitmap->YSize;
  GUI_DrawBitmap(pBitmap,(xSize - pBitmap->XSize) / 2, 10);
  GUI_SetFont(&GUI_FontComic24B_1);
  GUI_DispStringHCenterAt("www.segger.com",xSize / 2, yPos);
  GUI_Delay(1000);
  GUI_SetColor(GUI_RED);
  GUI_DispStringHCenterAt("© 2009\n", xSize / 2, yPos + 30);
  GUI_SetFont(&GUI_Font10S_1);
  GUI_DispStringHCenterAt("Segger Microcontroller Systeme GmbH", xSize / 2, yPos + 60);;
  GUI_Delay(1000);
}

/*************************** End of file ****************************/

