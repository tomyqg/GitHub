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
File        : GUIAlloc.C
Purpose     : Dynamic memory management
----------------------------------------------------------------------
*/

#include <string.h>           /* for memcpy, memset */

#include "GUI_Private.h"

/*********************************************************************
*
*       Internal memory management
*
**********************************************************************
*/

/*********************************************************************
*
*       GUI_ALLOC_AllocInit
*
* Purpose:
*   Alloc memory block of the given size and initialize from the given pointer.
*   If the pointer is a NULL pointer, zeroinit
*/
GUI_HMEM GUI_ALLOC_AllocInit(const void *pInitData, GUI_ALLOC_DATATYPE Size) {
  GUI_HMEM hMem;
  GUI_LOCK();
  if (Size == 0) {
    return (GUI_HMEM)0;
  }
  hMem = GUI_ALLOC_AllocNoInit(Size);
  if (hMem) {
    void *pMem;
    pMem = GUI_ALLOC_h2p(hMem);
    if (pInitData) {
      GUI_MEMCPY(pMem, pInitData, Size);
    } else {
      GUI_MEMSET((U8*)pMem, 0, Size);
    }
  }
  GUI_UNLOCK();
  return hMem;
}

/*************************** End of file ****************************/
