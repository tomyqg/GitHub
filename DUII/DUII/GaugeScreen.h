
#ifndef	_GAUGE_H
#define	_GAUGE_H

#ifdef __cplusplus
extern "C" {
#endif
  
extern WM_HWIN Display_GaugeScreen(WM_HWIN hParent);
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern void _cbGaugeCallback(WM_MESSAGE * pMsg);
extern void ConfigureGauge(void);

#ifdef __cplusplus
}
#endif

#endif
