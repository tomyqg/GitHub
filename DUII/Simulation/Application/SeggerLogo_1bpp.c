/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2009  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.00 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : SeggerLogo_1bpp.c
Purpose     : Bitmap file
---------------------------END-OF-HEADER------------------------------
*/

#include "stdlib.h"

#include "GUI.h"


const unsigned char acSeggerLogo_1bpp[] = {
   ________,________,________,________,________,________,________,________,
   ____XXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXX____,
   ___XXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXX___,
   __XXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXX__,
   __XXX___,________,________,________,________,________,________,___XXX__,
   __XXX___,________,________,________,________,________,________,___XXX__,
   __XXX___,________,________,________,________,________,________,___XXX__,
   __XXX___,________,________,________,________,________,________,___XXX__,
   __XXX_X_,_XX_____,________,________,________,________,________,___XXX__,
   __XXX_XX,__XX____,________,________,________,________,________,___XXX__,
   __XXX__X,X__XX___,________,________,________,________,________,___XXX__,
   __XXX___,XX__XX__,________,________,________,________,________,___XXX__,
   __XXX_X_,_XX__XXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,___XXX__,
   __XXX_XX,__XX__XX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,___XXX__,
   __XXX_XX,X__XX___,________,________,________,________,________,___XXX__,
   __XXX_XX,__XX__XX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,___XXX__,
   __XXX_X_,_XX__XXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,___XXX__,
   __XXX___,XX__XX__,________,________,________,________,________,___XXX__,
   __XXX__X,X__XX___,_XX__XXX,X__XX___,XX__XXXX,_XXX____,________,___XXX__,
   __XXX_XX,__XX____,X__X_X__,__X__X_X,__X_X___,_X__X___,________,___XXX__,
   __XXX_X_,_XX_____,_X___XXX,__X____X,____XXX_,_XXX____,________,___XXX__,
   __XXX___,________,__X__X__,__X_XX_X,_XX_X___,_XX_____,________,___XXX__,
   __XXX___,________,X__X_X__,__X__X_X,__X_X___,_X_X____,________,___XXX__,
   __XXX___,________,_XX__XXX,X_XXXX_X,XXX_XXXX,_X__X___,________,___XXX__,
   __XXX___,________,________,________,________,________,________,___XXX__,
   __XXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXX__,
   ___XXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXX___,
   ____XXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXX____,
   ________,________,________,________,________,________,________,________,
};

const GUI_BITMAP bmSeggerLogo_1bpp = {
 63, /* XSize */
 29, /* YSize */
 8, /* BytesPerLine */
 1, /* BitsPerPixel */
 acSeggerLogo_1bpp,  /* Pointer to picture data (indices) */
};

/* *** End of file *** */
