#ifndef DISPLAY_H
#define DISPLAY_H

#include <tmtypes.h>

// compilation controls
#undef DISPLAYATTACHED

//------------------------------------------------------------------------------------
// Display.h
//------------------------------------------------------------------------------------
// Copyright 2004-2008 Track Systems Technologies, LLC
//
// AUTH: GAS
// DATE: 21 MAR 04
// REVISED: 15 JUN 04 - adapted for AZ display
//
// This file contains the definitions for the Traqmate 122x32 LCD Display Unit
//

//------------------------------------------------------------------------------------
// Declarations
//------------------------------------------------------------------------------------

// Global Variables
#ifdef DU_MAIN
	xdata u16 numpages;					// number of pages in dataflash
	xdata u16 pagesize;					// page size of dataflash
#else
	extern xdata u16 numpages;					// number of pages in dataflash
	extern xdata u16 pagesize;					// page size of dataflash
#endif

#endif