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
File        : GUIDEV_GetPos.c
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
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_GetXPos
*/
int GUI_MEMDEV_GetXPos(GUI_MEMDEV_Handle hMem) {
  int r = 0;
  GUI_MEMDEV* pDevData;
  GUI_LOCK();
  if (hMem == 0) {
    hMem = GUI_Context.hDevData;
  }
  if (hMem) {
    pDevData = (GUI_MEMDEV*) GUI_ALLOC_h2p(hMem);  /* Convert to pointer */
    r = pDevData->x0;
  }
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_MEMDEV_GetYPos
*/
int GUI_MEMDEV_GetYPos(GUI_MEMDEV_Handle hMem) {
  int r = 0;
  GUI_MEMDEV* pDevData;
  GUI_LOCK();
  if (hMem == 0) {
    hMem = GUI_Context.hDevData;
  }
  if (hMem) {
    pDevData = (GUI_MEMDEV*) GUI_ALLOC_h2p(hMem);  /* Convert to pointer */
    r = pDevData->y0;
  }
  GUI_UNLOCK();
  return r;
}

#else

void GUIDEV_GetPos_C(void);
void GUIDEV_GetPos_C(void) {} /* avoid empty object files */

#endif /* GUI_MEMDEV_SUPPORT */

/*************************** end of file ****************************/
