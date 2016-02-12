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
File        : GUI_DRAW_BITMAP.c
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
*       _DrawBitmap
*/
static void _DrawBitmap(GUI_DRAW_HANDLE hDrawObj, WM_HWIN hWin, int x, int y) {
  GUI_DRAW * pDrawObj;
  const void * pData;
  I16 xOff, yOff;

  GUI_USE_PARA(hWin);
  pDrawObj = (GUI_DRAW *)GUI_LOCK_H(hDrawObj);
  pData = pDrawObj->Data.pData;
  xOff  = pDrawObj->xOff;
  yOff  = pDrawObj->yOff;
  GUI_UNLOCK_H(pDrawObj);
  GUI_DrawBitmap((const GUI_BITMAP *)pData, x + xOff, y + yOff);
}

/*********************************************************************
*
*       _GetXSize
*/
static int _GetXSize(GUI_DRAW_HANDLE hDrawObj) {
  GUI_DRAW * pDrawObj;
  const void * pData;

  pDrawObj = (GUI_DRAW *)GUI_LOCK_H(hDrawObj);
  pData = pDrawObj->Data.pData;
  GUI_UNLOCK_H(pDrawObj);
  return ((const GUI_BITMAP *)pData)->XSize;
}

/*********************************************************************
*
*       _GetYSize
*/
static int _GetYSize(GUI_DRAW_HANDLE hDrawObj) {
  GUI_DRAW * pDrawObj;
  const void * pData;

  pDrawObj = (GUI_DRAW *)GUI_LOCK_H(hDrawObj);
  pData = pDrawObj->Data.pData;
  GUI_UNLOCK_H(pDrawObj);
  return ((const GUI_BITMAP *)pData)->YSize;
}

/*********************************************************************
*
*       static data, ConstObj
*
**********************************************************************
*/
static const GUI_DRAW_CONSTS _ConstObjData = {
  _DrawBitmap,
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
*       GUI_DRAW_BITMAP_Create
*/
WM_HMEM GUI_DRAW_BITMAP_Create(const GUI_BITMAP* pBitmap, int x, int y) {
  WM_HMEM hMem = 0;
  if (pBitmap) {
    GUI_DRAW* pObj;
    hMem = GUI_ALLOC_AllocZero(sizeof(GUI_DRAW));
    if (hMem) {
      WM_LOCK();
      pObj = (GUI_DRAW*)GUI_LOCK_H(hMem);
      pObj->pConsts    = &_ConstObjData;
      pObj->Data.pData = (const void*)pBitmap;
      pObj->xOff       = x;
      pObj->yOff       = y;
      GUI_UNLOCK_H(pObj);
      WM_UNLOCK();
    }
  }
  return hMem;
}


#else                            /* Avoid problems with empty object modules */
  void GUI_DRAW_BITMAP_C(void) {}
#endif
