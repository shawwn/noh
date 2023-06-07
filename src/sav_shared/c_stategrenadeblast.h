// (C)2006 S2 Games
// c_stategrenadeblast.h
//
//=============================================================================
#ifndef __C_STATEGREANDEBLAST_H__
#define __C_STATEGREANDEBLAST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateGrenadeBlast
//=============================================================================
class CStateGrenadeBlast : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, DamagePerSecond)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, GrenadeBlast);

public:
    ~CStateGrenadeBlast()   {}
    CStateGrenadeBlast();

    void    StateFrame();
};
//=============================================================================

#endif //__C_STATEGREANDEBLAST_H__
