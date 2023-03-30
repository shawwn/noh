// (C)2006 S2 Games
// c_cliententity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_cliententity.h"
#include "c_gameclient.h"

#include "../hon_shared/c_player.h"
#include "../hon_shared/c_entityeffect.h"
#include "../hon_shared/c_teaminfo.h"
#include "../hon_shared/i_projectile.h"
#include "../hon_shared/i_unitentity.h"
#include "../hon_shared/i_gadgetentity.h"
#include "../hon_shared/i_heroentity.h"

#include "../k2/c_entitysnapshot.h"
#include "../k2/c_world.h"
#include "../k2/c_effect.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_particlesystem.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_draw2d.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_input.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_texture.h"
#include "../k2/c_sceneentitymodifier.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_model.h"
#include "../k2/c_voicemanager.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOL       (cg_debugServerSnapshot,                false);
CVAR_BOOL       (cg_debugMeleeAngles,                   false);
CVAR_STRING     (cg_effectTeamPlayer,                   "/shared/effects/team_player.effect");
CVAR_STRING     (cg_effectEye,                          "/shared/effects/eye.effect");
CVAR_STRING     (cg_talkingEffect,                      "/shared/effects/talking.efect");
CVAR_FLOAT      (cg_entityCullRange,                    2500.0f);
//=============================================================================

/*====================
  CClientEntity::~CClientEntity
  ====================*/
CClientEntity::~CClientEntity()
{
}


/*====================
  CClientEntity::CClientEntity
  ====================*/
CClientEntity::CClientEntity() :
m_uiFlags(0),

m_uiIndex(INVALID_INDEX),
m_unType(0),

m_hModel(INVALID_RESOURCE),
m_pSkeleton(NULL),

m_pNextState(NULL),
m_pPrevState(NULL),
m_pCurrentState(NULL),

m_iTalkingEffectChannel(-1),
m_iDisconnectedEffectChannel(-1),
m_iIllusionEffectChannel(-1),
m_iTeamTargetChannel(-1),

m_v3PositionLinked(FAR_AWAY, FAR_AWAY, FAR_AWAY),
m_yStatusLinked(EEntityStatus(-1)),
m_fScaleLinked(0.0f),
m_hModelLinked(INVALID_RESOURCE),

m_hNextClientEntity(INVALID_POOL_OFFSET)
{
    for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
        m_apEffectThread[i] = NULL;

    for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
        m_ahSoundHandle[i] = INVALID_INDEX;
}


/*====================
  CClientEntity::SetIndex
  ====================*/
void    CClientEntity::SetIndex(uint uiIndex)
{
    m_uiIndex = uiIndex;

    if (m_pNextState == NULL ||
        m_pPrevState == NULL ||
        m_pCurrentState == NULL)
        return;

    m_pNextState->SetIndex(m_uiIndex);
    m_pPrevState->SetIndex(m_uiIndex);
    m_pCurrentState->SetIndex(m_uiIndex);
}


/*====================
  CClientEntity::SetType
  ====================*/
void    CClientEntity::SetType(ushort unType)
{
    m_unType = unType;

    if (m_pNextState == NULL ||
        m_pPrevState == NULL ||
        m_pCurrentState == NULL)
        return;

    m_pNextState->SetType(m_unType);
    m_pPrevState->SetType(m_unType);
    m_pCurrentState->SetType(m_unType);
}


/*====================
  CClientEntity::SetSkeleton
  ====================*/
void    CClientEntity::SetSkeleton(CSkeleton *pSkeleton)
{
    m_pSkeleton = pSkeleton;

    if (m_pNextState == NULL ||
        m_pPrevState == NULL ||
        m_pCurrentState == NULL)
        return;

    m_pNextState->SetSkeleton(m_pSkeleton);
    m_pPrevState->SetSkeleton(m_pSkeleton);
    m_pCurrentState->SetSkeleton(m_pSkeleton);

    if (m_pSkeleton != NULL)
        m_pSkeleton->SetDefaultAnim(_CWS("idle"));
}


/*====================
  CClientEntity::GetWorldIndex
  ====================*/
uint    CClientEntity::GetWorldIndex() const
{
    return m_pNextState->GetWorldIndex();
}


/*====================
  CClientEntity::GetClientID
  ====================*/
int     CClientEntity::GetClientID() const
{
    if (m_pNextState != NULL && m_pNextState->IsUnit())
        return m_pNextState->GetAsUnit()->GetOwnerClientNumber();
    else
        return -1;
}


/*====================
  CClientEntity::Free
  ====================*/
void    CClientEntity::Free()
{
    const uint uiWorldIndex(m_pNextState ? m_pNextState->GetWorldIndex() : INVALID_INDEX);

    if (uiWorldIndex != INVALID_INDEX && GameClient.WorldEntityExists(uiWorldIndex))
    {
        GameClient.UnlinkEntity(uiWorldIndex);
        GameClient.DeleteWorldEntity(uiWorldIndex);
    }

    for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
        SAFE_DELETE(m_apEffectThread[i]);

    for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
    {
        if (m_ahSoundHandle[i] != INVALID_INDEX)
            K2SoundManager.StopHandle(m_ahSoundHandle[i]);
    }

    SAFE_DELETE(m_pSkeleton);

    if (m_pNextState != NULL && m_pNextState->IsGadget())
    {
        IGadgetEntity *pGadget(m_pNextState->GetAsGadget());

        CClientEntity *pOwner(GameClient.GetClientEntity(pGadget->GetMountIndex()));
        if (pOwner != NULL && pOwner->GetNextEntity() != NULL && pOwner->GetNextEntity()->IsUnit())
        {
            if (pOwner->GetNextEntity()->GetAsUnit() != NULL && 
                pOwner->GetNextEntity()->GetAsUnit()->GetMount() == pGadget)
                pOwner->GetNextEntity()->GetAsUnit()->SetMount(NULL);
            if (pOwner->GetPrevEntity() != NULL && 
                pOwner->GetPrevEntity()->GetAsUnit() != NULL && 
                pOwner->GetPrevEntity()->GetAsUnit()->GetMount() == pGadget)
                pOwner->GetPrevEntity()->GetAsUnit()->SetMount(NULL);
            if (pOwner->GetCurrentEntity() != NULL && 
                pOwner->GetCurrentEntity()->GetAsUnit() != NULL && 
                pOwner->GetCurrentEntity()->GetAsUnit()->GetMount() == pGadget)
                pOwner->GetCurrentEntity()->GetAsUnit()->SetMount(NULL);
        }
    }

    if (m_pNextState != NULL &&
        m_pNextState != NULL &&
        m_pPrevState != NULL &&
        m_pCurrentState != NULL &&
        m_pNextState->IsUnit())
    {
        // Only set inventory pointers to NULL because clients already delete slaves when the entity slave snapshot is deleted
        for (int i(0); i < MAX_INVENTORY; ++i)
        {
            m_pNextState->GetAsUnit()->SetInventorySlot(i, NULL);
            m_pPrevState->GetAsUnit()->SetInventorySlot(i, NULL);
            m_pCurrentState->GetAsUnit()->SetInventorySlot(i, NULL);
        }

        m_pNextState->GetAsUnit()->SetMount(NULL);
        m_pPrevState->GetAsUnit()->SetMount(NULL);
        m_pCurrentState->GetAsUnit()->SetMount(NULL);
    }

    SAFE_DELETE(m_pNextState);
    SAFE_DELETE(m_pPrevState);
    SAFE_DELETE(m_pCurrentState);

    m_hModel = INVALID_RESOURCE;
}


/*====================
  CClientEntity::Initialize
  ====================*/
void    CClientEntity::Initialize(IVisualEntity *pEntity)
{
    try
    {
        m_hNextClientEntity = INVALID_POOL_OFFSET;
        m_uiFlags = 0;

        for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        {
            m_aiActiveAnim[i] = -1;
            m_ayActiveAnimSequence[i] = 0;
        }

        for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
        {
            m_ahActiveEffect[i] = INVALID_RESOURCE;
            m_ayActiveEffectSequence[i] = 0;
            SAFE_DELETE(m_apEffectThread[i]);
        }

        for (int i(0); i < MAX_INVENTORY; ++i)
        {
            m_ahActiveSlaveEffect[i] = INVALID_RESOURCE;
            m_ayActiveSlaveEffectSequence[i] = 0;
            m_auiActiveSlaveIndex[i] = INVALID_INDEX;
            SAFE_DELETE(m_apEffectThread[NUM_EFFECT_CHANNELS + i]);
        }

        for (int i(NUM_EFFECT_CHANNELS + MAX_INVENTORY); i < NUM_CLIENT_EFFECT_THREADS; ++i)
        {
            SAFE_DELETE(m_apEffectThread[i]);
        }

        for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
        {
            if (m_ahSoundHandle[i] != INVALID_INDEX)
            {
                K2SoundManager.StopHandle(m_ahSoundHandle[i]);
                m_ahSoundHandle[i] = INVALID_INDEX;
            }
        }

        if (m_pNextState && m_pNextState->IsUnit()) // TODO: This is ugly
        {
            for (int i(0); i < MAX_INVENTORY; ++i)
            {
                if (m_pNextState) m_pNextState->GetAsUnit()->SetInventorySlot(i, NULL);
                if (m_pPrevState) m_pPrevState->GetAsUnit()->SetInventorySlot(i, NULL);
                if (m_pCurrentState) m_pCurrentState->GetAsUnit()->SetInventorySlot(i, NULL);
            }

            if (m_pNextState) m_pNextState->GetAsUnit()->SetMount(NULL);
            if (m_pPrevState) m_pPrevState->GetAsUnit()->SetMount(NULL);
            if (m_pCurrentState) m_pCurrentState->GetAsUnit()->SetMount(NULL);
        }

        m_hModel = INVALID_RESOURCE;

        m_v3PositionLinked = CVec3f(FAR_AWAY, FAR_AWAY, FAR_AWAY);
        m_yStatusLinked = EEntityStatus(-1);
        m_fScaleLinked = 0.0f;
        m_hModelLinked = INVALID_RESOURCE;

        if (m_pNextState)
            m_pNextState->Unlink();

        SAFE_DELETE(m_pSkeleton);
        
        if (m_iTeamTargetChannel != -1)
        {
            StopEffect(m_iTeamTargetChannel);
            m_iTeamTargetChannel = -1;
        }

        if (m_iTalkingEffectChannel != -1)
        {
            StopEffect(m_iTalkingEffectChannel);
            m_iTalkingEffectChannel = -1;
        }

        if (m_iDisconnectedEffectChannel != -1)
        {
            StopEffect(m_iDisconnectedEffectChannel);
            m_iDisconnectedEffectChannel = -1;
        }

        if (m_iIllusionEffectChannel != -1)
        {
            StopEffect(m_iIllusionEffectChannel);
            m_iIllusionEffectChannel = -1;
        }

        IGameEntity *pNewEnt(pEntity);

        SAFE_DELETE(m_pNextState);
        m_pNextState = pNewEnt->GetAsVisual();
        if (m_pNextState == NULL)
            EX_ERROR(_T("Allocation failed for NextState"));

        SAFE_DELETE(m_pPrevState);
        pNewEnt = EntityRegistry.Allocate(pEntity->GetType());
        if (pNewEnt == NULL)
            EX_ERROR(_T("Allocation failed for PrevState"));
        m_pPrevState = pNewEnt->GetAsVisual();
        if (m_pPrevState == NULL)
            EX_ERROR(_T("Allocation failed for PrevState"));

        SAFE_DELETE(m_pCurrentState);
        pNewEnt = EntityRegistry.Allocate(pEntity->GetType());
        if (pNewEnt == NULL)
            EX_ERROR(_T("Allocation failed for CurrentState"));
        m_pCurrentState = pNewEnt->GetAsVisual();
        if (m_pCurrentState == NULL)
            EX_ERROR(_T("Allocation failed for CurrentState"));

        SetIndex(pEntity->GetIndex());
        SetType(pEntity->GetType());
        SetSkeleton(pEntity->AllocateSkeleton());

        if (pEntity->IsStatic())
            m_uiFlags |= CE_STATIC;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CClientEntity::Initialize() - "));
    }
}


/*====================
  CClientEntity::CopyNextToPrev
  ====================*/
void    CClientEntity::CopyNextToPrev()
{
    m_pPrevState->Copy(*m_pNextState);
}


/*====================
  CClientEntity::CopyNextToCurrent
  ====================*/
void    CClientEntity::CopyNextToCurrent()
{
    m_pCurrentState->Copy(*m_pNextState);
}


/*====================
  CClientEntity::Interpolate
  ====================*/
void    CClientEntity::Interpolate(float fLerp)
{
    if (!IsValid())
        return;

    m_pCurrentState->Interpolate(fLerp, m_pPrevState, m_pNextState);

    // Check for things that might require a re-link
    bool bRelink(false);
    CVec3f v3CurrentPosition(m_pCurrentState->GetPosition());
    byte yCurrentStatus(m_pCurrentState->GetStatus());
    float fCurrentScale(m_pCurrentState->GetBaseScale() * m_pCurrentState->GetScale());
    m_hModel = m_pCurrentState->GetModel();

    if (m_v3PositionLinked != v3CurrentPosition)
        bRelink = true;
    else if (m_yStatusLinked != yCurrentStatus)
        bRelink = true;
    else if (m_fScaleLinked != fCurrentScale)
        bRelink = true;
    else if (m_hModelLinked != m_hModel)
        bRelink = true;

    if (bRelink)
        m_pCurrentState->Unlink();

    if (bRelink && yCurrentStatus != ENTITY_STATUS_DORMANT)
    {
        m_pCurrentState->Link();
        m_v3PositionLinked = v3CurrentPosition;
        m_yStatusLinked = yCurrentStatus;
        m_fScaleLinked = fCurrentScale;
        m_hModelLinked = m_hModel;

        m_bbEntityBounds = m_pCurrentState->GetBounds();
        m_bbEntityBounds.Offset(v3CurrentPosition);
    }

    m_pSkeleton = m_pCurrentState->GetSkeleton();
    m_pCurrentState->Validate();
    m_uiFlags |= CE_VALID;

    if (yCurrentStatus == ENTITY_STATUS_DORMANT)
        PassEffects();
}


/*====================
  CClientEntity::Frame
  ====================*/
void    CClientEntity::Frame()
{
}


/*====================
  CClientEntity::AddToScene
  ====================*/
void    CClientEntity::AddToScene()
{
    PROFILE("CClientEntity::AddToScene");

    if (!IsStatic())
    {
        if (~m_uiFlags & CE_VALID)
            return;

        m_pSkeleton = m_pCurrentState->GetSkeleton();
        ResHandle hModel(m_hModel);

        // Only set skeleton model on models that have animations
        CModel* pModel(g_ResourceManager.GetModel(hModel));
        if (pModel != NULL && pModel->GetModelFile()->GetType() == MODEL_K2)
        {
            CK2Model* pK2Model(static_cast<CK2Model*>(pModel->GetModelFile()));
            if (pK2Model->GetNumAnims() == 0)
                hModel = INVALID_RESOURCE;
        }

        // Update skeleton model
        if (m_pSkeleton != NULL && m_pSkeleton->GetModel() != hModel)
        {
            PROFILE("Update Skeleton Model");

            if (m_pSkeleton->GetModel() == INVALID_RESOURCE ||
                hModel == INVALID_RESOURCE)
            {
                m_pSkeleton->SetModel(m_pCurrentState->GetModel());
            }
            else
            {
                uint auiAnimStartTime[NUM_ANIM_CHANNELS];
                float afAnimSpeed[NUM_ANIM_CHANNELS];
                tstring asAnim[NUM_ANIM_CHANNELS];
                for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
                {
                    auiAnimStartTime[i] = m_pSkeleton->GetCurrentAnimStartTime(i);
                    asAnim[i] = m_pSkeleton->GetCurrentAnimName(i);
                    afAnimSpeed[i] = m_pSkeleton->GetCurrentAnimSpeed(i);
                }

                m_pSkeleton->SetModel(hModel);

                for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
                {
                    m_aiActiveAnim[i] = m_pSkeleton->GetAnimIndex(asAnim[i]);

                    if (m_aiActiveAnim[i] != -1)
                        m_pSkeleton->RequestStartAnim(asAnim[i], auiAnimStartTime[i], i, 0, afAnimSpeed[i]);
                    else
                        m_pSkeleton->RequestStartAnim(_CWS("idle"), Game.GetGameTime(), i, 0, 1.0f);
                }
            }
        }

        // Start any new animations on the skeleton
        if (m_pSkeleton != NULL && m_pSkeleton->GetModel() != INVALID_RESOURCE)
        {
            PROFILE("New Animations");

            bool bFrozen(false);
            if (m_pCurrentState->IsUnit())
            {
#if 0
                const tstring &sIdleAnim(m_pCurrentState->GetAsUnit()->GetIdleAnim());

                if (m_pSkeleton->GetDefaultAnim() != sIdleAnim)
                {
                    m_pSkeleton->SetDefaultAnim(sIdleAnim);
                }
#endif

                if (m_pCurrentState->GetAsUnit()->IsFrozen())
                    bFrozen = true;
            }

            if (bFrozen)
            {
                // Set animation speeds
                for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
                    m_pSkeleton->SetAnimSpeed(0.0f, i);
            }
            else
            {
                int iCurrentAnim[NUM_ANIM_CHANNELS];
                byte yCurrentSequence[NUM_ANIM_CHANNELS];
                float fCurrentSpeed[NUM_ANIM_CHANNELS];
                for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
                    m_pCurrentState->GetAnimState(i, iCurrentAnim[i], yCurrentSequence[i], fCurrentSpeed[i]);

                // Process StopAnim's
                for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
                {
                    if (iCurrentAnim[i] == ENTITY_STOP_ANIM &&
                        (iCurrentAnim[i] != m_aiActiveAnim[i] || 
                        yCurrentSequence[i] != m_ayActiveAnimSequence[i]))
                    {
                        m_aiActiveAnim[i] = iCurrentAnim[i];
                        m_ayActiveAnimSequence[i] = yCurrentSequence[i];

                        m_pSkeleton->RequestStopAnim(i, Game.GetGameTime());
                    }
                }

                // Start new animations
                for (int i(NUM_ANIM_CHANNELS - 1); i >= 0; --i)
                {
                    if (iCurrentAnim[i] != m_aiActiveAnim[i] || 
                        yCurrentSequence[i] != m_ayActiveAnimSequence[i])
                    {
                        m_aiActiveAnim[i] = iCurrentAnim[i];
                        m_ayActiveAnimSequence[i] = yCurrentSequence[i];

                        if (iCurrentAnim[i] == ENTITY_INVALID_ANIM)
                            continue;

                        m_pSkeleton->RequestStartAnim(
                            g_ResourceManager.GetAnimName(hModel, iCurrentAnim[i]),
                            Game.GetGameTime(), i, -1,
                            fCurrentSpeed[i], 0);
                    }
                }

                // Set animation speeds
                for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
                {
                    if (m_pSkeleton->GetAnimIndex(i) == iCurrentAnim[i])
                        m_pSkeleton->SetAnimSpeed(fCurrentSpeed[i], i);
                }
            }
        }
    }

    // Update existing sounds
    if (!IsStatic() && m_uiFlags & CE_SOUND_ACTIVE)
    {
        PROFILE("Sounds");

        m_uiFlags &= ~CE_SOUND_ACTIVE;

        CPlayer *pLocalPlayer(Game.GetLocalPlayer());
        bool bMute(pLocalPlayer ? !pLocalPlayer->CanSee(m_pCurrentState) : false);

        for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
        {
            if (m_ahSoundHandle[i] == INVALID_INDEX)
                continue;

            if (!K2SoundManager.UpdateHandle(m_ahSoundHandle[i], m_pCurrentState->GetPosition(), m_pCurrentState->GetVelocity()))
            {
                m_ahSoundHandle[i] = INVALID_INDEX;
            }
            else
            {
                K2SoundManager.SetMute(m_ahSoundHandle[i], bMute);
                m_uiFlags |= CE_SOUND_ACTIVE;
            }
        }
    }

    if (m_unType != Entity_Effect)
    {
        if (m_pCurrentState->GetPosition().xy() == V2_ZERO || DistanceSq(GameClient.GetCameraCenter().xy(), m_pCurrentState->GetPosition().xy()) > SQR<float>(cg_entityCullRange))
            return;
    }

    // Start any new effects on the entity
    if (!IsStatic())
    {
        PROFILE("New Effects");

        for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
        {
            if (m_pCurrentState->GetEffect(i) != m_ahActiveEffect[i] || 
                m_pCurrentState->GetEffectSequence(i) != m_ayActiveEffectSequence[i] ||
                (m_apEffectThread[i] == NULL && m_ahActiveEffect[i] != INVALID_RESOURCE))
            {
                ExpireEffect(m_apEffectThread[i]);

                m_ahActiveEffect[i] = m_pCurrentState->GetEffect(i);
                m_ayActiveEffectSequence[i] = m_pCurrentState->GetEffectSequence(i);

                CEffect *pEffect(g_ResourceManager.GetEffect(m_ahActiveEffect[i]));

                if (pEffect)
                {
                    if (m_pCurrentState->IsProjectile() && m_pPrevState->GetAsProjectile()->GetOriginTime() == Game.GetPrevServerTime() && Game.GetLocalPlayer() != NULL && Game.GetLocalPlayer()->CanSee(m_pPrevState))
                    {
                        // Hack to fix projectile trail start
                        IProjectile *pProjectile(m_pPrevState->GetAsProjectile());

                        uint uiTime(MIN(pProjectile->GetOriginTime(), Game.GetGameTime()));

                        m_apEffectThread[i] = pEffect->SpawnThread(uiTime);

                        if (m_apEffectThread[i] != NULL)
                        {
                            m_apEffectThread[i]->SetCamera(GameClient.GetCamera());
                            m_apEffectThread[i]->SetWorld(GameClient.GetWorldPointer());

                            m_apEffectThread[i]->SetSourceSkeleton(m_pSkeleton);
                            m_apEffectThread[i]->SetSourceModel(g_ResourceManager.GetModel(m_pCurrentState->GetModel()));
                            m_apEffectThread[i]->SetTargetSkeleton(NULL);
                            m_apEffectThread[i]->SetTargetModel(NULL);

                            m_apEffectThread[i]->SetActive(true);

                            pProjectile->UpdateEffectThreadSource(m_apEffectThread[i]);

                            // Execute effect
                            if (m_apEffectThread[i]->Execute(uiTime))
                            {
                                // Effect finished, so delete it
                                SAFE_DELETE(m_apEffectThread[i]);
                                continue;
                            }
                            
                            m_uiFlags |= CE_EFFECT_THREAD_ACTIVE;
                        }
                    }
                    else
                    {
                        m_apEffectThread[i] = pEffect->SpawnThread(GameClient.GetGameTime());

                        if (m_apEffectThread[i] != NULL)
                        {
                            m_apEffectThread[i]->SetCamera(GameClient.GetCamera());
                            m_apEffectThread[i]->SetWorld(GameClient.GetWorldPointer());

                            m_apEffectThread[i]->SetSourceSkeleton(m_pSkeleton);
                            m_apEffectThread[i]->SetSourceModel(g_ResourceManager.GetModel(m_pCurrentState->GetModel()));
                            m_apEffectThread[i]->SetTargetSkeleton(NULL);
                            m_apEffectThread[i]->SetTargetModel(NULL);

                            m_apEffectThread[i]->SetActive(true);
                        }

                        m_uiFlags |= CE_EFFECT_THREAD_ACTIVE;
                    }
                }
                else
                {
                    m_apEffectThread[i] = NULL;
                }
            }
        }

        // Manage slave passive effects
        if (m_pCurrentState->IsUnit())
        {
            IUnitEntity *pUnit(m_pCurrentState->GetAsUnit());

            // Remap existing slave passive effect slots
            for (int i(0); i < MAX_INVENTORY; ++i)
            {
                ISlaveEntity *pSlave(pUnit->GetInventorySlot(i));

                if (pSlave == NULL)
                    continue;

                uint uiIndex(pSlave->GetIndex());

                if (uiIndex != m_auiActiveSlaveIndex[i])
                {
                    for (int i2(0); i2 < MAX_INVENTORY; ++i2)
                    {
                        if (uiIndex == m_auiActiveSlaveIndex[i2])
                        {
                            SWAP(m_ahActiveSlaveEffect[i], m_ahActiveSlaveEffect[i2]);
                            SWAP(m_ayActiveSlaveEffectSequence[i], m_ayActiveSlaveEffectSequence[i2]);
                            SWAP(m_auiActiveSlaveIndex[i], m_auiActiveSlaveIndex[i2]);
                            SWAP(m_apEffectThread[NUM_EFFECT_CHANNELS + i], m_apEffectThread[NUM_EFFECT_CHANNELS + i2]);
                            break;
                        }
                    }
                }
            }

            // Start new slave passive effects and kill dead ones
            for (int i(0); i < MAX_INVENTORY; ++i)
            {
                ISlaveEntity *pSlave(pUnit->GetInventorySlot(i));

                ResHandle hEffect(pSlave ? pSlave->GetEffect() : INVALID_RESOURCE);
                byte ySequence(pSlave ? pSlave->GetEffectSequence() : 0);
                uint uiIndex(pSlave ? pSlave->GetIndex() : INVALID_INDEX);

                int iEffectSlot(NUM_EFFECT_CHANNELS + i);

                if (hEffect != m_ahActiveSlaveEffect[i] || 
                    ySequence != m_ayActiveSlaveEffectSequence[i] ||
                    uiIndex != m_auiActiveSlaveIndex[i] ||
                    (m_apEffectThread[iEffectSlot] == NULL && m_ahActiveSlaveEffect[i] != INVALID_RESOURCE))
                {
                    m_ahActiveSlaveEffect[i] = hEffect;
                    m_ayActiveSlaveEffectSequence[i] = ySequence;
                    m_auiActiveSlaveIndex[i] = uiIndex;

                    ExpireEffect(m_apEffectThread[iEffectSlot]);

                    CEffect *pEffect(g_ResourceManager.GetEffect(m_ahActiveSlaveEffect[i]));

                    if (pEffect)
                    {
                        m_apEffectThread[iEffectSlot] = pEffect->SpawnThread(GameClient.GetGameTime());

                        if (m_apEffectThread[iEffectSlot] != NULL)
                        {
                            m_apEffectThread[iEffectSlot]->SetCamera(GameClient.GetCamera());
                            m_apEffectThread[iEffectSlot]->SetWorld(GameClient.GetWorldPointer());

                            m_apEffectThread[iEffectSlot]->SetSourceSkeleton(m_pSkeleton);
                            m_apEffectThread[iEffectSlot]->SetSourceModel(g_ResourceManager.GetModel(m_pCurrentState->GetModel()));

                            // Setup state effect bridge
                            if (pSlave->IsState() && pSlave->GetAsState()->GetInflictor())
                            {
                                m_apEffectThread[iEffectSlot]->SetTargetSkeleton(pSlave->GetAsState()->GetInflictor()->GetSkeleton());
                                m_apEffectThread[iEffectSlot]->SetTargetModel(g_ResourceManager.GetModel(pSlave->GetAsState()->GetInflictor()->GetModel()));
                            }
                            else
                            {
                                m_apEffectThread[iEffectSlot]->SetTargetSkeleton(NULL);
                                m_apEffectThread[iEffectSlot]->SetTargetModel(NULL);
                            }

                            m_apEffectThread[iEffectSlot]->SetActive(true);

                            m_uiFlags |= CE_EFFECT_THREAD_ACTIVE;
                        }
                    }
                    else
                    {
                        m_apEffectThread[iEffectSlot] = NULL;
                    }
                }
            }
        }

            // Check for Team Pings
        ResHandle hPingProtect(GameClient.GetResource(CLIENT_RESOURCE_PING_PROTECT));
        ResHandle hPingAttack(GameClient.GetResource(CLIENT_RESOURCE_PING_ATTACK));
        ResHandle hPingAttackHero(GameClient.GetResource(CLIENT_RESOURCE_PING_ATTACK_HERO));
        CPlayer* pLocalPlayer(Game.GetLocalPlayer());
        if (pLocalPlayer && (m_pCurrentState->IsBuilding() || (m_pCurrentState->IsHero() && pLocalPlayer->IsEnemy(m_pCurrentState->GetAsUnit()))))
        {
            CTeamInfo* pTeam(pLocalPlayer->GetTeamInfo());
            
            ResHandle hPing(pLocalPlayer->IsEnemy(m_pCurrentState->GetAsUnit()) ? hPingAttack : hPingProtect);

            if (pLocalPlayer->IsEnemy(m_pCurrentState->GetAsUnit()) && m_pCurrentState->IsHero())
                hPing = hPingAttackHero;

            if (hPing != INVALID_RESOURCE && pTeam)
            {
                if (pTeam->IsTeamTarget(m_pCurrentState->GetIndex()) && m_iTeamTargetChannel == -1)
                {
                    m_iTeamTargetChannel = StartEffect(hPing, -1, 0);
                }
                else if (!pTeam->IsTeamTarget(m_pCurrentState->GetIndex()) && m_iTeamTargetChannel != -1)
                {
                    StopEffect(m_iTeamTargetChannel);
                    m_iTeamTargetChannel = -1;
                }
            }
        }

        if (m_pCurrentState->IsHero())
        {
            IHeroEntity *pHero(m_pCurrentState->GetAsHero());

            // Add talking effect
            ResHandle hTalkingEffect(GameClient.GetResource(CLIENT_RESOURCE_PLAYER_TALKING_EFFECT));

            if (hTalkingEffect != INVALID_RESOURCE && VoiceManager.IsTalking(pHero->GetOwnerClientNumber()))
            {
                if (m_iTalkingEffectChannel == -1)
                    m_iTalkingEffectChannel = StartEffect(hTalkingEffect, -1, 0);
            }
            else if (m_iTalkingEffectChannel != -1)
            {
                StopEffect(m_iTalkingEffectChannel);
                m_iTalkingEffectChannel = -1;
            }

            // Add disconnected effect
            ResHandle hDisconnectedEffect(GameClient.GetResource(CLIENT_RESOURCE_PLAYER_DISCONNECTED_EFFECT));

            CPlayer *pPlayer(GameClient.GetPlayer(pHero->GetOwnerClientNumber()));
            if (hDisconnectedEffect != INVALID_RESOURCE && pPlayer != NULL && pPlayer->IsDisconnected())
            {
                if (m_iDisconnectedEffectChannel == -1)
                    m_iDisconnectedEffectChannel = StartEffect(hDisconnectedEffect, -1, 0);
            }
            else if (m_iDisconnectedEffectChannel != -1)
            {
                StopEffect(m_iDisconnectedEffectChannel);
                m_iDisconnectedEffectChannel = -1;
            }
        }

        if (m_pCurrentState->IsUnit())
        {
            IUnitEntity *pUnit(m_pCurrentState->GetAsUnit());

            ResHandle hIllusionEffect(GameClient.GetResource(CLIENT_RESOURCE_ILLUSION_EFFECT));

            if (pUnit->IsIllusion())
            {
                if (m_iIllusionEffectChannel == -1)
                    m_iIllusionEffectChannel = StartEffect(hIllusionEffect, -1, 0);
            }
            else if (m_iIllusionEffectChannel != -1)
            {
                StopEffect(m_iIllusionEffectChannel);
                m_iIllusionEffectChannel = -1;
            }
        }
    }

    uint uiLocalPlayerIndex(GameClient.GetLocalPlayerIndex());

    if (cg_debugServerSnapshot)
    {
        m_pNextState->AddToScene(GREEN, SCENEENT_SOLID_COLOR);
        m_pPrevState->AddToScene(RED, SCENEENT_SOLID_COLOR);
    }

    uint uiOldSize(uint(SceneManager.GetEntityList().size()));

    CVec4f v4Color(WHITE);
    int iFlags(0);

    // Generate scene entities
    {
        PROFILE("AddToScene");
        m_pCurrentState->SetShowEffects(m_pCurrentState->AddToScene(v4Color, iFlags));
    }

    if (m_uiFlags & CE_EFFECT_THREAD_ACTIVE)
    {       
        PROFILE("Effects");

        m_uiFlags &= ~CE_EFFECT_THREAD_ACTIVE;
        int iStartThread(IsStatic() ? NUM_EFFECT_CHANNELS + MAX_INVENTORY : 0);

        // Process effect threads for this entity
        for (int i(iStartThread); i < NUM_CLIENT_EFFECT_THREADS; ++i)
        {
            if (!m_apEffectThread[i])
                continue;

            // Setup effect parameters
            m_pCurrentState->UpdateEffectThreadSource(m_apEffectThread[i]);

            // Update state effect bridge
            if (m_pCurrentState->IsUnit())
            {
                IUnitEntity *pUnit(m_pCurrentState->GetAsUnit());

                if (i >= NUM_EFFECT_CHANNELS && i < NUM_EFFECT_CHANNELS + MAX_INVENTORY)
                {
                    ISlaveEntity *pSlave(pUnit->GetInventorySlot(i - NUM_EFFECT_CHANNELS));

                    if (pSlave->IsState() && pSlave->GetAsState()->GetInflictor())
                    {
                        pSlave->GetAsState()->GetInflictor()->UpdateEffectThreadTarget(m_apEffectThread[i]);
                    }
                    else
                    {
                        m_apEffectThread[i]->SetTargetSkeleton(NULL);
                        m_apEffectThread[i]->SetTargetModel(NULL);
                    }
                }
            }

            // Pass off first person entity effects
            bool bUpdate(true);

            if (m_unType == Entity_Effect)
            {
                bool bFirstPerson(GameClient.GetCamera()->HasFlags(CAM_FIRST_PERSON));

                if (i == 0)
                {
                    if (static_cast<CEntityEffect *>(m_pCurrentState)->GetSourceEntityIndex() == uiLocalPlayerIndex && bFirstPerson)
                        bUpdate = false;
                }
                else if (i == 1)
                {
                    if (static_cast<CEntityEffect *>(m_pCurrentState)->GetSourceEntityIndex() == uiLocalPlayerIndex && bFirstPerson)
                        continue;
                    else
                        bUpdate = false;
                }
            }

            // Execute effect
            if (m_apEffectThread[i]->Execute(GameClient.GetGameTime()))
            {
                // Effect finished, so delete it
                SAFE_DELETE(m_apEffectThread[i]);
            }
            else
            {
                if (!bUpdate)
                {
                    // Cleanup
                    m_apEffectThread[i]->Cleanup();
                    m_uiFlags |= CE_EFFECT_THREAD_ACTIVE;
                    continue;
                }

                // Camera movement
                GameClient.AddCameraEffectOffset(m_apEffectThread[i]->GetCameraOffset());
                GameClient.AddCameraEffectAngleOffset(m_apEffectThread[i]->GetCameraAngleOffset());

                // Overlays
                if (m_apEffectThread[i]->HasActiveOverlay() && m_uiIndex == uiLocalPlayerIndex)
                    GameClient.AddOverlay(m_apEffectThread[i]->GetOverlayColor(), m_apEffectThread[i]->GetOverlayMaterial());

                // Update and render all particles systems associated with this effect thread
                const InstanceMap &mapInstances(m_apEffectThread[i]->GetInstances());
                m_apEffectThread[i]->SetCustomVisibility(m_pCurrentState->GetShowEffects());
                for (InstanceMap::const_iterator it(mapInstances.begin()); it != mapInstances.end(); ++it)
                {
                    IEffectInstance *pParticleSystem(it->second);

                    pParticleSystem->Update(GameClient.GetGameTime(), CGameClient::ParticleTrace);

                    if (!pParticleSystem->IsDead() && m_pCurrentState->GetShowEffects())
                    {
                        if (pParticleSystem->IsParticleSystem())
                            SceneManager.AddParticleSystem(static_cast<CParticleSystem *>(pParticleSystem), true);
                        else if (pParticleSystem->IsModifier())
                        {
                            SceneEntityList::iterator itEnd(SceneManager.GetEntityList().end());
                            for (SceneEntityList::iterator itEnts(SceneManager.GetEntityList().begin() + uiOldSize); itEnts != itEnd; ++itEnts)
                            {
                                if ((*itEnts)->cEntity.objtype != OBJTYPE_MODEL)
                                    continue;

                                static_cast<CSceneEntityModifier *>(pParticleSystem)->Modify((*itEnts)->cEntity);
                            }
                        }
                    }
                }

                // Cleanup
                m_apEffectThread[i]->Cleanup();
            }

            m_uiFlags |= CE_EFFECT_THREAD_ACTIVE;
        }

    } // PROFILE Effects

    // Add additional scene entity modifiers
    if (!IsStatic())
    {
        PROFILE("Modifiers");

        SSceneModifierEntry *pModifiers(SceneManager.GetModifiers(m_uiIndex));
        while (pModifiers)
        {
            SceneEntityList::iterator itEnd(SceneManager.GetEntityList().end());
            for (SceneEntityList::iterator itEnts(SceneManager.GetEntityList().begin() + uiOldSize); itEnts != itEnd; ++itEnts)
            {
                if ((*itEnts)->cEntity.objtype != OBJTYPE_MODEL)
                    continue;

                pModifiers->pModifier->Modify((*itEnts)->cEntity);
            }

            pModifiers = pModifiers->pNext;
        }
    }
}


/*====================
  CClientEntity::StartEffect
  ====================*/
int     CClientEntity::StartEffect(ResHandle hEffect, int iChannel, int iTimeNudge)
{
    PROFILE("CClientEntity::StartEffect");

    // Search from an unused effect slot
    if (iChannel == -1)
    {
        for (int i(NUM_CLIENT_EFFECT_THREADS - 1); i >= NUM_EFFECT_CHANNELS + MAX_INVENTORY; --i)
        {
            if (m_apEffectThread[i] == NULL)
            {
                iChannel = i;
                break;
            }
        }

        if (iChannel == -1)
            return -1;
    }
    else
    {
        if (iChannel >= int(NUM_CLIENT_EFFECTS))
            return -1;

        iChannel += NUM_EFFECT_CHANNELS + MAX_INVENTORY; // Offset to NUM_CLIENT_EFFECT_CHANNELS
    }

    ExpireEffect(m_apEffectThread[iChannel]);

    CEffect *pEffect(g_ResourceManager.GetEffect(hEffect));

    if (pEffect)
    {
        m_apEffectThread[iChannel] = pEffect->SpawnThread(GameClient.GetGameTime() + iTimeNudge);

        if (m_apEffectThread[iChannel] == NULL)
            return -1;
        
        m_apEffectThread[iChannel]->SetCamera(GameClient.GetCamera());
        m_apEffectThread[iChannel]->SetWorld(GameClient.GetWorldPointer());

        m_apEffectThread[iChannel]->SetSourceSkeleton(m_pSkeleton);
        m_apEffectThread[iChannel]->SetSourceModel(g_ResourceManager.GetModel(m_pNextState->GetModel()));
        m_apEffectThread[iChannel]->SetTargetSkeleton(NULL);
        m_apEffectThread[iChannel]->SetTargetModel(NULL);

        m_apEffectThread[iChannel]->SetActive(true);
        
        // TODO: we should use a timenudged lerped position instead of the previous frame's CurrentState
        m_pCurrentState->UpdateEffectThreadSource(m_apEffectThread[iChannel]);
        
        if (m_apEffectThread[iChannel]->Execute(GameClient.GetGameTime() + iTimeNudge))
            SAFE_DELETE(m_apEffectThread[iChannel])
        else
            m_uiFlags |= CE_EFFECT_THREAD_ACTIVE;
    }

    return iChannel;
}


/*====================
  CClientEntity::StartEffect
  ====================*/
int     CClientEntity::StartEffect(const tstring &sEffect, int iChannel, int iTimeNudge)
{   
    ResHandle hEffect(g_ResourceManager.LookUpPrecached(sEffect));
    if (hEffect == INVALID_RESOURCE)
    {
        K2_WITH_GAME_RESOURCE_SCOPE()
            hEffect = g_ResourceManager.Register(sEffect, RES_EFFECT);
    }
    return StartEffect(hEffect, iChannel, iTimeNudge);
}


/*====================
  CClientEntity::StopEffect
  ====================*/
void    CClientEntity::StopEffect(int iChannel)
{
    ExpireEffect(m_apEffectThread[iChannel]);
}


/*====================
  CClientEntity::ExpireEffect
  ====================*/
void    CClientEntity::ExpireEffect(CEffectThread *&pEffectThread)
{
    if (pEffectThread == NULL)
        return;

    if (!pEffectThread->IsDeferred() && !pEffectThread->IsPersistent())
    {
        SAFE_DELETE(pEffectThread);
        return;
    }

    // Search from an unused effect slot
    int iChannel(-1);

    for (int i(NUM_CLIENT_EFFECT_THREADS - 1); i >= NUM_EFFECT_CHANNELS + MAX_INVENTORY; --i)
    {
        if (m_apEffectThread[i] == NULL)
        {
            iChannel = i;
            break;
        }
    }

    if (iChannel == -1)
    {
        SAFE_DELETE(pEffectThread);
        return;
    }

    m_apEffectThread[iChannel] = pEffectThread;

    if (!m_apEffectThread[iChannel]->IsPersistent())
        m_apEffectThread[iChannel]->Expire(GameClient.GetGameTime());

    m_apEffectThread[iChannel]->SetTargetSkeleton(NULL);
    m_apEffectThread[iChannel]->SetTargetModel(NULL);
    m_apEffectThread[iChannel]->SetActive(pEffectThread->IsPersistent());

    pEffectThread = NULL;
}


/*====================
  CClientEntity::PassEffects

  Give control of persistant effects to game events (for lingering rocket trails, etc)
  ====================*/
void    CClientEntity::PassEffects()
{
    for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
    {
        if (!m_apEffectThread[i])
            continue;

        if (!m_apEffectThread[i]->IsDeferred() && !m_apEffectThread[i]->IsPersistent())
        {
            SAFE_DELETE(m_apEffectThread[i]);
            continue;
        }

        m_apEffectThread[i]->SetCamera(NULL);

        m_apEffectThread[i]->SetSourceSkeleton(NULL);
        m_apEffectThread[i]->SetSourceModel(NULL);
        m_apEffectThread[i]->SetTargetSkeleton(NULL);
        m_apEffectThread[i]->SetTargetModel(NULL);

        if (!m_apEffectThread[i]->IsPersistent())
            m_apEffectThread[i]->Expire(GameClient.GetGameTime());
        
        CGameEvent ev;
        ev.SetSourcePosition(m_pCurrentState->GetPosition());
        ev.SetSourceAngles(m_pCurrentState->GetAngles());
        ev.SetSourceScale(m_pCurrentState->GetBaseScale() * m_pCurrentState->GetScale());
        ev.SetSourceEffectScale(m_pCurrentState->GetEffectScale());
        ev.SetEffect(m_apEffectThread[i]);
        ev.SetEffectActive(m_apEffectThread[i]->IsPersistent());
        ev.SetCull(Game.GetLocalPlayer() != NULL && !Game.GetLocalPlayer()->CanSee(m_pCurrentState));
        ev.Spawn();
        Game.AddEvent(ev);

        m_apEffectThread[i] = NULL;
    }
}


/*====================
  CClientEntity::PlaySound
  ====================*/
int     CClientEntity::PlaySound(ResHandle hSample, float fVolume, float fFalloff, int iChannel, int iPriority, int iSoundFlags, int iFadeIn, int iFadeOutStartTime, int iFadeOut, bool bOverride, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime, float fFalloffEnd)
{
    PROFILE("CClientEntity::PlaySound");

    // Search from an unused sound slot
    if (iChannel == -1)
    {
        for (int i(NUM_CLIENT_SOUND_HANDLES - 1); i >= 0; --i)
        {
            if (m_ahSoundHandle[i] == INVALID_INDEX)
            {
                iChannel = i;
                break;
            }
        }

        if (iChannel == -1)
            return - 1;
    }
    else
    {
        //iChannel += NUM_SOUND_CHANNELS; // Offset to NUM_CLIENT_EFFECT_CHANNELS
    }

    if (m_ahSoundHandle[iChannel] != INVALID_INDEX)
    {
        if (!bOverride)
            return -1;

        K2SoundManager.StopHandle(m_ahSoundHandle[iChannel]);
        m_ahSoundHandle[iChannel] = INVALID_INDEX;
    }

    m_ahSoundHandle[iChannel] = K2SoundManager.PlaySFXSound
    (
        hSample,
        &m_pCurrentState->GetPosition(),
        &m_pCurrentState->GetVelocity(),
        fVolume,
        fFalloff,
        -1,
        iPriority,
        iSoundFlags,
        iFadeIn,
        iFadeOutStartTime,
        iFadeOut,
        iSpeedUpTime,
        fSpeed1,
        fSpeed2,
        iSlowDownTime,
        fFalloffEnd
    );

    m_uiFlags |= CE_SOUND_ACTIVE;

    return iChannel;
}


/*====================
  CClientEntity::StopSound
  ====================*/
void    CClientEntity::StopSound(int iChannel)
{
    if (iChannel == -1 || iChannel >= NUM_CLIENT_SOUND_HANDLES)
        return;
    
    //iChannel += NUM_SOUND_CHANNELS; // Offset to NUM_CLIENT_EFFECT_CHANNELS

    if (m_ahSoundHandle[iChannel] != INVALID_INDEX)
        K2SoundManager.StopHandle(m_ahSoundHandle[iChannel]);
}


/*====================
  CClientEntity::StopAllSounds
  ====================*/
void    CClientEntity::StopAllSounds()
{
    for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
    {
        if (m_ahSoundHandle[i] != INVALID_INDEX)
            K2SoundManager.StopHandle(m_ahSoundHandle[i]);
    }
}


/*====================
  CClientEntity::Rewind
  ====================*/
void    CClientEntity::Rewind()
{
    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
    {
        m_aiActiveAnim[i] = -1;
        m_ayActiveAnimSequence[i] = 0;
    }

#if 0
    for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
    {
        m_ahActiveEffect[i] = INVALID_RESOURCE;
        m_ayActiveEffectSequence[i] = 0;
        SAFE_DELETE(m_apEffectThread[i]);
    }
#endif
}
