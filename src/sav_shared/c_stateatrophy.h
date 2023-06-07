// (C)2006 S2 Games
// c_stateatrophy.h
//
//=============================================================================
#ifndef __C_STATEATROPHY_H__
#define __C_STATEATROPHY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateAtrophy
//=============================================================================
class CStateAtrophy : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ArmorMult)
        DECLARE_ENTITY_CVAR(float, ArmorAdd)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Atrophy);

public:
    ~CStateAtrophy()    {}
    CStateAtrophy();
};
//=============================================================================

#endif //__C_STATEATROPHY_H__
