/*********************************************************************
*               SEGGER MICROCONTROLLER SYSTEME GmbH                  *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2008  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS version: 3.52e                                         *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : GUI_HelloWorld.c
Purpose : emWin and embOS demo application
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "GUI.h"
#include "FS.h"
#include <stdio.h>
#include <string.h>
#if	_WINDOWS
#include "SIMConf.h"
#endif
#include "BSP.h"

char oneKbuffer[1024+2];


/*********************************************************************
*
*       main()
*/
void MainTask(void);
void MainTask(void)
{
U32          v;
FS_FILE    * pFile;
char         ac[256], kmc[100];
int r, i;
const char * sVolName = "";

	GUI_Init();
  	GUI_SetFont(&GUI_Font32_ASCII);
	
  	//
  	// Initialize file system
  	//
  	FS_Init();

#ifdef _WINDOWS
	SIM_Init();
#endif

  	//
  	// Check if low-level format is required
  	//
  	FS_FormatLLIfRequired("");
  
  	//
  	// Check if volume needs to be high level formatted.
  	//
  	if ((r = FS_IsHLFormatted("")) == 0) {
    	sprintf(kmc,"High level formatting\n");
    	GUI_DispString(kmc);
    	FS_Format("", NULL);
  	}
  	else if (r == -1)
  	{
		GUI_DispString("FS_IsHLFormatted returned an error\n");
   }
  
  	sprintf(kmc,"Running sample on \"%s\"\n", sVolName);
	GUI_DispString(kmc);
	
  	v = FS_GetVolumeFreeSpace(sVolName);
  	if (v <= 0x7fff) {
    	sprintf(kmc,"  Free space: %u bytes\n", v);
    	GUI_DispString(kmc);
  	} else if (v <= 0x2000000UL) {
    	v >>= 10;
    	sprintf(kmc,"  Free space: %u kBytes\n", v);
    	GUI_DispString(kmc);
  	} else {
    	v >>= 20;
    	sprintf(kmc,"  Free space: %u MBytes\n", v);
    	GUI_DispString(kmc);
  	}

  	// prepare buffer
	oneKbuffer[0] = 0;
	for (i=0; i < 16; i++)
	{
		strcat(oneKbuffer, "This line right here is 64 characters long including the CR....\n");
	}

	GUI_DispString("Begin timed section\n");
//  
// Write a 50k file
//
  
	// Start LED timed section
	BSP_SetLED(1);
	
	sprintf(ac, "%s\\fifty_K.txt", sVolName);
	pFile = FS_FOpen(ac, "w");
	if (pFile) {
		for (i = 0; i < 50; i++)
		{
			if (FS_Write(pFile, oneKbuffer, 1024) != 1024)
				GUI_DispString("Write Error\n");
		}
		if (FS_FClose(pFile))
			  	GUI_DispString("Could not close file\n");
	}  
	else {
    	sprintf(kmc,"Could not open file: %s to write.\n", ac);
    	GUI_DispString(kmc);
   }
  
  	// End LED timer section
	BSP_ClrLED(1);
	GUI_DispString("End timed section\n");

	// Main task loop
  while(1) {
	 
#ifdef _WINDOWS
		if (SIM_Tick())
			return;
#endif
	}
}
/****** End of File *************************************************/
