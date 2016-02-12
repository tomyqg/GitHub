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
File        : GUI_SelectLayer.c.C
Purpose     : Selection of Layer
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM_Intern_ConfDep.h"
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static unsigned  _aCursorLayer[GUI_NUM_LAYERS];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SelectLayer
*/
unsigned GUI_SelectLayer(unsigned Index) {
  unsigned OldIndex;
  OldIndex = GUI_Context.SelLayer;
  GUI_LOCK();
  if (Index < GUI_NUM_LAYERS) {
    GUI_Context.SelLayer = Index;
    GUI_SelectLCD();
  }
  #if (GUI_WINSUPPORT)
    #if (GUI_NUM_LAYERS > 1)
      WM__TOUCHED_LAYER = Index;
    #endif
  #endif
  GUI_UNLOCK();
  return OldIndex;
}

/*********************************************************************
*
*       GUI_AssignCursorLayer
*/
void GUI_AssignCursorLayer(unsigned LayerIndex, unsigned CursorLayer) {
  GUI_DEVICE * pDevice;

  pDevice = GUI_DEVICE__GetpDriver(LayerIndex);
  if (pDevice) {
    if (pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_SETPOS)) {
      _aCursorLayer[LayerIndex] = CursorLayer;
    }
  }
}

/*********************************************************************
*
*       GUI_GetCursorLayer
*/
unsigned GUI_GetCursorLayer(unsigned Index) {
  unsigned r;
  r = 0;
  if (Index < GUI_COUNTOF(_aCursorLayer)) {
    r = _aCursorLayer[Index];
  }
  return r;
}

/*********************************************************************
*
*       GUI_SetLayerPosEx
*/
int GUI_SetLayerPosEx(unsigned LayerIndex, int xPos, int yPos) {
  int r;
  
  GUI_LOCK();
  r = LCD_SetPosEx(LayerIndex, xPos, yPos);
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_SetLayerSizeEx
*/
int GUI_SetLayerSizeEx(unsigned LayerIndex, int xSize, int ySize) {
  int r;
  
  GUI_LOCK();
  r = LCD_SetSizeEx(LayerIndex, xSize, ySize);
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_SetLayerVisEx
*/
int GUI_SetLayerVisEx(unsigned LayerIndex, int OnOff) {
  int r;
  
  GUI_LOCK();
  r = LCD_SetVisEx(LayerIndex, OnOff);
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_SetLayerAlphaEx
*/
int GUI_SetLayerAlphaEx(unsigned LayerIndex, int Alpha) {
  int r;
  
  GUI_LOCK();
  r = LCD_SetAlphaEx(LayerIndex, Alpha);
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_GetLayerPosEx
*/
void GUI_GetLayerPosEx(unsigned LayerIndex, int * pxPos, int * pyPos) {
  int r;
  
  GUI_LOCK();
  r = LCD_GetPosEx(LayerIndex, pxPos, pyPos);
  if (r) {
    *pxPos = 0;
    *pyPos = 0;
  }
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
