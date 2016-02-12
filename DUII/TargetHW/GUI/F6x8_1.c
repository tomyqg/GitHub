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
File        : F6x8_1.C
Purpose     : Monospaced Font similar to Terminal
Height      : 8
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_FontIntern.h"

extern GUI_CONST_STORAGE GUI_FONT_PROP GUI_Font6x8ASCII_Prop0;

GUI_CONST_STORAGE unsigned char acF6x8_1_160[8] = { /* code 160 */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_161[8] = { /* code 161 */
   __X_____,
   ________,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   __X_____,
   ________};   /* inverted exclamation'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_162[8] = { /* code 162 */
   ________,
   __X_____,
   _XXX____,
   X_______,
   X_______,
   X___X___,
   _XXX____,
   __X_____};   /* cent sign */

GUI_CONST_STORAGE unsigned char acF6x8_1_163[8] = { /* code 163 */
   __XX____,
   _X__X___,
   _X______,
   XXX_____,
   _X______,
   _X__X___,
   X_XX____,
   ________};   /* pound sterling */

GUI_CONST_STORAGE unsigned char acF6x8_1_164[8] = { /* code 164 */
   ________,
   ________,
   X___X___,
   _XXX____,
   XX_XX___,
   _XXX____,
   X___X___,
   ________};   /* general currency sign */

GUI_CONST_STORAGE unsigned char acF6x8_1_165[8] = { /* code 165 */
   X___X___,
   _X_X____,
   __X_____,
   _XXX____,
   __X_____,
   _XXX____,
   __X_____,
   ________};   /* yen sign */

GUI_CONST_STORAGE unsigned char acF6x8_1_166[8] = { /* code 166 */
   __X_____,
   __X_____,
   __X_____,
   ________,
   __X_____,
   __X_____,
   __X_____,
   ________};   /* broken vertical bar */

GUI_CONST_STORAGE unsigned char acF6x8_1_167[8] = { /* code 167 */
   __XXX___,
   _X___X__,
   __XX____,
   _X__X___,
   __X__X__,
   ___XX___,
   _X___X__,
   __XXX___};   /* section sign */

GUI_CONST_STORAGE unsigned char acF6x8_1_168[8] = { /* code 168 */
   _X__X___,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* umlaut (dieresis) */

GUI_CONST_STORAGE unsigned char acF6x8_1_169[8] = { /* code 169 */
   _XXXX___,
   X____X__,
   X_XX_X__,
   X_X__X__,
   X_XX_X__,
   X____X__,
   _XXXX___,
   ________};   /* copyright */

GUI_CONST_STORAGE unsigned char acF6x8_1_170[8] = { /* code 170 */
   XX______,
   __X_____,
   XXX_____,
   X_X_____,
   XXX_____,
   ________,
   ________,
   ________};   /* feminine ordinal */

GUI_CONST_STORAGE unsigned char acF6x8_1_171[8] = { /* code 171 */
   ________,
   __X_X___,
   _X_X____,
   X_X_____,
   _X_X____,
   __X_X___,
   ________,
   ________};   /* left angle quote, guillemotleft */

GUI_CONST_STORAGE unsigned char acF6x8_1_172[8] = { /* code 172 */
   ________,
   ________,
   XXXXX___,
   ____X___,
   ____X___,
   ________,
   ________,
   ________};   /* not sign */

GUI_CONST_STORAGE unsigned char acF6x8_1_173[8] = { /* code 173 */
   ________,
   ________,
   ________,
   ________,
   _XXXX___,
   ________,
   ________,
   ________};   /* soft hyphen */

GUI_CONST_STORAGE unsigned char acF6x8_1_174[8] = { /* code 174 */
   XXXXX___,
   X___X___,
   X_X_X___,
   X__XX___,
   X_X_X___,
   X_X_X___,
   XXXXX___,
   ________};   /* registered trademark */

GUI_CONST_STORAGE unsigned char acF6x8_1_175[8] = { /* code 175 */
   XXXXX___,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* macron accent */

GUI_CONST_STORAGE unsigned char acF6x8_1_176[8] = { /* code 176 */
   __X_____,
   _X_X____,
   __X_____,
   ________,
   ________,
   ________,
   ________,
   ________};   /* degree sign */

GUI_CONST_STORAGE unsigned char acF6x8_1_177[8] = { /* code 177 */
   ________,
   __X_____,
   _XXX____,
   __X_____,
   ________,
   _XXX____,
   ________,
   ________};   /* plus or minus */

GUI_CONST_STORAGE unsigned char acF6x8_1_178[8] = { /* code 178 */
   _X______,
   X_X_____,
   __X_____,
   _X______,
   XXX_____,
   ________,
   ________,
   ________};   /* superscript two */

GUI_CONST_STORAGE unsigned char acF6x8_1_179[8] = { /* code 179 */
   XX______,
   __X_____,
   _X______,
   __X_____,
   XX______,
   ________,
   ________,
   ________};   /* superscript three */

GUI_CONST_STORAGE unsigned char acF6x8_1_180[8] = { /* code 180 */
   ___X____,
   __X_____,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________};   /* acute accent */

GUI_CONST_STORAGE unsigned char acF6x8_1_181[8] = { /* code 181 */
   ________,
   ________,
   X__X____,
   X__X____,
   X__X____,
   X__X____,
   XXX_X___,
   X_______};   /* micro sign */

GUI_CONST_STORAGE unsigned char acF6x8_1_182[8] = { /* code 182 */
   _XXXXX__,
   XXX_X___,
   XXX_X___,
   _XX_X___,
   __X_X___,
   __X_X___,
   __X_X___,
   __X_X___};   /* paragraph sign */

GUI_CONST_STORAGE unsigned char acF6x8_1_183[8] = { /* code 183 */
   ________,
   ________,
   ________,
   __X_____,
   ________,
   ________,
   ________,
   ________};   /* middle dot */

GUI_CONST_STORAGE unsigned char acF6x8_1_184[8] = { /* code 184 */
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   ________,
   XX______};   /* cedilla */

GUI_CONST_STORAGE unsigned char acF6x8_1_185[8] = { /* code 185 */
   __X_____,
   _XX_____,
   __X_____,
   __X_____,
   __X_____,
   ________,
   ________,
   ________};   /* superscript one */

GUI_CONST_STORAGE unsigned char acF6x8_1_186[8] = { /* code 186 */
   _X______,
   X_X_____,
   X_X_____,
   X_X_____,
   _X______,
   ________,
   ________,
   ________};   /* masculine ordinal */

GUI_CONST_STORAGE unsigned char acF6x8_1_187[8] = { /* code 187 */
   ________,
   _X_X____,
   __X_X___,
   ___X_X__,
   __X_X___,
   _X_X____,
   ________,
   ________};   /* right angle quote, guillemot right */

GUI_CONST_STORAGE unsigned char acF6x8_1_188[8] = { /* code 188 */
   _X______,
   XX___X__,
   _X__X___,
   _X_X_X__,
   __X_XX__,
   _X_X_X__,
   X__XXX__,
   _____X__};   /* fraction one-fourth */

GUI_CONST_STORAGE unsigned char acF6x8_1_189[8] = { /* code 189 */
   _X______,
   XX___X__,
   _X__X___,
   _X_X____,
   __X_XX__,
   _X___X__,
   X___X___,
   ____XX__};   /* fraction one-half */

GUI_CONST_STORAGE unsigned char acF6x8_1_190[8] = { /* code 190 */
   XX______,
   _X___X__,
   XX__X___,
   _X_X_X__,
   XXX_XX__,
   _X_X_X__,
   X__XXX__,
   _____X__};   /* fraction three-fourth */

GUI_CONST_STORAGE unsigned char acF6x8_1_191[8] = { /* code 191 */
   __X_____,
   ________,
   __X_____,
   __X_____,
   _X______,
   X__X____,
   _XXX____,
   ________};   /* inverted question mark */

GUI_CONST_STORAGE unsigned char acF6x8_1_192[8] = { /* code 192 */
   _X______,
   __X_____,
   _XXX____,
   X___X___,
   XXXXX___,
   X___X___,
   X___X___,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_193[8] = { /* code 193 */
   ___X____,
   __X_____,
   _XXX____,
   X___X___,
   XXXXX___,
   X___X___,
   X___X___,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_194[8] = { /* code 194 */
   __X_____,
   _X_X____,
   _XXX____,
   X___X___,
   XXXXX___,
   X___X___,
   X___X___,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_195[8] = { /* code 195 */
   _XX_X___,
   X__X____,
   _XXX____,
   X___X___,
   XXXXX___,
   X___X___,
   X___X___,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_196[8] = { /* code 196 */
   _X_X____,
   ________,
   _XXX____,
   X___X___,
   XXXXX___,
   X___X___,
   X___X___,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_197[8] = { /* code 197 */
   _XXX____,
   _X_X____,
   _XXX____,
   X___X___,
   XXXXX___,
   X___X___,
   X___X___,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_198[8] = { /* code 198 */
   _XXXXX__,
   _X_X____,
   X__X____,
   XXXXX___,
   X__X____,
   X__X____,
   X__XXX__,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_199[8] = { /* code 199 */
   _XXX____,
   X___X___,
   X_______,
   X_______,
   X_______,
   X___X___,
   _XXX____,
   XX______};   /* cedilla */

GUI_CONST_STORAGE unsigned char acF6x8_1_200[8] = { /* code 200 */
   _X______,
   __X_____,
   XXXXX___,
   X_______,
   XXX_____,
   X_______,
   XXXXX___,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_201[8] = { /* code 201 */
   ___X____,
   __X_____,
   XXXXX___,
   X_______,
   XXX_____,
   X_______,
   XXXXX___,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_202[8] = { /* code 202 */
   __X_____,
   _X_X____,
   XXXXX___,
   X_______,
   XXX_____,
   X_______,
   XXXXX___,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_203[8] = { /* code 203 */
   _X_X____,
   ________,
   XXXXX___,
   X_______,
   XXX_____,
   X_______,
   XXXXX___,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_204[8] = { /* code 204 */
   _X______,
   __X_____,
   _XXX____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_205[8] = { /* code 205 */
   ___X____,
   __X_____,
   _XXX____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_206[8] = { /* code 206 */
   __X_____,
   _X_X____,
   _XXX____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_207[8] = { /* code 207 */
   _X_X____,
   ________,
   _XXX____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_208[8] = { /* code 208 */
   XXX_____,
   _X_X____,
   _X__X___,
   XXX_X___,
   _X__X___,
   _X_X____,
   XXX_____,
   ________};   /* char 'D'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_209[8] = { /* code 209 */
   _XX_X___,
   X__X____,
   X___X___,
   XX__X___,
   X_X_X___,
   X__XX___,
   X___X___,
   ________};  /* char 'N'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_210[8] = { /* code 210 */
   _X______,
   __X_____,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_211[8] = { /* code 211 */
   ___X____,
   __X_____,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_212[8] = { /* code 212 */
   __X_____,
   _X_X____,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_213[8] = { /* code 213 */
   _XX_X___,
   X__X____,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_214[8] = { /* code 214 */
   _X_X____,
   ________,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_215[8] = { /* code 215 */
   ________,
   ________,
   X___X___,
   _X_X____,
   __X_____,
   _X_X____,
   X___X___,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_216[8] = { /* code 216 */
   _XXX____,
   X___X___,
   X__XX___,
   X_X_X___,
   XX__X___,
   X___X___,
   _XXX____,
   ________};   /* char 'O'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_217[8] = { /* code 217 */
   _X______,
   __X_____,
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char 'U'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_218[8] = { /* code 218 */
   ___X____,
   __X_____,
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char 'U'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_219[8] = { /* code 219 */
   __X_____,
   _X_X____,
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char 'U'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_220[8] = { /* code 220 */
   _X_X____,
   ________,
   X___X___,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char 'U'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_221[8] = { /* code 221 */
   ___X____,
   __X_____,
   X___X___,
   X___X___,
   _XXX____,
   __X_____,
   _XXX____,
   ________};   /* char 'Y'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_222[8] = { /* code 222 */
   _X______,
   _XXX____,
   _X__X___,
   _X__X___,
   _X__X___,
   _XXX____,
   _X______,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_223[8] = { /* code 223 */
   ________,
   __XX____,
   _X__X___,
   _X__X___,
   _X_XX___,
   _X___X__,
   _X_X_X__,
   _X__X___};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_224[8] = { /* code 224 */
   _X______,
   __X_____,
   _XXX____,
   ____X___,
   _XXXX___,
   X___X___,
   _XXXX___,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_225[8] = { /* code 225 */
   ___X____,
   __X_____,
   _XXX____,
   ____X___,
   _XXXX___,
   X___X___,
   _XXXX___,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_226[8] = { /* code 226 */
   __X_____,
   _X_X____,
   _XXX____,
   ____X___,
   _XXXX___,
   X___X___,
   _XXXX___,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_227[8] = { /* code 227 */
   _XX_X___,
   X__X____,
   _XXX____,
   ____X___,
   _XXXX___,
   X___X___,
   _XXXX___,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_228[8] = { /* code 228 */
   _X_X____,
   ________,
   _XXX____,
   ____X___,
   _XXXX___,
   X___X___,
   _XXXX___,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_229[8] = { /* code 229 */
   _XXX____,
   _X_X____,
   _XXX____,
   ____X___,
   _XXXX___,
   X___X___,
   _XXXX___,
   ________};   /* char 'a'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_230[8] = { /* code 230 */
   ________,
   ________,
   _XXXX___,
   __X_X___,
   _XXXX___,
   X_X_____,
   _XXXX___,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_231[8] = { /* code 231 */
   ________,
   ________,
   _XXX____,
   X_______,
   X_______,
   X___X___,
   _XXX____,
   XX______};   /* c cedilla */

GUI_CONST_STORAGE unsigned char acF6x8_1_232[8] = { /* code 232 */
   _X______,
   __X_____,
   _XXX____,
   X___X___,
   XXXXX___,
   X_______,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_233[8] = { /* code 233 */
   ___X____,
   __X_____,
   _XXX____,
   X___X___,
   XXXXX___,
   X_______,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_234[8] = { /* code 234 */
   __X_____,
   _X_X____,
   _XXX____,
   X___X___,
   XXXXX___,
   X_______,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_235[8] = { /* code 235 */
   _X_X____,
   ________,
   _XXX____,
   X___X___,
   XXXXX___,
   X_______,
   _XXX____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_236[8] = { /* code 236 */
   _X______,
   __X_____,
   _XX_____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_237[8] = { /* code 237 */
   ___X____,
   __X_____,
   _XX_____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_238[8] = { /* code 238 */
   __X_____,
   _X_X____,
   _XX_____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_239[8] = { /* code 239 */
   _X_X____,
   ________,
   _XX_____,
   __X_____,
   __X_____,
   __X_____,
   _XXX____,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_240[8] = { /* code 240 */
   __XX____,
   ____X___,
   _XXXX___,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};   /* char 'o'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_241[8] = { /* code 241 */
   _XX_X___,
   X__X____,
   X_XX____,
   XX__X___,
   X___X___,
   X___X___,
   X___X___,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_242[8] = { /* code 242 */
   _X______,
   __X_____,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_243[8] = { /* code 243 */
   ___X____,
   __X_____,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_244[8] = { /* code 244 */
   __X_____,
   _X_X____,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_245[8] = { /* code 245 */
   _XX_X___,
   X__X____,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_246[8] = { /* code 246 */
   _X_X____,
   ________,
   _XXX____,
   X___X___,
   X___X___,
   X___X___,
   _XXX____,
   ________};

GUI_CONST_STORAGE unsigned char acF6x8_1_247[8] = { /* code 247 */
   ________,
   ________,
   __X_____,
   ________,
   XXXXX___,
   ________,
   __X_____,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_248[8] = { /* code 248 */
   ________,
   ________,
   _XXXX___,
   X__XX___,
   X_X_X___,
   XX__X___,
   XXXX____,
   ________};   /* char 'o'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_249[8] = { /* code 249 */
   _X______,
   __X_____,
   X___X___,
   X___X___,
   X___X___,
   X__XX___,
   _XX_X___,
   ________};   /* char 'u'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_250[8] = { /* code 250 */
   ___X____,
   __X_____,
   X___X___,
   X___X___,
   X___X___,
   X__XX___,
   _XX_X___,
   ________};   /* char 'u'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_251[8] = { /* code 251 */
   __X_____,
   _X_X____,
   X___X___,
   X___X___,
   X___X___,
   X__XX___,
   _XX_X___,
   ________};   /* char 'u'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_252[8] = { /* code 252 */
   _X_X____,
   ________,
   X___X___,
   X___X___,
   X___X___,
   X__XX___,
   _XX_X___,
   ________};   /* char 'u'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_253[8] = { /* code 253 */
   ___X____,
   __X_____,
   X___X___,
   X___X___,
   X___X___,
   _XXXX___,
   ____X___,
   _XXX____};   /* char 'y'  */

GUI_CONST_STORAGE unsigned char acF6x8_1_254[8] = { /* code 254 */
   ________,
   _X______,
   _XXX____,
   _X__X___,
   _X__X___,
   _XXX____,
   _X______,
   ________};   /* char ' '  */

GUI_CONST_STORAGE unsigned char acF6x8_1_255[8] = { /* code 255 */
   _X_X____,
   ________,
   X___X___,
   X___X___,
   X___X___,
   _XXXX___,
   ____X___,
   _XXX____};   /* char 'y'  */

GUI_CONST_STORAGE GUI_CHARINFO GUI_CharInfoF6x8_1[96] = {
   {   6,   6,  1, acF6x8_1_160 } /* code 160 */
  ,{   6,   6,  1, acF6x8_1_161 } /* code 161 */
  ,{   6,   6,  1, acF6x8_1_162 } /* code 162 */
  ,{   6,   6,  1, acF6x8_1_163 } /* code 163 */
  ,{   6,   6,  1, acF6x8_1_164 } /* code 164 */
  ,{   6,   6,  1, acF6x8_1_165 } /* code 165 */
  ,{   6,   6,  1, acF6x8_1_166 } /* code 166 */
  ,{   6,   6,  1, acF6x8_1_167 } /* code 167 */
  ,{   6,   6,  1, acF6x8_1_168 } /* code 168 */
  ,{   6,   6,  1, acF6x8_1_169 } /* code 169 */
  ,{   6,   6,  1, acF6x8_1_170 } /* code 170 */
  ,{   6,   6,  1, acF6x8_1_171 } /* code 171 */
  ,{   6,   6,  1, acF6x8_1_172 } /* code 172 */
  ,{   6,   6,  1, acF6x8_1_173 } /* code 173 */
  ,{   6,   6,  1, acF6x8_1_174 } /* code 174 */
  ,{   6,   6,  1, acF6x8_1_175 } /* code 175 */
  ,{   6,   6,  1, acF6x8_1_176 } /* code 176 */
  ,{   6,   6,  1, acF6x8_1_177 } /* code 177 */
  ,{   6,   6,  1, acF6x8_1_178 } /* code 178 */
  ,{   6,   6,  1, acF6x8_1_179 } /* code 179 */
  ,{   6,   6,  1, acF6x8_1_180 } /* code 180 */
  ,{   6,   6,  1, acF6x8_1_181 } /* code 181 */
  ,{   6,   6,  1, acF6x8_1_182 } /* code 182 */
  ,{   6,   6,  1, acF6x8_1_183 } /* code 183 */
  ,{   6,   6,  1, acF6x8_1_184 } /* code 184 */
  ,{   6,   6,  1, acF6x8_1_185 } /* code 185 */
  ,{   6,   6,  1, acF6x8_1_186 } /* code 186 */
  ,{   6,   6,  1, acF6x8_1_187 } /* code 187 */
  ,{   6,   6,  1, acF6x8_1_188 } /* code 188 */
  ,{   6,   6,  1, acF6x8_1_189 } /* code 189 */
  ,{   6,   6,  1, acF6x8_1_190 } /* code 190 */
  ,{   6,   6,  1, acF6x8_1_191 } /* code 191 */
  ,{   6,   6,  1, acF6x8_1_192 } /* code 192 */
  ,{   6,   6,  1, acF6x8_1_193 } /* code 193 */
  ,{   6,   6,  1, acF6x8_1_194 } /* code 194 */
  ,{   6,   6,  1, acF6x8_1_195 } /* code 195 */
  ,{   6,   6,  1, acF6x8_1_196 } /* code 196 */
  ,{   6,   6,  1, acF6x8_1_197 } /* code 197 */
  ,{   6,   6,  1, acF6x8_1_198 } /* code 198 */
  ,{   6,   6,  1, acF6x8_1_199 } /* code 199 */
  ,{   6,   6,  1, acF6x8_1_200 } /* code 200 */
  ,{   6,   6,  1, acF6x8_1_201 } /* code 201 */
  ,{   6,   6,  1, acF6x8_1_202 } /* code 202 */
  ,{   6,   6,  1, acF6x8_1_203 } /* code 203 */
  ,{   6,   6,  1, acF6x8_1_204 } /* code 204 */
  ,{   6,   6,  1, acF6x8_1_205 } /* code 205 */
  ,{   6,   6,  1, acF6x8_1_206 } /* code 206 */
  ,{   6,   6,  1, acF6x8_1_207 } /* code 207 */
  ,{   6,   6,  1, acF6x8_1_208 } /* code 208 */
  ,{   6,   6,  1, acF6x8_1_209 } /* code 209 */
  ,{   6,   6,  1, acF6x8_1_210 } /* code 210 */
  ,{   6,   6,  1, acF6x8_1_211 } /* code 211 */
  ,{   6,   6,  1, acF6x8_1_212 } /* code 212 */
  ,{   6,   6,  1, acF6x8_1_213 } /* code 213 */
  ,{   6,   6,  1, acF6x8_1_214 } /* code 214 */
  ,{   6,   6,  1, acF6x8_1_215 } /* code 215 */
  ,{   6,   6,  1, acF6x8_1_216 } /* code 216 */
  ,{   6,   6,  1, acF6x8_1_217 } /* code 217 */
  ,{   6,   6,  1, acF6x8_1_218 } /* code 218 */
  ,{   6,   6,  1, acF6x8_1_219 } /* code 219 */
  ,{   6,   6,  1, acF6x8_1_220 } /* code 220 */
  ,{   6,   6,  1, acF6x8_1_221 } /* code 221 */
  ,{   6,   6,  1, acF6x8_1_222 } /* code 222 */
  ,{   6,   6,  1, acF6x8_1_223 } /* code 223 */
  ,{   6,   6,  1, acF6x8_1_224 } /* code 224 */
  ,{   6,   6,  1, acF6x8_1_225 } /* code 225 */
  ,{   6,   6,  1, acF6x8_1_226 } /* code 226 */
  ,{   6,   6,  1, acF6x8_1_227 } /* code 227 */
  ,{   6,   6,  1, acF6x8_1_228 } /* code 228 */
  ,{   6,   6,  1, acF6x8_1_229 } /* code 229 */
  ,{   6,   6,  1, acF6x8_1_230 } /* code 230 */
  ,{   6,   6,  1, acF6x8_1_231 } /* code 231 */
  ,{   6,   6,  1, acF6x8_1_232 } /* code 232 */
  ,{   6,   6,  1, acF6x8_1_233 } /* code 233 */
  ,{   6,   6,  1, acF6x8_1_234 } /* code 234 */
  ,{   6,   6,  1, acF6x8_1_235 } /* code 235 */
  ,{   6,   6,  1, acF6x8_1_236 } /* code 236 */
  ,{   6,   6,  1, acF6x8_1_237 } /* code 237 */
  ,{   6,   6,  1, acF6x8_1_238 } /* code 238 */
  ,{   6,   6,  1, acF6x8_1_239 } /* code 239 */
  ,{   6,   6,  1, acF6x8_1_240 } /* code 240 */
  ,{   6,   6,  1, acF6x8_1_241 } /* code 241 */
  ,{   6,   6,  1, acF6x8_1_242 } /* code 242 */
  ,{   6,   6,  1, acF6x8_1_243 } /* code 243 */
  ,{   6,   6,  1, acF6x8_1_244 } /* code 244 */
  ,{   6,   6,  1, acF6x8_1_245 } /* code 245 */
  ,{   6,   6,  1, acF6x8_1_246 } /* code 246 */
  ,{   6,   6,  1, acF6x8_1_247 } /* code 247 */
  ,{   6,   6,  1, acF6x8_1_248 } /* code 248 */
  ,{   6,   6,  1, acF6x8_1_249 } /* code 249 */
  ,{   6,   6,  1, acF6x8_1_250 } /* code 250 */
  ,{   6,   6,  1, acF6x8_1_251 } /* code 251 */
  ,{   6,   6,  1, acF6x8_1_252 } /* code 252 */
  ,{   6,   6,  1, acF6x8_1_253 } /* code 253 */
  ,{   6,   6,  1, acF6x8_1_254 } /* code 254 */
  ,{   6,   6,  1, acF6x8_1_255 } /* code 255 */
};

GUI_CONST_STORAGE GUI_FONT_PROP GUI_FontPropF6x8_1 = {
   160                      /* first character               */
  ,255                      /* last character                */
  ,&GUI_CharInfoF6x8_1[0]   /* address of first character    */
  ,&GUI_Font6x8ASCII_Prop0   /* pointer to next GUI_FONT_PROP */
};

GUI_CONST_STORAGE GUI_FONT GUI_Font6x8_1 = {
   GUI_FONTTYPE_PROP  /* type of font    */
  ,8                  /* height of font  */
  ,8                  /* space of font y */
  ,1                  /* magnification x */
  ,1                  /* magnification y */
  ,{&GUI_FontPropF6x8_1}
  , 7, 5, 7
};

/*************************** End of file ****************************/
