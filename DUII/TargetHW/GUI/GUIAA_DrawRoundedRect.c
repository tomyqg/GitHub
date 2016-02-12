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
File        : GUIAA_DrawRoundedRect.c
Purpose     : Draw rounded rectangle with Antialiasing
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
*       GUI_AA_DrawRoundedRect
*/
void GUI_AA_DrawRoundedRect(int x0, int y0, int x1, int y1, int r) {
  U8 OldShape;

  OldShape = GUI_GetPenShape();
  GUI_SetPenShape(GUI_PS_FLAT);
  GUI_AA_DrawLine(x0 + r, y0, x1 - r, y0);
  GUI_AA_DrawLine(x0 + r, y1, x1 - r, y1);
  GUI_AA_DrawLine(x0, y0 + r, x0, y1 - r);
  GUI_AA_DrawLine(x1, y0 + r, x1, y1 - r);
  GUI_SetPenShape(OldShape);
  GUI_AA_DrawArc(x1 - r, y0 + r, r, r,   0,  90);
  GUI_AA_DrawArc(x0 + r, y0 + r, r, r,  90, 180);
  GUI_AA_DrawArc(x0 + r, y1 - r, r, r, 180, 270);
  GUI_AA_DrawArc(x1 - r, y1 - r, r, r, 270, 360);
}

/*************************** End of file ****************************/
