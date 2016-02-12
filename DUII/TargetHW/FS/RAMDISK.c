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
File        : R_Misc.c
Purpose     : Device Driver for simple array in RAM
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h"

#ifdef WIN32
  #include <windows.h>
#endif


/*********************************************************************
*
*        Local defines
*
**********************************************************************
*/
#ifdef FS_RAMDISK_MAXUNIT
  #define NUM_UNITS FS_RAMDISK_MAXUNIT
#else
  #define NUM_UNITS 2
#endif



/*********************************************************************
*
*        Types
*
**********************************************************************
*/
typedef struct {
  U8 FS_HUGE * pData;
  U32  NumSectors;
  U16  BytesPerSector;
} RAMDISK_PROPS;

/*********************************************************************
*
*        Static data
*
**********************************************************************
*/
static RAMDISK_PROPS _aRAMDISK_PROPS[NUM_UNITS];
static int   _NumUnits;
#ifdef WIN32
static int _ReadDelay;
static int _WriteDelay;
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _GetStatus
*
*  Description:
*    FS driver function. Get status of the RAM disk.
*
*  Parameters:
*    Unit    - Device number.
*
*  Return value:
*    FS_MEDIA_STATE_UNKNOWN    - Media state is unknown
*    FS_MEDIA_NOT_PRESENT      - Media is not present
*    FS_MEDIA_IS_PRESENT       - Media is present
*/
static int _GetStatus(U8 Unit) {
  RAMDISK_PROPS * pRAMDISK_PROPS;

  pRAMDISK_PROPS = &_aRAMDISK_PROPS[Unit];
  return pRAMDISK_PROPS->pData ? FS_MEDIA_IS_PRESENT : FS_MEDIA_NOT_PRESENT;
}



/*********************************************************************
*
*       _Read
*
*  Description:
*    FS driver function. Read a sectors from the RAM disk.
*
*  Parameters:
*    Unit        - Unit number.
*    SectorNo    - Sector to be read from the device.
*    pBuffer     - Pointer to buffer for storing the data.
*    NumSectors  - number of sectors to be read
*
*  Return value:
*    ==0         - Sector has been read and copied to pBuffer.
*    <0          - An error has occurred.
*/
static int _Read(U8 Unit, U32 SectorNo, void *pBuffer, U32 NumSectors) {
  RAMDISK_PROPS * pRAMDISK_PROPS;
  U8 * FS_HUGE pData;

  pRAMDISK_PROPS = &_aRAMDISK_PROPS[Unit];
  if (SectorNo + NumSectors > pRAMDISK_PROPS->NumSectors) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "RAM: _ReadBurst: Sector out of range\n"));
    return -1;
  }
  pData  = pRAMDISK_PROPS->pData;
  pData += SectorNo * pRAMDISK_PROPS->BytesPerSector;
  FS_MEMCPY(pBuffer, (void *)pData, NumSectors * pRAMDISK_PROPS->BytesPerSector);
#ifdef WIN32
  Sleep(_ReadDelay);
#endif
  return 0;
}

/*********************************************************************
*
*       _Write
*
*  Description:
*    Write sectors.
*
*  Parameters:
*    Unit        - Unit number.
*    SectorNo    - First sector to be written to the device.
*    NumSectors  - Number of sectors to be written to the device.
*    pBuffer     - Pointer to buffer for holding the data.
*
*  Return value:
*    ==0         - O.K.: Sector has been written to device.
*    <0          - An error has occurred.
*/
static int _Write(U8 Unit, U32 SectorNo, const void *pBuffer, U32 NumSectors, U8 RepeatSame) {
  RAMDISK_PROPS * pRAMDISK_PROPS;
  U8 * FS_HUGE pData;

  pRAMDISK_PROPS = &_aRAMDISK_PROPS[Unit];
  if (SectorNo + NumSectors > pRAMDISK_PROPS->NumSectors) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "RAM: _WriteBurst: Sector out of range\n"));
    return -1;
  }
  pData  = pRAMDISK_PROPS->pData;
  pData += SectorNo * pRAMDISK_PROPS->BytesPerSector;
  if (RepeatSame) {
    do {
      FS_MEMCPY((void *)pData, pBuffer, pRAMDISK_PROPS->BytesPerSector);
      pData += pRAMDISK_PROPS->BytesPerSector;
    } while (--NumSectors);
  } else {
      FS_MEMCPY((void *)pData, pBuffer, pRAMDISK_PROPS->BytesPerSector * NumSectors);
  }
#ifdef WIN32
  Sleep(_WriteDelay);
#endif
  return 0;
}

/*********************************************************************
*
*       _IoCtl
*
*  Description:
*    FS driver function. Execute device command.
*
*  Parameters:
*    Unit        - Unit number.
*    Cmd         - Command to be executed.
*    Aux         - Parameter depending on command.
*    pBuffer     - Pointer to a buffer used for the command.
*
*  Return value:
*    Command specific. In general a negative value means an error.
*/
static int _IoCtl(U8 Unit, I32 Cmd, I32 Aux, void *pBuffer) {
  RAMDISK_PROPS * pRAMDISK_PROPS;
  FS_DEV_INFO *pInfo;
  FS_USE_PARA(Aux);

  pRAMDISK_PROPS = &_aRAMDISK_PROPS[Unit];
  switch (Cmd) {
  case FS_CMD_GET_DEVINFO:
    if (!pBuffer) {
      return -1;
    }
    pInfo = (FS_DEV_INFO *)pBuffer;
    pInfo->NumSectors      = pRAMDISK_PROPS->NumSectors;
    pInfo->BytesPerSector  = pRAMDISK_PROPS->BytesPerSector;
    break;
#ifdef WIN32
  case FS_CMD_SET_DELAY:
    _ReadDelay  = Aux;
    _WriteDelay = (int)pBuffer;
    break;
#endif

  default:
    break;
  }
  return 0;
}


/*********************************************************************
*
*       _AddDevice
*/
static int _AddDevice(void) {
  if (_NumUnits >= NUM_UNITS) {
    return -1;
  }
  return _NumUnits++;
}

/*********************************************************************
*
*       _GetNumUnits
*/
static int _GetNumUnits(void) {
  return _NumUnits;
}

/*********************************************************************
*
*       _GetDriverName
*/
static const char * _GetDriverName(U8 Unit) {
  FS_USE_PARA(Unit);
  return "ram";
}


/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

const FS_DEVICE_TYPE FS_RAMDISK_Driver = {
  _GetDriverName,
  _AddDevice,
  _Read,
  _Write,
  _IoCtl,
  NULL,
  _GetStatus,
  _GetNumUnits
};


/*********************************************************************
*
*       FS_RAMDISK_Configure
*
*  Description:
*    Configures a single instance of the driver
*/
void FS_RAMDISK_Configure(U8 Unit, void * pData, U16 BytesPerSector, U32 NumSectors) {
  RAMDISK_PROPS * pRAMDISK_PROPS;

// TBD: Add parameter checks in debug build

  pRAMDISK_PROPS = &_aRAMDISK_PROPS[Unit];
  pRAMDISK_PROPS->pData          = (U8 *)pData;
  pRAMDISK_PROPS->NumSectors     = NumSectors;
  pRAMDISK_PROPS->BytesPerSector = BytesPerSector;
}

/*************************** End of file ****************************/
