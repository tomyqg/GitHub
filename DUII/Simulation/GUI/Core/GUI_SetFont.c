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
File        : GUI_SetFont.C
Purpose     : Optional routines
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
*       GUI_SetFont
*/
const GUI_FONT GUI_UNI_PTR* GUI_SetFont(const GUI_FONT GUI_UNI_PTR * pNewFont) {
  const GUI_FONT GUI_UNI_PTR * pOldFont;
  GUI_LOCK();
  pOldFont = GUI_Context.pAFont;
  if (pNewFont) {
    GUI_Context.pAFont = pNewFont;
  }
  GUI_UNLOCK();
  return pOldFont;
}

/*************************** End of file ****************************/
