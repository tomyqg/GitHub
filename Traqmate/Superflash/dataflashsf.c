#undef WRITEPROTECT
// Part of traqmate.c
// 11/10/2003
// Author: GAStephens
//
// This file contains all the functions to read and write to an Atmel Dataflash
// serial flash memory chip. 

#ifdef DISPLAY
#include <display.h>
#endif
#ifdef SENSOR
#include <sensor.h>
#endif

#define DFWAIT do {int q; for (q=0; q<64; q++) ; } while(0)  // 64 seems to work well. should be verified.

// This function transfers a page from an array to the designated buffer and then to
// the designated memory page. Non-blocking.
void DataFlash_Page_Write_Erase(int buffnum, int bufsize, char *buffer, int pagenum) {

	u08 status;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status();
	} while (!(status & STATUS_READY_MASK));

	// set CS to low	
	DF_CS1 = 0;
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
	DF_CS1 = 1;

	DataFlash_Buffer_to_Page_Write_Erase(buffnum, pagenum);
}


// This function transfers a page from the designated buffer to
// the designated memory page. Non-blocking.
void DataFlash_Buffer_to_Page_Write_Erase(int buffnum, int pagenum) {
	u08 status;
	BOOL erased;

	erased = DataFlash_Page_Erased(pagesize, pagenum);

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status();
	} while (!(status & STATUS_READY_MASK));

	// set CS to low	
	DF_CS1 = 0;
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// writing low order pages
#endif
	DFWAIT;

	if (erased) {
		// shift out command
		if (1 == buffnum)
			xferSPI(MEMORY_PAGE_THRU_BUFFER1_WRITE_ERASE);
		else
			xferSPI(MEMORY_PAGE_THRU_BUFFER2_WRITE_ERASE);
	} // if
	else {
		// shift out command
		if (1 == buffnum)
			xferSPI(MEMORY_PAGE_THRU_BUFFER1_WRITE);
		else
			xferSPI(MEMORY_PAGE_THRU_BUFFER2_WRITE);
	} // else

	// shift out reserved bits and upper bits of page address
	xferSPI(pagenum >> 6);	

	// shift out bottom 6 bits of page address and top bits of byte address
	xferSPI((u08) pagenum << 2);

	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

#ifdef WRITEPROTECT
	DF_WP = 0;
#endif
	// raise CS to end command and start flash write
	DF_CS1 = 1;
}

// returns the value of DataFlash Status Register. Non-blocking.
u08 DataFlash_Read_Status(void)
{
   u08 dat;

	DF_CS1 = 0;
	DFWAIT;

	//command
	xferSPI(STATUS_REGISTER_READ);

	dat = xferSPI(0);

	DF_CS1 = 1;
  
   return (dat);
}

// This function transfers a page from the designated memory page to an array. Blocking.
void DataFlash_Page_Read(int bufsize, char *buffer, int pagenum) {

	u08 status;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status();
	} while (!(status & STATUS_READY_MASK));

	// set CS to low
	DF_CS1 = 0;
	DFWAIT;

	// shift out command
	xferSPI(MEMORY_PAGE_READ);
	
	// shift out reserved bits and upper bits of page address
	xferSPI(pagenum >> 6);	

	// shift out bottom 6 bits of page address and top bits of byte address
	xferSPI((u08) pagenum << 2);

	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

	// shift out 4 bytes of don't cares
	xferSPI(0);	xferSPI(0);	xferSPI(0);	xferSPI(0);
	
	// shift in the data
	for (i=0; i<bufsize; i++) buffer[i] = xferSPI(0);

	// raise CS to end operation
	DF_CS1 = 1;
}


// This function determines if a page is erased. Blocking.
BOOL DataFlash_Page_Erased(int bufsize, int pagenum) {

	u08 status;
	BOOL result = TRUE;
	int i;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status();
	} while (!(status & STATUS_READY_MASK));

	// set CS to low
	DF_CS1 = 0;
	DFWAIT;

	// shift out command
	xferSPI(MEMORY_PAGE_READ);
	
	// shift out reserved bits and upper bits of page address
	xferSPI(pagenum >> 6);	

	// shift out bottom 6 bits of page address and top bits of byte address
	xferSPI((u08) pagenum << 2);

	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

	// shift out 4 bytes of don't cares
	xferSPI(0);	xferSPI(0);	xferSPI(0);	xferSPI(0);
	
	// shift in the data while checking for erasure
	for (i=0; i<bufsize; i++)
		if (!(result = (0xFF == xferSPI(0)))) break;

	// raise CS to end operation
	DF_CS1 = 1;

	return(result);
}

// erases a block from memory. Non-blocking.
void DataFlash_Erase_Block(u16 pagenum)
{
	u08 status;

	// wait for Serial Flash to be available
	do {
		status = DataFlash_Read_Status();
	} while (!(status & STATUS_READY_MASK));

	DF_CS1 = 0;
#ifdef WRITEPROTECT
	if (pagenum < 256) DF_WP = 1;		// erasing low order pages
#endif
	DFWAIT;

	//command
	xferSPI(BLOCK_ERASE);

	// shift out reserved bits and upper bits of page address
	xferSPI(pagenum >> 6);

	// shift out bottom 6 bits of page address and top bits of byte address
	xferSPI((u08) pagenum << 2);

	// shift out bottom 8 bits of unsigned char address
	xferSPI(0);

#ifdef WRITEPROTECT
	DF_WP = 0;
#endif
	DF_CS1 = 1;
}


//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

// This function initializes the DataFlash. Returns size of part in Mbit
// Sets values for number of pages and bytes per page
void DataFlash_Init(u16 *pages, u16 *pagebytes) {
	u08 status;

#ifdef WRITEPROTECT
	DF_WP = 0;							// write protect on (pages 0-255)
#else
	DF_WP = 1;							// write protect off (pages 0-255)
#endif
	DF_CS1 = 1;							// deselect chip

	DF_RST = 0;							// reset dataflash
	DFWAIT;		
	DF_RST = 1;
	DFWAIT;

	// check for ready and determine which type of chip it is
	status = DataFlash_Read_Status();

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
			*pages = NUM_PAGE_64MBIT;
			*pagebytes = MAX_PAGE_64MBIT;
			break;
	}
}
