// (C)2007 S2 Games
// c_statecommandercenturionarmor.h
//
//=============================================================================
#ifndef __C_STATECOMMANDERCENTURIONARMOR_H__
#define __C_STATECOMMANDERCENTURIONARMOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCommanderCenturionArmor
//=============================================================================
class CStateCommanderCenturionArmor : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ArmorMult)
        DECLARE_ENTITY_CVAR(float, ArmorAdd)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, CommanderCenturionArmor);

public:
    ~CStateCommanderCenturionArmor()    {}
    CStateCommanderCenturionArmor();
};
//=============================================================================

#endif //__C_STATECOMMANDERCENTURIONARMOR_H__
