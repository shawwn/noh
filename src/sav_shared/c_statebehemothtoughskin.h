// (C)2007 S2 Games
// c_statebehemothtoughskin.h
//
//=============================================================================
#ifndef __C_STATEBEHEMOTHTOUGHSKIN_H__
#define __C_STATEBEHEMOTHTOUGHSKIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBehemothToughSkin
//=============================================================================
class CStateBehemothToughSkin : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ArmorMult)
        DECLARE_ENTITY_CVAR(float, ArmorAdd)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, BehemothToughSkin);

public:
    ~CStateBehemothToughSkin()  {}
    CStateBehemothToughSkin();
};
//=============================================================================

#endif //__C_STATEBEHEMOTHTOUGHSKIN_H__
