/* ubx.h */   
//*****************************************************************************           
//                                                                                        
// These are the definitions for the UBX Messaging Protocol
// GAStephens 11-15-2004
//
//*****************************************************************************

#ifndef UBX_H
#define UBX_H

#define UBXPAYLOAD		6			// payload starts on 6th byte

// constants
#define SYNCH1				0xB5	// first message synch byte
#define SYNCH2				0x62	// second message synch byte

// class definitions
#define NAV					0x01	// navigational
#define RXM					0x02	// reciever manager
#define INF					0x04	// informative
#define UBXACK				0x05	// ack - nak
#define CFG					0x06	// configuration
#define UPD					0x09	// sw updates
#define MON					0x0A	// monitor
#define AID					0x0B	// navigation aiding
#define USR					0x4F	// mask, actually 0x4x, user messages
#define TIM					0x0D	// timing

// commands
#define NAVPOSECEF			0x0101	// position in ECEF format
#define NAVPOSLLH			0x0102	// position in LLH format
#define NAVPOSUTM			0x0108	// position in UTM format
#define NAVDOP				0x0104	// dillution of precision
#define NAVSTATUS			0x0103	// navigation status
#define NAVSOL				0x0106	// multipart message, solution, time, accuracy
#define NAVVELECEF			0x0111	// velocity solution in ECEF
#define NAVVELNED			0x0112	// velocity solution in NED
#define NAVTIMEGPS			0x0120	// GPS time
#define NAVTIMEUTC			0x0121	// UTC time
#define NAVCLOCK			0x0122	// clock solution
#define NAVSVINFO			0x0130	// space vehicle information
#define NAVDGPS				0x0131	// DGPS corrected solution
#define NAVSBAS				0x0132	// status of SBAS subsystem
#define NAVEKFSTATUS		0x0140	// dead reckoning status info

#define ACKACK				0x0501	// acknowledgement
#define ACKNAK				0x0500	// negative acknowledgement

#define CFGPRT				0x0600	// get/set port configuration
#define CFGMSG				0x0601	// get/set message configuration
#define CFGNMEA				0x0617	// get/set NMEA protocol config
#define CFGRATE				0x0608	// get/set navigational solution rate
#define CFGCFG				0x0609	// clear, save, and load configurations
#define CFGTP				0x0607	// get/set timepulse configuration
#define CFGNAV				0x0603	// get/set nav engine config
#define CFGDAT				0x0606	// get/set datum
#define CFGINF				0x0602	// get/set informational message config
#define CFGRST				0x0604	// reset receiver
#define CFGRXM				0x0611	// get/set receiver sensitivity config
#define CFGANT				0x0613	// get/set antenna control settings
#define CFGFXN				0x060E	// get/set FixNow config
#define CFGSBAS				0x0616	// get/set SBAS config
#define CFGTM				0x0610	// get/set timemark config
#define CFGEKF				0x0612	// get/set dead reckoning config
#define CFGNAV2				0x061A	// get/set nav engine config (antaris 4)
#define CFGNAVX5			0x0623	// get/set nav engine config, expert (ublox 5)
#define CFGNAV5				0x0624	// get/set nav engine modes (ublox 5)
#define CFGUDOC				0x0625	// undocumented message for Ublox5 V4.00	

#define MONEXCEPT			0x0A05	// exception dump
#define MONHW				0x0A09	// hw status
#define MONIO				0x0A02	// I/O subsystem status
#define MONIPC				0x0A03	// IPC sybsystem status
#define MONMSGPP			0x0A06	// message parse/process status
#define MONRXBUF			0x0A07	// receive buffer status
#define MONSCHD				0x0A01	// system scheduler status
#define MONTXBUF			0x0A08	// transmit buffer status
#define MONVER				0x0A04	// receiver and sw version

#define INFERROR			0x0400	// ascii error string
#define INFWARNING			0x0401	// ascii warning string
#define INFNOTICE			0x0402	// ascii information string
#define INFTEST				0x0403	// ascii test output string
#define INFDEBUG			0x0404	// ascii string denoting debug output

#endif

