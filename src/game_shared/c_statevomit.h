// (C)2006 S2 Games
// c_statevomit.h
//
//=============================================================================
#ifndef __C_STATEVOMIT_H__
#define __C_STATEVOMIT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateVomit
//=============================================================================
class CStateVomit : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, DamagePerSecond)
        DECLARE_ENTITY_CVAR(float, ArmorAdd)
        DECLARE_ENTITY_CVAR(float, ArmorMult)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Vomit);

public:
    ~CStateVomit()  {}
    CStateVomit();

    void    StateFrame();
};
//=============================================================================

#endif //__C_STATEVOMIT_H__
