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
File        : GUI_Device.c
Purpose     : Implementation of device related functions
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _LinkDevice
*
* Purpose:
*   Link the given device into the device chain
*/
static void _LinkDevice(GUI_DEVICE * pDeviceNew, GUI_DEVICE * pDeviceCur) {
  int LayerIndex;

  LayerIndex = pDeviceNew->LayerIndex;
  if (pDeviceCur->pPrev) {
    pDeviceCur->pPrev->pNext = pDeviceNew;
    pDeviceNew->pPrev = pDeviceCur->pPrev;
  } else {
    GUI_Context.apDevice[LayerIndex] = pDeviceNew;
  }
  pDeviceCur->pPrev = pDeviceNew;
  pDeviceNew->pNext = pDeviceCur;
}

/*********************************************************************
*
*       _UnlinkDevice
*
* Purpose:
*   Unlink the given device from the device chain
*/
static void _UnlinkDevice(GUI_DEVICE * pDevice) {
  int LayerIndex;

  LayerIndex = pDevice->LayerIndex;
  if (GUI_Context.apDevice[LayerIndex] == pDevice) {
    GUI_Context.apDevice[LayerIndex] = pDevice->pNext;
  }
  if (pDevice->pPrev) {
    pDevice->pPrev->pNext = pDevice->pNext;
  }
  if (pDevice->pNext) {
    pDevice->pNext->pPrev = pDevice->pPrev;
  }
  pDevice->pNext = NULL;
  pDevice->pPrev = NULL;
}

/*********************************************************************
*
*       _SetColorConv
*
* Purpose:
*   Sets the color conversion API table for the given device.
*
* Parameter:
*   pDeviceAPI    - Device object to be modified
*   pColorConvAPI - Pointer to LCD_API_COLOR_CONV. If NULL, the display driver color conversion is used.
*/
static void _SetColorConv(GUI_DEVICE * pDevice, const LCD_API_COLOR_CONV * pColorConvAPI, int LayerIndex) {
  GUI_DEVICE * pDeviceCur;

  if (pColorConvAPI) {
    pDevice->pColorConvAPI = pColorConvAPI;
  } else {
    pDeviceCur = GUI_Context.apDevice[LayerIndex];
    if (pDeviceCur) {
      while (pDeviceCur->pNext) {
        pDeviceCur = pDeviceCur->pNext;
      }
      pDevice->pColorConvAPI = pDeviceCur->pColorConvAPI;
    }
  }
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DEVICE__GetpDevice
*
* Purpose:
*   Returns the first device of the given chain of the given device class
*/
GUI_DEVICE * GUI_DEVICE__GetpDevice(int LayerIndex, int DeviceClass) {
  GUI_DEVICE * pDevice = NULL;

  //
  // Check on legal range
  //
  if (LayerIndex >= GUI_NUM_LAYERS) {
    return NULL; // Error
  }
  //
  // Go to the last device
  //
  pDevice = GUI_Context.apDevice[LayerIndex];
  while (pDevice) {
    if (pDevice->pDeviceAPI->DeviceClassIndex == DeviceClass) {
      return pDevice;
    }
    pDevice = pDevice->pNext;
  }
  return NULL;
}

/*********************************************************************
*
*       GUI_DEVICE__GetpDriver
*
* Purpose:
*   Returns the first display driver of the device chain. If there is
*   no driver (should not occur) the function returns NULL.
*/
GUI_DEVICE * GUI_DEVICE__GetpDriver(int LayerIndex) {
  GUI_DEVICE * pDevice = NULL;

  //
  // Check on legal range
  //
  if (LayerIndex >= GUI_NUM_LAYERS) {
    return NULL; // Error
  }
  //
  // Get the first display driver
  //
  pDevice = GUI_Context.apDevice[LayerIndex];
  while (pDevice) {
    if (pDevice->pDeviceAPI->DeviceClassIndex == DEVICE_CLASS_DRIVER) {
      break;
    }
    pDevice = pDevice->pNext;
  }
  return pDevice;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DEVICE_Delete
*/
void GUI_DEVICE_Delete(GUI_DEVICE * pDevice) {
  GUI_ALLOC_FreeFixedBlock(pDevice);
}

/*********************************************************************
*
*       GUI_DEVICE_Create
*/
GUI_DEVICE * GUI_DEVICE_Create(const GUI_DEVICE_API * pDeviceAPI, const LCD_API_COLOR_CONV * pColorConvAPI, U16 Flags, int LayerIndex) {
  GUI_DEVICE * pDevice;

  //
  // Check on legal range
  //
  if (LayerIndex >= GUI_NUM_LAYERS) {
    return NULL; // Error
  }
  //
  // Create new device
  //
  pDevice = (GUI_DEVICE *)GUI_ALLOC_GetFixedBlock(sizeof(GUI_DEVICE));
  if (pDevice == NULL) {
    return NULL; // Error
  }
  //
  // Initialize members
  //
  _SetColorConv(pDevice, pColorConvAPI, LayerIndex);
  pDevice->LayerIndex = LayerIndex;
  pDevice->Flags      = Flags;
  pDevice->pDeviceAPI = pDeviceAPI;
  pDevice->pNext      = NULL;
  pDevice->pPrev      = NULL;
  pDevice->u.pContext = NULL;
  return pDevice;
}

/*********************************************************************
*
*       GUI_DEVICE_Unlink
*/
void GUI_DEVICE_Unlink(GUI_DEVICE * pDevice) {
  _UnlinkDevice(pDevice);
}

/*********************************************************************
*
*       GUI_DEVICE_Link
*
* Purpose:
*   Find the right position in the device chain
*/
int GUI_DEVICE_Link(GUI_DEVICE * pDeviceNew) {
  GUI_DEVICE * pDeviceCur;
  int LayerIndex;

  if (pDeviceNew == NULL) {
    return 1; // Error, obvious
  }
  //
  // Check on legal range
  //
  LayerIndex = pDeviceNew->LayerIndex;
  if (LayerIndex >= GUI_NUM_LAYERS) {
    return 1; // Error
  }
  pDeviceNew->LayerIndex = LayerIndex;
  pDeviceCur = GUI_Context.apDevice[LayerIndex];
  if (pDeviceCur == NULL) {
    GUI_Context.apDevice[LayerIndex] = pDeviceNew;
    return 0; // Ok, nothing else to do, because only one device exists
  }
  //
  // Start from the first device...
  //
  while (pDeviceCur->pPrev) {
    pDeviceCur = pDeviceCur->pPrev;
  }
  //
  // Find right position in dependence of 'DeviceClass'
  //
  while (pDeviceCur) {
    if (pDeviceCur->pDeviceAPI->DeviceClassIndex <= pDeviceNew->pDeviceAPI->DeviceClassIndex) {
      break;
    }
    pDeviceCur = pDeviceCur->pNext;
  }
  //
  // Manage 'stay-on-top' flag
  //
  if (pDeviceCur) {
    if (pDeviceCur->pDeviceAPI->DeviceClassIndex == pDeviceNew->pDeviceAPI->DeviceClassIndex) {
      if (pDeviceNew->Flags == GUI_DEVICE_STAYONTOP) {
        while (pDeviceCur) {
          if (pDeviceCur->pNext) {
            pDeviceCur = pDeviceCur->pNext;
            if (pDeviceCur->pDeviceAPI->DeviceClassIndex < pDeviceNew->pDeviceAPI->DeviceClassIndex) {
              break;
            }
          } else {
            break;
          }
        }
      }
    }
  }
  //
  // At least a driver should always exist
  //
  if (pDeviceCur == NULL) {
    return 1;
  }
  //
  // Link in new device
  //
  _LinkDevice(pDeviceNew, pDeviceCur);
  return 0;
}

/*********************************************************************
*
*       GUI_DEVICE_CreateAndLink
*/
GUI_DEVICE * GUI_DEVICE_CreateAndLink(const GUI_DEVICE_API * pDeviceAPI, const LCD_API_COLOR_CONV * pColorConvAPI, U16 Flags, int LayerIndex) {
  GUI_DEVICE * pDevice;
  
  //
  // Create device
  //
  pDevice = GUI_DEVICE_Create(pDeviceAPI, pColorConvAPI, Flags, LayerIndex);
  //
  // Link it into device list
  //
  if (pDevice) {
    GUI_DEVICE_Link(pDevice);
  }
  return pDevice;
}

/*************************** End of file ****************************/
