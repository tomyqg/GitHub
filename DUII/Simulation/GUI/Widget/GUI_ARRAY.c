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
File        : GUI_ARRAY.c
Purpose     : Array handling routines
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_ARRAY_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_ARRAY__SethItem
*
* Purpose:
*   Sets an item.
*
* Returns:
*   1: if operation has failed
*   0: OK
*
* Notes:
*   (1) Replacing Items
*       If the item is already assigned
*       (Which means the handle is already != 0), it is freeed. However,
*       the handle is treated as a handle to a data item, not an object.
*       This means the data item is freed, but if the pointer points to
*       an object, the destructor of the object is not called.
*/
int GUI_ARRAY__SethItem(GUI_ARRAY_OBJ * pThis, unsigned int Index, WM_HMEM hItem) {
  WM_HMEM   ha;
  WM_HMEM * pa;
  int r = 1;

  if (Index < (unsigned)pThis->NumItems) {
    ha = pThis->haHandle;
    if (ha) {
      pa = (WM_HMEM *)GUI_LOCK_H(ha);
      pa += Index;
      GUI_ALLOC_FreePtr(pa);
      *pa = hItem;
      GUI_UNLOCK_H(pa);
      r = 0;
    }
  }
  return r;
}

/*********************************************************************
*
*       GUI_ARRAY__GethItem
*
* Purpose:
*   Gets the handle of specified item
*
* Notes:
*   (1) Index out of bounds
*   It is permitted to specify an index larger than the
*   array size. In this case, a 0-handle is returned.
*/
WM_HMEM GUI_ARRAY__GethItem(const GUI_ARRAY_OBJ * pThis, unsigned int Index) {
  WM_HMEM h = 0;

  if (Index < (unsigned)pThis->NumItems) {
    WM_HMEM   ha;
    WM_HMEM * pa;
    ha = pThis->haHandle;
    if (ha) {
      pa = (WM_HMEM *)GUI_LOCK_H(ha);
      h = *(pa + Index);
      GUI_UNLOCK_H(pa);
    }
  }
  return h;
}

/*********************************************************************
*
*       GUI_ARRAY__GetpItemLocked
*
* Purpose:
*   Gets the pointer of specified item
*
* Notes:
*   (1) Index out of bounds
*       It is permitted to specify an index larger than the
*       array size. In this case, a 0-handle is returned.
*   (2) Locking
*       It is the caller's responsibility to lock before calling this
*       function.
*/
void * GUI_ARRAY__GetpItemLocked(const GUI_ARRAY_OBJ * pThis, unsigned int Index) {
  void * p = NULL;
  WM_HMEM h;

  h = GUI_ARRAY__GethItem(pThis, Index);
  if (h) {
    p = GUI_LOCK_H(h);
  }
  return p;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_ARRAY_GetNumItems
*/
unsigned int GUI_ARRAY_GetNumItems(GUI_ARRAY hArray) {
  GUI_ARRAY_OBJ * pThis;
  unsigned r = 0;
  
  if (hArray) {
    pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
    r = pThis->NumItems;
    GUI_UNLOCK_H(pThis);
  }
  return r;
}

/*********************************************************************
*
*       GUI_ARRAY_AddItem
*
* Purpose:
*   Add an item to a GUI_ARRAY_OBJ.
*   If the SIze is > 0, a memory block is allocated for storage.
*   If on top of this a pointer is specified, the memory block holding
*   the copy of the item is initialized.
*
* Return value:
*   If O.K. : 0
*   On error: 1
*   
*/
int GUI_ARRAY_AddItem(GUI_ARRAY hArray, const void * pNew, int Len) {
  GUI_ARRAY_OBJ * pThis;
  WM_HMEM         hNewItem = 0;
  WM_HMEM         hNewBuffer;
  WM_HMEM       * pNewBuffer;
  int             r = 0;
  U16 NumItems;
  WM_HMEM haHandle;

  if (hArray) {
    /* Alloc memory for new item */
    if (Len) {
      if ((hNewItem = GUI_ALLOC_AllocInit(pNew, Len)) == 0) {
        GUI_DEBUG_ERROROUT("GUI_ARRAY_AddItem failed to alloc buffer");
        r = 1;            /* Error */
      }
    }
    pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
    NumItems = pThis->NumItems;
    haHandle = pThis->haHandle;
    GUI_UNLOCK_H(pThis);
    /* Put handle of new item into the array */
    if (r == 0) {
      /* Add the handle to new item to the buffer */
      hNewBuffer = GUI_ALLOC_Realloc(haHandle, (NumItems + 1) * sizeof(WM_HMEM));
      if (hNewBuffer == 0) {
        GUI_DEBUG_ERROROUT("GUI_ARRAY_AddItem failed to alloc buffer");
        GUI_ALLOC_Free(hNewItem);
        r = 1;            /* Error */
      } else {
        pNewBuffer = (WM_HMEM *)GUI_LOCK_H(hNewBuffer);
        *(pNewBuffer + NumItems) = hNewItem;
        GUI_UNLOCK_H(pNewBuffer);
        pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
        pThis->haHandle = hNewBuffer;
        pThis->NumItems++;
        GUI_UNLOCK_H(pThis);
      }
    }
  }
  return r;
}

/*********************************************************************
*
*       GUI_ARRAY_Delete
*
* Purpose:
*  Free all allocated memory blocks
*
* Add. info:
*   Locking is not required, since this routine is considered internal
*   and should only be called after locking.
*/
void GUI_ARRAY_Delete(GUI_ARRAY hArray) {
  GUI_ARRAY_OBJ * pThis;
  int i;
  WM_HMEM   ha;
  WM_HMEM * pa;

  if (hArray) {
    pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
    ha = pThis->haHandle;
    if (ha) {
      pa = (WM_HMEM *)GUI_LOCK_H(ha);
      /* Free the attached items, one at a time */
      for (i = 0; i < pThis->NumItems; i++) {
        GUI_ALLOC_FreePtr(pa+i);
      }
      GUI_UNLOCK_H(pa);
      /* Free the handle buffer */
      GUI_ALLOC_FreePtr(&pThis->haHandle);
      pThis->NumItems = 0;                    /* For safety, in case the array is used after it has been deleted */
    }
    GUI_UNLOCK_H(pThis);
    GUI_ALLOC_Free(hArray);
  }
}

/*********************************************************************
*
*       GUI_ARRAY_SetItem
*
* Purpose:
*   Sets an item, returning the handle.
*   If a data pointer is given, the allocated memory is initialized from it thru memcpy.
*
* Returns:
*   Handle of the allocated memory block
*   
* Notes:
*   (1) Replacing Items
*       If the item is already assigned
*       (Which means the handle is already != 0), it is freeed. However,
*       the handle is treated as a handle to a data item, not an object.
*       This means the data item is freed, but if the pointer points to
*       an object, the destructor of the object is not called.
*/
WM_HMEM  GUI_ARRAY_SetItem(GUI_ARRAY hArray, unsigned int Index, const void* pData, int Len) {
  GUI_ARRAY_OBJ * pThis;
  WM_HMEM hItem = 0;
  U16 NumItems;
  WM_HMEM haHandle;

  if (hArray) {
    pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
    NumItems = pThis->NumItems;
    haHandle = pThis->haHandle;
    GUI_UNLOCK_H(pThis);
    if (Index < (unsigned)NumItems) {
      WM_HMEM ha;
      ha = haHandle;
      if (ha) {
        WM_HMEM* pa;
        pa = (WM_HMEM *)GUI_LOCK_H(ha);
        pa += Index;
        hItem = *pa;
        GUI_UNLOCK_H(pa);
        /*
         * If a buffer is already available, a new buffer is only needed when the
         * new item has a different size.
         */
        if (hItem) {
          if (GUI_ALLOC_GetSize(hItem) != Len) {
            hItem = 0;
          }
        }
        /*
         * Allocate a new buffer and free the old one (if needed). 
         */
        if (!hItem) {
          hItem = GUI_ALLOC_AllocZero(Len);
          if (hItem) {
            pa = (WM_HMEM *)GUI_LOCK_H(ha);
            pa += Index;
            GUI_ALLOC_FreePtr(pa);
            *pa = hItem;
            GUI_UNLOCK_H(pa);
          }
        }
        /*
         * Set the item (if needed)
         */
        if (pData && hItem) {
          char * pItem;
          pItem = (char *)GUI_LOCK_H(hItem);
          GUI_MEMCPY(pItem, pData, Len);
          GUI_UNLOCK_H(pItem);
        }
      }
    }
  }
  return hItem;
}

/*********************************************************************
*
*       GUI_ARRAY_GethItem
*
* Purpose:
*   Gets the handle of specified item
*
* Notes:
*   (1) Index out of bounds
*   It is permitted to specify an index larger than the
*   array size. In this case, a 0-handle is returned.
*/
WM_HMEM GUI_ARRAY_GethItem(GUI_ARRAY hArray, unsigned int Index) {
  GUI_ARRAY_OBJ * pThis;
  WM_HMEM h = 0;

  if (hArray) {
    pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
    h = GUI_ARRAY__GethItem(pThis, Index);
    GUI_UNLOCK_H(pThis);
  }
  return h;
}

/*********************************************************************
*
*       GUI_ARRAY_GetpItemLocked
*
* Purpose:
*   Gets the pointer of specified item
*
* Notes:
*   (1) Index out of bounds
*       It is permitted to specify an index larger than the
*       array size. In this case, a 0-handle is returned.
*   (2) Locking
*       It is the caller's responsibility to lock before calling this
*       function.
*       The pointer returned by this function is returned in 'locked'
*       state. This means the caller needs to 'unlock' the pointer
*       after use.
*/
void * GUI_ARRAY_GetpItemLocked(GUI_ARRAY hArray, unsigned int Index) {
  GUI_ARRAY_OBJ * pThis;
  void * p = NULL;

  if (hArray) {
    pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
    p = GUI_ARRAY__GetpItemLocked(pThis, Index);
    GUI_UNLOCK_H(pThis);
  }
  return p;
}

/*********************************************************************
*
*       GUI_ARRAY_Create
*
* Purpose:
*/
GUI_ARRAY GUI_ARRAY_Create(void) {
  GUI_ARRAY hArray;
  hArray = GUI_ALLOC_AllocZero(sizeof(GUI_ARRAY_OBJ));
  return hArray;
}

#else  /* avoid empty object files */

void GUI_ARRAY_C(void);
void GUI_ARRAY_C(void){}

#endif /* GUI_WINSUPPORT */
