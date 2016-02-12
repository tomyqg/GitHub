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
File        : GUI_RectsIntersect.C
Purpose     : Implementation of GUI_RectsIntersect
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_RectsIntersect
*
* Purpose:
*   Check if rectangle do intersect.
*
* Return value:
*   0 if they do not.
*   1 if they do.
*/
int GUI_RectsIntersect(const GUI_RECT* pr0, const GUI_RECT* pr1) {
  if (pr0->y0 <= pr1->y1) {
    if (pr1->y0 <= pr0->y1) {
      if (pr0->x0 <= pr1->x1) {
        if (pr1->x0 <= pr0->x1) {
          return 1;
        }
      }
    }
  }
  return 0;
}

/*************************** End of file ****************************/
