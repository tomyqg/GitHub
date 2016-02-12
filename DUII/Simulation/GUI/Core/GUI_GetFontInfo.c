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
File        : GUI_GetFontInfo.C
Purpose     : Get GUI_FONTINFO structure
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GetFontInfo
*/
void GUI_GetFontInfo(const GUI_FONT GUI_UNI_PTR * pFont, GUI_FONTINFO * pFontInfo) {
  GUI_LOCK();
  if (pFont == NULL) {
    pFont = GUI_Context.pAFont;
  }
  pFontInfo->Baseline = pFont->Baseline;
  pFontInfo->CHeight  = pFont->CHeight;
  pFontInfo->LHeight  = pFont->LHeight;
  pFont->pfGetFontInfo(pFont, pFontInfo);
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
