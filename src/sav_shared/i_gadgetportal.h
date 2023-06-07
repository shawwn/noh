// (C)2007 S2 Games
// i_gadgetportal.h
//
//=============================================================================
#ifndef __I_GADGETPORTAL_H__
#define __I_GADGETPORTAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// IGadgetPortal
//=============================================================================
class IGadgetPortal : public IGadgetEntity
{
protected:
    START_ENTITY_CONFIG(IGadgetEntity)
        DECLARE_ENTITY_CVAR(float, UseExperience)
    END_ENTITY_CONFIG
    
    CEntityConfig*  m_pEntityConfig;

public:
    virtual ~IGadgetPortal()    {}
    IGadgetPortal(CEntityConfig *pConfig) :
    IGadgetEntity(pConfig),
    m_pEntityConfig(pConfig)
    {
        m_auiCounter[0] = 0;
        m_vCounterLabels.push_back(_T("Uses"));
    }

    virtual const IGadgetPortal*    GetAsPortalGadget() const   { return this; }
    virtual IGadgetPortal*          GetAsPortalGadget()         { return this; }

    virtual void    Baseline();
    virtual void    Spawn();

    virtual bool    CanSpawnFrom(IPlayerEntity *pPlayer);
    virtual void    PlayerSpawnedFrom(IPlayerEntity *pPlayer);
};
//=============================================================================

#endif //__I_GADGETPORTAL_H__
