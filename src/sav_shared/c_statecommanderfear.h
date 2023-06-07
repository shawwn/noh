// (C)2007 S2 Games
// c_statecommanderfear.h
//
//=============================================================================
#ifndef __C_STATECOMMANDERFEAR_H__
#define __C_STATECOMMANDERFEAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCommanderFear
//=============================================================================
class CStateCommanderFear : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ArmorMult)
        DECLARE_ENTITY_CVAR(float, ArmorAdd)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, CommanderFear);

public:
    ~CStateCommanderFear()  {}
    CStateCommanderFear();
};
//=============================================================================

#endif //__C_STATECOMMANDERFEAR_H__
