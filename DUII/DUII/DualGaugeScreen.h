
#ifndef	_DUALGAUGE_H
#define	_DUALGAUGE_H

#ifdef __cplusplus
extern "C" {
#endif

extern WM_HWIN Display_DualGaugeScreen(WM_HWIN hParent);
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern void _cbDualGaugeCallback(WM_MESSAGE * pMsg);

#ifdef __cplusplus
}
#endif

#endif
