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
File        : RADIO_SetImage.c
Purpose     : Implementation of RADIO widget
---------------------------END-OF-HEADER------------------------------
*/

#include "RADIO.h"
#include "RADIO_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported code
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_SetImage
*/
void RADIO_SetImage(RADIO_Handle hObj, const GUI_BITMAP * pBitmap, unsigned int Index) {
  if (hObj) {
    RADIO_Obj * pObj;
    GUI_LOCK();
    pObj = RADIO_LOCK_H(hObj);
    switch (Index) {
    case RADIO_BI_INACTIV:
    case RADIO_BI_ACTIV:
      pObj->Props.apBmRadio[Index] = pBitmap;
      break;
    case RADIO_BI_CHECK:
      pObj->Props.pBmCheck = pBitmap;
      break;
    }
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    GUI_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void RADIO_SetImage_C(void);
  void RADIO_SetImage_C(void) {}
#endif

/************************* end of file ******************************/
