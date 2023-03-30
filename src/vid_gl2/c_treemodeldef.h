// (C)2005 S2 Games
// c_treemodeldef.h
//
//=============================================================================
#ifndef __C_TREEMODELDEF_H__
#define __C_TREEMODELDEF_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_treemodel.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct STreeGeometryBuffers
{
    int                             m_iNumVerts;
    GLuint                          m_VBuffer;

    ivector                         m_viNumIndices;
    vector<GLuint>                  m_vIBuffers;

    ivector                         m_viTextureIndex;

    uint                            m_dwAlphaTest;
};

struct SLeafGeometryBuffers
{
    int                             m_iNumVerts;
    GLuint                          m_VBuffer;

    ivector                         m_viNumIndices;
    vector<GLuint>                  m_vIBuffers;

    ivector                         m_viTextureIndex;

    uint                            m_dwAlphaTest;
    mutable bool                    m_bLeavesUpdated;
};

typedef map<int, STreeGeometryBuffers>  TreeBufferLODMap;
typedef map<int, SLeafGeometryBuffers>  LeafBufferLODMap;

// This value must match what is in the leaf shader!
// 48 allows for two leaf textures with three rocking groups
const int MAX_LEAF_CLUSTER_INDEX(48);
const int LEAF_CLUSTER_TABLE_SIZE(MAX_LEAF_CLUSTER_INDEX * 4);
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CTreeModel;
enum EMaterialPhase;
class CCamera;
struct STreeSceneEntry;

extern float g_afLeafClusterData[LEAF_CLUSTER_TABLE_SIZE];
//=============================================================================

//=============================================================================
// CTreeModelDef
//=============================================================================
class CTreeModelDef
{
private:
    static bool         s_bInitialized;

    bool                m_bValid;
    CTreeModel*         m_pTreeModel;

    TreeBufferLODMap    m_mapBranchGeometry;
    TreeBufferLODMap    m_mapFrondGeometry;
    LeafBufferLODMap    m_mapLeafGeometry;

    ResHandle           m_hBranchMaterial;
    vector<ResHandle>   m_vhFrondMaterials;
    vector<ResHandle>   m_vhLeafMaterials;
    vector<ResHandle>   m_vhBillboardMaterials;

    mutable bool        m_bLeavesUpdated;
    mutable float       m_afLeafClusterData[LEAF_CLUSTER_TABLE_SIZE];
    mutable uint        m_uiLeafClusterDataSize;

    CTreeModelDef() {}

    void    LoadBranches();
    void    LoadFronds();
    void    LoadLeaves();

public:
    ~CTreeModelDef();
    CTreeModelDef(class CTreeModel *pModel);

    static void     Init();

    bool            IsValid()   { return m_bValid; }

    void            ResetLeaves()   { m_bLeavesUpdated = false; }

    void            SetWindStrength(float fStrength) const;
    void            ComputeLODLevel() const;
    SLODData        GetDiscreetBranchLOD() const;
    SLODData        GetDiscreetFrondLOD() const;
    void            GetLeafLODData(SLODData avLeafLODs[]) const;
    const float*    GetLeafBillboardTable(uint &uiSize) const;

    void            FrameUpdate(CCamera &camera, STreeSceneEntry *pTree) const;
    void            SetShaderGlobals(STreeSceneEntry *pTree) const;

    const CBBoxf&   GetBounds() const;

    void            Destroy();

    bool            HasBranchGeometry() const   { return !m_mapBranchGeometry.empty(); }
    bool            HasFrondGeometry() const    { return !m_mapFrondGeometry.empty(); }
    bool            HasLeafGeometry() const     { return !m_mapLeafGeometry.empty(); }

    bool            HasBranchLOD(int iLOD) const    { return m_mapBranchGeometry.find(iLOD) != m_mapBranchGeometry.end(); }
    bool            HasFrondLOD(int iLOD) const     { return m_mapFrondGeometry.find(iLOD) != m_mapFrondGeometry.end(); }
    bool            HasLeafLOD(int iLOD) const      { return m_mapLeafGeometry.find(iLOD) != m_mapLeafGeometry.end(); }

    ResHandle       GetBranchMaterial() const       { return m_hBranchMaterial; }
    ResHandle       GetFrondMaterial() const        { return m_vhFrondMaterials[0]; }
    ResHandle       GetLeafMaterial() const         { return m_vhLeafMaterials[0]; }
    ResHandle       GetBillboardMaterial() const    { return m_vhBillboardMaterials[0]; }

    const STreeGeometryBuffers &GetBranchGeometry(int iLOD) const   { return m_mapBranchGeometry.find(iLOD)->second; }
    const STreeGeometryBuffers &GetFrondGeometry(int iLOD) const    { return m_mapFrondGeometry.find(iLOD)->second; }
    const SLeafGeometryBuffers &GetLeafGeometry(int iLOD) const     { return m_mapLeafGeometry.find(iLOD)->second; }

    static AttributeMap s_mapBranchAttributes;
    static AttributeMap s_mapFrondAttributes;
    static AttributeMap s_mapLeafAttributes;
};
//=============================================================================
#endif //__C_TREEMODELDEF_H__
