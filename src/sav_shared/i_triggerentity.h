// (C)2007 S2 Games
// i_triggerentity.h
//
//=============================================================================
#ifndef __I_TRIGGERENTITY_H__
#define __I_TRIGGERENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
//=============================================================================

//=============================================================================
// ITriggerEntity
//=============================================================================
class ITriggerEntity : public IVisualEntity
{
private:
    static vector<SDataField>   *s_pvFields;

    ITriggerEntity();

protected:
    // Cvar settings
    START_ENTITY_CONFIG(IVisualEntity)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    uint            m_uiLinkedEntityIndex;
    tstring         m_sLinkedEntityName;
    ResHandle       m_hEffect;
    ResHandle       m_hTriggerEffect;

    uint            m_uiEventIndex;

    uint            m_uiTimedStatus;

    void                StartEffect();
    void                StopEffect();

public:
    virtual ~ITriggerEntity();
    ITriggerEntity(CEntityConfig *pConfig);

    // Accessors
    virtual bool        IsTrigger() const               { return true; }
    virtual const bool  IsProximityTrigger() const      { return false; }
    virtual const bool  IsSpawnTrigger() const          { return false; }

    virtual void        Enable(uint uiTime = -1);
    virtual void        Disable(uint uiTime = -1);

    virtual void        Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);
    virtual void        Copy(const IGameEntity &B);
    virtual void        Baseline();
    virtual bool        ServerFrame();
    virtual void        Spawn();

    virtual void        ApplyWorldEntity(const CWorldEntity &ent);

    GAME_SHARED_API virtual void    Trigger(uint uiTriggeringEntIndex, const tstring &sTrigger, bool bPlayEffect = true);

    virtual bool    AIShouldTarget()                                { return false; }
    virtual bool    IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap) { return false; }
};
//=============================================================================

#endif //__I_TRIGGERENTITY_H__
