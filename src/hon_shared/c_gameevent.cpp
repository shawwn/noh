// (C)2006 S2 Games
// c_gameevent.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gameevent.h"
#include "i_visualentity.h"
#include "i_entitystate.h"
#include "c_player.h"

#include "../k2/c_scenemanager.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_effect.h"
#include "../k2/c_particlesystem.h"
#include "../k2/c_sceneentitymodifier.h"
#include "../k2/c_networkresourcemanager.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

/*====================
  CGameEvent::~CGameEvent
  ====================*/
CGameEvent::~CGameEvent()
{
}


/*====================
  CGameEvent::CGameEvent
  ====================*/
CGameEvent::CGameEvent() :
m_bValid(true),
m_uiIndex(INVALID_INDEX),
m_unFlags(0),
m_uiExpireTime(INVALID_TIME),
m_uiSourceEntityIndex(INVALID_INDEX),
m_v3SourcePosition(V3_ZERO),
m_v3SourceAngles(V3_ZERO),
m_fSourceScale(1.0f),
m_fSourceEffectScale(1.0f),
m_uiTargetEntityIndex(INVALID_INDEX),
m_v3TargetPosition(V3_ZERO),
m_v3TargetAngles(V3_ZERO),
m_fTargetScale(1.0f),
m_fTargetEffectScale(1.0f),
m_hEffect(INVALID_RESOURCE),
m_hSound(INVALID_RESOURCE),
m_pEffectThread(NULL),
m_bEffectActive(true),
m_bCull(false),
m_unVisibilityFlags(ushort(-1))
{
}

CGameEvent::CGameEvent(const CBufferBit &cBuffer) :
m_bValid(true),
m_uiIndex(INVALID_INDEX),
m_unFlags(0),
m_uiExpireTime(INVALID_TIME),
m_uiSourceEntityIndex(INVALID_INDEX),
m_v3SourcePosition(V3_ZERO),
m_v3SourceAngles(V3_ZERO),
m_fSourceScale(1.0f),
m_fSourceEffectScale(1.0f),
m_uiTargetEntityIndex(INVALID_INDEX),
m_v3TargetPosition(V3_ZERO),
m_v3TargetAngles(V3_ZERO),
m_fTargetScale(1.0f),
m_fTargetEffectScale(1.0f),
m_hEffect(INVALID_RESOURCE),
m_hSound(INVALID_RESOURCE),
m_pEffectThread(NULL),
m_bEffectActive(true),
m_bCull(false),
m_unVisibilityFlags(ushort(-1))
{
    try
    {
        m_unFlags = cBuffer.ReadBits(16);
        
        if (m_unFlags & EVENT_HAS_EXPIRE_TIME)
            m_uiExpireTime = cBuffer.ReadBits(32);
        if (m_unFlags & EVENT_HAS_SOURCE_ENTITY)
            m_uiSourceEntityIndex = cBuffer.ReadBits(16);
        if (m_unFlags & EVENT_HAS_SOURCE_POSITION)
        {
            m_v3SourcePosition.x = ushort(cBuffer.ReadBits(16));
            m_v3SourcePosition.y = ushort(cBuffer.ReadBits(16));
            m_v3SourcePosition.z = short(cBuffer.ReadBits(16));
        }
        if (m_unFlags & EVENT_HAS_SOURCE_ANGLES)
        {
            m_v3SourceAngles.x = M_GetAngle(cBuffer.ReadBits(8));
            m_v3SourceAngles.y = M_GetAngle(cBuffer.ReadBits(8));
            m_v3SourceAngles.z = M_GetAngle(cBuffer.ReadBits(8));
        }
        if (m_unFlags & EVENT_HAS_SOURCE_SCALE)
            m_fSourceScale = cBuffer.ReadBits(16) / 256.0f;
        if (m_unFlags & EVENT_HAS_SOURCE_EFFECT_SCALE)
            m_fSourceEffectScale = cBuffer.ReadBits(16) / 256.0f;
        if (m_unFlags & EVENT_HAS_TARGET_ENTITY)
            m_uiTargetEntityIndex = cBuffer.ReadBits(16);
        if (m_unFlags & EVENT_HAS_TARGET_POSITION)
        {
            m_v3TargetPosition.x = ushort(cBuffer.ReadBits(16));
            m_v3TargetPosition.y = ushort(cBuffer.ReadBits(16));
            m_v3TargetPosition.z = short(cBuffer.ReadBits(16));
        }
        if (m_unFlags & EVENT_HAS_TARGET_ANGLES)
        {
            m_v3TargetAngles.x = M_GetAngle(cBuffer.ReadBits(8));
            m_v3TargetAngles.y = M_GetAngle(cBuffer.ReadBits(8));
            m_v3TargetAngles.z = M_GetAngle(cBuffer.ReadBits(8));
        }
        if (m_unFlags & EVENT_HAS_TARGET_SCALE)
            m_fTargetScale = cBuffer.ReadBits(16) / 256.0f;
        if (m_unFlags & EVENT_HAS_TARGET_EFFECT_SCALE)
            m_fTargetEffectScale = cBuffer.ReadBits(16) / 256.0f;
        if (m_unFlags & EVENT_HAS_EFFECT)
            m_hEffect = NetworkResourceManager.GetLocalHandle(cBuffer.ReadBits(16));
        if (m_unFlags & EVENT_HAS_SOUND)
            m_hSound = NetworkResourceManager.GetLocalHandle(cBuffer.ReadBits(16));
        if (m_unFlags & EVENT_HAS_VISIBILITY)
            m_unVisibilityFlags = cBuffer.ReadBits(16);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameEvent::CGameEvent() - "), NO_THROW);
        m_bValid = false;
    }
}


/*====================
  CGameEvent::Clear
  ====================*/
void    CGameEvent::Clear()
{
    m_bValid = true;
    m_unFlags = 0;
    m_uiExpireTime = INVALID_TIME;
    m_uiSourceEntityIndex = INVALID_INDEX;
    m_v3SourcePosition.Clear();
    m_v3SourceAngles.Clear();
    m_fSourceScale = 1.0f;
    m_fSourceEffectScale = 1.0f;
    m_uiTargetEntityIndex = INVALID_INDEX;
    m_v3TargetPosition.Clear();
    m_v3TargetAngles.Clear();
    m_fTargetScale = 1.0f;
    m_fTargetEffectScale = 1.0f;
    m_hEffect = INVALID_RESOURCE;
    m_hSound = INVALID_RESOURCE;
    m_unVisibilityFlags = ushort(-1);

    SAFE_DELETE(m_pEffectThread);
}


/*====================
  CGameEvent::GetBuffer
  ====================*/
void    CGameEvent::GetBuffer(IBuffer &buffer)
{
    if (m_unVisibilityFlags != ushort(-1))
        m_unFlags |= EVENT_HAS_VISIBILITY;

    buffer.Clear();
    buffer.WriteShort(m_unFlags);
    if (m_unFlags & EVENT_HAS_EXPIRE_TIME)
        buffer.WriteInt(m_uiExpireTime);
    if (m_unFlags & EVENT_HAS_SOURCE_ENTITY)
        buffer.WriteShort(m_uiSourceEntityIndex);
    if (m_unFlags & EVENT_HAS_SOURCE_POSITION)
    {
        buffer.WriteShort(ushort(INT_ROUND(CLAMP(m_v3SourcePosition.x, 0.0f, float(USHRT_MAX)))));
        buffer.WriteShort(ushort(INT_ROUND(CLAMP(m_v3SourcePosition.y, 0.0f, float(USHRT_MAX)))));
        buffer.WriteShort(INT_ROUND(CLAMP(m_v3SourcePosition.z, float(SHRT_MIN), float(SHRT_MAX))));
    }
    if (m_unFlags & EVENT_HAS_SOURCE_ANGLES)
    {
        buffer.WriteByte(M_GetAngle8(m_v3SourceAngles.x));
        buffer.WriteByte(M_GetAngle8(m_v3SourceAngles.y));
        buffer.WriteByte(M_GetAngle8(m_v3SourceAngles.z));
    }
    if (m_unFlags & EVENT_HAS_SOURCE_SCALE)
        buffer.WriteShort(short(m_fSourceScale * 256.0f));
    if (m_unFlags & EVENT_HAS_SOURCE_EFFECT_SCALE)
        buffer.WriteShort(short(m_fSourceEffectScale * 256.0f));
    if (m_unFlags & EVENT_HAS_TARGET_ENTITY)
        buffer.WriteShort(m_uiTargetEntityIndex);
    if (m_unFlags & EVENT_HAS_TARGET_POSITION)
    {
        buffer.WriteShort(ushort(INT_ROUND(CLAMP(m_v3TargetPosition.x, 0.0f, float(USHRT_MAX)))));
        buffer.WriteShort(ushort(INT_ROUND(CLAMP(m_v3TargetPosition.y, 0.0f, float(USHRT_MAX)))));
        buffer.WriteShort(INT_ROUND(CLAMP(m_v3TargetPosition.z, float(SHRT_MIN), float(SHRT_MAX))));
    }
    else
        m_v3TargetPosition = m_v3SourcePosition; // For culling
    if (m_unFlags & EVENT_HAS_TARGET_ANGLES)
    {
        buffer.WriteByte(M_GetAngle8(m_v3TargetAngles.x));
        buffer.WriteByte(M_GetAngle8(m_v3TargetAngles.y));
        buffer.WriteByte(M_GetAngle8(m_v3TargetAngles.z));
    }
    if (m_unFlags & EVENT_HAS_TARGET_SCALE)
        buffer.WriteShort(short(m_fTargetScale * 256.0f));
    if (m_unFlags & EVENT_HAS_TARGET_EFFECT_SCALE)
        buffer.WriteShort(short(m_fTargetEffectScale * 256.0f));
    if (m_unFlags & EVENT_HAS_EFFECT)
        buffer.WriteShort(NetworkResourceManager.GetNetIndex(m_hEffect));
    if (m_unFlags & EVENT_HAS_SOUND)
        buffer.WriteShort(NetworkResourceManager.GetNetIndex(m_hSound));
    if (m_unFlags & EVENT_HAS_VISIBILITY)
        buffer.WriteShort(m_unVisibilityFlags);
}


/*====================
  CGameEvent::Translate2
  ====================*/
void     CGameEvent::Translate2(const CBufferBit &bufferIn, IBuffer &bufferOut)
{
    ushort unFlags(bufferIn.ReadBits(16));

    bufferOut.Clear();
    bufferOut.WriteShort(unFlags);

    uint uiSize(0);
    
    if (unFlags & EVENT_HAS_EXPIRE_TIME)
        uiSize += 4;
    if (unFlags & EVENT_HAS_SOURCE_ENTITY)
        uiSize += 2;
    if (unFlags & EVENT_HAS_SOURCE_POSITION)
        uiSize += 6;
    if (unFlags & EVENT_HAS_SOURCE_ANGLES)
        uiSize += 3;
    if (unFlags & EVENT_HAS_SOURCE_SCALE)
        uiSize += 2;
    if (unFlags & EVENT_HAS_SOURCE_EFFECT_SCALE)
        uiSize += 2;
    if (unFlags & EVENT_HAS_TARGET_ENTITY)
        uiSize += 2;
    if (unFlags & EVENT_HAS_TARGET_POSITION)
        uiSize += 6;
    if (unFlags & EVENT_HAS_TARGET_ANGLES)
        uiSize += 3;
    if (unFlags & EVENT_HAS_TARGET_SCALE)
        uiSize += 2;
    if (unFlags & EVENT_HAS_TARGET_EFFECT_SCALE)
        uiSize += 2;
    if (unFlags & EVENT_HAS_EFFECT)
        uiSize += 2;
    if (unFlags & EVENT_HAS_SOUND)
        uiSize += 2;
    if (unFlags & EVENT_HAS_VISIBILITY)
        uiSize += 2;

    for (; uiSize > 0; --uiSize)
        bufferOut.WriteByte(bufferIn.ReadBits(8));
}


/*====================
  CGameEvent::AdvanceBuffer2
  ====================*/
void     CGameEvent::AdvanceBuffer2(const CBufferBit &cBuffer)
{
    ushort unFlags(cBuffer.ReadBits(16));

    int uiSize(0);
    
    if (unFlags & EVENT_HAS_EXPIRE_TIME)
        uiSize += 4;
    if (unFlags & EVENT_HAS_SOURCE_ENTITY)
        uiSize += 2;
    if (unFlags & EVENT_HAS_SOURCE_POSITION)
        uiSize += 6;
    if (unFlags & EVENT_HAS_SOURCE_ANGLES)
        uiSize += 3;
    if (unFlags & EVENT_HAS_SOURCE_SCALE)
        uiSize += 2;
    if (unFlags & EVENT_HAS_SOURCE_EFFECT_SCALE)
        uiSize += 2;
    if (unFlags & EVENT_HAS_TARGET_ENTITY)
        uiSize += 2;
    if (unFlags & EVENT_HAS_TARGET_POSITION)
        uiSize += 6;
    if (unFlags & EVENT_HAS_TARGET_ANGLES)
        uiSize += 3;
    if (unFlags & EVENT_HAS_TARGET_SCALE)
        uiSize += 2;
    if (unFlags & EVENT_HAS_TARGET_EFFECT_SCALE)
        uiSize += 2;
    if (unFlags & EVENT_HAS_EFFECT)
        uiSize += 2;
    if (unFlags & EVENT_HAS_SOUND)
        uiSize += 2;
    if (unFlags & EVENT_HAS_VISIBILITY)
        uiSize += 2;

    for (; uiSize > 0; --uiSize)
        cBuffer.ReadBits(8);
}


/*====================
  CGameEvent::Print
  ====================*/
void    CGameEvent::Print() const
{
    Console.Dev << _T("GameEvent:");

    if (m_unFlags & EVENT_HAS_EXPIRE_TIME)
        Console.Dev << _T(" Expire: ") << m_uiExpireTime;
    if (m_unFlags & EVENT_HAS_SOURCE_ENTITY)
        Console.Dev << _T(" Ent: ") << m_uiSourceEntityIndex;
    if (m_unFlags & EVENT_HAS_SOURCE_POSITION)
        Console.Dev << _T(" Pos: ") << m_v3SourcePosition;
    if (m_unFlags & EVENT_HAS_SOURCE_ANGLES)
        Console.Dev << _T(" Angles: ") << m_v3SourceAngles;
    if (m_unFlags & EVENT_HAS_SOURCE_SCALE)
        Console.Dev << _T(" Scale: ") << m_fSourceScale;
    if (m_unFlags & EVENT_HAS_SOURCE_EFFECT_SCALE)
        Console.Dev << _T(" Effect Scale: ") << m_fSourceEffectScale;
    if (m_unFlags & EVENT_HAS_TARGET_ENTITY)
        Console.Dev << _T(" Ent: ") << m_uiTargetEntityIndex;
    if (m_unFlags & EVENT_HAS_TARGET_POSITION)
        Console.Dev << _T(" Pos: ") << m_v3TargetPosition;
    if (m_unFlags & EVENT_HAS_TARGET_ANGLES)
        Console.Dev << _T(" Angles: ") << m_v3TargetAngles;
    if (m_unFlags & EVENT_HAS_TARGET_SCALE)
        Console.Dev << _T(" Scale: ") << m_fTargetScale;
    if (m_unFlags & EVENT_HAS_TARGET_EFFECT_SCALE)
        Console.Dev << _T(" Effect Scale: ") << m_fTargetEffectScale;
    if (m_unFlags & EVENT_HAS_EFFECT)
        Console.Dev << _T(" Effect: ") << INT_HEX_STR(m_hEffect);
    if (m_unFlags & EVENT_HAS_SOUND)
        Console.Dev << _T(" Sound: ") << INT_HEX_STR(m_hSound);
    if (m_unFlags & EVENT_HAS_VISIBILITY)
        Console.Dev << _T(" Vis: ") << INT_HEX_STR(m_unVisibilityFlags);

    Console.Dev << newl;
}


/*====================
  CGameEvent::GetVisualEntityIndex

  returns state owner if the index is a state, otherwise just returns the index
  ====================*/
uint    CGameEvent::GetVisualEntityIndex(uint uiIndex)
{
    IGameEntity *pEntity(Game.GetEntity(uiIndex));
    if (pEntity == NULL)
        return INVALID_INDEX;

    ISlaveEntity *pSlave(pEntity->GetAsSlave());
    if (pSlave != NULL)
        return pSlave->GetOwnerIndex();

    return uiIndex;
}


/*====================
  CGameEvent::SynchWithEntity
  ====================*/
bool    CGameEvent::SynchWithEntity()
{
    // If this event references an entity, update from the
    // entities information
    IVisualEntity *pVisSourceEntity(NULL);
    if (m_uiSourceEntityIndex != INVALID_INDEX)
    {
        uint uiSourceIndex(GetVisualEntityIndex(m_uiSourceEntityIndex));

        pVisSourceEntity = Game.GetVisualEntity(uiSourceIndex);
        if (pVisSourceEntity == NULL)
        {
            if (!(m_unFlags & EVENT_SPAWNED_THIS_FRAME))
            {
                m_uiSourceEntityIndex = INVALID_INDEX;

                ExpireEffect();
                
                if (m_pEffectThread == NULL)
                    return false;
            }
        }
        else
        {
            if (!(m_unFlags & EVENT_HAS_SOURCE_POSITION))
                m_v3SourcePosition = pVisSourceEntity->GetPosition();
            if (!(m_unFlags & EVENT_HAS_SOURCE_ANGLES))
                m_v3SourceAngles = pVisSourceEntity->GetAngles();
            if (!(m_unFlags & EVENT_HAS_SOURCE_SCALE))
                m_fSourceScale = pVisSourceEntity->GetBaseScale() * pVisSourceEntity->GetScale();
            if (!(m_unFlags & EVENT_HAS_SOURCE_EFFECT_SCALE))
                m_fSourceEffectScale = pVisSourceEntity->GetEffectScale() / m_fSourceScale;
        }
    }

    IVisualEntity *pVisTargetEntity(NULL);
    if (m_uiTargetEntityIndex != INVALID_INDEX)
    {
        uint uiTargetIndex(GetVisualEntityIndex(m_uiTargetEntityIndex));

        pVisTargetEntity = Game.GetVisualEntity(uiTargetIndex);
        if (pVisTargetEntity == NULL)
        {
            if (!(m_unFlags & EVENT_SPAWNED_THIS_FRAME))
            {
                m_uiTargetEntityIndex = INVALID_INDEX;
                
                ExpireEffect();
                
                if (m_pEffectThread == NULL)
                    return false;
            }
        }
        else
        {
            if (!(m_unFlags & EVENT_HAS_TARGET_POSITION))
                m_v3TargetPosition = pVisTargetEntity->GetPosition();
            if (!(m_unFlags & EVENT_HAS_TARGET_ANGLES))
                m_v3TargetAngles = pVisTargetEntity->GetAngles();
            if (!(m_unFlags & EVENT_HAS_TARGET_SCALE))
                m_fTargetScale = pVisTargetEntity->GetBaseScale() * pVisTargetEntity->GetScale();
            if (!(m_unFlags & EVENT_HAS_TARGET_EFFECT_SCALE))
                m_fTargetEffectScale = pVisTargetEntity->GetEffectScale() / m_fTargetScale;
        }
    }

    if (m_pEffectThread == NULL)
        return true;

    m_pEffectThread->SetCamera(Game.GetCamera());

    if (pVisSourceEntity != NULL)
    {
        m_pEffectThread->SetSourceSkeleton(pVisSourceEntity->GetSkeleton());
        m_pEffectThread->SetSourceModel(g_ResourceManager.GetModel(pVisSourceEntity->GetModel()));
    }
    else
    {
        m_pEffectThread->SetSourceSkeleton(NULL);
        m_pEffectThread->SetSourceModel(NULL);
    }
    
    if (pVisTargetEntity != NULL)
    {
        m_pEffectThread->SetTargetSkeleton(pVisTargetEntity->GetSkeleton());
        m_pEffectThread->SetTargetModel(g_ResourceManager.GetModel(pVisTargetEntity->GetModel()));
    }
    else
    {
        m_pEffectThread->SetTargetSkeleton(NULL);
        m_pEffectThread->SetTargetModel(NULL);
    }

    return true;
}


/*====================
  CGameEvent::Spawn
  ====================*/
void    CGameEvent::Spawn()
{
    PROFILE("CGameEvent::Spawn");

    m_unFlags |= EVENT_SPAWNED_THIS_FRAME;

    if (!SynchWithEntity())
        SetExpireNextFrame();

    // Start requested effect
    if (m_pEffectThread != NULL)
        return;

    if (m_hEffect == INVALID_RESOURCE)
        return;

    CEffect *pEffect(g_ResourceManager.GetEffect(m_hEffect));
    if (pEffect == NULL)
        return;

    m_pEffectThread = pEffect->SpawnThread(Game.GetGameTime());
    if (m_pEffectThread == NULL)
        return;

    m_pEffectThread->SetCamera(Game.GetCamera());
    m_pEffectThread->SetWorld(Game.GetWorldPointer());

    IVisualEntity *pSourceEntity(Game.GetVisualEntity(m_uiSourceEntityIndex));
    if (pSourceEntity != NULL)
    {
        m_pEffectThread->SetSourceSkeleton(pSourceEntity->GetSkeleton());
        m_pEffectThread->SetSourceModel(g_ResourceManager.GetModel(pSourceEntity->GetModel()));
    }
    else
    {
        m_pEffectThread->SetSourceSkeleton(NULL);
        m_pEffectThread->SetSourceModel(NULL);
    }
    
    IVisualEntity *pTargetEntity(Game.GetVisualEntity(m_uiTargetEntityIndex));
    if (pTargetEntity != NULL)
    {
        m_pEffectThread->SetTargetSkeleton(pTargetEntity->GetSkeleton());
        m_pEffectThread->SetTargetModel(g_ResourceManager.GetModel(pTargetEntity->GetModel()));
    }
    else
    {
        m_pEffectThread->SetTargetSkeleton(NULL);
        m_pEffectThread->SetTargetModel(NULL);
    }
}


/*====================
  CGameEvent::Frame
  ====================*/
bool    CGameEvent::Frame()
{
    // Check to see if the event has expired
    if (m_uiExpireTime < Game.GetGameTime() || !m_pEffectThread || (m_unFlags & EVENT_EXPIRE_NEXT_FRAME))
    {
        //Console << _T("Effect finished: ") << INT_HEX_STR(m_hEffect) << newl;
        return false;
    }

    return SynchWithEntity();
}


/*====================
  ParticleTrace
  ====================*/
static bool ParticleTrace(const CVec3f &v3Start, const CVec3f &v3End, CVec3f &v3EndPos, CVec3f &v3Normal)
{
    STraceInfo trace;
    if (Game.TraceLine(trace, v3Start, v3End, TRACE_TERRAIN))
    {
        v3EndPos = trace.v3EndPos;
        v3Normal = trace.plPlane.v3Normal;
        return true;
    }
    else
    {
        v3EndPos = v3End;
        v3Normal = CVec3f(0.0f, 0.0f, 1.0f);
        return false;
    }
}


/*====================
  CGameEvent::AddToScene
  ====================*/
void    CGameEvent::AddToScene()
{
    // Draw effects
    if (!m_pEffectThread)
        return;

    CPlayer *pLocalPlayer(Game.GetLocalPlayer());
    if (pLocalPlayer == NULL)
        return;

    uint uiSourceIndex(GetVisualEntityIndex(m_uiSourceEntityIndex));
    uint uiTargetIndex(GetVisualEntityIndex(m_uiTargetEntityIndex));

    // Culling
    bool bCull(false);
    if (!m_bCull)
    {
        uint uiTeam(pLocalPlayer->GetTeam());

        if (uiTeam != TEAM_SPECTATOR && !HasVisibilityFlags(VIS_SIGHTED(uiTeam)))
        {
            bCull = true;
        }
        else if (uiSourceIndex != INVALID_INDEX && uiTargetIndex != INVALID_INDEX)
        {
            IVisualEntity *pSourceEntity(Game.GetVisualEntity(uiSourceIndex));
            IVisualEntity *pTargetEntity(Game.GetVisualEntity(uiTargetIndex));
            if ((pSourceEntity && (!pLocalPlayer->CanSee(pSourceEntity) || !pSourceEntity->GetShowEffects())) &&
                (pTargetEntity && (!pLocalPlayer->CanSee(pTargetEntity) || !pTargetEntity->GetShowEffects())))
                bCull = true;
        }
        else if (uiSourceIndex != INVALID_INDEX)
        {
            IVisualEntity *pEntity(Game.GetVisualEntity(uiSourceIndex));
            if (pEntity && (!pLocalPlayer->CanSee(pEntity) || !pEntity->GetShowEffects()))
                bCull = true;
        }
        else if (uiTargetIndex != INVALID_INDEX)
        {
            IVisualEntity *pEntity(Game.GetVisualEntity(uiTargetIndex));
            if (pEntity && (!pLocalPlayer->CanSee(pEntity) || !pEntity->GetShowEffects()))
                bCull = true;
        }
        else
        {
            if ((~m_unFlags & EVENT_HAS_SOURCE_POSITION || !SceneManager.GetFrustumBounds().Contains(m_v3SourcePosition)) &&
                (~m_unFlags & EVENT_HAS_TARGET_POSITION || !SceneManager.GetFrustumBounds().Contains(m_v3TargetPosition)))
                bCull = true;
        }
    }
    else
    {
        bCull = true;
    }

    m_pEffectThread->SetCustomVisibility(!bCull);

    // Set effect parameters
    m_pEffectThread->SetActive(m_bEffectActive);

    m_pEffectThread->SetSourcePos(m_v3SourcePosition);
    m_pEffectThread->SetSourceAxis(CAxis(m_v3SourceAngles));
    m_pEffectThread->SetSourceScale(m_fSourceScale);
    if (m_pEffectThread->GetUseEntityEffectScale())
        m_pEffectThread->SetSourceEffectScale(m_fSourceEffectScale);
    else
        m_pEffectThread->SetSourceEffectScale(1.0f);

    m_pEffectThread->SetTargetPos(m_v3TargetPosition);
    m_pEffectThread->SetTargetAxis(CAxis(m_v3TargetAngles));
    m_pEffectThread->SetTargetScale(m_fTargetScale);
    if (m_pEffectThread->GetUseEntityEffectScale())
        m_pEffectThread->SetTargetEffectScale(m_fTargetEffectScale);
    else
        m_pEffectThread->SetTargetEffectScale(1.0f);
    
    if (m_pEffectThread->Execute(Game.GetGameTime()))
    {
        SAFE_DELETE(m_pEffectThread);
        return;
    }

    // Camera effects
    Game.AddCameraEffectOffset(m_pEffectThread->GetCameraOffset());
    Game.AddCameraEffectAngleOffset(m_pEffectThread->GetCameraAngleOffset());

#if 0
    // Overlays
    IGameEntity *pEntity(Game.GetEntity(m_uiSourceEntityIndex));
    if (pEntity != NULL)
    {
        IPlayerEntity *pPlayer(pEntity->GetAsPlayerEnt());
        IEntityState *pState(pEntity->GetAsState());
        if (pPlayer == NULL && pState != NULL)
            pPlayer = Game.GetPlayerEntity(pState->GetOwner());

        if (pPlayer != NULL &&
            pPlayer->GetClientID() == Game.GetLocalClientNum() &&
            m_pEffectThread->HasActiveOverlay())
            Game.AddOverlay(m_pEffectThread->GetOverlayColor(), m_pEffectThread->GetOverlayMaterial());
    }
#endif

    // Update and render all particle systems associated with this effect thread
    const InstanceMap &mapInstances(m_pEffectThread->GetInstances());
    for (InstanceMap::const_iterator it(mapInstances.begin()); it != mapInstances.end(); ++it)
    {
        IEffectInstance *pParticleSystem(it->second);
        pParticleSystem->Update(Game.GetGameTime(), ParticleTrace);

        if (!pParticleSystem->IsDead() && !bCull)
        {
            if (pParticleSystem->IsParticleSystem())
                SceneManager.AddParticleSystem(static_cast<CParticleSystem *>(pParticleSystem), true);
            else if (pParticleSystem->IsModifier())
                SceneManager.AddModifier(static_cast<CSceneEntityModifier *>(pParticleSystem), m_uiSourceEntityIndex, m_uiTargetEntityIndex);
        }
    }

    m_pEffectThread->Cleanup();
}


/*====================
  CGameEvent::ExpireEffect
  ====================*/
void    CGameEvent::ExpireEffect()
{
    if (m_pEffectThread == NULL)
        return;

    if (!m_pEffectThread->IsDeferred() && !m_pEffectThread->IsPersistent())
    {
        SAFE_DELETE(m_pEffectThread);
        return;
    }

    if (!m_pEffectThread->IsPersistent())
        m_pEffectThread->Expire(Game.GetGameTime());

    m_bEffectActive = m_pEffectThread->IsPersistent();
}

