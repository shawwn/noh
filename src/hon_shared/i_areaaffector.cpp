// (C)2008 S2 Games
// i_areaaffector.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_areaaffector.h"

#include "i_unitentity.h"
#include "i_bitentity.h"

#include "../k2/c_hostclient.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint                IAffector::s_uiBaseType(ENTITY_BASE_TYPE_AFFECTOR);
ResHandle           IAffector::s_hDebugMaterial(INVALID_RESOURCE);

CVAR_BOOL(d_drawAreaAffectors, false);

DEFINE_ENTITY_DESC(IAffector, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unModifierBits"), TYPE_SHORT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Position"), TYPE_DELTAPOS3D, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Angles[YAW]"), TYPE_ANGLE8, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unVisibilityFlags"), TYPE_SHORT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamID"), TYPE_INT, 3, TEAM_INVALID));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLevel"), TYPE_INT, 5, 0));
}
//=============================================================================

/*====================
  IAffector::IAffector
  ====================*/
IAffector::IAffector() :
m_uiOwnerIndex(INVALID_INDEX),
m_uiAttachTargetUID(INVALID_INDEX),
m_uiProxyUID(INVALID_INDEX),
m_uiIgnoreUID(INVALID_INDEX),
m_uiFirstTargetIndex(INVALID_INDEX),
m_uiCreationTime(INVALID_TIME),
m_uiLastImpactTime(INVALID_TIME),
m_uiTotalImpactCount(0),
m_uiLevel(1),
m_uiChainCount(0),
m_fParam(0.0f),
m_bExpired(false)
{
}


/*====================
  IAffector::Baseline
  ====================*/
void    IAffector::Baseline()
{
    m_unModifierBits = 0;
    m_v3Position = V3_ZERO;
    m_v3Angles = V3_ZERO;
    m_unVisibilityFlags = 0;
    m_uiTeamID = TEAM_PASSIVE;
    m_uiLevel = 1;
}


/*====================
  IAffector::GetSnapshot
  ====================*/
void    IAffector::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    snapshot.WriteField(m_unModifierBits);

    if (uiFlags & SNAPSHOT_HIDDEN)
    {
        snapshot.WriteDeltaPos3D(CVec3f(0.0f, 0.0f, 0.0f));
        snapshot.WriteAngle8(0.0f);
    }
    else
    {
        snapshot.WriteDeltaPos3D(m_v3Position);
        snapshot.WriteAngle8(m_v3Angles.z);
    }

    snapshot.WriteField(m_unVisibilityFlags);
    snapshot.WriteInteger(m_uiTeamID);
    snapshot.WriteField(m_uiLevel);
}


/*====================
  IAffector::ReadSnapshot
  ====================*/
bool    IAffector::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        snapshot.ReadField(m_unModifierBits);
        snapshot.ReadDeltaPos3D(m_v3Position);
        snapshot.ReadAngle8(m_v3Angles.z);
        snapshot.ReadField(m_unVisibilityFlags);
        snapshot.ReadInteger(m_uiTeamID);
        snapshot.ReadField(m_uiLevel);

        Validate();

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IVisualEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  IAffector::Copy
  ====================*/
void    IAffector::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const IAffector *pB(B.GetAsAffector());
    if (!pB)
        return;
    const IAffector &C(*pB);

    m_uiOwnerIndex =            C.m_uiOwnerIndex;
    m_uiCreationTime =          C.m_uiCreationTime;
    m_uiLastImpactTime =        C.m_uiLastImpactTime;
    m_uiTotalImpactCount =      C.m_uiTotalImpactCount;
    m_uiIntervalCount =         C.m_uiIntervalCount;
    m_uiLevel =                 C.m_uiLevel;
}


/*====================
  IAffector::ClientPrecache
  ====================*/
void    IAffector::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IVisualEntity::ClientPrecache(pConfig, eScheme, sModifier);

    K2_WITH_GAME_RESOURCE_SCOPE()
        s_hDebugMaterial = g_ResourceManager.Register(_T("/shared/materials/area_affector.material"), RES_MATERIAL);
}


/*====================
  IAffector::ServerPrecache
  ====================*/
void    IAffector::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IVisualEntity::ServerPrecache(pConfig, eScheme, sModifier);
}


/*====================
  IAffector::Spawn
  ====================*/
void    IAffector::Spawn()
{
    IVisualEntity::Spawn();

    m_uiCreationTime = Game.GetGameTime();
    m_uiTotalImpactCount = 0;
    m_uiIntervalCount = 0;
    m_uiLastImpactTime = INVALID_TIME;
    m_uiLastMoveTime = INVALID_TIME;
    m_yStatus = ENTITY_STATUS_ACTIVE;

    if (Game.IsServer())
    {
        // Update visibility
        SetVisibilityFlags(Game.GetVision(GetPosition().x, GetPosition().y));

        IUnitEntity *pOwner(GetOwner());
        if (pOwner != NULL && (pOwner->GetTeam() == 1 || pOwner->GetTeam() == 2))
            SetVisibilityFlags(VIS_SIGHTED(pOwner->GetTeam()));

        IUnitEntity *pAttach(Game.GetUnitEntity(Game.GetGameIndexFromUniqueID(m_uiAttachTargetUID)));

        CAffectorDefinition *pDefinition(GetDefinition<CAffectorDefinition>(GetModifierBits()));
        if (pDefinition != NULL)
            pDefinition->ExecuteActionScript(ACTION_SCRIPT_SPAWN, this, GetOwner(), this, pAttach, GetPosition(), GetProxy(0), GetLevel());
    }
}


/*====================
  IAffector::GetPotentialImpactCount
  ====================*/
uint    IAffector::GetPotentialImpactCount(uint uiTargetUID, uint uiTotalRemainingImpacts)
{
    if (uiTotalRemainingImpacts == 0)
        return 0;

    // Get previous impact count
    uint uiPrevImpacts(0);
    map<uint, uint>::iterator itFind(m_mapImpacts.find(uiTargetUID));
    if (itFind != m_mapImpacts.end())
        uiPrevImpacts = itFind->second;

    // Determine maximum possible impacts
    uint uiPossibleImapcts(uiTotalRemainingImpacts);
    if (GetMaxImpactsPerTarget() > 0)
    {
        if (uiPrevImpacts >= GetMaxImpactsPerTarget())
            return 0;

        uiPossibleImapcts = MIN(uiPossibleImapcts, GetMaxImpactsPerTarget() - uiPrevImpacts);
    }

    if (GetMaxImpactsPerTargetPerInterval() > 0)
        uiPossibleImapcts = MIN(uiPossibleImapcts, GetMaxImpactsPerTargetPerInterval());

    return uiPossibleImapcts;
}


/*====================
  IAffector::ImpactEntity
  ====================*/
void    IAffector::ImpactEntity(IUnitEntity *pTarget)
{
    if (pTarget == NULL)
        return;

    //Console << _T("IAffector::ImpactEntity ") << Game.GetGameTime() << _T(" ") << pTarget->GetIndex() << newl;

    // Impact effect
    ResHandle hImpactEffect(GetImpactEffect());
    if (hImpactEffect != INVALID_RESOURCE)
    {
        CGameEvent evImpact;
        evImpact.SetSourceEntity(pTarget->GetIndex());
        evImpact.SetEffect(hImpactEffect);
        Game.AddEvent(evImpact);
    }

    // Bridge/Link effect
    ResHandle hBridgeEffect(GetBridgeEffect());
    ResHandle hLinkEffect(GetLinkEffect());
    if (GetChainCount() > 0 && hLinkEffect != INVALID_RESOURCE)
    {
        IUnitEntity *pAttachTarget(GetAttachTarget());
        if (pAttachTarget != NULL)
        {
            CGameEvent evBridge;
            evBridge.SetSourceEntity(pAttachTarget->GetIndex());
            evBridge.SetTargetEntity(pTarget->GetIndex());
            evBridge.SetEffect(hLinkEffect);
            Game.AddEvent(evBridge);
        }
    }
    else if (hBridgeEffect != INVALID_RESOURCE)
    {
        IUnitEntity *pAttachTarget(GetOwner());
        if (pAttachTarget != NULL)
        {
            CGameEvent evBridge;
            evBridge.SetSourceEntity(pAttachTarget->GetIndex());
            evBridge.SetTargetEntity(pTarget->GetIndex());
            evBridge.SetEffect(hBridgeEffect);
            Game.AddEvent(evBridge);
        }
    }
    
    map<uint, uint>::iterator itFind(m_mapImpacts.find(pTarget->GetUniqueID()));
    if (itFind == m_mapImpacts.end())
        m_mapImpacts.insert(pair<uint, uint>(pTarget->GetUniqueID(), 1));
    else
        ++itFind->second;

    ++m_uiTotalImpactCount;

    // Impact event
    CAffectorDefinition *pDefinition(GetDefinition<CAffectorDefinition>(GetModifierBits()));
    if (pDefinition != NULL)
        pDefinition->ExecuteActionScript(ACTION_SCRIPT_IMPACT, this, GetOwner(), this, pTarget, pTarget->GetPosition(), GetProxy(0), GetLevel());
}


/*====================
  IAffector::Impact
  ====================*/
void    IAffector::Impact(vector<IUnitEntity*> &vTargets)
{
    IUnitEntity *pAttach(Game.GetUnitEntity(Game.GetGameIndexFromUniqueID(m_uiAttachTargetUID)));

    // Interval event
    CAffectorDefinition *pDefinition(GetDefinition<CAffectorDefinition>(GetModifierBits()));
    if (pDefinition != NULL)
        pDefinition->ExecuteActionScript(ACTION_SCRIPT_INTERVAL, this, GetOwner(), this, pAttach, GetPosition(), GetProxy(0), GetLevel());
    
    if (vTargets.empty() && m_uiFirstTargetIndex == INVALID_INDEX &&
        GetTargetSelection() != TARGET_SELECT_RANDOM_POSITION &&
        GetTargetSelection() != TARGET_SELECT_RANDOM_ANGLE_DISTANCE)
        return;

    // Determine total possible impacts
    uint uiRemainingImpacts(UINT_MAX);
    if (GetMaxTotalImpacts() > 0)
    {
        if (m_uiTotalImpactCount >= GetMaxTotalImpacts())
            uiRemainingImpacts = 0;
        else
            uiRemainingImpacts = GetMaxTotalImpacts() - m_uiTotalImpactCount;
    }

    if (GetMaxImpactsPerInterval() > 0)
        uiRemainingImpacts = MIN(uiRemainingImpacts, GetMaxImpactsPerInterval());

    // Always hit the first target if it hasn't been hit yet
    if (m_uiFirstTargetIndex != INVALID_INDEX &&
        GetTargetSelection() != TARGET_SELECT_RANDOM_POSITION &&
        GetTargetSelection() != TARGET_SELECT_RANDOM_ANGLE_DISTANCE)
    {
        IUnitEntity *pTarget(Game.GetUnitEntity(m_uiFirstTargetIndex));
        m_uiFirstTargetIndex = INVALID_INDEX;
        if (pTarget != NULL && Game.IsValidTarget(GetTargetScheme(), GetEffectType(), GetOwner(), pTarget))
        {
            ImpactEntity(pTarget);
            if (uiRemainingImpacts <= 1)
                return;
            if (uiRemainingImpacts != UINT_MAX)
                --uiRemainingImpacts;
        }
    }

    vector<IUnitEntity*> vImpacts;

    // Sort the list
    switch (GetTargetSelection())
    {
    case TARGET_SELECT_ALL:
        for (vector<IUnitEntity*>::iterator it(vTargets.begin()); it != vTargets.end(); ++it)
        {
            IUnitEntity *pTarget(*it);

            uint uiNumImpacts(GetPotentialImpactCount(pTarget->GetUniqueID(), uiRemainingImpacts));
            for (uint uiImpacts(0); uiImpacts < uiNumImpacts; ++uiImpacts)
                vImpacts.push_back(pTarget);
            
            uiRemainingImpacts -= uiNumImpacts;
            if (uiRemainingImpacts == 0)
                break;
        }
        break;

    case TARGET_SELECT_CLOSEST:
    case TARGET_SELECT_FURTHEST:
        // Sort vTargets
        for (uint uiPass(0); uiPass < vTargets.size(); ++uiPass)
        {
            float fShortestDistance(FAR_AWAY);
            uint uiClosestIndex(-1);
            for (uint ui(uiPass); ui < vTargets.size(); ++ui)
            {
                float fDistance(DistanceSq(vTargets[ui]->GetPosition().xy(), GetPosition().xy()));
                if (fDistance < fShortestDistance)
                {
                    fShortestDistance = fDistance;
                    uiClosestIndex = ui;
                }
            }

            IUnitEntity *pTemp(vTargets[uiPass]);
            vTargets[uiPass] = vTargets[uiClosestIndex];
            vTargets[uiClosestIndex] = pTemp;
        }

        if (GetTargetSelection() == TARGET_SELECT_FURTHEST)
            std::reverse(vTargets.begin(), vTargets.end());

        // Fill impact vector
        for (vector<IUnitEntity*>::iterator it(vTargets.begin()); it != vTargets.end(); ++it)
        {
            IUnitEntity *pTarget(*it);

            uint uiNumImpacts(GetPotentialImpactCount(pTarget->GetUniqueID(), uiRemainingImpacts));
            for (uint uiImpacts(0); uiImpacts < uiNumImpacts; ++uiImpacts)
                vImpacts.push_back(pTarget);
            
            uiRemainingImpacts -= uiNumImpacts;
            if (uiRemainingImpacts == 0)
                break;
        }
        break;

    case TARGET_SELECT_RANDOM:
        {
            // Fill a vector with all possible impacts
            vector<IUnitEntity*> vPotentialImpacts;
            for (vector<IUnitEntity*>::iterator it(vTargets.begin()); it != vTargets.end(); ++it)
            {
                IUnitEntity *pTarget(*it);

                uint uiNumImpacts(GetPotentialImpactCount(pTarget->GetUniqueID(), uiRemainingImpacts));
                for (uint uiImpacts(0); uiImpacts < uiNumImpacts; ++uiImpacts)
                    vPotentialImpacts.push_back(pTarget);
            }

            std::random_shuffle(vPotentialImpacts.begin(), vPotentialImpacts.end());
            
            if (uiRemainingImpacts < vPotentialImpacts.size())
                vPotentialImpacts.resize(uiRemainingImpacts);
            
            vImpacts.insert(vImpacts.end(), vPotentialImpacts.begin(), vPotentialImpacts.end());
        }
        break;

    case TARGET_SELECT_RANDOM_POSITION:
        {
            CAffectorDefinition *pDefinition(GetDefinition<CAffectorDefinition>(GetModifierBits()));
            if (pDefinition == NULL)
                break;

            if (uiRemainingImpacts == UINT_MAX)
                uiRemainingImpacts = 1;
            for (uint ui(0); ui < uiRemainingImpacts; ++ui)
            {
                CVec2f v2Position(GetPosition().xy() + M_RandomPointInCircle() * GetRadius());
                pDefinition->ExecuteActionScript(ACTION_SCRIPT_IMPACT, this, GetOwner(), this, NULL, Game.GetTerrainPosition(v2Position), GetProxy(0), GetLevel());
            }
        }
        break;

    case TARGET_SELECT_RANDOM_ANGLE_DISTANCE:
        {
            CAffectorDefinition *pDefinition(GetDefinition<CAffectorDefinition>(GetModifierBits()));
            if (pDefinition == NULL)
                break;

            float fOuterRadius(GetRadius());
            float fInnerRadius(GetInnerRadiusOffset() > 0.0f ? MAX(0.0f, fOuterRadius - GetInnerRadiusOffset()) : 0.0f);

            float fArc(GetArc());
            if (fArc == 0.0f)
                fArc = 360.0f;
            float fAngle(GetAngles()[YAW] + GetAngle());

            if (uiRemainingImpacts == UINT_MAX)
                uiRemainingImpacts = 1;
            for (uint ui(0); ui < uiRemainingImpacts; ++ui)
            {
                float fRandomAngle(M_Randnum(-fArc / 2.0f, fArc / 2.0f));
                float fRandomDistance(M_Randnum(fInnerRadius, fOuterRadius));

                CVec2f v2Point(0.0f, fRandomDistance);
                v2Point.Rotate(fRandomAngle + fAngle);

                CVec2f v2Position(GetPosition().xy() + v2Point);
                pDefinition->ExecuteActionScript(ACTION_SCRIPT_IMPACT, this, GetOwner(), this, NULL, Game.GetTerrainPosition(v2Position), GetProxy(0), GetLevel());
            }
        }
        break;
    }

    // Do the impacts
    for (vector<IUnitEntity*>::iterator it(vImpacts.begin()); it != vImpacts.end(); ++it)
        ImpactEntity(*it);
}


/*====================
  IAffector::ServerFrameSetup
  ====================*/
bool    IAffector::ServerFrameSetup()
{
    if (HasLocalFlags(ENT_LOCAL_DELETE_NEXT_FRAME))
        return false;

    ClearVisibilityFlags();

    if (!GetPersist() && m_uiAttachTargetUID != INVALID_INDEX)
    {
        IVisualEntity *pTarget(Game.GetVisualEntity(Game.GetGameIndexFromUniqueID(m_uiAttachTargetUID)));
        if (pTarget == NULL || pTarget->GetStatus() != ENTITY_STATUS_ACTIVE)
        {
            CAffectorDefinition *pDefinition(GetDefinition<CAffectorDefinition>(GetModifierBits()));
            if (pDefinition != NULL)
                pDefinition->ExecuteActionScript(ACTION_SCRIPT_EXPIRED, this, GetOwner(), this, NULL, GetPosition(), GetProxy(0), GetLevel());

            return false;
        }
    }

    return true;
}


/*====================
  IAffector::ServerFrameMovement
  ====================*/
bool    IAffector::ServerFrameMovement()
{
    if (!IVisualEntity::ServerFrameMovement())
        return false;

    // Update visibility
    SetVisibilityFlags(Game.GetVision(GetPosition().x, GetPosition().y));

    IUnitEntity *pOwner(GetOwner());
    if (pOwner != NULL && (pOwner->GetTeam() == 1 || pOwner->GetTeam() == 2))
        SetVisibilityFlags(VIS_SIGHTED(pOwner->GetTeam()));

    if (m_uiLastMoveTime != INVALID_TIME)
    {
        float fSpeed(GetSpeed());

        if (fSpeed != 0.0f)
        {
            CVec3f v3Forward(M_GetForwardVecFromAngles(m_v3Angles));
            m_v3Position += v3Forward * fSpeed * MsToSec(Game.GetFrameLength());
        }
    }
    else
    {
        m_uiLastMoveTime = Game.GetGameTime();
    }

    IVisualEntity *pTarget(Game.GetVisualEntity(Game.GetGameIndexFromUniqueID(m_uiAttachTargetUID)));
    if (pTarget != NULL)
    {
        SetPosition(pTarget->GetPosition());

        if (GetCanTurn())
            SetAngles(pTarget->GetAngles());
    }

    // Snap to terrain
    m_v3Position.z = Game.GetTerrainHeight(m_v3Position.xy());

    return true;
}


/*====================
  IAffector::ServerFrameAction
  ====================*/
bool    IAffector::ServerFrameAction()
{
    if (!IVisualEntity::ServerFrameAction())
        return false;

    IUnitEntity *pAttach(Game.GetUnitEntity(Game.GetGameIndexFromUniqueID(m_uiAttachTargetUID)));

    // Frame event
    CAffectorDefinition *pDefinition(GetDefinition<CAffectorDefinition>(GetModifierBits()));
    if (pDefinition != NULL)
        pDefinition->ExecuteActionScript(ACTION_SCRIPT_FRAME, this, GetOwner(), this, pAttach, GetPosition(), GetProxy(0), GetLevel());

    bool bImpact(false);

    // Determine if we'll impact this frame
    if ((GetMaxTotalImpacts() == 0 || m_uiTotalImpactCount < GetMaxTotalImpacts()) &&
        Game.GetGameTime() >= m_uiCreationTime + GetImpactDelay() &&
        (GetMaxIntervals() == 0 || m_uiIntervalCount < GetMaxIntervals()))
    {
        if (GetImpactInterval() == 0)
        {
            bImpact = true;
        }
        else
        {
            if (m_uiLastImpactTime == INVALID_TIME)
            {
                bImpact = true;
                m_uiLastImpactTime = Game.GetGameTime();
            }

            while (Game.GetGameTime() - m_uiLastImpactTime >= GetImpactInterval())
            {
                bImpact = true;
                m_uiLastImpactTime += GetImpactInterval();
                if (GetMaxTotalImpacts() > 0 && m_uiTotalImpactCount >= GetMaxTotalImpacts())
                    break;
            }
        }
    }

    if (!bImpact)
        return true;

    ++m_uiIntervalCount;

    float fOuterRadius(GetRadius());
    float fInnerRadius(GetInnerRadiusOffset() > 0.0f ? MAX(0.0f, fOuterRadius - GetInnerRadiusOffset()) : 0.0f);
    float fArc(GetArc());
    if (fArc == 0.0f)
        fArc = 360.0f;

    CAxis axisSource(GetAngles() + CVec3f(0.0f, 0.0f, GetAngle()));
    CVec2f v2Forward(axisSource.Forward2d());

    bool bDestroyTrees(GetDestroyTrees());
    IUnitEntity *pOwner(GetOwner());
    uint uiTargetScheme(GetTargetScheme());
    uint uiEffectType(GetEffectType());

    uint uiIgnoreFlags(bDestroyTrees ? REGION_UNITS_AND_TREES : REGION_UNIT);

    // Find all entities within the outer radius
    static uivector vEntities;
    vEntities.clear();

    static vector<IUnitEntity*> vTargets;
    vTargets.clear();

    if (fOuterRadius > 0.0f || m_uiAttachTargetUID == INVALID_INDEX)
    {
        Game.GetEntitiesInRadius(vEntities, GetPosition().xy(), fOuterRadius, uiIgnoreFlags);
        
        for (uivector_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
        {
            uint uiTargetIndex(Game.GetGameIndexFromWorldIndex(*it));
            if (uiTargetIndex == INVALID_INDEX)
                continue;

            IGameEntity *pTargetBase(Game.GetEntity(uiTargetIndex));
            if (pTargetBase == NULL)
                continue;

            if (pTargetBase->IsBit())
            {
                if (!bDestroyTrees || pTargetBase->GetType() != Prop_Tree)
                    continue;

                IBitEntity *pTargetBit(pTargetBase->GetAsBit());
                
                CWorldEntity *pWorldEnt(Game.GetWorldEntity(pTargetBit->GetWorldIndex()));
                if (pWorldEnt == NULL)
                    continue;
                
                // Ignore entities completely within the inner radius
                CVec2f v2TargetDir(pWorldEnt->GetPosition().xy() - GetPosition().xy());
                float fDistance(v2TargetDir.Normalize());
                if (fDistance < MAX(0.0f, fInnerRadius - pWorldEnt->GetBounds().GetDim(X) * 0.5f))
                    continue;

                // Ignore entities outside of the current arc
                if (DotProduct(v2Forward, v2TargetDir) < DEGCOS(fArc / 2.0f))
                    continue;

                pTargetBit->Die(pOwner);
            }
            else
            {
                IUnitEntity *pTarget(pTargetBase->GetAsUnit());
                if (pTarget == NULL)
                    continue;
                
                // Ignore entities completely within the inner radius
                CVec2f v2TargetDir(pTarget->GetPosition().xy() - GetPosition().xy());
                float fDistance(v2TargetDir.Normalize());
                if (fDistance < MAX(0.0f, fInnerRadius - pTarget->GetBounds().GetDim(X) * 0.5f))
                    continue;

                // Ignore entities outside of the current arc
                if (DotProduct(v2Forward, v2TargetDir) < DEGCOS(fArc / 2.0f) - 0.001f)
                    continue;

                // Check targeting rules
                if (!Game.IsValidTarget(uiTargetScheme, uiEffectType, pOwner, pTarget, GetIgnoreInvulnerable()))
                    continue;

                // Execute frame script, this hits all valid targets every frame, ignoring impact rules
                if (pDefinition != NULL)
                    pDefinition->ExecuteActionScript(ACTION_SCRIPT_FRAME_IMPACT, this, pOwner, this, pTarget, pTarget->GetPosition(), GetProxy(0), GetLevel());

                // Skip first target (added in Impact)
                if (pTarget->GetIndex() == m_uiFirstTargetIndex)
                    continue;

                // Skip ignore entity
                if (pTarget->GetUniqueID() == m_uiIgnoreUID)
                    continue;

                // Filter already impacted entities
                if (GetMaxImpactsPerTarget() > 0)
                {
                    map<uint, uint>::iterator itFind(m_mapImpacts.find(pTarget->GetUniqueID()));
                    if (itFind != m_mapImpacts.end() && itFind->second > 0)
                    {
                        if (itFind->second >= GetMaxImpactsPerTarget())
                            continue;
                    }
                }

                vTargets.push_back(pTarget);
            }
        }
    }
    else if (m_uiAttachTargetUID != INVALID_INDEX)
    {
        IUnitEntity *pAttachTarget(Game.GetUnitFromUniqueID(m_uiAttachTargetUID));
        if (pAttachTarget != NULL && Game.IsValidTarget(uiTargetScheme, uiEffectType, pOwner, pAttachTarget, GetIgnoreInvulnerable()))
        {
            // Execute frame impact script, this hits all valid targets every frame, ignoring impact rules
            if (pDefinition != NULL)
                pDefinition->ExecuteActionScript(ACTION_SCRIPT_FRAME_IMPACT, this, pOwner, this, pAttachTarget, pAttachTarget->GetPosition(), GetProxy(0), GetLevel());

            vTargets.push_back(pAttachTarget);
        }
    }

    // Impact with potential targets
    Impact(vTargets);

    return true;
}


/*====================
  IAffector::ServerFrameCleanup
  ====================*/
bool    IAffector::ServerFrameCleanup()
{
    bool bExpire(m_bExpired);

    if (GetLifetime() != 0 && GetLifetime() != uint(-1) && Game.GetGameTime() - m_uiCreationTime > GetLifetime())
        bExpire = true;
    else if (GetLifetime() == 0 && m_uiIntervalCount > 0 && GetMaxIntervals() == 0)
        bExpire = true;
    else if (GetMaxIntervals() != 0 && m_uiIntervalCount >= GetMaxIntervals())
        bExpire = true;

    if (bExpire)
    {
        CAffectorDefinition *pDefinition(GetDefinition<CAffectorDefinition>(GetModifierBits()));
        if (pDefinition != NULL)
            pDefinition->ExecuteActionScript(ACTION_SCRIPT_EXPIRED, this, GetOwner(), this, NULL, GetPosition(), GetProxy(0), GetLevel());

        return false;
    }

    return IVisualEntity::ServerFrameCleanup();
}


/*====================
  IAffector::AddToScene
  ====================*/
bool    IAffector::AddToScene(const CVec4f &v4Color, int iFlags)
{
    CPlayer *pLocalPlayer(Game.GetLocalPlayer());
    if (pLocalPlayer == NULL)
        return false;

    if (!pLocalPlayer->CanSee(this))
        return false;

    CVec3f v3Angles(m_v3Angles + CVec3f(0.0f, 0.0f, GetAngle()));

    if (m_v3AxisAngles != v3Angles)
    {
        m_aAxis.Set(v3Angles);
        m_v3AxisAngles = v3Angles;
    }

    // If the cvar is false or it isn't a practice/replay game, return as it can be used to reveal abilities in game it shouldn't be able to
    if (!d_drawAreaAffectors || (Host.GetActiveClient() != NULL && !(Host.GetActiveClient()->GetPractice() || Host.IsReplay())))
        return true;

    float fOuterRadius(GetRadius());
    float fInnerRadius(GetInnerRadiusOffset() > 0.0f ? MAX(0.0f, fOuterRadius - GetInnerRadiusOffset()) : 0.0f);
    float fArc(GetArc());
    if (fArc == 0.0f)
        fArc = 360.0f;

    CSceneEntity scInnerRing;
    scInnerRing.Clear();
    scInnerRing.width = fInnerRadius;
    scInnerRing.height = fInnerRadius;
    scInnerRing.scale = 1.0f;
    scInnerRing.SetPosition(GetPosition());
    scInnerRing.angle = GetAngles();
    scInnerRing.objtype = OBJTYPE_GROUNDSPRITE;
    scInnerRing.hRes = s_hDebugMaterial;
    scInnerRing.flags = SCENEENT_SOLID_COLOR;
    scInnerRing.color = RED;
    SceneManager.AddEntity(scInnerRing);

    v3Angles[YAW] += fArc / 2.0f;
    CSceneEntity scArc;
    scArc.Clear();
    scArc.width = fOuterRadius;
    scArc.height = fOuterRadius;
    scArc.scale = 1.0f;
    scArc.SetPosition(GetPosition());
    scArc.angle = v3Angles;
    scArc.objtype = OBJTYPE_GROUNDSPRITE;
    scArc.hRes = s_hDebugMaterial;
    scArc.flags = SCENEENT_SOLID_COLOR;
    scArc.color = YELLOW;
    SceneManager.AddEntity(scArc);

    v3Angles[YAW] -= fArc;
    scArc.angle = v3Angles;
    SceneManager.AddEntity(scArc);

    CSceneEntity scOuterRing;
    scOuterRing.Clear();
    scOuterRing.width = fOuterRadius;
    scOuterRing.height = fOuterRadius;
    scOuterRing.scale = 1.0f;
    scOuterRing.SetPosition(GetPosition());
    scOuterRing.angle = GetAngles();
    scOuterRing.objtype = OBJTYPE_GROUNDSPRITE;
    scOuterRing.hRes = s_hDebugMaterial;
    scOuterRing.flags = SCENEENT_SOLID_COLOR;
    scOuterRing.color = LIME;
    SceneManager.AddEntity(scOuterRing);

    return true;
}


/*====================
  IAffector::ExecuteActionScript
  ====================*/
void    IAffector::ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target)
{
    CAffectorDefinition *pDef(GetDefinition<CAffectorDefinition>(GetModifierBits()));
    if (pDef == NULL)
        return;

    pDef->ExecuteActionScript(eScript, this, GetOwner(), this, pTarget, v3Target, GetProxy(0), GetLevel());
}


/*====================
  IAffector::UpdateModifiers
  ====================*/
void    IAffector::UpdateModifiers(const uivector &vModifiers)
{
    m_vModifierKeys = vModifiers;

    uint uiModifierBits(0);
    if (m_uiActiveModifierKey != INVALID_INDEX)
        uiModifierBits |= GetModifierBit(m_uiActiveModifierKey);

    SetModifierBits(uiModifierBits | GetModifierBits(vModifiers));

    // Activate conditional modifiers
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return;

    CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(m_hDefinition));
    if (pResource == NULL)
        return;
    IEntityDefinition *pDefinition(pResource->GetDefinition<IEntityDefinition>());
    if (pDefinition == NULL)
        return;

    const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
    for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
    {
        const tstring &sCondition(cit->second->GetCondition());
        if (sCondition.empty())
            continue;

        tsvector vsTypes(TokenizeString(sCondition, _T(' ')));

        tsvector_cit itType(vsTypes.begin()), itTypeEnd(vsTypes.end());
        for (; itType != itTypeEnd; ++itType)
        {
            if (!itType->empty() && (*itType)[0] == _T('!'))
            {
                if (pOwner->IsTargetType(itType->substr(1), pOwner))
                    break;
            }
            else
            {
                if (!pOwner->IsTargetType(*itType, pOwner))
                    break;
            }
        }
        if (itType == itTypeEnd)
            SetModifierBits(GetModifierBits() | cit->first);
    }
}


/*====================
  IAffector::SnapshotUpdate
  ====================*/
void    IAffector::SnapshotUpdate()
{
    SetEffect(0, GetEffect());
}
