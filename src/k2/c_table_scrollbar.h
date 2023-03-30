// (C)2005 S2 Games
// c_table_scrollbar.h
//
//=============================================================================
#ifndef __C_TABLE_SCROLLBAR_H__
#define __C_TABLE_SCROLLBAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"

#include "c_scrollbar.h"
//=============================================================================


//=============================================================================
// CTableScrollbar
//=============================================================================
class CTableScrollbar : public CScrollbar
{
public:
    ~CTableScrollbar();
    CTableScrollbar(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);
};
//=============================================================================

#endif //__C_TABLE_SCROLLBAR_H__
