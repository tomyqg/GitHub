/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2009     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : GUI_WIDGET_GraphXYDemo.c
Purpose : Demonstrates the use of the GRAPH widget
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "BSP.h"
#include "GUI.h"
#include "DIALOG.h"
#include "GRAPH.h"
#include "SysConf.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define PI 3.1415926f
#define DEG2RAD (PI / 180)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static GRAPH_SCALE_Handle _hScaleV;   /* Handle of vertical scale */
static GRAPH_SCALE_Handle _hScaleH;   /* Handle of horizontal scale */

static const GUI_COLOR _aColor[3] = {GUI_RED, GUI_DARKGREEN, GUI_MAGENTA};
static const U8 _aLStyle[3] = {GUI_LS_SOLID, GUI_LS_DOT, GUI_LS_DASH};

/* Dialog ressource */
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "GRAPH_DATA_XY demo", 0                ,   0,   0, 320, 240, FRAMEWIN_CF_MOVEABLE },
  { GRAPH_CreateIndirect,     0,                   GUI_ID_GRAPH0    ,   5,   5, 270, 175 },
  { TEXT_CreateIndirect,      "Spacing X:",        0                ,  10, 185,  50,  20 },
  { TEXT_CreateIndirect,      "Spacing Y:",        0                ,  10, 205,  50,  20 },
  { SLIDER_CreateIndirect,    0,                   GUI_ID_SLIDER0   ,  60, 185,  60,  16 },
  { SLIDER_CreateIndirect,    0,                   GUI_ID_SLIDER1   ,  60, 205,  60,  16 },
  { TEXT_CreateIndirect,      "Border",            0                , 280,   5,  35,  15 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK2    , 280,  20,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK3    , 280,  40,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK4    , 280,  60,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK5    , 280,  80,  35,   0 },
  { TEXT_CreateIndirect,      "Effect",            0                , 280, 100,  35,  15 },
  { RADIO_CreateIndirect,     0,                   GUI_ID_RADIO0    , 275, 115,  35,   0, 0, 3 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK6    , 180, 185,  50,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK7    , 180, 205,  50,   0 },
  { BUTTON_CreateIndirect,    "Full Screen",       GUI_ID_BUTTON0   , 235, 185,  65,  18 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK1    , 235, 205,  70,   0 },
};

static GUI_POINT _aPoint[3][46];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _InitPoints
*/
static void _InitPoints(void) {
  int i, r, m;
  m = 360 / (GUI_COUNTOF(_aPoint[0]) - 1);
  r = 95;
  for (i = 0; i <= GUI_COUNTOF(_aPoint[0]) - 1; i++) {
    float Rad;
    int Deg;
    Deg = i * m;
    Rad = Deg * DEG2RAD;
    _aPoint[0][i].x = (int)(100 - r * cos(Rad));
    _aPoint[0][i].y = (int)(100 + r * sin(Rad) * Deg / 360);
    _aPoint[1][i].x = (int)(100 - r * cos(Rad));
    _aPoint[1][i].y = (int)(100 + r * sin(Rad) * Deg / 300);
    _aPoint[2][i].x = (int)(100 - r * cos(Rad) * 0.9);
    _aPoint[2][i].y = (int)(100 + r * sin(Rad) * Deg / 320);
  }
}

/*********************************************************************
*
*       _UserDraw
*
* Purpose:
*   This routine is called by the GRAPH object before anything is drawn
*   and after the last drawing operation.
*/
static void _UserDraw(WM_HWIN hWin, int Stage) {
  if (Stage == GRAPH_DRAW_LAST) {
    char acText[] = "Flow Rate L/s";
    GUI_RECT Rect, RectInvalid;
    int FontSizeY;
    GUI_SetFont(&GUI_Font13_ASCII);
    FontSizeY = GUI_GetFontSizeY();
    WM_GetInsideRect(&Rect);
    WM_GetInvalidRect(hWin, &RectInvalid);
    Rect.x1 = Rect.x0 + FontSizeY;
    GUI_SetColor(GUI_RED);
    GUI_DispStringInRectEx(acText, &Rect, GUI_TA_HCENTER, strlen(acText), GUI_ROTATE_CCW);
  }
}

/*********************************************************************
*
*       _ForEach
*
* Purpose:
*   This routine hides/shows all windows except the button, graph and scroll bar widgets
*/
static void _ForEach(WM_HWIN hWin, void * pData) {
  int Id, FullScreenMode;
  FullScreenMode = *(int *)pData;
  Id = WM_GetId(hWin);
  if ((Id == GUI_ID_GRAPH0) || (Id == GUI_ID_BUTTON0) || (Id == GUI_ID_VSCROLL) || (Id == GUI_ID_HSCROLL)) {
    return;
  }
  if (FullScreenMode) {
    WM_HideWindow(hWin);
  } else {
    WM_ShowWindow(hWin);
  }
}

/*********************************************************************
*
*       _ToggleFullScreenMode
*
* Purpose:
*   This routine switches between full screen mode and normal mode by hiding or showing the
*   widgets of the dialog, enlarging/shrinking the graph widget and modifying some other
*   attributes of the dialog widgets.
*/
static void _ToggleFullScreenMode(WM_HWIN hDlg) {
  static int FullScreenMode;
  static GUI_RECT Rect;
  static unsigned ScalePos;
  WM_HWIN hGraph, hButton;
  hGraph  = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
  hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
  FullScreenMode ^= 1;
  if (FullScreenMode) {
    /* Enter the full screen mode */
    WM_HWIN hClient;
    GUI_RECT RectInside;
    hClient = WM_GetClientWindow(hDlg);
    BUTTON_SetText(hButton, "Back");
    WM_MoveWindow(hButton, 0, 11);
    FRAMEWIN_SetTitleVis(hDlg, 0);
    WM_GetInsideRectEx(hClient, &RectInside);
    WM_GetWindowRectEx(hGraph, &Rect);
    WM_ForEachDesc(hClient, _ForEach, &FullScreenMode); /* Hide all descendants */
    WM_SetWindowPos(hGraph, WM_GetWindowOrgX(hClient), WM_GetWindowOrgX(hClient), RectInside.x1, RectInside.y1);
    ScalePos = GRAPH_SCALE_SetPos(_hScaleH, RectInside.y1 - 105);
  } else {
    /* Return to normal mode */
    BUTTON_SetText(hButton, "Full Screen");
    WM_MoveWindow(hButton, 0, -11);
    WM_ForEachDesc(WM_GetClientWindow(hDlg), _ForEach, &FullScreenMode); /* Show all descendants */
    WM_SetWindowPos(hGraph, Rect.x0, Rect.y0, Rect.x1 - Rect.x0 + 1, Rect.y1 - Rect.y0 + 1);
    FRAMEWIN_SetTitleVis(hDlg, 1);
    GRAPH_SCALE_SetPos(_hScaleH, ScalePos);
  }
}

/*********************************************************************
*
*       _cbCallback
*
* Purpose:
*   Callback function of the dialog
*/
static void _cbCallback(WM_MESSAGE * pMsg) {
  int NCode, Id, Value, i;
  WM_HWIN hDlg, hItem;
  hDlg = pMsg->hWin;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
    _InitPoints();
    /* Add graphs */
    for (i = 0; i < 3; i++) {
      GRAPH_DATA_Handle  hDataXY;
      hDataXY = GRAPH_DATA_XY_Create(_aColor[i], GUI_COUNTOF(_aPoint[i]), _aPoint[i], GUI_COUNTOF(_aPoint[i]));
      GRAPH_DATA_XY_SetLineStyle(hDataXY, _aLStyle[i]);
      GRAPH_AttachData(hItem, hDataXY);
    }
    /* Set graph attributes */
    GRAPH_SetGridDistY(hItem, 25);
    GRAPH_SetGridVis(hItem, 1);
    GRAPH_SetUserDraw(hItem, _UserDraw);
    /* Create and add vertical scale */
    _hScaleV = GRAPH_SCALE_Create( 35, GUI_TA_RIGHT, GRAPH_SCALE_CF_VERTICAL, 25);
    GRAPH_SCALE_SetOff(_hScaleV, 100);
    GRAPH_SCALE_SetFactor(_hScaleV, 0.04f);
    GRAPH_SCALE_SetTextColor(_hScaleV, GUI_RED);
    GRAPH_AttachScale(hItem, _hScaleV);
    /* Create and add horizontal scale */
    _hScaleH = GRAPH_SCALE_Create(70, GUI_TA_HCENTER, GRAPH_SCALE_CF_HORIZONTAL, 50);
    GRAPH_SCALE_SetFactor(_hScaleH, 0.02f);
    GRAPH_SCALE_SetTextColor(_hScaleH, GUI_DARKGREEN);
    GRAPH_AttachScale(hItem, _hScaleH);
    /* Init check boxes */
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK2);
    CHECKBOX_SetText(hItem, "L");
    CHECKBOX_SetState(hItem, 1);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK3);
    CHECKBOX_SetText(hItem, "T");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK4);
    CHECKBOX_SetText(hItem, "R");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK5);
    CHECKBOX_SetText(hItem, "B");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK1);
    CHECKBOX_SetText(hItem, "Grid");
    CHECKBOX_SetState(hItem, 1);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK6);
    CHECKBOX_SetText(hItem, "HScroll");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK7);
    CHECKBOX_SetText(hItem, "VScroll");
    /* Init slider widgets */
    hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER0);
    SLIDER_SetRange(hItem, 0, 10);
    SLIDER_SetValue(hItem, 5);
    SLIDER_SetNumTicks(hItem, 6);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER1);
    SLIDER_SetRange(hItem, 0, 20);
    SLIDER_SetValue(hItem, 5);
    SLIDER_SetNumTicks(hItem, 6);
    /* Init radio widget */
    hItem = WM_GetDialogItem(hDlg, GUI_ID_RADIO0);
    RADIO_SetText(hItem, "3D", 0);
    RADIO_SetText(hItem, "flat", 1);
    RADIO_SetText(hItem, "-", 2);
    /* Init button widget */
    hItem = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
    WM_SetStayOnTop(hItem, 1);
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      /* Id of widget */
    NCode = pMsg->Data.v;                 /* Notification code */
    switch (NCode) {
    case WM_NOTIFICATION_CLICKED:
      switch (Id) {
      case GUI_ID_BUTTON0:
        _ToggleFullScreenMode(hDlg);
        break;
      }
      break;
    case WM_NOTIFICATION_VALUE_CHANGED:
      switch (Id) {
      case GUI_ID_CHECK1:
        /* Toggle grid */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        GRAPH_SetGridVis(hItem, CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK1)));
        break;
      case GUI_ID_CHECK2:
      case GUI_ID_CHECK3:
      case GUI_ID_CHECK4:
      case GUI_ID_CHECK5:
        /* Toggle border */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        GRAPH_SetBorder(hItem,
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK2)) * 40,
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK3)) * 5,
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK4)) * 5,
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK5)) * 5);
        break;
      case GUI_ID_SLIDER0:
        /* Set horizontal grid spacing */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        Value = SLIDER_GetValue(pMsg->hWinSrc) * 10;
        GRAPH_SetGridDistX(hItem, Value);
        GRAPH_SCALE_SetTickDist(_hScaleH, Value);
        if (Value != 50) {
          GRAPH_SCALE_SetNumDecs(_hScaleH, 1);
        } else {
          GRAPH_SCALE_SetNumDecs(_hScaleH, 0);
        }
        break;
      case GUI_ID_SLIDER1:
        /* Set vertical grid spacing */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        Value = SLIDER_GetValue(pMsg->hWinSrc) * 5;
        GRAPH_SetGridDistY(hItem, Value);
        GRAPH_SCALE_SetTickDist(_hScaleV, Value);
        if (Value != 25) {
          GRAPH_SCALE_SetNumDecs(_hScaleV, 1);
        } else {
          GRAPH_SCALE_SetNumDecs(_hScaleV, 0);
        }
        break;
      case GUI_ID_RADIO0:
        /* Set the widget effect */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        switch (RADIO_GetValue(pMsg->hWinSrc)) {
        case 0:
          WIDGET_SetEffect(hItem, &WIDGET_Effect_3D);
          break;
        case 1:
          WIDGET_SetEffect(hItem, &WIDGET_Effect_Simple);
          break;
        case 2:
          WIDGET_SetEffect(hItem, &WIDGET_Effect_None);
          break;
        }
        break;
      case GUI_ID_CHECK6:
        /* Toggle horizontal scroll bar */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK6))) {
          GRAPH_SetVSizeX(hItem, 500);
        } else {
          GRAPH_SetVSizeX(hItem, 0);
        }
        break;
      case GUI_ID_CHECK7:
        /* Toggle vertical scroll bar */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK7))) {
          GRAPH_SetVSizeY(hItem, 300);
        } else {
          GRAPH_SetVSizeY(hItem, 0);
        }
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       MainTask
*/
void MainTask(void);
void MainTask(void) {
  GUI_Init();
  GUI_CURSOR_Show();
  WM_SetDesktopColor(GUI_BLACK);
  #if GUI_SUPPORT_MEMDEV
    WM_SetCreateFlags(WM_CF_MEMDEV);
  #endif
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
  while (1) {
    GUI_Delay(10);
  }
}

/*************************** End of file ****************************/

