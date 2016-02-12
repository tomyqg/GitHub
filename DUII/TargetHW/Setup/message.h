// Part of traqmate.c
// 3/10/2004
// Author: BLBoyer
//
// This file contains the communications protocol definitions.
//
/* the following is for any CRC-32 Calculations */
#if !defined(MESSAGE_H)
#define MESSAGE_H

#ifndef _WINDOWS
s16 SendMessage(u08, u08, u08, u16 );
s16 Verify_CRC(u08*, u16) ;
s16 SendNAK(u08, u08);
s16 Parse_Message(u08, u08 *);
#endif

#define SOH 0x16

// message byte locations
#define	START				0
#define	SOURCE				1
#define	DESTINATION			2
#define	UPPER_BYTE_COUNT	3
#define LOWER_BYTE_COUNT	4
#define OPCODE				5
#define CHECKSUM			6
#define DATA_START			7

#define MSG_WRAPPER	(DATA_START + 2)	// length of message wrapper including CRC

enum states {
	LOOKING_FOR_SYNC = 0x01,
	POSSIBLE_SYNC,
	DO_CHECKSUM,
	FOUND_SYNC,
	MESSAGE_COMPLETE } ;

// communication timeout
#define CTS_COUNTDOWN	620		// 0.62 seconds to respond to message

// unit ids
#define 	SELF_UNIT				0
#define		PC_UNIT					1
#define 	SENSOR_UNIT				2
#define 	DISPLAY_UNIT			3
#define 	SMBUS_SLAVE_1			4
#define 	SMBUS_SLAVE_2			5
#define 	SMBUS_SLAVE_3			6
#define 	SMBUS_SLAVE_4			7
#define 	ANY_UNIT				0xFF

#if 0
enum units {
	SELF,
	PC,								// 1
	SENSOR_UNIT,					// 2
	DISPLAY_UNIT,					// 3
	SMBUS_SLAVE_1,					// 4
	SMBUS_SLAVE_2,					// 5
	SMBUS_SLAVE_3,					// 6
	SMBUS_SLAVE_4,					// 7
	ANY_UNIT = 0xFF
};
#endif

#define NUMUNITS		DISPLAY_UNIT		// total number of units

#endif