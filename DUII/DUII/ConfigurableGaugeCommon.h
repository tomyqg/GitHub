
#ifndef	CONFIGURABLE_GAUGE_COMMON_H
#define	CONFIGURABLE_GAUGE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

struct DISPLAY_ELEMENT {
	GUI_RECT box;
	int justification;
	int color;
	const GUI_FONT *pFont;
};

struct CONFIG_ARGS {
	struct DISPLAY_ELEMENT *pData;
	struct DISPLAY_ELEMENT *pName;
	struct DISPLAY_ELEMENT *pUnits;
};

extern void ConfigureGauge(void);
extern void ProcessGaugeButton(int lastx, int lasty);
extern void FormatAnalogData(char *scratch, float dValue);
extern void DisplayConfigurableItem(const struct CONFIG_ARGS *config);
extern char ScreenIsConfigurable(int screenID);
extern char IfAnalogConfigGaugePressed(int lastx, int lasty);
extern char IfRPMSweepConfigGaugePressed(int lastx, int lasty);
extern char IfDigitalConfigGaugePressed(int lastx, int lasty);
extern GlobalDataIndexType GetConfigurableItem(void);
extern void SetConfigurableItem(GlobalDataIndexType item);

extern char configureGaugeMode;
extern char drawFlag;
extern int boxStartTime;

#ifdef __cplusplus
}
#endif

#endif
