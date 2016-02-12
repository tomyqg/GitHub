//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <C8051F020.h>                 // SFR declarations
#include <tmtypes.h>

// erase the scratchpad area of FLASH
void EraseScratchpad() {

	unsigned char xdata * idata pwrite;	// pointer to FLASH scratchpad, must be data or idata
	unsigned char EA_save;				// saves the current state of the int enable bit

	// initialize write/erase pointer to any location in the scratchpad
	pwrite = 0x0000;   

	EA_save = EA;                       // save interrupt status   
	EA = 0;                             // disable interrupts (precautionary)
   
	FLSCL |= 0x01;                      // enable FLASH writes/erases from user sw

	PSCTL = 0x07;                       // MOVX writes erase FLASH page
										// (SFLE set directing FLASH 
										// reads/writes/erases to scratchpad memory

	*pwrite = 0;						// initiate page erase

	PSCTL = 0;							// MOVX writes target XRAM
										// (SFLE is cleared)
   
	FLSCL &= ~0x01;						// disable FLASH writes/erases from user sw

	EA = EA_save;                       // re-enable interrupts
} // EraseScratchpad

// Write a buffer to scratchpad area of FLASH
void WriteScratchpad( unsigned char *buf, int buflen) {

	unsigned char xdata * idata pwrite;	// pointer to FLASH scratchpad, must be data or idata
	unsigned char EA_save;				// saves the current state of the int enable bit
	int i;

	// initialize FLASH write pointer to the beginning of the scratchpad
	pwrite = 0x0000;                    

	EA_save = EA;                       // save interrupt status
	EA = 0;                             // disable interrupts (precautionary)
   
	FLSCL |= 0x01;						// enable FLASH writes/erases from user sw
      
	PSCTL = 0x05;						// turn on scratchpad and writes
	for (i=0; i<buflen; i++)			// copy data into scratchpad
		*pwrite++ = *buf++;

	PSCTL = 0x00;						// MOVX writes target XRAM
	FLSCL &= ~0x01;						// disable FLASH writes/erases from user sw

	EA = EA_save;						// re-enable interrupts
} // WriteScratchpad

// Read a buffer from scratchpad area of FLASH
void ReadScratchpad( unsigned char *buf) {

	unsigned char code *pread;			// pointer to FLASH used for reads, must be code
	int i;

	pread = 0x0000;						// point at scratchpad

	PSCTL = 0x04;						// swap in scratchpad

	for (i=0; i<SCRATCHLEN; i++)		// copy data from scratchpad
		*buf++ = *pread++;

	PSCTL = 0;							// swap it out

} // ReadScratchpad