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
File        : GUI_SetOrg.C
Purpose     : Defines the GUI_SetOrg function
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
*       GUI_SetOrg
*/
void GUI_SetOrg(int x, int y) {
  GUI_LOCK();
  GUI_OrgX = x;
  GUI_OrgY = y;
  GUI_Context.apDevice[GUI_Context.SelLayer]->pDeviceAPI->pfSetOrg(GUI_Context.apDevice[GUI_Context.SelLayer], x, y);
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
