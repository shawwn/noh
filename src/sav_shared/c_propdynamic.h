// (C)2007 S2 Games
// c_propdestructable.h
//
//=============================================================================
#ifndef __C_PROPDYNAMIC_H__
#define __C_PROPDYNAMIC_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propentity.h"
//=============================================================================

//=============================================================================
// CPropDynamic
//=============================================================================
class CPropDynamic : public IPropEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Prop, Dynamic);

    uint                    m_uiCorpseTime;
    float                   m_fMaxHealth;

    tstring                 m_sDeathAnimation;
    tstring                 m_sIdleAnimation;

public:
    ~CPropDynamic() {}

    CPropDynamic() :
    IPropEntity(GetEntityConfig()),
    m_uiCorpseTime(7500),
    m_fMaxHealth(0.0f),
    m_sDeathAnimation(_T("death"))
    {}

    virtual bool        IsStatic() const                { return false; }
    virtual void        ApplyWorldEntity(const CWorldEntity &ent);
    virtual void        Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

    virtual CSkeleton*  AllocateSkeleton();
    virtual void        Spawn();
    virtual bool        ServerFrame();

    virtual void        Link();

    virtual float       GetMaxHealth() const            { return m_fMaxHealth; }
    virtual void        SetMaxHealth(float fValue)      { m_fMaxHealth = fValue; }
};
//=============================================================================

#endif //__C_PROPDYNAMIC_H__
