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
File        : LCD_GetEx.C
Purpose     : Routines returning info at runtime
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetDevProp
*/
static int _GetDevProp(int LayerIndex, int Item) {
  GUI_DEVICE * pDevice;
  int r = 0;

  if (LayerIndex < GUI_NUM_LAYERS) {
    pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
    if (pDevice) {
      r = pDevice->pDeviceAPI->pfGetDevProp(pDevice, Item);
    }
  }
  return r;
}

/*********************************************************************
*
*       _GetDevData
*/
/*
static void * _GetDevData(int LayerIndex, int Item) {
  GUI_DEVICE * pDevice;
  void * p = NULL;

  if (LayerIndex < GUI_NUM_LAYERS) {
    pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
    if (pDevice) {
      p = pDevice->pDeviceAPI->pfGetDevData(pDevice, Item);
    }
  }
  return p;
}
*/

/*********************************************************************
*
*       _GetMaxValue
*/
static int _GetMaxValue(int Item) {
  int MaxValue;
  int Value;
  int LayerIndex;
  MaxValue   = 0;
  LayerIndex = GUI_NUM_LAYERS - 1;
  do {
    Value = _GetDevProp(LayerIndex, Item);
    if (MaxValue < Value) {
      MaxValue = Value;
    }
  } while (--LayerIndex >= 0);
  return MaxValue;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_GetxxxEx
*/
int LCD_GetXSizeEx          (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_XSIZE); }
int LCD_GetYSizeEx          (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_YSIZE); }
int LCD_GetVXSizeEx         (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_VXSIZE); }
int LCD_GetVYSizeEx         (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_VYSIZE);}
int LCD_GetBitsPerPixelEx   (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_BITSPERPIXEL);}
U32 LCD_GetNumColorsEx      (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_NUMCOLORS);}
int LCD_GetXMagEx           (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_XMAG);}
int LCD_GetYMagEx           (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_YMAG);}
int LCD_GetMirrorXEx        (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_MIRROR_X);}
int LCD_GetMirrorYEx        (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_MIRROR_Y);}
int LCD_GetSwapXYEx         (int LayerIndex) { return _GetDevProp(LayerIndex, LCD_DEVCAP_SWAP_XY);}

/*********************************************************************
*
*       LCD_Getxxx
*/
int LCD_GetXSize          (void) { return LCD_GetXSizeEx          (GUI_Context.SelLayer);}
int LCD_GetYSize          (void) { return LCD_GetYSizeEx          (GUI_Context.SelLayer);}
int LCD_GetVXSize         (void) { return LCD_GetVXSizeEx         (GUI_Context.SelLayer);}
int LCD_GetVYSize         (void) { return LCD_GetVYSizeEx         (GUI_Context.SelLayer);}
int LCD_GetBitsPerPixel   (void) { return LCD_GetBitsPerPixelEx   (GUI_Context.SelLayer);}
U32 LCD_GetNumColors      (void) { return LCD_GetNumColorsEx      (GUI_Context.SelLayer);}
int LCD_GetXMag           (void) { return LCD_GetXMagEx           (GUI_Context.SelLayer);}
int LCD_GetYMag           (void) { return LCD_GetYMagEx           (GUI_Context.SelLayer);}
int LCD_GetMirrorX        (void) { return LCD_GetMirrorXEx        (GUI_Context.SelLayer);}
int LCD_GetMirrorY        (void) { return LCD_GetMirrorYEx        (GUI_Context.SelLayer);}
int LCD_GetSwapXY         (void) { return LCD_GetSwapXYEx         (GUI_Context.SelLayer);}

/*********************************************************************
*
*       GUI_GetBitsPerPixelEx
*/
int GUI_GetBitsPerPixelEx(int LayerIndex) {
  return GUI_Context.apDevice[LayerIndex]->pDeviceAPI->pfGetDevProp(GUI_Context.apDevice[LayerIndex], LCD_DEVCAP_BITSPERPIXEL);
}

/*********************************************************************
*
*       LCD_GetPaletteEx
*/
LCD_COLOR * LCD_GetPaletteEx(int LayerIndex) {
  GUI_DEVICE * pDevice;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    return (LCD_COLOR *)pDevice->pDeviceAPI->pfGetDevData(pDevice, LCD_DEVDATA_PHYSPAL);
  }
  return NULL;
}

/*********************************************************************
*
*       LCD_GetPalette
*/
LCD_COLOR * LCD_GetPalette(void) {
  return LCD_GetPaletteEx(GUI_Context.SelLayer);
}

/*********************************************************************
*
*       LCD_GetpfIndex2ColorEx
*/
tLCDDEV_Index2Color * LCD_GetpfIndex2ColorEx(int LayerIndex) {
  GUI_DEVICE * pDevice;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    return pDevice->pColorConvAPI->pfIndex2Color;
  }
  return NULL;
}

/*********************************************************************
*
*       GUI_GetpfIndex2ColorEx
*/
tLCDDEV_Index2Color * GUI_GetpfIndex2ColorEx(int LayerIndex) {
  return GUI_Context.apDevice[LayerIndex]->pColorConvAPI->pfIndex2Color;
}

/*********************************************************************
*
*       LCD_GetpfColor2IndexEx
*/
tLCDDEV_Color2Index * LCD_GetpfColor2IndexEx(int LayerIndex) {
  GUI_DEVICE * pDevice;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    return pDevice->pColorConvAPI->pfColor2Index;
  }
  return NULL;
}

/*********************************************************************
*
*       GUI_GetpfColor2IndexEx
*/
tLCDDEV_Color2Index * GUI_GetpfColor2IndexEx(int LayerIndex) {
  return GUI_Context.apDevice[LayerIndex]->pColorConvAPI->pfColor2Index;
}

/*********************************************************************
*
*       LCD_Get...Max
*/
int LCD_GetXSizeMax(void)        { return _GetMaxValue(LCD_DEVCAP_XSIZE); }
int LCD_GetYSizeMax(void)        { return _GetMaxValue(LCD_DEVCAP_YSIZE); }
int LCD_GetVXSizeMax(void)       { return _GetMaxValue(LCD_DEVCAP_VXSIZE); }
int LCD_GetVYSizeMax(void)       { return _GetMaxValue(LCD_DEVCAP_VYSIZE); }
int LCD_GetBitsPerPixelMax(void) { return _GetMaxValue(LCD_DEVCAP_BITSPERPIXEL); }

/*********************************************************************
*
*       LCD__GetBPP
*
* Purpose:
*   This routine returns the number of bits used by the index mask.
*   It does not consider gaps in the mask.
*/
I32 LCD__GetBPP(U32 IndexMask) {
  I32 r = 0;

  if (IndexMask == 0) {
    r = 0;
  } else if ((IndexMask & 0xFFFFFFFE) == 0) {
    r = 1;
  } else if ((IndexMask & 0xFFFFFFFC) == 0) {
    r = 2;
  } else if ((IndexMask & 0xFFFFFFF8) == 0) {
    r = 3;
  } else if ((IndexMask & 0xFFFFFFF0) == 0) {
    r = 4;
  } else if ((IndexMask & 0xFFFFFFE0) == 0) {
    r = 5;
  } else if ((IndexMask & 0xFFFFFF00) == 0) {
    r = 8;
  } else if ((IndexMask & 0xFFFFFE00) == 0) {
    r = 9;
  } else if ((IndexMask & 0xFFFFF000) == 0) {
    r = 12;
  } else if ((IndexMask & 0xFFFF8000) == 0) {
    r = 15;
  } else if ((IndexMask & 0xFFFF0000) == 0) {
    r = 16;
  } else if ((IndexMask & 0xFFFC0000) == 0) {
    r = 18;
  } else if ((IndexMask & 0xFF000000) == 0) {
    r = 24;
  } else {
    r = 32;
  }
  return r;
}

/*********************************************************************
*
*       LCD__GetBPPDevice
*
* Purpose:
*   This routine returns the number of bits required for calculating
*   the size of memory devices.
*/
I32 LCD__GetBPPDevice(U32 IndexMask) {
  I32 BitsPerPixel;
  I32 r = 0;

  BitsPerPixel = LCD__GetBPP(IndexMask);
  if        (BitsPerPixel == 1) {
    r = 1;
  } else if ((BitsPerPixel > 1) && (BitsPerPixel <= 8)) {
    r = 8;
  } else if ((BitsPerPixel > 8) && (BitsPerPixel <= 16)) {
    r = 16;
  } else if ((BitsPerPixel > 16) && (BitsPerPixel <= 32)) {
    r = 32;
  }
  return r;
}

/************************* End of file ******************************/
