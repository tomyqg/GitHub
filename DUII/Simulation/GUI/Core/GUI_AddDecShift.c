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
File        : GUI_AddDecShift.c
Purpose     : Routines to display values as dec
----------------------------------------------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

const U32 GUI_Pow10[10] = {
  1 , 10, 100, 1000, 10000,
  100000, 1000000, 10000000, 100000000, 1000000000
};

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _Check_NegLong
*
* Purpose:
*   Checks if a long value is negative or not. In case of a negative value
*   it writes a '-' into the given character buffer, increments the buffer
*   pointer and makes the value positive.
*
* Attention:
*   The function does not work with -2147483648 (0x80000000), because the
*   sign of this value can't be changed!
*
* Parameter:
*   pv - Pointer to value
*   ps - Address of buffer pointer
*
* Return value:
*   1 if negatige, 
*/
static int _Check_NegLong(I32 *pv, char**ps) {
  if (*pv < 0) {
    *(*ps)++ = '-';
    *pv = -*pv;
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       Module internal routines
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_Long2Len
*/
int GUI_Long2Len(I32 vSign) {
  int Len = 1;
  I32 v = (vSign > 0) ? vSign : -vSign;
  while ((((U32)v) >= GUI_Pow10[Len]) && (Len < 10)) {
    Len++;
  }
  if (vSign < 0) {
		Len++;
  }
  return Len;
}

/*********************************************************************
*
*       GUI_AddSign
*/
long GUI_AddSign(long v, char**ps) {
  if (v > 0) {
    *(*ps)++ = '+';
    **ps     = '\0';
  }
  return v;
}

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_AddDecShift
*/
void GUI_AddDecShift(I32 v, U8 Len, U8 Shift, char**ps) {
  char c;
  long d;
  U8 Overflow;
  /* I32 range is 0x7fffffff (2147483647) to 0x80000000 (-2147483648).
   * In case of 0x80000000 the function _Check_NegLong() can't work correctly,
   * because the sign of -2147483648 can't be changed!
   */
  if (v == (I32)0x80000000) {
    Overflow = 1;
    v += 1;
    v *= -1;
    *(*ps)++ = '-';
    Len--;
  } else {
    Overflow = 0;
    Len -= _Check_NegLong(&v, ps);
  }
  if (Shift) {
    Len--;
  }
#if GUI_DEBUG_LEVEL > 1
  if (Len > 10) {
    Len = 10;
    GUI_DEBUG_ERROROUT("Can not display more than 10 dec. digits");
  }
#endif
  while (Len) {
    if (Len-- == Shift) {
      *(*ps)++ = GUI_DecChar;
    }
    d = GUI_Pow10[Len];
    c = (char) (v / d);
    if (c > 9) {
      break; /* In case of a too small number of digits stop calculation */
    }
    v -= c * d;
    if (!Len) {
      if (Overflow) {
        c++;
      }
    }
    *(*ps)++ = c + '0';
  }
  **ps = 0;
}

/*************************** End of file ****************************/
