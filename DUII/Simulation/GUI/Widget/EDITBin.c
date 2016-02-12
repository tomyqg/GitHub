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
File        : EDITBin.c
Purpose     : Support for binary editing for widgets
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "EDIT_Private.h"
#include "GUI_Debug.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*             Helpers
*
**********************************************************************
*/
/*********************************************************************
*
*       _BinChar2Int
*/
static int _BinChar2Int(int Char) {
  if ((Char >= '0') && (Char <= '1'))
    return Char - '0';
  return -1;
}

/*********************************************************************
*
*       _GetNumDigits
*/
static int _GetNumDigits(U32 Value) {
  int Ret;
  for (Ret = 0; Value; Value >>= 1, Ret++);
  return Ret;
}

/*********************************************************************
*
*       _UpdateBuffer
*/
static void _UpdateBuffer(EDIT_Handle hObj) {
  char * s;
  EDIT_Obj * pObj;
  pObj = EDIT_LOCK_H(hObj);
  s = (char *)GUI_LOCK_H(pObj->hpText);
  GUI_AddBin(pObj->CurrentValue, pObj->MaxLen, &s);
  GUI_UNLOCK_H(pObj);
  GUI_UNLOCK_H(s);
}

/*********************************************************************
*
*       _EditBin
*/
static void _EditBin(U8 Bit, EDIT_Obj * pObj, EDIT_Handle hObj) {
  int Pos = pObj->MaxLen - pObj->CursorPos - 1;   /* Bit position */
  U32 AndMask = ~(1   << Pos);
  U32 OrMask  =   Bit << Pos;
  I32 Result  = pObj->CurrentValue & AndMask;
  Result     |= OrMask;
  EDIT_SetValue(hObj, Result);
}

/*********************************************************************
*
*       _GetCurrentBit
*/
static U8 _GetCurrentBit(EDIT_Obj * pObj) {
  int Pos = pObj->MaxLen - pObj->CursorPos - 1;   /* Bit position */
  U32 AndMask = 1 << Pos;
  U8 Bit = (pObj->CurrentValue & AndMask) >> Pos;
  return Bit;
}

/*********************************************************************
*
*             Handle input
*
**********************************************************************
*/
/*********************************************************************
*
*       _AddKeyBin
*/
static void _AddKeyBin(EDIT_Handle hObj, int Key) {
  EDIT_Obj * pObj;
  pObj = EDIT_LOCK_H(hObj);
  if (pObj) {
    switch (Key) {
    case GUI_KEY_UP:
      {
        int Bit = _GetCurrentBit(pObj) + 1;
        if (Bit > 1) {
          Bit = 0;
        }
        _EditBin(Bit, pObj, hObj);
      }
      break;
    case GUI_KEY_DOWN:
      {
        int Bit = _GetCurrentBit(pObj) - 1;
        if (Bit < 0) {
          Bit = 1;
        }
        _EditBin(Bit, pObj, hObj);
      }
      break;
    case GUI_KEY_RIGHT:
      EDIT__SetCursorPos(hObj, pObj->CursorPos + 1);
      break;
    case GUI_KEY_LEFT:
      EDIT__SetCursorPos(hObj, pObj->CursorPos - 1);
      break;
    default:
      {
        int Bit = _BinChar2Int(Key);
        if (Bit >= 0) {
          _EditBin(Bit, pObj, hObj);
          EDIT__SetCursorPos(hObj, pObj->CursorPos + 1);
        }
      }
      break;
    }
    _UpdateBuffer(hObj);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*             Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_SetBinMode
*/
void EDIT_SetBinMode(EDIT_Handle hEdit, U32 Value, U32 Min, U32 Max) {
  EDIT_Obj * pObj;
  int MaxLen, MaxLenOld;
  WM_LOCK();
  if (hEdit) {
    pObj = EDIT_LOCK_H(hEdit);
    pObj->pfAddKeyEx    = _AddKeyBin;
    pObj->pfUpdateBuffer = _UpdateBuffer;
    pObj->CurrentValue = Value;
    pObj->CursorPos = 0;
    MaxLen = pObj->MaxLen;
    if (MaxLen <= 0 ) {
      MaxLen = _GetNumDigits(Max);
    }
    if (MaxLen > 32) {
      MaxLen = 32;
    }
    pObj->Min = Min;
    pObj->Max = Max;
    pObj->EditMode = GUI_EDIT_MODE_OVERWRITE;
    MaxLenOld = pObj->MaxLen;
    GUI_UNLOCK_H(pObj);
    if (MaxLen != MaxLenOld) {
      EDIT_SetMaxLen(hEdit, MaxLen);
    }
    _UpdateBuffer(hEdit);
    WM_Invalidate(hEdit);
  }
  WM_UNLOCK();
}

#else  /* avoid empty object files */

void EditBin_C(void);
void EditBin_C(void){}

#endif /* GUI_WINSUPPORT */
