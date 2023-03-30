// (C)2005 S2 Games
// c_skeleton.h
//
//=============================================================================
#ifndef __C_SKELETON_H__
#define __C_SKELETON_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resourcewatcher2.h"
#include "c_k2model.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int ANIM_EVENT_STRING_LENGTH = 256;

//the only fields here the game code needs to deal with directly are the 'events' fields
//all other fields are managed internally via the SetAnim() call

enum eSetBoneAnimReturnValues
{
    SKEL_INVALID,
    SKEL_NORMAL,
    SKEL_ANIM_NOT_FOUND,
    SKEL_ANIM_FINISHED,
    SKEL_BONE_NOT_FOUND,
    SKEL_INVALID_CLIP
};

struct SBoneState
{
    matrix43_t  tm;
    matrix43_t  tm_local;   //computed at CSkeleton::EndPose()
    matrix43_t  tm_world;   //computed at CSkeleton::EndPose()
    int         poseState;
    byte        visibility;
};

struct SBoneXForm
{
    CVec3f  v3Pos;
    CVec4f  v4Quat;
    CVec3f  v3Scale;
};

struct SAnimEventCmd
{
    tstring sCmd;
    int     iTimeNudge;

    SAnimEventCmd() {}
    SAnimEventCmd(const tstring &_sCmd, int _iTimeNudge) : sCmd(_sCmd), iTimeNudge(_iTimeNudge) {}
};

#define POSE_MASKED 0x0001

class CK2Model;
class CAnim;
class CBone;
struct SBoneMotion;
//=============================================================================

//=============================================================================
// CSkeleton
//=============================================================================
class CSkeleton : public IResourceWatcher
{
private:
    SBoneState* m_pBoneStates;  // State of each bone
    SBoneState* m_pTempBoneStates;  // State of each bone
    uint        m_uiNumBones;
    bool        m_bIsValid;     // Set to true if this skeleton is not valid for this frame

    bool        m_bIsCharacter;
    uint        m_uiPoseTime;
    uint        m_uiPrevPoseTime;
    uint        m_uiSavedPoseTime;
    float       m_fPrevPitch;
    float       m_fPitch;
    float       m_fPrevYaw;
    float       m_fYaw;

    tstring     m_sDefaultAnimName;

    ResHandle   m_hModel;
    CK2Model*   m_pModel;
    CAnim*      m_pAnim[NUM_ANIM_CHANNELS];
    uint        m_uiStartTime[NUM_ANIM_CHANNELS];
    float       m_fSpeed[NUM_ANIM_CHANNELS];
    uint        m_uiForceLength[NUM_ANIM_CHANNELS];
    int         m_iOffsetTime[NUM_ANIM_CHANNELS];

    uint        m_uiBlendStartTime[NUM_ANIM_CHANNELS];
    uint        m_uiBlendTime[NUM_ANIM_CHANNELS];

    uint        m_uiTime[NUM_ANIM_CHANNELS];            // Time skeleton was last posed
    int         m_iLastAnimTime[NUM_ANIM_CHANNELS];

    bool        m_bPassiveAnim[NUM_ANIM_CHANNELS];
    int         m_iParentChannel[NUM_ANIM_CHANNELS];

    vector<SAnimEventCmd>       m_vEventCmds;

    bool            m_bValidPose[NUM_ANIM_CHANNELS];
    SBoneXForm*     m_pSavedPose[NUM_ANIM_CHANNELS];
    SBoneXForm*     m_pCurrentPose[NUM_ANIM_CHANNELS];
    SBoneXForm*     m_pTempPose[NUM_ANIM_CHANNELS];

    struct SAnimRequest
    {
        tstring sAnim;
        uint    uiTime;
        int     iChannel;
        int     iBlendTime;
        float   fSpeed;
        uint    uiForceLength;
    };

    SAnimRequest    m_aAnimRequest[NUM_ANIM_CHANNELS];

    // Internal stuff
    void    GetBoneXform(SBoneMotion *pMotion, int iLoFrame, int iHiFrame, float fLerp, SBoneXForm &cBone);
    void    PoseBones(SBoneMotion **ppMotions, int iLoFrame, int iHiFrame, float fLerp, int iChannel);
    void    UpdateBones(int iChannel);
    void    UpdateBonesLerp(float fLerp, int iChannel);
    void    BlendBones(float fLerp, int iChannel);
    void    CalcWorldTransforms();
    void    RelativeToWorldRecurse(uint uiBone);
    bool    CheckAnims(uint uiTime);
    void    SetBoneState(uint uiBone, int iState);
    void    SetBoneState(int iState);
    void    SetAnim(const tstring &sAnim, uint uiTime, int iBlendTime, int iChannel, float fSpeed, uint uiForceLength, bool bStartEvent, bool bEndEvent);
    void    PoseSkeleton(uint uiTime, int iChannel);
    void    PoseSkeletonLite(uint uiTime, int iChannel);

    void    SetModel(CK2Model *pModel);

    void    TempPose(uint uiTime, uint uiBone, int iChannel);
    void    TempCalcWorldTransforms(uint uiBone);
    void    TempUpdateBonesLerp(uint uiBone, float fLerp, int iChannel);
    void    TempUpdateBones(uint uiBone, int iChannel);
    SBoneState*     GetTempBoneState(uint uiBone)           { return &m_pTempBoneStates[uiBone]; }
    void    SetTempBoneState(const tstring &sBoneName, int iState);
    void    SetTempBoneState(uint uiBone, int iState);
    void    SetTempBoneState(int iState);
    void    TempPoseBone(SBoneMotion **ppMotions, uint uiBone, int iLoFrame, int iHiFrame, float fLerp, int iChannel);
    void    TempRotateBone(const tstring &sBoneName, float yaw_offset, float roll_offset, float pitch_offset, bool multiply);

    void    PoseStandard(uint uiTime);
    void    PoseCharacter(uint uiTime, float fPitch, float fYaw);
    void    PoseVehicle(uint uiTime, uint uiWHeelTime);
    void    PoseGadget(uint uiTime, float fPitch, float fYaw);

    void    ProcessAnimRequests(uint uiTime);

    K2_API void Clear();

public:
    K2_API  ~CSkeleton();
    K2_API  CSkeleton();

    void            SetDefaultAnim(const tstring &sAnim)    { m_sDefaultAnimName = sAnim; }
    const tstring&  GetDefaultAnim() const                  { return m_sDefaultAnimName; }

    uint            GetBone(const tstring &sBone)           { return m_pModel ? m_pModel->GetBoneIndex(sBone) : INVALID_BONE; }
    SBoneState*     GetBoneState(uint uiBone)               { return &m_pBoneStates[uiBone]; }
    uint            GetNumBones()                           { return m_uiNumBones; }

    ResHandle       GetModel()                              { return m_hModel; }

    const vector<SAnimEventCmd>&    GetEventCmds()          { return m_vEventCmds; }

    // Error checking
    void            Invalidate()                            { m_bIsValid = false; }
    void            Validate()                              { m_bIsValid = true; }
    bool            IsValid()                               { return m_bIsValid; }

    void    Rebuild(ResHandle hResource);

    K2_API bool HasAnim(const tstring &sAnim);
    K2_API int  StartAnim(const tstring &sAnim, uint uiTime, int iChannel, int iBlendTime = -1, float fSpeed = 1.0f, uint uiForceLength = 0);
    K2_API void StopAnim(int iChannel, uint uiTime);
    K2_API void Pose(uint uiTime, float fParam1 = 0.0f, float fParam2 = 0.0f);
    K2_API void PoseLite(uint uiTime);

    K2_API void SetModel(ResHandle hModel);
    K2_API void SetBoneState(const tstring &sBoneName, int iState);
    K2_API void RotateBone(const tstring &sBoneName, float yaw_offset, float roll_offset, float pitch_offset, bool multiply);

    K2_API void ClearEvents();
    K2_API bool CheckEvents();
    K2_API void AddEvent(const tstring &sCmd, int iTimeNudge);

    K2_API uint     GetAnimIndex(int iChannel);
    K2_API int      GetAnimIndex(const tstring &sAnim);
    
    K2_API CVec3f   GetBonePose(uint uiBone, uint uiTime);
    K2_API void     GetBonePoseAxisPos(uint uiBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos);

    bool        IsPassiveAnim(int iChannel)             { return m_bPassiveAnim[iChannel]; }
    K2_API const tstring&   GetCurrentAnimName(int iChannel);
    uint        GetCurrentAnimStartTime(int iChannel)   { return m_uiStartTime[iChannel]; }
    float       GetCurrentAnimSpeed(int iChannel)       { return m_fSpeed[iChannel]; }
    int         GetCurrentAnimOffsetTime(int iChannel)      { return m_iOffsetTime[iChannel]; }

    K2_API bool HasValidAnims();

    K2_API void SetAnimSpeed(float fSpeed, int iChannel);
    K2_API void SetAnimTime(int iAnimTime, int iChannel);
    K2_API void SetAnimTime(float fFraction, int iChannel);

    K2_API void SetLastAnimTime(int iTime, int iChannel)    { m_iLastAnimTime[iChannel] = iTime; }

    K2_API void Pose(uint uiTime, SBoneXForm *pPose);

    SBoneXForm *GetCurrentPose(int iChannel)                { return m_pCurrentPose[iChannel]; }

    K2_API void ComputeAnimFrame(uint uiTime, int iChannel, int &iAnimTime, float &fFrame, float &fFraction);

    K2_API void RequestStartAnim(const tstring &sAnim, uint uiTime, int iChannel, int iBlendTime = -1, float fSpeed = 1.0f, uint uiForceLength = 0);
    K2_API void RequestStopAnim(int iChannel, uint uiTime);
};
//=============================================================================

#endif //__C_SKELETON_H__
