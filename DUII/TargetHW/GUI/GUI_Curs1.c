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
File        : GUICurs.c
Purpose     : Cursor routines of the graphics library based on sprites
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"

#if (GUI_SUPPORT_CURSOR)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  const GUI_CURSOR GUI_UNI_PTR * pCursor;
  char CursorIsVis;        /* Currently visible ? */
  char CursorOn;
  int  x, y;               /* Position of hot spot */
  GUI_HSPRITE hSprite;
} CURSOR_CONTEXT;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static CURSOR_CONTEXT _aCursorContext[GUI_NUM_LAYERS];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Show
*/
static void _Show(int Layer) {
  unsigned CursorLayer;
  CursorLayer = GUI_GetCursorLayer(Layer);
  if (CursorLayer) {
    GUI_SetLayerVisEx(CursorLayer, 1);
  } else {
    if (_aCursorContext[Layer].CursorOn) {
      _aCursorContext[Layer].CursorIsVis = 1;
      GUI_SPRITE_Show(_aCursorContext[Layer].hSprite);
    }
  }
}

/*********************************************************************
*
*       _Hide
*/
static void _Hide(int Layer) {
  unsigned CursorLayer;
  CursorLayer = GUI_GetCursorLayer(Layer);
  if (CursorLayer) {
    GUI_SetLayerVisEx(CursorLayer, 0);
    _aCursorContext[Layer].CursorIsVis = 0;
  } else {
    if (_aCursorContext[Layer].CursorIsVis) {
      GUI_SPRITE_Hide(_aCursorContext[Layer].hSprite);
      _aCursorContext[Layer].CursorIsVis = 0;
    }
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_CURSOR_SelectEx
*/
const GUI_CURSOR GUI_UNI_PTR * GUI_CURSOR_SelectEx(const GUI_CURSOR GUI_UNI_PTR * pCursor, int Layer) {
  unsigned CursorLayer;
  const GUI_CURSOR GUI_UNI_PTR * pOldCursor;
  GUI_LOCK();
  //GUI_DEBUG_ASSERT((Layer < GUI_NUM_CURSORS));
  CursorLayer = GUI_GetCursorLayer(Layer);
  pOldCursor = _aCursorContext[Layer].pCursor;
  if (CursorLayer) {
    /* Manage hardware cursors */
    unsigned OldLayer;
    OldLayer = GUI_SelectLayer(CursorLayer);
    GUI_SetBkColor(GUI_TRANSPARENT);
    GUI_Clear();
    GUI_DrawBitmap(pCursor->pBitmap, 0, 0);
    GUI_SetLayerSizeEx(CursorLayer, pCursor->pBitmap->XSize, pCursor->pBitmap->YSize);
    _aCursorContext[Layer].pCursor  = pCursor;
    _aCursorContext[Layer].CursorOn = 1;
    GUI_SelectLayer(OldLayer);
    _Show(Layer);
  } else {
    /* Manage sprite cursors */
    _aCursorContext[Layer].pCursor  = pCursor;
    _aCursorContext[Layer].CursorOn = 1;
    if (_aCursorContext[Layer].hSprite) {
      GUI_SPRITE_Delete(_aCursorContext[Layer].hSprite);
      _aCursorContext[Layer].hSprite = 0;
    }
    if (pCursor) {
      _aCursorContext[Layer].hSprite  = GUI_SPRITE__CreateEx(pCursor->pBitmap, _aCursorContext[Layer].x - pCursor->xHot, _aCursorContext[Layer].y - pCursor->yHot, Layer, GUI_SPRITE_CF_STAYONTOP);
      _Show(Layer);
    }
  }
  GUI_UNLOCK();
  return pOldCursor;
}

/*********************************************************************
*
*       GUI_CURSOR_HideEx
*/
void GUI_CURSOR_HideEx(int Layer) {
  GUI_LOCK();
  //GUI_DEBUG_ASSERT((Layer < GUI_NUM_CURSORS));
  _aCursorContext[Layer].CursorOn = 0;
  if (_aCursorContext[Layer].hSprite) {
    GUI_SPRITE_SetPosition(_aCursorContext[Layer].hSprite, _aCursorContext[Layer].x - _aCursorContext[Layer].pCursor->xHot, _aCursorContext[Layer].y - _aCursorContext[Layer].pCursor->yHot);
  }
  _Hide(Layer);
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_CURSOR_ShowEx
*/
void GUI_CURSOR_ShowEx(int Layer) {
  GUI_LOCK();
  //GUI_DEBUG_ASSERT((Layer < GUI_NUM_CURSORS));
  _aCursorContext[Layer].CursorOn = 1;
  if (!_aCursorContext[Layer].pCursor) {
    GUI_CURSOR_SelectEx(GUI_DEFAULT_CURSOR, Layer);
  } else {
    _Show(Layer);
  }
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_CURSOR_GetStateEx
*/
int GUI_CURSOR_GetStateEx(int Layer) {
  int r;
  GUI_LOCK();
  //GUI_DEBUG_ASSERT((Layer < GUI_NUM_CURSORS));
  r = _aCursorContext[Layer].CursorOn;
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_CURSOR_SetPositionEx
*/
void GUI_CURSOR_SetPositionEx(int xNewPos, int yNewPos, int Layer) {
  unsigned CursorLayer;
  GUI_LOCK();
  //GUI_DEBUG_ASSERT((Layer < GUI_NUM_CURSORS));
  CursorLayer = GUI_GetCursorLayer(Layer);
  if (CursorLayer) {
    xNewPos -= _aCursorContext[Layer].pCursor->xHot;
    yNewPos -= _aCursorContext[Layer].pCursor->yHot;
    GUI_SetLayerPosEx(CursorLayer, xNewPos, yNewPos);
  } else {
    if ((_aCursorContext[Layer].x != xNewPos) | (_aCursorContext[Layer].y != yNewPos)) {
      if (_aCursorContext[Layer].hSprite) {
        GUI_SPRITE_SetPosition(_aCursorContext[Layer].hSprite, xNewPos - _aCursorContext[Layer].pCursor->xHot, yNewPos - _aCursorContext[Layer].pCursor->yHot);
      }
      _aCursorContext[Layer].x = xNewPos;
      _aCursorContext[Layer].y = yNewPos;
    }
  }
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_CURSOR_Select
*/
const GUI_CURSOR GUI_UNI_PTR * GUI_CURSOR_Select(const GUI_CURSOR GUI_UNI_PTR * pCursor) {
  return GUI_CURSOR_SelectEx(pCursor, GUI_Context.SelLayer);
}

/*********************************************************************
*
*       GUI_CURSOR_Hide
*/
void GUI_CURSOR_Hide(void) {
  GUI_CURSOR_HideEx(GUI_Context.SelLayer);
}

/*********************************************************************
*
*       GUI_CURSOR_Show
*/
void GUI_CURSOR_Show(void) {
  GUI_CURSOR_ShowEx(GUI_Context.SelLayer);
}

/*********************************************************************
*
*       GUI_CURSOR_GetState
*/
int GUI_CURSOR_GetState(void) {
  return GUI_CURSOR_GetStateEx(GUI_Context.SelLayer);
}

/*********************************************************************
*
*       GUI_CURSOR_SetPosition
*/
void GUI_CURSOR_SetPosition(int xNewPos, int yNewPos) {
  GUI_CURSOR_SetPositionEx(xNewPos, yNewPos, GUI_Context.SelLayer);
}

#else

void GUICurs1_C(void);
void GUICurs1_C(void) {} /* avoid empty object files */

#endif   /* (GUI_SUPPORT_CURSOR) */

/*************************** End of file ****************************/
