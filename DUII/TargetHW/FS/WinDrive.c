/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

**** emFile file system for embedded applications ****
emFile is protected by international copyright laws. Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : WinDrive.c
Purpose     : Device Driver using Windows I/O function for 
              logical sector access .
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h" 

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "resource.h"
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <winioctl.h>
#include <stdio.h>
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifndef   WD_SECTOR_SIZE
  #define WD_SECTOR_SIZE 512
#endif

#ifdef FS_WD_MAXUNITS
  #define NUM_UNITS          FS_WD_MAXUNITS
#else
  #define NUM_UNITS          4
#endif

#define WIN_SIZE_X              (310)
#define WIN_SIZE_Y              (180)
#define WIN_MIN_SIZE_X          (WIN_SIZE_X)
#define WIN_MIN_SIZE_Y          (WIN_SIZE_Y)
#define WIN_DIST_XY             5


#define ID_CB_DRIVE             ID_COMBO0
#define ID_ED_FILE              ID_EDIT0
#define ID_DRIVE0               300
#define ID_BTN_SEL_FILE         ID_BUTTON0
#define ID_BTN_CREATE_IMG       ID_BUTTON1


#define ID_ED_NUMSECTORS        ID_EDIT1
#define ID_ED_SECTORSIZE        ID_EDIT2

#define REG_PATH                "Software\\Segger\\FS\\Windrive"

/*********************************************************************
*
*       Local data types
*
**********************************************************************
*/
typedef struct {
  HANDLE  hDrive;
  U32     BytesPerSector;
  char    IsDisk;
  char    acName[255];
} WD_PROPS;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static WD_PROPS * _apProps[NUM_UNITS];
static int        _NumUnits;
static HINSTANCE  _hInst;
static HWND       _hWndMain;
static RECT       _rPrev;                   // Contains the previous rectangle of main windows client area. (Used for resizing of dialog items)
static char       _acFile[MAX_PATH];
static U8         _UnitToConfig;
/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _GetInitialWinRect
*/
static void _GetInitialWinRect(RECT * pRect, unsigned Width, unsigned Height) {
  RECT rParent, rDesk;
  int  x, y;

  pRect->left   = 0;
  pRect->top    = 0;
  pRect->right  = Width;
  pRect->bottom = Height;
  GetWindowRect(_hWndMain, &rParent);
  SystemParametersInfo(SPI_GETWORKAREA, 0, &rDesk, 0);
  x = rParent.left + ((rParent.right  - rParent.left) - Width) / 2;
  y = rParent.top  + ((rParent.bottom - rParent.top)  - Height) / 2;
  x = MAX(MIN(x, rDesk.right  - (int)Width),  0);
  y = MAX(MIN(y, rDesk.bottom - (int)Height), 0);
  OffsetRect(pRect, x, y);
}

/*********************************************************************
*
*       _SetDefaultFont
*/
static void _SetDefaultFont(HWND hWnd) {
  HFONT  hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT); 
  SendMessage(hWnd, WM_SETFONT, (WPARAM) hfnt, MAKELPARAM(1, 0)); 
}                                  


/*********************************************************************
*
*       _AddDlgItemEx
*/
static HWND _AddDlgItemEx(HWND hDlg, const char* pClass, const char* pName,
                        int x, int y, int w, int h, int Id, int Flags, int ExFlags)
{
  HWND hWin;

  hWin = CreateWindowEx(ExFlags, pClass, pName, Flags, x, y, w, h, hDlg, NULL, _hInst, NULL);
  _SetDefaultFont(hWin);
  SetWindowLong(hWin, GWL_ID, Id);
  return hWin;
}

/*********************************************************************
*
*       _AddDlgItem
*
*
* Examples:
*   _AddDlgItem(hWnd, "COMBOBOX", NULL,         44,  12,  60, 320, ID_COMBO0, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, WS_EX_CLIENTEDGE);
*   _AddDlgItem(hWnd, "BUTTON",   "&CheckBox",  128, 100, 150,  15, ID_CHECK0,  BS_AUTOCHECKBOX | WS_TABSTOP, 0);
*/
static HWND _AddDlgItem(HWND hDlg, const char* pClass, const char* pName,
                      int x, int y, int w, int h, int Id, int Flags, int ExFlags)
{
  Flags |= WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE;
  return _AddDlgItemEx(hDlg, pClass, pName, x, y, w, h, Id, Flags, ExFlags);
}

/*********************************************************************
*
*       _ComboboxAddString
*/
static void _ComboboxAddString(HWND hCombo, const char* pText, int Id) {
  int NumItems = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
  SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)pText);
  SendMessage(hCombo, CB_SETITEMDATA ,(WPARAM)NumItems, (LPARAM) (DWORD) Id);
}

/*********************************************************************
*
*       _ComboboxFindItem
*/
int _ComboboxFindItem(HWND hCombo, int Id) {
  int i, NumItems = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
  for (i = 0; i < NumItems; i++) {
    if ((SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)i, 0)) == Id) {
      return i;
    }
  }
  return -1;
}

/*********************************************************************
*
*       _SetComboByID
*/
void _SetComboByID(HWND hCombo, int Id, int Default) {
  int Index = _ComboboxFindItem(hCombo, Id);
  if (Index < 0) {
    Index = _ComboboxFindItem(hCombo, Default);
  }
  SendMessage(hCombo, CB_SETCURSEL, ((Index < 0) ? 0 : Index), 0);
}


/*********************************************************************
*
*       _OnNewFile
*/
static void _OnNewFile(HWND hWnd) {
  OPENFILENAME Ofn = {0};
  char         acFileName[MAX_PATH];

  acFileName[0]         = 0;
  Ofn.lStructSize       = sizeof(Ofn);
  Ofn.hwndOwner         = hWnd; 
  Ofn.hInstance         = _hInst; 
  Ofn.lpstrFilter       = "Image Files (*.img, *.bin, *.raw)\0*.img;*.bin;*.raw\0\0"; 
  Ofn.lpstrCustomFilter = NULL; 
  Ofn.nMaxCustFilter    = 0; 
  Ofn.nFilterIndex      = 0; 
  Ofn.lpstrFile         = &acFileName[0]; 
  Ofn.nMaxFile          = sizeof(acFileName); 
  Ofn.lpstrFileTitle    = NULL; 
  Ofn.nMaxFileTitle     = 0; 
  Ofn.lpstrInitialDir   = NULL; 
  Ofn.lpstrTitle        = 0; 
  Ofn.Flags             = OFN_CREATEPROMPT | OFN_PATHMUSTEXIST; 
  Ofn.nFileOffset       = 0; 
  Ofn.nFileExtension    = 0; 
  Ofn.lpstrDefExt       = "img"; 
  Ofn.lCustData         = 0; 
  Ofn.lpfnHook          = NULL; 
  Ofn.lpTemplateName    = NULL;
  GetOpenFileName(&Ofn);
  SetDlgItemText(hWnd, ID_ED_FILE, acFileName);
}

static void _CreateImageFile(const char * sFileImageName, unsigned NumSectors, unsigned SectorSize) {
  HANDLE  hFile;
  U32     NumBytes;
  void  * pMemory;
  U32     NumBytesWritten;

  hFile = CreateFile(sFileImageName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if (hFile == INVALID_HANDLE_VALUE) {
    char ac[200];
    sprintf(ac, "File  %s could not be written", sFileImageName);
    MessageBox(NULL, ac, "_CreateImageFile", MB_OK);
    return;
  }
  NumBytes       = NumSectors * SectorSize;
  pMemory = malloc(NumBytes);
  if (pMemory) {
    memset(pMemory, 0, NumBytes);
    WriteFile(hFile, pMemory, NumBytes, &NumBytesWritten, NULL);
    CloseHandle(hFile);
    free(pMemory);
  }
}

/*********************************************************************
*
*       _OnInitDialog
*/
static BOOL _OnInitCreateImageDialog(HWND hWnd) {
  int     x, y;
  char    ac[256];
  RECT    r = {0};
  HICON   hIcon;
  int     Style;

  //
  // Init dialog window
  //
  _GetInitialWinRect(&r, 270, 160);
  SetWindowPos (hWnd, 0, r.left, r.top, (r.right - r.left), (r.bottom - r.top), SWP_NOZORDER);
//  GetClientRect(hWnd, &_rPrev);
  sprintf(ac, "Create Image file");
  SetWindowText(hWnd, ac);
  Style = GetWindowLong(hWnd, GWL_STYLE); 
  Style |= DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU;
  SetWindowLong(hWnd, GWL_STYLE, Style);
  //
  // Add icon to dialog box
  //
  hIcon = (HICON)LoadImage(_hInst, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
  x =  0; y = 0;
  _AddDlgItem(hWnd, "STATIC",   "Image file name",                15+x,   3+y, 105,  15, IDC_STATIC,        0                                           , 0);
  _AddDlgItem(hWnd, "EDIT",     NULL,                             15+x,  18+y, 210,  23, ID_ED_FILE,        WS_TABSTOP  | ES_AUTOHSCROLL                , WS_EX_CLIENTEDGE);
  _AddDlgItem(hWnd, "BUTTON",   "...",                           225+x,  18+y,  26,  23, ID_BTN_SEL_FILE,   WS_TABSTOP                                  , 0);
  _AddDlgItem(hWnd, "STATIC",   "Number of sectors",              15+x,  50+y, 105,  15, IDC_STATIC,        0                                           , 0);
  _AddDlgItem(hWnd, "EDIT",     NULL,                             15+x,  65+y, 105,  23, ID_ED_NUMSECTORS,  WS_TABSTOP  | ES_AUTOHSCROLL   |  ES_NUMBER , WS_EX_CLIENTEDGE);
  _AddDlgItem(hWnd, "STATIC",   "sectors size",                  184+x,  50+y,  70,  15, IDC_STATIC,        0                                           , 0);
  _AddDlgItem(hWnd, "EDIT",     NULL,                            184+x,  65+y,  70,  23, ID_ED_SECTORSIZE,  WS_TABSTOP  | ES_AUTOHSCROLL   |  ES_NUMBER | ES_READONLY, WS_EX_CLIENTEDGE);
  _AddDlgItem(hWnd, "BUTTON",   "&Create",                        15+x, 103+y,  60,  23, IDOK ,             WS_TABSTOP  | BS_DEFPUSHBUTTON              , 0);
  _AddDlgItem(hWnd, "BUTTON",   "C&ancel",                       195+x, 103+y,  60,  23, IDCANCEL,          WS_TABSTOP  | BS_PUSHBUTTON                 , 0);
  _AddDlgItem(hWnd, "STATIC",   "x",                             152+x,  68+y,  12,  16, IDC_STATIC,        0                                           , 0);
  SetDlgItemText(hWnd, ID_ED_SECTORSIZE, "512");
  SetFocus(GetDlgItem(hWnd, IDOK));
  return FALSE;  // We have initially set the focus, when we return FALSE.
}


/*********************************************************************
*
*       _cbChangeDialog
*/
static BOOL CALLBACK _cbCreateImageDialog(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
  int ItemId = LOWORD(wParam);
  int r      = 0;

  switch (Msg) {
  case WM_INITDIALOG:
    return _OnInitCreateImageDialog(hWnd);
  case WM_CLOSE:
    EndDialog(hWnd, 0);
    return FALSE;
  case WM_COMMAND:
    switch (ItemId) {
    case ID_BTN_SEL_FILE:
      _OnNewFile(hWnd);
      break;
    case IDOK:
      {
        char acBuffer[MAX_PATH];
        unsigned NumSectors;
        unsigned SectorSize;

        GetDlgItemText(hWnd, ID_ED_NUMSECTORS, &acBuffer[0], sizeof(acBuffer));
        NumSectors = atoi(acBuffer);
        if (NumSectors == 0) {
          MessageBox(hWnd, "Wrong number of sectors entered!", "Error", MB_OK | MB_ICONHAND);
          SetDlgItemText(hWnd, ID_ED_NUMSECTORS, "0");
          break;
        }
        GetDlgItemText(hWnd, ID_ED_SECTORSIZE, &acBuffer[0], sizeof(acBuffer));
        SectorSize = atoi(acBuffer);
        if (SectorSize != 512) {
          MessageBox(hWnd, "Sector size must be 512 bytes", "Error", MB_OK | MB_ICONHAND);
          SetDlgItemText(hWnd, ID_ED_SECTORSIZE, "512");
          break;
        }
        GetDlgItemText(hWnd, ID_ED_FILE, &acBuffer[0], sizeof(acBuffer));
        if (acBuffer[0] == 0) {
          MessageBox(hWnd, "Image file name is missing", "Error", MB_OK | MB_ICONHAND);
          _OnNewFile(hWnd);
          break;
        }
        _CreateImageFile(acBuffer, NumSectors, SectorSize);
        r = 1;
        strcpy(_acFile, acBuffer);
        EndDialog(hWnd, r);
        return FALSE;
      }
    case IDCANCEL:
      EndDialog(hWnd, r);
      return FALSE;
    default:
      break;
    }
    break;
  }
  return FALSE;
}

/*********************************************************************
*
*       _InitDriveCombo
*/
static void _InitDriveCombo(HWND hWnd) {
  HWND   hComboBox;
  char   acDir[MAX_PATH];
  char   acRootDrive[MAX_PATH];
  char * p;
  int    i;
  int    Id;
  U32    DriveMask;

  hComboBox = GetDlgItem(hWnd, ID_COMBO0);
  DriveMask = GetLogicalDrives();
  //
  //  Get the drive where windows is installed.
  //  This drive shall not be in the list.
  //
  GetWindowsDirectory(&acDir[0], sizeof(acDir));
  p = strchr(&acDir[0], '\\');
  if (p) {
    *p = 0;
  }
  sprintf(acRootDrive, "\\\\.\\%s", acDir);
  Id = 0;
  //
  // Check and add all available drives
  //
  for (i = 0; i < 26; i++) {
    char ac[20];
    
    sprintf(ac, "\\\\.\\%c:", i + 'A');
    if ((DriveMask & (1 << i))) {
      unsigned DriveType;

      sprintf(acDir, "%s\\", ac);
      DriveType = GetDriveType(acDir);
      if ((DriveType == DRIVE_REMOVABLE) || 
          (DriveType == DRIVE_RAMDISK)   ||
          (DriveType == DRIVE_FIXED))       {
        if (strcmp(acRootDrive, ac)) {
          _ComboboxAddString(hComboBox, ac, ID_DRIVE0 + Id++);
        }
      }
    }
  }
  SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
}

/*********************************************************************
*
*       _UpdateDialog
*/
static void _UpdateDialog(HWND hWnd) {
  if (IsDlgButtonChecked(hWnd, ID_RADIO0) == BST_CHECKED) {
    CheckDlgButton(hWnd, ID_RADIO1, BST_UNCHECKED);
    EnableWindow(GetDlgItem(hWnd, ID_CB_DRIVE), 1);
    EnableWindow(GetDlgItem(hWnd, ID_ED_FILE) , 0);
    EnableWindow(GetDlgItem(hWnd, ID_BTN_SEL_FILE) , 0);
  } else if (IsDlgButtonChecked(hWnd, ID_RADIO1) == BST_CHECKED) {
    CheckDlgButton(hWnd, ID_RADIO0, BST_UNCHECKED);
    EnableWindow(GetDlgItem(hWnd, ID_CB_DRIVE)    , 0);
    EnableWindow(GetDlgItem(hWnd, ID_ED_FILE)     , 1);
    EnableWindow(GetDlgItem(hWnd, ID_BTN_SEL_FILE), 1);
  }
}

/*********************************************************************
*
*       _OnCreateImage
*/
static void _OnCreateImage(HWND hWnd) {
  if (DialogBox(_hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), hWnd, (DLGPROC)_cbCreateImageDialog)) {
    SetDlgItemText(hWnd, ID_ED_FILE, _acFile);
    CheckDlgButton(hWnd, ID_RADIO1, BST_CHECKED);
  } else {
    CheckDlgButton(hWnd, ID_RADIO0, BST_CHECKED);
  }
  _UpdateDialog(hWnd);
}

/*********************************************************************
*
*       _OnInitDialog
*/
static BOOL _OnInitChangeDialog(HWND hWnd) {
  int     x, y;
  char    ac[256];
  RECT    r = {0};
  HICON   hIcon;
  const char * s = NULL;

  //
  // Check if there is a file name available.
  //
  if ((_acFile[0] != '\\') && (_acFile[0] != '\0')) {
    s = _acFile;
  }
  //
  // Init dialog window
  //
  _GetInitialWinRect(&r, WIN_SIZE_X, WIN_SIZE_Y);
  SetWindowPos (hWnd, 0, r.left, r.top, (r.right - r.left), (r.bottom - r.top), SWP_NOZORDER);
  GetClientRect(hWnd, &_rPrev);
  sprintf(ac, "WinDrive configuration");
  SetWindowText(hWnd, ac);
  //
  // Add icon to dialog box
  //
  hIcon = (HICON)LoadImage(_hInst, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
  //
  // Create separator
  //
  x =  2; y = 50;
  _AddDlgItem(hWnd, "STATIC",   NULL,                              1+x,   0+y, 443,   2,  0,                SS_BLACKFRAME      | SS_SUNKEN, 0);
  //
  // Create dialog items
  //
  x = -3; y = 0;
  _AddDlgItem(hWnd, "BUTTON",   "&Drive",                         11+x,  16+y,  55,  15, ID_RADIO0,         BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP  , 0);
  _AddDlgItem(hWnd, "BUTTON",   "&File",                          11+x,  65+y,  55,  15, ID_RADIO1,         BS_AUTORADIOBUTTON | WS_TABSTOP | 0         , 0);
  _AddDlgItem(hWnd, "COMBOBOX", NULL,                             80+x,  16+y, 220, 120, ID_CB_DRIVE,       CBS_DROPDOWNLIST   | WS_TABSTOP | WS_VSCROLL, WS_EX_CLIENTEDGE);
  _AddDlgItem(hWnd, "EDIT",     s,                                80+x,  65+y, 200,  20, ID_ED_FILE,        WS_TABSTOP         | ES_AUTOHSCROLL         , WS_EX_CLIENTEDGE);
  _AddDlgItem(hWnd, "BUTTON",   "...",                           279+x,  66+y,  18,  18, ID_BTN_SEL_FILE,   WS_TABSTOP                                  , 0);
  _AddDlgItem(hWnd, "BUTTON",   "C&reate Image",                 165+x,  95+y,  80,  23, ID_BTN_CREATE_IMG, WS_TABSTOP                                  , 0);
  _AddDlgItem(hWnd, "BUTTON",   "&OK",                           165+x, 125+y,  60,  23, IDOK ,             WS_TABSTOP         | BS_DEFPUSHBUTTON       , 0);
  _AddDlgItem(hWnd, "BUTTON",   "&Cancel",                       235+x, 125+y,  60,  23, IDCANCEL,          WS_TABSTOP         | BS_PUSHBUTTON          , 0);
  _InitDriveCombo(hWnd);
  if (s) {
    CheckDlgButton(hWnd, ID_RADIO1, BST_CHECKED);
  } else {
    CheckDlgButton(hWnd, ID_RADIO0, BST_CHECKED);
  }
  _UpdateDialog(hWnd);
  SetFocus(GetDlgItem(hWnd, IDOK));
  return FALSE;  // We have initially set the focus, when we return FALSE.
}

/*********************************************************************
*
*       _OnSelectFile
*/
static void _OnSelectFile(HWND hWnd) {
  OPENFILENAME Ofn = {0};
  char         acFileName[MAX_PATH];

  acFileName[0]         = 0;
  Ofn.lStructSize       = sizeof(Ofn);
  Ofn.hwndOwner         = hWnd; 
  Ofn.hInstance         = _hInst; 
  Ofn.lpstrFilter       = "Image Files (*.img, *.bin, *.raw)\0*.img;*.bin;*.raw\0\0"; 
  Ofn.lpstrCustomFilter = NULL; 
  Ofn.nMaxCustFilter    = 0; 
  Ofn.nFilterIndex      = 0; 
  Ofn.lpstrFile         = &acFileName[0]; 
  Ofn.nMaxFile          = sizeof(acFileName); 
  Ofn.lpstrFileTitle    = NULL; 
  Ofn.nMaxFileTitle     = 0; 
  Ofn.lpstrInitialDir   = NULL; 
  Ofn.lpstrTitle        = 0; 
  Ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST; 
  Ofn.nFileOffset       = 0; 
  Ofn.nFileExtension    = 0; 
  Ofn.lpstrDefExt       = NULL; 
  Ofn.lCustData         = 0; 
  Ofn.lpfnHook          = NULL; 
  Ofn.lpTemplateName    = NULL;
  GetOpenFileName(&Ofn);
  SetDlgItemText(hWnd, ID_ED_FILE, acFileName);
}

/*********************************************************************
*
*       _cbChangeDialog
*/
static BOOL CALLBACK _cbChangeDialog(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
  int ItemId = LOWORD(wParam);

  switch (Msg) {
  case WM_INITDIALOG:
    return _OnInitChangeDialog(hWnd);
  case WM_DESTROY:
    _hWndMain = NULL;
    break;
  case WM_CLOSE:
    EndDialog(hWnd, 0);
    return FALSE;
  case WM_COMMAND:
    switch (ItemId) {
    case ID_RADIO0:
    case ID_RADIO1:
      _UpdateDialog(hWnd);
      break;
    case ID_BTN_SEL_FILE:
      _OnSelectFile(hWnd);
      break;
    case ID_BTN_CREATE_IMG:
      _OnCreateImage(hWnd);
      break;
    case IDOK:
      {
        int    Item = 0;

        if (IsDlgButtonChecked(hWnd, ID_RADIO0) == BST_CHECKED) {
          Item = ID_CB_DRIVE;
        } else if (IsDlgButtonChecked(hWnd, ID_RADIO1) == BST_CHECKED) {
          Item = ID_ED_FILE;
        }
        GetDlgItemText(hWnd, Item, _acFile, MAX_PATH);
        strcpy(_apProps[_UnitToConfig]->acName, _acFile);
        EndDialog(hWnd, 0);
        return FALSE;
      }
    case IDCANCEL:
      EndDialog(hWnd, 0);
      return FALSE;
    default:
      break;
    }
    break;
  }
  return FALSE;
}

/*********************************************************************
*
*       _GethInstance
*
*/
static HINSTANCE _GethInstance(void) {
 MEMORY_BASIC_INFORMATION mbi; 

 VirtualQuery(_GethInstance, &mbi, sizeof(mbi)); 
 return (HINSTANCE)(mbi.AllocationBase); 
}


/*********************************************************************
*
*       _LoadInfo
*
*/
static int _LoadInfo(U8 Unit, char * sInfo, unsigned MaxLen) {  
  DWORD Type = REG_NONE;
  HKEY  hKey;
  int   r;
  r = RegCreateKey(HKEY_CURRENT_USER, REG_PATH, &hKey);
  if (r == 0) {
    char ac[10];

    sprintf(ac, "%d", Unit);
    r = RegQueryValueEx(hKey, ac, 0, &Type, (U8 *)sInfo, (U32 *)&MaxLen);
    RegCloseKey(hKey);
  }
  return (r ? 1 : ((Type != REG_SZ) ? 1 : 0));
}

/*********************************************************************
*
*       _LoadInfo
*
*/
static void _SaveInfo(U8 Unit, char * sInfo, unsigned MaxLen) {  
  HKEY  hKey;
  int   r;

  r = RegCreateKey(HKEY_CURRENT_USER, REG_PATH, &hKey);
  if (r == 0) {
    char ac[10];

    sprintf(ac, "%d", Unit);
    r = RegSetValueEx(hKey, ac, 0, REG_SZ, (const U8 *)sInfo, strlen(sInfo));
    RegCloseKey(hKey);
  }
}

/*********************************************************************
*
*       _ConfigDialog
*
*/
static void _ConfigDialog(U8 Unit) {  
  char ac[400];

  _hInst = _GethInstance();
  if (_hWndMain == NULL) {
    GetConsoleTitle(ac, sizeof(ac));
    _hWndMain = FindWindow("ConsoleWindowClass", ac);
  }
  InitCommonControls();
  _LoadInfo(Unit, _acFile, sizeof(_acFile));
  sprintf(ac, "win:%d: is configured as \"%s\".\n Do you want to keep this setting?", Unit, _acFile);
  if (MessageBox(_hWndMain, ac, "FS WinDrive Question", MB_YESNO | MB_ICONQUESTION) == IDNO) {
    _UnitToConfig = Unit;
    DialogBox(_hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), _hWndMain, (DLGPROC)_cbChangeDialog);
  } else {
    WD_PROPS * pProps;

    pProps = _apProps[Unit];
    strcpy(pProps->acName, _acFile);
  }
  _SaveInfo(Unit, _acFile, sizeof(_acFile));
}

/*********************************************************************
*
*       _Init
*
*/
static int _Init(WD_PROPS * pProps) {
  int r;

  r = -1;
  if (pProps->hDrive == INVALID_HANDLE_VALUE) {
    if (pProps->acName[0]) {
      DISK_GEOMETRY DiskGeometry;
      DWORD Size;
      U32 BytesPerSector;

      pProps->hDrive = CreateFile(pProps->acName, 
                                    GENERIC_READ    | GENERIC_WRITE, FILE_SHARE_READ,
                                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);
      if (pProps->hDrive != INVALID_HANDLE_VALUE) {
        Size = sizeof(DiskGeometry);
        FS_MEMSET(&DiskGeometry, 0, Size);
        if (DeviceIoControl(pProps->hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &DiskGeometry, Size, &Size, NULL)) {
          U32 Dummy;

          BytesPerSector = DiskGeometry.BytesPerSector;
          pProps->IsDisk = 1;
          //
          //  In order to use windrive with Windows Vista and Windows 7, we need to exclusively lock the volume
          //  otherwise the cannot perform write operation on that volume.
          //  On the operation system this does not hurt.
          //
          if (DeviceIoControl(pProps->hDrive, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &Dummy, NULL)) {
            DeviceIoControl(pProps->hDrive, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &Dummy, NULL);
            
          } else {
            MessageBox(NULL, "Unable to open drive for write operations.\n Device is opened read-only", "WinDrive warning", MB_OK | MB_ICONWARNING);
          }
        } else {
          BytesPerSector = WD_SECTOR_SIZE;
        }
        pProps->BytesPerSector = BytesPerSector;
        r = 0;
      }
    }
  }
  return r;
}

/*********************************************************************
*
*       _AllocIfRequired
*
*/
static void _AllocIfRequired(U8 Unit) {
  WD_PROPS * pProps;

  pProps = _apProps[Unit];
  if (pProps == NULL) {
    pProps  = (WD_PROPS *)FS_AllocZeroed(sizeof(WD_PROPS));   // Alloc memory. This is guaranteed to work by the memory module.
    pProps->hDrive = INVALID_HANDLE_VALUE;
    _apProps[Unit] = pProps;
  }
}

/*********************************************************************
*
*       _Read
*
*  Description:
*    FS driver function. Read sector(s) from the media.
*
*  Parameters:
*    Unit    - Device number.
*    Sector      - Sector to be read from the device.
*    pBuffer     - Pointer to data.
* 
*  Return value:
*    ==0         - Sector has been written to the device.
*    <0          - An error has occurred.
*/

static int _Read(WD_PROPS * pProps, U32 SectorNo, void *pBuffer, U32 NumSectors) {
  U32 NumBytesTransfered;
  U32 NumBytes;
  LARGE_INTEGER FilePos;

  NumBytes = pProps->BytesPerSector * NumSectors;
  FilePos.QuadPart     = (__int64)SectorNo * (__int64)pProps->BytesPerSector; 

  SetFilePointer(pProps->hDrive, FilePos.LowPart, &FilePos.HighPart, FILE_BEGIN);
  ReadFile(pProps->hDrive, pBuffer, NumBytes, &NumBytesTransfered, NULL);
  return (NumBytesTransfered == NumBytes) ? 0 : -1;
}

/*********************************************************************
*
*       _Write
*
*  Description:
*    FS driver function. Write sector(s) to the media.
*
*  Parameters:
*    Unit    - Device number.
*    Sector      - Sector to be written to the device.
*    pBuffer     - Pointer to data to be stored.
* 
*  Return value:
*    ==0         - Sector has been written to the device.
*    <0          - An error has occurred.
*/
static int _Write(WD_PROPS * pProps, U32 SectorNo, const void *pBuffer, U32 NumSectors) {
  U32 NumBytesTransfered;
  U32 NumBytes;
  LARGE_INTEGER FilePos;

  NumBytes = pProps->BytesPerSector * NumSectors;
  FilePos.QuadPart     = (__int64)SectorNo * (__int64)pProps->BytesPerSector; 

  SetFilePointer(pProps->hDrive, FilePos.LowPart, &FilePos.HighPart, FILE_BEGIN);
  WriteFile(pProps->hDrive, pBuffer, NumBytes, &NumBytesTransfered, NULL);
  return (NumBytesTransfered == NumBytes) ? 0 : -1;
}

/*********************************************************************
*
*       Public code (indirectly thru callback)
*
**********************************************************************
*/

/*********************************************************************
*
*       _WD_Read
*
*  Description:
*    FS driver function. Read a sector from the media.
*
*  Parameters:
*    Unit    - Device number.
*    Sector      - Sector to be read from the device.
*    pBuffer     - Pointer to buffer for storing the data.
* 
*  Return value:
*    ==0         - Sector has been read and copied to pBuffer.
*    <0          - An error has occurred.
*/

static int _WD_Read(U8 Unit, U32 Sector, void *pBuffer, U32 NumSectors) {
  WD_PROPS * pProps;

  pProps = _apProps[Unit];
  return _Read(pProps, Sector, pBuffer, NumSectors);
}


/*********************************************************************
*
*       _WD_Write
*
*  Description:
*    FS driver function. Write sector to the media.
*
*  Parameters:
*    Unit    - Device number.
*    Sector      - Sector to be written to the device.
*    pBuffer     - Pointer to data to be stored.
* 
*  Return value:
*    ==0         - Sector has been written to the device.
*    <0          - An error has occurred.
*/
static int _WD_Write(U8 Unit, U32 SectorNo, const void * pData, U32 NumSectors, U8 RepeatSame) {
  U8       * p;
  U32        i;
  int        r;
  U32        BytesPerSector;
  WD_PROPS * pProps;


  r = 0;
  pProps = _apProps[Unit];
  if (RepeatSame) {
    BytesPerSector = pProps->BytesPerSector;
    p = (U8 *)malloc(NumSectors * BytesPerSector);
    if (p) {
      for (i = 0; i < NumSectors; i++) {
        memcpy(p + i * BytesPerSector, pData, BytesPerSector);
      }
      _Write(pProps, SectorNo, p, NumSectors);
      free(p);
    } else {
      for (i = 0; i < NumSectors; i++) {
        if (_Write(pProps, SectorNo++, pData, 1)) {
          r = -1;
          break;
        }
      }
    }
  } else {
    r = _Write(pProps, SectorNo, pData, NumSectors);
  }

  return r;
}

/*********************************************************************
*
*       _WD_GetStatus
*
*  Description:
*    FS driver function. Get status of the media.
*
*  Parameters:
*    Unit                  - Device number.
* 
*  Return value:
*    FS_MEDIA_STATE_UNKNOWN  if the state of the media is unknown.
*    FS_MEDIA_NOT_PRESENT    if media is not present.
*    FS_MEDIA_IS_PRESENT     if media is     present.
*/
static int _WD_GetStatus(U8 Unit) {
  WD_PROPS * pProps;
  int        r;
  U32        Dummy;

  r = FS_MEDIA_NOT_PRESENT;
  pProps = _apProps[Unit];
  if (pProps->hDrive == INVALID_HANDLE_VALUE) {
    if (_Init(pProps) == 0) {
      if (pProps->IsDisk) {
        if (DeviceIoControl(pProps->hDrive, IOCTL_STORAGE_CHECK_VERIFY, NULL, 0, NULL, 0, &Dummy, NULL)) {
          r = FS_MEDIA_IS_PRESENT;
        }
      } else {
        r = FS_MEDIA_IS_PRESENT;
      }
    }

  } else {
    r = FS_MEDIA_IS_PRESENT;
  }
  return r;
}

/*********************************************************************
*
*       _WD_IoCtl
*
*  Description:
*    FS driver function. Execute device command.
*
*  Parameters:
*    Unit        - Unit number.
*    Cmd         - Command to be executed.
*    Aux         - Parameter depending on command.
*    pBuffer     - Pointer to a buffer used for the command.
* 
*  Return value:
*    Command specific. In general a negative value means an error.
*/
static int _WD_IoCtl(U8 Unit, I32 Cmd, I32 Aux, void *pBuffer) {
  FS_DEV_INFO *pInfo;
  U32 NumCylinders;
  U32 NumSectors;
  WD_PROPS * pProps;

  pProps = _apProps[Unit];
  FS_USE_PARA(Aux);
  switch (Cmd) {
  case FS_CMD_GET_DEVINFO:
    if (pBuffer) {
      DISK_GEOMETRY DiskGeometry;
      DWORD Size;
      BOOL  r;
      U32   LastError;

      Size = sizeof(DiskGeometry);
      FS_MEMSET(&DiskGeometry, 0, Size);
      pInfo = (FS_DEV_INFO *)pBuffer;
      r = DeviceIoControl(pProps->hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &DiskGeometry, Size, &Size, NULL);
      LastError = GetLastError();
      //
      // Verify whether we have a image file or a real disk device
      //
      if (r != 0) {
        NumCylinders = (U32)DiskGeometry.Cylinders.QuadPart;
        if (DiskGeometry.SectorsPerTrack == 63) {
          //
          // Some media report inaccurate values (MMC/SD).
          // Since we can not read the Number of sectors from the card info structure, we have to estimate: -6%
          //
          NumCylinders = (NumCylinders + 1) & ~1;
          NumSectors = NumCylinders * DiskGeometry.SectorsPerTrack * DiskGeometry.TracksPerCylinder;
          NumSectors = (U32)(((__int64)NumSectors * 94) / 100);
        } else {
          NumSectors = NumCylinders * DiskGeometry.SectorsPerTrack * DiskGeometry.TracksPerCylinder;
        }
      //
      // If we use an image file, DeviceIoControl 
      // will fail with the reason of having a invalid parameter
      // specified.
      //
      } else if (r == 0 && LastError == ERROR_INVALID_PARAMETER) {
        U32 NumBytesHigh;
        
        NumSectors = GetFileSize(pProps->hDrive, &NumBytesHigh) >> 9;
        NumSectors |= NumBytesHigh << (32 - 9);
        DiskGeometry.BytesPerSector    = WD_SECTOR_SIZE;
        DiskGeometry.TracksPerCylinder = 63;
        DiskGeometry.SectorsPerTrack   = 255;
      //
      // Otherwise operation failed.
      //
      } else {
        return -1;
      }
      pInfo->NumSectors      = NumSectors;
      pInfo->BytesPerSector  = (U16)DiskGeometry.BytesPerSector;
      pInfo->NumHeads        = (U16)DiskGeometry.TracksPerCylinder;
      pInfo->SectorsPerTrack = (U16)DiskGeometry.SectorsPerTrack;
      return 0;
    }
    break;
    case FS_CMD_UNMOUNT:
    case FS_CMD_UNMOUNT_FORCED:
      DeviceIoControl(pProps->hDrive, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &NumCylinders, NULL);
      CloseHandle(pProps->hDrive);
      pProps->hDrive = INVALID_HANDLE_VALUE;
      return 0;
#if FS_SUPPORT_DEINIT
  case FS_CMD_DEINIT:
    FS_FREE(pProps);
    _NumUnits--;
    return 0;
#endif
  }
  return -1;
}

/*********************************************************************
*
*       _WD_InitMedium
*
*  Description:
*    Initialize the specified medium.
*
*  Parameters:
*    Unit    - Unit number.
*
*  Return value:
*/
static int _WD_InitMedium(U8 Unit) {
  WD_PROPS * pProps;

  pProps = _apProps[Unit];
  return _Init(pProps);
}

/*********************************************************************
*
*       _WD_AddDevice
*
*  Description:
*    Initializes the low-level driver object.
*
*  Return value:
*    >= 0                       - Command successfully executed, Unit no.
*    <  0                       - Error, could not add device
*
*/
static int _WD_AddDevice(void) {
  U8         Unit;
  WD_PROPS * pProps;

  if (_NumUnits >= NUM_UNITS) {
    return -1;
  }
  Unit = _NumUnits++;
  _AllocIfRequired(Unit);
  pProps = _apProps[Unit];
  _Init(pProps);
  return Unit;
}

/*********************************************************************
*
*       _WD_GetNumUnits
*/
static int _WD_GetNumUnits(void) {
  return _NumUnits;
}

/*********************************************************************
*
*       _WD_GetDriverName
*/
static const char * _WD_GetDriverName(U8 Unit) {
  return "win";
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WINDRIVE_Configure
*/
void WINDRIVE_Configure(U8 Unit, const char * sWindowsDriveName) {
  WD_PROPS * pProps;

  if (Unit >= _NumUnits) {
    return;
  }
  _AllocIfRequired(Unit);
  pProps = _apProps[Unit];

  if (sWindowsDriveName == NULL) {
    _ConfigDialog(Unit);
  } else {
    strcpy(pProps->acName, sWindowsDriveName);
  }
}

/*********************************************************************
*
*       Public const
*
**********************************************************************
*/
const FS_DEVICE_TYPE FS_WINDRIVE_Driver = {
  _WD_GetDriverName,
  _WD_AddDevice,
  _WD_Read,
  _WD_Write,
  _WD_IoCtl,
  _WD_InitMedium,
  _WD_GetStatus,
  _WD_GetNumUnits
};

#endif  // _WIN32

/*************************** End of file ****************************/
