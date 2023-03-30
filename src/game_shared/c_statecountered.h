// (C)2007 S2 Games
// c_statecountered.h
//
//=============================================================================
#ifndef __C_STATECOUNTERED_H__
#define __C_STATECOUNTERED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCountered
//=============================================================================
class CStateCountered : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Countered)

public:
    ~CStateCountered()  {}
    CStateCountered();
};
//=============================================================================

#endif //__C_STATECOUNTERED_H__
