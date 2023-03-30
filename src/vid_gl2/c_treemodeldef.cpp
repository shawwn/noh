// (C)2005 S2 Games
// c_treemodeldef.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_treemodeldef.h"
#include "c_treescenemanager.h"

#include "../k2/c_camera.h"
#include "../k2/c_treemodel.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_material.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
bool            CTreeModelDef::s_bInitialized(false);
AttributeMap    CTreeModelDef::s_mapBranchAttributes;
AttributeMap    CTreeModelDef::s_mapFrondAttributes;
AttributeMap    CTreeModelDef::s_mapLeafAttributes;

CVAR_INTF(  vid_treeNumWindMatrices,    1,      CVAR_READONLY);
CVAR_BOOL(  vid_treeUseLODs,            true);
CVAR_FLOAT( vid_treeWindStrength,       0.2f);

float   g_afLeafClusterData[LEAF_CLUSTER_TABLE_SIZE];
//=============================================================================


/*====================
  CTreeModelDef::~CTreeModelDef
  ====================*/
CTreeModelDef::~CTreeModelDef()
{
    Destroy();
}


/*====================
  CTreeModelDef::CTreeModelDef
  ====================*/
CTreeModelDef::CTreeModelDef(class CTreeModel *pModel) :
m_bValid(true),
m_pTreeModel(pModel),
m_hBranchMaterial(0),
m_bLeavesUpdated(false)
{
    if (!s_bInitialized)
    {
        Console.Warn << _T("CTreeModelDef::Init() was not called before loading a tree") << newl;
        CTreeModelDef::Init();
    }

    try
    {
        // Load Materials
        m_hBranchMaterial = g_ResourceManager.Register(pModel->GetBranchMaterial(), RES_MATERIAL);

        const tsvector vsFrondMaterials(pModel->GetFrondMaterials());
        for (tsvector_cit it(vsFrondMaterials.begin()); it != vsFrondMaterials.end(); ++it)
            m_vhFrondMaterials.push_back(g_ResourceManager.Register(*it, RES_MATERIAL));
        if (vsFrondMaterials.size() == 0)
            m_vhFrondMaterials.push_back(INVALID_RESOURCE);

        const tsvector vsLeafMaterials(pModel->GetLeafMaterials());
        for (tsvector_cit it(vsLeafMaterials.begin()); it != vsLeafMaterials.end(); ++it)
            m_vhLeafMaterials.push_back(g_ResourceManager.Register(*it, RES_MATERIAL));
        if (vsLeafMaterials.size() == 0)
            m_vhLeafMaterials.push_back(INVALID_RESOURCE);

        const tsvector vsBillboardMaterials(pModel->GetBillboardMaterials());
        for (tsvector_cit it(vsBillboardMaterials.begin()); it != vsBillboardMaterials.end(); ++it)
            m_vhBillboardMaterials.push_back(g_ResourceManager.Register(*it, RES_MATERIAL));
        if (vsBillboardMaterials.size() == 0)
            m_vhBillboardMaterials.push_back(INVALID_RESOURCE);

        // Load Geometry
        LoadBranches();
        LoadFronds();
        LoadLeaves();
    }
    catch (const tstring &sReason)
    {
        Console.Err << sReason << SPACE << ParenStr(pModel->GetName()) << newl;
        m_bValid = false;
    }
}


/*====================
  CTreeModelDef::GetBounds
  ====================*/
const CBBoxf&   CTreeModelDef::GetBounds() const
{
    return m_pTreeModel->GetBounds();
}


/*====================
  CTreeModelDef::Init
  ====================*/
void    CTreeModelDef::Init()
{
    s_bInitialized = true;
    Console << _T("Initializing SpeedTree rendering") << newl;

    CTreeModel::SetNumWindMatrices(vid_treeNumWindMatrices);

    s_mapBranchAttributes[_T("a_vTangent")] = SVertexAttribute(GL_FLOAT, 3, 32, false);
    
    //s_mapFrondAttributes;
    
    s_mapLeafAttributes[_T("a_vWind")] = SVertexAttribute(GL_FLOAT, 2, 32, false);
    s_mapLeafAttributes[_T("a_vLeaf")] = SVertexAttribute(GL_FLOAT, 3, 40, false);
}


/*====================
  CTreeModelDef::SetWindStrengh
  ====================*/
void    CTreeModelDef::SetWindStrength(float fStrength) const
{
    m_pTreeModel->SetWindStrength(fStrength);
}


/*====================
  CTreeModel::ComputeLODLevel
  ====================*/
void    CTreeModelDef::ComputeLODLevel() const
{
    PROFILE("CTreeModelDef::ComputeLODLevel");

    m_pTreeModel->ComputeLODLevel();
}


/*====================
  CTreeModelDef::GetDiscreetBranchLOD
  ====================*/
SLODData    CTreeModelDef::GetDiscreetBranchLOD() const
{
    return m_pTreeModel->GetDiscreetBranchLOD();
}


/*====================
  CTreeModelDef::GetDiscreetFrondLOD
  ====================*/
SLODData    CTreeModelDef::GetDiscreetFrondLOD() const
{
    return m_pTreeModel->GetDiscreetFrondLOD();
}


/*====================
  CTreeModelDef::GetLeafLODData
  ====================*/
void    CTreeModelDef::GetLeafLODData(SLODData avLeafLODs[]) const
{
    m_pTreeModel->GetLeafLODData(avLeafLODs);
}


/*====================
  CTreeModelDef::GetLeafBillboardTable
  ====================*/
const float*    CTreeModelDef::GetLeafBillboardTable(uint &uiSize) const
{
    if (!m_bLeavesUpdated)
    {
        // Fill in the cluster data that the vertex shader will use
        const float *pTable = m_pTreeModel->GetLeafBillboardTable(m_uiLeafClusterDataSize);
        if (m_uiLeafClusterDataSize > LEAF_CLUSTER_TABLE_SIZE)
        {
            //Console.Warn << _T("Too many entries in this tree's billboard table") << newl;
            m_uiLeafClusterDataSize = LEAF_CLUSTER_TABLE_SIZE;
        }
        MemManager.Copy(m_afLeafClusterData, pTable, m_uiLeafClusterDataSize * sizeof(float));

        m_bLeavesUpdated = true;
        for (LeafBufferLODMap::const_iterator itLeaves(m_mapLeafGeometry.begin()); itLeaves != m_mapLeafGeometry.end(); ++itLeaves)
            itLeaves->second.m_bLeavesUpdated = false;
    }

    uiSize = m_uiLeafClusterDataSize;
    return m_afLeafClusterData;
}


/*====================
  CTreeModelDef::LoadBranches
  ====================*/
void    CTreeModelDef::LoadBranches()
{
    const TreeLODMap &BranchMap(m_pTreeModel->GetBranchLODMap());

    for (TreeLODMap::const_iterator it(BranchMap.begin()); it != BranchMap.end(); ++it)
    {
        STreeGeometryBuffers    branchLOD;
        branchLOD.m_iNumVerts = it->second->usNumVerts;

        branchLOD.m_dwAlphaTest = static_cast<uint>(it->second->fAlphaTest);

        glGenBuffersARB(1, &branchLOD.m_VBuffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, branchLOD.m_VBuffer);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, branchLOD.m_iNumVerts * sizeof(SBranchVert), NULL, GL_STATIC_DRAW_ARB);

        byte* pVertices = (byte*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);
        MemManager.Copy(pVertices, it->second->pfVerts, sizeof(SBranchVert) * branchLOD.m_iNumVerts);
        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

        // Create and fill index buffers
        for (unsigned short us(0); us < it->second->usNumLists; ++us)
        {
            int iStripLength(it->second->pusNumIndices[us]);

            GLuint uiIB;
            glGenBuffersARB(1, &uiIB);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, uiIB);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, iStripLength * sizeof(ushort), NULL, GL_STATIC_DRAW_ARB);

            byte* pIndices = (byte*)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
            MemManager.Copy(pIndices, it->second->ppwIndices[us], iStripLength * sizeof(ushort));
            glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER);

            branchLOD.m_viNumIndices.push_back(iStripLength);
            branchLOD.m_vIBuffers.push_back(uiIB);
        }

        m_mapBranchGeometry[it->first] = branchLOD;
    }
}


/*====================
  CTreeModelDef::LoadFronds
  ====================*/
void    CTreeModelDef::LoadFronds()
{
    const TreeLODMap &FrondMap(m_pTreeModel->GetFrondLODMap());

    for (TreeLODMap::const_iterator it(FrondMap.begin()); it != FrondMap.end(); ++it)
    {
        STreeGeometryBuffers    frondLOD;
        frondLOD.m_iNumVerts = it->second->usNumVerts;

        frondLOD.m_dwAlphaTest = static_cast<uint>(it->second->fAlphaTest);

        glGenBuffersARB(1, &frondLOD.m_VBuffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, frondLOD.m_VBuffer);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, frondLOD.m_iNumVerts * sizeof(SFrondVert), NULL, GL_STATIC_DRAW_ARB);

        byte* pVertices = (byte*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);
        MemManager.Copy(pVertices, it->second->pfVerts, sizeof(SFrondVert) * frondLOD.m_iNumVerts);
        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

        // Create and fill index buffers
        for (unsigned short us(0); us < it->second->usNumLists; ++us)
        {
            int iStripLength(it->second->pusNumIndices[us]);

            GLuint uiIB;
            glGenBuffersARB(1, &uiIB);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, uiIB);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, iStripLength * sizeof(ushort), NULL, GL_STATIC_DRAW_ARB);

            byte* pIndices = (byte*)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
            MemManager.Copy(pIndices, it->second->ppwIndices[us], iStripLength * sizeof(ushort));
            glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER);

            frondLOD.m_viNumIndices.push_back(iStripLength);
            frondLOD.m_vIBuffers.push_back(uiIB);
        }

        m_mapFrondGeometry[it->first] = frondLOD;
    }
}


/*====================
  CTreeModelDef::LoadLeaves
  ====================*/
void    CTreeModelDef::LoadLeaves()
{
    const TreeLODMap &LeafMap(m_pTreeModel->GetLeafLODMap());

    for (TreeLODMap::const_iterator it(LeafMap.begin()); it != LeafMap.end(); ++it)
    {
        SLeafGeometryBuffers    leafLOD;
        leafLOD.m_iNumVerts = it->second->usNumVerts;

        leafLOD.m_dwAlphaTest = static_cast<uint>(it->second->fAlphaTest);
        leafLOD.m_bLeavesUpdated = false;

        glGenBuffersARB(1, &leafLOD.m_VBuffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, leafLOD.m_VBuffer);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, leafLOD.m_iNumVerts * sizeof(SLeafVert), NULL, GL_STATIC_DRAW_ARB);

        byte* pVertices = (byte*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);
        MemManager.Copy(pVertices, it->second->pfVerts, sizeof(SLeafVert) * leafLOD.m_iNumVerts);
        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

        // Create and fill index buffers
        for (unsigned short us(0); us < it->second->usNumLists; ++us)
        {
            int iStripLength(it->second->pusNumIndices[us]);

            GLuint uiIB;
            glGenBuffersARB(1, &uiIB);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, uiIB);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, iStripLength * sizeof(ushort), NULL, GL_STATIC_DRAW_ARB);

            byte* pIndices = (byte*)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
            MemManager.Copy(pIndices, it->second->ppwIndices[us], iStripLength * sizeof(ushort));
            glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER);

            leafLOD.m_viNumIndices.push_back(iStripLength);
            leafLOD.m_vIBuffers.push_back(uiIB);
        }

        m_mapLeafGeometry[it->first] = leafLOD;
    }
}


/*====================
  CTreeModelDef::Destroy
  ====================*/
void    CTreeModelDef::Destroy()
{
    // Delete branch buffers
    for (TreeBufferLODMap::iterator itBranches(m_mapBranchGeometry.begin()); itBranches != m_mapBranchGeometry.end(); ++itBranches)
    {
        glDeleteBuffersARB(1, &itBranches->second.m_VBuffer);
        for (vector<GLuint>::iterator it(itBranches->second.m_vIBuffers.begin()); it != itBranches->second.m_vIBuffers.end(); ++it)
            glDeleteBuffersARB(1, &(*it));
    }

    // Delete frond buffers
    for (TreeBufferLODMap::iterator itFronds(m_mapFrondGeometry.begin()); itFronds != m_mapFrondGeometry.end(); ++itFronds)
    {
        glDeleteBuffersARB(1, &itFronds->second.m_VBuffer);
        for (vector<GLuint>::iterator it(itFronds->second.m_vIBuffers.begin()); it != itFronds->second.m_vIBuffers.end(); ++it)
            glDeleteBuffersARB(1, &(*it));
    }

    // Delete leaf buffers
    for (LeafBufferLODMap::iterator itLeaves(m_mapLeafGeometry.begin()); itLeaves != m_mapLeafGeometry.end(); ++itLeaves)
    {
        glDeleteBuffersARB(1, &itLeaves->second.m_VBuffer);
        for (vector<GLuint>::iterator it(itLeaves->second.m_vIBuffers.begin()); it != itLeaves->second.m_vIBuffers.end(); ++it)
            glDeleteBuffersARB(1, &(*it));
    }
}
