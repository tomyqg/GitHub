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
File        : GUI_GIF.c
Purpose     : Implementation of rendering GIF images
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#include "GUI_GIF_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
/* Input buffer configuration */
#ifndef   GUI_GIF_INPUT_BUFFER_SIZE
  #define GUI_GIF_INPUT_BUFFER_SIZE 0
#endif

/* Constants for reading GIFs */
#define GIF_INTRO_TERMINATOR ';'
#define GIF_INTRO_EXTENSION  '!'
#define GIF_INTRO_IMAGE      ','

#define GIF_COMMENT     0xFE
#define GIF_APPLICATION 0xFF
#define GIF_PLAINTEXT   0x01
#define GIF_GRAPHICCTL  0xF9

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/
#if GUI_GIF_INPUT_BUFFER_SIZE
  static U8 _aInputBuffer[GUI_GIF_INPUT_BUFFER_SIZE];
#endif

static const int _aMaskTbl[16] = {
  0x0000, 0x0001, 0x0003, 0x0007,
  0x000f, 0x001f, 0x003f, 0x007f,
  0x00ff, 0x01ff, 0x03ff, 0x07ff,
  0x0fff, 0x1fff, 0x3fff, 0x7fff,
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U8   _aBuffer[256];

/*********************************************************************
*
*       Private data
*
**********************************************************************
*/
const int GUI_GIF__aInterlaceOffset[4] = {  8, 8, 4, 2 };
const int GUI_GIF__aInterlaceYPos[4]   = {  0, 4, 2, 1 };

//GUI_COLOR   GUI_GIF__aColorTable[256];
//LZW_CONTEXT GUI_GIF__LZWContext;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _ReadBytes
*
* Purpose:
*   Reads a string from the given pointer if possible and increments the pointer
*/
static void _ReadBytes(GUI_GIF_CONTEXT * pContext, U8 * pBuffer, int Len) {
  const U8 * pData;
  if (GUI_GIF__ReadData(pContext, Len, &pData, 0)) {
    return; /* Error */
  }
  GUI_MEMCPY(pBuffer, pData, Len);
}

/*********************************************************************
*
*       _GetDataBlock
*
* Purpose:
*   Reads a LZW data block. The first byte contains the length of the block,
*   so the maximum length is 256 byte
*
* Return value:
*   Length of the data block
*/
static int _GetDataBlock(GUI_GIF_CONTEXT * pContext, U8 * pBuffer) {
  U8 Count;
  const U8 * pData;
  if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
    return 0; /* Error */
  }
  Count = *pData; /* Read the length of the data block */
  if (Count) {
    if (pBuffer) {
      _ReadBytes(pContext, pBuffer, Count);
    } else {
      if (GUI_GIF__ReadData(pContext, Count, &pData, 0)) {
        return 0; /* Error */
      }
    }
  }
  return((int)Count);
}

/*********************************************************************
*
*       _GetNextCode
*
* Purpose:
*   Returns the next LZW code from the LZW stack. One LZW code contains up to 12 bits.
*
* Return value:
*   >= 0 if succeed
*   <  0 if not succeed
*/
static int _GetNextCode(GUI_GIF_CONTEXT * pContext) {
  int i, j, End;
  long Result;
  if (pContext->ReturnClear) {
    /* The first code should be a clear code. */
    pContext->ReturnClear = 0;
    return pContext->ClearCode;
  }
  End = pContext->CurBit + pContext->CodeSize;
  if (End >= pContext->LastBit) {
    int Count;
    if (pContext->GetDone) {
      return -1; /* Error */
    }
    pContext->aBuffer[0] = pContext->aBuffer[pContext->LastByte - 2];
    pContext->aBuffer[1] = pContext->aBuffer[pContext->LastByte - 1];
    if ((Count = _GetDataBlock(pContext, &pContext->aBuffer[2])) == 0) {
      pContext->GetDone = 1;
    }
    if (Count < 0) {
      return -1; /* Error */
    }
    pContext->LastByte = 2 + Count;
    pContext->CurBit   = (pContext->CurBit - pContext->LastBit) + 16;
    pContext->LastBit  = (2 + Count) * 8 ;
    End                  = pContext->CurBit + pContext->CodeSize;
  }
  j = End >> 3;
  i = pContext->CurBit >> 3;
  if (i == j) {
    Result = (long)pContext->aBuffer[i];
  } else if (i + 1 == j) {
    Result = (long)pContext->aBuffer[i] | ((long)pContext->aBuffer[i + 1] << 8);
  }  else {
    Result = (long)pContext->aBuffer[i] | ((long)pContext->aBuffer[i + 1] << 8) | ((long)pContext->aBuffer[i + 2] << 16);
  }
  Result = (Result >> (pContext->CurBit & 0x7)) & _aMaskTbl[pContext->CodeSize];
  pContext->CurBit += pContext->CodeSize;
  return (int)Result;
}

/*********************************************************************
*
*       _ReadExtension
*
* Purpose:
*   Reads an extension block. One extension block can consist of several data blocks.
*   If an unknown extension block occures, the routine failes.
*/
static int _ReadExtension(GUI_GIF_CONTEXT * pContext, int * pTransIndex, GUI_GIF_IMAGE_INFO * pInfo, U8 * pDisposal) {
  U8 Label;
  const U8 * pData;
  if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
    return 1; /* Error */
  }
  Label = *pData;
  switch (Label) {
  case GIF_PLAINTEXT:
  case GIF_APPLICATION:
  case GIF_COMMENT:
    while (_GetDataBlock(pContext, _aBuffer) > 0);
    return 0;
  case GIF_GRAPHICCTL:
    if (_GetDataBlock(pContext, _aBuffer) != 4) { /* Length of a graphic control block must be 4 */
      return 1;
    }
    if (pInfo) {
      pInfo->Delay    = (_aBuffer[2] << 8) | _aBuffer[1];
    }
    if (pDisposal) {
      *pDisposal = (_aBuffer[0] >> 2) & 0x7;
    }
    if (pTransIndex) {
      if ((_aBuffer[0] & 0x1) != 0) {
        *pTransIndex = _aBuffer[3];
      }
    }
    /* Skip block terminator, must be 0 */
    if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
      return 1; /* Error */
    }
    if (*pData) {
      return 1; /* Error */
    }
    return 0;
  }
  return 1; /* Error */
}

/*********************************************************************
*
*       _ReadComment
*
* Purpose:
*   Reads a comment from the extension block if available and returns the number
*   of comment bytes.
*/
static int _ReadComment(GUI_GIF_CONTEXT * pContext, U8 * pBuffer, int MaxSize, int * pSize) {
  U8 Label;
  int Size;
  const U8 * pData;
  if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
    return 1; /* Error */
  }
  Label = *pData;
  switch (Label) {
  case GIF_PLAINTEXT:
  case GIF_APPLICATION:
    while (_GetDataBlock(pContext, _aBuffer) > 0);
    return 0;
  case GIF_COMMENT:
    Size = _GetDataBlock(pContext, _aBuffer);
    if (Size > MaxSize) {
      Size = MaxSize;
    }
    if (pBuffer) {
      *pSize = Size;
      GUI_MEMCPY(pBuffer, _aBuffer, Size);
    }
    return 0;
  case GIF_GRAPHICCTL:
    if (_GetDataBlock(pContext, _aBuffer) != 4) { /* Length of a graphic control block must be 4 */
      return 1;
    }
    /* Skip block terminator, must be 0 */
    if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
      return 1; /* Error */
    }
    if (*pData) {
      return 1; /* Error */
    }
    return 0;
  }
  return 1; /* Error */
}

/*********************************************************************
*
*       _DrawFromDataBlock
*
* Purpose:
*   This routine draws a GIF image from the current pointer which should point to a
*   valid GIF data block. The size of the desired image is given in the image descriptor.
*
* Return value:
*   0 if succeed
*   1 if not succeed
*
* Parameters:
*   pDescriptor  - Points to a IMAGE_DESCRIPTOR structure, which contains infos about size, colors and interlacing.
*   x0, y0       - Obvious.
*   Transparency - Color index which should be treated as transparent.
*   Disposal     - Contains the disposal method of the previous image. If Disposal == 2, the transparent pixels
*                  of the image are rendered with the background color.
*/
static int _DrawFromDataBlock(GUI_GIF_CONTEXT * pContext, IMAGE_DESCRIPTOR * pDescriptor, int x0, int y0, int Transparency, int Disposal, int Num, int Denom) {
  int Codesize, Index, OldIndex, XPos, YPos, YCnt, Pass, Interlace, XEnd;
  int Width, Height, NumColors, Cnt, BkColorIndex, ColorIndex;
  LCD_LOGPALETTE LogPalette;
  const LCD_PIXELINDEX * pTrans;
  const U8 * pData;
  GUI_USE_PARA(Num);
  GUI_USE_PARA(Denom);
  x0 += pDescriptor->XPos;
  y0 += pDescriptor->YPos;
  Width     = pDescriptor->XSize;
  Height    = pDescriptor->YSize;
  NumColors = pDescriptor->NumColors;
  XEnd      = Width + x0 - 1;
  BkColorIndex = LCD_GetBkColorIndex();
  /* Get color translation table  */
  LogPalette.NumEntries  = NumColors;
  LogPalette.HasTrans    = 0;
  LogPalette.pPalEntries = pContext->aColorTable;
  if ((pTrans = LCD_GetpPalConvTable((const LCD_LOGPALETTE *)&LogPalette)) == NULL) {
    return 1; /* Error */
  }
  if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
    return 1; /* Error */
  }
  Codesize  = *pData;                    /* Read the LZW codesize */
  GUI_GIF__InitLZW(pContext, Codesize);                    /* Initialize the LZW stack with the LZW codesize */
  Interlace = pDescriptor->Flags & 0x40; /* Evaluate if image is interlaced */
  for (YCnt = 0, YPos = y0, Pass = 0; YCnt < Height; YCnt++) {
    Cnt      = 0;
    OldIndex = -1;    
    for (XPos = x0; XPos <= XEnd; XPos++) {
      if (pContext->sp > pContext->aDecompBuffer) {
        Index = *--(pContext->sp);
      } else {
        Index = GUI_GIF__GetNextByte(pContext);
      }
      if (Index == -2) {
        return 0; /* End code */
      }
      if ((Index < 0) || (Index >= NumColors)) {
        return 1; /* If Index out of legal range stop decompressing, error */
      }
      /* If current index equals old index increment counter */
      if ((Index == OldIndex) && (XPos <= XEnd)) {
        Cnt++;
      } else {
        if (Cnt) {
          if (OldIndex != Transparency) {
            LCD_SetColorIndex(*(pTrans + OldIndex));
            LCD_DrawHLine(XPos - Cnt - 1, YPos, XPos - 1);
          } else if (Disposal == 2) {
            LCD_SetColorIndex(BkColorIndex);
            LCD_DrawHLine(XPos - Cnt - 1, YPos, XPos - 1);
          }
          Cnt = 0;
        } else {
          if (OldIndex >= 0) {
            if (OldIndex != Transparency) {
              LCD_SetPixelIndex(XPos - 1, YPos, *(pTrans + OldIndex));
            } else if (Disposal == 2) {
              LCD_SetPixelIndex(XPos - 1, YPos, BkColorIndex);
            }
          }
        }
      }
      OldIndex = Index;
    }
    if ((OldIndex != Transparency) || (Disposal == 2)) {
      if (OldIndex != Transparency) {
        ColorIndex = *(pTrans + OldIndex);
      } else {
        ColorIndex = BkColorIndex;
      }
      if (Cnt) {
        LCD_SetColorIndex(ColorIndex);
        LCD_DrawHLine(XPos - Cnt - 1, YPos, XPos - 1);
      } else {
        LCD_SetPixelIndex(XEnd, YPos, ColorIndex);
      }
    }
    /* Adjust YPos if image is interlaced */
    if (Interlace) {
      YPos += GUI_GIF__aInterlaceOffset[Pass];
      if ((YPos - y0) >= Height) {
        ++Pass;
        YPos = GUI_GIF__aInterlaceYPos[Pass] + y0;
      }
    } else {
      YPos++;
    }
  }
  return 0;
}

/*********************************************************************
*
*       _ReadColorMap
*/
static int _ReadColorMap(GUI_GIF_CONTEXT * pContext, int NumColors) {
  int i;
  for (i = 0; i < NumColors; i++) {
    U8 r, g, b;
    const U8 * pData;
    if (GUI_GIF__ReadData(pContext, 3, &pData, 0)) {
      return 1; /* Error */
    }
    r = *(pData + 0);
    g = *(pData + 1);
    b = *(pData + 2);
    pContext->aColorTable[i] = r | ((U16)g << 8) | ((U32)b << 16);
  }
  return 0;
}

/*********************************************************************
*
*       _InitGIFDecoding
*
* Purpose:
*   The routine initializes the static SOURCE structure and checks
*   if the file is a legal GIF file.
*
* Return value:
*   0 on success, 1 on error
*/
static int _InitGIFDecoding(GUI_GIF_CONTEXT * pContext) {
  const U8 * pData;
  if (GUI_GIF__ReadData(pContext, 6, &pData, 1)) {
    return 1; /* Error */
  }
  /* Check if the file is a legal GIF file by checking the 6 byte file header */
  if ( (*(pData + 0) != 'G') || 
       (*(pData + 1) != 'I') || 
       (*(pData + 2) != 'F') || 
       (*(pData + 3) != '8') ||
      ((*(pData + 4) != '7') && (*(pData + 4) != '9')) ||
       (*(pData + 5) != 'a')) {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       _GetImageDimension
*
* Purpose:
*   Reads the image dimension from the logical screen descriptor
*
* Return value:
*   0 on success, 1 on error
*/
static int _GetImageDimension(GUI_GIF_CONTEXT * pContext, int * pxSize, int * pySize) {
  int XSize, YSize;
  const U8 * pData;
  /* Initialize decoding */
  if (_InitGIFDecoding(pContext)) {
    return 1; /* Error */
  }
  /* Get data */
  if (GUI_GIF__ReadData(pContext, 4, &pData, 0)) {
    return 1; /* Error */
  }
  /* Read image size */
  XSize = GUI__Read16(&pData);
  YSize = GUI__Read16(&pData);
  if ((XSize > 2000) || (YSize > 2000)) {
    return 1; /* Error if image is too large */
  }
  if (pxSize) {
    *pxSize = XSize;
  }
  if (pySize) {
    *pySize = YSize;
  }
  return 0;
}

/*********************************************************************
*
*       _GetGlobalColorTable
*
* Purpose:
*   Reads the global color table if there is one. Returns the number of
*   available colors over the pointer pNumColors (can be NULL).
*
* Return value:
*   0 on success, 1 on error
*/
static int _GetGlobalColorTable(GUI_GIF_CONTEXT * pContext, int * pNumColors) {
  U8 Flags;
  int NumColors;
  const U8 * pData;
  if (GUI_GIF__ReadData(pContext, 3, &pData, 0)) {
    return 1; /* Error */
  }
  /* Read flags from logical screen descriptor */
  Flags = *pData;
  NumColors = 2 << (Flags & 0x7);
  if (Flags & 0x80) {
    /* Read global color table */
    if (_ReadColorMap(pContext, NumColors)) {
      return 1; /* Error */
    }
  }
  if (pNumColors) {
    *pNumColors = NumColors;
  }
  return 0;
}

/*********************************************************************
*
*       _GetSizeAndColorTable
*/
static int _GetSizeAndColorTable(GUI_GIF_CONTEXT * pContext, int * pxSize, int * pySize, int * pNumColors) {
  /* Get image size */
  if (_GetImageDimension(pContext, pxSize, pySize)) {
    return 1; /* Error */
  }
  /* Get global color table (if available) */
  if (_GetGlobalColorTable(pContext, pNumColors)) {
    return 1; /* Error */
  }
  return 0;
}

/*********************************************************************
*
*       _SkipLocalColorTable
*/
static void _SkipLocalColorTable(GUI_GIF_CONTEXT * pContext) {
  U8 Flags;
  const U8 * pData;
  if (GUI_GIF__ReadData(pContext, 9, &pData, 0)) {
    return; /* Error */
  }
  Flags = *(pData + 8);           /* Skip the first 8 bytes of the image descriptor, only 'Flags' are intresting */
  if (Flags & 0x80) {
    /* Skip local color table */
    int NumBytes, RemBytes, NumColors;
    NumColors = 2 << (Flags & 0x7);
    RemBytes = NumColors * 3 + 1; /* Skip colors (Number of colors * 3) and the codesize byte */
    while (RemBytes) {
      if (RemBytes > 256) {
        NumBytes = 256;
      } else {
        NumBytes = RemBytes;
      }
      if (GUI_GIF__ReadData(pContext, NumBytes, &pData, 0)) {
        return; /* Error */
      }
      RemBytes -= NumBytes;
    }
  } else {
    /* Skip codesize */
    if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
      return; /* Error */
    }
  }
}

/*********************************************************************
*
*       _GetGIFInfo
*/
static int _GetGIFInfo(GUI_GIF_CONTEXT * pContext, GUI_GIF_INFO * pInfo) {
  U8 Introducer;
  int NumColors, ImageCnt;
  const U8 * pData;
  /* Initialize decoding and get size and global color table */
  if (_GetSizeAndColorTable(pContext, &pInfo->xSize, &pInfo->ySize, &NumColors)) {
    return 1; /* Error */
  }
  ImageCnt = 0;
  /* Iterate over the blocks */
  do {
    if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
      return 1; /* Error */
    }
    Introducer = *pData;
    switch (Introducer) {
    case GIF_INTRO_IMAGE:
      _SkipLocalColorTable(pContext);
      while (_GetDataBlock(pContext, 0) > 0); /* Skip data blocks */
      ImageCnt++;
      break;
    case GIF_INTRO_TERMINATOR:
      break;
    case GIF_INTRO_EXTENSION:
      if (_ReadExtension(pContext, NULL, NULL, NULL)) { /* Skip image extension */
        return 1;
      }
      break;
    default:
      return 1;
    }
  } while (Introducer != GIF_INTRO_TERMINATOR); /* We do not support more than one image, so stop when the first terminator has been read */
  pInfo->NumImages = ImageCnt;
  return 0;
}

/*********************************************************************
*
*       _GetImageInfo
*/
static int _GetImageInfo(GUI_GIF_CONTEXT * pContext, GUI_GIF_IMAGE_INFO * pInfo, int Index) {
  U8 Introducer;
  int NumColors, ImageCnt;
  const U8 * pData;
  /* Initialize decoding and get size and global color table */
  if (_GetSizeAndColorTable(pContext, NULL, NULL, &NumColors)) {
    return 1; /* Error */
  }
  ImageCnt = 0;
  /* Iterate over the blocks */
  do {
    if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
      return 1; /* Error */
    }
    Introducer = *pData;
    switch (Introducer) {
    case GIF_INTRO_IMAGE:
      if (Index == ImageCnt) {
        if (GUI_GIF__ReadData(pContext, 8, &pData, 0)) {
          return 1; /* Error */
        }
        pInfo->xPos  = GUI__Read16(&pData);
        pInfo->xPos  = GUI__Read16(&pData);
        pInfo->xSize = GUI__Read16(&pData);
        pInfo->ySize = GUI__Read16(&pData);
        return 0;
      }
      _SkipLocalColorTable(pContext);
      while (_GetDataBlock(pContext, 0) > 0); /* Skip data blocks */
      ImageCnt++;
      break;
    case GIF_INTRO_TERMINATOR:
      break;
    case GIF_INTRO_EXTENSION:
      if (_ReadExtension(pContext, NULL, (Index == ImageCnt) ? pInfo : NULL, NULL)) {
        return 1;
      }
      break;
    default:
      return 1;
    }
  } while (Introducer != GIF_INTRO_TERMINATOR); /* We do not support more than one image, so stop when the first terminator has been read */
  return 0;
}

/*********************************************************************
*
*       _GetGIFComment
*
* Purpose:
*   Returns the given comment of the GIF image.
*
* Parameters:
*   pData            - Pointer to start of the GIF file
*   NumBytes         - Number of bytes in the file
*   pBuffer          - Pointer to buffer to be filled by the routine
*   MaxSize          - Number of bytes in buffer
*   Index            - Index of the comment to be returned
*
* Return value:
*   0 on success, 1 on error
*/
static int _GetGIFComment(GUI_GIF_CONTEXT * pContext, U8 * pBuffer, int MaxSize, int Index) {
  U8 Introducer;
  int NumColors, CommentCnt, Size;
  const U8 * pData;
  /* Initialize decoding and skip size and global color table */
  if (_GetSizeAndColorTable(pContext, NULL, NULL, &NumColors)) {
    return 1; /* Error */
  }
  CommentCnt = Size = 0;
  /* Iterate over the blocks */
  do {
    if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
      return 1; /* Error */
    }
    Introducer = *pData;
    switch (Introducer) {
    case GIF_INTRO_IMAGE:
      _SkipLocalColorTable(pContext);
      while (_GetDataBlock(pContext, 0) > 0); /* Skip data blocks */
      break;
    case GIF_INTRO_TERMINATOR:
      break;
    case GIF_INTRO_EXTENSION:
      _ReadComment(pContext, (Index == CommentCnt) ? pBuffer : NULL, MaxSize, &Size);
      if ((Size) && (Index == CommentCnt)) {
        return 0;
      }
      break;
    default:
      return 1;
    }
  } while (Introducer != GIF_INTRO_TERMINATOR); /* We do not support more than one image, so stop when the first terminator has been read */
  return 1;
}

/*********************************************************************
*
*       _ClearUnusedPixels
*
* Purpose:
*   Clears the pixels between the border of the previous drawn image and
*   the current image.
*/
static void _ClearUnusedPixels(int x0, int y0, IMAGE_DESCRIPTOR * pDescriptor, GUI_GIF_IMAGE_INFO * pInfo, int Num, int Denom) {
  GUI_USE_PARA(Num);
  GUI_USE_PARA(Denom);
  LCD_SetColorIndex(LCD_GetBkColorIndex());
  if (pDescriptor->YPos > pInfo->yPos) {
    LCD_FillRect(x0 + pInfo->xPos,
                 y0 + pInfo->yPos,
                 x0 + pInfo->xPos + pInfo->xSize - 1,
                 y0 + pDescriptor->YPos - 1);
  }
  if (pDescriptor->XPos > pInfo->xPos) {
    LCD_FillRect(x0 + pInfo->xPos,
                 y0 + pInfo->yPos,
                 x0 + pDescriptor->XPos - 1,
                 y0 + pInfo->yPos + pInfo->ySize - 1);
  }
  if ((pDescriptor->YPos + pDescriptor->YSize) < (pInfo->yPos + pInfo->ySize)) {
    LCD_FillRect(x0 + pInfo->xPos,
                 y0 + pDescriptor->YPos + pDescriptor->YSize - 1,
                 x0 + pInfo->xPos + pInfo->xSize - 1,
                 y0 + pInfo->yPos + pInfo->ySize - 1);
  }
  if ((pDescriptor->XPos + pDescriptor->XSize) < (pInfo->xPos + pInfo->xSize)) {
    LCD_FillRect(x0 + pDescriptor->XPos + pDescriptor->XSize - 1,
                 y0 + pInfo->yPos,
                 x0 + pInfo->xPos + pInfo->xSize - 1,
                 y0 + pInfo->yPos + pInfo->ySize - 1);
  }
}

/*********************************************************************
*
*       GUI_GIF__DrawFromFilePointer
*
* Purpose:
*   Draws the given sub image of a GIF file.
*
* Parameters:
*   pContext - Pointer to GUI_GIF_CONTEXT
*   x0       - Drawing position x
*   y0       - Drawing position y
*   Index    - Index of sub image to be drawn
*
* Return value:
*   0 on success, 1 on error
*/
int GUI_GIF__DrawFromFilePointer(GUI_GIF_CONTEXT * pContext, int x0, int y0, int Index, int Num, int Denom, DRAW_FROM_DATABLOCK pfDrawFromDataBlock, CLEAR_UNUSED_PIXELS pfClearUnusedPixels) {
  U8 Disposal = 0;
  int XSize, YSize, TransIndex, ImageCnt;
  IMAGE_DESCRIPTOR Descriptor = {0};
  GUI_GIF_IMAGE_INFO Info = {0};
  U8 Introducer;
  const U8 * pData;
  ImageCnt   =  0;
  TransIndex = -1;
  /* Initialize decoding and get size and global color table */
  if (_GetSizeAndColorTable(pContext, &XSize, &YSize, &Descriptor.NumColors)) {
    return 1; /* Error */
  }
  /* Iterate over the blocks */
  do {
    if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
      return 1; /* Error */
    }
    Introducer = *pData;
    switch (Introducer) {
    case GIF_INTRO_IMAGE:
      /* Read image descriptor */
      if (GUI_GIF__ReadData(pContext, 9, &pData, 0)) {
        return 1; /* Error */
      }
      Descriptor.XPos  = GUI__Read16(&pData);
      Descriptor.YPos  = GUI__Read16(&pData);
      Descriptor.XSize = GUI__Read16(&pData);
      Descriptor.YSize = GUI__Read16(&pData);
      Descriptor.Flags = *pData;
      if (Descriptor.Flags & 0x80) {
        /* Read local color table */
        Descriptor.NumColors = 2 << (Descriptor.Flags & 0x7);
        if (_ReadColorMap(pContext, Descriptor.NumColors)) {
          return 1; /* Error */
        }
      }
      if (ImageCnt == Index - 1) {
        Info.xPos  = Descriptor.XPos;
        Info.yPos  = Descriptor.YPos;
        Info.xSize = Descriptor.XSize;
        Info.ySize = Descriptor.YSize;
      }
      if (Index == ImageCnt) {
        if (Disposal == 2) {
          pfClearUnusedPixels(x0, y0, &Descriptor, &Info, Num, Denom);
        }
        if (pfDrawFromDataBlock(pContext, &Descriptor, x0, y0, TransIndex, Disposal, Num, Denom)) {
          return 1; /* Error */
        }
        /* Skip block terminator, must be 0 */
        if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
          return 1; /* Error */
        }
        #if 0
        if (*pData) {
          return 1; /* Error */
        }
        #endif
        return 0;
      } else {
        /* Skip codesize */
        if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
          return 1; /* Error */
        }
        while (_GetDataBlock(pContext, 0) > 0); /* Skip data blocks */
      }
      ImageCnt++;
      break;
    case GIF_INTRO_TERMINATOR:
      break;
    case GIF_INTRO_EXTENSION:
      /* Read image extension */
      if (_ReadExtension(pContext, &TransIndex, (Index == ImageCnt) ? &Info : NULL, (Index == ImageCnt) ? &Disposal : NULL)) {
        return 1;
      }
      break;
    default:
      return 1;
    }
  } while (Introducer != GIF_INTRO_TERMINATOR); /* We do not support more than one image, so stop when the first terminator has been read */
  return 0;
}

/*********************************************************************
*
*       _GetValue
*/
static U16 _GetValue(GUI_GET_DATA_FUNC * pfGetData, void * p, int Offset) {
  const U8        * pData;
  U16               r = 0;
  GUI_HMEM          hContext;
  GUI_GIF_CONTEXT * pContext;

  GUI_LOCK();
  hContext = GUI_ALLOC_AllocZero(sizeof(GUI_GIF_CONTEXT));
  if (hContext) {
    pContext = (GUI_GIF_CONTEXT *)GUI_ALLOC_h2p(hContext);
    pContext->pfGetData = pfGetData;
    pContext->pParam    = p;
    if (GUI_GIF__ReadData(pContext, Offset + 2, &pData, 1)) {
      return 0; /* Error */
    }
    pData += Offset;
    r = GUI__Read16(&pData);
    GUI_ALLOC_Free(hContext);
  }
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GIF__GetNextByte
*
* Purpose:
*   Reads the next LZW code from the LZW stack and returns the first byte from the LZW code.
*
* Return value:
*   >= 0 if succeed
*   -1   if not succeed
*   -2   if end code has been read
*/
int GUI_GIF__GetNextByte(GUI_GIF_CONTEXT * pContext) {
  int i, Code, Incode;
  while ((Code = _GetNextCode(pContext)) >= 0) {
    if (Code == pContext->ClearCode) {
      /* Corrupt GIFs can make this happen */
      if (pContext->ClearCode >= (1 << MAX_NUM_LWZ_BITS)) {
        return -1; /* Error */
      }
      /* Clear the tables */
      GUI__memset((U8 *)pContext->aCode, 0, sizeof(pContext->aCode));
      for (i = 0; i < pContext->ClearCode; ++i) {
        pContext->aPrefix[i] = i;
      }
      /* Calculate the 'special codes' in dependence of the initial code size
         and initialize the stack pointer */
      pContext->CodeSize    = pContext->SetCodeSize + 1;
      pContext->MaxCodeSize = pContext->ClearCode << 1;
      pContext->MaxCode     = pContext->ClearCode + 2;
      pContext->sp          = pContext->aDecompBuffer;
      /* Read the first code from the stack after clearing and initializing */
      do {
        pContext->FirstCode = _GetNextCode(pContext);
      } while (pContext->FirstCode == pContext->ClearCode);
      pContext->OldCode = pContext->FirstCode;
      return pContext->FirstCode;
    }
    if (Code == pContext->EndCode) {
      return -2; /* End code */
    }
    Incode = Code;
    if (Code >= pContext->MaxCode) {
      *(pContext->sp)++ = pContext->FirstCode;
      Code = pContext->OldCode;
    }
    while (Code >= pContext->ClearCode) {
      *(pContext->sp)++ = pContext->aPrefix[Code];
      if (Code == pContext->aCode[Code]) {
        return Code;
      }
      if ((pContext->sp - pContext->aDecompBuffer) >= sizeof(pContext->aDecompBuffer)) {
        return Code;
      }
      Code = pContext->aCode[Code];
    }
    *(pContext->sp)++ = pContext->FirstCode = pContext->aPrefix[Code];
    if ((Code = pContext->MaxCode) < (1 << MAX_NUM_LWZ_BITS)) {
      pContext->aCode  [Code] = pContext->OldCode;
      pContext->aPrefix[Code] = pContext->FirstCode;
      ++pContext->MaxCode;
      if ((pContext->MaxCode >= pContext->MaxCodeSize) && (pContext->MaxCodeSize < (1 << MAX_NUM_LWZ_BITS))) {
        pContext->MaxCodeSize <<= 1;
        ++pContext->CodeSize;
      }
    }
    pContext->OldCode = Incode;
    if (pContext->sp > pContext->aDecompBuffer) {
      return *--(pContext->sp);
    }
  }
  return Code;
}

/*********************************************************************
*
*       GUI_GIF__InitLZW
*
* Purpose:
*   Initializes the given LZW with the input code size
*/
void GUI_GIF__InitLZW(GUI_GIF_CONTEXT * pContext, int InputCodeSize) {
  pContext->SetCodeSize  = InputCodeSize;
  pContext->CodeSize     = InputCodeSize + 1;
  pContext->ClearCode    = (1 << InputCodeSize);
  pContext->EndCode      = (1 << InputCodeSize) + 1;
  pContext->MaxCode      = (1 << InputCodeSize) + 2;
  pContext->MaxCodeSize  = (1 << InputCodeSize) << 1;
  pContext->ReturnClear  = 1;
  pContext->LastByte     = 2;
  pContext->sp           = pContext->aDecompBuffer;
}

/*********************************************************************
*
*       GUI_GIF__ReadData
*/
int GUI_GIF__ReadData(GUI_GIF_CONTEXT * pContext, unsigned NumBytesReq, const U8 ** ppData, unsigned StartOfFile) {
#if GUI_GIF_INPUT_BUFFER_SIZE
  if (StartOfFile) {
    pContext->NumBytesInBuffer = 0;
  }
  /* Check if there are sufficient bytes in buffer  */
  if (pContext->NumBytesInBuffer < NumBytesReq) {
    /* Fill up to maximum size */
    unsigned NumBytesRead;
    NumBytesRead = GUI_GIF_INPUT_BUFFER_SIZE - pContext->NumBytesInBuffer;
    if (pContext->NumBytesInBuffer) {
      memmove(_aInputBuffer, pContext->pBuffer, pContext->NumBytesInBuffer); /* Shift the remaining few bytes from top to bottom */
    }
    pContext->pBuffer = _aInputBuffer + pContext->NumBytesInBuffer;
    if ((pContext->NumBytesInBuffer += pContext->pfGetData(pContext->pParam, NumBytesRead, ppData, StartOfFile)) < NumBytesReq) {
      return 1;
    }
    GUI_MEMCPY((void *)pContext->pBuffer, (const void *)*ppData, NumBytesRead);
    pContext->pBuffer = _aInputBuffer;
  }
  *ppData = pContext->pBuffer;
  pContext->pBuffer          += NumBytesReq;
  pContext->NumBytesInBuffer -= NumBytesReq;
  return 0;
#else
  if (StartOfFile) {
    pContext->Off = 0;
  }
  if ((unsigned)pContext->pfGetData(pContext->pParam, ppData, NumBytesReq, pContext->Off) != NumBytesReq) {
    return 1;
  }
  pContext->Off += NumBytesReq;
  /*
  if ((unsigned)pContext->pfGetData(pContext->pParam, NumBytesReq, ppData, StartOfFile) != NumBytesReq) {
    return 1;
  }
  */
  return 0;
#endif
}

/*********************************************************************
*
*       GUI_GIF__GetData
*/
int GUI_GIF__GetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) {
  int RemBytes, NumBytes;
  GUI_GIF_PARAM * pParam;
  pParam = (GUI_GIF_PARAM *)p;
  RemBytes = pParam->FileSize - Off;
  NumBytes = 0;
  if (RemBytes > 0) {
    NumBytes = ((unsigned)RemBytes > NumBytesReq) ? NumBytesReq : RemBytes;
    *ppData  = pParam->pFileData + Off;
  }
  return NumBytes;
}
/*
int GUI_GIF__GetData(void * p, int NumBytesReq, const U8 ** ppData, unsigned StartOfFile) {
  int RemBytes, NumBytes;
  GUI_GIF_PARAM * pParam;
  pParam = (GUI_GIF_PARAM *)p;
  if (StartOfFile) {
    pParam->Off = 0;
  }
  RemBytes = pParam->FileSize - pParam->Off;
  NumBytes = 0;
  if (RemBytes > 0) {
    NumBytes = (RemBytes > NumBytesReq) ? NumBytesReq : RemBytes;
    *ppData  = pParam->pFileData + pParam->Off;
    pParam->Off += NumBytes;
  }
  return NumBytes;
}
*/

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GIF_GetXSizeEx
*/
int GUI_GIF_GetXSizeEx(GUI_GET_DATA_FUNC * pfGetData, void * p) {
  return _GetValue(pfGetData, p, 6);
}

/*********************************************************************
*
*       GUI_GIF_GetXSize
*/
int GUI_GIF_GetXSize(const void * pGIF) {
  GUI_GIF_PARAM Param = {0};
  Param.pFileData = (const U8 *)pGIF;
  Param.FileSize  = 8;
  return GUI_GIF_GetXSizeEx(GUI_GIF__GetData, &Param);
}

/*********************************************************************
*
*       GUI_GIF_GetYSizeEx
*/
int GUI_GIF_GetYSizeEx(GUI_GET_DATA_FUNC * pfGetData, void * p) {
  return _GetValue(pfGetData, p, 8);
}

/*********************************************************************
*
*       GUI_GIF_GetYSize
*/
int GUI_GIF_GetYSize(const void * pGIF) {
  GUI_GIF_PARAM Param = {0};
  Param.pFileData = (const U8 *)pGIF;
  Param.FileSize  = 10;
  return GUI_GIF_GetYSizeEx(GUI_GIF__GetData, &Param);
}

/*********************************************************************
*
*       GUI_GIF_DrawSubEx
*/
int GUI_GIF_DrawSubEx(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0, int Index) {
  int OldColorIndex;
  int Result = 1;
  GUI_HMEM          hContext;
  GUI_GIF_CONTEXT * pContext;
  #if (GUI_WINSUPPORT)
    GUI_RECT Rect;
    int Width, Height;
  #endif

  GUI_LOCK();
  hContext = GUI_ALLOC_AllocZero(sizeof(GUI_GIF_CONTEXT));
  if (hContext) {
    pContext = (GUI_GIF_CONTEXT *)GUI_ALLOC_h2p(hContext);
    pContext->pfGetData = pfGetData;
    pContext->pParam    = p;
    #if (GUI_WINSUPPORT)
    if (_GetImageDimension(pContext, &Width, &Height) == 0) {
    #else
    {
    #endif
      OldColorIndex = LCD_GetColorIndex();
      #if (GUI_WINSUPPORT)
        WM_ADDORG(x0,y0);
        Rect.x1 = (Rect.x0 = x0) + Width - 1;
        Rect.y1 = (Rect.y0 = y0) + Height - 1;
        WM_ITERATE_START(&Rect) {
      #endif
      Result = GUI_GIF__DrawFromFilePointer(pContext, x0, y0, Index, 0, 0, _DrawFromDataBlock, _ClearUnusedPixels);
      #if (GUI_WINSUPPORT)
        } WM_ITERATE_END();
      #endif
      LCD_SetColorIndex(OldColorIndex);
    }
    GUI_ALLOC_Free(hContext);
  }
  GUI_UNLOCK();
  return Result;
}

/*********************************************************************
*
*       GUI_GIF_DrawSub
*/
int GUI_GIF_DrawSub(const void * pGIF, U32 NumBytes, int x0, int y0, int Index) {
  GUI_GIF_PARAM Param = {0};
  Param.pFileData = (const U8 *)pGIF;
  Param.FileSize  = NumBytes;
  return GUI_GIF_DrawSubEx(GUI_GIF__GetData, &Param, x0, y0, Index);
}

/*********************************************************************
*
*       GUI_GIF_DrawEx
*/
int GUI_GIF_DrawEx(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0) {
  return GUI_GIF_DrawSubEx(pfGetData, p, x0, y0, 0);
}

/*********************************************************************
*
*       GUI_GIF_Draw
*/
int GUI_GIF_Draw(const void * pGIF, U32 NumBytes, int x0, int y0) {
  return GUI_GIF_DrawSub(pGIF, NumBytes, x0, y0, 0);
}

/*********************************************************************
*
*       GUI_GIF_GetInfoEx
*/
int GUI_GIF_GetInfoEx(GUI_GET_DATA_FUNC * pfGetData, void * p, GUI_GIF_INFO * pInfo) {
  GUI_HMEM          hContext;
  GUI_GIF_CONTEXT * pContext;
  int r = 1;

  GUI_LOCK();
  hContext = GUI_ALLOC_AllocZero(sizeof(GUI_GIF_CONTEXT));
  if (hContext) {
    pContext = (GUI_GIF_CONTEXT *)GUI_ALLOC_h2p(hContext);
    pContext->pfGetData = pfGetData;
    pContext->pParam    = p;
    r = _GetGIFInfo(pContext, pInfo);
    GUI_ALLOC_Free(hContext);
  }
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_GIF_GetInfo
*/
int GUI_GIF_GetInfo(const void * pGIF, U32 NumBytes, GUI_GIF_INFO * pInfo) {
  GUI_GIF_PARAM Param = {0};
  Param.pFileData = (const U8 *)pGIF;
  Param.FileSize  = NumBytes;
  return GUI_GIF_GetInfoEx(GUI_GIF__GetData, &Param, pInfo);
}

/*********************************************************************
*
*       GUI_GIF_GetImageInfoEx
*/
int GUI_GIF_GetImageInfoEx(GUI_GET_DATA_FUNC * pfGetData, void * p, GUI_GIF_IMAGE_INFO * pInfo, int Index) {
  GUI_HMEM          hContext;
  GUI_GIF_CONTEXT * pContext;
  int r = 1;

  GUI_LOCK();
  hContext = GUI_ALLOC_AllocZero(sizeof(GUI_GIF_CONTEXT));
  if (hContext) {
    pContext = (GUI_GIF_CONTEXT *)GUI_ALLOC_h2p(hContext);
    pContext->pfGetData = pfGetData;
    pContext->pParam    = p;
    r = _GetImageInfo(pContext, pInfo, Index);
    GUI_ALLOC_Free(hContext);
  }
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_GIF_GetImageInfo
*/
int GUI_GIF_GetImageInfo(const void * pGIF, U32 NumBytes, GUI_GIF_IMAGE_INFO * pInfo, int Index) {
  GUI_GIF_PARAM Param = {0};
  Param.pFileData = (const U8 *)pGIF;
  Param.FileSize  = NumBytes;
  return GUI_GIF_GetImageInfoEx(GUI_GIF__GetData, &Param, pInfo, Index);
}

/*********************************************************************
*
*       GUI_GIF_GetCommentEx
*/
int GUI_GIF_GetCommentEx(GUI_GET_DATA_FUNC * pfGetData, void * p, U8 * pBuffer, int MaxSize, int Index) {
  GUI_HMEM          hContext;
  GUI_GIF_CONTEXT * pContext;
  int r = 1;

  GUI_LOCK();
  hContext = GUI_ALLOC_AllocZero(sizeof(GUI_GIF_CONTEXT));
  if (hContext) {
    pContext = (GUI_GIF_CONTEXT *)GUI_ALLOC_h2p(hContext);
    pContext->pfGetData = pfGetData;
    pContext->pParam    = p;
    r = _GetGIFComment(pContext, pBuffer, MaxSize, Index);
    GUI_ALLOC_Free(hContext);
  }
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_GIF_GetComment
*/
int GUI_GIF_GetComment(const void * pGIF, U32 NumBytes, U8 * pBuffer, int MaxSize, int Index) {
  GUI_GIF_PARAM Param = {0};
  Param.pFileData = (const U8 *)pGIF;
  Param.FileSize  = NumBytes;
  return GUI_GIF_GetCommentEx(GUI_GIF__GetData, &Param, pBuffer, MaxSize, Index);
}

/*************************** End of file ****************************/
