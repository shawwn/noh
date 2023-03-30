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
#include "d3d9f_main.h"
#include "d3d9f_material.h"
#include "d3d9f_shader.h"
#include "d3d9f_state.h"
#include "d3d9f_util.h"
#include "d3d9f_model.h"
#include "d3d9f_scene.h"

#include "../k2/c_camera.h"
#include "../k2/c_treemodel.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_material.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
DWORD           CTreeModelDef::s_dwBranchFVF;
int             CTreeModelDef::s_iBranchVertDecl;
uint            CTreeModelDef::s_uiBranchVertSize;
DWORD           CTreeModelDef::s_dwFrondFVF;
int             CTreeModelDef::s_iFrondVertDecl;
uint            CTreeModelDef::s_uiFrondVertSize;
DWORD           CTreeModelDef::s_dwLeafFVF;
int             CTreeModelDef::s_iLeafVertDecl;
uint            CTreeModelDef::s_uiLeafVertSize;

bool            CTreeModelDef::s_bInitialized(false);

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
        for (tsvector::const_iterator it(vsFrondMaterials.begin()); it != vsFrondMaterials.end(); ++it)
            m_vhFrondMaterials.push_back(g_ResourceManager.Register(*it, RES_MATERIAL));
        if (vsFrondMaterials.size() == 0)
            m_vhFrondMaterials.push_back(INVALID_RESOURCE);

        const tsvector vsLeafMaterials(pModel->GetLeafMaterials());
        for (tsvector::const_iterator it(vsLeafMaterials.begin()); it != vsLeafMaterials.end(); ++it)
            m_vhLeafMaterials.push_back(g_ResourceManager.Register(*it, RES_MATERIAL));
        if (vsLeafMaterials.size() == 0)
            m_vhLeafMaterials.push_back(INVALID_RESOURCE);

        const tsvector vsBillboardMaterials(pModel->GetBillboardMaterials());
        for (tsvector::const_iterator it(vsBillboardMaterials.begin()); it != vsBillboardMaterials.end(); ++it)
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

    s_dwBranchFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE3(1) | D3DFVF_TEX2;
    s_iBranchVertDecl = D3D_RegisterVertexDeclaration(s_dwBranchFVF);
    s_uiBranchVertSize = sizeof(SBranchVert);

    s_dwFrondFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
    s_iFrondVertDecl = D3D_RegisterVertexDeclaration(s_dwFrondFVF);
    s_uiFrondVertSize = sizeof(SFrondVert);

    s_dwLeafFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1) | D3DFVF_TEXCOORDSIZE3(2) | D3DFVF_TEX3;
    s_iLeafVertDecl = D3D_RegisterVertexDeclaration(s_dwLeafFVF);
    s_uiLeafVertSize = sizeof(SLeafVert);
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

        branchLOD.m_dwAlphaTest = static_cast<DWORD>(it->second->fAlphaTest);

        // Create and fill vertex buffer
        if (FAILED(g_pd3dDevice->CreateVertexBuffer(branchLOD.m_iNumVerts * s_uiBranchVertSize,
            D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &branchLOD.m_pVBuffer, NULL)))
            throw _TS("CTreeModelDef::LoadBranches(): Failed to create vertex buffer for branches");

        SBranchVert *pVerts;
        if (FAILED(branchLOD.m_pVBuffer->Lock(0, branchLOD.m_iNumVerts * s_uiBranchVertSize, (void**)&pVerts, D3DLOCK_NOSYSLOCK)))
            throw _TS("CTreeModelDef::LoadBranches(): Failed to lock branch vertex buffer");

        MemManager.Copy(pVerts, it->second->pfVerts, sizeof(SBranchVert) * branchLOD.m_iNumVerts);

        if (FAILED(branchLOD.m_pVBuffer->Unlock()))
            throw _TS("CTreeModelDef::LoadBranches(): Failed to unlock branch vertex buffer");

        if (vid_geometryPreload)
            branchLOD.m_pVBuffer->PreLoad();

        // Create and fill index buffers
        for (unsigned short us(0); us < it->second->usNumLists; ++us)
        {
            int iStripLength(it->second->pusNumIndices[us]);
            IDirect3DIndexBuffer9   *pIB(NULL);
            if (FAILED(g_pd3dDevice->CreateIndexBuffer(iStripLength * sizeof(WORD),
                D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIB, NULL)))
                throw _TS("CTreeModelDef::LoadBranches(): Failed to create index buffer for branches");

            WORD    *pIndices;
            if (FAILED(pIB->Lock(0, iStripLength * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
                throw _TS("CTreeModelDef::LoadBranches(): Failed to lock branch index buffer");

            MemManager.Copy(pIndices, it->second->ppwIndices[us], iStripLength * sizeof(WORD));

            if (FAILED(pIB->Unlock()))
                throw _T("CTreeModelDef::LoadBranches(): Failed to unlock branch index buffer");

            if (vid_geometryPreload)
                pIB->PreLoad();

            branchLOD.m_viNumIndices.push_back(iStripLength);
            branchLOD.m_vpIBuffers.push_back(pIB);
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

        frondLOD.m_dwAlphaTest = static_cast<DWORD>(it->second->fAlphaTest);

        // Create and fill vertex buffer
        if (FAILED(g_pd3dDevice->CreateVertexBuffer(frondLOD.m_iNumVerts * s_uiFrondVertSize,
            D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &frondLOD.m_pVBuffer, NULL)))
            throw _TS("CTreeModelDef::LoadFronds(): Failed to create vertex buffer for fronds");

        SFrondVert  *pVerts;
        if (FAILED(frondLOD.m_pVBuffer->Lock(0, frondLOD.m_iNumVerts * s_uiFrondVertSize, (void**)&pVerts, D3DLOCK_NOSYSLOCK)))
            throw _TS("CTreeModelDef::LoadFronds(): Failed to lock frond vertex buffer");

        MemManager.Copy(pVerts, it->second->pfVerts, sizeof(SFrondVert) * frondLOD.m_iNumVerts);

        if (FAILED(frondLOD.m_pVBuffer->Unlock()))
            throw _TS("CTreeModelDef::LoadFronds(): Failed to unlock frond vertex buffer");

        // Create and fill index buffers
        for (unsigned short us(0); us < it->second->usNumLists; ++us)
        {
            int iStripLength(it->second->pusNumIndices[us]);
            IDirect3DIndexBuffer9   *pIB(NULL);
            if (FAILED(g_pd3dDevice->CreateIndexBuffer(iStripLength * sizeof(WORD),
                D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIB, NULL)))
                throw _TS("CTreeModelDef::LoadFronds(): Failed to create index buffer for fronds");

            WORD    *pIndices;
            if (FAILED(pIB->Lock(0, iStripLength * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
                throw _TS("CTreeModelDef::LoadFronds(): Failed to lock frond index buffer");

            MemManager.Copy(pIndices, it->second->ppwIndices[us], iStripLength * sizeof(WORD));

            if (FAILED(pIB->Unlock()))
                throw _T("CTreeModelDef::LoadFronds(): Failed to unlock frond index buffer");

            frondLOD.m_viNumIndices.push_back(iStripLength);
            frondLOD.m_vpIBuffers.push_back(pIB);
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
        STreeGeometryBuffers    leafLOD;
        leafLOD.m_iNumVerts = it->second->usNumVerts;

        // Create and fill vertex buffer
        if (FAILED(g_pd3dDevice->CreateVertexBuffer(leafLOD.m_iNumVerts * s_uiLeafVertSize,
            D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &leafLOD.m_pVBuffer, NULL)))
            throw _TS("CTreeModelDef::LoadFronds(): Failed to create vertex buffer for leavees");

        SLeafVert   *pVerts;
        if (FAILED(leafLOD.m_pVBuffer->Lock(0, leafLOD.m_iNumVerts * s_uiLeafVertSize, (void**)&pVerts, D3DLOCK_NOSYSLOCK)))
            throw _TS("CTreeModelDef::LoadFronds(): Failed to lock leaf vertex buffer");

        MemManager.Copy(pVerts, it->second->pfVerts, sizeof(SLeafVert) * leafLOD.m_iNumVerts);

        if (FAILED(leafLOD.m_pVBuffer->Unlock()))
            throw _TS("CTreeModelDef::LoadFronds(): Failed to unlock leaf vertex buffer");

        // Create and fill index buffers
        for (unsigned short us(0); us < it->second->usNumLists; ++us)
        {
            int iStripLength(it->second->pusNumIndices[us]);
            IDirect3DIndexBuffer9   *pIB(NULL);
            if (FAILED(g_pd3dDevice->CreateIndexBuffer(iStripLength * sizeof(WORD),
                D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIB, NULL)))
                throw _TS("CTreeModelDef::LoadFronds(): Failed to create index buffer for leaves");

            WORD    *pIndices;
            if (FAILED(pIB->Lock(0, iStripLength * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
                throw _TS("CTreeModelDef::LoadFronds(): Failed to lock leaf index buffer");

            MemManager.Copy(pIndices, it->second->ppwIndices[us], iStripLength * sizeof(WORD));

            if (FAILED(pIB->Unlock()))
                throw _T("CTreeModelDef::LoadFronds(): Failed to unlock leaf index buffer");

            leafLOD.m_viNumIndices.push_back(iStripLength);
            leafLOD.m_vpIBuffers.push_back(pIB);
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
        SAFE_RELEASE(itBranches->second.m_pVBuffer);

        for (vector<IDirect3DIndexBuffer9*>::iterator it(itBranches->second.m_vpIBuffers.begin()); it != itBranches->second.m_vpIBuffers.end(); ++it)
            SAFE_RELEASE(*it);
    }

    // Delete frond buffers
    for (TreeBufferLODMap::iterator itFronds(m_mapFrondGeometry.begin()); itFronds != m_mapFrondGeometry.end(); ++itFronds)
    {
        SAFE_RELEASE(itFronds->second.m_pVBuffer);

        for (vector<IDirect3DIndexBuffer9*>::iterator it(itFronds->second.m_vpIBuffers.begin()); it != itFronds->second.m_vpIBuffers.end(); ++it)
            SAFE_RELEASE(*it);
    }

    // Delete leaf buffers
    for (TreeBufferLODMap::iterator itLeaves(m_mapLeafGeometry.begin()); itLeaves != m_mapLeafGeometry.end(); ++itLeaves)
    {
        SAFE_RELEASE(itLeaves->second.m_pVBuffer);

        for (vector<IDirect3DIndexBuffer9*>::iterator itIndices(itLeaves->second.m_vpIBuffers.begin()); itIndices != itLeaves->second.m_vpIBuffers.end(); ++itIndices)
            SAFE_RELEASE(*itIndices);
    }
}
