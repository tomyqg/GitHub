#include <display.h>
#include <ctype.h>

//------------------------------------------------------------------------------------
// Display.c
//------------------------------------------------------------------------------------
// Copyright 2004 Track Systems, LLC
//
// AUTH: GAS
// DATE: 18 JUL 04
//
// Updated for AZ Display LCD changes 2006-11-06
//
// This file contains the display control functions.
//
// port pin control definitions			// writing HI = Push Pull, LO = Open Drain
//
#define PA0_DISP_PP	P4MDOUT
#define BA0_DISP_PP	0
#define PE1_DISP_PP	P4MDOUT
#define BE1_DISP_PP	1
#define PE2_DISP_PP	P4MDOUT
#define BE2_DISP_PP	2
#define PRW_DISP_PP	P4MDOUT
#define BRW_DISP_PP	3

#define HI(x) ((P##x) |= (1<<(B##x)))
#define LO(x) ((P##x) &= ~(1<<(B##x)))
//------------------------------------------------------------------------------------
// PORT_Init_DU
//------------------------------------------------------------------------------------
//
// Configure the Crossbar and GPIO ports
//
void PORT_Init_DU (void) {

	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;			// set SFR page

	XBR0 = 0x6F;					// 01101111 Enable all but CEX5
	XBR1 = 0x00;					// everything disabled
	XBR2 = 0xC4;					// crossbar enabled, UART1 enabled, no weak pullups

	P0MDOUT = 0x15;					// 00010101 enable TX0, SI, SCK, as push-pulls
	P0 = 0xEA;						// set all open drain pins to 1

	P1MDIN	= 0xFF;					// set all digital inputs (disable analogs)
	P1MDOUT = 0x0D;					// 00001101 enable tx1 pwm0,1 as push-pull
	P1 = 0x02;						// set rx1 to high impedance

//	P1MDOUT = 0x0C;					// 00001100 enable pwm0,1 as push-pull
									// tx1 will be set to push-pull when usb is present
//	P1 = 0x03;						// set rx1, tx1 to high impedance

	P2MDOUT = 0x00;					// all pins open drain
	P2 = 0xFF;						// write a 1 to each bit

	P3MDOUT = 0x00;					// all pins open drain 
	P3 = 0xFF;						// write a 1 to each bit

	P7MDOUT = 0xFF;					// all push-pull
	P7 = 0x8E;						// 00000111 all backlights off, force disp on, dataflash wp on, 
									// dataflash cs off, dataflash reset off
	P6MDOUT = 0x00;
	P6 = 0xFF;						// all open drain

	P5MDOUT = 0x00;
	P5 = 0xFF;						// 11111111 all inputs

	// Port 4 is LCD  and USB control
	// P4.0 = A0
	// P4.1 = E1
	// P4.2 = E2
	// P4.3 = R/W
	// P4.4 = /RST
	// P4.5	= CTS_USB
	// P4.6	= RESET_USB
	// P4.7 = EN_DISP

	P4MDOUT = 0xE0;					// 11100000 P4.0 - P4.4 are open drain, others pushpull
	P4 = 0xF9;						// 11111001

	// configure the PCA for PWM operation
	SFRPAGE = PCA0_PAGE;
	PCA0MD =	0x80;					// 1000000 suspend with microp, SYSCLK / 12
	PCA0CN =	0x40;					// 01000000 PCA0 enabled
	PCA0CPM0 =	0x42;					// CCM0 in 8-bit PWM mode
	PCA0CPM1 =	0x42;					// CCM1 in 8-bit PWM mode

	// get contrast value
	ReadScratchpad(scratch.scratchbuff);

	// if contrast setting bad or never programmed, fix it
	if (scratch.calibrate.contrast > CONTRAST_UPPER ||
		scratch.calibrate.contrast < CONTRAST_LOWER)
			scratch.calibrate.contrast = CONTRAST_CENTER;

	// initialize PWM values for middle of the road
	PCA0CPL0 = scratch.calibrate.contrast;
	PCA0CPH0 = scratch.calibrate.contrast;

	// set up the ADC for reading battery voltage and temperature
	SFRPAGE = ADC0_PAGE;
	ADC0CN = 0xC0;						// 11000001 ADC0 enabled; special tracking
										// ADC0 conversions are initiated 
										// on AD0BUSY=1; ADC0 data is right-justified

	REF0CN = 0x07;                      // enable temp sensor, on-chip VREF,
   										// and VREF output buffer
	AMX0CF = 0x00;						// all non-differential inputs, no gain
	AMX0SL = 0x00;                      // Select AIN0 external input on mux
	ADC0CF = ((SYSCLK/2/ADC_RATE) << 3) | 0x00;	// ** ADC conv clock = 2.5MHz, Gain = 1

	SFRPAGE = SFRPAGE_SAVE;			// Restore SFR page
}

/******************************************************************************
** Display_Init ********************************************** GAS 01JUN04 ****
*******************************************************************************
This performs all of the necessary initialization of the SED-1565 controller
inside the display.
******************************************************************************/
void Display_Init(void) {

	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;			// set SFR page

	RST_DISP = 0; // Hold the display in reset
	DELAY_MS(1);
	RST_DISP = 1; // Release the display in reset
	DELAY_MS(1);

	// NOTE: Initial state of E1, E2, R/W and A0 are set in port setup

	/*
	When the RES input comes to the "L" level, the display is initialized to a
	default state. That default is as follows:

	1) Display OFF
	2) Display start line set to first line
	3) Static drive is OFF
	4) Column address set to Address 0
	5) Page address set to Page 3
	6) 1/32 Duty cycle is selected
	7) Forward ADC is selected(ADC command D0 -is 1, ADC status flag is 1)
	8) Read-modify-write is OFF
	*/

	Write_Display(CMD,RESET_DISPLAY, 0);
	Write_Display(CMD,RESET_DISPLAY, 1);
	Write_Display(CMD,DUTY_CYCLE_SET + 1, 0);
	Write_Display(CMD,DUTY_CYCLE_SET + 1, 1);
	Write_Display(CMD,ADC_SELECT_NORMAL, 0);
	Write_Display(CMD,ADC_SELECT_NORMAL, 1);
	Write_Display(CMD,START_LINE_SET, 0);
	Write_Display(CMD,START_LINE_SET, 1);
	Write_Display(CMD,DISPLAY_ON, 0);
	Write_Display(CMD,DISPLAY_ON, 1);

	SFRPAGE = SFRPAGE_SAVE;
}

/******************************************************************************
** Write_Display ********************************************* MKO 13JUN01 ****
*******************************************************************************
This is the one of the main low level routines used to write either commands
or data to the SED1565 LCD driver. A flag must be passed to the routine to
indicate whether it is being used to pass a command or data to the controller.
	Usage Examples:
		Write_Display(CMD,RESET_DISPLAY);
		Write_Display(DATA,i);
******************************************************************************/

void Write_Display(u08 command, u08 dat, u08 side) {
	u08 status;
	int timeout = 0;

	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page
//	u08 EA_SAVE = EA;

//	EA = 0;
	SFRPAGE = CONFIG_PAGE;			// set SFR page

	// check for busy. timeout eventually and move on
	do {
		status = Read_Display(CMD, side);
		timeout++;
	} while ((status & DISP_BUSY) && (timeout < DISPLAY_TRIES));

	RW_DISP = 0;

	// Select command or data
	if(command == CMD){		// command
		A0_DISP = 0;
	}
	else {					// data
		A0_DISP = 1;
	}

	// select a side of the display
	if (side) { E2_DISP = 1; }
	else { E1_DISP = 1; } 

	// Place the data on the bus
	DATA_8 = dat;

	// change to pushpull
	if (side) { HI(E2_DISP_PP); }
	else { HI(E1_DISP_PP); }
	
	// change to open drain
	if (side) { LO(E2_DISP_PP); }
	else { LO(E1_DISP_PP); }

	// waits at least 300 nsec
	for (status = 1; status != 0; status--) timeout++;

	// latch the data into the display and set up lines for next write
	if (side) { E2_DISP = 0; }
	else { E1_DISP = 0; }
	A0_DISP = 1;
	RW_DISP = 1;

	// set control lines to pushpull
	HI(A0_DISP_PP);
	HI(RW_DISP_PP);

	// set back to open drain
	LO(A0_DISP_PP);
	LO(RW_DISP_PP);

	// set data lines high
	DATA_8 = 0xFF;

	SFRPAGE = SFRPAGE_SAVE;
//	EA = EA_SAVE;

} // Write_Display

/******************************************************************************
** Read_Display ******************************************** GAS 15 JUL 04 ****
*******************************************************************************
This is the one of the main low level routines used to read data and the status
byte from the SED1565 LCD controller. A flag must be passed to the routine to
indicate whether it is being used to pass a command or data to the controller.
	Usage Examples:
		status_read_byte = Read_Display(CMD, side);
		display_data = Read_Display(DATA, side);
******************************************************************************/

u08 Read_Display(u08 command, u08 side) {
	u08 return_byte;

	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page
//	u08 EA_SAVE = EA;

//	EA = 0;
	SFRPAGE = CONFIG_PAGE;			// set SFR page

	// Load the control signals and data for a particular command
	if(command == CMD){		// status read & delay 2 cycles
		A0_DISP = 0;
	}
	else {					// data read & delay 2 cycles
		A0_DISP = 1;
	}

	// write Ex=1, set Ex as push-pull, wait 2 cycles, set Ex as open drain
	if (side)	{ E2_DISP = 1; 
				HI(E2_DISP_PP);
				LO(E2_DISP_PP);
				}
	else		{ E1_DISP = 1;
				HI(E1_DISP_PP);
				LO(E1_DISP_PP);
				}

	// waits at least 300 nsec
	for (return_byte = 3; return_byte != 0; return_byte--) command++;

	// grab the data from the bus
	return_byte = DATA_8;

	// Leave display in correct state
	if (side) { E2_DISP = 0; }
	else { E1_DISP = 0; }
	 
	A0_DISP = 1;

	// change A0 to push-pull
	HI(A0_DISP_PP);

	// A0 back to open drain
	LO(A0_DISP_PP);

	SFRPAGE = SFRPAGE_SAVE;
//	EA = EA_SAVE;

	return(return_byte);
} // Read_Display

/******************************************************************************
** Write_Screen **************************************** GAS 22 JUN 04 ****
*******************************************************************************
This writes an entire bit pattern to the LCD display
If operation = ~, writes reverse image
******************************************************************************/
void Write_Screen(u08 bitmap[NUMBER_OF_PAGES][NUMBER_OF_COLUMNS], char operation) {
	u08 page;
	u16 column;
	u08 side;

	/* Initialize the column address to zero and allow the auto increment to
	move the column address after each write. */

	for(page=0; page<NUMBER_OF_PAGES; page++) {
		column = 0;			// keep track of column number to switch sides
		side = 0;

		Write_Display(CMD, PAGE_ADDRESS_SET + page, 0);
		Write_Display(CMD, COLUMN_SET, 0);

		Write_Display(CMD, PAGE_ADDRESS_SET + page, 1);
		Write_Display(CMD, COLUMN_SET, 1);

		for (column=0;column<NUMBER_OF_COLUMNS;column++) {
				// switch sides as appropriate
				if (column == NUMBER_OF_COLUMNS/2) // select right side of display
					side = 1;

				Write_Display(DATA, (operation == '~')? ~bitmap[page][column] : bitmap[page][column], side);
		} // for
	} // for

	/* Turn on the display to see the new pattern */
	Write_Display(CMD, DISPLAY_ON, 0);
	Write_Display(CMD, DISPLAY_ON, 1);
}

/******************************************************************************
** Write_Icon **************************************** GAS 1 JUL 04 ****
*******************************************************************************
This writes an bit pattern to the LCD display at the prescribed location
locx, locy are the bit address to place icon (upper left 0,0, lower right 121,31)
sizex, sizey are the dimensions of the bitmap
bytex, bytey are the number of bytes in the bitmap
operation is either AND &, OR |, XOR ^, REVERSE ~, or REPLACE -

known issues: if font definition is outside its bounds, Write_Icon will also write
outside the bounds.

******************************************************************************/
void Write_Icon(int locx, int locy, int sizex, int sizey, u08 *bitmap, char operation) {
	u08 page;
	u16 column;
	u08 side;
	int startcolumn, endcolumn;
	u08 startpage, endpage, startside, iconrow;
	xdata u08 bitoffset;
	u08 tempdata, uppermask, lowermask, rcvdata;

	code u08 mask1[] = { 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };
	code u08 mask3[] = { 0x00, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 };
	code u08 mask2[] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 };

	/* Turn off the display during writes */
//	Write_Display(CMD, DISPLAY_OFF, 0);
//	Write_Display(CMD, DISPLAY_OFF, 1);

	startpage = locy / 8;			// figure out initial page
	endpage = MIN(((locy + sizey - 1)/8), NUMBER_OF_PAGES-1);

	bitoffset = locy % 8;			// where icon overlaps page edge
	startcolumn = locx;
	startside = (startcolumn < NUMBER_OF_COLUMNS/2)?  0 : 1;	// pick initial side of display
	endcolumn = MIN((locx + sizex - 1), NUMBER_OF_COLUMNS-1);

	if (1 == startside) {
		startcolumn -= NUMBER_OF_COLUMNS/2;
		endcolumn -= NUMBER_OF_COLUMNS/2;
	}

	for (page=startpage; page <= endpage; page++) {

		side = startside;	// pick initial side of display
		iconrow = (startpage - page + bitoffset) / 8;

		// set the starting write location
		if (0 == startside) {
			Write_Display(CMD, PAGE_ADDRESS_SET + page, 0);
			Write_Display(CMD, COLUMN_SET + startcolumn, 0);

			Write_Display(CMD, PAGE_ADDRESS_SET + page, 1);
			Write_Display(CMD, COLUMN_SET + 0, 1);
		}
		else {
			Write_Display(CMD, PAGE_ADDRESS_SET + page, 1);
			Write_Display(CMD, COLUMN_SET + startcolumn, 1);
		} // else

		for (column=startcolumn; column <= endcolumn; column++) {
			xdata u08 savemask;

			// switch sides as appropriate
			if (column == NUMBER_OF_COLUMNS/2)	// select right side of display
				side = 1;

			if (page == startpage)
				uppermask = 0;
			else {
				// get upper overlap
				tempdata = *(bitmap+(page-startpage-1)*sizex+(column-startcolumn));
				uppermask =	(tempdata & ~mask1[bitoffset]) >> (8-bitoffset);
			}
			
			if ((page == endpage) && ((bitoffset % 8) > (8-(sizey % 8)))) 
				lowermask = 0;
			else {
				// get lower overlap
				tempdata = *(bitmap+(page-startpage)*sizex+(column-startcolumn));
				lowermask = (tempdata & mask1[bitoffset]) << bitoffset;
			} // else

			tempdata = uppermask | lowermask;

			// go into rmw mode to hold the column position
			Write_Display(CMD, READ_MODIFY_WRITE, side);

			rcvdata = Read_Display(DATA, side); // Dummy Read
			rcvdata = Read_Display(DATA, side); // Get current LCD column

			switch (operation) {
				case '&':	// AND
					tempdata &= rcvdata;
					break;
				case '|':	// OR
					tempdata |= rcvdata;
					break;
				case '^':	// XOR
					tempdata ^= rcvdata;
					break;
				case '~':	// REVERSE IMAGE
					tempdata = ~tempdata;
					break;
				default:	// REPLACE
					break;
			} // switch

			// if top or bottom of icon, put back in surrounding parts of screen
			savemask = 0;
			if (startpage == page)
				savemask = ~mask2[bitoffset];
			if (endpage == page)
//				savemask |= ~mask1[(locy + sizey) % 8];
				savemask |= mask3[(locy + sizey) % 8];
			tempdata = (tempdata & ~savemask) | (rcvdata & savemask);

			// restore normal mode
			Write_Display(CMD, END, side);
			
			// put the data on the display
			Write_Display(DATA, tempdata, side);

		} // for
	} // for

	/* Turn on the display to see the new pattern */
//	Write_Display(CMD, DISPLAY_ON, 0);
//	Write_Display(CMD, DISPLAY_ON, 1);
}

/******************************************************************************
** Full_Screen **************************************** GAS 22 JUN 04 ****
*******************************************************************************
This writes the same byte to every location on the LCD display
******************************************************************************/
void Full_Screen(u08 dat) {
	u08 page;
	u16 column;
	u08 side;

	// Turn off the display while writing
	Write_Display(CMD, DISPLAY_OFF, 0);
	Write_Display(CMD, DISPLAY_OFF, 1);

	/* Initialize the column address to zero and allow the auto increment to
	move the column address after each write. */

	for(page=0; page<NUMBER_OF_PAGES; page++) {
		column = 0;			// keep track of column number to switch sides
		side = 0;

		Write_Display(CMD, PAGE_ADDRESS_SET + page, 0);
		Write_Display(CMD, COLUMN_SET, 0);

		Write_Display(CMD, PAGE_ADDRESS_SET + page, 1);
		Write_Display(CMD, COLUMN_SET, 1);

		for (column=0;column<NUMBER_OF_COLUMNS;column++) {
				// switch sides as appropriate
				if (column == NUMBER_OF_COLUMNS/2) // select right side of display
					side = 1;
				Write_Display(DATA, dat, side);
		} // for
	} // for

	// Turn on the display to see the new pattern
	Write_Display(CMD, DISPLAY_ON, 0);
	Write_Display(CMD, DISPLAY_ON, 1);
}

/******************************************************************************
** Write_Line **************************************** GAS 19 JUL 04 ****
*******************************************************************************
This writes a 20 char line using the smallest font to the screen. It will blank
pad or truncate the line but does not wrap. Line ranges from 0-3.
operation is either AND &, OR |, XOR ^, REVERSE ~, or REPLACE -
******************************************************************************/
void Write_Line(u08 linenum, char *linetext, BOOL scroll, char operation) {
	int i;

	if (scroll) {
		linenum = topline;
		topline = ++topline % NUMBER_OF_PAGES;
		Write_Display(CMD, START_LINE_SET + (topline*8), 0);
		Write_Display(CMD, START_LINE_SET + (topline*8), 1);
	} // if 
	
	for (i=0; i<(NUMBER_OF_COLUMNS-5); i += 6) {
		
		if ((*linetext >= ' ') && (*linetext <= '~'))
			Write_Icon(i, 8*linenum, 5, 8, &(smallascii[*linetext -' '][0][0]), operation);
		else // print a blank
			Write_Icon(i, 8*linenum, 5, 8, &smallascii[0][0][0], operation);

		if ('\0' != *linetext) linetext++;

	} // for
} // Write_Line

/******************************************************************************
** Write_Text **************************************** GAS 19 JUL 04 ****
*******************************************************************************
This writes a text string using the smallest font to the screen.
Line ranges from 0-3.
Column ranges from 0-121.
operation is either AND &, OR |, XOR ^, REVERSE ~, or REPLACE -
******************************************************************************/
void Write_Text(u08 linenum, u08 colnum, char *linetext, char operation) {
	xdata int i;

	for (i=colnum; i<(NUMBER_OF_COLUMNS-5); i += 6) {

		if ('\0' == *linetext) break;
		
		if ((*linetext >= ' ') && (*linetext <= '~'))
			Write_Icon(i, 8*linenum, 5, 8, &(smallascii[*linetext -' '][0][0]), operation);
		else // print a blank
			Write_Icon(i, 8*linenum, 5, 8, &smallascii[0][0][0], operation);

		linetext++;

	} // for
} // Write_Text

/******************************************************************************
** Write_Big_Line **************************************** GAS 17 JAN 05 ****
*******************************************************************************
This writes a 12 char line using the large font to the screen. It will blank
pad or truncate the line but does not wrap. Line ranges from 0-3. For 2 lines,
use lines 0 and 2.
operation is either AND &, OR |, XOR ^, REVERSE ~, or REPLACE -
******************************************************************************/
void Write_Big_Line(u08 linenum, char *linetext, BOOL scroll, char operation) {
	int i;

	if (scroll) {
		// get topline to even number
		topline &= 0x02;
		linenum = topline;
		topline = (2+topline) % NUMBER_OF_PAGES;
		Write_Display(CMD, START_LINE_SET + (topline*8), 0);
		Write_Display(CMD, START_LINE_SET + (topline*8), 1);
	} // if 
	
	for (i=0; i<(NUMBER_OF_COLUMNS-9); i += 10) {
		
		if ((*linetext >= ' ') && (*linetext <= '~'))
			Write_Icon(i, 8*linenum, 8, 16, &(largeascii[*linetext -' '][0][0]), operation);
		else // print a blank
			Write_Icon(i, 8*linenum, 8, 16, &largeascii[0][0][0], operation);

		if ('\0' != *linetext) linetext++;

	} // for
} // Write_Big_Line

/******************************************************************************
** Write_Big_Text **************************************** GAS 17 JAN 05 ****
*******************************************************************************
This writes a text string using the large font to the screen.
Line ranges from 0-3. For 2 lines, use lines 0, 2.
Column ranges from 0-121.
operation is either AND &, OR |, XOR ^, REVERSE ~, or REPLACE -
******************************************************************************/
void Write_Big_Text(u08 linenum, u08 colnum, char *linetext, char operation) {
	int i;

	for (i=colnum; i<(NUMBER_OF_COLUMNS-9); i += 10) {

		if ('\0' == *linetext) break;
		
		if ((*linetext >= ' ') && (*linetext <= '~'))
			Write_Icon(i, 8*linenum, 8, 16, &(largeascii[*linetext -' '][0][0]), operation);
		else // print a blank
			Write_Icon(i, 8*linenum, 8, 16, &largeascii[0][0][0], operation);

		linetext++;

	} // for
} // Write_Big_Text

/******************************************************************************
** Write_Med_Text **************************************** GAS 29 JAN 05 ****
*******************************************************************************
This writes a text string containing only digits using the medium font to the
screen. Row ranges from 0-31. Column ranges from 0-121.
operation is either AND &, OR |, XOR ^, REVERSE ~, or REPLACE -
******************************************************************************/
void Write_Med_Text(u08 rownum, u08 colnum, char *linetext, char operation) {
	int i;

	for (i=colnum; i<(NUMBER_OF_COLUMNS-6); i += 7) {

		if ('\0' == *linetext) break;
		
		if ((*linetext >= '0') && (*linetext <= '9'))
			Write_Icon(i, rownum, 6, 10, &(meddigits[*linetext -'0'][0][0]), operation);
		else // write a space
			Write_Icon(i, rownum, 6, 10, &(medspace[0][0]), operation);

		linetext++;
	} // for
} // Write_Med_Text
#if 0
/******************************************************************************
** Draw_Rect *********************************************** GAS 07 MAR 05 ****
*******************************************************************************
This draws a rectangle from upper left to lower right. Row ranges from 0-31.
Column ranges from 0-121.

operation is either AND &, OR |, XOR ^, REVERSE ~, or REPLACE -
******************************************************************************/
void Draw_Rect(int ulcol, int ulrow, int lrcol, int lrrow, char operation) {

	Write_Icon(ulcol, ulrow, lrcol-ulcol+1, lrrow-ulrow+1, (u08 *) allonbckgnd, operation);

} // Draw_Rect
#endif
/******************************************************************************
** Reset_Top_Line **************************************** GAS 19 AUG 04 ****
*******************************************************************************
This function sets the top line of the display. Used to recover from scrolling.
******************************************************************************/
void Reset_Top_Line(void) {

	Write_Display(CMD, START_LINE_SET, 0);
	Write_Display(CMD, START_LINE_SET, 1);

} // Reset_Top_Line

#ifndef SAVEMEM
/******************************************************************************
** Info_Screen **************************************** GAS 31 JAN 05 ****
*******************************************************************************
This function writes text to the display in the information bubble.
******************************************************************************/
void Info_Screen(char *infotext) {

	Write_Screen(infoscreen, '-');
	Write_Big_Text(1, 7, infotext, '-');

} // Info_Screen
#endif