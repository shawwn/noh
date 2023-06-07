// (C)2007 S2 Games
// c_stateofficer.h
//
//=============================================================================
#ifndef __C_STATEOFFICER_H__
#define __C_STATEOFFICER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateOfficer
//=============================================================================
class CStateOfficer : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, HealthRegenMult)
        DECLARE_ENTITY_CVAR(float, ManaRegenMult)
        DECLARE_ENTITY_CVAR(float, StaminaRegenMult)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Officer);

public:
    ~CStateOfficer()    {}
    CStateOfficer();
};
//=============================================================================

#endif //__C_STATEOFFICER_H__
