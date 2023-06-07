// (C)2007 S2 Games
// i_gadgetusable.h
//
//=============================================================================
#ifndef __I_GADGETUSABLE_H__
#define __I_GADGETUSABLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// IGadgetUsable
//=============================================================================
class IGadgetUsable : public IGadgetEntity
{
protected:
    START_ENTITY_CONFIG(IGadgetEntity)
        DECLARE_ENTITY_CVAR(tstring, UseEffectPath)
        DECLARE_ENTITY_CVAR(float, UseExperience)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

public:
    virtual ~IGadgetUsable()    {}
    IGadgetUsable(CEntityConfig *pConfig) :
    IGadgetEntity(pConfig),
    m_pEntityConfig(pConfig)
    {
        m_vCounterLabels.push_back(_T("Unique users"));
        m_auiCounter[0] = 0;
    }

    virtual bool    Use(IGameEntity *pActivator);
    virtual bool    UseEffect(IGameEntity *pActivator) = 0;

    static void ClientPrecache(CEntityConfig *pConfig);
    static void ServerPrecache(CEntityConfig *pConfig);
};
//=============================================================================

#endif //__I_GADGETUSABLE_H__
