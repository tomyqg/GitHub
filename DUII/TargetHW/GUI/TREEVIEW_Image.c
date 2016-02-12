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
File        : TREEVIEW_Image.c
Purpose     : Implementation of TREEVIEW widget
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "TREEVIEW_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Open/Closed
*/
static GUI_CONST_STORAGE GUI_COLOR ColorsOpenClosed[] = {
  0x0000FF,0x000000,0x7F7F7F,0xE0E0E0,0x60FFFF
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalOpenClosed = {
  5, /* number of entries */
  1, /* Has transparency */
  &ColorsOpenClosed[0]
};

static GUI_CONST_STORAGE unsigned char acOpen[] = {
  0x00, 0x02, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00,
  0x00, 0x02, 0x33, 0x33, 0x32, 0x00, 0x00, 0x00,
  0x00, 0x22, 0x33, 0x33, 0x32, 0x22, 0x22, 0x20,
  0x00, 0x23, 0x33, 0x33, 0x33, 0x33, 0x33, 0x21,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x33, 0x21,
  0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x23, 0x21,
  0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x23, 0x21,
  0x02, 0x44, 0x44, 0x44, 0x44, 0x44, 0x42, 0x21,
  0x02, 0x44, 0x44, 0x44, 0x44, 0x44, 0x42, 0x21,
  0x00, 0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x21,
  0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x21,
  0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
};

static GUI_CONST_STORAGE unsigned char acClosed[] = {
  0x00, 0x02, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00,
  0x00, 0x02, 0x33, 0x33, 0x32, 0x00, 0x00, 0x00,
  0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x20,
  0x00, 0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x21,
  0x00, 0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x21,
  0x00, 0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x21,
  0x00, 0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x21,
  0x00, 0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x21,
  0x00, 0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x21,
  0x00, 0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x21,
  0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x21,
  0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
};

GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmOpen = {
  16,            /* XSize */
  12,            /* YSize */
  8,             /* BytesPerLine */
  4,             /* BitsPerPixel */
  acOpen,        /* Pointer to picture data (indices) */
  &PalOpenClosed /* Pointer to palette */
};

GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmClosed = {
  16,            /* XSize */
  12,            /* YSize */
  8,             /* BytesPerLine */
  4,             /* BitsPerPixel */
  acClosed,      /* Pointer to picture data (indices) */
  &PalOpenClosed /* Pointer to palette */
};

/*********************************************************************
*
*       Leaf
*/
static GUI_CONST_STORAGE GUI_COLOR ColorsLeaf[] = {
  0x0000FF,0xF0F0F0,0x000000,0x7F7F7F
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalLeaf = {
  4, /* number of entries */
  1, /* Has transparency */
  &ColorsLeaf[0]
};

static GUI_CONST_STORAGE unsigned char acLeaf[] = {
  0xFF, 0xFF, 0xC0,
  0xD5, 0x55, 0xF0,
  0xD5, 0x55, 0xDC,
  0xDF, 0xFD, 0xAA,
  0xD5, 0x55, 0x5E,
  0xDF, 0xFF, 0xDE,
  0xD5, 0x55, 0x5E,
  0xDF, 0xFF, 0xDE,
  0xD5, 0x55, 0x5E,
  0xDF, 0xFF, 0xDE,
  0xD5, 0x55, 0x5E,
  0xDF, 0xFF, 0xDE,
  0xD5, 0x55, 0x5E,
  0xFF, 0xFF, 0xFE,
  0x2A, 0xAA, 0xAA
};

GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmLeaf = {
  12,      /* XSize */
  15,      /* YSize */
  3,       /* BytesPerLine */
  2,       /* BitsPerPixel */
  acLeaf,  /* Pointer to picture data (indices) */
  &PalLeaf /* Pointer to palette */
};

/*********************************************************************
*
*       Plus/Minus
*/
static GUI_CONST_STORAGE GUI_COLOR ColorsPlusMinus[] = {
  0xFFFFFF,0x000000
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalPlusMinus = {
  2, /* number of entries */
  0, /* No transparency */
  &ColorsPlusMinus[0]
};

static GUI_CONST_STORAGE unsigned char acMinus[] = {
  XXXXXXXX, X_______,
  X_______, X_______,
  X_______, X_______,
  X_______, X_______,
  X_XXXXX_, X_______,
  X_______, X_______,
  X_______, X_______,
  X_______, X_______,
  XXXXXXXX, X_______
};

static GUI_CONST_STORAGE unsigned char acPlus[] = {
  XXXXXXXX, X_______,
  X_______, X_______,
  X___X___, X_______,
  X___X___, X_______,
  X_XXXXX_, X_______,
  X___X___, X_______,
  X___X___, X_______,
  X_______, X_______,
  XXXXXXXX, X_______
};

GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmPlus = {
  9,            /* XSize */
  9,            /* YSize */
  2,            /* BytesPerLine */
  1,            /* BitsPerPixel */
  acPlus,       /* Pointer to picture data (indices) */
  &PalPlusMinus /* Pointer to palette */
};

GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmMinus = {
  9,            /* XSize */
  9,            /* YSize */
  2,            /* BytesPerLine */
  1,            /* BitsPerPixel */
  acMinus,      /* Pointer to picture data (indices) */
  &PalPlusMinus /* Pointer to palette */
};

#else

void TREEVIEW_Image_C(void);
void TREEVIEW_Image_C(void){}

#endif

/*************************** End of file ****************************/
