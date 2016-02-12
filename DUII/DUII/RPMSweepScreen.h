
#ifndef RPMSWEEP_H
#define RPMSWEEP_H

#ifdef __cplusplus
extern "C" {
#endif

extern WM_HWIN Display_RPMSweepScreen(WM_HWIN hParent);
extern void _cbRPMSweepCallback(WM_MESSAGE * pMsg);
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;

#ifdef __cplusplus
}
#endif

#endif