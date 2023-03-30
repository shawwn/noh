// (C)2006 S2 Games
// c_orbiter.h
// Polar coordinate based particle
//=============================================================================
#ifndef __C_ORBITER_H__
#define __C_ORBITER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
#include "c_simpleparticle.h"
//=============================================================================

//=============================================================================
// COrbiter
//=============================================================================
class COrbiter
{
private:
    bool        m_bActive;
    uint        m_uiStartTime;
    int         m_iLife;

    CVec3f      m_v3Pos;
    CVec3f      m_v3Velocity;
    CVec3f      m_v3Dir;
    float       m_fAcceleration;

    CVec3f      m_v3Up;

    CTemporalPropertyv3 m_tv3Color;
    CTemporalPropertyf  m_tfAlpha;
    CTemporalPropertyf  m_tfWidth;
    CTemporalPropertyf  m_tfHeight;
    CTemporalPropertyf  m_tfScale;
    CTemporalPropertyf  m_tfAngle;
    CTemporalPropertyf  m_tfPitch;
    CTemporalPropertyf  m_tfYaw;
    CTemporalPropertyf  m_tfFrame;
    CTemporalPropertyf  m_tfParam;

    float       m_fS1, m_fS2;
    float       m_fT1, m_fT2;

    CVec2f      m_v2Center;

    uint        m_uiFlags;

public:
    ~COrbiter() {};
    COrbiter() : m_bActive(false) {};

    COrbiter
    (
        uint uiStartTime,
        int iLife,
        const CVec3f &v3Pos,
        const CVec3f &v3Velocity,
        const CVec3f &v3Dir,
        float fAcceleration,
        const CVec3f &v3Up,
        const CSimpleParticleDef &settings
    );

    bool    IsDead(uint uiLastUpdateTime, bool bExpired)    { return m_iLife == -1 ? bExpired : uiLastUpdateTime > m_uiStartTime + m_iLife; }
    void    AddToStartTime(uint uiPauseTime) { m_uiStartTime += uiPauseTime; }

    void    Update(float fDeltaTime, const CVec3f &v3Acceleration, float fDrag, float fFriction);

    void    GetBillboard(uint uiMilliseconds, SBillboard &outBillboard);

    bool    IsActive()              { return m_bActive; }
    void    SetActive(bool bActive) { m_bActive = bActive; }

    const CVec3f&   GetUp()         { return m_v3Up; }
    const CVec3f&   GetPos()        { return m_v3Pos; }
    const CVec3f&   GetDir()        { return m_v3Dir; }
    const CVec3f&   GetVelocity()   { return m_v3Velocity; }
    uint            GetFlags()      { return m_uiFlags; }
};
//=============================================================================
#endif  //__C_ORBITER_H__
