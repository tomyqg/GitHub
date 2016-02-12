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
File        : GUI_MOUSE_DriverPS2
Purpose     : GUI driver for PS2 mouse
----------------------------------------------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static int  _ScreenX              = 0;    /* x-pos              */
static int  _ScreenY              = 0;    /* y-pos              */
static int  _NumBytesInBuffer     = 0;    /* bytes in rx buffer */
static U8   _Buttons              = 0;    /* button status      */
static U8   _abInBuffer[3];               /* mouse rx buffer    */

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _EvaPacket
*
* Purpose:
*   Process data packet from mouse:
*
*             | D7    D6    D5    D4    D3    D2    D1    D0
*   ----------+----------------------------------------------
*   1st byte  | --    --    Y-    X-     1    --    LB    RB
*   2nd byte  | X7    X6    X5    X4    X3    X2    X1    X0
*   3rd byte  | Y7    Y6    Y5    Y4    Y3    Y2    Y1    Y0
*/
static void _EvaPacket(void) {
  char a;
  int xSize, ySize;
  GUI_PID_STATE State;
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  _Buttons = _abInBuffer[0] & 0x03;
  a = _abInBuffer[1];
  /* test x move sign. */
  if(_abInBuffer[0] & 0x10) {
    a=-a;
    _ScreenX  -= a;
  }        /* direction is negative, move left */
  else {
    _ScreenX  += a;
  }
  a = _abInBuffer[2];
  /* test y move sign. */
  if(_abInBuffer[0] & 0x20) {
    a=-a;
    _ScreenY  += a;
  }  /* direction is negative, move down */ else {
    _ScreenY  -= a;
  }
  /* check min/max positions */    
  if (_ScreenX < 0) {
    _ScreenX = 0;
  } else if (_ScreenX > xSize-1) {
    _ScreenX = xSize-1;
  } if (_ScreenY < 0) {
    _ScreenY = 0;
  } else if (_ScreenY > ySize-1) {
    _ScreenY = ySize-1;
  }
  /* signal new mouse data */
  State.x       = _ScreenX;
  State.y       = _ScreenY;
  State.Pressed = _Buttons;
  GUI_MOUSE_StoreState(&State);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MOUSE_DRIVER_PS2_OnRx
*
* Purpose:
*   Mouse receive interrupt handler. The PS2 mouse interrupt gets
*   in three bytes from the mouse, then wakes up the mouse LSR.
*/
void GUI_MOUSE_DRIVER_PS2_OnRx(unsigned char Data) {
  if (!_NumBytesInBuffer) {
    /* check for start frame */
    if ((Data & 0x0c) == 0x08) {
      _abInBuffer[0] = Data;
      _NumBytesInBuffer++;
    }
  } else {
    _abInBuffer[_NumBytesInBuffer] = Data;
    _NumBytesInBuffer++;
    if (_NumBytesInBuffer >= 3) {
      _EvaPacket();
      _NumBytesInBuffer = 0;
    }
  }
}

/*********************************************************************
*
*       GUI_MOUSE_DRIVER_PS2_Init
*/
void GUI_MOUSE_DRIVER_PS2_Init(void) {
  _NumBytesInBuffer = 0; 
}

/*************************** End of file ****************************/
