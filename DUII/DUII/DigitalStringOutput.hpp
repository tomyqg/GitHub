/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: DigitalStringOutput.hpp
Description: The digital display control
-----------------------------------------------------------------
*/
#ifndef DIGITALOUTPUT_DEFINE
#define DIGITALOUTPUT_DEFINE

#include "GUI.h"
#include "WM.h"
#include <stdio.h>
#include "ControlBase.hpp"
#include "alarm.hpp"
#include "DigitalDisplay.hpp"

//#define DIGITAL_LARGE_FONT GUI_FontD80
#define DIGITAL_LARGE_FONT GUI_Font32B_ASCII
#define DIGITAL_MEDIUM_FONT GUI_Font24B_ASCII
#define DIGITAL_SMALL_FONT GUI_Font16_ASCII
#define DIGITAL_TINY_FONT GUI_Font13_ASCII
#define DIGITAL_TEXT_FONT GUI_Font32B_ASCII

class DigitalStringOutput : public ControlBase {
public:
	DigitalStringOutput(int x, int y, int xSize, int ySize, DigitalOptions Options, WM_HWIN _hParent, GUI_FONT font, GUI_COLOR cColor);
	~DigitalStringOutput();
	void DrawControl(void);
	void SetColor(GUI_COLOR col) { Color = col; }
	void SetValue(char *sValue);
	GUI_MEMDEV_Handle DrawToMemArea();
	//GUI_RECT GetRect(void);
	char Value[100]; // the value of the control
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

	//GUI_MEMDEV_Handle memBackground; // the background memory device
	GUI_MEMDEV_Handle mem; // the memory device that is used to draw the background and the values
	GUI_FONT fTextFont;
	GUI_COLOR Color;
	GUI_RECT rect;
};

#endif
