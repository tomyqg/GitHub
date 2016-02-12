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
File        : LISTBOX_GetItemText.c
Purpose     : Implementation of said function
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTBOX_Private.h"
#include <string.h>

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines:  Various methods
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTBOX_GetItemText
*/
void LISTBOX_GetItemText(LISTBOX_Handle hObj, unsigned Index, char * pBuffer, int MaxSize) {
  if (hObj) {
    unsigned NumItems;
    WM_LOCK();
    NumItems = LISTBOX_GetNumItems(hObj);
    if (Index < NumItems) {
      const char * pString;
      int CopyLen;
      pString = LISTBOX__GetpStringLocked(hObj, Index);
      CopyLen = strlen(pString);
      if (CopyLen > (MaxSize - 1)) {
        CopyLen = MaxSize - 1;
      }
      GUI_MEMCPY(pBuffer, pString, CopyLen);
      GUI_UNLOCK_H(pString);
      pBuffer[CopyLen] = 0;
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_GetItemText_C(void) {}
#endif

/*************************** End of file ****************************/

