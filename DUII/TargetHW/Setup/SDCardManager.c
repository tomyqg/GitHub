#define SDCARDMANAGERMAIN

#ifdef PROFILER
#include "profiler.h"
#define THIS_TASK	SDCARDMGRTASK_NUM
#endif

#include <stdio.h>
#include "FS.h"
#include "DUII.h"
#include "SDCardManager.h"
#include "dbgu.h"

extern void ConfigureCache(void);

// this task saves periodically depending on the system mode
//
void SDCardManagerTask ( void ) {
	while (1) {
		
		if (DU2_DONT_INTERRUPT(unsavedSysData.systemMode)) {
			OS_Delay(10000);
		} // if
		
//#ifdef BUILD_DEBUG_PORT
//		DBGU_Print("SDCardManager\n\r");
//#endif
		else {
			FS_Sync("");
			if (DU2_RECORDING(unsavedSysData.systemMode))	// recording
				OS_Delay(5000);			// save every 5 seconds
			else											// sitting in menus
				OS_Delay(60000);		// save every minute
		} // else
		
	} // while
} // SDCardManagerTask

#undef SDCARDMANAGERMAIN