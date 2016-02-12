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
File        : GUIDEV_CreateFixed.c
Purpose     : Implementation of GUI_MEMDEV_CreateFixed
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/* Memory device capabilities are compiled only if support for them is enabled.*/ 
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       Exported code
*
**********************************************************************
*/
GUI_MEMDEV_Handle GUI_MEMDEV_CreateFixed(int x0, int y0, 
                                         int xsize, int ysize, int Flags,
                                         const GUI_DEVICE_API     * pDeviceAPI,
                                         const LCD_API_COLOR_CONV * pColorConvAPI) {
  GUI_MEMDEV_Handle hMemDev = 0;
  if (pColorConvAPI) {
    GUI_LOCK();
    hMemDev = GUI_MEMDEV__CreateFixed(x0, y0, xsize, ysize, Flags, pDeviceAPI, pColorConvAPI);
    GUI_UNLOCK();
  }
  return hMemDev;
}

#else

void GUIDEV_CreateFixed_C(void);
void GUIDEV_CreateFixed_C(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
