#undef WRITEPROTECT
// Part of traqmate.c
// 11/10/2003
// Author: GAStephens
//
// This file contains all the functions to read and write to an Atmel Dataflash
// serial flash memory chip.

#include "commondefs.h"
#include "dataflash.h"
#include "SPI_API.h"
#include "RTOS.h"
#include "DUII.h"
#include "CommTask.h"

void mSDelay(int);
void uSDelay(int);

#define DFWAIT uSDelay(150)				// was 500 through 2.60. 100 usec seems to work

static void ShiftAddress(u16 page, u16 pagesize) {
	if (MAX_PAGE_64MBIT	== pagesize) {		// atmel 64Mbit dataflash
		// shift out reserved bits and upper bits of page address
		xferSPI(page >> 5);
	
		// shift out bottom 5 bits of page address and top bits of byte address
		xferSPI((u08) page << 3);
	} // if
	else if (MAX_PAGE_64EMBIT == pagesize) {	// adesto 64Mbit dataflash
		// shift out upper bits of page address
		xferSPI(page >> 7);
	
		// shift out bottom 7 bits of page address and top bits of byte address
		xferSPI((u08) page << 1);
	} // if
	else {		// atmel 32Mbit dataflash
		// shift out reserved bits and upper bits of page address
		xferSPI(page >> 6);
	
		// shift out bottom 6 bits of page address and top bits of byte address
		xferSPI((u08) page << 2);
	} // else
} // ShiftAddress
	
// This function transfers a page from an array to the designated buffer and then to
// the designated memory page while erasing. Non-blocking.
void DataFlash_Page_Write_Erase(int chip, int buffnum, int bufsize, char *buffer, u16 pagenum) {

	u08 status;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	// set CS to low	
	DF_SPI_X_EnableCS(chip);
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// writing low order pages
#endif
	DFWAIT;
	// shift out command
	if (1 == buffnum)
		xferSPI(BUFFER1_WRITE);
	else
		xferSPI(BUFFER2_WRITE);

	// shift out 24 more bits to start at byte 0
	xferSPI(0); xferSPI(0); xferSPI(0);

	// shift out the data
	for (i=0; i<bufsize; i++) xferSPI(buffer[i]);

	// set CS to high to end buffer transfer
	DF_SPI_X_DisableCS(chip);

	DataFlash_Buffer_to_Page_Write_Erase(chip, buffnum, pagenum);
}

// This function transfers a page from an array to the designated buffer and then to
// the designated memory page. Non-blocking.
void DataFlash_Page_Write(int chip, int buffnum, int bufsize, char *buffer, u16 pagenum) {
	u08 status;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	// set CS to low	
	DF_SPI_X_EnableCS(chip);
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// writing low order pages
#endif
	DFWAIT;
	// shift out command
	if (1 == buffnum)
		xferSPI(BUFFER1_WRITE);
	else
		xferSPI(BUFFER2_WRITE);

	// shift out 24 more bits to start at byte 0
	xferSPI(0); xferSPI(0); xferSPI(0);

	// shift out the data
	for (i=0; i<bufsize; i++) xferSPI(buffer[i]);

	// set CS to high to end buffer transfer
	DF_SPI_X_DisableCS(chip);

	DataFlash_Buffer_to_Page_Write(chip, buffnum, pagenum);
}

// This function transfers a page from the designated buffer to
// the designated memory page with an erase. Non-blocking.
void DataFlash_Buffer_to_Page_Write_Erase(int chip, int buffnum, u16 pagenum) {
	u08 status;
	BOOL erased;

	erased = DataFlash_Page_Erased(chip, unsavedSysData.dataflash[chip].bytesPerPage, pagenum);

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	// set CS to low	
	DF_SPI_X_EnableCS(chip);
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// writing low order pages
#endif
	DFWAIT;

	if (erased) {
		// shift out command
		if (1 == buffnum)
			xferSPI(MEMORY_PAGE_THRU_BUFFER1_WRITE);
		else
			xferSPI(MEMORY_PAGE_THRU_BUFFER2_WRITE);
	} // if
	else {
		// shift out command
		if (1 == buffnum)
			xferSPI(MEMORY_PAGE_THRU_BUFFER1_WRITE_ERASE);
		else
			xferSPI(MEMORY_PAGE_THRU_BUFFER2_WRITE_ERASE);
	} // else

	ShiftAddress(pagenum, unsavedSysData.dataflash[chip].bytesPerPage);

	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

#ifdef WRITEPROTECT
	DF_WP = 0;
#endif
	// raise CS to end command and start flash write
	DF_SPI_X_DisableCS(chip);
}

// This function transfers a page from the designated buffer to
// the designated memory page, no erase. Non-blocking.
void DataFlash_Buffer_to_Page_Write(int chip, int buffnum, u16 pagenum) {
	u08 status;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	// set CS to low	
	DF_SPI_X_EnableCS(chip);
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// writing low order pages
#endif
	DFWAIT;

	// shift out command
	if (1 == buffnum)
//		xferSPI(MEMORY_PAGE_THRU_BUFFER1_WRITE);
		xferSPI(BUFFER1_WRITE_NO_ERASE);
	else
//		xferSPI(MEMORY_PAGE_THRU_BUFFER2_WRITE);
		xferSPI(BUFFER2_WRITE_NO_ERASE);
		
	ShiftAddress(pagenum, unsavedSysData.dataflash[chip].bytesPerPage);

	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

#ifdef WRITEPROTECT
	DF_WP = 0;
#endif
	// raise CS to end command and start flash write
	DF_SPI_X_DisableCS(chip);
}

// This function transfers one or more bytes from an array to the designated buffer. Blocking.
// address is 10 bit address of which byte of buffer to start transfer
void DataFlash_Buffer_Write(int chip, int buffnum, int bufsize, u16 address, char *buffer) {
	u08 status;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	// set CS to low	
	DF_SPI_X_EnableCS(chip);
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// writing low order pages
#endif
	DFWAIT;
	// shift out command
	if (1 == buffnum)
		xferSPI(BUFFER1_WRITE);
	else
		xferSPI(BUFFER2_WRITE);

	// shift out 24 more bits containing address
	xferSPI(0); xferSPI((u08) ((address & 0x0300) >> 8)); xferSPI((u08) (address & 0x00FF));

	// shift out the data
	for (i=0; i<bufsize; i++) xferSPI(buffer[i]);

	// set CS to high to end buffer transfer
	DF_SPI_X_DisableCS(chip);
}

// This function sets every byte in the designated buffer to 0xFF. Blocking.
void DataFlash_Buffer_Clear(int chip, int buffnum, int bufsize) {
	u08 status;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	// set CS to low	
	DF_SPI_X_EnableCS(chip);
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// writing low order pages
#endif
	DFWAIT;
	// shift out command
	if (1 == buffnum)
		xferSPI(BUFFER1_WRITE);
	else
		xferSPI(BUFFER2_WRITE);

	// shift out 24 more bits to start at byte 0
	xferSPI(0); xferSPI(0); xferSPI(0);

	// shift out the data
	for (i=0; i<bufsize; i++) xferSPI(0xFF);

	// set CS to high to end buffer transfer
	DF_SPI_X_DisableCS(chip);
}

// returns the value of DataFlash Status Register. Non-blocking.
unsigned char DataFlash_Read_Status(int chip)
{
   u08 dat;

	DF_SPI_X_EnableCS(chip);
	DFWAIT;

	//command
	xferSPI(STATUS_REGISTER_READ);

	dat = xferSPI(0);

	DF_SPI_X_DisableCS(chip);

   return (dat);
}

// returns the value of DataFlash Status Register. Non-blocking.
void DataFlash_Read_Id(int chip, int len, unsigned char *id_string)
{
	u08 status;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	DF_SPI_X_EnableCS(chip);
	DFWAIT;

	// shift out command
	xferSPI(MANUFACTURER_ID_READ);

	for (i=0; i<len; i++)
		id_string[i] = xferSPI(0);

	DF_SPI_X_DisableCS(chip);
}

// This function transfers a page from the designated memory page to an array. Blocking.
void DataFlash_Page_Read(int chip, int bufsize, char *buffer, u16 pagenum) {

	u08 status;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	// set CS to low
	DF_SPI_X_EnableCS(chip);
	DFWAIT;

	// shift out command
	xferSPI(MEMORY_PAGE_READ);
	
	ShiftAddress(pagenum, unsavedSysData.dataflash[chip].bytesPerPage);

	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

	// shift out 4 bytes of don't cares
	xferSPI(0);	xferSPI(0);	xferSPI(0);	xferSPI(0);
	
	// shift in the data
	for (i=0; i<bufsize; i++) buffer[i] = xferSPI(0);

	// raise CS to end operation
	DF_SPI_X_DisableCS(chip);
}

// This function determines if a page is erased. Blocking.
unsigned char DataFlash_Page_Erased(int chip, int bufsize, u16 pagenum) {

	u08 status;
	BOOL result = TRUE;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	// set CS to low
	DF_SPI_X_EnableCS(chip);
	DFWAIT;

	// shift out command
	xferSPI(MEMORY_PAGE_READ);
	
	ShiftAddress(pagenum, unsavedSysData.dataflash[chip].bytesPerPage);

	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

	// shift out 4 bytes of don't cares
	xferSPI(0);	xferSPI(0);	xferSPI(0);	xferSPI(0);
	
	// shift in the data while checking for erasure
	for (i=0; i<bufsize; i++)
		if (!(result = (0xFF == xferSPI(0)))) break;

	// raise CS to end operation
	DF_SPI_X_DisableCS(chip);

	return(result);
}

// erases a block from memory. Non-blocking.
void DataFlash_Erase_Block(int chip, u16 pagenum)
{
	u08 status;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	DF_SPI_X_EnableCS(chip);
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// erasing low order pages
#endif
	DFWAIT;

	//command
	xferSPI(BLOCK_ERASE);

	ShiftAddress(pagenum, unsavedSysData.dataflash[chip].bytesPerPage);
	
	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

#ifdef WRITEPROTECT
	DF_WP = 0;
#endif
	DF_SPI_X_DisableCS(chip);
}

// erases a page from memory. Non-blocking.
void DataFlash_Erase_Page(int chip, u16 pagenum)
{
	u08 status;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status(chip);
	} while (!(status & STATUS_READY_MASK));

	DF_SPI_X_EnableCS(chip);
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// erasing low order pages
#endif
	DFWAIT;

	//command
	xferSPI(PAGE_ERASE);
	
	ShiftAddress(pagenum, unsavedSysData.dataflash[chip].bytesPerPage);

	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

#ifdef WRITEPROTECT
	DF_WP = 0;
#endif
	DF_SPI_X_DisableCS(chip);
}

// erases a range of pages from memory.
void DataFlash_Erase_Range(int chip, u16 firstpage, u16 lastpage) {
	u16 segment;

	for (segment = firstpage; segment <= lastpage; segment++) {
		// check for even block boundary and that we are not near end of segment
		if (0 == (segment % 8) && (lastpage - segment) >= 8) {

			// erase block
			DataFlash_Erase_Block(chip, segment);
			
			segment += 7;		// will get incremented again at start of loop
		} // if
		else
			DataFlash_Erase_Page(chip, segment);
	} // for
} // DataFlash_Erase_Range

//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

// This function initializes the DataFlash. Returns size of part in Mbit
// Sets values for number of pages and bytes per page
void DataFlash_Init(int chip, u16 *pages, u16 *pagebytes) {
	u08 trys = 0;			// can be set to higher number to shorten timeout
	u08 status;

	*pages = 0;
	*pagebytes = 0;

	// wait for Serial Flash to be available
	do {
		trys++;
		status = DataFlash_Read_Status(chip);
	} while ((0 != trys) && !(status & STATUS_READY_MASK));

	if (0 != trys) {			// did not time out

		// check for ready and determine which type of chip it is

		switch (status & STATUS_DENSITY_MASK) {
			case DENSITY_1MBIT:
				*pages = NUM_PAGE_1MBIT;
				*pagebytes = MAX_PAGE_1MBIT;
				break;
			case DENSITY_2MBIT:
				*pages = NUM_PAGE_2MBIT;
				*pagebytes = MAX_PAGE_2MBIT;
				break;
			case DENSITY_4MBIT:
				*pages = NUM_PAGE_4MBIT;
				*pagebytes = MAX_PAGE_4MBIT;
				break;
			case DENSITY_8MBIT:
				*pages = NUM_PAGE_8MBIT;
				*pagebytes = MAX_PAGE_8MBIT;
				break;
			case DENSITY_16MBIT:
				*pages = NUM_PAGE_16MBIT;
				*pagebytes = MAX_PAGE_16MBIT;
				break;
			case DENSITY_32MBIT:
				*pages = NUM_PAGE_32MBIT;
				*pagebytes = MAX_PAGE_32MBIT;
				break;
			case DENSITY_64MBIT:			
				// wait for Serial Flash to be available
				do {
					trys++;
					status = DataFlash_Read_Status(chip);
				} while ((0 != trys) && !(status & STATUS_READY_MASK));
				
				if (0 != trys) {			// did not time out		
					unsigned char df_chip_id[5];
					
					// get manufacturer id to see which type of 64mbit chip we have (atmel or adesto)
					DataFlash_Read_Id(chip, 5, df_chip_id);
				
					if (0x00 == df_chip_id[3]) {		// old style 64Mbit dataflash (atmel)
						*pages = NUM_PAGE_64MBIT;
						*pagebytes = MAX_PAGE_64MBIT;
					}
					else {		// new style 64Mbit dataflash (adesto)
						*pages = NUM_PAGE_64EMBIT;
						*pagebytes = MAX_PAGE_64EMBIT;
					}
				} // if
				break;
			default:
				break;
		} // switch
	} // if
}

void DataflashTest(int whichtest, int whichchip) {

#define PAGESIZE	unsavedSysData.dataflash[whichchip].bytesPerPage
#define NUMPAGES	unsavedSysData.dataflash[whichchip].numPages
	
	u16 pagenum = 0;
	int i;
	BOOL flashfail = FALSE;
	char flashpage[MAX_PAGE_64MBIT];

	// initialize
	PRINTPC0( "Dataflash Test");
	
	// initialize page to write
	for (i=0; i < PAGESIZE; i++) flashpage[i] = i % 256;

	do {
		// write buffer to flash
		DataFlash_Page_Write_Erase(whichchip, 0, PAGESIZE, flashpage, pagenum);

		// read back and verify
		DataFlash_Page_Read(whichchip, PAGESIZE, flashpage, pagenum);

		// initialize page to write
		for (i=0; i<PAGESIZE; i++) {
			if (flashpage[i] != (i % 256)) {
				flashfail = TRUE;
				break;
			} // if
		} // for

		if (flashfail) {
			PRINTPC2("** Flash FAILED **, page %d, byte%d", pagenum, i);
			break;
		} // if
		
		mSDelay(TEXTDELAY);
		PRINTPC1("Page %d ok", pagenum);

		// erase the page
		DataFlash_Erase_Page(whichchip, pagenum);

		// at the end yet?
		if (pagenum >= NUMPAGES)
			pagenum = 0;
		else
			// go to next page
			pagenum++;

	} while (whichtest == DATAFLASHTEST || pagenum <= 4);

	mSDelay((TEXTDELAY));
	PRINTPC0( "Test Passed.");

#undef PAGESIZE
#undef NUMPAGES

} // DataflashTest

/*
These functions read and write the user personalization information and the serial number and build dates into the dataflash

	If 1056 page size, writes at page 61
	If 1024 page size, writes at page 63
	If 264 page size, writes at page 247
*/
	
void ReadUserData() {
	u16 pagenum;

	pagenum = (CODESTART / unsavedSysData.dataflash[0].bytesPerPage) - 1;

	DataFlash_Page_Read(0, sizeof(UNIT_INFO_STRUCTURE), (char *) &(unsavedSysData.unitInfo), pagenum);

} // ReadUserData

void WriteUserData() {
	u16 pagenum;
	
	pagenum = (CODESTART / unsavedSysData.dataflash[0].bytesPerPage) - 1;
	
	DataFlash_Page_Write_Erase(0, GENBUFFER, sizeof(UNIT_INFO_STRUCTURE), (char *) &(unsavedSysData.unitInfo), pagenum);
						
} // WriteUserData