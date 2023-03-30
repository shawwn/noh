// (C)2005 S2 Games
// d3d9_model.h
//
// Direct3D Model / Meshes
//=============================================================================
#ifndef __D3D9_MODEL_H__
#define __D3D9_MODEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_sceneentity.h"
//=============================================================================

class CSceneEntity;
struct SSceneEntityEntry;

int     D3D_RegisterModel(class CModel *pModel);
void    D3D_UnregisterModel(class CModel *pModel);

bool    D3D_AddModelToLists(const CSceneEntity &cEntity, const SSceneEntityEntry &cEntry);
void    D3D_InitModel();

const int MAX_GPU_BONES = 72;
const int MAX_BONE_TABLE_SIZE = MAX_GPU_BONES;

extern D3DXMATRIXA16        g_vBoneData[MAX_BONE_TABLE_SIZE];

extern int g_iNumActiveBones;

extern list<vector<int>>    g_lCustomMappings;

#endif // __D3D9_MODEL_H__