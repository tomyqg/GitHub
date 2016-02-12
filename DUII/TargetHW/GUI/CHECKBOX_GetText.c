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
File        : CHECKBOX_GetText.c
Purpose     : Implementation of CHECKBOX_GetText
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "CHECKBOX_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CHECKBOX_GetText
*/
int CHECKBOX_GetText(CHECKBOX_Handle hObj, char * pBuffer, int MaxLen) {
  int Len = 0;
  if (hObj) {
    CHECKBOX_Obj  * pObj;
    WM_HMEM hpText;
    WM_LOCK();
    pObj = CHECKBOX_LOCK_H(hObj);
    hpText = pObj->hpText;
    GUI_UNLOCK_H(pObj);
    if (pObj->hpText) {
      const char * pText;
      pText = (const char *)GUI_LOCK_H(hpText);
      Len = strlen(pText);
      if (Len > (MaxLen - 1)) {
        Len = MaxLen - 1;
      }
      GUI_MEMCPY(pBuffer, pText, Len);
      GUI_UNLOCK_H(pText);
      *(pBuffer + Len) = 0;
    }
    WM_UNLOCK();
  }
  return Len;
}

#else  /* avoid empty object files */

void CHECKBOX_GetText_c(void);
void CHECKBOX_GetText_c(void) {}

#endif  /* #if GUI_WINSUPPORT */

/************************* end of file ******************************/
