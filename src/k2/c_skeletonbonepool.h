// (C)2008 S2 Games
// c_skeletonbonepool.h
//
//=============================================================================
#ifndef __C_SKELETONBONEPOOL_H__
#define __C_SKELETONBONEPOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_singleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint NUM_SKELETONBONEPOOL_BUCKETS(128);

#define SkeletonBonePool    (*CSkeletonBonePool::GetInstance())

struct SBoneState;
struct SBoneXForm;
//=============================================================================

//=============================================================================
// SSkeletonBoneData
//=============================================================================
struct SSkeletonBoneData
{
    uint            uiNumBones;

    SBoneState*     pBoneStates;
    SBoneState*     pTempBoneStates;

    SBoneXForm*     pSavedPose[NUM_ANIM_CHANNELS];
    SBoneXForm*     pCurrentPose[NUM_ANIM_CHANNELS];
    SBoneXForm*     pTempPose[NUM_ANIM_CHANNELS];
};
//=============================================================================

//=============================================================================
// CSkeletonBonePool
//=============================================================================
class CSkeletonBonePool
{
    SINGLETON_DEF(CSkeletonBonePool)

private:
    vector<SSkeletonBoneData>   m_vFreeLists[NUM_SKELETONBONEPOOL_BUCKETS];

public:
    ~CSkeletonBonePool();

    void                Allocate(uint uiNumBones, SSkeletonBoneData &cBoneData);
    void                Deallocate(const SSkeletonBoneData &cBoneData);
};
//=============================================================================

#endif  //__C_SKELETONBONEPOOL_H__
