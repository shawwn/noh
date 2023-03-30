// (C)2006 S2 Games
// i_spellsnapcast.h
//
//=============================================================================
#ifndef __I_SPELLSNAPCAST_H__
#define __I_SPELLSNAPCAST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// ISpellSnapCast
//=============================================================================
class ISpellSnapCast : public ISpellItem
{
private:
	ISpellSnapCast();

protected:
	class CCvarSettings : public ISpellItem::CCvarSettings
	{
	private:
		CCvarv3	m_cvarTargetColor;

		CCvarSettings();

	public:
		virtual ~CCvarSettings()	{}
		CCvarSettings(const tstring &sType, const tstring &sName) :
		ISpellItem::CCvarSettings(sType, sName),
		INIT_ITEM_SETTING_CVAR(TargetColor, GREEN.rgb())
		{
		}
	};

	CCvarSettings*	m_pCvarSettings;

public:
	virtual ~ISpellSnapCast()	{}
	ISpellSnapCast(CCvarSettings *pSettings);
};
//=============================================================================

#endif //__I_SPELLSNAPCAST_H__
