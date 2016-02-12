/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2010  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.06 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUI_OnKey.c
Purpose     : Implementation of keyboard buffer
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM_Intern.h"
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// The following value defines the maximum number of key events + 1
// to be managed by this module. Please note that one element remains
// empty.
//
#ifndef   GUI_KEY_BUFFER_SIZE
  #define GUI_KEY_BUFFER_SIZE 10
#endif

#define IS_NOT_EMPTY() (_pReadKey != _pWriteKey)
#define IS_EMPTY()     (_pReadKey == _pWriteKey)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int Key;
  int Pressed;
} GUI_KEY_STATE;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static GUI_KEY_STATE _aState[GUI_KEY_BUFFER_SIZE + 1]; // Buffer

static GUI_KEY_STATE * _pReadKey  = &_aState[0]; // Read pointer
static GUI_KEY_STATE * _pWriteKey = &_aState[0]; // Write pointer

static int _Key; // Used for GUI_GetKey() and GUI_StoreKey()

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _IncPointer
*
* Purpose:
*   Increments the given pointer. It makes sure that the result is not
*   the given reference pointer. In this case it returns the old value
*
* Parameters:
*   p    - Pointer to be incremented
*   pRef - Reference pointer
*
* Return value:
*   The new pointer
*/
#if GUI_WINSUPPORT
static GUI_KEY_STATE * _IncPointer(GUI_KEY_STATE * p, GUI_KEY_STATE * pRef) {
  GUI_KEY_STATE * pNew;
  pNew = p;
  if (pNew == (_aState + GUI_KEY_BUFFER_SIZE)) {
    //
    // Set pointer to first element of array
    //
    pNew = _aState;
  } else {
    //
    // Increment pointer
    //
    ++pNew;
  }
  //
  // Do not reach reference pointer
  //
  if (pRef) {
    if (pNew != pRef) {
      p = pNew;
    }
  } else {
    p = pNew;
  }
  return p;
}

/*********************************************************************
*
*       _IncReadPointer
*/
static GUI_KEY_STATE * _IncReadPointer(GUI_KEY_STATE * pRead) {
  GUI_KEY_STATE * pState;
  pState = _IncPointer(pRead, NULL);
  return pState;
}

/*********************************************************************
*
*       _IncWritePointer
*/
static GUI_KEY_STATE * _IncWritePointer(GUI_KEY_STATE * pWrite) {
  GUI_KEY_STATE * pState;
  pState = _IncPointer(pWrite, _pReadKey);
  return pState;
}

/*********************************************************************
*
*       _GetState
*/
static int _GetState(GUI_KEY_STATE * pState) {
  //
  // Get value
  //
  *pState = *_pReadKey;
  //
  // Increment read pointer if not empty
  //
  if (_pReadKey != _pWriteKey) {
    _pReadKey = _IncReadPointer(_pReadKey);
  }
  //
  // Return
  //
  return (pState->Pressed != 0) ? 1 : 0;
}

/*********************************************************************
*
*       _StoreState
*/
static void _StoreState(const GUI_KEY_STATE * pState) {
  //
  // Store value
  //
  *_pWriteKey = *pState;
  //
  // Increment write pointer
  //
  _pWriteKey = _IncWritePointer(_pWriteKey);
}
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
#if 1
/*********************************************************************
*
*       GUI_GetKey
*/
int GUI_GetKey(void) {
  int r = _Key;
  _Key = 0;
  return r;
}

/*********************************************************************
*
*       GUI_StoreKey
*/
void GUI_StoreKey(int Key) {
  if (!_Key) {
    _Key = Key;
  }
  GUI_X_SIGNAL_EVENT();
}
#else
/*********************************************************************
*
*       GUI_GetKey
*/
int GUI_GetKey(void) {
  GUI_KEY_STATE State;
  
  _GetState(&State);
  return State.Key;
}

/*********************************************************************
*
*       GUI_StoreKey
*/
void GUI_StoreKey(int Key) {
  GUI_KEY_STATE State;

  if (IS_EMPTY()) {
    State.Key     = Key;
    State.Pressed = 1;
    _StoreState(&State);
  }
  GUI_X_SIGNAL_EVENT();
}
#endif
/*********************************************************************
*
*       GUI_ClearKeyBuffer
*/
void GUI_ClearKeyBuffer(void) {
  while (IS_NOT_EMPTY()) {
    GUI_GetKey();
  }
}

/*********************************************************************
*
*       GUI_StoreKeyMsg
*/
void GUI_StoreKeyMsg(int Key, int PressedCnt) {
  #if GUI_WINSUPPORT
  GUI_KEY_STATE State;

  State.Key = Key;
  State.Pressed = PressedCnt;
  _StoreState(&State);
  GUI_X_SIGNAL_EVENT();
  #else
    GUI_USE_PARA(PressedCnt);
    GUI_StoreKey(Key);
  #endif
}

/*********************************************************************
*
*       GUI_PollKeyMsg
*/
#if GUI_WINSUPPORT
int GUI_PollKeyMsg(void) {
  GUI_KEY_STATE State;
  int r = 0;

  GUI_LOCK();
  if (IS_NOT_EMPTY()) {
    _GetState(&State);
    WM_OnKey(State.Key, State.Pressed);
    r = 1;
  }
  GUI_UNLOCK();
  return r;
}
#endif

/*********************************************************************
*
*       GUI_SendKeyMsg
*
* Purpose:
*   Send the key to a window using the window manager (if available).
*   If no window is ready to take the input, we call the store routine
*   and wait for somebody to poll the buffer.
*/
void GUI_SendKeyMsg(int Key, int PressedCnt) {
  #if GUI_WINSUPPORT
    if (!WM_OnKey(Key, PressedCnt)) {
      GUI_StoreKeyMsg(Key, PressedCnt);
    }
  #else
    GUI_StoreKeyMsg(Key, PressedCnt);
  #endif
}

/*************************** End of file ****************************/
