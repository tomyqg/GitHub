/*! \file tsip.h \brief TSIP (Trimble Standard Interface Protocol) function library. */   
//*****************************************************************************           
//                                                                                        
// File Name    : 'tsip.h'                                                                
// Title        : TSIP (Trimble Standard Interface Protocol) function library             
// Author       : Pascal Stang - Copyright (C) 2002
// Created      : 2002.08.27
// Revised      : 2002.08.27
// Version      : 0.1
// Target MCU   : Atmel AVR Series
// Editor Tabs  : 4
//
// NOTE: This code is currently below version 1.0, and therefore is considered
// to be lacking in some functionality or documentation, or may not be fully
// tested.  Nonetheless, you can expect most functions to work.
//
// This code is distributed under the GNU Public License
//      which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#ifndef TSIP_H
#define TSIP_H

// constants/macros/typdefs
// packet delimiters
#define DLE                 0x10
#define ETX                 0x03

// Set I/O Options bit positions
// byte 0
#define POS_XYZ_ECEF        0x01   // outputs 0x42 and 0x83 packets
#define POS_LLA             0x02   // outputs 0x4A and 0x84 packets
#define POS_ALT             0x04   // outputs 0x4A/0x84 and 0x8F-17/0x8F-18
#define ALT_REF_MSL         0x08   // bit cleared = HAE Reference datum
#define POS_DBL_PRECISION   0x10   // bit cleared = single precision
#define SUPER_PACKETS       0x20   // 0x8F-17,0x8F-18,0x8F-20,0x8F-23
// byte 1
#define VEL_ECEF            0   // outputs 0x43
#define VEL_ENU             1   // outputs 0x56
// byte 2
#define TIME_UTC            0   // 0/1 time format GPS/UTC
// byte 3
#define RAWDATA             0   // outputs 0x5A packets 
#define RAWDATA_FILTER      1   // 0/1 raw data unfiltered/filtered 
#define SIGNAL_DBHZ         3   // 0/1 signal strength in AMU/dBHz

// command packets
#define CMD_CCLROSC		0x1D	/* clear oscillator offset */
#define CMD_CCLRRST		0x1E	/* clear battery backup and RESET */
#define CMD_CVERSION	0x1F	/* return software version */
#define CMD_CALMANAC	0x20	/* almanac */
#define CMD_CCURTIME	0x21	/* current time */
#define CMD_CMODESEL	0x22	/* mode select (2-d, 3-D, auto) */
#define CMD_CINITPOS	0x23	/* initial position */
#define	CMD_CRECVPOS	0x24	/* receiver position fix mode */
#define CMD_CRESET		0x25	/* soft reset & selftest */
#define CMD_CRECVHEALTH	0x26	/* receiver health */
#define CMD_CSIGNALLV	0x27	/* signal levels */
#define CMD_CMESSAGE	0x28	/* GPS system message */
#define CMD_CALMAHEALTH	0x29	/* almanac healt page */
#define CMD_C2DALTITUDE	0x2A	/* altitude for 2-D mode */
#define CMD_CINITPOSLLA	0x2B	/* initial position LLA */
#define CMD_COPERPARAM	0x2C	/* operating parameters */
#define CMD_COSCOFFSET	0x2D	/* oscillator offset */
#define CMD_CSETGPSTIME	0x2E	/* set GPS time */
#define CMD_CUTCPARAM	0x2F	/* UTC parameters */
#define CMD_CACCPOSXYZ	0x31	/* accurate initial position (XYZ/ECEF) */
#define CMD_CACCPOS		0x32	/* accurate initial position */
#define CMD_CANALOGDIG	0x33	/* analog to digital */
#define CMD_CSAT1SAT	0x34	/* satellite for 1-Sat mode */
#define CMD_CIOOPTIONS	0x35	/* I/O options */
#define CMD_CVELOCAID	0x36	/* velocity aiding of acquisition */
#define CMD_CSTATLSTPOS	0x37	/* status and values of last pos. and vel. */
#define CMD_CLOADSSATDT	0x38	/* load satellite system data */
#define CMD_CSATDISABLE	0x39	/* satellite disable */
#define CMD_CLASTRAW	0x3A	/* last raw measurement */
#define CMD_CSTATSATEPH	0x3B	/* satellite ephemeris status */
#define CMD_CSTATTRACK	0x3C	/* tracking status */
#define CMD_CCHANADGPS	0x3D	/* configure channel A for differential GPS */
#define CMD_CADDITFIX	0x3E	/* additional fix data */
#define CMD_CDGPSFIXMD	0x62	/* set/request differential GPS position fix mode */
#define CMD_CDGPSCORR	0x65	/* differential correction status */
#define CMD_CPOSFILT	0x71	/* position filter parameters */
#define CMD_CHEIGHTFILT	0x73	/* height filter control */
#define CMD_CHIGH8CNT	0x75	/* high-8 (best 4) / high-6 (overdetermined) control */
#define CMD_CMAXDGPSCOR	0x77	/* maximum rate of DGPS corrections */
#define CMD_CSUPER		0x8E	/* super packet */

#define CMD_RDATAA		0x3D	/* data channel A configuration:trimble_channelA:RO */
#define CMD_RALMANAC	0x40	/* almanac data for sat:gps_almanac:RO */
#define CMD_RCURTIME	0x41	/* GPS time:gps_time:RO */
#define CMD_RSPOSXYZ	0x42	/* single precision XYZ position:gps_position(XYZ):RO|DEF */
#define CMD_RVELOXYZ	0x43	/* velocity fix (XYZ ECEF):gps_velocity(XYZ):RO|DEF */
#define	CMD_RBEST4		0x44	/* best 4 satellite selection:trimble_best4:RO|DEF */
#define CMD_RVERSION	0x45	/* software version:trimble_version:RO|DEF */
#define CMD_RRECVHEALTH	0x46	/* receiver health:trimble_receiver_health:RO|DEF */
#define CMD_RSIGNALLV	0x47	/* signal levels of all satellites:trimble_signal_levels:RO */
#define CMD_RMESSAGE	0x48	/* GPS system message:gps-message:RO|DEF */
#define CMD_RALMAHEALTH	0x49	/* almanac health page for all satellites:gps_almanac_health:RO */
#define CMD_RSLLAPOS	0x4A	/* single LLA position:gps_position(LLA):RO|DEF */
#define CMD_RMACHSTAT	0x4B	/* machine code / status:trimble_status:RO|DEF */
#define CMD_ROPERPARAM	0x4C	/* operating parameters:trimble_opparam:RO */
#define CMD_ROSCOFFSET	0x4D	/* oscillator offset:trimble_oscoffset:RO */
#define CMD_RSETGPSTIME	0x4E	/* response to set GPS time:trimble_setgpstime:RO */
#define CMD_RUTCPARAM	0x4F	/* UTC parameters:gps_utc_correction:RO|DEF */
#define CMD_RANALOGDIG	0x53	/* analog to digital:trimble_analogdigital:RO */
#define CMD_RSAT1BIAS	0x54	/* one-satellite bias & bias rate:trimble_sat1bias:RO */
#define CMD_RIOOPTIONS	0x55	/* I/O options:trimble_iooptions:RO */
#define CMD_RVELOCFIX	0x56	/* velocity fix (ENU):trimble_velocfix */
#define CMD_RSTATLSTFIX	0x57	/* status and values of last pos. and vel.:trimble_status_lastpos:RO */
#define CMD_RLOADSSATDT	0x58	/* response to load satellite system data:trimble_loaddata:RO */
#define CMD_RSATDISABLE	0x59	/* satellite disable:trimble_satdisble:RO */
#define CMD_RLASTRAW	0x5A	/* last raw measurement:trimble_lastraw:RO */
#define CMD_RSTATSATEPH	0x5B	/* satellite ephemeris status:trimble_ephstatus:RO */
#define CMD_RSTATTRACK	0x5C	/* tracking status:trimble_tracking_status:RO|DEF */
#define CMD_RADDITFIX	0x5E	/* additional fix data:trimble_addfix:RO */
#define CMD_RALLINVIEW	0x6D	/* all in view satellite selection:trimble_satview:RO|DEF */
#define CMD_RPOSFILT	0x72	/* position filter parameters:trimble_posfilt:RO */
#define CMD_RHEIGHTFILT	0x74	/* height filter control:trimble_heightfilt:RO */
#define CMD_RHIGH8CNT	0x76	/* high-8 (best 4) / high-6 (overdetermined) control:trimble_high8control:RO */
#define CMD_RMAXAGE		0x78	/* DC MaxAge:trimble_dgpsmaxage:RO */
#define CMD_RDGPSFIX	0x82	/* differential position fix mode:trimble_dgpsfixmode:RO */
#define CMD_RDOUBLEXYZ	0x83	/* double precision XYZ:gps_position_ext(XYZ):RO|DEF */
#define CMD_RDOUBLELLA	0x84	/* double precision LLA:gps_position_ext(LLA):RO|DEF */
#define CMD_RDGPSSTAT	0x85	/* differential correction status:trimble_dgpsstatus:RO */
#define CMD_RSUPER		0x8F	/* super packet::0 */
#define CMD_PORTSETUP	0xBC	/* used to setup baud rates, etc. */

#define CMD_SUPERLARGE	0x20	/* large economy size super packet */
#define CMD_SUPERSMALL	0x23	/* compact size super packet */

#endif

