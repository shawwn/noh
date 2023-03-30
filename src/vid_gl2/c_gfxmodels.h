// (C)2008 S2 Games
// c_gfxmodels.h
//
//=============================================================================
#ifndef __C_GFXMODELS_H__
#define __C_GFXMODELS_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_gfx3d.h"
#include "c_bonelist.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CSceneEntity;
class CMesh;
enum EMaterialPhase;

const int MAX_GPU_BONES = 72;
const int MAX_BONE_TABLE_SIZE = MAX_GPU_BONES * 3;

extern CVec4f   g_vBoneData[MAX_BONE_TABLE_SIZE];
extern int      g_iNumActiveBones;
//=============================================================================

//=============================================================================
// CGfxModels
//=============================================================================
class CGfxModels
{
    SINGLETON_DEF(CGfxModels)

protected:
    
    bool    AddK2ModelToLists(CK2Model *pModel, const CSceneEntity &cEntity, EMaterialPhase ePhase);
    bool    AddTreeModelToLists(CTreeModel *pTreeModel, const CSceneEntity &cEntity, EMaterialPhase ePhase);

    bool    RegisterK2Model(CK2Model *pModel);
    void    UnregisterK2Model(CK2Model *pModel);

    int     RegisterTreeModel(CTreeModel *pModel);
    void    UnregisterTreeModel(CTreeModel *pModel);

    int     *s_pMapping;

public:
    ~CGfxModels();

    void    Shutdown();

    int     RegisterModel(class CModel *pModel);
    void    UnregisterModel(class CModel *pModel);
    bool    AddModelToLists(const CSceneEntity &cEntity, EMaterialPhase ePhase);

    CBoneList   BoneRemap[MAX_MESHES];
    int         iCurrentSkelbone;

    GLuint      VBMeshes[MAX_MESHES];
    GLuint      DBMeshes[MAX_MESHES];
    GLuint      IBMeshes[MAX_MESHES];

    list<vector<int> >  lCustomMappings;
};
extern CGfxModels *GfxModels;
//=============================================================================

#endif //__C_GFXMATERIALS_H__
