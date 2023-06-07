// (C)2007 S2 Games
// c_propdynamic.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_propdynamic.h"

#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Prop, Dynamic);
//=============================================================================

/*====================
  CPropDynamic::Kill
  ====================*/
void    CPropDynamic::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
    SetStatus(ENTITY_STATUS_DEAD);
    m_uiCorpseTime = Game.GetGameTime() + m_uiCorpseTime;
    StartAnimation(m_sDeathAnimation, -1);
    Unlink();

    IGameEntity *pEnt(Game.GetFirstEntity());
    while (pEnt)
    {
        if (pEnt->IsPlayer())
        {
            IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

            // Clear this order for all players
            for (byte ySeq(0); ySeq < pPlayer->GetNumOrders(); ySeq++)
                if (pPlayer->GetOrderEntIndex(ySeq) == m_uiIndex)
                    pPlayer->DeleteOrder(ySeq);
            
            if (pPlayer->GetOfficerOrderEntIndex() == m_uiIndex)
                pPlayer->SetOfficerOrder(OFFICERCMD_INVALID);
        }

        pEnt = Game.GetNextEntity(pEnt);
    }

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
  CPropDynamic::Spawn
  ====================*/
void    CPropDynamic::Spawn()
{
    IPropEntity::Spawn();

    SetStatus(ENTITY_STATUS_ACTIVE);

    m_fHealth = GetMaxHealth();

    if (m_uiWorldIndex == INVALID_INDEX)
        m_uiWorldIndex = Game.AllocateNewWorldEntity();

    if (m_pSkeleton != NULL)
        m_pSkeleton->SetModel(GetModelHandle());

    Link();

    StartAnimation(m_sIdleAnimation, -1);
}


/*====================
  CPropDynamic:ApplyWorldEntity
  ====================*/
void    CPropDynamic::ApplyWorldEntity(const CWorldEntity &ent)
{
    IPropEntity::ApplyWorldEntity(ent);

    m_uiCorpseTime = ent.GetPropertyInt(_T("corpsetime"), 7500);
    m_fMaxHealth = ent.GetPropertyFloat(_T("maxhealth"), 0.0f);
    m_sDeathAnimation = ent.GetProperty(_T("deathanim"), _T("death"));
    m_sIdleAnimation = ent.GetProperty(_T("anim"), _T("idle"));
    
    if (GetStatus() != ENTITY_STATUS_DEAD)
        StartAnimation(m_sIdleAnimation, -1);
}


/*====================
  CPropDynamic::ServerFrame
  ====================*/
bool    CPropDynamic::ServerFrame()
{
    // Corpse
    if (GetStatus() == ENTITY_STATUS_DEAD)
        if (Game.GetGameTime() >= m_uiCorpseTime)
            return false;

    return IPropEntity::ServerFrame();
}


/*====================
  CPropDynamic::AllocateSkeleton
  ====================*/
CSkeleton*  CPropDynamic::AllocateSkeleton()
{
    return NULL;
}


/*====================
  CPropDynamic::Link
  ====================*/
void    CPropDynamic::Link()
{
    if (m_uiWorldIndex != INVALID_INDEX)
    {
        CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));

        if (pWorldEnt != NULL)
        {
            pWorldEnt->SetPosition(GetPosition());
            pWorldEnt->SetScale(GetScale());
            pWorldEnt->SetScale2(GetScale2());
            pWorldEnt->SetAngles(GetAngles());
            pWorldEnt->SetModelHandle(GetModelHandle());
            pWorldEnt->SetGameIndex(GetIndex());

            Game.LinkEntity(m_uiWorldIndex, LINK_MODEL|LINK_SURFACE, SURF_PROP);
        }
    }
}
