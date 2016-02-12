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
File        : LCD_ClipRectEx.c
Purpose     : Implementation of optional routines
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_SetClipRectEx
*
* Purpose:
*   This function is actually a driver function.
*   Since it is identical for all drivers with only one controller,
*   it is placed here.
*   For multi-controller systems, this routine is placed in the
*   distribution driver.
*/

void LCD_SetClipRectEx(const GUI_RECT* pRect) {
  LCD_RECT r;
  LCDDEV_L0_GetRect(GUI_Context.apDevice[GUI_Context.SelLayer], &r);
  GUI__IntersectRects(&GUI_Context.ClipRect, pRect, &r);
}

/*************************** End of file ****************************/
