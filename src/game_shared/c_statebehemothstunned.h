// (C)2007 S2 Games
// c_statebehemothstunned.h
//
//=============================================================================
#ifndef __C_STATEBEHEMOTHSTUNNED_H__
#define __C_STATEBEHEMOTHSTUNNED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBehemothStunned
//=============================================================================
class CStateBehemothStunned : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, BehemothStunned);

public:
    ~CStateBehemothStunned()    {}
    CStateBehemothStunned();

    void    Activated();
};
//=============================================================================

#endif //__C_STATEBEHEMOTHSTUNNED_H__
