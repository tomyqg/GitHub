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
File        : BUTTON_GetBitmap.c
Purpose     : Implementation of BUTTON_GetBitmap()
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>
#include "BUTTON_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_GetBitmap
*/
const GUI_BITMAP GUI_UNI_PTR * BUTTON_GetBitmap(BUTTON_Handle hObj,unsigned int Index) {
  const GUI_BITMAP GUI_UNI_PTR * pBM;
  pBM = NULL;
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    if (Index <= GUI_COUNTOF(pObj->ahDrawObj)) {
      WM_HMEM hDrawObj;
      hDrawObj = pObj->ahDrawObj[Index];
      if (hDrawObj) {
        GUI_DRAW * pDrawObj;
        pDrawObj = (GUI_DRAW *)GUI_LOCK_H(hDrawObj);
        pBM = (const GUI_BITMAP GUI_UNI_PTR *)pDrawObj->Data.pData;
        GUI_UNLOCK_H(pDrawObj);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return pBM;
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON_GetBitmap_C(void);
  void BUTTON_GetBitmap_C(void) {}
#endif
