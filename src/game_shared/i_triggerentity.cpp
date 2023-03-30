// (C)2007 S2 Games
// i_triggerentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_triggerentity.h"
#include "../k2/c_function.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*     ITriggerEntity::s_pvFields;
//=============================================================================

/*====================
  ITriggerEntity::CEntityConfig::CEntityConfig
  ====================*/
ITriggerEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName)
{
}


/*====================
  ITriggerEntity::ITriggerEntity
  ====================*/
ITriggerEntity::ITriggerEntity(CEntityConfig *pConfig) :
IVisualEntity(pConfig),
m_pEntityConfig(pConfig),
m_uiLinkedEntityIndex(INVALID_INDEX),
m_hEffect(INVALID_RESOURCE),
m_hTriggerEffect(INVALID_RESOURCE),
m_uiEventIndex(INVALID_INDEX),
m_uiTimedStatus(INVALID_TIME)
{
}


/*====================
  ITriggerEntity::~ITriggerEntity
  ====================*/
ITriggerEntity::~ITriggerEntity()
{
    StopEffect();

    if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
    {
        Game.UnlinkEntity(m_uiWorldIndex);
        Game.DeleteWorldEntity(m_uiWorldIndex);
    }
}


/*====================
  ITriggerEntity::Spawn
  ====================*/
void    ITriggerEntity::Spawn()
{
    IVisualEntity::Spawn();

    SetStatus(ENTITY_STATUS_ACTIVE);
    StartEffect();
}


/*====================
  ITriggerEntity::Trigger
  ====================*/
void    ITriggerEntity::Trigger(uint uiTriggeringEntIndex, const tstring &sTriggerName, bool bPlayEffect)
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE || !Game.IsServer())
        return;

    tstring sTrigger(sTriggerName);

    if (sTrigger.empty())
        sTrigger = _T("trigger");

    if (bPlayEffect && m_hTriggerEffect != INVALID_RESOURCE)
    {
        CWorldEntity *pEnt(Game.GetWorldEntity(uiTriggeringEntIndex));

        if (pEnt != NULL)
        {
            CGameEvent ev;
            ev.SetSourceEntity(pEnt->GetGameIndex());
            ev.SetEffect(m_hTriggerEffect);
            ev.Spawn();
            Game.AddEvent(ev);
        }
    }

    IGameEntity *pLinkedEnt(Game.GetVisualEntity(m_uiLinkedEntityIndex));

    Game.RegisterTriggerParam(_T("triggerindex"), XtoA(GetIndex()));
    Game.RegisterTriggerParam(_T("index"), XtoA(uiTriggeringEntIndex));

    if (pLinkedEnt != NULL)
        Game.RegisterTriggerParam(_T("linkedindex"), XtoA(pLinkedEnt->GetIndex()));
    else
        Game.RegisterTriggerParam(_T("linkedindex"), _T("-1"));

    Game.TriggerEntityScript(GetIndex(), sTrigger);
}


/*====================
  ITriggerEntity::Baseline
  ====================*/
void    ITriggerEntity::Baseline()
{
    IVisualEntity::Baseline();

    m_uiLinkedEntityIndex = INVALID_INDEX;

    m_hEffect = INVALID_RESOURCE;
    m_hTriggerEffect = INVALID_RESOURCE;
    m_uiEventIndex = INVALID_INDEX;

    m_uiTimedStatus = INVALID_TIME;
}


/*====================
  ITriggerEntity::Kill
  ====================*/
void    ITriggerEntity::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
    Disable();

    tstring sMethod(_T("Unknown"));
    if (unKillingObjectID != INVALID_ENT_TYPE)
    {
        ICvar *pCvar(EntityRegistry.GetGameSetting(unKillingObjectID, _T("Name")));

        if (pCvar != NULL)
            sMethod = pCvar->GetString();
    }

    Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));
    Game.RegisterTriggerParam(_T("attackingindex"), XtoA(pAttacker != NULL ? pAttacker->GetIndex() : INVALID_INDEX));
    Game.RegisterTriggerParam(_T("method"), sMethod);
    Game.TriggerEntityScript(GetIndex(), _T("death"));
}


/*====================
  ITriggerEntity::Copy
  ====================*/
void    ITriggerEntity::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const ITriggerEntity *pB(B.GetAsTrigger());

    if (!pB)    
        return;

    const ITriggerEntity &C(*pB);

    m_uiLinkedEntityIndex = C.m_uiLinkedEntityIndex;
    m_hEffect =             C.m_hEffect;
    m_hTriggerEffect =      C.m_hTriggerEffect;
    m_uiEventIndex =        C.m_uiEventIndex;

    m_uiTimedStatus =       C.m_uiTimedStatus;
}


/*====================
  ITriggerEntity::ApplyWorldEntity
  ====================*/
void    ITriggerEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);

    m_sLinkedEntityName = ent.GetProperty(_T("linkedentity"), _T(""));
    m_hEffect = Game.RegisterEffect(ent.GetProperty(_T("effect")));
    m_hTriggerEffect = Game.RegisterEffect(ent.GetProperty(_T("triggereffect")));
}


/*====================
  ITriggerEntity::StopEffect
  ====================*/
void    ITriggerEntity::StopEffect()
{
    if (m_uiLinkedEntityIndex == INVALID_INDEX)
    {
        if (m_uiEventIndex != INVALID_INDEX)
        {
            Game.DeleteEvent(m_uiEventIndex);
            m_uiEventIndex = INVALID_INDEX;
        }
    }
    else
    {
        IVisualEntity *pEnt(Game.GetVisualEntity(m_uiLinkedEntityIndex));

        if (pEnt != NULL)
        {
            if (pEnt->GetEffect(EFFECT_CHANNEL_TRIGGER) == m_hEffect)
            {
                pEnt->SetEffect(EFFECT_CHANNEL_TRIGGER, INVALID_RESOURCE);
                pEnt->IncEffectSequence(EFFECT_CHANNEL_TRIGGER);
            }
        }
    }
}


/*====================
  ITriggerEntity::StartEffect
  ====================*/
void    ITriggerEntity::StartEffect()
{
    if (m_hEffect != INVALID_RESOURCE)
    {
        if (m_uiLinkedEntityIndex == INVALID_INDEX)
        {
            if (m_uiEventIndex != INVALID_INDEX)
                Game.DeleteEvent(m_uiEventIndex);

            CGameEvent ev;
            ev.SetSourcePosition(GetPosition());
            ev.SetSourceAngles(GetAngles());
            ev.SetEffect(m_hEffect);
            ev.Spawn();
            m_uiEventIndex = Game.AddEvent(ev);
        }
    }
}


/*====================
  ITriggerEntity::Enable
  ====================*/
void    ITriggerEntity::Enable(uint uiTime)
{
    SetStatus(ENTITY_STATUS_ACTIVE);
    StartEffect();

    if (uiTime != INVALID_TIME)
        m_uiTimedStatus = Game.GetGameTime() + uiTime;
    else
        m_uiTimedStatus = INVALID_TIME;
}


/*====================
  ITriggerEntity::Disable
  ====================*/
void    ITriggerEntity::Disable(uint uiTime)
{
    SetStatus(ENTITY_STATUS_DORMANT);
    StopEffect();

    if (uiTime != INVALID_TIME)
        m_uiTimedStatus = Game.GetGameTime() + uiTime;
    else
        m_uiTimedStatus = INVALID_TIME;
}


/*====================
  ITriggerEntity::ServerFrame
  ====================*/
bool    ITriggerEntity::ServerFrame()
{
    IVisualEntity::ServerFrame();

    if (m_uiTimedStatus != INVALID_TIME && Game.GetGameTime() > m_uiTimedStatus)
    {
        if (GetStatus() == ENTITY_STATUS_ACTIVE)
            Disable();
        else
            Enable();

        m_uiTimedStatus = INVALID_TIME;
    }

    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return true;

    if (!m_sLinkedEntityName.empty())
    {
        if (m_uiLinkedEntityIndex == INVALID_INDEX)
        {
            WorldEntMap mapEnts(Game.GetWorldEntityMap());

            for (WorldEntMap::iterator it(mapEnts.begin()); it != mapEnts.end(); it++)
            {
                CWorldEntity *pWorldEnt(Game.GetWorldEntity(it->first));

                if (pWorldEnt != NULL && pWorldEnt->GetName() == m_sLinkedEntityName)
                {
                    m_uiLinkedEntityIndex = pWorldEnt->GetGameIndex();
                    break;
                }
            }

        }

        IVisualEntity *pEnt(Game.GetVisualEntity(m_uiLinkedEntityIndex));

        if (pEnt != NULL && pEnt->GetStatus() == ENTITY_STATUS_ACTIVE)
        {
            SetPosition(pEnt->GetPosition());
            SetAngles(pEnt->GetAngles());

            CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));

            if (pWorldEnt != NULL)
            {
                pWorldEnt->SetPosition(GetPosition());
                pWorldEnt->SetAngles(GetAngles());
            }

            if (m_hEffect != INVALID_RESOURCE && pEnt->GetEffect(EFFECT_CHANNEL_TRIGGER) != m_hEffect)
            {
                pEnt->SetEffect(EFFECT_CHANNEL_TRIGGER, m_hEffect);
                pEnt->IncEffectSequence(EFFECT_CHANNEL_TRIGGER);
            }
        }
        else if (m_uiLinkedEntityIndex != INVALID_INDEX)
        {
            Disable();
            return false;
        }
    }
    else if (m_hEffect != INVALID_RESOURCE && m_uiEventIndex == INVALID_INDEX)
        StartEffect();

    return true;
}
