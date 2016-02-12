/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

**** emFile file system for embedded applications ****
emFile is protected by international copyright laws. Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : FS_memcpy.c
Purpose     : Implementation of said function
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "FS_Int.h"

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       FS_memcpy
*
* Purpose:
*  Replacement for the memset function. The advantage is high speed
*  on all systems (sometime up to 10 times as fast as the one
*  in the library)
*  Main idea is to write int-wise.
*/
void FS_memcpy(void * pDest, const void * pSrc, int NumBytes) {
  char * pd;
  const char * ps;
  pd = (char*)pDest;
  ps = (const char*)pSrc;
  //
  // Check if size of integer is 4 bytes
  // so that we can copy 32 bitwise.
  //
  if (sizeof(int) == 4) {
    //
    // Copy words if possible
    //
    if ((((U32)ps & 3) == 0) && (((U32)pd & 3) == 0)) {
      unsigned NumWords = NumBytes >> 2;
      while (NumWords >= 4) {
        *(U32 *)pd = *(const U32 *)ps;
        pd += 4;
        ps += 4;
        *(U32 *)pd = *(const U32*)ps;
        pd += 4;
        ps += 4;
        *(U32 *)pd = *(const U32 *)ps;
        pd += 4;
        ps += 4;
        *(U32 *)pd = *(const U32 *)ps;
        pd += 4;
        ps += 4;
        NumWords -= 4;
      }
      if (NumWords) {
        do {
          *(U32 *)pd = *(const U32 *)ps;
          pd += 4;
          ps += 4;
        } while (--NumWords);
      }
      NumBytes &= 3;
    }
  }
  //
  // Copy halfwords if possible
  //
  if ((((U32)ps & 1) == 0) && (((U32)pd & 1) == 0)) {
    unsigned NumHWords = NumBytes >> 1;
    while (NumHWords >= 4) {
      *(short *)pd = *(const short *)ps;
      pd += 2;
      ps += 2;
      *(short *)pd = *(const short *)ps;
      pd += 2;
      ps += 2;
      *(short *)pd = *(const short *)ps;
      pd += 2;
      ps += 2;
      *(short *)pd = *(const short *)ps;
      pd += 2;
      ps += 2;
      NumHWords -= 4;
    }
    if (NumHWords) {
      do {
        *(short *)pd = *(const short *)ps;
        pd += 2;
        ps += 2;
      } while (--NumHWords);
    }
    NumBytes &= 1;
  }
  //
  // Copy bytes, bulk
  //
  while (NumBytes >= 4) {
    *(char *)pd++ = *(const char *)ps++;
    *(char *)pd++ = *(const char *)ps++;
    *(char *)pd++ = *(const char *)ps++;
    *(char *)pd++ = *(const char *)ps++;
    NumBytes -= 4;
  };
  //
  // Copy bytes, one at a time
  //
  if (NumBytes) {
    do {
      *(char*)pd++ = *(const char*)ps++;
    } while (--NumBytes);
  };
}

/*************************** End of file ****************************/
