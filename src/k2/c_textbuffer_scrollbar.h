// (C)2005 S2 Games
// c_textbuffer_scrollbar.h
//
//=============================================================================
#ifndef __C_TEXTBUFFER_SCROLLBAR_H__
#define __C_TEXTBUFFER_SCROLLBAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"

#include "c_scrollbar.h"
//=============================================================================


//=============================================================================
// CTextBufferScrollbar
//=============================================================================
class CTextBufferScrollbar : public CScrollbar
{
	void	DoChange();

public:
	~CTextBufferScrollbar()	{}
	CTextBufferScrollbar(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);
};
//=============================================================================

#endif //__C_TEXTBUFFER_SCROLLBAR_H__
