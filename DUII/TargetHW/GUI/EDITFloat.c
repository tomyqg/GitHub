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
File        : EDITFloat.c
Purpose     : Edit decimal values
---------------------------END-OF-HEADER------------------------------
*/

#include <math.h>

#include "EDIT_Private.h"
#include "GUI_Debug.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*             Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_SetFloatMode
*/
void EDIT_SetFloatMode(EDIT_Handle hEdit, float Value, float Min, float Max, int Shift, U8 Flags) {
  I32 _Value, _Min, _Max;
  float Scale;
  if (hEdit) {
    WM_LOCK();
    Scale  =(float)GUI_Pow10[Shift];
    _Value = (I32)floor(Scale * Value + 0.5);
    _Min   = (I32)floor(Scale * Min + 0.5);
    _Max   = (I32)floor(Scale * Max + 0.5);
    EDIT_SetDecMode(hEdit, _Value, _Min, _Max, Shift, Flags);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       EDIT_GetFloatValue
*/
float EDIT_GetFloatValue(EDIT_Handle hObj) {
  float Value = 0;
  if (hObj) {
    float Scale;
    EDIT_Obj * pObj;
    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    Scale = (float)GUI_Pow10[pObj->NumDecs];
    Value = (float)(I32)pObj->CurrentValue / Scale;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Value;
}

/*********************************************************************
*
*       EDIT_SetFloatValue
*/
void EDIT_SetFloatValue(EDIT_Handle hObj, float Value) {
  if (hObj) {
    float Scale;
    EDIT_Obj * pObj;
    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    Scale = (float)GUI_Pow10[pObj->NumDecs];
    Value *= Scale;
    GUI_UNLOCK_H(pObj);
    EDIT_SetValue(hObj, (I32)(Value + (Value >= 0 ? 0.5 : -0.5)));
    WM_UNLOCK();
  }
}

#else  /* avoid empty object files */

void EditFloat_C(void);
void EditFloat_C(void){}

#endif /* GUI_WINSUPPORT */
