// Part of traqmate.c
// 3/10/2004
// Author: BBoyer
//
// This file contains the crc calculation routines
//

/* 
	The following function calculates a 16 bit CRC on the 
	data pointed to by and over the number of bytes specified.
	The value of the CRC is returned.
*/

extern code crc16_table[];

/* crc_calc() -- calculate cumulative crc-16 for buffer */
unsigned int crc16(unsigned char *buf, unsigned int nbytes)
{
	unsigned char *p, *lim;
	unsigned int crc;

	p = (unsigned char *)buf;
	lim = p + nbytes;
	crc = 0;
	while (p < lim) {
		crc = (crc >> 8 ) ^ crc16_table[(crc & 0xFF) ^ *p++];
	}
	return crc;
}
