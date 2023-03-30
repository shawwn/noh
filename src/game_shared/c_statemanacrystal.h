// (C)2006 S2 Games
// c_statemanacrystal.h
//
//=============================================================================
#ifndef __C_STATEMANACRYSTAL_H__
#define __C_STATEMANACRYSTAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateManaCrystal
//=============================================================================
class CStateManaCrystal : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ManaCostMultiplier)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, ManaCrystal);

public:
    ~CStateManaCrystal()    {}
    CStateManaCrystal();
};
//=============================================================================

#endif //__C_STATEMANACRYSTAL_H__
