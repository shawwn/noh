// (C)2006 S2 Games
// c_piegraph.h
//
//=============================================================================
#ifndef __C_PIEGRAPH_H__
#define __C_PIEGRAPH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// CPieGraph
//=============================================================================
class CPieGraph : public IWidget
{
protected:
	float			m_fStart;
	float			m_fEnd;
	float			m_fSign;
	int				m_iDetail;
	bool			m_bSquare;
	bool			m_bWrapTexture;

	float			m_fValue;
	CCvarReference	m_refCvar;

public:
	~CPieGraph();
	CPieGraph(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style);

	void	SetValue(const tstring &s)							{ SetValue(AtoF(s)); }
	void	SetValue(float f)									{ m_fValue = f; }
	tstring	GetValue() const									{ return XtoA(m_fEnd - m_fStart); }
	
	void	RenderWidget(const CVec2f &v2Origin, float fFade);
	void	DrawCircle(const CVec2f &v2Origin);
	void	DrawSquare(const CVec2f &v2Origin);
};
//=============================================================================

#endif //__C_PIEGRAPH_H__
