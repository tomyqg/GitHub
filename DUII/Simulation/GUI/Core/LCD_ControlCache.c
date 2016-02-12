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
File        : LCD_ControlCache.c
Purpose     : Controlling the cache of the display driver
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_Private.h"
#include "LCD_SIM.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_ControlCacheEx
*/
int LCD_ControlCacheEx(int LayerIndex, int Cmd) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE * pDevice, int Cmd);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE * pDevice, int Cmd))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_CONTROLCACHE);
    if (pFunc) {
      pFunc(pDevice, Cmd);
      r = 0;
    }
  }
  return r;
}

/*********************************************************************
*
*       LCD_ControlCache
*/
int LCD_ControlCache(int Cmd) {
  return LCD_ControlCacheEx(GUI_Context.SelLayer, Cmd);
}

/*************************** End of file ****************************/
