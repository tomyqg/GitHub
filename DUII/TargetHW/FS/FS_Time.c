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
File        : FS_Time.c
Purpose     : Implementation of file system's time stamp functions
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include "FS_ConfDefaults.h"        /* FS Configuration */
#include "FS_Int.h"
#include "FS_CLib.h"


/*********************************************************************
*
*       Public code, internal code
*
**********************************************************************
*/


/*********************************************************************
*
*       FS__GetFileTime
*
*  Description:
*    Internal version of FS_GetFileTime.
*    Gets the creation timestamp of a given file/directory name.
*
*  Parameters:
*
*/
int FS__GetFileTimeEx(const char * pName, U32 * pTimeStamp, int TimeIndex) {
  int          r;
  int          TypeMask;
  const char * s;
  FS_VOLUME *  pVolume;

  r = -1;
  /* Find correct FSL (device:unit:name) */
  pVolume = FS__FindVolume(pName, &s);
  if (pVolume) {
    if ((FS__AutoMount(pVolume) & FS_MOUNT_R) == FS_MOUNT_R)  {
      switch (TimeIndex) {
      case FS_FILETIME_CREATE:
        TypeMask = FS_DIRENTRY_GET_TIMESTAMP_CREATE;
        break;
      case FS_FILETIME_ACCESS:
        TypeMask = FS_DIRENTRY_GET_TIMESTAMP_ACCESS;
        break;
      case FS_FILETIME_MODIFY:
        TypeMask = FS_DIRENTRY_GET_TIMESTAMP_MODIFY;
        break;
      default:
        return -1;  // Unknown TimeIndex used
      }
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      r = FS_GETDIRENTRYINFO(pVolume, s, pTimeStamp, TypeMask);
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
    }
  }
  return r;
}

/*********************************************************************
*
*       FS_SetFileTime
*
*  Description:
*    Sets the creation timestamp of a given file/directory name.
*
*  Parameters:
*
*/
int FS__SetFileTimeEx(const char * pName, U32 TimeStamp, int TimeIndex) {
  int          r;
  const char * s;
  int          TypeMask;
  FS_VOLUME  * pVolume;

  r = -1;
  /* Find correct FSL (device:unit:name) */
  pVolume = FS__FindVolume(pName, &s);
  if (pVolume) {
    if (FS__AutoMount(pVolume) == FS_MOUNT_RW)  {
      switch (TimeIndex) {
      case FS_FILETIME_CREATE:
        TypeMask = FS_DIRENTRY_SET_TIMESTAMP_CREATE;
        break;
      case FS_FILETIME_ACCESS:
        TypeMask = FS_DIRENTRY_SET_TIMESTAMP_ACCESS;
        break;
      case FS_FILETIME_MODIFY:
        TypeMask = FS_DIRENTRY_SET_TIMESTAMP_MODIFY;
        break;
      default:
        return -1;  // Unknown TimeIndex used
      }
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      FS_JOURNAL_BEGIN(pVolume);
      r = FS_SETDIRENTRYINFO(pVolume, s, &TimeStamp, TypeMask);
      FS_JOURNAL_END(pVolume);
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
    }
  }
  return r;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_GetFileTimeEx
*
*  Description:
*    Gets the timestamp of a given file/directory name.
*
*  Parameters:
*
*/
int FS_GetFileTimeEx(const char * pName, U32 * pTimeStamp, int TimeIndex) {
  int  r;
  FS_LOCK();
  r = FS__GetFileTimeEx(pName, pTimeStamp, TimeIndex);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_SetFileTimeEx
*
*  Description:
*    Sets the timestamp of a given file/directory name.
*
*  Parameters:
*
*/
int FS_SetFileTimeEx(const char * pName, U32 TimeStamp, int Index) {
  int  r;
  FS_LOCK();
  r = FS__SetFileTimeEx(pName, TimeStamp, Index);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetFileTime
*
*  Description:
*    Gets the timestamp of a given file/directory name.
*
*  Parameters:
*
*/
int FS_GetFileTime(const char * pName, U32 * pTimeStamp) {
  int  r;
  FS_LOCK();
  r = FS__GetFileTimeEx(pName, pTimeStamp, FS_FILETIME_CREATE);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_SetFileTime
*
*  Description:
*    Sets the timestamp of a given file/directory name.
*
*  Parameters:
*
*/
int FS_SetFileTime(const char * pName, U32 TimeStamp) {
  int  r;
  FS_LOCK();
  r = FS__SetFileTimeEx(pName, TimeStamp,  FS_FILETIME_CREATE);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_TimeStampToFileTime
*
*  Description:
*    Fills the pFileTime structure with the given TimeStamp.
*
*  Parameters:
*
*/
void FS_TimeStampToFileTime(U32 TimeStamp, FS_FILETIME * pFileTime) {
  U16 Date;
  U16 Time;

  Date = (U16)(TimeStamp >> 16);
  Time = (U16)(TimeStamp & 0xffff);
  if (pFileTime) {
    pFileTime->Year   = (U16) ((Date >> 9) + 1980);
    pFileTime->Month  = (U16) ((Date & 0x1e0) >> 5);
    pFileTime->Day    = (U16) ((Date & 0x1f));
    pFileTime->Hour   = (U16) (Time >> 11);
    pFileTime->Minute = (U16) ((Time & 0x7e0) >> 5);
    pFileTime->Second = (U16) ((Time & 0x1f) << 1);
  }
}

/*********************************************************************
*
*       FS_FileTimeToTimeStamp
*
*  Description:
*    Fills the pFileTime structure with the given TimeStamp.
*
*  Parameters:
*
*/
void FS_FileTimeToTimeStamp(const FS_FILETIME * pFileTime, U32 * pTimeStamp) {
  U16 Date;
  U16 Time;

  if (pTimeStamp) {
    Date  = (U16) (((pFileTime->Year - 1980) << 9) |  (pFileTime->Month << 5) | pFileTime->Day);
    Time  = (U16) ((pFileTime->Hour << 11) |  (pFileTime->Minute << 5) |  (pFileTime->Second >> 1));
   *pTimeStamp = (U32)((U32)Date << 16) | (Time & 0xffff);

  }
}


/*************************** End of file ****************************/
