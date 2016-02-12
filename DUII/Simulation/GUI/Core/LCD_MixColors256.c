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
File        : LCD_MixColors256.c
Purpose     : Implementation of LCD_MixColors256
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_MixColors256
*
* Purpose:
*   Mix 2 colors.
*
* Parameters:
*   Intens:    Intensity of first color in 257 steps, from 0 to 256, where 256 equals 100%
*/
LCD_COLOR LCD_MixColors256(LCD_COLOR Color, LCD_COLOR BkColor, unsigned Intens) {
  #if 1
    LCD_COLOR r;
    r = GUI__MixColors(Color, BkColor, Intens) & 0xFFFFFF;
    return r;
  #else
    /* Calc Color seperations for FgColor first */
    U32 R = (Color & 0xff)    * Intens;
    U32 G = (Color & 0xff00)  * Intens;
    U32 B = (Color & 0xff0000)* Intens;
    /* Add Color seperations for BkColor */
    Intens = 256 - Intens;
    R += (BkColor & 0xff)     * Intens;
    G += (BkColor & 0xff00)   * Intens;
    B += (BkColor & 0xff0000) * Intens;
    R = (R >> 8);
    G = (G >> 8) & 0xff00;
    B = (B >> 8) & 0xff0000;
    return R + G + B;
  #endif
}

/*************************** End of file ****************************/
