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
File        : FS_LogBlock.c
Purpose     : Logical Block Layer
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "FS_ConfDefaults.h"        /* FS Configuration */
#include "FS_Int.h"

/*********************************************************************
*
*       Macros
*
**********************************************************************
*/

#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA
  #define CALL_ONDEVICE_HOOK(pDevice, Operation, StartSector, NumSectors, Sectortype)                        \
    {                                                                                                        \
      if (pDevice->Data.pfOnDeviceActivityHook) {                                                            \
        (pDevice->Data.pfOnDeviceActivityHook)(pDevice, Operation, StartSector, NumSectors, Sectortype);     \
      }                                                                                                      \
    }
#else
  #define CALL_ONDEVICE_HOOK(pDevice, Operation, StartSector, NumSectors, Sectortype)
#endif

#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA
  #define INC_READ_SECTOR_CNT(NumSectors)           {FS_STORAGE_Counters.ReadOperationCnt++;  FS_STORAGE_Counters.ReadSectorCnt   += NumSectors;}
  #define INC_WRITE_SECTOR_CNT(NumSectors)          {FS_STORAGE_Counters.WriteOperationCnt++; FS_STORAGE_Counters.WriteSectorCnt  += NumSectors;}
  #define INC_READ_CACHE_HIT_CNT()                  {FS_STORAGE_Counters.ReadSectorCachedCnt++;}
#else
  #define INC_READ_SECTOR_CNT(NumSectors)
  #define INC_WRITE_SECTOR_CNT(NumSectors)
  #define INC_READ_CACHE_HIT_CNT()
#endif

/*********************************************************************
*
*       Code & data for debug builds
*
**********************************************************************
*/
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_ALL

typedef struct {
  int Type;
  const char *s;
} TYPE_DESC;

#define TYPE2NAME(Type)  { Type,          #Type }

static const TYPE_DESC _aDesc[] = {
  { FS_SECTOR_TYPE_DATA, "DATA" },
  { FS_SECTOR_TYPE_MAN,  "MAN " },
  { FS_SECTOR_TYPE_DIR,  "DIR " },
};

/*********************************************************************
*
*       _Type2Name
*
*/
// static   -- Code is static, but not declared as such in order to avoid compiler warnings if this function is not referenced (lower debug levels)
const char * _Type2Name(int Type);    // Avoid "No prototype" warning
const char * _Type2Name(int Type) {
  unsigned i;
  for (i = 0; i < COUNTOF(_aDesc); i++) {
    if (_aDesc[i].Type == Type) {
      return _aDesc[i].s;
    }
  }
  return "Unknown Type";
}

#else

#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

#if FS_SUPPORT_BUSY_LED
  #define CLR_BUSY_LED(pDevice) _ClrBusyLED(pDevice)
  #define SET_BUSY_LED(pDevice) _SetBusyLED(pDevice)


/*********************************************************************
*
*       _ClrBusyLED
*
*  Function description
*    Calls the user supplied callback (hook) to switch off the BUSY-LED.
*/
static void _ClrBusyLED(FS_DEVICE * pDevice) {
  if (pDevice->Data.pfSetBusyLED) {
    pDevice->Data.pfSetBusyLED(0);
  }
}

/*********************************************************************
*
*       _SetBusyLED
*
*  Function description
*    Calls the user supplied callback (hook) to switch on the BUSY-LED.
*/
static void _SetBusyLED(FS_DEVICE * pDevice) {
  if (pDevice->Data.pfSetBusyLED) {
    pDevice->Data.pfSetBusyLED(1);
  }

}

#else
  #define CLR_BUSY_LED(pDevice)
  #define SET_BUSY_LED(pDevice)
#endif

/*********************************************************************
*
*       _Read
*
*  Function description
*    Static helper function for Write either to device or thru journal.
*
*  Return value
*    ==0         - All sectors have been read sucessfully.
*    <0          - An error has occurred.
*
*/
static int _Read(FS_DEVICE * pDevice, U32 SectorNo, void * pBuffer, U32 NumSectors, U8 Type) {
  int r;
  const FS_DEVICE_TYPE * pDeviceType;
  pDeviceType = pDevice->pType;
  CALL_ONDEVICE_HOOK(pDevice, FS_OPERATION_READ, SectorNo, NumSectors, Type);
#if FS_SUPPORT_JOURNAL
  if (pDevice->Data.JournalIsActive) {
    r = FS__JOURNAL_Read(pDevice, SectorNo, pBuffer, NumSectors);
  } else
#endif
  {
    r = (pDeviceType->pfRead)(pDevice->Data.Unit, SectorNo, pBuffer, NumSectors);
  }
#if FS_SUPPORT_CACHE
  if (pDevice->Data.pCacheAPI) {
    if (r == 0) {                   // Read from device was successfully
      U16 SectorSize;
      
      SectorSize = FS_GetSectorSize(pDevice);
      do {
        if (pDevice->Data.pCacheAPI->pfUpdateCache(pDevice, SectorNo, pBuffer, Type)) {
          FS_DEBUG_WARN((FS_MTYPE_STORAGE, "Could not update sector in cache"));
        }
        SectorNo++;
        pBuffer = (void *)((U8 *)pBuffer + SectorSize);
      } while (--NumSectors);
    }
  }
#endif
  return r;
}

/*********************************************************************
*
*       _Write
*
*  Function description
*    Static helper function for Write either to device or thru journal.
*
*  Return value
*    ==0           - All sectors have been written successfully.
*    < 0           - An error has occurred.
*
*/
static int _Write(const FS_DEVICE * pDevice, U32 SectorNo, const void * pBuffer, U32 NumSectors, U8 RepeatSame) {
  int r;
  const FS_DEVICE_TYPE * pDeviceType;
  pDeviceType = pDevice->pType;
#if FS_SUPPORT_JOURNAL
  if (pDevice->Data.JournalIsActive) {
    r = FS__JOURNAL_Write(pDevice, SectorNo, pBuffer, NumSectors, RepeatSame);
  } else
#endif
  {
    r = (pDeviceType->pfWrite)(pDevice->Data.Unit, SectorNo, pBuffer, NumSectors, RepeatSame);
  }
  return r;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*
*        They should not be called by user application.
*/

/*********************************************************************
*
*       FS_LB_GetStatus
*
*  Function description
*    FS internal function. Get status of a device.
*
*  Parameters
*    pDevice     - Pointer to a device driver structure.
*
*  Return value
*    FS_MEDIA_STATE_UNKNOWN  if the state of the media is unknown.
*    FS_MEDIA_NOT_PRESENT    if media is not present.
*    FS_MEDIA_IS_PRESENT     if media is     present.
*
*/
int FS_LB_GetStatus(FS_DEVICE * pDevice) {
  int x;
  const FS_DEVICE_TYPE * pDeviceType;

  pDeviceType = pDevice->pType;
  x = (pDeviceType->pfGetStatus)(pDevice->Data.Unit);
  return x;
}

/*********************************************************************
*
*       FS_LB_InitMedium
*
*  Function description
*    This function calls the initialize routine of the driver, if one exists.
*    If there if no initialization routine available, we assume the driver is
*    handling this automatically.
*
*  Parameters
*    pDevice     - Pointer to a device driver structure.
*
*  Return value
*    1           - Device/medium has been initialized.
*    0           - Error, device/medium could not be initialized.
*/
int FS_LB_InitMedium(FS_DEVICE * pDevice) {
  int IsInited = 0;
  const FS_DEVICE_TYPE * pDeviceType;

  pDeviceType = pDevice->pType;
  if (pDeviceType->pfInitMedium) {
    if ((pDeviceType->pfInitMedium)(pDevice->Data.Unit) == 0) {
      IsInited = 1;
    }
  } else {
    IsInited = 1;
  }
  pDevice->Data.IsInited = (U8)IsInited;
  return IsInited;
}

/*********************************************************************
*
*       FS_LB_InitMediumIfRequired
*
*  Function description
*    Initialize medium if it has not already been initialized.
*
*  Parameters
*    pDevice     - Pointer to a device driver structure.
*
*/
int FS_LB_InitMediumIfRequired(FS_DEVICE * pDevice) {
  if (pDevice->Data.IsInited == 0) {
    FS_LB_InitMedium(pDevice);
  }
  return pDevice->Data.IsInited;
}


/*********************************************************************
*
*       FS_LB_ReadDevice
*
*  Function description
*    Read sector from device. It also checks whether the sector can be
*    read from the cache if available.
*
*  Parameters
*    pDriver     - Pointer to a device driver structure.
*    Sector      - Physical sector to be read from the device.
*                  The Partition start sector is not added.
*    pBuffer     - Pointer to buffer for storing the data.
*    Type        - The type of sector that shall be read.
*
*  Return value
*    ==0         - Sector has been read and copied to pBuffer.
*    < 0         - An error has occurred.
*/
int FS_LB_ReadDevice(FS_DEVICE *pDevice, U32 Sector, void *pBuffer, U8 Type) {
  int x;

  FS_LB_InitMediumIfRequired(pDevice);
  INC_READ_SECTOR_CNT(1);             // For statistics / debugging only
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "Read          %s:%d: %s Sector: 0x%8x", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, _Type2Name(Type), Sector));
  SET_BUSY_LED(pDevice);
#if FS_SUPPORT_CACHE
  {
    const FS_DEVICE_DATA * pDevData;
    const FS_CACHE_API   * pCacheAPI;
    pDevData   = &pDevice->Data;
    pCacheAPI  = pDevData->pCacheAPI;
    if (pCacheAPI) {
      if ((pCacheAPI->pfReadFromCache)(pDevice, Sector, pBuffer, Type)) {
        x = _Read(pDevice, Sector, pBuffer, 1, Type);
      } else {
        INC_READ_CACHE_HIT_CNT();             // For statistics / debugging only
        FS_DEBUG_LOG((FS_MTYPE_STORAGE, " (cache)"));
        x = 0;
      }
    } else {
      x = _Read(pDevice, Sector, pBuffer, 1, Type);
    }
  }
#else
    FS_USE_PARA(Type);
    x = _Read(pDevice, Sector, pBuffer, 1, Type);
#endif
  CLR_BUSY_LED(pDevice);
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "\n"));
  return  x;
}

/*********************************************************************
*
*       FS_LB_ReadPart
*
*  Function description
*    Read sector from volume.
*
*  Parameters
*    pDriver     - Pointer to a device driver structure.
*    Sector      - Physical sector to be read from the partition.
*                  The partition start sector is added.
*    pBuffer     - Pointer to buffer for storing the data.
*    Type        - The type of sector that shall be read.
*
*  Return value
*    ==0         - Sector has been read and copied to pBuffer.
*    <0          - An error has occurred.
*/
int FS_LB_ReadPart(FS_PARTITION *pPart, U32 Sector, void *pBuffer, U8 Type) {
  return FS_LB_ReadDevice(&pPart->Device, pPart->StartSector + Sector, pBuffer, Type);
}

/*********************************************************************
*
*       FS_LB_ReadBurst
*
*  Function description
*    Read multiple sectors from device.
*
*  Parameters
*    pDriver     - Pointer to a device driver structure.
*    Unit        - Unit number.
*    SectorNo    - First sector to be read from the device.
*    NumSectors  - Number of sectors to be read from the device.
*    pBuffer     - Pointer to buffer for storing the data.
*    Type        - The type of sector that shall be read.
*
*  Return value
*    ==0         - Sectors have been read and copied to pBuffer.
*    <0          - An error has occurred.
*/
int FS_LB_ReadBurst(FS_DEVICE * pDevice, U32 SectorNo, U32 NumSectors, void *pBuffer, U8 Type) {
  int x = 0;

  FS_LB_InitMediumIfRequired(pDevice);
  INC_READ_SECTOR_CNT(NumSectors);             // For statistics / debugging only
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "ReadBurst     %s:%d: %s Sector: 0x%8x, NumSectors: %d", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, _Type2Name(Type), SectorNo, NumSectors));
  SET_BUSY_LED(pDevice);
#if FS_SUPPORT_CACHE
  {
    const FS_CACHE_API * pCacheAPI;
    U32                  NumSectors2Read;
    U32                  FirstSector;
    U16                  SectorSize;
    char                 NeedReadBurst;
    void               * pReadBuffer;
    NeedReadBurst   = 0;
    NumSectors2Read = 0;
    FirstSector     = 0;
    pReadBuffer     = NULL;
    pCacheAPI       = pDevice->Data.pCacheAPI;
    SectorSize      = FS_GetSectorSize(pDevice);
    if (pCacheAPI) {
      U8 * p;
      char    r;
      p = (U8 *)pBuffer;
      do {
        r = (pCacheAPI->pfReadFromCache)(pDevice, SectorNo, p, Type);
        if (r) {
          /* Cache miss. We need to read from hardware. Since we try to use burst mode, we do not read immediately */
          if (NeedReadBurst) {
            NumSectors2Read++;
          } else {
            FirstSector     = SectorNo;
            pReadBuffer     = p;
            NumSectors2Read = 1;
            NeedReadBurst   = 1;
          }
        } else {
          INC_READ_CACHE_HIT_CNT();             // For statistics / debugging only
          if (NeedReadBurst) {
            NeedReadBurst   = 0;
            x = _Read(pDevice, FirstSector, pReadBuffer, NumSectors2Read, Type);
            if (x) {
              break;                     /* End read operation because of read failure */
            }
          }
        }
        p  += SectorSize;
        SectorNo++;
      } while(--NumSectors);
      /*
       * End of read routine reached. There may be a hardware "read burst" operation pending, which needs to be executed in this case.
       */
      if (NeedReadBurst) {
        x = _Read(pDevice, FirstSector, pReadBuffer, NumSectors2Read, Type);
      }
    } else {
      x = _Read(pDevice, SectorNo, pBuffer, NumSectors, Type);
    }
  }
#else
  FS_USE_PARA(Type);
  x = _Read(pDevice, SectorNo, pBuffer, NumSectors, Type);
#endif
  CLR_BUSY_LED(pDevice);
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "\n"));
  return  x;
}

/*********************************************************************
*
*       FS_LB_ReadBurstPart
*
*  Function description
*    Read multiple sectors from device.
*
*  Parameters
*    pPart       - Pointer to a partition structure.
*    SectorNo    - First sector to be read from the device.
*    NumSectors  - Number of sectors to be read from the device.
*    pBuffer     - Pointer to buffer for storing the data.
*    Type        - The type of sector that shall be read.
*
*  Return value
*    ==0         - Sectors have been read and copied to pBuffer.
*    <0          - An error has occurred.
*/
int FS_LB_ReadBurstPart(FS_PARTITION *pPart, U32 SectorNo, U32 NumSectors, void *pBuffer, U8 Type) {
  FS_DEVICE      * pDevice;

  pDevice     = &pPart->Device;
  SectorNo   += pPart->StartSector;
  return  FS_LB_ReadBurst(pDevice, SectorNo, NumSectors, pBuffer, Type);
}

/*********************************************************************
*
*       FS_LB_WriteBurst
*
*  Function description
*    Write multiple sectors to device.
*
*  Parameters
*    pDriver     - Pointer to a device driver structure.
*    SectorNo    - First sector to be written to the device.
*    NumSectors  - Number of sectors to be written.
*    pBuffer     - Pointer to buffer for holding the data.
*    Type        - The type of sector that shall be written.
*
*  Return value
*    ==0         - Sectors have been read and copied to pBuffer.
*    <0          - An error has occurred.
*/
int FS_LB_WriteBurst(FS_DEVICE * pDevice, U32 SectorNo, U32 NumSectors, const void *pBuffer, U8 Type) {
  int         x;

  FS_USE_PARA(Type);
  FS_LB_InitMediumIfRequired(pDevice);
  INC_WRITE_SECTOR_CNT(NumSectors);             // For statistics / debugging only
  CALL_ONDEVICE_HOOK(pDevice, FS_OPERATION_WRITE, SectorNo, NumSectors, Type);
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "WriteBurst    %s:%d: %s Sector: 0x%8x, NumSectors: %d", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, _Type2Name(Type), SectorNo, NumSectors));
  SET_BUSY_LED(pDevice);
  x = _Write(pDevice, SectorNo, pBuffer, NumSectors, 0);
  CLR_BUSY_LED(pDevice);
#if FS_SUPPORT_CACHE
  if (x == 0) {
    const FS_DEVICE_DATA * pDevData;
    const FS_CACHE_API   * pCacheAPI;
    U16                 SectorSize;
    pDevData   = &pDevice->Data;
    pCacheAPI  = pDevData->pCacheAPI;
    SectorSize = FS_GetSectorSize(pDevice);
    if (pCacheAPI) {
      do {
        const U8 * p;
        /* ToDo: Return value of write cache is ignored now. Optimization possible */
        if ((pDevData->pCacheAPI->pfUpdateCache)(pDevice, SectorNo++, pBuffer, Type)) {
          FS_DEBUG_WARN((FS_MTYPE_STORAGE, "Could not update sector in cache"));
        }
        p = (const U8 *)pBuffer;
        pBuffer = p + SectorSize;
      } while(--NumSectors);
    }
  }
#endif
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "\n"));
  return  x;
}


/*********************************************************************
*
*       FS_LB_WriteBurstPart
*
*  Function description
*    Write multiple sectors to device.
*
*  Parameters
*    pPart       - Pointer to the partition structure.
*    SectorNo    - First sector to be written to the device.
*    NumSectors  - Number of sectors to be written.
*    pBuffer     - Pointer to buffer for holding the data.
*    Type        - The type of sector that shall be written.
*
*  Return value
*    ==0         - Sectors have been read and copied to pBuffer.
*    <0          - An error has occurred.
*/
int FS_LB_WriteBurstPart(FS_PARTITION *pPart, U32 SectorNo, U32 NumSectors, const void *pBuffer, U8 Type) {
  FS_DEVICE * pDevice;

  FS_USE_PARA(Type);
  pDevice     = &pPart->Device;
  SectorNo   += pPart->StartSector;
  return  FS_LB_WriteBurst(pDevice, SectorNo, NumSectors, pBuffer, Type);
}

/*********************************************************************
*
*       FS_LB_WriteMultiple
*
*  Function description
*    Write multiple sectors to device.
*
*  Parameters
*    pDriver     - Pointer to a device driver structure.
*    Unit        - Unit number.
*    SectorNo    - First sector to be written to the device.
*    NumSectors  - Number of sectors to be written.
*    pBuffer     - Pointer to buffer for holding the data.
*    Type        - The type of sector that shall be written.
*
*  Return value
*    ==0         - Sectors have been read and copied to pBuffer.
*    <0          - An error has occurred.
*/
int FS_LB_WriteMultiple(FS_DEVICE * pDevice, U32 SectorNo, U32 NumSectors, const void *pBuffer, U8 Type) {
  int         x;

  FS_USE_PARA(Type);
  FS_LB_InitMediumIfRequired(pDevice);
  INC_WRITE_SECTOR_CNT(NumSectors);             // For statistics / debugging only
  CALL_ONDEVICE_HOOK(pDevice, FS_OPERATION_WRITE, SectorNo, NumSectors, Type);
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "WriteMultiple %s:%d: %s Sector: 0x%8x, NumSectors: %d", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, _Type2Name(Type), SectorNo, NumSectors));
  SET_BUSY_LED(pDevice);
  x = _Write(pDevice, SectorNo, pBuffer, NumSectors, 1);
  CLR_BUSY_LED(pDevice);
#if FS_SUPPORT_CACHE
  if (x == 0) {
    const FS_DEVICE_DATA * pDevData;
    const FS_CACHE_API   * pCacheAPI;
    pDevData   = &pDevice->Data;
    pCacheAPI  = pDevData->pCacheAPI;
    if (pCacheAPI) {
      do {
        /* ToDo: Return value of write cache is ignored now. Optimization possible */
        if (pDevData->pCacheAPI->pfUpdateCache(pDevice, SectorNo++, pBuffer, Type)) {
          FS_DEBUG_WARN((FS_MTYPE_STORAGE, "Could not update sector in cache"));
        }
      } while(--NumSectors);
    }
  }
#endif
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "\n"));
  return  x;
}

/*********************************************************************
*
*       FS_LB_WriteMultiplePart
*
*  Function description
*    Write multiple sectors to device.
*
*  Parameters
*    pPart       - Pointer to the partition structure.
*    Unit        - Unit number.
*    SectorNo    - First sector to be written to the device.
*    NumSectors  - Number of sectors to be written.
*    pBuffer     - Pointer to buffer for holding the data.
*    Type        - The type of sector that shall be written.
*
*  Return value
*    ==0         - Sectors have been read and copied to pBuffer.
*    <0          - An error has occurred.
*/
int FS_LB_WriteMultiplePart(FS_PARTITION *pPart, U32 SectorNo, U32 NumSectors, const void *pBuffer, U8 Type) {
  FS_DEVICE * pDevice;

  FS_USE_PARA(Type);
  pDevice     = &pPart->Device;
  SectorNo   += pPart->StartSector;
  return  FS_LB_WriteMultiple(pDevice, SectorNo, NumSectors, pBuffer, Type);
}

/*********************************************************************
*
*       FS_LB_WriteDevice
*
*  Function description
*    FS internal function. Write sector to device.
*
*  Parameters
*    pDriver     - Pointer to a device driver structure.
*    Unit        - Unit number.
*    Sector      - Sector to be written to the device.
*    pBuffer     - Pointer to data to be stored.
*    Type        - The type of sector that shall be written.
*
*  Return value
*    == 0        - Sector has been written to the device.
*    <  0        - An error has occurred.
*/
#if FS_VERIFY_WRITE
static U8 * _pVerifyBuffer;
#endif

int FS_LB_WriteDevice(FS_DEVICE *pDevice, U32 Sector, const void *pBuffer, U8 Type) {
  char IsWritten = 0;

  FS_USE_PARA(Type);
  FS_LB_InitMediumIfRequired(pDevice);
  INC_WRITE_SECTOR_CNT(1);             // For statistics / debugging only
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "Write         %s:%d: %s Sector: 0x%8x", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, _Type2Name(Type), Sector));
  CALL_ONDEVICE_HOOK(pDevice, FS_OPERATION_WRITE, Sector, 1, Type);
#if FS_SUPPORT_CACHE
  {
    const FS_DEVICE_DATA * pDevData;
    const FS_CACHE_API   * pCacheAPI;
    pDevData   = &pDevice->Data;
    pCacheAPI  = pDevData->pCacheAPI;
    if (pCacheAPI) {
      IsWritten = (pCacheAPI->pfWriteIntoCache)(pDevice, Sector, pBuffer, Type);
      if (IsWritten) {
        FS_DEBUG_LOG((FS_MTYPE_STORAGE, " (cache)"));
      }
    }
  }
#endif
  if (IsWritten == 0) {
    SET_BUSY_LED(pDevice);
    if (_Write(pDevice, Sector, pBuffer, 1, 0) == 0) {
      IsWritten = 1;
    } else {
      FS_DEBUG_ERROROUT((FS_MTYPE_STORAGE, "Failed to write Sector: %d on %s:%d.\n", Sector, pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit));
    }
#if FS_VERIFY_WRITE
    /* In higher debug levels, read medium once more to verify write operation */
    {
      U16 SectorSize;

      SectorSize = FS_GetSectorSize(pDevice);
      FS_AllocZeroedPtr((void **)&_pVerifyBuffer, SectorSize);
      _Read(pDevice, Sector, _pVerifyBuffer, 1, Type);
      if (FS_MEMCMP(pBuffer, (const void *)_pVerifyBuffer, SectorSize)) {
        FS_DEBUG_ERROROUT((FS_MTYPE_STORAGE, "Verify failed after write. Sector %d on %s:%d:.\n", Sector, pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit));
      }
    }
#endif
    CLR_BUSY_LED(pDevice);
  }
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "\n"));
  return IsWritten ? 0 : -1;
}

/*********************************************************************
*
*         FS_LB_WritePart
*
*  Function description
*    Read sector from volume.
*
*  Parameters
*    pDriver     - Pointer to a device driver structure.
*    Sector      - Physical sector to be written to the partition.
*                  The partition start sector is added.
*    pBuffer     - Pointer to buffer for storing the data.
*    Type        - The type of sector that shall be written.
*
*  Return value
*    ==0         - Sector has been read and copied to pBuffer.
*    <0          - An error has occurred.
*/
int FS_LB_WritePart(FS_PARTITION *pPart, U32 Sector, const void *pBuffer, U8 Type) {
  return FS_LB_WriteDevice(&pPart->Device, pPart->StartSector + Sector, pBuffer, Type);
}


/*********************************************************************
*
*       FS_LB_Ioctl
*
*  Function description
*    Executes device command.
*
*  Parameters
*    pDriver     - Pointer to a device driver structure.
*    Cmd         - Command to be executed.
*    Aux         - Parameter depending on command.
*    pBuffer     - Pointer to a buffer used for the command.
*
*  Return value
*    Command specific. In general a negative value means an error.
*/
int FS_LB_Ioctl(FS_DEVICE * pDevice, I32 Cmd, I32 Aux, void *pBuffer) {
  int x;
  const FS_DEVICE_TYPE * pDeviceType;

  pDeviceType = pDevice->pType;
  switch (Cmd) {
  case FS_CMD_UNMOUNT:
  case FS_CMD_UNMOUNT_FORCED:
    break;
  default:
    FS_LB_InitMediumIfRequired(pDevice);
    break;
  }
  x = (pDeviceType->pfIoCtl)(pDevice->Data.Unit, Cmd, Aux, pBuffer);
  return x;
}

/*********************************************************************
*
*       FS_GetSectorSize
*
*  Function description
*    Returns the sector size of a device.
*
*/
U16 FS_GetSectorSize(FS_DEVICE * pDevice) {
  U16 r = 0;
  FS_DEV_INFO DevInfo;

  FS_LB_InitMediumIfRequired(pDevice);
  if (pDevice->pType->pfIoCtl(pDevice->Data.Unit, FS_CMD_GET_DEVINFO, 0, &DevInfo) == 0) {
    r = DevInfo.BytesPerSector;
  }
  return r;
}

/*********************************************************************
*
*       FS_LB_GetDeviceInfo
*
*/
int FS_LB_GetDeviceInfo(FS_DEVICE * pDevice, FS_DEV_INFO * pDevInfo) {
  int r;

  r = 0;
  FS_LB_InitMediumIfRequired(pDevice);
  if (pDevice->pType->pfIoCtl(pDevice->Data.Unit, FS_CMD_GET_DEVINFO, 0, (void *)pDevInfo)) {
    r = -1;
  }
  return r;
}

/*********************************************************************
*
*       FS_LB_FreeSectors
*
*  Function description
*    Frees unused sectors (from cache and devices) of a partition
*
*/
void FS_LB_FreePartSectors(FS_PARTITION* pPart, U32 SectorIndex, U32 NumSectors) {
  FS_DEVICE * pDevice;

  pDevice = &pPart->Device;
  SectorIndex += pPart->StartSector;      /* Convert into device sector index */
  FS_USE_PARA(pDevice);
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "FreeSectors   %s:%d:      Sector: 0x%8x, NumSectors:  0x%8x", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, SectorIndex, NumSectors));
#if FS_SUPPORT_JOURNAL
  if (pDevice->Data.JournalIsActive) {
//    r = FS_JOURNAL_FreeSector(pDevice, SectorNo, pBuffer, NumSectors, RepeatSame);
// TBD: This can be optimized if the journal can also manage "freed" sectors.
  } else
#endif
  {
    FS_LB_Ioctl(pDevice, FS_CMD_FREE_SECTORS, SectorIndex, &NumSectors);
  }

  #if FS_SUPPORT_CACHE
  {
    CACHE_FREE CacheFree;

    CacheFree.FirstSector = SectorIndex;
    CacheFree.NumSectors  = NumSectors;
    FS__CACHE_CommandDeviceNL(pDevice, FS_CMD_CACHE_FREE_SECTORS, &CacheFree);
  }
  #endif
  FS_DEBUG_LOG((FS_MTYPE_STORAGE, "\n"));
}

/*************************** End of file ****************************/
