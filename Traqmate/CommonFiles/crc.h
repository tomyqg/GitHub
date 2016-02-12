// Part of traqmate.c
// 3/10/2004
// Author: BLBoyer
//
// This file contains the definitions for crc calculations

/* the following is for any CRC-16 Calculations */
#if !defined(CRC16)
#define CRC16
#define CRC16_POLY 0x4003   /* AUTODIN II, Ethernet, & FDDI */
	static xdata u16 crc16_table[256];
	void init_crc16(void) ;
	unsigned int crc16(unsigned char *buf, unsigned nbytes);
#endif

/* the following is for any CRC-32 Calculations */
#if !defined(CRC32)
#define CRC32
#define CRC32_POLY 0x04c11db7   /* AUTODIN II, Ethernet, & FDDI */

/* Uncomment the following line to compile the CRC32 code */
/* #define DO_CRC32 */

	static xdata u32 crc32_table[256] ;
	void init_crc32(void);
	unsigned long crc32(unsigned char *, int) ;
#endif