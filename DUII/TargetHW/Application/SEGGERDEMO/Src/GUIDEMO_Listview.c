/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2008  SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

***** emWin - Graphical user interface for embedded applications *****
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEMO_Listview.c
Purpose     : Listview demo
----------------------------------------------------------------------
*/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "GUI.h"
#include "GUIDEMO.h"

#if GUI_WINSUPPORT

#include "DIALOG.h"
#include "LISTVIEW.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define PI 3.14159

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
enum {
  JOB_ADDROWS = 1,
  JOB_SETSORT,
  JOB_REVERSE,
  JOB_INCSEL,
  JOB_DECSEL,
  JOB_COLOR,
};

/*********************************************************************
*
*       COL_PROP
*/
typedef struct {
  char * pText;
  int    Width;
  int    Align;
  int (* fpCompare)(const void * p0, const void * p1);
} COL_PROP;

/*********************************************************************
*
*       ACTION_ITEM
*/
typedef struct {
  int Delay;
  int Job;
  const char * pDescription;
} ACTION_ITEM;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*********************************************************************
*
*       _pNames
*/
static const char * _pNames[] = {
  "Adina", "Adis", "Aimee", "Alea", "Alessio", "Aletta", "Alexia", "Alissa", "Amelia",
  "Anakin", "Ania", "Ankea", "Annalies", "Annina", "Annrike", "Arkadius", "Arvin",
  "Aurelia", "Ava", "Beeke", "Benita", "Bennit", "Bent", "Berenike", "Berit", "Birk",
  "Bosse", "Carsta", "Catalina", "Celia", "Charis", "Charlin", "Charlotta", "Cordelia",
  "Corey", "Cornelis", "Corvin", "Cosima", "Crisula", "Darian", "Darina", "Deik", "Derek",
  "Destina", "Devin", "Diane", "Dina", "Dion", "Dorette", "Dorle", "Duncan", "Eddy",
  "Einar", "Elea", "Elin", "Elita", "Elona", "Elouise", "Emilian", "Emina", "Emmeline",
  "Enna", "Estelle", "Etta", "Fabiola", "Fabrice", "Falko", "Felice", "Feline", "Femke",
  "Fiete", "Filippa", "Fine", "Finia", "Finley", "Fleur", "Florentine" "Florenz", "Florin",
  "Floyd", "Fokko", "Fritzi", "Gero", "Gerret", "Gerwin", "Gideon", "Gillian", "Glen",
  "Gunilla", "Hanka", "Heinke", "Helmar", "Henner", "Hennes", "Henrieke", "Immo",
  "India", "Isalie", "Iven", "Jade", "Janice", "Janika", "Janka", "Janneke", "Jannika",
  "Jano", "Jarla", "Jarne", "Jarno", "Jaromir", "Jaron", "Jelana", "Jesper", "Joleen", "Jolie",
  "Jonte", "Joost", "Jördis", "Joris", "Josie", "Josina", "Jula", "Julina", "Jytte", "Kalle",
  "Kennet", "Keno", "Kiana", "Kieran", "Laila", "Laureen", "Laurens", "Laurent",
  "Lauritz", "Leana", "Leandra", "Leandro", "Len", "Lenore", "Leona", "Leonore",
  "Levke", "Lewin", "Lientje", "Lilith", "Line", "Linna", "Linnea", "Lisanne", "Livia",
  "Lorena", "Lorna", "Lovis", "Luana", "Ludger", "Madita", "Mads", "Maiga", "Mailin",
  "Malina", "Manja", "Marieke", "Marielle", "Mariette", "Mariola", "Marla", "Marlena",
  "Marlin", "Marlo", "Marthe", "Martje", "Matz", "Maureen", "Mayra", "Medina",
  "Melda", "Melek", "Melike", "Melinda", "Melis", "Melisa", "Merit", "Mieke", "Mila",
  "Mina", "Mio", "Mirella", "Momke", "Momme", "Morris", "Nantke", "Nelson", "Nika",
  "Norwin", "Oke", "Onno", "Otis", "Ove", "Peer", "Pelle", "Philine", "Philomena",
  "Quentin", "Quint", "Rahel", "Raik", "Riana", "Ricardo", "Rika", "Rixa", "Rolex",
  "Ruwen", "Ryan", "Salma", "Sandrine", "Severin", "Sidonie", "Silas", "Silja", "Simeon",
  "Sinja", "Smilla", "Stig", "Stine", "Talisa", "Tamina", "Tammo", "Tara", "Thyra",
  "Tiana", "Tjalf", "Tjark", "Tjelle", "Tjorben", "Tomma", "Tore", "Vanity", "Violet",
  "Violetta", "Wilko", "Wulf", "Yasmina", "Ylvie", "Younes", "Yves", "Yvette",
};

/*********************************************************************
*
*       _aColProps
*/
static const COL_PROP _aColProps[] = {
  { "Name",    70, GUI_TA_LEFT,    LISTVIEW_CompareText},
  { "Code",    50, GUI_TA_HCENTER, LISTVIEW_CompareText},
  { "Balance", 60, GUI_TA_RIGHT,   LISTVIEW_CompareDec }
};

/*********************************************************************
*
*       _aAction
*/
static const ACTION_ITEM _aAction[] = {
  {   0, JOB_ADDROWS,    "Adding some rows" },
  { 900, 0,              NULL },
  { 500, 0,              "Enable sorting" },
  { 500, 0,              NULL },
  { 600, JOB_SETSORT,    NULL  },
  { 600, JOB_SETSORT,    NULL  },
  { 600, JOB_SETSORT,    NULL  },
  { 500, 0,              "Using reverse/normal\nsorting order" },
  { 500, 0,              NULL },
  { 600, JOB_REVERSE,    NULL  },
  { 600, JOB_REVERSE,    NULL  },
  { 600, JOB_REVERSE,    NULL  },
  { 500, 0,              "Changing selection" },
  { 500, 0,              NULL },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_INCSEL,     NULL  },
  {  50, JOB_DECSEL,     NULL  },
  {  50, JOB_DECSEL,     NULL  },
  {  50, JOB_DECSEL,     NULL  },
  {  50, JOB_DECSEL,     NULL  },
  {  50, JOB_DECSEL,     NULL  },
  { 500, 0,              "Setting some individual colors" },
  { 500, 0,              NULL },
  {  50, JOB_COLOR,      NULL  },
  {1800, 0,              NULL },
};

/*********************************************************************
*
*       _aColor
*/
static const GUI_COLOR _aColor[] = {
  GUI_BLUE        , GUI_GREEN       , GUI_RED         , GUI_CYAN        ,
  GUI_MAGENTA     , GUI_YELLOW      , GUI_LIGHTBLUE   , GUI_LIGHTGREEN  ,
  GUI_LIGHTRED    , GUI_LIGHTCYAN   , GUI_LIGHTMAGENTA, GUI_LIGHTYELLOW ,
  GUI_DARKBLUE    , GUI_DARKGREEN   , GUI_DARKRED     , GUI_DARKCYAN    ,
  GUI_DARKMAGENTA , GUI_DARKYELLOW  , GUI_WHITE       , GUI_LIGHTGRAY   ,
  GUI_GRAY        , GUI_DARKGRAY    , GUI_BLACK       , GUI_BROWN       ,
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _AddRow
*
* Purpose:
*   Inserts or adds a new row of data to the given LISTVIEW
*
* Parameter:
*   hItem  - Handle of LISTVIEW widget
*   Insert - 1 for inserting a row,0 for adding a row
*/
static int _AddRow(WM_HWIN hItem, int Insert) {
  int i, r;
  char * pText[3];
  char acText[3][20] = {0};
  /* Create name field */
  i = rand() % GUI_COUNTOF(_pNames);
  strcpy(acText[0], _pNames[i]);
  /* Create code field */
  for (i = 0; i < 5; i++) {
    acText[1][i] = rand() % 26 + 'A';
  }
  /* Create balance field */
	i = (rand() % 10000) - 5000;
	sprintf(acText[2], "%i", i);
  /* Fill pointer array */
  for (i = 0; i < 3; i++) {
    pText[i] = acText[i];
  }
  /* Add/Insert row */
  if (Insert) {
    r = LISTVIEW_InsertRow(hItem, 0, pText);
  } else {
    r = LISTVIEW_AddRow(hItem, pText);
  }
  return r;
}

/*********************************************************************
*
*       _InitListview
*
* Purpose:
*   Adds some data to the treeview widget
*/
static void _InitListview(WM_HWIN hList) {
  int i;
  HEADER_SetDragLimit(LISTVIEW_GetHeader(hList), 1);
  LISTVIEW_SetAutoScrollV(hList, 1);
  LISTVIEW_SetGridVis(hList, 1);
  LISTVIEW_SetRBorder(hList, 5);
  LISTVIEW_SetLBorder(hList, 5);
  for (i = 0; i < 3; i++) {
    LISTVIEW_AddColumn(hList, _aColProps[i].Width, _aColProps[i].pText, _aColProps[i].Align);
    LISTVIEW_SetCompareFunc(hList, i, _aColProps[i].fpCompare);
  }
  LISTVIEW_EnableSort(hList);
}

/*********************************************************************
*
*       _Action
*
* Purpose:
*   Does some action with the widget
*/
static int _Action(WM_HWIN hList) {
  I32 TimeNext;
  int Index;
  int SortedColumn = -1;
  int Reverse = 0;
  unsigned ColoredRow = 0;
  unsigned ColoredCol = 0;
  int ColorIndex = 0;
  unsigned i, NumRows;

  Index = 0;
  do {
    TimeNext = GUI_GetTime() + _aAction[Index].Delay;
    if (_aAction[Index].pDescription) {
      GUIDEMO_ShowInfo(_aAction[Index].pDescription);
    }
    do {
      if (GUIDEMO_CheckCancel()) {
        return 1;
      }
      GUI_Delay(10);
    } while (GUI_GetTime() < TimeNext);
    switch (_aAction[Index].Job) {
    case JOB_COLOR:
      NumRows = LISTVIEW_GetNumRows(hList);
      for (i = 0; i < NumRows; i++) {
        LISTVIEW_SetItemBkColor(hList, ColoredCol, ColoredRow, LISTVIEW_CI_UNSEL, _aColor[ColorIndex]);
        if (++ColorIndex == GUI_COUNTOF(_aColor)) {
          ColorIndex = 0;
        }
        if (++ColoredCol == LISTVIEW_GetNumColumns(hList)) {
          ColoredCol = 0;
        }
        if (++ColoredRow == NumRows) {
          ColoredRow = 0;
        }
      }
      break;
    case JOB_INCSEL:
      LISTVIEW_IncSel(hList);
      break;
    case JOB_DECSEL:
      LISTVIEW_DecSel(hList);
      break;
    case JOB_REVERSE:
      Reverse ^= 1;
      LISTVIEW_SetSort(hList, SortedColumn, Reverse);
      break;
    case JOB_ADDROWS:
      for (i = 0; i < 40; i++) {
        _AddRow(hList, 0);
      }
      break;
    case JOB_SETSORT:
      if (SortedColumn == -1) {
        SortedColumn = 0;
      } else {
        SortedColumn++;
        if (SortedColumn == (int)LISTVIEW_GetNumColumns(hList)) {
          SortedColumn = 0;
        }
      }
      LISTVIEW_SetSort(hList, SortedColumn, Reverse);
      break;
    }
  } while (++Index < GUI_COUNTOF(_aAction));
  return 0;
}

/*********************************************************************
*
*       _cbWindow
*/
static void _cbWindow(WM_MESSAGE * pMsg) {
  switch (pMsg->MsgId) {
  case WM_PAINT:
    GUI_SetBkColor(GUI_RED);
    GUI_Clear();
    break;
  }
}

/*********************************************************************
*
*       _MoveWindow
*/
static void _MoveWindow(WM_HWIN hWin) {
  int i, xPos, yPos;
  float dx, dy;
  WM_CALLBACK * pOld;

  GUIDEMO_ShowInfo("Moving listview...");
  pOld = WM_SetCallback(WM_HBKWIN, _cbWindow);
  xPos = WM_GetWindowOrgX(hWin);
  yPos = WM_GetWindowOrgY(hWin);
  for (i = 270; i < 630; i += 5) {
    dy = 50 * (float) sin(i * PI / 180);
    dx = 50 * (float) cos(i * PI / 180);
    WM_MoveTo(hWin, (int)(xPos + dx), (int)(yPos - 50 - dy));
    if (GUIDEMO_CheckCancel()) {
      break;
    }
    GUI_Delay(10);
  }
  WM_MoveTo(hWin, xPos, yPos);
  GUIDEMO_Delay(1000);
  WM_SetCallback(WM_HBKWIN, pOld);
}

/*********************************************************************
*
*       _DemoListview
*/
static void _DemoListview(void) {
  WM_HWIN hList;
  int ySize;
  ySize = LCD_GetYSize();
  hList = LISTVIEW_CreateEx(0, 65, 210, ySize - 65, WM_HBKWIN, WM_CF_SHOW, 0, GUI_ID_LISTVIEW0);
  WM_SetFocus(hList);
  _InitListview(hList);
  if (_Action(hList) == 0) {
    _MoveWindow(hList);
  }
  WM_DeleteWindow(hList);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDEMO_Listview
*/
void GUIDEMO_Listview(void) {
  GUIDEMO_ShowIntro("Listview demo",
                    "Shows some features of\n"
                    "the LISTVEW widget");
  _DemoListview(),
  GUIDEMO_NotifyStartNext();
}

#else

void GUIDEMO_Listview(void) {}

#endif

/*************************** End of file ****************************/
