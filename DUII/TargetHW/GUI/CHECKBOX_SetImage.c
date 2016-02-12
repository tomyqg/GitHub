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
File        : CHECKBOX_SetImage.c
Purpose     : Implementation of checkbox widget
---------------------------END-OF-HEADER------------------------------
*/

#include "CHECKBOX_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported code
*
**********************************************************************
*/
void CHECKBOX_SetImage(CHECKBOX_Handle hObj, const GUI_BITMAP * pBitmap, unsigned int Index) {
  if (hObj) {
    CHECKBOX_Obj * pObj;
    GUI_LOCK();
    pObj = CHECKBOX_LOCK_H(hObj);
    if (Index <= GUI_COUNTOF(pObj->Props.apBm)) {
      pObj->Props.apBm[Index] = pBitmap;
    }
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void CHECKBOX_SetImage_C(void);
  void CHECKBOX_SetImage_C(void) {}
#endif
