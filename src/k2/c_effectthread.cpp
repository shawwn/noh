// (C)2006 S2 Games
// c_effectthread.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_effectthread.h"
#include "i_effectcmd.h"
#include "c_particlesystem.h"
#include "c_effect.h"
#include "c_skeleton.h"
#include "c_model.h"
#include "c_k2model.h"
#include "c_mesh.h"
#include "c_camera.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_INTF   (efx_activeThreads,     0,      CVAR_READONLY | CVAR_DONTSAVE);
//=============================================================================


/*====================
  CEffectThread::~CEffectThread
  ====================*/
CEffectThread::~CEffectThread()
{
    for (InstanceMap::iterator itInstance(m_mapInstances.begin()); itInstance != m_mapInstances.end(); ++itInstance)
        K2_DELETE(itInstance->second);

    // m_vCmdBuffer is only populated in the effect thread definition instance
    for (EffectCmdBuffer::iterator itCmd(m_vCmdBuffer.begin()); itCmd != m_vCmdBuffer.end(); ++itCmd)
        K2_DELETE(*itCmd);

    g_ResourceManager.RemoveResourceWatcher(this, m_hEffect);

    if (m_uiStartTime != 0xffffffff)
        --efx_activeThreads;
}


/*====================
  CEffectThread::CEffectThread

  Allocated effect thread definition
  ====================*/
CEffectThread::CEffectThread(CEffect *pEffect) :
m_pEffect(pEffect),
m_hEffect(INVALID_RESOURCE),
m_uiStartTime(0xffffffff),
m_pvCmdBuffer(&m_vCmdBuffer),
m_pSourceSkeleton(NULL),
m_pTargetSkeleton(NULL)
{
    /*      DO NOT DO THIS -- the pEffect OWNS us, we are not dependent on it! 
    m_pEffect->AddResourceWatcher(this);
    */

    m_itCurrentCmd = m_vCmdBuffer.end();
}


/*====================
  CEffectThread::CEffectThread
  ====================*/
CEffectThread::CEffectThread(uint uiStartTime, const CEffectThread &etSettings) :
m_pEffect(etSettings.m_pEffect),
m_hEffect(etSettings.m_pEffect->GetHandle()),
m_uiStartTime(uiStartTime),
m_uiLastUpdateTime(INVALID_TIME),
m_uiWaitTime(0),
m_pvCmdBuffer(&etSettings.m_vCmdBuffer),
m_itStartCmd(etSettings.m_itCurrentCmd),
m_itCurrentCmd(etSettings.m_itCurrentCmd),

m_bActive(false),
m_bExpire(false),

m_pCamera(NULL),
m_v3CameraOffset(V3_ZERO),
m_v3CameraAngleOffset(V3_ZERO),

m_uiCameraShakeTime(0),
m_uiCameraShakeDuration(0),
m_fCameraShakeFrequency(0.0f),
m_v3CameraShake(V3_ZERO),

m_uiCameraKickTime(INVALID_TIME),
m_fCameraKickHalfLife(0.0f),
m_v3CameraKickAngles(V3_ZERO),
m_v3CameraKickPosition(V3_ZERO),

m_bActiveOverlay(false),
m_uiOverlayStartTime(0),
m_uiOverlayDuration(0),
m_v4OverlayColor(V4_ZERO),
m_hOverlayMaterial(INVALID_RESOURCE),

m_pSourceSkeleton(NULL),
m_pSourceModel(NULL),
m_v3SourcePos(CVec3f(0.0f, 0.0f, 0.0f)),
m_aSourceAxis(),
m_fSourceScale(1.0f),
m_fSourceEffectScale(1.0f),
m_bSourceVisibility(true),

m_pTargetSkeleton(NULL),
m_pTargetModel(NULL),
m_v3TargetPos(CVec3f(0.0f, 0.0f, 0.0f)),
m_aTargetAxis(),
m_fTargetScale(1.0f),
m_fTargetEffectScale(1.0f),
m_bTargetVisibility(true),
m_bCustomVisibility(true),

m_v3Color(1.0f, 1.0f, 1.0f)
{
    g_ResourceManager.AddResourceWatcher(this, m_hEffect);

    ++efx_activeThreads;
}


/*====================
  CEffectThread::Rebuild
  ====================*/
void    CEffectThread::Rebuild(ResHandle hResource)
{
    for (InstanceMap::iterator it(m_mapInstances.begin()); it != m_mapInstances.end(); ++it)
        K2_DELETE(it->second);

    m_mapInstances.clear();

    assert(m_hEffect != INVALID_RESOURCE);
    if (m_hEffect != INVALID_RESOURCE)
        m_hEffect = hResource;

    m_pEffect = g_ResourceManager.GetEffect(hResource);

    if (!m_pEffect || !m_pEffect->GetEffectThread())
    {
        m_pvCmdBuffer = NULL;
        return;
    }

    const CEffectThread &etSettings(*m_pEffect->GetEffectThread());

    m_pvCmdBuffer = &etSettings.m_vCmdBuffer;
    m_itCurrentCmd = etSettings.m_itCurrentCmd;
}


/*====================
  CEffectThread::AddCmd
  ====================*/
void    CEffectThread::AddCmd(IEffectCmd *pCmd)
{
    m_vCmdBuffer.push_back(pCmd);
    m_itCurrentCmd = m_vCmdBuffer.begin();
}


/*====================
  CEffectThread::Execute

  true is returned if the thread finishes, otherwise false (on a pause or such)
  ====================*/
bool    CEffectThread::Execute(uint uiMilliseconds)
{
    if (!m_pvCmdBuffer)
    {
        // if the effect has been unregistered, then pause until it's been reloaded.
        if (m_hEffect != INVALID_RESOURCE && g_ResourceManager.Get(m_hEffect) == NULL)
            return false;

        return true;
    }

    if (m_uiLastUpdateTime == INVALID_TIME)
    {
        if (uiMilliseconds < m_uiStartTime)
            return false;
        else
            m_uiLastUpdateTime = uiMilliseconds;
    }

    if (uiMilliseconds < m_uiLastUpdateTime)
    {
        if (GetEffect() != NULL)
            Console.Dev << _T("Rewinding effect ") << GetEffect()->GetPath() << _T(" to ") << uiMilliseconds << _T(" milliseconds") << newl;
        else
            Console.Dev << _T("Rewinding unknown effect to ") << uiMilliseconds << _T(" milliseconds") << newl;

        if (Rewind(uiMilliseconds))
            return true;
    }

    uint uiDeltaTime(uiMilliseconds > m_uiLastUpdateTime ? uiMilliseconds - m_uiLastUpdateTime : 0);

    if (m_uiWaitTime > 0)
    {
        if (uiDeltaTime > m_uiWaitTime)
        {
            uiDeltaTime -= m_uiWaitTime;
            m_uiWaitTime = 0;
        }
        else if (uiDeltaTime > 0)
        {
            m_uiWaitTime -= uiDeltaTime;
        }
    }

    if (m_uiWaitTime == 0)
    {
        if (m_itCurrentCmd == m_pvCmdBuffer->end())
            return true;

        while (m_itCurrentCmd != m_pvCmdBuffer->end() && (*m_itCurrentCmd)->Execute(this, uiMilliseconds))
        {
            ++m_itCurrentCmd;

            if (m_uiWaitTime > 0)
                break;
        }
    }

    // Update camera effects
    m_v3CameraOffset.Clear();
    m_v3CameraAngleOffset.Clear();

    UpdateCameraShake(uiMilliseconds);
    UpdateCameraKick(uiMilliseconds);
    UpdateOverlay(uiMilliseconds);

    if (uiMilliseconds > m_uiLastUpdateTime)
        m_uiLastUpdateTime = uiMilliseconds;

    return (m_itCurrentCmd == m_pvCmdBuffer->end() && m_uiWaitTime == 0);
}


/*====================
  CEffectThread::Cleanup

  delete dead particle system instances
  ====================*/
void    CEffectThread::Cleanup()
{
    InstanceMap::iterator it(m_mapInstances.begin());
    while (it != m_mapInstances.end())
    {
        if (it->second->IsDead())
        {
            K2_DELETE(it->second);
            STL_ERASE(m_mapInstances, it);
        }
        else
        {
            ++it;
        }
    }
}


/*====================
  CEffectThread::AddInstance
  ====================*/
void    CEffectThread::AddInstance(const tstring &sName, IEffectInstance *pParticleSystem)
{
    if (m_bExpire)
    {
        K2_DELETE(pParticleSystem);
        return;
    }

    InstanceMap::iterator findit(m_mapInstances.find(sName));

    if (findit != m_mapInstances.end())
    {
        K2_DELETE(findit->second);
        m_mapInstances.erase(findit);
    }

    m_mapInstances[sName] = pParticleSystem;
}


/*====================
  CEffectThread::GetInstance
  ====================*/
IEffectInstance*    CEffectThread::GetInstance(const tstring &sName)
{
    InstanceMap::iterator findit(m_mapInstances.find(sName));

    if (findit != m_mapInstances.end())
        return findit->second;
    else
        return NULL;
}


/*====================
  CEffectThread::GetBonePosition
  ====================*/
CVec3f  CEffectThread::GetBonePosition(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime)
{
    if (pSkeleton)
    {
        uint uiBone(pSkeleton->GetBone(sBone));

        if (uiBone != INVALID_BONE)
            return pSkeleton->GetBonePose(uiBone, uiTime);
    }

    return CVec3f(0.0f, 0.0f, 0.0f);
}


/*====================
  CEffectThread::GetBoneAxisPos
  ====================*/
void    CEffectThread::GetBoneAxisPos(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos)
{
    try
    {
        if (pSkeleton)
        {
            uint uiBone(pSkeleton->GetBone(sBone));

            if (uiBone != INVALID_BONE)
            {
                pSkeleton->GetBonePoseAxisPos(uiBone, uiTime, aOutAxis, v3OutPos);
                return;
            }
        }

        aOutAxis = CAxis(0.0f, 0.0f, 0.0f);
        v3OutPos = CVec3f(0.0f, 0.0f, 0.0f);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEffectThread::GetBoneAxisPos() - "), NO_THROW);
        return;
    }
}


/*====================
  CEffectThread::GetRandomPositionOnMesh
  ====================*/
CVec3f  CEffectThread::GetRandomPositionOnMesh(CModel *pModel, const tstring &sMesh)
{
    if (!pModel)
        return CVec3f(0.0f, 0.0f, 0.0f);

    IModel *pModelFile(pModel->GetModelFile());

    if (pModelFile && pModelFile->GetType() == MODEL_K2)
    {
        CK2Model *pK2Model(static_cast<CK2Model *>(pModelFile));

        CMesh *pMesh(pK2Model->GetMesh(sMesh));

        if (!pMesh || pMesh->numFaces == 0)
            return CVec3f(0.0f, 0.0f, 0.0f);

        if (pMesh->numFaces == 1)
        {
            float fA(M_Randnum(0.0f, 1.0f));
            float fB(M_Randnum(0.0f, 1.0f));

            if (fA + fB > 1.0f)
            {
                fA = 1.0f - fA;
                fB = 1.0f - fB;
            }

            CVec3f  v30(CVec3_cast(pMesh->verts[pMesh->faceList[0][0]]));
            CVec3f  v3A(CVec3_cast(pMesh->verts[pMesh->faceList[0][1]]) - v30);
            CVec3f  v3B(CVec3_cast(pMesh->verts[pMesh->faceList[0][2]]) - v30);

            return v30 + v3A * fA + v3B * fB;
        }
        else
        {
            // Determine cumulative area of the entire mesh
            // and distribute in vFaceAreas for weighted random
            // face picking
            vector<float> &vFaceAreas(pMesh->GetFaceAreas());
            float fCumArea(pMesh->GetCumArea());
            if (fCumArea == 0.0f)
            {
                vFaceAreas.resize(pMesh->numFaces);

                for (uint uiFace(0); uiFace != pMesh->numFaces; ++uiFace)
                {
                    vFaceAreas[uiFace] = fCumArea;
                    fCumArea += M_AreaOfTriangle(
                            CVec3_cast(pMesh->verts[pMesh->faceList[uiFace][0]]),
                            CVec3_cast(pMesh->verts[pMesh->faceList[uiFace][1]]),
                            CVec3_cast(pMesh->verts[pMesh->faceList[uiFace][2]]));
                }

                pMesh->SetCumArea(fCumArea);
            }

            float   fRand(M_Randnum(0.0f, fCumArea));

            // Binary search for the face we just picked
            uint    ui0(0);
            uint    ui1(pMesh->numFaces);
            uint    ui((ui1 - ui0) >> 1);

            while (ui != 0 && ui + 1 != ui1 && (vFaceAreas[ui] > fRand || vFaceAreas[ui + 1] < fRand))
            {
                if (vFaceAreas[ui] < fRand)
                    ui0 = ui;
                else
                    ui1 = ui;

                ui = (ui1 + ui0) >> 1;
            }

            if (ui == pMesh->numFaces)
                return CVec3f(0.0f, 0.0f, 0.0f);

            float fA(M_Randnum(0.0f, 1.0f));
            float fB(M_Randnum(0.0f, 1.0f));

            if (fA + fB > 1.0f)
            {
                fA = 1.0f - fA;
                fB = 1.0f - fB;
            }

            CVec3f  v30(CVec3_cast(pMesh->verts[pMesh->faceList[ui][0]]));
            CVec3f  v3A(CVec3_cast(pMesh->verts[pMesh->faceList[ui][1]]) - v30);
            CVec3f  v3B(CVec3_cast(pMesh->verts[pMesh->faceList[ui][2]]) - v30);

            return v30 + v3A * fA + v3B * fB;
        }
    }

    return CVec3f(0.0f, 0.0f, 0.0f);
}


/*====================
  CEffectThread::GetRandomPositionOnMesh
  ====================*/
CVec3f  CEffectThread::GetRandomPositionOnMesh(CModel *pModel, const tstring &sMesh, const CVec3f &v3Dir)
{
    if (!pModel)
        return CVec3f(0.0f, 0.0f, 0.0f);

    IModel *pModelFile(pModel->GetModelFile());

    if (pModelFile && pModelFile->GetType() == MODEL_K2)
    {
        CK2Model *pK2Model(static_cast<CK2Model *>(pModelFile));

        CMesh *pMesh(pK2Model->GetMesh(sMesh));

        if (!pMesh || pMesh->numFaces == 0)
            return CVec3f(0.0f, 0.0f, 0.0f);

        if (pMesh->numFaces == 1)
        {
            float fA(M_Randnum(0.0f, 1.0f));
            float fB(M_Randnum(0.0f, 1.0f));

            if (fA + fB > 1.0f)
            {
                fA = 1.0f - fA;
                fB = 1.0f - fB;
            }

            CVec3f  v30(CVec3_cast(pMesh->verts[pMesh->faceList[0][0]]));
            CVec3f  v3A(CVec3_cast(pMesh->verts[pMesh->faceList[0][1]]) - v30);
            CVec3f  v3B(CVec3_cast(pMesh->verts[pMesh->faceList[0][2]]) - v30);

            return v30 + v3A * fA + v3B * fB;
        }
        else
        {
            // Determine cumulative area of the entire mesh
            // and distribute in vFaceAreas for weighted random
            // face picking
            vector<float> &vFaceAreas(pMesh->GetFaceAreas());
            float fCumArea(pMesh->GetCumArea());
            if (fCumArea == 0.0f)
            {
                vFaceAreas.resize(pMesh->numFaces);

                for (uint uiFace(0); uiFace != pMesh->numFaces; ++uiFace)
                {
                    vFaceAreas[uiFace] = fCumArea;
                    fCumArea += M_AreaOfTriangle(
                            CVec3_cast(pMesh->verts[pMesh->faceList[uiFace][0]]),
                            CVec3_cast(pMesh->verts[pMesh->faceList[uiFace][1]]),
                            CVec3_cast(pMesh->verts[pMesh->faceList[uiFace][2]]));
                }

                pMesh->SetCumArea(fCumArea);
            }

            float   fRand(M_Randnum(0.0f, fCumArea));

            // Binary search for the face we just picked
            uint    ui0(0);
            uint    ui1(pMesh->numFaces);
            uint    ui((ui1 - ui0) >> 1);

            while (ui != 0 && ui + 1 != ui1 && (vFaceAreas[ui] > fRand || vFaceAreas[ui + 1] < fRand))
            {
                if (vFaceAreas[ui] < fRand)
                    ui0 = ui;
                else
                    ui1 = ui;

                ui = (ui1 + ui0) >> 1;
            }

            if (ui == pMesh->numFaces)
                return CVec3f(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity());

            CPlane  plane
            (
                CVec3_cast(pMesh->verts[pMesh->faceList[ui][0]]),
                CVec3_cast(pMesh->verts[pMesh->faceList[ui][1]]),
                CVec3_cast(pMesh->verts[pMesh->faceList[ui][2]]),
                true
            );

            if (M_Randnum(0.0f, 1.0f) > DotProduct(plane.v3Normal, v3Dir))
                return CVec3f(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity());


            float fA(M_Randnum(0.0f, 1.0f));
            float fB(M_Randnum(0.0f, 1.0f));

            if (fA + fB > 1.0f)
            {
                fA = 1.0f - fA;
                fB = 1.0f - fB;
            }

            CVec3f  v30(CVec3_cast(pMesh->verts[pMesh->faceList[ui][0]]));
            CVec3f  v3A(CVec3_cast(pMesh->verts[pMesh->faceList[ui][1]]) - v30);
            CVec3f  v3B(CVec3_cast(pMesh->verts[pMesh->faceList[ui][2]]) - v30);

            return v30 + v3A * fA + v3B * fB;
        }
    }

    return CVec3f(0.0f, 0.0f, 0.0f);
}


/*====================
  CEffectThread::GetRandomPositionWithNormalOnMesh
  ====================*/
CVec3f  CEffectThread::GetRandomPositionWithNormalOnMesh(CModel *pModel, const tstring &sMesh, CVec3f &v3Normal)
{
    if (!pModel)
    {
        v3Normal = CVec3f(0.0f, 0.0f, 1.0f);
        return CVec3f(0.0f, 0.0f, 0.0f);
    }

    IModel *pModelFile(pModel->GetModelFile());

    if (pModelFile && pModelFile->GetType() == MODEL_K2)
    {
        CK2Model *pK2Model(static_cast<CK2Model *>(pModelFile));

        CMesh *pMesh(pK2Model->GetMesh(sMesh));

        if (!pMesh || pMesh->numFaces == 0)
        {
            v3Normal = CVec3f(0.0f, 0.0f, 1.0f);
            return CVec3f(0.0f, 0.0f, 0.0f);
        }

        if (pMesh->numFaces == 1)
        {
            float fA(M_Randnum(0.0f, 1.0f));
            float fB(M_Randnum(0.0f, 1.0f));

            if (fA + fB > 1.0f)
            {
                fA = 1.0f - fA;
                fB = 1.0f - fB;
            }

            CVec3f  v30(CVec3_cast(pMesh->verts[pMesh->faceList[0][0]]));
            CVec3f  v3A(CVec3_cast(pMesh->verts[pMesh->faceList[0][1]]) - v30);
            CVec3f  v3B(CVec3_cast(pMesh->verts[pMesh->faceList[0][2]]) - v30);

            CPlane  plane
            (
                CVec3_cast(pMesh->verts[pMesh->faceList[0][0]]),
                CVec3_cast(pMesh->verts[pMesh->faceList[0][1]]),
                CVec3_cast(pMesh->verts[pMesh->faceList[0][2]]),
                true
            );
            v3Normal = plane.v3Normal;

            return v30 + v3A * fA + v3B * fB;
        }
        else
        {
            // Determine cumulative area of the entire mesh
            // and distribute in vFaceAreas for weighted random
            // face picking
            vector<float> &vFaceAreas(pMesh->GetFaceAreas());
            float fCumArea(pMesh->GetCumArea());
            if (fCumArea == 0.0f)
            {
                vFaceAreas.resize(pMesh->numFaces);

                for (uint uiFace(0); uiFace != pMesh->numFaces; ++uiFace)
                {
                    vFaceAreas[uiFace] = fCumArea;
                    fCumArea += M_AreaOfTriangle(
                            CVec3_cast(pMesh->verts[pMesh->faceList[uiFace][0]]),
                            CVec3_cast(pMesh->verts[pMesh->faceList[uiFace][1]]),
                            CVec3_cast(pMesh->verts[pMesh->faceList[uiFace][2]]));
                }

                pMesh->SetCumArea(fCumArea);
            }

            float   fRand(M_Randnum(0.0f, fCumArea));

            // Binary search for the face we just picked
            uint    ui0(0);
            uint    ui1(pMesh->numFaces);
            uint    ui((ui1 - ui0) >> 1);

            while (ui != 0 && ui + 1 != ui1 && (vFaceAreas[ui] > fRand || vFaceAreas[ui + 1] < fRand))
            {
                if (vFaceAreas[ui] < fRand)
                    ui0 = ui;
                else
                    ui1 = ui;

                ui = (ui1 + ui0) >> 1;
            }

            if (ui == pMesh->numFaces)
            {
                v3Normal = CVec3f(0.0f, 0.0f, 1.0f);
                return CVec3f(0.0f, 0.0f, 0.0f);
            }

            CPlane  plane
            (
                CVec3_cast(pMesh->verts[pMesh->faceList[ui][0]]),
                CVec3_cast(pMesh->verts[pMesh->faceList[ui][1]]),
                CVec3_cast(pMesh->verts[pMesh->faceList[ui][2]]),
                true
            );
            v3Normal = plane.v3Normal;

            float fA(M_Randnum(0.0f, 1.0f));
            float fB(M_Randnum(0.0f, 1.0f));

            if (fA + fB > 1.0f)
            {
                fA = 1.0f - fA;
                fB = 1.0f - fB;
            }

            CVec3f  v30(CVec3_cast(pMesh->verts[pMesh->faceList[ui][0]]));
            CVec3f  v3A(CVec3_cast(pMesh->verts[pMesh->faceList[ui][1]]) - v30);
            CVec3f  v3B(CVec3_cast(pMesh->verts[pMesh->faceList[ui][2]]) - v30);

            return v30 + v3A * fA + v3B * fB;
        }
    }

    return CVec3f(0.0f, 0.0f, 0.0f);
}


/*====================
  CEffectThread::GetRandomPositionOnSkeleton
  ====================*/
CVec3f  CEffectThread::GetRandomPositionOnSkeleton(CSkeleton *pSkeleton)
{
    if (!pSkeleton || !pSkeleton->IsValid() || pSkeleton->GetModel() == INVALID_RESOURCE)
        return CVec3f(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity());

    CModel *pModel(g_ResourceManager.GetModel(pSkeleton->GetModel()));
    CK2Model *pK2Model(static_cast<CK2Model *>(pModel->GetModelFile()));

    // Determine cumulative volume of the entire skeleton
    // and distribute in vFaceAreas for weighted random
    // face picking
    float fCumVolume(0.0f);
    vector<float>   vBoneVolumes(pSkeleton->GetNumBones());

    for (uint uiBone(0); uiBone != pSkeleton->GetNumBones(); ++uiBone)
    {
        vBoneVolumes[uiBone] = fCumVolume;

        CBone *pBone(pK2Model->GetBone(uiBone));
        if (pBone == NULL)
        {
            Console.Warn << _T("CEffectThread::GetRandomPositionOnSkeleton() - Invalid bone") << newl;
            continue;
        }

        if (pBone->GetName() == _T("Scene Root") ||
            pBone->GetName() == _T("Bip01") ||
            pBone->GetName()[0] == _T('_'))
            continue;

        uint uiParent(pBone->GetParentIndex());

        if (uiParent != INVALID_BONE)
        {
            CVec3f  v3Start(pSkeleton->GetBoneState(uiBone)->tm_local.pos);
            CVec3f  v3End(pSkeleton->GetBoneState(uiParent)->tm_local.pos);

            float fLength(Length(v3End - v3Start));

            fCumVolume += fLength * M_PI * (fLength/8) * (fLength/8);
        }
    }

    float   fRand(M_Randnum(0.0f, fCumVolume));

    // Binary search for the bone we just picked
    uint    ui0(0);
    uint    ui1(pSkeleton->GetNumBones());
    uint    ui((ui1 - ui0) / 2);

    while (ui != 0 && ui + 1 != ui1 && (vBoneVolumes[ui] > fRand || vBoneVolumes[ui + 1] < fRand))
    {
        if (vBoneVolumes[ui] < fRand)
            ui0 = ui;
        else
            ui1 = ui;

        ui = (ui1 + ui0) / 2;
    }

    if (ui == pSkeleton->GetNumBones())
        return CVec3f(0.0f, 0.0f, 0.0f);

    CBone *pBone(pK2Model->GetBone(ui));
    uint uiParent(pBone->GetParentIndex());

    SBoneState *pStart(pSkeleton->GetBoneState(ui));
    SBoneState *pEnd(pSkeleton->GetBoneState(uiParent));

    if (pStart == NULL)
    {
        Console.Warn << _T("CEffectThread::GetRandomPositionOnSkeleton() - Accessing invalid start bone on model ") << pK2Model->GetName() << _T(", accessing bone: ") << ui << _T(", model bones: ") << pK2Model->GetNumBones() << _T(", skeleton bones: ") << pSkeleton->GetNumBones() << newl;
        return CVec3f(0.0f, 0.0f, 0.0f);
    }

    if (pEnd == NULL)
    {
        Console.Warn << _T("CEffectThread::GetRandomPositionOnSkeleton() - Accessing invalid end bone on model ") << pK2Model->GetName() << _T(", accessing bone: ") << uiParent << _T(", model bones: ") << pK2Model->GetNumBones() << _T(", skeleton bones: ") << pSkeleton->GetNumBones() << newl;
        return CVec3f(0.0f, 0.0f, 0.0f);
    }

    CVec3f  v3Start(pSkeleton->GetBoneState(ui)->tm_local.pos);
    CVec3f  v3End(pSkeleton->GetBoneState(uiParent)->tm_local.pos);

    return CVec3f(LERP(M_Randnum(0.0f, 1.0f), v3Start, v3End));
}


/*====================
  CEffectThread::GetBoneTransform
  ====================*/
matrix43_t* CEffectThread::GetBoneTransform(CSkeleton *pSkeleton, const tstring &sBone)
{
    if (pSkeleton)
    {
        uint uiBone(pSkeleton->GetBone(sBone));

        if (uiBone != INVALID_BONE)
            return &pSkeleton->GetBoneState(pSkeleton->GetBone(sBone))->tm_local;
    }

    return NULL;
}


/*====================
  CEffectThread::GetVisibility
  ====================*/
bool    CEffectThread::GetVisibility(CSkeleton *pSkeleton, const tstring &sBone)
{
    if (pSkeleton)
    {
        uint uiBone(pSkeleton->GetBone(sBone));

        if (uiBone != INVALID_BONE)
            return pSkeleton->GetBoneState(pSkeleton->GetBone(sBone))->visibility != 0;
    }

    return false;
}


/*====================
  CEffectThread::GetSourceBonePosition
  ====================*/
CVec3f  CEffectThread::GetSourceBonePosition(const tstring &sBone, uint uiTime)
{
    return GetBonePosition(m_pSourceSkeleton, sBone, uiTime);
}


/*====================
  CEffectThread::GetSourceBoneAxisPos
  ====================*/
void    CEffectThread::GetSourceBoneAxisPos(const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos)
{
    GetBoneAxisPos(m_pSourceSkeleton, sBone, uiTime, aOutAxis, v3OutPos);
}


/*====================
  CEffectThread::GetSourceMeshPosition
  ====================*/
CVec3f  CEffectThread::GetSourceMeshPosition(const tstring &sMesh)
{
    return CVec3f(0.0f, 0.0f, 0.0f);
}


/*====================
  CEffectThread::GetSourceRandomPositionOnMesh
  ====================*/
CVec3f  CEffectThread::GetSourceRandomPositionOnMesh(const tstring &sMesh)
{
    return GetRandomPositionOnMesh(m_pSourceModel, sMesh);
}


/*====================
  CEffectThread::GetSourceRandomPositionOnMesh
  ====================*/
CVec3f  CEffectThread::GetSourceRandomPositionOnMesh(const tstring &sMesh, const CVec3f &v3Dir)
{
    return GetRandomPositionOnMesh(m_pSourceModel, sMesh, v3Dir);
}


/*====================
  CEffectThread::GetSourceRandomPositionWithNormalOnMesh
  ====================*/
CVec3f  CEffectThread::GetSourceRandomPositionWithNormalOnMesh(const tstring &sMesh, CVec3f &v3Normal)
{
    return GetRandomPositionWithNormalOnMesh(m_pSourceModel, sMesh, v3Normal);
}


/*====================
  CEffectThread::GetSourceRandomPositionOnSkeleton
  ====================*/
CVec3f  CEffectThread::GetSourceRandomPositionOnSkeleton()
{
    return GetRandomPositionOnSkeleton(m_pSourceSkeleton);
}


/*====================
  CEffectThread::GetSourceBoneTransform
  ====================*/
matrix43_t* CEffectThread::GetSourceBoneTransform(const tstring &sBone)
{
    return GetBoneTransform(m_pSourceSkeleton, sBone);
}


/*====================
  CEffectThread::GetSourceVisibility
  ====================*/
bool    CEffectThread::GetSourceVisibility(const tstring &sBone)
{
    return GetVisibility(m_pSourceSkeleton, sBone);
}


/*====================
  CEffectThread::SourceTransformPosition
  ====================*/
CVec3f  CEffectThread::SourceTransformPosition(const CVec3f &v3Pos)
{
    return TransformPoint(v3Pos, m_aSourceAxis, m_v3SourcePos);
}


/*====================
  CEffectThread::SourceTransformAxis
  ====================*/
CAxis   CEffectThread::SourceTransformAxis(const CAxis &aAxis)
{
    return CAxis(aAxis * m_aSourceAxis);
}


/*====================
  CEffectThread::GetTargetBonePosition
  ====================*/
CVec3f  CEffectThread::GetTargetBonePosition(const tstring &sBone, uint uiTime)
{
    return GetBonePosition(m_pTargetSkeleton, sBone, uiTime);
}


/*====================
  CEffectThread::GetTargetBoneAxisPos
  ====================*/
void    CEffectThread::GetTargetBoneAxisPos(const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos)
{
    GetBoneAxisPos(m_pTargetSkeleton, sBone, uiTime, aOutAxis, v3OutPos);
}


/*====================
  CEffectThread::GetTargetMeshPosition
  ====================*/
CVec3f  CEffectThread::GetTargetMeshPosition(const tstring &sMesh)
{
    return CVec3f(0.0f, 0.0f, 0.0f);
}


/*====================
  CEffectThread::GetTargetRandomPositionOnMesh
  ====================*/
CVec3f  CEffectThread::GetTargetRandomPositionOnMesh(const tstring &sMesh)
{
    return GetRandomPositionOnMesh(m_pTargetModel, sMesh);
}


/*====================
  CEffectThread::GetTargetRandomPositionOnMesh
  ====================*/
CVec3f  CEffectThread::GetTargetRandomPositionOnMesh(const tstring &sMesh, const CVec3f &v3Dir)
{
    return GetRandomPositionOnMesh(m_pTargetModel, sMesh, v3Dir);
}


/*====================
  CEffectThread::GetTargetRandomPositionWithNormalOnMesh
  ====================*/
CVec3f  CEffectThread::GetTargetRandomPositionWithNormalOnMesh(const tstring &sMesh, CVec3f &v3Normal)
{
    return GetRandomPositionWithNormalOnMesh(m_pTargetModel, sMesh, v3Normal);
}


/*====================
  CEffectThread::GetTargetRandomPositionOnSkeleton
  ====================*/
CVec3f  CEffectThread::GetTargetRandomPositionOnSkeleton()
{
    return GetRandomPositionOnSkeleton(m_pTargetSkeleton);
}


/*====================
  CEffectThread::GetTargetBoneTransform
  ====================*/
matrix43_t* CEffectThread::GetTargetBoneTransform(const tstring &sBone)
{
    return GetBoneTransform(m_pTargetSkeleton, sBone);
}


/*====================
  CEffectThread::GetTargetVisibility
  ====================*/
bool    CEffectThread::GetTargetVisibility(const tstring &sBone)
{
    return GetVisibility(m_pTargetSkeleton, sBone);
}


/*====================
  CEffectThread::TargetTransformPosition
  ====================*/
CVec3f  CEffectThread::TargetTransformPosition(const CVec3f &v3Pos)
{
    return TransformPoint(v3Pos, m_aTargetAxis, m_v3TargetPos);
}


/*====================
  CEffectThread::TargetTransformAxis
  ====================*/
CAxis   CEffectThread::TargetTransformAxis(const CAxis &aAxis)
{
    return CAxis(aAxis * m_aTargetAxis);
}


/*====================
  CEffectThread::GetCustomBonePosition
  ====================*/
CVec3f  CEffectThread::GetCustomBonePosition(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime)
{
    return GetBonePosition(pSkeleton, sBone, uiTime);
}


/*====================
  CEffectThread::GetCustomVisibility
  ====================*/
bool    CEffectThread::GetCustomVisibility(CSkeleton *pSkeleton, const tstring &sBone)
{
    return GetVisibility(pSkeleton, sBone);
}


/*====================
  CEffectThread::GetCustomBoneAxisPos
  ====================*/
void    CEffectThread::GetCustomBoneAxisPos(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos)
{
    GetBoneAxisPos(pSkeleton, sBone, uiTime, aOutAxis, v3OutPos);
}


/*====================
  CEffectThread::IsDeferred
  ====================*/
bool    CEffectThread::IsDeferred()
{
    if (!m_pEffect)
        return false;
    return m_pEffect->GetDeferred();
}


/*====================
  CEffectThread::IsPersistent
  ====================*/
bool    CEffectThread::IsPersistent()
{
    if (!m_pEffect)
        return false;
    return m_pEffect->GetPersistent();
}


/*====================
  CEffectThread::IsPausable
  ====================*/
bool    CEffectThread::IsPausable()
{
    if (!m_pEffect)
        return false;
    return m_pEffect->GetPausable();
}


/*====================
  CEffectThread::GetUseEntityEffectScale
  ====================*/
bool    CEffectThread::GetUseEntityEffectScale()
{
    if (!m_pEffect)
        return false;
    return m_pEffect->GetUseEntityEffectScale();
}

/*====================
  CEffectThread::StartCameraShake
  ====================*/
void    CEffectThread::StartCameraShake(uint uiMilliseconds, float fFalloffStart, float fFalloffEnd, float fFrequency, float fScale, uint uiDuration)
{
    if (m_bExpire)
        return;

    float fDistance(Distance(m_v3SourcePos, m_pCamera->GetOrigin()));

    fScale *= CLAMP(ILERP(fDistance, fFalloffEnd, fFalloffStart), 0.0f, 1.0f);

    // Only override an older shake if this one is stronger
    if (m_uiCameraShakeTime + m_uiCameraShakeDuration > uiMilliseconds)
    {
        if (fScale == 0.0f ||
            (fScale < m_v3CameraShake.x &&
            fScale < m_v3CameraShake.y &&
            fScale < m_v3CameraShake.z))
            return;
    }

    m_uiCameraShakeTime = uiMilliseconds;
    m_uiCameraShakeDuration = uiDuration;
    m_fCameraShakeFrequency = fFrequency;
    m_v3CameraShake = CVec3f(fScale, fScale, fScale);
}


/*====================
  CEffectThread::UpdateCameraShake
  ====================*/
void    CEffectThread::UpdateCameraShake(uint uiMilliseconds)
{
    if (m_uiCameraShakeTime + m_uiCameraShakeDuration <= uiMilliseconds || m_uiCameraShakeTime > uiMilliseconds)
        return;

    float t(float(uiMilliseconds - m_uiCameraShakeTime) / m_uiCameraShakeDuration);

    CVec3f v3Offset
    (
        M_Randnum(-m_v3CameraShake.x, m_v3CameraShake.x) * t,
        M_Randnum(-m_v3CameraShake.y, m_v3CameraShake.y) * t,
        M_Randnum(-m_v3CameraShake.z, m_v3CameraShake.z) * t
    );
    
    v3Offset.SetLength(MAX(MAX(m_v3CameraShake.x, m_v3CameraShake.y), m_v3CameraShake.z) * t);

    m_v3CameraOffset += v3Offset;
}


/*====================
  CEffectThread::StartCameraKick
  ====================*/
void    CEffectThread::StartCameraKick(uint uiMilliseconds, const CRangef &rfPitch, float fTurn, const CRangef &rfBack, const CRangef &rfUp, const CRangef &rfRight, float fHalfLife)
{
    if (m_bExpire)
        return;

    m_uiCameraKickTime = uiMilliseconds;
    m_fCameraKickHalfLife = fHalfLife;

    float fRandTurn(M_Randnum(-fTurn, fTurn));
    m_v3CameraKickAngles = CVec3f(rfPitch, -fRandTurn, fRandTurn);
    m_v3CameraKickPosition = CVec3f(rfRight, rfUp, -rfBack);
}


/*====================
  CEffectThread::UpdateCameraKick
  ====================*/
void    CEffectThread::UpdateCameraKick(uint uiMilliseconds)
{
    if (m_uiCameraKickTime == INVALID_TIME || m_uiCameraKickTime > uiMilliseconds)
        return;

    if (fabs(m_v3CameraKickAngles.x) < 0.01f && fabs(m_v3CameraKickAngles.y) < 0.01f && fabs(m_v3CameraKickAngles.z) < 0.01f &&
        fabs(m_v3CameraKickPosition.x) < 0.01f && fabs(m_v3CameraKickPosition.y) < 0.01f && fabs(m_v3CameraKickPosition.z) < 0.01f)
    {
        m_uiCameraKickTime = INVALID_TIME;
        return;
    }

    float fDeltaTime(MsToSec(uiMilliseconds - m_uiCameraKickTime));

    if (fDeltaTime > 0.0f)
    {
        m_v3CameraKickAngles = DECAY(m_v3CameraKickAngles, V3_ZERO, m_fCameraKickHalfLife, fDeltaTime);
        m_v3CameraKickPosition = DECAY(m_v3CameraKickPosition, V3_ZERO, m_fCameraKickHalfLife, fDeltaTime);
    }

    if (m_pCamera)
    {
        m_v3CameraAngleOffset += m_v3CameraKickAngles;
        m_v3CameraOffset += m_v3CameraKickPosition;
    }
    else
    {
        m_v3CameraAngleOffset += m_v3CameraKickAngles;
        m_v3CameraOffset += m_v3CameraKickPosition;
    }

    m_uiCameraKickTime = uiMilliseconds;
}


/*====================
  CEffectThread::StartOverlay
  ====================*/
void    CEffectThread::StartOverlay(uint uiMilliseconds, const CTemporalPropertyv3 &tv3Color,
                                  const CTemporalPropertyf &tfAlpha, ResHandle hMaterial, uint uiDuration)
{
    if (m_bExpire)
        return;

    m_bActiveOverlay = true;
    m_uiOverlayStartTime = uiMilliseconds;
    m_uiOverlayDuration = uiDuration;
    m_tv3OverlayColor = tv3Color;
    m_tfOverlayAlpha = tfAlpha;
    m_hOverlayMaterial = hMaterial;
}


/*====================
  CEffectThread::UpdateOverlay
  ====================*/
void    CEffectThread::UpdateOverlay(uint uiMilliseconds)
{
    if (m_uiOverlayStartTime + m_uiOverlayDuration <= uiMilliseconds)
    {
        m_bActiveOverlay = false;
        return;
    }

    if (m_uiOverlayStartTime > uiMilliseconds)
        return;

    float fLerp(float(uiMilliseconds - m_uiOverlayStartTime) / m_uiOverlayDuration);
    float fTime(MsToSec(uiMilliseconds - m_uiOverlayStartTime));
    m_v4OverlayColor = CVec4f(m_tv3OverlayColor.Evaluate(fLerp, fTime), m_tfOverlayAlpha.Evaluate(fLerp, fTime));
}


/*====================
  CEffectThread::StartCameraShake2
  ====================*/
void    CEffectThread::StartCameraShake2(uint uiMilliseconds, float fFalloffStart, float fFalloffEnd, float fFrequency, float fScale, uint uiDuration)
{
    if (m_bExpire)
        return;

    float fDistance(Distance(m_v3SourcePos, m_pCamera->GetCenter()));

    fScale *= CLAMP(ILERP(fDistance, fFalloffEnd, fFalloffStart), 0.0f, 1.0f);

    // Only override an older shake if this one is stronger
    if (m_uiCameraShakeTime + m_uiCameraShakeDuration > uiMilliseconds)
    {
        if (fScale == 0.0f ||
            (fScale < m_v3CameraShake.x &&
            fScale < m_v3CameraShake.y &&
            fScale < m_v3CameraShake.z))
            return;
    }

    m_uiCameraShakeTime = uiMilliseconds;
    m_uiCameraShakeDuration = uiDuration;
    m_fCameraShakeFrequency = fFrequency;
    m_v3CameraShake = CVec3f(fScale, fScale, fScale);
}


/*====================
  CEffectThread::Rewind

  Rewind the effect thread to a specific time.
  Rewinding to before start time will kill the effect,
  any other time will rebuild the effect and resimulate
  to the specified time
  ====================*/
bool    CEffectThread::Rewind(uint uiMilliseconds)
{
    if (uiMilliseconds < m_uiStartTime)
    {
        m_itCurrentCmd = m_pvCmdBuffer->end();
        return true;
    }

    // Delete all particle system instances
    InstanceMap::iterator itEnd(m_mapInstances.end());
    for (InstanceMap::iterator it(m_mapInstances.begin()); it != itEnd; ++it)
        K2_DELETE(it->second);
    m_mapInstances.clear();

    m_uiLastUpdateTime = m_uiStartTime;
    m_itCurrentCmd = m_itStartCmd;

    return false;
}


/*====================
  CEffectThread::Expire
  ====================*/
void    CEffectThread::Expire(uint uiMilliseconds)
{
    m_bExpire = true;

    InstanceMap::iterator itEnd(m_mapInstances.end());
    for (InstanceMap::iterator it(m_mapInstances.begin()); it != itEnd; ++it)
        it->second->Expire(uiMilliseconds);
}
