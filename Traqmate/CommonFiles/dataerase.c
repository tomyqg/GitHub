// Part of traqmate.c
// 11/10/2003
// Author: GAStephens
//
// This function erase the contents of the dataflash up to 32Mbit

#ifdef SENSOR
#include <sensor.h>
#endif
#ifdef DISPLAY
#include <display.h>
#endif

void dataerase ( int numpages ) {
	unsigned int page;

#ifdef SENSOR
	LO(LED1);
#endif

	// eventually you may want to only erase 3 - 7! -- GAS

	// erase pages 0 - 7 (0,1,2 are indices)
//	for (page = 0; page < 8 && page < numpages; page++)
		// erase page
//		DataFlash_Erase_Page(page);

	for (page = 0; page < numpages; page += 8) {
#ifdef SENSOR
		if (!(page % 80)) {
			// invert LED
			if (P5 & 0x10) LO(LED1);
			else HI(LED1);
		} // if
#endif
		// erase block
		DataFlash_Erase_Block(page);
	}
#ifdef SENSOR
	LO(LED1);
#endif
}
