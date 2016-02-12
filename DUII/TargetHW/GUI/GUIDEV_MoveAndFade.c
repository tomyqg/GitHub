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
File        : GUIDEV_MoveAndFade.c
Purpose     : Routines for fading and moving memory devices
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include <stddef.h>

#include "GUI_Private.h"

#if (GUI_SUPPORT_MEMDEV)

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifdef WIN32
  #define TIME_PER_FRAME 10
#else
  #define TIME_PER_FRAME 40
#endif

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV__FadeDeviceEx
*/
void GUI_MEMDEV__FadeDeviceEx(GUI_MEMDEV_Handle hMemWin, GUI_MEMDEV_Handle hMemBk, GUI_MEMDEV_Handle hMemDst, U8 Intens, int xPosWin, int yPosWin) {
  int xSizeWin, ySizeWin, y, xSizeBk;
  U32 Color, ColorDst, ColorBk, NumPixels, a;
  U32 * pWin; // Device of window to fade in
  U32 * pBk;  // Device of background
  U32 * pDst; // Destination device to be drawn
  
  xSizeWin = GUI_MEMDEV_GetXSize(hMemWin);
  ySizeWin = GUI_MEMDEV_GetYSize(hMemWin);
  pWin     = (U32 *)GUI_MEMDEV_GetDataPtr(hMemWin);
  pBk      = (U32 *)GUI_MEMDEV_GetDataPtr(hMemBk);
  pDst     = (U32 *)GUI_MEMDEV_GetDataPtr(hMemDst);
  xSizeBk  = GUI_MEMDEV_GetXSize(hMemBk);
  pBk     += (yPosWin * xSizeBk) + xPosWin;
  //
  // Iterate over all lines of window device
  //
  for (y = 0; y < ySizeWin; y++) {
    NumPixels = xSizeWin;
    do {
      //
      // Get colors
      //
      Color   = *pWin++;
      ColorBk = *pBk++;
      //
      // Calculate intensity
      //
      a = ((255 - (Color >> 24)) * Intens) / 255;
      //
      // Mix up colors
      //
      ColorDst = GUI__MixColors(Color, ColorBk, a) & 0xFFFFFF; // No alpha value after this operation!
      //
      // Save result and continue
      //
      *pDst++ = ColorDst;
    } while (--NumPixels);
    //
    // Move background pointer to the beginning of the next line
    //
    pBk += xSizeBk - xSizeWin;
  }
}

/*********************************************************************
*
*       GUI_MEMDEV__FadeDevice
*
* Purpose:
*   This function mixes up 2 memory devices into a destination device.
*
*   Please note: hMemWin and hMemDst have to have same size and position.
*
* Parameters:
*    hMemWin - Memory device to be mixed up with background
*    hMemBk  - Background to be used
*    hMemDst - Destination device
*    Intens  - Intensity of hMemWin
*/
void GUI_MEMDEV__FadeDevice(GUI_MEMDEV_Handle hMemWin, GUI_MEMDEV_Handle hMemBk, GUI_MEMDEV_Handle hMemDst, U8 Intens) {
  GUI_MEMDEV__FadeDeviceEx(hMemWin, hMemBk, hMemDst, Intens, 0, 0);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_FadeDevices
*/
int GUI_MEMDEV_FadeDevices(GUI_MEMDEV_Handle hMem0, GUI_MEMDEV_Handle hMem1, int Period) {
  int TimeStart, TimeNow, TimeDiff, TimeUsed, Intens;
  int xSize0, xSize1, ySize0, ySize1, xPos0, yPos0, xPos1, yPos1;
  int r;
  GUI_MEMDEV_Handle hMemWork;

  //
  // Check if size and position of hMem0 and hMem1 are the same
  //
  xSize0 = GUI_MEMDEV_GetXSize(hMem0);
  xSize1 = GUI_MEMDEV_GetXSize(hMem1);
  if (xSize0 != xSize1) {
    return 1;
  }
  ySize0 = GUI_MEMDEV_GetYSize(hMem0);
  ySize1 = GUI_MEMDEV_GetYSize(hMem1);
  if (ySize0 != ySize1) {
    return 1;
  }
  xPos0 = GUI_MEMDEV_GetXPos(hMem0);
  xPos1 = GUI_MEMDEV_GetXPos(hMem1);
  if (xPos0 != xPos1) {
    return 1;
  }
  yPos0 = GUI_MEMDEV_GetYPos(hMem0);
  yPos1 = GUI_MEMDEV_GetYPos(hMem1);
  if (yPos0 != yPos1) {
    return 1;
  }
  //
  // Create working device
  //
  GUI_LOCK();
  hMemWork = GUI_MEMDEV_CreateFixed(xPos0, yPos0, xSize0, ySize0, GUI_MEMDEV_NOTRANS, GUI_MEMDEV_APILIST_32, GUI_COLOR_CONV_888);
  if (hMemWork) {
    //
    // Do animation
    //
    TimeStart = GUI_GetTime();
    GUI_X_Delay(TIME_PER_FRAME);
    do {
      //
      // Calculate elapsed time
      //
      TimeNow  = GUI_GetTime();
      TimeDiff = TimeNow - TimeStart;
      TimeDiff = (TimeDiff > Period) ? Period : TimeDiff;
      //
      // Calculate parameters
      //
      Intens = (TimeDiff * 255) / Period;
      Intens = (Intens > 255) ? 255 : Intens;
      //
      // Fade device
      //
      GUI_MEMDEV__FadeDevice(hMem0, hMem1, hMemWork, Intens);
      //
      // Draw memory device
      //
      GUI_MEMDEV_CopyToLCD(hMemWork);
      //
      // Make sure not more than 25 frames per second are drawn
      //
      TimeUsed = GUI_GetTime() - TimeNow;
      if (TimeUsed < TIME_PER_FRAME) {
        GUI_X_Delay(TIME_PER_FRAME - TimeUsed);
      }
    } while (TimeNow < (TimeStart + Period));
    GUI_MEMDEV_Delete(hMemWork);
    r = 0;
  } else {
    r = 1;
  }
  GUI_UNLOCK();
  return r;
}

#else

void GUIDEV_MoveAndFade_C(void);
void GUIDEV_MoveAndFade_C(void) {} /* avoid empty object files */

#endif

/*************************** end of file ****************************/
