/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2002         SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

**** emWin/GSC Grafical user interface for embedded applications ****
emWin is protected by international copyright laws. Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIConf.h
Purpose     : Configures emWins abilities, fonts etc.
----------------------------------------------------------------------
*/

#ifndef GUICONF_H
#define GUICONF_H

/*********************************************************************
*
*       Multi layer/display support
*/
#define GUI_NUM_LAYERS            1    // Maximum number of available layers

/*********************************************************************
*
*       Multi tasking support
*/
#define GUI_OS                    (0)  // Compile with multitasking support

/*********************************************************************
*
*       Configuration of touch support
*/
#define GUI_SUPPORT_TOUCH         (1)  // Support a touch screen (req. win-manager)

/*********************************************************************
*
*       Default font
*/
#define GUI_DEFAULT_FONT          &GUI_Font6x8

/*********************************************************************
*
*         Configuration of available packages
*/
#define GUI_SUPPORT_MOUSE             1    /* Support a mouse */
#define GUI_WINSUPPORT                1    /* Use window manager */
#define GUI_SUPPORT_MEMDEV            1    /* Memory device package available */

// NOTE!  THE HARDWARE BUILD HAS A DIFFERENT GUIConf.h!!!
// COPY CHANGES FROM HERE TO THERE
// KMC
#define	BUTTON_REACT_ON_LEVEL		1
///* Define colors */
//// KMC - turn these defaults off to use bitmap buttons
//  #define BUTTON_BKCOLOR0_DEFAULT   GUI_BLUE
//  #define BUTTON_BKCOLOR1_DEFAULT   GUI_WHITE
//  #define BUTTON_BKCOLOR2_DEFAULT   GUI_LIGHTCYAN
  #define BUTTON_TEXTCOLOR0_DEFAULT GUI_WHITE
//  #define BUTTON_TEXTCOLOR1_DEFAULT GUI_BLACK
//  #define BUTTON_TEXTCOLOR2_DEFAULT GUI_LIGHTGREEN
  #define BUTTON_FOCUSCOLOR_DEFAULT GUI_GRAY
//  #define BUTTON_FRAMECOLOR_DEFAULT GUI_GRAY
#define	BUTTON_3D_MOVE_X	0
#define	BUTTON_3D_MOVE_Y	0

// Define ICONVIEW selected state background color as transparent
// = max alpha blending (0xFF) on BLACK - any color would do (0xxxxxxx)
#define ICONVIEW_BKCOLOR1_DEFAULT 0xFF000000

#endif  /* Avoid multiple inclusion */
