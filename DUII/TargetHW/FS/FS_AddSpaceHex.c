/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

**** emFile file system for embedded applications ****
emFile is protected by international copyright laws. Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : FS_AddSpaceHex.c
Purpose     : add a hex number to string
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "FS_ConfDefaults.h"        /* FS Configuration */
#include "FS_Int.h"


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static const char _acHex[16] =  {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _AddHex
*/
static void _AddHex(U32 v, U8 Len, char**ps) {
  char *s = *ps;
  if (Len > 8) {
    return;
  }
  (*ps) += Len;
  **ps   = '\0';     /* Make sure string is 0-terminated */
  while(Len--) {
    *(s + Len) = _acHex[v & 15];
    v >>= 4;
  }
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__AddSpaceHex
*/
void FS__AddSpaceHex(U32 v, U8 Len, char** ps) {
  char* s = *ps;
  *s++ = ' ';
  *ps = s;
  _AddHex(v, Len, ps);
}

/*************************** End of file ****************************/
