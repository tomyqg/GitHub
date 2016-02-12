#ifndef _WINDOWS
#include "RTOS.H"
#endif

#include "InterTask.h"

void notifyModeChange ( void ) {
#ifndef _WINDOWS
	OS_SignalEvent((1 << DU2_MODE_CHANGE), &commTCB);
#endif
} // notifyModeChange

void notifyConfigChange ( void ) {
#ifndef _WINDOWS
	OS_SignalEvent((1 << DU2_CONFIG_CHANGE), &commTCB);
#endif
} // notifyConfigChange

void notifyCameraChange ( void ) {
#ifndef _WINDOWS
	OS_SignalEvent((1 << DU2_CAMERA_CHANGE), &commTCB);
#endif
} // notifyCameraChange

void notifyGPSSimChange ( int enable ) {
#ifndef _WINDOWS
	if (enable)
		OS_SignalEvent((1 << DU2_SIMULATE_GPS_ON), &commTCB);
	else
		OS_SignalEvent((1 << DU2_SIMULATE_GPS_OFF), &commTCB);
#endif
} // notifyGPSSimChange

void notifyStartSet ( void ) {
#ifndef _WINDOWS
	OS_SignalEvent((1 << DU2_START_SET), &lapTimerTCB);
#endif
} // notifyStartSet

void notifyFinishSet ( void ) {
#ifndef _WINDOWS
	OS_SignalEvent((1 << DU2_FINISH_SET), &lapTimerTCB);
#endif
} // notifyFinishSet

void notifyGPS ( void ) {
#ifndef _WINDOWS
	OS_SignalEvent((1 << DU2_NEW_GPS), &lapTimerTCB);
#endif
} // notifyGPS

void notifyTQM ( int command) {
#ifndef _WINDOWS
	OS_SignalEvent((1 << command), &TQMWriteTCB);
#endif
} // notifyGPS

int tqmOpen ( void *pData, int datasize) {
#ifdef _WINDOWS
	return 1;
#else
	int retval = (OS_Q_Put( &Queue_tqm, pData, datasize ));
	OS_SignalEvent((1 << DU2_TQM_OPEN), &TQMWriteTCB);
	return retval;
#endif
} // tqmOpen

int tqmWrite ( void *pData, int datasize) {
#ifdef _WINDOWS
	return 1;
#else
	int retval = (OS_Q_Put( &Queue_tqm, pData, datasize ));
	OS_SignalEvent((1 << DU2_TQM_WRITE), &TQMWriteTCB);
	return retval;
#endif
} // tqmWrite