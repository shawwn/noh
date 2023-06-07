// (C)2005 S2 Games
// c_cliententity.h
//
//=============================================================================
#ifndef __C_CLIENTENTITY_H__
#define __C_CLIENTENTITY_H__

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
class CWorld;
class CSkeleton;
class CEffectThread;
class CSceneEntity;

const int NUM_CLIENT_SOUNDS(32);
const int NUM_CLIENT_SOUND_HANDLES(NUM_CLIENT_SOUNDS);
//=============================================================================

//=============================================================================
// CClientEntity
//=============================================================================
class CClientEntity
{
private:
    int             m_iFramesReceived;

    uint            m_uiIndex;
    ushort          m_unType;

    CSkeleton*      m_pSkeleton;
    bool            m_bStatic;
    bool            m_bClientEntity;

    int             m_aiActiveAnim[NUM_ANIM_CHANNELS];
    byte            m_ayActiveAnimSequence[NUM_ANIM_CHANNELS];

    ResHandle       m_hNoBuildEffect;
    ResHandle       m_hStatePreviewEffect;
    ResHandle       m_ahActiveEffect[NUM_EFFECT_CHANNELS];
    byte            m_ayActiveEffectSequence[NUM_EFFECT_CHANNELS];

    CEffectThread*  m_apEffectThread[NUM_CLIENT_EFFECT_THREADS];
    SoundHandle     m_ahSoundHandle[NUM_CLIENT_SOUND_HANDLES];

    IVisualEntity*  m_pNextState;
    IVisualEntity*  m_pPrevState;
    IVisualEntity*  m_pCurrentState;

    int             m_iIndicatorEffectChannel;
    ResHandle       m_hIndicatorEffect;
    int             m_iEyeEffectChannel;
    int             m_iBuildEffectChannel;
    int             m_iStatePreviewChannel;

    CVec3f          m_v3PositionLinked;
    byte            m_yStatusLinked;
    float           m_fScaleLinked;
    ResHandle       m_hModelLinked;

public:
    ~CClientEntity();
    CClientEntity();

    bool            IsValid() const;
    bool            IsStatic() const    { return m_bStatic; }
    bool            IsClientEntity() const  { return m_bClientEntity; }
    void            SetClientEntity(bool b) { m_bClientEntity = b; }

    // Accessors to IGameEntity functions
    void            SetIndex(uint uiIndex);
    void            SetType(ushort unType);
    void            SetSkeleton(CSkeleton *pSkeleton);
    void            SetWorldIndex(uint uiWorldIndex);
    uint            GetWorldIndex() const;
    int             GetClientID() const;
    void            SetNoBuildEffect(tstring sEffectPath)   { m_hNoBuildEffect = g_ResourceManager.Register(sEffectPath, RES_EFFECT); }
    void            SetStatePrevEffect(tstring sPath)       { m_hStatePreviewEffect = g_ResourceManager.Register(sPath, RES_EFFECT); }

    void            StopNoBuildEffect()                     { if (m_iBuildEffectChannel == -1) return; StopEffect(m_iBuildEffectChannel); m_hNoBuildEffect = INVALID_RESOURCE; m_iBuildEffectChannel = -1; }
    void            StopStatePreviewEffect()                { if (m_iStatePreviewChannel == -1) return; StopEffect(m_iStatePreviewChannel); m_hStatePreviewEffect = INVALID_RESOURCE; m_iStatePreviewChannel = -1; }

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
    void            PassEffects();

    void            PlaySound(ResHandle hSample, float fVolume, float fFalloff, int iChannel, int iPriority, bool bLoop = false, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, bool bOverride = true, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0);
    void            StopSound(int iChannel);
    void            StopAllSounds();

    CEffectThread*  GetEffectThread(uint uiIndex)           { return m_apEffectThread[uiIndex]; }
};
//=============================================================================

#endif //__C_CLIENTENTITY_H__
