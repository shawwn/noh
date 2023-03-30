// (C)2006 S2 Games
// c_simpleparticle.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_simpleparticle.h"
#include "i_emitter.h"
#include "c_vid.h"
#include "c_camera.h"
//=============================================================================

//=============================================================================
// Globals
//============================================================================
//=============================================================================

/*====================
  CSimpleParticleDef::CSimpleParticleDef
  ====================*/
CSimpleParticleDef::CSimpleParticleDef
(
    const CTemporalPropertyv3 &tv3Color,
    const CTemporalPropertyRangef &tfAlpha,
    const CTemporalPropertyRangef &trfWidth,
    const CTemporalPropertyRangef &trfHeight,
    const CTemporalPropertyRangef &trfScale,
    const CTemporalPropertyRangef &trfAngle,
    const CTemporalPropertyRangef &trfPitch,
    const CTemporalPropertyRangef &trfYaw,
    const CTemporalPropertyRangef &trfFrame,
    const CTemporalPropertyRangef &trfParam,
    const CTemporalPropertyRangef &trfStickiness,
    const CTemporalPropertyRangef &trfAnchor,
    const CTemporalPropertyRangef &trfWidthDistort,
    const CTemporalPropertyRangef &trfHeightDistort,
    float fScaleU,
    float fScaleV,
    float fOffsetU,
    float fOffsetV,
    const CVec2f &v2Center,
    float fSelectionWeight,
    uint uiFlags,
    const tsvector &vEmitters
) :
m_tv3Color(tv3Color),
m_trfAlpha(tfAlpha),
m_trfWidth(trfWidth),
m_trfHeight(trfHeight),
m_trfScale(trfScale),
m_trfAngle(trfAngle),
m_trfPitch(trfPitch),
m_trfYaw(trfYaw),
m_trfFrame(trfFrame),
m_trfParam(trfParam),
m_trfStickiness(trfStickiness),
m_trfAnchor(trfAnchor),
m_trfWidthDistort(trfWidthDistort),
m_trfHeightDistort(trfHeightDistort),
m_fScaleU(fScaleU),
m_fScaleV(fScaleV),
m_fOffsetU(fOffsetU),
m_fOffsetV(fOffsetV),
m_v2Center(v2Center),
m_fSelectionWeight(fSelectionWeight),
m_uiFlags(uiFlags),
m_vEmitters(vEmitters)
{
}


/*====================
  CSimpleParticleDef::AddEmitterDef
  ====================*/
void    CSimpleParticleDef::AddEmitterDef(IEmitterDef *pEmitterDef)
{
    m_vEmitterDefs.push_back(pEmitterDef);
}


/*====================
  CSimpleParticle::CSimpleParticle
  ====================*/
CSimpleParticle::~CSimpleParticle()
{
    SAFE_DELETE(m_pImbeddedEmitter);
}


/*====================
  CSimpleParticle::Spawn
  ====================*/
void    CSimpleParticle::Spawn
(
    uint uiStartTime,
    int iLife,
    const CVec3f &v3Pos,
    const CVec3f &v3Velocity,
    const CVec3f &v3Dir,
    float fAcceleration,
    float fScale,
    const CVec3f &v3EmitterPos,
    IEmitter *pImbeddedEmitter,
    const CSimpleParticleDef &settings
)
{
    assert(iLife != 0);

    SAFE_DELETE(m_pImbeddedEmitter);

    m_bActive = true;
    m_uiStartTime = uiStartTime;
    m_iLife = iLife;
    m_v3Pos = v3Pos;
    m_v3Velocity = v3Velocity;
    m_v3Dir = v3Dir;
    m_fAcceleration = fAcceleration;
    m_fScale = fScale;
    m_v3EmitterPos = v3EmitterPos;
    m_tv3Color = settings.GetColor();
    m_tfAlpha = settings.GetAlpha();
    m_tfWidth = settings.GetWidth();
    m_tfHeight = settings.GetHeight();
    m_tfScale = settings.GetScale();
    m_tfAngle = settings.GetAngle();
    m_tfPitch = settings.GetPitch();
    m_tfYaw = settings.GetYaw();
    m_tfFrame = settings.GetFrame();
    m_tfParam = settings.GetParam();
    m_tfStickiness = settings.GetStickiness();
    m_tfAnchor = settings.GetAnchor();
    m_tfWidthDistort = settings.GetWidthDistort();
    m_tfHeightDistort = settings.GetHeightDistort();
    m_uiFlags = settings.GetFlags();
    m_pImbeddedEmitter = pImbeddedEmitter;

    float fScaleU(settings.GetScaleU());
    float fScaleV(settings.GetScaleV());
    float fOffsetU(settings.GetOffsetU());
    float fOffsetV(settings.GetOffsetV());

    if (fScaleU < 0.0f)
    {
        m_fS1 = fOffsetU - fScaleU;
        m_fS2 = fOffsetU;
    }
    else
    {
        m_fS1 = fOffsetU;
        m_fS2 = fOffsetU + fScaleU;
    }

    if (fScaleV < 0.0f)
    {
        m_fT1 = fOffsetV - fScaleV;
        m_fT2 = fOffsetV;
    }
    else
    {
        m_fT1 = fOffsetV;
        m_fT2 = fOffsetV + fScaleV;
    }

    m_v2Center = settings.GetCenter();
}


/*====================
  CSimpleParticle::Update
  ====================*/
void    CSimpleParticle::Update(float fDeltaTime, const CVec3f &v3Acceleration, float fDrag, float fFriction, ParticleTraceFn_t pfnTrace)
{
    if (fDeltaTime == 0.0f)
        return;

    CVec3f v3Pos(m_v3Pos);
    CVec3f v3Velocity(m_v3Velocity);
    CVec3f v3Dir(m_v3Dir);

    if (m_fScale != 1.0f)
    {
        if (fDrag != 0.0f || fFriction != 0.0)
        {
            float fSpeedSq(v3Velocity.LengthSq());

            if (fSpeedSq > 0.0f)
            {
                float fSpeed(sqrt(fSpeedSq));

                v3Dir = v3Velocity / fSpeed;
                float vDrag(-MIN(fSpeedSq / (m_fScale * m_fScale) * 0.5f * fDrag, fSpeed / fDeltaTime));

                if (fFriction != 0.0f)
                {
                    CVec3f  v3Friction(v3Dir * fFriction * m_fScale * fDeltaTime);

                    // Apply friction
                    if (v3Velocity.x > 0.0f)
                        v3Velocity.x = MAX(v3Velocity.x - v3Friction.x, 0.0f);
                    else
                        v3Velocity.x = MIN(v3Velocity.x - v3Friction.x, 0.0f);

                    if (v3Velocity.y > 0.0f)
                        v3Velocity.y = MAX(v3Velocity.y - v3Friction.y, 0.0f);
                    else
                        v3Velocity.y = MIN(v3Velocity.y - v3Friction.y, 0.0f);

                    if (v3Velocity.z > 0.0f)
                        v3Velocity.z = MAX(v3Velocity.z - v3Friction.z, 0.0f);
                    else
                        v3Velocity.z = MIN(v3Velocity.z - v3Friction.z, 0.0f);
                }

                CVec3f v3ScaleAccel(v3Acceleration);
                v3ScaleAccel *= m_fScale;
                v3ScaleAccel.ScaleAdd(v3Dir, (m_fAcceleration + vDrag) * m_fScale);

                v3Pos.ScaleAdd(v3Velocity, fDeltaTime);
                v3Pos.ScaleAdd(v3ScaleAccel, 0.5f * fDeltaTime * fDeltaTime);
                
                v3Velocity.ScaleAdd(v3ScaleAccel, fDeltaTime);
            }
            else
            {
                CVec3f v3ScaleAccel(v3Acceleration);
                v3ScaleAccel *= m_fScale;

                if (m_fAcceleration != 0.0f)
                    v3ScaleAccel.ScaleAdd(v3Dir, m_fAcceleration * m_fScale);

                v3Pos.ScaleAdd(v3ScaleAccel, 0.5f * fDeltaTime * fDeltaTime);
                
                v3Velocity.ScaleAdd(v3ScaleAccel, fDeltaTime);
            }
        }
        else
        {
            float fSpeedSq(v3Velocity.LengthSq());

            if (fSpeedSq > 0.0f)
            {
                v3Dir = v3Velocity / sqrt(fSpeedSq);

                v3Pos += v3Velocity * fDeltaTime + (v3Acceleration + (v3Dir * m_fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime * m_fScale);
                v3Velocity += (v3Acceleration + (v3Dir * m_fAcceleration)) * (fDeltaTime * m_fScale);
            }
            else
            {
                v3Pos += (v3Acceleration + (v3Dir * m_fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime * m_fScale);
                v3Velocity += (v3Acceleration + (v3Dir * m_fAcceleration)) * (fDeltaTime * m_fScale);
            }
        }
    }
    else
    {
        if (fDrag != 0.0f || fFriction != 0.0)
        {
            float fSpeedSq(v3Velocity.LengthSq());

            if (fSpeedSq > 0.0f)
            {
                float fSpeed(sqrt(fSpeedSq));

                v3Dir = v3Velocity / fSpeed;
                float vDrag(-MIN(fSpeedSq * 0.5f * fDrag, fSpeed / fDeltaTime));

                if (fFriction != 0.0f)
                {
                    CVec3f  v3Friction(v3Dir * fFriction * fDeltaTime);

                    // Apply friction
                    if (v3Velocity.x > 0.0f)
                        v3Velocity.x = MAX(v3Velocity.x - v3Friction.x, 0.0f);
                    else
                        v3Velocity.x = MIN(v3Velocity.x - v3Friction.x, 0.0f);

                    if (v3Velocity.y > 0.0f)
                        v3Velocity.y = MAX(v3Velocity.y - v3Friction.y, 0.0f);
                    else
                        v3Velocity.y = MIN(v3Velocity.y - v3Friction.y, 0.0f);

                    if (v3Velocity.z > 0.0f)
                        v3Velocity.z = MAX(v3Velocity.z - v3Friction.z, 0.0f);
                    else
                        v3Velocity.z = MIN(v3Velocity.z - v3Friction.z, 0.0f);
                }

                v3Pos += v3Velocity * fDeltaTime + (v3Acceleration + (v3Dir * (m_fAcceleration + vDrag))) * (0.5f * fDeltaTime * fDeltaTime);
                v3Velocity += (v3Acceleration + (v3Dir * (m_fAcceleration + vDrag))) * fDeltaTime;
            }
            else
            {
                v3Pos +=  (v3Acceleration + (v3Dir * m_fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime);
                v3Velocity += (v3Acceleration + (v3Dir * m_fAcceleration)) * fDeltaTime;
            }
        }
        else
        {
            float fSpeedSq(v3Velocity.LengthSq());

            if (fSpeedSq > 0.0f)
            {
                if (m_fAcceleration)
                {
                    v3Dir = v3Velocity / sqrt(fSpeedSq);

                    v3Pos += v3Velocity * fDeltaTime + (v3Acceleration + (v3Dir * m_fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime);
                    v3Velocity += (v3Acceleration + (v3Dir * m_fAcceleration)) * fDeltaTime;
                }
                else
                {
                    v3Pos += v3Velocity * fDeltaTime + v3Acceleration * (0.5f * fDeltaTime * fDeltaTime);
                    v3Velocity += v3Acceleration * fDeltaTime;
                }
            }
            else
            {
                v3Pos += (v3Acceleration + (v3Dir * m_fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime);
                v3Velocity += (v3Acceleration + (v3Dir * m_fAcceleration)) * fDeltaTime;
            }
        }
    }

    if (pfnTrace)
    {
        CVec3f v3Normal;

        if (pfnTrace(m_v3Pos, v3Pos, m_v3Pos, v3Normal))
        {
            m_v3Velocity = Reflect(v3Velocity, v3Normal, 0.5f);
            m_v3Dir = Normalize(m_v3Velocity);
        }
        else
        {
            m_v3Velocity = v3Velocity;
            m_v3Dir = v3Dir;
        }
    }
    else
    {
        m_v3Pos = v3Pos;
        m_v3Velocity = v3Velocity;
        m_v3Dir = v3Dir;
    }
}


/*====================
  CSimpleParticle::Update
  ====================*/
void    CSimpleParticle::Update(float fDeltaTime, const CVec3f &v3Acceleration, const CVec3f &v3AccelTime, const CVec3f &v3AccelTimeSq)
{
    if (fDeltaTime == 0.0f)
        return;

    if (m_fScale != 1.0f)
    {
        if (m_fAcceleration != 0.0f)
        {
            float fSpeedSq(m_v3Velocity.LengthSq());

            if (fSpeedSq > 0.0f)
            {
                m_v3Dir = m_v3Velocity / sqrt(fSpeedSq);

                CVec3f v3ScaleAccel(v3Acceleration);
                v3ScaleAccel *= m_fScale;
                v3ScaleAccel.ScaleAdd(m_v3Dir, m_fAcceleration * m_fScale);

                m_v3Pos.ScaleAdd(m_v3Velocity, fDeltaTime);
                m_v3Pos.ScaleAdd(v3ScaleAccel, 0.5f * fDeltaTime * fDeltaTime);
                
                m_v3Velocity.ScaleAdd(v3ScaleAccel, fDeltaTime);
            }
            else
            {
                CVec3f v3ScaleAccel(v3Acceleration);
                v3ScaleAccel *= m_fScale;

                if (m_fAcceleration != 0.0f)
                    v3ScaleAccel.ScaleAdd(m_v3Dir, m_fAcceleration * m_fScale);

                m_v3Pos.ScaleAdd(v3ScaleAccel, 0.5f * fDeltaTime * fDeltaTime);

                m_v3Velocity.ScaleAdd(v3ScaleAccel, fDeltaTime);
            }
        }
        else
        {
            m_v3Pos.ScaleAdd(m_v3Velocity, fDeltaTime);
            m_v3Pos.ScaleAdd(v3AccelTimeSq, m_fScale);

            m_v3Velocity.ScaleAdd(v3AccelTime, m_fScale);

        }
    }
    else
    {
        if (m_fAcceleration != 0.0f)
        {
            float fSpeedSq(m_v3Velocity.LengthSq());

            if (fSpeedSq > 0.0f)
            {
                m_v3Dir = m_v3Velocity / sqrt(fSpeedSq);

                m_v3Pos.ScaleAdd(m_v3Velocity, fDeltaTime);
                m_v3Pos.ScaleAdd(v3Acceleration + (m_v3Dir * m_fAcceleration), 0.5f * fDeltaTime * fDeltaTime);

                m_v3Velocity.ScaleAdd(v3Acceleration + (m_v3Dir * m_fAcceleration), fDeltaTime);
            }
            else
            {
                m_v3Pos.ScaleAdd(v3Acceleration + (m_v3Dir * m_fAcceleration), 0.5f * fDeltaTime * fDeltaTime);

                m_v3Velocity.ScaleAdd(v3Acceleration + (m_v3Dir * m_fAcceleration), fDeltaTime);
            }
        }
        else
        {
            m_v3Pos.ScaleAdd(m_v3Velocity, fDeltaTime);

            m_v3Pos += v3AccelTimeSq;
            m_v3Velocity += v3AccelTime;
        }
    }
}


/*====================
  CSimpleParticle::Update
  ====================*/
void    CSimpleParticle::Update(float fDeltaTime)
{
    if (fDeltaTime == 0.0f)
        return;

    m_v3Pos.ScaleAdd(m_v3Velocity, fDeltaTime);
}


/*====================
  CSimpleParticle::GetBillboard
  ====================*/
void    CSimpleParticle::GetBillboard(uint uiMilliseconds, SBillboard &outBillboard, float *pfStickiness, float *pfAnchor)
{
    float fTime((uiMilliseconds - m_uiStartTime) * SEC_PER_MS);

    outBillboard.v3Pos = m_v3Pos;

    float fLerp(m_iLife != -1 ? float(uiMilliseconds - m_uiStartTime) / m_iLife : 0.0f);
    float fScale(m_tfScale.Lerp(fLerp) * m_fScale);

    float fWidthDistort(m_tfWidthDistort.Evaluate(fLerp, fTime));
    float fHeightDistort(m_tfHeightDistort.Evaluate(fLerp, fTime));

    if (fWidthDistort != 0.0f || fHeightDistort != 0.0f)
    {
        const CCamera *pCam(Vid.GetCamera());
        float fSpeed(Length(m_v3Velocity - pCam->GetViewAxis(FORWARD) * DotProduct(pCam->GetViewAxis(FORWARD), m_v3Velocity)));

        fWidthDistort *= fSpeed;
        fHeightDistort *= fSpeed;

        outBillboard.width = (m_tfWidth.Evaluate(fLerp, fTime) + fWidthDistort) * fScale;
        outBillboard.height = (m_tfHeight.Evaluate(fLerp, fTime) + fHeightDistort) * fScale;
    }
    else
    {
        outBillboard.width = m_tfWidth.Evaluate(fLerp, fTime) * fScale;
        outBillboard.height = m_tfHeight.Evaluate(fLerp, fTime) * fScale;
    }

    outBillboard.angle = m_tfAngle.Evaluate(fLerp, fTime);
    outBillboard.fPitch = m_tfPitch.Evaluate(fLerp, fTime);
    outBillboard.fYaw = m_tfYaw.Evaluate(fLerp, fTime);
    outBillboard.frame = m_tfFrame.Evaluate(fLerp, fTime);
    outBillboard.param = m_tfParam.Evaluate(fLerp, fTime);
    outBillboard.color.xyz() = m_tv3Color.Evaluate(fLerp, fTime);
    outBillboard.color.w = m_tfAlpha.Evaluate(fLerp, fTime);

    outBillboard.s1 = m_fS1;
    outBillboard.s2 = m_fS2;
    outBillboard.t1 = m_fT1;
    outBillboard.t2 = m_fT2;

    if (m_uiFlags & BBOARD_OFFCENTER)
        outBillboard.v2Center = m_v2Center;
    
    if (m_uiFlags & BBOARD_TURN)
    {
        const CCamera *pCam(Vid.GetCamera());

        CVec3f v3Dir(m_v3Velocity);

        // Orthogonalize
        v3Dir -= pCam->GetViewAxis(FORWARD) * DotProduct(pCam->GetViewAxis(FORWARD), v3Dir);
        v3Dir.Normalize();

        float fDotUp(DotProduct(pCam->GetViewAxis(UP), v3Dir));
        float fDotRight(DotProduct(pCam->GetViewAxis(RIGHT), v3Dir));

        if (fDotUp > 0.0f)
        {
            if (fDotRight > 0.0f) // Quadrant I
                outBillboard.angle -= RAD2DEG(acos(fDotUp));
            else // Quadrant II
                outBillboard.angle += RAD2DEG(acos(fDotUp));
        }
        else
        {
            if (fDotRight < 0.0f) // Quadrant III
                outBillboard.angle += RAD2DEG(acos(fDotUp));
            else // Quadrant IV
                outBillboard.angle -= RAD2DEG(acos(fDotUp));
        }
    }

    outBillboard.fDepthBias = 0.0f;
    outBillboard.uiFlags = m_uiFlags;

    if (pfStickiness)
        *pfStickiness = m_tfStickiness.Evaluate(fLerp, fTime);
    
    if (pfAnchor)
        *pfAnchor = m_tfAnchor.Evaluate(fLerp, fTime);
}
