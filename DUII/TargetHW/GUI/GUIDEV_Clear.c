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
File        : GUIDEV_Clear.c
Purpose     : Implementation of memory devices, add. module
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_Private.h"
#include "GUI_Debug.h"

/* Memory device capabilities are compiled only if support for them is enabled. */ 
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_Clear
*/
void GUI_MEMDEV_Clear(GUI_MEMDEV_Handle hMem) {
  GUI_MEMDEV * pDev;
  GUI_USAGE_h  hUsage;
  GUI_USAGE  * pUsage;

  if (!hMem) {
    if ((hMem = GUI_Context.hDevData) == 0) {
      return;
    }
  }
  GUI_LOCK();
  pDev = GUI_MEMDEV_H2P(hMem);  /* Convert to pointer */
  #if 1
  hUsage = pDev->hUsage; 
  if (hUsage) {
    pUsage = GUI_USAGE_H2P(hUsage);
    GUI_USAGE_Clear(pUsage);
  }
  #else
  if (pDev->BitsPerPixel == 32) {
    U32 * pData;
    int xSize, ySize;
    U32 NumPixels, Data;

    pData = GUI_MEMDEV_GetDataPtr(hMem);
    xSize = GUI_MEMDEV_GetXSize(hMem);
    ySize = GUI_MEMDEV_GetYSize(hMem);
    NumPixels = xSize * ySize;
    while (NumPixels--) {
      Data = *pData;
      Data &= 0x00FFFFFF;
      Data |= 0xFF000000;
      *pData++ = Data;
    }
  } else {
    hUsage = pDev->hUsage; 
    if (hUsage) {
      pUsage = GUI_USAGE_H2P(hUsage);
      GUI_USAGE_Clear(pUsage);
    }
  }
  #endif
  GUI_UNLOCK();
}

#else

void GUIDEV_Clear(void);
void GUIDEV_Clear(void) {} /* avoid empty object files */

#endif /* GUI_MEMDEV_SUPPORT */

/*************************** end of file ****************************/
