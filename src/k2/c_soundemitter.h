// (C)2007 S2 Games
// c_soundemitter.h
//
//=============================================================================
#ifndef __C_SOUNDEMITTER_H__
#define __C_SOUNDEMITTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_emitter.h"
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
#include "c_range.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CParticleSystem;
//=============================================================================

//=============================================================================
// CSoundEmitterDef
//=============================================================================
class CSoundEmitterDef : public IEmitterDef
{
private:
    // Emitter Properties
    CRangei                     m_riLife;
    CRangei                     m_riExpireLife;
    CRangei                     m_riTimeNudge;
    CRangei                     m_riDelay;
    bool                        m_bLoop;
    int                         m_iFadeIn;
    int                         m_iFadeOutStartTime;
    int                         m_iFadeOut;
    tstring                     m_sBone;
    CVec3f                      m_v3Pos;
    CVec3f                      m_v3Offset;
    CRangef                     m_rfFalloff;
    CRangef                     m_rfVolume;
    CTemporalPropertyRangef     m_trfPitch;
    uint                        m_uiSoundFlags;
    ResHandle                   m_hSample;
    CRangef                     m_rfSpeed1;
    CRangef                     m_rfSpeed2;
    int                         m_iSpeedUpTime;
    int                         m_iSlowDownTime;
    CRangef                     m_rfFalloffStart;
    CRangef                     m_rfFalloffEnd;

public:
    virtual ~CSoundEmitterDef();
    CSoundEmitterDef
    (
        const CRangei &riLife,
        const CRangei &riExpireLife,
        const CRangei &riTimeNudge,
        const CRangei &riDelay,
        bool bLoop,
        int iFadeIn,
        int iFadeOutStartTime,
        int iFadeOut,
        const tstring &sBone,
        const CVec3f &v3Pos,
        const CVec3f &v3Offset,
        const CRangef &rfFalloff,
        const CRangef &rfVolume,
        const CTemporalPropertyRangef &trfPitch,
        uint uiSoundFlags,
        ResHandle hSample,
        const CRangef &rfSpeed1,
        const CRangef &rfSpeed2,
        int iSpeedUpTime,
        int iSlowDownTime,
        const CRangef &rfFalloffStart,
        const CRangef &rfFalloffEnd
    );

    IEmitter*   Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner);

    int         GetLife() const         { return m_riLife; }
    int         GetExpireLife() const   { return m_riExpireLife; }
    int         GetTimeNudge() const    { return m_riTimeNudge; }
    int         GetDelay() const        { return m_riDelay; }
    bool        GetLoop() const         { return m_bLoop; }
    int         GetFadeIn() const       { return m_iFadeIn; }
    int         GetFadeOutStartTime() const { return m_iFadeOutStartTime; }
    int         GetFadeOut() const      { return m_iFadeOut; }

    const tstring&  GetBone() const     { return m_sBone; }
    const CVec3f&   GetPos() const      { return m_v3Pos; }
    const CVec3f&   GetOffset() const   { return m_v3Offset; }

    CRangef                     GetFalloff() const          { return m_rfFalloff; }
    CRangef                     GetFalloffStart() const     { return m_rfFalloffStart; }
    CRangef                     GetFalloffEnd() const       { return m_rfFalloffEnd; }
    CRangef                     GetVolume() const           { return m_rfVolume; }
    CTemporalPropertyf          GetPitch() const            { return m_trfPitch; }

    uint                        GetSoundFlags() const       { return m_uiSoundFlags; }

    ResHandle                   GetSample() const           { return m_hSample; }

    CRangef     GetSpeed1() const       { return m_rfSpeed1; }
    CRangef     GetSpeed2() const       { return m_rfSpeed2; }
    int         GetSpeedUpTime() const  { return m_iSpeedUpTime; }
    int         GetSlowDownTime() const { return m_iSlowDownTime; }
};


//=============================================================================
// CSoundEmitter
//=============================================================================
class CSoundEmitter : public IEmitter
{
private:
    SoundHandle             m_hSoundHandle;

    // Emitter Properties
    CRangef                 m_rfFalloff;
    CRangef                 m_rfVolume;
    CTemporalPropertyf      m_tfPitch;
    int                     m_iSoundFlags;
    ResHandle               m_hSample;
    int                     m_iFadeIn;
    int                     m_iFadeOutStartTime;
    int                     m_iFadeOut;
    CRangef                 m_rfSpeed1;
    CRangef                 m_rfSpeed2;
    int                     m_iSpeedUpTime;
    int                     m_iSlowDownTime;
    CRangef                 m_rfFalloffStart;
    CRangef                 m_rfFalloffEnd;
    bool                    m_bStarted;

public:
    virtual ~CSoundEmitter();
    CSoundEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CSoundEmitterDef &eSettings);

    bool    Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);
};
//=============================================================================
#endif  //__C_SOUNDEMITTER_H__
