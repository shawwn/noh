// (C)2006 S2 Games
// c_stateattrition.h
//
//=============================================================================
#ifndef __C_STATEATTRITION_H__
#define __C_STATEATTRITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateAttrition
//=============================================================================
class CStateAttrition : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ArmorFactor)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Attrition);

public:
    ~CStateAttrition()  {}
    CStateAttrition();
};
//=============================================================================

#endif //__C_STATEATTRITION_H__
