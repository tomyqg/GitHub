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
File        : GUI_DrawFocusRect.C
Purpose     : Implementation of GUI_DrawFocusRect
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawFocusRect
*/
static void _DrawFocusRect(const GUI_RECT* pr) {
  int i; 
  for (i = pr->x0; i <= pr->x1; i += 2) {
    LCD_DrawPixel(i, pr->y0);
    LCD_DrawPixel(i, pr->y1);
  }
  for (i = pr->y0; i <= pr->y1; i += 2) {
    LCD_DrawPixel(pr->x0, i);
    LCD_DrawPixel(pr->x1, i);
  }
}
/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DrawFocusRect
*/
void GUI_DrawFocusRect(const GUI_RECT *pRect, int Dist) {
  GUI_RECT r;
  GUI__ReduceRect(&r, pRect, Dist);
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(r.x0, r.y0);
    WM_ADDORG(r.x1, r.y1);
    WM_ITERATE_START(&r); {
  #endif
    _DrawFocusRect(&r);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
