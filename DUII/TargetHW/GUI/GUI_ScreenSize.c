/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2010  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.06 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUI_ScreenSize.c
Purpose     : Implementation of screen size functions
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static int _ScreenSizeX;
static int _ScreenSizeY;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SetScreenSizeX
*/
void GUI_SetScreenSizeX(int xSize) {
  _ScreenSizeX = xSize;
}

/*********************************************************************
*
*       GUI_SetScreenSizeY
*/
void GUI_SetScreenSizeY(int ySize) {
  _ScreenSizeY = ySize;
}

/*********************************************************************
*
*       GUI_GetScreenSizeX
*/
int GUI_GetScreenSizeX(void) {
  if (!_ScreenSizeX) {
    _ScreenSizeX = LCD_GetXSize();
  }
  return _ScreenSizeX;
}

/*********************************************************************
*
*       GUI_GetScreenSizeY
*/
int GUI_GetScreenSizeY(void) {
  if (!_ScreenSizeY) {
    _ScreenSizeY = LCD_GetYSize();
  }
  return _ScreenSizeY;
}

/*************************** End of file ****************************/
