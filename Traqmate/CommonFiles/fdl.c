// software downloader
//
// this must be less than 510 bytes when compiled which is very tight. It must
// not extend into address 0xFFFE or 0xFFFF, the micro flash lock bytes
// it is loaded at 0xFE00 and must be self-contained so the code copy will
// not overwrite it.
//
// initial implementation John Paulos, 04/26/2005
// re-written Glenn Stephens, 05/02/2005
//

#include <tmtypes.h>
#ifdef DISPLAY
#include <display.h>
#else
#include <sensor.h>
#endif

//#define DFWAITREAD do {int q; for (q=0; q<64; q++) ; } while(0)  // 64 seems to work well. should be verified.
//#define DFWAITWRITE do {int q; for (q=0; q<64; q++) ; } while(0)  // longer for erase.
#define DFWAITFDL do {int q; for (q=0; q<200; q++) ; } while(0)  // longer for erase.

u08 FDL_xferSPI (u08);
u08 FDL_DataFlash_Read_Status(void);

void CopytoCodeSpace(u16 lastpage) {
	char * data pwrite;
	char EA_save;
	u16 i, page;
	u08 status;
#pragma ASM
// support code versions with roadblock at 0xfe00
// this code duplicates the c compiler argument passing code
//	argument passing, R6 = MSB, R7 = LSB
	MOV  	lastpage?040,R6
	MOV  	lastpage?040+01H,R7
#pragma ENDASM

	pwrite = 0x0;						// initialize code pointer

	EA_save = EA;						// save interrupt status
	EA = 0;								// disable interrupts

	SFRPAGE = LEGACY_PAGE;
	FLSCL |= 0x01;						// enable flash writes/erases from user software

	PSCTL = 0x03;						// movx writes erase flash page

	// erase code memory up to last 1024 page containing bootstrapper
	while (pwrite < LAST_CODE_PAGE) {	// stop one page from end of codespace
//		*pwrite = 0x88;					// write any value to initiate page erase
// this assembler code must be here to force compiler to use the MOVX instruction to cause flash rewrite
// XXXXXXXXXXXXXXXXXX
#pragma ASM
		MOV  	DPL,pwrite?045+01H
		MOV  	DPH,pwrite?045	
		MOV  	A,#088H					// put a value in A, doesn't matter what
		MOVX 	@DPTR,A					// MOVX causes code flash page to be erased
#pragma ENDASM
		pwrite += CODE_PAGE_SIZE;		// point to next page
	}	// while

	PSCTL = 0x00;						// movx writes to target ram
	pwrite = 0x00;						// reset code pointer

	// copy code over from dataflash to micro flash codespace 512 bytes at a time
	for (page=0; page < (BOOTSTRAP_ADDR/CODE_DOWNLOAD_SIZE); page++) {

		SFRPAGE = CONFIG_PAGE;

//			FDL_DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, read_page);
// function is put in line here to avoid passing pointer which uses code put in by 
// compiler in lower code space

#ifdef SENSOR
		if (!(page % 4)) LED1 = ~LED1;;
#endif

		// wait for Serial Flash to be available
		do {
			status = FDL_DataFlash_Read_Status();
		} while (!(status & STATUS_READY_MASK));

		// set CS to low
		DF_CS1 = 0;
		DFWAITFDL;

		// shift out command
		FDL_xferSPI(MEMORY_PAGE_READ);
	
		// shift out reserved bits and upper bits of page address
		FDL_xferSPI(page >> 6);	

		// shift out bottom 6 bits of page address and top bits of byte address
		FDL_xferSPI((u08) page << 2);

		// shift out bottom 8 bits of unsigned char address
		FDL_xferSPI(0);

		// shift out 4 bytes of don't cares
		FDL_xferSPI(0);	FDL_xferSPI(0);	FDL_xferSPI(0);	FDL_xferSPI(0);

		// shift in the data
		for (i=0; i < CODE_DOWNLOAD_SIZE; i++)
			flashpage.bigbuff[i] =  FDL_xferSPI(0);

		// raise CS to end operation
		DF_CS1 = 1;

// end of inline function

		SFRPAGE = LEGACY_PAGE;
		PSCTL = 0x01;						// movx writes to codeflash

// XXXXXXXXXXXXXXXXXX
		// copy dataflash page into micro flash
		for (i=0; i < CODE_DOWNLOAD_SIZE; i++) {
			unsigned char temp;

			temp = flashpage.bigbuff[i];
//			*pwrite = temp;
// this assembler code must be here to force compiler to use the MOVX instruction to cause flash rewrite
#pragma ASM
			MOV  	A,temp?042				// put code byte into accum	
			MOV  	DPL,pwrite?045+02H		// load data pointer with address in code flash
			MOV  	DPH,pwrite?045+01H	
			MOVX 	@DPTR,A					// MOVX causes code flash page to be programmed
#pragma ENDASM

			pwrite++;
		} // for
		
		PSCTL = 0x00;
	} // for

	PSCTL = 0;							// movx writes target ram

	FLSCL &= ~0x01;						// disable flash writes from user sw
	EA = EA_save;
	PSW = 0;

	SFRPAGE = CONFIG_PAGE;

	// clear the flash - NOTE: cannot use global variables. Must choose max erase pages.
	for (page = 0; page < MAX_NUM_PAGES; page += 8) {

#ifdef SENSOR
		// invert LED every so often
		if (!(page % 64)) LED2 = ~LED2;
#endif

		// inline dataflash erase block

		// wait for Serial Flash to be available
		do {
			status = FDL_DataFlash_Read_Status();
		} while (!(status & STATUS_READY_MASK));

		DF_CS1 = 0;
		DFWAITFDL;

		//command
		FDL_xferSPI(BLOCK_ERASE);

		// shift out reserved bits and upper bits of page address
		FDL_xferSPI(page >> 6);

		// shift out bottom 6 bits of page address and top bits of byte address
		FDL_xferSPI((u08) page << 2);

		// shift out bottom 8 bits of unsigned char address
		FDL_xferSPI(0);

		DF_CS1 = 1;

		// end of inline function
	} // for

	DFWAITFDL;
	
	// wait for Serial Flash to be finished
	do {
		status = FDL_DataFlash_Read_Status();
	} while (!(status & STATUS_READY_MASK));

	SFRPAGE = LEGACY_PAGE;

	// pull the reset line to restart everything
	RSTSRC = RSTSRC | 0x01;
}

// simultaneously write	and read one unsigned char to SPI (mode 3). Blocking.
u08 FDL_xferSPI (u08 ch) {

	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	SFRPAGE = SPI0_PAGE;

	SPI0DAT = ch;

	// wait for shifting
	while (!SPIF) ;

	SPIF = 0;			// clear the xfer complete flag

	ch = SPI0DAT;
	
	SFRPAGE = SFRPAGE_SAVE;

	return (ch);
}

// returns the value of DataFlash Status Register. Non-blocking.
u08 FDL_DataFlash_Read_Status(void) {
   u08 dat;

	DF_CS1 = 0;
	DFWAITFDL;

	//command
	FDL_xferSPI(STATUS_REGISTER_READ);

	dat = FDL_xferSPI(0);

	DF_CS1 = 1;
  
   return (dat);
}
