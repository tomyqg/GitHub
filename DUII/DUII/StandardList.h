
#ifndef	_STANDARDLIST
#define	_STANDARDLIST

#ifdef __cplusplus
extern "C" {
#endif

#define	BUTTON_LEFT			258
#define	BUTTON_HEIGHT		39
#define	BUTTON_WIDTH		202
#define	BOX_LEFT			27
#define	BOX_RIGHT			245
#define	BOX_TOP				51
#define	BOX_BOTTOM			255

#define	TRIPLE_SPINNER_START_1		225
#define	TRIPLE_SPINNER_START_2		304
#define	TRIPLE_SPINNER_START_3		383
#define	TRIPLE_SPINNER_TEXT_START	22
#define	SPINNER_START				240
#define	SPINNER_SCREEN_BUTTON_POS	30

enum LIST_TYPE {
	TEXT_ONLY,
	ONE_BUTTON,
	TWO_BUTTON,
	THREE_BUTTON,
	FOUR_BUTTON,
	FOUR_BUTTON_NO_TEXT,
	FIVE_BOTTON_NO_TEXT
};

void DisplayStdListScreen(WM_HWIN hParent, ...);
void DisplayTripleListScreen(WM_HWIN hParent,
			char *pList1[], int listWheelID1, LISTWHEEL_Handle *pWheel1, int selection1,
			char *pList2[], int listWheelID2, LISTWHEEL_Handle *pWheel2, int selection2,
			char *pList3[], int listWheelID3, LISTWHEEL_Handle *pWheel3, int selection3,
			char *pText,
//			char *pOptionText1, char *pOptionText2,
			char *pButtonText, int buttonID);
GUI_HWIN GetButtonID(int button);


#ifdef __cplusplus
}
#endif

#endif
