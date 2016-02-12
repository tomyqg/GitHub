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
File        : GUIDEV_Measure.c
Purpose     : Implementation of measurement devices
              The purpose of a measurement device is to find out the
              area (Rectangle) affected by a sequence of drawing
              operations.
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

/* Memory device capabilities are compiled only if support for them is enabled.*/ 
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       typedefs
*
**********************************************************************
*/

typedef struct {
  GUI_DEVICE * pDevice;
  GUI_RECT rUsed;
  tLCDDEV_Color2Index*  pfColor2Index;
  tLCDDEV_Index2Color*  pfIndex2Color;
  tLCDDEV_GetIndexMask* pfGetIndexMask;
} GUI_MEASDEV;

/*********************************************************************
*
*       defines
*
**********************************************************************
*/

#define GUI_MEASDEV_H2P(h) ((GUI_MEASDEV *)GUI_ALLOC_h2p(h))

/*********************************************************************
*
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _MarkPixel
*/
static void _MarkPixel(int x, int y) {
  GUI_MEASDEV * pDev = (GUI_MEASDEV *)(GUI_MEMDEV_H2P(GUI_Context.hDevData));
  if (x < pDev->rUsed.x0) {
    pDev->rUsed.x0 = x;
  }
  if (x > pDev->rUsed.x1) {
    pDev->rUsed.x1 = x;
  }
  if (y < pDev->rUsed.y0) {
    pDev->rUsed.y0 = y;
  }
  if (y > pDev->rUsed.y1) {
    pDev->rUsed.y1 = y;
  }
}

/*********************************************************************
*
*       _MarkRect
*/
static void _MarkRect(int x0, int y0, int x1, int y1) {
  GUI_MEASDEV * pDev = (GUI_MEASDEV *)(GUI_MEMDEV_H2P(GUI_Context.hDevData));
  if (x0 < pDev->rUsed.x0) {
    pDev->rUsed.x0 = x0;
  }
  if (x1 > pDev->rUsed.x1) {
    pDev->rUsed.x1 = x1;
  }
  if (y0 < pDev->rUsed.y0) {
    pDev->rUsed.y0 = y0;
  }
  if (y1 > pDev->rUsed.y1) {
    pDev->rUsed.y1 = y1;
  }
}

/*********************************************************************
*
*       _DrawBitmap
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0, int xsize, int ysize,
                       int BitsPerPixel, int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX* pTrans)
{
  GUI_USE_PARA(pDevice);
  GUI_USE_PARA(BitsPerPixel);
  GUI_USE_PARA(BytesPerLine);
  GUI_USE_PARA(pData);
  GUI_USE_PARA(Diff);
  GUI_USE_PARA(pTrans);
  _MarkRect(x0, y0, x0 + xsize - 1, y0 + ysize - 1);
}

/*********************************************************************
*
*       _DrawHLine
*/
static void _DrawHLine(GUI_DEVICE * pDevice, int x0, int y,  int x1) {
  GUI_USE_PARA(pDevice);
  _MarkRect(x0, y, x1, y);
}

/*********************************************************************
*
*       _DrawVLine
*/
static void _DrawVLine(GUI_DEVICE * pDevice, int x , int y0,  int y1) {
  GUI_USE_PARA(pDevice);
  _MarkRect(x, y0, x, y1);
}

/*********************************************************************
*
*       _SetPixelIndex
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int Index) {
  GUI_USE_PARA(pDevice);
  GUI_USE_PARA(Index);
  _MarkPixel(x, y);
}

/*********************************************************************
*
*       _XorPixel
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  GUI_USE_PARA(pDevice);
  _MarkPixel(x, y);
}

/*********************************************************************
*
*       _GetPixelIndex
*/
static unsigned int _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y) { 
  GUI_USE_PARA(pDevice);
  GUI_USE_PARA(x);
  GUI_USE_PARA(y);
  return 0;
}

/*********************************************************************
*
*       _FillRect
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  GUI_USE_PARA(pDevice);
  _MarkRect(x0, y0, x1, y1);
}

/*********************************************************************
*
*       _GetRect
*/
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  GUI_USE_PARA(pDevice);
  pRect->x0 = pRect->y0 = -4095;
  pRect->x1 = pRect->y1 =  4095;
}

/*********************************************************************
*
*       _CalcPolyRect
*/
static void _CalcPolyRect(GUI_RECT *pr, const GUI_POINT* paPoint, int NumPoints) {
  int i;
  int xMin, xMax, yMin, yMax;
  xMin = GUI_XMAX;
  yMin = GUI_YMAX;
  xMax = GUI_XMIN;
  yMax = GUI_YMIN;
  for (i = 0; i  <NumPoints; i++) {
    int x = paPoint->x;
    int y = paPoint->y;
    if (xMin > x)
      xMin = x;
    if (xMax < x)
      xMax = x;
    if (yMin > y)
      yMin = y;
    if (yMax < y)
      yMax = y;
    paPoint++;
  }
  pr->x0 = xMin;
  pr->x1 = xMax;
  pr->y0 = yMin;
  pr->y1 = yMax;
}

/*********************************************************************
*
*       _FillPolygon
*/
static void _FillPolygon(GUI_DEVICE * pDevice, const GUI_POINT* paPoint, int NumPoints, int x0, int y0) {
  GUI_RECT r;

  GUI_USE_PARA(pDevice);
  _CalcPolyRect(&r, paPoint, NumPoints);
  GUI_MoveRect(&r, x0, y0);
  _MarkRect(r.x0, r.y0, r.x1, r.y1);
}

/*********************************************************************
*
*       _FillPolygonAA
*/
static void _FillPolygonAA(GUI_DEVICE * pDevice, const GUI_POINT* paPoint, int NumPoints, int x0, int y0) {
  GUI_RECT r;

  GUI_USE_PARA(pDevice);
  _CalcPolyRect(&r, paPoint, NumPoints);
  GUI_MoveRect(&r, x0, y0);
  if (GUI_Context.AA_HiResEnable) {
    r.x0 /= GUI_Context.AA_Factor;
    r.y0 /= GUI_Context.AA_Factor;
    r.x1 = (r.x1 + GUI_Context.AA_Factor - 1) / GUI_Context.AA_Factor;
    r.y1 = (r.y1 + GUI_Context.AA_Factor - 1) / GUI_Context.AA_Factor;
  }
  _MarkRect(r.x0, r.y0, r.x1, r.y1);
}

/*********************************************************************
*
*       _GetDevData
*/
static void * _GetDevData(GUI_DEVICE * pDevice, int Index) {
  GUI_USE_PARA(pDevice);
  GUI_USE_PARA(Index);
  return NULL;
}

/*********************************************************************
*
*       _SetOrg
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfSetOrg(pDevice, x, y);
}
/*********************************************************************
*
*       _GetDevFunc
*/
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  GUI_USE_PARA(ppDevice);
  switch (Index) {
  case LCD_DEVFUNC_FILLPOLY:
    return (void (*)(void))_FillPolygon;
  case LCD_DEVFUNC_FILLPOLYAA:
    return (void (*)(void))_FillPolygonAA;
  }
  return NULL;
}
/*********************************************************************
*
*       _GetDevProp
*/
static I32 _GetDevProp(GUI_DEVICE * pDevice, int Index) {
  GUI_USE_PARA(pDevice);
  GUI_USE_PARA(Index);
  return 0;
}

/*********************************************************************
*
*       GUI_DEVICE_API structure
*/
static const GUI_DEVICE_API _MeasDev_API = {
  //
  // Data
  //
  DEVICE_CLASS_MEASDEV,
  //
  // Drawing functions
  //
  _DrawBitmap   ,
  _DrawHLine    ,
  _DrawVLine    ,
  _FillRect     ,
  _GetPixelIndex,
  _SetPixelIndex,
  _XorPixel     ,
  //
  // Set origin
  //
  _SetOrg       ,
  //
  // Request information
  //
  _GetDevFunc   ,
  _GetDevProp   ,
  _GetDevData,
  _GetRect      ,
};

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEASDEV_Delete
*/
void GUI_MEASDEV_Delete(GUI_MEASDEV_Handle hMemDev) {
  GUI_DEVICE * pDevice;
  GUI_MEASDEV * pDevData;

  //
  // Make sure memory device is not used
  //
  if ((GUI_Context.hDevData == hMemDev) != 0) {
	  GUI_SelectLCD();
  }
  GUI_LOCK();
  pDevData = (GUI_MEASDEV *)GUI_LOCK_H(hMemDev);
  pDevice = pDevData->pDevice;
  GUI_UNLOCK_H(pDevData);
  GUI_ALLOC_Free(hMemDev);
  GUI_DEVICE_Unlink(pDevice);
  GUI_DEVICE_Delete(pDevice);
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_MEASDEV_ClearRect
*/
void GUI_MEASDEV_ClearRect(GUI_MEASDEV_Handle hMemDev) {
  if (hMemDev) {
    GUI_MEASDEV * pDevData;
    GUI_LOCK();
    pDevData = (GUI_MEASDEV *)GUI_ALLOC_h2p(hMemDev);
    pDevData->rUsed.x0 = GUI_XMAX;
    pDevData->rUsed.y0 = GUI_YMAX;
    pDevData->rUsed.x1 = GUI_XMIN;
    pDevData->rUsed.y1 = GUI_YMIN;
    GUI_UNLOCK();
  } 
}

/*********************************************************************
*
*       GUI_MEASDEV_Create
*/
GUI_MEASDEV_Handle GUI_MEASDEV_Create(void) {
  int MemSize;
  GUI_MEASDEV_Handle hMemDev;
  GUI_MEASDEV * pDevData;
  GUI_DEVICE * pDevice;

  pDevice = GUI_DEVICE_Create(&_MeasDev_API, GUI_Context.apDevice[GUI_Context.SelLayer]->pColorConvAPI, 0, GUI_Context.SelLayer);
  if (pDevice == 0) {
    return 0;
  }
  MemSize = sizeof(GUI_MEASDEV);
  hMemDev = GUI_ALLOC_AllocZero(MemSize);
  if (hMemDev) {
    GUI_LOCK();
    pDevice->u.hContext = hMemDev;
    pDevData = (GUI_MEASDEV *)GUI_LOCK_H(hMemDev);
    pDevData->pDevice       = pDevice;
    GUI_MEASDEV_ClearRect(hMemDev);
    GUI_UNLOCK_H(pDevData);
    GUI_UNLOCK();
  } else {
    GUI_DEVICE_Delete(pDevice);
    GUI_DEBUG_WARN("GUI_MEASDEV_Create: Alloc failed");
  }
  return hMemDev;
}

/*********************************************************************
*
*       GUI_MEASDEV_Select
*/
void GUI_MEASDEV_Select(GUI_MEASDEV_Handle hMemDev) {
  GUI_MEASDEV * pDevData;

  if (hMemDev == 0) {
    GUI_SelectLCD();
  } else {
    #if GUI_WINSUPPORT
      WM_Deactivate();
    #endif
    GUI_Context.hDevData     = hMemDev;
    GUI_LOCK();
    pDevData = (GUI_MEASDEV *)GUI_LOCK_H(hMemDev);
    GUI_DEVICE_Link(pDevData->pDevice);
    GUI_Context.pClipRect_HL = NULL;
    GUI_UNLOCK_H(pDevData);
    GUI_UNLOCK();
    LCD_SetClipRectMax();
  }
}

/*********************************************************************
*
*       GUI_MEASDEV_GetRect
*/
void GUI_MEASDEV_GetRect(GUI_MEASDEV_Handle hMem, GUI_RECT * pRect) {
  if (hMem) {
    GUI_MEASDEV * pDev;
    GUI_LOCK();
    pDev = (GUI_MEASDEV *)GUI_ALLOC_h2p(hMem);
    if (pRect) {
      pRect->x0 = pDev->rUsed.x0;
      pRect->y0 = pDev->rUsed.y0;
      pRect->x1 = pDev->rUsed.x1;
      pRect->y1 = pDev->rUsed.y1;
    }
    GUI_UNLOCK();
  }
}

#else

void GUIDEV_Measure(void);
void GUIDEV_Measure(void) {} /* avoid empty object files */

#endif /* GUI_MEMDEV_SUPPORT */

/*************************** end of file ****************************/
