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
#define UNIT_BUSY				0x20	// unit cannot perform request because it is busy

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
										// 1 byte data - session number
#define ERASE_ALL				0x41	// erases the entire dataflash including session headers
#define ERASE_ALL_SESS			0x42	// erases the entire dataflash except session headers and user config data
#define PACK_SESSIONS			0x43	// packs all sessions by moving down in memory (SLOW)

#define GPS_DATA				0x22	// contains the latest GPS position data
										// 24 bytes data - formatted gpstype
#define GPS_PULSE				0x27	// notifies of GPS pulse received
										// 1 byte data = number of satellites in use
#define ACCEL_DATA				0x92	// contains the latest accelerometer data and data i/o
										// 10 bytes data - 3 bytes formatted acceltype, DI, A0, A1, A2, A3, F0, F1
#define MODE_CHANGE				0x91	// changes operating mode of the unit
										// 3 bytes data - formatted tmstatetype, sampling rate, io data selection

#define CAM_STATUS				0x29	// status of camera

#define POWER_DOWN				0x93	// notification of power shutdown
#define AUTO_ON					0x94	// sets AutoOn status in SU2
										// 1 bytes data - 1 = AutoOn, 0 = Not AutoOn

#define SW_DOWNLOAD				0x50	// part of new software being downloaded
#define SW_REPROGRAM			0x51	// instructs machine to change to new SW

#define SERIAL_PASSTHRU			0x55	// cause SU to pass serial from COM0 to COM1
										// must power cycle to exit this mode
#define SIMULATE_GPS			0x56	// cause SU to simulate GPS lock or disable simulation

// High Level Test Commands
#define READ_GPS_DATA			0x20	// read GPS record
#define ERASE_SEGMENT_RANGE		0x94	// erase specified segment(s)
										// 2 byte integer initial segment, 2 byte ending segment
#define READ_GPS_STATUS			0x26	// gets the status word for GPS unit
#define READ_ACCEL				0x21	// read Accelerometer AtoDs
#define WRITE_DISPLAY_TEXT		0x72	// writes the attached text to display
										// 0 terminated string
#define WRITE_TEXT				0x73	// writes the attached text to PC screen
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
#define RESET_UNIT				0x30	// causes unit to reset
#define CALIBRATE				0x31	// puts unit into calibration mode (SU only)
#define INPUTTEST				0x32	// puts unit into input test mode (SU only)
#define OUTPUTTEST				0x33	// puts unit into output test mode (SU only)
#define DATAFLASHTEST			0x34	// puts unit into dataflash test mode
#define ADTEST					0x35	// factory tests a/d inputs, pass/fail
#define DIGIOTEST				0x36	// factory tests digital io, pass/fail
#define AUTOONTEST				0x37	// factory tests su2 autoon circuitry, pass/fail
#define DATAFLASHTEST2			0x38	// factory tests dataflash, pass/fail
#define SMBMASTERTEST			0x39	// factory tests SMBus (I2C) serial interface, pass/fail
#define SMBSLAVETEST			0x3A	// makes SU2 into slave for use in SMB test

#endif 	// the end of the header file