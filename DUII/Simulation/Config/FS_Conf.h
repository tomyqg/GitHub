/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2007     SEGGER Microcontroller GmbH & Co KG       *
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
File        : FS_Conf.h
Purpose     : File system configuration
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_CONF_H_
#define _FS_CONF_H_

#ifdef DEBUG
 #if (DEBUG)
   #define FS_DEBUG_LEVEL                     5
 #endif
#endif

#define FS_OS_LOCKING                         1

#ifdef __ICCARM__
  #if __ICCARM__
    #if ((__TID__ >> 4) & 0x0F) < 6   // For any ARM CPU core < v7, we will use optimized routines
      #include "SEGGER.h"
      #define FS_MEMCPY(pDest, pSrc, NumBytes) SEGGER_ARM_memcpy(pDest, pSrc, NumBytes)    // Speed optimization: Our memcpy is much faster!
    #endif
  #endif
#endif

#endif  /* Avoid multiple inclusion */


