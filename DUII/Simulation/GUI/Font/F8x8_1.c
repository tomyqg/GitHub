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
File        : F8x8_1.C
Purpose     : Monospaced Font similar to Terminal
Height      : 8
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_FontIntern.h"

extern GUI_CONST_STORAGE GUI_FONT_PROP GUI_Font8x8ASCII_Prop0;

GUI_CONST_STORAGE unsigned char acF8x8_1_160[8] = { /* code 160 */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_161[8] = { /* code 161 */
   ___XX___,
   ________,
   ___XX___,
   ___XX___,
   __XXXX__,
   __XXXX__,
   ___XX___,
   ________};   /* inverted exclamation'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_162[8] = { /* code 162 */
   ________,
   ___X____,
   _XXXXX__,
   XX___XX_,
   XX______,
   XX___XX_,
   _XXXXX__,
   ___X____};   /* cent sign */

GUI_CONST_STORAGE unsigned char acF8x8_1_163[8] = { /* code 163 */
   __XXXX__,
   _X____X_,
   _X______,
   XXXXX___,
   _X______,
   _XX___X_,
   X__XXX__,
   ________};   /* pound sterling */

GUI_CONST_STORAGE unsigned char acF8x8_1_164[8] = { /* code 164 */
   ________,
   ________,
   X_XXX_X_,
   _X___X__,
   XX___XX_,
   _X___X__,
   X_XXX_X_,
   ________};   /* general currency sign */

GUI_CONST_STORAGE unsigned char acF8x8_1_165[8] = { /* code 165 */
   X_____X_,
   _X___X__,
   __X_X___,
   _XXXXX__,
   ___X____,
   _XXXXX__,
   ___X____,
   ___X____};   /* yen sign */

GUI_CONST_STORAGE unsigned char acF8x8_1_166[8] = { /* code 166 */
   ___X____,
   ___X____,
   ___X____,
   ________,
   ___X____,
   ___X____,
   ___X____,
   ________};   /* broken vertical bar */

GUI_CONST_STORAGE unsigned char acF8x8_1_167[8] = { /* code 167 */
   __XXXX__,
   _X____X_,
   __XXX___,
   _X___X__,
   __X___X_,
   ___XXX__,
   _X____X_,
   __XXXX__};   /* section sign */

GUI_CONST_STORAGE unsigned char acF8x8_1_168[8] = { /* code 168 */
   _X___X__,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* umlaut (dieresis) */

GUI_CONST_STORAGE unsigned char acF8x8_1_169[8] = { /* code 169 */
   _XXXXX__,
   _X___X__,
   _X_XXX__,
   _X_XXX__,
   _X_XXX__,
   _X___X__,
   _XXXXX__,
   ________};   /* copyright */

GUI_CONST_STORAGE unsigned char acF8x8_1_170[8] = { /* code 170 */
   XX______,
   __X_____,
   XXX_____,
   X_X_____,
   XXX_____,
   ________,
   ________,
   ________};   /* feminine ordinal */

GUI_CONST_STORAGE unsigned char acF8x8_1_171[8] = { /* code 171 */
   ___X__X_,
   __X__X__,
   _X__X___,
   X__X____,
   _X__X___,
   __X__X__,
   ___X__X_,
   ________};   /* left angle quote, guillemotleft */

GUI_CONST_STORAGE unsigned char acF8x8_1_172[8] = { /* code 172 */
   ________,
   ________,
   XXXXXXX_,
   ______X_,
   ______X_,
   ________,
   ________,
   ________};   /* not sign */

GUI_CONST_STORAGE unsigned char acF8x8_1_173[8] = { /* code 173 */
   ________,
   ________,
   ________,
   ________,
   _XXXXXX_,
   ________,
   ________,
   ________};   /* soft hyphen */

GUI_CONST_STORAGE unsigned char acF8x8_1_174[8] = { /* code 174 */
   _XXXXX__,
   _X___X__,
   _X_X_X__,
   _X__XX__,
   _X_X_X__,
   _X_X_X__,
   _XXXXX__,
   ________};   /* registered trademark */

GUI_CONST_STORAGE unsigned char acF8x8_1_175[8] = { /* code 175 */
   XXXXXXX_,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* macron accent */

GUI_CONST_STORAGE unsigned char acF8x8_1_176[8] = { /* code 176 */
   __X_____,
   _X_X____,
   __X_____,
   ________,
   ________,
   ________,
   ________,
   ________};   /* degree sign */

GUI_CONST_STORAGE unsigned char acF8x8_1_177[8] = { /* code 177 */
   ___XX___,
   ___XX___,
   _XXXXXX_,
   ___XX___,
   ___XX___,
   ________,
   _XXXXXX_,
   ________};   /* plus or minus */

GUI_CONST_STORAGE unsigned char acF8x8_1_178[8] = { /* code 178 */
   _X______,
   X_X_____,
   __X_____,
   _X______,
   XXX_____,
   ________,
   ________,
   ________};   /* superscript two */

GUI_CONST_STORAGE unsigned char acF8x8_1_179[8] = { /* code 179 */
   XX______,
   __X_____,
   _X______,
   __X_____,
   XX______,
   ________,
   ________,
   ________};   /* superscript three */

GUI_CONST_STORAGE unsigned char acF8x8_1_180[8] = { /* code 180 */
   ____X___,
   ___X____,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* acute accent */

GUI_CONST_STORAGE unsigned char acF8x8_1_181[8] = { /* code 181 */
   ________,
   ________,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XXXX_XX_,
   XX______};   /* micro sign */

GUI_CONST_STORAGE unsigned char acF8x8_1_182[8] = { /* code 182 */
   _XXXXXX_,
   XXXX_X__,
   XXXX_X__,
   _XXX_X__,
   ___X_X__,
   ___X_X__,
   ___X_X__,
   ___X_X__};   /* paragraph sign */

GUI_CONST_STORAGE unsigned char acF8x8_1_183[8] = { /* code 183 */
   ________,
   ________,
   ________,
   ___X____,
   ________,
   ________,
   ________,
   ________};   /* middle dot */

GUI_CONST_STORAGE unsigned char acF8x8_1_184[8] = { /* code 184 */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   _XX_____};   /* cedilla */

GUI_CONST_STORAGE unsigned char acF8x8_1_185[8] = { /* code 185 */
   _XX_____,
   XXX_____,
   __X_____,
   __X_____,
   __X_____,
   ________,
   ________,
   ________};   /* superscript one */

GUI_CONST_STORAGE unsigned char acF8x8_1_186[8] = { /* code 186 */
   _X______,
   X_X_____,
   X_X_____,
   X_X_____,
   _X______,
   ________,
   ________,
   ________};   /* masculine ordinal */

GUI_CONST_STORAGE unsigned char acF8x8_1_187[8] = { /* code 187 */
   X__X____,
   _X__X___,
   __X__X__,
   ___X__X_,
   __X__X__,
   _X__X___,
   X__X____,
   ________};   /* right angle quote, guillemot right */

GUI_CONST_STORAGE unsigned char acF8x8_1_188[8] = { /* code 188 */
   _X____X_,
   XX___X__,
   _X__X___,
   _X_X_X__,
   __X_XX__,
   _X_X_X__,
   X__XXXX_,
   _____X__};   /* fraction one-fourth */

GUI_CONST_STORAGE unsigned char acF8x8_1_189[8] = { /* code 189 */
   _X____X_,
   XX___X__,
   _X__X___,
   _X_X_XX_,
   __X___X_,
   _X___XX_,
   X____X__,
   _____XX_};   /* fraction one-half */

GUI_CONST_STORAGE unsigned char acF8x8_1_190[8] = { /* code 190 */
   XX____X_,
   _X___X__,
   XX__X___,
   _X_X_X__,
   XXX_XX__,
   _X_X_X__,
   X__XXXX_,
   _____X__};   /* fraction three-fourth */

GUI_CONST_STORAGE unsigned char acF8x8_1_191[8] = { /* code 191 */
   __XX____,
   ________,
   __XX____,
   __XX____,
   _XX_____,
   XX___XX_,
   _XXXXX__,
   ________};   /* inverted question mark */

GUI_CONST_STORAGE unsigned char acF8x8_1_192[8] = { /* code 192 */
   __X_____,
   ___X____,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   XXXXXXX_,
   XX___XX_,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_193[8] = { /* code 193 */
   ____X___,
   ___X____,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   XXXXXXX_,
   XX___XX_,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_194[8] = { /* code 194 */
   ___X____,
   __X_X___,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   XXXXXXX_,
   XX___XX_,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_195[8] = { /* code 195 */
   _XXX_XX_,
   XX_XXX__,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   XXXXXXX_,
   XX___XX_,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_196[8] = { /* code 196 */
   _X___X__,
   ________,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   XXXXXXX_,
   XX___XX_,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_197[8] = { /* code 197 */
   __XXX___,
   __X_X___,
   __XXX___,
   _XX_XX__,
   XX___XX_,
   XXXXXXX_,
   XX___XX_,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_198[8] = { /* code 198 */
   __XXXXX_,
   __X_X___,
   _X__X___,
   _XXXXXX_,
   _X__X___,
   X___X___,
   X___XXX_,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_199[8] = { /* code 199 */
   __XXXX__,
   _XX__XX_,
   XX______,
   XX______,
   XX______,
   _XX__XX_,
   __XXXX__,
   _XX_____};   /* cedilla */

GUI_CONST_STORAGE unsigned char acF8x8_1_200[8] = { /* code 200 */
   __X_____,
   ___X____,
   _XXXXXX_,
   _XX_____,
   _XXXX___,
   _XX_____,
   _XXXXXX_,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_201[8] = { /* code 201 */
   ____X___,
   ___X____,
   _XXXXXX_,
   _XX_____,
   _XXXX___,
   _XX_____,
   _XXXXXX_,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_202[8] = { /* code 202 */
   ___X____,
   __X_X___,
   _XXXXXX_,
   _XX_____,
   _XXXX___,
   _XX_____,
   _XXXXXX_,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_203[8] = { /* code 203 */
   __X__X__,
   ________,
   _XXXXXX_,
   _XX_____,
   _XXXX___,
   _XX_____,
   _XXXXXX_,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_204[8] = { /* code 204 */
   __X_____,
   ___X____,
   __XXXX__,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_205[8] = { /* code 205 */
   _____X__,
   ____X___,
   __XXXX__,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_206[8] = { /* code 206 */
   ___XX___,
   __X__X__,
   __XXXX__,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_207[8] = { /* code 207 */
   __X__X__,
   ________,
   __XXXX__,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_208[8] = { /* code 208 */
   XXXXX___,
   _XX_XX__,
   _XX__XX_,
   XXXX_XX_,
   _XX__XX_,
   _XX_XX__,
   XXXXX___,
   ________};   /* char 'D'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_209[8] = { /* code 209 */
   _XXX_XX_,
   XX_XXX__,
   XX___XX_,
   XXX__XX_,
   XXXX_XX_,
   XX_XXXX_,
   XX__XXX_,
   ________};  /* char 'N'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_210[8] = { /* code 210 */
   __X_____,
   ___X____,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_211[8] = { /* code 211 */
   ____X___,
   ___X____,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_212[8] = { /* code 212 */
   ___X____,
   __X_X___,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_213[8] = { /* code 213 */
   _XXX_XX_,
   XX_XXX__,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_214[8] = { /* code 214 */
   _X___X__,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_215[8] = { /* code 215 */
   ________,
   ________,
   _X___X__,
   __X_X___,
   ___X____,
   __X_X___,
   _X___X__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_216[8] = { /* code 216 */
   _XXXXXX_,
   XX___XX_,
   XX__XXX_,
   XX_X_XX_,
   XXX__XX_,
   XX___XX_,
   XXXXXX__,
   ________};   /* char 'O'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_217[8] = { /* code 217 */
   __X_____,
   ___X____,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char 'U'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_218[8] = { /* code 218 */
   ____X___,
   ___X____,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char 'U'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_219[8] = { /* code 219 */
   ___X____,
   __X_X___,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char 'U'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_220[8] = { /* code 220 */
   _X___X__,
   ________,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char 'U'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_221[8] = { /* code 221 */
   ____X___,
   ___X____,
   _XX__XX_,
   _XX__XX_,
   __XXXX__,
   ___XX___,
   __XXXX__,
   ________};   /* char 'Y'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_222[8] = { /* code 222 */
   _X______,
   _XXXXX__,
   _X____X_,
   _X____X_,
   _X____X_,
   _XXXXX__,
   _X______,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_223[8] = { /* code 223 */
   ________,
   __XXX___,
   _X___X__,
   _X___X__,
   _X_XXX__,
   _X____X_,
   _X_X__X_,
   _X__XX__};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_224[8] = { /* code 224 */
   __X_____,
   ___X____,
   _XXXX___,
   ____XX__,
   _XXXXX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_225[8] = { /* code 225 */
   ___X____,
   __X_____,
   _XXXX___,
   ____XX__,
   _XXXXX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_226[8] = { /* code 226 */
   ___X____,
   __X_X___,
   _XXXX___,
   ____XX__,
   _XXXXX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_227[8] = { /* code 227 */
   _XXX_XX_,
   XX_XXX__,
   _XXXX___,
   ____XX__,
   _XXXXX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_228[8] = { /* code 228 */
   _X___X__,
   ________,
   _XXXX___,
   ____XX__,
   _XXXXX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_229[8] = { /* code 229 */
   __XXX___,
   __X_X___,
   _XXXX___,
   ____XX__,
   _XXXXX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_230[8] = { /* code 230 */
   ________,
   ________,
   _XXX_XX_,
   ___XX_X_,
   _XXXXXX_,
   XX_XX___,
   _XX_XXX_,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_231[8] = { /* code 231 */
   ________,
   ________,
   _XXXXX__,
   XX___XX_,
   XX______,
   XX___XX_,
   _XXXXX__,
   _XX_____};   /* cedilla */

GUI_CONST_STORAGE unsigned char acF8x8_1_232[8] = { /* code 232 */
   __X_____,
   ___X____,
   _XXXXX__,
   XX___XX_,
   XXXXXXX_,
   XX______,
   _XXXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_233[8] = { /* code 233 */
   ____X___,
   ___X____,
   _XXXXX__,
   XX___XX_,
   XXXXXXX_,
   XX______,
   _XXXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_234[8] = { /* code 234 */
   ___X____,
   __X_X___,
   _XXXXX__,
   XX___XX_,
   XXXXXXX_,
   XX______,
   _XXXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_235[8] = { /* code 235 */
   _X___X__,
   ________,
   _XXXXX__,
   XX___XX_,
   XXXXXXX_,
   XX______,
   _XXXXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_236[8] = { /* code 236 */
   __X_____,
   ___X____,
   __XXX___,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_237[8] = { /* code 237 */
   _____X__,
   ____X___,
   __XXX___,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_238[8] = { /* code 238 */
   ___XX___,
   __X__X__,
   __XXX___,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_239[8] = { /* code 239 */
   __X__X__,
   ________,
   __XXX___,
   ___XX___,
   ___XX___,
   ___XX___,
   __XXXX__,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_240[8] = { /* code 240 */
   __XXX___,
   ____XX__,
   _XXXXXX_,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};   /* char 'o'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_241[8] = { /* code 241 */
   _XXX_XX_,
   XX_XXX__,
   XX_XXX__,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   _XX__XX_,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_242[8] = { /* code 242 */
   __X_____,
   ___X____,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_243[8] = { /* code 243 */
   ____X___,
   ___X____,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_244[8] = { /* code 244 */
   ___X____,
   __X_X___,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_245[8] = { /* code 245 */
   _XXX_XX_,
   XX_XXX__,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_246[8] = { /* code 246 */
   _X___X__,
   ________,
   _XXXXX__,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXX__,
   ________};

GUI_CONST_STORAGE unsigned char acF8x8_1_247[8] = { /* code 247 */
   ________,
   ________,
   ___X____,
   ________,
   _XXXXX__,
   ________,
   ___X____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_248[8] = { /* code 248 */
   ________,
   ______X_,
   _XXXXX__,
   XX__XXX_,
   XX_X_XX_,
   XXX__XX_,
   _XXXXX__,
   X_______};   /* char 'o'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_249[8] = { /* code 249 */
   __X_____,
   ___X____,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'u'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_250[8] = { /* code 250 */
   ___X____,
   __X_____,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'u'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_251[8] = { /* code 251 */
   __XX____,
   _X__X___,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'u'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_252[8] = { /* code 252 */
   _X__X___,
   ________,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   XX__XX__,
   _XXX_XX_,
   ________};   /* char 'u'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_253[8] = { /* code 253 */
   ____X___,
   ___X____,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXXX_,
   _____XX_,
   XXXXXX__};   /* char 'y'  */

GUI_CONST_STORAGE unsigned char acF8x8_1_254[8] = { /* code 254 */
   ________,
   _X______,
   _XXXX___,
   _X___X__,
   _X___X__,
   _XXXX___,
   _X______,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF8x8_1_255[8] = { /* code 255 */
   _X___X__,
   ________,
   XX___XX_,
   XX___XX_,
   XX___XX_,
   _XXXXXX_,
   _____XX_,
   XXXXXX__};   /* char 'y'  */

GUI_CONST_STORAGE GUI_CHARINFO GUI_CharInfoF8x8_1[96] = {
   {   8,   8,  1, acF8x8_1_160 } /* code 160 */
  ,{   8,   8,  1, acF8x8_1_161 } /* code 161 */
  ,{   8,   8,  1, acF8x8_1_162 } /* code 162 */
  ,{   8,   8,  1, acF8x8_1_163 } /* code 163 */
  ,{   8,   8,  1, acF8x8_1_164 } /* code 164 */
  ,{   8,   8,  1, acF8x8_1_165 } /* code 165 */
  ,{   8,   8,  1, acF8x8_1_166 } /* code 166 */
  ,{   8,   8,  1, acF8x8_1_167 } /* code 167 */
  ,{   8,   8,  1, acF8x8_1_168 } /* code 168 */
  ,{   8,   8,  1, acF8x8_1_169 } /* code 169 */
  ,{   8,   8,  1, acF8x8_1_170 } /* code 170 */
  ,{   8,   8,  1, acF8x8_1_171 } /* code 171 */
  ,{   8,   8,  1, acF8x8_1_172 } /* code 172 */
  ,{   8,   8,  1, acF8x8_1_173 } /* code 173 */
  ,{   8,   8,  1, acF8x8_1_174 } /* code 174 */
  ,{   8,   8,  1, acF8x8_1_175 } /* code 175 */
  ,{   8,   8,  1, acF8x8_1_176 } /* code 176 */
  ,{   8,   8,  1, acF8x8_1_177 } /* code 177 */
  ,{   8,   8,  1, acF8x8_1_178 } /* code 178 */
  ,{   8,   8,  1, acF8x8_1_179 } /* code 179 */
  ,{   8,   8,  1, acF8x8_1_180 } /* code 180 */
  ,{   8,   8,  1, acF8x8_1_181 } /* code 181 */
  ,{   8,   8,  1, acF8x8_1_182 } /* code 182 */
  ,{   8,   8,  1, acF8x8_1_183 } /* code 183 */
  ,{   8,   8,  1, acF8x8_1_184 } /* code 184 */
  ,{   8,   8,  1, acF8x8_1_185 } /* code 185 */
  ,{   8,   8,  1, acF8x8_1_186 } /* code 186 */
  ,{   8,   8,  1, acF8x8_1_187 } /* code 187 */
  ,{   8,   8,  1, acF8x8_1_188 } /* code 188 */
  ,{   8,   8,  1, acF8x8_1_189 } /* code 189 */
  ,{   8,   8,  1, acF8x8_1_190 } /* code 190 */
  ,{   8,   8,  1, acF8x8_1_191 } /* code 191 */
  ,{   8,   8,  1, acF8x8_1_192 } /* code 192 */
  ,{   8,   8,  1, acF8x8_1_193 } /* code 193 */
  ,{   8,   8,  1, acF8x8_1_194 } /* code 194 */
  ,{   8,   8,  1, acF8x8_1_195 } /* code 195 */
  ,{   8,   8,  1, acF8x8_1_196 } /* code 196 */
  ,{   8,   8,  1, acF8x8_1_197 } /* code 197 */
  ,{   8,   8,  1, acF8x8_1_198 } /* code 198 */
  ,{   8,   8,  1, acF8x8_1_199 } /* code 199 */
  ,{   8,   8,  1, acF8x8_1_200 } /* code 200 */
  ,{   8,   8,  1, acF8x8_1_201 } /* code 201 */
  ,{   8,   8,  1, acF8x8_1_202 } /* code 202 */
  ,{   8,   8,  1, acF8x8_1_203 } /* code 203 */
  ,{   8,   8,  1, acF8x8_1_204 } /* code 204 */
  ,{   8,   8,  1, acF8x8_1_205 } /* code 205 */
  ,{   8,   8,  1, acF8x8_1_206 } /* code 206 */
  ,{   8,   8,  1, acF8x8_1_207 } /* code 207 */
  ,{   8,   8,  1, acF8x8_1_208 } /* code 208 */
  ,{   8,   8,  1, acF8x8_1_209 } /* code 209 */
  ,{   8,   8,  1, acF8x8_1_210 } /* code 210 */
  ,{   8,   8,  1, acF8x8_1_211 } /* code 211 */
  ,{   8,   8,  1, acF8x8_1_212 } /* code 212 */
  ,{   8,   8,  1, acF8x8_1_213 } /* code 213 */
  ,{   8,   8,  1, acF8x8_1_214 } /* code 214 */
  ,{   8,   8,  1, acF8x8_1_215 } /* code 215 */
  ,{   8,   8,  1, acF8x8_1_216 } /* code 216 */
  ,{   8,   8,  1, acF8x8_1_217 } /* code 217 */
  ,{   8,   8,  1, acF8x8_1_218 } /* code 218 */
  ,{   8,   8,  1, acF8x8_1_219 } /* code 219 */
  ,{   8,   8,  1, acF8x8_1_220 } /* code 220 */
  ,{   8,   8,  1, acF8x8_1_221 } /* code 221 */
  ,{   8,   8,  1, acF8x8_1_222 } /* code 222 */
  ,{   8,   8,  1, acF8x8_1_223 } /* code 223 */
  ,{   8,   8,  1, acF8x8_1_224 } /* code 224 */
  ,{   8,   8,  1, acF8x8_1_225 } /* code 225 */
  ,{   8,   8,  1, acF8x8_1_226 } /* code 226 */
  ,{   8,   8,  1, acF8x8_1_227 } /* code 227 */
  ,{   8,   8,  1, acF8x8_1_228 } /* code 228 */
  ,{   8,   8,  1, acF8x8_1_229 } /* code 229 */
  ,{   8,   8,  1, acF8x8_1_230 } /* code 230 */
  ,{   8,   8,  1, acF8x8_1_231 } /* code 231 */
  ,{   8,   8,  1, acF8x8_1_232 } /* code 232 */
  ,{   8,   8,  1, acF8x8_1_233 } /* code 233 */
  ,{   8,   8,  1, acF8x8_1_234 } /* code 234 */
  ,{   8,   8,  1, acF8x8_1_235 } /* code 235 */
  ,{   8,   8,  1, acF8x8_1_236 } /* code 236 */
  ,{   8,   8,  1, acF8x8_1_237 } /* code 237 */
  ,{   8,   8,  1, acF8x8_1_238 } /* code 238 */
  ,{   8,   8,  1, acF8x8_1_239 } /* code 239 */
  ,{   8,   8,  1, acF8x8_1_240 } /* code 240 */
  ,{   8,   8,  1, acF8x8_1_241 } /* code 241 */
  ,{   8,   8,  1, acF8x8_1_242 } /* code 242 */
  ,{   8,   8,  1, acF8x8_1_243 } /* code 243 */
  ,{   8,   8,  1, acF8x8_1_244 } /* code 244 */
  ,{   8,   8,  1, acF8x8_1_245 } /* code 245 */
  ,{   8,   8,  1, acF8x8_1_246 } /* code 246 */
  ,{   8,   8,  1, acF8x8_1_247 } /* code 247 */
  ,{   8,   8,  1, acF8x8_1_248 } /* code 248 */
  ,{   8,   8,  1, acF8x8_1_249 } /* code 249 */
  ,{   8,   8,  1, acF8x8_1_250 } /* code 250 */
  ,{   8,   8,  1, acF8x8_1_251 } /* code 251 */
  ,{   8,   8,  1, acF8x8_1_252 } /* code 252 */
  ,{   8,   8,  1, acF8x8_1_253 } /* code 253 */
  ,{   8,   8,  1, acF8x8_1_254 } /* code 254 */
  ,{   8,   8,  1, acF8x8_1_255 } /* code 255 */
};

GUI_CONST_STORAGE GUI_FONT_PROP GUI_FontPropF8x8_1 = {
   160                      /* first character               */
  ,255                      /* last character                */
  ,&GUI_CharInfoF8x8_1[0]   /* address of first character    */
  ,&GUI_Font8x8ASCII_Prop0   /* pointer to next GUI_FONT_PROP */
};

GUI_CONST_STORAGE GUI_FONT GUI_Font8x8_1 = {
   GUI_FONTTYPE_PROP  /* type of font    */
  ,8                  /* height of font  */
  ,8                  /* space of font y */
  ,1                  /* magnification x */
  ,1                  /* magnification y */
  ,{&GUI_FontPropF8x8_1}
  , 7, 5, 7
};

/*************************** End of file ****************************/
