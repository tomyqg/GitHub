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
File        : FS_LogVolume.c
Purpose     : Logical volume driver
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h"

#ifndef   NUM_UNITS
  #define NUM_UNITS  4
#endif

typedef struct DEV_INFO DEV_INFO;

struct DEV_INFO {
  DEV_INFO             * pNext;
  const FS_DEVICE_TYPE * pDevice;
  U8                     Unit;
  U32                    StartOff;
  U32                    NumSectors;
};

typedef struct {
  const char * sVolName;
  DEV_INFO   * pDevInfo;
  U16          BytesPerSector;
} LOG_VOL;



/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static LOG_VOL      * _apLogVol[NUM_UNITS];
static int            _NumUnits;

/*********************************************************************
*
*       Macros, function replacement
*
**********************************************************************
*/

/*********************************************************************
*
*       ASSERT_UNIT_NO_IS_IN_RANGE
*/
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL
  #define ASSERT_UNIT_NO_IS_IN_RANGE(Unit)                                \
  if (Unit >= _NumUnits) {                                \
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "NAND: Illegal unit number.\n"));     \
    return -1;                                            \
  }
#else
  #define ASSERT_UNIT_NO_IS_IN_RANGE(Unit)
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _GetNumSectors
*
*/
static U32 _GetNumSectors(LOG_VOL * pLogVol) {
  U32 NumSectors;
  DEV_INFO * pDevInfo;
  
  
  NumSectors = 0;
  for (pDevInfo = pLogVol->pDevInfo; pDevInfo; pDevInfo = pDevInfo->pNext) {
    NumSectors += pDevInfo->NumSectors;
  }
  return NumSectors;
}


/*********************************************************************
*
*       _UpdateNumSectors
*
*/
static void _UpdateNumSectors(LOG_VOL * pLogVol) {
  DEV_INFO     * pDevInfo;
  FS_DEV_INFO    DevInfo;
  
  for (pDevInfo = pLogVol->pDevInfo; pDevInfo; pDevInfo = pDevInfo->pNext) {
    if (pDevInfo->pDevice->pfIoCtl(pDevInfo->Unit, FS_CMD_GET_DEVINFO, 0, (void*)&DevInfo) == 0) {
       pDevInfo->NumSectors    = DevInfo.NumSectors;
       pLogVol->BytesPerSector = DevInfo.BytesPerSector;
    }
  }
}

/*********************************************************************
*
*       _LOGVOL_GetStatus
*
*/
static int _LOGVOL_GetStatus(U8 Unit) {  
  FS_USE_PARA(Unit);
  return 1;
}

/*********************************************************************
*
*       _ReadWrite
*/
static int _ReadWrite(U8 Unit, U32 FirstSectorReq, const void *pBuffer, U32 NumSectorsReq, U8 IsWrite, U8 RepeatSame) {
  LOG_VOL  * pLogVol;
  DEV_INFO * pDevInfo;
  int        r;
  U32        NumSectorsAtOnce;
  U32        NumSectorsInDev;           // Number of sectors in device

  r = -1;
  ASSERT_UNIT_NO_IS_IN_RANGE(Unit);
  pLogVol = _apLogVol[Unit];
  //
  // Iterate over devices until we have reached the last device or all data has been read.
  //
  pDevInfo = pLogVol->pDevInfo;
  while (pDevInfo) {
    NumSectorsInDev = pDevInfo->NumSectors;
    if (FirstSectorReq < NumSectorsInDev) {
      NumSectorsAtOnce = MIN(NumSectorsInDev - FirstSectorReq, NumSectorsReq);
      if (IsWrite) {
        r = pDevInfo->pDevice->pfWrite(pDevInfo->Unit, FirstSectorReq + pDevInfo->StartOff, pBuffer, NumSectorsAtOnce, RepeatSame);
      } else {
        r = pDevInfo->pDevice->pfRead (pDevInfo->Unit, FirstSectorReq + pDevInfo->StartOff, (void *)pBuffer, NumSectorsAtOnce);
      }
      if (r) {
        break;    // Error;
      }
      NumSectorsReq   -= NumSectorsAtOnce;
      FirstSectorReq  += NumSectorsAtOnce;
      {
        const U8 * p;
        p        = (const U8 *)pBuffer;
        p       += NumSectorsAtOnce * pLogVol->BytesPerSector;
        pBuffer  = p;
      }
    }
    FirstSectorReq -= NumSectorsInDev;
    if (NumSectorsReq == 0) {
      r = 0;                    // O.K., all sectors read
      break;
    }
    pDevInfo = pDevInfo->pNext;
  }
  return r;
}

/*********************************************************************
*
*       _LOGVOL_Read
*/
static int _LOGVOL_Read(U8 Unit, U32 FirstSectorReq, void *pBuffer, U32 NumSectorsReq) {
  return _ReadWrite(Unit, FirstSectorReq, pBuffer, NumSectorsReq, 0, 0);
}

/*********************************************************************
*
*       _LOGVOL_Write
*/
static int _LOGVOL_Write(U8 Unit, U32 FirstSector, const void *pBuffer, U32 NumSectors, U8 RepeatSame) {
  return _ReadWrite(Unit, FirstSector, (const void*)pBuffer, NumSectors, 1, RepeatSame);
}

/*********************************************************************
*
*       _LOGVOL_IoCtl
*
*/
static int _LOGVOL_IoCtl(U8 Unit, I32 Cmd, I32 Aux, void *pBuffer) {
  LOG_VOL     * pLogVol;
  FS_DEV_INFO * pInfo;
  DEV_INFO    * pDevInfo;
  int           DriverReturn;
  int           r;
  U32           NumSectors; 

  ASSERT_UNIT_NO_IS_IN_RANGE(Unit);
  pLogVol = _apLogVol[Unit];
  FS_USE_PARA(Aux);
  r = -1;
  switch (Cmd) {
  case FS_CMD_GET_DEVINFO:
    if (!pBuffer) {
      return -1;
    }
    pInfo = (FS_DEV_INFO *)pBuffer;
    NumSectors             = _GetNumSectors(pLogVol);
    if (NumSectors == 0) {
      _UpdateNumSectors(pLogVol);
    }
    NumSectors             = _GetNumSectors(pLogVol);
    pInfo->NumSectors      = NumSectors;
    pInfo->BytesPerSector  = pLogVol->BytesPerSector;
    r                      = 0;
    break;
  case FS_CMD_DEINIT:
    pDevInfo = pLogVol->pDevInfo;
    while (pDevInfo) {
      DEV_INFO * pDevNext;

      pDevNext = pDevInfo->pNext;
      FS_Free(pDevInfo);
      pDevInfo = pDevNext;
    }
    FS_FREE(pLogVol);
    _NumUnits--;
    return 0;
  default:
    pDevInfo = pLogVol->pDevInfo;
    r        = 0;
    while (pDevInfo) {      
      DriverReturn = pDevInfo->pDevice->pfIoCtl(pDevInfo->Unit, Cmd, Aux, pBuffer);
      if (DriverReturn != 0) {
        r = DriverReturn;
      }
      pDevInfo = pDevInfo->pNext;
    }
    break;
  }
  return r;
}

/*********************************************************************
*
*       _LOGVOL_AddDevice
*
*  Note:
*    No functionality required here.
*/
static int _LOGVOL_AddDevice(void) {
  return _NumUnits;
}

/*********************************************************************
*
*       _LOGVOL_GetNumUnits
*
*/
static int _LOGVOL_GetNumUnits(void) {
  return _NumUnits;
}


/*********************************************************************
*
*       _GetDriverName
*/
static const char * _LOGVOL_GetDriverName(U8 Unit) {
  const char * sVolName;
  if (_apLogVol[Unit]) {
    sVolName = _apLogVol[Unit]->sVolName;
  } else {
    sVolName = "";
  }
  return sVolName;
}

/*********************************************************************
*
*       Public code, internal
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_LOGVOL_AddDevice
*
*/
static const FS_DEVICE_TYPE _LOGVOL_Driver = {
  _LOGVOL_GetDriverName,
  _LOGVOL_AddDevice,
  _LOGVOL_Read,
  _LOGVOL_Write,
  _LOGVOL_IoCtl,
  NULL,
  _LOGVOL_GetStatus,
  _LOGVOL_GetNumUnits
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_LOGVOL_Create
*
*/
int FS_LOGVOL_Create(const char * sVolName) {
  int r;
  LOG_VOL * pLogVol;
  FS_VOLUME * pVolume;

  FS_LOCK();
  r = -1;
  if ((_NumUnits + 1) < NUM_UNITS) {
    pVolume = FS__AddDevice(&_LOGVOL_Driver);
    if (pVolume) {
      pLogVol = (LOG_VOL *)FS_AllocZeroed(sizeof(LOG_VOL));
      if (pLogVol) {
        pLogVol->sVolName = sVolName;
        _apLogVol[_NumUnits++] = pLogVol;
        r = 0;
      }
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_LOGVOL_AddDevice
*
*/
int FS_LOGVOL_AddDevice(const char * sLogVolName, const FS_DEVICE_TYPE * pDevice, U8 Unit, U32 StartOff, U32 NumSectors) {
  int       i;
  LOG_VOL * pLogVol;
  int       r;

  FS_LOCK();
  r = -1;
  //
  // Find log volume
  //
  for (i = 0; i < _NumUnits; i++) {
    pLogVol = _apLogVol[i];
    if (strcmp(sLogVolName, pLogVol->sVolName) == 0) {
      DEV_INFO             * pDevInfo;
      DEV_INFO             ** ppPrevNext;
      FS_DEV_INFO            DevInfo;

      FS_MEMSET(&DevInfo, 0, sizeof(DevInfo));
      //
      // Get info from device
      //
      if (pDevice->pfIoCtl(Unit, FS_CMD_GET_DEVINFO, 0, (void*)&DevInfo)) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "FS_LOGVOL_AddDevice: Could not retrieve device information from device.\n"));
      }
      if (NumSectors == 0) {
        NumSectors = DevInfo.NumSectors;
      }
      if (DevInfo.NumSectors < NumSectors) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "FS_LOGVOL_AddDevice: Device has less sector than requested.\n"));
        break;
      }
      //
      // For first device added, set BytesPerSector. All add. devices added need to have the same sector size.
      //
      if ((void*)pLogVol->pDevInfo == NULL) {
        pLogVol->BytesPerSector = DevInfo.BytesPerSector;
      }
      if (pLogVol->BytesPerSector != DevInfo.BytesPerSector) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "FS_LOGVOL_AddDevice: Devices with different sector size can not be combined.\n"));
        break;
      }
      //
      // Goto end of list
      //
      ppPrevNext   = &pLogVol->pDevInfo;
      for (; pDevInfo = *ppPrevNext, pDevInfo; ppPrevNext = &pDevInfo->pNext);
      //
      // Alloc memory for new device, fill it in and add it to the linked list
      //
      pDevInfo    = (DEV_INFO *)FS_AllocZeroed(sizeof(DEV_INFO));
      pDevInfo->NumSectors = NumSectors;
      pDevInfo->StartOff   = StartOff;
      pDevInfo->pDevice    = pDevice;
      pDevInfo->Unit       = Unit;
      *ppPrevNext = pDevInfo;
      r = 0;
      break;
    }
  }
  FS_UNLOCK();
  return r;
}





/*************************** End of file ****************************/

