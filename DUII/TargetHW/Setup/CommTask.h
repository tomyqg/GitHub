#ifndef COMM_TASK_H
#define COMM_TASK_H
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "commondefs.h"
#include "tmtypes.h"
#include "RTOS.h"
#include "FS.h"

void CommTask(void);

#define NUM_RX_BUFF			3			// triple buffer

// macros
#define WAIT_ACK(x)	while(0 != outstanding_msg[x-1]) OS_Delay(10)

// function declarations
extern void App_putstring(void *, unsigned int);
extern void XmitUART(comporthandle *, u16 );
void App_Comm_Setup( void );
u08 getModeSamp( void );
u08 getIoData( void );
void BufferWrite (FS_FILE *, u08 *, int , int );

// accelerometer stuff
typedef struct {
	float min;
	float zero;
	float max;
} accelscaling;

// commands for BufferWrite()
#define BUFFER_INIT		0
#define BUFFER_WRITE	1
#define BUFFER_FLUSH	2

// variables

#ifdef COMM_MAIN
comporthandle com[NUMPORTS];					// handles to com ports
u08 msgrxbuff[NUM_RX_BUFF][MSGSIZE];			// inbound msg buffers
u08 msgtxbuff[MSGSIZE];							// outbound msg buffers
const s08 msgrouting[NUMUNITS+1] = {DISPLAY_UNIT, 0, 0, -1};	// what port to route messages to
//	0 place = this units id, 1 unit = PC = COM1, 2 unit = SU = COM0, 3 unit = DISPLAY = self = -1
OS_TIMER COMMCTSTIMERS[NUMUNITS-1];				// communication timers
u32 COMMCTS[NUMUNITS-1];						// timeout value, non-zero when waiting for response, 0 when clear to send, for SU, PC
// *** GAS TEST
u08 outstanding_msg[NUMUNITS+2] = { 0 };		// lists which messages are outstanding from each device, +2 eliminates crash
scratchpadtype scratchpad;						// structure to store config data from SU
BOOL gotscratchpad = FALSE;						// TRUE when scratchpad received from SU
TQMSampType10 tqmsample;						// one full sample for the tqm file
accelscaling accelx, accely, accelz;			// accelerometer calibration info
pdrlancstatustype pdrlancstatus = UNKNOWN;		// what mode is chasecam pdr or lanc device is in
unsigned char firstpulse = TRUE;				// used to get synched up
u08 iocollect;									// which io points are enabled
u08 iobytesenabled;								// total number of iopoints enabled
tqmheadertype tqmheader;						// image of tqm tile header for fast access
FS_FILE *sessionFile = NULL;

#else
extern comporthandle com[NUMPORTS];				// handles to com ports
extern u08 msgrxbuff[NUM_RX_BUFF][MSGSIZE];		// inbound msg buffers
extern u08 msgtxbuff[MSGSIZE];					// outbound msg buffers
extern const s08 msgrouting[NUMUNITS+1];		// what port to route messages to
extern OS_TIMER COMMCTSTIMERS[NUMUNITS-1];		// communication timers
extern u32 COMMCTS[NUMUNITS-1];					// timeout value, non-zero when waiting for response, 0 when clear to send, for SU, PC
extern u08 outstanding_msg[NUMUNITS+2];			// lists which messages are outstanding from each device
extern scratchpadtype scratchpad;				// structure to store config data from SU
extern BOOL gotscratchpad;						// TRUE when scratchpad received from SU
extern TQMSampType10 tqmsample;					// one full sample for the tqm file
extern accelscaling accelx, accely, accelz;		// accelerometer calibration info
extern pdrlancstatustype pdrlancstatus;			// what mode is chasecam pdr or lanc device is in
extern unsigned char firstpulse;				// used to get synched up
extern u08 iocollect;							// which io points are enabled
extern u08 iobytesenabled;						// total number of iopoints enabled
extern tqmheadertype tqmheader;					// image of tqm tile header for fast access
extern FS_FILE *sessionFile;
#ifdef __cplusplus
}
#endif

#undef COMM_TASK_H
#endif // ifndef

