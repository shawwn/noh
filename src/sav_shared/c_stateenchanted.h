// (C)2007 S2 Games
// c_stateenchanted.h
//
//=============================================================================
#ifndef __C_STATEENCHANTED_H__
#define __C_STATEENCHANTED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateEnchanted
//=============================================================================
class CStateEnchanted : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Enchanted)

public:
    ~CStateEnchanted()  {}
    CStateEnchanted();
};
//=============================================================================

#endif //__C_STATEENCHANTED_H__
