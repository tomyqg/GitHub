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
File        : F8x16_ASCII.C
Purpose     : Monospaced Font similar to Terminal
Height      : 8
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_FontIntern.h"

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0020[16] = { /* code 0020 */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char ' '  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0021[16] = { /* code 0021 */
   ________,
   ________,
   ___XX___,
   __XXXX__,
   __XXXX__,
   __XXXX__,
   ___XX___,
   ___XX___,
   ___XX___,
   ________,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ________}; /* char '!'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0022[16] = { /* code 0022 */
   ________,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   __X__X__,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char '"'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0023[16] = { /* code 0023 */
   ________,
   ________,
   ________,
   _XX_XX__,
   _XX_XX__,
   XXXXXXX_,
   _XX_XX__,
   _XX_XX__,
   _XX_XX__,
   XXXXXXX_,
   _XX_XX__,
   _XX_XX__,
   ________,
   ________,
   ________,
   ________}; /* char '#'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0024[16] = { /* code 0024 */
   ___XX___,
   ___XX___,
   _XXXXX__,
   XX___XX_,
   XX____X_,
   XX______,
   _XXXXX__,
   _____XX_,
   _____XX_,
   X____XX_,
   XX___XX_,
   _XXXXX__,
   ___XX___,
   ___XX___,
   ________,
   ________}; /* char '$'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0025[16] = { /* code 0025 */
   ________,
   ________,
   ________,
   ________,
   XX____X_,
   XX___XX_,
   ____XX__,
   ___XX___,
   __XX____,
   _XX_____,
   XX___XX_,
   X____XX_,
   ________,
   ________,
   ________,
   ________}; /* char '%'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0026[16] = { /* code 0026 */
   ________,
   ________,
   __XXX___,
   _XX_XX__,
   _XX_XX__,
   __XXX___,
   _XXX_XX_,
   XX_XXX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXX_XX_,
   ________,
   ________,
   ________,
   ________}; /* char '&'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0027[16] = { /* code 0027 */
   ________,
   __XX____,
   __XX____,
   __XX____,
   _XX_____,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char '''  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0028[16] = { /* code 0028 */
   ________,
   ________,
   ____XX__,
   ___XX___,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   ___XX___,
   ____XX__,
   ________,
   ________,
   ________,
   ________}; /* char '('  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0029[16] = { /* code 0029 */
   ________,
   ________,
   __XX____,
   ___XX___,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   ___XX___,
   __XX____,
   ________,
   ________,
   ________,
   ________}; /* char ')'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_002A[16] = { /* code 002A */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XX__XX_,
   __XXXX__,
   XXXXXXXX,
   __XXXX__,
   _XX__XX_,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char '*'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_002B[16] = { /* code 002B */
   ________,
   ________,
   ________,
   ________,
   ________,
   ___XX___,
   ___XX___,
   _XXXXXX_,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char '+'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_002C[16] = { /* code 002C */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ___XX___,
   ___XX___,
   ___XX___,
   __XX____,
   ________,
   ________,
   ________}; /* char ','  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_002D[16] = { /* code 002D */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char '-'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_002E[16] = { /* code 002E */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ________}; /* char '.'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_002F[16] = { /* code 002F */
   ________,
   ________,
   ________,
   ________,
   ______X_,
   _____XX_,
   ____XX__,
   ___XX___,
   __XX____,
   _XX_____,
   XX______,
   X_______,
   ________,
   ________,
   ________,
   ________}; /* char '/'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0030[16] = { /* code 0030 */
   ________,
   ________,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XX_XX__,
   __XXX___,
   ________,
   ________,
   ________,
   ________}; /* char '0'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0031[16] = { /* code 0031 */
   ________,
   ________,
   ___XX___,
   __XXX___,
   _XXXX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   _XXXXXX_,
   ________,
   ________,
   ________,
   ________}; /* char '1'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0032[16] = { /* code 0032 */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   _____XX_,
   ____XX__,
   ___XX___,
   __XX____,
   _XX_____,
   XX______,
   XX___XX_,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________}; /* char '2'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0033[16] = { /* code 0033 */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   _____XX_,
   _____XX_,
   __XXXX__,
   _____XX_,
   _____XX_,
   _____XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char '3'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0034[16] = { /* code 0034 */
   ________,
   ________,
   ____XX__,
   ___XXX__,
   __XXXX__,
   _XX_XX__,
   XX__XX__,
   XXXXXXX_,
   ____XX__,
   ____XX__,
   ____XX__,
   ___XXXX_,
   ________,
   ________,
   ________,
   ________}; /* char '4'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0035[16] = { /* code 0035 */
   ________,
   ________,
   XXXXXXX_,
   XX______,
   XX______,
   XX______,
   XXXXXX__,
   _____XX_,
   _____XX_,
   _____XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char '5'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0036[16] = { /* code 0036 */
   ________,
   ________,
   __XXX___,
   _XX_____,
   XX______,
   XX______,
   XXXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char '6'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0037[16] = { /* code 0037 */
   ________,
   ________,
   XXXXXXX_,
   XX___XX_,
   _____XX_,
   _____XX_,
   ____XX__,
   ___XX___,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   ________,
   ________,
   ________,
   ________}; /* char '7'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0038[16] = { /* code 0038 */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char '8'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0039[16] = { /* code 0039 */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXXX_,      
   _____XX_,
   _____XX_,
   _____XX_,
   ____XX__,
   _XXXX___,
   ________,
   ________,
   ________,
   ________}; /* char '9'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_003A[16] = { /* code 003A */
   ________,
   ________,
   ________,
   ________,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char ':'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_003B[16] = { /* code 003B */
   ________,
   ________,
   ________,
   ________,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ___XX___,
   ___XX___,
   __XX____,
   ________,
   ________,
   ________,
   ________}; /* char ';'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_003C[16] = { /* code 003C */
   ________,
   ________,
   ________,
   _____XX_,
   ____XX__,
   ___XX___,
   __XX____,
   _XX_____,
   __XX____,
   ___XX___,
   ____XX__,
   _____XX_,
   ________,
   ________,
   ________,
   ________}; /* char '<'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_003D[16] = { /* code 003D */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XXXXXX_,
   ________,
   ________,
   _XXXXXX_,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char '='  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_003E[16] = { /* code 003E */
   ________,
   ________,
   ________,
   _XX_____,
   __XX____,
   ___XX___,
   ____XX__,
   _____XX_,
   ____XX__,
   ___XX___,
   __XX____,
   _XX_____,
   ________,
   ________,
   ________,
   ________}; /* char '>'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_003F[16] = { /* code 003F */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   ____XX__,
   ___XX___,
   ___XX___,
   ___XX___,
   ________,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ________}; /* char '?'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0040[16] = { /* code 0040 */
   ________,
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX_XXXX_,
   XX_XXXX_,
   XX_XXXX_,
   XX_XXX__,
   XX______,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char '@'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0041[16] = { /* code 0041 */
   ________,
   ________,
   ___X____,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   XX___XX_,
   XXXXXXX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'A'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0042[16] = { /* code 0042 */
   ________,
   ________,
   XXXXXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XXXXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   XXXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'B'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0043[16] = { /* code 0043 */
   ________,
   ________,
   __XXXX__,
   _XX__XX_,
   XX____X_,
   XX______,
   XX______,
   XX______,
   XX______,
   XX____X_,
   _XX__XX_,
   __XXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'C'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0044[16] = { /* code 0044 */
   ________,
   ________,
   XXXXX___,
   _XX_XX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX_XX__,
   XXXXX___,
   ________,
   ________,
   ________,
   ________}; /* char 'D'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0045[16] = { /* code 0045 */
   ________,
   ________,
   XXXXXXX_,
   _XX__XX_,
   _XX___X_,
   _XX_X___,
   _XXXX___,
   _XX_X___,
   _XX_____,
   _XX___X_,
   _XX__XX_,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________}; /* char 'E'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0046[16] = { /* code 0046 */
   ________,
   ________,
   XXXXXXX_,
   _XX__XX_,
   _XX___X_,
   _XX_X___,
   _XXXX___,
   _XX_X___,
   _XX_____,
   _XX_____,
   _XX_____,
   XXXX____,
   ________,
   ________,
   ________,
   ________}; /* char 'F'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0047[16] = { /* code 0047 */
   ________,
   ________,
   __XXXX__,
   _XX__XX_,
   XX____X_,
   XX______,
   XX______,
   XX_XXXX_,
   XX___XX_,
   XX___XX_,
   _XX__XX_,
   __XXX_X_,
   ________,
   ________,
   ________,
   ________}; /* char 'G'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0048[16] = { /* code 0048 */
   ________,
   ________,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XXXXXXX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'H'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0049[16] = { /* code 0049 */
   ________,
   ________,
   __XXXX__,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'I'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_004A[16] = { /* code 004A */
   ________,
   ________,
   ___XXXX_,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXXX___,
   ________,
   ________,
   ________,
   ________}; /* char 'J'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_004B[16] = { /* code 004B */
   ________,
   ________,
   XXX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX_XX__,
   _XXXX___,
   _XXXX___,
   _XX_XX__,
   _XX__XX_,
   _XX__XX_,
   XXX__XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'K'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_004C[16] = { /* code 004C */
   ________,
   ________,
   XXXX____,
   _XX_____,
   _XX_____,
   _XX_____,
   _XX_____,
   _XX_____,
   _XX_____,
   _XX___X_,
   _XX__XX_,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________}; /* char 'L'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_004D[16] = { /* code 004D */
   ________,
   ________,
   XX___XX_,
   XXX_XXX_,
   XXXXXXX_,
   XXXXXXX_,
   XX_X_XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'M'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_004E[16] = { /* code 004E */
   ________,
   ________,
   XX___XX_,
   XXX__XX_,
   XXXX_XX_,
   XXXXXXX_,
   XX_XXXX_,
   XX__XXX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'N'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_004F[16] = { /* code 004F */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'O'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0050[16] = { /* code 0050 */
   ________,
   ________,
   XXXXXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XXXXX__,
   _XX_____,
   _XX_____,
   _XX_____,
   _XX_____,
   XXXX____,
   ________,
   ________,
   ________,
   ________}; /* char 'P'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0051[16] = { /* code 0051 */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX_X_XX_,
   XX_XXXX_,
   _XXXXX__,
   ____XX__,
   ____XXX_,
   ________,
   ________}; /* char 'Q'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0052[16] = { /* code 0052 */
   ________,
   ________,
   XXXXXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XXXXX__,
   _XX_XX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   XXX__XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'R'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0053[16] = { /* code 0053 */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   _XX_____,
   __XXX___,
   ____XX__,
   _____XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'S'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0054[16] = { /* code 0054 */
   ________,
   ________,
   _XXXXXX_,
   _XXXXXX_,
   _X_XX_X_,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'T'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0055[16] = { /* code 0055 */
   ________,
   ________,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'U'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0056[16] = { /* code 0056 */
   ________,
   ________,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XX_XX__,
   __XXX___,
   ___X____,
   ________,
   ________,
   ________,
   ________}; /* char 'V'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0057[16] = { /* code 0057 */
   ________,
   ________,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX_X_XX_,
   XX_X_XX_,
   XX_X_XX_,
   XXXXXXX_,
   XXX_XXX_,
   _XX_XX__,
   ________,
   ________,
   ________,
   ________}; /* char 'W'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0058[16] = { /* code 0058 */
   ________,
   ________,
   XX___XX_,
   XX___XX_,
   _XX_XX__,
   _XXXXX__,
   __XXX___,
   __XXX___,
   _XXXXX__,
   _XX_XX__,
   XX___XX_,
   XX___XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'X'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0059[16] = { /* code 0059 */
   ________,
   ________,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   __XXXX__,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'Y'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_005A[16] = { /* code 005A */
   ________,
   ________,
   XXXXXXX_,
   XX___XX_,
   X____XX_,
   ____XX__,
   ___XX___,
   __XX____,
   _XX_____,
   XX____X_,
   XX___XX_,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________}; /* char 'Z'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_005B[16] = { /* code 005B */
   ________,
   ________,
   __XXXX__,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   __XXXX__,
   ________,
   ________,
   ________,
   ________}; /* char '['  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_005C[16] = { /* code 005C */
   ________,
   ________,
   ________,
   X_______,
   XX______,
   XXX_____,
   _XXX____,
   __XXX___,
   ___XXX__,
   ____XXX_,
   _____XX_,
   ______X_,
   ________,
   ________,
   ________,
   ________}; /* char '\'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_005D[16] = { /* code 005D */
   ________,
   ________,
   __XXXX__,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   ____XX__,
   __XXXX__,
   ________,
   ________,
   ________,
   ________}; /* char ']'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_005E[16] = { /* code 005E */
   ___X____,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char '^'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_005F[16] = { /* code 005F */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   XXXXXXXX,
   ________,
   ________}; /* char '_'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0060[16] = { /* code 0060 */
   ________,
   __XX____,
   ___XX___,
   ____XX__,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}; /* char '`'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0061[16] = { /* code 0061 */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XXXX___,
   ____XX__,
   _XXXXX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXX_XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'a'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0062[16] = { /* code 0062 */
   ________,
   ________,
   XXX_____,
   _XX_____,
   _XX_____,
   _XXXX___,
   _XX_XX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'b'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0063[16] = { /* code 0063 */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX______,
   XX______,
   XX______,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'c'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0064[16] = { /* code 0064 */
   ________,
   ________,
   ___XXX__,
   ____XX__,
   ____XX__,
   __XXXX__,
   _XX_XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXX_XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'd'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0065[16] = { /* code 0065 */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XXXXXXX_,
   XX______,
   XX______,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'e'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0066[16] = { /* code 0066 */
   ________,
   ________,
   ___XXX__,
   __XX_XX_,
   __XX__X_,
   __XX____,
   _XXXX___,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   _XXXX___,
   ________,
   ________,
   ________,
   ________}; /* char 'f'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0067[16] = { /* code 0067 */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XXX_XX_,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXXXX__,
   ____XX__,
   XX__XX__,
   _XXXX___,
   ________}; /* char 'g'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0068[16] = { /* code 0068 */
   ________,
   ________,
   XXX_____,
   _XX_____,
   _XX_____,
   _XX_XX__,
   _XXX_XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   XXX__XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'h'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0069[16] = { /* code 0069 */
   ________,
   ________,
   ___XX___,
   ___XX___,
   ________,
   __XXX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'i'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_006A[16] = { /* code 006A */
   ________,
   ________,
   _____XX_,
   _____XX_,
   ________,
   ____XXX_,
   _____XX_,
   _____XX_,
   _____XX_,
   _____XX_,
   _____XX_,
   _____XX_,
   _XX__XX_,
   _XX__XX_,
   __XXXX__,
   ________}; /* char 'j'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_006B[16] = { /* code 006B */
   ________,
   ________,
   XXX_____,
   _XX_____,
   _XX_____,
   _XX__XX_,
   _XX_XX__,
   _XXXX___,
   _XXXX___,
   _XX_XX__,
   _XX__XX_,
   XXX__XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'k'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_006C[16] = { /* code 006C */
   ________,
   ________,
   __XXX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'l'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_006D[16] = { /* code 006D */
   ________,
   ________,
   ________,
   ________,
   ________,
   XXX_XX__,
   XXXXXXX_,
   XX_X_XX_,
   XX_X_XX_,
   XX_X_XX_,
   XX_X_XX_,
   XX___XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'm'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_006E[16] = { /* code 006E */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX_XXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'n'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_006F[16] = { /* code 006F */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 'o'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0070[16] = { /* code 0070 */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX_XXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XXXXX__,
   _XX_____,
   _XX_____,
   XXXX____,
   ________}; /* char 'p'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0071[16] = { /* code 0071 */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XXX_XX_,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXXXX__,
   ____XX__,
   ____XX__,
   ___XXXX_,
   ________}; /* char 'q'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0072[16] = { /* code 0072 */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX_XXX__,
   _XXX_XX_,
   _XX__XX_,
   _XX_____,
   _XX_____,
   _XX_____,
   XXXX____,
   ________,
   ________,
   ________,
   ________}; /* char 'r'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0073[16] = { /* code 0073 */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   _XX_____,
   __XXX___,
   ____XX__,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________}; /* char 's'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0074[16] = { /* code 0074 */
   ________,
   ________,
   ___X____,
   __XX____,
   __XX____,
   XXXXXX__,
   __XX____,
   __XX____,
   __XX____,
   __XX____,
   __XX_XX_,
   ___XXX__,
   ________,
   ________,
   ________,
   ________}; /* char 't'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0075[16] = { /* code 0075 */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXX_XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'u'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0076[16] = { /* code 0076 */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XX_XX__,
   __XXX___,
   ________,
   ________,
   ________,
   ________}; /* char 'v'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0077[16] = { /* code 0077 */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX___XX_,
   XX___XX_,
   XX_X_XX_,
   XX_X_XX_,
   XX_X_XX_,
   XXXXXXX_,
   _XX_XX__,
   ________,
   ________,
   ________,
   ________}; /* char 'w'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0078[16] = { /* code 0078 */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX___XX_,
   _XX_XX__,
   __XXX___,
   __XXX___,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   ________,
   ________,
   ________,
   ________}; /* char 'x'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_0079[16] = { /* code 0079 */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXXX_,
   _____XX_,
   ____XX__,
   XXXXX___,
   ________}; /* char 'y'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_007A[16] = { /* code 007A */
   ________,
   ________,
   ________,
   ________,
   ________,
   XXXXXXX_,
   XX__XX__,
   ___XX___,
   __XX____,
   _XX_____,
   XX___XX_,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________}; /* char 'z'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_007B[16] = { /* code 007B */
   ________,
   ________,
   ____XXX_,
   ___XX___,
   ___XX___,
   ___XX___,
   _XXX____,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ____XXX_,
   ________,
   ________,
   ________,
   ________}; /* char '{'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_007C[16] = { /* code 007C */
   ________,
   ________,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ________}; /* char '|'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_007D[16] = { /* code 007D */
   ________,
   ________,
   _XXX____,
   ___XX___,
   ___XX___,
   ___XX___,
   ____XXX_,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   _XXX____,
   ________,
   ________,
   ________,
   ________}; /* char '}'  */

GUI_CONST_STORAGE unsigned char acFont8x16ASCII_007E[16] = { /* code 007E */
   ________,
   _XXX_XX_,
   XX_XXX__,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};  /* char '~'  */

GUI_CONST_STORAGE GUI_CHARINFO GUI_Font8x16ASCII_CharInfo[95] = {
   {   8,   8,  1, acFont8x16ASCII_0020 } /* code 0020 */
  ,{   8,   8,  1, acFont8x16ASCII_0021 } /* code 0021 */
  ,{   8,   8,  1, acFont8x16ASCII_0022 } /* code 0022 */
  ,{   8,   8,  1, acFont8x16ASCII_0023 } /* code 0023 */
  ,{   8,   8,  1, acFont8x16ASCII_0024 } /* code 0024 */
  ,{   8,   8,  1, acFont8x16ASCII_0025 } /* code 0025 */
  ,{   8,   8,  1, acFont8x16ASCII_0026 } /* code 0026 */
  ,{   8,   8,  1, acFont8x16ASCII_0027 } /* code 0027 */
  ,{   8,   8,  1, acFont8x16ASCII_0028 } /* code 0028 */
  ,{   8,   8,  1, acFont8x16ASCII_0029 } /* code 0029 */
  ,{   8,   8,  1, acFont8x16ASCII_002A } /* code 002A */
  ,{   8,   8,  1, acFont8x16ASCII_002B } /* code 002B */
  ,{   8,   8,  1, acFont8x16ASCII_002C } /* code 002C */
  ,{   8,   8,  1, acFont8x16ASCII_002D } /* code 002D */
  ,{   8,   8,  1, acFont8x16ASCII_002E } /* code 002E */
  ,{   8,   8,  1, acFont8x16ASCII_002F } /* code 002F */
  ,{   8,   8,  1, acFont8x16ASCII_0030 } /* code 0030 */
  ,{   8,   8,  1, acFont8x16ASCII_0031 } /* code 0031 */
  ,{   8,   8,  1, acFont8x16ASCII_0032 } /* code 0032 */
  ,{   8,   8,  1, acFont8x16ASCII_0033 } /* code 0033 */
  ,{   8,   8,  1, acFont8x16ASCII_0034 } /* code 0034 */
  ,{   8,   8,  1, acFont8x16ASCII_0035 } /* code 0035 */
  ,{   8,   8,  1, acFont8x16ASCII_0036 } /* code 0036 */
  ,{   8,   8,  1, acFont8x16ASCII_0037 } /* code 0037 */
  ,{   8,   8,  1, acFont8x16ASCII_0038 } /* code 0038 */
  ,{   8,   8,  1, acFont8x16ASCII_0039 } /* code 0039 */
  ,{   8,   8,  1, acFont8x16ASCII_003A } /* code 003A */
  ,{   8,   8,  1, acFont8x16ASCII_003B } /* code 003B */
  ,{   8,   8,  1, acFont8x16ASCII_003C } /* code 003C */
  ,{   8,   8,  1, acFont8x16ASCII_003D } /* code 003D */
  ,{   8,   8,  1, acFont8x16ASCII_003E } /* code 003E */
  ,{   8,   8,  1, acFont8x16ASCII_003F } /* code 003F */
  ,{   8,   8,  1, acFont8x16ASCII_0040 } /* code 0040 */
  ,{   8,   8,  1, acFont8x16ASCII_0041 } /* code 0041 */
  ,{   8,   8,  1, acFont8x16ASCII_0042 } /* code 0042 */
  ,{   8,   8,  1, acFont8x16ASCII_0043 } /* code 0043 */
  ,{   8,   8,  1, acFont8x16ASCII_0044 } /* code 0044 */
  ,{   8,   8,  1, acFont8x16ASCII_0045 } /* code 0045 */
  ,{   8,   8,  1, acFont8x16ASCII_0046 } /* code 0046 */
  ,{   8,   8,  1, acFont8x16ASCII_0047 } /* code 0047 */
  ,{   8,   8,  1, acFont8x16ASCII_0048 } /* code 0048 */
  ,{   8,   8,  1, acFont8x16ASCII_0049 } /* code 0049 */
  ,{   8,   8,  1, acFont8x16ASCII_004A } /* code 004A */
  ,{   8,   8,  1, acFont8x16ASCII_004B } /* code 004B */
  ,{   8,   8,  1, acFont8x16ASCII_004C } /* code 004C */
  ,{   8,   8,  1, acFont8x16ASCII_004D } /* code 004D */
  ,{   8,   8,  1, acFont8x16ASCII_004E } /* code 004E */
  ,{   8,   8,  1, acFont8x16ASCII_004F } /* code 004F */
  ,{   8,   8,  1, acFont8x16ASCII_0050 } /* code 0050 */
  ,{   8,   8,  1, acFont8x16ASCII_0051 } /* code 0051 */
  ,{   8,   8,  1, acFont8x16ASCII_0052 } /* code 0052 */
  ,{   8,   8,  1, acFont8x16ASCII_0053 } /* code 0053 */
  ,{   8,   8,  1, acFont8x16ASCII_0054 } /* code 0054 */
  ,{   8,   8,  1, acFont8x16ASCII_0055 } /* code 0055 */
  ,{   8,   8,  1, acFont8x16ASCII_0056 } /* code 0056 */
  ,{   8,   8,  1, acFont8x16ASCII_0057 } /* code 0057 */
  ,{   8,   8,  1, acFont8x16ASCII_0058 } /* code 0058 */
  ,{   8,   8,  1, acFont8x16ASCII_0059 } /* code 0059 */
  ,{   8,   8,  1, acFont8x16ASCII_005A } /* code 005A */
  ,{   8,   8,  1, acFont8x16ASCII_005B } /* code 005B */
  ,{   8,   8,  1, acFont8x16ASCII_005C } /* code 005C */
  ,{   8,   8,  1, acFont8x16ASCII_005D } /* code 005D */
  ,{   8,   8,  1, acFont8x16ASCII_005E } /* code 005E */
  ,{   8,   8,  1, acFont8x16ASCII_005F } /* code 005F */
  ,{   8,   8,  1, acFont8x16ASCII_0060 } /* code 0060 */
  ,{   8,   8,  1, acFont8x16ASCII_0061 } /* code 0061 */
  ,{   8,   8,  1, acFont8x16ASCII_0062 } /* code 0062 */
  ,{   8,   8,  1, acFont8x16ASCII_0063 } /* code 0063 */
  ,{   8,   8,  1, acFont8x16ASCII_0064 } /* code 0064 */
  ,{   8,   8,  1, acFont8x16ASCII_0065 } /* code 0065 */
  ,{   8,   8,  1, acFont8x16ASCII_0066 } /* code 0066 */
  ,{   8,   8,  1, acFont8x16ASCII_0067 } /* code 0067 */
  ,{   8,   8,  1, acFont8x16ASCII_0068 } /* code 0068 */
  ,{   8,   8,  1, acFont8x16ASCII_0069 } /* code 0069 */
  ,{   8,   8,  1, acFont8x16ASCII_006A } /* code 006A */
  ,{   8,   8,  1, acFont8x16ASCII_006B } /* code 006B */
  ,{   8,   8,  1, acFont8x16ASCII_006C } /* code 006C */
  ,{   8,   8,  1, acFont8x16ASCII_006D } /* code 006D */
  ,{   8,   8,  1, acFont8x16ASCII_006E } /* code 006E */
  ,{   8,   8,  1, acFont8x16ASCII_006F } /* code 006F */
  ,{   8,   8,  1, acFont8x16ASCII_0070 } /* code 0070 */
  ,{   8,   8,  1, acFont8x16ASCII_0071 } /* code 0071 */
  ,{   8,   8,  1, acFont8x16ASCII_0072 } /* code 0072 */
  ,{   8,   8,  1, acFont8x16ASCII_0073 } /* code 0073 */
  ,{   8,   8,  1, acFont8x16ASCII_0074 } /* code 0074 */
  ,{   8,   8,  1, acFont8x16ASCII_0075 } /* code 0075 */
  ,{   8,   8,  1, acFont8x16ASCII_0076 } /* code 0076 */
  ,{   8,   8,  1, acFont8x16ASCII_0077 } /* code 0077 */
  ,{   8,   8,  1, acFont8x16ASCII_0078 } /* code 0078 */
  ,{   8,   8,  1, acFont8x16ASCII_0079 } /* code 0079 */
  ,{   8,   8,  1, acFont8x16ASCII_007A } /* code 007A */
  ,{   8,   8,  1, acFont8x16ASCII_007B } /* code 007B */
  ,{   8,   8,  1, acFont8x16ASCII_007C } /* code 007C */
  ,{   8,   8,  1, acFont8x16ASCII_007D } /* code 007D */
  ,{   8,   8,  1, acFont8x16ASCII_007E } /* code 007E */
};

GUI_CONST_STORAGE GUI_FONT_PROP GUI_Font8x16ASCII_Prop0 = {
   32                             /* first character               */
  ,126                            /* last character                */
  ,&GUI_Font8x16ASCII_CharInfo[0] /* address of first character    */
  ,(const GUI_FONT_PROP*)0        /* pointer to next GUI_FONT_PROP */
};

GUI_CONST_STORAGE GUI_FONT GUI_Font8x16_ASCII = {
   GUI_FONTTYPE_PROP              /* type of font    */
  ,16                             /* height of font  */
  ,16                             /* space of font y */
  , 1                             /* magnification x */
  , 1                             /* magnification y */
  ,{&GUI_Font8x16ASCII_Prop0}
  ,14
  , 7
  ,14
};

/*************************** End of file ****************************/
