#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>

/* universal data elements */
typedef unsigned char u08;
typedef unsigned int u16;
typedef unsigned long u32;
typedef char s08;
typedef int s16;
typedef long s32;

#include <crc16table.c>
#include <boolean.h>
#include <c8051f120.h>
//#include <message.h>
#include <opcode.h>

#define DISPLAYATTACHED
#define NOTEST 0

//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F02x
//-----------------------------------------------------------------------------

sfr16 DP       = 0x82;                 // data pointer
sfr16 ADC0     = 0xbe;                 // ADC0 data
sfr16 ADC0GT   = 0xc4;                 // ADC0 greater than window
sfr16 ADC0LT   = 0xc6;                 // ADC0 less than window
sfr16 RCAP2    = 0xca;                 // Timer2 capture/reload
sfr16 RCAP3    = 0xca;                 // Timer3 capture/reload
sfr16 RCAP4    = 0xca;                 // Timer4 capture/reload
sfr16 TMR2     = 0xcc;                 // Timer2
sfr16 TMR3     = 0xcc;                 // Timer3
sfr16 TMR4     = 0xcc;                 // Timer4
sfr16 DAC0     = 0xd2;                 // DAC0 data
sfr16 DAC1     = 0xd2;                 // DAC1 data
sfr16 PCA0CP5  = 0xe1;                 // PCA0 Module 5 capture
sfr16 PCA0CP2  = 0xe9;                 // PCA0 Module 2 capture
sfr16 PCA0CP3  = 0xeb;                 // PCA0 Module 3 capture
sfr16 PCA0CP4  = 0xed;                 // PCA0 Module 4 capture
sfr16 PCA0     = 0xf9;                 // PCA0 counter
sfr16 PCA0CP0  = 0xfb;                 // PCA0 Module 0 capture
sfr16 PCA0CP1  = 0xfd;                 // PCA0 Module 1 capture

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------

#define SAMPLE_RATE	160					// THIS AFFECTS COMM TIMEOUT!!
										// ADC record rate in hz (must be 10x INT_DEC)
#define SAMPS_PER_SEC	10				// number of accel samples to store
#define INT_DEC		(SAMPLE_RATE / SAMPS_PER_SEC)	// integrate and decimate ratio (64x oversample)


#define SOH 0x16
// communication timeout
#define CTS_COUNTDOWN	40			// MAX = 256!!, 40 / 160 = .250 sec

enum {
	START,
	SOURCE,
	DESTINATION,
	UPPER_BYTE_COUNT,
	LOWER_BYTE_COUNT,
	OPCODE,
	CHECKSUM,
	DATA_START } ;

#define MSG_WRAPPER	(DATA_START + 2)	// length of message wrapper including CRC

enum states {
	LOOKING_FOR_SYNC = 0x01,
	POSSIBLE_SYNC,
	DO_CHECKSUM,
	FOUND_SYNC,
	MESSAGE_COMPLETE } ;

// unit ids
enum units {
	SELF,
	PC,								// 1
	SENSOR_UNIT,					// 2
	DISPLAY_UNIT					// 3
};
#define NUMUNITS		DISPLAY_UNIT		// total number of units

#define ASCIIBAUD		38400L			// Baud rate of UART0 in bps
#define MSGBAUD			115200L			// Baud rate of UART0 in bps
#define INTCLK			24500000		// Internal oscillator frequency in Hz
#define SYSCLK			98000000		// Output of PLL derived from (INTCLK*2)
// #define SYSCLK			22118400L		// SYSCLK frequency in Hz
#define MSGDATA			530				// max size data transmitted in a message
#define MSGSIZE			(MSGDATA + MSG_WRAPPER + 2)	// each packet is MSGSIZE bytes + wrapper + CRC
#define NAMELEN			16				// length of text string for driver, car, track
#define NUMSESSIONS		26				// 512 / 19 = 26 sessions
#define NUMDRIVERS		12				// 12 individual drivers allowed
#define NUMCARS			12				// 12 individual cars allowed
#define NUMTRACKS		16				// 16 tracks allowed
#define INDEXPAGE		0				// session index segment
#define USERPAGE		1				// user data segment
#define TRACKPAGE		2				// track data segment
#define DATAPAGE		3				// first data segment

//Application specific definitions
#define MAX_PAGE_SIZE				MAX_PAGE_32MBIT	// change to 64 bit if using bigger part
										
#define SAMPS_PER_PAGE				8		// number of seconds per storage page
#define OWNER_LEN					80		// number of characters for owner name & contact

//Cygnal 8051 SPI Definitions
#define	SPICFG_MODE_MASK			0xC0
#define	SPICFG_BITCNT_MASK			0x38
#define	SPICFG_FRMSIZ_MASK			0x07
#define SCRATCHLEN					128		// size of scratchpad in flash */
#define FLASHLEN					528		// size of a page in flash
#define NUMPORTS					2 		// number of UARTS on MCU

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------

#ifdef PRINTASCII		// print it out the serial port
#define PRINT(FORMAT, TEXT) do { sprintf (tx0buff, FORMAT"\r\n", TEXT); XmitUART0(tx0buff, strlen(tx0buff)); } while (0)
#else
#ifdef PRINTDISPLAY		// send a message to display unit
#define PRINT(FORMAT, TEXT) do { sprintf (MsgPayload, FORMAT, TEXT); SendMessage((u08) DISPLAY_UNIT, (u08) WRITE_DISPLAY_TEXT, strlen(MsgPayload)+1); } while (0)
#else		// swallow any prints
#define PRINT(FORMAT, TEXT) 
#endif
#endif

#define TEXTPRINT(TEXT) PRINT("%s",TEXT)

// Useful Macros
#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))? (x):(y))
#define ABS(a) ((a >= 0)? (a) : -(a))
#define HI(x) ((P##x) |= (1<<(B##x)))
#define LO(x) ((P##x) &= ~(1<<(B##x)))
#define XOR(x) ((P##x) ^= (1<<(B##x)))
#define ISHI(x) ((P##x) & (1<<(B##x)))
#define WAIT(x) do {xdata int qqq; for (qqq=x; qqq>0; qqq--); } while(0)
#define DELAY_MS(x) do {xdata long zzz; for (zzz=(1100L * x); zzz>0; zzz--); } while(0)
#define tohex(c) (c<10? (c+'0'):(c+'A'-10))

#define PDF_RST	P7						// DataFlash chip reset
#define BDF_RST	2
#define PDF_WP	P7						// DataFlash chip write protect
#define BDF_WP	0
#define PDF_CS	P7						// DataFlash chip select
#define BDF_CS	1

#define PRS232ON	P5					// 1 forces RS-232 on
#define BRS232ON	5
#define PRS232OFF	P5					// 0 forces RS-232 off
#define BRS232OFF	6

#define P3VDISPEN	P7					// 1 turns on display
#define B3VDISPEN	3
#define P3VGPSEN	P7					// 1 turns on GPS circuitry
#define B3VGPSEN	4
#define P3VIFEN		P7					// 1 turns on external interface
#define B3VIFEN		5
#define P5VEN		P7					// 1 keeps power on main unit
#define B5VEN		7

#define PACCST		P5					// 1 turns on accelerometer selftest
#define BACCST		2

//types of com ports
typedef enum {
	MESSAGING,
	TSIPGPS,
	ASCII
} comusagetype;

typedef enum {
	OPEN,
	CLOSE
} comaction;

typedef struct {
	u08 port;						// port number
	comusagetype use;				// what port is used for
	u16 txbytes;					// number of bytes left to xmit on port
	u08 *rxptr;						// NULL when no rx'd msg, else point to rx'd msg
	u08 *txptr;						// points to current byte in output buffer
	char parity;					// which parity is in use
	void (*int_func)(void);			// interrupt driver
	u08 rxidx;						// index to tsip rx buffer
	u08 txbuff[MSGSIZE];			// pointer to transmit buffer
	u08 rxbuff[2][MSGSIZE];			// pointer to receive buffer
} comporthandle;

// Display Unit Modes
typedef enum {
	WAITING,
	DRIVE,
	DRAGRACE,
	AUTOCROSS,
	LAPS,
	TIMING,
	ERASE
} tmstatetype;

//------------------------------------------------------------------------------------
// Function PROTOTYPES
//------------------------------------------------------------------------------------
s16 SendMessage(u08, u08, u16 );
s16 Verify_CRC(u08 *, u16) ;
s16 SendNAK(u08, u08);
s16 Parse_Message(u08 *, u08);
void SYSCLK_Init (void);
void UART_Init (comporthandle *, u32 );
void XmitUART(comporthandle *, u16 );
void Timer3_Init (u16);
void PORT_Init_DU (void);
unsigned int crc16(u08 *, u16);
s16 Verify_CRC(u08 * , u16 );

// Global Variables
xdata comporthandle com[NUMPORTS];			// handles to com ports
//	0 place = this units id, 1 unit = PC = COM1, 2 unit = SU = COM0, 3 unit = DISPLAY = self = -1
code s08 msgrouting[NUMUNITS+1] = {DISPLAY_UNIT, 1, 0, -1};	// what port to route messages to
xdata u08 CTS[NUMUNITS] = {0};	// non-zero when waiting for response, 0 when clear to send
xdata u16 clocktick = 0;			// increments in 1/SAMPLE_RATE sec intervals
xdata tmstatetype dispstate = WAITING;		// where in the state machine
					// timing and position of lap


void main (	void ) {
	xdata BOOL collecting = FALSE;			// TRUE when collecting data
	int retval;								// function call return value

	SFRPAGE = CONFIG_PAGE;

	// disable watchdog timer
	WDTCN = 0xde;
	WDTCN = 0xad;

	SYSCLK_Init();						// initialize oscillator
	PORT_Init_DU();						// enable ports

	// create com ports

	// initialize
	com[0].txbytes = 0;
	com[0].rxidx = 0;
	com[0].rxptr = com[0].txptr = NULL;
	com[0].use = MESSAGING;
	com[0].port = 0;
	com[0].parity = 'N';	// parity = NONE

	UART_Init(&(com[0]), MSGBAUD);
	Timer3_Init ((u16) (SYSCLK/SAMPLE_RATE/12));	// initialize Timer3 to overflow at sample rate

	EA = 1;								// enable global interrupts

	// Reset the sensor unit
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = DRIVE;
	SendMessage((u08) SENSOR_UNIT, (u08) MODE_CHANGE, 1);
	while (1) {
		u08 *fred;

		// check for incoming sensor unit message
		if (NULL != com[0].rxptr) {

			fred = com[0].rxptr;
			// process the message
			retval = Parse_Message(fred, (u08) 0);

			// clear the semaphore to signal buffer free
			com[0].rxptr = NULL;
		} // if

	} // while
} // main

//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SYSCLK_Init
//-----------------------------------------------------------------------------
//
// This routine initializes the system clock to use the internal oscillator
// at 24.5 MHz multiplied by two using the PLL.
//
void SYSCLK_Init (void)
{
	int i;									// software timer
	u08 SFRPAGE_SAVE = SFRPAGE;				// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;					// set SFR page
	SFRPGCN = 0x01;							// turn on auto SFR paging for interrupts

	OSCICN = 0x83;							// 10000011 set internal oscillator to run at max frequency
	CLKSEL = 0x00;							// Select the internal osc. as the SYSCLK source

	PLL0CN= 0x00;							// Set internal osc. as PLL source

	SFRPAGE = LEGACY_PAGE;
	FLSCL	= 0x50;							// Set FLASH read time for 100MHz clk

	SFRPAGE = CONFIG_PAGE;
	PLL0CN |= 0x01;							// Enable Power to PLL
	PLL0DIV = 0x01;							// Set Pre-divide value to N (N = 1)
	PLL0FLT = 0x01;							// Set the PLL filter register for
											// a reference clock from 19 - 30 MHz
											// and an output clock from 45 - 80 MHz
	PLL0MUL = 0x04;							// Multiply SYSCLK by M (M = 4)

	for (i=0; i < 256; i++) ;				// Wait at least 5us
	PLL0CN |= 0x02;							// Enable the PLL
	while(!(PLL0CN & 0x10));				// Wait until PLL frequency is locked
	CLKSEL= 0x02;							// Select PLL as SYSCLK source

	SFRPAGE = SFRPAGE_SAVE;					// Restore SFR page
}

//-----------------------------------------------------------------------------
// UART_Init
//-----------------------------------------------------------------------------
//
// Configure the UART for interrupt driven operation, <baud> and <parity>.
//
void UART_Init (comporthandle *handle, u32 baud)
{
	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	switch(handle->port) {
		case 0:									// Use Timer 2 for baud

			SFRPAGE = UART0_PAGE;
			SSTA0 = 0x15;						// 00010101 no doubler, user timer 2

			switch (handle->parity) {
				case 'n':
				case 'N':
					SCON0 = 0x50;				// SCON0: mode 1, 8-bit UART, enable RX
					break;
				case 'e':
				case 'E':
				case 'o':
				case 'O':
					SCON0 = 0xD0;				// SCON0: mode 3, 9-bit UART, enable RX
					break;
			} // switch

			SFRPAGE = TMR2_PAGE;
			TMR2CN = 0x04;						// 00000100 no ext trig, enabled, timer, auto reload
			TMR2CF = 0x08;						// 00001000 sysclk source, count up

			RCAP2 = -(SYSCLK/baud/16);			// set timer 2 reload value
			TMR2 = 0xFFFF;						// set to reload immediately
			TR2 = 1;							// start timer

			SFRPAGE = UART0_PAGE;
			RI0 = TI0 = 0;						// Clear interrupt flags
			ES0 = 1;							// Enable UART0 interrupts
			break;
		case 1:									// User Timer 1 for baud

			SFRPAGE = UART1_PAGE;
			switch (handle->parity) {
				case 'n':
				case 'N':
					SCON1 = 0x10;				// 00010000 SCON1: mode 1, 8-bit UART, enable RX
					break;
				case 'e':
				case 'E':
				case 'o':
				case 'O':
					SCON1 = 0x90;				// 10010000 SCON1: mode 3, 9-bit UART, enable RX
					break;
			} // switch

			SFRPAGE = TIMER01_PAGE;
			TMOD	&= ~0xF0;
			TMOD	|=  0x20;						// TMOD: timer 1, mode 2, 8-bit reload

			if (SYSCLK/baud/2/256 < 1) {
				TH1 = -(SYSCLK/baud/2);
				CKCON |= 0x10;						// T1M = 1; SCA1:0 = xx
			} else if (SYSCLK/baud/2/256 < 4) {
				TH1 = -(SYSCLK/baud/2/4);
				CKCON &= ~0x13;						// Clear all T1 related bits
				CKCON |=0x01;						// T1M = 0; SCA1:0 = 01
			} else if (SYSCLK/baud/2/256 < 12) {
				TH1 = -(SYSCLK/baud/2/12);
				CKCON &= ~0x13;						// T1M = 0; SCA1:0 = 00
			} else {
				TH1 = -(SYSCLK/baud/2/48);
				CKCON &= ~0x13;						// Clear all T1 related bits
				CKCON |=0x02;						// T1M = 0; SCA1:0 = 10
			}

			TL1 = TH1;								// initialize Timer1
			TR1 = 1;								// start Timer1

			SFRPAGE = UART1_PAGE;
			RI1 = TI1 = 0;							// Indicate TX1 ready

			EIE2 |= 0x40;							// ES1=1, Enable UART1 interrupts
			break;
	} // switch

	SFRPAGE = SFRPAGE_SAVE;			// Restore SFR page

} // UART_Init

//------------------------------------------------------------------------------------
// PORT_Init_DU
//------------------------------------------------------------------------------------
//
// Configure the Crossbar and GPIO ports
//
void PORT_Init_DU (void) {

	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;			// set SFR page

	XBR0 = 0xFF;					// all functions available at port pins
	XBR1 = 0xFF;					// ditto
	XBR2 = 0xC4;					// 11000100 all functions, no pullups, xbar enable, t4 off,
									// Uart1 Enable, Ext Mem Disable, Ext Convert off

	P0MDOUT = 0x15;					// 00010101 enable TX0, SI, SCK, as push-pulls
	P0 = 0xEA;						// set all open drain pins to 1

	P1MDIN	= 0x03;					// 00000011 set all analog inputs except TX1, RX1
	P1MDOUT = 0x01;					// 00000001 enable tx1 as push-pull

	P2MDOUT = 0x00;					// all pins open drain
	P2 = 0xFF;						// write a 1 to each bit

	P3MDOUT = 0x00;					// all pins open drain 
	P3 = 0xFF;						// write a 1 to each bit

	P7MDOUT = 0xFF;
	P7 = 0x86;						// 00000111 all backlights off, force disp on, dataflash wp on, 
									//			dataflash cs off, dataflash reset off
	P6MDOUT = 0x00;
	P6 = 0xFF;						// 11110111

	P5MDOUT = 0x00;
	P5 = 0xFF;						// 11111111 all inputs

	P4MDOUT = 0x00;					// 4 all pushpull
	P4 = 0xFF;						// 11111111

	SFRPAGE = SFRPAGE_SAVE;			// Restore SFR page
}
//-----------------------------------------------------------------------------
// Timer3_Init
//-----------------------------------------------------------------------------
//
// Configure Timer3 to auto-reload at interval specified by <counts> and generate
// an interrupt which will start a sample sycle. Uses SYSCLK as its time base.
//
void Timer3_Init (u16 counts)
{
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = TMR3_PAGE;

	TMR3CN = 0x00;						// Stop Timer3; Clear TF3;
	TMR3CF = 0x01;						// use SYSCLK/12 as timebase, count up

	RCAP3 = - (int) counts;				// Init reload values

	TMR3 = 0xFFFF;						// set to reload immediately
	EIE2 |= 0x01;						// enable Timer3 interrupts
	TMR3CN |= 0x04;						// start Timer3

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
}

//////
/// MESSAGING
/////

#define SendACK_Data(DEST, LEN) SendMessage((u08) DEST, (u08) ACK,(LEN))
#define SendACK(DEST) SendACK_Data(DEST, 0)
// #define Verify_CRC(DAT, COUNT) (((((u16)*(DAT))<<8) + *(DAT+1)) == crc16((DAT), (COUNT)))
#define MSG_SEG	(pagesize + MSG_WRAPPER)

// The following function gets passed all the input data
// necessary to build a complete message. For efficiency it does not
// get passed the data buffer itself.
// In addition to being passed the data, the function will also
// call supporting functions to calculate checksums for the header
// and if appropriate, crc's for the data portion of the message

s16 SendMessage(	u08 dest,		// unit id of destination
			 		u08 opcode,		// message opcode
					u16 bytes)	{	// number of bytes to transmit
	u08 chk = 0xFF;					// used to start the header checksum calculation
	u16 i = 0;						// general index
	u16 crc = 0;					// used to hold the calculated CRC
	u08 port = msgrouting[dest];

	// grab the unit
	CTS[dest-1] = CTS_COUNTDOWN;

	com[port].txbuff[START] = SOH ;  // start of header
	com[port].txbuff[SOURCE] = msgrouting[0]; // source of the message
	com[port].txbuff[DESTINATION] = dest; // destination of the message (same as above)

	bytes += 2;					// add two bytes to the total for the CRC

	com[port].txbuff[UPPER_BYTE_COUNT] = (bytes & 0xff00)>>8 ;  // format byte count to
	com[port].txbuff[LOWER_BYTE_COUNT] = bytes & 0xff;          // two byte field

	com[port].txbuff[OPCODE]=opcode ;	// store the message op code
	
	for (i=0; i<CHECKSUM;i++)	// calculate the message header checksum
		chk ^= com[port].txbuff[i] ;		// over all bytes from SOH through op code
	
	com[port].txbuff[CHECKSUM] = chk ;	// store the checksum in final header byte

	bytes -= 2;					// remove crc from total

	crc = crc16(&(com[port].txbuff[DATA_START]),bytes);	// calculate the crc

	com[port].txbuff[++bytes+CHECKSUM] = (crc & 0xff00)>>8 ;		// store the high crc byte
	com[port].txbuff[++bytes+CHECKSUM] = (crc & 0xff) ;			// store the low crc byte

	XmitUART(&(com[port]), DATA_START+bytes);						// send it
	return(DATA_START+bytes) ;  // return total number of bytes processed in message

} // SendMessage

// the following routine will parse a message based on the op code
// it is up to the opcode to determine any additional processing, such as
// CRC calculations, or the meaning of the data
s16 Parse_Message(u08 *msgptr, u08 port) 
{
	int retval;
	static int counter = 0;
	xdata u16 msgdatalen;
//	xdata u16 temp1, temp2, temp3, temp6;
//	xdata u08 *tmpptr1;
//	xdata u08 *tmpptr2;
//	xdata int temp4, temp5;

	/// ***** DATA TRAP
	if ((* ((u08 *) &msgptr)) == 0) {
		int retval = 1;
	}

	counter++;
//	tmpptr1 = msgptr+UPPER_BYTE_COUNT;
//	tmpptr2 = msgptr+LOWER_BYTE_COUNT;

//	temp1 = *tmpptr1 << 8;
//	temp2 = *tmpptr2;
//	temp3 = temp1 + temp2 - 2;
//	temp4 = UPPER_BYTE_COUNT;
//	temp5 = LOWER_BYTE_COUNT;

	msgdatalen = (msgptr[UPPER_BYTE_COUNT] << 8) + msgptr[LOWER_BYTE_COUNT] - 2;
//	msgdatalen = temp3;

/// ***** DATA TRAP
	if (msgdatalen > 0x100) {
		int retval = 1;
	}

	retval = Verify_CRC(com[0].rxptr+DATA_START, msgdatalen);

	if (retval) {		// bad data
		if (NAK != msgptr[OPCODE] && ACK != msgptr[OPCODE])	{ // don't NAK an ACK or NAK
			// grab the port
			CTS[msgptr[SOURCE]-1] = CTS_COUNTDOWN;

			retval = SendNAK(msgptr[SOURCE], (u08) BAD_CRC);		// no data is returned and no crc
		} // if
	} // if
	else {				// parse the good data
		// ***** if message not for me, then route it

		switch (com[0].rxptr[OPCODE]) {
		case ACK:
			// free up unit for more communications
			CTS[msgptr[SOURCE]-1] = 0;
			break;

		case NAK:
			// fall back and retry PUT CODE HERE

			// free up unit for more communications
			CTS[msgptr[SOURCE]-1] = 0;
			break;

		case GPS_DATA: {

			// ACK the message
//			retval = SendACK(msgptr[SOURCE]);

			// free up unit for more communications
			CTS[msgptr[SOURCE]-1] = 0;

		} // GPS_DATA
		case GPS_PULSE: {

//			retval = SendACK(msgptr[SOURCE]);

			// free up unit for more communications
			CTS[msgptr[SOURCE]-1] = 0;

		} // GPS_PULSE

		case ACCEL_DATA: {

//			retval = SendACK(*(msgptr+SOURCE));
			
			// free up unit for more communications
			CTS[msgptr[SOURCE]-1] = 0;

			break;			
		}
		} // switch

	} // else

	return(retval);  // this should be the return value of sending ACK or ACK with
	                 // data, assuming the opcode processes successfully
}


// The following function calculates the crc16 result and verifies it against*/
// the received value in the data.  The function returns:    
// 0 if the crc compares
// -1 if the crc does not match

s16 Verify_CRC(u08 * dat, u16 bytecount) {
	u16 retval, data_crc ;

	retval = crc16(dat, bytecount); // calculate the crc on the received data

	data_crc = (dat[bytecount])<<8 ;  // get the crc from the message
	data_crc += (dat[bytecount+1]);

	if (retval == data_crc)  // check calculated against received
		return(0) ;  // if the crc's match
	else
		return(-1); // -1 otherwise
}

s16 SendNAK(u08 dest,		// unit id of destination
			u08 reason) {
	com[msgrouting[dest]].txbuff[DATA_START] = reason ;
	SendMessage(dest,NAK,1);

	return(0) ;

}

// This function starts an interrupt-driven transmit of the trasmit buffer 
// on designated UART. Blocks only if previous transmission is in progress.
void XmitUART(comporthandle *handle, u16 numbytes) {

	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

// ***** don't think we need to check CTS here but maybe
	while (0 != handle->txbytes) ;		// wait for previous transmission to finish
//	while (0 != CTS[handle->port]);
	handle->txptr = handle->txbuff;
	handle->txbytes = numbytes;
	if (0 == handle->port) {
		SFRPAGE = UART0_PAGE;
		TI0 = 1;						// generate an interrupt to get started
	}
	else {
		SFRPAGE = UART1_PAGE;
		TI1 = 1;						// generate an interrupt to get started
	} // else
	SFRPAGE = SFRPAGE_SAVE;
}

//**********************
// Interrupt Functions
//**********************


// MSG0 ISR - Message Protocol on UART 0
#define SCON SCON0
#define SBUF SBUF0
#define COM 0
void MSG0_ISR (void) interrupt 4 using 2 {
	u08 tempch;
	static int msg_state = LOOKING_FOR_SYNC;	// init state machine variable
	static int msg_length ; 					// variables to build message
	static u08 chk = 0;							// init the checksum variable
	static int index = 0;						// init the receive character storage index
	static u08 *buff;							// for quicker access to buffer

	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	// determine if tx or rx or both caused interrupt

	if (SCON & 0x01) {			// byte received
		tempch = SBUF;
		SCON &= ~0x01;						// clear receive interrupt flag

		switch(msg_state) { 					// based on the state of reception we are in
		case LOOKING_FOR_SYNC:				// state = 1
			index = 0;       				// init the storage index
			if (tempch==SOH) {				// if start of header byte received
				buff = &(com[COM].rxbuff[com[COM].rxidx][0]);	// for quicker access to buffer
				buff[index++]=tempch ;	// store SOH
				chk ^= tempch ;				// start checksum calc
				msg_state = POSSIBLE_SYNC;      // update state
			}
			break ;
		
		case POSSIBLE_SYNC:					// state = 2
			buff[index++] = tempch ;	// get the next byte
			chk ^= tempch ;				// continue checksum calc
			if (index == CHECKSUM)			// see if we should have
				msg_state = DO_CHECKSUM;		// received the checksum
			break ;

		case DO_CHECKSUM:					// state = 3
			buff[index++]=tempch ;		// store rcv checksum
			chk ^= 0xff ;					// final calculation exor
			if (chk == tempch) {			// check against rcv'd checksum
											// if they match, get the message length
				msg_length = buff[UPPER_BYTE_COUNT];
				msg_length = msg_length << 8 ;
				msg_length += buff[LOWER_BYTE_COUNT];

				if(msg_length) msg_state=FOUND_SYNC ;	// remainder of message
				else msg_state=MESSAGE_COMPLETE;		// no message remaining
			}
			else {							// if checksum failed, start looking again
				index=0;					// discard all received data
				msg_state=LOOKING_FOR_SYNC;		// set new state
				chk = 0;					// re-init checksum byte
			}
			break ;
		
		case FOUND_SYNC:					// get remainder of message // state = 4
			buff[index++]=tempch ;		// store the data
			if (index == DATA_START+msg_length)		// see if finished
				msg_state=MESSAGE_COMPLETE ;			// set message complete
			break;

		default:            // should never get here if state machine works
			break ;
		} // switch
									// state = 5
		if(msg_state==MESSAGE_COMPLETE) { // check to see if complete msg received
			msg_state=LOOKING_FOR_SYNC ; // if so, set up for new msg
			index = 0 ;              // reset index
			chk = 0;                 // reset checksum variable
			if (NULL == com[COM].rxptr)	{				// if last buffer processed
				com[COM].rxptr = buff; 					// set semaphore
				com[COM].rxidx = com[COM].rxidx ^ 1;	// swap buffers
			} // if
			else {										// overwrite current buffer
				// ******************* Just for test
				index = 0;
			}
		}
	} // if

	if (SCON & 0x02) {		// byte transmitted
		if (0 != com[COM].txbytes) {
			SBUF = *(com[COM].txptr);
			(com[COM].txptr)++;
			(com[COM].txbytes)--;
		}
		else {	 
			com[COM].txptr = NULL;
		}
		SCON &= ~0x02;						// clear transmit interrupt flag
	} // if

	SFRPAGE = SFRPAGE_SAVE;

} // MSG0_ISR

// Timer 3 Interrupt
//
// When T3 expires, start a series of ADC conversions and flash LED as required
void T3_ISR (void) interrupt 14 using 3
{
	u16 i;

	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = TMR3_PAGE;

	// clear Timer 3 interrupt
	TMR3CN &= 0x7F;

	// increment a clock tick
	clocktick++;

	// count down timeouts for serial communication
	for (i = 0; i<NUMUNITS; i++)
		if(CTS[i]) CTS[i]--;

	SFRPAGE = SFRPAGE_SAVE;

}