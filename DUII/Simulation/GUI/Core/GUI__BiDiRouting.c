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
File        : GUI__BiDiRouting.c
Purpose     : Function pointers and status for bidirectional text
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
*       Public data
*
**********************************************************************
*/
#if GUI_SUPPORT_BIDI
  int GUI__BIDI_Enabled = 1;
#else
  int GUI__BIDI_Enabled;
#endif

/*********************************************************************
*
*       Function pointers
*
* The following function pointers can be used without having
* references to GUI__BIDI.c
*/
#if GUI_SUPPORT_BIDI
  int (* _pfGUI__BIDI_Log2Vis         )(const char GUI_UNI_PTR * s, int NumChars, char * pBuffer, int BufferSize) = GUI__BIDI_Log2Vis;
  int (* _pfGUI__BIDI_GetCursorPosX   )(const char GUI_UNI_PTR * s, int NumChars, int Index)                      = GUI__BIDI_GetCursorPosX;
  int (* _pfGUI__BIDI_GetCursorPosChar)(const char GUI_UNI_PTR * s, int NumChars, int x)                          = GUI__BIDI_GetCursorPosChar;
  U16 (* _pfGUI__BIDI_GetLogChar      )(const char GUI_UNI_PTR * s, int NumChars, int Index)                      = GUI__BIDI_GetLogChar;
  int (* _pfGUI__BIDI_GetCharDir      )(const char GUI_UNI_PTR * s, int NumChars, int Index)                      = GUI__BIDI_GetCharDir;
  int (* _pfGUI__BIDI_IsNSM           )(U16 Char)                                                                 = GUI__BIDI_IsNSM;
#else
  int (* _pfGUI__BIDI_Log2Vis         )(const char GUI_UNI_PTR * s, int NumChars, char * pBuffer, int BufferSize);
  int (* _pfGUI__BIDI_GetCursorPosX   )(const char GUI_UNI_PTR * s, int NumChars, int Index);
  int (* _pfGUI__BIDI_GetCursorPosChar)(const char GUI_UNI_PTR * s, int NumChars, int x);
  U16 (* _pfGUI__BIDI_GetLogChar      )(const char GUI_UNI_PTR * s, int NumChars, int Index);
  int (* _pfGUI__BIDI_GetCharDir      )(const char GUI_UNI_PTR * s, int NumChars, int Index);
  int (* _pfGUI__BIDI_IsNSM           )(U16 Char);
#endif

/*************************** End of file ****************************/
