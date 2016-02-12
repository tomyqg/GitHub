/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: DigitalDisplay.hpp
Description: The digital display control
-----------------------------------------------------------------
*/
#ifndef DIGITALDISPLAY_DEFINE
#define DIGITALDISPLAY_DEFINE

#include "GUI.h"
#include "WM.h"
#include <stdio.h>
#include "ControlBase.hpp"
#include "alarm.hpp"

//#define DIGITAL_LARGE_FONT GUI_FontD80
#define DIGITAL_LARGE_FONT GUI_Font32B_ASCII
#define DIGITAL_MEDIUM_FONT GUI_Font24B_ASCII
#define DIGITAL_SMALL_FONT GUI_Font16_ASCII
#define DIGITAL_TINY_FONT GUI_Font13_ASCII
#define DIGITAL_TEXT_FONT GUI_Font32B_ASCII

typedef enum {
	Left,
	Top
} DigitalOptions;

class DigitalDisplay : public ControlBase {
public:
	DigitalDisplay(int x, int y, int xSize, int ySize, char *Name, char *Units, float dMin, float dMax, int iPrecision, int iDividerLock, DigitalOptions Options, Alarm *AlarmList, bool bShowLabel, WM_HWIN _hParent);
	DigitalDisplay(int x, int y, int xSize, int ySize, char *Name, char *Units, float dMin, float dMax, int iPrecision, int iDividerLock, DigitalOptions Options, Alarm *AlarmList, bool bShowLabel, GUI_FONT fNumberFont, GUI_FONT fLabelFont, WM_HWIN _hParent);
	~DigitalDisplay();
	void DrawControl(void);
	void SetValue(float dValue);
	GUI_MEMDEV_Handle DrawToMemArea();

	//GUI_RECT GetRect(void);
	float Value; // the value of the control
	float Min, Max;
	void GetRect(GUI_RECT *rect)
	{
		rect->x0 = ControlBase::x;
		rect->y0 = ControlBase::y;
		rect->x1 = ControlBase::x + Width;
		rect->y1 = ControlBase::y + Height;
	}

private:
	void CreateBackground(void);
	char *GetDisplayText();
	int GetMaxTextLength();
	void PickFont();
	//int GetMaxLength();

	//int x, y;  // the location of the top left of the control
	int Width, Height; // the width and height of the control
	//char *Name; // the Text to display for the control
	//char *Units; // the units the control is measuring
	int Precision; // the number of places to the right of the decimial to display
	char strValue[20];
	int DividerLoc; // the location from the left sideof the control for the divider.  -1 if not used
	DigitalOptions DisplayOptions;
	int MaxStrLen; // the maximum length of the string output to the screen.  Used to calculate text location so the text doesn't move
	bool ShowLabel; // show the label for the control

	Point pDisp;

	GUI_MEMDEV_Handle memBackground; // the background memory device
	GUI_MEMDEV_Handle mem; // the memory device that is used to draw the background and the values
	GUI_FONT fTextFont, fUnitsFont;
};

#endif
