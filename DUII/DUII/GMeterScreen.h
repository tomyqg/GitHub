
#ifndef	_GMETER_H
#define	_GMETER_H

#ifdef __cplusplus
extern "C" {
#endif
  
extern WM_HWIN Display_GMeterScreen(WM_HWIN hParent);
extern void DrawGMeter(void);
extern void _cbGMeterCallback(WM_MESSAGE * pMsg);
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
char IfGMeterConfigGaugePressed(int lastx, int lasty);

#ifdef __cplusplus
}
#endif

#define	G_METER_SCREEN_REFRESH_TIME		50
#define	RESET_BUTTON					GUI_ID_USER + 1	

#endif
