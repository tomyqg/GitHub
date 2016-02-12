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
File        : FRAMEWIN_Get.c
Purpose     : 
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*        Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_GetFont
*/
const GUI_FONT GUI_UNI_PTR * FRAMEWIN_GetFont(FRAMEWIN_Handle hObj) {
  const GUI_FONT GUI_UNI_PTR * r = NULL;
  GUI_LOCK();
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    pObj = FRAMEWIN_LOCK_H(hObj);
    r = pObj->Props.pFont;
    GUI_UNLOCK_H(pObj);
    FRAMEWIN_Invalidate(hObj);
  }
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FRAMEWIN_GetTitleHeight
*/
int FRAMEWIN_GetTitleHeight(FRAMEWIN_Handle hObj) {
  int r = 0;
  FRAMEWIN_POSITIONS Pos;
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    WM_LOCK();
    FRAMEWIN__CalcPositions(hObj, &Pos);
    pObj = FRAMEWIN_LOCK_H(hObj);
    r = pObj->Props.TitleHeight;
    GUI_UNLOCK_H(pObj);
    if (r == 0) {
      r = Pos.TitleHeight;
    }
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       FRAMEWIN_GetBorderSize
*/
int FRAMEWIN_GetBorderSize(FRAMEWIN_Handle hObj) {
  int BorderSizeT;
  FRAMEWIN_Obj * pObj;
  FRAMEWIN_SKIN_PRIVATE * pSkinPrivate;

  BorderSizeT = 0;
  if (hObj) {
    WM_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    pSkinPrivate = (FRAMEWIN_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
    BorderSizeT = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_T);
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return BorderSizeT;
}

/*********************************************************************
*
*       FRAMEWIN_GetTextAlign
*/
int FRAMEWIN_GetTextAlign(FRAMEWIN_Handle hObj) {
  int r = 0;
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    WM_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    r = pObj->Props.TextAlign;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       FRAMEWIN_GetText
*/
void FRAMEWIN_GetText(FRAMEWIN_Handle hObj, char * pBuffer, int MaxLen) {
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    WM_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    if (pObj->hText) {
      int Len;
      const char * pText;
      pText = (const char *)GUI_LOCK_H(pObj->hText);
      Len = strlen(pText);
      if (Len > (MaxLen - 1)) {
        Len = MaxLen - 1;
      }
      GUI_MEMCPY((void *)pBuffer, (const void *)pText, Len);
      *(pBuffer + Len) = 0;
      GUI_UNLOCK_H(pText);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       FRAMEWIN_GetBarColor
*/
GUI_COLOR FRAMEWIN_GetBarColor(FRAMEWIN_Handle hObj, unsigned Index) {
  GUI_COLOR r = 0;
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    WM_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aBarColor)) {
      r = pObj->Props.aBarColor[Index];
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       FRAMEWIN_GetActive
*/
int FRAMEWIN_GetActive(FRAMEWIN_Handle hObj) {
  int r = 0;
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    WM_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    if (pObj->Flags & FRAMEWIN_CF_ACTIVE) {
      r = 1;
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

#else
  void FRAMEWIN_Get_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
