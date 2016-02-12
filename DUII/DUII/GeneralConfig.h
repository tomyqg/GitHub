
#ifndef	_GENERALCONFIG_H
#define	_GENERALCONFIG_H

void Display_GeneralConfig(WM_HWIN);



#ifdef __cplusplus
extern "C" {
#endif

void SystemError(void);
void GeneralConfigIgnoreNextKeyRelease(void);
WM_HWIN GetGeneralConfinScreenWindowHandle(void);

extern const GUI_RECT BannerRect;
extern const GUI_RECT BannerTimeRect;
extern const GUI_RECT InfoTimeRect;

#ifdef __cplusplus
}
#endif

#endif
