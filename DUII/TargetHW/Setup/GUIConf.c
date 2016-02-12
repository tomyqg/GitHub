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

** emWin V5.00 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIConf.c
Purpose     : Display controller initialization
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
//
// Define the available number of bytes available for the GUI
//
// KMC #define GUI_NUMBYTES  16 * 1024 * 1024  // 16 MByte
// 16MB gives Memory Overflow error wren attempting to program flash with SAM-BA
// 1 MB is the magic number that will program into flash, run on the eval board and on the PC
// KMC TODO - this needs to be understood!!!
// try same size as application #define GUI_NUMBYTES  1 * 1024 * 1024
// This size works on PC, JTAG and FLASH
#define GUI_NUMBYTES  0x200000

//
// Define the average block size
//
#define GUI_BLOCKSIZE 0x80

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   available memory for the GUI.
*/
void GUI_X_Config(void) {
  //
  // 32 bit aligned memory area
  //
  static U32 aMemory[GUI_NUMBYTES / 4];
  //
  // Assign memory to emWin
  //
  GUI_ALLOC_AssignMemory(aMemory, GUI_NUMBYTES);
  GUI_ALLOC_SetAvBlockSize(GUI_BLOCKSIZE);
}

/*************************** End of file ****************************/
