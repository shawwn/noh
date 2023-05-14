// (C)2008 S2 Games
// c_gfxmodels.cpp
//
// Models
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfxmodels.h"

#include "c_gfxutils.h"
#include "c_bonelist.h"
#include "c_treemodeldef.h"
#include "c_treescenemanager.h"
#include "c_renderlist.h"
#include "c_meshrenderer.h"

#include "../public/blendedlink_t.h"
#include "../k2/c_k2model.h"
#include "../k2/c_treemodel.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_bone.h"
#include "../k2/c_mesh.h"
#include "../k2/c_camera.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_material.h"
#include "../k2/c_model.h"
#include "../k2/c_skin.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_convexpolyhedron.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
SINGLETON_INIT(CGfxModels)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGfxModels *GfxModels(CGfxModels::GetInstance());

CVec4f  g_vBoneData[MAX_BONE_TABLE_SIZE];
int     g_iNumActiveBones;
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOLF  (vid_lodUse, true, CVAR_SAVECONFIG);
CVAR_INTF   (vid_lodBias, 0, CVAR_SAVECONFIG);
CVAR_INTF   (vid_lodCurve, 2, CVAR_SAVECONFIG);
CVAR_INTF   (vid_lodForce, -1, CVAR_SAVECONFIG);
CVAR_BOOL   (vid_drawModelBounds, false);
CVAR_BOOL   (vid_drawModelSurfs, false);
CVAR_BOOL   (vid_treeRotate, false);

EXTERN_CVAR_FLOAT(vid_treeWindStrength);
EXTERN_CVAR_BOOL(vid_meshGPUDeform);
EXTERN_CVAR_BOOL(vid_drawBones);
EXTERN_CVAR_BOOL(vid_drawBoneNames);
//=============================================================================

/*====================
  CGfxModels::~CGfxModels
  ====================*/
CGfxModels::~CGfxModels()
{
}


/*====================
  CGfxModels::CGfxModels
  ====================*/
CGfxModels::CGfxModels()
{
    MemManager.Set(VBMeshes, 0, sizeof(VBMeshes));
    MemManager.Set(DBMeshes, 0, sizeof(DBMeshes));
    MemManager.Set(IBMeshes, 0, sizeof(IBMeshes));
}


/*====================
  CGfxModels::AddK2ModelToLists
  ====================*/
bool    CGfxModels::AddK2ModelToLists(CK2Model *pModel, const CSceneEntity &cEntity, EMaterialPhase ePhase)
{
    if (pModel == nullptr)
        return false;

    if (vid_lodUse && pModel->GetNumLods() && pModel->GetLodDistance() > 0.0f)
    {
        if (vid_lodForce != -1)
        {
            if (vid_lodForce != 0)
                pModel = pModel->GetLod(CLAMP<int>(vid_lodForce - 1, 0, pModel->GetNumLods() - 1));
        }
        else
        {
            float fDistance(g_pCam->GetLodDistance() == 0.0f ? Distance(g_vCamOrigin, cEntity.GetPosition()) : g_pCam->GetLodDistance());
            float fLodFactor(fDistance / pModel->GetLodDistance());
            
            if (fLodFactor > 1.0f)
                pModel = pModel->GetLod(CLAMP<int>(MAX(INT_FLOOR(log(fLodFactor) / log(float(vid_lodCurve))), 0) + vid_lodBias, 0, pModel->GetNumLods() - 1));
        }
    }

    if (ePhase == PHASE_COLOR && (vid_drawModelBounds || cEntity.flags & SCENEENT_SHOW_BOUNDS))
    {
        D3DXMATRIXA16 mWorldTranslation;
        D3DXMATRIXA16 mWorldScaling;
        D3DXMATRIXA16 mWorldRotation;

        CVec3f v3Pos(cEntity.GetPosition());
        D3DXMatrixTranslation(&mWorldTranslation, v3Pos[X], v3Pos[Y], v3Pos[Z]);
        GfxUtils->AxisToMatrix(cEntity.axis, &mWorldRotation);

        if (cEntity.scale != 1.0f)
            D3DXMatrixScaling(&mWorldScaling, cEntity.scale, cEntity.scale, cEntity.scale);
        else
            D3DXMatrixIdentity(&mWorldScaling);

        D3DXMATRIXA16 mWorld = mWorldScaling * mWorldRotation * mWorldTranslation;

        Gfx3D->AddBox(pModel->GetBounds(), cEntity.color, mWorld);
    }

    if (ePhase == PHASE_COLOR && cEntity.skeleton && cEntity.skeleton->IsValid() && vid_drawBones)
    {
        D3DXMATRIXA16 mWorldTranslation;
        D3DXMATRIXA16 mWorldScaling;
        D3DXMATRIXA16 mWorldRotation;

        CVec3f v3Pos(cEntity.GetPosition());
        D3DXMatrixTranslation(&mWorldTranslation, v3Pos[X], v3Pos[Y], v3Pos[Z]);
        GfxUtils->AxisToMatrix(cEntity.axis, &mWorldRotation);

        if (cEntity.scale != 1.0f)
            D3DXMatrixScaling(&mWorldScaling, cEntity.scale, cEntity.scale, cEntity.scale);
        else
            D3DXMatrixIdentity(&mWorldScaling);

        D3DXMATRIXA16 mWorld = mWorldScaling * mWorldRotation * mWorldTranslation;

        ResHandle hFont(g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));

        CK2Model *pBaseModel(pModel->GetBaseLod() ? pModel->GetBaseLod() : pModel);

        for (uint uiBone(0); uiBone < pBaseModel->GetNumBones(); ++uiBone)
        {
            CVec3f  v3Pos(cEntity.skeleton->GetBoneState(uiBone)->tm_local.pos);

            v3Pos = GfxUtils->TransformPoint(v3Pos, mWorld);

            if (cEntity.skeleton->GetBoneState(uiBone)->visibility)
                Gfx3D->AddPoint(v3Pos, CVec4f(1.0f, 0.0f, 0.0f, 1.0f));
            else
                Gfx3D->AddPoint(v3Pos, CVec4f(0.5f, 0.5f, 0.5f, 1.0f));

            CBone *pBone(pBaseModel->GetBone(uiBone));
            uint uiParent(pBone->GetParentIndex());

            if (uiParent != INVALID_BONE)
            {
                CVec3f  v3End(cEntity.skeleton->GetBoneState(uiParent)->tm_local.pos);

                v3End = GfxUtils->TransformPoint(v3End, mWorld);

                Gfx3D->AddLine(v3Pos, v3End, CVec4f(1.0f, 1.0f, 0.0f, 1.0f));
            }

            if (vid_drawBoneNames)
            {
                CVec2f  v2ScreenPos;

                if (g_pCam->WorldToScreen(v3Pos, v2ScreenPos))
                {
                    Draw2D.SetColor(CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
                    Draw2D.String(floor(v2ScreenPos.x), floor(v2ScreenPos.y), pBaseModel->GetBoneName(uiBone), hFont);
                }
            }
        }
    }

    SkinHandle hSkin(cEntity.hSkin);

    // make sure skin index is valid
    if (hSkin >= pModel->NumSkins() || hSkin < 0)
        hSkin = 0;

    // make sure the materials we'll be using for the model are loaded
    pModel->LoadSkinMaterials(hSkin);

    if (cEntity.skeleton)
    {
        if (pModel->GetNumBones() != 0 && cEntity.skeleton->IsValid())
        {
            CK2Model *pSkeletonModel = static_cast<CK2Model *>(g_ResourceManager.GetModel(cEntity.skeleton->GetModel())->GetModelFile());
            if (pSkeletonModel != pModel)
            {
                lCustomMappings.push_back(vector<int>());

                vector<int> &vCustomMapping(lCustomMappings.back());
                vCustomMapping.resize(pModel->GetNumBones());

                // we're using a skeleton from a different model
                // some scene entities do this (character armor for instance)

                // for the model to be deformed correctly, we'll need
                // to create a custom bone mapping.  This gets slow if
                // lots of models are doing this
                for (uint i = 0; i < pModel->GetNumBones(); ++i)
                {
                    vCustomMapping[i] = -1;

                    for (uint j = 0; j < pSkeletonModel->GetNumBones(); ++j)
                    {
                        if (pModel->GetBoneName(i) == pSkeletonModel->GetBoneName(j))
                        {
                            vCustomMapping[i] = int(j);
                            break;
                        }
                    }
                }

                cEntity.custom_mapping = (int *)&vCustomMapping[0];
            }
            else
            {
                if (pSkeletonModel->GetNumBones() != pModel->GetNumBones())
                {
                    Console.Warn << _T("CGfxModels::AddK2ModelToLists() - invalid skeleton (skeleton->model != cEntity.model)") << newl;
                    return false;
                }
            }
        }
    }

    D3DXMATRIXA16 mWorldTranslation;
    D3DXMATRIXA16 mWorldScaling;
    D3DXMATRIXA16 mWorldRotation;

    CVec3f v3Pos(cEntity.GetPosition());
    D3DXMatrixTranslation(&mWorldTranslation, v3Pos.x, v3Pos.y, v3Pos.z);
    GfxUtils->AxisToMatrix(cEntity.axis, &mWorldRotation);

    if (cEntity.scale != 1.0f)
        D3DXMatrixScaling(&mWorldScaling, cEntity.scale, cEntity.scale, cEntity.scale);
    else
        D3DXMatrixIdentity(&mWorldScaling);

    D3DXMATRIXA16 mWorld = mWorldScaling * mWorldRotation * mWorldTranslation;

    for (uint uiMesh(0); uiMesh < pModel->GetNumMeshes(); ++uiMesh)
    {
        CMesh *pMesh = pModel->GetMesh(uiMesh);

        if (pMesh->renderflags & MESH_INVIS)
            continue;

        int *pMapping(pMesh->GetModel()->GetBoneMapping());

        if (cEntity.skeleton && cEntity.skeleton->IsValid() && pMesh->bonelink != -1 && pMapping[pMesh->bonelink] != -1)
        {
            int iCurrentSkelbone = pMapping[pMesh->bonelink];

            assert(iCurrentSkelbone < int(cEntity.skeleton->GetNumBones()));

            // Mesh is invisible on this frame
            if (iCurrentSkelbone != -1 && cEntity.skeleton->GetBoneState(iCurrentSkelbone)->visibility == 0)
                continue;
        }

        ResHandle hMaterial(pModel->GetSkin(hSkin)->GetMaterial(uiMesh));
        if (hMaterial == INVALID_RESOURCE)
            continue;

        if (cEntity.flags & SCENEENT_SINGLE_MATERIAL)
            hMaterial = cEntity.hSkin;

        if (hMaterial == INVALID_RESOURCE || GfxUtils->GetMaterial(hMaterial).HasPhase(ePhase))
            g_RenderList.Add(K2_NEW(ctx_GL2,    CMeshRenderer)(hMaterial, cEntity, pMesh, mWorld, mWorldRotation));
    }

    return true;
}


/*====================
  CGfxModels::AddTreeModelToLists
  ====================*/
bool    CGfxModels::AddTreeModelToLists(CTreeModel *pTreeModel, const CSceneEntity &cEntity, EMaterialPhase ePhase)
{
    D3DXMATRIXA16 x_mWorld;
    D3DXMATRIXA16 x_mWorldViewProj;
    D3DXMATRIXA16 x_mWorldInverse;
    D3DXMATRIXA16 x_mWorldInverseRotate;

    if (vid_treeRotate)
    {
        // Matrix calculations
        D3DXMATRIXA16 mWorldTranslation;
        D3DXMATRIXA16 mWorldScaling;
        D3DXMATRIXA16 mWorldRotation;

        const CVec3f &v3Pos(cEntity.GetPosition());
        D3DXMatrixTranslation(&mWorldTranslation, v3Pos.x, v3Pos.y, v3Pos.z);
        D3DXMatrixScaling(&mWorldScaling, cEntity.scale, cEntity.scale, cEntity.scale);
        GfxUtils->AxisToMatrix(cEntity.axis, &mWorldRotation);

        x_mWorld = mWorldScaling * mWorldRotation * mWorldTranslation;
        x_mWorldViewProj = x_mWorld * g_mViewProj;

        D3DXMATRIXA16 mScaleInverse;
        D3DXMatrixScaling(&mScaleInverse, 1.0f / cEntity.scale, 1.0f / cEntity.scale, 1.0f / cEntity.scale);

        D3DXMATRIXA16 mRotationInverse;
        D3DXMatrixTranspose(&mRotationInverse, &mWorldRotation);

        D3DXMATRIXA16 mTranslationInverse;
        D3DXMatrixTranslation(&mTranslationInverse, -v3Pos.x, -v3Pos.y, -v3Pos.z);
        
        x_mWorldInverse = mTranslationInverse * mRotationInverse * mScaleInverse;
        D3DXMatrixTranspose(&x_mWorldInverseRotate, &mWorldRotation);
    }
    else
    {
        // Matrix calculations
        const CVec3f &v3Pos(cEntity.GetPosition());

        x_mWorld = D3DXMATRIXA16(cEntity.scale, 0.0f,          0.0f,          0.0f,
                                 0.0f,          cEntity.scale, 0.0f,          0.0f,
                                 0.0f,          0.0f,          cEntity.scale, 0.0f,
                                 v3Pos.x,       v3Pos.y,       v3Pos.z,       1.0f);
        
        x_mWorldViewProj = x_mWorld * g_mViewProj;

        x_mWorldInverse = D3DXMATRIXA16( 1.0f / cEntity.scale,     0.0f,                     0.0f,                    0.0f,
                                         0.0f,                     1.0f / cEntity.scale,     0.0f,                    0.0f,
                                         0.0f,                     0.0f,                     1.0f / cEntity.scale,    0.0f,
                                        -v3Pos.x / cEntity.scale, -v3Pos.y / cEntity.scale, -v3Pos.z / cEntity.scale, 1.0f);
        
        D3DXMatrixIdentity(&x_mWorldInverseRotate);
    }

    const CTreeModelDef *pTreeDef(g_pTreeSceneManager->GetDefinition(pTreeModel->GetVidDefIndex()));

    // Update the camera
    {
        PROFILE("Update Camera");

        CVec3f v3CamPos;
        CVec3f v3CamDir;
        if (ePhase == PHASE_SHADOW)
            v3CamDir = SceneManager.GetSunPos();
        else
            v3CamDir = -SceneManager.GetCameraDir();

        if (g_pCam->GetLodDistance() > 0.0f)
            v3CamPos = cEntity.GetPosition() + v3CamDir * g_pCam->GetLodDistance();
        else
            v3CamPos = SceneManager.GetCameraPos();

        D3DXVECTOR4 d3dvCamPos(v3CamPos.x, v3CamPos.y, v3CamPos.z, 1.0f);
        D3DXVec4Transform(&d3dvCamPos, &d3dvCamPos, &x_mWorldInverse);

        d3dvCamPos.x /= d3dvCamPos.w;
        d3dvCamPos.y /= d3dvCamPos.w;
        d3dvCamPos.z /= d3dvCamPos.w;

        D3DXVECTOR3 d3dvCamDir(v3CamDir.x, v3CamDir.y, v3CamDir.z);
        D3DXVec3TransformNormal(&d3dvCamDir, &d3dvCamDir, &x_mWorldInverseRotate);

        CVec3f v3CameraPos = CVec3_cast(d3dvCamPos);
        CVec3f v3CameraDir = CVec3_cast(d3dvCamDir);
        
        // Update tree wind/camera/time settings
        CTreeModel::UpdateCamera(v3CameraPos, v3CameraDir);
        
        pTreeDef->SetWindStrength(vid_treeWindStrength);
        pTreeDef->ComputeLODLevel();
    }

    g_RenderList.Add(K2_NEW(ctx_GL2,    CTreeBranchRenderer)(cEntity, pTreeDef, x_mWorldViewProj, x_mWorld, x_mWorldInverseRotate));
    g_RenderList.Add(K2_NEW(ctx_GL2,    CTreeFrondRenderer)(cEntity, pTreeDef, x_mWorldViewProj, x_mWorld, x_mWorldInverseRotate));
    g_RenderList.Add(K2_NEW(ctx_GL2,    CTreeLeafRenderer)(cEntity, pTreeDef, x_mWorldViewProj, x_mWorld, x_mWorldInverseRotate));

    STreeBillboardData avBillboards[2];
    avBillboards[0].m_bActive = avBillboards[1].m_bActive = false;
    pTreeModel->GetBillboardData(avBillboards);

    dword dwColor(((cEntity.flags & SCENEENT_SOLID_COLOR) ? cEntity.color : WHITE).GetAsDWordGL());

    for (int i(0); i < 2; ++i)
    {
        if (avBillboards[i].m_bActive)
        {
            Gfx3D->AddTreeBillboard
            (
                CVec3f(avBillboards[i].m_pCoords[0], avBillboards[i].m_pCoords[1], avBillboards[i].m_pCoords[2]),
                CVec3f(avBillboards[i].m_pCoords[3], avBillboards[i].m_pCoords[4], avBillboards[i].m_pCoords[5]),
                CVec3f(avBillboards[i].m_pCoords[6], avBillboards[i].m_pCoords[7], avBillboards[i].m_pCoords[8]),
                CVec3f(avBillboards[i].m_pCoords[9], avBillboards[i].m_pCoords[10], avBillboards[i].m_pCoords[11]),
                dwColor, dwColor, dwColor, dwColor,
                CVec2f(avBillboards[i].m_pTexCoords[0], avBillboards[i].m_pTexCoords[1]),
                CVec2f(avBillboards[i].m_pTexCoords[2], avBillboards[i].m_pTexCoords[3]),
                CVec2f(avBillboards[i].m_pTexCoords[4], avBillboards[i].m_pTexCoords[5]),
                CVec2f(avBillboards[i].m_pTexCoords[6], avBillboards[i].m_pTexCoords[7]),
                pTreeDef->GetBillboardMaterial(),
                avBillboards[i].m_dwAlphaTest,
                x_mWorld
            );
        }
    }

    return true;
}


/*====================
  CGfxModels::RegisterModel
  ====================*/
int     CGfxModels::RegisterModel(class CModel *pModel)
{
    switch (pModel->GetModelFile()->GetType())
    {
    case MODEL_K2:
        return RegisterK2Model(static_cast<CK2Model *>(pModel->GetModelFile()));

    case MODEL_SPEEDTREE:
        return RegisterTreeModel(static_cast<CTreeModel *>(pModel->GetModelFile()));

    default:
        Console.Err << _T("CGfxModels::RegisterModel() - Unknown model type") << newl;
        return false;
    }
}


/*====================
  CGfxModels::UnregisterModel
  ====================*/
void    CGfxModels::UnregisterModel(class CModel *pModel)
{
    IModel* pModelFile(pModel->GetModelFile());
    if (pModelFile != nullptr)
    {
        switch (pModelFile->GetType())
        {
        case MODEL_K2:
            UnregisterK2Model(static_cast<CK2Model *>(pModelFile));
            break;

        case MODEL_SPEEDTREE:
            UnregisterTreeModel(static_cast<CTreeModel *>(pModelFile));
            break;

        default:
            Console.Err << _T("CGfxModels::UnregisterModel() - Unknown model type") << newl;
        }
    }
}


/*====================
  CGfxModels::AddModelToLists
  ====================*/
bool    CGfxModels::AddModelToLists(const CSceneEntity &cEntity, EMaterialPhase ePhase)
{
    try
    {
        // Grab the scene object's model and decide what to do with it
        CModel* pModelResource(g_ResourceManager.GetModel(cEntity.hRes));
        if (pModelResource == nullptr)
            EX_ERROR(_T("Invalid model handle"));

        IModel *pModel(pModelResource->GetModelFile());
        if (pModel == nullptr)
            EX_ERROR(_T("Couldn't retrieve model data"));

        switch (pModel->GetType())
        {
        case MODEL_K2:
            return AddK2ModelToLists(static_cast<CK2Model *>(pModel), cEntity, ePhase);

        case MODEL_SPEEDTREE:
            return AddTreeModelToLists(static_cast<CTreeModel *>(pModel), cEntity, ePhase);

        default:
            EX_ERROR(_T("Unknown model type"));
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGfxModels::AddModelToLists() - "), NO_THROW);
        return false;
    }
}


/*====================
  CGfxModels::RegisterK2Model
  ====================*/
bool    CGfxModels::RegisterK2Model(CK2Model *pModel)
{
    // Register each mesh
    for (uint uiMesh(0); uiMesh < pModel->GetNumMeshes(); ++uiMesh)
    {
        CMesh *pMesh(pModel->GetMesh(uiMesh));

        if (pMesh->renderflags & MESH_INVIS)
            continue;

        // Find empty mesh slot
        int i;
        for (i = 0; i < MAX_MESHES; ++i)
        {
            if (VBMeshes[i] == 0)
                break;
        }

        if (i == MAX_MESHES)
        {
            K2System.Error(_T("MAX_MESHES"));
            return false;
        }

        // Calculate mesh bone index remapping so bone deform shaders only needs to send
        // bones that are used by this mesh to the shader
        CBoneList   &oBoneList = BoneRemap[i];

        if (pMesh->blendedLinks)
        {
            oBoneList.SetNumModelBones(pModel->GetNumBones());

            for (int v = 0; v < pMesh->num_verts; ++v)
            {
                const SBlendedLink &link = pMesh->blendedLinks[v];

                for (int b = 0; b < link.num_weights; ++b)
                    oBoneList.AddBoneInstance(link.indexes[b]);
            }
        }
        else if (pMesh->singleLinks)
        {
            oBoneList.SetNumModelBones(pModel->GetNumBones());

            for (int v = 0; v < pMesh->num_verts; ++v)
            {
                const singleLink_t link = pMesh->singleLinks[v];

                oBoneList.AddBoneInstance(link);
            }
        }

        if (oBoneList.GetNumBones() > MAX_GPU_BONES)
            Console.Warn << QuoteStr(pMesh->GetName()) << _T(" has ") << XtoA(oBoneList.GetNumBones()) << _T(" bones") << newl;

        int iNumTexcoords = 0;

        for (int j = 0; j < MAX_UV_CHANNELS; ++j)
        {
            if (pMesh->tverts[j])
                iNumTexcoords = j + 1;
            else
                break;
        }

        pMesh->vertexStride = 12;

        if (pMesh->normals)
            pMesh->vertexStride += 12;

        int iNumTangents = 0;

        for (int j = 0; j < MAX_UV_CHANNELS; ++j)
        {
            if (pMesh->tangents[j])
                iNumTangents = j + 1;
            else
                break;
        }

        iNumTangents = MIN(iNumTangents, 1);

        int iNumData = 0;

        // Bone indices and weights
        if (vid_meshGPUDeform && (pMesh->blendedLinks || pMesh->singleLinks))
            iNumData = 2;

        pMesh->vertexStride += iNumTexcoords * sizeof(CVec2f);
        pMesh->vertexStride += iNumTangents * sizeof(CVec4f);
        pMesh->vertexStride += iNumData * sizeof(CVec4b);

        if (pMesh->colors[0])
            pMesh->vertexStride += 4;

        if (pMesh->colors[1])
            pMesh->vertexStride += 4;

        int v_offset = 0;
        int n_offset = v_offset + sizeof(CVec3f);

        int c1_offset = n_offset + ((pMesh->normals) ? sizeof(CVec3f) : 0);
        int c2_offset = c1_offset + ((pMesh->colors[0]) ? sizeof(dword) : 0);

        int t_offset = c2_offset + ((pMesh->colors[1]) ? sizeof(dword) : 0);

        int tan_offset = t_offset + (iNumTexcoords * sizeof(CVec2f));

        int indices_offset = tan_offset + (iNumTangents * sizeof(CVec4f));
        int weights_offset = indices_offset + sizeof(CVec4b);

        // Tell OpenGL which custom attributes we're offering
        if (iNumTangents > 0)
            pMesh->mapAttributes[_T("a_vTangent")] = SVertexAttribute(GL_FLOAT, 4, tan_offset, false);

        if (vid_meshGPUDeform && (pMesh->blendedLinks || pMesh->singleLinks))
        {
            pMesh->mapAttributes[_T("a_vBoneIndex")] = SVertexAttribute(GL_UNSIGNED_BYTE, 4, indices_offset, false);
            pMesh->mapAttributes[_T("a_vBoneWeight")] = SVertexAttribute(GL_UNSIGNED_BYTE, 4, weights_offset, true);
        }

        //
        // Initialize Static Vertex Buffer
        //

        glGenBuffersARB(1, &VBMeshes[i]);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBMeshes[i]);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, pMesh->num_verts * pMesh->vertexStride, nullptr, GL_STATIC_DRAW_ARB);

        byte* pVertices = (byte*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);

        for (int v = 0; v < pMesh->num_verts; ++v)
        {
            {
                vec3_t  *p = (vec3_t *)(&pVertices[v * pMesh->vertexStride + v_offset]);
                M_CopyVec3(pMesh->verts[v], *p);
            }

            if (pMesh->normals)
            {
                vec3_t  *p = (vec3_t *)(&pVertices[v * pMesh->vertexStride + n_offset]);
                M_CopyVec3(pMesh->normals[v], *p);
            }

            if (pMesh->colors[0])
            {
                dword   *p = (dword *)(&pVertices[v * pMesh->vertexStride + c1_offset]);
                *p = D3DCOLOR_ARGB(pMesh->colors[0][v][3], pMesh->colors[0][v][0], pMesh->colors[0][v][1], pMesh->colors[0][v][2]);
            }

            if (pMesh->colors[1])
            {
                dword   *p = (dword *)(&pVertices[v * pMesh->vertexStride + c2_offset]);
                *p = D3DCOLOR_ARGB(pMesh->colors[1][v][3], pMesh->colors[1][v][0], pMesh->colors[1][v][1], pMesh->colors[1][v][2]);
            }

            for (int n = 0; n < iNumTexcoords; ++n)
            {
                vec2_t  *p = (vec2_t *)(&pVertices[v * pMesh->vertexStride + t_offset + (n * sizeof(vec2_t))]);
                M_CopyVec2(pMesh->tverts[n][v], *p);
            }

            for (int m = 0; m < iNumTangents; ++m)
            {
                vec4_t  *p = (vec4_t *)(&pVertices[v * pMesh->vertexStride + tan_offset + (m * sizeof(vec4_t))]);

                if (pMesh->tangents[m] != nullptr)
                {
                    if (pMesh->signs[m] != nullptr)
                    {
                        M_SetVec4(*p,
                            pMesh->tangents[m][v][0],
                            pMesh->tangents[m][v][1],
                            pMesh->tangents[m][v][2],
                            2.0f * pMesh->signs[m][v] / 255.0f - 1.0f);
                    }
                    else
                    {
                        M_SetVec4(*p,
                            pMesh->tangents[m][v][0],
                            pMesh->tangents[m][v][1],
                            pMesh->tangents[m][v][2],
                            1.0f);
                    }
                }
                else
                    M_SetVec4(*p, 0.0f, 0.0f, 0.0f, 0.0f);
            }

            if (vid_meshGPUDeform)
            {
                if (pMesh->blendedLinks)
                {
                    CVec4b  &p_i = *(CVec4b *)(&pVertices[v * pMesh->vertexStride + indices_offset]);
                    CVec4b  &p_w = *(CVec4b *)(&pVertices[v * pMesh->vertexStride + weights_offset]);

                    const SBlendedLink &link = pMesh->blendedLinks[v];

                    if (link.num_weights > 4)
                    {
                        p_i[0] = byte(oBoneList.GetBoneRemappedIndex(link.indexes[0]));
                        p_i[1] = byte(oBoneList.GetBoneRemappedIndex(link.indexes[1]));
                        p_i[2] = byte(oBoneList.GetBoneRemappedIndex(link.indexes[2]));
                        p_i[3] = byte(oBoneList.GetBoneRemappedIndex(link.indexes[3]));

                        CVec4f v4Weights(link.weights[0], link.weights[1], link.weights[2], link.weights[3]);

                        // Normalize vertex weights
                        float fWeight(v4Weights[0] + v4Weights[1] + v4Weights[2] + v4Weights[3]);
                        v4Weights /= fWeight;

                        p_w[0] = BYTE_ROUND(v4Weights[0] * 255.0f);
                        p_w[1] = BYTE_ROUND(v4Weights[1] * 255.0f);
                        p_w[2] = BYTE_ROUND(v4Weights[2] * 255.0f);
                        p_w[3] = BYTE_ROUND(v4Weights[3] * 255.0f);
                    }
                    else if (link.num_weights > 0)
                    {
                        // Fill up the index and weight properties of each vertex
                        for (int b = 0; b < link.num_weights; ++b)
                        {
                            p_i[b] = byte(oBoneList.GetBoneRemappedIndex(link.indexes[b]));
                            p_w[b] = BYTE_ROUND(link.weights[b] * 255.0f);
                        }

                        // Copy earlier indices into later ones of a vertex has
                        // less than 4 weights for vertex shader cache improvements
                        for (int b = min(link.num_weights, 4); b < 4; ++b)
                        {
                            p_i[b] = byte(oBoneList.GetBoneRemappedIndex(link.indexes[link.num_weights - 1]));
                            p_w[b] = 0;
                        }
                    }
                    else
                    {
                        p_i[0] = 0;
                        p_w[0] = 255;

                        p_i[1] = 0;
                        p_w[1] = 0;

                        p_i[2] = 0;
                        p_w[2] = 0;

                        p_i[3] = 0;
                        p_w[3] = 0;
                    }
                }
                else if (pMesh->singleLinks)
                {
                    CVec4b  &p_i = *(CVec4b *)(&pVertices[v * pMesh->vertexStride + indices_offset]);
                    CVec4b  &p_w = *(CVec4b *)(&pVertices[v * pMesh->vertexStride + weights_offset]);

                    const singleLink_t link = pMesh->singleLinks[v];

                    p_i[0] = byte(oBoneList.GetBoneRemappedIndex(link));
                    p_w[0] = 255;

                    p_i[1] = byte(oBoneList.GetBoneRemappedIndex(link));
                    p_w[1] = 0;

                    p_i[2] = byte(oBoneList.GetBoneRemappedIndex(link));
                    p_w[2] = 0;

                    p_i[3] = byte(oBoneList.GetBoneRemappedIndex(link));
                    p_w[3] = 0;
                }
            }
        }

        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
        
        glGenBuffersARB(1, &IBMeshes[i]);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, IBMeshes[i]);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, pMesh->numFaces * 3 * sizeof(GLushort), nullptr, GL_STATIC_DRAW_ARB);

        GLushort* dataIB = (GLushort*)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        for (int n = 0; n < pMesh->numFaces; ++n)
        {
            dataIB[n * 3 + 0] = pMesh->faceList[n][0];
            dataIB[n * 3 + 1] = pMesh->faceList[n][1];
            dataIB[n * 3 + 2] = pMesh->faceList[n][2];
        }
        glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER);

        pMesh->sbuffer = i;
        pMesh->ibuffer = i;

        if (vid_meshGPUDeform && (pMesh->blendedLinks || pMesh->singleLinks))
        {
            pMesh->flags |= MESH_GPU_DEFORM;

            pMesh->dbuffer = i;
            DBMeshes[i] = 0;
        }
        else
        {
            pMesh->dbuffer = i;
            
            int iVertexStride(12);

            if (pMesh->normals)
                iVertexStride += 12;

            int iNumTangents(0);

            for (int j(0); j < MAX_UV_CHANNELS; ++j)
            {
                if (pMesh->tangents[j])
                    iNumTangents = j + 1;
                else
                    break;
            }

            iNumTangents = MIN(iNumTangents, 1);

            iVertexStride += iNumTangents * sizeof(CVec3f);

            glGenBuffersARB(1, &DBMeshes[i]);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, DBMeshes[i]);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, pMesh->num_verts * iVertexStride, nullptr, GL_STREAM_DRAW_ARB);
        }
    }

    // Register each lod
    for (uint uiLod(0); uiLod < pModel->GetNumLods(); ++uiLod)
        RegisterK2Model(pModel->GetLod(uiLod));

    return 0;
}


/*====================
  CGfxModels::UnregisterK2Model
  ====================*/
void    CGfxModels::UnregisterK2Model(CK2Model *pModel)
{
    // Unregister each mesh
    for (uint uiMesh(0); uiMesh < pModel->GetNumMeshes(); ++uiMesh)
    {
        CMesh *pMesh = pModel->GetMesh(uiMesh);

        if (pMesh->sbuffer == -1)
            continue;

        BoneRemap[pMesh->sbuffer].Clear();

        glDeleteBuffersARB(1, &VBMeshes[pMesh->sbuffer]);
        glDeleteBuffersARB(1, &IBMeshes[pMesh->ibuffer]);
        VBMeshes[pMesh->sbuffer] = 0;
        DBMeshes[pMesh->sbuffer] = 0;
        IBMeshes[pMesh->ibuffer] = 0;
    }
}


/*====================
  CGfxModels::RegisterTreeModel
  ====================*/
int     CGfxModels::RegisterTreeModel(CTreeModel *pModel)
{
    CTreeModelDef *pNewTreeDef(K2_NEW(ctx_GL2,    CTreeModelDef)(pModel));

    try
    {
        if (pNewTreeDef == nullptr)
            throw _TS("Failed to allocate new CTreeModelDef");

        if (!pNewTreeDef->IsValid())
            throw _TS("Failure while initializing new CTreeModelDef");

        pModel->SetVidDefIndex(g_pTreeSceneManager->AddDefinition(pNewTreeDef));
        return 0;
    }
    catch (const tstring &sReason)
    {
        if (pNewTreeDef != nullptr)
            K2_DELETE(pNewTreeDef);
        Console.Err << _T("CGfxModels::RegisterTreeModel - ") << ParenStr(pModel->GetName()) << _T(" - ") << sReason << newl;
        return 0;
    }
}


/*====================
  CGfxModels::UnregisterTreeModel
  ====================*/
void    CGfxModels::UnregisterTreeModel(CTreeModel *pModel)
{
    uint uiIndex(pModel->GetVidDefIndex());
    const CTreeModelDef *pTreeDef(g_pTreeSceneManager->GetDefinition(uiIndex));
    if (!pTreeDef)
        return;

    K2_DELETE(pTreeDef);
    g_pTreeSceneManager->RemoveDefinition(uiIndex);
    pModel->SetVidDefIndex(INVALID_INDEX);
}


/*====================
  CGfxModels::Shutdown
  ====================*/
void    CGfxModels::Shutdown()
{
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);

    for (int i(0); i < MAX_MESHES; ++i)
        GL_SAFE_DELETE(glDeleteBuffersARB, VBMeshes[i]);

    for (int i(0); i < MAX_MESHES; ++i)
        GL_SAFE_DELETE(glDeleteBuffersARB, DBMeshes[i]);

    for (int i(0); i < MAX_MESHES; ++i)
        GL_SAFE_DELETE(glDeleteBuffersARB, IBMeshes[i]);

    PRINT_GLERROR_BREAK();
}
