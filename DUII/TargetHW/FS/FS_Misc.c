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
File        : FS_Misc.c
Purpose     : Misc. API functions
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*        #include Section
*
**********************************************************************
*/

#include "FS_Int.h"

/*********************************************************************
*
*        Local data types
*
**********************************************************************
*/

typedef struct {
  const char * Mode;
  U8 AccessFlags;
  U8 DoDel;
  U8 DoOpen;
  U8 DoCreate;
} _FS_MODE_TYPE;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/

static const _FS_MODE_TYPE _aAccessMode[] = {
                                         /* DEL  OPEN  CREATE  READ  WRITE  APPEND  CREATE  BINARY */
  { "r"   ,  FS_FILE_ACCESS_FLAG_R,         0,   1,    0},
  { "rb"  ,  FS_FILE_ACCESS_FLAGS_BR,       0,   1,    0}, /* 1,    0,     0,       0,     1 */
  { "w"   ,  FS_FILE_ACCESS_FLAGS_CW,       1,   0,    1}, /* 0,    1,     0,       1,     0 */
  { "wb"  ,  FS_FILE_ACCESS_FLAGS_BCW,      1,   0,    1}, /* 0,    1,     0,       1,     1 */
  { "a"   ,  FS_FILE_ACCESS_FLAGS_ACW,      0,   1,    1}, /* 0,    1,     1,       1,     0 */
  { "ab"  ,  FS_FILE_ACCESS_FLAGS_ABCW,     0,   1,    1}, /* 0,    1,     1,       1,     1 */
  { "r+"  ,  FS_FILE_ACCESS_FLAGS_RW,       0,   1,    0}, /* 1,    1,     0,       0,     0 */
  { "r+b" ,  FS_FILE_ACCESS_FLAGS_BRW,      0,   1,    0}, /* 1,    1,     0,       0,     1 */
  { "rb+" ,  FS_FILE_ACCESS_FLAGS_BRW,      0,   1,    0}, /* 1,    1,     0,       0,     1 */
  { "w+"  ,  FS_FILE_ACCESS_FLAGS_CRW,      1,   0,    1}, /* 1,    1,     0,       1,     0 */
  { "w+b" ,  FS_FILE_ACCESS_FLAGS_BCRW,     1,   0,    1}, /* 1,    1,     0,       1,     1 */
  { "wb+" ,  FS_FILE_ACCESS_FLAGS_BCRW,     1,   0,    1}, /* 1,    1,     0,       1,     1 */
  { "a+"  ,  FS_FILE_ACCESS_FLAGS_ACRW,     0,   1,    1}, /* 1,    1,     1,       1,     0 */
  { "a+b" ,  FS_FILE_ACCESS_FLAGS_ABCRW,    0,   1,    1}, /* 1,    1,     1,       1,     1 */
  { "ab+" ,  FS_FILE_ACCESS_FLAGS_ABCRW,    0,   1,    1}  /* 1,    1,     1,       1,     1 */
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _CalcSizeInKB
*
*  Function description
*     Given the numbers of clusters, sectors per cluster and bytes per sector,
*     calculate the equivalent number of kilo bytes.
*
*  Parameters:
*     NumClusters         - The Number of sectors
*     SectorsPerCluster   - The number of sectors in a cluster
*     BytesPerSector      - The number of bytes in a sector
*
*  Return value:
*     The number of kilo bytes (KB).
*/
static U32 _CalcSizeInKB(U32 NumClusters, U32 SectorsPerCluster, U32 BytesPerSector) {
  U32 BytesPerCluster;
  int NumShifts;

  BytesPerCluster = SectorsPerCluster * BytesPerSector;
  NumShifts = 10;
  do {
    if (BytesPerCluster == 1) {
      break;
    }
    BytesPerCluster >>= 1;
  } while (--NumShifts);
  return BytesPerCluster * (NumClusters >> NumShifts);
}


/*********************************************************************
*
*       _Text2Mode
*
*  Function description
*    Converts the "open-mode-string" into flags using a table.
*/
static int _Text2Mode(const char * sMode) {
  unsigned j;
  for (j = 0; j < COUNTOF(_aAccessMode); j++) { /* Go through whole list */
    if (FS_STRCMP(sMode, _aAccessMode[j].Mode) == 0) {
      return j;
    }
  }
  return -1;       /* Not a valid access mode */
}

/*********************************************************************
*
*       _SB_Clean
*/
static void _SB_Clean(FS_SB * pSB) {
  if (pSB->HasError) {
    return;       /* Previous error, do not continue */
  }
  if (pSB->IsDirty) {
    if (FS_LB_WritePart(pSB->pPart, pSB->SectorNo, pSB->pBuffer, pSB->Type)) {
      pSB->HasError = 1;
    }
    /* Handle the optional sector copy (Typically used for the second FAT) */
#if FS_MAINTAIN_FAT_COPY
    if (pSB->WriteCopyOff) {
      FS_LB_WritePart(pSB->pPart, pSB->SectorNo + pSB->WriteCopyOff, pSB->pBuffer, pSB->Type);
    }
#endif
    pSB->IsDirty = 0;
  }
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Public code, internal functions
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__AllocSectorBuffer
*
*  Description:
*    Allocate a sector buffer.
*
*  Return value:
*    ==0         - Cannot allocate a buffer.
*    !=0         - Address of a buffer.
*/
U8 * FS__AllocSectorBuffer(void) {
  unsigned i;
  U8 * r;

  r = (U8*)NULL;
  FS_LOCK_SYS();
  for (i = 0; i < FS_Global.NumSectorBuffers; i++) {
    if (FS_Global.paSectorBuffer[i].InUse == 0) {
      FS_Global.paSectorBuffer[i].InUse  = 1;
      r = (U8 *)FS_Global.paSectorBuffer[i].pBuffer;
      break;
    }
  }
  if (r == (U8 *)NULL) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__AllocSectorBuffer: No sector buffer available.\n"));
  }
  FS_UNLOCK_SYS();
  return r;
}

/*********************************************************************
*
*       FS__FreeSectorBuffer
*
*  Description:
*    Free sector buffer.
*
*  Parameters:
*    pBuffer     - Pointer to a buffer, which has to be set free.
*/
void FS__FreeSectorBuffer(void *pBuffer) {
  unsigned i;
  FS_LOCK_SYS();
  for (i = 0; i < FS_Global.NumSectorBuffers; i++) {
    if (((void*)FS_Global.paSectorBuffer[i].pBuffer) == pBuffer) {
      FS_Global.paSectorBuffer[i].InUse = 0;
      break;
    }
  }
  FS_UNLOCK_SYS();
}

/*********************************************************************
*
*       FS__SB_Flush
*/
void FS__SB_Flush(FS_SB * pSB) {
  pSB->IsDirty = 1;
  _SB_Clean(pSB);
}

/*********************************************************************
*
*       FS__SB_Create
*
*  Function description
*    Creates a Smart buffer
*/
char FS__SB_Create(FS_SB * pSB, FS_PARTITION * pPart) {
  U8   * pBuffer;

  FS_MEMSET(pSB, 0, sizeof(FS_SB));
  pBuffer = FS__AllocSectorBuffer();
  if (pBuffer == NULL) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS_SB_Create: No sector buffer available.\n"));
    return 1;               /* Error, no buffer */
  }
  pSB->pBuffer  = pBuffer;
  pSB->pPart    = pPart;
  pSB->SectorNo = 0xFFFFFFFFUL;
  pSB->Type     = FS_SB_TYPE_DATA;
  return 0;               /* O.K. */
}

/*********************************************************************
*
*       FS__SB_Delete
*/
void FS__SB_Delete(FS_SB * pSB) {
  _SB_Clean(pSB);
  FS__FreeSectorBuffer(pSB->pBuffer);
}

/*********************************************************************
*
*       FS__SB_Clean
*
*  Function description
*    Cleans the smart buffer: If the buffer is marked as being dirty,
*    it is written.
*/
void FS__SB_Clean(FS_SB * pSB) {
  _SB_Clean(pSB);
}
/*********************************************************************
*
*       FS__SB_MarkDirty
*/
void FS__SB_MarkDirty(FS_SB * pSB) {
  pSB->IsDirty = 1;
}

/*********************************************************************
*
*       FS__SB_SetWriteCopyOff
*
*  Function description
*    Sets the "WriteCopyOffset", which is the offset of the sector to write a copy to.
*    Typically used for FAT  sectors only.
*/
#if FS_MAINTAIN_FAT_COPY
void FS__SB_SetWriteCopyOff(FS_SB * pSB, U32 Off) {
  pSB->WriteCopyOff = Off;
}
#endif

/*********************************************************************
*
*       FS__SB_SetSector
*
*  Function description
*    Assigns a sector to a smart buffer.
*
*/
void FS__SB_SetSector(FS_SB * pSB, U32 SectorNo, U8 Type) {
  if (SectorNo != pSB->SectorNo) {
    if (pSB->IsDirty) {
      _SB_Clean(pSB);
    }
    pSB->SectorNo = SectorNo;
    pSB->Type     = Type;
    pSB->Read     = 0;
#if FS_MAINTAIN_FAT_COPY
    pSB->WriteCopyOff = 0;
#endif
  }
}

/*********************************************************************
*
*       FS__SB_MarkValid
*
*  Function description
*    Marks a buffer as containing valid sector data.
*    Useful if a buffer is filled and needs to be written later on.
*/
void FS__SB_MarkValid(FS_SB * pSB, U32 SectorNo, U8 Type) {
  FS__SB_SetSector(pSB, SectorNo, Type);
  pSB->Read     = 1;
  pSB->IsDirty  = 1;
}


/*********************************************************************
*
*       FS__SB_MarkNotDirty
*
*  Function description
*    Marks a buffer as containing valid sector data.
*    Useful if a buffer is filled and needs to be written later on.
*/
void FS__SB_MarkNotDirty(FS_SB * pSB) {
  pSB->IsDirty  = 0;
}


/*********************************************************************
*
*       FS__SB_Read
*
*  Return value
*    0    O.K.
*    !=0  Error
*/
char FS__SB_Read(FS_SB * pSB) {
  if (pSB->HasError) {
    return 1;       /* Previous error, do not continue */
  }
  if ((pSB->Read) == 0) {
    if (FS_LB_ReadPart(pSB->pPart, pSB->SectorNo, pSB->pBuffer, pSB->Type)) {
      pSB->HasError = 1;
      return 1;     /* Read failed */
    } else {
      pSB->Read = 1;
    }
  }
  return 0;       /* Sector read successfully */
}

/*********************************************************************
*
*       FS__SB_Write
*
*  Return value
*    0    O.K.
*    !=0  Error
*/
char FS__SB_Write(FS_SB * pSB) {
  FS_DEBUG_ASSERT(FS_MTYPE_FS, pSB->SectorNo != 0xFFFFFFFF);
  if (pSB->HasError) {
    return 1;       /* Previous error, do not continue */
  }
  if (FS_LB_WritePart(pSB->pPart, pSB->SectorNo, pSB->pBuffer, pSB->Type)) {
    pSB->HasError = 1;
    return 1;     /* Write failed */
  }
  pSB->IsDirty = 0;
  return 0;       /* Sector written successfully */
}


/*********************************************************************
*
*       FS__AllocFileHandle
*
*  Function description:
*    Returns a free file handle.
*
*  Return value:
*    pFile      - A valid free file handle
*
*/
FS_FILE * FS__AllocFileHandle(void) {
  FS_FILE * pFile;

  FS_LOCK_SYS();
  pFile = FS_Global.pFirstFilehandle;
  while (1) { /* While no free entry found. */
    if (pFile->InUse == 0) {
      FS_FILE * pNext;
      //
      // Save the pNext pointer to restore it back.
      //
      pNext = pFile->pNext;
      FS_MEMSET(pFile, 0, sizeof(FS_FILE));
      pFile->InUse = 1;
      pFile->pNext = pNext;
      break;
    }
#ifndef FS_TRIAL
    if (pFile->pNext == NULL) {
      pFile->pNext = (FS_FILE *)FS_TryAlloc(sizeof(FS_FILE));
      //
      // Check if we got a valid pointer.
      //
      if (pFile->pNext) {
        FS_MEMSET(pFile->pNext, 0, sizeof(FS_FILE));
      }
    }
#endif
    pFile = pFile->pNext;
    //
    // Neither a free file handle found
    // nor enough space to allocate a new one.
    //
    if (pFile == NULL) {
      break;
    }
  }
  FS_UNLOCK_SYS();
  return pFile;
}

/*********************************************************************
*
*       FS__FreeFileHandle
*
*  Function description:
*    Closes the file handle and mark it as free.
*
*  Parameters:
*    pFile       - Pointer to an opened file handle.
*
*/
void FS__FreeFileHandle(FS_FILE * pFile) {
  if (pFile) {
    FS_LOCK_SYS();
    pFile->InUse = 0;
    pFile->pFileObj = (FS_FILE_OBJ*)NULL;
    FS_UNLOCK_SYS();
  }
}


/*********************************************************************
*
*       FS__AllocFileObj
*
*  Function description:
*    Returns a free file handle.
*
*  Parameter(s)
*    sFullFileName    Points to a full file name. May NOT be NULL.
*
*  Return value:
*    pFileObj   - A valid free file Obj
*
*/
FS_FILE_OBJ * FS__AllocFileObj(const char * sFullFileName) {
  FS_FILE_OBJ * pFileObj;

  /*  Find next free entry */
  FS_LOCK_SYS();
  pFileObj = FS_Global.pFirstFileObj;
  while (1) {
    if (pFileObj->UseCnt == 0) {
      //
      // Init File object
      //
      pFileObj->FirstCluster = 0;
      pFileObj->Size         = 0;
      pFileObj->pVolume      = NULL;
      FS_MEMSET(&pFileObj->Data, 0, sizeof(pFileObj->Data));
      pFileObj->UseCnt++;
#if FS_MULTI_HANDLE_SAFE
      FS_STRCPY(pFileObj->acFullFileName, sFullFileName);
#else
      FS_USE_PARA(sFullFileName);
#endif
      break;
    }
#ifndef FS_TRIAL
    if (pFileObj->pNext == NULL) {
      pFileObj->pNext = (FS_FILE_OBJ *)FS_TryAlloc(sizeof(FS_FILE_OBJ));
      //
      // Check if we got a valid pointer.
      //
      if (pFileObj->pNext) {
        FS_MEMSET(pFileObj->pNext, 0, sizeof(FS_FILE_OBJ));
      }
    }
#endif
    pFileObj = pFileObj->pNext;
    //
    // Neither a free file handle found
    // nor enough space to allocate a new one.
    //
    if (pFileObj == NULL) {
      break;
    }
  }
  FS_UNLOCK_SYS();
  return pFileObj;
}

/*********************************************************************
*
*       FS__GetFileObj
*
*  Function description:
*    Returns a free file handle.
*
*  Return value:
*    pFileObj     - A valid file obj
*
*/
FS_FILE_OBJ * FS__GetFileObj(const char * sFullFileName) {
  FS_FILE_OBJ * pFileObj;
#if FS_MULTI_HANDLE_SAFE
  pFileObj = FS_Global.pFirstFileObj;
  /*  Find next free entry */
  FS_LOCK_SYS();
  while (pFileObj) {
    if (FS_STRCMP(sFullFileName, pFileObj->acFullFileName) == 0) {
      pFileObj->UseCnt++;
      break;
    }
    pFileObj = pFileObj->pNext;
  }
  FS_UNLOCK_SYS();
#else
  FS_USE_PARA(sFullFileName);
  pFileObj = (FS_FILE_OBJ *)NULL;
#endif
  return pFileObj;
}

/*********************************************************************
*
*       FS__FreeFileObj
*
*  Function description:
*    Closes the file object.
*
*  Parameters:
*    pFile       - Pointer to an open file obj.
*
*/
void FS__FreeFileObj(FS_FILE_OBJ * pFileObj) {
  if (pFileObj) {
    FS_LOCK_SYS();
    if (pFileObj->UseCnt) {
      pFileObj->UseCnt--;
    }
#if FS_MULTI_HANDLE_SAFE
    if (pFileObj->UseCnt == 0) {
      //
      // Empty string when this file object is not used anymore.
      //
      pFileObj->acFullFileName[0] = '\0';
    }
#endif
    FS_UNLOCK_SYS();
  }
}

/*********************************************************************
*
*       _BuildFullFileName
*
*  Function description
*    Stores the full filename (including volume and path) into the destination
*    Buffer
*
*  Return value:
*    0      O.K.
*    1
*/
#if FS_MULTI_HANDLE_SAFE
static int _BuildFullFileName(FS_VOLUME * pVolume, const char * sFileName, char * sDest) {
  int Len;
  int Len1;
  const char * sDriverName;
  FS_DEVICE  * pDevice;

  if (pVolume == (FS_VOLUME *)NULL) {
    return 1;
  }

  pDevice = &pVolume->Partition.Device;
  sDriverName = pDevice->pType->pfGetName(pDevice->Data.Unit);
  Len = FS_STRLEN(sDriverName);
  FS_STRCPY(sDest, sDriverName);
  *(sDest + Len++) = ':';
  *(sDest + Len++) = '0' + pDevice->Data.Unit;
  *(sDest + Len++) = ':';
  Len1 = FS_STRLEN(sFileName);
  if ((Len1 + Len) >= FS_MAX_LEN_FULL_FILE_NAME) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API,"_BuildFullFileName: sFileName is too long to store in sDest."));
    return 1;   /* Error: file name is too long to store in sDest */
  }
  if (*sFileName != FS_DIRECTORY_DELIMITER) {
    *(sDest + Len++) = FS_DIRECTORY_DELIMITER;
  }
  FS_STRCPY((sDest + Len), sFileName);
  return 0;
}
#endif

/*********************************************************************
*
*       FS__FTell
*
*  Function description:
*    Internal version if FS_FTell
*    Return position of a file pointer.
*
*  Parameters:
*    pFile         - Pointer to a FS_FILE data structure.
*
*  Return value:
*    >=0           - Current position of the file pointer.
*    ==-1          - An error has occurred.
*/
I32 FS__FTell(FS_FILE *pFile) {
  I32 r;
  r = -1;
  if (pFile) {
    FS_LOCK_SYS();
    r =  pFile->FilePos;
    FS_UNLOCK_SYS();
  }
  return r;
}

/*********************************************************************
*
*       FS__FCloseNL
*
*  Function description:
*    Internal version of FS_FClose.
*    Close a file referred by pFile.
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*
*  Return value:
*    1           - Error, File handle can not be closed.
*    0           - File handle has been closed.
*/
int FS__FCloseNL(FS_FILE *pFile) {
  if (pFile->InUse) {
    FS_FILE_OBJ * pFileObj;

    pFileObj = pFile->pFileObj;
    FS_CLOSE_FILE(pFile);  /* Execute the FSL function */
    FS__FreeFileObj(pFileObj);
    FS__FreeFileHandle(pFile);
  }
  return 0;
}

/*********************************************************************
*
*       FS__FClose
*
*  Function description:
*    Internal version of FS_FClose.
*    Close a file referred by pFile.
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*
*  Return value:
*    1           - Error, File handle can not be closed.
*    0           - File handle has been closed.
*/
int FS__FClose(FS_FILE *pFile) {
  FS_FILE_OBJ * pFileObj;
  FS_DEVICE   * pDevice;
  FS_VOLUME   * pVolume;
  char          InUse;

  pVolume = (FS_VOLUME *)NULL;
  pDevice = (FS_DEVICE *)NULL;
  FS_LOCK_SYS();
  InUse = pFile->InUse;
  pFileObj = pFile->pFileObj;
  if (pFileObj) {
    pVolume  = pFileObj->pVolume;
  }
  if (pVolume) {
    pDevice  = &pVolume->Partition.Device;
  }
  FS_UNLOCK_SYS();
  if ((InUse == 0) || (pVolume == NULL)) {
    return 1;
  }
  FS_USE_PARA(pDevice);
  FS_USE_PARA(pVolume);
  FS_LOCK_DRIVER(pDevice);
#if FS_OS_LOCK_PER_DRIVER
  FS_LOCK_SYS();
  if (pFileObj != pFile->pFileObj) {
    InUse = 0;
  }
  if (pFile->InUse == 0) {
    InUse = 0;
  }
  FS_UNLOCK_SYS();
  if (InUse == 0) {      // Let's make sure the file is still valid
    FS_DEBUG_ERROROUT((FS_MTYPE_API,"Application error: File handle has been invalidated by other thread during wait"));
    FS_UNLOCK_DRIVER(pDevice);
    return 1;

  } else
#endif
  {
    FS_JOURNAL_BEGIN (pVolume);
    FS__FCloseNL(pFile);
    FS_JOURNAL_END (pVolume);
  }
  FS_UNLOCK_DRIVER(pDevice);
  return 0;
}


/*********************************************************************
*
*       _FSeekNL
*
*  Function description:
*    Internal version of FS_FSeek
*    Set current position of a file pointer.
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*    Offset      - Offset for setting the file pointer position.
*    Origin      - Mode for positioning the file pointer.
*
*  Return value:
*    ==0         - File pointer has been positioned according to the
*                  parameters.
*    ==-1        - An error has occurred.
*/
static int _FSeekNL(FS_FILE *pFile, I32 Offset, int Origin) {
  U32  uOffset;

  uOffset = (U32)Offset;
  if (pFile == NULL) {
    return -1;
  }
  switch (Origin) {
  case FS_SEEK_SET:
    break;
  case FS_SEEK_CUR:
    uOffset += pFile->FilePos;
    break;
  case FS_SEEK_END:
    uOffset += pFile->pFileObj->Size;
    break;
  default:
    pFile->Error = FS_ERR_INVALIDPAR;
    FS_DEBUG_WARN((FS_MTYPE_API, "FS__FSeek: Illegal parameter"));
    return -1;
  }
  if (pFile->FilePos != uOffset) {
    pFile->FilePos  = uOffset;
    pFile->Error    = FS_ERR_OK;    /* Clear any previous error */
  }
  return 0;
}

/*********************************************************************
*
*       FS__FSeek
*
*  Function description:
*    Internal version of FS_FSeek
*    Set current position of a file pointer.
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*    Offset      - Offset for setting the file pointer position.
*    Origin      - Mode for positioning the file pointer.
*
*  Return value:
*    ==0         - File pointer has been positioned according to the
*                  parameters.
*    ==-1        - An error has occurred.
*/
int FS__FSeek(FS_FILE *pFile, I32 Offset, int Origin) {
  int r;

  FS_LOCK_SYS();
  r = _FSeekNL(pFile, Offset, Origin);
  FS_UNLOCK_SYS();
  return r;
}

/*********************************************************************
*
*       FS__CalcSizeInBytes
*
*  Function description
*     Given the numbers of clusters, sectors per cluster and bytes per sector,
*     calculate the equivalent number of bytes.
*
*  Parameters:
*     NumClusters         - The Number of sectors
*     SectorsPerCluster   - The number of sectors in a cluster
*     BytesPerSector      - The number of bytes in a sector
*
*  Return value:
*     The number of bytes, or if the number would exceed the range U32
*     can represent, return 0xFFFFFFFF
*/
U32 FS__CalcSizeInBytes(U32 NumClusters, U32 SectorsPerCluster, U32 BytesPerSector) {
  if (_CalcSizeInKB(NumClusters, SectorsPerCluster, BytesPerSector) < 0x400000UL) {
    return NumClusters * SectorsPerCluster * BytesPerSector;
  } else {
    return 0xFFFFFFFFUL;    // Max. value of U32. The size in bytes does not fit into a U32.
  }
}

/*********************************************************************
*
*       FS__CalcSizeInKB
*
*  Function description
*     Given the numbers of clusters, sectors per cluster and bytes per sector,
*     calculate the equivalent number of KBytes.
*
*  Parameters:
*     NumClusters         - The Number of sectors
*     SectorsPerCluster   - The number of sectors in a cluster
*     BytesPerSector      - The number of bytes in a sector
*
*  Return value:
*     The value in KBytes.
*/
U32 FS__CalcSizeInKB(U32 NumClusters, U32 SectorsPerCluster, U32 BytesPerSector) {
  U32 v;

  v = _CalcSizeInKB(NumClusters, SectorsPerCluster, BytesPerSector);
  return v;
}

/*********************************************************************
*
*       FS__Remove
*
*  Function description:
*    Internal version of FS_Remove
*    Removes a file.
*    There is no real 'delete' function in the FSL, but the FSL's 'open'
*    function can delete a file.
*
*  Parameters:
*    pFileName   - Fully qualified file name.
*
*  Return value:
*    ==0         - File has been removed.
*    ==-1        - An error has occurred.
*/
int FS__Remove(const char *pFileName) {
  int r;
  FS_FILE * pFile;
  r = -1;

  pFile = FS__FOpenEx(pFileName, FS_FILE_ACCESS_FLAG_W, 0, 1, 0);
  if (pFile) {
    FS__FreeFileObj(pFile->pFileObj);
    FS__FreeFileHandle(pFile);
    r = 0;
  }
  return r;
}


/*********************************************************************
*
*       FS__OpenEx
*
*  Function description:
*    Either opens an existing file or create a new one or delete
*    a file existing file.
*
*  Parameters:
*    pVolume     - Pointer to a volume structure.
*    sFilePath   - String to the relative path of the file.
*    AccessFlags - Type of Access.
*    DoCreate    - Shall the file be created.
*    DoDel       - Shall the existing file be deleted.
*    DoOpen      - Shall the file be opened.
*
*  Return value:
*    ==0         - Unable to open the file.
*    !=0         - Address of an FS_FILE data structure.
*/
FS_FILE * FS__OpenEx(FS_VOLUME * pVolume, const char * sFilePath, U8 AccessFlags, char DoCreate, char DoDel, char DoOpen) {
  FS_FILE      * pFile;
  FS_FILE_OBJ  * pFileObj;
  int            r;

  pFile    = NULL;
  pFileObj = NULL;
  /* Find correct FSL  (device:unit:name) */
  //
  // Allocate file object.
  // The procedure depends. If multiple handles per file are allowed, we first need to check if
  // the file is already open.
  //
#if FS_MULTI_HANDLE_SAFE
  {
    char ac[FS_MAX_LEN_FULL_FILE_NAME];
    if (_BuildFullFileName(pVolume, sFilePath, ac)) {
      goto Error;
    }
    /* Find file obj (if the file is already open), else alloc one */
    pFileObj = FS__GetFileObj(ac);
    if ((void*)pFileObj == NULL) {
      pFileObj = FS__AllocFileObj(ac);
    }
  }
#else
  pFileObj = FS__AllocFileObj(NULL);
#endif
  if ((void *)pFileObj == NULL) {
#ifdef FS_TRIAL
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS_FOpen: Trial limitation: Only one file can be opened at once.\n"));
#else
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS_FOpen: No file object available.\n"));
#endif
    goto Error;
  }
  //
  // Allocate file handle.
  //
  pFile    = FS__AllocFileHandle();
  if ((void*)pFile == NULL) {
#ifdef FS_TRIAL
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS_FOpen: Trial limitation: Only one file can be opened at once.\n"));
#else
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS_FOpen: No file handle available.\n"));
#endif
    goto Error;
  }
  //
  // Allocate or invalidate file buffer
  //
#if FS_USE_FILE_BUFFER
  if (pFileObj->pBuffer) {
    pFileObj->pBuffer->NumBytesInBuffer = 0;
  } else {
    if (FS_Global.FileBufferSize) {
      pFileObj->pBuffer = (FS_FILE_BUFFER *)FS_TryAlloc(sizeof(FS_FILE_BUFFER));
      if (pFileObj->pBuffer) {
        void * p;

        p = FS_TryAlloc(FS_Global.FileBufferSize);
        if (p) {
          pFileObj->pBuffer->pData = (U8 *)p;
          pFileObj->pBuffer->BufferSize = FS_Global.FileBufferSize;
          pFileObj->pBuffer->Flags      = FS_Global.FileBufferFlags;
        } else {
          FS_DEBUG_WARN((FS_MTYPE_API, "File buffer could not be allocated to file object"));
          FS_FREE(pFileObj->pBuffer);
          pFileObj->pBuffer = NULL;
        }
      }
    }
  }
#endif
  //
  // Write information to the file handle.
  //
  FS_LOCK_SYS();
  pFile->AccessFlags = AccessFlags;
  pFile->pFileObj    = pFileObj;
  pFileObj->pVolume  = pVolume;
  FS_UNLOCK_SYS();
  FS_JOURNAL_BEGIN (pVolume);
  r = FS_OPEN_FILE(sFilePath, pFile, DoDel, DoOpen, DoCreate);
  FS_JOURNAL_END (pVolume);
  if (r) {
    goto Error;        /* Illegal access flags */
  }
  goto Done;
Error:
  FS__FreeFileHandle(pFile);
  pFile = (FS_FILE*)NULL;
  FS__FreeFileObj(pFileObj);
Done:
  return pFile;
}

/*********************************************************************
*
*       FS__FOpenEx
*
*  Function description:
*    Either opens an existing file or create a new one or delete
*    a file existing file.
*
*  Parameters:
*    sFullFileName - Fully qualified file name.
*    AccessFlags   - Type of Access.
*    DoCreate      - Shall the file be created.
*    DoDel         - Shall the existing file be deleted.
*    DoOpen        - Shall the file be opened.
*
*  Return value:
*    ==0         - Unable to open the file.
*    !=0         - Address of an FS_FILE data structure.
*/
FS_FILE * FS__FOpenEx(const char * sFullFileName, U8 AccessFlags, char DoCreate, char DoDel, char DoOpen) {
  FS_FILE      * pFile;
  const char   * sFileName = NULL;
  FS_VOLUME    * pVolume;
  int            r;

  pVolume = FS__FindVolume(sFullFileName, &sFileName);
  if ((void*)pVolume == NULL) {
    return NULL;
  }
  //
  //  Pre-check if we have a valid file name, the correct handling is done in the FS layer
  //
  if (sFileName) {
    if (*sFileName == '\0') {
      FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__OpenEx: No file name was specified.\n"));
      return NULL;
    }
  }
  r = FS__AutoMount(pVolume);
  if (r <= 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__OpenEx: Volume can not be mounted.\n"));
    return (FS_FILE *)NULL;
  }
  //
  //  Check if we want to write data to the volume and the device is mounted read-only.
  //
  if ((AccessFlags & FS_FILE_ACCESS_FLAGS_ACW) && (r != FS_MOUNT_RW)) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__OpenEx: Volume is mounted read-only, cannot either create file nor write/append to file.\n."));
    return NULL;
  }
  pFile = NULL;
  FS_LOCK_DRIVER(&pVolume->Partition.Device);
#if FS_OS_LOCK_PER_DRIVER
  if (pVolume->IsMounted == 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Application error: Volume has been unmounted by other thread during wait"));
  } else
#endif
  {
    if (sFileName) {
      pFile = FS__OpenEx(pVolume, sFileName, AccessFlags, DoCreate, DoDel, DoOpen);
    }
  }
  FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
  return pFile;
}

/*********************************************************************
*
*       FS__FOpen
*
*  Function description:
*    Internal version of FS_FOpen.
*    Open an existing file or create a new one.
*
*  Parameters:
*    pFileName   - Fully qualified file name.
*    pMode       - Mode for opening the file.
*
*  Return value:
*    ==0         - Unable to open the file.
*    !=0         - Address of an FS_FILE data structure.
*/
FS_FILE * FS__FOpen(const char * pFileName, const char * pMode) {
  FS_FILE      * pFile;
  int            ModeIndex;
  U8          AccessFlags;
  char           DoCreate;
  char           DoDel;
  char           DoOpen;

  pFile = NULL;
  /* Check mode */
  ModeIndex = _Text2Mode(pMode);
  if (ModeIndex < 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__FOpen: Illegal access flags.\n"));
  } else {
    /* All checks have been performed, lets do the work  */
    AccessFlags        = _aAccessMode[ModeIndex].AccessFlags;
    DoDel              = _aAccessMode[ModeIndex].DoDel;
    DoOpen             = _aAccessMode[ModeIndex].DoOpen;
    DoCreate           = _aAccessMode[ModeIndex].DoCreate;
    pFile = FS__FOpenEx(pFileName, AccessFlags, DoCreate, DoDel, DoOpen);
  }
  return pFile;
}

/*********************************************************************
*
*       Public code, API functions
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FOpen
*
*  Function description:
*    Open an existing file or create a new one.
*
*  Parameters:
*    pFileName   - Fully qualified file name.
*    pMode       - Mode for opening the file.
*
*  Return value:
*    ==0         - Unable to open the file.
*    !=0         - Address of an FS_FILE data structure.
*/
FS_FILE * FS_FOpen(const char * pFileName, const char * pMode) {
  FS_FILE      * pFile;

  FS_LOCK();
  pFile = FS__FOpen(pFileName, pMode);
  FS_UNLOCK();
  return pFile;
}



/*********************************************************************
*
*       FS_Remove
*
*  Function description:
*    Remove a file.
*    There is no real 'delete' function in the FSL, but the FSL's 'open'
*    function can delete a file.
*
*  Parameters:
*    pFileName   - Fully qualified file name.
*
*  Return value:
*    ==0         - File has been removed.
*    ==-1        - An error has occurred.
*/
int FS_Remove(const char *sFileName) {
  int r;

  FS_LOCK();
  r = FS__Remove(sFileName);
  FS_UNLOCK();
  return r;
}


/*********************************************************************
*
*       FS_FClose
*
*  Function description:
*    Close a file referred by pFile.
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*
*  Return value:
*    1           - Error, File handle can not be closed.
*    0           - File handle has been closed.
*/
int FS_FClose(FS_FILE *pFile) {
  int r;

  FS_LOCK();
  r  = 1;
  if (pFile) {
    r = FS__FClose(pFile);
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_FSeek
*
*  Function description:
*    Set current position of a file pointer.
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*    Offset      - Offset for setting the file pointer position.
*    Origin      - Mode for positioning the file pointer.b
*
*  Return value:
*    ==0         - File pointer has been positioned according to the
*                  parameters.
*    ==-1        - An error has occurred.
*/
int FS_FSeek(FS_FILE * pFile, I32 Offset, int Origin) {
  int r;
  FS_LOCK();
  r = FS__FSeek(pFile, Offset, Origin);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_FTell
*
*  Function description:
*    Return position of a file pointer.
*
*  Parameters:
*    pFile         - Pointer to a FS_FILE data structure.
*
*  Return value:
*    >=0           - Current position of the file pointer.
*    ==-1          - An error has occurred.
*/
I32 FS_FTell(FS_FILE *pFile) {
  I32 r;
  FS_LOCK();
  r = FS__FTell(pFile);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_FEof
*
*  Function description:
*    Returns if end of file has been reached.
*
*  Parameters:
*    pFile         - Pointer to a FS_FILE data structure.
*
*  Return value:
*    == 1          - End of File has been reached.
*    == 0          - End of File has not been reached.
*/
int FS_FEof(FS_FILE * pFile) {
  int r;
  char InUse;
  FS_LOCK();
  r = 1;
  if (pFile) {

    FS_LOCK_SYS();
    InUse = pFile->InUse;
    FS_UNLOCK_SYS();
    if (InUse) {
      r = (pFile->FilePos >= pFile->pFileObj->Size);
    } else {
      r = -1;
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_FError
*
*  Function description:
*    Return error status of a file.
*
*  Parameters:
*    pFile         - Pointer to a FS_FILE data structure.
*
*  Return value:
*    == FS_ERR_OK  - No error.
*    != FS_ERR_OK  - An error has occurred.
*/
I16 FS_FError(FS_FILE * pFile) {
  I16 r;

  FS_LOCK();
  r = FS_ERR_INVALIDPAR;
  if (pFile) {
    r = pFile->Error;
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_ClearErr
*
*  Function description:
*    API function. Clear error status of a file.
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*
*  Return value:
*    None.
*/
void FS_ClearErr(FS_FILE * pFile) {
  FS_LOCK();
  if (pFile) {
    pFile->Error = FS_ERR_OK;
  }
  FS_UNLOCK();
}

/*********************************************************************
*
*       FS__RemoveDevice
*
*  Description:
*    Removes a volume from the file system.
*
*  Parameters:
*    pVolume    - Pointer to a volume that should be removed.
*  Return value:
*/
void FS__RemoveDevice(FS_VOLUME * pVolume) {
  int          i;
  int          NumVolumes;
  FS_VOLUME  * pVolume2Check;

  pVolume2Check = &FS_Global.FirstVolume;
  NumVolumes = FS_Global.NumVolumes;
  //
  //  Get through the whole volume list.
  //
  for (i = 0; i < NumVolumes; i++) {
    FS_VOLUME * pVolumeNext;

    //
    //  Did we found the correct volume
    //
    if (pVolume2Check == pVolume) {
      FS_DEVICE * pDevice;

      //
      // Send to the underlying layer that the device
      // will be unmounted and removed.
      //
      pDevice = &pVolume->Partition.Device;
      if (FS_LB_GetStatus(pDevice) == FS_MEDIA_NOT_PRESENT) {
        FS__UnmountForcedNL(pVolume);
      } else {
        FS__UnmountNL(pVolume);
      }
      FS__IoCtlNL(pVolume, FS_CMD_DEINIT, 0, NULL);
      //
      //  Remove all relevant journaling information if necessary
      //  (FS_SUPPORT_JOURNAL == 1)
      //
      FS_JOURNAL_DEINIT(pVolume);
      //
      //  Remove the volume instance from the OS device lock instance,
      //  if necessary. (FS_OS_LOCKIN == 2)
      FS_OS_REMOVE_DRIVER(pVolume->Partition.Device.pType);
      pVolumeNext = pVolume->pNext;
      FS_MEMSET(pVolume, 0, sizeof(FS_VOLUME));
      if (pVolume != &FS_Global.FirstVolume) {
        FS_FREE(pVolume);
      }
      if ((pVolume == &FS_Global.FirstVolume) && (pVolumeNext != NULL)) {
        //
        // In case we remove the first volume, we need to move the volume
        // next after the first volume to the first volume.
        // The moved entry will then be removed if freeing memory is available.
        //
        FS_MEMCPY(&FS_Global.FirstVolume, pVolumeNext, sizeof(FS_VOLUME));
        FS_FREE(pVolumeNext);
      } else {
        FS_VOLUME ** ppNextVolume;
        FS_VOLUME  * pNextVolume;
        ppNextVolume = &FS_Global.FirstVolume.pNext;
        pNextVolume  = FS_Global.FirstVolume.pNext;
        do {
          if (pNextVolume == pVolume) {
            *ppNextVolume = pVolumeNext;
            break;
          }
          if (pNextVolume == NULL) {
            break;
          }
          pNextVolume = pNextVolume->pNext;
          ppNextVolume = &(*ppNextVolume)->pNext;

        } while(1);
      }
      FS_Global.NumVolumes--;
      break;
    } else {
      pVolume2Check = pVolume2Check->pNext;
    }

  }
}

/*********************************************************************
*
*       FS__RemoveDevices
*
*  Description:
*    Removes a volume from the file system.
*
*  Parameters:
*    pVolume    - Pointer to a volume that should be removed.
*  Return value:
*/
void FS__RemoveDevices(void) {
  int         i;
  int         NumVolumes;
  FS_VOLUME * pVolume;

  pVolume    = &FS_Global.FirstVolume;
  NumVolumes = FS_Global.NumVolumes;
  //
  //  Get through the whole volume list.
  //
  for (i = 0; i < NumVolumes; i++) {
    FS_VOLUME * pVolumeNext;
    FS_DEVICE * pDevice;

    pDevice = &pVolume->Partition.Device;
    if (FS_LB_GetStatus(pDevice) == FS_MEDIA_NOT_PRESENT) {
      FS__UnmountForcedNL(pVolume);
    } else {
      FS__UnmountNL(pVolume);
    }
    FS__IoCtlNL(pVolume, FS_CMD_DEINIT, 0, NULL);
    FS_JOURNAL_DEINIT(pVolume);
    FS_OS_REMOVE_DRIVER(pVolume->Partition.Device.pType);
    pVolumeNext = pVolume->pNext;
    FS_MEMSET(pVolume, 0, sizeof(FS_VOLUME));
    if (pVolume != &FS_Global.FirstVolume) {
      FS_FREE(pVolume);
    }
    pVolume = pVolumeNext;
  }
  FS_Global.NumVolumes = 0;
}

/*********************************************************************
*
*       FS_Init
*
*  Function description:
*    Start the file system.
*
*/
void FS_Init(void) {
  unsigned NumDriverLocks;
  //
  // Allocate memory for sector buffers
  //
  NumDriverLocks = FS_STORAGE_Init();
  if (NumDriverLocks) {
    U8            * pBuffer;
    SECTOR_BUFFER * pSectorBuffer;
    unsigned        i;

    FS_Global.NumSectorBuffers = FS_NUM_MEMBLOCKS_PER_OPERATION * NumDriverLocks;
    //
    // Alloc memory for the SECTOR_BUFFER structure.
    //
    FS_Global.paSectorBuffer   = (SECTOR_BUFFER *)FS_AllocZeroed(FS_Global.NumSectorBuffers * sizeof(SECTOR_BUFFER));
    pBuffer           = (U8 *)FS_AllocZeroed(FS_Global.MaxSectorSize * FS_Global.NumSectorBuffers);
    pSectorBuffer     = FS_Global.paSectorBuffer;
    for (i = 0; i < FS_Global.NumSectorBuffers; i++) {
      pSectorBuffer->pBuffer = (U32 *)pBuffer;
      pBuffer               += FS_Global.MaxSectorSize;
      pSectorBuffer++;
    }
    FS_Global.pFirstFilehandle = (FS_FILE     *)FS_AllocZeroed(sizeof(FS_FILE));
    FS_Global.pFirstFileObj    = (FS_FILE_OBJ *)FS_AllocZeroed(sizeof(FS_FILE_OBJ));
    FS_Global.IsInited |= (1 << 1);  // Set InitStatus to FS-Complete init state.
  }
}

/*********************************************************************
*
*       FS_ConfigUpdateDirOnWrite
*
*  Description:
*    This sets whether the directory entry after writing the data to file
*    shall be updated or not.
*
*  Parameters:
*    OnOff       - 1 means enable  update directory after write
*                  0 means do not  update directory. FS_FClose will
*                    update the directory entry
*/
void FS_ConfigUpdateDirOnWrite(char OnOff) {
  FS_LOCK();
  FS_LOCK_SYS();
  if (OnOff) {
    FS_Global.WriteMode = FS_WRITEMODE_SAFE;
  } else {
    FS_Global.WriteMode = FS_WRITEMODE_MEDIUM;
  }
  FS_UNLOCK_SYS();
  FS_UNLOCK();
}

#if FS_USE_FILE_BUFFER

/*********************************************************************
*
*       FS_ConfigFileBufferDefault
*
*  Function description
*    Configures default file buffer size and flags for all files.
*    Should only be called once, in FS_X_AddDevices()
*/
void FS_ConfigFileBufferDefault(int BufferSize, int Flags) {
  FS_LOCK();
  FS_LOCK_SYS();
  FS_Global.FileBufferSize  = BufferSize;
  FS_Global.FileBufferFlags = Flags;
  FS_UNLOCK_SYS();
  FS_UNLOCK();
}

/*********************************************************************
*
*       FS_ConfigFileBufferFlags
*
*  Description:
*    Is only allowed to be called immediately after a FS_FOpen(),
*    in order to change the buffer flags of a specific file
*/
void FS_ConfigFileBufferFlags(FS_FILE * pFile, int Flags) {
  FS_FILE_BUFFER * pFileBuffer;

  pFileBuffer = pFile->pFileObj->pBuffer;
  if (pFileBuffer) {
    pFileBuffer->Flags = Flags;
  } else {
    FS_DEBUG_WARN((FS_MTYPE_API, "File buffer not set"));
  }
}

#endif

/*********************************************************************
*
*       FS_RemoveDevice
*
*  Function description:
*    Removes an instance of a device driver.
*
*/
void FS_RemoveDevice(const char * sVolume) {
  FS_VOLUME * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    FS__RemoveDevice(pVolume);
  }
  FS_UNLOCK();
}

/*********************************************************************
*
*       FS_SetFileWriteMode
*
*  Description:
*    Configures the file write mode.
*
*
*  Parameters:
*    WriteMode:   FS_WRITEMODE_SAFE   - Updates FAT and directory entry after each write operation
*                 FS_WRITEMODE_MEDIUM - Updates FAT after each write operation
*                 FS_WRITEMODE_FAST   - FAT and directory entry are updated when the file is closed.
*/
void FS_SetFileWriteMode(FS_WRITEMODE WriteMode) {
  FS_LOCK();
  FS_LOCK_SYS();
  FS_Global.WriteMode = WriteMode;
  FS_UNLOCK_SYS();
  FS_UNLOCK();
}


/*************************** End of file ****************************/
