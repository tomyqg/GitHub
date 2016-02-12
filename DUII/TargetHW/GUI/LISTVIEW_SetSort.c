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
File        : LISTVIEW_SetSort.c
Purpose     : Implementation of LISTVIEW_SetSort
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "LISTVIEW_Private.h"
// KMC
#include "Utilities.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  LISTVIEW_Obj  * pObj;
  LISTVIEW_SORT * pSort;
  SORT_TYPE     * paSortArray;
} SORT_OBJECT;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _Free
*
* Purpose:
*   Frees the memory used by the attached LISTVIEW_SORT object.
*
* Parameter:
*   hObj - Handle of LISTVIEW_SORT object
*/
static void _Free(WM_HMEM hObj) {
  LISTVIEW_SORT * pSort;
  pSort = (LISTVIEW_SORT *)GUI_LOCK_H(hObj);
  GUI_ALLOC_Free(pSort->hSortArray);
  GUI_UNLOCK_H(pSort);
  GUI_ALLOC_Free(hObj);
}

/*********************************************************************
*
*       _GetSelUnsorted
*/
static int _GetSelUnsorted(LISTVIEW_Obj * pObj) {
  int r;
  if ((LISTVIEW__GetSel(pObj) >= 0) && (pObj->hSort)) {
    LISTVIEW_SORT * pSort;
    pSort = (LISTVIEW_SORT *)GUI_LOCK_H(pObj->hSort);
    if (!pSort->SortArrayNumItems) {
      r = LISTVIEW__GetSel(pObj);
    } else {
      SORT_TYPE * paSortArray;
      paSortArray = (SORT_TYPE *)GUI_LOCK_H(pSort->hSortArray);
      r = *(paSortArray + LISTVIEW__GetSel(pObj));
      GUI_UNLOCK_H(paSortArray);
    }
    GUI_UNLOCK_H(pSort);
  } else {
    r = LISTVIEW__GetSel(pObj);
  }
  return r;
}

/*********************************************************************
*
*       _SetSelUnsorted
*/
static void _SetSelUnsorted(LISTVIEW_Handle hObj, int Sel) {
  LISTVIEW_Obj * pObj;
  WM_HMEM hSort;
  int i, NumItems;
  pObj = LISTVIEW_LOCK_H(hObj);
  hSort = pObj->hSort;
  GUI_UNLOCK_H(pObj);
  if (hSort) {
    LISTVIEW_SORT * pSort;
    SORT_TYPE * paSortArray;
    pSort = (LISTVIEW_SORT *)GUI_LOCK_H(hSort);
    NumItems = pSort->SortArrayNumItems;
    paSortArray = (SORT_TYPE *)GUI_LOCK_H(pSort->hSortArray);
    for (i = 0; i < NumItems; i++) {
      if (*(paSortArray + i) == Sel) {
        LISTVIEW__SetSel(hObj, i);
        break;
      }
    }
    GUI_UNLOCK_H(paSortArray);
    GUI_UNLOCK_H(pSort);
  } else {
    LISTVIEW__SetSel(hObj, Sel);
  }
}

/*********************************************************************
*
*       _Compare
*/
static int _Compare(unsigned lb, unsigned ub, SORT_OBJECT * pSortObject) {
  char * p0, * p1;
  LISTVIEW_ROW * pRow0, * pRow1;
  LISTVIEW_CELL * pCell0, * pCell1;
  LISTVIEW_COLUMN * pColumn;
  SORT_TYPE i0, i1;
  int Result = 0;
  i0 = *(pSortObject->paSortArray + lb);
  i1 = *(pSortObject->paSortArray + ub);
  pRow0   = (LISTVIEW_ROW  *)GUI_ARRAY_GetpItemLocked(pSortObject->pObj->RowArray,  i0);
  pCell0  = (LISTVIEW_CELL *)GUI_ARRAY_GetpItemLocked(pRow0->CellArray, pSortObject->pObj->SortIndex);
  GUI_UNLOCK_H(pRow0);
  if (pCell0) {
    p0 = pCell0->acText;
    GUI_UNLOCK_H(pCell0);
    pRow1   = (LISTVIEW_ROW  *)GUI_ARRAY_GetpItemLocked(pSortObject->pObj->RowArray,  i1);
    pCell1  = (LISTVIEW_CELL *)GUI_ARRAY_GetpItemLocked(pRow1->CellArray, pSortObject->pObj->SortIndex);
    GUI_UNLOCK_H(pRow1);
    if (pCell1) {
      p1 = pCell1->acText;
      GUI_UNLOCK_H(pCell1);
      pColumn = (LISTVIEW_COLUMN *)GUI_ARRAY_GetpItemLocked(pSortObject->pObj->ColumnArray, pSortObject->pObj->SortIndex);
      if (pColumn) {
        Result = pColumn->fpCompare(p0, p1);
        GUI_UNLOCK_H(pColumn);
        if (pSortObject->pSort->Reverse) {
          Result *= -1;
        }
      } else {
        Result = 0;
      }
    }
  }
  return Result;
}

/*********************************************************************
*
*       _Swap
*/
static void _Swap(unsigned lb, unsigned ub, SORT_OBJECT * pSortObject) {
  int Temp;
  Temp = *(pSortObject->paSortArray + lb);
  *(pSortObject->paSortArray + lb) = *(pSortObject->paSortArray + ub);
  *(pSortObject->paSortArray + ub) = Temp;
}

/*********************************************************************
*
*       _BubbleSort
*
* Purpose:
*   Bubble sort algorithm.
*/
static void _BubbleSort(unsigned lb, unsigned ub, SORT_OBJECT * pSortObject) {
  int Swapped;
  do {
    unsigned i;
    Swapped = 0;
    for (i = ub; i > lb; i--) {
      if (_Compare(i - 1, i, pSortObject) > 0) {
        _Swap(i - 1, i, pSortObject);
        Swapped = 1;
      }
    }
  } while (Swapped);
}

/*********************************************************************
*
*       _Reverse
*
* Purpose:
*   Reverse the current sorting order
*/
static void _Reverse(SORT_OBJECT * pSortObject) {
  int i, NumItems;
  NumItems = pSortObject->pSort->SortArrayNumItems;
  for (i = 0; i < NumItems / 2; i++) {
    int Temp;
    Temp                                           = *(pSortObject->paSortArray + i);
    *(pSortObject->paSortArray + i)                = *(pSortObject->paSortArray + NumItems - i - 1);
    *(pSortObject->paSortArray + NumItems - i - 1) = Temp;
  }
}

/*********************************************************************
*
*       _Sort
*
* Purpose:
*   Sorts the contents of the LISTVIEW by using the qsort algorithm.
*   The compare function is called for each compare operation with valid
*   pointers to cell data of the specified column.
*/
static int _Sort(LISTVIEW_Handle hObj) {
  WM_HMEM hSortArray;
  SORT_OBJECT SortObject;
  int NumItems, NumItemsReq, i, Sel;
  SortObject.pObj = LISTVIEW_LOCK_H(hObj); /* +1 */
  if (((SortObject.pObj->IsPresorted) && (SortObject.pObj->IsSorted)) || (SortObject.pObj->hSort == 0)) {
    GUI_UNLOCK_H(SortObject.pObj);
    return 0;
  }
  SortObject.pSort = (LISTVIEW_SORT *)GUI_LOCK_H(SortObject.pObj->hSort); /* +1 */
  NumItemsReq = GUI_ARRAY_GetNumItems(SortObject.pObj->RowArray);
  NumItems    = SortObject.pSort->SortArrayNumItems;
  Sel = _GetSelUnsorted(SortObject.pObj);
  /* Adjust number of items in sort array */
  if (NumItems != NumItemsReq) {
    hSortArray = SortObject.pSort->hSortArray;
    GUI_UNLOCK_H(SortObject.pObj); /* -1 */
    GUI_UNLOCK_H(SortObject.pSort); /* -1 */
    if (!hSortArray) {
      hSortArray = GUI_ALLOC_AllocZero(sizeof(SORT_TYPE) * NumItemsReq);
    } else {
      hSortArray = GUI_ALLOC_Realloc(hSortArray, sizeof(SORT_TYPE) * NumItemsReq);
    }
    if (!hSortArray) {
      return 1;
    }
    SortObject.pObj = LISTVIEW_LOCK_H(hObj); /* +1 */
    SortObject.pSort = (LISTVIEW_SORT *)GUI_LOCK_H(SortObject.pObj->hSort); /* +1 */
    SortObject.pSort->hSortArray = hSortArray;
  }
  SortObject.paSortArray = (SORT_TYPE *)GUI_LOCK_H(SortObject.pSort->hSortArray); /* +1 */
  if (SortObject.pObj->IsPresorted) {
    /* Add new indices */
    if (NumItems < NumItemsReq) {
      SortObject.pObj->ReverseSort = 0; /* Reverse sort only allowed if listview is presorted and no rows are added */
      for (i = NumItems; i < NumItemsReq; i++) {
        *(SortObject.paSortArray + i) = i;
      }
    }
  } else {
    SortObject.pObj->ReverseSort = 0; /* Reverse sort only allowed if listview is presorted */
    /* Fill with indices if not presorted */
    for (i = 0; i < NumItemsReq; i++) {
      *(SortObject.paSortArray + i) = i;
    }
  }
  SortObject.pSort->SortArrayNumItems = NumItemsReq;
  /* Sort only if more than one item is available */
  if (NumItemsReq > 1) {
    if (SortObject.pObj->ReverseSort) {
      _Reverse(&SortObject);
    } else {
      _BubbleSort(0, NumItemsReq - 1, &SortObject);
    }
    _SetSelUnsorted(hObj, Sel);
  }
  SortObject.pObj->IsPresorted = 1;
  SortObject.pObj->IsSorted    = 1;
  HEADER_SetDirIndicator(SortObject.pObj->hHeader, SortObject.pObj->SortIndex, SortObject.pSort->Reverse);
  GUI_UNLOCK_H(SortObject.paSortArray);
  GUI_UNLOCK_H(SortObject.pObj);
  GUI_UNLOCK_H(SortObject.pSort);
  return 0;
}

/*********************************************************************
*
*       _CreateSortObject
*/
static void _CreateSortObject(LISTVIEW_Handle hObj) {
  WM_HMEM hSort;
  LISTVIEW_Obj  * pObj;
  LISTVIEW_SORT * pSort;
  pObj = LISTVIEW_LOCK_H(hObj); /* +1 */
  hSort = pObj->hSort;
  GUI_UNLOCK_H(pObj); /* -1 */
  if (!hSort) {
    hSort = GUI_ALLOC_AllocZero(sizeof(LISTVIEW_SORT));
    if (!hSort) {
      return;
    }
  }
  pObj = LISTVIEW_LOCK_H(hObj);
  pObj->hSort = hSort;
  pSort = (LISTVIEW_SORT *)GUI_LOCK_H(pObj->hSort);
  pObj->SortIndex      = -1;
  pSort->fpFree        = _Free;
  pSort->fpSort        = _Sort;
  GUI_UNLOCK_H(pSort);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTVIEW_CompareText
*/
int LISTVIEW_CompareText(const void * p0, const void * p1) {
// KMC - use case insensitive compare instead  return strcmp((const char *)p1, (const char *)p0);
  return kmc_stricmp((const char *)p1, (const char *)p0);
}

/*********************************************************************
*
*       LISTVIEW_CompareDec
*/
int LISTVIEW_CompareDec(const void * p0, const void * p1) {
  int v[2] = {0}, i, d;
  const char * ps[2];
  char c;
  ps[0] = (const char *)p0;
  ps[1] = (const char *)p1;
  for (i = 0; i < 2; i++) {
    int Neg = 0;
    if (*ps[i] == '-') {
      Neg = 1;
      ps[i]++;
    }
    while ((c = *ps[i]++) != 0) {
      d = c - '0';
      if ((d < 0) || (d > 9)) {
        break;
      }
      v[i] *= 10;
      v[i] += d;
    }
    if (Neg) {
      v[i] *= -1;
    }
  }
  if (v[0] > v[1]) {
    return -1;
  } else if (v[0] < v[1]) {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       LISTVIEW_GetSelUnsorted
*
* Purpose:
*   If sorting is disabled, the behaviour is the same as LISTVIEW_GetSel().
*   It returns the 'visible' selection index.
*   If sorting is enabled, the function returns the 'unselected' index of
*   the selected row.
*/
int LISTVIEW_GetSelUnsorted(LISTVIEW_Handle hObj) {
  int r = -1;
  if (hObj) {
    LISTVIEW_Obj * pObj;
    WM_LOCK();
    _Sort(hObj);
    pObj = LISTVIEW_LOCK_H(hObj);
    r = _GetSelUnsorted(pObj);
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       LISTVIEW_SetSelUnsorted
*
* Purpose:
*   If sorting is disabled, the behaviour is the same as LISTVIEW_SetSel().
*   It sets the 'visible' selection index.
*   If sorting is enabled, the function sets the 'unselected' index of the selected
*   the selected row.
*/
void LISTVIEW_SetSelUnsorted(LISTVIEW_Handle hObj, int Sel) {
  if (hObj) {
    WM_LOCK();
    _Sort(hObj);
    _SetSelUnsorted(hObj, Sel);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTVIEW_SetCompareFunc
*
* Purpose:
*   Sets the compare function for the given column, which is called by
*   the sorting algorithm if sorting is enabled.
*/
void LISTVIEW_SetCompareFunc(LISTVIEW_Handle hObj, unsigned Column, int (* fpCompare)(const void * p0, const void * p1)) {
  int NumColumns;
  LISTVIEW_Obj    * pObj;
  LISTVIEW_COLUMN * pColumn;
  if (hObj) {
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    NumColumns = LISTVIEW_GetNumColumns(hObj);
    if ((int)Column <= NumColumns) {
      pColumn = (LISTVIEW_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->ColumnArray, Column);
      pColumn->fpCompare = fpCompare;
      GUI_UNLOCK_H(pColumn);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTVIEW_DisableSort
*
* Purpose:
*   Disables sorting for the given listview object.
*/
void LISTVIEW_DisableSort(LISTVIEW_Handle hObj) {
  if (hObj) {
    LISTVIEW_Obj * pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    if (pObj->hSort) {
      int Sel = _GetSelUnsorted(pObj);
      _Free(pObj->hSort);
      pObj->hSort       = 0;
      pObj->SortIndex   = -1;
      pObj->IsPresorted = 0;
      pObj->IsSorted    = 0;
      HEADER_SetDirIndicator(pObj->hHeader, -1, 0);
      LISTVIEW_SetSel(hObj, Sel);
      LISTVIEW__InvalidateInsideArea(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTVIEW_EnableSort
*
* Purpose:
*   Enables sorting for the given listview object.
*/
void LISTVIEW_EnableSort(LISTVIEW_Handle hObj) {
  if (hObj) {
    WM_HMEM hSort;
    LISTVIEW_Obj * pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    hSort = pObj->hSort;
    GUI_UNLOCK_H(pObj);
    if (!hSort) {
      _CreateSortObject(hObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTVIEW_SetSort
*
* Purpose:
*   Enables sorting for the given listview object. The function returns,
*   if the object is 0, the column index is >= number of columns or if
*   no compare function has been set for the desired column
*
* Return value:
*   0 if succeed
*   1 if not succeed
*/
unsigned LISTVIEW_SetSort(LISTVIEW_Handle hObj, unsigned Column, unsigned Reverse) {
  LISTVIEW_Obj    * pObj;
  LISTVIEW_SORT   * pSort;
  LISTVIEW_COLUMN * pColumn;
  unsigned NumColumns, Error = 0;
  int (* fpCompare)(const void * p0, const void * p1);

  if (!hObj) {
    return 1;
  }
  NumColumns = LISTVIEW_GetNumColumns(hObj);
  if (Column >= NumColumns) {
    return 1;
  }
  WM_LOCK();
  pObj = LISTVIEW_LOCK_H(hObj);
  pColumn = (LISTVIEW_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->ColumnArray, Column);
  GUI_UNLOCK_H(pObj);
  fpCompare = pColumn->fpCompare;
  GUI_UNLOCK_H(pColumn); /* -1 */
  if (fpCompare) {
    _CreateSortObject(hObj);
    pObj = LISTVIEW_LOCK_H(hObj); /* +1 */
    pSort = (LISTVIEW_SORT *)GUI_LOCK_H(pObj->hSort); /* +1 */
    if (pSort) {
      pObj->SortIndex = Column;
      pSort->Reverse  = Reverse;
      pObj->IsSorted  = 0;
      LISTVIEW__InvalidateInsideArea(hObj);
    } else {
      Error = 1;
    }
    GUI_UNLOCK_H(pSort); /* -1 */
    GUI_UNLOCK_H(pObj); /* -1 */
  } else {
    Error = 1;
  }
  WM_UNLOCK();
  return Error;
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_SetSort_C(void);
  void LISTVIEW_SetSort_C(void) {}
#endif

/*************************** End of file ****************************/
