// (C)2008 S2 Games
// c_skeletonbonepool.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_skeletonbonepool.h"

#include "c_skeleton.h"
//=============================================================================

//=============================================================================
// Globals
//============================================================================
SINGLETON_INIT(CSkeletonBonePool)
//=============================================================================

/*====================
  CSkeletonBonePool::~CSkeletonBonePool
  ====================*/
CSkeletonBonePool::~CSkeletonBonePool()
{
    for (uint uiBucket(0); uiBucket < NUM_SKELETONBONEPOOL_BUCKETS; ++uiBucket)
    {
        for (vector<SSkeletonBoneData>::iterator it(m_vFreeLists[uiBucket].begin()), itEnd(m_vFreeLists[uiBucket].end()); it != itEnd; ++it)
        {
            K2_DELETE_ARRAY(it->pBoneStates);
            K2_DELETE_ARRAY(it->pTempBoneStates);

            for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
            {
                K2_DELETE_ARRAY(it->pCurrentPose[i]);
                K2_DELETE_ARRAY(it->pSavedPose[i]);
                K2_DELETE_ARRAY(it->pTempPose[i]);
            }
        }
    }
}


/*====================
  CSkeletonBonePool::CSkeletonBonePool
  ====================*/
CSkeletonBonePool::CSkeletonBonePool()
{
}


/*====================
  CSkeletonBonePool::Allocate
  ====================*/
void    CSkeletonBonePool::Allocate(uint uiNumBones, SSkeletonBoneData &cBoneData)
{
    if (uiNumBones == 0)
    {
        cBoneData.uiNumBones = 0;
        cBoneData.pBoneStates = nullptr;
        cBoneData.pTempBoneStates = nullptr;

        for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        {
            cBoneData.pCurrentPose[i] = nullptr;
            cBoneData.pSavedPose[i] = nullptr;
            cBoneData.pTempPose[i] = nullptr;
        }

        return;
    }

    uint uiBucket(uiNumBones - 1);

    // Reuse the next free allocation from the appropriate bucket if available
    if (uiBucket < NUM_SKELETONBONEPOOL_BUCKETS && !m_vFreeLists[uiBucket].empty())
    {
        cBoneData = m_vFreeLists[uiBucket].back();
        m_vFreeLists[uiBucket].pop_back();
    }
    else
    {
        cBoneData.uiNumBones = uiNumBones;
        cBoneData.pBoneStates = K2_NEW_ARRAY(ctx_Models, SBoneState, uiNumBones);
        cBoneData.pTempBoneStates = K2_NEW_ARRAY(ctx_Models, SBoneState, uiNumBones);

        for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        {
            cBoneData.pCurrentPose[i] = K2_NEW_ARRAY(ctx_Models, SBoneXForm, uiNumBones);
            cBoneData.pSavedPose[i] = K2_NEW_ARRAY(ctx_Models, SBoneXForm, uiNumBones);
            cBoneData.pTempPose[i] = K2_NEW_ARRAY(ctx_Models, SBoneXForm, uiNumBones);
        }
    }
}


/*====================
  CSkeletonBonePool::Deallocate
  ====================*/
void    CSkeletonBonePool::Deallocate(const SSkeletonBoneData &cBoneData)
{
    uint uiBucket(cBoneData.uiNumBones - 1);

    if (uiBucket < NUM_SKELETONBONEPOOL_BUCKETS)
    {
        // Store this allocation for later use
        m_vFreeLists[uiBucket].push_back(cBoneData);
    }
    else if (cBoneData.uiNumBones > 0)
    {
        K2_DELETE_ARRAY(cBoneData.pBoneStates);
        K2_DELETE_ARRAY(cBoneData.pTempBoneStates);

        for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        {
            K2_DELETE_ARRAY(cBoneData.pCurrentPose[i]);
            K2_DELETE_ARRAY(cBoneData.pSavedPose[i]);
            K2_DELETE_ARRAY(cBoneData.pTempPose[i]);
        }
    }
}
