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
File        : GUI_SetClipRect.c
Purpose     : Implementation of GUI_SetClipRect
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI.h"

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
*       GUI_SetClipRect
*
* Purpose:
*   Sets the rectangle used for clipping.
*
* Parameter:
*   pRect - Points to the rectangle to be used for clipping. If the WM
*           is used, the pointer needs to remain valid during the given
*           rectangle should be used for clipping.
*/
const GUI_RECT * GUI_SetClipRect(const GUI_RECT * pRect) {
  const GUI_RECT * pOldRect;
  GUI_LOCK();
  #if GUI_WINSUPPORT
    pOldRect = WM_SetUserClipRect(pRect);
  #else
    pOldRect = NULL;
    {
      GUI_RECT Rect;
      if (!pRect) {
        Rect.x0 = -4096;
        Rect.y0 = -4096;
        Rect.x1 =  4096;
        Rect.y1 =  4096;
        pRect = &Rect;
      }
      LCD_SetClipRectEx(pRect);
    }
  #endif
  GUI_UNLOCK();
  return pOldRect;
}

/*************************** End of file ****************************/
