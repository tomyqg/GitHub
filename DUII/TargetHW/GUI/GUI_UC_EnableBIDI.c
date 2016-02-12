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
File        : GUI_UC_EnableBIDI.c
Purpose     : Enabling of drawing bidirectional text
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static const char GUI_UNI_PTR * (* _GUI_CharLine_pfLog2Vis)          (const char GUI_UNI_PTR * s, int * pMaxNumChars);
static int                      (* _GUI__GetCursorPos_pfGetPosX)     (const char GUI_UNI_PTR * s, int Index, int MaxNumChars);
static int                      (* _GUI__GetCursorPos_pfGetPosChar)  (const char GUI_UNI_PTR * s, int x, int MaxNumChars);
static U16                      (* _GUI__GetCursorPos_pfGetCharacter)(const char GUI_UNI_PTR * s, int Index, int MaxNumChars, int * pIsRTL);
static int                      (* _GUI__Wrap_pfGetWordWrap)         (const char GUI_UNI_PTR * s, int xSize, int * pxDist);
static int                      (* _GUI__Wrap_pfGetCharWrap)         (const char GUI_UNI_PTR * s, int xSize);

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI__BIDI_Log2VisBuffered
*/
const char GUI_UNI_PTR * GUI__BIDI_Log2VisBuffered(const char GUI_UNI_PTR * s, int * pMaxNumChars) {
  static char acBuffer[GUI_BIDI_MAX_CHARS_PER_LINE * 3];
  int MaxNumChars;

  MaxNumChars = *pMaxNumChars;
  MaxNumChars = GUI__BIDI_Log2Vis(s, MaxNumChars, acBuffer, sizeof(acBuffer));
  s = acBuffer;
  *pMaxNumChars = MaxNumChars;
  return s;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_UC_EnableBIDI
*/
int GUI_UC_EnableBIDI(int OnOff) {
  int PreviousState;

  GUI_LOCK();
  PreviousState = GUI__BIDI_Enabled;
  if ((OnOff > 0) && (GUI__BIDI_Enabled == 0)) {
    //
    // Get current values of function pointers from GUI_CharLine.c, GUI__GetCursorPos.c and GUI__Wrap.c
    //
    _GUI_CharLine_pfLog2Vis           = GUI_CharLine_pfLog2Vis;
    _GUI__GetCursorPos_pfGetPosX      = GUI__GetCursorPos_pfGetPosX;
    _GUI__GetCursorPos_pfGetPosChar   = GUI__GetCursorPos_pfGetPosChar;
    _GUI__GetCursorPos_pfGetCharacter = GUI__GetCursorPos_pfGetCharacter;
    _GUI__Wrap_pfGetWordWrap          = GUI__Wrap_pfGetWordWrap;
    _GUI__Wrap_pfGetCharWrap          = GUI__Wrap_pfGetCharWrap;
    //
    // Set function pointers in GUI__BiDiRouting.c
    //
    _pfGUI__BIDI_Log2Vis          = GUI__BIDI_Log2Vis;
    _pfGUI__BIDI_GetCursorPosX    = GUI__BIDI_GetCursorPosX;
    _pfGUI__BIDI_GetCursorPosChar = GUI__BIDI_GetCursorPosChar;
    _pfGUI__BIDI_GetLogChar       = GUI__BIDI_GetLogChar;
    _pfGUI__BIDI_GetCharDir       = GUI__BIDI_GetCharDir;
    _pfGUI__BIDI_IsNSM            = GUI__BIDI_IsNSM;
    //
    // Set function pointer in GUI_CharLine.c
    //
    GUI_CharLine_pfLog2Vis = GUI__BIDI_Log2VisBuffered;
    //
    // Set function pointers in GUI__GetCursorPos.c
    //
    GUI__GetCursorPos_pfGetPosX      = GUI__BIDI_GetCursorPosX;
    GUI__GetCursorPos_pfGetPosChar   = GUI__BIDI_GetCursorPosChar;
    GUI__GetCursorPos_pfGetCharacter = GUI__BIDI_GetCursorCharacter;
    //
    // Set function pointers in GUI__Wrap.c
    //
    GUI__Wrap_pfGetWordWrap = GUI__BIDI_GetWordWrap;
    GUI__Wrap_pfGetCharWrap = GUI__BIDI_GetCharWrap;
    //
    // Set common flag
    //
    GUI__BIDI_Enabled = 1;
  } else if ((OnOff == 0) && (GUI__BIDI_Enabled > 0)) {
    if ((_GUI_CharLine_pfLog2Vis           != NULL) &&
        (_GUI__GetCursorPos_pfGetPosX      != NULL) &&
        (_GUI__GetCursorPos_pfGetPosChar   != NULL) &&
        (_GUI__GetCursorPos_pfGetCharacter != NULL) &&
        (_GUI__Wrap_pfGetWordWrap          != NULL) &&
        (_GUI__Wrap_pfGetCharWrap          != NULL)) {
      //
      // Restore function pointers in GUI_CharLine.c
      //
      GUI_CharLine_pfLog2Vis = _GUI_CharLine_pfLog2Vis;
      //
      // Restore function pointers in GUI__GetCursorPos.c
      //
      GUI__GetCursorPos_pfGetPosX      = _GUI__GetCursorPos_pfGetPosX;
      GUI__GetCursorPos_pfGetPosChar   = _GUI__GetCursorPos_pfGetPosChar;
      GUI__GetCursorPos_pfGetCharacter = _GUI__GetCursorPos_pfGetCharacter;
      //
      // Restore function pointers in GUI__Wrap.c
      //
      GUI__Wrap_pfGetWordWrap = _GUI__Wrap_pfGetWordWrap;
      GUI__Wrap_pfGetCharWrap = _GUI__Wrap_pfGetCharWrap;
      //
      // Set common flag
      //
      GUI__BIDI_Enabled = 0;
    }
  }
  GUI_UNLOCK();
  return PreviousState;
}

/*************************** End of file ****************************/
