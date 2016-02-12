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
File        : FS_CacheRWQuota.c
Purpose     : Logical Block Layer, Cache module
              Cache Strategy:
                Read / write cache, caching all sectors equally.
              Limitations:
                None.
                This cache module can be used on any device with any
                file system.
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

#if FS_SUPPORT_CACHE

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define SECTOR_INDEX_INVALID   0xFFFFFFFFUL

#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_ALL

#define TYPE2NAME(Type)        _aDesc[Type].s

typedef struct {
  int Type;
  const char *s;
} TYPE_DESC;

static const TYPE_DESC _aDesc[] = {
  { FS_SECTOR_TYPE_DATA, "DATA" },
  { FS_SECTOR_TYPE_DIR,  "DIR " },
  { FS_SECTOR_TYPE_MAN,  "MAN " },
};
#endif


/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/* Block info. One instance per block.
   Every cache block can cache a single sector.
   It starts with CACHE_BLOCK_INFO_RW_QUOTA, followed by the cached data. */
typedef struct {
  U32     SectorNo;
  unsigned   IsDirty;
} CACHE_BLOCK_INFO_RW_QUOTA;

/* Cache data. Once instance per cache.
   Size needs to be a multiple of 4 */
typedef struct {
  U32        NumSectors;
  U32        SectorSize;
  U8         aCacheMode[FS_SECTOR_TYPE_COUNT];
  U32        aNumSectorsReq[FS_SECTOR_TYPE_COUNT];
  U32        aNumSectorsAct[FS_SECTOR_TYPE_COUNT];
  U32        aHashStart[FS_SECTOR_TYPE_COUNT];
  U32        NumBytesCache;
} CACHE_DATA_RW_QUOTA;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _GetHashCode
*
*  Description:
*    Calculates hashcode, based on sector number and type
*
*  Return value
*    0xFFFFFFFF    no hashcodes (invalid, because this type is not cached)
*    else          hashcode
*/
static U32 _GetHashCode(CACHE_DATA_RW_QUOTA * pCacheData, U32 SectorNo, U8 SectorType) {
  U32 NumSectors;
  NumSectors = pCacheData->aNumSectorsAct[SectorType];
  if (NumSectors == 0) {
    return 0xFFFFFFFFUL;
  }
  return (SectorNo % NumSectors) + pCacheData->aHashStart[SectorType];
}


/*********************************************************************
*
*       _InvalidateCache
*
*  Description:
*    Invalidates all data in cache
*
*  Notes
*    (1)   The cache needs to be clean; it may not contain dirty sectors
*/
static void _InvalidateCache(CACHE_DATA_RW_QUOTA * pCacheData) {
  U32             i;
  U32             NumSectors;
  CACHE_BLOCK_INFO_RW_QUOTA * pBlockInfo;
  U32             SectorSize;

  NumSectors = pCacheData->NumSectors;
  SectorSize = pCacheData->SectorSize;
  pBlockInfo = (CACHE_BLOCK_INFO_RW_QUOTA *)(pCacheData + 1);
  /* Init Cache entries */
  for (i = 0; i < NumSectors; i++) {
    pBlockInfo->SectorNo = SECTOR_INDEX_INVALID;
    pBlockInfo->IsDirty  = 0;
    pBlockInfo = (CACHE_BLOCK_INFO_RW_QUOTA*)(((U8*)(pBlockInfo + 1)) + SectorSize);
  }
}


/*********************************************************************
*
*       _ComputeNumSectors
*
*/
static U32 _ComputeNumSectors(FS_DEVICE * pDevice, CACHE_DATA_RW_QUOTA * pCacheData) {
  U32 NumSectors;
  U16 SectorSize;
  U32 NumBytes;

  if (pCacheData->SectorSize) {
    return pCacheData->NumSectors;
  }
  NumBytes   = pCacheData->NumBytesCache;
  SectorSize = FS_GetSectorSize(pDevice);
  if ((SectorSize == 0) || (NumBytes < sizeof(CACHE_DATA_RW_QUOTA))) {
    return 0;
  }
  /* Compute number of sectors in cache */
  NumSectors = (NumBytes - sizeof(CACHE_DATA_RW_QUOTA)) / (sizeof(CACHE_BLOCK_INFO_RW_QUOTA) + SectorSize);
  if (NumSectors > 0) {
    pCacheData->NumSectors    = NumSectors;
    pCacheData->SectorSize    = SectorSize;
    _InvalidateCache(pCacheData);
  }
  return NumSectors;
}


/*********************************************************************
*
*       _CacheRWQuota_ReadFromCache
*
*  Description:
*    Read sector from cache if possible
*/
static void _UpdateHashAreas(CACHE_DATA_RW_QUOTA * pCacheData) {
  U32 SectorPos;
  U32 SectorPosNext;
  U32 ReqSize;
  int    i;

  SectorPos = 0;
  for (i = 0; i < FS_SECTOR_TYPE_COUNT; i++) {
    pCacheData->aHashStart[i] = SectorPos;
    ReqSize = pCacheData->aNumSectorsReq[i];
    if (i == FS_SECTOR_TYPE_COUNT - 1) {
      ReqSize = 0xFFFF;                      /* For last type, use the rest of the avail. memory */
    }
    SectorPosNext = SectorPos + ReqSize;
    if (SectorPosNext > pCacheData->NumSectors) {
      SectorPosNext = pCacheData->NumSectors;
    }
    pCacheData->aNumSectorsAct[i] = SectorPosNext - SectorPos;
    SectorPos = SectorPosNext;
  }
}



/*********************************************************************
*
*       Static code (callbacks)
*
**********************************************************************
*/

/*********************************************************************
*
*       _CacheRWQuota_ReadFromCache
*
*  Description:
*    Read sector from cache if possible
*
*  Return value
*    1    Sector not found
*    0    Sector found
*/
static char _CacheRWQuota_ReadFromCache(FS_DEVICE * pDevice, U32 SectorNo,       void * pData, U8 SectorType) {
  U32             Off;
  CACHE_DATA_RW_QUOTA       * pCacheData;
  CACHE_BLOCK_INFO_RW_QUOTA * pBlockInfo;
  U32             SectorSize;
  U32             HashCode;

  FS_USE_PARA(SectorType);
  pCacheData  = (CACHE_DATA_RW_QUOTA *)pDevice->Data.pCacheData;
  if (_ComputeNumSectors(pDevice, pCacheData) == 0) {
    return 1;                          /* Device is not available */
  }
  SectorSize  = pCacheData->SectorSize;
  HashCode    = _GetHashCode(pCacheData, SectorNo, SectorType);
  if (HashCode == 0xFFFFFFFFUL) {
    return 1;                         /* Sector not found */
  }
  Off         = HashCode * (sizeof(CACHE_BLOCK_INFO_RW_QUOTA) + SectorSize);
  pBlockInfo  = (CACHE_BLOCK_INFO_RW_QUOTA *) (((U8 *)(pCacheData + 1)) + Off);
  if (pBlockInfo->SectorNo == SectorNo) {
    FS_MEMCPY(pData, pBlockInfo + 1, SectorSize);
    return 0;                         /* Sector found */
  }
  return 1;                         /* Sector not found */
}

/*********************************************************************
*
*       _WriteIntoCache
*
*  Description:
*    Writes a sector in cache.
*/
static void _WriteIntoCache(CACHE_BLOCK_INFO_RW_QUOTA  * pBlockInfo, U32 SectorNo, const void * pData, U32 SectorSize) {
  pBlockInfo->SectorNo = SectorNo;
  FS_MEMCPY(pBlockInfo + 1, pData, SectorSize);
}

/*********************************************************************
*
*       _RemoveFromCache
*
*  Description:
*    Make sure that this sector is not in cache.
*    This functions does not write dirty data; even dirty entries can be removed
*
*  Notes
*    (1) What for ?
*        This can be useful (and import as well to maintain cache coherency).
*        The function is called whenever clusters (data or directory) are freed.
*/
static void _RemoveFromCache(FS_DEVICE * pDevice, U32 FirstSector, U32 NumSectors) {
  U32 HashCode;
  CACHE_BLOCK_INFO_RW_QUOTA * pBlockInfo;
  CACHE_DATA_RW_QUOTA       * pCacheData;
  U32             Off;
  U32             SectorNo;
  U32             NumSectorsInCache;
  U32             HashOff;
  U32             HashBase;
  U32             iType;
  U32             iSector;

  pCacheData    = (CACHE_DATA_RW_QUOTA *)pDevice->Data.pCacheData;
  //
  // Iterate over types
  //
  for (iType = 0; iType < 2; iType++) {     // Only data and dir caches
    NumSectorsInCache = pCacheData->aNumSectorsAct[iType];
    if (NumSectorsInCache) {
      HashOff  = pCacheData->aHashStart[iType];
      HashBase = FirstSector % NumSectorsInCache;
      for (iSector = 0; iSector < NumSectors; iSector++) {
        SectorNo = FirstSector + iSector;
        HashCode = HashBase + HashOff;
        if (++HashBase == NumSectorsInCache) {
          HashBase = 0;
        }
        Off = HashCode * (sizeof(CACHE_BLOCK_INFO_RW_QUOTA) + pCacheData->SectorSize);
        pBlockInfo = (CACHE_BLOCK_INFO_RW_QUOTA *)((U8 *)(pCacheData + 1) + Off);
        if (pBlockInfo->SectorNo == SectorNo) {
          FS_DEBUG_LOG((FS_MTYPE_CACHE, "Removing sector 0x%8x from data cache. Device: %s:%d", pBlockInfo->SectorNo, pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit));
          pBlockInfo->SectorNo = SECTOR_INDEX_INVALID;
          pBlockInfo->IsDirty = 0;
        }
      }
    }
  }
}

/*********************************************************************
*
*       _CacheRWQuota_UpdateCache
*
*  Description:
*    Updates a sector in cache.
*    Called after a READ operation to update the cache.
*    This means that the sector can not be in the cache.
*
*  Return value
*    0    Not in write cache, the physical write operation still needs to be performed (Since this cache is a pure read-cache).
*/
static char _CacheRWQuota_UpdateCache(FS_DEVICE * pDevice, U32 SectorNo, const void * pData, U8 SectorType) {
  U32             Off;
  CACHE_DATA_RW_QUOTA       * pCacheData;
  CACHE_BLOCK_INFO_RW_QUOTA * pBlockInfo;
  U32             SectorSize;
  int                CacheMode;

  pCacheData  = (CACHE_DATA_RW_QUOTA *)pDevice->Data.pCacheData;
  if (_ComputeNumSectors(pDevice, pCacheData) == 0) {
    return 0;                          /* Device is not available */
  }
  CacheMode   = pCacheData->aCacheMode[SectorType];
  if (CacheMode & FS_CACHE_MODE_R) {           /* Read cache is on for this type of sector */
    U32             HashCode;

    SectorSize  = pCacheData->SectorSize;
    HashCode    = _GetHashCode(pCacheData, SectorNo, SectorType);
    if (HashCode == 0xFFFFFFFFUL) {
      return 0;                         /* Sector not found */
    }
    Off         = HashCode * (sizeof(CACHE_BLOCK_INFO_RW_QUOTA) + SectorSize);
    pBlockInfo  = (CACHE_BLOCK_INFO_RW_QUOTA *) (((U8 *)(pCacheData + 1)) + Off);
    /* If we replace an other, dirty sector, we need to write it out */
    if ((pBlockInfo->SectorNo != SectorNo) && (pBlockInfo->IsDirty)) {
      if ((pDevice->pType->pfWrite)(pDevice->Data.Unit, pBlockInfo->SectorNo, pBlockInfo + 1, 1, 0)) {
        FS_DEBUG_ERROROUT((FS_MTYPE_CACHE, "Failure when cleaning cache"));   /* FATAL error ! */
      }
      FS_DEBUG_LOG((FS_MTYPE_CACHE, "Cleaning %s:%d: Sector: 0x%8x (%s)", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, pBlockInfo->SectorNo, TYPE2NAME(SectorType)));
    }
    _WriteIntoCache(pBlockInfo, SectorNo, pData, SectorSize);
    pBlockInfo->IsDirty = 0;
  }
  return 0;
}


/*********************************************************************
*
*       _CacheRWQuota_WriteCache
*
*  Description:
*    Writes a sector into cache.
*
*  Return value
*    0    Not  in write cache, the physical write operation still needs to be performed.
*    1    Data in write cache, the physical write operation does not need to be performed.
*/
static char _CacheRWQuota_WriteCache(FS_DEVICE * pDevice, U32 SectorNo, const void * pData, U8 SectorType) {
  U32             Off;
  CACHE_DATA_RW_QUOTA       * pCacheData;
  CACHE_BLOCK_INFO_RW_QUOTA * pBlockInfo;
  U32             SectorSize;
  U32             HashCode;
  int                CacheMode;
  char               WriteRequired;

  pCacheData    = (CACHE_DATA_RW_QUOTA *)pDevice->Data.pCacheData;
  if (_ComputeNumSectors(pDevice, pCacheData) == 0) {
    return 0;                          /* Device is not available */
  }
  SectorSize    = pCacheData->SectorSize;
  CacheMode     = pCacheData->aCacheMode[SectorType];
  HashCode    = _GetHashCode(pCacheData, SectorNo, SectorType);
  if (HashCode == 0xFFFFFFFFUL) {
    return 0;                         /* Sector not found */
  }
  Off         = HashCode * (sizeof(CACHE_BLOCK_INFO_RW_QUOTA) + SectorSize);
  pBlockInfo    = (CACHE_BLOCK_INFO_RW_QUOTA *) (((U8 *)(pCacheData + 1)) + Off);
  WriteRequired = 0;
  if (CacheMode & FS_CACHE_MODE_W) {              /* Write cache on for this type of sector ? */
    WriteRequired = 1;
  } else if (pBlockInfo->SectorNo == SectorNo) {  /* Sector already in cache ? */
    WriteRequired = 1;                            /* Update required ! */
  }
  if (WriteRequired) {
    /* If we replace an other, dirty sector, we need to write it out */
    if ((pBlockInfo->IsDirty) && (pBlockInfo->SectorNo != SectorNo)) {
      if ((pDevice->pType->pfWrite)(pDevice->Data.Unit, pBlockInfo->SectorNo, pBlockInfo + 1, 1, 0)) {
        FS_DEBUG_ERROROUT((FS_MTYPE_CACHE, "Failure when cleaning cache"));   /* FATAL error ! */
      }
      FS_DEBUG_LOG((FS_MTYPE_CACHE, "Cleaning %s:%d: Sector: 0x%8x (%s)", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, pBlockInfo->SectorNo, TYPE2NAME(SectorType)));
    }
    pBlockInfo->IsDirty = 0;
    _WriteIntoCache(pBlockInfo, SectorNo, pData, SectorSize);
  }
  if (CacheMode & FS_CACHE_MODE_D) {              /* Delayed write allowed cache on for this type of sector ? */
    pBlockInfo->IsDirty = 1;
    return 1;                                     /* Write is delayed (data in cache) and does not need to be performed */
  } else {
    return 0;                                     /* Write still needs to be performed. */
  }
}

/*********************************************************************
*
*       _CacheRWQuota_InvalidateCache
*
*  Description:
*    Invalidates all data in cache
*
*  Notes
*    (1)   The cache needs to be clean; it may not contain dirty sectors
*/
static void _CacheRWQuota_InvalidateCache(void * p) {
  CACHE_DATA_RW_QUOTA *       pCacheData;

  pCacheData = (CACHE_DATA_RW_QUOTA *)p;
  _InvalidateCache(pCacheData);
  pCacheData->NumSectors = 0;
  pCacheData->SectorSize = 0;
}


/*********************************************************************
*
*       _Clean
*
*  Description:
*    Writes out all dirty sectors from cache.
*
*  Return value:
*    0
*/
static int _Clean(FS_DEVICE * pDevice) {
  U32             i;
  U32             NumSectors;
  CACHE_DATA_RW_QUOTA *       pCacheData;
  CACHE_BLOCK_INFO_RW_QUOTA * pBlockInfo;
  U32             SectorSize;
  U32             SizeOfCacheBlock;

  pCacheData = (CACHE_DATA_RW_QUOTA *)pDevice->Data.pCacheData;
  NumSectors = pCacheData->NumSectors;
  SectorSize = pCacheData->SectorSize;
  SizeOfCacheBlock = sizeof(CACHE_BLOCK_INFO_RW_QUOTA) + SectorSize;
  for (i = 0; i < NumSectors; i++) {
    pBlockInfo = (CACHE_BLOCK_INFO_RW_QUOTA *) (((U8 *)(pCacheData + 1)) + (i * SizeOfCacheBlock));

    if (pBlockInfo->IsDirty) {
      FS_DEBUG_LOG((FS_MTYPE_CACHE, "Cleaning %s:%d: Sector: 0x%8x", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, pBlockInfo->SectorNo));
      if ((pDevice->pType->pfWrite)(pDevice->Data.Unit, pBlockInfo->SectorNo, pBlockInfo + 1, 1, 0)) {
        FS_DEBUG_ERROROUT((FS_MTYPE_CACHE, "Failure when cleaning cache"));   /* FATAL error ! */
      }
      pBlockInfo->IsDirty = 0;
    }
  }
  return 0;
}



/*********************************************************************
*
*       _SetMode
*
*  Description:
*    Sets the mode for the give type of sectors.
*/
static void _SetMode(FS_DEVICE * pDevice, CACHE_MODE * pCacheMode) {
  int i;
  CACHE_DATA_RW_QUOTA * pCacheData;

  pCacheData = (CACHE_DATA_RW_QUOTA *)pDevice->Data.pCacheData;
  _Clean(pDevice);
  _InvalidateCache(pCacheData);
  for (i = 0; i < FS_SECTOR_TYPE_COUNT; i++) {
    int TypeMask;
    TypeMask = 1 << i;
    if (TypeMask & pCacheMode->TypeMask) {
      pCacheData->aCacheMode[i] = pCacheMode->ModeMask;
    }
  }
}


/*********************************************************************
*
*       _SetQuota
*
*  Description:
*    Sets the quota for the give type of sectors.
*/
static void _SetQuota(FS_DEVICE * pDevice, CACHE_QUOTA *pQuota) {
  int i;
  CACHE_DATA_RW_QUOTA * pCacheData;

  pCacheData = (CACHE_DATA_RW_QUOTA *)pDevice->Data.pCacheData;
  _Clean(pDevice);
  _InvalidateCache(pCacheData);
  for (i = 0; i < FS_SECTOR_TYPE_COUNT; i++) {
    int TypeMask;
    TypeMask = 1 << i;
    if (TypeMask & pQuota->TypeMask) {
      pCacheData->aNumSectorsReq[i] = pQuota->NumSectors;
    }
  }
  _UpdateHashAreas(pCacheData);
}

/*********************************************************************
*
*       _CacheRWQuota_Command
*
*  Description:
*    Execute commands on the cache
*
*  Return value:
*    Unsupported command:    -1
*    Supported commands:     <0: Error. Precise value depends on command
*/
static int _CacheRWQuota_Command(FS_DEVICE * pDevice, int Cmd, void * p) {
  int r;

  r  = -1;
  switch (Cmd) {
  case FS_CMD_CACHE_FREE_SECTORS:
    {
      CACHE_FREE * pCacheFree;
      pCacheFree = (CACHE_FREE *)p;
      _RemoveFromCache(pDevice, pCacheFree->FirstSector, pCacheFree->NumSectors);
      r = 0;
    }
    break;
  case FS_CMD_CACHE_CLEAN:
    r = _Clean(pDevice);
    break;
  case FS_CMD_CACHE_SET_MODE:
    _SetMode(pDevice, (CACHE_MODE *)p);
    r = 0;
    break;
  case FS_CMD_CACHE_SET_QUOTA:
    _SetQuota(pDevice, (CACHE_QUOTA *)p);
    r = 0;
    break;
  case FS_CMD_CACHE_INVALIDATE:
    _CacheRWQuota_InvalidateCache(p);
    r = 0;
    break;
  }
  return r;
}

/*********************************************************************
*
*       _CacheRW_QUOTA_API
*
*/
static const FS_CACHE_API _CacheRW_QUOTA_API = {
  _CacheRWQuota_ReadFromCache,
  _CacheRWQuota_UpdateCache,
  _CacheRWQuota_InvalidateCache,
  _CacheRWQuota_Command,
  _CacheRWQuota_WriteCache
};

/*********************************************************************
*
*       FS_CacheRWQuota_Init
*
*  Description:
*    Initializes the cache
*
*  Return value
*     Returns the number of cache blocks (Number of sectors that can be cached)
*/
U32 FS_CacheRWQuota_Init(FS_DEVICE * pDevice, void * pData, I32 NumBytes) {
  FS_DEVICE_DATA      * pDevData;
  CACHE_DATA_RW_QUOTA * pCacheData;
  U8                  * pData8;
  U32                   NumSectors;

  NumSectors = 0;
  pDevData   = &pDevice->Data;
  pData8     = (U8 *)pData;
  //
  // Align pointer to a 32bit boundary
  //
  if ((U32)pData8 & 3) {
    NumBytes  -= (4 - (((U32)pData8) & 3));
    pData8    += (4 - (((U32)pData8) & 3));
  }
  //
  // If less memory is available as we need to hold the
  // management structure, we leave everything as it is.
  // A cache module is then not attached to the device.
  //
  if ((U32)NumBytes < sizeof(CACHE_DATA_RW_QUOTA)) {
    return 0;
  }
  pCacheData = (CACHE_DATA_RW_QUOTA *)pData8;
  FS_MEMSET(pCacheData, 0, sizeof(CACHE_DATA_RW_QUOTA));
  pDevData->pCacheAPI       = &_CacheRW_QUOTA_API;
  pDevData->pCacheData      = pCacheData;
  pCacheData->NumBytesCache = NumBytes;
  NumSectors = _ComputeNumSectors(pDevice, pCacheData);
  return NumSectors;
}
#else

void CacheRWQuota_c(void);
void CacheRWQuota_c(void){}

#endif /* FS_SUPPORT_CACHE */

/*************************** End of file ****************************/
