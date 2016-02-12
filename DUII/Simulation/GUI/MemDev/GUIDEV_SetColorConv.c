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
File        : GUIDEV_SetColorConv.c
Purpose     : Implementation of memory devices, add. module
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/* Memory device capabilities are compiled only if support for them is enabled. */ 
#if GUI_SUPPORT_MEMDEV
/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_SetColorConv
*/
void GUI_MEMDEV_SetColorConv(GUI_MEMDEV_Handle hMemDev, const LCD_API_COLOR_CONV * pColorConvAPI) {
  GUI_MEMDEV * pDevData;
  if (!hMemDev) {
    return;
  }
  GUI_LOCK();
  pDevData = GUI_MEMDEV_H2P(hMemDev);
  pDevData->pDevice->pColorConvAPI = pColorConvAPI;
  GUI_UNLOCK();
}

#else
void GUIDEV_SetColorConv_C(void);
void GUIDEV_SetColorConv_C(void) {} /* avoid empty object files */
#endif /* GUI_MEMDEV_SUPPORT */

/*************************** end of file ****************************/
