
#include <stdio.h>
#include <string.h>
#include "DUII.h"
#include "GUI.h"
#include "commondefs.h"
#include "RunningGauges.h"
#include "DataHandler.h"
#include "ConfigurableGaugeCommon.h"
#include "InputScreens.h"
#include "GMeterScreen.h"
#include "Utilities.h"

/*********************************************************************
*
*       Public data Definitions
*
**********************************************************************
*/
char configureGaugeMode = false;
char drawFlag;
int boxStartTime;


/*********************************************************************
*
*       Local Constants Definitions
*
**********************************************************************
*/
#define	LINE_HEIGHT	20

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
int GetCurrentScreenID(void);


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
GlobalDataIndexType GetInputID(int input);
char ButtonPressIsOnConfigurableGauge(int lastx, int lasty);
void ShowOtherInput(const struct CONFIG_ARGS *config, GlobalDataIndexType input, char *pLabel, char *pUnits);
void ShowInput(const struct CONFIG_ARGS *config, int input);
void ShowDigitalInput(const struct CONFIG_ARGS *config, int input);
void ShowTime(const struct CONFIG_ARGS *config, GlobalDataIndexType item);

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/

char ScreenIsConfigurable(int screenID)
{
	if ((screenID == ANALOG_GAUGE) || (screenID == RPM_SWEEP_GAUGE) || (screenID == DIGITAL_GAUGE) || (screenID == GMETER))
		return true;
	else return false;
}

//
// Center of screen hit - enter gauge configuration mode
// NOTE: This routein applies to BOTH this (GaugeScreen) screen and RPMSweepScreen
void ConfigureGauge(void)
{
	configureGaugeMode = true;
	drawFlag = true;
	boxStartTime = GUI_GetTime();
}

//
// See if the current screen touch is on the configurable gauge on this screen
//
// I'm sure there's a better (C++) way to do this but it looks like the screens
// themselves are not classes which would have made this a cinch.  Off the top
// of my head I didn't see a cleaner way to do this - and I know this will work.
//
char ButtonPressIsOnConfigurableGauge(int lastx, int lasty)
{
	int screenID;

	if ((screenID = GetCurrentScreenID()) == ANALOG_GAUGE)
		return IfAnalogConfigGaugePressed(lastx, lasty);
	else if (screenID == RPM_SWEEP_GAUGE)
		return IfRPMSweepConfigGaugePressed(lastx, lasty);
	else if (screenID == DIGITAL_GAUGE)
		return IfDigitalConfigGaugePressed(lastx, lasty);
	else if (screenID == GMETER)
		return IfGMeterConfigGaugePressed(lastx, lasty);
	else return false;
}


//
// Screen hit somewhere while in gauge mode - process accordingly depending on 
// position relative to configurable gauges
//
void ProcessGaugeButton(int lastx, int lasty)
{
	if (ButtonPressIsOnConfigurableGauge(lastx, lasty))
	{
		boxStartTime = GUI_GetTime();		// Restart timer
		switch (GetConfigurableItem())			// Go to next control item
		{
		default:
		case VMAX:				SetConfigurableItem( VMIN );				break;
		case VMIN:				SetConfigurableItem( ANALOG_INPUT_0 );		break;
		case ANALOG_INPUT_0:	SetConfigurableItem( ANALOG_INPUT_1 );		break;
		case ANALOG_INPUT_1:	SetConfigurableItem( ANALOG_INPUT_2 );		break;
		case ANALOG_INPUT_2:	SetConfigurableItem( ANALOG_INPUT_3 );		break;
		case ANALOG_INPUT_3:	SetConfigurableItem( DIGITAL_INPUT_4 );		break;
		case DIGITAL_INPUT_4:	SetConfigurableItem( DIGITAL_INPUT_5 );		break;
		case DIGITAL_INPUT_5:	SetConfigurableItem( VEHICLE_12V );			break;
		case VEHICLE_12V:		SetConfigurableItem( X_G );					break;
		case X_G:				SetConfigurableItem( Y_G );					break;
		case Y_G:				SetConfigurableItem( Z_G );					break;
		case Z_G:				SetConfigurableItem( COMBINED_G );			break;
		case COMBINED_G:		SetConfigurableItem( PREDICTIVE_LAP );		break;
		case PREDICTIVE_LAP:	SetConfigurableItem( BEST_SESSION_LAP );	break;
		case BEST_SESSION_LAP:	SetConfigurableItem( ALTITUDE );			break;
		case ALTITUDE:			SetConfigurableItem( ELAPSED_TIME );		break;
		case ELAPSED_TIME:		SetConfigurableItem( GPS_TIME );			break;
		case GPS_TIME:			SetConfigurableItem( SPEED );				break;
		case SPEED:				SetConfigurableItem( VEHICLE_RPM );			break;
		case VEHICLE_RPM:		SetConfigurableItem( VMAX );				break;
		}
	}
}

GlobalDataIndexType GetConfigurableItem(void)
{
	return ((GlobalDataIndexType)(sysData.multiSettings_1 & 0x000000FF));
}

void SetConfigurableItem(GlobalDataIndexType item)
{
	sysData.multiSettings_1 = (sysData.multiSettings_1 & 0xFFFFFF00) | item;
}


//
// Display one of the four analog inputs in the position and as specified by the input parameters
//
// Input:	CONFIG_ARGS *config - see description in header file
//			input	number (0-3) of the analog input to display
//
void ShowInput(const struct CONFIG_ARGS *config, int input)
{
	float ftemp;
	char scratch[30], *pStr;

	if (vehicleData.analogInput[input].enabled)
	{
		// Display value
		GUI_SetColor(config->pData->color);
		ftemp = GetValue(GetInputID(input)).fVal;
		FormatAnalogData(scratch, ftemp);
		GUI_SetFont(config->pData->pFont);
		GUI_DispStringInRect(scratch, &config->pData->box, config->pData->justification);

		// Display Name
		GUI_SetColor(config->pName->color);
		GUI_SetFont(config->pName->pFont);
		// To shorten strings to fit in most windows remove leading AEM or PLX from name
		if ((strncmp(vehicleData.analogInput[input].Name, "AEM ", 4) == 0) || (strncmp(vehicleData.analogInput[input].Name, "PLX ", 4) == 0))
			pStr = vehicleData.analogInput[input].Name + 4;
		else pStr = vehicleData.analogInput[input].Name;
		GUI_DispStringInRect(pStr, &config->pName->box, config->pName->justification);

		// Display Units
		GUI_SetColor(config->pUnits->color);
		GUI_SetFont(config->pUnits->pFont);
		GUI_DispStringInRect(vehicleData.analogInput[input].Units, &config->pUnits->box, config->pUnits->justification);
	}
	else 
	{
		// Display Input Number
		GUI_SetColor(config->pName->color);
		GUI_SetFont(config->pName->pFont);
		sprintf(scratch, "Analog %d", input);
		GUI_DispStringInRect(scratch, &config->pName->box, config->pName->justification);

		// Display Input Disabled
		GUI_SetColor(config->pUnits->color);
		GUI_SetFont(config->pUnits->pFont);
		GUI_DispStringInRect("Disabled", &config->pUnits->box, config->pUnits->justification);
	}
}

//
// Display one of the two digital inputs in the position and as specified by the input parameters
//
// Input:	CONFIG_ARGS *config - see description in header file
//			input - number (0-1) of the digital input to display
//
void ShowDigitalInput(const struct CONFIG_ARGS *config, int input)
{
	char scratch[30];

	if (vehicleData.digitalInput[input].enabled)
	{
		// Display Value
		GUI_SetColor(config->pData->color);
		GUI_SetFont(&GUI_Font32B_ASCII);		// Select largest alpha/numeric font available to display text
		GUI_DispStringInRect(GetDigitalText(USER, input), &config->pData->box, config->pData->justification);

		// Draw Name
		GUI_SetColor(config->pName->color);
		GUI_SetFont(config->pName->pFont);
		GUI_DispStringInRect(vehicleData.digitalInput[input].Name, &config->pName->box, config->pName->justification);
	}
	else 
	{
		// Draw Input Number
		GUI_SetColor(config->pName->color);
		GUI_SetFont(config->pName->pFont);
		sprintf(scratch, "Digital %d", input + 4);
		GUI_DispStringInRect(scratch, &config->pName->box, config->pName->justification);

		// Draw Disabled
		GUI_SetColor(config->pUnits->color);
		GUI_SetFont(config->pUnits->pFont);
		GUI_DispStringInRect("Disabled", &config->pUnits->box, config->pUnits->justification);
	}
}


void ShowOtherInput(const struct CONFIG_ARGS *config, GlobalDataIndexType input, char *pLabel, char *pUnits)
{
	float ftemp;
	char scratch[30];

	// Format Data Value
	if (input == VEHICLE_RPM)
		sprintf(scratch, "%d", GetValue(VEHICLE_RPM).iVal);
	else
	{
		ftemp = GetValue(input).fVal;
		if ((input == ALTITUDE) || (input == SPEED))	// Altitude and speed too big for decimal place, just display integer portion
			sprintf(scratch, "%d", (int)ftemp);
		else if ((input == PREDICTIVE_LAP) || (input == COMBINED_G) || (input == X_G) || (input == Y_G) || (input == Z_G))
			sprintf(scratch, "%0.2f", ftemp);
		else sprintf(scratch, "%0.1f", ftemp);
	}

	// Set text color based on data type and value
	if (input == PREDICTIVE_LAP)
	{
		if (ftemp < 0)
			GUI_SetColor(GUI_GREEN);
		else GUI_SetColor(GUI_RED);
	}
	else GUI_SetColor(config->pData->color);

	// Display Data Value
	GUI_SetFont(config->pData->pFont);
	GUI_DispStringInRect(scratch, &config->pData->box, config->pData->justification);

	// Display Name
	GUI_SetColor(config->pName->color);
	GUI_SetFont(config->pName->pFont);
	GUI_DispStringInRect(pLabel, &config->pName->box, config->pName->justification);

	// Display Units
	GUI_SetColor(config->pUnits->color);
	GUI_SetFont(config->pUnits->pFont);
	GUI_DispStringInRect(pUnits, &config->pUnits->box, config->pUnits->justification);
}

//
// Format string - essentially copied from LinearGauge.cpp but without the font setting
//
// Input:	pointer to character buffer to receive characters
//			floating point value to format
//
void FormatAnalogData(char *pStr, float dValue)
{
	if (dValue == 0.0)
		strcpy(pStr, "0");					// 1 character
	else if ((dValue > 1000.0) || (dValue < -100.0))
		sprintf(pStr, "%0.0f", dValue);		// 4 or more characters
	else if ((dValue > 100.0) || (dValue < -10.0))
		sprintf(pStr, "%0.0f", dValue);		// 3 characters
	else if ((dValue > 10.0) || (dValue < -1.0))
		sprintf(pStr, "%0.1f", dValue);		// 3 characters
	else if ((dValue > 1.0) || (dValue < -0.1))
		sprintf(pStr, "%0.2f", dValue);		// 3 characters
	else sprintf(pStr, "%0.2f", dValue);	// 3 digits (or 2 plus minus sign)
}


void ShowTime(const struct CONFIG_ARGS *config, GlobalDataIndexType item)
{
	char scratch[10], *pText1, *pText2;
	
	if (item == BEST_SESSION_LAP)
	{
		strcpy(scratch, formatLapTime(GetValue(item).uVal, false));
	}
	else if (item == GPS_TIME)
	{
		GetTimeString(SHORT12, scratch);
	}
	else
	{
		unsigned int time;

		time = GetValue(item).uVal / 1000;		// time = time in seconds
		
		// Up to 99 minutes, 59 seconds display minutes and seconds, then display hours and minutes
		if (time < 6000)
			sprintf(scratch, "%02d:%02d", time / 60, time % 60);
		else sprintf(scratch, "%02d:%02d", time/3600, (time/60)%60);
	}

	// Display Data Value
	GUI_SetColor(config->pData->color);
	GUI_SetFont(config->pData->pFont);
	GUI_DispStringInRect(scratch, &config->pData->box, config->pData->justification);

	// Display text
	GUI_SetColor(config->pName->color);
	GUI_SetFont(config->pName->pFont);
	switch (item)
	{
		case ELAPSED_RACE_TIME:	pText1 = "elapsed";	pText2 = "time";	break;
		case GPS_TIME:			pText1 = "GPS";		pText2 = "time";	break;
		case BEST_SESSION_LAP:	pText1 = "Best";	pText2 = "Lap";		break;
		default:				pText1 = "";		pText2 = "";		break;
	}
	GUI_DispStringInRect(pText1, &config->pName->box, config->pName->justification);
	GUI_DispStringInRect(pText2, &config->pUnits->box, config->pUnits->justification);
}

//
// Redefined here in pure C rather than C++
//
GlobalDataIndexType GetInputID(int input)
{
	switch (input)
	{
	default:
	case 0:	return ANALOG_INPUT_0;
	case 1:	return ANALOG_INPUT_1;
	case 2:	return ANALOG_INPUT_2;
	case 3:	return ANALOG_INPUT_3;
	}
}

//
// Display (configurable) data item in the position and as specified by the input parameters
//
// Input:	item - data item to display
//			CONFIG_ARGS *config - see description in header file
//
void DisplayConfigurableItem(const struct CONFIG_ARGS *config)
{
	switch (GetConfigurableItem())
	{
	default:
	case VMAX:
		ShowOtherInput(config, VMAX, "vmax", sysData.units == STD ? "mph" : "kph");
		break;

	case VMIN:
		ShowOtherInput(config, VMIN, "vmin", sysData.units == STD ? "mph" : "kph");
		break;

	case VEHICLE_12V:
		ShowOtherInput(config, VEHICLE_12V, "Battery", "Volts");
		break;

	case X_G:
		ShowOtherInput(config, X_G, "Accel/Bk", "Gs");
		break;

	case Y_G:
		ShowOtherInput(config, Y_G, "Lateral", "Gs");
		break;

	case Z_G:
		ShowOtherInput(config, Z_G, "Vertical", "Gs");
		break;

	case COMBINED_G:
		ShowOtherInput(config, COMBINED_G, "Total", "Gs");
		break;

	case PREDICTIVE_LAP:
		ShowOtherInput(config, PREDICTIVE_LAP, "Gap", "secs");
		break;

	case ALTITUDE:
		ShowOtherInput(config, ALTITUDE, "Altitude", sysData.units == STD ? "feet" : "meters");
		break;

	case VEHICLE_RPM:
		ShowOtherInput(config, VEHICLE_RPM, "rpm", "");
		break;

	case ANALOG_INPUT_0:
		ShowInput(config, 0);
		break;

	case ANALOG_INPUT_1:
		ShowInput(config, 1);
		break;

	case ANALOG_INPUT_2:
		ShowInput(config, 2);
		break;

	case ANALOG_INPUT_3:
		ShowInput(config, 3);
		break;

	case DIGITAL_INPUT_4:
		ShowDigitalInput(config, 0);
		break;

	case DIGITAL_INPUT_5:
		ShowDigitalInput(config, 1);
		break;

	case ELAPSED_TIME:
		ShowTime(config, ELAPSED_RACE_TIME);
		break;

	case GPS_TIME:
		ShowTime(config, GPS_TIME);
		break;

	case BEST_SESSION_LAP:
		ShowTime(config, BEST_SESSION_LAP);
		break;

	case SPEED:
		// For speed only display units in the name field because the only screen we really 
		// want this input on is more symmetrical with units in the top position.  Other
		// screens that use the configurable gauge item already have speed on them so the
		// format of this input is not a priority on those screens.
		ShowOtherInput(config, SPEED, sysData.units == STD ? "mph" : "kph", "");
		break;
	}
}
