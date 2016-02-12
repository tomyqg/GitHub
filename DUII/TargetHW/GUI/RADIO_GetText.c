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
File        : RADIO_GetText.c
Purpose     : Implementation of RADIO_GetText
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "RADIO_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_GetText
*/
int RADIO_GetText(RADIO_Handle hObj, unsigned Index, char * pBuffer, int MaxLen) {
  int Len = 0;
  if (hObj) {
    const char * pText;
    RADIO_Obj  * pObj;
    WM_LOCK();
    pObj = RADIO_LOCK_H(hObj);
    if (Index < (unsigned)pObj->NumItems) {
      pText = (const char *)GUI_ARRAY_GetpItemLocked(pObj->TextArray, Index);
      if (pText) {
        Len = strlen(pText);
        if (Len > (MaxLen - 1)) {
          Len = MaxLen - 1;
        }
        GUI_MEMCPY(pBuffer, pText, Len);
        *(pBuffer + Len) = 0;
        GUI_UNLOCK_H(pText);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Len;
}

#else  /* avoid empty object files */

void RADIO_GetText_c(void);
void RADIO_GetText_c(void) {}

#endif  /* #if GUI_WINSUPPORT */

/************************* end of file ******************************/
