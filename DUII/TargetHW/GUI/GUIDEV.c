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
File        : GUIDEV.c
Purpose     : Implementation of memory devices
---------------------------END-OF-HEADER------------------------------
*/


#include <string.h>

#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

//
// Memory device capabilities are compiled only if support for them is enabled.
//
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV__WriteToActiveAt
*/
void GUI_MEMDEV__WriteToActiveAt(GUI_MEMDEV_Handle hMem,int x, int y) {
  GUI_MEMDEV * pDev;
  void (* pFunc)(GUI_MEMDEV_Handle,int, int);

  pDev  = GUI_MEMDEV_H2P(hMem);
  pFunc = (void (*)(GUI_MEMDEV_Handle,int, int))pDev->pDevice->pDeviceAPI->pfGetDevFunc(NULL, MEMDEV_DEVFUNC_WRITETOACTIVE);
  if (pFunc) {
    pFunc(hMem, x, y);
  }
}

/*********************************************************************
*
*       GUI_MEMDEV__CreateFixed
*/
GUI_MEMDEV_Handle GUI_MEMDEV__CreateFixed(int x0, int y0, int xSize, int ySize, int Flags,
                              const GUI_DEVICE_API     * pDeviceAPI,
                              const LCD_API_COLOR_CONV * pColorConvAPI) {
  GUI_ALLOC_DATATYPE MaxLines;
  GUI_MEMDEV_Handle hMemDev;
  I32 MemSize;
  unsigned BitsPerPixel;
  unsigned BytesPerLine;
  GUI_USAGE_Handle hUsage = 0;
  GUI_MEMDEV * pDevData;
  GUI_DEVICE * pDevice;

  //
  // Create device object
  //
  pDevice = GUI_DEVICE_Create(pDeviceAPI, pColorConvAPI, 0, GUI_Context.SelLayer);
  if (pDevice == NULL) {
    return 0; // Error on creating device object
  }
  //
  // Calculate BitsPerPixel and BytesPerLine
  //
  BitsPerPixel = LCD__GetBPPDevice(pDevice->pColorConvAPI->pfGetIndexMask());
  BytesPerLine = (xSize * BitsPerPixel + 7) >> 3;
  //
  // Calc available MemSize
  //
  MemSize = GUI_ALLOC_RequestSize();
  if (!(Flags & GUI_MEMDEV_NOTRANS)) {
    MemSize = (MemSize / 4) * 3;   // We need to reserve some memory for usage object ... TBD: This can be optimized as we do not use memory perfectly.
  }
  if (ySize<=0) {
    MaxLines = (MemSize - sizeof(GUI_MEMDEV)) / BytesPerLine;
    ySize = (MaxLines > -ySize) ? -ySize : MaxLines;
  }
  if (!(Flags & GUI_MEMDEV_NOTRANS)) {
    //
    // Create the usage map
    //
    hUsage = GUI_USAGE_BM_Create(x0, y0, xSize, ySize, 0);
  }
  //
  // Check if we can alloc sufficient memory
  //
  if (ySize <= 0) {
    GUI_DEBUG_WARN("GUI_MEMDEV_Create: Too little memory");
    GUI_DEVICE_Delete(pDevice);
    return 0;    
  }
  MemSize = (I32)ySize * BytesPerLine + sizeof(GUI_MEMDEV);
  //
  // Allocate memory device object
  //
  if (Flags & GUI_MEMDEV_NOTRANS) {
    hMemDev = GUI_ALLOC_AllocNoInit(MemSize);
  } else {
    hMemDev = GUI_ALLOC_AllocZero(MemSize);
  }
  //
  // Return on error
  //
  if (hMemDev == 0) {
    if (hUsage) {
      GUI_ALLOC_Free(hUsage);
    }
    GUI_DEBUG_WARN("GUI_MEMDEV_Create: Alloc failed");
    return 0; // Error on creating memory device object
  }
  //
  // Set memory device properties
  //
  pDevData = GUI_MEMDEV_H2P(hMemDev);
  pDevData->pDevice      = pDevice;
  pDevData->x0           = x0;
  pDevData->y0           = y0;
  pDevData->XSize        = xSize;
  pDevData->YSize        = ySize;
  pDevData->BytesPerLine = BytesPerLine;
  pDevData->BitsPerPixel = BitsPerPixel;
  pDevData->hUsage       = hUsage;
  //
  // Set device object properties
  //
  pDevice->pColorConvAPI = pColorConvAPI;
  pDevice->u.hContext    = hMemDev;
  pDevice->LayerIndex    = GUI_Context.SelLayer;
  return hMemDev;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_GetBitsPerPixel
*/
int GUI_MEMDEV_GetBitsPerPixel(GUI_MEMDEV_Handle hMemDev) {
  int r = 0;
  GUI_MEMDEV * pDev;

  if (hMemDev) {
    GUI_LOCK();
    pDev = GUI_MEMDEV_H2P(hMemDev);
    r = LCD__GetBPPDevice(pDev->pDevice->pColorConvAPI->pfGetIndexMask());
    GUI_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       GUI_MEMDEV_Delete
*/
void GUI_MEMDEV_Delete(GUI_MEMDEV_Handle hMemDev) {
  GUI_MEMDEV * pMemDev;

  if (hMemDev) {
    GUI_LOCK();
    if (GUI_Context.hDevData == hMemDev) {
      GUI_SelectLCD();
    }
    pMemDev = GUI_MEMDEV_H2P(hMemDev);
    //
    // Delete the associated usage device
    //
    if (pMemDev->hUsage) {
      GUI_USAGE_DecUseCnt(pMemDev->hUsage);
    }
    //
    // Delete the device object
    //
    GUI_DEVICE_Delete(pMemDev->pDevice);
    //
    // Delete the memory device object
    //
    GUI_ALLOC_Free(hMemDev);
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       GUI_MEMDEV_CreateEx
*/
GUI_MEMDEV_Handle GUI_MEMDEV_CreateEx(int x0, int y0, int xSize, int ySize, int Flags) {
  GUI_DEVICE * pDevice;
  GUI_MEMDEV_Handle hMemDev = 0;
  const GUI_DEVICE_API * pDeviceMemdev;

  GUI_LOCK();
  //
  // Check if there is a memory device in the device chain
  //
  pDevice = GUI_Context.apDevice[GUI_Context.SelLayer];
  do {
    if (pDevice->pDeviceAPI->DeviceClassIndex == DEVICE_CLASS_MEMDEV) {
      break; // Found memory device, so stop here
    }
    pDevice = pDevice->pNext;
  } while (pDevice);
  //
  // If no memory device has been found, create a device compatible to the display
  //
  if (pDevice == NULL) {
    pDevice = GUI_DEVICE__GetpDriver(GUI_Context.SelLayer);
  }
  //
  // Use device data for creating a compatible device
  //
  pDeviceMemdev = (const GUI_DEVICE_API *)pDevice->pDeviceAPI->pfGetDevData(pDevice, LCD_DEVDATA_MEMDEV);
  if (pDeviceMemdev) {
    hMemDev = GUI_MEMDEV__CreateFixed(x0, y0, xSize, ySize, Flags, pDeviceMemdev, pDevice->pColorConvAPI);
  }
  GUI_UNLOCK();
  return hMemDev;
}

/*********************************************************************
*
*       GUI_MEMDEV_Create
*/
GUI_MEMDEV_Handle GUI_MEMDEV_Create(int x0, int y0, int xSize, int ySize) {
  return GUI_MEMDEV_CreateEx(x0, y0, xSize, ySize, GUI_MEMDEV_HASTRANS);
}

/*********************************************************************
*
*       GUI_MEMDEV_Select
*/
GUI_MEMDEV_Handle GUI_MEMDEV_Select(GUI_MEMDEV_Handle hMemDev) {
  GUI_MEMDEV * pMemDev;
  GUI_MEMDEV_Handle r;

  GUI_LOCK();
  r = GUI_Context.hDevData;
  if (hMemDev == 0) {
    GUI_SelectLCD();
  } else {
    if (r) {
      GUI_SelectLCD();
    }
    pMemDev = GUI_MEMDEV_H2P(hMemDev);
    #if GUI_WINSUPPORT
      WM_Deactivate();
    #endif
    //
    // If LCD was selected save cliprect
    //
    if (GUI_Context.hDevData == 0) {
      GUI_Context.ClipRectPrev = GUI_Context.ClipRect;
    }
    GUI_Context.hDevData = hMemDev;
    GUI_DEVICE_Link(pMemDev->pDevice);
    LCD_UpdateColorIndices();
    LCD_SetClipRectMax();
  }
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_MEMDEV_CopyToLCDAt
*/
void GUI_MEMDEV_CopyToLCDAt(GUI_MEMDEV_Handle hMem, int x, int y) {
  if (hMem) {
    GUI_DRAWMODE DrawModeOld;
    GUI_MEMDEV_Handle hMemPrev;
    GUI_MEMDEV* pDevData;
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  #if GUI_NUM_LAYERS > 1
    int PrevLayer;
  #endif
    GUI_LOCK();
    DrawModeOld = GUI_SetDrawMode(GUI_DM_NORMAL);
    hMemPrev = GUI_Context.hDevData;
    pDevData = (GUI_MEMDEV*) GUI_ALLOC_h2p(hMem);  // Convert to pointer
    //
    // Make sure LCD is selected as device
    //
  #if GUI_NUM_LAYERS > 1
    PrevLayer = GUI_SelectLayer(pDevData->pDevice->LayerIndex);
  #else
    GUI_SelectLCD();  // Activate LCD
  #endif
    if (x == GUI_POS_AUTO) {
      x = pDevData->x0;
      y = pDevData->y0;
    }
  #if (GUI_WINSUPPORT)
    //
    // Calculate rectangle
    //
    r.x1 = (r.x0 = x) + pDevData->XSize-1;
    r.y1 = (r.y0 = y) + pDevData->YSize-1;;
    //
    // Do the drawing. Window manager has to be on
    //
    WM_Activate();
    WM_ITERATE_START(&r) {
  #endif
    GUI_MEMDEV__WriteToActiveOpaque(hMem, x, y);
    //GUI_MEMDEV__WriteToActiveAt(hMem, x, y);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  #if GUI_NUM_LAYERS > 1
    GUI_SelectLayer(PrevLayer);
  #endif
    //
    // Reactivate previously used device
    //
    GUI_MEMDEV_Select(hMemPrev);
    GUI_SetDrawMode(DrawModeOld);
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       GUI_MEMDEV_CopyToLCD
*/
void GUI_MEMDEV_CopyToLCD(GUI_MEMDEV_Handle hMem) {
  GUI_MEMDEV_CopyToLCDAt(hMem, GUI_POS_AUTO, GUI_POS_AUTO);
}

#else

void GUIDEV_C(void);
void GUIDEV_C(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
