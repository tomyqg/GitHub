#define SDCARDRWMAIN

#ifdef PROFILER
#include "profiler.h"
#define THIS_TASK	SDCARDRWTASK_NUM
#endif

#include <stdio.h>
#include "FS.h"
#include "DUII.h"
#include "SDCardRW.h"
#include "FlashReadWrite.h"

// this task receives queued data and writes it to the SD card
//
void SDCardRWTask(void)
{
	int newSettings;
			
	while (1)
	{
		OS_GetMail(&MB_SDcardRW, &newSettings);
		WriteUserDataEvent((enum DATATYPES)newSettings);
	
		OS_Delay(1);
	}
} // SDCardRWTask

#undef SDCARDRWMAIN
