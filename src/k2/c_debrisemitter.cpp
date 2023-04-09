// (C)2008 S2 Games
// c_debrisemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_debrisemitter.h"

#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
#include "c_skeleton.h"
#include "c_model.h"
#include "c_effectthread.h"
#include "c_effect.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CDebrisEmitterDef::~CDebrisEmitterDef
  ====================*/
CDebrisEmitterDef::~CDebrisEmitterDef()
{
}


/*====================
  CDebrisEmitterDef::CDebrisEmitterDef
  ====================*/
CDebrisEmitterDef::CDebrisEmitterDef
(
    const tstring &sName,
    const tstring &sOwner,
    const CRangei &riLife,
    const CRangei &riExpireLife,
    const CRangei &riTimeNudge,
    const CRangei &riDelay,
    bool bLoop,
    EDirectionalSpace eDirectionalSpace,
    const tstring &sBone,
    const CVec3f &v3Pos,
    const CVec3f &v3Offset,
    const CTemporalPropertyv3 &tv3Color,
    const CTemporalPropertyf &tfAlpha,
    const CTemporalPropertyRangef &trfPitch,
    const CTemporalPropertyRangef &trfRoll,
    const CTemporalPropertyRangef &trfYaw,
    const CTemporalPropertyRangef &trfScale,
    const CTemporalPropertyRangef &trfParam0,
    const CTemporalPropertyRangef &trfParam1,
    const CTemporalPropertyRangef &trfParam2,
    const CTemporalPropertyRangef &trfParam3,
    ResHandle hModel,
    SkinHandle hSkin,
    ResHandle hMaterial,
    const tstring &sAnim,
    const tsvector &vEmitters,
    const CTemporalPropertyRangef &rfGravity,
    const CTemporalPropertyRangef &rfMinSpeed,
    const CTemporalPropertyRangef &rfMaxSpeed,
    const CTemporalPropertyRangef &rfMinAcceleration,
    const CTemporalPropertyRangef &rfMaxAcceleration,
    const CTemporalPropertyRangef &rfMinAngle,
    const CTemporalPropertyRangef &rfMaxAngle,
    const CTemporalPropertyRangef &rfMinInheritVelocity,
    const CTemporalPropertyRangef &rfMaxInheritVelocity,
    const CTemporalPropertyRangef &rfLimitInheritVelocity,
    const CVec3f &v3Dir,
    float fDrag,
    float fFriction,
    const CTemporalPropertyv3 &tv3OffsetSphere,
    const CTemporalPropertyv3 &tv3OffsetCube,
    const CTemporalPropertyRangef &rfMinOffsetDirection,
    const CTemporalPropertyRangef &rfMaxOffsetDirection,
    const CTemporalPropertyRangef &rfMinOffsetRadial,
    const CTemporalPropertyRangef &rfMaxOffsetRadial,
    const CTemporalPropertyRangef &rfMinOffsetRadialAngle,
    const CTemporalPropertyRangef &rfMaxOffsetRadialAngle,
    bool bCollide,
    const CTemporalPropertyRangef &rfMinRotationSpeed,
    const CTemporalPropertyRangef &rfMaxRotationSpeed,
    float fBounce,
    float fReflect,
    bool bAnimPose,
    bool bUseAnim
) :
m_sName(sName),
m_sOwner(sOwner),
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_eDirectionalSpace(eDirectionalSpace),
m_sBone(sBone),
m_v3Pos(v3Pos),
m_v3Offset(v3Offset),
m_tv3Color(tv3Color),
m_tfAlpha(tfAlpha),
m_trfPitch(trfPitch),
m_trfRoll(trfRoll),
m_trfYaw(trfYaw),
m_trfScale(trfScale),
m_trfParam0(trfParam0),
m_trfParam1(trfParam1),
m_trfParam2(trfParam2),
m_trfParam3(trfParam3),
m_hModel(hModel),
m_hSkin(hSkin),
m_hMaterial(hMaterial),
m_sAnim(sAnim),
m_vEmitters(vEmitters),
m_rfGravity(rfGravity),
m_rfMinSpeed(rfMinSpeed),
m_rfMaxSpeed(rfMaxSpeed),
m_rfMinAcceleration(rfMinAcceleration),
m_rfMaxAcceleration(rfMaxAcceleration),
m_rfMinAngle(rfMinAngle),
m_rfMaxAngle(rfMaxAngle),
m_rfMinInheritVelocity(rfMinInheritVelocity),
m_rfMaxInheritVelocity(rfMaxInheritVelocity),
m_rfLimitInheritVelocity(rfLimitInheritVelocity),
m_v3Dir(v3Dir),
m_fDrag(fDrag),
m_fFriction(fFriction),
m_tv3OffsetSphere(tv3OffsetSphere),
m_tv3OffsetCube(tv3OffsetCube),
m_rfMinOffsetDirection(rfMinOffsetDirection),
m_rfMaxOffsetDirection(rfMaxOffsetDirection),
m_rfMinOffsetRadial(rfMinOffsetRadial),
m_rfMaxOffsetRadial(rfMaxOffsetRadial),
m_rfMinOffsetRadialAngle(rfMinOffsetRadialAngle),
m_rfMaxOffsetRadialAngle(rfMaxOffsetRadialAngle),
m_bCollide(bCollide),
m_rfMinRotationSpeed(rfMinRotationSpeed),
m_rfMaxRotationSpeed(rfMaxRotationSpeed),
m_fBounce(fBounce),
m_fReflect(fReflect),
m_bAnimPose(bAnimPose),
m_bUseAnim(bUseAnim)
{
}


/*====================
  CDebrisEmitterDef::Spawn
  ====================*/
IEmitter*   CDebrisEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("CDebrisEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, CDebrisEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CDebrisEmitterDef::AddEmitterDef
  ====================*/
void    CDebrisEmitterDef::AddEmitterDef(IEmitterDef *pEmitterDef)
{
    m_vEmitterDefs.push_back(pEmitterDef);
}


/*====================
  CDebrisEmitter::~CDebrisEmitter
  ====================*/
CDebrisEmitter::~CDebrisEmitter()
{
    SAFE_DELETE(m_pSkeleton);
}


/*====================
  CDebrisEmitter::CDebrisEmitter
  ====================*/
CDebrisEmitter::CDebrisEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CDebrisEmitterDef &eSettings) :
IEmitter
(
    eSettings.GetLife(),
    eSettings.GetExpireLife(),
    eSettings.GetTimeNudge(),
    eSettings.GetDelay(),
    eSettings.GetLoop(),
    eSettings.GetName(),
    eSettings.GetOwner(),
    eSettings.GetBone(),
    eSettings.GetPos(),
    eSettings.GetOffset(),
    eSettings.GetDirectionalSpace(),
    &eSettings.GetParticleDefinitions(),
    pParticleSystem,
    pOwner,
    uiStartTime
),
m_pSkeleton(NULL),
m_tv3Color(eSettings.GetColor()),
m_tfAlpha(eSettings.GetAlpha()),
m_tfPitch(eSettings.GetPitch()),
m_tfRoll(eSettings.GetRoll()),
m_tfYaw(eSettings.GetYaw()),
m_tfScale(eSettings.GetScale()),
m_tfParam0(eSettings.GetParam0()),
m_tfParam1(eSettings.GetParam1()),
m_tfParam2(eSettings.GetParam2()),
m_tfParam3(eSettings.GetParam3()),
m_hModel(eSettings.GetModel()),
m_hSkin(eSettings.GetSkin()),
m_hMaterial(eSettings.GetMaterial()),
m_rfGravity(eSettings.GetGravity()),
m_rfMinSpeed(eSettings.GetMinSpeed()),
m_rfMaxSpeed(eSettings.GetMaxSpeed()),
m_rfMinAcceleration(eSettings.GetMinAcceleration()),
m_rfMaxAcceleration(eSettings.GetMaxAcceleration()),
m_rfMinAngle(eSettings.GetMinAngle()),
m_rfMaxAngle(eSettings.GetMaxAngle()),
m_rfMinInheritVelocity(eSettings.GetMinInheritVelocity()),
m_rfMaxInheritVelocity(eSettings.GetMaxInheritVelocity()),
m_rfLimitInheritVelocity(eSettings.GetLimitInheritVelocity()),
m_tv3OffsetSphere(eSettings.GetOffsetSphere()),
m_tv3OffsetCube(eSettings.GetOffsetCube()),
m_rfMinOffsetDirection(eSettings.GetMinOffsetDirection()),
m_rfMaxOffsetDirection(eSettings.GetMaxOffsetDirection()),
m_rfMinOffsetRadial(eSettings.GetMinOffsetRadial()),
m_rfMaxOffsetRadial(eSettings.GetMaxOffsetRadial()),
m_rfMinOffsetRadialAngle(eSettings.GetMinOffsetRadialAngle()),
m_rfMaxOffsetRadialAngle(eSettings.GetMaxOffsetRadialAngle()),
m_v3Dir(eSettings.GetDir()),
m_fDrag(eSettings.GetDrag()),
m_fFriction(eSettings.GetFriction()),
m_bCollide(eSettings.GetCollide()),
m_rfMinRotationSpeed(eSettings.GetMinRotationSpeed()),
m_rfMaxRotationSpeed(eSettings.GetMaxRotationSpeed()),
m_fBounce(eSettings.GetBounce()),
m_fReflect(eSettings.GetReflect()),
m_bAnimPose(eSettings.GetAnimPose()),
m_bUseAnim(eSettings.GetUseAnim())
{
    m_uiLastUpdateTime -= m_iTimeNudge;

    m_uiStartTime += m_iDelay;
    m_uiLastUpdateTime += m_iDelay;

    CVec3f v3Pos(GetPosition());
    CAxis aAxis(GetAxis());
    float fScale(GetScale());

    if (m_eDirectionalSpace == DIRSPACE_LOCAL)
    {
        CAxis   aBoneAxis;
        CVec3f  v3BonePos;

        GetBoneAxisPos(uiStartTime, m_pOwner, m_sBone, aBoneAxis, v3BonePos);

        v3Pos = TransformPoint(v3BonePos, aAxis, v3Pos, fScale);
        aAxis = aAxis * aBoneAxis;
    }
    else
    {
        v3Pos = TransformPoint(GetBonePosition(uiStartTime, m_pOwner, m_sBone), aAxis, v3Pos, fScale);
    }

    m_aLastAxis = aAxis;
    m_v3LastPos = v3Pos;
    m_fLastScale = fScale * m_tfScale.Evaluate(0.0f, 0.0f);
    m_fLastLerp = 0.0f;
    m_fLastTime = 0.0f;

    const tstring &sAnim(eSettings.GetAnim());
    if (m_hModel != INVALID_RESOURCE)
    {
        m_pSkeleton = K2_NEW(ctx_Effects,  CSkeleton)();

        m_pSkeleton->SetModel(m_hModel);
        m_pSkeleton->StartAnim(sAnim, uiStartTime, 0);

        SBoneXForm *pCurrentPose(NULL);
        if (m_bAnimPose || m_bUseAnim)
        {
            m_pSkeleton->Pose(uiStartTime);
            pCurrentPose = m_pSkeleton->GetCurrentPose(0);
        }

        uint uiNumBones(m_pSkeleton->GetNumBones());

        m_vDebrisState.resize(uiNumBones);
        m_vBonePose.resize(uiNumBones);
        
        // Scene Root
        m_vDebrisState[0].v3Position = V3_ZERO;
        m_vDebrisState[0].v4Rotation = CVec4f(0.0f, 0.0f, 0.0f, 1.0f);
        m_vDebrisState[0].v3Velocity = V3_ZERO;
        m_vDebrisState[0].v4RotationVelocity = CVec4f(0.0f, 0.0f, 0.0f, 1.0f);
        m_vDebrisState[0].pImbeddedEmitter = NULL;

        float fLerp(0.0f);

        for (uint ui(1); ui < uiNumBones; ++ui)
        {
            float   fMinSpeed(m_rfMinSpeed.Lerp(fLerp));
            float   fMaxSpeed(m_rfMaxSpeed.Lerp(fLerp));
            float   fMinAcceleration(m_rfMinAcceleration.Lerp(fLerp));
            float   fMaxAcceleration(m_rfMaxAcceleration.Lerp(fLerp));
            float   fMinAngle(m_rfMinAngle.Lerp(fLerp));
            float   fMaxAngle(m_rfMaxAngle.Lerp(fLerp));
#if 0
            float   fMinInheritVelocity(m_rfMinInheritVelocity.Lerp(fLerp));
            float   fMaxInheritVelocity(m_rfMaxInheritVelocity.Lerp(fLerp));
#endif
            float   fMinOffsetDirection(m_rfMinOffsetDirection.Lerp(fLerp));
            float   fMaxOffsetDirection(m_rfMaxOffsetDirection.Lerp(fLerp));
            float   fMinOffsetRadial(m_rfMinOffsetRadial.Lerp(fLerp));
            float   fMaxOffsetRadial(m_rfMaxOffsetRadial.Lerp(fLerp));
            float   fMinOffsetRadialAngle(m_rfMinOffsetRadialAngle.Lerp(fLerp));
            float   fMaxOffsetRadialAngle(m_rfMaxOffsetRadialAngle.Lerp(fLerp));
            CVec3f  v3OffsetSphere(m_tv3OffsetSphere.Lerp(fLerp));
            CVec3f  v3OffsetCube(m_tv3OffsetCube.Lerp(fLerp));
            float   fMinRotationSpeed(m_rfMinRotationSpeed.Lerp(fLerp));
            float   fMaxRotationSpeed(m_rfMaxRotationSpeed.Lerp(fLerp));

            CVec3f  v3Dir(fMaxAngle - fMinAngle >= 180.0f ? M_RandomDirection() : M_RandomDirection(m_v3Dir, fMinAngle, fMaxAngle));
            v3Dir = TransformPoint(v3Dir, m_aLastAxis);

            //
            // Position
            //

            CVec3f  v3OffsetPos(0.0f, 0.0f, 0.0f);
            {
                if (v3OffsetSphere != V3_ZERO)
                {
                    CVec3f  v3Rand(M_RandomPointInSphere());

                    v3OffsetPos += v3OffsetSphere * v3Rand;
                }

                if (v3OffsetCube != V3_ZERO)
                {
                    CVec3f  v3Rand(M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f));

                    v3OffsetPos += v3OffsetCube * v3Rand;
                }

                float   fOffsetDirection(M_Randnum(fMinOffsetDirection, fMaxOffsetDirection));
                if (fOffsetDirection != 0.0f)
                {
                    v3OffsetPos += v3Dir * fOffsetDirection;
                }

                float   fOffsetRadial(M_Randnum(fMinOffsetRadial, fMaxOffsetRadial));
                if (fOffsetRadial != 0.0f)
                {
                    v3OffsetPos += M_RandomDirection(m_v3Dir, fMinOffsetRadialAngle, fMaxOffsetRadialAngle) * fOffsetRadial;
                }
            }

            CVec3f  v3Position(TransformPoint(v3OffsetPos, m_aLastAxis, m_v3LastPos, m_fLastScale));

            CVec3f  v3Velocity(v3Dir * (M_Randnum(fMinSpeed, fMaxSpeed) * m_fLastScale));

#if 0
            float   fLimitInheritVelocity(m_rfLimitInheritVelocity.Lerp(fLerp) * m_fLastScale);
            
            CVec3f  v3InheritVelocity(LERP(fTimeNudge / fDeltaTime, v3BaseVelocity, m_v3LastBaseVelocity) * M_Randnum(fMinInheritVelocity, fMaxInheritVelocity));
            if (fLimitInheritVelocity > 0.0f && v3InheritVelocity.Length() > fLimitInheritVelocity)
                v3InheritVelocity.SetLength(fLimitInheritVelocity);

            v3Velocity += v3InheritVelocity;
#endif

            float   fAcceleration(M_Randnum(fMinAcceleration, fMaxAcceleration) * m_fLastScale);

            // Spawn embedded emitters
            IEmitter *pImbeddedEmitter(NULL);
            IEmitter *pCurrentEmitter(NULL);
            const tsvector &vEmitters(eSettings.GetEmitters());
            if (!vEmitters.empty())
            {
                CEffect *pEffect(m_pParticleSystem->GetEffect());
                
                tsvector_cit cit(vEmitters.begin());

                IEmitterDef *pEmitterDef(pEffect->GetEmitterDef(*cit));

                m_pParticleSystem->SetCustomPos(v3Position);
                m_pParticleSystem->SetCustomAxis(AXIS_IDENTITY);
                m_pParticleSystem->SetCustomScale(1.0f);

                if (pEmitterDef != NULL)
                    pImbeddedEmitter = pEmitterDef->Spawn(uiStartTime + m_iDelay, m_pParticleSystem, OWNER_CUSTOM);

                if (pImbeddedEmitter != NULL)
                {
                    ++cit;

                    pCurrentEmitter = pImbeddedEmitter;
                    for (; cit != vEmitters.end(); ++cit)
                    {
                        IEmitterDef *pEmitterDef(pEffect->GetEmitterDef(*cit));
                        IEmitter *pNewEmitter(NULL);

                        if (pEmitterDef != NULL)
                            pNewEmitter = pEmitterDef->Spawn(uiStartTime + m_iDelay, m_pParticleSystem, OWNER_CUSTOM);

                        if (pNewEmitter != NULL)
                        {
                            pNewEmitter->SetOwner(OWNER_CUSTOM);
                            pNewEmitter->SetCustomPos(v3Position);
                            pNewEmitter->SetCustomScale(1.0f);
                        }

                        pCurrentEmitter->SetNextEmitter(pNewEmitter);
                        pCurrentEmitter = pNewEmitter;
                    }
                }
            }

            vector<IEmitterDef *>::const_iterator itEnd(eSettings.GetEmitterDefs().end());
            for (vector<IEmitterDef *>::const_iterator it(eSettings.GetEmitterDefs().begin()); it != itEnd; ++it)
            {
                for (int i(0); i < (*it)->GetCount(); ++i)
                {
                    m_pParticleSystem->SetCustomPos(v3Position);
                    m_pParticleSystem->SetCustomAxis(AXIS_IDENTITY);
                    m_pParticleSystem->SetCustomScale(1.0f);
                    
                    IEmitter *pNewEmitter((*it)->Spawn(uiStartTime + m_iDelay, m_pParticleSystem, OWNER_CUSTOM));
                    if (pNewEmitter != NULL)
                    {
                        if (pCurrentEmitter == NULL)
                        {
                            pImbeddedEmitter = pNewEmitter;
                            pCurrentEmitter = pImbeddedEmitter;
                        }
                        else
                        {
                            pCurrentEmitter->SetNextEmitter(pNewEmitter);
                            pCurrentEmitter = pNewEmitter;
                        }
                    }
                }
            }

            if (m_bUseAnim)
            {
                m_vDebrisState[ui].v3Position = pCurrentPose[ui].v3Pos;
                m_vDebrisState[ui].v4Rotation = pCurrentPose[ui].v4Quat;
            }
            else if (m_bAnimPose)
            {
                m_vDebrisState[ui].v3Position = TransformPoint(pCurrentPose[ui].v3Pos, aAxis * g_axisYaw180, v3Position, fScale);
                m_vDebrisState[ui].v4Rotation = M_MultQuat(pCurrentPose[ui].v4Quat, M_AxisAngleToQuat(V_UP, 180.0f));
            }
            else
            {
                m_vDebrisState[ui].v3Position = v3Position;
                m_vDebrisState[ui].v4Rotation = M_AxisAngleToQuat(M_RandomDirection(), M_Randnum(0.0f, 360.0f));
            }           
            
            m_vDebrisState[ui].v3Velocity = v3Velocity;
            m_vDebrisState[ui].v4RotationVelocity = CVec4f(M_RandomDirection(), M_Randnum(fMinRotationSpeed, fMaxRotationSpeed));
            m_vDebrisState[ui].v3Direction = v3Dir;
            m_vDebrisState[ui].fScale = m_fLastScale;
            m_vDebrisState[ui].fAcceleration = fAcceleration;
            m_vDebrisState[ui].pImbeddedEmitter = pImbeddedEmitter;
        }
    }
}


/*====================
  CDebrisEmitter::UpdateEmbeddedEmitter
  ====================*/
inline
bool    CDebrisEmitter::UpdateEmbeddedEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace, IEmitter *pEmitter, SDebrisState &cDebris)
{
    pEmitter->SetCustomPos(cDebris.v3Position);
    pEmitter->SetCustomAxis(M_QuatToAxis(cDebris.v4Rotation));
    pEmitter->SetCustomScale(cDebris.fScale);

    bool bRet(pEmitter->Update(uiMilliseconds, pfnTrace));

    return bRet;
}


/*====================
  CDebrisEmitter::UpdateSkeleton
  ====================*/
void    CDebrisEmitter::UpdateSkeleton(float fDeltaTime, const CVec3f &v3Acceleration, float fDrag, float fFriction, ParticleTraceFn_t pfnTrace)
{
    uint uiNumBones(m_pSkeleton->GetNumBones());

    m_vBonePose[0].v3Pos = m_vDebrisState[0].v3Position;
    m_vBonePose[0].v4Quat = m_vDebrisState[0].v4Rotation;
    m_vBonePose[0].v3Scale = CVec3f(1.0f, 1.0f, 1.0f);

    if (m_bUseAnim)
    {
        m_pSkeleton->Pose(m_uiLastUpdateTime);
        SBoneXForm *pCurrentPose(m_pSkeleton->GetCurrentPose(0));

        for (uint ui(1); ui < uiNumBones; ++ui)
        {
            SDebrisState &cDebrisState(m_vDebrisState[ui]);
            SBoneXForm &cBonePose(m_vBonePose[ui]);

            cDebrisState.v3Position = pCurrentPose[ui].v3Pos;
            cDebrisState.v4Rotation = pCurrentPose[ui].v4Quat;
            cDebrisState.fScale = (pCurrentPose[ui].v3Scale.x + pCurrentPose[ui].v3Scale.y + pCurrentPose[ui].v3Scale.z) / 3.0f;

            if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
            {
                cDebrisState.v3Position = TransformPoint(cDebrisState.v3Position, m_aLastAxis, m_v3LastPos, m_fLastScale);

                CAxis aAxis(m_aLastAxis * M_QuatToAxis(cDebrisState.v4Rotation));
                cDebrisState.v4Rotation = M_AxisToQuat(aAxis);

                cDebrisState.fScale *= m_fLastScale;

                cBonePose.v3Pos = cDebrisState.v3Position;
                cBonePose.v4Quat = cDebrisState.v4Rotation;
                cBonePose.v3Scale = CVec3f(cDebrisState.fScale);
            }
            else
            {
                cDebrisState.v3Position = TransformPoint(cDebrisState.v3Position, m_aLastAxis, m_v3LastPos, m_fLastScale);

                CAxis aAxis(m_aLastAxis * M_QuatToAxis(cDebrisState.v4Rotation));
                cDebrisState.v4Rotation = M_AxisToQuat(aAxis);

                cDebrisState.fScale *= m_fLastScale;

                cBonePose.v3Pos = pCurrentPose[ui].v3Pos;
                cBonePose.v4Quat = pCurrentPose[ui].v4Quat;
                cBonePose.v3Scale = CVec3f((pCurrentPose[ui].v3Scale.x + pCurrentPose[ui].v3Scale.y + pCurrentPose[ui].v3Scale.z) / 3.0f);
            }
        }

        return;
    }

    for (uint ui(1); ui < uiNumBones; ++ui)
    {
        SDebrisState &cDebrisState(m_vDebrisState[ui]);
        SBoneXForm &cBonePose(m_vBonePose[ui]);

        CVec3f v3Pos(cDebrisState.v3Position);
        CVec3f v3Velocity(cDebrisState.v3Velocity);
        CVec3f v3Dir(cDebrisState.v3Direction);

        float fSpeedSq(v3Velocity.LengthSq());

        if (fSpeedSq > 0.0f)
        {
            float fSpeed(sqrt(fSpeedSq));

            v3Dir = v3Velocity / fSpeed;
            float vDrag(-MIN(fSpeedSq / (cDebrisState.fScale * cDebrisState.fScale) * 0.5f * fDrag, fSpeed / fDeltaTime));

            if (fFriction != 0.0f)
            {
                CVec3f  v3Friction(v3Dir * fFriction * cDebrisState.fScale * fDeltaTime);

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

            v3Pos += v3Velocity * fDeltaTime + (v3Acceleration + (v3Dir * (cDebrisState.fAcceleration + vDrag))) * (0.5f * fDeltaTime * fDeltaTime * cDebrisState.fScale);
            v3Velocity += (v3Acceleration + (v3Dir * (cDebrisState.fAcceleration + vDrag))) * (fDeltaTime * cDebrisState.fScale);
        }
        else
        {
            v3Pos += (v3Acceleration + (v3Dir * cDebrisState.fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime * cDebrisState.fScale);
            v3Velocity += (v3Acceleration + (v3Dir * cDebrisState.fAcceleration)) * (fDeltaTime * cDebrisState.fScale);
        }

        if (pfnTrace)
        {
            CVec3f v3Normal;

            if (pfnTrace(cDebrisState.v3Position, v3Pos, cDebrisState.v3Position, v3Normal))
            {
                cDebrisState.v3Velocity = Reflect(v3Velocity, v3Normal, m_fReflect);
                cDebrisState.v3Velocity *= m_fBounce;

                cDebrisState.v3Direction = Normalize(cDebrisState.v3Velocity);

                cDebrisState.v4RotationVelocity.xyz() = M_RandomDirection();
                cDebrisState.v4RotationVelocity.w *= m_fBounce;
            }
            else
            {
                cDebrisState.v3Velocity = v3Velocity;
                cDebrisState.v3Direction = v3Dir;
            }
        }
        else
        {
            cDebrisState.v3Position = v3Pos;
            cDebrisState.v3Velocity = v3Velocity;
            cDebrisState.v3Direction = v3Dir;
        }

        cDebrisState.v4Rotation = M_MultQuat(cDebrisState.v4Rotation,
            M_AxisAngleToQuat(cDebrisState.v4RotationVelocity.xyz(), cDebrisState.v4RotationVelocity.w * fDeltaTime));

        M_QuatNormalize(cDebrisState.v4Rotation);

        cBonePose.v3Pos = cDebrisState.v3Position;
        cBonePose.v4Quat = cDebrisState.v4Rotation;
        cBonePose.v3Scale = CVec3f(cDebrisState.fScale);
    }
}


/*====================
  CDebrisEmitter::Update
  ====================*/
bool    CDebrisEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CDebrisEmitter::Update");

    int iDeltaTime;

    if (uiMilliseconds == INVALID_TIME)
    {
        uiMilliseconds = m_uiStartTime;
        iDeltaTime = 0;
    }
    else
    {
        if (m_uiPauseBegin)
            ResumeFromPause(uiMilliseconds);

        iDeltaTime = uiMilliseconds - m_uiLastUpdateTime;

        if (iDeltaTime <= 0)
        {
            UpdateNextEmitter(uiMilliseconds, pfnTrace);
            return true;
        }

        m_uiLastUpdateTime = uiMilliseconds;
    }

    // Calculate temporal properties
    float fTime((m_uiLastUpdateTime - m_uiStartTime) * SEC_PER_MS);
    float fLerp;

    if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= m_uiLastUpdateTime && (m_iLife == -1 || m_bLoop))
    {
        // Kill us if we've lived out our entire expire life
        if ((m_iExpireLife != -1 && (uiMilliseconds > m_iExpireLife + m_uiExpireTime)) || m_iExpireLife == 0)
        {
            m_bActive = false;
            return false;
        }
        
        if (m_iExpireLife != -1)
            fLerp = float(m_uiLastUpdateTime - m_uiExpireTime) / m_iExpireLife;
        else
            fLerp = 0.0f;
    }
    else
    {
        if (m_iLife != -1)
            fLerp = float(m_uiLastUpdateTime - m_uiStartTime) / m_iLife;
        else
            fLerp = 0.0f;
    }

    m_fLastLerp = fLerp;
    m_fLastTime = fTime;

    m_bActive = GetVisibility();

    // Kill us if we've lived out our entire life
    if (m_iLife != -1 && (uiMilliseconds > m_iLife + m_uiStartTime))
    {
        if (m_bLoop)
        {
            m_uiStartTime += m_iLife * ((uiMilliseconds - m_uiStartTime) / m_iLife);
        }
        else
        {
            m_bActive = false;
            return false;
        }
    }

    CVec3f v3Pos(GetPosition());
    CAxis aAxis(GetAxis());
    float fScale(GetScale());

    if (m_eDirectionalSpace == DIRSPACE_LOCAL)
    {
        CAxis   aBoneAxis;
        CVec3f  v3BonePos;

        GetBoneAxisPos(uiMilliseconds, m_pOwner, m_sBone, aBoneAxis, v3BonePos);

        v3Pos = TransformPoint(v3BonePos, aAxis, v3Pos, fScale);
        aAxis = aAxis * aBoneAxis;
        m_aLastAxis = aAxis * CAxis(m_tfPitch.Evaluate(fLerp, fTime), m_tfRoll.Evaluate(fLerp, fTime), m_tfYaw.Evaluate(fLerp, fTime));
    }
    else
    {
        v3Pos = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBone), aAxis, v3Pos, fScale);
        m_aLastAxis = CAxis(m_tfPitch.Evaluate(fLerp, fTime), m_tfRoll.Evaluate(fLerp, fTime), m_tfYaw.Evaluate(fLerp, fTime));
    }

    m_v3LastPos = v3Pos;
    m_fLastScale = GetScale() * m_tfScale.Evaluate(fLerp, fTime);

    float   fGravity(m_rfGravity.Evaluate(fLerp, fTime));
    CVec3f v3Acceleration(0.0f, 0.0f, fGravity * -20.0f);

    UpdateSkeleton(MsToSec(uint(iDeltaTime)), v3Acceleration, m_fDrag, m_fFriction, m_bCollide ? pfnTrace : NULL);

    for (uint ui(1), uiNumBones(m_pSkeleton->GetNumBones()); ui < uiNumBones; ++ui)
    {
        SDebrisState &cDebrisState(m_vDebrisState[ui]);

        if (cDebrisState.pImbeddedEmitter != NULL)
        {
            if (!UpdateEmbeddedEmitter(uiMilliseconds, pfnTrace, cDebrisState.pImbeddedEmitter, cDebrisState))
            {
                K2_DELETE(cDebrisState.pImbeddedEmitter);
                cDebrisState.pImbeddedEmitter = NULL;
            }
        }
    }

    if (m_pSkeleton)
        m_pSkeleton->Pose(uiMilliseconds, &m_vBonePose[0]);

    UpdateNextEmitter(uiMilliseconds, pfnTrace);

    m_bbBounds.Clear();

    if (m_hModel != INVALID_RESOURCE)
    {
        m_bbBounds = g_ResourceManager.GetModelBounds(m_hModel);
        m_bbBounds.Transform(m_v3LastPos, m_aLastAxis, m_fLastScale);
    }

    return true;
}


/*====================
  CDebrisEmitter::GetNumEntities
  ====================*/
uint    CDebrisEmitter::GetNumEntities()
{
    if (m_bActive)
        return 1;
    else
        return 0;
}


/*====================
  CDebrisEmitter::GetEntity
  ====================*/
bool    CDebrisEmitter::GetEntity(uint uiIndex, CSceneEntity &outEntity)
{
    if (uiIndex != 0)
        return false;

    float fTime((m_uiLastUpdateTime - m_uiStartTime) * SEC_PER_MS);
    float fLerp;

    if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= m_uiLastUpdateTime && (m_iLife == -1 || m_bLoop))
    {
        if (m_iExpireLife != -1)
            fLerp = float(m_uiLastUpdateTime - m_uiExpireTime) / m_iExpireLife;
        else
            fLerp = 0.0f;
    }
    else
    {
        if (m_iLife != -1)
            fLerp = float(m_uiLastUpdateTime - m_uiStartTime) / m_iLife;
        else
            fLerp = 0.0f;
    }

    outEntity.color = CVec4f(m_tv3Color.Evaluate(fLerp, fTime), m_tfAlpha.Evaluate(fLerp, fTime));

    outEntity.s1 = m_tfParam0.Evaluate(fLerp, fTime);
    outEntity.t1 = m_tfParam1.Evaluate(fLerp, fTime);
    outEntity.s2 = m_tfParam2.Evaluate(fLerp, fTime);
    outEntity.t2 = m_tfParam3.Evaluate(fLerp, fTime);
    
    switch (m_pParticleSystem->GetSpace())
    {
    case WORLD_SPACE:
        {
            outEntity.SetPosition(V3_ZERO);
            outEntity.axis = AXIS_IDENTITY;
            outEntity.scale = 1.0f;
            outEntity.bounds = m_bbBounds;
        } break;
    case BONE_SPACE:
    case ENTITY_SPACE:
        {
            outEntity.SetPosition(m_v3LastPos);
            outEntity.axis = m_aLastAxis;
            outEntity.scale = m_fLastScale;
            outEntity.bounds = m_bbBounds;

            const CVec3f    &v3Pos(m_pParticleSystem->GetSourcePosition());
            const CAxis     &aAxis(m_pParticleSystem->GetSourceAxis());
            float           fScale(m_pParticleSystem->GetSourceScale());

            outEntity.scale *= fScale;
            outEntity.SetPosition(TransformPoint(outEntity.GetPosition(), aAxis, v3Pos, fScale));

            outEntity.axis = aAxis * outEntity.axis;
            outEntity.bounds.Transform(v3Pos, aAxis, fScale);
        } break;
    }
    
    outEntity.skeleton = m_pSkeleton;
    outEntity.hRes = m_hModel;
    outEntity.hSkin = m_hSkin;
        
    outEntity.objtype = OBJTYPE_MODEL;
    outEntity.flags = SCENEENT_USE_AXIS | SCENEENT_SOLID_COLOR | SCENEENT_USE_BOUNDS;
    

    if (m_hMaterial != INVALID_RESOURCE)
    {
        outEntity.flags |= SCENEENT_SINGLE_MATERIAL;
        outEntity.hSkin = m_hMaterial;
    }

    return true;
}


/*====================
  CDebrisEmitter::GetNumEmitters
  ====================*/
uint    CDebrisEmitter::GetNumEmitters()
{
    return uint(m_vDebrisState.size());
}


/*====================
  CDebrisEmitter::GetEmitter
  ====================*/
IEmitter*   CDebrisEmitter::GetEmitter(uint uiIndex)
{
    return m_vDebrisState[uiIndex].pImbeddedEmitter;
}
