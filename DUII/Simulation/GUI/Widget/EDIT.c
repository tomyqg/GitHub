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
File        : EDIT.c
Purpose     : Implementation of edit widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#define EDIT_C       /* Required to generate intermodule data */

#include "EDIT.h"
#include "GUI_Debug.h"
#include "GUI_Private.h"
#include "EDIT_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

/* Define default fonts */
#ifndef EDIT_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define EDIT_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define EDIT_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define EDIT_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif

#ifndef EDIT_ALIGN_DEFAULT
  #define EDIT_ALIGN_DEFAULT GUI_TA_LEFT | GUI_TA_VCENTER
#endif

/* Define colors */
#ifndef EDIT_BKCOLOR0_DEFAULT
  #define EDIT_BKCOLOR0_DEFAULT 0xC0C0C0
#endif

#ifndef EDIT_BKCOLOR1_DEFAULT
  #define EDIT_BKCOLOR1_DEFAULT GUI_WHITE
#endif

#ifndef EDIT_TEXTCOLOR0_DEFAULT
  #define EDIT_TEXTCOLOR0_DEFAULT GUI_BLACK
#endif

#ifndef EDIT_TEXTCOLOR1_DEFAULT
  #define EDIT_TEXTCOLOR1_DEFAULT GUI_BLACK
#endif

#ifndef EDIT_BORDER_DEFAULT
  #define EDIT_BORDER_DEFAULT 1
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
EDIT_PROPS EDIT__DefaultProps = {
  EDIT_ALIGN_DEFAULT,
  EDIT_BORDER_DEFAULT,
  EDIT_FONT_DEFAULT,
  {
    EDIT_TEXTCOLOR0_DEFAULT,
    EDIT_TEXTCOLOR1_DEFAULT,
  },
  {
    EDIT_BKCOLOR0_DEFAULT,
    EDIT_BKCOLOR1_DEFAULT
  }
};

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _CalcSizes
*/
static void _CalcSizes(EDIT_Handle hObj, EDIT_Obj * pObj, GUI_RECT * pFillRect, GUI_RECT * pInsideRect, GUI_RECT * pTextRect, GUI_RECT * pInvertRect, int * pCursorWidth, int ManageSelection) {
  const char GUI_UNI_PTR * pText = NULL;
  int IsEnabled;
  IsEnabled = WM__IsEnabled(hObj);
  /* Set colors and font */
  LCD_SetBkColor(pObj->Props.aBkColor[IsEnabled]);
  LCD_SetColor(pObj->Props.aTextColor[IsEnabled]);
  GUI_SetFont(pObj->Props.pFont);
  /* Calculate size */
  WIDGET__GetInsideRect(&pObj->Widget, pFillRect);
  if (pObj->hpText) {
    pText = (const char *)GUI_LOCK_H(pObj->hpText);
  }
  *pInsideRect = *pFillRect;
  pInsideRect->x0 += pObj->Props.Border + EDIT_XOFF;
  pInsideRect->x1 -= pObj->Props.Border + EDIT_XOFF;
  GUI__CalcTextRect(pText, pInsideRect, pTextRect, pObj->Props.Align);
  /* Calculate position and size of cursor */
  if (pObj->Widget.State & WIDGET_STATE_FOCUS) {
    int NumChars;
    *pCursorWidth = ((pObj->XSizeCursor > 0) ? (pObj->XSizeCursor) : (1));
    NumChars    = GUI__GetNumChars(pText);
    if (pText) {
      U16 Char;
      int i, IsRTL = 0;
      if ((pObj->EditMode != GUI_EDIT_MODE_INSERT) || ((pObj->SelSize) && ManageSelection)) {
        if (pObj->CursorPos < NumChars) {
          if ((pObj->SelSize) && (ManageSelection)) {
            *pCursorWidth = 0;
            for (i = pObj->CursorPos; i < (int)(pObj->CursorPos + pObj->SelSize); i++) {
              Char = GUI__GetCursorCharacter(pText, i, NumChars, 0);
              *pCursorWidth += GUI_GetCharDistX(Char);
            }
            if (!*pCursorWidth) {
              *pCursorWidth = 1;
            }
          } else {
            Char = GUI__GetCursorCharacter(pText, pObj->CursorPos, NumChars, &IsRTL);
            *pCursorWidth = GUI_GetCharDistX(Char);
          }
        }
      } else {
        if (GUI__BIDI_Enabled == 1) {
          IsRTL = _pfGUI__BIDI_GetCharDir(pText, NumChars, pObj->CursorPos);
        }
      }
      *pInvertRect = *pTextRect;
      if (IsRTL) {
        pInvertRect->x0 -= *pCursorWidth;
      }
      pInvertRect->x0 += GUI__GetCursorPosX(pText, pObj->CursorPos, NumChars);
    }
  }
  if (pText) {
    GUI_UNLOCK_H(pText);
  }
}

/*********************************************************************
*
*       _Paint
*/
static void _Paint(EDIT_Handle hObj, int Complete) {
  EDIT_Obj * pObj;
  int CursorWidth;
  const char GUI_UNI_PTR * pText = NULL;
  GUI_RECT rFillRect, rInside, rText, rInvert;

  pObj = EDIT_LOCK_H(hObj);
  _CalcSizes(hObj, pObj, &rFillRect, &rInside, &rText, &rInvert, &CursorWidth, Complete);
  if (pObj->hpText) {
    pText = (const char *)GUI_LOCK_H(pObj->hpText);
  }
  /* WM loop */
  WM_ITERATE_START(NULL) {
    /* Set clipping rectangle */
    WM_SetUserClipRect(&rFillRect);
    if (Complete) {
      /* Display text */
      WIDGET__FillStringInRect(pText, &rFillRect, &rInside, &rText);
    }
    /* Display cursor if needed */
    if (pObj->Widget.State & WIDGET_STATE_FOCUS) {
      GUI_InvertRect(rInvert.x0, rInvert.y0, rInvert.x0 + CursorWidth - 1, rInvert.y1);
    }
    WM_SetUserClipRect(NULL);
    /* Draw the 3D effect (if configured) */
    WIDGET__EFFECT_DrawDown(&pObj->Widget);
  } WM_ITERATE_END();
  if (Complete) {
    WM_RestartTimer(pObj->hTimer, 0);
  }
  GUI_UNLOCK_H(pObj);
  if (pText) {
    GUI_UNLOCK_H(pText);
  }
}

/*********************************************************************
*
*       _InvertCursor
*/
static void _InvertCursor(EDIT_Handle hObj) {
  _Paint(hObj, 0);
}

/*********************************************************************
*
*       _Delete
*/
static void _Delete(EDIT_Handle hObj) {
  EDIT_Obj * pObj;
  pObj = EDIT_LOCK_H(hObj);
  GUI_ALLOC_FreePtr(&pObj->hpText);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _IncrementBuffer
*
* Increments the buffer size by AddBytes.
*/
static int _IncrementBuffer(EDIT_Handle hObj, unsigned AddBytes) {
  EDIT_Obj * pObj;
  WM_HMEM hNew;
  WM_HMEM hpText;
  int NewSize;
  pObj = EDIT_LOCK_H(hObj);
  hpText = pObj->hpText;
  NewSize = pObj->BufferSize + AddBytes;
  GUI_UNLOCK_H(pObj);
  hNew = GUI_ALLOC_Realloc(hpText, NewSize);
  if (hNew) {
    if (!(hpText)) {
      char * pText;
      pText  = (char *)GUI_LOCK_H(hNew);
      *pText = 0;
      GUI_UNLOCK_H(pText);
    }
    pObj = EDIT_LOCK_H(hObj);
    pObj->BufferSize = NewSize;
    pObj->hpText     = hNew;
    GUI_UNLOCK_H(pObj);
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       _IsSpaceInBuffer
*
* Checks the available space in the buffer. If there is not enough
* space left this function attempts to get more.
*
* Returns:
*  1 = requested space is available
*  0 = failed to get enough space
*/
static int _IsSpaceInBuffer(EDIT_Handle hObj, int BytesNeeded) {
  int NumBytes = 0;
  EDIT_Obj * pObj;
  pObj = EDIT_LOCK_H(hObj);
  if (pObj->hpText) {
    char * pText;
    pText = (char *)GUI_LOCK_H(pObj->hpText);
    NumBytes = strlen(pText);
    GUI_UNLOCK_H(pText);
  }
  BytesNeeded = (BytesNeeded + NumBytes + 1) - pObj->BufferSize;
  GUI_UNLOCK_H(pObj);
  if (BytesNeeded > 0) {
    if (!_IncrementBuffer(hObj, BytesNeeded + EDIT_REALLOC_SIZE)) {
      return 0;
    }
  }
  return 1;
}

/*********************************************************************
*
*       _CharsAreAvailable
*
* Checks weither the maximum number of characters is reached or not.
*
* Returns:
*  1 = requested number of chars are available
*  0 = maximum number of chars have reached
*/
static int _CharsAreAvailable(EDIT_Obj * pObj, int CharsNeeded) {
  if ((CharsNeeded > 0) && (pObj->MaxLen > 0)) {
    int NumChars = 0;
    if (pObj->hpText) {
      char * pText;
      pText = (char *)GUI_LOCK_H(pObj->hpText);
      NumChars = GUI__GetNumChars(pText);
      GUI_UNLOCK_H(pText);
    }
    if ((CharsNeeded + NumChars) > pObj->MaxLen) {
      return 0;
    }
  }
  return 1;
}

/*********************************************************************
*
*       _DeleteChar
*
* Deletes a character at the current cursor position and moves
* all bytes after the cursor position.
*/
static void _DeleteChar(EDIT_Handle hObj) {
  EDIT_Obj * pObj;
  WM_HMEM hpText;
  int CursorPos;
  pObj = EDIT_LOCK_H(hObj);
  hpText = pObj->hpText;
  CursorPos = pObj->CursorPos;
  GUI_UNLOCK_H(pObj);
  if (hpText) {
    unsigned CursorOffset;
    char * pText;
    pText = (char *)GUI_LOCK_H(hpText);
    CursorOffset = GUI_UC__NumChars2NumBytes(pText, CursorPos);
    if (CursorOffset < strlen(pText)) {
      int NumBytes;
      pText += CursorOffset;
      NumBytes = GUI_UC_GetCharSize(pText);
      strcpy(pText, pText + NumBytes);
      GUI_UNLOCK_H(pText);
      WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
    } else {
      GUI_UNLOCK_H(pText);
    }
  }
}

/*********************************************************************
*
*       _InsertChar
*
* Create space at the current cursor position and inserts a character.
*/
static int _InsertChar(EDIT_Handle hObj, U16 Char) {
  EDIT_Obj * pObj;
  int CharsAreAvailable;
  int CursorPos;
  WM_HMEM hpText;
  pObj = EDIT_LOCK_H(hObj);
  CharsAreAvailable = _CharsAreAvailable(pObj, 1);
  hpText = pObj->hpText;
  CursorPos = pObj->CursorPos;
  GUI_UNLOCK_H(pObj);
  if (CharsAreAvailable) {
    int BytesNeeded;
    BytesNeeded = GUI_UC__CalcSizeOfChar(Char);
    if (_IsSpaceInBuffer(hObj, BytesNeeded)) {
      int CursorOffset;
      char * pText;
      pText = (char *)GUI_LOCK_H(hpText);
      CursorOffset = GUI_UC__NumChars2NumBytes(pText, CursorPos);
      pText += CursorOffset;
      memmove(pText + BytesNeeded, pText, strlen(pText) + 1);
      GUI_UC_Encode(pText, Char);
      GUI_UNLOCK_H(pText);
      WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
      return 1;
    }
  }
  return 0;
}

/*********************************************************************
*
*       _OnTouch
*/
static void _OnTouch(EDIT_Handle hObj, WM_MESSAGE * pMsg) {
  const GUI_PID_STATE * pState;
  pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
    if (pState->Pressed) {
      GUI_DEBUG_LOG1("EDIT_Callback(WM_TOUCH, Pressed, Handle %d)\n",1);
      EDIT_SetCursorAtPixel(hObj, pState->x);
      WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
    } else {
      GUI_DEBUG_LOG1("EDIT_Callback(WM_TOUCH, Released, Handle %d)\n",1);
      WM_NotifyParent(hObj, WM_NOTIFICATION_RELEASED);
    }
  } else {
    GUI_DEBUG_LOG1("EDIT_Callback(WM_TOUCH, Moved out, Handle %d)\n",1);
    WM_NotifyParent(hObj, WM_NOTIFICATION_MOVED_OUT);
  }
}

/*********************************************************************
*
*       _SetValue
*/
static void _SetValue(EDIT_Handle hObj, I32 Value, int Unsigned) {
  EDIT_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    /* Put in min/max range */
    if (Unsigned) {
      if ((unsigned)Value < (unsigned)pObj->Min) {
        Value = pObj->Min;
      }
      if ((unsigned)Value > (unsigned)pObj->Max) {
        Value = pObj->Max;
      }
    } else {
      if (Value < pObj->Min) {
        Value = pObj->Min;
      }
      if (Value > pObj->Max) {
        Value = pObj->Max;
      }
    }
    if (pObj->CurrentValue != (U32)Value) {
      pObj->CurrentValue = Value;
      if (pObj->pfUpdateBuffer) {
        pObj->pfUpdateBuffer(hObj);
      }
      WM_InvalidateWindow(hObj);
      GUI_UNLOCK_H(pObj);
      WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       _ChangeCharacter
*/
static void _ChangeCharacter(EDIT_Handle hObj, U16 Char, int Index, char * pText) {
  char acBuffer[3] = {0};
  int SizeNew, SizeOld;
  SizeOld = GUI_UC__CalcSizeOfChar(Char);
  Char += Index;
  SizeNew = GUI_UC__CalcSizeOfChar(Char);
  if (SizeOld == SizeNew) {
    GUI_UC_Encode(acBuffer, Char);
    memcpy(pText, acBuffer, SizeNew);
    WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
  }
}

/*********************************************************************
*
*       Private routines:
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
EDIT_Obj * EDIT_LockH(EDIT_Handle h) {
  EDIT_Obj * p = (EDIT_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != EDIT_ID) {
      GUI_DEBUG_ERROROUT("EDIT.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       EDIT__GetCurrentChar
*/
U16 EDIT__GetCurrentChar(EDIT_Obj * pObj) {
  U16 Char = 0;
  if (pObj->hpText) {
    const char * pText;
    pText  = (const char *)GUI_LOCK_H(pObj->hpText);
    pText += GUI_UC__NumChars2NumBytes(pText, pObj->CursorPos);
    Char   = GUI_UC_GetCharCode(pText);
    GUI_UNLOCK_H(pText);
  }
  return Char;
}

/*********************************************************************
*
*       EDIT__SetCursorPos
*
* Sets a new cursor position.
*/
void EDIT__SetCursorPos(EDIT_Handle hObj, int CursorPos) {
  EDIT_Obj * pObj;
  pObj = EDIT_LOCK_H(hObj);
  if (pObj->hpText) {
    char * pText;
    int NumChars, Offset;
    pText    = (char *)GUI_LOCK_H(pObj->hpText);
    NumChars = GUI__GetNumChars(pText);
    Offset   = (pObj->EditMode == GUI_EDIT_MODE_INSERT) ? 0 : 1;
    if (CursorPos < 0) {
      CursorPos = 0;
    }
    if (CursorPos > NumChars) {
      CursorPos = NumChars;
    }
    if (CursorPos > (pObj->MaxLen - Offset)) {
      CursorPos = pObj->MaxLen - Offset;
    }
    if (pObj->CursorPos != CursorPos) {
      pObj->CursorPos = CursorPos;
    }
    pObj->SelSize = 0;
    GUI_UNLOCK_H(pText);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       EDIT__SetValueUnsigned
*/
void EDIT__SetValueUnsigned(EDIT_Handle hObj, I32 Value) {
  _SetValue(hObj, Value, 1);
}

/*********************************************************************
*
*       Exported routines:  Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_Callback
*/
void EDIT_Callback (WM_MESSAGE * pMsg) {
  int IsEnabled;
  EDIT_Handle hObj;
  EDIT_Obj *  pObj;
  hObj = (EDIT_Handle) pMsg->hWin;       
  IsEnabled = WM__IsEnabled(hObj);
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_TIMER:
    WM_SelectWindow(hObj);
    _InvertCursor(hObj);
    pObj = EDIT_LOCK_H(hObj);
    WM_RestartTimer(pObj->hTimer, 0);
    GUI_UNLOCK_H(pObj);
    break;
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    break;
  case WM_PAINT:
    GUI_DEBUG_LOG("EDIT: _Callback(WM_PAINT)\n");
    _Paint(hObj, 1);
    return;
  case WM_DELETE:
    GUI_DEBUG_LOG("EDIT: _Callback(WM_DELETE)\n");
    _Delete(hObj);
    break;       /* No return here ... WM_DefaultProc needs to be called */
  case WM_KEY:
    if (IsEnabled) {
      if ( ((const WM_KEY_INFO*)(pMsg->Data.p))->PressedCnt >0) {
        int Key = ((const WM_KEY_INFO*)(pMsg->Data.p))->Key;
        switch (Key) {
        case GUI_KEY_ENTER:
        case GUI_KEY_ESCAPE:
        case GUI_KEY_TAB:
        case GUI_KEY_BACKTAB:
          break;                    /* Send to parent by not doing anything */
        default:
          EDIT_AddKey(hObj, Key);
          return;
        }
      }
    }
    break;
  }
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       Exported routines:  Create
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_CreateEx
*/
EDIT_Handle EDIT_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int WinFlags, int ExFlags,
                          int Id, int MaxLen)
{
  EDIT_Handle hObj;
  GUI_USE_PARA(ExFlags);
  WM_LOCK();
  /* Alloc memory for obj */
  WinFlags |= WM_CF_LATE_CLIP;    /* Always use late clipping since widget is optimized for it. */
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, EDIT_Callback,
                                sizeof(EDIT_Obj) - sizeof(WM_Obj));
  if (hObj) {
    int BufferSize;
    EDIT_Obj * pObj;
    pObj = (EDIT_Obj *)GUI_LOCK_H(hObj);
    /* init widget specific variables */
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
    /* init member variables */
    EDIT_INIT_ID(pObj);
    pObj->Props         = EDIT__DefaultProps;
    pObj->XSizeCursor   = 1;
    pObj->MaxLen        = (MaxLen == 0) ? 8 : MaxLen;
    pObj->BufferSize    = 0;
    pObj->hpText        = 0;
    BufferSize = pObj->MaxLen + 1;
    GUI_UNLOCK_H(pObj);
    if (_IncrementBuffer(hObj, BufferSize) == 0) {
      GUI_DEBUG_ERROROUT("EDIT_Create failed to alloc buffer");
      EDIT_Delete(hObj);
      hObj = 0;
    }
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       Exported routines:  Various methods
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_SetCursorAtPixel
*/
void EDIT_SetCursorAtPixel(EDIT_Handle hObj, int xPos) {
  if (hObj) {
    EDIT_Obj * pObj;
    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    if (pObj->hpText) {    
      const GUI_FONT GUI_UNI_PTR *pOldFont;
      int xSize, TextWidth, NumChars;
      const char GUI_UNI_PTR * pText;
      pText = (char *)GUI_LOCK_H(pObj->hpText);
      pOldFont = GUI_SetFont(pObj->Props.pFont);
      xSize = WM_GetWindowSizeX(hObj);
      TextWidth = GUI_GetStringDistX(pText);
      switch (pObj->Props.Align & GUI_TA_HORIZONTAL) {
      case GUI_TA_HCENTER:
        xPos -= (xSize - TextWidth + 1) / 2;
        break;
      case GUI_TA_RIGHT:
        xPos -= xSize - TextWidth - (pObj->Props.Border + EDIT_XOFF);
        break;
      default:
        xPos -= (pObj->Props.Border + EDIT_XOFF) + pObj->Widget.pEffect->EffectSize;
      }
      NumChars = GUI__GetNumChars(pText);
      if (xPos < 0) {
        EDIT__SetCursorPos(hObj, 0);
      } else if (xPos > TextWidth) {
        EDIT__SetCursorPos(hObj, NumChars);
      } else {
        EDIT__SetCursorPos(hObj, GUI__GetCursorPosChar(pText, xPos, NumChars));
      }
      GUI_UNLOCK_H(pText);
      GUI_SetFont(pOldFont);
      EDIT_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       EDIT_AddKey
*/
void EDIT_AddKey(EDIT_Handle hObj, int Key) {
  if (hObj) {
    EDIT_Obj * pObj;
    tEDIT_AddKeyEx * pfAddKeyEx;
    U8 EditMode;
    WM_HMEM hpText;
    int CursorPos;
    
    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    pfAddKeyEx = pObj->pfAddKeyEx;
    EditMode   = pObj->EditMode;
    hpText     = pObj->hpText;
    CursorPos  = pObj->CursorPos;
    GUI_UNLOCK_H(pObj);
    if (pfAddKeyEx) {
      pfAddKeyEx(hObj, Key);
    } else {
      switch (Key) {
      case GUI_KEY_UP:
        if (hpText) {
          char * pText;
          U16 Char;
          pText  = (char *)GUI_LOCK_H(hpText);
          pText += GUI_UC__NumChars2NumBytes(pText, CursorPos);
          Char   = GUI_UC_GetCharCode(pText);
          _ChangeCharacter(hObj, Char, +1, pText);
          GUI_UNLOCK_H(pText);
        }
        break;
      case GUI_KEY_DOWN:
        if (hpText) {
          char * pText;
          U16 Char;
          pText  = (char *)GUI_LOCK_H(hpText);
          pText += GUI_UC__NumChars2NumBytes(pText, CursorPos);
          Char   = GUI_UC_GetCharCode(pText);
          _ChangeCharacter(hObj, Char, -1, pText);
          GUI_UNLOCK_H(pText);
        }
        break;
      case GUI_KEY_RIGHT:
        EDIT__SetCursorPos(hObj, CursorPos + 1);
        break;
      case GUI_KEY_LEFT:
        EDIT__SetCursorPos(hObj, CursorPos - 1);
        break;
      case GUI_KEY_BACKSPACE:
        if (CursorPos) {
          EDIT__SetCursorPos(hObj, CursorPos - 1);
          _DeleteChar(hObj);
        }
        break;
      case GUI_KEY_DELETE:
        _DeleteChar(hObj);
        break;
      case GUI_KEY_INSERT:
        pObj = EDIT_LOCK_H(hObj);
        if (EditMode == GUI_EDIT_MODE_OVERWRITE) {
          pObj->EditMode = GUI_EDIT_MODE_INSERT;
        } else {
          pObj->EditMode = GUI_EDIT_MODE_OVERWRITE;
          EDIT__SetCursorPos(hObj, CursorPos);
        }
        GUI_UNLOCK_H(pObj);
        break;
      default:
        if (Key >= 0x20) {
          if (EditMode != GUI_EDIT_MODE_INSERT) {
            _DeleteChar(hObj);
          }
          if (_InsertChar(hObj, Key)) {
            EDIT__SetCursorPos(hObj, CursorPos + 1);
          }
        }
      }
    }
    EDIT_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       EDIT_SetFont
*/
void EDIT_SetFont(EDIT_Handle hObj, const GUI_FONT GUI_UNI_PTR * pfont) {
  EDIT_Obj * pObj;
  if (hObj == 0) {
    return;
  }
  WM_LOCK();
  pObj = EDIT_LOCK_H(hObj);
  if (pObj) {
    pObj->Props.pFont = pfont;
    EDIT_Invalidate(hObj);
  }
  GUI_UNLOCK_H(pObj);
  WM_UNLOCK();
}

/*********************************************************************
*
*       EDIT_SetBkColor
*/
void EDIT_SetBkColor(EDIT_Handle hObj, unsigned int Index, GUI_COLOR color) {
  EDIT_Obj * pObj;
  if (hObj == 0) {
    return;
  }
  WM_LOCK();
  pObj = EDIT_LOCK_H(hObj);
  if (pObj) {
    if (Index < GUI_COUNTOF(pObj->Props.aBkColor)) {
      pObj->Props.aBkColor[Index] = color;
      EDIT_Invalidate(hObj);
    }
  }
  GUI_UNLOCK_H(pObj);
  WM_UNLOCK();
}

/*********************************************************************
*
*       EDIT_SetTextColor
*/
void EDIT_SetTextColor(EDIT_Handle hObj, unsigned int Index, GUI_COLOR color) {
  EDIT_Obj * pObj;
  if (hObj == 0) {
    return;
  }
  WM_LOCK();
  pObj = EDIT_LOCK_H(hObj);
  if (pObj) {
    if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
      pObj->Props.aTextColor[Index] = color;
      EDIT_Invalidate(hObj);
    }
  }
  GUI_UNLOCK_H(pObj);
  WM_UNLOCK();
}

/*********************************************************************
*
*       EDIT_SetText
*/
void EDIT_SetText(EDIT_Handle hObj, const char * s) {
  if (hObj) {
    EDIT_Obj * pObj;
    WM_HMEM hpText;
    I16 MaxLen;

    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    hpText = pObj->hpText;
    MaxLen = pObj->MaxLen;
    GUI_UNLOCK_H(pObj);
    if (s) {
      int NumBytesNew, NumBytesOld = 0;
      int NumCharsNew;
      if (hpText) {
        char * pText;
        pText       = (char *)GUI_LOCK_H(hpText);
        NumBytesOld = strlen(pText) + 1;
        GUI_UNLOCK_H(pText);
      }
      NumCharsNew = GUI__GetNumChars(s);
      if (NumCharsNew > MaxLen) {
        NumCharsNew = MaxLen;
      }
      NumBytesNew = GUI_UC__NumChars2NumBytes(s, NumCharsNew) + 1;
      if (_IsSpaceInBuffer(hObj, NumBytesNew - NumBytesOld)) {
        char * pText;
        pObj  = (EDIT_Obj *)GUI_LOCK_H(hObj); /* Pointer recalculation required */
        pText = (char *)GUI_LOCK_H(pObj->hpText);
        GUI_MEMCPY(pText, s, NumBytesNew);
        pObj->CursorPos = NumCharsNew;
        if (pObj->CursorPos == pObj->MaxLen) {
          if (pObj->EditMode == GUI_EDIT_MODE_OVERWRITE) {
            pObj->CursorPos--;
          }
        }
        GUI_UNLOCK_H(pObj);
        GUI_UNLOCK_H(pText);
      }
    } else {
      GUI_ALLOC_FreePtr(&pObj->hpText);
      pObj->BufferSize = 0;
      pObj->CursorPos  = 0;
    }
    EDIT_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       EDIT_GetText
*/
void EDIT_GetText(EDIT_Handle hObj, char * sDest, int MaxLen) {
  if (sDest) {
    *sDest = 0;
    if (hObj) {
      EDIT_Obj * pObj;
      WM_LOCK();
      pObj = EDIT_LOCK_H(hObj);
      if (pObj->hpText) {
        char * pText;
        int NumChars, NumBytes;
        pText = (char *)GUI_LOCK_H(pObj->hpText);
        NumChars = GUI__GetNumChars(pText);
        NumBytes = GUI_UC__NumChars2NumBytes(pText, NumChars);
        if (NumBytes > (MaxLen - 1)) {
          NumBytes = MaxLen - 1;
        }
        GUI_MEMCPY(sDest, pText, NumBytes);
        GUI_UNLOCK_H(pText);
        *(sDest + NumBytes) = 0;
      }
      GUI_UNLOCK_H(pObj);
      WM_UNLOCK();
    }
  }
}

/*********************************************************************
*
*       EDIT_GetValue
*/
I32  EDIT_GetValue(EDIT_Handle hObj) {
  EDIT_Obj * pObj;
  I32 r = 0;
  if (hObj) {
    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    r = pObj->CurrentValue;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       EDIT_SetValue
*/
void EDIT_SetValue(EDIT_Handle hObj, I32 Value) {
  _SetValue(hObj, Value, 0);
}

/*********************************************************************
*
*       EDIT_SetMaxLen
*/
void EDIT_SetMaxLen(EDIT_Handle  hObj, int MaxLen) {
  if (hObj) {
    EDIT_Obj * pObj;
    U16 BufferSize;
    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    BufferSize = pObj->BufferSize;
    if (MaxLen != pObj->MaxLen) {
      if (MaxLen < pObj->MaxLen) {
        if (pObj->hpText) {
          char * pText;
          int   NumChars;
          pText    = (char *)GUI_LOCK_H(pObj->hpText);
          NumChars = GUI__GetNumChars(pText);
          if (NumChars > MaxLen) {
            int NumBytes;
            NumBytes = GUI_UC__NumChars2NumBytes(pText, MaxLen);
            *(pText + NumBytes) = 0;
          }
          GUI_UNLOCK_H(pText);
        }
      }
      GUI_UNLOCK_H(pObj);
      _IncrementBuffer(hObj, MaxLen - BufferSize + 1);
      pObj = EDIT_LOCK_H(hObj);
      pObj->MaxLen = MaxLen;
      EDIT_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       EDIT_SetTextAlign
*/
void EDIT_SetTextAlign(EDIT_Handle hObj, int Align) {
  EDIT_Obj * pObj;
  if (hObj == 0) {
    return;
  }
  WM_LOCK();
  pObj = EDIT_LOCK_H(hObj);
  if (pObj) {
    pObj->Props.Align = Align;
    EDIT_Invalidate(hObj);
  }
  GUI_UNLOCK_H(pObj);
  WM_UNLOCK();
}

#else  /* avoid empty object files */

void Edit_C(void) {}

#endif

