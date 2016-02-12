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
File        : GUIStreamCreate.c
Purpose     : Support for streamed bitmaps
----------------------------------------------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define DEFINE_CREATE_FROM_STREAM(ID, PTR, FORMAT)                                             \
int GUI_CreateBitmapFromStream##ID(GUI_BITMAP * pBMP, GUI_LOGPALETTE * pPAL, const void * p) { \
  return _CreateBitmapFromStream(p, pBMP, pPAL, PTR, FORMAT);                                  \
}

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _CreateBitmapFromStream
*/
static int _CreateBitmapFromStream(const void * p, GUI_BITMAP * pBMP, GUI_LOGPALETTE * pPAL, const GUI_BITMAP_METHODS * pMethods, int Format) {
  GUI_BITMAP_STREAM BitmapHeader;
  int r = 0;

  //
  // Read header data
  //
  GUI__ReadHeaderFromStream(&BitmapHeader, (const U8 *)p);
  if (BitmapHeader.Format == Format) {
    //
    // Initialize bitmap (and palette) structure
    //
    GUI__CreateBitmapFromStream(&BitmapHeader, (const U8 *)p, pBMP, pPAL, pMethods);
  } else {
    //
    // Error in case of wrong bitmap format
    //
    GUI_DEBUG_ERROROUT(__FILE__": Format of streamed bitmap does not match!");
    r = 1;
  }
  return r;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_CreateBitmapFromStream
*/
int GUI_CreateBitmapFromStream(GUI_BITMAP * pBMP, GUI_LOGPALETTE * pPAL, const void * p) {
  GUI_BITMAP_STREAM BitmapHeader;
  const GUI_BITMAP_METHODS * pMethods;

  //
  // Read header data
  //
  GUI__ReadHeaderFromStream(&BitmapHeader, (const U8 *)p);
  //
  // Get the right function pointer
  //
  switch (BitmapHeader.Format) {
  case GUI_STREAM_FORMAT_INDEXED:
    pMethods = NULL;
    break;
  case GUI_STREAM_FORMAT_RLE4:
    pMethods = GUI_DRAW_RLE4;
    break;
  case GUI_STREAM_FORMAT_RLE8:
    pMethods = GUI_DRAW_RLE8;
    break;
  case GUI_STREAM_FORMAT_565:
    pMethods = GUI_DRAW_BMP565;
    break;
  case GUI_STREAM_FORMAT_M565:
    pMethods = GUI_DRAW_BMPM565;
    break;
  case GUI_STREAM_FORMAT_555:
    pMethods = GUI_DRAW_BMP555;
    break;
  case GUI_STREAM_FORMAT_M555:
    pMethods = GUI_DRAW_BMPM555;
    break;
  case GUI_STREAM_FORMAT_RLE16:
    pMethods = GUI_DRAW_RLE16;
    break;
  case GUI_STREAM_FORMAT_RLEM16:
    pMethods = GUI_DRAW_RLEM16;
    break;
  case GUI_STREAM_FORMAT_24:
    pMethods = GUI_DRAW_BMP24;
    break;
  case GUI_STREAM_FORMAT_8888:
    pMethods = GUI_DRAW_BMP8888;
    break;
  default:
    GUI_DEBUG_ERROROUT(__FILE__": Format of streamed bitmap not supported!");
    return 1;
  }
  GUI__CreateBitmapFromStream(&BitmapHeader, (const U8 *)p, pBMP, pPAL, pMethods);
  return 0;
}

/*********************************************************************
*
*       GUI_CreateBitmapFromStreamXXX
*/
DEFINE_CREATE_FROM_STREAM(IDX,    NULL,             GUI_STREAM_FORMAT_INDEXED)
DEFINE_CREATE_FROM_STREAM(RLE4,   GUI_DRAW_RLE4,    GUI_STREAM_FORMAT_RLE4)
DEFINE_CREATE_FROM_STREAM(RLE8,   GUI_DRAW_RLE8,    GUI_STREAM_FORMAT_RLE8)
DEFINE_CREATE_FROM_STREAM(565,    GUI_DRAW_BMP565,  GUI_STREAM_FORMAT_565)
DEFINE_CREATE_FROM_STREAM(M565,   GUI_DRAW_BMPM565, GUI_STREAM_FORMAT_M565)
DEFINE_CREATE_FROM_STREAM(555,    GUI_DRAW_BMP555,  GUI_STREAM_FORMAT_555)
DEFINE_CREATE_FROM_STREAM(M555,   GUI_DRAW_BMPM555, GUI_STREAM_FORMAT_M555)
DEFINE_CREATE_FROM_STREAM(RLE16,  GUI_DRAW_RLE16,   GUI_STREAM_FORMAT_RLE16)
DEFINE_CREATE_FROM_STREAM(RLEM16, GUI_DRAW_RLEM16,  GUI_STREAM_FORMAT_RLEM16)
DEFINE_CREATE_FROM_STREAM(24,     GUI_DRAW_BMP24,   GUI_STREAM_FORMAT_24)
DEFINE_CREATE_FROM_STREAM(Alpha,  GUI_DRAW_BMP8888, GUI_STREAM_FORMAT_8888)

/*************************** End of file ****************************/
