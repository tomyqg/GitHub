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
File        : GUI__memcpy.c
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
*       GUI__memcpy
*
* Purpose:
*  Replacement for the memset function. The advantage is high speed
*  on all systems (sometime up to 10 times as fast as the one
*  in the library)
*  Main idea is to write int-wise.
*/
void GUI__memcpy(void * pDest, const void * pSrc, int NumBytes) {
  char * pd;
  const char * ps;
  pd = (char*)pDest;
  ps = (const char*)pSrc;
  /*
  * Copy words if possible
  */
  if ((((U32)ps & 3) == 0) && (((U32)pd & 3) == 0)) {
    unsigned NumWords = NumBytes >> 2;
    while (NumWords >= 4) {
      *(int*)pd = *(const int*)ps;
      pd += 4;
      ps += 4;
      *(int*)pd = *(const int*)ps;
      pd += 4;
      ps += 4;
      *(int*)pd = *(const int*)ps;
      pd += 4;
      ps += 4;
      *(int*)pd = *(const int*)ps;
      pd += 4;
      ps += 4;
      NumWords -= 4;
    }
    if (NumWords) {
      do {
        *(int*)pd = *(const int*)ps;
        pd += 4;
        ps += 4;
      } while (--NumWords);
    }
    NumBytes &= 3;
  }
  /*
  * Copy halfwords if possible
  */
  if ((((U32)ps & 1) == 0) && (((U32)pd & 1) == 0)) {
    unsigned NumHWords = NumBytes >> 1;
    while (NumHWords >= 4) {
      *(short*)pd = *(const short*)ps;
      pd += 2;
      ps += 2;
      *(short*)pd = *(const short*)ps;
      pd += 2;
      ps += 2;
      *(short*)pd = *(const short*)ps;
      pd += 2;
      ps += 2;
      *(short*)pd = *(const short*)ps;
      pd += 2;
      ps += 2;
      NumHWords -= 4;
    }
    if (NumHWords) {
      do {
        *(short*)pd = *(const short*)ps;
        pd += 2;
        ps += 2;
      } while (--NumHWords);
    }
    NumBytes &= 1;
  }
  /*
  * Copy bytes, bulk
  */
  while (NumBytes >= 4) {
    *(char*)pd++ = *(const char*)ps++;
    *(char*)pd++ = *(const char*)ps++;
    *(char*)pd++ = *(const char*)ps++;
    *(char*)pd++ = *(const char*)ps++;
    NumBytes -= 4;
  };
  /*
  * Copy bytes, one at a time
  */
  if (NumBytes) {
    do {
      *(char*)pd++ = *(const char*)ps++;
    } while (--NumBytes);
  };
}

/*************************** End of file ****************************/
