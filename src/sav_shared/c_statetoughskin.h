// (C)2006 S2 Games
// c_statetoughskin.h
//
//=============================================================================
#ifndef __C_STATETOUGHSKIN_H__
#define __C_STATETOUGHSKIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateToughSkin
//=============================================================================
class CStateToughSkin : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ArmorAdd)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, ToughSkin);

public:
    ~CStateToughSkin()  {}
    CStateToughSkin();
};
//=============================================================================

#endif //__C_STATETOUGHSKIN_H__
