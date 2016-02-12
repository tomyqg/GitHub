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
File        : GUIPolyR.c
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
*       GUI_MagnifyPolygon
*/
void GUI_MagnifyPolygon(GUI_POINT* pDest, const GUI_POINT* pSrc, int NumPoints, int Mag) {
  int j;
  for (j=0; j<NumPoints; j++) {
    (pDest+j)->x = (pSrc+j)->x * Mag;
    (pDest+j)->y = (pSrc+j)->y * Mag;
  }
}

/*************************** End of file ****************************/
