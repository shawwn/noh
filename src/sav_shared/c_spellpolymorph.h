// (C)2007 S2 Games
// c_spellpolymorph.h
//
//=============================================================================
#ifndef __C_SPELLPOLYMORPH_H__
#define __C_SPELLPOLYMORPH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellPolymorph
//=============================================================================
class CSpellPolymorph : public ISpellItem
{
private:
    START_ENTITY_CONFIG(ISpellItem)
        DECLARE_ENTITY_CVAR(tstring, EndAnimName)
        DECLARE_ENTITY_CVAR(uint, FinishTime)
        DECLARE_ENTITY_CVAR(float, ManaCostPerSecond)
        DECLARE_ENTITY_CVAR(uint, MaxTime)
    END_ENTITY_CONFIG
    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Spell, Polymorph)
    
    bool    m_bActive;
    int     m_iSelfStateSlot;
    uint    m_uiActivationTime;

public:
    ~CSpellPolymorph()  {}
    CSpellPolymorph() :
    ISpellItem(GetEntityConfig()),
    m_pEntityConfig(GetEntityConfig()),
    m_bActive(false),
    m_iSelfStateSlot(-1),
    m_uiActivationTime(0)
    {}
    bool    IsSpellToggle() const   { return true; }

    void    ActiveFrame();

    void    Deactivate();
    bool    ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget = true);

    ENTITY_CVAR_ACCESSOR(tstring, EndAnimName, _T(""))
    ENTITY_CVAR_ACCESSOR(uint, FinishTime, 0)
    ENTITY_CVAR_ACCESSOR(float, ManaCostPerSecond, 0.0f)
};
//=============================================================================

#endif //__C_SPELLPOLYMORPH_H__
