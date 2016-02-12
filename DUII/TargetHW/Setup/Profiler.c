#define PROFILER_C

// definitions for task resource profiler
// These values will be displayed on a new test screen enabled by "testmode" in root of sd card.
// GAStephens 2-22-2012

#include "profiler.h"
#include "DUII.h"

void PF_Calculate( int taskNum );
void PF_Increment( int taskNum );
	
void PF_OS_Delay( int taskNum, int ms) {
	PF_Calculate( taskNum );		
	OS_Delay( ms );
	PF_Increment( taskNum );
}; // PF_OS_Delay

char PF_OS_WaitEventTimed( int taskNum, unsigned char EventMask, OS_TIME TimeOut) {
	char returnVal;
	
	PF_Calculate( taskNum );	
	returnVal = OS_WaitEventTimed( EventMask, TimeOut );
	PF_Increment( taskNum );
	return (returnVal);
}; // PF_OS_WaitEventTimed

char PF_OS_WaitSingleEventTimed( int taskNum, unsigned char EventMask, OS_TIME TimeOut) {
	char returnVal;
	
	PF_Calculate( taskNum );	
	returnVal = OS_WaitSingleEventTimed( EventMask, TimeOut );
	PF_Increment( taskNum );
	return (returnVal);
}; // PF_OS_WaitSingleEventTimed

char PF_OS_WaitEvent( int taskNum, unsigned char EventMask) {
	char returnVal;
	
	PF_Calculate( taskNum );	
	returnVal = OS_WaitEvent( EventMask );
	PF_Increment( taskNum );
	return (returnVal);
}; // PF_OS_WaitEvent

void PF_OS_Use( int taskNum, OS_RSEMA* pRSema) {
	PF_Calculate( taskNum );	
	OS_Use( pRSema );
	PF_Increment( taskNum );
}; // PF_OS_Use

void PF_OS_Unuse( int taskNum, OS_RSEMA* pRSema) {
	PF_Calculate( taskNum );	
	OS_Unuse( pRSema );
	PF_Increment( taskNum );
}; // PF_OS_Unuse

void PF_OS_PutMail( int taskNum, OS_MAILBOX* pMB, void* pDest) {
	PF_Calculate( taskNum );	
	OS_PutMail( pMB, pDest );
	PF_Increment( taskNum );
}; // PF_OS_PutMail

void PF_OS_GetMail( int taskNum, OS_MAILBOX* pMB, void* pDest) {
	PF_Calculate( taskNum );	
	OS_GetMail( pMB, pDest );
	PF_Increment( taskNum );
}; // PF_OS_GetMail

void PF_OS_SetPriority(int taskNum, OS_TASK* pTask, unsigned char Priority) {
	PF_Calculate( taskNum );	
	OS_SetPriority( pTask, Priority );
	PF_Increment( taskNum );
}; // PF_OS_SetPriority

void PF_OS_WakeTask (int taskNum, OS_TASK* pTask) {
	PF_Calculate( taskNum );	
	OS_WakeTask( pTask );
	PF_Increment( taskNum );
}; // PF_OS_WakeTask

void PF_OS_Yield (int taskNum ) {
	PF_Calculate( taskNum );	
	OS_Yield();
	PF_Increment( taskNum );
}; // PF_OS_Yield

void PF_OS_SignalEvent( int taskNum, char Event, OS_TASK* pTask) {
	PF_Calculate( taskNum );	
	OS_SignalEvent( Event, pTask );
	PF_Increment( taskNum );
}; // PF_OS_SignalEvent

int PF_OS_Q_GetPtr( int taskNum, OS_Q* pQ, void** ppData) {
	int returnVal;
	
	PF_Calculate( taskNum );	
	returnVal = OS_Q_GetPtr( pQ, ppData );
	PF_Increment( taskNum );
	return (returnVal);
}; // PF_OS_Q_GetPtr

U32 PF_FS_Write(int taskNum, FS_FILE *pFile, const void *pData, U32 NumBytes) {
	U32 returnVal;
	
	PF_Calculate( taskNum );	
	returnVal = FS_Write(pFile, pData, NumBytes);
	PF_Increment( taskNum );
	return (returnVal);
}; // PF_FS_Write

U32 PF_FS_Read(int taskNum, FS_FILE *pFile, void *pData, U32 NumBytes) {
	U32 returnVal;
	
	PF_Calculate( taskNum );	
	returnVal = FS_Read(pFile, pData, NumBytes);
	PF_Increment( taskNum );
	return (returnVal);
}; // PF_FS_Read

// common functions
//
void PF_Calculate( int taskNum ) {
	U32 execTime;
	
	if (0 != taskProfile[taskNum].lastStartTime) {
		execTime = (OS_GetTime32() - taskProfile[taskNum].lastStartTime);
		if (execTime > taskProfile[taskNum].maxExecTime)
			taskProfile[taskNum].maxExecTime = execTime;
		taskProfile[taskNum].totalExecTime += execTime;
	} // if
} // PF_Calculate

void PF_Increment( int taskNum ) {
	taskProfile[taskNum].numberOfStarts++;
	taskProfile[taskNum].lastStartTime = OS_GetTime32();
} // PF_Increment

#undef PROFILER_C