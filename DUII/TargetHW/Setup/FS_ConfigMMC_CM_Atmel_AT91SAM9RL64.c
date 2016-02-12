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
----------------------------------------------------------------------
File        : FS_ConfigMMC_CM_Atmel_AT91SAM9RL64.c
Purpose     : Configuration file for FS with MMC SPI mode driver
---------------------------END-OF-HEADER------------------------------
*/


#include <stdio.h>
#include "SPI_API.h"
#include "FS.h"

/*********************************************************************
*
*       Defines, configurable
*
*       This section is the only section which requires changes
*       using the MMC/SD card mode disk driver as a single device.
*
**********************************************************************
*/
extern const FS_DEVICE_TYPE FS_MMC_CM_Driver4Atmel;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
// *** GAS TEST
//static U32   _aMemBlock[ALLOC_SIZE / 4];      // Memory pool used for semi-dynamic allocation.
U32 *_aMemBlock = (U32 *) (TOP_OF_ISRAM	- DMA_SIZE - FS_ALLOC_SIZE);		// just below DMA buffer

/*********************************************************************
*
*       Public code
*
*       This section does not require modifications in most systems.
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_X_AddDevices
*
*  Function description
*    This function is called by the FS during FS_Init().
*    It is supposed to add all devices, using primarily FS_AddDevice().
*
*  Note
*    (1) Other API functions
*        Other API functions may NOT be called, since this function is called
*        during initialization. The devices are not yet ready at this point.
*/
void FS_X_AddDevices(void) {
// *** GAS TEST
//  FS_AssignMemory(&_aMemBlock[0], sizeof(_aMemBlock));
//	FS_ConfigFileBufferDefault(512, 0);		// this does not work. don't do it.
//	FS_ConfigFileBufferDefault(ALLOC_SIZE, 0);		// this works. does not help.
	FS_AssignMemory(_aMemBlock, FS_ALLOC_SIZE);
  FS_AddDevice(&FS_MMC_CM_Driver4Atmel);
}

/*********************************************************************
*
*       FS_X_GetTimeDate
*
*  Description:
*    Current time and date in a format suitable for the file system.
*
*    Bit 0-4:   2-second count (0-29)
*    Bit 5-10:  Minutes (0-59)
*    Bit 11-15: Hours (0-23)
*    Bit 16-20: Day of month (1-31)
*    Bit 21-24: Month of year (1-12)
*    Bit 25-31: Count of years from 1980 (0-127)
*
*/

extern U32 getTimeStamp ( void );

U32 FS_X_GetTimeDate(void) {
	U32 r;

	if (0 == (r = getTimeStamp())) {
		// if no GPS Time yet, set to Jan 1, 2011 at midnight
		U16 Sec   = 0;			// 0 based.  Valid range: 0..59
		U16 Min   = 0;			// 0 based.  Valid range: 0..59
		U16 Hour  = 0;			// 0 based.  Valid range: 0..23
		U16 Day   = 1;			// 1 based.    Means that 1 is 1. Valid range is 1..31 (depending on month)
		U16 Month = 1;			// 1 based.    Means that January is 1. Valid range is 1..12.
		U16 Year  = 31;			// 1980 based. Means that 2007 would be 27.

		r   = Sec / 2 + (Min << 5) + (Hour  << 11);
		r  |= (U32)(Day + (Month << 5) + (Year  << 9)) << 16;
	} // if

  return r;
}

/*************************** End of file ****************************/
