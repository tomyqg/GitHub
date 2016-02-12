// Part of traqmate.c
// 3/10/2004
// Author: BLBoyer
//
// This file contains the communications protocol definitions.
//
/* the following is for any CRC-32 Calculations */
#if !defined(MESSAGE_H)
#define MESSAGE_H

s16 SendMessage(u08, u08, u08, u16 );
s16 Verify_CRC(u08*, u16) ;
s16 SendNAK(u08, u08);
s16 Parse_Message(u08, u08 *);

#define SOH 0x16

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

// communication timeout
#define CTS_COUNTDOWN	100			// MAX = 256!!, 100 / 160 = .625 sec
// #define CTS_COUNTDOWN	80			// MAX = 256!!, 80 / 160 = .50 sec -- changed for V2.10

// unit ids
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
#define NUMUNITS		DISPLAY_UNIT		// total number of units

#endif