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
File        : FS_Attrib.c
Purpose     : Implementation of FS_GetFileAttributes
                                FS_SetFileAttributes
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h"

/*********************************************************************
*
*       Public code, internal code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__SetFileAttributes
*
*  Function description
*    Internal version of FS_SetFileAttributes
*    Sets the attributes for a given file/directory name.
*
*  Parameters:
*    sName       - pointer to a string that contains the file/directory name.
*    Attributes  - Attributes that shall be set.
*
*/
int FS__SetFileAttributes(const char * sName, U8 Attributes) {
  int  r;
  const char *s;
  FS_VOLUME * pVolume;

  r = -1;
  pVolume = FS__FindVolume(sName, &s);
  if (pVolume) {
    if (FS__AutoMount(pVolume) == FS_MOUNT_RW)  {
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      FS_JOURNAL_BEGIN(pVolume);
      r = FS_SETDIRENTRYINFO(pVolume, s, &Attributes, FS_DIRENTRY_SET_ATTRIBUTES);
      FS_JOURNAL_END(pVolume);
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
    }
  }
  return r;
}

/*********************************************************************
*
*       FS_GetFileAttributes
*
*  Function description:
*    Internal version of FS__GetFileAttributes
*    Gets the attributes from a given file/directory name.
*
*  Parameters:
*    sName       - pointer to a string that contains the file/directory name.
*
*  Return value:
*    0xff             - Error
*    other than 0xff   file's/directory's attributtes (or-combined)
*/
U8 FS__GetFileAttributes(const char * pName) {
  U8      Attributes;
  U8      r;
  const char * s;
  FS_VOLUME  * pVolume;

  Attributes = 0;
  pVolume = FS__FindVolume(pName, &s);
  if (pVolume) {
    if (FS__AutoMount(pVolume) & FS_MOUNT_R)  {
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      r = FS_GETDIRENTRYINFO(pVolume, s, &Attributes, FS_DIRENTRY_GET_ATTRIBUTES);
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
      if (r) {
        Attributes = 0xff;
      }
    }
  }
  return Attributes;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_SetFileAttributes
*
*  Function description:
*    Sets the attributes for a given file/directory name.
*
*  Parameters:
*    pName      - Pointer to a name of the file/directory.
*    Attributes - Attribute to set.
*
*  Return value:
*    == 0             - O.K.
*    != 0             - Error
*/
int FS_SetFileAttributes(const char * sName, U8 Attributes) {
  int  r;

  FS_LOCK();
  r = FS__SetFileAttributes(sName, Attributes);  
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetFileAttributes
*
*  Function description:
*    Gets the attributes of a given file/directory name.
*
*  Parameters:
*    sName  - Pointer to a name of the file/directory.
*
*  Return value:
*    0xff             - Error
*    other than 0xff   file's/directory's attributtes (or-combined)
*/
U8 FS_GetFileAttributes(const char * sName) {
  U8 Attributes;

  FS_LOCK();
  Attributes = FS__GetFileAttributes(sName);
  FS_UNLOCK();
  return Attributes;
}

/*************************** End of file ****************************/
