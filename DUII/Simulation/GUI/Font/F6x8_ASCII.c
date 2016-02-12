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
File        : F6x8_ASCII.C
Purpose     : Monospaced Font similar to Terminal
Height      : 8
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_FontIntern.h"

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0020[8] = { /* code 0020 */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};  /* char ' '  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0021[8] = { /* code 0021 */
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   ________,
   __X_____,
   ________};  /* char '!'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0022[8] = { /* code 0022 */
   _X__X___,
   _X__X___,
   _X__X___,
   ________,
   ________,
   ________,
   ________,
   ________};  /* char '"'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0023[8] = { /* code 0023 */
   _X_X____,
   _X_X____,
   XXXXX___,
   _X_X____,
   XXXXX___,
   _X_X____,
   _X_X____,
   ________};  /* char '#'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0024[8] = { /* code 0024 */
   __X_____,
   _XXXX___,
   X_X_____,
   _XXX____,
   __X_X___,
   XXXX____,
   __X_____,
   ________};  /* char '$'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0025[8] = { /* code 0025 */
   XX______,
   XX__X___,
   ___X____,
   __X_____,
   _X______,
   X__XX___,
   ___XX___,
   ________};  /* char '%'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0026[8] = { /* code 0026 */
   _XX_____,
   X__X____,
   X_X_____,
   _X______,
   X_X_X___,
   X__X____,
   _XX_X___,
   ________};  /* char '&'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0027[8] = { /* code 0027 */
   _XX_____,
   __X_____,
   _X______,
   ________,
   ________,
   ________,
   ________,
   ________};  /* char '''  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0028[8] = { /* code 0028 */
   ___X____,
   __X_____,
   _X______,
   _X______,
   _X______,
   __X_____,
   ___X____,
   ________};  /* char '('  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0029[8] = { /* code 0029 */
   _X______,
   __X_____,
   ___X____,
   ___X____,
   ___X____,
   __X_____,
   _X______,
   ________};  /* char ')'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_002A[8] = { /* code 002A */
   ________,
   _X_X____,
   __X_____,
   XXXXX___,
   __X_____,
   _X_X____,
   ________,
   ________};  /* char '*'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_002B[8] = { /* code 002B */
   ________,
   __X_____,
   __X_____,
   XXXXX___,
   __X_____,
   __X_____,
   ________,
   ________};  /* char '+'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_002C[8] = { /* code 002C */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XX_____,
   __X_____,
   _X______};  /* char ','  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_002D[8] = { /* code 002D */
   ________,
   ________,
   ________,
   XXXXX___,
   ________,
   ________,
   ________,
   ________};  /* char '-'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_002E[8] = { /* code 002E */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XX_____,
   _XX_____,
   ________};  /* char '.'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_002F[8] = { /* code 002F */
   ________,
   ____X___,
   ___X____,
   __X_____,
   _X______,
   X_______,
   ________,
   ________};  /* char '/'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0030[8] = { /* code 0030 */
   _XXX____,
   X___X___,
   X__XX___,
   X_X_X___,
   XX__X___,
   X___X___,
   _XXX____,
   ________};  /* char '0'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0031[8] = { /* code 0031 */
   __X_____,
   _XX_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};  /* char '1'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0032[8] = { /* code 0032 */
   _XXX____,
   X___X___,
   ____X___,
   __XX____,
   _X______,
   X_______,
   XXXXX___,
   ________};  /* char '2'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0033[8] = { /* code 0033 */
   _XXX____,
   X___X___,
   ____X___,
   __XX____,
   ____X___,
   X___X___,
   _XXX____,
   ________};  /* char '3'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0034[8] = { /* code 0034 */
   ___X____,
   __XX____,
   _X_X____,
   X__X____,
   XXXXX___,
   ___X____,
   ___X____,
   ________};  /* char '4'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0035[8] = { /* code 0035 */
   XXXXX___,
   X_______,
   XXXX____,
   ____X___,
   ____X___,
   X___X___,
   _XXX____,
   ________};  /* char '5'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0036[8] = { /* code 0036 */
   __XX____,
   _X______,
   X_______,
   XXXX____,
   X___X___,
   X___X___,
   _XXX____,
   ________};  /* char '6'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0037[8] = { /* code 0037 */
   XXXXX___,
   ____X___,
   ___X____,
   __X_____,
   _X______,
   _X______,
   _X______,
   ________};  /* char '7'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0038[8] = { /* code 0038 */
   _XXX____,
   X___X___,
   X___X___,
   _XXX____,
   X___X___,
   X___X___,
   _XXX____,
   ________};  /* char '8'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0039[8] = { /* code 0039 */
   _XXX____,
   X___X___,
   X___X___,
   _XXXX___,
   ____X___,
   ___X____,
   _XX_____,
   ________};  /* char '9'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_003A[8] = { /* code 003A */
   ________,
   _XX_____,
   _XX_____,
   ________,
   _XX_____,
   _XX_____,
   ________,
   ________};  /* char ':'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_003B[8] = { /* code 003B */
   ________,
   ________,
   _XX_____,
   _XX_____,
   ________,
   _XX_____,
   __X_____,
   _X______};  /* char ';'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_003C[8] = { /* code 003C */
   ___X____,
   __X_____,
   _X______,
   X_______,
   _X______,
   __X_____,
   ___X____,
   ________};  /* char '<'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_003D[8] = { /* code 003D */
   ________,
   ________,
   XXXXX___,
   ________,
   XXXXX___,
   ________,
   ________,
   ________};  /* char '='  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_003E[8] = { /* code 003E */
   X_______,
   _X______,
   __X_____,
   ___X____,
   __X_____,
   _X______,
   X_______,
   ________};  /* char '>'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_003F[8] = { /* code 003F */
   _XXX____,
   X___X___,
   ____X___,
   ___X____,
   __X_____,
   ________,
   __X_____,
   ________};  /* char '?'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0040[8] = { /* code 0040 */
   _XXX____,
   X___X___,
   ____X___,
   _XX_X___,
   X_X_X___,
   X_X_X___,
   _XXX____,
   ________};  /* char '@'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0041[8] = { /* code 0041 */
   _XXX____,
   X___X___,
   X___X___,
   XXXXX___,
   X___X___,
   X___X___,
   X___X___,
   ________};  /* char 'A'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0042[8] = { /* code 0042 */
   XXXX____,
   X___X___,
   X___X___,
   XXXX____,
   X___X___,
   X___X___,
   XXXX____,
   ________};  /* char 'B'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0043[8] = { /* code 0043 */
   _XXX____,
   X___X___,
   X_______,
   X_______,
   X_______,
   X___X___,
   _XXX____,
   ________};  /* char 'C'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0044[8] = { /* code 0044 */
   XXX_____,
   X__X____,
   X___X___,
   X___X___,
   X___X___,
   X__X____,
   XXX_____,
   ________};  /* char 'D'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0045[8] = { /* code 0045 */
   XXXXX___,
   X_______,
   X_______,
   XXXX____,
   X_______,
   X_______,
   XXXXX___,
   ________};  /* char 'E'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0046[8] = { /* code 0046 */
   XXXXX___,
   X_______,
   X_______,
   XXXX____,
   X_______,
   X_______,
   X_______,
   ________};  /* char 'F'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0047[8] = { /* code 0047 */
   _XXX____,
   X___X___,
   X_______,
   X_______,
   X__XX___,
   X___X___,
   _XXXX___,
   ________};  /* char 'G'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0048[8] = { /* code 0048 */
   X___X___,
   X___X___,
   X___X___,
   XXXXX___,
   X___X___,
   X___X___,
   X___X___,
   ________};  /* char 'H'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0049[8] = { /* code 0049 */
   _XXX____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};  /* char 'I'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_004A[8] = { /* code 004A */
   __XXX___,
   ___X____,
   ___X____,
   ___X____,
   ___X____,
   X__X____,
   _XX_____,
   ________};  /* char 'J'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_004B[8] = { /* code 004B */
   X___X___,
   X__X____,
   X_X_____,
   XX______,
   X_X_____,
   X__X____,
   X___X___,
   ________};  /* char 'K'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_004C[8] = { /* code 004C */
   X_______,
   X_______,
   X_______,
   X_______,
   X_______,
   X_______,
   XXXXX___,
   ________};  /* char 'L'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_004D[8] = { /* code 004D */
   X___X___,
   XX_XX___,
   X_X_X___,
   X_X_X___,
   X___X___,
   X___X___,
   X___X___,
   ________};  /* char 'M'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_004E[8] = { /* code 004E */
   X___X___,
   X___X___,
   XX__X___,
   X_X_X___,
   X__XX___,
   X___X___,
   X___X___,
   ________};  /* char 'N'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_004F[8] = { /* code 004F */
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};  /* char 'O'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0050[8] = { /* code 0050 */
   XXXX____,
   X___X___,
   X___X___,
   XXXX____,
   X_______,
   X_______,
   X_______,
   ________};  /* char 'P'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0051[8] = { /* code 0051 */
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   X_X_X___,
   X__X____,
   _XX_X___,
   ________};  /* char 'Q'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0052[8] = { /* code 0052 */
   XXXX____,
   X___X___,
   X___X___,
   XXXX____,
   X_X_____,
   X__X____,
   X___X___,
   ________};  /* char 'R'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0053[8] = { /* code 0053 */
   _XXX____,
   X___X___,
   X_______,
   _XXX____,
   ____X___,
   X___X___,
   _XXX____,
   ________};  /* char 'S'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0054[8] = { /* code 0054 */
   XXXXX___,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   ________};  /* char 'T'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0055[8] = { /* code 0055 */
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};  /* char 'U'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0056[8] = { /* code 0056 */
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   _X_X____,
   __X_____,
   ________};  /* char 'V'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0057[8] = { /* code 0057 */
   X___X___,
   X___X___,
   X___X___,
   X_X_X___,
   X_X_X___,
   X_X_X___,
   _X_X____,
   ________};  /* char 'W'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0058[8] = { /* code 0058 */
   X___X___,
   X___X___,
   _X_X____,
   __X_____,
   _X_X____,
   X___X___,
   X___X___,
   ________};  /* char 'X'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0059[8] = { /* code 0059 */
   X___X___,
   X___X___,
   X___X___,
   _X_X____,
   __X_____,
   __X_____,
   __X_____,
   ________};  /* char 'Y'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_005A[8] = { /* code 005A */
   XXXXX___,
   ____X___,
   ___X____,
   __X_____,
   _X______,
   X_______,
   XXXXX___,
   ________};  /* char 'Z'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_005B[8] = { /* code 005B */
   _XXX____,
   _X______,
   _X______,
   _X______,
   _X______,
   _X______,
   _XXX____,
   ________};  /* char '['  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_005C[8] = { /* code 005C */
   ________,
   X_______,
   _X______,
   __X_____,
   ___X____,
   ____X___,
   ________,
   ________};  /* char '\'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_005D[8] = { /* code 005D */
   _XXX____,
   ___X____,
   ___X____,
   ___X____,
   ___X____,
   ___X____,
   _XXX____,
   ________};  /* char ']'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_005E[8] = { /* code 005E */
   __X_____,
   _X_X____,
   X___X___,
   ________,
   ________,
   ________,
   ________,
   ________};  /* char '^'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_005F[8] = { /* code 005F */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   XXXXX___};  /* char '_'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0060[8] = { /* code 0060 */
   _X______,
   __X_____,
   ___X____,
   ________,
   ________,
   ________,
   ________,
   ________};  /* char '`'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0061[8] = { /* code 0061 */
   ________,
   ________,
   _XXX____,
   ____X___,
   _XXXX___,
   X___X___,
   _XXXX___,
   ________};  /* char 'a'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0062[8] = { /* code 0062 */
   X_______,
   X_______,
   X_XX____,
   XX__X___,
   X___X___,
   X___X___,
   XXXX____,
   ________};  /* char 'b'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0063[8] = { /* code 0063 */
   ________,
   ________,
   _XXX____,
   X_______,
   X_______,
   X___X___,
   _XXX____,
   ________};  /* char 'c'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0064[8] = { /* code 0064 */
   ____X___,
   ____X___,
   _XX_X___,
   X__XX___,
   X___X___,
   X___X___,
   _XXXX___,
   ________};  /* char 'd'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0065[8] = { /* code 0065 */
   ________,
   ________,
   _XXX____,
   X___X___,
   XXXXX___,
   X_______,
   _XXX____,
   ________};  /* char 'e'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0066[8] = { /* code 0066 */
   __XX____,
   _X__X___,
   _X______,
   XXX_____,
   _X______,
   _X______,
   _X______,
   ________};  /* char 'f'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0067[8] = { /* code 0067 */
   ________,
   ________,
   _XXXX___,
   X___X___,
   X___X___,
   _XXXX___,
   ____X___,
   _XXX____};  /* char 'g'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0068[8] = { /* code 0068 */
   X_______,
   X_______,
   X_XX____,
   XX__X___,
   X___X___,
   X___X___,
   X___X___,
   ________};  /* char 'h'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0069[8] = { /* code 0069 */
   __X_____,
   ________,
   _XX_____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};  /* char 'i'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_006A[8] = { /* code 006A */
   ___X____,
   ________,
   __XX____,
   ___X____,
   ___X____,
   ___X____,
   X__X____,
   _XX_____};  /* char 'j'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_006B[8] = { /* code 006B */
   X_______,
   X_______,
   X__X____,
   X_X_____,
   XX______,
   X_X_____,
   X__X____,
   ________};  /* char 'k'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_006C[8] = { /* code 006C */
   _XX_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};  /* char 'l'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_006D[8] = { /* code 006D */
   ________,
   ________,
   XX_X____,
   X_X_X___,
   X___X___,
   X___X___,
   X___X___,
   ________};  /* char 'm'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_006E[8] = { /* code 006E */
   ________,
   ________,
   X_XX____,
   XX__X___,
   X___X___,
   X___X___,
   X___X___,
   ________};  /* char 'n'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_006F[8] = { /* code 006F */
   ________,
   ________,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};  /* char 'o'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0070[8] = { /* code 0070 */
   ________,
   ________,
   XXXX____,
   X___X___,
   X___X___,
   XXXX____,
   X_______,
   X_______};  /* char 'p'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0071[8] = { /* code 0071 */
   ________,
   ________,
   _XXXX___,
   X___X___,
   X___X___,
   _XXXX___,
   ____X___,
   ____X___};  /* char 'q'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0072[8] = { /* code 0072 */
   ________,
   ________,
   X_XX____,
   XX__X___,
   X_______,
   X_______,
   X_______,
   ________};  /* char 'r'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0073[8] = { /* code 0073 */
   ________,
   ________,
   _XXXX___,
   X_______,
   _XXX____,
   ____X___,
   XXXX____,
   ________};  /* char 's'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0074[8] = { /* code 0074 */
   _X______,
   _X______,
   XXX_____,
   _X______,
   _X______,
   _X__X___,
   __XX____,
   ________};  /* char 't'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0075[8] = { /* code 0075 */
   ________,
   ________,
   X___X___,
   X___X___,
   X___X___,
   X__XX___,
   _XX_X___,
   ________};  /* char 'u'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0076[8] = { /* code 0076 */
   ________,
   ________,
   X___X___,
   X___X___,
   X___X___,
   _X_X____,
   __X_____,
   ________};  /* char 'v'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0077[8] = { /* code 0077 */
   ________,
   ________,
   X___X___,
   X___X___,
   X_X_X___,
   X_X_X___,
   _X_X____,
   ________};  /* char 'w'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0078[8] = { /* code 0078 */
   ________,
   ________,
   X___X___,
   _X_X____,
   __X_____,
   _X_X____,
   X___X___,
   ________};  /* char 'x'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_0079[8] = { /* code 0079 */
   ________,
   ________,
   X___X___,
   X___X___,
   X___X___,
   _XXXX___,
   ____X___,
   _XXX____};  /* char 'y'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_007A[8] = { /* code 007A */
   ________,
   ________,
   XXXXX___,
   ___X____,
   __X_____,
   _X______,
   XXXXX___,
   ________};  /* char 'z'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_007B[8] = { /* code 007B */
   ___X____,
   __X_____,
   __X_____,
   _X______,
   __X_____,
   __X_____,
   ___X____,
   ________};  /* char '{'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_007C[8] = { /* code 007C */
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   ________};  /* char '|'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_007D[8] = { /* code 007D */
   _X______,
   __X_____,
   __X_____,
   ___X____,
   __X_____,
   __X_____,
   _X______,
   ________};  /* char '}'  */

GUI_CONST_STORAGE unsigned char acFont6x8ASCII_007E[8] = { /* code 007E */
   _XX_X___,
   X__X____,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* char '~'  */

GUI_CONST_STORAGE GUI_CHARINFO GUI_Font6x8ASCII_CharInfo[95] = {
   {   6,   6,  1, acFont6x8ASCII_0020 } /* code 0020 */
  ,{   6,   6,  1, acFont6x8ASCII_0021 } /* code 0021 */
  ,{   6,   6,  1, acFont6x8ASCII_0022 } /* code 0022 */
  ,{   6,   6,  1, acFont6x8ASCII_0023 } /* code 0023 */
  ,{   6,   6,  1, acFont6x8ASCII_0024 } /* code 0024 */
  ,{   6,   6,  1, acFont6x8ASCII_0025 } /* code 0025 */
  ,{   6,   6,  1, acFont6x8ASCII_0026 } /* code 0026 */
  ,{   6,   6,  1, acFont6x8ASCII_0027 } /* code 0027 */
  ,{   6,   6,  1, acFont6x8ASCII_0028 } /* code 0028 */
  ,{   6,   6,  1, acFont6x8ASCII_0029 } /* code 0029 */
  ,{   6,   6,  1, acFont6x8ASCII_002A } /* code 002A */
  ,{   6,   6,  1, acFont6x8ASCII_002B } /* code 002B */
  ,{   6,   6,  1, acFont6x8ASCII_002C } /* code 002C */
  ,{   6,   6,  1, acFont6x8ASCII_002D } /* code 002D */
  ,{   6,   6,  1, acFont6x8ASCII_002E } /* code 002E */
  ,{   6,   6,  1, acFont6x8ASCII_002F } /* code 002F */
  ,{   6,   6,  1, acFont6x8ASCII_0030 } /* code 0030 */
  ,{   6,   6,  1, acFont6x8ASCII_0031 } /* code 0031 */
  ,{   6,   6,  1, acFont6x8ASCII_0032 } /* code 0032 */
  ,{   6,   6,  1, acFont6x8ASCII_0033 } /* code 0033 */
  ,{   6,   6,  1, acFont6x8ASCII_0034 } /* code 0034 */
  ,{   6,   6,  1, acFont6x8ASCII_0035 } /* code 0035 */
  ,{   6,   6,  1, acFont6x8ASCII_0036 } /* code 0036 */
  ,{   6,   6,  1, acFont6x8ASCII_0037 } /* code 0037 */
  ,{   6,   6,  1, acFont6x8ASCII_0038 } /* code 0038 */
  ,{   6,   6,  1, acFont6x8ASCII_0039 } /* code 0039 */
  ,{   6,   6,  1, acFont6x8ASCII_003A } /* code 003A */
  ,{   6,   6,  1, acFont6x8ASCII_003B } /* code 003B */
  ,{   6,   6,  1, acFont6x8ASCII_003C } /* code 003C */
  ,{   6,   6,  1, acFont6x8ASCII_003D } /* code 003D */
  ,{   6,   6,  1, acFont6x8ASCII_003E } /* code 003E */
  ,{   6,   6,  1, acFont6x8ASCII_003F } /* code 003F */
  ,{   6,   6,  1, acFont6x8ASCII_0040 } /* code 0040 */
  ,{   6,   6,  1, acFont6x8ASCII_0041 } /* code 0041 */
  ,{   6,   6,  1, acFont6x8ASCII_0042 } /* code 0042 */
  ,{   6,   6,  1, acFont6x8ASCII_0043 } /* code 0043 */
  ,{   6,   6,  1, acFont6x8ASCII_0044 } /* code 0044 */
  ,{   6,   6,  1, acFont6x8ASCII_0045 } /* code 0045 */
  ,{   6,   6,  1, acFont6x8ASCII_0046 } /* code 0046 */
  ,{   6,   6,  1, acFont6x8ASCII_0047 } /* code 0047 */
  ,{   6,   6,  1, acFont6x8ASCII_0048 } /* code 0048 */
  ,{   6,   6,  1, acFont6x8ASCII_0049 } /* code 0049 */
  ,{   6,   6,  1, acFont6x8ASCII_004A } /* code 004A */
  ,{   6,   6,  1, acFont6x8ASCII_004B } /* code 004B */
  ,{   6,   6,  1, acFont6x8ASCII_004C } /* code 004C */
  ,{   6,   6,  1, acFont6x8ASCII_004D } /* code 004D */
  ,{   6,   6,  1, acFont6x8ASCII_004E } /* code 004E */
  ,{   6,   6,  1, acFont6x8ASCII_004F } /* code 004F */
  ,{   6,   6,  1, acFont6x8ASCII_0050 } /* code 0050 */
  ,{   6,   6,  1, acFont6x8ASCII_0051 } /* code 0051 */
  ,{   6,   6,  1, acFont6x8ASCII_0052 } /* code 0052 */
  ,{   6,   6,  1, acFont6x8ASCII_0053 } /* code 0053 */
  ,{   6,   6,  1, acFont6x8ASCII_0054 } /* code 0054 */
  ,{   6,   6,  1, acFont6x8ASCII_0055 } /* code 0055 */
  ,{   6,   6,  1, acFont6x8ASCII_0056 } /* code 0056 */
  ,{   6,   6,  1, acFont6x8ASCII_0057 } /* code 0057 */
  ,{   6,   6,  1, acFont6x8ASCII_0058 } /* code 0058 */
  ,{   6,   6,  1, acFont6x8ASCII_0059 } /* code 0059 */
  ,{   6,   6,  1, acFont6x8ASCII_005A } /* code 005A */
  ,{   6,   6,  1, acFont6x8ASCII_005B } /* code 005B */
  ,{   6,   6,  1, acFont6x8ASCII_005C } /* code 005C */
  ,{   6,   6,  1, acFont6x8ASCII_005D } /* code 005D */
  ,{   6,   6,  1, acFont6x8ASCII_005E } /* code 005E */
  ,{   6,   6,  1, acFont6x8ASCII_005F } /* code 005F */
  ,{   6,   6,  1, acFont6x8ASCII_0060 } /* code 0060 */
  ,{   6,   6,  1, acFont6x8ASCII_0061 } /* code 0061 */
  ,{   6,   6,  1, acFont6x8ASCII_0062 } /* code 0062 */
  ,{   6,   6,  1, acFont6x8ASCII_0063 } /* code 0063 */
  ,{   6,   6,  1, acFont6x8ASCII_0064 } /* code 0064 */
  ,{   6,   6,  1, acFont6x8ASCII_0065 } /* code 0065 */
  ,{   6,   6,  1, acFont6x8ASCII_0066 } /* code 0066 */
  ,{   6,   6,  1, acFont6x8ASCII_0067 } /* code 0067 */
  ,{   6,   6,  1, acFont6x8ASCII_0068 } /* code 0068 */
  ,{   6,   6,  1, acFont6x8ASCII_0069 } /* code 0069 */
  ,{   6,   6,  1, acFont6x8ASCII_006A } /* code 006A */
  ,{   6,   6,  1, acFont6x8ASCII_006B } /* code 006B */
  ,{   6,   6,  1, acFont6x8ASCII_006C } /* code 006C */
  ,{   6,   6,  1, acFont6x8ASCII_006D } /* code 006D */
  ,{   6,   6,  1, acFont6x8ASCII_006E } /* code 006E */
  ,{   6,   6,  1, acFont6x8ASCII_006F } /* code 006F */
  ,{   6,   6,  1, acFont6x8ASCII_0070 } /* code 0070 */
  ,{   6,   6,  1, acFont6x8ASCII_0071 } /* code 0071 */
  ,{   6,   6,  1, acFont6x8ASCII_0072 } /* code 0072 */
  ,{   6,   6,  1, acFont6x8ASCII_0073 } /* code 0073 */
  ,{   6,   6,  1, acFont6x8ASCII_0074 } /* code 0074 */
  ,{   6,   6,  1, acFont6x8ASCII_0075 } /* code 0075 */
  ,{   6,   6,  1, acFont6x8ASCII_0076 } /* code 0076 */
  ,{   6,   6,  1, acFont6x8ASCII_0077 } /* code 0077 */
  ,{   6,   6,  1, acFont6x8ASCII_0078 } /* code 0078 */
  ,{   6,   6,  1, acFont6x8ASCII_0079 } /* code 0079 */
  ,{   6,   6,  1, acFont6x8ASCII_007A } /* code 007A */
  ,{   6,   6,  1, acFont6x8ASCII_007B } /* code 007B */
  ,{   6,   6,  1, acFont6x8ASCII_007C } /* code 007C */
  ,{   6,   6,  1, acFont6x8ASCII_007D } /* code 007D */
  ,{   6,   6,  1, acFont6x8ASCII_007E } /* code 007E */
};

GUI_CONST_STORAGE GUI_FONT_PROP GUI_Font6x8ASCII_Prop0 = {
   32                             /* first character               */
  ,126                            /* last character                */
  ,&GUI_Font6x8ASCII_CharInfo[0]  /* address of first character    */
  ,(const GUI_FONT_PROP*)0        /* pointer to next GUI_FONT_PROP */
};

GUI_CONST_STORAGE GUI_FONT GUI_Font6x8_ASCII = {
   GUI_FONTTYPE_PROP /* type of font    */
  ,8                 /* height of font  */
  ,8                 /* space of font y */
  ,1                 /* magnification x */
  ,1                 /* magnification y */
  ,{&GUI_Font6x8ASCII_Prop0}
  ,7, 5, 7
};

/*************************** End of file ****************************/
