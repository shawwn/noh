// (C)2007 S2 Games
// c_stateentangled.h
//
//=============================================================================
#ifndef __C_STATEENTANGLED_H__
#define __C_STATEENTANGLED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateEntangled
//=============================================================================
class CStateEntangled : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Entangled);

public:
    ~CStateEntangled()  {}
    CStateEntangled();
};
//=============================================================================

#endif //__C_STATEENTANGLED_H__
