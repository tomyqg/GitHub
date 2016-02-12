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
File        : GUI__memset16.c
Purpose     : Implementation of said function
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI__memset
*
* Purpose:
*  Equivalent to the memset function, for 16 bit Data.
*/
void GUI__memset16(U16 * p, U16 Fill, int NumWords) {
  /* Code for 16 bit CPUs */
  if (sizeof(int) == 2) { /* May some compilers generate a warning at this line: Condition is alwaws true/false */
    if (NumWords >= 8) {  /* May some compilers generate a warning at this line: Unreachable code */
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      NumWords -= 8;
    }
    while (NumWords) {
      *p++ = Fill;
      NumWords--;
    }
  } else {
    int NumInts;          /* May some compilers generate a warning at this line: Unreachable code */
    /* Write 16 bit until we are done or have reached an int boundary */
    if (2 & (U32)p) {
      *p++ = Fill;
      NumWords--;
    }
    /* Write Ints */
    NumInts = (unsigned)NumWords / (sizeof(int) / 2);
    if (NumInts) {
      int FillInt;
      int *pInt;
      FillInt = Fill * 0x10001;

      pInt = (int*)p;
      /* Fill large amount of data at a time */
      if (NumInts >= 4) {
        do {
          *pInt       = FillInt;
          *(pInt + 1) = FillInt;
          *(pInt + 2) = FillInt;
          *(pInt + 3) = FillInt;
          pInt += 4;
        } while ((NumInts -= 4) >= 4);
      }
      /* Fill one int at a time */
      while (NumInts) {
        *pInt++ = FillInt;
        NumInts--;
      }
      p = (U16*)pInt;
    }
    /* Fill the remainder */
    if (NumWords & 1) {
      *p = Fill;
    }
  }
}

/*************************** End of file ****************************/
