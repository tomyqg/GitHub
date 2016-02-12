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
File        : GUI_DRAW_STREAMED.c
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
  const void * pData;
  I16 xOff, yOff;

  GUI_USE_PARA(hWin);
  pDrawObj = (GUI_DRAW *)GUI_LOCK_H(hDrawObj);
  pData = pDrawObj->Data.pData;
  xOff  = pDrawObj->xOff;
  yOff  = pDrawObj->yOff;
  GUI_UNLOCK_H(pDrawObj);
  GUI_DrawStreamedBitmap((const GUI_BITMAP_STREAM *)pData, x + xOff, y + yOff);
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
  return ((const GUI_BITMAP_STREAM *)pData)->XSize;
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
  return ((const GUI_BITMAP_STREAM *)pData)->YSize;
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
*       GUI_DRAW_STREAMED_Create
*/
WM_HMEM GUI_DRAW_STREAMED_Create(const GUI_BITMAP_STREAM * pBitmap, int x, int y) {
  WM_HMEM hMem;
  hMem = GUI_ALLOC_AllocZero(sizeof(GUI_DRAW));
  if (hMem) {
    GUI_DRAW* pObj;
    WM_LOCK();
    pObj = (GUI_DRAW*)GUI_LOCK_H(hMem);
    pObj->pConsts      = &_ConstObjData;
    pObj->Data.pData   = (const void*)pBitmap;
    pObj->xOff         = x;
    pObj->yOff         = y;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return hMem;
}


#else                            /* Avoid problems with empty object modules */
  void GUI_DRAW_STREAMED_C(void) {}
#endif
