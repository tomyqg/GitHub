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
File        : MULTIEDIT.c
Purpose     : Implementation of MULTIEDIT widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "MULTIEDIT.h"
#include "WIDGET.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

/* Define default fonts */
#ifndef MULTIEDIT_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define MULTIEDIT_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define MULTIEDIT_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define MULTIEDIT_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif

/* Define colors */
#ifndef MULTIEDIT_BKCOLOR0_DEFAULT
  #define MULTIEDIT_BKCOLOR0_DEFAULT   GUI_WHITE
#endif

#ifndef MULTIEDIT_BKCOLOR1_DEFAULT
  #define MULTIEDIT_BKCOLOR1_DEFAULT   0xC0C0C0
#endif

#ifndef MULTIEDIT_TEXTCOLOR0_DEFAULT
  #define MULTIEDIT_TEXTCOLOR0_DEFAULT GUI_BLACK
#endif

#ifndef MULTIEDIT_TEXTCOLOR1_DEFAULT
  #define MULTIEDIT_TEXTCOLOR1_DEFAULT GUI_BLACK
#endif

/* Define character for password mode */
#define MULTIEDIT_PASSWORD_CHAR   '*'

#define MULTIEDIT_REALLOC_SIZE  16

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/

#define NUM_DISP_MODES 2

#define INVALID_NUMCHARS (1 << 0)
#define INVALID_NUMLINES (1 << 1)
#define INVALID_TEXTSIZE (1 << 2)
#define INVALID_CURSORXY (1 << 3)
#define INVALID_LINEPOSB (1 << 4)

typedef struct {
  WIDGET Widget;
  GUI_COLOR aBkColor[NUM_DISP_MODES];
  GUI_COLOR aColor[NUM_DISP_MODES];
  WM_HMEM hText;
  U16 MaxNumChars;         /* Maximum number of characters including the prompt */
  U16 NumChars;            /* Number of characters (text and prompt) in object */
  U16 NumCharsPrompt;      /* Number of prompt characters */
  U16 NumLines;            /* Number of text lines needed to show all data */
  U16 TextSizeX;           /* Size in X of text depending of wrapping mode */
  U16 BufferSize;
  U16 CursorLine;          /* Number of current cursor line */
  U16 CursorPosChar;       /* Character offset number of cursor */
  U16 CursorPosByte;       /* Byte offset number of cursor */
  I16 CursorPosX;          /* Cursor position in X */
  U16 CursorPosY;          /* Cursor position in Y */
  U16 CacheLinePosByte;    /*  */
  U16 CacheLineNumber;     /*  */
  U16 CacheFirstVisibleLine;
  U16 CacheFirstVisibleByte;
  U16 Align;
  WM_SCROLL_STATE ScrollStateV;
  WM_SCROLL_STATE ScrollStateH;
  const GUI_FONT GUI_UNI_PTR * pFont;
  U8 Flags;
  U8 InvalidFlags;         /* Flags to save validation status */
  U8 EditMode;
  U8 HBorder;
  WM_HTIMER hTimer;
  GUI_WRAPMODE WrapMode;
  #if GUI_DEBUG_LEVEL >1
    int DebugId;
  #endif
} MULTIEDIT_OBJ;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static GUI_COLOR _aDefaultBkColor[2] = {
  MULTIEDIT_BKCOLOR0_DEFAULT,
  MULTIEDIT_BKCOLOR1_DEFAULT,
};

static GUI_COLOR _aDefaultColor[2] = {
  MULTIEDIT_TEXTCOLOR0_DEFAULT,
  MULTIEDIT_TEXTCOLOR1_DEFAULT,
};

static const GUI_FONT GUI_UNI_PTR * _pDefaultFont = MULTIEDIT_FONT_DEFAULT;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define MULTIEDIT_INIT_ID(p) p->DebugId = MULTIEDIT_ID
#else
  #define MULTIEDIT_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  MULTIEDIT_OBJ * MULTIEDIT_LockH(MULTIEDIT_HANDLE h);
  #define MULTIEDIT_LOCK_H(h)   MULTIEDIT_LockH(h)
#else
  #define MULTIEDIT_LOCK_H(h)   (MULTIEDIT_OBJ *)GUI_LOCK_H(h)
#endif

/*********************************************************************
*
*       static code, helper functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _InvalidateNumChars
*
* Invalidates the number of characters including the prompt
*/
static void _InvalidateNumChars(MULTIEDIT_OBJ * pObj) {
  pObj->InvalidFlags |= INVALID_NUMCHARS;
}

/*********************************************************************
*
*       _GetNumChars
*
* Calculates (if needed) and returns the number of characters including the prompt
*/
static int _GetNumChars(MULTIEDIT_OBJ * pObj) {
  if (pObj->InvalidFlags & INVALID_NUMCHARS) {
    char * pText;
    pText = (char *)GUI_LOCK_H(pObj->hText);
    pObj->NumChars = GUI__GetNumChars(pText);
    GUI_UNLOCK_H(pText);
    pObj->InvalidFlags &= ~INVALID_NUMCHARS;
  }
  return pObj->NumChars;
}

/*********************************************************************
*
*       _GetXSize
*
* Returns the x size for displaying text.
*/
static int _GetXSize(MULTIEDIT_HANDLE hObj, const MULTIEDIT_OBJ * pObj) {
  GUI_RECT Rect;
  WM_GetInsideRectExScrollbar(hObj, &Rect);
  return Rect.x1 - Rect.x0 - (pObj->HBorder * 2) - 1;
}

/*********************************************************************
*
*       _GetNumCharsInPrompt
*/
static int _GetNumCharsInPrompt(const MULTIEDIT_OBJ * pObj, const char GUI_UNI_PTR * pText) {
  char *pString, *pEndPrompt;
  int r = 0;
  pString = (char *)GUI_LOCK_H(pObj->hText);
  pEndPrompt = pString + GUI_UC__NumChars2NumBytes(pString, pObj->NumCharsPrompt);
  if (pText < pEndPrompt) {
    r = GUI_UC__NumBytes2NumChars(pText, pEndPrompt - pText);
  }
  GUI_UNLOCK_H(pString);
  return r;
}

/*********************************************************************
*
*       _WrapGetNumCharsDisp
*/
static int _WrapGetNumCharsDisp(MULTIEDIT_HANDLE hObj, const MULTIEDIT_OBJ * pObj, const char GUI_UNI_PTR * pText) {
  int xSize, r;
  xSize = _GetXSize(hObj, pObj);
  if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
    int NumCharsPrompt;
    NumCharsPrompt = _GetNumCharsInPrompt(pObj, pText);
    r = GUI__WrapGetNumCharsDisp(pText, xSize, pObj->WrapMode);
    if (r >= NumCharsPrompt) {
      int x;
      switch (pObj->WrapMode) {
      case GUI_WRAPMODE_NONE:
        r = GUI__GetNumChars(pText);
        break;
      default:
        r = NumCharsPrompt;
        x = GUI__GetLineDistX(pText, NumCharsPrompt);
        pText += GUI_UC__NumChars2NumBytes(pText, NumCharsPrompt);
        while (GUI_UC__GetCharCodeInc(&pText) != 0) {
          x += GUI_GetCharDistX(MULTIEDIT_PASSWORD_CHAR);
          if (r && (x > xSize)) {
            break;
          }
          r++;
        }
        break;
      }
    }
  } else {
    r = GUI__WrapGetNumCharsDisp(pText, xSize, pObj->WrapMode);
  }
  return r;
}

/*********************************************************************
*
*       _WrapGetNumBytesToNextLine
*/
static int _WrapGetNumBytesToNextLine(MULTIEDIT_HANDLE hObj, const MULTIEDIT_OBJ * pObj, const char * pText) {
  int xSize, r;
  xSize = _GetXSize(hObj, pObj);
  if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
    int NumChars, NumCharsPrompt;
    NumCharsPrompt = _GetNumCharsInPrompt(pObj, pText);
    NumChars = _WrapGetNumCharsDisp(hObj, pObj, pText);
    r        = GUI_UC__NumChars2NumBytes(pText, NumChars);
    if (NumChars < NumCharsPrompt) {
      if (*(pText + r) == '\n') {
        r++;
      }
    }
  } else {
    r = GUI__WrapGetNumBytesToNextLine(pText, xSize, pObj->WrapMode);
  }
  return r;
}

/*********************************************************************
*
*       _GetCharDistX
*/
static int _GetCharDistX(const MULTIEDIT_OBJ * pObj, const char * pText) {
  int r;
  if ((pObj->Flags & MULTIEDIT_SF_PASSWORD) && (_GetNumCharsInPrompt(pObj, pText) == 0)) {
    r = GUI_GetCharDistX(MULTIEDIT_PASSWORD_CHAR);
  } else {
    U16 c;
    c = GUI_UC_GetCharCode(pText);
    r = GUI_GetCharDistX(c);
  }
  return r;
}

/*********************************************************************
*
*       _DispString
*/
static void _DispString(MULTIEDIT_HANDLE hObj, const MULTIEDIT_OBJ * pObj, const char * pText, GUI_RECT* pRect) {
  int NumCharsDisp;
  NumCharsDisp = _WrapGetNumCharsDisp(hObj, pObj, pText);
  if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
    int x, NumCharsPrompt, NumCharsLeft = 0;
    NumCharsPrompt = _GetNumCharsInPrompt(pObj, pText);
    if (NumCharsDisp < NumCharsPrompt) {
      NumCharsPrompt = NumCharsDisp;
    } else {
      NumCharsLeft = NumCharsDisp - NumCharsPrompt;
    }
    GUI_DispStringInRectMax(pText, pRect, GUI_TA_LEFT, NumCharsPrompt);
    x = pRect->x0 + GUI__GetLineDistX(pText, NumCharsPrompt);
    if (NumCharsLeft) {
      GUI_DispCharAt(MULTIEDIT_PASSWORD_CHAR, x, pRect->y0);
      GUI_DispChars(MULTIEDIT_PASSWORD_CHAR, NumCharsLeft - 1);
    }
  } else {
    GUI_DispStringInRectMax(pText, pRect, pObj->Align, NumCharsDisp);
  }
}

/*********************************************************************
*
*       static code, cursor routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetpLineLocked
*
* Returns a pointer to the beginning of the line with the
* given line number.
*/
static char * _GetpLineLocked(MULTIEDIT_HANDLE hObj, MULTIEDIT_OBJ * pObj, unsigned LineNumber) {
  char * pText, * pLine;
  pText = (char *)GUI_LOCK_H(pObj->hText);
  if ((unsigned)pObj->CacheLineNumber != LineNumber) {
    if (LineNumber > (unsigned)pObj->CacheLineNumber) {
      /* If new line number > cache we can start with old pointer */
      int OldNumber = pObj->CacheLineNumber;
      pLine = pText + pObj->CacheLinePosByte;
      pObj->CacheLineNumber  = LineNumber;
      LineNumber -= OldNumber;
    } else {
      /* If new line number < cache we need to start with first byte */
      pLine = pText;
      pObj->CacheLineNumber  = LineNumber;
    }
    while (LineNumber--) {
      pLine += _WrapGetNumBytesToNextLine(hObj, pObj, pLine);
    }
    pObj->CacheLinePosByte = pLine - pText;
  }
  return pText + pObj->CacheLinePosByte;
}

/*********************************************************************
*
*       _ClearCache
*
* Clears the cached position of the linenumber and the first byte
* of the line which holds the cursor.
*/
static void _ClearCache(MULTIEDIT_OBJ * pObj) {
  pObj->CacheLineNumber = 0;
  pObj->CacheLinePosByte = 0;
  pObj->CacheFirstVisibleByte = 0;
  pObj->CacheFirstVisibleLine = 0;
}

/*********************************************************************
*
*       _GetCursorLine
*
* Returns the line number of the cursor position.
*/
static int _GetCursorLine(MULTIEDIT_HANDLE hObj, const MULTIEDIT_OBJ * pObj, const char * pText, int CursorPosChar) {
  const char *pCursor;
  const char *pEndLine;
  int NumChars, ByteOffsetNewCursor, LineNumber = 0;
  ByteOffsetNewCursor = GUI_UC__NumChars2NumBytes(pText, CursorPosChar);
  pCursor = pText + ByteOffsetNewCursor;
  if (pObj->CacheLinePosByte < ByteOffsetNewCursor) {
    /* If cache pos < new position we can use it as start position */
    pText      += pObj->CacheLinePosByte;
    LineNumber += pObj->CacheLineNumber;
  }
  while (*pText && (pCursor > pText)) {
    NumChars = _WrapGetNumCharsDisp(hObj, pObj, pText);
    pEndLine = pText + GUI_UC__NumChars2NumBytes(pText, NumChars);
    pText   += _WrapGetNumBytesToNextLine(hObj, pObj, pText);
    if (pCursor <= pEndLine) {
      if ((pCursor == pEndLine) && (pEndLine == pText) && *pText) {
        LineNumber++;
      }
      break;
    }
    LineNumber++;
  }
  return LineNumber;
}

/*********************************************************************
*
*       _GetCursorXY
*/
static void _GetCursorXY(MULTIEDIT_HANDLE hObj, /*const*/ MULTIEDIT_OBJ * pObj, int* px, int* py) {
  if (pObj->InvalidFlags & INVALID_CURSORXY) {
    int CursorLine = 0, x = 0;
    GUI_SetFont(pObj->pFont);
    if (pObj->hText) {
      const char * pLine;
      const char * pCursor;
      pLine      = (const char *)GUI_LOCK_H(pObj->hText);
      pCursor    = pLine + pObj->CursorPosByte;
      CursorLine = pObj->CursorLine;
      GUI_UNLOCK_H(pLine);
      pLine      = _GetpLineLocked(hObj, pObj, CursorLine);
      if ((pObj->Flags & MULTIEDIT_SF_PASSWORD)) {
        while (pLine < pCursor) {
          x     += _GetCharDistX(pObj, pLine);
          pLine += GUI_UC_GetCharSize(pLine);
        }
      } else {
        int xSize;
        const char * p;
        int NumChars = 0, NumCharsToNextLine;
        xSize = _GetXSize(hObj, pObj);
        p = pLine;
        while (pLine < pCursor) {
          NumChars++;
          pLine += GUI_UC_GetCharSize(pLine);
        }
        NumCharsToNextLine = _WrapGetNumCharsDisp(hObj, pObj, p);
        x = GUI__GetCursorPosX(p, NumChars, NumCharsToNextLine);
        if (pObj->Align == GUI_TA_RIGHT) {
          x += xSize - GUI__GetLineDistX(p, NumCharsToNextLine);
        }
      }
      GUI_UNLOCK_H(pLine);
    }
    pObj->CursorPosX = x;
    pObj->CursorPosY = CursorLine * GUI_GetFontDistY();
    pObj->InvalidFlags &= ~INVALID_CURSORXY;
  }
  *px = pObj->CursorPosX;
  *py = pObj->CursorPosY;
}

/*********************************************************************
*
*       _InvalidateCursorXY
*/
static void _InvalidateCursorXY(MULTIEDIT_OBJ * pObj) {
  pObj->InvalidFlags |= INVALID_CURSORXY;
}

/*********************************************************************
*
*       _SetScrollState
*/
static void _SetScrollState(WM_HWIN hObj) {
  MULTIEDIT_OBJ * pObj;
  WM_SCROLL_STATE ScrollStateV, ScrollStateH;
  pObj = MULTIEDIT_LOCK_H(hObj);
  ScrollStateV = pObj->ScrollStateV;
  ScrollStateH = pObj->ScrollStateH;
  GUI_UNLOCK_H(pObj);
  WIDGET__SetScrollState(hObj, &ScrollStateV, &ScrollStateH);
}

/*********************************************************************
*
*       _CalcScrollPos
*
* Purpose:
*   Find out if the current position of the cursor is still in the
*   visible area. If it is not, the scroll position is updated.
*   Needs to be called every time the cursor is move, wrap, font or
*   window size are changed.
*/
static void _CalcScrollPos(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  int xCursor, yCursor;
  pObj = MULTIEDIT_LOCK_H(hObj);
  _GetCursorXY(hObj, pObj, &xCursor, &yCursor);
  yCursor /= GUI_GetYDistOfFont(pObj->pFont);
  WM_CheckScrollPos(&pObj->ScrollStateV, yCursor, 0, 0);       /* Vertical */
  if (pObj->Align == GUI_TA_RIGHT) {
    xCursor = _GetXSize(hObj, pObj) - xCursor;
  }
  WM_CheckScrollPos(&pObj->ScrollStateH, xCursor, 30, 30);     /* Horizontal */
  GUI_UNLOCK_H(pObj);
  _SetScrollState(hObj);
}

/*********************************************************************
*
*       _GetTextSizeX
*
* Returns the width of the displayed text.
*/
static int _GetTextSizeX(MULTIEDIT_HANDLE hObj, MULTIEDIT_OBJ * pObj) {
  if (pObj->InvalidFlags & INVALID_TEXTSIZE) {
    pObj->TextSizeX = 0;
    if (pObj->hText) {
      int NumChars, xSizeLine;
      char * pText;
      char * pLine;
      GUI_SetFont(pObj->pFont);
      pText = (char *)GUI_LOCK_H(pObj->hText);
      do {
        NumChars = _WrapGetNumCharsDisp(hObj, pObj, pText);
        xSizeLine = 0;
        pLine = pText;
        if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
          while (NumChars--) {
            xSizeLine += _GetCharDistX(pObj, pLine);
            pLine     += GUI_UC_GetCharSize(pLine);
          }
        } else {
          xSizeLine += GUI__GetLineDistX(pLine, NumChars);
        }
        if (xSizeLine > pObj->TextSizeX) {
          pObj->TextSizeX = xSizeLine;
        }
        pText += _WrapGetNumBytesToNextLine(hObj, pObj, pText);
      } while (*pText);
      GUI_UNLOCK_H(pText);
    }
    pObj->InvalidFlags &= ~INVALID_TEXTSIZE;
  }
  return pObj->TextSizeX;
}

/*********************************************************************
*
*       _GetNumVisLines
*/
static int _GetNumVisLines(MULTIEDIT_HANDLE hObj, const MULTIEDIT_OBJ * pObj) {
  GUI_RECT Rect;
  WM_GetInsideRectExScrollbar(hObj, &Rect);
  return (Rect.y1 - Rect.y0 + 1) / GUI_GetYDistOfFont(pObj->pFont);
}

/*********************************************************************
*
*       _GetNumLines
*
* Calculates (if needed) and returns the number of lines
*/
static int _GetNumLines(MULTIEDIT_HANDLE hObj, MULTIEDIT_OBJ * pObj) {
  if (pObj->InvalidFlags & INVALID_NUMLINES) {
    int NumLines = 0;
    if (pObj->hText) {
      int NumChars, NumBytes;
      char * pText;
      U16 Char;
      pText = (char *)GUI_LOCK_H(pObj->hText);
      GUI_SetFont(pObj->pFont);
      do {
        NumChars = _WrapGetNumCharsDisp(hObj, pObj, pText);
        NumBytes = GUI_UC__NumChars2NumBytes(pText, NumChars);
        Char     = GUI_UC_GetCharCode(pText + NumBytes);
        if (Char) {
          NumLines++;
        }
        pText += _WrapGetNumBytesToNextLine(hObj, pObj, pText);
      } while (Char);
      GUI_UNLOCK_H(pText);
    }
    pObj->NumLines = NumLines + 1;
    pObj->InvalidFlags &= ~INVALID_NUMLINES;
  }
  return pObj->NumLines;
}

/*********************************************************************
*
*       _InvalidateNumLines
*
* Invalidates the number of lines
*/
static void _InvalidateNumLines(MULTIEDIT_OBJ * pObj) {
  pObj->InvalidFlags |= INVALID_NUMLINES;
}

/*********************************************************************
*
*       _InvalidateTextSizeX
*
* Invalidates the TextSizeX
*/
static void _InvalidateTextSizeX(MULTIEDIT_OBJ * pObj) {
  pObj->InvalidFlags |= INVALID_TEXTSIZE;
}

/*********************************************************************
*
*       _CalcScrollParas
*
* Purpose:
*   Calculate page size ,number of items & position
*/
static void _CalcScrollParas(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  pObj = MULTIEDIT_LOCK_H(hObj);
  /* Calc vertical scroll parameters */
  pObj->ScrollStateV.NumItems = _GetNumLines(hObj, pObj);
  pObj->ScrollStateV.PageSize = _GetNumVisLines(hObj, pObj);
  /* Calc horizontal scroll parameters */
  pObj->ScrollStateH.NumItems = _GetTextSizeX(hObj, pObj);
  pObj->ScrollStateH.PageSize = _GetXSize(hObj, pObj);
  GUI_UNLOCK_H(pObj);
  _CalcScrollPos(hObj);
}

/*********************************************************************
*
*       _ManageAutoScrollV
*/
static void _ManageAutoScrollV(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  char IsRequired;
  int r;
  pObj = MULTIEDIT_LOCK_H(hObj);
  if (pObj->Flags & MULTIEDIT_SF_AUTOSCROLLBAR_V) {
    IsRequired = _GetNumVisLines(hObj, pObj) < _GetNumLines(hObj, pObj);
    GUI_UNLOCK_H(pObj);
    r = WM_SetScrollbarV(hObj, IsRequired);
    if (r != IsRequired) {
      pObj = MULTIEDIT_LOCK_H(hObj);
      _InvalidateNumLines(pObj);
      _InvalidateTextSizeX(pObj);
      _InvalidateCursorXY(pObj);
      _ClearCache(pObj);
      GUI_UNLOCK_H(pObj);
    }
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       _ManageScrollers
*
* Function:
* If autoscroll mode is enabled, add or remove the horizonatal and
* vertical scrollbars as required.
* Caution: This routine should not be called as reaction to a message
* From the child, as this could lead to a recursion problem
*/
static void _ManageScrollers(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  /* 1. Step: Check if vertical scrollbar is required */
  _ManageAutoScrollV(hObj);
  pObj = MULTIEDIT_LOCK_H(hObj);
  /* 2. Step: Check if horizontal scrollbar is required */
  if (pObj->Flags & MULTIEDIT_SF_AUTOSCROLLBAR_H) {
    char IsRequired;
    int r;
    IsRequired = (_GetXSize(hObj, pObj) < _GetTextSizeX(hObj, pObj));
    GUI_UNLOCK_H(pObj);
    r = WM_SetScrollbarH(hObj, IsRequired);
    if (r != IsRequired) {
      /* 3. Step: Check vertical scrollbar again if horizontal has changed */
      _ManageAutoScrollV(hObj);
    }
  } else {
    GUI_UNLOCK_H(pObj);
  }
  _CalcScrollParas(hObj);
}

/*********************************************************************
*
*       _Invalidate
*/
static void _Invalidate(MULTIEDIT_HANDLE hObj) {
  _ManageScrollers(hObj);
  WM_Invalidate(hObj);
}

/*********************************************************************
*
*       _InvalidateTextArea
*
* Invalidates the text area only
*/
static void _InvalidateTextArea(MULTIEDIT_HANDLE hObj) {
  GUI_RECT rInsideRect;
  _ManageScrollers(hObj);
  WM_GetInsideRectExScrollbar(hObj, &rInsideRect);
  WM_InvalidateRect(hObj, &rInsideRect);
}

/*********************************************************************
*
*       _InvalidateCursorPos
*
* Sets the position of the cursor to an invalid value
*/
static int _InvalidateCursorPos(MULTIEDIT_OBJ * pObj) {
  int Value;
  Value = pObj->CursorPosChar;
  pObj->CursorPosChar = 0xffff;
  return Value;
}

/*********************************************************************
*
*       _SetFlag
*/
static void _SetFlag(MULTIEDIT_HANDLE hObj, int OnOff, U8 Flag) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    if (OnOff) {
      pObj->Flags |= Flag;
    } else {
      pObj->Flags &= ~(int)Flag;
    }
    GUI_UNLOCK_H(pObj);
    _InvalidateTextArea(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       _CalcNextValidCursorPos
*
* Purpose:
*   Calculates the next valid cursor position of the desired position.
*
* Parameters:
*   hObj, pObj    : Obvious
*   CursorPosChar : New character position of the cursor
*   pCursorPosByte: Pointer to save the cursorposition in bytes. Used to abolish further calculations. Could be 0.
*   pCursorLine   : Pointer to save the line number of the cursor. Used to abolish further calculations. Could be 0.
*/
static int _CalcNextValidCursorPos(MULTIEDIT_HANDLE hObj, MULTIEDIT_OBJ * pObj, int CursorPosChar, int * pCursorPosByte, int * pCursorLine) {
  if (pObj->hText) {
    char * pNextLine;
    char * pCursor;
    char * pText;
    int CursorLine, NumChars, CursorPosByte;
    pText    = (char *)GUI_LOCK_H(pObj->hText);
    NumChars = _GetNumChars(pObj);
    /* Set offset in valid range */
    if (CursorPosChar < pObj->NumCharsPrompt) {
      CursorPosChar = pObj->NumCharsPrompt;
    }
    if (CursorPosChar > NumChars) {
      CursorPosChar = NumChars;
    }
    CursorPosByte = GUI_UC__NumChars2NumBytes(pText, CursorPosChar);
    CursorLine    = _GetCursorLine(hObj, pObj, pText, CursorPosChar);
    pCursor       = pText + CursorPosByte;
    pNextLine     = _GetpLineLocked(hObj, pObj, CursorLine);
    if (pNextLine > pCursor) {
      if (pObj->CursorPosChar < CursorPosChar) {
        pCursor = pNextLine;
      } else {
        char * pPrevLine;
        pPrevLine  = _GetpLineLocked(hObj, pObj, CursorLine - 1);
        NumChars   = _WrapGetNumCharsDisp(hObj, pObj, pPrevLine);
        pPrevLine += GUI_UC__NumChars2NumBytes(pPrevLine, NumChars);
        pCursor = pPrevLine;
        GUI_UNLOCK_H(pPrevLine);
      }
      CursorPosChar = GUI_UC__NumBytes2NumChars(pText, pCursor - pText);
      CursorPosByte = GUI_UC__NumChars2NumBytes(pText, CursorPosChar);
      CursorLine    = _GetCursorLine(hObj, pObj, pText, CursorPosChar);
    }
    GUI_UNLOCK_H(pNextLine);
    GUI_UNLOCK_H(pText);
    if (pCursorPosByte) {
      *pCursorPosByte = CursorPosByte;
    }
    if (pCursorLine) {
      *pCursorLine = CursorLine;
    }
    return CursorPosChar;
  }
  return 0;
}

/*********************************************************************
*
*       _SetCursorPos
*
* Sets a new cursor position.
*/
static void _SetCursorPos(MULTIEDIT_HANDLE hObj, int CursorPosChar) {
  MULTIEDIT_OBJ * pObj;
  int CursorPosByte, CursorLine;
  pObj = MULTIEDIT_LOCK_H(hObj);
  CursorPosChar = _CalcNextValidCursorPos(hObj, pObj, CursorPosChar, &CursorPosByte, &CursorLine);
  /* Assign value and recalc whatever necessary */
  if (pObj->CursorPosChar != CursorPosChar) {
    /* Save values */
    pObj->CursorPosByte = CursorPosByte;
    pObj->CursorPosChar = CursorPosChar;
    pObj->CursorLine = CursorLine;
    _InvalidateCursorXY(pObj); /* Invalidate X/Y position */
    GUI_UNLOCK_H(pObj);
    _CalcScrollPos(hObj);
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       _SetWrapMode
*/
static int _SetWrapMode(MULTIEDIT_HANDLE hObj, GUI_WRAPMODE WrapMode) {
  int r;
  r = 0;
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    r = pObj->WrapMode;
    if (pObj->WrapMode != WrapMode) {
      int Position;
      pObj->WrapMode = WrapMode;
      _ClearCache(pObj);
      _InvalidateNumLines(pObj);
      _InvalidateTextSizeX(pObj);
      GUI_UNLOCK_H(pObj);
      _InvalidateTextArea(hObj);
      pObj = MULTIEDIT_LOCK_H(hObj);
      Position = _InvalidateCursorPos(pObj);
      _SetCursorPos(hObj, Position);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       _SetCursorXY
*
* Sets the cursor position from window coordinates.
*/
static void _SetCursorXY(MULTIEDIT_HANDLE hObj, MULTIEDIT_OBJ * pObj, int x, int y) {
  int CursorPosChar = 0;
  if ((x < 0) || (y < 0)) {
    return;
  }
  if (pObj->hText) {
    char *pLine, *pText;
    int CursorLine, WrapChars;
    int SizeX = 0;
    U16 Char;
    GUI_SetFont(pObj->pFont);
    CursorLine = y / GUI_GetFontDistY();
    pLine      = _GetpLineLocked(hObj, pObj, CursorLine);
    pText      = (char *)GUI_LOCK_H(pObj->hText);
    WrapChars  = _WrapGetNumCharsDisp(hObj, pObj, pLine);
    Char       = GUI_UC__GetCharCode(pLine + GUI_UC__NumChars2NumBytes(pLine, WrapChars));
    if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
      if (!Char) {
        WrapChars++;
      }
    } else {
      if (!Char || (Char == '\n') || ((Char == ' ') && (pObj->WrapMode == GUI_WRAPMODE_WORD))) {
        WrapChars++;
      }
    }
    if (pObj->Align == GUI_TA_RIGHT) {
      x -= _GetXSize(hObj, pObj) - GUI__GetLineDistX(pLine, WrapChars);
      x -= pObj->ScrollStateH.v << 1;
      if (x < 0) {
        x = 0;
      }
    }

    if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
      while (--WrapChars > 0) {
        Char   = GUI_UC_GetCharCode(pLine);
        SizeX += _GetCharDistX(pObj, pLine);
        if (!Char || (SizeX > x)) {
          break;
        }
        pLine += GUI_UC_GetCharSize(pLine);
      }
      CursorPosChar = GUI_UC__NumBytes2NumChars(pText, pLine - pText);
    } else {
      CursorPosChar = GUI_UC__NumBytes2NumChars(pText, pLine - pText) + GUI__GetCursorPosChar(pLine, x, WrapChars);
    }
    GUI_UNLOCK_H(pLine);
    GUI_UNLOCK_H(pText);
  }
  _SetCursorPos(hObj, CursorPosChar);
}

/*********************************************************************
*
*       _MoveCursorUp
*/
static void _MoveCursorUp(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  int xPos, yPos;
  pObj = MULTIEDIT_LOCK_H(hObj);
  _GetCursorXY(hObj, pObj, &xPos, &yPos);
  yPos -= GUI_GetYDistOfFont(pObj->pFont);
  _SetCursorXY(hObj, pObj, xPos, yPos);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _MoveCursorDown
*/
static void _MoveCursorDown(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  int xPos, yPos;
  pObj = MULTIEDIT_LOCK_H(hObj);
  _GetCursorXY(hObj, pObj, &xPos, &yPos);
  yPos += GUI_GetYDistOfFont(pObj->pFont);
  _SetCursorXY(hObj, pObj, xPos, yPos);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _MoveCursor2NextLine
*/
static void _MoveCursor2NextLine(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  int xPos, yPos;
  pObj = MULTIEDIT_LOCK_H(hObj);
  _GetCursorXY(hObj, pObj, &xPos, &yPos);
  yPos += GUI_GetYDistOfFont(pObj->pFont);
  _SetCursorXY(hObj, pObj, 0, yPos);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _MoveCursor2LineEnd
*/
static void _MoveCursor2LineEnd(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  int xPos, yPos;
  pObj = MULTIEDIT_LOCK_H(hObj);
  _GetCursorXY(hObj, pObj, &xPos, &yPos);
  _SetCursorXY(hObj, pObj, 0x7FFF, yPos);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _MoveCursor2LinePos1
*/
static void _MoveCursor2LinePos1(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  int xPos, yPos;
  pObj = MULTIEDIT_LOCK_H(hObj);
  _GetCursorXY(hObj, pObj, &xPos, &yPos);
  _SetCursorXY(hObj, pObj, 0, yPos);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _IsOverwriteAtThisChar
*/
static int _IsOverwriteAtThisChar(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  int r = 0;
  pObj = MULTIEDIT_LOCK_H(hObj);
  if (pObj->hText && !(pObj->Flags & MULTIEDIT_CF_INSERT)) {
    const char *pText;
    int CurPos, Line1, Line2;
    U16 Char;
    pText  = (const char *)GUI_LOCK_H(pObj->hText);
    Line1  = pObj->CursorLine;
    CurPos = _CalcNextValidCursorPos(hObj, pObj, pObj->CursorPosChar + 1, 0, 0);
    Line2  = _GetCursorLine(hObj, pObj, pText, CurPos);
    pText += pObj->CursorPosByte;
    Char   = GUI_UC_GetCharCode(pText);
    GUI_UNLOCK_H(pText);
    if (Char) {
      if ((Line1 == Line2) || (pObj->Flags & MULTIEDIT_SF_PASSWORD)) {
        r = 1;
      } else {
        if (Char != '\n') {
          if ((Char != ' ') || (pObj->WrapMode == GUI_WRAPMODE_CHAR)) {
            r = 1;
          }
        }
      }
    }
  }
  GUI_UNLOCK_H(pObj);
  return r;
}

/*********************************************************************
*
*       _GetCursorSizeX
*
* Returns the width of the cursor to be draw according to the
* insert mode flag and the cursor position.
*/
static int _GetCursorSizeX(MULTIEDIT_HANDLE hObj, MULTIEDIT_OBJ * pObj, int * pIsRTL) {
  const char * pText;
  U16 Char;
  int r;

  if (_IsOverwriteAtThisChar(hObj)) {
    if (GUI__BIDI_Enabled == 1) {
      if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
        pText  = (const char *)GUI_LOCK_H(pObj->hText);
        pText += pObj->CursorPosByte;
        r = _GetCharDistX(pObj, pText);
        GUI_UNLOCK_H(pText);
        return r;
      } else {
        pText  = (const char *)GUI_LOCK_H(pObj->hText);
        Char = _pfGUI__BIDI_GetLogChar(pText, pObj->NumChars, pObj->CursorPosChar);
        *pIsRTL = _pfGUI__BIDI_GetCharDir(pText, pObj->NumChars, pObj->CursorPosChar);
        r = GUI_GetCharDistX(Char);
        GUI_UNLOCK_H(pText);
        return r;
      }
    } else {
      *pIsRTL = 0;
      pText  = (const char *)GUI_LOCK_H(pObj->hText);
      pText += pObj->CursorPosByte;
      r = _GetCharDistX(pObj, pText);
      GUI_UNLOCK_H(pText);
      return r;
    }
  } else {
    if (GUI__BIDI_Enabled == 1) {
      pText  = (const char *)GUI_LOCK_H(pObj->hText);
      *pIsRTL = _pfGUI__BIDI_GetCharDir(pText, pObj->NumChars, pObj->CursorPosChar);
      GUI_UNLOCK_H(pText);
    }
    return 2;
  }
}

/*********************************************************************
*
*       static code, buffer management
*
**********************************************************************
*/
/*********************************************************************
*
*       _IncrementBuffer
*
* Increments the buffer size by AddBytes.
*/

static int _IncrementBuffer(MULTIEDIT_HANDLE hObj, unsigned AddBytes) {
  MULTIEDIT_OBJ * pObj;
  WM_HMEM hNew;
  WM_HMEM hText;
  int NewSize;
  pObj = MULTIEDIT_LOCK_H(hObj);
  hText = pObj->hText;
  NewSize = pObj->BufferSize + AddBytes;
  GUI_UNLOCK_H(pObj);
  hNew    = GUI_ALLOC_Realloc(hText, NewSize);
  if (hNew) {
    pObj = MULTIEDIT_LOCK_H(hObj);
    if (!(pObj->hText)) {
      char * pText;
      pText  = (char *)GUI_LOCK_H(hNew);
      *pText = 0;
      GUI_UNLOCK_H(pText);
    }
    pObj->BufferSize = NewSize;
    pObj->hText = hNew;
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
static int _IsSpaceInBuffer(MULTIEDIT_HANDLE hObj, int BytesNeeded) {
  MULTIEDIT_OBJ * pObj;
  int NumBytes = 0;
  pObj = MULTIEDIT_LOCK_H(hObj);
  if (pObj->hText) {
    char * pText;
    pText  = (char *)GUI_LOCK_H(pObj->hText);
    NumBytes = strlen(pText);
    GUI_UNLOCK_H(pText);
  }
  BytesNeeded = (BytesNeeded + NumBytes + 1) - pObj->BufferSize;
  GUI_UNLOCK_H(pObj);
  if (BytesNeeded > 0) {
    if (!_IncrementBuffer(hObj, BytesNeeded + MULTIEDIT_REALLOC_SIZE)) {
      return 0;
    }
  }
  return 1;
}

/*********************************************************************
*
*       _IsCharsAvailable
*
* Checks weither the maximum number of characters is reached or not.
*
* Returns:
*  1 = requested number of chars is available
*  0 = maximum number of chars have reached
*/
static int _IsCharsAvailable(MULTIEDIT_HANDLE hObj, int CharsNeeded) {
  MULTIEDIT_OBJ * pObj;
  int r = 1;
  pObj = MULTIEDIT_LOCK_H(hObj);
  if ((CharsNeeded > 0) && (pObj->MaxNumChars > 0)) {
    int NumChars = 0;
    if (pObj->hText) {
      NumChars = _GetNumChars(pObj);
    }
    if ((CharsNeeded + NumChars) > pObj->MaxNumChars) {
      r = 0;
    }
  }
  GUI_UNLOCK_H(pObj);
  return r;
}

/*********************************************************************
*
*       static code, string manipulation routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _DeleteChar
*
* Deletes a character at the current cursor position and moves
* all bytes after the cursor position.
*/
static void _DeleteChar(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  pObj = MULTIEDIT_LOCK_H(hObj);
  if (pObj->hText) {
    unsigned CursorOffset;
    char * s;
    s = (char *)GUI_LOCK_H(pObj->hText);
    CursorOffset = pObj->CursorPosByte;
    if (CursorOffset < strlen(s)) {
      char *pCursor, *pLine, *pEndLine;
      int CursorLine, NumChars, NumBytes;
      pCursor    = s + CursorOffset;
      CursorLine = pObj->CursorLine;
      pLine      = _GetpLineLocked(hObj, pObj, CursorLine);
      NumChars   = _WrapGetNumCharsDisp(hObj, pObj, pLine);
      pEndLine   = pLine + GUI_UC__NumChars2NumBytes(pLine, NumChars);
      pLine      = pLine + _WrapGetNumBytesToNextLine(hObj, pObj, pLine);
      if (pCursor == pEndLine) {
        NumBytes = pLine - pEndLine;
      } else {
        NumBytes = GUI_UC_GetCharSize(pCursor);
      }
      GUI_UNLOCK_H(pLine);
      NumChars = GUI_UC__NumBytes2NumChars(pCursor, NumBytes);
      strcpy(pCursor, pCursor + NumBytes);
      GUI_UNLOCK_H(pObj);
      WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
      pObj = MULTIEDIT_LOCK_H(hObj);
      pObj->NumChars -= NumChars;
      _InvalidateNumLines(pObj);
      _InvalidateTextSizeX(pObj);
      _InvalidateCursorXY(pObj); /* Invalidate X/Y position */
      _ClearCache(pObj);
      pObj->CursorLine = _GetCursorLine(hObj, pObj, s, pObj->CursorPosChar);
    }
    GUI_UNLOCK_H(s);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _InsertChar
*
* Create space at the current cursor position and inserts a character.
*/
static int _InsertChar(MULTIEDIT_HANDLE hObj, U16 Char) {
  MULTIEDIT_OBJ * pObj;
  if (_IsCharsAvailable(hObj, 1)) {
    int BytesNeeded;
    BytesNeeded = GUI_UC__CalcSizeOfChar(Char);
    if (_IsSpaceInBuffer(hObj, BytesNeeded)) {
      int CursorOffset;
      char * pText;
      pObj = MULTIEDIT_LOCK_H(hObj);
      pText = (char *)GUI_LOCK_H(pObj->hText);
      CursorOffset = pObj->CursorPosByte;
      pText += CursorOffset;
      memmove(pText + BytesNeeded, pText, strlen(pText) + 1);
      GUI_UC_Encode(pText, Char);
      GUI_UNLOCK_H(pText);
      pObj->NumChars += 1;
      _InvalidateNumLines(pObj);
      _InvalidateTextSizeX(pObj);
      _ClearCache(pObj);
      GUI_UNLOCK_H(pObj);
      WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
      return 1;
    }
  }
  return 0;
}

/*********************************************************************
*
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _InvertCursor
*/
static void _InvertCursor(MULTIEDIT_HANDLE hObj) {
  MULTIEDIT_OBJ * pObj;
  int x, y,IsRTL, xOff, yOff, FontSizeY, CursorSize, ScrollPosX, ScrollPosY;
  GUI_RECT r;
  IsRTL      = 0;
  pObj = MULTIEDIT_LOCK_H(hObj);
  FontSizeY  = GUI_GetYSizeOfFont(pObj->pFont);
  ScrollPosX = pObj->ScrollStateH.v;
  ScrollPosY = pObj->ScrollStateV.v;
  if (WM_HasFocus(hObj)) {
    xOff       = pObj->Widget.pEffect->EffectSize + pObj->HBorder - ScrollPosX;
    yOff       = pObj->Widget.pEffect->EffectSize - ScrollPosY * FontSizeY;
    _GetCursorXY(hObj, pObj, &x, &y);
    CursorSize = _GetCursorSizeX(hObj, pObj, &IsRTL);
    if (IsRTL) {
      r.x0 = x + xOff - CursorSize;
      r.x1 = r.x0 + CursorSize - 1;
    } else {
      r.x0 = x + xOff;
      r.x1 = r.x0 + CursorSize - 1;
    }
    r.y0 = y + yOff;
    r.y1 = r.y0 + FontSizeY - 1;
    if (pObj->Align == GUI_TA_RIGHT) {
      r.x0 += ScrollPosX << 1;
      r.x1 += ScrollPosX << 1;
    }
    GUI_InvertRect(r.x0, r.y0, r.x1, r.y1);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _MULTIEDIT_Paint
*/
static void _MULTIEDIT_Paint(MULTIEDIT_HANDLE hObj, MULTIEDIT_OBJ * pObj) {
  int ScrollPosX, ScrollPosY, EffectSize, HBorder;
  int xOff, ColorIndex, FontSizeY;
  GUI_RECT r, rClip;
  const GUI_RECT * prOldClip;
  /* Init some values */
  GUI_SetFont(pObj->pFont);
  FontSizeY  = GUI_GetFontDistY();
  ScrollPosX = pObj->ScrollStateH.v;
  ScrollPosY = pObj->ScrollStateV.v;
  EffectSize = pObj->Widget.pEffect->EffectSize;
  HBorder    = pObj->HBorder;
  xOff       = EffectSize + HBorder - ScrollPosX;
  ColorIndex = ((pObj->Flags & MULTIEDIT_SF_READONLY) ? 1 : 0);
  /* Set colors and draw the background */
  LCD_SetBkColor(pObj->aBkColor[ColorIndex]);
  LCD_SetColor(pObj->aColor[ColorIndex]);
  GUI_Clear();
  /* Draw the text if necessary */
  rClip.x0 = EffectSize + HBorder;
  rClip.y0 = EffectSize;
  rClip.x1 = WM_GetWindowSizeX(hObj) - EffectSize - HBorder - 1;
  rClip.y1 = WM_GetWindowSizeY(hObj) - EffectSize - 1;
  prOldClip = WM_SetUserClipRect(&rClip);
  if (pObj->hText) {
    const char * pText;
    int Line = 0;
    int xSize       = _GetXSize(hObj, pObj);
    int NumVisLines = _GetNumVisLines(hObj, pObj);
    /* Get the text */
    pText = (const char *)GUI_LOCK_H(pObj->hText);
    /* Set the rectangle for drawing */
    r.x0 = xOff;
    r.y0 = EffectSize;
    r.x1 = xSize + EffectSize + HBorder - 1;
    r.y1 = pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0 + 1;
    /* Use cached position of first visible byte if possible */
    if (ScrollPosY >= pObj->CacheFirstVisibleLine) {
      if (pObj->CacheFirstVisibleByte) {
        pText += pObj->CacheFirstVisibleByte;
        Line   = pObj->CacheFirstVisibleLine;
      }
    }
    if (pObj->Align == GUI_TA_RIGHT) {
      r.x0 += ScrollPosX;
      r.x1 += ScrollPosX;
    }
    /* Do the drawing of the text */
    do {
      /* Cache the position of the first visible byte and the depending line number */
      if (pObj->CacheFirstVisibleLine != ScrollPosY) {
        if (Line == ScrollPosY) {
          const char * p;
          p = (const char *)GUI_LOCK_H(pObj->hText);
          pObj->CacheFirstVisibleByte = pText - p;
          pObj->CacheFirstVisibleLine = ScrollPosY;
          GUI_UNLOCK_H(p);
        }
      }
      /* Draw it */
      if ((Line >= ScrollPosY) && ((Line - ScrollPosY) <= NumVisLines)) {
        _DispString(hObj, pObj, pText, &r);
        r.y0 += FontSizeY;  /* Next line */
      }
      pText += _WrapGetNumBytesToNextLine(hObj, pObj, pText);
      Line++;
    } while (GUI_UC_GetCharCode(pText) && ((Line - ScrollPosY) <= NumVisLines));
    GUI_UNLOCK_H(pText);
  }
  /* Draw cursor if necessary */
  if (WM__IsEnabled(hObj)) {
    _InvertCursor(hObj);
    WM_RestartTimer(pObj->hTimer, 0);
  }
  WM_SetUserClipRect(prOldClip);
  /* Draw the 3D effect (if configured) */
  WIDGET__EFFECT_DrawDown(&pObj->Widget);
}

/*********************************************************************
*
*       _OnTouch
*/
static void _OnTouch(MULTIEDIT_HANDLE hObj, MULTIEDIT_OBJ * pObj, WM_MESSAGE * pMsg) {
  int Notification;
  const GUI_PID_STATE * pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
    if (pState->Pressed) {
      int Effect, xPos, yPos;
      Effect = pObj->Widget.pEffect->EffectSize;
      xPos   = pState->x + pObj->ScrollStateH.v - Effect - pObj->HBorder;
      yPos   = pState->y + pObj->ScrollStateV.v * GUI_GetYDistOfFont(pObj->pFont) - Effect;
      _SetCursorXY(hObj, pObj, xPos, yPos);
      _Invalidate(hObj);
      Notification = WM_NOTIFICATION_CLICKED;
    } else {
      Notification = WM_NOTIFICATION_RELEASED;
    }
  } else {
    Notification = WM_NOTIFICATION_MOVED_OUT;
  }
  WM_NotifyParent(hObj, Notification);
}

/*********************************************************************
*
*       _AddKey
*
* Returns: 1 if Key has been consumed
*          0 else
*/
static int _AddKey(MULTIEDIT_HANDLE hObj, U16 Key) {
  int r = 0;               /* Key has not been consumed */
  const char * pText;
  GUI_RECT RectInvalid;
  int NoScroll;
  int OldCursorPosY;
  int OldCharSizeX;
  MULTIEDIT_OBJ * pObj;
  int OldScrollPosV;
  int OldScrollPosH;
  U16 CursorPosChar;
  U16 NumCharsPrompt;
  U8 Flags;
  int OldCursorPosX;
  int NoChanges = 0;

  if (WM__IsEnabled(hObj) == 0) {
    /* The following makes sure, that the contents of the widget can be scrolled
     * with the keys GUI_KEY_UP and GUI_KEY_DOWN also in disabled state if a
     * vertical scrollbar exists
     */
    void (* pScrollFunc)(WM_HWIN);
    switch (Key) {
    case GUI_KEY_UP:
      pScrollFunc = SCROLLBAR_Dec;
      break;
    case GUI_KEY_DOWN:
      pScrollFunc = SCROLLBAR_Inc;
      break;
    default:
      pScrollFunc = NULL;
    }
    if (pScrollFunc) {
      pScrollFunc(WM_GetScrollbarV(hObj));
      return 1;
    }
    return 0;
  }
  pObj = MULTIEDIT_LOCK_H(hObj);
  pText = (const char *)GUI_LOCK_H(pObj->hText);
  if (GUI__BIDI_Enabled == 0) {
    OldCursorPosX = pObj->CursorPosX - pObj->ScrollStateH.v;
  }
  OldCursorPosY = pObj->CursorPosY - pObj->ScrollStateV.v * GUI_GetYSizeOfFont(pObj->pFont);
  OldCharSizeX  = GUI_GetCharDistX(GUI_UC_GetCharCode(pText + pObj->CursorPosByte));
  OldScrollPosV = pObj->ScrollStateV.v;
  OldScrollPosH = pObj->ScrollStateH.v;
  CursorPosChar = pObj->CursorPosChar;
  NumCharsPrompt = pObj->NumCharsPrompt;
  Flags = pObj->Flags;
  GUI_UNLOCK_H(pText);
  if (!OldCharSizeX) {
    int IsRTL;
    OldCharSizeX = _GetCursorSizeX(hObj, pObj, &IsRTL);
  }
  GUI_UNLOCK_H(pObj);
  switch (Key) {
  case GUI_KEY_UP:
    _MoveCursorUp(hObj);
    if (GUI__BIDI_Enabled == 0) {
      NoChanges = 1;
    }
    r = 1;               /* Key has been consumed */
    break;
  case GUI_KEY_DOWN:
    _MoveCursorDown(hObj);
    if (GUI__BIDI_Enabled == 0) {
      NoChanges = 1;
    }
    r = 1;               /* Key has been consumed */
    break;
  case GUI_KEY_RIGHT:
    _SetCursorPos(hObj, CursorPosChar + 1);
    if (GUI__BIDI_Enabled == 0) {
      NoChanges = 1;
    }
    r = 1;               /* Key has been consumed */
    break;
  case GUI_KEY_LEFT:
    _SetCursorPos(hObj, CursorPosChar - 1);
    if (GUI__BIDI_Enabled == 0) {
      NoChanges = 1;
    }
    r = 1;               /* Key has been consumed */
    break;
  case GUI_KEY_END:
    _MoveCursor2LineEnd(hObj);
    if (GUI__BIDI_Enabled == 0) {
      NoChanges = 1;
    }
    r = 1;               /* Key has been consumed */
    break;
  case GUI_KEY_HOME:
    _MoveCursor2LinePos1(hObj);
    if (GUI__BIDI_Enabled == 0) {
      NoChanges = 1;
    }
    r = 1;               /* Key has been consumed */
    break;
  case GUI_KEY_BACKSPACE:
    if (!(Flags & MULTIEDIT_SF_READONLY)) {
      if (CursorPosChar > NumCharsPrompt) {
        _SetCursorPos(hObj, CursorPosChar - 1);
        _DeleteChar(hObj);
      }
      r = 1;               /* Key has been consumed */
    }
    break;
  case GUI_KEY_DELETE:
    if (!(Flags & MULTIEDIT_SF_READONLY)) {
      _DeleteChar(hObj);
      r = 1;               /* Key has been consumed */
    }
    break;
  case GUI_KEY_INSERT:
    pObj = MULTIEDIT_LOCK_H(hObj);
    if (!(Flags & MULTIEDIT_CF_INSERT)) {
      pObj->Flags |= MULTIEDIT_CF_INSERT;
    } else {
      pObj->Flags &= ~MULTIEDIT_CF_INSERT;
    }
    GUI_UNLOCK_H(pObj);
    if (GUI__BIDI_Enabled == 0) {
      NoChanges = 1;
    }
    r = 1;               /* Key has been consumed */
    break;
  case GUI_KEY_ENTER:
    if (Flags & MULTIEDIT_SF_READONLY) {
      _MoveCursor2NextLine(hObj);
    } else {
      if (_InsertChar(hObj, (U8)('\n'))) {
        if (Flags & MULTIEDIT_SF_PASSWORD) {
          _SetCursorPos(hObj, CursorPosChar + 1);
        } else {
          _MoveCursor2NextLine(hObj);
        }
      }
    }
    r = 1;               /* Key has been consumed */
    break;
  case GUI_KEY_ESCAPE:
    break;
  default:
    if (!(Flags & MULTIEDIT_SF_READONLY) && (Key >= 0x20)) {
      if (_IsOverwriteAtThisChar(hObj)) {
        _DeleteChar(hObj); /* Pointer recalculation required, will be done later in this routine */
      }
      if (_InsertChar(hObj, Key)) {
        _SetCursorPos(hObj, CursorPosChar + 1);
      }
      r = 1;               /* Key has been consumed */
    }
  }
  /* Calculate the invalid rectangle */
  _ManageScrollers(hObj);
  WM_GetInsideRectExScrollbar(hObj, &RectInvalid); /* Initialize invalid rectangle */
  /* Check if the scroll position has been changed */
  pObj = MULTIEDIT_LOCK_H(hObj);
  if ((pObj->ScrollStateH.v == OldScrollPosH) && (pObj->ScrollStateV.v == OldScrollPosV)) {
    NoScroll = 1;
  } else {
    NoScroll = 0;
  }


  if ((GUI__BIDI_Enabled == 0) && (NoChanges == 1) && (NoScroll == 1)) {
    /* If only the cursor has been moved, the invalid
     * rectangle is the surrounding rectangle of the old
     * and new position of the cursor
     */
    int EffectSize, HBorder, FontSizeY, CharSizeX;
    int CharRectX0, CharRectX1, CharRectY0, OldCharRectX1;
    pText = (const char *)GUI_LOCK_H(pObj->hText);
    EffectSize = pObj->Widget.pEffect->EffectSize;
    HBorder    = pObj->HBorder;
    FontSizeY  = GUI_GetYSizeOfFont(pObj->pFont);
    CharSizeX  = GUI_GetCharDistX(GUI_UC_GetCharCode(pText + pObj->CursorPosByte));
    if (!CharSizeX) {
      int IsRTL;
      CharSizeX = _GetCursorSizeX(hObj, pObj, &IsRTL);
    }
    /* Calculate leftmost pixel of invalid rectangle */
    CharRectX0 = pObj->CursorPosX - pObj->ScrollStateH.v;
    if (OldCursorPosX < CharRectX0) {
      RectInvalid.x0 = OldCursorPosX;
    } else {
      RectInvalid.x0 = CharRectX0;
    }
    RectInvalid.x0 += EffectSize + HBorder;
    /* Calculate rightmost pixel of invalid rectangle */
    CharRectX1    = CharRectX0 + CharSizeX - 1;
    OldCharRectX1 = OldCursorPosX + OldCharSizeX - 1;
    if (OldCharRectX1 > CharRectX1) {
      RectInvalid.x1 = OldCharRectX1;
    } else {
      RectInvalid.x1 = CharRectX1;
    }
    RectInvalid.x1 += EffectSize + HBorder;
    /* Calculate topmost pixel of invalid rectangle */
    CharRectY0 = pObj->CursorPosY - pObj->ScrollStateV.v * FontSizeY;
    if (OldCursorPosY < CharRectY0) {
      RectInvalid.y0 = OldCursorPosY;
    } else {
      RectInvalid.y0 = CharRectY0;
    }
    RectInvalid.y0 += EffectSize;
    /* Calculate bottommost pixel of invalid rectangle */
    if (OldCursorPosY > CharRectY0) {
      RectInvalid.y1 = FontSizeY - 1 + OldCursorPosY;
    } else {
      RectInvalid.y1 = FontSizeY - 1 + CharRectY0;
    }
    RectInvalid.y1 += EffectSize;
    GUI_UNLOCK_H(pText);
  } else {
    if ((GUI__BIDI_Enabled == 0) || (NoScroll)) {
      /* If the contents has been changed and no scroll has been detected, 
       * invalidate all lines from the topmost cursor position until the 
       * bottom of the widget.
       */
      int CharRectY0;
      CharRectY0 = pObj->CursorPosY - pObj->ScrollStateV.v * GUI_GetYSizeOfFont(pObj->pFont);
      if (OldCursorPosY < CharRectY0) {
        RectInvalid.y0 = OldCursorPosY;
      } else {
        RectInvalid.y0 = CharRectY0;
      }
    }
  }
  GUI_UNLOCK_H(pObj);
  WM_InvalidateRect(hObj, &RectInvalid);
  return r;
}

/*********************************************************************
*
*       _SetText
*/
static void _SetText(MULTIEDIT_HANDLE hObj, const char * pNew) {
  MULTIEDIT_OBJ * pObj;
  int NumCharsNew, NumCharsOld, NumBytesNew, NumBytesOld;
  char * pText;
  pObj = MULTIEDIT_LOCK_H(hObj);
  NumCharsNew = NumCharsOld = NumBytesNew = NumBytesOld = 0;
  if (pObj->hText) {
    pText  = (char *)GUI_LOCK_H(pObj->hText);
    pText += GUI_UC__NumChars2NumBytes(pText, pObj->NumCharsPrompt);
    NumCharsOld = GUI__GetNumChars(pText);
    NumBytesOld = GUI_UC__NumChars2NumBytes(pText, NumCharsOld);
    GUI_UNLOCK_H(pText);
  }
  if (pNew) {
    NumCharsNew = GUI__GetNumChars(pNew);
    NumBytesNew = GUI_UC__NumChars2NumBytes(pNew, NumCharsNew);
  }
  GUI_UNLOCK_H(pObj);
  if (_IsCharsAvailable(hObj, NumCharsNew - NumCharsOld)) {
    if (_IsSpaceInBuffer(hObj, NumBytesNew - NumBytesOld)) {
      U16 NumCharsPrompt;
      pObj = MULTIEDIT_LOCK_H(hObj);
      pText  = (char *)GUI_LOCK_H(pObj->hText);
      pText += GUI_UC__NumChars2NumBytes(pText, pObj->NumCharsPrompt);
      if (pNew) {
        strcpy(pText, pNew);
      } else {
        *pText = 0;
      }
      NumCharsPrompt = pObj->NumCharsPrompt;
      GUI_UNLOCK_H(pText);
      GUI_UNLOCK_H(pObj);
      _SetCursorPos(hObj, NumCharsPrompt);
      pObj = MULTIEDIT_LOCK_H(hObj);
      _InvalidateNumChars(pObj);
      _InvalidateNumLines(pObj);
      _InvalidateTextSizeX(pObj);
      _InvalidateCursorXY(pObj);
      GUI_UNLOCK_H(pObj);
      _InvalidateTextArea(hObj);
    }
  }
}

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       MULTIEDIT_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
MULTIEDIT_OBJ * MULTIEDIT_LockH(MULTIEDIT_HANDLE h) {
  MULTIEDIT_OBJ * p = (MULTIEDIT_OBJ *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != MULTIEDIT_ID) {
      GUI_DEBUG_ERROROUT("MULTIEDIT.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       Exported routines:  Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       MULTIEDIT_Callback
*/
void MULTIEDIT_Callback (WM_MESSAGE *pMsg) {
  MULTIEDIT_HANDLE hObj;
  MULTIEDIT_OBJ * pObj;
  WM_SCROLL_STATE ScrollState;
  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  pObj = (MULTIEDIT_OBJ *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
  switch (pMsg->MsgId) {
  case WM_TIMER:
    WM_SelectWindow(hObj);
    _InvertCursor(hObj);
    WM_RestartTimer(pObj->hTimer, 0);
    break;
  case WM_NOTIFY_CLIENTCHANGE:
    _InvalidateCursorXY(pObj);
    _InvalidateNumLines(pObj);
    _InvalidateTextSizeX(pObj);
    _ClearCache(pObj);
    GUI_UNLOCK_H(pObj);
    _CalcScrollParas(hObj);
    break;
  case WM_SIZE:
    _InvalidateCursorXY(pObj);
    _InvalidateNumLines(pObj);
    _InvalidateTextSizeX(pObj);
    _ClearCache(pObj);
    GUI_UNLOCK_H(pObj);
    _Invalidate(hObj);
    break;
  case WM_NOTIFY_PARENT:
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_VALUE_CHANGED:
      if (pMsg->hWinSrc  == WM_GetScrollbarV(hObj)) {
        WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
        pObj->ScrollStateV.v = ScrollState.v;
        GUI_UNLOCK_H(pObj);
        WM_InvalidateWindow(hObj);
        WM_NotifyParent(hObj, WM_NOTIFICATION_SCROLL_CHANGED);
      } else if (pMsg->hWinSrc == WM_GetScrollbarH(hObj)) {
        WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
        pObj->ScrollStateH.v = ScrollState.v;
        GUI_UNLOCK_H(pObj);
        WM_InvalidateWindow(hObj);
        WM_NotifyParent(hObj, WM_NOTIFICATION_SCROLL_CHANGED);
      }
      break;
    case WM_NOTIFICATION_SCROLLBAR_ADDED:
      #if WIDGET_USE_PARENT_EFFECT
        WIDGET_SetEffect(pMsg->hWinSrc, pObj->Widget.pEffect);
      #endif
      GUI_UNLOCK_H(pObj);
      _SetScrollState(hObj);
      break;
    }
    break;
  case WM_PAINT:
    _MULTIEDIT_Paint(hObj, pObj);
    GUI_UNLOCK_H(pObj);
    return;
  case WM_TOUCH:
    _OnTouch(hObj, pObj, pMsg);
    break;
  case WM_DELETE:
    GUI_ALLOC_FreePtr(&pObj->hText);
    break;
  case WM_KEY:
    if (((const WM_KEY_INFO*)(pMsg->Data.p))->PressedCnt >0) {
      int Key = ((const WM_KEY_INFO*)(pMsg->Data.p))->Key;
      /* Leave code for test purpose
      switch (Key) {
      case '4': Key = GUI_KEY_LEFT;  break;
      case '8': Key = GUI_KEY_UP;    break;
      case '6': Key = GUI_KEY_RIGHT; break;
      case '2': Key = GUI_KEY_DOWN;  break;
      }
      */
      GUI_UNLOCK_H(pObj);
      if (_AddKey(hObj, Key)) {
        return;
      }
    } else {
      if (!(pObj->Flags & MULTIEDIT_SF_READONLY)) {
        GUI_UNLOCK_H(pObj);
        return;                /* Key release is consumed (not sent to parent) */
      }
    }
  }
  if (pObj) {
    GUI_UNLOCK_H(pObj);
  }
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       Exported routines:  Create
*
**********************************************************************
*/

/* Note: the parameters to a create function may vary.
         Some widgets may have multiple create functions */

/*********************************************************************
*
*       MULTIEDIT_CreateEx
*/
MULTIEDIT_HANDLE MULTIEDIT_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int WinFlags, int ExFlags,
                                    int Id, int BufferSize, const char * pText)
{
  MULTIEDIT_HANDLE hObj;
  /* Create the window */
  WM_LOCK();
  if ((xsize == 0) && (ysize == 0) && (x0 == 0) && (y0 == 0)) {
    GUI_RECT Rect;
    WM_GetClientRectEx(hParent, &Rect);
    xsize = Rect.x1 - Rect.x0 + 1;
    ysize = Rect.y1 - Rect.y0 + 1;
  }
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, &MULTIEDIT_Callback,
                                sizeof(MULTIEDIT_OBJ) - sizeof(WM_Obj));
  if (hObj) {
    int i;
    MULTIEDIT_OBJ * pObj;
    pObj = (MULTIEDIT_OBJ *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
    /* init widget specific variables */
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
    /* init member variables */
    MULTIEDIT_INIT_ID(pObj);
    for (i = 0; i < NUM_DISP_MODES; i++) {
      pObj->aBkColor[i]  = _aDefaultBkColor[i];
      pObj->aColor[i]    = _aDefaultColor[i];
    }
    pObj->pFont          = _pDefaultFont;
    pObj->Flags          = ExFlags;
    pObj->CursorPosChar  = 0;
    pObj->CursorPosByte  = 0;
    pObj->HBorder        = 1;
    pObj->MaxNumChars    = 0;
    pObj->NumCharsPrompt = 0;
    pObj->BufferSize     = 0;
    pObj->hText          = 0;
    GUI_UNLOCK_H(pObj);
    if (BufferSize > 0) {
      WM_HWIN hText;
      hText = GUI_ALLOC_AllocZero(BufferSize);
      if (hText != 0) {
        pObj = MULTIEDIT_LOCK_H(hObj);
        pObj->BufferSize = BufferSize;
        pObj->hText      = hText;
        GUI_UNLOCK_H(pObj);
      } else {
        GUI_DEBUG_ERROROUT("MULTIEDIT_CreateEx failed to alloc buffer");
        WM_DeleteWindow(hObj);
        hObj = 0;
      }
    }
    MULTIEDIT_SetText(hObj, pText);
    _ManageScrollers(hObj);
  } else {
    GUI_DEBUG_ERROROUT_IF(hObj == 0, "MULTIEDIT_CreateEx failed")
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
*       MULTIEDIT_AddKey
*/
int MULTIEDIT_AddKey(MULTIEDIT_HANDLE hObj, U16 Key) {
  int r = 0;
  if (hObj) {
    WM_LOCK();
    r = _AddKey(hObj, Key);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       MULTIEDIT_AddText
*/
int MULTIEDIT_AddText(MULTIEDIT_HANDLE hObj, const char * s) {
  int Result;
  Result = 1;
  if (hObj && s) {
    MULTIEDIT_OBJ * pObj;
    WM_HMEM hText;
    U16 MaxNumChars;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    hText = pObj->hText;
    MaxNumChars = pObj->MaxNumChars;
    GUI_UNLOCK_H(pObj);
    if (!hText) {
      _SetText(hObj, s);
    } else {
      char * pText;
      int NumCharsNew, NumCharsOld, NumBytesNew, NumBytesOld;
      pText = (char *)GUI_LOCK_H(hText);
      NumCharsOld = GUI__GetNumChars(pText);
      NumBytesOld = GUI_UC__NumChars2NumBytes(pText, NumCharsOld);
      NumCharsNew = GUI__GetNumChars(s);
      if (MaxNumChars > 0) {
        if ((NumCharsOld + NumCharsNew) > MaxNumChars) {
          NumCharsNew = MaxNumChars - NumCharsOld;
        }
      }
      GUI_UNLOCK_H(pText);
      if (NumCharsNew > 0) {
        NumBytesNew = GUI_UC__NumChars2NumBytes(s, NumCharsNew);
        if (_IsSpaceInBuffer(hObj, NumBytesNew)) {
          U16 CursorPosChar;
          pObj = MULTIEDIT_LOCK_H(hObj);
          CursorPosChar = pObj->CursorPosChar;
          pText = (char *)GUI_LOCK_H(pObj->hText);
          memmove(pText + pObj->CursorPosByte + NumBytesNew,
                  pText + pObj->CursorPosByte,
                  NumBytesOld - pObj->CursorPosByte);
          GUI_MEMCPY(pText + pObj->CursorPosByte, s, NumBytesNew);
          *(pText + NumBytesOld + NumBytesNew) = 0;
          GUI_UNLOCK_H(pText);
          pObj->NumChars += NumCharsNew;
          GUI_UNLOCK_H(pObj);
          _SetCursorPos(hObj, CursorPosChar + NumCharsNew);
          pObj = MULTIEDIT_LOCK_H(hObj);
          _InvalidateNumLines(pObj);
          _InvalidateTextSizeX(pObj);
          GUI_UNLOCK_H(pObj);
          _InvalidateTextArea(hObj);
          pObj = MULTIEDIT_LOCK_H(hObj);
          _InvalidateCursorXY(pObj);
          _ClearCache(pObj);
          GUI_UNLOCK_H(pObj);
          WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
          Result = 0;
        }
      }
    }
    WM_UNLOCK();
  }
  return Result;
}

/*********************************************************************
*
*       MULTIEDIT_SetText
*/
void MULTIEDIT_SetText(MULTIEDIT_HANDLE hObj, const char * pNew) {
  if (hObj) {
    WM_LOCK();
    _SetText(hObj, pNew);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_GetText
*/
void MULTIEDIT_GetText(MULTIEDIT_HANDLE hObj, char * sDest, int MaxLen) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    char * pText;
    int Len;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    pText  = (char *)GUI_LOCK_H(pObj->hText);
    pText += GUI_UC__NumChars2NumBytes(pText, pObj->NumCharsPrompt);
    Len    = strlen(pText);
    if (Len > (MaxLen - 1)) {
      Len = MaxLen - 1;
    }
    GUI_MEMCPY(sDest, pText, Len);
    *(sDest + Len) = 0;
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK_H(pText);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_GetPrompt
*/
void MULTIEDIT_GetPrompt(MULTIEDIT_HANDLE hObj, char * sDest, int MaxLen) {
  if (hObj) {
    char * sSource;
    int Len;
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    sSource = (char *)GUI_LOCK_H(pObj->hText);
    Len = GUI_UC__NumChars2NumBytes(sSource, pObj->NumCharsPrompt);
    if (Len > (MaxLen - 1)) {
      Len = MaxLen - 1;
    }
    GUI_MEMCPY(sDest, sSource, Len);
    *(sDest + Len) = 0;
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK_H(sSource);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_SetWrapWord
*/
void MULTIEDIT_SetWrapWord(MULTIEDIT_HANDLE hObj) {
  _SetWrapMode(hObj, GUI_WRAPMODE_WORD);
}

/*********************************************************************
*
*       MULTIEDIT_SetWrapChar
*/
void MULTIEDIT_SetWrapChar(MULTIEDIT_HANDLE hObj) {
  _SetWrapMode(hObj, GUI_WRAPMODE_CHAR);
}

/*********************************************************************
*
*       MULTIEDIT_SetWrapNone
*/
void MULTIEDIT_SetWrapNone(MULTIEDIT_HANDLE hObj) {
  _SetWrapMode(hObj, GUI_WRAPMODE_NONE);
}

/*********************************************************************
*
*       MULTIEDIT_SetInsertMode
*/
void MULTIEDIT_SetInsertMode(MULTIEDIT_HANDLE hObj, int OnOff) {
  _SetFlag(hObj, OnOff, MULTIEDIT_SF_INSERT);
}

/*********************************************************************
*
*       MULTIEDIT_SetReadOnly
*/
void MULTIEDIT_SetReadOnly(MULTIEDIT_HANDLE hObj, int OnOff) {
  _SetFlag(hObj, OnOff, MULTIEDIT_SF_READONLY);
}

/*********************************************************************
*
*       MULTIEDIT_SetPasswordMode
*/
void MULTIEDIT_SetPasswordMode(MULTIEDIT_HANDLE hObj, int OnOff) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    _SetFlag(hObj, OnOff, MULTIEDIT_SF_PASSWORD);
    pObj = MULTIEDIT_LOCK_H(hObj);
    _InvalidateCursorXY(pObj);
    _InvalidateNumLines(pObj);
    _InvalidateTextSizeX(pObj);
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_SetAutoScrollV
*/
void MULTIEDIT_SetAutoScrollV(MULTIEDIT_HANDLE hObj, int OnOff) {
  _SetFlag(hObj, OnOff, MULTIEDIT_SF_AUTOSCROLLBAR_V);
}

/*********************************************************************
*
*       MULTIEDIT_SetAutoScrollH
*/
void MULTIEDIT_SetAutoScrollH(MULTIEDIT_HANDLE hObj, int OnOff) {
  _SetFlag(hObj, OnOff, MULTIEDIT_SF_AUTOSCROLLBAR_H);
}

/*********************************************************************
*
*       MULTIEDIT_SetHBorder
*/
void MULTIEDIT_SetHBorder(MULTIEDIT_HANDLE hObj, unsigned HBorder) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    if ((unsigned)pObj->HBorder != HBorder) {
      pObj->HBorder = HBorder;
      GUI_UNLOCK_H(pObj);
      _Invalidate(hObj);
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_SetFont
*/
void MULTIEDIT_SetFont(MULTIEDIT_HANDLE hObj, const GUI_FONT GUI_UNI_PTR * pFont) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    if (pObj->pFont != pFont) {
      pObj->pFont = pFont;
      GUI_UNLOCK_H(pObj);
      _InvalidateTextArea(hObj);
      pObj = MULTIEDIT_LOCK_H(hObj);
      _InvalidateCursorXY(pObj);
      _InvalidateNumLines(pObj);
      _InvalidateTextSizeX(pObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_SetBkColor
*/
void MULTIEDIT_SetBkColor(MULTIEDIT_HANDLE hObj, unsigned Index, GUI_COLOR color) {
  if (hObj && (Index < NUM_DISP_MODES)) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    pObj->aBkColor[Index] = color;
    GUI_UNLOCK_H(pObj);
    _InvalidateTextArea(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_SetCursorOffset
*/
void MULTIEDIT_SetCursorOffset(MULTIEDIT_HANDLE hObj, int Offset) {
  if (hObj) {
    WM_LOCK();
    _SetCursorPos(hObj, Offset);
    WM_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_SetTextColor
*/
void MULTIEDIT_SetTextColor(MULTIEDIT_HANDLE hObj, unsigned Index, GUI_COLOR color) {
  if (hObj && (Index < NUM_DISP_MODES)) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    pObj->aColor[Index] = color;
    GUI_UNLOCK_H(pObj);
    WM_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_SetPrompt
*/
void MULTIEDIT_SetPrompt(MULTIEDIT_HANDLE hObj, const char * pPrompt) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    int NumCharsNew = 0, NumCharsOld = 0;
    int NumBytesNew = 0, NumBytesOld = 0;
    char * pText;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    if (pObj->hText) {
      pText = (char *)GUI_LOCK_H(pObj->hText);
      NumCharsOld = pObj->NumCharsPrompt;
      NumBytesOld = GUI_UC__NumChars2NumBytes(pText, NumCharsOld);
      GUI_UNLOCK_H(pText);
    }
    GUI_UNLOCK_H(pObj);
    if (pPrompt) {
      NumCharsNew = GUI__GetNumChars(pPrompt);
      NumBytesNew = GUI_UC__NumChars2NumBytes(pPrompt, NumCharsNew);
    }
    if (_IsCharsAvailable(hObj, NumCharsNew - NumCharsOld)) {
      if (_IsSpaceInBuffer(hObj, NumBytesNew - NumBytesOld)) {
        pObj = MULTIEDIT_LOCK_H(hObj);
        pText = (char *)GUI_LOCK_H(pObj->hText);
        memmove(pText + NumBytesNew, pText + NumBytesOld, strlen(pText + NumBytesOld) + 1);
        if (pPrompt) {
          GUI_MEMCPY(pText, pPrompt, NumBytesNew);
        }
        pObj->NumCharsPrompt = NumCharsNew;
        GUI_UNLOCK_H(pText);
        GUI_UNLOCK_H(pObj);
        _SetCursorPos(hObj, NumCharsNew);
        _InvalidateTextArea(hObj);
        pObj = MULTIEDIT_LOCK_H(hObj);
        _InvalidateNumChars(pObj);
        _InvalidateNumLines(pObj);
        _InvalidateTextSizeX(pObj);
        GUI_UNLOCK_H(pObj);
      }
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_SetBufferSize
*/
void MULTIEDIT_SetBufferSize(MULTIEDIT_HANDLE hObj, int BufferSize) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_HMEM hText;
    WM_LOCK();
    if ((hText = GUI_ALLOC_AllocZero(BufferSize)) == 0) {
      GUI_DEBUG_ERROROUT("MULTIEDIT_SetBufferSize failed to alloc buffer");
    } else {
      pObj = MULTIEDIT_LOCK_H(hObj);
      GUI_ALLOC_FreePtr(&pObj->hText);
      pObj->hText          = hText;
      pObj->BufferSize     = BufferSize;
      pObj->NumCharsPrompt = 0;
      _InvalidateNumChars(pObj);
      _InvalidateCursorXY(pObj);
      _InvalidateNumLines(pObj);
      _InvalidateTextSizeX(pObj);
      GUI_UNLOCK_H(pObj);
      _SetCursorPos(hObj, 0);
      _InvalidateTextArea(hObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_SetMaxNumChars
*/
void MULTIEDIT_SetMaxNumChars(MULTIEDIT_HANDLE hObj, unsigned MaxNumChars) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    pObj->MaxNumChars = MaxNumChars;
    if (MaxNumChars < (unsigned)pObj->NumCharsPrompt) {
      pObj->NumCharsPrompt = MaxNumChars;
    }
    if (pObj->hText && MaxNumChars) {
      char * pText;
      int Offset, LenText;
      unsigned NumCharsCur;
      pText = (char *)GUI_LOCK_H(pObj->hText);
      LenText = strlen(pText);
      NumCharsCur = GUI__GetLineNumChars(pText, LenText);
      if (NumCharsCur > MaxNumChars) {
        Offset = GUI_UC__NumChars2NumBytes(pText, MaxNumChars);
        if (Offset < pObj->BufferSize) {
          pText += Offset;
          *pText = 0;
          _InvalidateNumChars(pObj);
          GUI_UNLOCK_H(pText);
          GUI_UNLOCK_H(pObj);
          _SetCursorPos(hObj, Offset);
          _InvalidateTextArea(hObj);
        } else {
          GUI_UNLOCK_H(pText);
          GUI_UNLOCK_H(pObj);
        }
      } else {
        GUI_UNLOCK_H(pText);
        GUI_UNLOCK_H(pObj);
      }
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_GetTextSize
*
* Purpose:
*   Returns the number of bytes required to store the text.
*   It is typically used when allocating a buffer to pass to
*   MULTIEDIT_GetText().
*/
int MULTIEDIT_GetTextSize(MULTIEDIT_HANDLE hObj) {
  int r = 0;
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    if (pObj->hText) {
      const char * s;
      s = (const char *)GUI_LOCK_H(pObj->hText);
      s += GUI_UC__NumChars2NumBytes(s, pObj->NumCharsPrompt);
      r = 1 + strlen(s);
      GUI_UNLOCK_H(s);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}


/*********************************************************************
*
*       MULTIEDIT_SetTextAlign
*/
void MULTIEDIT_SetTextAlign(MULTIEDIT_HANDLE hObj, int Align) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    Align &= (GUI_TA_LEFT | GUI_TA_RIGHT);
    if (pObj->Align != Align) {
      pObj->Align = Align;
      GUI_UNLOCK_H(pObj);
      _InvalidateTextArea(hObj);
      pObj = MULTIEDIT_LOCK_H(hObj);
      _InvalidateCursorXY(pObj);
      _InvalidateTextSizeX(pObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_GetCursorCharPos
*/
int MULTIEDIT_GetCursorCharPos(MULTIEDIT_HANDLE hObj) {
  int r = 0;
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    r = pObj->CursorPosChar;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       MULTIEDIT_GetCursorPixelPos
*/
void MULTIEDIT_GetCursorPixelPos(MULTIEDIT_HANDLE hObj, int * pxPos, int * pyPos) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    int FontSizeY, EffectSize;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    FontSizeY  = GUI_GetYSizeOfFont(pObj->pFont);
    EffectSize = pObj->Widget.pEffect->EffectSize;
    _GetCursorXY(hObj, pObj, pxPos, pyPos);
    *pxPos += EffectSize - pObj->ScrollStateH.v + pObj->HBorder;
    *pyPos += EffectSize - pObj->ScrollStateV.v * FontSizeY;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MULTIEDIT_EnableBlink
*/
void MULTIEDIT_EnableBlink(MULTIEDIT_HANDLE hObj, int Period, int OnOff) {
  if (hObj) {
    MULTIEDIT_OBJ * pObj;
    WM_LOCK();
    pObj = MULTIEDIT_LOCK_H(hObj);
    if (OnOff) {
      WM_HTIMER hTimer;
      if (pObj->hTimer) {
        WM_RestartTimer(pObj->hTimer, Period);
      } else {
        GUI_UNLOCK_H(pObj);
        hTimer = WM_CreateTimer(hObj, 0, Period, 0);
        pObj = MULTIEDIT_LOCK_H(hObj);
        pObj->hTimer = hTimer;
      }
    } else {
      WM_DeleteTimer(pObj->hTimer);
      pObj->hTimer = 0;
      WM_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else /* avoid empty object files */

void MULTIEDIT_C(void);
void MULTIEDIT_C(void){}

#endif  /* #if GUI_WINSUPPORT */
