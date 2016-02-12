
#ifndef	_LINEARGAUGE_H
#define	_LINEARGAUGE_H

#ifdef __cplusplus
extern "C" {
#endif

extern WM_HWIN DisplayMultiAnalogGaugesScreen(WM_HWIN hParent);
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern void _cbLinearGaugeCallback(WM_MESSAGE * pMsg);

#ifdef __cplusplus
}
#endif

#endif
