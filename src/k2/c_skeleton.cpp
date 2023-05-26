// (C)2005 S2 Games
// c_skeleton.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_skeleton.h"
#include "c_k2model.h"
#include "c_anim.h"
#include "c_bone.h"
#include "c_mesh.h"
#include "c_model.h"
#include "c_clip.h"
#include "i_resource.h"
#include "c_skeletonbonepool.h"
#include "c_resourcemanager.h"

#include "../public/modelclip_t.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOL(skel_interpolate, true);
CVAR_BOOL(skel_blendAnims, true);
CVAR_BOOL(skel_noPose, false);
CVAR_BOOL(skel_debug, false);
//=============================================================================

/*====================
  CSkeleton::CSkeleton
  ====================*/
CSkeleton::CSkeleton() :
m_pBoneStates(nullptr),
m_pTempBoneStates(nullptr),
m_uiNumBones(0),
m_bIsValid(false),

m_bIsCharacter(false),
m_uiPoseTime(INVALID_TIME),
m_uiPrevPoseTime(INVALID_TIME),
m_uiSavedPoseTime(INVALID_TIME),
m_fPrevPitch(0.0f),
m_fPitch(0.0f),
m_fPrevYaw(0.0f),
m_fYaw(0.0f),

m_sDefaultAnimName(_T("idle")),

m_hModel(INVALID_RESOURCE),
m_pModel(nullptr)
{
    for (int i = 0; i < NUM_ANIM_CHANNELS; ++i)
    {
        m_pAnim[i] = nullptr;
        m_fSpeed[i] = 1.0f;
        m_uiForceLength[i] = 0;
        m_bPassiveAnim[i] = true;
        m_pCurrentPose[i] = nullptr;
        m_pSavedPose[i] = nullptr;
        m_pTempPose[i] = nullptr;
        m_iParentChannel[i] = -1;
        m_bValidPose[i] = false;
        m_uiStartTime[i] = INVALID_TIME;
        m_fSpeed[i] = 1.0f;
        m_uiForceLength[i] = 0;
        m_iOffsetTime[i] = 0;
        m_aAnimRequest[i].sAnim.clear();
        m_aAnimRequest[i].uiTime = INVALID_TIME;
    }

    Clear();
}


/*====================
  CSkeleton::~CSkeleton
  ====================*/
CSkeleton::~CSkeleton()
{
    // Give the bone data back to the pool
    SSkeletonBoneData cBoneData;
    cBoneData.uiNumBones = m_uiNumBones;
    
    cBoneData.pBoneStates = m_pBoneStates;
    cBoneData.pTempBoneStates = m_pTempBoneStates;

    for (int i = 0; i < NUM_ANIM_CHANNELS; ++i)
    {
        cBoneData.pCurrentPose[i] = m_pCurrentPose[i];
        cBoneData.pSavedPose[i] = m_pSavedPose[i];
        cBoneData.pTempPose[i] = m_pTempPose[i];
    }

    SkeletonBonePool.Deallocate(cBoneData);
}


/*====================
  CSkeleton::SetBoneState
  ====================*/
void    CSkeleton::SetBoneState(uint uiBone, int iState)
{
    if (uiBone == INVALID_BONE)
        return;

    m_pBoneStates[uiBone].poseState = iState;

    CBone *pBone(m_pModel->GetBone(uiBone));
    uint uiNumChildren(pBone->NumChildren());
    for (uint n(0); n < uiNumChildren; ++n)
    {
        uint uiChild(pBone->GetChildIndex(n));
        SetBoneState(uiChild, iState);
    }
}


/*====================
  CSkeleton::SetBoneState
  ====================*/
void    CSkeleton::SetBoneState(const tstring &sBoneName, int iState)
{
    uint uiBone(m_pModel->GetBoneIndex(sBoneName));
    if (uiBone == INVALID_BONE)
        return;

    SetBoneState(uiBone, iState);
}


/*====================
  CSkeleton::SetBoneState
  ====================*/
void    CSkeleton::SetBoneState(int iState)
{
    for (uint n(0); n < m_uiNumBones; ++n)
        m_pBoneStates[n].poseState = iState;
}



/*====================
  CSkeleton::Clear
  ====================*/
void    CSkeleton::Clear()
{
    // Give the bone data back to the pool
    SSkeletonBoneData cBoneData;
    cBoneData.uiNumBones = m_uiNumBones;
    cBoneData.pBoneStates = m_pBoneStates;
    cBoneData.pTempBoneStates = m_pTempBoneStates;

    for (int i = 0; i < NUM_ANIM_CHANNELS; ++i)
    {
        cBoneData.pCurrentPose[i] = m_pCurrentPose[i];
        cBoneData.pSavedPose[i] = m_pSavedPose[i];
        cBoneData.pTempPose[i] = m_pTempPose[i];
    }

    SkeletonBonePool.Deallocate(cBoneData);

    m_uiNumBones = 0;
    m_pBoneStates = nullptr;
    m_pTempBoneStates = nullptr;

    for (int i = 0; i < NUM_ANIM_CHANNELS; ++i)
    {
        m_pCurrentPose[i] = nullptr;
        m_pSavedPose[i] = nullptr;
        m_pTempPose[i] = nullptr;
    }

    m_pModel = nullptr;
    m_bIsValid = false;

    m_bIsCharacter = false;
    m_uiPoseTime = INVALID_TIME;
    m_uiPrevPoseTime = INVALID_TIME;
    m_uiSavedPoseTime = INVALID_TIME;

    m_fYaw = 0.0f;
    m_fPitch = 0.0f;
    m_fPrevYaw = 0.0f;
    m_fPrevPitch = 0.0f;

    for (int n(0); n < NUM_ANIM_CHANNELS; ++n)
    {
        m_pAnim[n] = nullptr;
        m_uiBlendTime[n] = 0;
        m_uiBlendStartTime[n] = 0;
        m_uiTime[n] = 0;
        m_iParentChannel[n] = n;
        m_iLastAnimTime[n] = 0;
        m_bPassiveAnim[n] = true;
        m_bValidPose[n] = false;
        m_uiStartTime[n] = INVALID_TIME;
        m_fSpeed[n] = 1.0f;
        m_uiForceLength[n] = 0;
        m_iOffsetTime[n] = 0;

        m_aAnimRequest[n].sAnim.clear();
        m_aAnimRequest[n].uiTime = INVALID_TIME;
    }

    ClearEvents();
}


/*====================
  CSkeleton::GetBoneXform
  ====================*/
void    CSkeleton::GetBoneXform(SBoneMotion *pMotion, int iLoFrame, int iHiFrame, float fLerp, SBoneXForm &cBone)
{   
    if (skel_interpolate)
    {
        // Translation
        cBone.v3Pos.x = pMotion->keys_x.num_keys == 1 ? pMotion->keys_x.keys[0] : LERP(fLerp, pMotion->keys_x.keys[iLoFrame], pMotion->keys_x.keys[iHiFrame]);
        cBone.v3Pos.y = pMotion->keys_y.num_keys == 1 ? pMotion->keys_y.keys[0] : LERP(fLerp, pMotion->keys_y.keys[iLoFrame], pMotion->keys_y.keys[iHiFrame]);
        cBone.v3Pos.z = pMotion->keys_z.num_keys == 1 ? pMotion->keys_z.keys[0] : LERP(fLerp, pMotion->keys_z.keys[iLoFrame], pMotion->keys_z.keys[iHiFrame]);

        // Rotation
        if (pMotion->keys_quat.num_keys == 1)
        {
            M_CopyVec4(pMotion->keys_quat.keys[0], vec4_cast(cBone.v4Quat));
        }
        else
        {
            M_LerpQuat(fLerp,
                pMotion->keys_quat.keys[iLoFrame % pMotion->keys_quat.num_keys],
                pMotion->keys_quat.keys[iHiFrame % pMotion->keys_quat.num_keys],
                vec4_cast(cBone.v4Quat));
        }

        // Scale
        cBone.v3Scale.x = pMotion->keys_scalex.num_keys == 1 ? pMotion->keys_scalex.keys[0] : LERP(fLerp, pMotion->keys_scalex.keys[iLoFrame], pMotion->keys_scalex.keys[iHiFrame]);
        cBone.v3Scale.y = pMotion->keys_scaley.num_keys == 1 ? pMotion->keys_scaley.keys[0] : LERP(fLerp, pMotion->keys_scaley.keys[iLoFrame], pMotion->keys_scaley.keys[iHiFrame]);
        cBone.v3Scale.z = pMotion->keys_scalez.num_keys == 1 ? pMotion->keys_scalez.keys[0] : LERP(fLerp, pMotion->keys_scalez.keys[iLoFrame], pMotion->keys_scalez.keys[iHiFrame]);
    }
    else
    {
        // Translation
        cBone.v3Pos.x = pMotion->keys_x.keys[iLoFrame % pMotion->keys_x.num_keys];
        cBone.v3Pos.y = pMotion->keys_y.keys[iLoFrame % pMotion->keys_y.num_keys];
        cBone.v3Pos.z = pMotion->keys_z.keys[iLoFrame % pMotion->keys_z.num_keys];
        M_CopyVec4(pMotion->keys_quat.keys[iLoFrame % pMotion->keys_quat.num_keys], vec4_cast(cBone.v4Quat));

        // Scale
        cBone.v3Scale.x = pMotion->keys_scalex.keys[iLoFrame % pMotion->keys_scalex.num_keys];
        cBone.v3Scale.y = pMotion->keys_scaley.keys[iLoFrame % pMotion->keys_scaley.num_keys];
        cBone.v3Scale.z = pMotion->keys_scalez.keys[iLoFrame % pMotion->keys_scalez.num_keys];
    }
}


/*====================
  CSkeleton::PoseBones
  ====================*/
void    CSkeleton::PoseBones(SBoneMotion **ppMotions, int iLoFrame, int iHiFrame, float fLerp, int iChannel)
{
    for (uint uiBone(0); uiBone < m_uiNumBones; ++uiBone, ++ppMotions)
    {
        SBoneState  *pBoneState(GetBoneState(uiBone));

        if (pBoneState->poseState & POSE_MASKED)
            continue;

        SBoneMotion *pMotion(*ppMotions);

        if (!pMotion)
        {
            m_pCurrentPose[iChannel][uiBone].v3Pos = CVec3f(0.0f, 0.0f, 0.0f);
            m_pCurrentPose[iChannel][uiBone].v4Quat = CVec4f(0.0f, 0.0f, 0.0f, 1.0f);
            m_pCurrentPose[iChannel][uiBone].v3Scale = CVec3f(1.0f, 1.0f, 1.0f);
            
            pBoneState->visibility = 255;
            continue;
        }

        GetBoneXform(pMotion, iLoFrame, iHiFrame, fLerp, m_pCurrentPose[iChannel][uiBone]);
        
        // Update bone sisibility
        if (pMotion->keys_visibility.num_keys == 0)
            pBoneState->visibility = 255;
        else
            pBoneState->visibility = pMotion->keys_visibility.num_keys == 1 ?
                pMotion->keys_visibility.keys[0] : pMotion->keys_visibility.keys[iLoFrame % pMotion->keys_visibility.num_keys];
    }
}


/*====================
  CSkeleton::UpdateBones

  update skeleton local tms
  ====================*/
void    CSkeleton::UpdateBones(int iChannel)
{
    for (uint uiBone(0); uiBone < m_uiNumBones; ++uiBone)
    {
        SBoneState  *pBoneState(GetBoneState(uiBone));

        if (pBoneState->poseState & POSE_MASKED)
            continue;
        
        SBoneXForm &cCurrentPose(m_pCurrentPose[iChannel][uiBone]);

        // Translation
        CVec3_cast(pBoneState->tm.pos) = cCurrentPose.v3Pos;

        // Rotation
        M_QuatToAxis(vec4_cast(cCurrentPose.v4Quat), pBoneState->tm.axis);

        // Scale
        if (cCurrentPose.v3Scale[X] != 1.0f)
            M_MultVec3(pBoneState->tm.axis[X], cCurrentPose.v3Scale[X], pBoneState->tm.axis[X]);
        if (cCurrentPose.v3Scale[Y] != 1.0f)
            M_MultVec3(pBoneState->tm.axis[Y], cCurrentPose.v3Scale[Y], pBoneState->tm.axis[Y]);
        if (cCurrentPose.v3Scale[Z] != 1.0f)
            M_MultVec3(pBoneState->tm.axis[Z], cCurrentPose.v3Scale[Z], pBoneState->tm.axis[Z]);
    }
}


/*====================
  CSkeleton::UpdateBonesLerp

  update skeleton local tms
  ====================*/
void    CSkeleton::UpdateBonesLerp(float fLerp, int iChannel)
{
    for (uint uiBone(0); uiBone < m_uiNumBones; ++uiBone)
    {
        SBoneState  *pBoneState(GetBoneState(uiBone));

        if (pBoneState->poseState & POSE_MASKED)
            continue;
        
        SBoneXForm &cSavedPose(m_pSavedPose[iChannel][uiBone]);
        SBoneXForm &cCurrentPose(m_pCurrentPose[iChannel][uiBone]);

        // Translation
        CVec3_cast(pBoneState->tm.pos) = LERP(fLerp, cSavedPose.v3Pos, cCurrentPose.v3Pos);

        // Rotation
        vec4_t quat;
        M_LerpQuat(fLerp, vec4_cast(cSavedPose.v4Quat), vec4_cast(cCurrentPose.v4Quat), vec4_cast(quat));
        M_QuatToAxis(quat, pBoneState->tm.axis);

        // Scale
        M_MultVec3(pBoneState->tm.axis[X], LERP(fLerp, cSavedPose.v3Scale[X], cCurrentPose.v3Scale[X]), pBoneState->tm.axis[X]);
        M_MultVec3(pBoneState->tm.axis[Y], LERP(fLerp, cSavedPose.v3Scale[Y], cCurrentPose.v3Scale[Y]), pBoneState->tm.axis[Y]);
        M_MultVec3(pBoneState->tm.axis[Z], LERP(fLerp, cSavedPose.v3Scale[Z], cCurrentPose.v3Scale[Z]), pBoneState->tm.axis[Z]);
    }
}



/*====================
  CSkeleton::BlendBones

  Update the current skeleton quats to most recent blend so we can save the values
  to be used in a new blend.  This only happens during a StartAnim call.
  ====================*/
void    CSkeleton::BlendBones(float fLerp, int iChannel)
{
    for (uint uiBone(0); uiBone < m_uiNumBones; ++uiBone)
    {
        SBoneState  *pBoneState(GetBoneState(uiBone));

        if (pBoneState->poseState & POSE_MASKED)
            continue;
        
        SBoneXForm &cSavedPose(m_pSavedPose[iChannel][uiBone]);
        SBoneXForm &cCurrentPose(m_pCurrentPose[iChannel][uiBone]);

        cCurrentPose.v3Pos = LERP(fLerp, cSavedPose.v3Pos, cCurrentPose.v3Pos);
        M_LerpQuat(fLerp, vec4_cast(cSavedPose.v4Quat), vec4_cast(cCurrentPose.v4Quat), vec4_cast(cCurrentPose.v4Quat));
        cCurrentPose.v3Scale = LERP(fLerp, cSavedPose.v3Scale, cCurrentPose.v3Scale);
    }
}


/*====================
  CSkeleton::SetAnim
  ====================*/
void    CSkeleton::SetAnim(const tstring &sAnim, uint uiTime, int iBlendTime, int iChannel, float fSpeed, uint uiForceLength, bool bStartEvent, bool bEndEvent)
{
    if (!m_pModel)
        return;

    m_bPassiveAnim[iChannel] = (sAnim == m_sDefaultAnimName);

    CAnim *pNewAnim(m_pModel->GetAnim(sAnim));

    if (!pNewAnim)
        return;

    CAnim *pOldAnim(m_pAnim[iChannel]);

    // Add end events
    if (pOldAnim && bEndEvent)
    {
        const tsvector &vEndEvents(pOldAnim->GetEndEvents());
        for (tsvector::const_iterator it(vEndEvents.begin()); it != vEndEvents.end(); ++it)
            AddEvent(*it, 0);
    }

    m_pAnim[iChannel] = pNewAnim;

    // Calculate and save the current pose for blending
    if (iChannel == 1)
    {
        SetBoneState(POSE_MASKED);
        SetBoneState(_T("Bip01 Spine"), 0);
    }

    uint uiBlendTime(m_fSpeed[iChannel] != 0.0f ? INT_ROUND(m_uiBlendTime[iChannel] / m_fSpeed[iChannel]) : 0);

    if (uiBlendTime && uiTime - m_uiBlendStartTime[iChannel] < uiBlendTime && skel_blendAnims)
    {
        float fBlend(M_SmoothStepN((uiTime - m_uiBlendStartTime[iChannel]) / float(uiBlendTime)));
        BlendBones(fBlend, iChannel);
    }

    if (iChannel == 1)
        SetBoneState(0);

    if (pOldAnim && m_bValidPose[iChannel])
    {
        // Start a new blend
        MemManager.Copy(m_pSavedPose[iChannel], m_pCurrentPose[iChannel], sizeof(SBoneXForm) * m_pModel->GetNumBones());

        if (iBlendTime == -1)
            iBlendTime = pNewAnim->GetBlendTime();

        m_uiBlendTime[iChannel] = iBlendTime;
    }
    else
    {
        m_uiBlendTime[iChannel] = 0;
    }

    m_uiBlendStartTime[iChannel] = uiTime;
    m_uiStartTime[iChannel] = uiTime;
    m_fSpeed[iChannel] = fSpeed;
    m_uiForceLength[iChannel] = uiForceLength;
    m_iOffsetTime[iChannel] = (M_Randnum(m_pAnim[iChannel]->GetMinStartFrame(), m_pAnim[iChannel]->GetMaxStartFrame()) - m_pAnim[iChannel]->GetStartFrame()) * m_pAnim[iChannel]->GetMSperFrame();
    m_iLastAnimTime[iChannel] = m_iOffsetTime[iChannel];
    m_uiPoseTime = INVALID_TIME;

    // Add start events
    if (pNewAnim && bStartEvent)
    {
        const tsvector &vStartEvents(pNewAnim->GetStartEvents());
        for (tsvector::const_iterator it(vStartEvents.begin()); it != vStartEvents.end(); ++it)
            AddEvent(*it, 0);
    }
}


/*====================
  CSkeleton::StartAnim
  ====================*/
int     CSkeleton::StartAnim(const tstring &sAnim, uint uiTime, int iChannel, int iBlendTime, float fSpeed, uint uiForceLength)
{
    PROFILE("CSkeleton::StartAnim");

    if (!m_pModel)
        return -1;

    assert(iChannel >= 0 && iChannel < NUM_ANIM_CHANNELS); 

    //Console.Dev << "CSkeleton::StartAnim() - " << sAnim << _T(" ") << uiTime << _T(" ") << iChannel << newl;

    // Update old anims to the current time
    if (m_uiPoseTime != uiTime && HasValidAnims())
        Pose(uiTime);

    SetAnim(sAnim, uiTime, iBlendTime, iChannel, fSpeed, uiForceLength, true, true);
    m_iParentChannel[iChannel] = iChannel;

    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
    {
        CAnim *pDefaultAnim = m_pModel->GetAnim(m_sDefaultAnimName);

        if (iChannel != i && (m_bPassiveAnim[i] || m_iParentChannel[i] == iChannel) && !(pDefaultAnim && pDefaultAnim->GetLock()))
        {
            SetAnim(sAnim, uiTime, iBlendTime, i, fSpeed, uiForceLength, false, false);
            m_iParentChannel[i] = iChannel;
        }
    }
    return -1;
}


/*====================
  CSkeleton::StopAnim
  ====================*/
void    CSkeleton::StopAnim(int iChannel, uint uiTime)
{
    PROFILE("CSkeleton::StartAnim");

    if (iChannel < 0 || iChannel > NUM_ANIM_CHANNELS)
    {
        Console.Warn << _T("CSkeleton::StopAnim() - Invalid channel: ") << iChannel << newl;
        return;
    }

    CAnim *pDefaultAnim(m_pModel->GetAnim(m_sDefaultAnimName));

    // If possible, synch with channel zero, otherwise fall
    // back to the current default anim
    if (iChannel != 0 && m_pAnim[0] != nullptr && !(pDefaultAnim && pDefaultAnim->GetLock()))
    {
        if (m_iParentChannel[0] == iChannel)
            StopAnim(0, uiTime);

        SetAnim(m_pAnim[0]->GetName(), uiTime, -1, iChannel, m_fSpeed[0], m_uiForceLength[0], false, m_iParentChannel[iChannel] != iChannel);
        m_uiStartTime[iChannel] = m_uiStartTime[0];
        m_fSpeed[iChannel] = m_fSpeed[0];
        m_uiForceLength[iChannel] = m_uiForceLength[0];
        m_iOffsetTime[iChannel] = m_iOffsetTime[0];
        m_iParentChannel[iChannel] = 0;
    }
    else
    {
        SetAnim(m_sDefaultAnimName, uiTime, -1, iChannel, 1.0f, 0, iChannel == 0, iChannel == 0);
        m_iParentChannel[iChannel] = iChannel;
    }
}


/*====================
  CSkeleton::HasAnim
  ====================*/
bool    CSkeleton::HasAnim(const tstring &sAnim)
{
    if (!m_pModel)
        return false;

    return (m_pModel->GetAnim(sAnim) != nullptr);
}


/*====================
  CSkeleton::ProcessAnimRequests
  ====================*/
void    CSkeleton::ProcessAnimRequests(uint uiTime)
{
    // Process StopAnim's
    for (int n(0); n < NUM_ANIM_CHANNELS; ++n)
    {
        if (m_aAnimRequest[n].uiTime == INVALID_TIME)
            continue;

        if (m_aAnimRequest[n].sAnim.empty())
        {
            StopAnim(n, m_aAnimRequest[n].uiTime);

            m_aAnimRequest[n].sAnim.clear();
            m_aAnimRequest[n].uiTime = INVALID_TIME;
        }
    }

    // Process StartAnim's
    for (int n(NUM_ANIM_CHANNELS - 1); n >= 0; --n)
    {
        if (m_aAnimRequest[n].uiTime == INVALID_TIME)
            continue;

        if (!m_aAnimRequest[n].sAnim.empty())
        {
            StartAnim(m_aAnimRequest[n].sAnim, m_aAnimRequest[n].uiTime, n, m_aAnimRequest[n].iBlendTime, m_aAnimRequest[n].fSpeed, m_aAnimRequest[n].uiForceLength);

            m_aAnimRequest[n].sAnim.clear();
            m_aAnimRequest[n].uiTime = INVALID_TIME;
        }
    }
}


/*====================
  CSkeleton::PoseStandard
  ====================*/
void    CSkeleton::PoseStandard(uint uiTime)
{
    PROFILE("CSkeleton::PoseStandard");

    m_bIsCharacter = false;
    m_fPrevPitch = 0.0f;
    m_fPrevYaw = 0.0f;
    m_fPitch = 0.0f;
    m_fYaw = 0.0f;

    if (!m_pModel)
        return;

    if (!CheckAnims(uiTime))
        return;

    PoseSkeleton(uiTime, 0);
    CalcWorldTransforms();

    if (m_vEventCmds.size() >= 256)
        m_vEventCmds.clear();
}


/*====================
  CSkeleton::Pose
  ====================*/
void    CSkeleton::Pose(uint uiTime, float fParam1, float fParam2)
{
    if (m_pModel == nullptr)
        return;

    m_uiPrevPoseTime = m_uiPoseTime;
    m_uiPoseTime = uiTime;

    ProcessAnimRequests(uiTime);

    switch(m_pModel->GetPoseType())
    {
    default:
    case POSE_STANDARD:
        PoseStandard(uiTime);
        break;

    case POSE_CHARACTER:
        PoseCharacter(uiTime, fParam1, fParam2);
        break;

    case POSE_VEHICLE:
        PoseVehicle(uiTime, fParam1);
        break;

    case POSE_GADGET:
        PoseGadget(uiTime, fParam1, fParam2);
        break;
    }
}


/*====================
  CSkeleton::PoseCharacter

  Specialized character version of the Pose functions
  ====================*/
void    CSkeleton::PoseCharacter(uint uiTime, float fPitch, float fYaw)
{
    PROFILE("CSkeleton::PoseCharacter");

    m_bIsCharacter = true;
    m_fPrevPitch = m_fPitch;
    m_fPrevYaw = m_fYaw;
    m_fPitch = fPitch;
    m_fYaw = fYaw;
    m_uiSavedPoseTime = uiTime;

    if (!m_pModel)
        return;

    if (!CheckAnims(uiTime))
        return;

    uint uiBip01Spine(m_pModel->GetBoneIndex(_T("Bip01 Spine")));

    // *** Channel 0 ***
    // Mask everything above the spine
    SetBoneState(0);
    SetBoneState(uiBip01Spine, POSE_MASKED);

    PoseSkeleton(uiTime, 0);

    // *** Channel 1 ***
    // Mask everything below the spine so only the upper
    // body gets posed
    SetBoneState(POSE_MASKED);
    SetBoneState(uiBip01Spine, 0);

    PoseSkeleton(uiTime, 1);

    if (fPitch > 180.0f)
        fPitch -= 360.0f;
    if (fPitch < -180.0f)
        fPitch += 360.0f;

    if (fYaw > 180.0f)
        fYaw -= 360.0f;
    if (fYaw < -180.0f)
        fYaw += 360.0f;

    // Head movements
    // HACK: For some reason pitch and yaw values are swapped in the biped
    // Biped orients X towards children
    RotateBone(_T("Bip01 Head"), -fPitch / 4.0f, 0, fYaw * 0.25f, true);
    RotateBone(_T("Bip01 Spine"), -fPitch / 6.0f, 0, fYaw * 0.75f, true);

    // Get the final world coordinate transformation, then
    // unmask the legs
    SetBoneState(0);

    CalcWorldTransforms();

    if (m_vEventCmds.size() >= 256)
        m_vEventCmds.clear();
}


/*====================
  CSkeleton::PoseVehicle

  Specialized character version of the Pose functions
  ====================*/
void    CSkeleton::PoseVehicle(uint uiTime, uint uiWheelTime)
{
    PROFILE("CSkeleton::PoseVehicle");

    m_bIsCharacter = false;
    m_fPrevPitch = 0.0f;
    m_fPrevYaw = 0.0f;
    m_fPitch = 0.0f;
    m_fYaw = 0.0f;

    // Make the posing happier
    m_uiStartTime[0] = 0;
    m_uiBlendTime[0] = 0;
    m_uiBlendStartTime[0] = 0;
    m_iOffsetTime[0] = 0;

    if (!m_pModel)
        return;

    if (!CheckAnims(uiTime))
        return;

    // *** Channel 0 ***
    // Mask everything above the spine
    SetBoneState(0);
    SetBoneState(_T("Bip01 Spine"), POSE_MASKED);

    PoseSkeleton(uiWheelTime, 0);

    // *** Channel 1 ***
    // Mask everything below the spine so only the upper
    // body gets posed
    SetBoneState(POSE_MASKED);
    SetBoneState(_T("Bip01 Spine"), 0);

    PoseSkeleton(uiTime, 1);

    // Get the final world coordinate transformation, then
    // unmask the legs
    SetBoneState(0);

    CalcWorldTransforms();

    if (m_vEventCmds.size() >= 256)
        m_vEventCmds.clear();
}


/*====================
  CSkeleton::PoseGadget

  Specialized character version of the Pose functions
  ====================*/
void    CSkeleton::PoseGadget(uint uiTime, float fPitch, float fYaw)
{
    PROFILE("CSkeleton::PoseGadget");

    m_bIsCharacter = true;
    m_fPrevPitch = m_fPitch;
    m_fPrevYaw = m_fYaw;
    m_fPitch = fPitch;
    m_fYaw = fYaw;

    if (!m_pModel)
        return;

    if (!CheckAnims(uiTime))
        return;

    PoseSkeleton(uiTime, 0);

    if (fPitch > 180.0f)
        fPitch -= 360.0f;
    if (fPitch < -180.0f)
        fPitch += 360.0f;

    if (fYaw > 180.0f)
        fYaw -= 360.0f;
    if (fYaw < -180.0f)
        fYaw += 360.0f;

    RotateBone(_T("gun_pivot"), fYaw, 0, -fPitch, true);

    CalcWorldTransforms();

    if (m_vEventCmds.size() >= 256)
        m_vEventCmds.clear();
}


/*====================
  CSkeleton::PoseLite
  ====================*/
void    CSkeleton::PoseLite(uint uiTime)
{
    if (m_pModel == nullptr)
        return;

    ProcessAnimRequests(uiTime);

    switch (m_pModel->GetPoseType())
    {
    default:
    case POSE_STANDARD:
    case POSE_GADGET:
        PoseSkeletonLite(uiTime, 0);
        break;

    case POSE_CHARACTER:
    case POSE_VEHICLE:
        PoseSkeletonLite(uiTime, 0);
        PoseSkeletonLite(uiTime, 1);
        break;
    }
}


/*====================
  CSkeleton::CheckAnims
  ====================*/
bool    CSkeleton::CheckAnims(uint uiTime)
{
    for (int iChannel(0); iChannel < NUM_ANIM_CHANNELS; ++iChannel)
    {
        if (m_pAnim[iChannel] == nullptr)
            StartAnim(m_sDefaultAnimName, uiTime, iChannel);
        if (m_pAnim[iChannel] == nullptr)
        {
            Invalidate();
            return false;
        }
        //else
        //  m_bPassiveAnim[iChannel] = (m_pAnim[iChannel]->GetName() == m_sDefaultAnimName);
    }
    return true;
}


/*====================
  CSkeleton::HasValidAnims
  ====================*/
bool    CSkeleton::HasValidAnims()
{
    for (int iChannel(0); iChannel < NUM_ANIM_CHANNELS; ++iChannel)
    {
        if (m_pAnim[iChannel] == nullptr)
            return false;
    }
    return true;
}


/*====================
  CSkeleton::PoseSkeleton
  ====================*/
void    CSkeleton::PoseSkeleton(uint uiTime, int iChannel)
{
    PROFILE("PoseSkeleton");

    if (skel_noPose)
        return;

    // Make sure we have an anim to pose
    if (!CheckAnims(uiTime))
        return;

    if (uiTime < m_uiStartTime[iChannel])
        uiTime = m_uiStartTime[iChannel];

    // Determine hi and lo frames and interpolation amount
    int     iAnimTime(m_fSpeed[iChannel] == 0.0f ? m_iOffsetTime[iChannel] : INT_FLOOR((uiTime - m_uiStartTime[iChannel] + m_iOffsetTime[iChannel]) * m_fSpeed[iChannel]));
    int     iLoFrame, iHiFrame;
    float   fLerp;

    if (iAnimTime - m_iLastAnimTime[iChannel] > 0xffff)
    {
        if (skel_debug)
            Console.Warn << _T("CSkeleton::PoseSkeleton delta AnimTime == ") << iAnimTime - m_iLastAnimTime[iChannel] << newl;

        if (iAnimTime < 0xffff)
            m_iLastAnimTime[iChannel] = 0;
        else
            m_iLastAnimTime[iChannel] = iAnimTime - 0xffff;
    }

    bool bEnd = false;
    if (!m_pAnim[iChannel]->ComputeAnimFrame(iAnimTime, iLoFrame, iHiFrame, fLerp, m_uiForceLength[iChannel]) &&
        !m_bPassiveAnim[iChannel])
        bEnd = true;

    // Check for events on this frame
    if ((m_pModel->GetPoseType() != POSE_VEHICLE && (iChannel == 0 || m_pAnim[iChannel] != m_pAnim[0]))
        || (m_pModel->GetPoseType() == POSE_VEHICLE && (iChannel == 1 || m_pAnim[iChannel] != m_pAnim[1])))
        m_pAnim[iChannel]->CheckEvents(this, m_iLastAnimTime[iChannel], iAnimTime, iChannel);
    m_iLastAnimTime[iChannel] = iAnimTime;

    // Get the clip
    CClip *pClip = g_ResourceManager.GetClip(m_pAnim[iChannel]->GetClip());
    if (pClip)
    {
        int iNumFrames = pClip->GetNumFrames();
        if (iNumFrames != 0)
        {
            // Make sure we don't get an access violation trying to access bad frames
            iLoFrame = (int)iLoFrame % iNumFrames;
            iHiFrame = (int)iHiFrame % iNumFrames;
        }
    }

    {
        PROFILE("PoseBones");

        // iLoFrame and iHiFrame should be valid array indexes at this point
        PoseBones(m_pAnim[iChannel]->GetMotions(), iLoFrame, iHiFrame, fLerp, iChannel);
    }

    {
        PROFILE("UpdateBones");

        uint uiBlendTime(m_fSpeed[iChannel] != 0.0f ? INT_ROUND(m_uiBlendTime[iChannel] / m_fSpeed[iChannel]) : 0);

        // Blending
        if (uiBlendTime && uiTime - m_uiBlendStartTime[iChannel] < uiBlendTime && skel_blendAnims)
        {
            float fBlend(M_SmoothStepN((uiTime - m_uiBlendStartTime[iChannel]) / float(uiBlendTime)));
            UpdateBonesLerp(fBlend, iChannel);
        }
        else
            UpdateBones(iChannel);
    }

    Validate();

    m_uiTime[iChannel] = uiTime;

    if (bEnd)
        StopAnim(iChannel, uiTime);

    m_bValidPose[iChannel] = true;
}


/*====================
  CSkeleton::PoseSkeletonLite
  ====================*/
void    CSkeleton::PoseSkeletonLite(uint uiTime, int iChannel)
{
    // Make sure we have an anim to pose
    if (!CheckAnims(uiTime))
        return;

    if (uiTime < m_uiStartTime[iChannel])
        uiTime = m_uiStartTime[iChannel];

    // Determine hi and lo frames and interpolation amount
    int     iAnimTime(m_fSpeed[iChannel] == 0.0f ? m_iOffsetTime[iChannel] : INT_FLOOR((uiTime - m_uiStartTime[iChannel] + m_iOffsetTime[iChannel]) * m_fSpeed[iChannel]));
    int     iLoFrame, iHiFrame;
    float   fLerp;

    if (iAnimTime - m_iLastAnimTime[iChannel] > 0xffff)
    {
        if (skel_debug)
            Console.Warn << _T("CSkeleton::PoseSkeletonLite delta AnimTime == ") << iAnimTime - m_iLastAnimTime[iChannel] << newl;

        if (iAnimTime < 0xffff)
            m_iLastAnimTime[iChannel] = 0;
        else
            m_iLastAnimTime[iChannel] = iAnimTime - 0xffff;
    }

    bool bEnd = false;
    if (!m_pAnim[iChannel]->ComputeAnimFrame(iAnimTime, iLoFrame, iHiFrame, fLerp, m_uiForceLength[iChannel]) &&
        !m_bPassiveAnim[iChannel])
        bEnd = true;

    // Check for events on this frame
    if ((m_pModel->GetPoseType() != POSE_VEHICLE && (iChannel == 0 || m_pAnim[iChannel] != m_pAnim[0]))
        || (m_pModel->GetPoseType() == POSE_VEHICLE && (iChannel == 1 || m_pAnim[iChannel] != m_pAnim[1])))
        m_pAnim[iChannel]->CheckEvents(this, m_iLastAnimTime[iChannel], iAnimTime, iChannel);
    m_iLastAnimTime[iChannel] = iAnimTime;
}


/*====================
  CSkeleton::RelativeToWorldRecurse
  ====================*/
void    CSkeleton::RelativeToWorldRecurse(uint uiBone)
{
    SBoneState  *pBoneState(GetBoneState(uiBone));

    if (!(pBoneState->poseState & POSE_MASKED))
    {
        const CBone *pParent(m_pModel->GetBoneParent(uiBone));
        const CBone *pThis(m_pModel->GetBone(uiBone));
        if (pParent != nullptr)
        {
            M_MultiplyMatrix(&m_pBoneStates[pParent->GetIndex()].tm_local, &pBoneState->tm, &pBoneState->tm_local);
            M_MultiplyMatrix(&pBoneState->tm_local, &pThis->m_invBase, &pBoneState->tm_world);
        }
        else
        {
            pBoneState->tm_local = pBoneState->tm;
            M_MultiplyMatrix(&pBoneState->tm_local, &pThis->m_invBase, &pBoneState->tm_world);
        }
    }

    const uivector *pvChildren(m_pModel->GetBoneChildren(uiBone));
    uivector::const_iterator itEnd(pvChildren->end());
    for (uivector::const_iterator it(pvChildren->begin()); it != itEnd; ++it)
        RelativeToWorldRecurse(*it);
}


/*====================
  CSkeleton::CalcWorldTransforms
  ====================*/
void    CSkeleton::CalcWorldTransforms()
{
    PROFILE("CSkeleton::CalcWorldTransforms");

    RelativeToWorldRecurse(0);
}


/*====================
  CSkeleton::RotateBone

  set euler angles for bone.  'multiply' specifies whether or not
  to multiply by the current transform or to use an absolute
  orientation
  ====================*/
void    CSkeleton::RotateBone(const tstring &sBoneName, float yaw_offset, float roll_offset, float pitch_offset, bool multiply)
{
    uint uiBone = m_pModel->GetBoneIndex(sBoneName);
    if (uiBone == INVALID_BONE)
        return;

    matrix43_t rotation = g_identity43;

    // Postmultiply the rotation
    M_GetAxis(pitch_offset, roll_offset, yaw_offset, rotation.axis);

    if (multiply)
    {
        matrix43_t temp;
        M_MultiplyMatrix(&GetBoneState(uiBone)->tm, &rotation, &temp);
        GetBoneState(uiBone)->tm = temp;
    }
    else
    {
        MemManager.Copy(GetBoneState(uiBone)->tm.axis, rotation.axis, sizeof(rotation.axis));
    }
}


/*====================
  CSkeleton::SetModel
  ====================*/
void    CSkeleton::SetModel(ResHandle hModel)
{
    // store the new model and add it as a resource dependent.
    g_ResourceManager.RemoveResourceWatcher(this, m_hModel);
    m_hModel = hModel;
    g_ResourceManager.AddResourceWatcher(this, hModel);

    CModel *pModel(g_ResourceManager.GetModel(hModel));

    if (pModel == nullptr)
    {
        Clear();
        Invalidate();
        return;
    }

    IModel *pIModel = pModel->GetModelFile();

    if (pIModel == nullptr || pIModel->GetType() != MODEL_K2)
    {
        Clear();
        Invalidate();
        return;
    }

    CK2Model *pK2Model = static_cast<CK2Model *>(pIModel);

    if (pK2Model == nullptr)
    {
        Clear();
        Invalidate();
        return;
    }

    // If the skeleton is using a new model, clear all the data
    if (pK2Model != m_pModel)
    {
        Clear();
        m_pModel = pK2Model;
        m_uiNumBones = pK2Model->GetNumBones();

        // Grab bone allocated bone data from bone pool
        SSkeletonBoneData cBoneData;
        SkeletonBonePool.Allocate(m_uiNumBones, cBoneData);

        m_pBoneStates = cBoneData.pBoneStates;
        m_pTempBoneStates = cBoneData.pTempBoneStates;

        MemManager.Set(m_pBoneStates, 0, sizeof(SBoneState) * m_uiNumBones);
        MemManager.Set(m_pTempBoneStates, 0, sizeof(SBoneState) * m_uiNumBones);

        // Reset masks, etc
        for (size_t z(0); z < pK2Model->GetNumBones(); ++z)
        {
            m_pBoneStates[z].poseState = 0;
            m_pBoneStates[z].visibility = 255;

            m_pTempBoneStates[z].poseState = 0;
            m_pTempBoneStates[z].visibility = 255;
        }

        for (int i = 0; i < NUM_ANIM_CHANNELS; ++i)
        {
            m_pCurrentPose[i] = cBoneData.pCurrentPose[i];
            m_pSavedPose[i] = cBoneData.pSavedPose[i];
            m_pTempPose[i] = cBoneData.pTempPose[i];
        }

        const tsvector &vSpawnEvents(pIModel->GetSpawnEvents());
        for (tsvector::const_iterator it(vSpawnEvents.begin()); it != vSpawnEvents.end(); ++it)
            AddEvent(*it, 0);

        g_ResourceManager.AddResourceWatcher(this, hModel);
    }

    Validate();
}


/*====================
  CSkeleton::Rebuild
  ====================*/
void    CSkeleton::Rebuild(ResHandle hResource)
{
    Clear();
    SetModel(hResource);
}


/*====================
  CSkeleton::ClearEvents
  ====================*/
void    CSkeleton::ClearEvents()
{
    m_vEventCmds.clear();
}


/*====================
  CSkeleton::CheckEvents
  ====================*/
bool    CSkeleton::CheckEvents()
{
    return m_vEventCmds.size() > 0;
}


/*====================
  CSkeleton::AddEvent
  ====================*/
void    CSkeleton::AddEvent(const tstring &sCmd, int iTimeNudge)
{
    if (m_vEventCmds.size() >= 255)
    {
        Console.Warn << _T("Large number of events on skeleton") << newl;
        return;
    }

    m_vEventCmds.push_back(SAnimEventCmd(sCmd, iTimeNudge));
}


/*====================
  CSkeleton::GetAnimIndex
  ====================*/
uint    CSkeleton::GetAnimIndex(int iChannel)
{
    if (m_pAnim[iChannel])
        return m_pAnim[iChannel]->GetIndex();
    else
        return -1;
}


/*====================
  CSkeleton::GetAnimIndex
  ====================*/
int     CSkeleton::GetAnimIndex(const tstring &sAnim)
{
    if (m_pModel != nullptr)
        return m_pModel->GetAnimIndex(sAnim);
    else
        return -1;
}


/*====================
  CSkeleton::TempUpdateBonesLerp

  update skeleton local tms (temp version)
  ====================*/
void    CSkeleton::TempUpdateBonesLerp(uint uiBone, float fLerp, int iChannel)
{
    while (uiBone != INVALID_INDEX)
    {
        SBoneState  *pBoneState(GetTempBoneState(uiBone));

        if (pBoneState->poseState & POSE_MASKED)
        {
            uiBone = m_pModel->GetBone(uiBone)->GetParentIndex();
            continue;
        }

        SBoneXForm &cSavedPose(m_pSavedPose[iChannel][uiBone]);
        SBoneXForm &cTempPose(m_pTempPose[iChannel][uiBone]);

        // Translation
        CVec3_cast(pBoneState->tm.pos) = LERP(fLerp, cSavedPose.v3Pos, cTempPose.v3Pos);

        // Rotation
        vec4_t quat;
        M_LerpQuat(fLerp, vec4_cast(cSavedPose.v4Quat), vec4_cast(cTempPose.v4Quat), vec4_cast(quat));
        M_QuatToAxis(quat, pBoneState->tm.axis);

        // Scale
        M_MultVec3(pBoneState->tm.axis[X], LERP(fLerp, cSavedPose.v3Scale[X], cTempPose.v3Scale[X]), pBoneState->tm.axis[X]);
        M_MultVec3(pBoneState->tm.axis[Y], LERP(fLerp, cSavedPose.v3Scale[Y], cTempPose.v3Scale[Y]), pBoneState->tm.axis[Y]);
        M_MultVec3(pBoneState->tm.axis[Z], LERP(fLerp, cSavedPose.v3Scale[Z], cTempPose.v3Scale[Z]), pBoneState->tm.axis[Z]);

        uiBone = m_pModel->GetBone(uiBone)->GetParentIndex();
    }
}


/*====================
  CSkeleton::TempUpdateBones

  update skeleton local tms (temp version)
  ====================*/
void    CSkeleton::TempUpdateBones(uint uiBone, int iChannel)
{
    while (uiBone != INVALID_INDEX)
    {
        SBoneState  *pBoneState(GetTempBoneState(uiBone));

        if (pBoneState->poseState & POSE_MASKED)
        {
            uiBone = m_pModel->GetBone(uiBone)->GetParentIndex();
            continue;
        }

        SBoneXForm &cTempPose(m_pTempPose[iChannel][uiBone]);

        // Translation
        CVec3_cast(pBoneState->tm.pos) = cTempPose.v3Pos;
    
        // Rotation
        M_QuatToAxis(vec4_cast(cTempPose.v4Quat), pBoneState->tm.axis);

        // Scale
        M_MultVec3(pBoneState->tm.axis[X], cTempPose.v3Scale[X], pBoneState->tm.axis[X]);
        M_MultVec3(pBoneState->tm.axis[Y], cTempPose.v3Scale[Y], pBoneState->tm.axis[Y]);
        M_MultVec3(pBoneState->tm.axis[Z], cTempPose.v3Scale[Z], pBoneState->tm.axis[Z]);

        uiBone = m_pModel->GetBone(uiBone)->GetParentIndex();
    }
}


/*====================
  CSkeleton::TempPoseBone
  ====================*/
void    CSkeleton::TempPoseBone(SBoneMotion **ppMotions, uint uiBone, int iLoFrame, int iHiFrame, float fLerp, int iChannel)
{
    if (m_pModel->GetNumBones() <= uiBone)
        return;

    while (uiBone != INVALID_INDEX)
    {
        if (GetTempBoneState(uiBone)->poseState & POSE_MASKED)
        {
            uiBone = m_pModel->GetBone(uiBone)->GetParentIndex();
            continue;
        }

        if (!ppMotions[uiBone])
        {
            m_pTempPose[iChannel][uiBone].v3Pos = CVec3f(0.0f, 0.0f, 0.0f);
            m_pTempPose[iChannel][uiBone].v4Quat = CVec4f(0.0f, 0.0f, 0.0f, 1.0f);
            m_pTempPose[iChannel][uiBone].v3Scale = CVec3f(1.0f, 1.0f, 1.0f);

            uiBone = m_pModel->GetBone(uiBone)->GetParentIndex();
            continue;
        }

        GetBoneXform(ppMotions[uiBone], iLoFrame, iHiFrame, fLerp, m_pTempPose[iChannel][uiBone]);

        uiBone = m_pModel->GetBone(uiBone)->GetParentIndex();
    }
}


/*====================
  CSkeleton::TempPose

  Pose function which changes a temp bone state structure instead of the normal bone state structure
  ====================*/
void    CSkeleton::TempPose(uint uiTime, uint uiBone, int iChannel)
{
    if (uiTime < m_uiStartTime[iChannel])
        uiTime = m_uiStartTime[iChannel]; // don't try to pose back farther then we started this animation

    if (skel_noPose)
        return;

    // Make sure we have an anim to pose
    if (!CheckAnims(uiTime))
        return;

    // Determine hi and lo frames and interpolation amount
    int     iAnimTime(m_fSpeed[iChannel] == 0.0f ? m_iOffsetTime[iChannel] : INT_FLOOR((uiTime - m_uiStartTime[iChannel] + m_iOffsetTime[iChannel]) * m_fSpeed[iChannel]));
    int     iLoFrame, iHiFrame;
    float   fLerp;

    m_pAnim[iChannel]->ComputeAnimFrame(iAnimTime, iLoFrame, iHiFrame, fLerp, m_uiForceLength[iChannel]);

    // Get the clip
    CClip *pClip = g_ResourceManager.GetClip(m_pAnim[iChannel]->GetClip());
    if (pClip)
    {
        int iNumFrames = pClip->GetNumFrames();
        if (iNumFrames != 0)
        {
            // Make sure we don't get an access violation trying to access bad frames
            iLoFrame = (int)iLoFrame % iNumFrames;
            iHiFrame = (int)iHiFrame % iNumFrames;
        }
    }

    // iLoFrame and iHiFrame should be valid array indices at this point
    TempPoseBone(m_pAnim[iChannel]->GetMotions(), uiBone, iLoFrame, iHiFrame, fLerp, iChannel);

    // Blending
    if (m_uiBlendTime[iChannel] && uiTime - m_uiBlendStartTime[iChannel] < m_uiBlendTime[iChannel] && skel_blendAnims)
    {
        float fBlend(M_SmoothStepN((uiTime - m_uiBlendStartTime[iChannel]) / float(m_uiBlendTime[iChannel])));
        TempUpdateBonesLerp(uiBone, fBlend, iChannel);
    }
    else
        TempUpdateBones(uiBone, iChannel);
}


/*====================
  CSkeleton::TempCalcWorldTransforms
  ====================*/
void    CSkeleton::TempCalcWorldTransforms(uint uiBone)
{
    uint uiParentIndex(m_pModel->GetBone(uiBone)->GetParentIndex());

    if (uiParentIndex != INVALID_INDEX)
    {
        if (uiParentIndex > m_pModel->GetNumBones())
        {
            Console.Warn << _T("CSkeleton::TempCalcWorldTransforms() - Invalid bone parent index: ") << uiParentIndex << _T(", model: ") << m_pModel->GetName() << _T(", model bones: ") << m_pModel->GetNumBones() << _T(", skeleton bones: ") << GetNumBones() << _T(", child bone: ") << uiBone << newl;
            uiParentIndex = INVALID_INDEX;
        }
        else
            TempCalcWorldTransforms(uiParentIndex);
    }

    SBoneState  *pBoneState(GetTempBoneState(uiBone));

    if (!(pBoneState->poseState & POSE_MASKED))
    {
        if (uiParentIndex != INVALID_INDEX)
            M_MultiplyMatrix(&m_pTempBoneStates[uiParentIndex].tm_local, &pBoneState->tm, &pBoneState->tm_local);
        else
            pBoneState->tm_local = pBoneState->tm;
    }
}


/*====================
  CSkeleton::SetTempBoneState
  ====================*/
void    CSkeleton::SetTempBoneState(uint uiBone, int iState)
{
    if (uiBone == INVALID_BONE)
        return;

    m_pTempBoneStates[uiBone].poseState = iState;

    CBone *pBone(m_pModel->GetBone(uiBone));
    uint uiNumChildren(pBone->NumChildren());
    for (uint n(0); n < uiNumChildren; ++n)
    {
        uint uiChild(pBone->GetChildIndex(n));
        SetTempBoneState(uiChild, iState);
    }
}


/*====================
  CSkeleton::SetTempBoneState
  ====================*/
void    CSkeleton::SetTempBoneState(const tstring &sBoneName, int iState)
{
    uint uiBone(m_pModel->GetBoneIndex(sBoneName));
    if (uiBone == INVALID_BONE)
        return;

    SetTempBoneState(uiBone, iState);
}


/*====================
  CSkeleton::SetTempBoneState
  ====================*/
void    CSkeleton::SetTempBoneState(int iState)
{
    for (uint n(0); n < m_uiNumBones; ++n)
        m_pTempBoneStates[n].poseState = iState;
}


/*====================
  CSkeleton::TempRotateBone

  set euler angles for bone.  'multiply' specifies whether or not
  to multiply by the current transform or to use an absolute
  orientation
  ====================*/
void    CSkeleton::TempRotateBone(const tstring &sBoneName, float yaw_offset, float roll_offset, float pitch_offset, bool multiply)
{
    uint uiBone = m_pModel->GetBoneIndex(sBoneName);
    if (uiBone == INVALID_BONE)
        return;

    SBoneState  *pBoneState(GetTempBoneState(uiBone));

    if (m_pModel->GetBone(uiBone)->GetParentIndex() != INVALID_INDEX)
        TempCalcWorldTransforms(m_pModel->GetBone(uiBone)->GetParentIndex());

    matrix43_t rotation = g_identity43;

    // Postmultiply the rotation
    M_GetAxis(pitch_offset, roll_offset, yaw_offset, rotation.axis);

    if (multiply)
    {
        matrix43_t temp;
        M_MultiplyMatrix(&pBoneState->tm, &rotation, &temp);
        pBoneState->tm = temp;
    }
    else
    {
        MemManager.Copy(pBoneState->tm.axis, rotation.axis, sizeof(rotation.axis));
    }
}


/*====================
  CSkeleton::GetBonePose
  ====================*/
CVec3f  CSkeleton::GetBonePose(uint uiBone, uint uiTime)
{
    PROFILE("CSkeleton::GetBonePose");

    if (uiBone == INVALID_BONE || !m_pModel)
        return V3_ZERO;

    if (uiTime == m_uiPoseTime || m_pModel->GetPoseType() == POSE_VEHICLE)
        return CVec3_cast(m_pBoneStates[uiBone].tm_local.pos);

    if (m_pModel->GetPoseType() == POSE_CHARACTER)
    {
        uint uiBip01Spine(m_pModel->GetBoneIndex(_T("Bip01 Spine")));

        // *** Channel 0 ***
        // Mask everything above the spine
        SetTempBoneState(0);
        SetTempBoneState(uiBip01Spine, POSE_MASKED);

        TempPose(uiTime, uiBone, 0);

        // *** Channel 1 ***
        // Mask everything below the spine so only the upper
        // body gets posed
        SetTempBoneState(POSE_MASKED);
        SetTempBoneState(uiBip01Spine, 0);

        TempPose(uiTime, uiBone, 1);

        // Head/Torso movements
        if (m_uiSavedPoseTime != INVALID_TIME)
        {
            float fPitch;
            float fYaw;

            if (m_uiPrevPoseTime != INVALID_TIME)
            {
                fPitch = M_LerpAngle(ILERP(uiTime, m_uiPrevPoseTime, m_uiSavedPoseTime), m_fPrevPitch, m_fPitch);
                fYaw = M_LerpAngle(ILERP(uiTime, m_uiPrevPoseTime, m_uiSavedPoseTime), m_fPrevYaw, m_fYaw);
            }
            else
            {
                fPitch = m_fPitch;
                fYaw = m_fYaw;
            }

            if (fPitch > 180.0f)
                fPitch -= 360.0f;
            if (fPitch < -180.0f)
                fPitch += 360.0f;

            if (fYaw > 180.0f)
                fYaw -= 360.0f;
            if (fYaw < -180.0f)
                fYaw += 360.0f;
            
            if (fYaw || fPitch)
            {
                // HACK: For some reason pitch and yaw values are swapped in the biped
                // Biped orients X towards children
                TempRotateBone(_T("Bip01 Head"), -fPitch / 4.0f, 0, fYaw * 0.25f, true);
                TempRotateBone(_T("Bip01 Spine"), -fPitch / 6.0f, 0, fYaw * 0.75f, true);
            }
        }

        // Get the final world coordinate transformation, then
        // unmask the legs
        SetTempBoneState(0);
    }
    else
    {
        TempPose(uiTime, uiBone, 0);
    }

    {
        PROFILE("CSkeleton::TempCalcWorldTransforms");
        TempCalcWorldTransforms(uiBone);
    }

    return CVec3_cast(m_pTempBoneStates[uiBone].tm_local.pos);
}


/*====================
  CSkeleton::GetBonePoseAxisPos
  ====================*/
void    CSkeleton::GetBonePoseAxisPos(uint uiBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos)
{
    PROFILE("CSkeleton::GetBonePoseAxisPos");

    if (uiBone == INVALID_BONE || !m_pModel)
    {
        aOutAxis = CAxis(0.0f, 0.0f, 0.0f);
        v3OutPos = CVec3f(0.0f, 0.0f, 0.0f);
        return;
    }

    if (uiTime == m_uiPoseTime || m_pModel->GetPoseType() == POSE_VEHICLE)
    {
        aOutAxis = CAxis_cast(m_pBoneStates[uiBone].tm_local.axis);
        v3OutPos = CVec3_cast(m_pBoneStates[uiBone].tm_local.pos);
        return;
    }

    if (m_pModel->GetPoseType() == POSE_CHARACTER)
    {
        uint uiBip01Spine(m_pModel->GetBoneIndex(_T("Bip01 Spine")));

        // *** Channel 0 ***
        // Mask everything above the spine
        SetTempBoneState(0);
        SetTempBoneState(uiBip01Spine, POSE_MASKED);

        TempPose(uiTime, uiBone, 0);

        // *** Channel 1 ***
        // Mask everything below the spine so only the upper
        // body gets posed
        SetTempBoneState(POSE_MASKED);
        SetTempBoneState(uiBip01Spine, 0);

        TempPose(uiTime, uiBone, 1);

        // Head/Torso movements
        if (m_uiSavedPoseTime != INVALID_TIME)
        {
            float fPitch;
            float fYaw;

            if (m_uiPrevPoseTime != INVALID_TIME)
            {
                fPitch = M_LerpAngle(ILERP(uiTime, m_uiPrevPoseTime, m_uiSavedPoseTime), m_fPrevPitch, m_fPitch);
                fYaw = M_LerpAngle(ILERP(uiTime, m_uiPrevPoseTime, m_uiSavedPoseTime), m_fPrevYaw, m_fYaw);
            }
            else
            {
                fPitch = m_fPitch;
                fYaw = m_fYaw;
            }

            if (fPitch > 180.0f)
                fPitch -= 360.0f;
            if (fPitch < -180.0f)
                fPitch += 360.0f;

            if (fYaw > 180.0f)
                fYaw -= 360.0f;
            if (fYaw < -180.0f)
                fYaw += 360.0f;
            
            if (fYaw || fPitch)
            {
                // HACK: For some reason pitch and yaw values are swapped in the biped
                // Biped orients X towards children
                TempRotateBone(_T("Bip01 Head"), -fPitch / 4.0f, 0, fYaw * 0.25f, true);
                TempRotateBone(_T("Bip01 Spine"), -fPitch / 6.0f, 0, fYaw * 0.75f, true);
            }
        }

        // Get the final world coordinate transformation, then
        // unmask the legs
        SetTempBoneState(0);
    }
    else
    {
        TempPose(uiTime, uiBone, 0);
    }

    {
        TempCalcWorldTransforms(uiBone);
    }

    aOutAxis = CAxis_cast(m_pTempBoneStates[uiBone].tm_local.axis);
    v3OutPos = CVec3_cast(m_pTempBoneStates[uiBone].tm_local.pos);

#if 0
    if (!aOutAxis.Forward().IsValid() || 
        !aOutAxis.Right().IsValid() ||
        !aOutAxis.Up().IsValid() ||
        !v3OutPos.IsValid())
    {
        Console << _T("CSkeleton::GetBonePoseAxisPos - NAN") << newl;
        
        aOutAxis = CAxis(0.0f, 0.0f, 0.0f);
        v3OutPos = CVec3f(0.0f, 0.0f, 0.0f);
    }
#endif

    return;
}


/*====================
  CSkeleton::GetCurrentAnimName
  ====================*/
const tstring&  CSkeleton::GetCurrentAnimName(int iChannel)
{
    if (m_pAnim[iChannel])
        return m_pAnim[iChannel]->GetName();
    else
        return TSNULL;
}


/*====================
  CSkeleton::SetAnimSpeed
  ====================*/
void    CSkeleton::SetAnimSpeed(float fSpeed, int iChannel)
{
    if (m_fSpeed[iChannel] == fSpeed ||
        m_uiTime[iChannel] == INVALID_TIME ||
        m_uiStartTime[iChannel] == INVALID_TIME ||
        m_iParentChannel[iChannel] != iChannel ||
        !m_pModel || m_pModel->GetPoseType() == POSE_VEHICLE)
        return;

#if 0
    int iOldOffsetTime(m_iOffsetTime[iChannel]);
#endif

    if (fSpeed == 0.0f)
    {
        int iAnimTime(INT_FLOOR((m_uiTime[iChannel] - m_uiStartTime[iChannel] + m_iOffsetTime[iChannel])) * m_fSpeed[iChannel]);
        m_iOffsetTime[iChannel] = iAnimTime;
    }
    else
    {
        if (m_fSpeed[iChannel] == 0.0f)
        {
            int iAnimTime(m_iOffsetTime[iChannel]);
            m_iOffsetTime[iChannel] = INT_FLOOR((iAnimTime - (m_uiTime[iChannel] - m_uiStartTime[iChannel]) * fSpeed) / fSpeed);
        }
        else
        {
            int iAnimTime(INT_FLOOR((m_uiTime[iChannel] - m_uiStartTime[iChannel] + m_iOffsetTime[iChannel])) * m_fSpeed[iChannel]);
            m_iOffsetTime[iChannel] = INT_FLOOR((iAnimTime - (m_uiTime[iChannel] - m_uiStartTime[iChannel]) * fSpeed) / fSpeed);
        }
    }

#if 0
    if (m_iOffsetTime[iChannel] < -1000000)
    {
        int x = 0;
    }
#endif

    m_fSpeed[iChannel] = fSpeed;

    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
    {
        if (i != iChannel && m_iParentChannel[i] == iChannel)
        {
            m_iOffsetTime[i] = m_iOffsetTime[iChannel];
            m_fSpeed[i] = m_fSpeed[iChannel];
        }
    }
}


/*====================
  CSkeleton::SetAnimTime
  ====================*/
void    CSkeleton::SetAnimTime(int iAnimTime, int iChannel)
{
    if (m_fSpeed[iChannel] == 0.0f)
    {
        m_iOffsetTime[iChannel] = iAnimTime;
    }
}


/*====================
  CSkeleton::SetAnimTime
  ====================*/
void    CSkeleton::SetAnimTime(float fFraction, int iChannel)
{
    if (m_fSpeed[iChannel] == 0.0f)
    {
        m_iOffsetTime[iChannel] = INT_ROUND(m_pAnim[iChannel]->GetNumFrames() * m_pAnim[iChannel]->GetMSperFrame() * fFraction);
    }
}


/*====================
  CSkeleton::Pose
  ====================*/
void    CSkeleton::Pose(uint uiTime, SBoneXForm *pPose)
{
    PROFILE("CSkeleton::Pose");

    m_bIsCharacter = false;
    m_fPrevPitch = 0.0f;
    m_fPrevYaw = 0.0f;
    m_fPitch = 0.0f;
    m_fYaw = 0.0f;
    int iChannel(0);

    if (!m_pModel)
        return;

    MemManager.Copy(m_pCurrentPose[iChannel], pPose, sizeof(SBoneXForm) * m_uiNumBones);

    {
        PROFILE("UpdateBones");

        uint uiBlendTime(m_fSpeed[iChannel] != 0.0f ? INT_ROUND(m_uiBlendTime[iChannel] / m_fSpeed[iChannel]) : 0);

        // Blending
        if (uiBlendTime && uiTime - m_uiBlendStartTime[iChannel] < uiBlendTime && skel_blendAnims)
        {
            float fBlend(M_SmoothStepN((uiTime - m_uiBlendStartTime[iChannel]) / float(uiBlendTime)));
            UpdateBonesLerp(fBlend, iChannel);
        }
        else
            UpdateBones(iChannel);
    }

    Validate();

    m_uiTime[iChannel] = uiTime;

    m_bValidPose[iChannel] = true;

    CalcWorldTransforms();
}


/*====================
  CSkeleton::ComputeAnimFrame
  ====================*/
void    CSkeleton::ComputeAnimFrame(uint uiTime, int iChannel, int &iAnimTime, float &fFrame, float &fFraction)
{
    if (!CheckAnims(uiTime))
        return;

    // Determine hi and lo frames and interpolation amount
    iAnimTime = m_fSpeed[iChannel] == 0.0f ? m_iOffsetTime[iChannel] : INT_FLOOR((uiTime - m_uiStartTime[iChannel] + m_iOffsetTime[iChannel]) * m_fSpeed[iChannel]);
    int     iLoFrame, iHiFrame;
    float   fLerp;

    bool bEnd = false;
    if (!m_pAnim[iChannel]->ComputeAnimFrame(iAnimTime, iLoFrame, iHiFrame, fLerp, m_uiForceLength[iChannel]) &&
        !m_bPassiveAnim[iChannel])
        bEnd = true;

    fFrame = iLoFrame - m_pAnim[iChannel]->GetStartFrame() + fLerp;
    fFraction = fFrame / m_pAnim[iChannel]->GetNumFrames();
}


/*====================
  CSkeleton::RequestStartAnim
  ====================*/
void    CSkeleton::RequestStartAnim(const tstring &sAnim, uint uiTime, int iChannel, int iBlendTime, float fSpeed, uint uiForceLength)
{
    PROFILE("CSkeleton::RequestStartAnim");

    //Console.Dev << "CSkeleton::RequestStartAnim() - " << sAnim << _T(" ") << uiTime << _T(" ") << iChannel << newl;

    m_aAnimRequest[iChannel].sAnim = sAnim;
    m_aAnimRequest[iChannel].uiTime = uiTime;
    m_aAnimRequest[iChannel].iBlendTime = iBlendTime;
    m_aAnimRequest[iChannel].fSpeed = fSpeed;
    m_aAnimRequest[iChannel].uiForceLength = uiForceLength;
}


/*====================
  CSkeleton::RequestStopAnim
  ====================*/
void    CSkeleton::RequestStopAnim(int iChannel, uint uiTime)
{
    PROFILE("CSkeleton::RequestStopAnim");

    m_aAnimRequest[iChannel].sAnim.clear();
    m_aAnimRequest[iChannel].uiTime = uiTime;
}

