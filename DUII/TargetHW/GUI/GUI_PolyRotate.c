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
File        : GUI_PolyRotate.c
Purpose     : Polygon rotation
----------------------------------------------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_RotatePolygon
*/
void GUI_RotatePolygon(GUI_POINT * pDest, const GUI_POINT * pSrc, int NumPoints, float Angle) {
  int x, y, j;
  I32 AngDEG_1000, SinHQ, CosHQ;

  AngDEG_1000 = (I32)((Angle * 180000) / 3.1415926f);
  CosHQ = GUI__CosHQ(AngDEG_1000);
  SinHQ = GUI__SinHQ(AngDEG_1000);
  for (j = 0; j < NumPoints; j++) {
    x = (pSrc + j)->x;
    y = (pSrc + j)->y;
    (pDest + j)->x = (int)(( x * CosHQ + y * SinHQ) >> 16);
    (pDest + j)->y = (int)((-x * SinHQ + y * CosHQ) >> 16);
  }
}

/*************************** End of file ****************************/
