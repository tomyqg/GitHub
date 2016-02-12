// Part of traqmate.c
// 3/10/2004
// Author: BLBoyer
//
// This file contains the communications protocol opcodes.
//

#if !defined(OPCODE_H)
#define OPCODE_H

// message opcodes ACK/NAK
#define ACK 0x06
#define NAK 0x15

// NAK reasons for failure
#define BAD_CHKSUM				0x01 	// may not be used - could be not valid header
#define UNKNOWN_FUNCTION		0x02 	// opcode not assigned to function
#define BAD_CRC					0x04 	// CRC did not match sent value
#define FUNCTION_FAILED			0x08 	// function valid - but failed
#define FUNCTION_UNIMPLEMENTED  0x10	// function place holder, but not yet ready

#define MSG_TIMEOUT				0x80

// Normal operation Opcodes
#define WHO_ARE_YOU				0x10	// returns an ACK with no data
										// can be used with 0xFF as a dest address.
										// lets device figure out direct connections
#define READ_SEGMENT			0x90	// get specified segment data
										// 2 bytes data - integer segment number
#define WRITE_SEGMENT			0x98	// write data to specified segment
										// 2 bytes integer segment number + segment data
#define ERASE_SESSION			0x40	// erases a complete session
										// 1 byte data - segment number
#define ERASE_ALL				0x41	// erases the entire dataflash including session headers
#define GPS_DATA				0x22	// contains the latest GPS position data
										// 24 bytes data - formatted gpstype
#define GPS_PULSE				0x27	// notifies of GPS pulse received
										// 1 byte data = number of satellites in use
#define ACCEL_DATA				0x92	// contains the latest accelerometer data
										// 3 bytes data - formatted acceltype
#define MODE_CHANGE				0x91	// changes operating mode of the unit
										// 1 byte data - formatted tmstatetype
#define SW_DOWNLOAD				0x50	// part of new software being downloaded
#define SW_REPROGRAM			0x51	// instructs machine to change to new SW


//JFP
#define ENTER_DOWNLOAD			0xF0    // enter download mode
#define DOWNLOAD_COMPLETE		0xF1    // download complete
#define CANCEL_DOWNLOAD			0xF2	// cancel download
//JFP



// High Level Test Commands
#define READ_GPS_DATA			0x20	// read GPS record
#define ERASE_SEGMENT_RANGE		0x94	// erase specified segment(s)
										// 2 byte integer initial segment, 2 byte ending segment
#define READ_GPS_STATUS			0x26	// gets the status word for GPS unit
#define READ_ACCEL				0x21	// read Accelerometer AtoDs
#define WRITE_DISPLAY_TEXT		0x72	// writes the attached text to display
										// 0 terminated string
// Low Level Test Commands
#define READ_ADC				0x23	// general purpose AtoD read
#define SET_ADC_GAIN			0x26	// sets gain for a particular ADC channel
#define READ_DIGIO				0x24    // read specified I/O port
#define WRITE_DIGIO				0x25    // write data to port address indicated

// Factory Calibration and Testing
#define READ_UNIT_INFO			0x70	// gets the factory calibration and serno data
#define WRITE_UNIT_INFO			0x71	// writes the factory calibration and serno data
										// 128 byte unit information - formatted scratchpadtype
#endif 	// the end of the header file