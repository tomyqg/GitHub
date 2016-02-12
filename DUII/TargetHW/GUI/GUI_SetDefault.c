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
File        : GUI_SetDefault.c
Purpose     : Implementation of GUI_SetDefault
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
*       GL_SetDefault
*/
void GL_SetDefault(void) {
  GUI_SetBkColor(GUI_DEFAULT_BKCOLOR);
  GUI_SetColor  (GUI_DEFAULT_COLOR);
  GUI_SetPenSize(1);
  GUI_SetTextAlign(0);
  GUI_SetTextMode(0);
  GUI_SetDrawMode(0);
  GUI_SetFont(GUI__pFontDefault);
  GUI_SetLineStyle(GUI_LS_SOLID);
}

/*********************************************************************
*
*       GUI_SetDefault
*/
void GUI_SetDefault(void) {
  GUI_LOCK();
  GL_SetDefault();
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
