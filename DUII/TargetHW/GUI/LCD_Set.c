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
File        : LCD_SetAlphaEx.c
Purpose     : Setting the alpha blending factor of a layer
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_Private.h"
#include "LCD_SIM.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_SetAlphaEx
*/
int LCD_SetAlphaEx(int LayerIndex, int Alpha) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, int);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, int))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_SETALPHA);
    if (pFunc) {
      pFunc(pDevice, Alpha);
      r = 0;
    }
    #ifdef WIN32
      LCDSIM_SetAlpha(Alpha, pDevice->LayerIndex);
    #endif
  }
  return r;
}

/*********************************************************************
*
*       LCD_SetVRAMAddrEx
*/
int LCD_SetVRAMAddrEx(int LayerIndex, void * pVRAM) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, void *);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, void *))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_SET_VRAM_ADDR);
    if (pFunc) {
      pFunc(pDevice, pVRAM);
      r = 0;
    }
  }
  return r;
}

/*********************************************************************
*
*       LCD_SetVSizeEx
*/
int LCD_SetVSizeEx(int LayerIndex, int xSize, int ySize) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, int, int);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, int, int))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_SET_VSIZE);
    if (pFunc) {
      pFunc(pDevice, xSize, ySize);
      r = 0;
    }
  }
  return r;
}

/*********************************************************************
*
*       LCD_SetSizeEx
*/
int LCD_SetSizeEx(int LayerIndex, int xSize, int ySize) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, int, int);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, int, int))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_SET_SIZE);
    if (pFunc) {
      pFunc(pDevice, xSize, ySize);
      r = 0;
    }
    #ifdef WIN32
      LCDSIM_SetSize(LayerIndex, xSize, ySize);
    #endif
  }
  return r;
}

/*********************************************************************
*
*       LCD_SetVisEx
*/
int LCD_SetVisEx(int LayerIndex, int OnOff) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, int);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, int))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_SETVIS);
    if (pFunc) {
      pFunc(pDevice, OnOff);
      r = 0;
    }
    #ifdef WIN32
      LCDSIM_SetLayerVis(OnOff, LayerIndex);
    #endif
  }
  return r;
}

/*********************************************************************
*
*       LCD_SetPosEx
*/
int LCD_SetPosEx(int LayerIndex, int xPos, int yPos) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, int, int);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, int, int))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_SETPOS);
    if (pFunc) {
      pFunc(pDevice, xPos, yPos);
      r = 0;
    }
    #ifdef WIN32
      LCDSIM_SetLayerPos(xPos, yPos, LayerIndex);
    #endif
  }
  return r;
}

/*********************************************************************
*
*       LCD_SetAlphaModeEx
*/
int LCD_SetAlphaModeEx(int LayerIndex, int AlphaMode) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, int);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, int))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_ALPHAMODE);
    if (pFunc) {
      pFunc(pDevice, AlphaMode);
      r = 0;
    }
    #ifdef WIN32
      //SIM_GUI_SetTransMode(LayerIndex, AlphaMode);
    #endif
  }
  return r;
}

/*********************************************************************
*
*       LCD_SetChromaModeEx
*/
int LCD_SetChromaModeEx(int LayerIndex, int ChromaMode) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, int);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, int))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_CHROMAMODE);
    if (pFunc) {
      pFunc(pDevice, ChromaMode);
      r = 0;
    }
    #ifdef WIN32
      SIM_GUI_SetTransMode(LayerIndex, ChromaMode);
    #endif
  }
  return r;
}

/*********************************************************************
*
*       LCD_SetChromaEx
*/
int LCD_SetChromaEx(int LayerIndex, LCD_COLOR ChromaMin, LCD_COLOR ChromaMax) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, LCD_COLOR, LCD_COLOR);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, LCD_COLOR, LCD_COLOR))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_CHROMA);
    if (pFunc) {
      pFunc(pDevice, ChromaMin, ChromaMax);
      r = 0;
    }
    #ifdef WIN32
      SIM_GUI_SetChroma(LayerIndex, ChromaMin, ChromaMax);
    #endif
  }
  return r;
}

/*********************************************************************
*
*       LCD_SetLUTEntryEx
*/
int LCD_SetLUTEntryEx(int LayerIndex, U8 Pos, LCD_COLOR Color) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, U8, LCD_COLOR);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, U8, LCD_COLOR))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_SETLUTENTRY);
    if (pFunc) {
      pFunc(pDevice, Pos, Color);
      r = 0;
    }
    #ifdef WIN32
      LCDSIM_SetLUTEntry(Pos, ((Color & 0xFF0000) >> 16) | (Color & 0x00FF00) | ((Color & 0x0000FF) << 16), LayerIndex);
    #endif
  }
  return r;
}

/*********************************************************************
*
*       LCD_GetPosEx
*/
int LCD_GetPosEx(int LayerIndex, int * pxPos, int * pyPos) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, int *, int *);
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, int *, int *))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_GETPOS);
    if (pFunc) {
      pFunc(pDevice, pxPos, pyPos);
      r = 0;
    }
  }
  return r;
}

/*********************************************************************
*
*       The functions above without layer index for easier use...
*/
int LCD_SetAlpha     (int Alpha)                                { return LCD_SetAlphaEx     (GUI_Context.SelLayer, Alpha); }
int LCD_SetVRAMAddr  (void * pVRAM)                             { return LCD_SetVRAMAddrEx  (GUI_Context.SelLayer, pVRAM); }
int LCD_SetVSize     (int xSize, int ySize)                     { return LCD_SetVSizeEx     (GUI_Context.SelLayer, xSize, ySize); }
int LCD_SetSize      (int xSize, int ySize)                     { return LCD_SetSizeEx      (GUI_Context.SelLayer, xSize, ySize); }
int LCD_SetVis       (int OnOff)                                { return LCD_SetVisEx       (GUI_Context.SelLayer, OnOff); }
int LCD_SetPos       (int xPos, int yPos)                       { return LCD_SetPosEx       (GUI_Context.SelLayer, xPos, yPos); }
int LCD_SetAlphaMode (int AlphaMode)                            { return LCD_SetAlphaModeEx (GUI_Context.SelLayer, AlphaMode); }
int LCD_SetChromaMode(int ChromaMode)                           { return LCD_SetChromaModeEx(GUI_Context.SelLayer, ChromaMode); }
int LCD_SetChroma    (LCD_COLOR ChromaMin, LCD_COLOR ChromaMax) { return LCD_SetChromaEx    (GUI_Context.SelLayer, ChromaMin, ChromaMax); }
int LCD_SetLUTEntry  (U8 Pos, LCD_COLOR Color)                  { return LCD_SetLUTEntryEx  (GUI_Context.SelLayer, Pos, Color); }
int LCD_GetPos       (int * pxPos, int * pyPos)                 { return LCD_GetPosEx       (GUI_Context.SelLayer, pxPos, pyPos); }

/*********************************************************************
*
*       LCD_SetDevFunc
*/
int LCD_SetDevFunc(int LayerIndex, int IdFunc, void (* pDriverFunc)(void)) {
  GUI_DEVICE * pDevice;
  void (* pFunc)(GUI_DEVICE *, int, void (*)(void));
  int r = 1;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    pFunc = (void (*)(GUI_DEVICE *, int, void (*)(void)))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_SETFUNC);
    if (pFunc) {
      pFunc(pDevice, IdFunc, pDriverFunc);
      r = 0;
    }
  }
  return r;
}

/*************************** End of file ****************************/
