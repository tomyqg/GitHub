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
File        : BUTTON_Get.c
Purpose     : Button widget, various (optional) Get routines
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "BUTTON.h"
#include "BUTTON_Private.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_GetFont     
*/
const GUI_FONT GUI_UNI_PTR * BUTTON_GetFont(BUTTON_Handle hObj) {
  const GUI_FONT GUI_UNI_PTR * pFont = 0;
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    pFont = pObj->Props.pFont;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return pFont;
}

/*********************************************************************
*
*       BUTTON_GetText  
*/
void BUTTON_GetText(BUTTON_Handle hObj, char * pBuffer, int MaxLen) {
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    if (pObj->hpText) {
      const char * pText = (const char *)GUI_LOCK_H(pObj->hpText);
      int Len = strlen(pText);
      if (Len > (MaxLen - 1)) {
        Len = MaxLen - 1;
      }
      GUI_MEMCPY((void *)pBuffer, (const void *)pText, Len);
      GUI_UNLOCK_H(pText);
      *(pBuffer + Len) = 0;
    } else {
      *pBuffer = 0;     /* Empty string */
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON_Get_C(void) {}
#endif
