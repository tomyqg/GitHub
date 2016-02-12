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
File    : SeggerDemo.c
Purpose : SEGGER product presentation
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "SysConf.h"

#include "GUI.h"


/*********************************************************************
*
*       Configuration of included middleware components for simulation
*/

#ifndef INCLUDE_IP
  #ifdef _WINDOWS
    #define INCLUDE_IP       (0)
  #endif
#endif

#ifndef INCLUDE_USB
  #ifdef _WINDOWS
    #define INCLUDE_USB      (0)
  #endif
#endif


/*********************************************************************
*
*       Configuration of demo applications included/excluded in SEGGERDEMO
*/
#ifndef   INCLUDE_INCONVIEWDEMO
  #define INCLUDE_INCONVIEWDEMO      (1)
#endif

#ifndef   INCLUDE_LISTVIEW
  #define INCLUDE_LISTVIEW           (1)
#endif

#ifndef   INCLUDE_GRAPH
  #define INCLUDE_GRAPH              (1)
#endif

#ifndef   INCLUDE_AUTOMOTIVE
  #define INCLUDE_AUTOMOTIVE         (1)
#endif

#ifndef   INCLUDE_NAVIGATIONDEMO
  #define INCLUDE_NAVIGATIONDEMO     (1)
#endif

#ifndef   INCLUDE_TREEVIEWDEMO
  #define INCLUDE_TREEVIEWDEMO       (1)
#endif

#ifndef   INCLUDE_BITMAPDEMO
  #define INCLUDE_BITMAPDEMO         (1)
#endif

#ifndef   INCLUDE_CURSORDEMO
  #define INCLUDE_CURSORDEMO         (1)
#endif

#ifndef   INCLUDE_POLYGONDEMO
  #define INCLUDE_POLYGONDEMO        (1)
#endif

#ifndef   INCLUDE_CIRCLEDEMO
  #define INCLUDE_CIRCLEDEMO         (1)
#endif

#ifndef   INCLUDE_JLINKPRESENTATION
  #define INCLUDE_JLINKPRESENTATION  (1)
#endif

#ifndef   INCLUDE_PRESENTATION
  #define INCLUDE_PRESENTATION       (0)
#endif

/*********************************************************************
*
*       Configuration of included middleware components
*/
#ifndef   INCLUDE_IP
  #define INCLUDE_IP       (1)
#endif

#ifndef   INCLUDE_USB
  #define INCLUDE_USB      (1)
#endif

#ifndef   INCLUDE_FS
  #define INCLUDE_FS       (1)
#endif

#ifndef INCLUDE_VNC
  #if ((INCLUDE_IP == 1) || defined(_WINDOWS))
    #define INCLUDE_VNC    (1)
  #else
    #define INCLUDE_VNC  (0)
  #endif
#endif

/*********************************************************************
*
*       Includes
*/
#include <stddef.h>
#include "GUI.h"
#if GUI_WINSUPPORT
  #include "PROGBAR.h"
  #include "LISTBOX.h"
  #include "FRAMEWIN.h"
  #include "BUTTON.h"
#endif
#include <string.h>
#include "GUIDEMO.h"

#include "TaskPrio.h"
#include "BSP.h"
#include <stdio.h>

#ifndef _WINDOWS
  #include "RTOS.h"
#endif

#if (INCLUDE_IP == 1)
  #include "IP.h"
  #include "WM.h"
#endif
#if (INCLUDE_VNC == 1)
  #include "GUI_VNC.h"
#endif

#if ((INCLUDE_USB == 1) && (INCLUDE_FS == 1))
  #include "USB.h"
  #include "USB_MSD.h"
  #include "FS.h"
#endif

/*********************************************************************
*
*       Configuration of SEGGERDEMO
*/
#define SEGGERDEMO_DELAY 5000  // 5 sec

#define TRIALWIN_XSIZE  (200)
#define TRIALWIN_YSIZE  (70)
#define TRIALWIN_XPOS   ((LCD_GetXSize() / 2) - (LCD_GetXSize() / 2))
#define TRIALWIN_YPOS   ((LCD_GetYSize() / 2) - (LCD_GetYSize() / 2))

/******* End of configuration ***************************************/


/*********************************************************************
*
*       Defines non-configurable
*
**********************************************************************
*/
#define countof(Obj) (sizeof(Obj)/sizeof(Obj[0]))


/*********************************************************************
*
*       Externals
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_SeggerLogo_300x150_565c;
#if (INCLUDE_PRESENTATION == 1)
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_SeggerProducts_320x180_565c;
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_SeggerProductsCombined_320x233_565c;
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_embOS_52x45;
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_embOSIP_64x46;
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_embOSIPLayer_301x124;
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_emWin_64x52;
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_emUSB_57x18;
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_emFile_305x167_565c;
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_JLink_307x240_565c;
extern GUI_CONST_STORAGE GUI_BITMAP bmIMAGE_SeggerLogo_100x50;
#endif

#if (INCLUDE_IP == 1)
extern void WebserverTask(void);
extern void FTPServerTask(void);
#endif

/*********************************************************************
*
*       Static variables
*
**********************************************************************
*/

#if (INCLUDE_IP == 1)
static OS_STACKPTR int _FTPServer_Stack[4096];
static OS_TASK         _FTPServer_TCB;
static OS_STACKPTR int _WEBServer_Stack[4096];
static OS_TASK         _WEBServer_TCB;
static OS_STACKPTR int _IP_Stack[512];
static OS_TASK         _IP_TCB;
static OS_STACKPTR int _IPRx_Stack[256];
static OS_TASK         _IPRx_TCB;
static OS_STACKPTR int _EthWindow_Stack[1024];
static OS_TASK         _EthWindow_TCB;

static FRAMEWIN_Handle _hEthFrameWin;
static FRAMEWIN_Handle _hEthWin1;
static U32             _IPAddr;
static char            _acIP[16];

#if (INCLUDE_PRESENTATION == 1)
  static WM_HWIN         _hWindow1;
  static char            _acTextBuffer[72];
#endif

#endif

#if ((INCLUDE_USB == 1) && (INCLUDE_FS == 1))
static OS_STACKPTR int _USB_MSD_Stack[1024];
static OS_TASK         _USB_MSD_TCB;
#endif

#if (!defined _WINDOWS) && (((INCLUDE_IP == 1) || (INCLUDE_USB == 1) || (INCLUDE_FS == 1)))
static OS_STACKPTR int _TrialWindow_Stack[1024];
static OS_TASK         _TrialWindow_TCB;
#endif

#if GUI_WINSUPPORT
  static BUTTON_Handle   _ahButton[2];
  static FRAMEWIN_Handle _ahFrameWin[3];
  static int             _ButtonSizeX,      _ButtonSizeY;
  static FRAMEWIN_Handle _ahInfoWin[2];
  static const char* _sInfo;
#endif

static int         _iTest;
static int         _iTestMinor;
static char        _CmdNext;
static const char* _sExplain;

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/

/*********************************************************************
*
*       Routine table
*
**********************************************************************

  The routines to be called are listed here. The advantage to calling
  directly is that we can execute some code before and after.
*/

typedef void fVoidVoid(void);

static fVoidVoid * const _apfTest[] = {
  GUIDEMO_HardwareInfo,
  GUIDEMO_Speed,
  #if INCLUDE_INCONVIEWDEMO
    GUIDEMO_IconView,
  #endif
  #if INCLUDE_TREEVIEWDEMO
    GUIDEMO_Treeview,
  #endif
  #if INCLUDE_LISTVIEW
    GUIDEMO_Listview,
  #endif
  GUIDEMO_VScreen,
  #if GUI_SUPPORT_MEMDEV
    #if INCLUDE_GRAPH
      GUIDEMO_Graph,
    #endif
  #endif
  #if INCLUDE_BITMAPDEMO
    GUIDEMO_Bitmap,
  #endif
  #if INCLUDE_CURSORDEMO
    GUIDEMO_Cursor,
  #endif
  GUIDEMO_ShowInternationalFonts,
  GUIDEMO_ShowColorBar,
  GUIDEMO_ShowColorList,
  GUIDEMO_DemoLUT,
  #if INCLUDE_POLYGONDEMO
    GUIDEMO_DemoPolygon,
  #endif
  #if INCLUDE_CIRCLEDEMO
    GUIDEMO_Circle,
  #endif
  #if INCLUDE_AUTOMOTIVE
    GUIDEMO_Automotive,
  #endif
  #if INCLUDE_NAVIGATIONDEMO
    GUIDEMO_Navigation,
  #endif
  #if GUI_WINSUPPORT
    GUIDEMO_Dialog,
    GUIDEMO_DemoProgBar,
    GUIDEMO_DemoFrameWin,
    #if GUI_SUPPORT_TOUCH
      GUIDEMO_Touch,
    #endif
  #endif
  0
};

static void _UpdateCmdWin(void) {
  #if GUI_WINSUPPORT
    WM_InvalidateWindow(_ahInfoWin[1]);
  #endif
}

/*********************************************************************
*
*       _cbCmdWin
*/
#if (GUI_WINSUPPORT)
static void _cbCmdWin(WM_MESSAGE* pMsg) {
  switch (pMsg->MsgId) {
  case WM_PAINT:
    /* Update info in command window */
    GUI_SetBkColor(GUI_GRAY);
    GUI_Clear();
    GUI_DispStringAt("Demo ", 0, 0);
    GUI_DispDecMin(_iTest + 1);
    GUI_DispString(".");
    GUI_DispDecMin(_iTestMinor);
    GUI_DispString("/");
    GUI_DispDecMin(countof(_apfTest));
  default:
    WM_DefaultProc(pMsg);
  }
}
#endif

/*********************************************************************
*
*       _UpdateInfoWin
*/
static void _UpdateInfoWin(void) {
  #if GUI_WINSUPPORT
    GUI_RECT rClient;
    WM_HWIN hWin;
    GUI_CONTEXT ContextOld;
    GUI_SaveContext(&ContextOld);
    /* Update info  window */
    WM_ShowWindow(_ahInfoWin[0]);
    WM_ShowWindow(_ahFrameWin[0]);
    hWin = WM_SelectWindow(_ahInfoWin[0]);
    GUI_GetClientRect(&rClient);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();
    GUI_SetColor(GUI_BLACK);
    GUI_SetFont(&GUI_Font13_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringHCenterAt(_sInfo, (rClient.x1 - rClient.x0) / 2, 0);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_SetFont(&GUI_Font8_ASCII);
    GUI_DispStringAt(_sExplain, 0, 20);
    WM_SelectWindow(hWin);
    GUI_RestoreContext(&ContextOld);
  #endif
}

#ifndef _WINDOWS
#if ((INCLUDE_IP == 1) || (INCLUDE_USB == 1) || (INCLUDE_FS == 1))
/*********************************************************************
*
*       _TrialWinCallBack()
*/
static void _TrialWinCallBack(WM_MESSAGE* pMsg) {
  GUI_RECT Rect = {0, 0, TRIALWIN_XSIZE, TRIALWIN_YSIZE};
  switch (pMsg->MsgId) {
  case WM_PAINT:
    GUI_SetBkColor(GUI_LIGHTGRAY);
    GUI_SetColor(GUI_BLACK);
    GUI_Clear();
    GUI_DispStringInRect("\n"
                         "You have reached the\n"
                         "15 minutes time limit.\n"
                         "\n"
                         "Thank you for evaluating\n"
                         "SEGGER software.",
                         &Rect, GUI_TA_CENTER);
    break;
  }
}

/*********************************************************************
*
*       _TrialWindowTask()
*/
static void _TrialWindowTask(void) {
  OS_U32 t;

  t = 880000;
  do {
	OS_Delay(10000);
  } while (t > OS_GetTime());
  FRAMEWIN_CreateEx(TRIALWIN_XPOS, TRIALWIN_YPOS, TRIALWIN_XSIZE, TRIALWIN_YSIZE, WM_HBKWIN, WM_CF_SHOW | WM_CF_STAYONTOP, 0, 0, "Trial limit reached", _TrialWinCallBack);
  WM_Exec();
  OS_SetPriority(&_TrialWindow_TCB, 255);
  while(1);
}
#endif
#endif

#if (INCLUDE_IP == 1)
/*********************************************************************
*
*       _EthWinCallBack()
*/
static void _EthWinCallBack(WM_MESSAGE* pMsg) {
  static char acText[50];
  switch (pMsg->MsgId) {
  case WM_PAINT:
    if (IP_IFaceIsReady() == 0) {
       strcpy(acText, "IP: Not connected");
    } else {
      _IPAddr = IP_GetIPAddr(0);
      IP_PrintIPAddr(_acIP, _IPAddr, sizeof(_acIP));
      strcpy(acText,"IP: ");
      strcat(acText, _acIP);
    }
    GUI_SetBkColor(GUI_LIGHTGRAY);
    GUI_SetColor(GUI_BLACK);
    GUI_Clear();
    GUI_GotoXY(0, 3);
    GUI_DispString(acText);
    break;
  }
}

/*********************************************************************
*
*       _EthWindowTask()
*/
static void _EthWindowTask(void) {
  int IPState = -1;
  _hEthFrameWin = FRAMEWIN_CreateEx(0, 210, 120, 30, WM_HBKWIN, WM_CF_SHOW | WM_CF_STAYONTOP, 0, 0, "Ethernet", _EthWinCallBack);
  WM_Exec();
  _hEthWin1 = WM_GetClientWindow(_hEthFrameWin);
  for (;;) {
    if (IP_IFaceIsReady() != IPState) {
      WM_InvalidateWindow(_hEthWin1);
    }
    IPState = IP_IFaceIsReady();
    OS_Delay(500);
  }
}
#endif  // (INCLUDE_IP == 1)

#if ((INCLUDE_USB == 1) && (INCLUDE_FS == 1))

/*********************************************************************
*
*       _AddMSD
*
*  Function description
*    Add mass storage device to USB stack
*
*  Notes:
*   (1)  -   This examples uses the internal driver of the file system.
*            The module intializes the low-level part of the file system if necessary.
*            If FS_Init() was not previously called, none of the high level functions
*            such as FS_FOpen, FS_Write etc will work.
*            Only functions that are driver related will be called.
*            Initialization, sector read/write, retrieve device information.
*            The following members of the DriverData are used as follows:
*              DriverData.pStart       = Index no. of the volume that shall be used.
*              DriverData.NumSectors   = Number of sectors to be used - 0 means auto-detect.
*              DriverData.StartSector  = The first sector that shall be used.
*              DriverData.SectorSize will not be used.
*/
static void _AddMSD(void) {
  static U8 _abOutBuffer[USB_MAX_PACKET_SIZE];
  USB_MSD_INIT_DATA     InitData;
  USB_MSD_INST_DATA     InstData;

  InitData.EPIn  = USB_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut = USB_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _abOutBuffer, USB_MAX_PACKET_SIZE);
  USB_MSD_Add(&InitData);
  //
  // Add logical unit 0:
  //
  memset(&InstData, 0,  sizeof(InstData));
  InstData.pAPI                    = &USB_MSD_StorageByName;    // s. Note (1)
  InstData.DriverData.pStart       = "";
  USB_MSD_AddUnit(&InstData);
}

/*********************************************************************
*
*       _USBMSDTask()
*/
static void _USBMSDTask(void) {
  USB_Init();
  FS_Init();
  if (FS_IsLLFormatted("") == 0) {
    FS_X_Log("Low level formatting");
    FS_FormatLow("");          /* Erase & Low-level  format the volume */
  }
  if (FS_IsHLFormatted("") == 0) {
    FS_X_Log("High level formatting\n");
    FS_Format("", NULL);       /* High-level format the volume */
  }
  FS_Unmount("");
  _AddMSD();
  USB_Start();
  while (1) {
    //
    // Wait for configuration
    //
    while ((USB_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      USB_OS_Delay(50);
      BSP_ToggleLED(0);
    }
    USB_MSD_Task();
  }
}

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/


/*********************************************************************
*
*       Get information that are used during enumeration
*/

/*********************************************************************
*
*       USB_GetVendorName
*
*  Function description
*    Returns vendor Id
*/
U16 USB_GetVendorId(void) {
  return 0x8765;
}

/*********************************************************************
*
*       USB_GetProductId
*
*  Function description
*    Returns the product Id
*/
U16 USB_GetProductId(void) {
  return 0x1000;
}

/*********************************************************************
*
*       USB_GetVendorName
*
*  Function description
*    Returns vendor name.
*/
const char * USB_GetVendorName(void) {
  return "Vendor";
}

/*********************************************************************
*
*       USB_GetProductName
*
*  Function description
*    Returns product name
*/
const char * USB_GetProductName(void) {
  return "MSD device";
}

/*********************************************************************
*
*       USB_GetSerialNumber
*
*  Function description
*    Returns serial number
*/
const char * USB_GetSerialNumber(void) {
  return "13245678";
}

/*********************************************************************
*
*       String information routines when inquiring the volume
*/
/*********************************************************************
*
*       USB_MSD_GetVendorName
*/
const char * USB_MSD_GetVendorName(U8 Lun) {
  return "Vendor";
}

/*********************************************************************
*
*       USB_MSD_GetProductName
*/
const char * USB_MSD_GetProductName(U8 Lun) {
  return "MSD Volume";
}

/*********************************************************************
*
*       USB_MSD_GetProductVer
*/
const char * USB_MSD_GetProductVer(U8 Lun) {
  return "1.00";
}

/*********************************************************************
*
*       USB_MSD_GetSerialNo
*/
const char * USB_MSD_GetSerialNo(U8 Lun) {
  return "134657890";
}

#endif  // #if ((INCLUDE_USB == 1) && (INCLUDE_FS == 1))

/*********************************************************************
*
*       GUIDEMO_SetBkColor
*/
GUI_COLOR GUIDEMO_SetBkColor(GUI_COLOR Color) {
  GUI_COLOR r;
  if (GUI_ColorIsAvailable(GUI_RED)) {
    r = Color;
  } else {
    r = GUI_BLACK;
  }
  GUI_SetBkColor(r);
  return r;
}

/*********************************************************************
*
*       GUIDEMO_SetColor
*/
GUI_COLOR GUIDEMO_SetColor(GUI_COLOR Color) {
  GUI_COLOR r;
  if (GUI_ColorIsAvailable(GUI_RED)) {
    r = Color;
  } else {
    r = GUI_WHITE;
  }
  GUI_SetColor(r);
  return r;
}

/*********************************************************************
*
*       GUIDEMO_Delay
*/
int GUIDEMO_Delay(int t) {
  int r = 0;
  #if GUI_WINSUPPORT
    U32 value, tm;
    PROGBAR_Handle hProg;
    WM_SetForegroundWindow(_ahButton[0]);
    hProg = (t > 100) ? PROGBAR_CreateAsChild(10, 10, 60, 7, _ahInfoWin[1], 0,
                                        WM_CF_SHOW | WM_CF_STAYONTOP| WM_CF_MEMDEV) : 0;
    PROGBAR_SetMinMax(hProg, 0, t);
    tm = GUI_GetTime();
    do {
      value = GUI_GetTime() - tm;
      PROGBAR_SetValue(hProg, value);
      WM_ExecIdle();
      GUI_Delay(50);
    } while ((value < (U32)t) && !GUIDEMO_CheckCancel());
    PROGBAR_Delete(hProg);
    _UpdateCmdWin();
    WM_ExecIdle();
  #else
    GUI_Delay(t);
  #endif
  return r;
}

/*********************************************************************
*
*       GUIDEMO_Wait
*/
void GUIDEMO_Wait(void) {
  GUIDEMO_Delay(4000);
  GUIDEMO_NotifyStartNext();
}

/*********************************************************************
*
*       GUIDEMO_CheckCancel
*/
int GUIDEMO_CheckCancel(void) {
  int c = GUI_GetKey();
  #if GUI_WINSUPPORT
    WM_ExecIdle();
  #endif
  if ((c == 'n') || (c=='N'))
    _CmdNext =1;
  if ((c == 'h') || (c=='H')) {
    GUI_Delay(500);
    do {
      GUI_Delay(10);
      #if GUI_WINSUPPORT
        WM_ExecIdle();
      #endif
      c = GUI_GetKey();
    } while (!c);
  }
  return _CmdNext;
}

/*********************************************************************
*
*       GUIDEMO_ShowInfo
*/
void GUIDEMO_ShowInfo(const char* s) {
  _sExplain = s;
  _UpdateInfoWin();
}

/*********************************************************************
*
*       GUIDEMO_HideInfoWin
*/
void GUIDEMO_HideInfoWin(void) {
  #if GUI_WINSUPPORT
    WM_HideWindow(_ahFrameWin[0]);
  #endif
}

/*********************************************************************
*
*       GUIDEMO_ShowControlWin
*/
void GUIDEMO_ShowControlWin(int Show) {
  #if GUI_WINSUPPORT
    if (Show) {
      WM_ShowWindow(_ahFrameWin[1]);
    }else {
      WM_HideWindow(_ahFrameWin[1]);
    }
  #endif
}

/*********************************************************************
*
*       GUIDEMO_NotifyStartNext
*/
void GUIDEMO_NotifyStartNext(void) {
  _CmdNext = 0;
  _UpdateCmdWin();
  _iTestMinor++;
}

/*********************************************************************
*
*       GUIDEMO_WaitKey
*/
int GUIDEMO_WaitKey(void) {
  int r = 0;
  U32 tMax = GUI_GetTime() + 4000;
  U32 tDiff;
  #if GUI_WINSUPPORT
    PROGBAR_Handle hProg = PROGBAR_Create(LCD_GetXSize() - 70,
                                          LCD_GetYSize() - 40,
                                          80, 5, WM_CF_SHOW);
  #endif
  while (tDiff = tMax - GUI_GetTime(), (tDiff > 0) && !GUIDEMO_CheckCancel()) {
    if ((r = GUI_GetKey()) != 0)
      break;
  }
  #if GUI_WINSUPPORT
    PROGBAR_Delete(hProg);
    WM_ExecIdle();
  #endif
  return r;
}

/*********************************************************************
*
*       GUIDEMO_ShowIntro
*/
void GUIDEMO_ShowIntro(const char * s, const char * sExp) {
  GUI_CONTEXT ContextOld;
  GUI_SaveContext(&ContextOld);
#if GUI_WINSUPPORT
  _sInfo = s;
#endif
  _sExplain = sExp;
  GUI_SetDefault();
  GUIDEMO_HideInfoWin();
  GUIDEMO_SetBkColor(GUI_BLUE);
  GUI_SetColor(GUI_WHITE);
  GUI_Clear();
  #if !(GUIDEMO_TINY)
    GUI_SetFont(&GUI_FontComic18B_ASCII);
    GUI_DispStringHCenterAt(s, LCD_GetXSize() / 2, LCD_GetYSize() / 3 - 10);
  #else
    GUI_SetFont(&GUI_Font13B_ASCII);
    GUI_DispStringHCenterAt(s, LCD_GetXSize() / 2, LCD_GetYSize() / 3 - 18);
  #endif
  if (_sExplain) {
    GUI_SetFont(&GUI_Font8_ASCII);
    #if !(GUIDEMO_TINY)
      GUI_DispStringHCenterAt(_sExplain, LCD_GetXSize() / 2, LCD_GetYSize() / 2 - 10);
    #else
      GUI_DispStringHCenterAt(_sExplain, LCD_GetXSize() / 2, LCD_GetYSize() / 2 - 14);
    #endif
  }
  GUIDEMO_Delay(_sExplain ? 2000 : 4000);
#if GUI_WINSUPPORT
  _UpdateInfoWin();
  _UpdateCmdWin();
#endif
  GUI_RestoreContext(&ContextOld);
  #if GUI_WINSUPPORT
    WM_ExecIdle();
  #endif
  GUIDEMO_NotifyStartNext();
  GUIDEMO_SetBkColor(GUI_RED);
  GUI_Clear();
}

/*********************************************************************
*
*       GUIDEMO_main()
*
*/
void GUIDEMO_main(void) {
  #if GUI_WINSUPPORT
    int i;
    int xSize, ySize, xPos, yPos;
  #endif
  #if GUI_SUPPORT_CURSOR
    GUI_CURSOR_Show();
  #endif
  GUI_Clear();
  #if GUI_WINSUPPORT
    WM_SetCreateFlags(WM_CF_MEMDEV);  /* Automatically use memory devices on all windows */
    /* Calculate position and size of FrameWin[1] */
    _ButtonSizeX = 30;
    _ButtonSizeY = 20;
    /* Create the control window incl. buttons */
    _ahFrameWin[1] = FRAMEWIN_Create("Control", &_cbCmdWin, WM_CF_SHOW | WM_CF_STAYONTOP,
                                                LCD_GetXSize() - 80, LCD_GetYSize() - 60, 80, 60);
    _ahInfoWin[1] = WM_GetClientWindow(_ahFrameWin[1]);
    _ahButton[0] = BUTTON_CreateAsChild(4, 20, _ButtonSizeX, _ButtonSizeY, _ahInfoWin[1],
                                        'H' , WM_CF_SHOW | WM_CF_STAYONTOP | WM_CF_MEMDEV);
    _ahButton[1] = BUTTON_CreateAsChild(40, 20, _ButtonSizeX, _ButtonSizeY, _ahInfoWin[1],
                                        'N' , WM_CF_SHOW | WM_CF_STAYONTOP | WM_CF_MEMDEV);
    BUTTON_SetText(_ahButton[0], "Halt");
    BUTTON_SetText(_ahButton[1], "Next");
    _UpdateCmdWin();
    WM_ExecIdle();
  #endif
  /* Show Intro */
  GUIDEMO_Intro();
  #if GUI_WINSUPPORT
    /* Calculate position and size of FrameWin[0] */
    xSize = LCD_GetXSize() / 2;
    ySize = 65;
    xPos  = LCD_GetXSize() - xSize;
    yPos  = 0;
    /* Create info window and run the individual demos */
    _ahFrameWin[0] = FRAMEWIN_Create("emWin Demo", NULL, WM_CF_STAYONTOP,
                                     xPos, yPos, xSize, ySize);
    _ahInfoWin[0] = WM_CreateWindowAsChild(0, 0, 0, 0, WM_GetFirstChild(_ahFrameWin[0]), WM_CF_SHOW | WM_CF_STAYONTOP, 0, 0);
    WM_ExecIdle();
  #endif
  /* Run the individual demos !  */
  for (_iTest = 0; _apfTest[_iTest]; _iTest++) {
    GUI_CONTEXT ContextOld;
    GUI_SaveContext(&ContextOld);
    _iTestMinor = 0;
    _UpdateCmdWin();
    (*_apfTest[_iTest])();
    _CmdNext = 0;
    GUI_RestoreContext(&ContextOld);
  }
  /* Cleanup */
  #if GUI_WINSUPPORT
    for (i = 0; i < countof(_ahFrameWin); i++) {
      WM_DeleteWindow(_ahFrameWin[i]);
    }
  #endif
}


/*********************************************************************
*
*       _ShowPresentation
*
*/
static void _ShowPresentation(void) {
#if (INCLUDE_PRESENTATION == 1)
  GUI_RECT Rect4 = {100, 55, 320, 240};
  GUI_RECT Rect5 = {100, 55, 320, 240};
  GUI_RECT Rect6 = {100, 55, 320, 240};
  GUI_RECT Rect8 = {100, 55, 320, 240};
  GUI_RECT Rect9 = {100, 55, 320, 240};
  GUI_RECT Rect12 = {0, 61, 320, 149};
  GUI_RECT Rect13 = {0, 150, 320, 210};
  //
  // Presentation Page 2: SEGGER Products
  //
  GUI_Clear();
  GUI_FillRect(0, 0, 320, 240);
  GUI_DrawBitmap(&bmIMAGE_SeggerProducts_320x180_565c, 0, 21);
  GUI_Delay(SEGGERDEMO_DELAY);
  //
  // Presentation Page 3: SEGGER Products Combined
  //
  GUI_Clear();
  GUI_FillRect(0, 0, 320, 240);
  GUI_DrawBitmap(&bmIMAGE_SeggerProductsCombined_320x233_565c, 0, 5);
  GUI_Delay(SEGGERDEMO_DELAY);
  //
  // Presentation Page 4: embOSFeatures
  //
  GUI_Clear();
  GUI_FillRect(0, 0, 320, 240);
  GUI_SetBkColor(GUI_WHITE);
  GUI_SetColor(GUI_BLACK);

  GUI_DrawBitmap(&bmIMAGE_embOS_52x45, 22, 35);

  GUI_SetFont(&GUI_Font20B_ASCII);
  GUI_GotoXY(5, 5);
  GUI_DispString("embOS");
  GUI_GotoXY(100, 35);
  GUI_DispString("Features");

  GUI_SetFont(&GUI_Font13B_ASCII);
  GUI_GotoXY(5, 22);
  GUI_DispString("Real Time OS");
  GUI_GotoXY(100, 55);
  GUI_DispStringInRect("- Zero interrupt latency\n"
                       "- Fast context switch\n"
                       "- Independent of number of tasks\n"
                       "- Small footprint\n"
                       "   ROM size  3KB (Kernel only,\n"
                       "                      ARM7/IAR compiler)\n"
                       "   RAM size  Kernel: app. 51bytes\n"
                       "                      Task: 32bytes +\n"
                       "                                 Stack per task"
                       , &Rect4, GUI_TA_LEFT);

  GUI_SetFont(&GUI_Font16B_ASCII);
  GUI_SetColor(GUI_RED);
  GUI_GotoXY(15, 180);

  GUI_DispString("One of the smallest and most efficient kernels!");
  GUI_Delay(SEGGERDEMO_DELAY);
  //
  // Presentation Page 5: embOS/IP
  //
  GUI_SetColor(GUI_WHITE);
  GUI_FillRect(0, 0, 320, 240);
  GUI_SetBkColor(GUI_WHITE);
  GUI_SetColor(GUI_BLACK);

  GUI_DrawBitmap(&bmIMAGE_embOSIP_64x46, 22, 35);

  GUI_SetFont(&GUI_Font20B_ASCII);
  GUI_GotoXY(5, 5);
  GUI_DispString("embOS-IP");
  GUI_GotoXY(100, 35);
  GUI_DispString("Features");

  GUI_SetFont(&GUI_Font13B_ASCII);
  GUI_GotoXY(5, 22);
  GUI_DispString("TCP/IP stack");
  GUI_GotoXY(100, 55);
  GUI_DispStringInRect("- API for user applications similar to\n"
                       "   ANSI C socket.h\n"
                       "- Very simple device driver structure\n"
                       "- High performance\n"
                       "- Small footprint\n"
                       "- Runs \"out-of-the-box\"\n"
                       "- No configuration required\n"
                       "- Works seamlessly with embOS in\n"
                       "   multitasking environment\n"
                       "- High performance\n"
                       "- Standard socket interface\n"
                       "- Raw Socket Support\n"
                       , &Rect5, GUI_TA_LEFT);
  GUI_Delay(SEGGERDEMO_DELAY);
  //
  // Presentation Page 6: embOS/IP more features
  //
  GUI_SetColor(GUI_WHITE);
  GUI_FillRect(0, 0, 320, 240);
  GUI_SetBkColor(GUI_WHITE);
  GUI_SetColor(GUI_BLACK);

  GUI_DrawBitmap(&bmIMAGE_embOSIP_64x46, 22, 35);

  GUI_SetFont(&GUI_Font20B_ASCII);
  GUI_GotoXY(5, 5);
  GUI_DispString("embOS-IP");
  GUI_GotoXY(100, 35);
  GUI_DispString("More Features");

  GUI_SetFont(&GUI_Font13B_ASCII);
  GUI_GotoXY(5, 22);
  GUI_DispString("TCP/IP stack");
  GUI_GotoXY(100, 55);
  GUI_DispStringInRect("- Non-blocking versions of all functions\n"
                       "- Connections limited only by available\n"
                       "   memory\n"
                       "- Nagle Algorithm (slow start)\n"
                       "- Delayed ACKs\n"
                       "- BSD style \"Keepalive\" option\n"
                       "- Works seamlessly with embOS in\n"
                       "- Re-assembly of fragmented packets\n"
                       "- Optional drivers for the most\n"
                       "   common devices are available\n"
                       "- Support for messages and\n"
                       "   warnings in debug build\n"
                       "- Royalty-free\n"
                       , &Rect6, GUI_TA_LEFT);
  GUI_Delay(SEGGERDEMO_DELAY);
  //
  // Presentation Page 7: embOS/IP layer
  //
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_FillRect(0, 0, 320, 240);
  GUI_SetBkColor(GUI_WHITE);
  GUI_SetColor(GUI_BLACK);

  GUI_DrawBitmap(&bmIMAGE_embOSIP_64x46, 22, 35);
  GUI_DrawBitmap(&bmIMAGE_embOSIPLayer_301x124, 10, 100);

  GUI_SetFont(&GUI_Font20B_ASCII);
  GUI_GotoXY(5, 5);
  GUI_DispString("embOS-IP");

  GUI_SetFont(&GUI_Font13B_ASCII);
  GUI_GotoXY(5, 22);
  GUI_DispString("TCP/IP stack");
  GUI_Delay(SEGGERDEMO_DELAY);
  //
  // Presentation Page 8: emWin
  //
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_FillRect(0, 0, 320, 240);
  GUI_SetBkColor(GUI_WHITE);
  GUI_SetColor(GUI_BLACK);

  GUI_DrawBitmap(&bmIMAGE_emWin_64x52, 30, 35);

  GUI_SetFont(&GUI_Font20B_ASCII);
  GUI_GotoXY(5, 5);
  GUI_DispString("emWin");
  GUI_GotoXY(100, 35);
  GUI_DispString("Features");

  GUI_SetFont(&GUI_Font13B_ASCII);
  GUI_GotoXY(5, 22);
  GUI_DispString("Graphics Software and GUI");
  GUI_GotoXY(100, 55);
  GUI_DispStringInRect("- Any CPU, any LCD, any LCD controller\n"
                       "- ANSI \"C\" no C++ required\n"
                       "- Multiple layer / multi display support\n"
                       "- Small footprint\n"
                       "- Touch screen support\n"
                       "- Alpha blending\n"
                       "- Support for transparent windows\n"
                       "- JPEG support\n"
                       "- Font converter available\n"
                       "- VNC Server available\n"
                       "- and much more..."
                       , &Rect8, GUI_TA_LEFT);
  GUI_Delay(SEGGERDEMO_DELAY);
  //
  // Presentation Page 9: emUSB
  //
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_FillRect(0, 0, 320, 240);
  GUI_SetBkColor(GUI_WHITE);
  GUI_SetColor(GUI_BLACK);

  GUI_DrawBitmap(&bmIMAGE_emUSB_57x18, 5, 35);

  GUI_SetFont(&GUI_Font20B_ASCII);
  GUI_GotoXY(5, 5);
  GUI_DispString("emUSB");
  GUI_GotoXY(100, 35);
  GUI_DispString("Features");

  GUI_SetFont(&GUI_Font13B_ASCII);
  GUI_GotoXY(5, 22);
  GUI_DispString("USB Device Stack");
  GUI_GotoXY(100, 55);
  GUI_DispStringInRect("- Supports USB 1.1/2.0 devices\n"
                       "- ISO/ANSI C source code\n"
                       "- Full/High speed support\n"
                       "- Bulk communication component with\n"
                       "   Windows kernelmode driver available\n"
                       "- MSD Component available\n"
                       "- MSD-CDROM support\n"
                       "- MSD supports any type of storage\n"
                       "   incl. NAND\n"
                       "- CDC component available\n"
                       "- HID component available\n"
                       "- Target drivers for most popular USB\n"
                       "   devices\n"
                       "- No royalties"
                       , &Rect9, GUI_TA_LEFT);
  GUI_Delay(SEGGERDEMO_DELAY);
  //
  // Presentation Page 10: emFile
  //
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_FillRect(0, 0, 320, 240);
  GUI_DrawBitmap(&bmIMAGE_emFile_305x167_565c, 0, 0);
  GUI_Delay(SEGGERDEMO_DELAY);
#if INCLUDE_JLINKPRESENTATION
  //
  // Presentation Page 11: J-Link
  //
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_FillRect(0, 0, 320, 240);
  GUI_DrawBitmap(&bmIMAGE_JLink_307x240_565c, 6, 0);
  GUI_Delay(SEGGERDEMO_DELAY);
#endif
#if ((INCLUDE_IP == 1) || ((INCLUDE_USB == 1) && (INCLUDE_FS == 1)))
  //
  // Presentation Page 12: Outro
  //
  GUI_SetBkColor(GUI_GRAY);
  GUI_Clear();
  GUI_SetBkColor(GUI_WHITE);
  GUI_SetColor(GUI_WHITE);
  GUI_FillRect(0, 0, 320, 60);

  GUI_SetColor(GUI_BLACK);
  GUI_DrawBitmap(&bmIMAGE_SeggerLogo_100x50, 5, 5);

  GUI_SetFont(&GUI_Font32B_ASCII);
  GUI_GotoXY(110, 15);
  GUI_DispString("SEGGER Demo");

  GUI_SetColor(GUI_BLUE);
  GUI_FillRect(0, 60, 320, 240);
  GUI_SetColor(GUI_WHITE);
  GUI_SetBkColor(GUI_BLUE);
  GUI_SetFont(&GUI_Font24B_ASCII);
  GUI_GotoXY(5, 70);
  _acTextBuffer[0] = '\0';
  strcat(_acTextBuffer, "The following demo application shows embOS");
  if (INCLUDE_IP == 1) {
    strcat(_acTextBuffer, ", embOS/IP");
  }
  if ((INCLUDE_USB == 1) && (INCLUDE_FS == 1)) {
    strcat(_acTextBuffer, ", USB MSD");
  }
  strcat(_acTextBuffer, " and emWin");
  GUI_DispStringInRectWrap(_acTextBuffer, &Rect12, GUI_TA_CENTER, GUI_WRAPMODE_WORD);

  GUI_SetColor(GUI_GREEN);
  GUI_GotoXY(20, 150);
  _acTextBuffer[0] = '\0';
  if (INCLUDE_IP == 1) {
    sprintf(_acTextBuffer, "Webserver/ FTP Server");
  }
  if ((INCLUDE_USB == 1) && (INCLUDE_FS == 1)) {
    if (_acTextBuffer[0] != '\0') {
      sprintf(_acTextBuffer, "%s/ ", _acTextBuffer);
    }
    sprintf(_acTextBuffer, "%sUSB MSD", _acTextBuffer);
  }
  sprintf(_acTextBuffer, "%s  is already running!", _acTextBuffer);
  GUI_DispStringInRectWrap(_acTextBuffer, &Rect13, GUI_TA_CENTER, GUI_WRAPMODE_WORD);

  GUI_SetFont(&GUI_FontComic18B_ASCII);
  GUI_Delay(SEGGERDEMO_DELAY);
  if (INCLUDE_IP == 1) {
    //
    // Delete Ethernet Window
    //
    FRAMEWIN_Delete(_hWindow1);
  }
  WM_SelectWindow(WM_HBKWIN);
#endif
#endif // of INCLUDE_PRESENTATION
}

/*********************************************************************
*
*       MainTask()
*/
void MainTask(void);
void MainTask(void) {
  GUI_Init();
#if (INCLUDE_IP == 1)
  IP_Init();
  //
  // Start TCP/IP task
  //
  OS_CREATETASK(&_IP_TCB,        "IP_Task",    IP_Task,       TASKPRIO_IPMAIN, _IP_Stack);
  OS_CREATETASK(&_IPRx_TCB,      "IP_RxTask",  IP_RxTask,     TASKPRIO_IPRX,   _IPRx_Stack);
  // Start Webserver and FTP Server
  OS_CREATETASK(&_FTPServer_TCB, "FTP Server", FTPServerTask, TASKPRIO_FTPS,   _FTPServer_Stack);
  OS_CREATETASK(&_WEBServer_TCB, "Webserver",  WebserverTask, TASKPRIO_WEBS,   _WEBServer_Stack);
#if (INCLUDE_PRESENTATION == 1)
  _hWindow1 = WM_CreateWindow(0, 0, 320, 240, WM_CF_SHOW, NULL, 0);
  WM_SelectWindow(_hWindow1);
#endif
#endif
#if (INCLUDE_VNC == 1)
  //
  // Start VNC server
  //
  GUI_VNC_X_StartServer(0, 0);
#endif
#if ((INCLUDE_USB == 1) && (INCLUDE_FS == 1))
  //
  // Start USB MSD task
  //
  OS_CREATETASK(&_USB_MSD_TCB, "USBMSDTask", _USBMSDTask, TASKPRIO_USB, _USB_MSD_Stack);
#endif
#if (!defined _WINDOWS) && ((INCLUDE_IP == 1) || (INCLUDE_USB == 1) || (INCLUDE_FS == 1))
  if (strncmp(OS_GetLibName(), "(T", 2) == 0) {
    //
    // Setup task for trial window
    //
    OS_CREATETASK(&_TrialWindow_TCB, "TrialWindow", _TrialWindowTask, TASKPRIO_TRIALWINDOW, _TrialWindow_Stack);
  }
#endif
#if GUI_SUPPORT_CURSOR
  GUI_CURSOR_Hide();
#endif
  //
  // Presentation Page 1: SEGGER logo
  //
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_FillRect(0, 0, 320, 240);
  GUI_DrawBitmap(&bmIMAGE_SeggerLogo_300x150_565c, 10, 45);
  GUI_Delay(SEGGERDEMO_DELAY);
#if (INCLUDE_IP == 1)
  //
  // Give ethernet link state some time to show get IP address and show it
  //
  OS_CREATETASK(&_EthWindow_TCB, "EthWinTask", _EthWindowTask,   TASKPRIO_WINDOW, _EthWindow_Stack);
#endif
  //
  // Show presentation if set
  //
  _ShowPresentation();
  GUI_Clear();
  //
  // Start GUIDemo
  //
  while(1) {
    GUIDEMO_main();
  }
}

/****** End of File *************************************************/
