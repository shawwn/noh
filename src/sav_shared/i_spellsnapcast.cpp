// (C)2006 S2 Games
// i_spellsnapcast.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_spellsnapcast.h"
//=============================================================================

/*====================
  ISpellSnapCast::ISpellSnapCast
  ====================*/
ISpellSnapCast::ISpellSnapCast(CCvarSettings *pSettings) :
ISpellItem(pSettings),
m_pCvarSettings(pSettings)
{
}
