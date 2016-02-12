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
File        : GUIAAPolyOut.c
Purpose     : Draw Polygon outline routines with Antialiasing
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <string.h>

#include "GUI.h"
#include "GUI_Debug.h"

#ifndef   GUI_AA_MAX_NUM_OUTLINEPOINTS
  #define GUI_AA_MAX_NUM_OUTLINEPOINTS 10
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_AA_DrawPolyOutlineEx
*/
void GUI_AA_DrawPolyOutlineEx(const GUI_POINT * pSrc, int NumPoints, int Thickness, int x, int y, GUI_POINT * pBuffer) {
  U8 PrevDraw;
  GUI_EnlargePolygon(pBuffer, pSrc, NumPoints, Thickness);
  GUI_AA_FillPolygon(pBuffer, NumPoints, x, y );
  PrevDraw = LCD_SetDrawMode(GUI_DRAWMODE_REV);
/* Copy points due to const qualifier */
  GUI_MEMCPY(pBuffer, pSrc, NumPoints * sizeof(GUI_POINT));
  GUI_AA_FillPolygon(pBuffer, NumPoints, x, y );
  LCD_SetDrawMode(PrevDraw);
}

/*********************************************************************
*
*       GUI_AA_DrawPolyOutline
*/
void GUI_AA_DrawPolyOutline(const GUI_POINT * pSrc, int NumPoints, int Thickness, int x, int y) {
  GUI_POINT aiTemp[GUI_AA_MAX_NUM_OUTLINEPOINTS];
  if (NumPoints > GUI_AA_MAX_NUM_OUTLINEPOINTS) {
    GUI_DEBUG_ERROROUT("GUI_AA_DrawPolyOutline: Outline has too many points!");
    return;
  }
  GUI_AA_DrawPolyOutlineEx(pSrc, NumPoints, Thickness, x, y, aiTemp);
}

/*************************** End of file ****************************/
