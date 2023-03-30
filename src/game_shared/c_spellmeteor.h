// (C)2007 S2 Games
// c_spellmeteor.h
//
//=============================================================================
#ifndef __C_SPELLMETEOR_H__
#define __C_SPELLMETEOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellMeteor
//=============================================================================
class CSpellMeteor : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, Meteor)

public:
    ~CSpellMeteor() {}
    CSpellMeteor() :
    ISpellItem(GetEntityConfig())
    {}

    bool    TryImpact();

    static void             ClientPrecache(CEntityConfig *pConfig);
    static void             ServerPrecache(CEntityConfig *pConfig);
};
//=============================================================================

#endif //__C_SPELLMETEOR_H__
