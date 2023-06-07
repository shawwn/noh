// (C)2007 S2 Games
// c_triggerproximity.h
//
//=============================================================================
#ifndef __C_TRIGGERPROXIMITY_H__
#define __C_TRIGGERPROXIMITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_triggerentity.h"
//=============================================================================

//=============================================================================
// CTriggerProximity
//=============================================================================
class CTriggerProximity : public ITriggerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Trigger, Proximity);

    uiset               m_setEntitiesInRadius;

    bool                m_bTriggerOnPlayer;
    bool                m_bTriggerOnNPC;
    bool                m_bTriggerOnBuilding;
    bool                m_bTriggerOnProp;
    bool                m_bTriggerOnGadget;

    float               m_fRadius;

public:
    ~CTriggerProximity()    {}
    CTriggerProximity() :
    ITriggerEntity(GetEntityConfig())
    {}

    virtual void            ApplyWorldEntity(const CWorldEntity &ent);
    virtual void            RegisterEntityScripts(const CWorldEntity &ent);

    virtual void            Copy(const IGameEntity &B);

    GAME_SHARED_API bool    ServerFrame();

    const bool              IsProximityTrigger() const      { return true; }
};
//=============================================================================

#endif //__C_TRIGGERPROXIMITY_H__
