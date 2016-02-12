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
File        : GUI_DrawPoint.c
Purpose     : Implementation of point drawing routines
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       GL_DrawPoint
*/
void GL_DrawPoint(int x, int y) {
  if (GUI_Context.PenSize == 1) {
    LCD_HL_DrawPixel(x, y);
  } else {
    GL_FillCircle(x, y, (GUI_Context.PenSize - 1) / 2);
  }
}

/*********************************************************************
*
*       GUI_DrawPoint
*/
void GUI_DrawPoint(int x, int y) {
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x, y);
    WM_ITERATE_START(NULL); {
  #endif
  GL_DrawPoint(x, y);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
