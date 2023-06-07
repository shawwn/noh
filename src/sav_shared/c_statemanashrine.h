// (C)2006 S2 Games
// c_statemanashrine.h
//
//=============================================================================
#ifndef __C_STATEMANASHRINE_H__
#define __C_STATEMANASHRINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateManaShrine
//=============================================================================
class CStateManaShrine : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ManaRegenBoost)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, ManaShrine);

public:
    ~CStateManaShrine() {}
    CStateManaShrine();
};
//=============================================================================

#endif //__C_STATEMANASHRINE_H__
