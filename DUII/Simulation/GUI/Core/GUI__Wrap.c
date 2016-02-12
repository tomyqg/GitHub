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
File        : GUI__Wrap.c
Purpose     : Implementation of wrap routines
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifndef   GUI_SUPPORT_BIDI
  #define GUI_SUPPORT_BIDI 0
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _IsLineEnd
*/
static int _IsLineEnd(U16 Char) {
  if (!Char || (Char == '\n')) {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       _GetNoWrap
*/
static int _GetNoWrap(const char GUI_UNI_PTR * s) {
  return GUI__GetLineNumChars(s, 0x7FFF);
}

/*********************************************************************
*
*       _GetWordWrap (without Arabic support)
*/
static int _GetWordWrap(const char GUI_UNI_PTR * s, int xSize, int * pxDist) {
  int xDist = 0, NumChars = 0, WordWrap = 0, CharDist = 0;
  U16 Char, PrevChar = 0, FirstChar = 0;
  while (1) {
    Char = GUI_UC__GetCharCodeInc(&s);   /* Similar to:  *s++ */
    if (!FirstChar) {
      FirstChar = Char;
    }
    /* Let's first check if the line end is reached. In this case we are done. */
    if (_IsLineEnd(Char)) {
      WordWrap = NumChars;
      break;
    }
    /* If the current character is part of leading spaces the position can be used for wrapping */
    if ((Char != ' ') && (FirstChar == ' ')) {
      FirstChar = 0;
      WordWrap = NumChars;
    /* If the current character is a space and the previous character not, we found a wrapping position */
    } else if ((Char == ' ') && (Char != PrevChar)) {
      WordWrap = NumChars;
    /* If the current character is a trailing space the position can be used for wrapping */
    } else if ((Char == ' ') && (PrevChar == ' ')) {
      WordWrap = NumChars;
    }
    PrevChar = Char;
    CharDist = GUI_GetCharDistX(Char);
    xDist += CharDist;
    if ((xDist <= xSize) || (NumChars == 0)) {
      NumChars++;
    } else {
      break;
    }
  }
  if (!WordWrap) {
    WordWrap = NumChars;
  }
  if (pxDist) {
    xDist -= CharDist;
    *pxDist = xDist;
  }
  return WordWrap;
}

/*********************************************************************
*
*       _GetCharWrap (without Arabic support)
*/
static int _GetCharWrap(const char GUI_UNI_PTR * s, int xSize) {
  int xDist = 0, NumChars = 0;
  U16 Char;
  while ((Char = GUI_UC__GetCharCodeInc(&s)) != 0) {
    xDist += GUI_GetCharDistX(Char);
    if ((NumChars && (xDist > xSize)) || (Char == '\n')) {
      break;
    }
    NumChars++;
  }
  return NumChars;
}

/*********************************************************************
*
*       Private data
*
**********************************************************************
*/
/*********************************************************************
*
*       Function pointers
*/
#if (GUI_SUPPORT_BIDI)
  int (* GUI__Wrap_pfGetWordWrap)(const char GUI_UNI_PTR * s, int xSize, int * pxDist) = GUI__BIDI_GetWordWrap;
  int (* GUI__Wrap_pfGetCharWrap)(const char GUI_UNI_PTR * s, int xSize)               = GUI__BIDI_GetCharWrap;
#else
  int (* GUI__Wrap_pfGetWordWrap)(const char GUI_UNI_PTR * s, int xSize, int * pxDist) = _GetWordWrap;
  int (* GUI__Wrap_pfGetCharWrap)(const char GUI_UNI_PTR * s, int xSize)               = _GetCharWrap;
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _WrapGetNumCharsDisp
*/
static int _WrapGetNumCharsDisp(const char GUI_UNI_PTR * pText, int xSize, GUI_WRAPMODE WrapMode, int * pxDist) {
  int r;
  switch (WrapMode) {
  case GUI_WRAPMODE_WORD:
    r = GUI__Wrap_pfGetWordWrap(pText, xSize, pxDist);
    break;
  case GUI_WRAPMODE_CHAR:
    r = GUI__Wrap_pfGetCharWrap(pText, xSize);
    break;
  default:
    r = _GetNoWrap(pText);
  }
  return r;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI__WrapGetNumCharsDisp
*
* Returns:
*  Number of characters to display in the line.
*  Trailing spaces and line end character are
*  not counted
*/
int GUI__WrapGetNumCharsDisp(const char GUI_UNI_PTR * pText, int xSize, GUI_WRAPMODE WrapMode) {
  return _WrapGetNumCharsDisp(pText, xSize, WrapMode, 0);
}

/*********************************************************************
*
*       GUI__WrapGetNumCharsToNextLine
*/
int GUI__WrapGetNumCharsToNextLine(const char GUI_UNI_PTR * pText, int xSize, GUI_WRAPMODE WrapMode) {
  int NumChars;
  int xDist;
  U16 Char;
  NumChars = _WrapGetNumCharsDisp(pText, xSize, WrapMode, &xDist);
  pText   += GUI_UC__NumChars2NumBytes(pText, NumChars);
  Char     = GUI_UC__GetCharCodeInc(&pText);
  if (Char == '\n') {
    NumChars++;
  } else {
    if ((WrapMode == GUI_WRAPMODE_WORD) && (Char == ' ')) {
      int xDistChar = GUI_GetCharDistX(Char);
      while ((Char == ' ') && (xSize >= xDist)) {
        NumChars++;
        xDist += xDistChar;
        Char = GUI_UC__GetCharCodeInc(&pText);
      }
    }
  }
  return NumChars;
}

/*********************************************************************
*
*       GUI__WrapGetNumBytesToNextLine
*/
int GUI__WrapGetNumBytesToNextLine(const char GUI_UNI_PTR * pText, int xSize, GUI_WRAPMODE WrapMode) {
  int NumChars, NumBytes;
  NumChars = GUI__WrapGetNumCharsToNextLine(pText, xSize, WrapMode);
  NumBytes = GUI_UC__NumChars2NumBytes(pText, NumChars);
  return NumBytes;
}

/*************************** End of file ****************************/
