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
File        : GUIRealloc.C
Purpose     : Dynamic memory management
----------------------------------------------------------------------
*/

#include <string.h>           /* for memcpy, memset */

#include "GUI_Private.h"

/*********************************************************************
*
*       Public routines: Using internal memory management
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_ALLOC_Realloc
*
* Purpose:
*   Reallocate a memory block. This is typically used to grow memory
*   blocks. The contents of the old memory block are copied into the
*   new block (or as much as fits in case of shrinkage).
*   In case of error the old memory block (and its handle) remain
*   unchanged.
*
*  Parameters
*    hOld       Current handle of the memory block. 0 (Invalid handle) is permitted.
*    NewSize    Number of bytes required
*
* Return value:
*   On success:    Handle of newly allocated memory block
*   On error:      0
*/
GUI_HMEM GUI_ALLOC_Realloc(GUI_HMEM hOld, int NewSize) {
  GUI_HMEM hNew;
  hNew = GUI_ALLOC_AllocNoInit(NewSize);
  if (hNew && hOld) {
    void *pNew, *pOld;
    int Size, OldSize;
    OldSize = GUI_ALLOC_GetSize(hOld);
    Size = (OldSize < NewSize) ? OldSize : NewSize;
    GUI_LOCK();
    pNew = GUI_ALLOC_h2p(hNew);
    pOld = GUI_ALLOC_h2p(hOld);
    GUI_MEMCPY(pNew, pOld, Size);
    GUI_ALLOC_Free(hOld);
    GUI_UNLOCK();
  }
  return hNew;
}

/*************************** End of file ****************************/
