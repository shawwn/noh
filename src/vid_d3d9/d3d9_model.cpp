// (C)2005 S2 Games
// d3d9_model.cpp
//
// Direct3D model functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "../public/mdlsprite_t.h"
#include "../public/blendedlink_t.h"

#include "d3d9_main.h"
#include "d3d9_model.h"
#include "d3d9_state.h"
#include "d3d9_material.h"
#include "d3d9_shader.h"
#include "d3d9_scene.h"
#include "d3d9_util.h"
#include "c_treemodeldef.h"
#include "c_treescenemanager.h"
#include "c_bonelist.h"
#include "c_renderlist.h"

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
// Globals
//=============================================================================
CVAR_BOOLF	(vid_lodUse, true, CVAR_SAVECONFIG);
CVAR_INTF	(vid_lodBias, 0, CVAR_SAVECONFIG);
CVAR_INTF	(vid_lodCurve, 2, CVAR_SAVECONFIG);
CVAR_INTF	(vid_lodForce, -1, CVAR_SAVECONFIG);
CVAR_BOOL	(vid_drawModelBounds, false);
CVAR_BOOL	(vid_drawModelSurfs, false);
CVAR_BOOLF	(vid_precreateDynamicBuffers, false, CVAR_SAVECONFIG);
CVAR_BOOL	(vid_treeRotate, false);

extern CCvar<bool>	vid_meshGPUDeform;
extern CCvar<bool>	vid_drawBones;
extern CCvar<bool>	vid_drawBoneNames;
extern CCvar<bool>	vid_meshAlwaysStatic;
extern CCvar<float>	vid_treeWindStrength;

D3DXMATRIXA16	g_vBoneData[MAX_BONE_TABLE_SIZE];
int				g_iNumActiveBones = 0;

static int	*s_pMapping = NULL;
int g_iCurrentSkelbone = -1;

CBoneList g_BoneRemap[MAX_MESHES];
list<vector<int>>	g_lCustomMappings;
//=============================================================================


/*====================
  D3D_AddK2ModelToLists

  Queue all meshes of this model up for rendering
  ====================*/
bool	D3D_AddK2ModelToLists(CK2Model *pModel, const CSceneEntity &cEntity, const SSceneEntityEntry &cEntry)
{
	PROFILE("D3D_AddK2ModelToLists");

	if (pModel == NULL)
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

	if (vid_drawModelBounds || cEntity.flags & SCENEENT_SHOW_BOUNDS)
	{
		D3DXMATRIXA16 mWorldTranslation;
		D3DXMATRIXA16 mWorldScaling;
		D3DXMATRIXA16 mWorldRotation;

		CVec3f v3Pos(cEntity.GetPosition());
		D3DXMatrixTranslation(&mWorldTranslation, v3Pos[X], v3Pos[Y], v3Pos[Z]);
		D3D_AxisToMatrix(cEntity.axis, &mWorldRotation);

		if (cEntity.scale != 1.0f)
			D3DXMatrixScaling(&mWorldScaling, cEntity.scale, cEntity.scale, cEntity.scale);
		else
			D3DXMatrixIdentity(&mWorldScaling);

		D3DXMATRIXA16 mWorld = mWorldScaling * mWorldRotation * mWorldTranslation;

		D3D_AddBox(pModel->GetBounds(), cEntity.color, mWorld);
	}

	if (cEntity.skeleton && cEntity.skeleton->IsValid() && vid_drawBones)
	{
		D3DXMATRIXA16 mWorldTranslation;
		D3DXMATRIXA16 mWorldScaling;
		D3DXMATRIXA16 mWorldRotation;

		CVec3f v3Pos(cEntity.GetPosition());
		D3DXMatrixTranslation(&mWorldTranslation, v3Pos[X], v3Pos[Y], v3Pos[Z]);
		D3D_AxisToMatrix(cEntity.axis, &mWorldRotation);

		if (cEntity.scale != 1.0f)
			D3DXMatrixScaling(&mWorldScaling, cEntity.scale, cEntity.scale, cEntity.scale);
		else
			D3DXMatrixIdentity(&mWorldScaling);

		D3DXMATRIXA16 mWorld = mWorldScaling * mWorldRotation * mWorldTranslation;

		ResHandle hFont(g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));

		CK2Model *pBaseModel(pModel->GetBaseLod() ? pModel->GetBaseLod() : pModel);

		for (uint uiBone(0); uiBone < pBaseModel->GetNumBones(); ++uiBone)
		{
			CVec3f	v3Pos(cEntity.skeleton->GetBoneState(uiBone)->tm_local.pos);

			v3Pos = D3D_TransformPoint(v3Pos, mWorld);

			if (cEntity.skeleton->GetBoneState(uiBone)->visibility)
				D3D_AddPoint(v3Pos, CVec4f(1.0f, 0.0f, 0.0f, 1.0f));
			else
				D3D_AddPoint(v3Pos, CVec4f(0.5f, 0.5f, 0.5f, 1.0f));

			CBone *pBone(pBaseModel->GetBone(uiBone));
			uint uiParent(pBone->GetParentIndex());

			if (uiParent != INVALID_BONE)
			{
				CVec3f	v3End(cEntity.skeleton->GetBoneState(uiParent)->tm_local.pos);

				v3End = D3D_TransformPoint(v3End, mWorld);

				D3D_AddLine(v3Pos, v3End, CVec4f(1.0f, 1.0f, 0.0f, 1.0f));
			}

			if (vid_drawBoneNames)
			{
				CVec2f	v2ScreenPos;

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
				g_lCustomMappings.push_back(vector<int>());

				vector<int> &vCustomMapping(g_lCustomMappings.back());
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
					Console.Warn << _T("D3D_AddK2ModelToLists(): invalid skeleton (skeleton->model != cEntity.model)") << newl;
					return false;
				}
			}
		}
	}

	if (vid_drawModelSurfs)
	{
		D3DXMATRIXA16 mWorldTranslation;
		D3DXMATRIXA16 mWorldScaling;
		D3DXMATRIXA16 mWorldRotation;

		CVec3f v3Pos(cEntity.GetPosition());
		D3DXMatrixTranslation(&mWorldTranslation, v3Pos[X], v3Pos[Y], v3Pos[Z]);
		D3D_AxisToMatrix(cEntity.axis, &mWorldRotation);

		if (cEntity.scale != 1.0f)
			D3DXMatrixScaling(&mWorldScaling, cEntity.scale, cEntity.scale, cEntity.scale);
		else
			D3DXMatrixIdentity(&mWorldScaling);

		D3DXMATRIXA16 mWorld = mWorldScaling * mWorldRotation * mWorldTranslation;

		SurfVector &surfs(pModel->GetSurfs());

		for (SurfVector::iterator itSurf(surfs.begin()); itSurf != surfs.end(); ++itSurf)
		{
			const vector<uint> &vTris(itSurf->GetTriList());

			for (uint f(0); f < vTris.size() / 3; ++f)
			{
				D3D_AddEffectTriangle
				(
					D3D_TransformPoint(itSurf->GetPoint(vTris[f * 3 + 0]), mWorld),
					D3D_TransformPoint(itSurf->GetPoint(vTris[f * 3 + 1]), mWorld),
					D3D_TransformPoint(itSurf->GetPoint(vTris[f * 3 + 2]), mWorld),
					D3DCOLOR_ARGB(128, 0, 255, 0),
					D3DCOLOR_ARGB(128, 0, 255, 0),
					D3DCOLOR_ARGB(128, 0, 255, 0),
					CVec4f(0.0f, 0.0f, 0.0f, 0.0f),
					CVec4f(0.0f, 0.0f, 0.0f, 0.0f),
					CVec4f(0.0f, 0.0f, 0.0f, 0.0f),
					g_ResourceManager.Register(_T("/core/materials/effect_solid.material"), RES_MATERIAL)
				);
			}
		}
	}

	D3DXMATRIXA16 mWorldTranslation;
	D3DXMATRIXA16 mWorldScaling;
	D3DXMATRIXA16 mWorldRotation;

	CVec3f v3Pos(cEntity.GetPosition());
	D3DXMatrixTranslation(&mWorldTranslation, v3Pos.x, v3Pos.y, v3Pos.z);
	D3D_AxisToMatrix(cEntity.axis, &mWorldRotation);

	if (cEntity.scale != 1.0f)
		D3DXMatrixScaling(&mWorldScaling, cEntity.scale, cEntity.scale, cEntity.scale);
	else
		D3DXMatrixIdentity(&mWorldScaling);

	D3DXMATRIXA16 mWorld = mWorldScaling * mWorldRotation * mWorldTranslation;

	int *pMapping(pModel->GetBoneMapping());
	uint uiNumMeshes(pModel->GetNumMeshes());
	for (uint uiMesh(0); uiMesh < uiNumMeshes; ++uiMesh)
	{
		CMesh *pMesh(pModel->GetMesh(uiMesh));

		if (pMesh->renderflags & MESH_INVIS)
			continue;

		if (cEntity.skeleton && cEntity.skeleton->IsValid() && pMesh->bonelink != -1 && pMapping[pMesh->bonelink] != -1 && !vid_meshAlwaysStatic)
		{
			int iCurrentSkelbone(pMapping[pMesh->bonelink]);

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

		g_RenderList.Add(K2_NEW(ctx_D3D9,   CMeshRenderer)(hMaterial, cEntity, pMesh, mWorld, mWorldRotation, cEntry));
	}

	return true;
}


/*====================
  D3D_AddTreeModelToLists
  ====================*/
bool	D3D_AddTreeModelToLists(CTreeModel *pTreeModel, const CSceneEntity &cEntity, const SSceneEntityEntry &cEntry)
{
	PROFILE("D3D_AddTreeModelToLists");

	D3DXMATRIXA16 x_mWorld;
	D3DXMATRIXA16 x_mWorldViewProj;
	D3DXMATRIXA16 x_mWorldInverse;
	D3DXMATRIXA16 x_mWorldRotate;

	if (vid_treeRotate)
	{
		// Matrix calculations
		D3DXMATRIXA16 mWorldTranslation;
		D3DXMATRIXA16 mWorldScaling;

		const CVec3f &v3Pos(cEntity.GetPosition());
		D3DXMatrixTranslation(&mWorldTranslation, v3Pos.x, v3Pos.y, v3Pos.z);
		D3DXMatrixScaling(&mWorldScaling, cEntity.scale, cEntity.scale, cEntity.scale);
		D3D_AxisToMatrix(cEntity.axis, &x_mWorldRotate);

		x_mWorld = mWorldScaling * x_mWorldRotate * mWorldTranslation;
		x_mWorldViewProj = x_mWorld * g_mViewProj;

		D3DXMATRIXA16 mScaleInverse;
		D3DXMatrixScaling(&mScaleInverse, 1.0f / cEntity.scale, 1.0f / cEntity.scale, 1.0f / cEntity.scale);

		D3DXMATRIXA16 mRotationInverse;
		D3DXMatrixTranspose(&mRotationInverse, &x_mWorldRotate);

		D3DXMATRIXA16 mTranslationInverse;
		D3DXMatrixTranslation(&mTranslationInverse, -v3Pos.x, -v3Pos.y, -v3Pos.z);
		
		x_mWorldInverse = mTranslationInverse * mRotationInverse * mScaleInverse;
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
		
		D3DXMatrixIdentity(&x_mWorldRotate);
	}

	if (vid_drawModelBounds || cEntity.flags & SCENEENT_SHOW_BOUNDS)
	{
		D3D_AddBox(pTreeModel->GetBounds(), (cEntity.flags & SCENEENT_SOLID_COLOR) ? cEntity.color : WHITE, x_mWorld);
	}

	const CTreeModelDef *pTreeDef(g_pTreeSceneManager->GetDefinition(pTreeModel->GetVidDefIndex()));


	// Update the camera
	{
		PROFILE("Update Camera");

		CVec3f v3CamPos;
		CVec3f v3CamDir;
		
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
		
		D3DXMATRIXA16 mRotationInverse;
		D3DXMatrixTranspose(&mRotationInverse, &x_mWorldRotate);

		D3DXVECTOR3 d3dvCamDir(v3CamDir.x, v3CamDir.y, v3CamDir.z);
		//D3DXVec3TransformNormal(&d3dvCamDir, &d3dvCamDir, &x_mWorldInverseRotate);

		CVec3f v3CameraPos = CVec3_cast(d3dvCamPos);
		CVec3f v3CameraDir = CVec3_cast(d3dvCamDir);
		
		// Update tree wind/camera/time settings
		CTreeModel::UpdateCamera(v3CameraPos, v3CameraDir);
		
		pTreeDef->SetWindStrength(vid_treeWindStrength);
		pTreeDef->ComputeLODLevel();
	}

	g_RenderList.Add(K2_NEW(ctx_D3D9,   CTreeBranchRenderer)(cEntity, pTreeDef, x_mWorldViewProj, x_mWorld, x_mWorldRotate));
	g_RenderList.Add(K2_NEW(ctx_D3D9,   CTreeFrondRenderer)(cEntity, pTreeDef, x_mWorldViewProj, x_mWorld, x_mWorldRotate));
	g_RenderList.Add(K2_NEW(ctx_D3D9,   CTreeLeafRenderer)(cEntity, pTreeDef, x_mWorldViewProj, x_mWorld, x_mWorldRotate));

	STreeBillboardData avBillboards[2];
	avBillboards[0].m_bActive = avBillboards[1].m_bActive = false;
	pTreeModel->GetBillboardData(avBillboards);

	dword dwColor(((cEntity.flags & SCENEENT_SOLID_COLOR) ? cEntity.color : WHITE).GetAsDWord());

	for (int i(0); i < 2; ++i)
	{
		if (avBillboards[i].m_bActive)
		{
			D3D_AddTreeBillboard
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
  D3D_AddModelToLists
  ====================*/
bool	D3D_AddModelToLists(const CSceneEntity &cEntity, const SSceneEntityEntry &cEntry)
{
	PROFILE("D3D_AddModelToLists");

	try
	{
		// Grab the scene entity's model and decide what to do with it
		CModel* pModelResource(g_ResourceManager.GetModel(cEntity.hRes));
		if (pModelResource == NULL)
			EX_ERROR(_T("Invalid model handle"));

		IModel *pModel(pModelResource->GetModelFile());
		if (pModel == NULL)
			EX_ERROR(_T("Couldn't retrieve model data"));

		switch (pModel->GetType())
		{
		case MODEL_K2:
			return D3D_AddK2ModelToLists(static_cast<CK2Model *>(pModel), cEntity, cEntry);

		case MODEL_SPEEDTREE:
			return D3D_AddTreeModelToLists(static_cast<CTreeModel *>(pModel), cEntity, cEntry);

		default:
			EX_ERROR(_T("Unknown model type"));
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("D3D_AddModelToLists() - "), NO_THROW);
		return false;
	}
}


/*====================
  D3D_RegisterK2Model
  ====================*/
bool	D3D_RegisterK2Model(CK2Model *pModel)
{
	PROFILE("D3D_RegisterK2Model");

	if (g_pd3dDevice == NULL)
		return false;

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
			if (g_pVBStaticMeshes[i] == NULL)
				break;
		}

		if (i == MAX_MESHES)
		{
			K2System.Error(_T("MAX_MESHES"));
			return false;
		}

		// Calculate mesh bone index remapping so bone deform shaders only needs to send
		// bones that are used by this mesh to the shader
		CBoneList	&oBoneList(g_BoneRemap[i]);

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

#if 0
		// Calculate face direction in texture space
		vector<bool>	vFaceSign(pMesh->numFaces);
		vector<int>		vFaceIndex(pMesh->num_verts, -1);

		if (pMesh->tverts[0] != NULL)
		{
			for (int n(0); n < pMesh->numFaces; ++n)
			{
				const CVec2f &v2A(pMesh->tverts[0][pMesh->faceList[n][0]]);
				const CVec2f &v2B(pMesh->tverts[0][pMesh->faceList[n][1]]);
				const CVec2f &v2C(pMesh->tverts[0][pMesh->faceList[n][2]]);

				vFaceSign[n] = ((v2B.x - v2A.x) * (v2C.y - v2A.y) - (v2B.y - v2A.y) * (v2C.x - v2A.x)) > 0.0f;

				vFaceIndex[pMesh->faceList[n][0]] = n;
				vFaceIndex[pMesh->faceList[n][1]] = n;
				vFaceIndex[pMesh->faceList[n][2]] = n;
			}
		}
#endif

		int iNumTexcoords = 0;

		for (int j = 0; j < MAX_UV_CHANNELS; ++j)
		{
			if (pMesh->tverts[j])
				iNumTexcoords = j + 1;
		}

		pMesh->vertexFVF = D3DFVF_XYZ;
		pMesh->vertexStride = 12;

		if (pMesh->normals)
		{
			pMesh->vertexFVF |= D3DFVF_NORMAL4B;
			pMesh->vertexStride += 4;
		}

		int iNumTangents = 0;

		for (int j = 0; j < MAX_UV_CHANNELS; ++j)
		{
			if (pMesh->tangents[j])
				iNumTangents = j + 1;
		}

		for (int k = 0; k < iNumTangents; ++k)
			pMesh->vertexFVF |= D3DFVF_TEXCOORDSIZE1((iNumTexcoords + k));

		int iNumData = 0;

		if (vid_meshGPUDeform && (pMesh->blendedLinks || pMesh->singleLinks))
		{
			// Bone indices and weights
			iNumData = 2;
			pMesh->vertexFVF |= D3DFVF_TEXCOORDSIZE1((iNumTexcoords + iNumTangents + 0));
			pMesh->vertexFVF |= D3DFVF_TEXCOORDSIZE1((iNumTexcoords + iNumTangents + 1));
		}

		pMesh->vertexFVF |= (iNumTexcoords + iNumTangents + iNumData) << D3DFVF_TEXCOUNT_SHIFT;
		pMesh->vertexStride += iNumTexcoords * sizeof(CVec2f);
		pMesh->vertexStride += iNumTangents * sizeof(CVec4b);
		pMesh->vertexStride += iNumData * sizeof(CVec4b);

		if (pMesh->colors[0])
		{
			pMesh->vertexFVF |= D3DFVF_DIFFUSE;
			pMesh->vertexStride += 4;
		}

		if (pMesh->colors[1])
		{
			pMesh->vertexFVF |= D3DFVF_SPECULAR;
			pMesh->vertexStride += 4;
		}

		pMesh->vertexDecl = D3D_RegisterVertexDeclaration(pMesh->vertexFVF);

		byte *pVertices;

		int v_offset = 0;
		int n_offset = v_offset + sizeof(CVec3f);

		int c1_offset = n_offset + ((pMesh->vertexFVF & D3DFVF_NORMAL4B) ? sizeof(CVec4b) : 0);
		int c2_offset = c1_offset + ((pMesh->vertexFVF & D3DFVF_DIFFUSE) ? sizeof(DWORD) : 0);

		int t_offset = c2_offset + ((pMesh->vertexFVF & D3DFVF_SPECULAR) ? sizeof(DWORD) : 0);

		int tan_offset = t_offset + (iNumTexcoords * sizeof(vec2_t));

		int indices_offset = tan_offset + (iNumTangents * sizeof(CVec4b));
		int weights_offset = indices_offset + sizeof(CVec4b);

		//
		// Initialize Static Vertex Buffer
		//

		if (FAILED(g_pd3dDevice->CreateVertexBuffer(pMesh->num_verts * pMesh->vertexStride,
				g_d3dManagedUsage | D3DUSAGE_WRITEONLY, 0, g_d3dManagedPool, &g_pVBStaticMeshes[i], NULL)))
			K2System.Error(_T("D3D_RegisterK2Model(): CreateVertexBuffer failed"));

		if (FAILED(g_pVBStaticMeshes[i]->Lock(0, pMesh->num_verts * pMesh->vertexStride, (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
			continue;

		for (int v = 0; v < pMesh->num_verts; ++v)
		{
			if (pMesh->vertexFVF & D3DFVF_XYZ)
			{
				vec3_t	*p_v = (vec3_t *)(&pVertices[v * pMesh->vertexStride + v_offset]);
				M_CopyVec3(pMesh->verts[v], *p_v);
			}

			if (pMesh->vertexFVF & D3DFVF_NORMAL4B)
			{
				CVec4b	*p_n = (CVec4b *)(&pVertices[v * pMesh->vertexStride + n_offset]);

				*p_n = CVec4b(
				BYTE_ROUND((pMesh->normals[v][0] + 1.0f) * 0.5f * 255.0f),
				BYTE_ROUND((pMesh->normals[v][1] + 1.0f) * 0.5f * 255.0f),
				BYTE_ROUND((pMesh->normals[v][2] + 1.0f) * 0.5f * 255.0f),
				0);
			}

			if (pMesh->vertexFVF & D3DFVF_DIFFUSE)
			{
				DWORD	*p_c1 = (DWORD *)(&pVertices[v * pMesh->vertexStride + c1_offset]);
				if (pMesh->colors[0] != NULL)
					*p_c1 = D3DCOLOR_ARGB(pMesh->colors[0][v][3], pMesh->colors[0][v][0], pMesh->colors[0][v][1], pMesh->colors[0][v][2]);
				else
					*p_c1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			}

			if (pMesh->vertexFVF & D3DFVF_SPECULAR)
			{
				DWORD	*p_c2 = (DWORD *)(&pVertices[v * pMesh->vertexStride + c2_offset]);

				if (pMesh->colors[1] != NULL)
					*p_c2 = D3DCOLOR_ARGB(pMesh->colors[1][v][3], pMesh->colors[1][v][0], pMesh->colors[1][v][1], pMesh->colors[1][v][2]);
				else
					*p_c2 = D3DCOLOR_ARGB(255, 255, 255, 255);
			}

			for (int n = 0; n < iNumTexcoords; ++n)
			{
				vec2_t	*p_t = (vec2_t *)(&pVertices[v * pMesh->vertexStride + t_offset + (n * sizeof(vec2_t))]);

				if (pMesh->tverts[n] != NULL)
					M_CopyVec2(pMesh->tverts[n][v], *p_t);
				else
					M_SetVec2(*p_t, 0.0f, 0.0f);
			}

			for (int m = 0; m < iNumTangents; ++m)
			{
				CVec4b	*p_t = (CVec4b *)(&pVertices[v * pMesh->vertexStride + tan_offset + (m * sizeof(CVec4b))]);

				if (pMesh->tangents[m] != NULL)
				{
#if 1
					if (pMesh->signs[m] != NULL)
					{
						*p_t = CVec4b(
						BYTE_ROUND((pMesh->tangents[m][v][0] + 1.0f) * 0.5 * 255.0f),
						BYTE_ROUND((pMesh->tangents[m][v][1] + 1.0f) * 0.5 * 255.0f),
						BYTE_ROUND((pMesh->tangents[m][v][2] + 1.0f) * 0.5 * 255.0f),
						pMesh->signs[m][v]);
					}
					else
					{
						*p_t = CVec4b(
						BYTE_ROUND((pMesh->tangents[m][v][0] + 1.0f) * 0.5 * 255.0f),
						BYTE_ROUND((pMesh->tangents[m][v][1] + 1.0f) * 0.5 * 255.0f),
						BYTE_ROUND((pMesh->tangents[m][v][2] + 1.0f) * 0.5 * 255.0f),
						255);
					}
#else
					bool bSign(vFaceSign[vFaceIndex[v]]);

					*p_t = CVec4b(
						BYTE_ROUND((pMesh->tangents[m][v][0] + 1.0f) * 0.5 * 255.0f),
						BYTE_ROUND((pMesh->tangents[m][v][1] + 1.0f) * 0.5 * 255.0f),
						BYTE_ROUND((pMesh->tangents[m][v][2] + 1.0f) * 0.5 * 255.0f),
						bSign ? 255 : 0);
#endif
				}
				else
					*p_t = CVec4b(0, 0, 0, 0);
			}

			if (vid_meshGPUDeform)
			{
				if (pMesh->blendedLinks)
				{
					CVec4b	&p_i = *(CVec4b *)(&pVertices[v * pMesh->vertexStride + indices_offset]);
					CVec4b	&p_w = *(CVec4b *)(&pVertices[v * pMesh->vertexStride + weights_offset]);

					const SBlendedLink &link = pMesh->blendedLinks[v];

					if (link.num_weights > 4)
					{
						// Only use the first 4 links if we have more (engine sorts by weight)
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

						// Copy earlier indices into later ones if a vertex has
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
					CVec4b	&p_i = *(CVec4b *)(&pVertices[v * pMesh->vertexStride + indices_offset]);
					CVec4b	&p_w = *(CVec4b *)(&pVertices[v * pMesh->vertexStride + weights_offset]);

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

		g_pVBStaticMeshes[i]->Unlock();

		if (vid_geometryPreload && !g_bD3D9Ex)
			g_pVBStaticMeshes[i]->PreLoad();

		//
		// Initialize Dynamic Vertex Buffer
		//

		if (pMesh->blendedLinks || pMesh->singleLinks)
		{
			if (vid_precreateDynamicBuffers)
			{
				if (FAILED(g_pd3dDevice->CreateVertexBuffer(pMesh->num_verts * pMesh->vertexStride,
						D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBDynamicMeshes[i], NULL)))
					K2System.Error(_T("D3D_RegisterK2Model(): CreateVertexBuffer failed"));
			}

			pMesh->dbuffer = i;
		}
		else
		{
			pMesh->dbuffer = -1;
		}

		//
		// Initialize Index Buffer
		//

		if (FAILED(g_pd3dDevice->CreateIndexBuffer(pMesh->numFaces * 3 * sizeof(WORD),
					g_d3dManagedUsage | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, g_d3dManagedPool, &g_pIBMeshes[i], NULL)))
			K2System.Error(_T("D3D_RegisterK2Model(): CreateIndexBuffer failed"));

		WORD *pIndices;
		if (FAILED(g_pIBMeshes[i]->Lock(0, pMesh->numFaces * 3 * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
			continue;

		for (int n = 0; n < pMesh->numFaces; ++n)
		{
			pIndices[n * 3 + 0] = pMesh->faceList[n][0];
			pIndices[n * 3 + 1] = pMesh->faceList[n][1];
			pIndices[n * 3 + 2] = pMesh->faceList[n][2];
		}

		g_pIBMeshes[i]->Unlock();

		if (vid_geometryPreload && !g_bD3D9Ex)
			g_pIBMeshes[i]->PreLoad();

		pMesh->sbuffer = i;
		pMesh->ibuffer = i;

		if (vid_meshGPUDeform && g_DeviceCaps.bShaders)
			pMesh->flags |= MESH_GPU_DEFORM;
	}

	// Register each lod
	for (uint uiLod(0); uiLod < pModel->GetNumLods(); ++uiLod)
		D3D_RegisterK2Model(pModel->GetLod(uiLod));

	return 0;
}


/*====================
  D3D_UnregisterK2Model
  ====================*/
void	D3D_UnregisterK2Model(CK2Model *pModel)
{
	// Unregister each mesh
	for (uint uiMesh(0); uiMesh < pModel->GetNumMeshes(); ++uiMesh)
	{
		CMesh *pMesh = pModel->GetMesh(uiMesh);

		if (pMesh->sbuffer == -1)
			continue;

		g_BoneRemap[pMesh->sbuffer].Clear();

		SAFE_RELEASE(g_pVBStaticMeshes[pMesh->sbuffer]);
		SAFE_RELEASE(g_pVBDynamicMeshes[pMesh->dbuffer]);
		SAFE_RELEASE(g_pVBStaticMeshNormals[pMesh->sbuffer]);
		SAFE_RELEASE(g_pVBDynamicMeshNormals[pMesh->dbuffer]);
		SAFE_RELEASE(g_pIBMeshes[pMesh->ibuffer]);
	}
}


/*====================
  D3D_RegisterTreeModel
  ====================*/
int		D3D_RegisterTreeModel(CTreeModel *pModel)
{
	PROFILE("D3D_RegisterTreeModel");

	CTreeModelDef *pNewTreeDef(K2_NEW(ctx_D3D9,   CTreeModelDef)(pModel));

	try
	{
		if (pNewTreeDef == NULL)
			throw _TS("Failed to allocate new CTreeModelDef");

		if (!pNewTreeDef->IsValid())
			throw _TS("Failure while initializing new CTreeModelDef");

		pModel->SetVidDefIndex(g_pTreeSceneManager->AddDefinition(pNewTreeDef));
		return 0;
	}
	catch (const tstring &sReason)
	{
		if (pNewTreeDef != NULL)
			K2_DELETE(pNewTreeDef);
		Console.Err << _T("D3D_RegisterTreeModel") << ParenStr(pModel->GetName()) << _T(" - ") << sReason << newl;
		return 0;
	}
}


/*====================
  D3D_UnregisterTreeModel
  ====================*/
void	D3D_UnregisterTreeModel(CTreeModel *pModel)
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
  D3D_RegisterModel
  ====================*/
int		D3D_RegisterModel(CModel *pModel)
{
	PROFILE("D3D_RegisterModel");

	if (g_pd3dDevice == NULL)
		return false;

	switch (pModel->GetModelFile()->GetType())
	{
	case MODEL_K2:
		return D3D_RegisterK2Model(static_cast<CK2Model *>(pModel->GetModelFile()));

	case MODEL_SPEEDTREE:
		return D3D_RegisterTreeModel(static_cast<CTreeModel *>(pModel->GetModelFile()));

	default:
		Console.Err << _T("D3D_RegisterModel(): Unknown model type") << newl;
		return false;
	}
}


/*====================
  D3D_UnregisterModel
  ====================*/
void	D3D_UnregisterModel(CModel *pModel)
{
	IModel* pModelFile(pModel->GetModelFile());
	if (pModelFile != NULL)
	{
		switch (pModelFile->GetType())
		{
		case MODEL_K2:
			D3D_UnregisterK2Model(static_cast<CK2Model *>(pModelFile));
			break;

		case MODEL_SPEEDTREE:
			D3D_UnregisterTreeModel(static_cast<CTreeModel *>(pModelFile));
			break;

		default:
			Console.Err << _T("D3D_UnregisterModel(): Unknown model type") << newl;
		}
	}
}


/*====================
  D3D_InitModel
  ====================*/
void	D3D_InitModel()
{
}


/*====================
  PrintMeshSize
  ====================*/
CMD(PrintMeshSize)
{
	uint uiVertexBuffer(0);
	uint uiIndexBuffer(0);

	for (int i(0); i < MAX_MESHES; ++i)
	{
		if (g_pVBStaticMeshes[i])
		{
			D3DVERTEXBUFFER_DESC desc;
			g_pVBStaticMeshes[i]->GetDesc(&desc);
			uiVertexBuffer += desc.Size;
		}

		if (g_pIBMeshes[i])
		{
			D3DINDEXBUFFER_DESC desc;
			g_pIBMeshes[i]->GetDesc(&desc);
			uiIndexBuffer += desc.Size;
		}
	}

	Console << _T("Mesh Vertex Buffers: ") << GetByteString(uiVertexBuffer) << newl;
	Console << _T("Mesh Index Buffers: ") << GetByteString(uiIndexBuffer) << newl;

	return true;
}
