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
File        : GUI_DRAW.c
Purpose     : member functions of GUI_DRAW Object
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUI_Private.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DRAW__Draw
*/
void GUI_DRAW__Draw(GUI_DRAW_HANDLE hDrawObj, WM_HWIN hWin, int x, int y) {
  GUI_DRAW * pDrawObj;
  void (* pfDraw)(GUI_DRAW_HANDLE hDrawObj, WM_HWIN hWin, int x, int y);

  if (hDrawObj) {
    pDrawObj = (GUI_DRAW *)GUI_LOCK_H(hDrawObj);
    pfDraw = pDrawObj->pConsts->pfDraw;
    GUI_UNLOCK_H(pDrawObj);
    pfDraw(hDrawObj, hWin, x, y);
  }
}

/*********************************************************************
*
*       GUI_DRAW__GetXSize
*/
int GUI_DRAW__GetXSize(GUI_DRAW_HANDLE hDrawObj) {
  GUI_DRAW * pDrawObj;
  int  (* pfGetXSize)(GUI_DRAW_HANDLE hDrawObj);
  int r = 0;

  if (hDrawObj) {
    pDrawObj = (GUI_DRAW *)GUI_LOCK_H(hDrawObj);
    pfGetXSize = pDrawObj->pConsts->pfGetXSize;
    GUI_UNLOCK_H(pDrawObj);
    r = pfGetXSize(hDrawObj);
  }
  return r;
}

/*********************************************************************
*
*       GUI_DRAW__GetYSize
*/
int GUI_DRAW__GetYSize(GUI_DRAW_HANDLE hDrawObj) {
  GUI_DRAW * pDrawObj;
  int  (* pfGetYSize)(GUI_DRAW_HANDLE hDrawObj);
  int r = 0;

  if (hDrawObj) {
    pDrawObj = (GUI_DRAW *)GUI_LOCK_H(hDrawObj);
    pfGetYSize = pDrawObj->pConsts->pfGetYSize;
    GUI_UNLOCK_H(pDrawObj);
    r = pfGetYSize(hDrawObj);
  }
  return r;
}

#else                            /* Avoid problems with empty object modules */
  void GUI_DRAW_C(void) {}
#endif

/*************************** End of file ****************************/

