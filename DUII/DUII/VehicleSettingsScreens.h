
#ifndef	_VEHICLESETTINGSSCREENS_H
#define	_VEHICLESETTINGSSCREENS_H


#ifdef __cplusplus
extern "C" {
#endif

WM_HWIN GetVehicleSettingsWindowHandle(void);
extern GUI_HWIN GetAeroGripWindowHandle(void);
extern void CreateAeroGripScreen(WM_HWIN hParent);
extern void SetVehicleSettingsWindowHandle(WM_HWIN);
extern struct SCREEN_LIST VehicleSettingsScreenList[];
extern void VehicleSettingsScreensCallback(WM_MESSAGE *);
extern void VehicleSettingsIgnoreNextKeyRelease(void);
extern void CreateCameraSelection(WM_HWIN hParent);
extern void CreateCameraDelay(WM_HWIN hParent);
void SetCameraDelay(void);
GUI_HWIN GetCameraDelayWindowHandle(void);
GUI_HWIN GetCameraSelectionWindowHandle(void);
void TestCameras(int);

void Display_VehicleSettingMainScreen(WM_HWIN hPrev);
WM_HWIN GetVehicleSettingMainWindowHandle(void);
void SetVehicleSettingMainWindowHandle(WM_HWIN);
int ValidateVehicleData(void);
int ValidateDiffRatio(void);
int ValidateCamera(void);
const char *GetCameraName(void);
char *GetFullCameraText(char *pBuffer);
void ValidateCameraData(void);
void CreateTraqdashCamScreen(WM_HWIN hParent);
GUI_HWIN GetTraqdashCamWindowHandle(void);
unsigned char GetSUCam(void);
unsigned char GetTDCam(void);


#ifdef __cplusplus
}
#endif


#define	NUM_VEHICLE_SETTINGS_SCREENS	6

#define	TACH_SELECTION			0
#define	CARMETRICS_SELECTION	1
#define	GEARRATIO_SELECTION		2
#define	INPUTS_SELECTION		3
#define	WEIGHT_SELECTION		4
#define	CAMERA_SETUP_SELECTION	5

#define	TACH_SCREEN_REFRESH_TIME	100

#endif
