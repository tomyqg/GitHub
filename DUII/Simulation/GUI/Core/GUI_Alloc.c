/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2010  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.06 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIAllocX.c
Purpose     : Dynamic memory management
----------------------------------------------------------------------
*/

#include <stddef.h>
#include <string.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Internal memory management
*
**********************************************************************
*/

#ifndef GUI_ALLOC_ALLOC

#ifndef   GUI_ALLOC_MIN_BLOCKSIZE
  #define GUI_ALLOC_MIN_BLOCKSIZE 16UL
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define NET(x)   (U32)((U32)x - (U32)sizeof(MEM_TAG) - (U32)sizeof(U32))
#define GROSS(x) (U32)((U32)x + (U32)sizeof(MEM_TAG) + (U32)sizeof(U32))

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_HEAP
*
* Purpose:
*   32 bit aligned memory block to be managed by the memory management.
*/
typedef struct {
  U8 abHeap[1];
} GUI_HEAP;

/*********************************************************************
*
*       MEM_TAG
*
* Purpose:
*   Memory information structure.
*/
typedef struct {
  U32 Block; /* Index of block which refers to this memory block */
  U32 Size;  /* Gross size of memory block */
} MEM_TAG;

/*********************************************************************
*
*       FREE_TAG
*
* Purpose:
*   Used to link free blocks of memory. Locations of these tags are the
*   unused memory areas of free blocks.
*/
typedef struct FREE_TAG FREE_TAG;
typedef struct TAG      TAG;

struct FREE_TAG {
  FREE_TAG * pNext; /* Pointer to next free tag */
  FREE_TAG * pPrev; /* Pointer to previous free tag */
  TAG      * pTag;  /* Pointer to memory tag */
};

/*********************************************************************
*
*       TAG
*/
struct TAG {
  MEM_TAG  MemTag;
  FREE_TAG FreeTag;
};

/*********************************************************************
*
*       BLOCK
*/
typedef struct {
  #if GUI_DEBUG_LEVEL > 0
    U8 InUse;
  #endif
  union {
    GUI_HMEM NextBlock; /* If block is free, handle of next block */
    TAG *    pTag;      /* If block is in use, pointer to memory tag */
  } u;
} BLOCK;

/*********************************************************************
*
*       FREE_MEM
*/
typedef struct {
  U32        MinSize;       /* Minimum size of memory blocks linked in the list */
  FREE_TAG * pFirstFreeTag; /* Pointer to first tag */
} FREE_MEM;

/*********************************************************************
*
*       CONFIG
*/
typedef struct {
  void * pData;
  void * pBlocks;
  U32    NumBytes;
  U32    NumBlocks;
  U32    BlockSize;
  U8     Customized;
} CONFIG;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static int _LockCnt;       /* If >0 moving of memory is not allowed */
/*static*/ int _LockCntHandles;/* If >0 a pointer has been locked and moving memory is not allowed */
static U8  _IsInitialized; /* 1 if already initialized */
/*static*/ U32 _NumFreeBytes;  /* Number of free bytes */
static U32 _NumFreeBlocks; /* Number of free blocks */
static U32 _AllocSize;     /* Number of bytes available for memory management */
static U32 _NumBlocks;     /* Number of available memory blocks */
static U32 _NumFixedBytes; /* Number of bytes in fixed blocks */

static int  _MaxRequestPercent = 90;

static GUI_HEAP * _pHeap;
static BLOCK    * _pBlocks;

static GUI_HMEM _FirstFreeBlock;        /* Index of first free block */

static FREE_MEM _aFreeMem[16];          /* List of free memory */

static FREE_TAG * _pFirstFixedFreeTag;  /* Linked list of free fixed memory blocks */

#ifndef   __NO_INIT
  #define __NO_INIT
#endif

static CONFIG _Config = {
  NULL,
  NULL,
  0,
  0,
  32,
  0
};

/*********************************************************************
*
*       Static code, internal
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetMinIndex
*
* Purpose:
*   Calculates the index of the free memory list to be used to store
*   memory of the given size.
*
* Return value:
*   Index of the free memory list
*/
static int _GetMinIndex(U32 Size) {
  int Index;
  for (Index = GUI_COUNTOF(_aFreeMem) - 1; Index >= 0; Index--) {
    if (Size >= _aFreeMem[Index].MinSize) {
      break;
    }
  }
  return Index;
}

/*********************************************************************
*
*       _GetFreeTag
*
* Purpose:
*   Returns a free memory tag of the requested size or greater
*/
static TAG * _GetFreeTag(U32 Size) {
  TAG * pTag = NULL;
  int Index;
  //
  // Iterate over free memory list
  //
  Index = _GetMinIndex(Size);
  for (; Index < GUI_COUNTOF(_aFreeMem); Index++) {
    //
    // Check if one or more entry exist
    //
    if (_aFreeMem[Index].pFirstFreeTag) {
      //
      // Set tag pointer to first entry
      //
      pTag = _aFreeMem[Index].pFirstFreeTag->pTag;
      //
      // Check if the requested size is larger than the size of the block
      //
      if (Size > pTag->MemTag.Size) {
        //
        // Iterate over list until size does match
        //
        while (Size > pTag->MemTag.Size) {
          if (pTag->FreeTag.pNext) {
            pTag = pTag->FreeTag.pNext->pTag;
          } else {
            if (Size > pTag->MemTag.Size) {
              pTag = NULL;
            }
            break; /* No further pointer */
          }
        }
      } else {
        break; /* Ok, we found one */
      }
    } else {
      //
      // No entry, no tag
      //
      pTag = NULL;
    }
  }
  return pTag;
}

/*********************************************************************
*
*       _WriteTagData
*
* Purpose:
*   Routine to write the tag data: memory tag, free tag and 
*   additional size information at end of block.
*/
static void _WriteTagData(TAG * pTag, GUI_HMEM Block, U32 Size, FREE_TAG * pNext) {
  //
  // Memory tag info
  //
  pTag->MemTag.Block  = Block;
  pTag->MemTag.Size   = Size;
  //
  // Free tag info
  //
  pTag->FreeTag.pNext = pNext;
  pTag->FreeTag.pTag  = pTag;
  //
  // Additional size information at end of block
  //
  *((U32 *)((U32)pTag + Size - sizeof(U32))) = Size;
}

/*********************************************************************
*
*       _Unlink
*
* Purpose:
*   Unkinks a free memory list entry from the list
*/
static void _Unlink(FREE_TAG * pFreeTag, FREE_TAG ** ppFirstFreeTag) {
  FREE_TAG * pNext;
  FREE_TAG * pPrev;
  pNext = pFreeTag->pNext;
  pPrev = pFreeTag->pPrev;
  if (pNext) {
    pNext->pPrev = pPrev;
  }
  if (pPrev) {
    pPrev->pNext = pNext;
  } else {
    *(ppFirstFreeTag) = pNext;
  }
}

/*********************************************************************
*
*       _Append
*
* Purpose:
*   Appends a free memory list entry to the list
*/
static void _Append(FREE_TAG * pFreeTag, FREE_TAG ** ppFirstFreeTag) {
  FREE_TAG * pFirstFreeTag;
  pFirstFreeTag = *ppFirstFreeTag;
  if (pFirstFreeTag) {
    pFirstFreeTag->pPrev = pFreeTag;
  }
  pFreeTag->pNext     = pFirstFreeTag;
  pFreeTag->pPrev     = NULL;
  *(ppFirstFreeTag)   = pFreeTag;
}

/*********************************************************************
*
*       _RemoveFreeMemory
*
* Purpose:
*   Removes the given memory block from the according list and subtracts
*   the block size from the free bytes.
*/
static void _RemoveFreeMemory(TAG * pTag) {
  int Index;

  //
  // Get index of free memory list
  //
  Index = _GetMinIndex(pTag->MemTag.Size);
  //
  // Subtract free bytes
  //
  _NumFreeBytes -= pTag->MemTag.Size;
  //
  // Unling from list
  //
  _Unlink(&pTag->FreeTag, &_aFreeMem[Index].pFirstFreeTag);
}

/*********************************************************************
*
*       _AddFreeMemory
*
* Purpose:
*   Adds the given memory block to the free memory and adds the block size
*   to the free bytes. If adjacent blocks at top or bottom are also free,
*   the routine will merge them together with the given block.
*/
static void _AddFreeMemory(U32 Size, void * p) {
  int Index;
  TAG * pTag;
  U32 AddrMem;
  U32 AddrEnd;
  
  //
  // End address of heap
  //
  AddrEnd = (U32)_pHeap->abHeap + _AllocSize - 1;
  //
  // Check if next adjacent block can be merged
  //
  AddrMem = (U32)p + Size;
  if (AddrMem <= AddrEnd) {
    pTag = (TAG *)AddrMem;
    //
    // If block member of memory tag is 0, it is empty and should be merged
    //
    if (pTag->MemTag.Block == 0) {
      //
      // Add size of merged block
      //
      Size += pTag->MemTag.Size;
      //
      // Remove merged block
      //
      _RemoveFreeMemory(pTag);
    }
  }
  //
  // Check if previous adjacent block can be merged
  //
  if ((U8 *)p > _pHeap->abHeap) {
    U32 SizePrev;
    SizePrev = *(U32 *)((U32)p - sizeof(U32));
    AddrMem = (U32)p - SizePrev;
    pTag = (TAG *)AddrMem;
    //
    // If block member of memory tag is 0, it is empty and should be merged
    //
    if (pTag->MemTag.Block == 0) {
      //
      // Add size of merged block
      //
      Size += pTag->MemTag.Size;
      //
      // Remove merged block
      //
      _RemoveFreeMemory(pTag);
      //
      // Adjust pointer
      //
      p = pTag;
    }
  }
  //
  // Get the index of the free memory list
  //
  Index = _GetMinIndex(Size);
  //
  // Make tag pointer
  //
  pTag = (TAG *)p;
  //
  // Write memory tag information
  //
  _WriteTagData(pTag, 0, Size, _aFreeMem[Index].pFirstFreeTag);
  //
  // Add to free memory list
  //
  _Append(&pTag->FreeTag, &_aFreeMem[Index].pFirstFreeTag);
  //
  // Adjust free bytes
  //
  _NumFreeBytes += Size;
}

/*********************************************************************
*
*       _RemoveHoles
*
* Purpose:
*   Removes all holes from memory and resets the list of free memory
*
* Return value:
*   1 if no hole detected
*/
static int _RemoveHoles(void) {
  TAG * pTag;
  U32 Addr;
  U32 Size;
  U32 AddrCur;
  U32 AddrEnd;
  U32 FreeBytes;
  int NoHoleDetected;
  int i;
  
  NoHoleDetected = 1;
  FreeBytes      = 0;
  AddrCur        = (U32)_pHeap->abHeap;
  AddrEnd        = AddrCur + _AllocSize - 1;
  //
  // Iterate over whole heap
  //
  do {
    U32 SizeCur;
    pTag    = (TAG *)AddrCur;
    SizeCur = pTag->MemTag.Size;
    //
    // Check if we have a block at the current address
    //
    if ((pTag->MemTag.Block == 0) && ((AddrCur + SizeCur) < AddrEnd)) {
      U32 SizeBlock;
      U32 AddrBlock;
      NoHoleDetected = 0;
      AddrBlock = AddrCur + SizeCur;
      //
      // Move all blocks above the hole
      //
      do {
        U32 AddrNew;
        GUI_HMEM hBlock;
        pTag                 = (TAG *)AddrBlock;
        hBlock               = pTag->MemTag.Block;
        SizeBlock            = pTag->MemTag.Size;
        AddrNew              = AddrBlock - SizeCur;
        (_pBlocks + hBlock)->u.pTag = (TAG *)AddrNew;
        memmove((void *)(AddrNew), (void *)(AddrBlock), SizeBlock);
        AddrBlock += SizeBlock;
      } while (AddrBlock < AddrEnd);
      FreeBytes += SizeCur;
      AddrEnd   -= SizeCur;
    } else {
      AddrCur += SizeCur;
    }
  } while(AddrCur < AddrEnd);
  //
  // Continue only if at least one hole has been found
  //
  if (NoHoleDetected == 0) {
    //
    // Check if last block is free memory. If yes add size to free bytes
    //
    Size = *(U32 *)(AddrEnd + 1 - sizeof(U32));
    Addr = AddrEnd + 1 - Size;
    pTag = (TAG *)Addr;
    if (pTag->MemTag.Block == 0) {
      FreeBytes += pTag->MemTag.Size;
    }
    //
    // Clear free memory list
    //
    for (i = 0; i < GUI_COUNTOF(_aFreeMem); i++) {
      _aFreeMem[i].pFirstFreeTag = NULL;
    }
    //
    // Set free bytes to 0
    //
    _NumFreeBytes = 0;
    //
    // Add the one and only free memory block
    //
    _AddFreeMemory(FreeBytes, (void *)((U32)_pHeap->abHeap + _AllocSize - FreeBytes));
  }
  return NoHoleDetected;
}

/*********************************************************************
*
*       _TestRAM
*
* Purpose:
*   Checks if the RAM can be accessed 32, 16 and 8 bit wise.
*
* Return value:
*   0 on success, 1 on error
*   1 on  8 bit access error
*   2 on 16 bit access error
*   3 on 32 bit access error
*/
#if (GUI_DEBUG_LEVEL > 0)
static int _TestRAM(U32 Addr, U32 NumBytes) {
  U8 Byte;
  U8 Short;
  U8 Long;
  U8  * pByte;
  U16 * pShort;
  U32 * pLong;
  U32 NumItems;
  U32 i, j;

  //
  // Test 32 bit access
  //
  NumItems = NumBytes / 4;
  for (j = 0; j < 2; j++) {
    pLong = (U32 *)Addr;
    for (i = 0; i < NumItems; i++) {
      Long = (i + j) % 255;
      *pLong++ = Long;
    }
    pLong = (U32 *)Addr;
    for (i = 0; i < NumItems; i++) {
      Long = *pLong++;
      if (Long != ((i + j) % 255)) {
        return 3; /* 32 bit access error */
      }
    }
  }
  //
  // Test 16 bit access
  //
  NumItems = NumBytes / 2;
  for (j = 0; j < 2; j++) {
    pShort = (U16 *)Addr;
    for (i = 0; i < NumItems; i++) {
      Short = (i + j) % 255;
      *pShort++ = Short;
    }
    pShort = (U16 *)Addr;
    for (i = 0; i < NumItems; i++) {
      Short = *pShort++;
      if (Short != ((i + j) % 255)) {
        return 2; /* 16 bit access error */
      }
    }
  }
  //
  // Test 8 bit access
  //
  NumItems = NumBytes;
  for (j = 0; j < 2; j++) {
    pByte = (U8 *)Addr;
    for (i = 0; i < NumItems; i++) {
      Byte = (i + j) % 255;
      *pByte++ = Byte;
    }
    pByte = (U8 *)Addr;
    for (i = 0; i < NumItems; i++) {
      Byte = *pByte++;
      if (Byte != ((i + j) % 255)) {
        return 1; /* 8 bit access error */
      }
    }
  }
  //
  // Clear test area
  //
  NumItems = NumBytes;
  pByte = (U8 *)Addr;
  for (i = 0; i < NumItems; i++) {
    *pByte++ = 0;
  }
  return 0;
}
#endif

/*********************************************************************
*
*       _AssignMemory
*/
static int _AssignMemory(void) {
  if (_Config.pData == NULL) {
    while (1); /* No data available: Stop execution */
  }
  if (_Config.Customized == 0) {
    //
    // Default, use static arrays
    //
    GUI_DEBUG_ERROROUT("No memory available");
  } else {
    //
    // Customized, use given memory block only
    //
    _NumBlocks = _Config.NumBytes / (_Config.BlockSize + sizeof(BLOCK)) + 2;
    _AllocSize = _Config.NumBytes - _NumBlocks * sizeof(BLOCK);
    _pBlocks   = (BLOCK *)_Config.pData;
    _pHeap     = (GUI_HEAP *)((U8 *)_Config.pData + _NumBlocks * sizeof(BLOCK));
    //_pHeap     = (GUI_HEAP *)_Config.pData;
    //_pBlocks   = (BLOCK *)((U8 *)_Config.pData + _AllocSize);
    #if (GUI_DEBUG_LEVEL > 0)
      if (_TestRAM((U32)_Config.pData, (_Config.NumBytes > 0x2800) ? 0x2800 : _Config.NumBytes)) {
        GUI_DEBUG_ERROROUT("Memory test failed");
      }
    #endif
  }
  return 0;
}

/*********************************************************************
*
*       _InitOnce
*
* Purpose:
*   Initialization of memory management. Empty blocks will be joined
*   together, the minimum size of the list of free blocks will be
*   initialized and the one and only free memory block will be added.
*/
static void _InitOnce(void) {
  unsigned i;
  //
  // Return if already initialized
  //
  if (_IsInitialized) {
    return;
  }
  //
  // Assign memory
  //
  if (_AssignMemory()) {
    return;
  }
  _IsInitialized = 1;
  //
  // Create linked list of empty blocks
  //
  for (i = 0; i < _NumBlocks - 1; i++) {
    (_pBlocks + i)->u.NextBlock = i + 1;
  }
  _FirstFreeBlock = 1;
  _NumFreeBlocks  = _NumBlocks - 1;
  //
  // Initialize list of free memory free blocks
  //
  for (i = 0; i < GUI_COUNTOF(_aFreeMem); i++) {
    _aFreeMem[i].MinSize = GUI_ALLOC_MIN_BLOCKSIZE << i;
  }
  //
  // Add the one and only initial memory block
  //
  _AddFreeMemory(_AllocSize, _pHeap->abHeap);
}

/*********************************************************************
*
*       _PopFreeBlock
*
* Purpose:
*   Returns a free block index and removes it from the list of free blocks.
*/
static GUI_HMEM _PopFreeBlock(void) {
  GUI_HMEM hMem;
  //
  // Return if no free block is available
  //
  if (_NumFreeBlocks == 0) {
    return 0;
  }
  _NumFreeBlocks--;
  hMem = _FirstFreeBlock;
  _FirstFreeBlock = (_pBlocks + _FirstFreeBlock)->u.NextBlock;
  #if GUI_DEBUG_LEVEL > 0
    (_pBlocks + hMem)->InUse = 1;
  #endif
  return hMem;
}

/*********************************************************************
*
*       _PushFreeBlock
*
* Purpose:
*   Adds a free block index to the list of free blocks.
*/
static void _PushFreeBlock(GUI_HMEM hMem) {
  _NumFreeBlocks++;
  (_pBlocks + hMem)->u.NextBlock = _FirstFreeBlock;
  #if GUI_DEBUG_LEVEL > 0
    (_pBlocks + hMem)->InUse = 0;
  #endif
  _FirstFreeBlock = hMem;
}

/*********************************************************************
*
*       _SizeToLegalSize
*
* Purpose:
*   Makes sure, that the size is not smaller than the minimum block size.
*/
static U32 _SizeToLegalSize(U32 Size) {
  Size = (Size < _aFreeMem[0].MinSize) ? _aFreeMem[0].MinSize : (Size + ((1 << 2) - 1)) & (U32)~((1 << 2) - 1);
  return GROSS(Size);
}

/*********************************************************************
*
*       Static code, used by public code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetSize
*
* Purpose:
*   Returns the number of available bytes of the given block.
*/
static GUI_ALLOC_DATATYPE _GetSize(GUI_HMEM  hMem) {
  TAG * pTag;
  pTag = (_pBlocks + hMem)->u.pTag;
  return NET(pTag->MemTag.Size);
}

/*********************************************************************
*
*       _Alloc
*
* Purpose:
*   Allocates a new memory block of the requested size. The minimum size 
*   of a free block is the smallest legal size of a free block. If the 
*   requested size is less it will be enlarged to the minimum legal size.
*
* Return value:
*   Handle of the memory block, 0 if the requested number of bytes are not available
*/
static GUI_HMEM _Alloc(GUI_ALLOC_DATATYPE Size) {
  GUI_HMEM hMem;
  GUI_ALLOC_DATATYPE OldSize;
  TAG * pTag;
  _InitOnce();

  if (_LockCntHandles > 0) {
    //
    // "Locked handles exist" means, that pointers are currently in use. In case of
    // allocating memory it can occur, that memory blocks are moved during allocation
    // and these pointers become invalid. Moving memory blocks is only required if the
    // the system is short on memory.
    //
    GUI_DEBUG_ERROROUT("Locked handles exist. More details can be found in GUI_Alloc.c");
  }
  //
  // Adjust size
  //
  Size = _SizeToLegalSize(Size);
  //
  // Get pointer to free memory block
  //
  do {
    //
    // Get free memory tag
    //
    pTag = _GetFreeTag(Size);
    if (pTag == NULL) {
      //
      // Check if the memory is locked
      //
      if (_LockCnt || _LockCntHandles) {
        //
        // If memory or pointer are locked, return
        //
        return 0;
      } else {
        //
        // If there is no memory block available remove the holes
        //
        if (_RemoveHoles()) {
          //
          // If there is no hole of the requested size, return
          //
          return 0;
        }
      }
    }
  } while (pTag == NULL);
  //
  // Get block index to be returned
  //
  hMem = _PopFreeBlock();
  //
  // Save old size of memory block
  //
  OldSize = pTag->MemTag.Size;
  //
  // Make sure rest of block is not too small
  //
  if ((U32)(OldSize - Size) < GROSS(_aFreeMem[0].MinSize)) {
    Size = OldSize;
  }
  //
  // Remove block from free memory
  //
  _RemoveFreeMemory(pTag);
  //
  // Set memory pointer
  //
  (_pBlocks + hMem)->u.pTag = pTag;
  //
  // Write tag information
  //
  _WriteTagData(pTag, hMem, Size, NULL);
  //
  // Add rest of block to empty memory
  //
  if (OldSize > Size) {
    _AddFreeMemory(OldSize - Size, (void *)((U32)pTag + Size));
  }

  return hMem;
}

/*********************************************************************
*
*       _Free
*
* Purpose:
*   Adds the given memory block to the free memory.
*/
static void _Free(GUI_HMEM hMem) {
  TAG * pTag;
  #if GUI_DEBUG_LEVEL > 0
  if ((_pBlocks + hMem)->InUse == 0) {
    GUI_DEBUG_ERROROUT("Block already free");
    return;
  }
  #endif
  pTag = (_pBlocks + hMem)->u.pTag;
  #ifdef WIN32
    GUI_MEMSET((U8 *)&pTag->FreeTag, 0xcc, NET(pTag->MemTag.Size));
  #endif
  //
  // Add block to free memory
  //
  _AddFreeMemory(pTag->MemTag.Size, pTag);
  //
  // Put block into list of free blocks
  //
  _PushFreeBlock(hMem);
}

/*********************************************************************
*
*       _GetFixedBlock
*/
static void * _GetFixedBlock(GUI_ALLOC_DATATYPE Size) {
  FREE_TAG * pFree;

  pFree = _pFirstFixedFreeTag;
  while (pFree) {
    if (*((I32 *)pFree - 1) == Size) {
      return pFree;
    }
    pFree = pFree->pNext;
  }
  return NULL;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_ALLOC_p2h
*/
GUI_HMEM GUI_ALLOC_p2h(void * p) {
  GUI_HMEM hMem;
  MEM_TAG * pTag;
  
  pTag = (MEM_TAG *)p - 1;
  hMem = pTag->Block;
  return hMem;
}

/*********************************************************************
*
*       GUI_ALLOC_FreeFixedBlock
*/
void GUI_ALLOC_FreeFixedBlock(void * pBlock) {
  FREE_TAG * pFree;
  
  pFree = (FREE_TAG *)pBlock;
  pFree->pNext = _pFirstFixedFreeTag;
  if (pFree->pNext) {
    pFree->pNext->pPrev = pFree;
  }
  _pFirstFixedFreeTag = (FREE_TAG *)pBlock;
}

/*********************************************************************
*
*       GUI_ALLOC_GetFixedBlock
*/
void * GUI_ALLOC_GetFixedBlock(GUI_ALLOC_DATATYPE Size) {
  void * p;
  TAG * pTag;
  FREE_TAG * pFree;

  if (_LockCntHandles > 0) {
    //
    // "Locked handles exist" means, that pointers are currently in use. In case of
    // allocating memory it can occur, that memory blocks are moved during allocation
    // and these pointers become invalid. Moving memory blocks is only required if the
    // the system is short on memory.
    //
    GUI_DEBUG_ERROROUT("Locked handles exist. More details can be found in GUI_Alloc.c");
  }
  _InitOnce();
  Size = (((Size + 3) >> 2) << 2) /* 4 byte alignment */
         + sizeof(U32);           /* Additional size entry at bottom of block */
  if ((U32)Size < _aFreeMem[0].MinSize) {
    Size = _aFreeMem[0].MinSize;
  }
  //
  // Try to get an already existing block
  //
  p = _GetFixedBlock(Size);
  if (p) {
    //
    // Unlink free block from linked list
    //
    pFree = (FREE_TAG *)p;
    if (pFree->pNext) {
      pFree->pNext->pPrev = pFree->pPrev;
    }
    if (pFree->pPrev) {
      pFree->pPrev->pNext = pFree->pNext;
    }
    if (_pFirstFixedFreeTag == pFree) {
      _pFirstFixedFreeTag = pFree->pNext;
    }
  } else {
    //
    // Remove holes to make sure that there is only one free memory block
    //
    _RemoveHoles();
    //
    // Get tag of the one and only free memory block
    //
    pTag = _GetFreeTag(Size);
    //
    // Check available number of bytes
    //
    if (pTag) {
      if (Size > (GUI_ALLOC_DATATYPE)pTag->MemTag.Size) {
        p = NULL; /* Not enough memory available */
      } else {
        //
        // Create pointer. Please note that at this point the whole 
        // existing free memory is referenced by pTag.
        //
        p = (void *)((U8 *)pTag                     /* First byte of the free memory block */
                           + pTag->MemTag.Size      /* Number of free bytes */
                           - (Size - sizeof(U32))); /* Number of requested bytes */
        //
        // Remove complete free memory
        //
        _RemoveFreeMemory(pTag);
        //
        // Adjust sizes
        //
        _AllocSize     -= Size; /* Bytes are no longer available for dynamic memory access */
        _NumFixedBytes += Size; /* Add them to the number of fixed bytes */
        //
        // Add remaining bytes to the free memory
        //
        _AddFreeMemory(pTag->MemTag.Size - Size, pTag);
        //
        // Write block size below block
        //
        *((U32 *)p - 1) = Size;
      }
    }
  }
  return p;
}

/*********************************************************************
*
*       GUI_ALLOC_AllocNoInit
*/
GUI_HMEM GUI_ALLOC_AllocNoInit(GUI_ALLOC_DATATYPE Size) {
  GUI_HMEM hMem;
  if (Size == 0) {
    return (GUI_HMEM)0;
  }
  GUI_LOCK();
  GUI_DEBUG_LOG2("\nGUI_ALLOC_AllocNoInit... requesting %d, %d avail", Size, _NumFreeBytes);
  hMem = _Alloc(Size);
  GUI_DEBUG_LOG1("\nGUI_ALLOC_AllocNoInit : Handle", hMem);
  GUI_UNLOCK();
  return hMem;
}

/*********************************************************************
*
*       GUI_ALLOC_h2p
*/
void * GUI_ALLOC_h2p(GUI_HMEM  hMem) {
  GUI_ASSERT_LOCK();
  #if GUI_DEBUG_LEVEL > 0
    if (!hMem) {
      GUI_DEBUG_ERROROUT("\n"__FILE__ " GUI_ALLOC_h2p: illegal argument (0 handle)");
      return 0;
    }
    if ((_pBlocks + hMem)->InUse == 0) {
      GUI_DEBUG_ERROROUT("Dereferencing free block");
    }
  #endif
  return (void *)&(_pBlocks + hMem)->u.pTag->FreeTag;
}

/*********************************************************************
*
*       GUI_ALLOC_LockH
*/
void * GUI_ALLOC_LockH(GUI_HMEM hMem) {
  _LockCntHandles++;
  return GUI_ALLOC_h2p(hMem);
}

/*********************************************************************
*
*       GUI_ALLOC_UnlockH
*/
void * GUI_ALLOC_UnlockH(void) {
  if (_LockCntHandles == 0) {
    GUI_DEBUG_ERROROUT(__FILE__": Underflow of locked handles");
  } else {
    _LockCntHandles--;
  }
  return NULL;
}

/*********************************************************************
*
*       GUI_ALLOC_GetNumFreeBytes
*/
GUI_ALLOC_DATATYPE GUI_ALLOC_GetNumFreeBytes(void) {
  _InitOnce();
  return NET(_NumFreeBytes);
}

/*********************************************************************
*
*       GUI_ALLOC_GetNumFreeBlocks
*/
GUI_ALLOC_DATATYPE GUI_ALLOC_GetNumFreeBlocks(void) {
  _InitOnce();
  return _NumFreeBlocks;
}

/*********************************************************************
*
*       GUI_ALLOC_GetNumUsedBytes
*/
GUI_ALLOC_DATATYPE GUI_ALLOC_GetNumUsedBytes(void) {
  _InitOnce();
  return _AllocSize - _NumFreeBytes;
}

/*********************************************************************
*
*       GUI_ALLOC_GetNumUsedBlocks
*/
GUI_ALLOC_DATATYPE GUI_ALLOC_GetNumUsedBlocks(void) {
  _InitOnce();
  return _NumBlocks - _NumFreeBlocks;
}

/*********************************************************************
*
*       GUI_ALLOC_GetMaxSize
*
* Purpose:
*   Returns the biggest available blocksize (without relocation).
*/
GUI_ALLOC_DATATYPE GUI_ALLOC_GetMaxSize(void) {
  GUI_ALLOC_DATATYPE MaxSize = 0;
  int Index;
  TAG * pTag;
  _InitOnce();
  for (Index = GUI_COUNTOF(_aFreeMem) - 1; Index >= 0; Index--) {
    if (_aFreeMem[Index].pFirstFreeTag) {
      pTag = _aFreeMem[Index].pFirstFreeTag->pTag;
      break;
    } else {
      pTag = NULL;
    }
  }
  while (pTag) {
    if ((U32)MaxSize < pTag->MemTag.Size) {
      MaxSize = pTag->MemTag.Size;
    }
    pTag = pTag->FreeTag.pNext ? pTag->FreeTag.pNext->pTag : NULL;
  }
  return NET(MaxSize);
}

/*********************************************************************
*
*       GUI_ALLOC_RequestSize
*/
GUI_ALLOC_DATATYPE GUI_ALLOC_RequestSize(void) {
  GUI_ALLOC_DATATYPE MaxSize;
  GUI_LOCK();
  MaxSize = GUI_ALLOC_GetMaxSize();
  if ((U32)MaxSize > 0x400000L) {
    MaxSize = (MaxSize / 100) * _MaxRequestPercent;
  } else {
    MaxSize = ((U32)MaxSize * _MaxRequestPercent) / 100;
  }
  GUI_UNLOCK();
  return MaxSize;
}

/*********************************************************************
*
*       GUI_ALLOC_Lock
*
* Purpose:
*   The function increments the lock counter. If the lock counter is >0
*   the internal memory management should not shift allocated memory.
*/
void GUI_ALLOC_Lock(void) {
  _LockCnt++;
}

/*********************************************************************
*
*       GUI_ALLOC_Unlock
*
* Purpose:
*   The function decrements the lock counter. If the lock counter is >0
*   the internal memory management should not shift allocated memory.
*/
void GUI_ALLOC_Unlock(void) {
  _LockCnt--;
}

/*********************************************************************
*
*       GUI_ALLOC_AssignMemory
*/
void GUI_ALLOC_AssignMemory(void * p, U32 NumBytes) {
  _Config.pData      = p;
  _Config.NumBytes   = NumBytes;
  _Config.Customized = 1;
}

/*********************************************************************
*
*       GUI_ALLOC_SetAvBlockSize
*/
void GUI_ALLOC_SetAvBlockSize(U32 BlockSize) {
  _Config.BlockSize  = BlockSize;
  _Config.NumBlocks  = 0;
  _Config.Customized = 1;
}

#else

/*********************************************************************
*
*       External memory management functions
*
* The functions below will generate code only if the GUI memory
* management is not used (GUI_ALLOC_ALLOC defined).
*
* Note:
* The memory block allocated is bigger than the requested one, as we
* store some add. information (size of the memory block) there.
*
**********************************************************************
*/

typedef struct {
  union {
    GUI_ALLOC_DATATYPE Size;
    int Dummy;               /* Needed to guarantee alignment on 32 / 64 bit CPUs */
  } Info;      /* Unnamed would be best, but is not supported by all compilers */
} INFO;

/*********************************************************************
*
*       _GetSize
*/
static GUI_ALLOC_DATATYPE _GetSize(GUI_HMEM  hMem) {
  INFO * pInfo;
  pInfo = (INFO *)GUI_ALLOC_H2P(hMem);
  return pInfo->Info.Size;
}

/*********************************************************************
*
*       _Free
*/
static void _Free(GUI_HMEM  hMem) {
  GUI_ALLOC_FREE(hMem);
}

/*********************************************************************
*
*       GUI_ALLOC_AllocNoInit
*/
GUI_HMEM GUI_ALLOC_AllocNoInit(GUI_ALLOC_DATATYPE Size) {
  GUI_HMEM hMem;
  if (Size == 0) {
    return (GUI_HMEM)0;
  }
  hMem= GUI_ALLOC_ALLOC(Size + sizeof(INFO));
  //
  // Init info structure
  //
  if (hMem) {
    INFO * pInfo;
    pInfo = (INFO *)GUI_ALLOC_H2P(hMem);
    pInfo->Info.Size = Size;
  }
  return hMem;
}

/*********************************************************************
*
*       GUI_ALLOC_h2p
*/
void* GUI_ALLOC_h2p(GUI_HMEM  hMem) {
  U8 * p;
  p = (U8*)GUI_ALLOC_H2P(hMem);    /* Pointer to memory block from memory manager */
  p += sizeof(INFO);               /* Convert to pointer to usable area */
  return p;
}

/*********************************************************************
*
*       GUI_ALLOC_GetMaxSize
*/
GUI_ALLOC_DATATYPE GUI_ALLOC_GetMaxSize(void) {
  return GUI_ALLOC_GETMAXSIZE();
}

/*********************************************************************
*
*       GUI_ALLOC_RequestSize
*/
GUI_ALLOC_DATATYPE GUI_ALLOC_RequestSize(void) {
  return GUI_ALLOC_GETMAXSIZE();
}

/*********************************************************************
*
*       GUI_ALLOC_Lock
*/
void GUI_ALLOC_Lock(void) {
}

/*********************************************************************
*
*       GUI_ALLOC_Unlock
*/
void GUI_ALLOC_Unlock(void) {
}

/*********************************************************************
*
*       GUI_ALLOC_LockH
*/
void * GUI_ALLOC_LockH(GUI_HMEM hMem) {
  return GUI_ALLOC_h2p(hMem);
}

/*********************************************************************
*
*       GUI_ALLOC_UnlockH
*/
void * GUI_ALLOC_UnlockH(void) {
  return NULL;
}

/*********************************************************************
*
*       GUI_ALLOC_AssignMemory
*/
void GUI_ALLOC_AssignMemory(void * p, U32 NumBytes) {
  GUI_USE_PARA(p);
  GUI_USE_PARA(NumBytes);
}

/*********************************************************************
*
*       GUI_ALLOC_SetAvBlockSize
*/
void GUI_ALLOC_SetAvBlockSize(U32 BlockSize) {
  GUI_USE_PARA(BlockSize);
}

/*********************************************************************
*
*       GUI_ALLOC_GetFixedBlock
*/
void * GUI_ALLOC_GetFixedBlock(GUI_ALLOC_DATATYPE Size) {
  void * p = NULL;
  p = GUI_ALLOC_ALLOC(Size);
  GUI_MEMSET(p, 0, Size);
  return p;
}

/*********************************************************************
*
*       GUI_ALLOC_FreeFixedBlock
*/
void GUI_ALLOC_FreeFixedBlock(void * pBlock) {
  GUI_LOCK();
  GUI_ALLOC_FREE(pBlock);
  GUI_UNLOCK();
}

#endif

/*********************************************************************
*
*       Public code, common memory management functions
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_ALLOC_GetSize
*
*  Parameter
*    hMem      Handle of the memory block. A 0-handle is permitted; return value is 0 in this case.
*/
GUI_ALLOC_DATATYPE GUI_ALLOC_GetSize(GUI_HMEM  hMem) {
  GUI_ALLOC_DATATYPE r;

  r = 0;
  if (hMem) {
    r = _GetSize(hMem);
  }
  return r;
}

/*********************************************************************
*
*       GUI_ALLOC_Free
*/
void GUI_ALLOC_Free(GUI_HMEM hMem) {
  if (hMem == GUI_HMEM_NULL) { /* Note: This is not an error, it is permitted */
    return;
  }
  GUI_LOCK();
  GUI_DEBUG_LOG1("\nGUI_ALLOC_Free(%d)", hMem);
  _Free(hMem);
  GUI_UNLOCK();
}


/*********************************************************************
*
*       GUI_ALLOC_FreePtr
*/
void GUI_ALLOC_FreePtr(GUI_HMEM *ph) {
  GUI_LOCK();
  GUI_ALLOC_Free(*ph);
  *ph =0;
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_ALLOC_FreePtrArray
*/
void GUI_ALLOC_FreePtrArray(GUI_HMEM * pArray, int NumElems) {
  int i;
  for (i = 0; i < NumElems; i++) {
    GUI_ALLOC_FreePtr(&pArray[i]);
  }
}

/*************************** End of file ****************************/
