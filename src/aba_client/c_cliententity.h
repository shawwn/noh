// (C)2005 S2 Games
// c_cliententity.h
//
//=============================================================================
#ifndef __C_CLIENTENTITY_H__
#define __C_CLIENTENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "../aba_shared/i_unitentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
class CWorld;
class CSkeleton;
class CEffectThread;
class CSceneEntity;

const uint  NUM_CLIENT_EFFECTS(32);
const uint  NUM_CLIENT_EFFECT_THREADS(NUM_EFFECT_CHANNELS + NUM_CLIENT_EFFECTS + MAX_INVENTORY);

const uint  NUM_CLIENT_SOUNDS(32);
const uint  NUM_CLIENT_SOUND_HANDLES(NUM_CLIENT_SOUNDS);

const uint  CE_VALID                (BIT(0));
const uint  CE_STATIC               (BIT(1));
const uint  CE_EFFECT_THREAD_ACTIVE (BIT(2));
const uint  CE_SOUND_ACTIVE         (BIT(3));
//=============================================================================

//=============================================================================
// CClientEntity
//=============================================================================
class CClientEntity
{
private:
    uint            m_uiIndex;
    ushort          m_unType;
    uint            m_uiFlags;

    ResHandle       m_hModel;
    CSkeleton*      m_pSkeleton;

    CBBoxf          m_bbEntityBounds;

    int             m_aiActiveAnim[NUM_ANIM_CHANNELS];
    byte            m_ayActiveAnimSequence[NUM_ANIM_CHANNELS];

    ResHandle       m_ahActiveEffect[NUM_EFFECT_CHANNELS];
    byte            m_ayActiveEffectSequence[NUM_EFFECT_CHANNELS];
    ResHandle       m_ahActiveSlaveEffect[MAX_INVENTORY];
    byte            m_ayActiveSlaveEffectSequence[MAX_INVENTORY];
    uint            m_auiActiveSlaveIndex[MAX_INVENTORY];

    CEffectThread*  m_apEffectThread[NUM_CLIENT_EFFECT_THREADS];
    SoundHandle     m_ahSoundHandle[NUM_CLIENT_SOUND_HANDLES];

    IVisualEntity*  m_pNextState;
    IVisualEntity*  m_pPrevState;
    IVisualEntity*  m_pCurrentState;

    int             m_iTalkingEffectChannel;
    int             m_iDisconnectedEffectChannel;
    int             m_iIllusionEffectChannel;

    int             m_iFramesReceived;

    CVec3f          m_v3PositionLinked;
    byte            m_yStatusLinked;
    float           m_fScaleLinked;
    ResHandle       m_hModelLinked;

    PoolOffset      m_hNextClientEntity;

public:
    ~CClientEntity();
    CClientEntity();

    bool            IsValid() const;
    bool            IsStatic() const    { return (m_uiFlags & CE_STATIC) != 0; }

    // Accessors to IGameEntity functions
    void            SetIndex(uint uiIndex);
    void            SetType(ushort unType);
    void            SetSkeleton(CSkeleton *pSkeleton);
    void            SetWorldIndex(uint uiWorldIndex);
    uint            GetWorldIndex() const;
    int             GetClientID() const;

    void            Free();
    void            Initialize(IVisualEntity *pEntity);
    void            Interpolate(float fLerp);
    void            Frame();
    void            AddToScene();
    void            CopyNextToPrev();
    void            CopyNextToCurrent();

    IVisualEntity*  GetNextEntity()                         { return m_pNextState; }
    IVisualEntity*  GetPrevEntity()                         { return m_pPrevState; }
    IVisualEntity*  GetCurrentEntity()                      { return m_pCurrentState; }

    ushort          GetType() const                         { return m_unType; }
    uint            GetIndex() const                        { return m_uiIndex; }

    int             StartEffect(ResHandle hEffect, int iChannel, int iTimeNudge);
    int             StartEffect(const tstring &sEffect, int iChannel, int iTimeNudge);
    void            StopEffect(int iChannel);
    void            ExpireEffect(CEffectThread *&pEffectThread);
    void            PassEffects();

    int             PlaySound(ResHandle hSample, float fVolume, float fFalloff, int iChannel, int iPriority, int iSoundFlags = 0, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, bool bOverride = true, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0, float fFalloffEnd = 0.0f);
    void            StopSound(int iChannel);
    void            StopAllSounds();

    CEffectThread*  GetEffectThread(uint uiIndex)           { return m_apEffectThread[uiIndex]; }

    void            SetNextClientEntity(CClientEntity *pNext)   { if (pNext == NULL) m_hNextClientEntity = INVALID_POOL_OFFSET; else m_hNextClientEntity = pNext - this; }
    CClientEntity*  GetNextClientEntity()                       { if (m_hNextClientEntity == INVALID_POOL_OFFSET) return NULL; else return this + m_hNextClientEntity; }

    void            Rewind();
};
//=============================================================================

//=============================================================================
// Inline Functions
//=============================================================================

/*====================
  CClientEntity::IsValid
  ====================*/
inline
bool    CClientEntity::IsValid() const
{
    if (m_pPrevState == NULL ||
        m_pNextState == NULL ||
        !m_pPrevState->IsValid() ||
        !m_pNextState->IsValid())
        return false;

    return true;
}
//=============================================================================

#endif //__C_CLIENTENTITY_H__
