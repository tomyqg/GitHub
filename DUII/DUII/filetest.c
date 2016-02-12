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

#define	COUNT	300
  char kmc[COUNT*30];


/*********************************************************************
*
*       main()
*/
void MainTask(void);
void MainTask(void)
{
U32          v;
FS_FILE    * pFile;
char         ac[256];
int r, i, retval, length;
const char * sVolName = "";

	GUI_Init();
	
  //
  // Initialize file system
  //
  FS_Init();

#ifdef _WINDOWS
	SIM_Init();
#endif


  	GUI_SetFont(&GUI_Font32_ASCII);

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

#ifdef ONE_BIG_FILE
	
//
//  Many big writes using FS_Write
//
	sprintf(ac, "%s\\huge.txt", sVolName);
	pFile = FS_FOpen(ac, "w");
	if (pFile) {
	  	kmc[0] = 0;
	  	for (i=0; i < COUNT; i++)
	  	{
			char scratch[30];
		 
			sprintf(scratch,"Writing the number %d\n", i);
			strcat(kmc, scratch);
	  	}
	  	length = strlen(kmc);

		GUI_DispString("Write Starts\n");

		for (i = 0; i < 200; i++)
		{
			if (FS_Write(pFile, kmc, length) != length)
				GUI_DispString("Write Error\n");
		}

		GUI_DispString("Write Ends\n");

		if (FS_FClose(pFile))
			  	GUI_DispString("Could not close file\n");
	}  
	else {
    	sprintf(kmc,"Could not open file: %s to write.\n", ac);
    	GUI_DispString(kmc);
   }

#else	
  
//
// Simple write from demo software
//  
	GUI_DispString("Simple write from demo software\n");

	sprintf(ac, "%s\\simple.txt", sVolName);
	pFile = FS_FOpen(ac, "w");
	if (pFile) {
		if (FS_Write(pFile, "Test", 4) != 4)
             GUI_DispString("Write Error\n");
		if (FS_FClose(pFile))
			  	GUI_DispString("Could not close file\n");
	}  
	else {
    sprintf(kmc,"Could not open file: %s to write.\n", ac);
    GUI_DispString(kmc);
  	}
  
//
//  One big write using FS_Write
//
	GUI_DispString("One big write using FS_Write\n");

	sprintf(ac, "%s\\FS_Write.txt", sVolName);
	pFile = FS_FOpen(ac, "w");
	if (pFile) {
	  	kmc[0] = 0;
	  	for (i=0; i < COUNT; i++)
	  	{
			char scratch[30];
		 
			sprintf(scratch,"Writing the number %d\n", i);
			strcat(kmc, scratch);
	  	}
	  	length = strlen(kmc);
		
	  	if (FS_Write(pFile, kmc, length) != length)
         GUI_DispString("Write Error\n");
		if (FS_FClose(pFile))
			  	GUI_DispString("Could not close file\n");
	}  
	else {
    	sprintf(kmc,"Could not open file: %s to write.\n", ac);
    	GUI_DispString(kmc);
   }

	
//  
// One big write using FS_FWrite
//
	GUI_DispString("One big write using FS_FWrite\n");
	
	sprintf(ac, "%s\\FS_FWrit.txt", sVolName);
	pFile = FS_FOpen(ac, "w");
	if (pFile) {
	  	kmc[0] = 0;
	  	for (i=0; i < COUNT; i++)
	  	{
			char scratch[30];
		 
			sprintf(scratch,"Writing the number %d\n", i);
			strcat(kmc, scratch);
	  	}
	  	length = strlen(kmc);
		
	  	if (FS_FWrite(kmc, length, 1, pFile) != 1)
         GUI_DispString("Write Error\n");
		if (FS_FClose(pFile))
			  	GUI_DispString("Could not close file\n");
	}  
	else {
    	sprintf(kmc,"Could not open file: %s to write.\n", ac);
    	GUI_DispString(kmc);
   }

//
// Many small writes using FS_Write
//
	GUI_DispString("Many small writes using FS_Write\n");

	sprintf(ac, "%s\\multi_wr.txt", sVolName);
	pFile = FS_FOpen(ac, "w");
	if (pFile) {
	  for (i=0; i < COUNT; i++)
	  {
		   char scratch[30];

		   sprintf(scratch,"Writing the number %d\n", i);
			retval = FS_Write(pFile, scratch, length = strlen(scratch));
			if (retval != length)
			  	GUI_DispString("Write Error\n");
	  }
		if (FS_FClose(pFile))
			  	GUI_DispString("Could not close file\n");
	}  
	else {
    sprintf(kmc,"Could not open file: %s to write.\n", ac);
    GUI_DispString(kmc);
  }
  
//
// Many small writes using FS_FWrite
//
	GUI_DispString("Many small writes using FS_FWrite\n");

	sprintf(ac, "%s\\multi_FW.txt", sVolName);
	pFile = FS_FOpen(ac, "w");
	if (pFile) {
	  for (i=0; i < COUNT; i++)
	  {
		   char scratch[30];

		   sprintf(scratch,"Writing the number %d\n", i);

		  	if (FS_FWrite(scratch, length = strlen(scratch), 1, pFile) != 1)
			  	GUI_DispString("Write Error\n");
	  }
		if (FS_FClose(pFile))
			  	GUI_DispString("Could not close file\n");
	}  
	else {
    sprintf(kmc,"Could not open file: %s to write.\n", ac);
    GUI_DispString(kmc);
  }
  
#endif  
  
  v = FS_GetVolumeFreeSpace(sVolName);
  if (v <= 0x7fff) {
    sprintf(kmc,"  Free space: %u bytes\n", v);
    GUI_DispString(kmc);
  } else if (v <= 0x2000000UL) {
    v >>= 10;
    sprintf(kmc,"  Free space: %u kBytes\n", v);
  } else {
    v >>= 20;
    sprintf(kmc,"  Free space: %u MBytes\n", v);
    GUI_DispString(kmc);
  }

  GUI_DispString("Finished\n");
	// Main task loop
  while(1) {
#ifdef _WINDOWS
		if (SIM_Tick())
			return;
#endif
	}
}
/****** End of File *************************************************/
