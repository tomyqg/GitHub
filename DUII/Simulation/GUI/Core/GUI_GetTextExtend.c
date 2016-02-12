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
File        : GUI_GetTextExtend.c
Purpose     : Implementation of GUI_GetTextExtend
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GetTextExtend
*/
void GUI_GetTextExtend(GUI_RECT* pRect, const char GUI_UNI_PTR * s, int MaxNumChars) {
  int xMax, NumLines, LineSizeX;
  U16 Char;
  xMax = NumLines = LineSizeX = 0;
  pRect->x0 = GUI_Context.DispPosX;
  pRect->y0 = GUI_Context.DispPosY;
  if (!s) {
    return;
  }
  if (*s) {
    NumLines++;
    while (MaxNumChars--) {
      Char = GUI_UC__GetCharCodeInc(&s);
      if ((Char == '\n') || (Char == 0)) {
        if (LineSizeX > xMax) {
          xMax = LineSizeX;
        }
        LineSizeX = 0;
        if (Char == '\n') {
          NumLines++;
        }
        if (!Char) {
          break;
        }
      } else {
        LineSizeX += GUI_GetCharDistX(Char);
      }
    }
  }
  if (LineSizeX > xMax) {
    xMax = LineSizeX;
  }
  pRect->x1 = pRect->x0 + xMax - 1;
  pRect->y1 = pRect->y0 + GUI__GetFontSizeY() * NumLines - 1;
}

/*************************** End of file ****************************/
