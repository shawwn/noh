// (C)2005 S2 Games
// c_table_scrollbar.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_table_scrollbar.h"
#include "c_table.h"
//=============================================================================

/*====================
  CTableScrollbar::~CTableScrollbar
  ====================*/
CTableScrollbar::~CTableScrollbar()
{
}


/*====================
  CTableScrollbar::CTableScrollbar
  ====================*/
CTableScrollbar::CTableScrollbar(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
CScrollbar(pInterface, pParent, style)
{
}
