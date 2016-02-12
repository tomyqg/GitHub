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
File        : GUI_DRAW_Self.c
Purpose     : 
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUI_Private.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Draw
*/
static void _Draw(GUI_DRAW_HANDLE hDrawObj, WM_HWIN hWin, int x, int y) {
  GUI_DRAW * pDrawObj;
  GUI_DRAW_SELF_CB * pfDraw;
  
  pDrawObj = (GUI_DRAW *)GUI_LOCK_H(hDrawObj);
  pfDraw = pDrawObj->Data.pfDraw;
  GUI_UNLOCK_H(pDrawObj);

  GUI_USE_PARA(x);
  GUI_USE_PARA(y);
  pfDraw(hWin);
}

/*********************************************************************
*
*       _GetXSize
*/
static int _GetXSize(GUI_DRAW_HANDLE hDrawObj) {
  GUI_USE_PARA(hDrawObj);
  return 0;
}

/*********************************************************************
*
*       _GetYSize
*/
static int _GetYSize(GUI_DRAW_HANDLE hDrawObj) {
  GUI_USE_PARA(hDrawObj);
  return 0;
}

/*********************************************************************
*
*       static data, ConstObj
*
**********************************************************************
*/
static const GUI_DRAW_CONSTS _ConstObjData = {
  _Draw,
  _GetXSize,
  _GetYSize
};

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DRAW_SELF_Create
*/
WM_HMEM GUI_DRAW_SELF_Create(GUI_DRAW_SELF_CB * pfDraw, int x, int y) {
  WM_HMEM hMem;
  hMem = GUI_ALLOC_AllocZero(sizeof(GUI_DRAW));
  if (hMem) {
    GUI_DRAW * pObj;
    GUI_LOCK();
    pObj = (GUI_DRAW *)GUI_LOCK_H(hMem);
    pObj->pConsts = &_ConstObjData;
    pObj->Data.pfDraw = pfDraw;
    pObj->xOff    = x;
    pObj->yOff    = y;
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK();
  }
  return hMem;
}


#else                            /* Avoid problems with empty object modules */
  void GUI_DRAW_Self_C(void) {}
#endif
