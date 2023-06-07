// (C)2007 S2 Games
// i_buildingmine.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_buildingmine.h"
#include "c_teaminfo.h"
#include "c_entityclientinfo.h"

#include "../k2/c_sceneentity.h"
#include "../k2/c_scenemanager.h"
//=============================================================================

/*====================
  IBuildingMine::HarvestGold
  ====================*/
uint    IBuildingMine::HarvestGold()
{
    IPropEntity *pProp(Game.GetPropEntity(m_uiFoundation));
    if (pProp == NULL)
        return 0;
    IPropFoundation *pMine(pProp->GetAsFoundation());
    if (pMine == NULL)
        return 0;

    float fInitialGoldPercent(pMine->GetRemainingGoldPercent());
    uint uiGoldHarvested(pMine->HarvestGold());
    if (fInitialGoldPercent >= 0.15f && pMine->GetRemainingGoldPercent() < 0.15f)
    {
        ivector vClients(Game.GetTeam(GetTeam())->GetClientList());

        CBufferFixed<6> buffer;
        buffer << GAME_CMD_GOLD_MINE_LOW << GetIndex();

        for (ivector_cit it(vClients.begin()); it != vClients.end(); ++it)
            Game.SendGameData(*it, buffer, true);
    }
    if (fInitialGoldPercent > 0.0f && pMine->GetRemainingGoldPercent() <= 0.0f)
    {
        ivector vClients(Game.GetTeam(GetTeam())->GetClientList());

        CBufferFixed<6> buffer;
        buffer << GAME_CMD_GOLD_MINE_DEPLETED << GetIndex();

        StartAnimation(_T("idle_empty"), 0);

        for (ivector_cit it(vClients.begin()); it != vClients.end(); ++it)
            Game.SendGameData(*it, buffer, true);
    }

    return uiGoldHarvested;
}


/*====================
  IBuildingMine::GetIncomeAmount
  ====================*/
uint    IBuildingMine::GetIncomeAmount() const
{
    IPropEntity *pProp(Game.GetPropEntity(m_uiFoundation));
    if (pProp == NULL)
        return 0;
    IPropFoundation *pMine(pProp->GetAsFoundation());
    if (pMine == NULL)
        return 0;

    return MIN(pMine->GetRemainingGold(), pMine->GetHarvestRate());
}


/*====================
  IBuildingMine::GetIncomeAmount
  ====================*/
void    IBuildingMine::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
    CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam != NULL)
    {
        IPropEntity *pProp(Game.GetPropEntity(m_uiFoundation));
        if (pProp != NULL)
        {
            IPropFoundation *pMine(pProp->GetAsFoundation());
            if (pMine != NULL)
                pTeam->GiveGold(pMine->RaidGold());
        }
    }

    IBuildingEntity::Kill(pAttacker, unKillingObjectID);
}


/*====================
  IBuildingMine::AddToScene
  ====================*/
bool    IBuildingMine::AddToScene(const CVec4f &v4Color, int iFlags)
{
    if (m_hModel == INVALID_INDEX)
        return false;

    CEntityClientInfo *pLocalClient(Game.GetClientInfo(Game.GetLocalClientNum()));
    if (GetStatus() == ENTITY_STATUS_PREVIEW &&
        pLocalClient != NULL &&
        pLocalClient->GetTeam() != GetTeam())
        return false;

    CVec4f v4TintedColor(v4Color);

    if (Game.IsCommander() && GetStatus() != ENTITY_STATUS_PREVIEW)
    {
        if (!(m_bSighted || m_bPrevSighted))
            return false;
    
        if (!m_bSighted)
        {
            //v4TintedColor[R] *= 0.333f;
            //v4TintedColor[G] *= 0.333f;
            //v4TintedColor[B] *= 0.333f;
        }
    }

    if (m_v3AxisAngles != m_v3Angles)
    {
        m_aAxis.Set(m_v3Angles);
        m_v3AxisAngles = m_v3Angles;
    }

    static CSceneEntity sceneEntity;

    sceneEntity.Clear();

    sceneEntity.scale = GetScale() * GetScale2();
    sceneEntity.SetPosition(m_v3Position);
    sceneEntity.axis = m_aAxis;
    sceneEntity.objtype = OBJTYPE_MODEL;
    sceneEntity.hModel = m_hModel;
    sceneEntity.skeleton = m_pSkeleton;
    sceneEntity.color = v4TintedColor;
    sceneEntity.flags = iFlags | SCENEOBJ_SOLID_COLOR | SCENEOBJ_USE_AXIS;

    bool bEmpty(false);
    IPropEntity *pProp(Game.GetPropEntity(m_uiFoundation));
    if (pProp != NULL)
    {
        IPropFoundation *pMine(pProp->GetAsFoundation());
        if (pMine != NULL)
        {
            if (pMine->GetRemainingGold() == 0)
                bEmpty = true;
        }
    }

    if (bEmpty)
    {
        if (Game.LooksLikeEnemy(m_uiIndex))
            sceneEntity.hSkin = g_ResourceManager.GetSkin(GetModelHandle(), _T("red_empty"));
        else
            sceneEntity.hSkin = g_ResourceManager.GetSkin(GetModelHandle(), _T("empty"));
    }
    else
    {
        if (Game.LooksLikeEnemy(m_uiIndex))
            sceneEntity.hSkin = g_ResourceManager.GetSkin(GetModelHandle(), _T("red"));
    }

    if (m_uiClientRenderFlags & ECRF_SNAPSELECTED)
        sceneEntity.color *= m_v4SelectColor;

    if (m_uiClientRenderFlags & ECRF_HALFTRANSPARENT)
        sceneEntity.color[A] *= 0.5f;

    SSceneEntityEntry &cEntry(SceneManager.AddEntity(sceneEntity));

    if (!cEntry.bCull || !cEntry.bCullShadow)
    {
        if (GetStatus() != ENTITY_STATUS_PREVIEW)
            AddSelectionRingToScene();
        
        UpdateSkeleton(true);
    }
    else
    {
        UpdateSkeleton(false);
    }

    return true;
}
