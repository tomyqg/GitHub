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
File        : EDITUlong.c
Purpose     : Edit unsigned long values
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "EDIT_Private.h"
#include "GUI_Debug.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define EDIT_ULONG_MAXLEN 10

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _IncrementCursor
*/
static void _IncrementCursor(EDIT_Handle hObj, EDIT_Obj * pObj) {
  EDIT__SetCursorPos(hObj, pObj->CursorPos + 1);
}

/*********************************************************************
*
*       _DecChar2Int
*/
static int _DecChar2Int(int Char) {
  if ((Char >= '0') && (Char <= '9')) {
    return Char - '0';
  }
  return -1;
}

/*********************************************************************
*
*       _UpdateBuffer
*/
static void _UpdateBuffer(EDIT_Handle hObj) {
  char acValue[EDIT_ULONG_MAXLEN] = {0};
  int NumDigits = 0;
  U32 Value;
  char * s;
  EDIT_Obj * pObj;
  pObj = EDIT_LOCK_H(hObj);
  s = (char *)GUI_LOCK_H(pObj->hpText);
  /* Convert an unsigned long value into text */
  Value = pObj->CurrentValue;
  /* Fill the buffer with the digits (reverse order) */
  while (Value) {
    U8 c = Value % 10;
    acValue[NumDigits++] = '0' + c;
    Value /= 10;
  }
  /* Fill with leading spaces */
  while (NumDigits < pObj->MaxLen) {
    acValue[NumDigits++] = '0';
  }
  /* Reverse order */
  while (NumDigits) {
    *(s++) = acValue[--NumDigits];
  }
  GUI_UNLOCK_H(pObj);
  GUI_UNLOCK_H(s);
}

/*********************************************************************
*
*       _EditULong
*/
static void _EditULong(int Digit, EDIT_Obj * pObj, EDIT_Handle hObj) {
  U32 Result = 0;
  int i, j, Pos = 0;
  char * s;
  s = (char *)GUI_LOCK_H(pObj->hpText);
  for (i = 0; i < pObj->MaxLen; i++) {
    int Index = pObj->MaxLen - i - 1;
    if (Index == pObj->CursorPos) {
      /* In this case we need to check if an overflow occures */
      char c;
      int Value;
      U32 OldResult;
      /* Calculate old value (until cursor position) */
      c         = *(s + Index);
      Value     = _DecChar2Int(c);
      OldResult = Result;
      if (Value >= 0) {
        OldResult += GUI_Pow10[Pos] * Value;
      }
      /* Calculate new value (until cursor position).
       * A multiplication like Result += GUI_Pow10[Pos++] * Digit
       * does not work, because in this case we have no chance to
       * notice an overflow.
       */
      for (j = 0; j < Digit; j++) {
        U32 NewResult;
        NewResult = Result + GUI_Pow10[Pos];
        /* Check if we have an overflow */
        if (NewResult < Result) {
          Result = pObj->Max;
          break;
        }
        Result = NewResult;
      }
      /* Check if we have an overflow */
      if (Digit > Value) {
        if (Result < OldResult) {
          Result = pObj->Max;
          break;
        }
      }
      Pos++;
    } else {
      /* If current index is not the cursor position */
      char c = *(s + Index);
      int Value = _DecChar2Int(c);
      if (Value >= 0) {
        U32 OldResult;
        OldResult = Result;
        Result += GUI_Pow10[Pos++] * Value;
        /* Check if we have an overflow */
        if (Result < OldResult) {
          Result = pObj->Max;
          break;
        }
      }
    }
  }
  GUI_UNLOCK_H(s);
  EDIT__SetValueUnsigned(hObj, Result);
}

/*********************************************************************
*
*       _AddPosition
*/
static void _AddPosition(EDIT_Obj * pObj, EDIT_Handle hObj, int Sign) {
  int Pos;
  I32 v, NewValue;
  v = Sign;
  Pos = pObj->MaxLen - pObj->CursorPos-1;
  if (pObj->NumDecs && (Pos > pObj->NumDecs)) {
    Pos--;
  }
  while (Pos--) {
    v *= 10;
  }
  NewValue = pObj->CurrentValue + v;
  /*
   * Check if overflow occurs
   */
  if (v > 0) {
    /* If result of addition is less than previous value he have an overflow */
    if (((U32)pObj->CurrentValue + v) < (U32)pObj->CurrentValue) {
      NewValue = pObj->Max; /* Set maximum value */
    }
  } else if (v < 0) {
    /* If result of addition is more than previous value he have an overflow */
    if (((U32)pObj->CurrentValue + v) > (U32)pObj->CurrentValue) {
      NewValue = pObj->Min; /* Set minimum value */
    }
  }
  EDIT__SetValueUnsigned(hObj, NewValue);
}

/*********************************************************************
*
*       _AddKey
*/
static void _AddKey(EDIT_Handle hObj, int Key) {
  EDIT_Obj * pObj;
  pObj = EDIT_LOCK_H(hObj);
  if (pObj) {
    switch (Key) {
    case GUI_KEY_UP:
      _AddPosition(pObj, hObj, 1);
      break;
    case GUI_KEY_DOWN:
      _AddPosition(pObj, hObj, -1);
      break;
    case GUI_KEY_RIGHT:
      _IncrementCursor(hObj, pObj);
      break;
    case GUI_KEY_LEFT:
      EDIT__SetCursorPos(hObj, pObj->CursorPos - 1);
      break;
    default:
      {
        int Digit = _DecChar2Int(Key);
        if (Digit >= 0) {
          _EditULong(Digit, pObj, hObj);
          _IncrementCursor(hObj, pObj);
        }
      }
      break;
    }
  }
  GUI_UNLOCK_H(pObj);
  _UpdateBuffer(hObj);
}

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_SetUlongMode
*/
void EDIT_SetUlongMode(EDIT_Handle hEdit, U32 Value, U32 Min, U32 Max) {
  EDIT_Obj * pObj;
  WM_LOCK();
  if (hEdit) {
    pObj = EDIT_LOCK_H(hEdit);
    if (Value > Max) {
      Value = Max;
    } else if (Value < Min) {
      Value = Min;
    }
    pObj->pfAddKeyEx    = _AddKey;
    pObj->pfUpdateBuffer= _UpdateBuffer;
    pObj->CurrentValue  = Value;
    pObj->CursorPos     = 0;
    pObj->Min           = Min;
    pObj->Max           = Max;
    pObj->EditMode      = GUI_EDIT_MODE_OVERWRITE;
    if (pObj->MaxLen > EDIT_ULONG_MAXLEN) {
      pObj->MaxLen = EDIT_ULONG_MAXLEN;
    }
    GUI_UNLOCK_H(pObj);
    _UpdateBuffer(hEdit);
    WM_Invalidate(hEdit);
  }
  WM_UNLOCK();
}

#else  /* avoid empty object files */

void EDITUlong_C(void);
void EDITUlong_C(void){}

#endif /* GUI_WINSUPPORT */
