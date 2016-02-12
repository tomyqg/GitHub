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
File        : GUI_GetClientRect.c
Purpose     : Implementation of GUI_GetClientRect
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"
#include "GUI_Debug.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GetClientRect
*/
void GUI_GetClientRect(GUI_RECT* pRect) {
  if (!pRect)
    return;
  #if GUI_WINSUPPORT
    WM_GetClientRect(pRect);
  #else
    pRect->x0 = 0;
    pRect->y0 = 0;
    pRect->x1 = LCD_GetVXSize();
    pRect->y1 = LCD_GetVYSize();
  #endif
}

/*************************** End of file ****************************/
