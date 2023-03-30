// (C)2005 S2 Games
// c_swatch.h
//
//=============================================================================
#ifndef __C_SWATCH_H__
#define __C_SWATCH_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_panel.h"
//=============================================================================

//=============================================================================
// CSwatch
//=============================================================================
class CSwatch : public CPanel
{
protected:
	CCvarReference	m_refRedCvar;
	CCvarReference	m_refGreenCvar;
	CCvarReference	m_refBlueCvar;
	CCvarReference	m_refAlphaCvar;
	CCvarReference	m_refColorCvar;

public:
	~CSwatch();
	CSwatch(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

	void	Frame(uint uiFrameLength, bool bProcessFrame);
};
//=============================================================================

#endif //__C_SWATCH_H__
