// This is the main program for superflash.c, part of Traqmate
//
// Change Log:
// 1-18-2010 - created, GAStephens
//
// This Traqmate image is used during a Traqmate reflash to reload the upper memory bootloader.
// This code is not portable. It depends on 8052 byte ordering MSB..LSB.
//
// Dependent files:
//
// config page 0xF
// legacy page 0x0

#define DU_MAIN

#include <stdlib.h>
#include <superflash.h>
#include <dataflash.h>

#define DFWAIT do {int q; for (q=0; q<64; q++) ; } while(0)  // longer for erase
sbit BACKLITEDS = P7 ^ 6;

void main (	void ) {
	char xdata* data pwrite;
	unsigned int i, page;
	void (*f)( void);				// used for hyperjump to bootloader
	xdata flashpagetype flashpage;		// structure to access one page of flash

	SFRPAGE = CONFIG_PAGE;
	FLACL = 0x00;						// allow access to all pages of flash

	// disable watchdog timer
	WDTCN = 0xde;
	WDTCN = 0xad;

	SYSCLK_Init();						// initialize oscillator
	SPIO_Init();						// set up the SPI

	DataFlash_Init(&numpages, &pagesize);	// set up the DataFlash

	EA = 0;								// disable interrupts

// =========== copy bootloader from Dataflash upper code segment to microprocessor flash


	SFRPAGE = LEGACY_PAGE;
	FLSCL = 0x41;						// set flash speed and enable flash writes/erases from user software
//	PSCTL = 0x03;						// movx writes erase flash page

	// erase code memory last 1024 page containing bootloader
//	pwrite = LAST_CODE_PAGE;			// stop one page from end of codespace
//	*pwrite = 0x88;						// write any value to initiate page erase

//	PSCTL = 0x00;						// movx writes to target ram
//	FLSCL &= ~0x01;						// disable flash writes from user sw

	SFRPAGE = CONFIG_PAGE;

//	PSW = 0;

	// for copy, start back at top code page
	pwrite = LAST_CODE_PAGE;			// stop one page from end of codespace

	// copy code over from dataflash to micro flash codespace 512 bytes at a time
	for (page = 254; page <= 255; page++) {

		SFRPAGE = CONFIG_PAGE;

#ifdef SENSOR
		LED1 = ~LED1;;
#endif
#ifdef DISPLAY
		// toggle display backlight
		BACKLITEDS = ~BACKLITEDS;
#endif

		// get the code from data flash 512 bytes at a time
//		DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, page);

		SFRPAGE = LEGACY_PAGE;
		PSCTL = 0x01;						// movx writes to codeflash
		FLSCL |= 0x01;						// enable flash writes/erases from user software

		// copy dataflash page into micro flash
		for (i=0; i < CODE_DOWNLOAD_SIZE; i++)
//			*pwrite++ = flashpage.bigbuff[i];
			*pwrite++ = 0x00;

		PSCTL = 0x00;						// movx writes to target ram
		FLSCL &= ~0x01;						// disable flash writes from user sw
	} // for

	PSCTL = 0;							// movx writes target ram
	FLSCL &= ~0x01;						// disable flash writes from user sw

	SFRPAGE = CONFIG_PAGE;

// =========== copy code from upper Dataflash code segment to lower Dataflash code segment

	// clear out lower 128 segments for copy
	for (i = 0; i < 128; i += 8) {
		DataFlash_Erase_Block(i);
	} // for

	// copy code over from dataflash to micro flash codespace 512 bytes at a time
	for (page = 0; page < 128; page++) {

#ifdef SENSOR
		LED1 = ~LED1;;
#endif
#ifdef DISPLAY
		// toggle display backlight
		BACKLITEDS = ~BACKLITEDS;
#endif

		// get the code from data flash 512 bytes at a time
		DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, (page+128));

		DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, flashpage.bigbuff, page);
	} // for

// =========== jump to CopytoCodeSpace in new bootloader placed into upper code space

		f = (void *) 0xFE02;
		(*f)();

} // main

