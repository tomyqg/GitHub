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
File        : GUI_PID.C
Purpose     : PID (Pointer input device) management with buffer
----------------------------------------------------------------------
*/

#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// The following value defines the maximum number of states
// to be managed by this module.
//
#ifndef   GUI_PID_BUFFER_SIZE
  #define GUI_PID_BUFFER_SIZE 5
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
//
// Input buffer.  Please note that one element remains empty.
//
static GUI_PID_STATE _aState[GUI_PID_BUFFER_SIZE + 1]; // Buffer

static GUI_PID_STATE * _pRead  = &_aState[0]; // Read pointer
static GUI_PID_STATE * _pWrite = &_aState[0]; // Write pointer

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DecPointer
*
* Purpose:
*   Decrements the given pointer. It makes sure that the result is not
*   the given reference pointer. In this case it returns the old value
*
* Parameters:
*   p    - Pointer to be decremented
*   pRef - Reference pointer
*
* Return value:
*   The new pointer
*/
static GUI_PID_STATE * _DecPointer(GUI_PID_STATE * p, GUI_PID_STATE * pRef) {
  GUI_PID_STATE * pNew;
  pNew = p;
  if (pNew == _aState) {
    //
    // Set pointer to last element of array
    //
    pNew = _aState + GUI_PID_BUFFER_SIZE;
  } else {
    //
    // Decrement pointer
    //
    --pNew;
  }
  if (pNew != pRef) {
    //
    // Return new pointer
    //
    return pNew;
  }
  //
  // Return old pointer
  //
  return p;
}

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
static GUI_PID_STATE * _IncPointer(GUI_PID_STATE * p, GUI_PID_STATE * pRef) {
  GUI_PID_STATE * pNew;
  pNew = p;
  if (pNew == (_aState + GUI_PID_BUFFER_SIZE)) {
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
  if (pNew != pRef) {
    //
    // Return new pointer
    //
    return pNew;
  }
  //
  // Return old pointer
  //
  return p;
}

/*********************************************************************
*
*       _DecWritePointer
*/
static GUI_PID_STATE * _DecWritePointer(GUI_PID_STATE * pWrite) {
  return _DecPointer(pWrite, _pRead);
}

/*********************************************************************
*
*       _IncReadPointer
*/
static GUI_PID_STATE * _IncReadPointer(GUI_PID_STATE * pRead) {
  return _IncPointer(pRead, _pWrite);
}

/*********************************************************************
*
*       _IncWritePointer
*/
static GUI_PID_STATE * _IncWritePointer(GUI_PID_STATE * pWrite) {
  return _IncPointer(pWrite, _pRead);
}

/*********************************************************************
*
*       _StoreState
*
* Purpose:
*   Puts the given PID state into the state buffer. In case of an
*   empty buffer it writes without any checks into the buffer. If
*   the buffer is not empty the routine checks if the pressed state
*   of the previous 2 entries are the same as from the new entry to
*   to be written. If it is the same the last element will be
*   overwritten. So the buffer contains a maximum of 2 consecutive
*   entries of the same pressed state.
*/
static void _StoreState(const GUI_PID_STATE * pState) {
  GUI_PID_STATE * pPrev0;
  GUI_PID_STATE * pPrev1;
  //
  // Check if not empty
  //
  if (_pWrite != _pRead) {
    //
    // Get previous element
    //
    pPrev0 = _DecWritePointer(_pWrite);
    if (pPrev0 != _pWrite) {
      //
      // Check if pressed state of new element is different
      //
      if (pState->Pressed == pPrev0->Pressed) {
        //
        // Get previous element
        //
        pPrev1 = _DecWritePointer(pPrev0);
        if (pPrev1 != pPrev0) {
          //
          // Check if pressed state of new element is different
          //
          if (pState->Pressed == pPrev1->Pressed) {
            //
            // Set write pointer on previous element to be overwritten
            //
            _pWrite = _DecWritePointer(_pWrite);
          }
        }
      }
    }
  }
  *_pWrite = *pState;
  _pWrite = _IncWritePointer(_pWrite);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_PID_GetState
*/
int GUI_PID_GetState(GUI_PID_STATE * pState) {
  //
  // Get value
  //
  *pState = *_pRead;
  //
  // Increment read pointer if not empty
  //
  if (_pRead != _pWrite) {
    _pRead = _IncReadPointer(_pRead);
  }
  //
  // Return
  //
  return (pState->Pressed != 0) ? 1 : 0;
}

/*********************************************************************
*
*       GUI_PID_StoreState
*/
void GUI_PID_StoreState(const GUI_PID_STATE * pState) {
  //
  // If the window manager is available, set the function pointer for
  // the PID handler.
  //
  #if (GUI_WINSUPPORT)
    WM_pfHandlePID = WM_HandlePID;
  #endif
  //
  // Store value
  //
  _StoreState(pState);
  //
  // Wake up emWin (if required)
  //
  GUI_X_SIGNAL_EVENT();
}

/*************************** End of file ****************************/
