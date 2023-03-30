// (C)2006 S2 Games
// c_stateriposte.h
//
//=============================================================================
#ifndef __C_STATERIPOSTE_H__
#define __C_STATERIPOSTE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateRiposte
//=============================================================================
class CStateRiposte : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, DamageReturn)
        DECLARE_ENTITY_CVAR(tstring, TriggerEffectPath)
        DECLARE_ENTITY_CVAR(tstring, TriggerTargetEffectPath)
        DECLARE_ENTITY_CVAR(float, ArmorMult)
        DECLARE_ENTITY_CVAR(float, ArmorAdd)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Riposte);

public:
    ~CStateRiposte()    {}
    CStateRiposte();

    float   OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker);

    static void         ClientPrecache(CEntityConfig *pConfig);
    static void         ServerPrecache(CEntityConfig *pConfig);
};
//=============================================================================

#endif //__C_STATERIPOSTE_H__
