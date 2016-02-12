//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <ctype.h>
#ifdef DISPLAY
#include <display.h>
#else
#include <sensor.h>
#endif

#ifdef DISPLAY
// initializes the scratchpad
// rewrite if:	SWREV or DATAREV not right
//				config areas of DU != config areas of SU
//
// if su_present, assumes that global scratch contains SU scratchpad
//
// Returns FALSE if DU and SU have incompatible data revisions
//
BOOL InitDUScratchpad(BOOL su_present) {
	xdata scratchpadtype duscratch;
	xdata BOOL reprogram = FALSE;	 // used to signal sw mismatch and data change

	// check for incompatible SU and DU data revisions
	if (su_present && scratch.calibrate.datarev != DATAREV)
		// software mismatch condition
		reprogram = TRUE;
	
	ReadScratchpad(duscratch.scratchbuff);

	// if sw mismatch jump out
	if (reprogram) return(FALSE);

	// if contrast never programmed or has bad value, fix it
	if (0xFF == duscratch.calibrate.contrast ||
		duscratch.calibrate.contrast > CONTRAST_UPPER ||
		duscratch.calibrate.contrast < CONTRAST_LOWER
	) {
		duscratch.calibrate.contrast = CONTRAST_CENTER;
		reprogram = TRUE;
	}

	// if laphold never programmed, fix it
	if (0xFF == duscratch.calibrate.lapholdtime) {
		duscratch.calibrate.lapholdtime = LAPHOLD_DEFAULT;
		reprogram = TRUE;
	}

	// initialize usersettings here, when necessary
	if (0xFF == duscratch.calibrate.usersettings) {
		duscratch.calibrate.usersettings = USERSETTINGS_DEFAULT;
		reprogram = TRUE;
	}

	// if username, phone, or note is unprintable, NULL it out
	if (!isprint(duscratch.calibrate.owner[0]) && duscratch.calibrate.owner[0] != '\0') {
		duscratch.calibrate.owner[0] = '\0';
		reprogram = TRUE;
	} // if
	if (!isprint(duscratch.calibrate.phone[0]) && duscratch.calibrate.phone[0] != '\0') {
		duscratch.calibrate.phone[0] = '\0';
		reprogram = TRUE;
	} // if
	if (!isprint(duscratch.calibrate.note[0]) && duscratch.calibrate.note[0] != '\0') {
		duscratch.calibrate.note[0] = '\0';
		reprogram = TRUE;
	} // if

	// check to see if software or data rev in scratchpad does not match current code
	if (duscratch.calibrate.swrev != SWREV ||
		duscratch.calibrate.datarev != DATAREV) {

		// update the values
		duscratch.calibrate.swrev = SWREV;
		duscratch.calibrate.datarev = DATAREV;
		reprogram = TRUE;
	}

	if (duscratch.calibrate.hwrev != HWREV ||
		duscratch.calibrate.model != MODELNUM) {

		// update the values
		duscratch.calibrate.hwrev = HWREV;
		duscratch.calibrate.model = MODELNUM;
		reprogram = TRUE;
	}

	// set bit for type of dataflash one time only
	if (numpages == NUM_PAGE_16MBIT && (scratch.calibrate.hwconfig & DATAFLASHMASK) != 0) {
		scratch.calibrate.hwconfig &= ~DATAFLASHMASK;
		reprogram = TRUE;
	} // if
	if (numpages == NUM_PAGE_32MBIT && (scratch.calibrate.hwconfig & DATAFLASHMASK) != DATAFLASHMASK) {
		scratch.calibrate.hwconfig |= DATAFLASHMASK;
		reprogram = TRUE;
	} // if	

	// compare SU and DU config data
	if (su_present) {
		if (duscratch.calibrate.xzero != scratch.calibrate.xzero) {
			duscratch.calibrate.xzero = scratch.calibrate.xzero;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.yzero != scratch.calibrate.yzero) {
			duscratch.calibrate.yzero = scratch.calibrate.yzero;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.zzero != scratch.calibrate.zzero) {
			duscratch.calibrate.zzero = scratch.calibrate.zzero;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.xminusg != scratch.calibrate.xminusg) {
			duscratch.calibrate.xminusg = scratch.calibrate.xminusg;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.xplusg != scratch.calibrate.xplusg) {
			duscratch.calibrate.xplusg = scratch.calibrate.xplusg;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.yminusg != scratch.calibrate.yminusg) {
			duscratch.calibrate.yminusg = scratch.calibrate.yminusg;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.yplusg != scratch.calibrate.yplusg) {
			duscratch.calibrate.yplusg = scratch.calibrate.yplusg;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.zminusg != scratch.calibrate.zminusg) {
			duscratch.calibrate.zminusg = scratch.calibrate.zminusg;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.zplusg != scratch.calibrate.zplusg) {
			duscratch.calibrate.zplusg = scratch.calibrate.zplusg;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.calx != scratch.calibrate.calx) {
			duscratch.calibrate.calx = scratch.calibrate.calx;
			reprogram = TRUE;
		}
		if (duscratch.calibrate.caly != scratch.calibrate.caly) {
			duscratch.calibrate.caly = scratch.calibrate.caly;
			reprogram = TRUE;
		}
	} // if

	if (reprogram) {
		// write the software rev into the scratchpad
		EraseScratchpad();
		WriteScratchpad(duscratch.scratchbuff, SCRATCHLEN);
	} // if

	// leave with DU scratchpad contents in scratch
	ReadScratchpad(scratch.scratchbuff);

	return (TRUE);

} // InitDUScratchpad
#endif
#ifdef SENSOR
// initializes the scratchpad
// rewrite if:	SWREV or DATAREV not right
//
void InitSUScratchpad(void) {
	xdata BOOL reprogram = FALSE;

	ReadScratchpad(scratch.scratchbuff);

	if (scratch.calibrate.swrev != SWREV || scratch.calibrate.datarev != DATAREV) {
		// update the values
		scratch.calibrate.swrev = SWREV;
		scratch.calibrate.datarev = DATAREV;
		reprogram = TRUE;
	} // if

	if (scratch.calibrate.hwrev == 0xFF) {
		// update the values
		if (su2)
			scratch.calibrate.hwrev = HWREVSU2;
		else
			scratch.calibrate.hwrev = HWREVSU1;

		reprogram = TRUE;
	} // if
	
	if (scratch.calibrate.model != MODELNUM) {
		// update the values
		scratch.calibrate.model = MODELNUM;
		reprogram = TRUE;
	} // if
	
	// set bit for type of dataflash one time only
	if (numpages == NUM_PAGE_16MBIT && (scratch.calibrate.hwconfig & DATAFLASHMASK) != 0) {
		scratch.calibrate.hwconfig &= ~DATAFLASHMASK;
		reprogram = TRUE;
	} // if
	if (numpages == NUM_PAGE_32MBIT && (scratch.calibrate.hwconfig & DATAFLASHMASK) != DATAFLASHMASK) {
		scratch.calibrate.hwconfig |= DATAFLASHMASK;
		reprogram = TRUE;
	} // if	

	// if username, phone, or note is unprogrammed, NULL it out
	if (!isprint(scratch.calibrate.owner[0]) && scratch.calibrate.owner[0] != '\0') {
		scratch.calibrate.owner[0] = '\0';
		reprogram = TRUE;
	} // if
	if (!isprint(scratch.calibrate.phone[0]) && scratch.calibrate.phone[0] != '\0') {
		scratch.calibrate.phone[0] = '\0';
		reprogram = TRUE;
	} // if
	if (!isprint(scratch.calibrate.note[0]) && scratch.calibrate.note[0] != '\0') {
		scratch.calibrate.note[0] = '\0';
		reprogram = TRUE;
	} // if

	// initialize turn on speeds for Traqmate Basic only
	if (scratch.calibrate.turnonspeed == 0xFF) {
		scratch.calibrate.turnonspeed = (u08) 0;		// mph
		reprogram = TRUE;
	} // if
	
	if (scratch.calibrate.turnofftime == 0xFFFF) {
		scratch.calibrate.turnofftime = (u16) (0);		// seconds
		reprogram = TRUE;
	} // if
		
	if (reprogram) {
		// write the software rev into the scratchpad
		EraseScratchpad();
		WriteScratchpad(scratch.scratchbuff, SCRATCHLEN);
	} // if

#ifndef DISPLAYATTACHED // DU sends message to do this. Basic does it based on programming
	// check AutoOn bit and program digipot to appropriate rail
	if (su2)
		DigiPot(AUTOONENABLED);		// usersettings bit 4
#endif

} // InitSUScratchpad
#endif

// erase the scratchpad area of FLASH
void EraseScratchpad() {
	unsigned char SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	unsigned char xdata *idata pwrite;	// pointer to FLASH scratchpad, must be data or idata
	unsigned char EA_save;				// saves the current state of the int enable bit

	SFRPAGE = LEGACY_PAGE;				// set SFR page

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

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
} // EraseScratchpad

// Write a buffer to scratchpad area of FLASH
void WriteScratchpad( unsigned char *buf, int buflen) {
	unsigned char SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	unsigned char xdata *idata pwrite;	// pointer to FLASH scratchpad, must be data or idata
	unsigned char EA_save;				// saves the current state of the int enable bit
	int i;

	SFRPAGE = LEGACY_PAGE;				// set SFR page

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

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
} // WriteScratchpad

// Read a buffer from scratchpad area of FLASH
void ReadScratchpad( unsigned char *buf) {
	unsigned char SFRPAGE_SAVE = SFRPAGE;	// Save Current SFR page
	unsigned char code *pread;			// pointer to FLASH used for reads, must be code
	unsigned char EA_save;				// saves the current state of the int enable bit

	int i;

	pread = 0x0000;						// point at scratchpad

	SFRPAGE = LEGACY_PAGE;				// set SFR page

	EA_save = EA;                       // save interrupt status
	EA = 0;                             // disable interrupts (precautionary)

	PSCTL = 0x04;						// swap in scratchpad

	for (i=0; i<SCRATCHLEN; i++)		// copy data from scratchpad
		*buf++ = *pread++;

	PSCTL = 0;							// swap it out

	EA = EA_save;						// re-enable interrupts

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
} // ReadScratchpad