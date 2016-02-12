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
File        : GUIDEMO_Circle.c
Purpose     : Drawing circles
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "GUIDEMO.h"

/*********************************************************************
*
*       GUIDEMO_Circle
*
**********************************************************************
*/

void GUIDEMO_Circle(void) {
  int x0, y0, r;
  int lcd_xsize;
  int lcd_ysize;
  lcd_xsize = LCD_GetXSize();
  lcd_ysize = LCD_GetYSize();
  GUIDEMO_ShowIntro("Circles",
                    "\nArbitrary circles"
                    "\nand arcs.");
  GUI_Clear();
  GUI_SetFont(&GUI_Font8x16);
  GUI_SetColor(GUI_WHITE);
  x0 = lcd_xsize >> 1;
  y0 = lcd_ysize >> 1;
  r = 50;
  GUI_DispStringAt("Circle", 0, 0);
  GUI_FillCircle (x0, y0, r);
  GUIDEMO_Wait();
  GUI_SetDrawMode(GUI_DM_XOR);
  GUI_FillCircle (x0, y0, r + 10);
  GUI_SetDrawMode(0);
  GUIDEMO_Wait();
  #if GUI_SUPPORT_AA
    GUIDEMO_ShowInfo("Circles with\nAntialiasing");
    GUI_Clear();
    GUI_SetColor(GUI_WHITE);
    GUI_DispStringAt("Antialiased\ncircle", 0, 0);
    GUI_AA_FillCircle(x0, y0, r - 2);
    GUIDEMO_Wait();
  #endif
}
