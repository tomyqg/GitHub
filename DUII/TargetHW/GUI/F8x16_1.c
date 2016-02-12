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
File        : F8x16_1.C
Purpose     : Monospaced Font similar to Terminal
Height      : 8
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_FontIntern.h"

extern GUI_CONST_STORAGE GUI_FONT_PROP GUI_Font8x16ASCII_Prop0;

GUI_CONST_STORAGE unsigned char acF8x16_1_160[16] = { /* code 160 */
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
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x16_1_161[16] = { /* code 161 */
   ________,
   ________,
   ___XX___,
   ___XX___,
   ________,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   __XXXX__,
   __XXXX__,
   ___XX___,
   ________,
   ________,
   ________,
   ________};   /* inverted exclamation'  */

GUI_CONST_STORAGE unsigned char acF8x16_1_162[16] = { /* code 162 */
   ________,
   ________,
   ________,
   ___X____,
   ___X____,
   _XXXXX__,
   XX___XX_,
   XX______,
   XX______,
   XX______,
   XX___XX_,
   _XXXXX__,
   ___X____,
   ___X____,
   ________,
   ________};   /* cent sign */

GUI_CONST_STORAGE unsigned char acF8x16_1_163[16] = { /* code 163 */
   ________,
   ________,
   __XXXX__,
   _XXXXXX_,
   _XX__XX_,
   _XX__XX_,
   _XX_____,
   XXXXXX__,
   XXXXXX__,
   __XX____,
   _XX_____,
   _XX___X_,
   XXXXXXX_,
   XX_XXX__,
   ________,
   ________};   /* pound sterling */

GUI_CONST_STORAGE unsigned char acF8x16_1_164[16] = { /* code 164 */
   ________,
   ________,
   ________,
   ________,
   ________,
   X_____X_,
   _X_X_X__,
   __XXX___,
   XXX_XXX_,
   __XXX___,
   _X_X_X__,
   X_____X_,
   ________,
   ________,
   ________,
   ________};   /* general currency sign */

GUI_CONST_STORAGE unsigned char acF8x16_1_165[16] = { /* code 165 */
   ________,
   ________,
   XX____XX,
   XX____XX,
   _XX__XX_,
   _XX__XX_,
   __XXXX__,
   _XXXXXX_,
   ___XX___,
   ___XX___,
   _XXXXXX_,
   ___XX___,
   ___XX___,
   ___XX___,
   ________,
   ________};   /* yen sign */

GUI_CONST_STORAGE unsigned char acF8x16_1_166[16] = { /* code 166 */
   ________,
   ________,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ___XX___,
   ___XX___,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ________}
;   /* broken vertical bar */

GUI_CONST_STORAGE unsigned char acF8x16_1_167[16] = { /* code 167 */
   ________,
   ________,
   __XXXX__,
   _XX__X__,
   _XXX____,
   __XXX___,
   _XX_XX__,
   _XX_XX__,
   __XXX___,
   ___XXX__,
   _X__XX__,
   _XXXX___,
   ________,
   ________,
   ________,
   ________};   /* section sign */

GUI_CONST_STORAGE unsigned char acF8x16_1_168[16] = { /* code 168 */
   ________,
   XX___XX_,
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
   ________,
   ________}
;   /* umlaut (dieresis) */

GUI_CONST_STORAGE unsigned char acF8x16_1_169[16] = { /* code 169 */
   ________,
   ________,
   ________,
   _XXXXX__,
   X_____X_,
   X_XXX_X_,
   X_X___X_,
   X_X___X_,
   X_X___X_,
   X_XXX_X_,
   X_____X_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________};   /* copyright */

GUI_CONST_STORAGE unsigned char acF8x16_1_170[16] = { /* code 170 */
   ________,
   ________,
   _XXX____,
   ____X___,
   _XXXX___,
   X___X___,
   X___X___,
   _XXX_X__,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* feminine ordinal */

GUI_CONST_STORAGE unsigned char acF8x16_1_171[16] = { /* code 171 */
   ________,
   ________,
   ________,
   ________,
   ________,
   __XX_XX_,
   _XX_XX__,
   XX_XX___,
   _XX_XX__,
   __XX_XX_,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* left angle quote, guillemotleft */

GUI_CONST_STORAGE unsigned char acF8x16_1_172[16] = { /* code 172 */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   XXXXXX__,
   _____X__,
   _____X__,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________}
;   /* not sign */

GUI_CONST_STORAGE unsigned char acF8x16_1_173[16] = { /* code 173 */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* soft hyphen */

GUI_CONST_STORAGE unsigned char acF8x16_1_174[16] = { /* code 174 */
   ________,
   ________,
   ________,
   _XXXXX__,
   X_____X_,
   X_XXX_X_,
   X_X_X_X_,
   X_XX__X_,
   X_X_X_X_,
   X_X_X_X_,
   X_____X_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________};   /* registered trademark */

GUI_CONST_STORAGE unsigned char acF8x16_1_175[16] = { /* code 175 */
   XXXXXXX_,
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
   ________};   /* macron accent */

GUI_CONST_STORAGE unsigned char acF8x16_1_176[16] = { /* code 176 */
   ________,
   _XXX____,
   XX_XX___,
   X___X___,
   XX_XX___,
   _XXX____,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* degree sign */

GUI_CONST_STORAGE unsigned char acF8x16_1_177[16] = { /* code 177 */
   ________,
   ________,
   ________,
   ________,
   ___X____,
   ___X____,
   _XXXXX__,
   ___X____,
   ___X____,
   ________,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________,
   ________};   /* plus or minus */

GUI_CONST_STORAGE unsigned char acF8x16_1_178[16] = { /* code 178 */
   _XXXX___,
   X___XX__,
   ____XX__,
   ___XX___,
   __XX____,
   _XX_____,
   XXXXXX__,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* superscript two */

GUI_CONST_STORAGE unsigned char acF8x16_1_179[16] = { /* code 179 */
   _XXXX___,
   X___XX__,
   ____XX__,
   _XXXX___,
   ____XX__,
   X___XX__,
   _XXXX___,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* superscript three */

GUI_CONST_STORAGE unsigned char acF8x16_1_180[16] = { /* code 180 */
   ________,
   ____XXX_,
   ___XX___,
   __X_____,
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
   ________};   /* acute accent */

GUI_CONST_STORAGE unsigned char acF8x16_1_181[16] = { /* code 181 */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XXXXX__,
   _XX_____,
   _XX_____,
   XX______,
   ________};   /* micro sign */

GUI_CONST_STORAGE unsigned char acF8x16_1_182[16] = { /* code 182 */
   ________,
   ________,
   _XXXXXX_,
   XX_X_X__,
   XX_X_X__,
   XX_X_X__,
   _XXX_X__,
   ___X_X__,
   ___X_X__,
   ___X_X__,
   ___X_X__,
   __XXXXX_,
   ________,
   ________,
   ________,
   ________};   /* paragraph sign */

GUI_CONST_STORAGE unsigned char acF8x16_1_183[16] = { /* code 183 */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   __XXX___,
   __XXX___,
   __XXX___,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* middle dot */

GUI_CONST_STORAGE unsigned char acF8x16_1_184[16] = { /* code 184 */
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
   ___XX___,
   __XX____,
   ________,
   ________};   /* cedilla */

GUI_CONST_STORAGE unsigned char acF8x16_1_185[16] = { /* code 185 */
   _XX_____,
   XXX_____,
   _XX_____,
   _XX_____,
   _XX_____,
   _XX_____,
   XXXX____,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* superscript one */

GUI_CONST_STORAGE unsigned char acF8x16_1_186[16] = { /* code 186 */
   ________,
   ________,
   __XXXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   __XXXX__,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* masculine ordinal */

GUI_CONST_STORAGE unsigned char acF8x16_1_187[16] = { /* code 187 */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX_XX___,
   _XX_XX__,
   __XX_XX_,
   _XX_XX__,
   XX_XX___,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* right angle quote, guillemot right */

GUI_CONST_STORAGE unsigned char acF8x16_1_188[16] = { /* code 188 */
   ________,
   ________,
   _X______,
   XX______,
   _X___X__,
   _X__X___,
   ___X____,
   __X__X__,
   _X__XX__,
   X__X_X__,
   ___XXXX_,
   _____X__,
   ________,
   ________,
   ________,
   ________};   /* fraction one-fourth */

GUI_CONST_STORAGE unsigned char acF8x16_1_189[16] = { /* code 189 */
   ________,
   ________,
   _X______,
   XX______,
   _X___X__,
   _X__X___,
   ___X____,
   __X_XX__,
   _X_X__X_,
   X____X__,
   ____X___,
   ___XXXX_,
   ________,
   ________,
   ________,
   ________};   /* fraction one-half */

GUI_CONST_STORAGE unsigned char acF8x16_1_190[16] = { /* code 190 */
   ________,
   ________,
   XX______,
   __X_____,
   _X___X__,
   __X_X___,
   XX_X____,
   __X__X__,
   _X__XX__,
   X__X_X__,
   ___XXXX_,
   _____X__,
   ________,
   ________,
   ________,
   ________};   /* fraction three-fourth */

GUI_CONST_STORAGE unsigned char acF8x16_1_191[16] = { /* code 191 */
   ________,
   ________,
   __XX____,
   __XX____,
   ________,
   __XX____,
   __XX____,
   __XX____,
   _XX_____,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________};   /* inverted question mark */

GUI_CONST_STORAGE unsigned char acF8x16_1_192[16] = { /* code 192 */
   _XXX____,
   ___XX___,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_193[16] = { /* code 193 */
   ___XXX__,
   __XX____,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_194[16] = { /* code 194 */
   __XXX___,
   _XX_XX__,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_195[16] = { /* code 195 */
  /*
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
   ________};
  */
  _XXX_XX_,
   XX_XXX__,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_196[16] = { /* code 196 */
   XX___XX_,
   XX___XX_,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_197[16] = { /* code 197 */
   ___X____,
   __X_X___,
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
   ________}
;

GUI_CONST_STORAGE unsigned char acF8x16_1_198[16] = { /* code 198 */
   ________,
   ________,
   __XXXXX_,
   _XX_XX__,
   XX__XX__,
   XX__XX__,
   XX__XXX_,
   XXXXXX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XXX_,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_199[16] = { /* code 199 */
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
   ___XX___,
   __XX____,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_200[16] = { /* code 200 */
   _XXX____,
   ___XX___,
   ________,
   XXXXXXX_,
   _XX__XX_,
   _XX___X_,
   _XX_X___,
   _XXXX___,
   _XX_X___,
   _XX___X_,
   _XX__XX_,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________};
   
GUI_CONST_STORAGE unsigned char acF8x16_1_201[16] = { /* code 201 */
   ___XXX__,
   __XX____,
   ________,
   XXXXXXX_,
   _XX__XX_,
   _XX___X_,
   _XX_X___,
   _XXXX___,
   _XX_X___,
   _XX___X_,
   _XX__XX_,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_202[16] = { /* code 202 */
   __XXX___,
   _XX_XX__,
   ________,
   XXXXXXX_,
   _XX__XX_,
   _XX___X_,
   _XX_X___,
   _XXXX___,
   _XX_X___,
   _XX___X_,
   _XX__XX_,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_203[16] = { /* code 203 */
   XX___XX_,
   XX___XX_,
   ________,
   XXXXXXX_,
   _XX__XX_,
   _XX___X_,
   _XX_X___,
   _XXXX___,
   _XX_X___,
   _XX___X_,
   _XX__XX_,
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_204[16] = { /* code 204 */
   _XXX____,
   ___XX___,
   ________,
   __XXXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_205[16] = { /* code 205 */
   ___XXX__,
   __XX____,
   ________,
   __XXXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_206[16] = { /* code 206 */
   ___XX___,
   __X__X__,
   ________,
   __XXXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_207[16] = { /* code 207 */
   _XX__XX_,
   _XX__XX_,
   ________,
   __XXXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_208[16] = { /* code 208 */
   ________,
   ________,
   XXXXXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   XXXX_XX_,
   XXXX_XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   XXXXXX__,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_209[16] = { /* code 209 */
   _XXX_XX_,
   XX_XXX__,
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
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_210[16] = { /* code 210 */
   _XXX____,
   ___XX___,
   ________,
   _XXXXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_211[16] = { /* code 211 */
   ___XXX__,
   __XX____,
   ________,
   _XXXXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_212[16] = { /* code 212 */
   __XXX___,
   _XX_XX__,
   ________,
   _XXXXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_213[16] = { /* code 213 */
   _XXX_XX_,
   XX_XXX__,
   ________,
   _XXXXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_214[16] = { /* code 214 */
   XX___XX_,
   XX___XX_,
   ________,
   _XXXXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_215[16] = { /* code 215 */
   ________,
   ________,
   ________,
   ________,
   ________,
   _XX__XX_,
   __XXXX__,
   ___XX___,
   __XXXX__,
   _XX__XX_,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* multiply sign */

GUI_CONST_STORAGE unsigned char acF8x16_1_216[16] = { /* code 216 */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX__XXX_,
   XX__XXX_,
   XX_X_XX_,
   XX_X_XX_,
   XXX__XX_,
   XXX__XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_217[16] = { /* code 217 */
   _XXX____,
   ___XX___,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_218[16] = { /* code 218 */
   ___XXX__,
   __XX____,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_219[16] = { /* code 219 */
   __XXX___,
   _XX_XX__,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_220[16] = { /* code 220 */
   XX___XX_,
   XX___XX_,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_221[16] = { /* code 221 */
   ___XXX__,
   __XX____,
   ________,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   __XXXX__,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_222[16] = { /* code 222 */
   ________,
   ________,
   XXXX____,
   _XX_____,
   _XXXXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XXXXX__,
   _XX_____,
   XXXX____,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_223[16] = { /* code 223 */
   ________,
   ________,
   __XXX___,
   _XX_XX__,
   _XX_XX__,
   _XX_XX__,
   _XXXX___,
   _XX_XX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX_XX__,
   _XX_____,
   XX______,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_224[16] = { /* code 224 */
   ________,
   ________,
   _XXX____,
   ___XX___,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_225[16] = { /* code 225 */
   ________,
   ________,
   ___XXX__,
   __XX____,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_226[16] = { /* code 226 */
   ________,
   ___X____,
   __XXX___,
   _XX_XX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_227[16] = { /* code 227 */
   ________,
   _XXX_XX_,
   XX_XXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_228[16] = { /* code 228 */
   ________,
   XX___XX_,
   XX___XX_,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_229[16] = { /* code 229 */
   ________,
   ___X____,
   __X_X___,
   ___X____,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_230[16] = { /* code 230 */
   ________,
   ________,
   ________,
   ________,
   ________,
   XX__XX__,
   __XX__X_,
   __XXXXX_,
   _XXX____,
   X_XX____,
   X_XX__X_,
   _XX_XX__,
   ________,
   ________,
   ________,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x16_1_231[16] = { /* code 231 */
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
   ___XX___,
   __XX____,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_232[16] = { /* code 232 */
   ________,
   ________,
   _XXX____,
   ___XX___,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_233[16] = { /* code 233 */
   ________,
   ________,
   ___XXX__,
   __XX____,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_234[16] = { /* code 234 */
   ________,
   ___X____,
   __XXX___,
   _XX_XX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_235[16] = { /* code 235 */
   ________,
   XX___XX_,
   XX___XX_,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_236[16] = { /* code 236 */
   ________,
   ________,
   _XXX____,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_237[16] = { /* code 237 */
   ________,
   ________,
   ___XXX__,
   __XX____,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_238[16] = { /* code 238 */
   ________,
   ___X____,
   __XXX___,
   _XX_XX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_239[16] = { /* code 239 */
   ________,
   XX___XX_,
   XX___XX_,
   ________,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_240[16] = { /* code 240 */
   ________,
   ________,
   XX_X____,
   _XX_____,
   X_XX____,
   ___XX___,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_241[16] = { /* code 241 */
   ________,
   _XXX_XX_,
   XX_XXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_242[16] = { /* code 242 */
   ________,
   ________,
   _XXX____,
   ___XX___,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_243[16] = { /* code 243 */
   ________,
   ________,
   ___XXX__,
   __XX____,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_244[16] = { /* code 244 */
   ________,
   ___X____,
   __XXX___,
   _XX_XX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_245[16] = { /* code 245 */
   ________,
   _XXX_XX_,
   XX_XXX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_246[16] = { /* code 246 */
   ________,
   XX___XX_,
   XX___XX_,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_247[16] = { /* code 247 */
   ________,
   ________,
   ________,
   ________,
   ___XX___,
   ___XX___,
   ________,
   _XXXXXX_,
   ________,
   ___XX___,
   ___XX___,
   ________,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_248[16] = { /* code 248 */
   ________,
   ________,
   ________,
   ________,
   ________,
   __XXX_X_,
   _X___X__,
   XX__XXX_,
   XX_X_XX_,
   XXX__XX_,
   _X___X__,
   X_XXX___,
   ________,
   ________,
   ________,
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_249[16] = { /* code 249 */
   ________,
   ________,
   _XXX____,
   ___XX___,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_250[16] = { /* code 250 */
   ________,
   ________,
   ___XXX__,
   __XX____,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_251[16] = { /* code 251 */
   ________,
   ___X____,
   __XXX___,
   _XX_XX__,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_252[16] = { /* code 252 */
   ________,
   XX___XX_,
   XX___XX_,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_253[16] = { /* code 253 */
   ________,
   ________,
   ___XXX__,
   __XX____,
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
   ________};

GUI_CONST_STORAGE unsigned char acF8x16_1_254[16] = { /* code 254 */
   ________,
   ________,
   XXX_____,
   _XX_____,
   _XX_____,
   _XXXX___,
   _XX_XX__,
   _XX_XX__,
   _XXXX___,
   _XX_____,
   _XX_____,
   XXXX____,
   ________,
   ________,
   ________,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x16_1_255[16] = { /* code 255 */
   ________,
   XX___XX_,
   XX___XX_,
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
   ________};


GUI_CONST_STORAGE GUI_CHARINFO GUI_CharInfoF8x16_1[96] = {
   {   8,   8,  1, acF8x16_1_160 } /* code 160 */
  ,{   8,   8,  1, acF8x16_1_161 } /* code 161 */
  ,{   8,   8,  1, acF8x16_1_162 } /* code 162 */
  ,{   8,   8,  1, acF8x16_1_163 } /* code 163 */
  ,{   8,   8,  1, acF8x16_1_164 } /* code 164 */
  ,{   8,   8,  1, acF8x16_1_165 } /* code 165 */
  ,{   8,   8,  1, acF8x16_1_166 } /* code 166 */
  ,{   8,   8,  1, acF8x16_1_167 } /* code 167 */
  ,{   8,   8,  1, acF8x16_1_168 } /* code 168 */
  ,{   8,   8,  1, acF8x16_1_169 } /* code 169 */
  ,{   8,   8,  1, acF8x16_1_170 } /* code 170 */
  ,{   8,   8,  1, acF8x16_1_171 } /* code 171 */
  ,{   8,   8,  1, acF8x16_1_172 } /* code 172 */
  ,{   8,   8,  1, acF8x16_1_173 } /* code 173 */
  ,{   8,   8,  1, acF8x16_1_174 } /* code 174 */
  ,{   8,   8,  1, acF8x16_1_175 } /* code 175 */
  ,{   8,   8,  1, acF8x16_1_176 } /* code 176 */
  ,{   8,   8,  1, acF8x16_1_177 } /* code 177 */
  ,{   8,   8,  1, acF8x16_1_178 } /* code 178 */
  ,{   8,   8,  1, acF8x16_1_179 } /* code 179 */
  ,{   8,   8,  1, acF8x16_1_180 } /* code 180 */
  ,{   8,   8,  1, acF8x16_1_181 } /* code 181 */
  ,{   8,   8,  1, acF8x16_1_182 } /* code 182 */
  ,{   8,   8,  1, acF8x16_1_183 } /* code 183 */
  ,{   8,   8,  1, acF8x16_1_184 } /* code 184 */
  ,{   8,   8,  1, acF8x16_1_185 } /* code 185 */
  ,{   8,   8,  1, acF8x16_1_186 } /* code 186 */
  ,{   8,   8,  1, acF8x16_1_187 } /* code 187 */
  ,{   8,   8,  1, acF8x16_1_188 } /* code 188 */
  ,{   8,   8,  1, acF8x16_1_189 } /* code 189 */
  ,{   8,   8,  1, acF8x16_1_190 } /* code 190 */
  ,{   8,   8,  1, acF8x16_1_191 } /* code 191 */
  ,{   8,   8,  1, acF8x16_1_192 } /* code 192 */
  ,{   8,   8,  1, acF8x16_1_193 } /* code 193 */
  ,{   8,   8,  1, acF8x16_1_194 } /* code 194 */
  ,{   8,   8,  1, acF8x16_1_195 } /* code 195 */
  ,{   8,   8,  1, acF8x16_1_196 } /* code 196 */
  ,{   8,   8,  1, acF8x16_1_197 } /* code 197 */
  ,{   8,   8,  1, acF8x16_1_198 } /* code 198 */
  ,{   8,   8,  1, acF8x16_1_199 } /* code 199 */
  ,{   8,   8,  1, acF8x16_1_200 } /* code 200 */
  ,{   8,   8,  1, acF8x16_1_201 } /* code 201 */
  ,{   8,   8,  1, acF8x16_1_202 } /* code 202 */
  ,{   8,   8,  1, acF8x16_1_203 } /* code 203 */
  ,{   8,   8,  1, acF8x16_1_204 } /* code 204 */
  ,{   8,   8,  1, acF8x16_1_205 } /* code 205 */
  ,{   8,   8,  1, acF8x16_1_206 } /* code 206 */
  ,{   8,   8,  1, acF8x16_1_207 } /* code 207 */
  ,{   8,   8,  1, acF8x16_1_208 } /* code 208 */
  ,{   8,   8,  1, acF8x16_1_209 } /* code 209 */
  ,{   8,   8,  1, acF8x16_1_210 } /* code 210 */
  ,{   8,   8,  1, acF8x16_1_211 } /* code 211 */
  ,{   8,   8,  1, acF8x16_1_212 } /* code 212 */
  ,{   8,   8,  1, acF8x16_1_213 } /* code 213 */
  ,{   8,   8,  1, acF8x16_1_214 } /* code 214 */
  ,{   8,   8,  1, acF8x16_1_215 } /* code 215 */
  ,{   8,   8,  1, acF8x16_1_216 } /* code 216 */
  ,{   8,   8,  1, acF8x16_1_217 } /* code 217 */
  ,{   8,   8,  1, acF8x16_1_218 } /* code 218 */
  ,{   8,   8,  1, acF8x16_1_219 } /* code 219 */
  ,{   8,   8,  1, acF8x16_1_220 } /* code 220 */
  ,{   8,   8,  1, acF8x16_1_221 } /* code 221 */
  ,{   8,   8,  1, acF8x16_1_222 } /* code 222 */
  ,{   8,   8,  1, acF8x16_1_223 } /* code 223 */
  ,{   8,   8,  1, acF8x16_1_224 } /* code 224 */
  ,{   8,   8,  1, acF8x16_1_225 } /* code 225 */
  ,{   8,   8,  1, acF8x16_1_226 } /* code 226 */
  ,{   8,   8,  1, acF8x16_1_227 } /* code 227 */
  ,{   8,   8,  1, acF8x16_1_228 } /* code 228 */
  ,{   8,   8,  1, acF8x16_1_229 } /* code 229 */
  ,{   8,   8,  1, acF8x16_1_230 } /* code 230 */
  ,{   8,   8,  1, acF8x16_1_231 } /* code 231 */
  ,{   8,   8,  1, acF8x16_1_232 } /* code 232 */
  ,{   8,   8,  1, acF8x16_1_233 } /* code 233 */
  ,{   8,   8,  1, acF8x16_1_234 } /* code 234 */
  ,{   8,   8,  1, acF8x16_1_235 } /* code 235 */
  ,{   8,   8,  1, acF8x16_1_236 } /* code 236 */
  ,{   8,   8,  1, acF8x16_1_237 } /* code 237 */
  ,{   8,   8,  1, acF8x16_1_238 } /* code 238 */
  ,{   8,   8,  1, acF8x16_1_239 } /* code 239 */
  ,{   8,   8,  1, acF8x16_1_240 } /* code 240 */
  ,{   8,   8,  1, acF8x16_1_241 } /* code 241 */
  ,{   8,   8,  1, acF8x16_1_242 } /* code 242 */
  ,{   8,   8,  1, acF8x16_1_243 } /* code 243 */
  ,{   8,   8,  1, acF8x16_1_244 } /* code 244 */
  ,{   8,   8,  1, acF8x16_1_245 } /* code 245 */
  ,{   8,   8,  1, acF8x16_1_246 } /* code 246 */
  ,{   8,   8,  1, acF8x16_1_247 } /* code 247 */
  ,{   8,   8,  1, acF8x16_1_248 } /* code 248 */
  ,{   8,   8,  1, acF8x16_1_249 } /* code 249 */
  ,{   8,   8,  1, acF8x16_1_250 } /* code 250 */
  ,{   8,   8,  1, acF8x16_1_251 } /* code 251 */
  ,{   8,   8,  1, acF8x16_1_252 } /* code 252 */
  ,{   8,   8,  1, acF8x16_1_253 } /* code 253 */
  ,{   8,   8,  1, acF8x16_1_254 } /* code 254 */
  ,{   8,   8,  1, acF8x16_1_255 } /* code 255 */
};

GUI_CONST_STORAGE GUI_FONT_PROP GUI_FontPropF8x16_1 = {
   160                      /* first character               */
  ,255                      /* last character                */
  ,&GUI_CharInfoF8x16_1[0]  /* address of first character    */
  ,&GUI_Font8x16ASCII_Prop0 /* pointer to next GUI_FONT_PROP */
};

GUI_CONST_STORAGE GUI_FONT GUI_Font8x16_1 = {
   GUI_FONTTYPE_PROP        /* type of font    */
  ,16                       /* height of font  */
  ,16                       /* space of font y */
  , 1                       /* magnification x */
  , 1                       /* magnification y */
  ,{&GUI_FontPropF8x16_1}
  ,14
  , 7
  ,14
};

/*************************** End of file ****************************/
