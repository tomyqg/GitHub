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
File        : GUI_EditBin.c
Purpose     : Widget, add. module
----------------------------------------------------------------------
*/

#include "EDIT.h"
#include "GUI_Private.h"
#include "EDIT_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*             Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_EditBin
*/
U32 GUI_EditBin(U32 Value, U32 Min, U32 Max, int Len, int xsize) {
  U32 Ret = Value;
  int Key, x, y, ysize, Id;
  EDIT_Handle hEdit;
  EDIT_Obj* pObj;
  const GUI_FONT GUI_UNI_PTR * pOldFont;
  WM_LOCK();
  pOldFont = GUI_SetFont(EDIT_GetDefaultFont());
  x = GUI_GetDispPosX();
  y = GUI_GetDispPosY();
  if (xsize == 0)
    xsize = GUI_GetCharDistX('X') * Len + 6;
  ysize = GUI_GetFontSizeY();
  Id = 0x1234;
  hEdit = EDIT_Create(x, y, xsize, ysize, Id, Len, 0);
  EDIT_SetBinMode(hEdit, Value, Min, Max);
  WM_SetFocus(hEdit);
  do {
    Key = GUI_WaitKey();
  } while ((Key != GUI_KEY_ESCAPE) && (Key != GUI_KEY_ENTER) && (Key != 0));
  GUI_SetFont(pOldFont);
  pObj = EDIT_LOCK_H(hEdit);
  if (Key == GUI_KEY_ENTER) {
    Ret = pObj->CurrentValue;
  }
  GUI_UNLOCK_H(pObj);
  EDIT_Delete(hEdit);
  WM_UNLOCK();
  return Ret;
}

#else /* avoid empty object files */

void GUI_EditBin_C(void);
void GUI_EditBin_C(void){}

#endif
