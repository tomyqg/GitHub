// definitions for task resource profiler
// These values will be displayed on a new test screen enabled by "testmode" in root of sd card.
// GAStephens 2-22-2012

// task numbers
#define MAINTASK_NUM		0
#define SDCARDRWTASK_NUM	1
#define COMMTASK_NUM		2
#define LAPTIMERTASK_NUM	3
#define TQMWRITETASK_NUM	4
#define LEDHANDLERTASK_NUM	5
#define SDCARDMGRTASK_NUM	6
// Note NUMBER_OF_TASKS is also defined in DUII.h for use on the PC side for profiler simulation
// Did not want to include Profiler.h on PC side as this is a hardware file.
#define NUMBER_OF_TASKS		7

// function ids -- future implementation
#define MESSAGEPARSER		0
#define SWEEPTACH			1
#define DRIVESCREEN			2
#define DOUBLEANALOG		3
#define NUMBER_OF_FUNCTIONS	4

#include "RTOS.h"
#include "FS.h"
#include "commondefs.h"

// substitute function definitions
void PF_OS_Delay( int, int);
char PF_OS_WaitEventTimed( int, unsigned char, OS_TIME );
char PF_OS_WaitSingleEventTimed( int, unsigned char, OS_TIME );
char PF_OS_WaitEvent( int, unsigned char );
void PF_OS_Use( int, OS_RSEMA* );
void PF_OS_Unuse( int, OS_RSEMA* );
void PF_OS_PutMail( int, OS_MAILBOX*, void* );
void PF_OS_GetMail( int, OS_MAILBOX*, void* );
void PF_OS_SetPriority(int, OS_TASK*, unsigned char );
void PF_OS_WakeTask (int, OS_TASK* );
void PF_OS_Yield( int );
void PF_OS_SignalEvent( int, char, OS_TASK* );
int PF_OS_Q_GetPtr( int, OS_Q*, void** );
U32 PF_FS_Write(int, FS_FILE *, const void *, U32);
U32 PF_FS_Read(int, FS_FILE *, void *, U32);

// when PROFILER is defined, every call to a system blocking event records the start and stop times for the task
//
#ifndef PROFILER_C			// don't do substitution within profiler.c because it is calling the real function
#define OS_Delay(a) PF_OS_Delay(THIS_TASK, (a))
#define OS_WaitEventTimed(a,b)	PF_OS_WaitEventTimed(THIS_TASK, (a), (b))
#define OS_WaitSingleEventTimed(a,b)	PF_OS_WaitSingleEventTimed(THIS_TASK, (a), (b))
#define OS_WaitEvent(a)	PF_OS_WaitEvent(THIS_TASK, (a))
#define OS_Use(a) PF_OS_Use(THIS_TASK, (a))
#define OS_Unuse(a) PF_OS_Unuse(THIS_TASK, (a))
#define OS_PutMail(a, b) PF_OS_PutMail(THIS_TASK, (a), (b))
#define OS_GetMail(a, b) PF_OS_GetMail(THIS_TASK, (a), (b))
#define OS_SetPriority(a, b) PF_OS_SetPriority(THIS_TASK, (a), (b))
#define OS_WakeTask(a) PF_OS_WakeTask(THIS_TASK, (a))
#define OS_Yield() PF_OS_Yield(THIS_TASK)
#define OS_SignalEvent( a, b) PF_OS_SignalEvent(THIS_TASK, (a), (b))
#define OS_Q_GetPtr( a, b) PF_OS_Q_GetPtr(THIS_TASK, (a), (b))
#define FS_Write(a, b, c) PF_FS_Write(THIS_TASK, (a), (b), (c))
#define FS_Read(a, b, c) PF_FS_Read(THIS_TASK, (a), (b), (c))
#endif // PROFILER_C

