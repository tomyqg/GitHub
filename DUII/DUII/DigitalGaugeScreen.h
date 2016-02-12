
#ifndef	_DIGITALGAUGE_H
#define	_DIGITALGAUGE_H

#ifdef __cplusplus
extern "C" {
#endif

extern WM_HWIN Display_DigitalGaugeScreen(WM_HWIN hParent);
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern void _cbDigitalGaugeCallback(WM_MESSAGE * pMsg);

#ifdef __cplusplus
}
#endif

#endif
