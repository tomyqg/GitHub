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
File        : LCDInfo.C
Purpose     : Routines returning info at runtime
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetDevCap
*/
static I32 _GetDevCap(int LayerIndex, int Index) {
  GUI_DEVICE * pDevice;
  I32 r = 0;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    r = pDevice->pDeviceAPI->pfGetDevProp(pDevice, Index);
  }
  return r;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_GetDevCap
*/
I32 LCD_GetDevCap(int Index) {
  return _GetDevCap(GUI_Context.SelLayer, Index);
}

/*********************************************************************
*
*       LCD_GetDevCapEx
*/
I32 LCD_GetDevCapEx(int LayerIndex, int Index) {
  return _GetDevCap(LayerIndex, Index);
}

/*************************** End of file ****************************/
