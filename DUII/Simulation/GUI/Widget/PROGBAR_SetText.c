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
File        : PROGBAR_SetText.c
Purpose     : Implementation of progbar widget
---------------------------END-OF-HEADER------------------------------
*/

#include "PROGBAR_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       PROGBAR_SetText
*/
void PROGBAR_SetText(PROGBAR_Handle hObj, const char * s) {
  if (hObj) {
    PROGBAR_Obj * pObj;
    const GUI_FONT GUI_UNI_PTR * pOldFont;
    GUI_RECT r1, r2;
    char * pText;
    WM_HMEM hpText;
    WM_LOCK();
    pObj = PROGBAR_LOCK_H(hObj);
    hpText = pObj->hpText;
    pOldFont = GUI_SetFont(pObj->Props.pFont);
    pText = PROGBAR__GetTextLocked(pObj);
    PROGBAR__GetTextRect(pObj, &r1, pText);
    GUI_UNLOCK_H(pText);
    if (!s) {
      pObj->Flags &= ~PROGBAR_SF_USER;
      WM_InvalidateRect(hObj, &r1);
      GUI_UNLOCK_H(pObj);
    } else {
      GUI_UNLOCK_H(pObj);
      if (GUI__SetText(&hpText, s)) {
        pObj = PROGBAR_LOCK_H(hObj);
        pObj->hpText = hpText;
        pObj->Flags |= PROGBAR_SF_USER;
        PROGBAR__GetTextRect(pObj, &r2, s);
        GUI_UNLOCK_H(pObj);
        GUI_MergeRect(&r1, &r1, &r2);
        WM_InvalidateRect(hObj, &r1);
      }
    }
    GUI_SetFont(pOldFont);
    WM_UNLOCK();
  }
}

#else  /* avoid empty object files */
  void PROGBAR_SetText_C(void);
  void PROGBAR_SetText_C(void) {}
#endif
