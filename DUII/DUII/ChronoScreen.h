
#ifndef	_CHRONO_H
#define	_CHRONO_H

#ifdef __cplusplus
extern "C" {
#endif
  
extern WM_HWIN DisplayChronoScreen(WM_HWIN hParent);
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern void _cbChronoCallback(WM_MESSAGE * pMsg);

#ifdef __cplusplus
}
#endif

#endif
