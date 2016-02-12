// Part of traqmate.c
// 3/30/2009
//
// Author: GAStephens
//
// These functions control the onboard SMB bus (I2C) serial communications bus in the SiLabs microcontroller.
// These functions are only valid when used with the second generation Sensor Unit (SU2).
// 
#include <stdlib.h>
#include <sensor.h>

//-----------------------------------------------------------------------------
// F12x_SMBus_Master_Multibyte.c
//-----------------------------------------------------------------------------
// Copyright 2006 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// C8051F12x SMBus interface in Master mode.
// - Interrupt-driven SMBus implementation
// - Only master states defined (no slave or arbitration)
// - multiple-byte SMBus data holders used for each transmit and receive
// - Timer3 used by SMBus for SMBSCL low timeout detection
// - SMBSCL frequency defined by <SMB_FREQUENCY> constant
// - ARBLOST support included
// - supports multiple-byte writes and multiple-byte reads
//
// How To Test:
//
// 1) Download code to a 'F12x device that is connected to a SMBus slave.
// 2) Run the code:
//			a) The test will indicate proper communication with the slave by
//				toggling the LED on and off each time a value is sent and
//				received.
//			b) The best method to view the proper functionality is to run to
//				the indicated line of code in the TEST CODE section of main and
//				view the smbusrxbuff and smbustxbuff variable arrays in the
//				Watch Window.
//
//
// SMB0CN registers for reference
// BUSY			BUSY
// ENSMB		ENABLE
// STA			START FLAG
// STO			STOP FLAG
// SI			INTERRUPT PENDING FLAG
// AA			ASSERT/ACKNOWLEDGE FLAG
// SMBFTE		FREE TIMER ENABLE
// SMBTOE		TIMEOUT ENABLE
//
//

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------
bit SMB_BUSY;                          // Software flag to indicate when the
                                       // SMB_Read() or SMB_Write() functions
                                       // have claimed the SMBus

bit SMB_RW;                            // Software flag to indicate the
                                       // direction of the current transfer
//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------
#define  SMB_FREQUENCY  100000L			// Target SMBSCL clock rate
										// This example supports between 10kHz and 100kHz

#define  WRITE			0x00			// WRITE direction bit
#define  READ			0x01			// READ direction bit

#define  SMB_BUS_ERROR	0x00			// (all modes) BUS ERROR

// master states
#define  SMB_START		0x08			// (MT & MR) START transmitted
#define  SMB_RP_START	0x10			// (MT & MR) repeated START
#define  SMB_MTADDACK	0x18			// (MT) Slave address + W transmitted;
											//	 ACK received
#define  SMB_MTADDNAK	0x20			// (MT) Slave address + W transmitted;
											//	 NAK received
#define  SMB_MTDBACK	0x28			// (MT) data byte transmitted;
											//	 ACK rec'vd
#define  SMB_MTDBNAK	0x30			// (MT) data byte transmitted;
											//	 NAK rec'vd
#define  SMB_MTARBLOST	0x38			// (MT) arbitration lost
#define  SMB_MRADDACK	0x40			// (MR) Slave address + R transmitted;
											//	 ACK received
#define  SMB_MRADDNAK	0x48			// (MR) Slave address + R transmitted;
											//	 NAK received
#define  SMB_MRDBACK	0x50			// (MR) data byte rec'vd;
											//	 ACK transmitted
#define  SMB_MRDBNAK	0x58			// (MR) data byte rec'vd;
											//	 NAK transmitted

// slave states
#define  SMB_SROADACK	0x60			// (SR) SMB's own slave address + W
											//    rec'vd; ACK transmitted
#define  SMB_SRGADACK	0x70			// (SR) general call address rec'vd;
											//    ACK transmitted
#define  SMB_SRODBACK	0x80			// (SR) data byte received under own
											//    slave address; ACK returned
#define  SMB_SRODBNAK	0x88			// (SR) data byte received under own
											//    slave address; NACK returned
#define  SMB_SRGDBACK	0x90			// (SR) data byte received under general
											//    call address; ACK returned
#define  SMB_SRGDBNAK	0x98			// (SR) data byte received under general
											//    call address; NACK returned
#define  SMB_SRSTOP		0xA0			// (SR) STOP or repeated START received
											//    while addressed as a slave
#define  SMB_STOADACK	0xA8			// (ST) SMB's own slave address + R
											//    rec'vd; ACK transmitted
#define  SMB_STDBACK	0xB8			// (ST) data byte transmitted; ACK
											//    rec'ed
#define  SMB_STDBNAK	0xC0			// (ST) data byte transmitted; NACK
											//    rec'ed
#define  SMB_STDBLAST	0xC8			// (ST) last data byte transmitted
											//    (AA=0); ACK received
#define  SMB_SCLHIGHTO	0xD0			// (ST & SR) SCL clock high timer per
											//    SMB0CR timed out (FTE=1)
#define  SMB_IDLE		0xF8			// (all modes) Idle

//-----------------------------------------------------------------------------
// SMB_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// The SMBus peripheral is configured as follows:
// - SMBus enabled
// - Assert Acknowledge low (AA bit = 1b)
// - Free and SMBSCL low timeout detection enabled
//
void SMB_Init (void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = SMB0_PAGE;
//	SMB0CN = 0x07;						// Assert Acknowledge low (AA bit = 1b)
											// Enable SMBus Free timeout detect
											// Enable SMBSCL low timeout detect
	SMB0CN = 0x44;						// Assert Acknowledge low (AA bit = 1b)
											// Enable SMBus
											// Disable SMBus Free timeout detect

	SMB0CR = 236;						// see SMB0CR documentation for derivation of this number
										// 241 = 375 KHz does not work
										// 235 = 294 KHz works ** CLEAN **
										// 238 = 322 KHz works but not clean
										// 240 = 344 KHz works but not clean
	
										// NOTE: This is 400KHz from a 75MHz system clock rate
	SMB0CN |= 0x40;						// Enable SMBus
	SI = 0;								// Clear interrupt pending flag
	EIE1 |= 0x02;						// Enable the SMBus interrupt

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page detector
} // SMB_Init

//-----------------------------------------------------------------------------
// SMBus Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
//
// NOTE: This is implemented for variable length Master-Slave transmissions. The first byte in the
// transmission is the number of bytes of the payload. For example:
// 8,0,1,2,3,4,5,6,7 would be a valid tranmission. 8 = trans length followed by 8 data bytes.
//
// SMBus ISR state machine
// - Master only implementation - no slave or arbitration states defined
// - All incoming data is written to global array smbrxbuff
// - All outgoing data is read from global array smbtxbuff
//
void SMB_ISR (void) interrupt 7 {
	bit FAIL = 0;							// Used by the ISR to flag failed transfers

	// Status code for the SMBus (SMB0STA register)
	switch (SMB0STA) {
		// Master Transmitter/Receiver: START condition transmitted.
		// Load SMB0DAT with slave device address.
		case SMB_START:
		// Master Transmitter/Receiver: repeated START condition transmitted.
		// Load SMB0DAT with slave device address
		case SMB_RP_START:
			SMB0DAT = smbus.target;			// Load address of the slave
			smbus.data_ready = FALSE;		// clear semaphore
			SMB0DAT &= 0xFE;				// Clear the LSB of the address for the R/W bit
			SMB0DAT |= SMB_RW;				// Load R/W bit
			STA = 0;						// Manually clear STA bit

			smbus.rec_byte_counter = 0;		// Reset the counter
			smbus.sent_byte_counter = 0;	// Reset the counter
			break;

		// Master Transmitter: Slave address + WRITE transmitted.  ACK received.
		// For a READ: N/A
		//
		// For a WRITE: Send the first data byte to the slave.
		// For a WRITE: Send all data.  After the last data byte, send the stop bit.
		case SMB_MTADDACK:
		case SMB_MTDBACK:
			if (smbus.sent_byte_counter < smbus.txbytes) {
				// send data byte
				SMB0DAT = smbus.txbuff[smbus.sent_byte_counter];
				smbus.sent_byte_counter++;
			} // if
			else {
				STO = 1;					// Set STO to terminate transfer
				SMB_BUSY = 0;				// And free SMBus interface
			} // else
			break;

		// Master Transmitter: Slave address + WRITE transmitted.  NAK received.
		case SMB_MTADDNAK:
		// Master Transmitter: Data byte transmitted.  NAK received.
		case SMB_MTDBNAK:
			STA = 1;						// Restart transfer
		// Master Receiver: Slave address + READ transmitted.  NAK received.
		// Restart the transfer.
		case SMB_MRADDNAK:
			STA = 1;						// Restart transfer
			break;

		// Master Receiver: Data byte received.  ACK transmitted.
		// For a READ: receive each byte from the slave.  if this is the last
		//  byte, send a NAK and set the STOP bit.
		case SMB_MRADDACK:
		// Master Receiver: Data byte received.  ACK transmitted.
		// For a READ: receive each byte from the slave.  if this is the last
		//  byte, send a NAK and set the STOP bit.
		// For a WRITE: N/A
		case SMB_MRDBACK:
			if (0 != smbus.rec_byte_counter) {			// skip first byte = slave address			
				smbus.rxbuff[smbus.rec_byte_counter-1] = SMB0DAT; // Store received byte and increment
			} // if
			smbus.rec_byte_counter++;

			if (smbus.rec_byte_counter < smbus.rxbytes)
				AA = 1;						// Send ACK to indicate byte received
			else
				AA = 0;						// Send NAK to indicate one more byte in this transfer
			break;

		// Master Receiver: Data byte received.  NAK transmitted.
		// For a READ: Read operation has completed.  Read data register and send STOP.
		case SMB_MRDBNAK:
			smbus.rxbuff[smbus.rec_byte_counter-1] = SMB0DAT; // Store received byte
			smbus.rec_byte_counter++;
			smbus.data_ready = TRUE;
			STO = 1;
			SMB_BUSY = 0;
			AA = 1;							// Set AA for next transfer
			break;

		// Master Transmitter: Arbitration lost.
		case SMB_MTARBLOST:
			FAIL = 1;						// Indicate failed transfer and handle at end of ISR
			break;

		// Slave Receiver: Slave address + WRITE received.  ACK transmitted.
		// Fall through.
		case SMB_SROADACK:
		// Slave Receiver: General call address received.  ACK transmitted.
		case SMB_SRGADACK:
			smbus.data_ready = FALSE;			// clear semaphore
			smbus.sent_byte_counter = 0;        // Reinitialize the data counters
			smbus.rec_byte_counter = 0;
			break;

		// Slave Receiver: Data byte received after addressed by general
		// call address + WRITE.
		// ACK transmitted.  Fall through.
		case SMB_SRGDBACK:
		// Slave Receiver: Data byte received after addressed by own slave address + WRITE.
		// ACK transmitted.
		// Receive each byte from the master.
		case SMB_SRODBACK:
			if (0 == smbus.rec_byte_counter) {	// first byte is number of characters in transmission
				smbus.rxbytes = SMB0DAT;
				smbus.data_ready = FALSE;
			} // if
			else
				smbus.rxbuff[smbus.rec_byte_counter-1] = SMB0DAT; // Store received byte
			smbus.rec_byte_counter++;
			
			if (smbus.rec_byte_counter > smbus.rxbytes) {
				smbus.data_ready = TRUE;	// Buffer full so indicate received
				smbus.rec_byte_counter = 0;	// set for next time
			} // if
			AA = 1;							// Send ACK to indicate byte received
			break;

		// Slave Receiver: Data byte received while addressed as slave.
		// NAK transmitted. Should not occur since AA will not be cleared
		// as slave.  Fall through to next state.
		case SMB_SRODBNAK:
		// Slave Receiver: Data byte received while addressed by general call.
		// NAK transmitted.
		// Should not occur since AA will not be cleared as slave.
		case SMB_SRGDBNAK:
			AA = 1;
			break;

		// Slave Receiver: STOP or repeated START received while addressed as slave.
		case SMB_SRSTOP:
			break;

		// Slave Transmitter: Own slave address + READ received.  ACK transmitted.
		// Load SMB0DAT with data to be output.
		case SMB_STOADACK:
		// Slave Transmitter: Data byte transmitted.  ACK received.
		// Send each requested byte to the master.
		case SMB_STDBACK:
			SMB0DAT = smbus.txbuff[smbus.sent_byte_counter++];
//			if (smbus.sent_byte_counter <= smbus.txbytes)
				// send data byte
//				SMB0DAT = smbus.txbuff[smbus.sent_byte_counter++];
			break;

		// Slave Transmitter: Data byte transmitted.  NACK received.
		// Resend the previous byte.
		case SMB_STDBNAK:
			SMB0DAT = smbus.txbuff[smbus.sent_byte_counter-1];
			break;

		// Slave Transmitter: Last data byte transmitted.  ACK received.
		// No action necessary.
		case SMB_STDBLAST:
			break;
		 
		// All other status codes invalid.  Reset communication.
		default:
			FAIL = 1;
			break;
	} // switch

	if (FAIL) {								// If the transfer failed,
		SMB0CN &= ~0x40;					// Reset communication
		SMB0CN |= 0x40;
		STA = 0;
		STO = 0;
		AA = 0;

		SMB_BUSY = 0;						// Free SMBus
		FAIL = 0;
	} // if

	SI = 0;									// Clear interrupt flag
} // SMB_ISR

//-----------------------------------------------------------------------------
// SMB_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// Writes a single byte to the slave with address specified by the <TARGET>
// variable.
// Calling sequence:
// 1) Write target slave address to the <TARGET> variable
// 2) Write outgoing data to the <smbustxbuff> array
// 3) Call SMB_Write()
//
void SMB_Write(void) {
	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	SFRPAGE = SMB0_PAGE;
	
	ENSMB = 1;						// Enable the bus
	while (SMB_BUSY);				// Wait for SMBus to be free
	SMB_BUSY = 1;					// Claim SMBus (set to busy)
	SMB_RW = 0;						// Mark this transfer as a WRITE
	SMB0ADR = smbus.target;			// Set the address of unit to write to
	STA = 1;						// Start transfer

	SFRPAGE = SFRPAGE_SAVE;			// Restore SFR page detector
} // SMB_Write

//-----------------------------------------------------------------------------
// SMB_Read
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// Reads a single byte from the slave with address specified by the <TARGET>
// variable.
// Calling sequence:
// 1) Write target slave address to the <TARGET> variable
// 2) Call SMB_Write()
// 3) Read input data from <smbusrxbuff> array
//
void SMB_Read (void) {
	u08 SFRPAGE_SAVE = SFRPAGE;		  // Save Current SFR page

	SFRPAGE = SMB0_PAGE;

	ENSMB = 1;						// Enable the bus
	while (SMB_BUSY);				// wait for bus to be free.
	SMB_BUSY = 1;					// claim SMBus (set to busy)
	SMB_RW = 1;						// mark this transfer as a READ
	SMB0ADR = smbus.target;			// Set the address of unit to read from
	STA = 1;						// start transfer

	while (SMB_BUSY);				// wait for transfer to complete

	SFRPAGE = SFRPAGE_SAVE;			// restore SFR page
} // SMB_Read
