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
File        : EDITDec.c
Purpose     : Edit decimal values
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "EDIT_Private.h"
#include "GUI_Debug.h"

#if GUI_WINSUPPORT


/*********************************************************************
*
*        Defaults for config switches
*
**********************************************************************
*/
#ifndef EDIT_DEC_DIGITONLY
  #define EDIT_DEC_DIGITONLY     0
#endif


/*********************************************************************
*
*        static Helpers
*
**********************************************************************
*/
/*********************************************************************
*
*       _DecChar2Int
*/
static int _DecChar2Int(int Char) {
  if ((Char >= '0') && (Char <= '9'))
    return Char - '0';
  return -1;
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
  if (pObj->Flags & GUI_EDIT_SIGNED) {
    if (((I32)pObj->CurrentValue) >= 0) {
      I32 Result = GUI_AddSign(pObj->CurrentValue, &s);
      GUI_AddDecShift(Result, pObj->MaxLen - 1, pObj->NumDecs, &s);
    } else {
      GUI_AddDecShift(pObj->CurrentValue, pObj->MaxLen, pObj->NumDecs, &s);
    }
  } else {
    int Len;
    Len = pObj->MaxLen;
    if (Len > 10) {
      if ((I32)pObj->CurrentValue >= 0) {
        Len = 10;
      }
    }
    GUI_AddDecShift(pObj->CurrentValue, Len, pObj->NumDecs, &s);
  }
  GUI_UNLOCK_H(s);
  if (pObj->Flags & GUI_EDIT_SUPPRESS_LEADING_ZEROES) {
    int i, LastIndex;
    LastIndex = pObj->MaxLen - 2;
    if (pObj->NumDecs) {
      LastIndex -= pObj->NumDecs + 1;
    }
    s = (char *)GUI_LOCK_H(pObj->hpText);
    for (i = 0; i <= LastIndex; i++, s++) {
      if (i == 0) {
        if ((*s == '+') || (*s == '-')) {
          continue;
        }
      }
      if (*s != '0') {
        break;
      }
      *s = ' ';
    }
    GUI_UNLOCK_H(s);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _EditDec
*/
static void _EditDec(int Digit, EDIT_Obj * pObj, EDIT_Handle hObj) {
  I32 Result = 0;
  int i, Pos = 0;
  char * s;
  s = (char *)GUI_LOCK_H(pObj->hpText);
  for (i = 0; i < pObj->MaxLen; i++) {
    int Index = pObj->MaxLen - i - 1;
    if (Index == pObj->CursorPos) {
      Result += GUI_Pow10[Pos++] * Digit;
    } else {
      char c;
      int Value;
      c = *(s + Index);
      if (pObj->Flags & GUI_EDIT_SUPPRESS_LEADING_ZEROES) {
        if (c == ' ') {
          c = '0';
        }
      }
      Value = _DecChar2Int(c);
      if (Value >= 0) {
        Result += GUI_Pow10[Pos++] * Value;
      }
      if (c == '-') {
        Result *= -1;
      }
    }
  }
  GUI_UNLOCK_H(s);
  EDIT_SetValue(hObj, Result);
}

/*********************************************************************
*
*       EDIT_DEC_DIGITONLY
*/
#if EDIT_DEC_DIGITONLY
static int _GetCurrentDigit(EDIT_Obj * pObj) {
  return _DecChar2Int(EDIT__GetCurrentChar(pObj));
}
#endif

/*********************************************************************
*
*       _MakePositive
*/
static void _MakePositive(EDIT_Obj * pObj, EDIT_Handle hObj) {
  if ((I32)pObj->CurrentValue < 0) {
    EDIT_SetValue(hObj, (I32)pObj->CurrentValue * -1);
  }
}

/*********************************************************************
*
*       _MakeNegative
*/
static void _MakeNegative(EDIT_Obj * pObj, EDIT_Handle hObj) {
  if ((I32)pObj->CurrentValue > 0) {
    EDIT_SetValue(hObj, (I32)pObj->CurrentValue * -1);
  }
}

/*********************************************************************
*
*       _SwapSign
*/
static void _SwapSign(EDIT_Obj * pObj, EDIT_Handle hObj) {
  if ((I32)pObj->CurrentValue > 0)
    _MakeNegative(pObj, hObj);
  else
    _MakePositive(pObj, hObj);
}

/*********************************************************************
*
*       _IncrementCursor
*/
static void _IncrementCursor(EDIT_Handle hObj, EDIT_Obj * pObj) {
  EDIT__SetCursorPos(hObj, pObj->CursorPos + 1);
  if (EDIT__GetCurrentChar(pObj) == '.') {
    if (pObj->CursorPos < (pObj->MaxLen - 1)) {
      EDIT__SetCursorPos(hObj, pObj->CursorPos + 1);
    } else {
      EDIT__SetCursorPos(hObj, pObj->CursorPos - 1);
    }
  }
}

/*********************************************************************
*
*       _AddPosition
*/
#if !EDIT_DEC_DIGITONLY
static void _AddPosition(EDIT_Obj * pObj, EDIT_Handle hObj, int Sign) {
  int Pos;
  I32 v;
  I32 OldValue, NewValue;
  OldValue = pObj->CurrentValue;
  v = Sign;
  Pos = pObj->MaxLen - pObj->CursorPos-1;
  if ((pObj->Flags && GUI_EDIT_SIGNED) && (OldValue == 0) && (Pos > 0)) {
    Pos--;
  }
  if (pObj->NumDecs && (Pos > pObj->NumDecs)) {
    Pos--;
  }
  while (Pos--) {
    v *= 10;
  }
  NewValue = pObj->CurrentValue + v;
  if (((Sign < 0) && (NewValue < OldValue)) || ((Sign > 0) && (NewValue > OldValue))) {
    EDIT_SetValue(hObj, NewValue);
  }
  //
  // In case of old value == 0 and new value != 0 the cursor position needs to be adjusted
  //
  if (pObj->Flags && GUI_EDIT_SIGNED) {
    if ((OldValue == 0) && (NewValue != 0) && (pObj->CursorPos < (pObj->MaxLen - 1))) {
      pObj->CursorPos++;
    }
    if ((OldValue != 0) && (NewValue == 0) && (pObj->CursorPos > 0)) {
      pObj->CursorPos--;
    }
  }
}
#endif

/*********************************************************************
*
*             Handle input
*
**********************************************************************
*/
/*********************************************************************
*
*       _AddKeyDec
*/
static void _AddKeyDec(EDIT_Handle hObj, int Key) {
  char c;
  EDIT_Obj * pObj;
  pObj = EDIT_LOCK_H(hObj);
  if (pObj) {
    switch (Key) {
    case '+':
      if (pObj->CursorPos == 0) {
        _MakePositive(pObj, hObj);
        _IncrementCursor(hObj, pObj);
      }
      break;
    case '-':
      if (pObj->CursorPos == 0) {
        _MakeNegative(pObj, hObj);
        _IncrementCursor(hObj, pObj);
      }
      break;
    #if EDIT_DEC_DIGITONLY
      case GUI_KEY_UP:
        c = EDIT__GetCurrentChar(pObj);
        if ((c == '-') || (c == '+')) {
          _SwapSign(pObj, hObj);
        } else {
          int Digit = _GetCurrentDigit(pObj) + 1;
          if (Digit > 9)
            Digit = 0;
          _EditDec(Digit, pObj, hObj);
        }
        break;
      case GUI_KEY_DOWN:
        c = EDIT__GetCurrentChar(pObj);
        if ((c == '-') || (c == '+')) {
          _SwapSign(pObj, hObj);
        } else {
          int Digit = _GetCurrentDigit(pObj) - 1;
          if (Digit < 0)
            Digit = 9;
          _EditDec(Digit, pObj, hObj);
        }
        break;
    #else
      case GUI_KEY_UP:
        c = EDIT__GetCurrentChar(pObj);
        if ((c == '-') || (c == '+')) {
          _SwapSign(pObj, hObj);
        } else {
          _AddPosition(pObj, hObj, 1);
        }
        break;
      case GUI_KEY_DOWN:
        c = EDIT__GetCurrentChar(pObj);
        if ((c == '-') || (c == '+')) {
          _SwapSign(pObj, hObj);
        } else {
          _AddPosition(pObj, hObj, -1);
        }
        break;
    #endif
    case GUI_KEY_RIGHT:
      _IncrementCursor(hObj, pObj);
      break;
    case GUI_KEY_LEFT:
      EDIT__SetCursorPos(hObj, pObj->CursorPos - 1);
      if (EDIT__GetCurrentChar(pObj) == '.') {
        if (pObj->CursorPos > 0) {
          EDIT__SetCursorPos(hObj, pObj->CursorPos - 1);
        } else {
          EDIT__SetCursorPos(hObj, pObj->CursorPos + 1);
        }
      }
      break;
    default:
      {
        c = EDIT__GetCurrentChar(pObj);
        if ((c != '-') && (c != '+')) {
          int Digit = _DecChar2Int(Key);
          if (Digit >= 0) {
            _EditDec(Digit, pObj, hObj);
            _IncrementCursor(hObj, pObj);
          }
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
*             Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_SetDecMode
*/
void EDIT_SetDecMode(EDIT_Handle hEdit, I32 Value, I32 Min, I32 Max, int Shift, U8 Flags) {
  EDIT_Obj * pObj;
  WM_LOCK();
  if (hEdit) {
    pObj = EDIT_LOCK_H(hEdit);
    pObj->pfAddKeyEx    = _AddKeyDec;
    pObj->pfUpdateBuffer= _UpdateBuffer;
    pObj->CurrentValue  = Value;
    pObj->CursorPos     = 0;
    pObj->Min           = Min;
    pObj->Max           = Max;
    pObj->NumDecs       = Shift;
    pObj->Flags         = Flags;
    pObj->EditMode      = GUI_EDIT_MODE_OVERWRITE;
    _UpdateBuffer(hEdit);
    if (EDIT__GetCurrentChar(pObj) == '.') {
      EDIT__SetCursorPos(hEdit, pObj->CursorPos + 1);
    }
    GUI_UNLOCK_H(pObj);
    WM_Invalidate(hEdit);
  }
  WM_UNLOCK();
}

#else  /* avoid empty object files */

void EditDec_C(void);
void EditDec_C(void){}

#endif /* GUI_WINSUPPORT */
