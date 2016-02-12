#ifndef HELPSCREENS_H
#define HELPSCREENS_H

#ifdef __cplusplus
extern "C" 
#endif

void CreateHelpSelection(WM_HWIN caller);
void DeleteHelpScreen(void);
void DrawJPEGSplash(void);

typedef struct { unsigned char *pScreen; int size; } HELP_SCREENS;

extern HELP_SCREENS MainHelpScreens[];
extern HELP_SCREENS GoRacingHelpScreens[];
extern HELP_SCREENS SUCameraHelpScreens[];
extern HELP_SCREENS DUCameraHelpScreens[];
extern HELP_SCREENS SUSpecificCameraHelpScreens[];
extern HELP_SCREENS DUSpecificCameraHelpScreens[];
extern HELP_SCREENS GenericCameraHelpScreens[];
extern HELP_SCREENS VehicleSetupHelpScreens[];
extern HELP_SCREENS VehicleSetupSpecificHelpScreens[];
extern HELP_SCREENS VehicleSelectionHelpScreens[];
extern HELP_SCREENS InputsHelpScreens[];
extern HELP_SCREENS AnalogInputsHelpScreens[];
extern HELP_SCREENS EditScaleHelpScreens[];
extern HELP_SCREENS ReviewSessionsHelpScreens[];
extern HELP_SCREENS TachHelpScreens[];
extern HELP_SCREENS TachSpecificHelpScreens[];
extern HELP_SCREENS TrackSettingsHelpScreens[];
extern HELP_SCREENS TrackSettingsSpecificHelpScreens[];
extern HELP_SCREENS AutoOnHelpScreens[];
extern HELP_SCREENS AutoOnSpecificHelpScreens[];
extern HELP_SCREENS DynoHelpScreens[];
extern HELP_SCREENS TimeZoneGPSScreens[];

extern HELP_SCREENS *pHelpScreens;

#endif