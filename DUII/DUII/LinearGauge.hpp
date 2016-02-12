/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: LinearGauge.hpp
Description: linear gauge object header file
-----------------------------------------------------------------
*/
#ifndef LINEARGAUGE_HPP
#define LINEARGAUGE_HPP

#include "DUII.h"

class LinearGauge {
public:
	LinearGauge(int x, int y, int xSize, int ySize, float MinValue, float MaxValue, 
		int inUseLowerRed, float lowerRed, int inUseUpperRed, float upperRed, char *inName, 
		char *inUnits, int Multiplier, WM_HWIN hParent, bool inEnabled);
	~LinearGauge();
	void DrawBackground();
	void SetValue(float Value);

private:
	void _cbCallback(WM_MESSAGE * pMsg);
	void CreateGaugeBackground(void);
	void Draw();

/****************************************************************
*
*       class variables
*
******************************************************************/
	WM_HWIN hDlg;
	float value;

	int xStart, yStart, xEnd, yEnd;
	int yPos;
	char *Name, *Units;
	float minVal, maxVal;
	int useLowerRed, useUpperRed;
	float lowerRed, upperRed;
	float stepSize;
	BUTTON_Handle hPointer;
	bool enabled;
};

#endif