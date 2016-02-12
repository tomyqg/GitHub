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
File        : FS_CacheRW.c
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

#include "FS_ConfDefaults.h"
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

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/* Block info. One instance per block.
   Every cache block can cache a single sector.
   It starts with CACHE_BLOCK_INFO_RW, followed by the cached data. */
typedef struct {
  U32     SectorNo;
  unsigned   IsDirty;
} CACHE_BLOCK_INFO_RW;

/* Cache data. Once instance per cache.
   Size needs to be a multiple of 4 */
typedef struct {
  U32        NumSectors;
  U32        SectorSize;
  U8         aCacheMode[FS_SECTOR_TYPE_COUNT];
#if FS_SECTOR_TYPE_COUNT % 4
  U8         aPadding[4 - (FS_SECTOR_TYPE_COUNT % 4)];     /* Make sure we pad this to a multiple of 4 bytes */
#endif
  U32        NumBytesCache;
} CACHE_DATA_RW;

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
*    Calculates hash code, based on sector number and Number of sectors in cache
*/
static U32 _GetHashCode(U32 SectorNo, U32 NumSectorIndices) {
  return SectorNo % NumSectorIndices;
}


/*********************************************************************
*
*       _InvalidateCache 
*
*  Description:
*    Invalidates all data in cache
*/
static void _InvalidateCache(CACHE_DATA_RW * pCacheData) {
  U32             i;
  U32             NumSectors;
  CACHE_BLOCK_INFO_RW * pBlockInfo;
  U32             SectorSize;

  NumSectors = pCacheData->NumSectors;
  SectorSize = pCacheData->SectorSize;
  pBlockInfo = (CACHE_BLOCK_INFO_RW *)(pCacheData + 1);
  /* Init Cache entries */
  for (i = 0; i < NumSectors; i++) {
    pBlockInfo->SectorNo = SECTOR_INDEX_INVALID;
    pBlockInfo->IsDirty  = 0;
    pBlockInfo = (CACHE_BLOCK_INFO_RW*)(((U8*)(pBlockInfo + 1)) + SectorSize);
  }
}

/*********************************************************************
*
*       _ComputeNumSectors
*
*/
static U32 _ComputeNumSectors(FS_DEVICE * pDevice, CACHE_DATA_RW * pCacheData) {
  U32 NumSectors;
  U16 SectorSize;
  U32 NumBytes;

  if (pCacheData->SectorSize) {
    return pCacheData->NumSectors;
  }
  NumBytes   = pCacheData->NumBytesCache;
  SectorSize = FS_GetSectorSize(pDevice);
  if ((SectorSize == 0) || (NumBytes < sizeof(CACHE_DATA_RW))) {
    return 0;
  }
  /* Compute number of sectors in cache */
  NumSectors = (NumBytes - sizeof(CACHE_DATA_RW)) / (sizeof(CACHE_BLOCK_INFO_RW) + SectorSize);
  if (NumSectors > 0) {
    pCacheData->NumSectors    = NumSectors;
    pCacheData->SectorSize    = SectorSize;
    _InvalidateCache(pCacheData);
  }
  return NumSectors;
}


/*********************************************************************
*
*       Static code (callbacks)
*
**********************************************************************
*/

/*********************************************************************
*
*       _CacheRW_ReadFromCache
*
*  Description:
*    Read sector from cache if possible
*
*  Return value
*    1    Sector not found
*    0    Sector found
*/
static char _CacheRW_ReadFromCache(FS_DEVICE * pDevice, U32 SectorNo,       void * pData, U8 SectorType) {
  U32             Off;
  CACHE_DATA_RW       * pCacheData;
  CACHE_BLOCK_INFO_RW * pBlockInfo;
  U32             SectorSize;

  FS_USE_PARA(SectorType);
  pCacheData  = (CACHE_DATA_RW *)pDevice->Data.pCacheData;
  if (_ComputeNumSectors(pDevice, pCacheData) == 0) {
    return 1;                          /* Device is not available */
  }
  SectorSize  = pCacheData->SectorSize;
  Off         = _GetHashCode(SectorNo, pCacheData->NumSectors) * (sizeof(CACHE_BLOCK_INFO_RW) + SectorSize);
  pBlockInfo  = (CACHE_BLOCK_INFO_RW *) (((U8 *)(pCacheData + 1)) + Off);
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
static void _WriteIntoCache(CACHE_BLOCK_INFO_RW  * pBlockInfo, U32 SectorNo, const void * pData, U32 SectorSize) {
  pBlockInfo->SectorNo = SectorNo;
  FS_MEMCPY(pBlockInfo + 1, pData, SectorSize);
}

/*********************************************************************
*
*       _CacheRW_UpdateCache
*
*  Description:
*    Updates a sector in cache.
*    Called after a READ operation to update the cache.
*    This means that the sector can not be in the cache.
*
*  Return value
*    0    Not in write cache, the physical write operation still needs to be performed (Since this cache is a pure read-cache).
*/
static char _CacheRW_UpdateCache(FS_DEVICE * pDevice, U32 SectorNo, const void * pData, U8 SectorType) {
  U32             Off;
  CACHE_DATA_RW       * pCacheData;
  CACHE_BLOCK_INFO_RW * pBlockInfo;
  U32             SectorSize;
  int                CacheMode;

  pCacheData  = (CACHE_DATA_RW *)pDevice->Data.pCacheData;
  CacheMode   = pCacheData->aCacheMode[SectorType];
  if (_ComputeNumSectors(pDevice, pCacheData) == 0) {
    return 0;                          /* Device is not available */
  }
  if (CacheMode & FS_CACHE_MODE_R) {           /* Read cache is on for this type of sector */
    SectorSize  = pCacheData->SectorSize;
    Off         = _GetHashCode(SectorNo, pCacheData->NumSectors) * (sizeof(CACHE_BLOCK_INFO_RW) + SectorSize);
    pBlockInfo  = (CACHE_BLOCK_INFO_RW *) (((U8 *)(pCacheData + 1)) + Off);
    /* If we replace an other, dirty sector, we need to write it out */
    if ((pBlockInfo->SectorNo != SectorNo) && (pBlockInfo->IsDirty)) {
      if ((pDevice->pType->pfWrite)(pDevice->Data.Unit, pBlockInfo->SectorNo, pBlockInfo + 1, 1, 0)) {
        FS_DEBUG_ERROROUT((FS_MTYPE_CACHE, "Failure when cleaning cache"));   /* FATAL error ! */
      }
      FS_DEBUG_LOG((FS_MTYPE_CACHE, "Cleaning %s:%d: Sector: 0x%8x", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, pBlockInfo->SectorNo));
    }
    _WriteIntoCache(pBlockInfo, SectorNo, pData, SectorSize);
    pBlockInfo->IsDirty = 0;
  }
  return 0;
}

/*********************************************************************
*
*       _CacheRW_WriteCache
*
*  Description:
*    Writes a sector into cache.
*
*  Return value
*    0    Not  in write cache, the physical write operation still needs to be performed.
*    1    Data in write cache, the physical write operation does not need to be performed.
*/
static char _CacheRW_WriteCache(FS_DEVICE * pDevice, U32 SectorNo, const void * pData, U8 SectorType) {
  U32             Off;
  CACHE_DATA_RW       * pCacheData;
  CACHE_BLOCK_INFO_RW * pBlockInfo;
  U32             SectorSize;
  int                CacheMode;
  char               WriteRequired;

  pCacheData    = (CACHE_DATA_RW *)pDevice->Data.pCacheData;
  if (_ComputeNumSectors(pDevice, pCacheData) == 0) {
    return 0;                          /* Device is not available */
  }
  CacheMode     = pCacheData->aCacheMode[SectorType];
  SectorSize    = pCacheData->SectorSize;
  Off           = _GetHashCode(SectorNo, pCacheData->NumSectors) * (sizeof(CACHE_BLOCK_INFO_RW) + SectorSize);
  pBlockInfo    = (CACHE_BLOCK_INFO_RW *) (((U8 *)(pCacheData + 1)) + Off);
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
      FS_DEBUG_LOG((FS_MTYPE_CACHE, "Cleaning %s:%d: Sector: 0x%8x", pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit, pBlockInfo->SectorNo));
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
*       _CacheRW_InvalidateCache 
*
*  Description:
*    Invalidates all data in cache
*/
static void _CacheRW_InvalidateCache(void * p) {
  CACHE_DATA_RW * pCacheData;

  pCacheData = (CACHE_DATA_RW *)p;
  _InvalidateCache(pCacheData);
  pCacheData->NumSectors = 0;
  pCacheData->SectorSize = 0;
}

/*********************************************************************
*
*       _SetMode
*
*  Description:
*    Sets the mode for the give type of sectors.
*
*  Return value:
*/
static void _SetMode(FS_DEVICE * pDevice, CACHE_MODE * pCacheMode) {
  int i;
  CACHE_DATA_RW * pCacheData;

  pCacheData = (CACHE_DATA_RW *)pDevice->Data.pCacheData;
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
*       _Clean
*
*  Description:
*    Writes out all dirty sectors from cache.
*
*  Return value:
*/
static int _Clean(FS_DEVICE * pDevice) {
  U32             i;
  U32             NumSectors;
  CACHE_DATA_RW *       pCacheData;
  CACHE_BLOCK_INFO_RW * pBlockInfo;
  U32             SectorSize;
  U32             SizeOfCacheBlock;

  pCacheData = (CACHE_DATA_RW *)pDevice->Data.pCacheData;
  NumSectors = pCacheData->NumSectors;
  SectorSize = pCacheData->SectorSize;
  SizeOfCacheBlock = sizeof(CACHE_BLOCK_INFO_RW) + SectorSize;
  for (i = 0; i < NumSectors; i++) {
    pBlockInfo = (CACHE_BLOCK_INFO_RW *) (((U8 *)(pCacheData + 1)) + (i * SizeOfCacheBlock));

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
*       _CacheRW_Command
*
*  Description:
*    Execute commands on the cache
*
*  Return value:
*    Unsupported command:    -1
*    Supported commands:     <0: Error. Precise value depends on command
*/
static int _CacheRW_Command(FS_DEVICE * pDevice, int Cmd, void *p) {
  int r;

  r  = -1;
  switch (Cmd) {
  case FS_CMD_CACHE_CLEAN:
    r = _Clean(pDevice);
    break;
  case FS_CMD_CACHE_SET_MODE:
    _SetMode(pDevice, (CACHE_MODE *)p);
    r = 0;
    break;
  case FS_CMD_CACHE_INVALIDATE:
    _CacheRW_InvalidateCache(p);
    r = 0;
    break;
  }
  return r;
}

/*********************************************************************
*
*       _CacheRWAPI
*
*/
static const FS_CACHE_API _CacheRWAPI = {
  _CacheRW_ReadFromCache,
  _CacheRW_UpdateCache,
  _CacheRW_InvalidateCache,
  _CacheRW_Command,
  _CacheRW_WriteCache
};

/*********************************************************************
*
*       _CacheRW_Init
*
*  Description:
*    Initializes the cache
*
*  Return value
*     Returns the number of cache blocks (Number of sectors that can be cached)
*/
U32 FS_CacheRW_Init(FS_DEVICE * pDevice, void * pData, I32 NumBytes) {
  FS_DEVICE_DATA * pDevData;
  U32              NumSectors;
  CACHE_DATA_RW  * pCacheData;
  U8             * pData8;

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
  if ((U32)NumBytes < sizeof(CACHE_DATA_RW)) {
    return 0;
  }
  pCacheData = (CACHE_DATA_RW *)pData8;
  FS_MEMSET(pCacheData, 0, sizeof(CACHE_DATA_RW));
  pDevData->pCacheAPI       = &_CacheRWAPI;
  pDevData->pCacheData      = pCacheData;
  pCacheData->NumBytesCache = NumBytes;
  NumSectors = _ComputeNumSectors(pDevice, pCacheData);
  return NumSectors;
}
#else

void CacheRW_c(void);
void CacheRW_c(void) {}

#endif /* FS_SUPPORT_CACHE */

/*************************** End of file ****************************/
