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
File        : GUI__GetCursorPos.c
Purpose     : Implementation of optional routines

              The routines below are used by EDIT and MULTIEDIT widget
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
*       _GetCursorPosX
*/
static int _GetCursorPosX(const char GUI_UNI_PTR * s, int MaxNumChars, int Index) {
  int CursorPosX = 0;
  U16 Char;
  GUI_USE_PARA(MaxNumChars);
  while (Index--) {
    Char        = GUI_UC__GetCharCodeInc(&s);
    CursorPosX += GUI_GetCharDistX(Char);
  }
  return CursorPosX;
}

/*********************************************************************
*
*       _GetCursorPosChar
*/
static int _GetCursorPosChar(const char GUI_UNI_PTR * s, int MaxNumChars, int x) {
  int SizeX = 0;
  const char GUI_UNI_PTR * p;
  p = s;
  while (--MaxNumChars> 0) {
    U16 Char;
    Char   = GUI_UC_GetCharCode(s);
    SizeX += GUI_GetCharDistX(Char);
    if (!Char || (SizeX > x)) {
      break;
    }
    s += GUI_UC_GetCharSize(s);
  }
  return GUI_UC__NumBytes2NumChars(p, s - p);
}

/*********************************************************************
*
*       _GetCursorCharacter
*/
static U16 _GetCursorCharacter(const char GUI_UNI_PTR * s, int MaxNumChars, int Index, int * pIsRTL) {
  GUI_USE_PARA(MaxNumChars);
  if (pIsRTL) {
    *pIsRTL = 0;
  }
  while (Index--) {
    s  += GUI_UC__GetCharSize(s);
  }
  return GUI_UC_GetCharCode(s);
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
#if GUI_SUPPORT_BIDI
  int (* GUI__GetCursorPos_pfGetPosX)     (const char GUI_UNI_PTR * s, int MaxNumChars, int Index)               = GUI__BIDI_GetCursorPosX;
  int (* GUI__GetCursorPos_pfGetPosChar)  (const char GUI_UNI_PTR * s, int MaxNumChars, int x)                   = GUI__BIDI_GetCursorPosChar;
  U16 (* GUI__GetCursorPos_pfGetCharacter)(const char GUI_UNI_PTR * s, int MaxNumChars, int Index, int * pIsRTL) = GUI__BIDI_GetCursorCharacter;
#else
  int (* GUI__GetCursorPos_pfGetPosX)     (const char GUI_UNI_PTR * s, int MaxNumChars, int Index)               = _GetCursorPosX;
  int (* GUI__GetCursorPos_pfGetPosChar)  (const char GUI_UNI_PTR * s, int MaxNumChars, int x)                   = _GetCursorPosChar;
  U16 (* GUI__GetCursorPos_pfGetCharacter)(const char GUI_UNI_PTR * s, int MaxNumChars, int Index, int * pIsRTL) = _GetCursorCharacter;
#endif

/*********************************************************************
*
*       GUI__GetCursorPosX
*
* Purpose:
*   Returns the cursor position in pixels by the given index
*
* Parameters:
*   s           : Pointer to the string
*   Index       : Index of cursor character
*   MaxNumChars : Number of charaters in the string
*/
int GUI__GetCursorPosX(const char GUI_UNI_PTR * s, int Index, int MaxNumChars) {
  return GUI__GetCursorPos_pfGetPosX(s, MaxNumChars, Index);
}

/*********************************************************************
*
*       GUI__GetCursorPosChar
*
* Purpose:
*   Returns the cursor character index by the given pixel position
*
* Parameters:
*   s           : Pointer to the string
*   x           : X position of the cursor
*   MaxNumChars : Number of charaters in the string
*/
int GUI__GetCursorPosChar(const char GUI_UNI_PTR * s, int x, int MaxNumChars) {
  return GUI__GetCursorPos_pfGetPosChar(s, MaxNumChars, x);
}

/*********************************************************************
*
*       GUI__GetCursorCharacter
*/
U16 GUI__GetCursorCharacter(const char GUI_UNI_PTR * s, int Index, int MaxNumChars, int * pIsRTL) {
  return GUI__GetCursorPos_pfGetCharacter(s, MaxNumChars, Index, pIsRTL);
}

/*************************** End of file ****************************/
