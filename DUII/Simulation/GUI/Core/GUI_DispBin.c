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
File        : GUI_DispBin.c
Purpose     : Routines to display values as binary
----------------------------------------------------------------------
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
*     GUI_DispBin
*/
void GUI_DispBin(U32 v, U8 Len) {
	char ac[33];
	char* s = ac;
  GUI_AddBin(v, Len, &s);
  GUI_DispString(ac);
}

/*********************************************************************
*
*     GUI_DispBinAt
*/
void GUI_DispBinAt(U32 v, I16P x, I16P y, U8 Len) {
	char ac[33];
	char* s = ac;
  GUI_AddBin(v, Len, &s);
  GUI_DispStringAt(ac, x, y);
}

/*************************** End of file ****************************/
