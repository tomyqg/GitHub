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
File        : FS_Core.c
Purpose     : File system's Core routines
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#define FS_CORE_C            // Identify this C-file for header

#include <stdlib.h>
#include "FS_Int.h"

/*********************************************************************
*
*       #define constants
*
**********************************************************************
*/


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#if FS_SUPPORT_EXT_MEM_MANAGER
static FS_PF_ALLOC  *  _pfAlloc;
static FS_PF_FREE   *  _pfFree;
#else
static U32 * _pMem;
static U32   _NumBytesAvailable;
       U32   FS_NumBytesAllocated;            // Public for diagnostic purposes only. Allows user to check how much memory was really needed.
static U32   _NumAllocs;
#endif

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
*       FS__DivideU32Up
*/
U32 FS__DivideU32Up(U32 Nom, U32 Div) {
  return (Nom + Div - 1) / Div;
}

/*********************************************************************
*
*       FS__strchr
*
*  Description:
*    FS internal function. Locate the first occurrence of c (converted
*    to a char) in the string pointed to by s.
*
*  Parameters:
*    s           - Pointer to a zero terminated string.
*    c           - 'Character' value to find.
*
*  Return value:
*    NULL        - c was not found
*    != NULL     - Pointer to the located character in s.
*/
const char * FS__strchr(const char *s, int c) {
  const char ch = c;

  for (; *s != ch; ++s) {
    if (*s == '\0') {
      return (const char *)0;
    }
  }
  return s;
}

/*********************************************************************
*
*       FS_SetBusyLEDCallback
*
*/
void FS_SetBusyLEDCallback(const char * sVolumeName, FS_BUSY_LED_CALLBACK * pfBusyLEDCallback) {
#if FS_SUPPORT_BUSY_LED
  FS_VOLUME * pVolume;
  FS_LOCK();
  if (sVolumeName) {
    pVolume = FS__FindVolume(sVolumeName, NULL);
    FS_LOCK_SYS();
    if (pVolume) {
      pVolume->Partition.Device.Data.pfSetBusyLED = pfBusyLEDCallback;
    }
    FS_UNLOCK_SYS();
  }
  FS_UNLOCK();
#else
  FS_USE_PARA(sVolumeName);
  FS_USE_PARA(pfBusyLEDCallback);
  FS_DEBUG_WARN((FS_MTYPE_STORAGE,"FS_SetBusyLEDCallback() has no function because FS_SUPPORT_BUSY_LED is disabled."));
#endif
}

/*********************************************************************
*
*       FS_SetMemAccessCallback
*
*/
void FS_SetMemAccessCallback(const char * sVolumeName, FS_MEMORY_IS_ACCESSIBLE_CALLBACK * pfIsAccessibleCallback) {
#if FS_SUPPORT_CHECK_MEMORY
  FS_VOLUME * pVolume;
  FS_LOCK();
  if (sVolumeName) {
    pVolume = FS__FindVolume(sVolumeName, NULL);
    FS_LOCK_SYS();
    if (pVolume) {
      pVolume->Partition.Device.Data.pfMemoryIsAccessible = pfIsAccessibleCallback;
    }
    FS_UNLOCK_SYS();
  }
  FS_UNLOCK();
#else
  FS_USE_PARA(sVolumeName)
  FS_USE_PARA(pfIsAccessibleCallback);
  FS_DEBUG_WARN((FS_MTYPE_API, "FS_SetMemAccessCallback() has no function because FS_SUPPORT_CHECK_MEMORY is disabled."));
#endif
}


/*********************************************************************
*
*       FS_LoadU16BE
*
*  Function description:
*    Reads a 16 bit value stored in big endian format from a byte array.
*/
U16 FS_LoadU16BE(const U8 *pBuffer) {
  U16 r;
  r = *pBuffer++;
  r = (r << 8) | *pBuffer;
  return r;
}

/*********************************************************************
*
*       FS_LoadU32BE
*
*  Function description:
*    Reads a 32 bit value stored in big endian format from a byte array.
*/
U32 FS_LoadU32BE(const U8 *pBuffer) {
  U32 r;
  r = *pBuffer++;
  r = (r << 8) | *pBuffer++;
  r = (r << 8) | *pBuffer++;
  r = (r << 8) | *pBuffer;
  return r;
}

/*********************************************************************
*
*       FS_StoreU16BE
*
*  Function description:
*    Stores a 16 bit value in big endian format into a byte array.
*/
void FS_StoreU16BE(U8 *pBuffer, unsigned Data) {
  *pBuffer++ = (U8)(Data >> 8);
  *pBuffer   = (U8) Data;
}

/*********************************************************************
*
*       FS_StoreU32BE
*
*  Function description:
*    Stores a 32 bit value in big endian format into a byte array.
*/
void FS_StoreU32BE(U8 *pBuffer, U32 Data) {
  *pBuffer++ = (U8)(Data >> 24);
  *pBuffer++ = (U8)(Data >> 16);
  *pBuffer++ = (U8)(Data >> 8);
  *pBuffer   = (U8) Data;
}


/*********************************************************************
*
*       FS_LoadU32LE
*
*  Function description:
*    Reads a 32 bit little endian from a char array.
*
*  Parameters:
*    pBuffer     - Pointer to a char array.
*
*  Return value:
*    result      - The value as U32 data type
*
*/
U32 FS_LoadU32LE(const U8 *pBuffer) {
  U32 r;
  r = (U32)pBuffer[3] & 0x000000FF;
  r <<= 8;
  r += (U32)pBuffer[2] & 0x000000FF;
  r <<= 8;
  r += (U32)pBuffer[1] & 0x000000FF;
  r <<= 8;
  r += (U32)pBuffer[0] & 0x000000FF;
  return r;
}

/*********************************************************************
*
*       FS_StoreU32LE
*
*  Function description:
*    Stores 32 bits little endian into memory.
*/
void FS_StoreU32LE(U8 *pBuffer, U32 Data) {
  *pBuffer++ = (U8)Data;
  Data >>= 8;
  *pBuffer++ = (U8)Data;
  Data >>= 8;
  *pBuffer++ = (U8)Data;
  Data >>= 8;
  *pBuffer   = (U8)Data;
}

/*********************************************************************
*
*       FS_StoreU24LE
*
*  Function description:
*    Stores 24 bits little endian into memory.
*/
void FS_StoreU24LE(U8 *pBuffer, U32 Data) {
  *pBuffer++ = (U8)Data;
  Data >>= 8;
  *pBuffer++ = (U8)Data;
  Data >>= 8;
  *pBuffer = (U8)Data;
}

/*********************************************************************
*
*       FS_StoreU16LE
*
*  Function description:
*    Writes 16 bit little endian.
*/
void FS_StoreU16LE(U8 *pBuffer, unsigned Data) {
  *pBuffer++ = (U8)Data;
  Data >>= 8;
  *pBuffer = (U8)Data;
}

/*********************************************************************
*
*       FS_LoadU16LE
*
*  Function description:
*    Reads a 16 bit little endian from a char array.
*
*  Parameters:
*    pBuffer     - Pointer to a char array.
*
*  Return value:
*    The value as U16 data type
*
*  Notes
*    (1) This cast should not be necessary, but on some compilers (NC30)
*        it is required in higher opt. levels since otherwise the
*        argument promotion to integer size is skipped, leading to wrong result of 0.
*
*/
U16 FS_LoadU16LE(const U8 *pBuffer) {
  U16 r;
  r = (U16)(*pBuffer | ((unsigned)*(pBuffer + 1) << 8));
  return r;
}

/*********************************************************************
*
*       FS_STORAGE_Init
*
*  Function description:
*    This function only initializes the driver and OS if necessary.
*    It stores then the information of the drivers in FS__aVolume.
*    This allows to use the file system as a pure sector read/write
*    software. This can be useful when using the file system as
*    USB mass storage client driver.
*
*  Return value:
*    The return value is used to tell the high level init how many
*    drivers can be used at the same time. The function will accordingly
*    allocate the sectors buffers that are necessary for a FS operation.
*
*/
unsigned FS_STORAGE_Init(void) {
  unsigned NumDriverLocks;
  unsigned NumLocks;

  NumDriverLocks = 0;
  if (FS_Global.IsInited == 0) {
    //
    // Setup the default value for max sector size
    //
    FS_Global.MaxSectorSize = 512;
    //
    //  Add all drivers that should be used
    //
    FS_X_AddDevices();
    //
    //  Calc the number of locks that are needed.
    //
    NumDriverLocks = FS_OS_GETNUM_DRIVERLOCKS();
    NumLocks       = FS_OS_GETNUM_SYSLOCKS() + NumDriverLocks;
    //
    //  Tell OS layer how many locks are necessary
    //
    FS_OS_INIT(NumLocks);
    FS_USE_PARA(NumLocks);
#if (FS_OS_LOCK_PER_DRIVER == 0)
    NumDriverLocks++;
#endif
    FS_Global.IsInited |= (1 << 0);  // Set InitStatus to FS-Storage init state.
  }
  return NumDriverLocks;
}

/*********************************************************************
*
*       FS_SetMaxSectorSize
*
*  Function description:
*    Sets the max sector size that can be used.
*
*/
void FS_SetMaxSectorSize(unsigned MaxSectorSize) {
  if (FS_Global.IsInited != 3) {
    if (MaxSectorSize & 0xFE00)
    // ToDO: Check MaxSectorSize for valid value ( 512, 1024, 2048 ...)
    FS_Global.MaxSectorSize = MaxSectorSize;
  } else {
    FS_DEBUG_WARN((FS_MTYPE_API, "FS_SetMaxSectorSize() can only be called before FS_Init() or in FS_X_AddDevices()."));
  }
}

#if FS_SUPPORT_DEINIT


static void _RemoveFileHandles(void) {
  FS_FILE * pFile;

  pFile = FS_Global.pFirstFilehandle;
  while (pFile) {
    FS_FILE * pFileNext;

    pFileNext = pFile->pNext;
    FS_Free(pFile);
    pFile = pFileNext;
  }
}

static void _RemoveFileObjects(void) {
  FS_FILE_OBJ * pFileObj;

  pFileObj = FS_Global.pFirstFileObj;
  while (pFileObj) {
    FS_FILE_OBJ * pFileObjNext;

    pFileObjNext = pFileObj->pNext;
    FS_Free(pFileObj);
    pFileObj = pFileObjNext;
  }

}

/*********************************************************************
*
*       FS_STORAGE_DeInit
*
*  Function description:
*    Deinitialize the storage layer.
*
*/
void FS_STORAGE_DeInit(void) {
  if (FS_Global.IsInited & 1) {
    FS__RemoveDevices();
    FS_OS_DEINIT();
    FS_Global.IsInited &= ~(1 << 0);  // Set InitStatus to FS-Storage init state.
  }
}

/*********************************************************************
*
*       FS_DeInit
*
*  Function description:
*    Deinitialize the file system.
*
*/
void FS_DeInit(void) {
  //
  // Allocate memory for sector buffers
  //
  FS_STORAGE_DeInit();
  if (FS_Global.IsInited & 2) {
    //
    // Free memory that was used by sector buffers.
    //
    FS_Free(FS_Global.paSectorBuffer->pBuffer);
    FS_Free(FS_Global.paSectorBuffer);
    FS_Global.NumSectorBuffers = 0;
    _RemoveFileObjects();
    _RemoveFileHandles();
    FS_Global.IsInited &= ~(1 << 1);  // Set InitStatus to FS-Complete init state.
  }
}

#endif



#if FS_SUPPORT_EXT_MEM_MANAGER

/*********************************************************************
*
*       _Alloc
*
*  Function description
*    As the name indicates, this function provides memory to the File system.
*
*
*/
static void * _Alloc(I32 NumBytes) {
  void * p;

  p = NULL;
  FS_DEBUG_LOG((FS_MTYPE_MEM, "Allocating %ld bytes.\n", NumBytes));
  if (_pfAlloc) {
    p = (_pfAlloc)(NumBytes);
  }
  return p;
}


/*********************************************************************
*
*       FS_Alloc
*
*  Function description
*    As the name indicates, this function provides memory to the File system.
*    In case there is no memory or the allocation routine is not available/set
*    this will cause a FS PANIC.
*
*
*/
void * FS_Alloc(I32 NumBytes) {
  void * p;

  p = _Alloc(NumBytes);
  if (p == NULL) {
    FS_X_Panic(FS_ERROR_ALLOC);
  }
  return p;
}
#else

/*********************************************************************
*
*       _Alloc
*
*  Function description
*    Semi-dynamic memory allocation.
*    This function is called during FS_Init() to allocate memory required
*    for the different components of the file system.
*    Since in a typical embedded application this process is not reversed,
*    there is no counterpart such as "Free()", which helps us to keep the
*    allocation function very simple and associated memory overhead small.
*/
static void * _Alloc(U32 NumBytes) {
  void * p;

  FS_DEBUG_LOG((FS_MTYPE_MEM, "Allocating %ld bytes.\n", NumBytes));
  if (_pMem == NULL) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "No memory assigned yet, FS_AssignMemory() must be called before!\n"));
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA
    FS_X_Panic(FS_ERROR_ALLOC);
#endif
  }
  NumBytes = (NumBytes + 3) & ~3UL;     // Round upwards to a multiple of 4 (memory is managed in 32-bit units)
  if (NumBytes + FS_NumBytesAllocated > _NumBytesAvailable) {
    return NULL;                      // Out of memory. Fatal error caught in caller.
  }
  p                     = ((U8 *)(_pMem) + FS_NumBytesAllocated);
  FS_NumBytesAllocated += NumBytes;
  _NumAllocs++;
  return p;
}

/*********************************************************************
*
*       FS_Alloc
*
*  Function description
*    As the name indicates, this function provides memory to the File system.
*
*  Notes
*    (1)  Fragmentation
*         The file system allocates memory only in the configuration phase, not during
*         normal operation, so that fragmentation should not occur.
*    (2)  Failure
*         Since the memory is required for proper operation of the file system,
*         this function does not return on failure.
*         In case of a configuration problem where insufficient memory is available
*         to the application, this is normally detected by the programmer in the debug phase.
*
*/
void * FS_Alloc(I32 NumBytes) {
  void * p;

  p = _Alloc(NumBytes);
  if (!p) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Could not allocate memory!\n"));
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA
    FS_X_Panic(FS_ERROR_ALLOC);
#endif
    while (1);                // Do NOT remove since the file system assumes allocation to work.
  }
  return p;
}
#endif

/*********************************************************************
*
*       FS_TryAlloc
*
*  Function description
*    Semi-dynamic memory allocation.
*    This function is called primarily during Init.
*    by different components of the file system.
*    Since in a typical embedded application this process is not reversed,
*    there is no counterpart such as "Free()", which helps us to keep the
*    allocation function very simple and associated memory overhead small.
*
*/
void * FS_TryAlloc(I32 NumBytesReq) {
  void * p;

  p = _Alloc(NumBytesReq);
  return p;
}


/*********************************************************************
*
*       FS_AllocZeroed
*/
void * FS_AllocZeroed(I32 NumBytes) {
  void * p;

  p = FS_Alloc(NumBytes);
  FS_MEMSET(p, 0, NumBytes);           // Note: p must be valid, no need to check.
  return p;
}

/*********************************************************************
*
*       FS_AllocZeroedPtr
*
*  Function description
*    Makes sure that zeroed memory is allocated to the specified pointer.
*    If pointer is NULL, memory is allocated and pointer is updated.
*    In either case memory is zeroed.
*
*/
void FS_AllocZeroedPtr(void ** pp, I32 NumBytes) {
  void * p;

  p = *pp;
  if (p == NULL) {
    p   = FS_Alloc(NumBytes);
    *pp = p;
  }
  FS_MEMSET(p, 0, NumBytes);    // Note: p must be valid, no need to check.
}



/*********************************************************************
*
*       FS_Free
*
*  Function description
*    Frees a memory block, that was allocated by FS_Alloc().
*
*/
void FS_Free(void * p) {
  FS_USE_PARA(p);
#if FS_SUPPORT_DEINIT
#if FS_SUPPORT_EXT_MEM_MANAGER
  if (_pfFree) {
    _pfFree(p);
  }
#else
  if (p) {
    p = NULL;
    if (--_NumAllocs == 0) {
      FS_NumBytesAllocated = 0;
    }
  }
#endif
#endif
}

/*********************************************************************
*
*       FS_AssignMemory
*
*  Function description
*    This function is called in the init phase.
*/
void FS_AssignMemory(U32 *pMem, U32 NumBytes) {
#if FS_SUPPORT_EXT_MEM_MANAGER == 0
  _pMem              = pMem;
  _NumBytesAvailable = NumBytes;
#else
  FS_DEBUG_WARN((FS_MTYPE_API, "FS_AssignMemory() shall only be used with the internal memory allocation routines.\n"));
#endif
}

/*********************************************************************
*
*       FS_SetMemHandler
*
*  Function description
*    This function is called in the init phase.
*    It sets the memory allocation handler.
*/
void FS_SetMemHandler(FS_PF_ALLOC * pfAlloc, FS_PF_FREE * pfFree) {
#if FS_SUPPORT_EXT_MEM_MANAGER
  if (_pfAlloc) {
    FS_DEBUG_WARN((FS_MTYPE_API, "FS_SetMemHandler: pfAlloc already set\n"));
  }
  if (_pfFree) {
    FS_DEBUG_WARN((FS_MTYPE_API, "FS_SetMemHandler: pfFree already set\n"));
  }
  _pfAlloc = pfAlloc;
  _pfFree  = pfFree;
#else
  FS_USE_PARA(pfAlloc);
  FS_USE_PARA(pfFree);
  FS_DEBUG_WARN((FS_MTYPE_API, "FS_SUPPORT_EXT_MEM_MANAGER == 0, FS_SetMemHandler() is disabled\n"));
#endif
}
/*********************************************************************
*
*       FS_GetMaxSectorSize
*
*  Function description
*    Returns the max sector size that was set by the user.
*    Default value is 512 bytes.
*/
U32 FS_GetMaxSectorSize(void) {
  return FS_Global.MaxSectorSize;
}

/*********************************************************************
*
*       FS_BITFIELD_CalcNumBitsUsed
*
*  Function description:
*    Computes the number of bits used to store the give value
*/
unsigned FS_BITFIELD_CalcNumBitsUsed(U32 NumItems) {
  unsigned r;

  r = 0;
  do {
    r++;
    NumItems >>= 1;
  } while (NumItems);
  return r;
}

/**********************************************************
*
*      FS_BITFIELD_ReadEntry
*
*  Function description:
*    Reads a single entry of <NumBits> from the bitfield
*/
U32 FS_BITFIELD_ReadEntry(const U8 * pBase, U32 Index, unsigned NumBits) {
  U32 v;
  U32 Off;
  U32 OffEnd;
  U32 Mask;
  U32 BitOff;
  int i;

  BitOff = Index * NumBits;
  Off    = BitOff >> 3;
  OffEnd = (BitOff + NumBits - 1) >> 3;
  pBase += Off;
  i = OffEnd - Off;
  //
  // Read data little endian
  //
  v = *pBase++;
  if (i) {
    unsigned Shift = 0;
    do {
      Shift += 8;
      v     |= (U32)*pBase++ << Shift;
    } while (--i);
  }
  //
  // Shift, mask & return result
  //
  v    >>= (BitOff & 7);
  Mask   = (1UL << NumBits) - 1;
  v &= Mask;
  return v;
}

/**********************************************************
*
*      FS_BITFIELD_WriteEntry
*
*  Function Description
*    Writes a single entry of <NumBits> into the bitfield
*/
void FS_BITFIELD_WriteEntry(U8 * pBase, U32 Index, unsigned NumBits, U32 v) {
  U32   Mask;
  U8  * p;
  U32   u;
  U32   BitOff;

  BitOff = Index * NumBits;
  p      = (U8 *)pBase + (BitOff >> 3);
  Mask   = (1UL << NumBits) - 1;
  Mask <<= (BitOff & 7);
  v    <<= (BitOff & 7);
  //
  // Read, mask, or and write data little endian byte by byte
  //
  do {
    u  = *p;
    u &= ~Mask;
    u |= v;
    *p = (U8)u;
    p++;
    Mask  >>= 8;
    v     >>= 8;
  } while (Mask);
}

/*********************************************************************
*
*       FS_BITFIELD_CalcSize
*
*  Function Description
*    Returns the size of bit field in bytes.
*/
unsigned FS_BITFIELD_CalcSize(U32 NumItems, unsigned BitsPerItem) {
  unsigned v;
  v =  NumItems * BitsPerItem;  // Compute the number of bits used for storage
  v = (v + 7) >> 3;             // Convert into bytes
  return v;
}

/**************************** End of file ****************************/
