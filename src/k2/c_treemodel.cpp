// (C)2005 S2 Games
// c_treemodel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_treemodel.h"
#include "c_xmlmanager.h"
#include "i_modelallocator.h"
#include "c_convexpolyhedron.h"
#include "c_vid.h"

#include "SpeedTreeRT.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_BOOL   (trees_useGPUWind,  true);

DEFINE_MODEL_ALLOCATOR(CTreeModel, SpeedTree);
//=============================================================================

/*====================
  CTreeModel::~CTreeModel
  ====================*/
CTreeModel::~CTreeModel()
{
}


/*====================
  CTreeModel::CTreeModel
  ====================*/
CTreeModel::CTreeModel() :
IModel(MODEL_SPEEDTREE),
m_pSpeedTree(NULL),

m_uiVidDefIndex(INVALID_INDEX),

m_fWindPrevStrength(-1.0f),
m_fWindFreqOffset(-1.0f)
{
}


/*====================
  CTreeModel::Load
  ====================*/
bool    CTreeModel::Load(const tstring &sFileName, uint uiIgnoreFlags)
{
    try
    {
        // Allocate a SpeedTree object
        m_pSpeedTree = ::K2_NEW(ctx_Models,  CSpeedTreeRT);
        if (m_pSpeedTree == NULL)
            EX_ERROR(_T("Failed to allocate a CSpeedTreeRT object"));

        // Read file from disk
        CFileHandle hFile(sFileName, FILE_READ | FILE_BINARY);
        if (!hFile.IsOpen())
            EX_ERROR(_T("Failed to open file"));

        uint uiSize(0);
        const byte *pBuffer(reinterpret_cast<const byte*>(hFile.GetBuffer(uiSize)));
        if (!m_pSpeedTree->LoadTree(pBuffer, uiSize))
            EX_ERROR(_T("CSpeedTreeRT::LoadTree() failed"));

        // Tell SpeedTree that we want to do our own lighting
        m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
        m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
        m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);

        // Tell SpeedTree we want to do GPU transforms
        m_pSpeedTree->SetBranchWindMethod(trees_useGPUWind ? CSpeedTreeRT::WIND_GPU : CSpeedTreeRT::WIND_CPU);
        m_pSpeedTree->SetFrondWindMethod(trees_useGPUWind ? CSpeedTreeRT::WIND_GPU : CSpeedTreeRT::WIND_CPU);
        m_pSpeedTree->SetLeafWindMethod(trees_useGPUWind ? CSpeedTreeRT::WIND_GPU : CSpeedTreeRT::WIND_CPU);

        // FIXME: Load this from the xml file
        m_pSpeedTree->SetLeafRockingState(true);
        m_pSpeedTree->SetNumLeafRockingGroups(3);
        CSpeedTreeRT::SetDropToBillboard(false);

        // Generate and retrieve tree data
        if (!m_pSpeedTree->Compute(NULL, 1, true))
            EX_ERROR(_T("CSpeedTreeRT::Compute() failed"));

        // Reset rand seed, as m_pSpeedTree->Compute changes it to a preset value
        srand(K2System.GetRandomSeed32());

        // Load Geometry
        LoadBranchGeometry();
        LoadFrondGeometry();
        LoadLeafGeometry();

        // Collision surfaces
        for (uint ui(0); ui < m_pSpeedTree->GetCollisionObjectCount(); ++ui)
        {
            CSpeedTreeRT::ECollisionObjectType eType;
            CVec3f  v3Pos, v3Dim;
            m_pSpeedTree->GetCollisionObject(ui, eType, v3Pos, v3Dim);

            CVec3f v3Min, v3Max;
            switch (eType)
            {
            case CSpeedTreeRT::CO_BOX:
                v3Min = CVec3f(-v3Dim.x / 2.0f, -v3Dim.y / 2.0f, 0.0f);
                v3Max = CVec3f(v3Dim.x / 2.0f, v3Dim.y / 2.0f, v3Dim.z);
                break;

            // FIXME: Make an actual cylinder
            case CSpeedTreeRT::CO_CYLINDER:
                v3Min = CVec3f(-v3Dim.x / 2.0f, -v3Dim.x / 2.0f, 0.0f);
                v3Max = CVec3f(v3Dim.x / 2.0f, v3Dim.x / 2.0f, v3Dim.y);
                break;

            // FIXME: Make an actual sphere
            case CSpeedTreeRT::CO_SPHERE:
                v3Min = CVec3f(-v3Dim.x / 2.0f, -v3Dim.x / 2.0f, -v3Dim.x / 2.0f);
                v3Max = CVec3f(v3Dim.x / 2.0f, v3Dim.x / 2.0f, v3Dim.x / 2.0f);
                break;
            }

            AddCollisionSurf(CConvexPolyhedron(CBBoxf(v3Pos + v3Min, v3Pos + v3Max)));

            m_bbCollisionBounds.push_back(CBBoxf(v3Pos + v3Min, v3Pos + v3Max));
        }

        m_pSpeedTree->GetBoundingBox((float*)&m_bbBounds);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTreeModel::Load") + ParenStr(sFileName) + _T(" - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CTreeModel::PostLoad
  ====================*/
void    CTreeModel::PostLoad()
{
    FreeGeometry();
}


/*====================
  CTreeModel::LoadBranchGeometry
  ====================*/
void    CTreeModel::LoadBranchGeometry()
{
    for (unsigned short usLOD(0); usLOD < m_pSpeedTree->GetNumBranchLodLevels(); ++usLOD)
    {
        m_pSpeedTree->GetGeometry(m_Geometry, SpeedTree_BranchGeometry, usLOD, -1, -1);
        CSpeedTreeRT::SGeometry::SIndexed &branches = m_Geometry.m_sBranches;

        if (branches.m_usVertexCount == 0)
            continue;

        // Allocate geometry chunk
        STreeGeometry *pBranchGeometry = K2_NEW(ctx_Models,  STreeGeometry);
        if (pBranchGeometry == NULL)
            EX_WARN(_T("CTreeModel::LoadBranchGeometry() - Failed to allocate geometry chunk"));

        // Store misc data
        pBranchGeometry->fAlphaTest = m_Geometry.m_fBranchAlphaTestValue;

        // Allocate memory for verts
        pBranchGeometry->usNumVerts = branches.m_usVertexCount;
        pBranchGeometry->pfVerts = K2_NEW_ARRAY(ctx_Models, float, sizeof(SBranchVert)*pBranchGeometry->usNumVerts);
        if (pBranchGeometry->pfVerts == NULL)
            EX_WARN(_T("CTreeModel::LoadBranchGeometry() - Failed to allocate memory for verts"));

        // Store interlaced vertex data
        SBranchVert *pfVerts = (SBranchVert*)pBranchGeometry->pfVerts;
        for (unsigned short us(0); us < pBranchGeometry->usNumVerts; ++us)
        {
            pfVerts->v3Pos.Set(branches.m_pCoords[us * 3 + X], branches.m_pCoords[us * 3 + Y], branches.m_pCoords[us * 3 + Z]);
            pfVerts->v3Nml.Set(branches.m_pNormals[us * 3 + X], branches.m_pNormals[us * 3 + Y], branches.m_pNormals[us * 3 + Z]);
            pfVerts->v2Tex0.Set(branches.m_pTexCoords0[us * 2 + X], branches.m_pTexCoords0[us * 2 + Y]);
            pfVerts->v3Tan.Set(branches.m_pTangents[us * 3 + X], branches.m_pTangents[us * 3 + Y], branches.m_pTangents[us * 3 + Z]);
            ++pfVerts;
        }

        // Allocate memory for index lists
        pBranchGeometry->usNumLists = branches.m_usNumStrips;
        pBranchGeometry->pusNumIndices = K2_NEW(ctx_Models,  unsigned)short[pBranchGeometry->usNumLists];
        if (pBranchGeometry->pusNumIndices == NULL)
            EX_WARN(_T("CTreeModel::LoadBranchGeometry() - Failed to allocate memory for index list lengths"));
        pBranchGeometry->ppwIndices = K2_NEW_ARRAY(ctx_Models, word*, pBranchGeometry->usNumLists);
        if (pBranchGeometry->ppwIndices == NULL)
            EX_WARN(_T("CTreeModel::LoadBranchGeometry() - Failed to allocate memory for index lists"));

        // Create and fill index lists
        for (unsigned short usLists(0); usLists < pBranchGeometry->usNumLists; ++usLists)
        {
            pBranchGeometry->pusNumIndices[usLists] = branches.m_pStripLengths[usLists];
            pBranchGeometry->ppwIndices[usLists] = K2_NEW_ARRAY(ctx_Models, word, pBranchGeometry->pusNumIndices[usLists]);
            if (pBranchGeometry->ppwIndices == NULL)
                EX_WARN(_T("CTreeModel::LoadBranchGeometry() - Failed to allocate memory for index list"));

            for (size_t z(0); z < pBranchGeometry->pusNumIndices[usLists]; ++z)
                pBranchGeometry->ppwIndices[usLists][z] = branches.m_pStrips[usLists][pBranchGeometry->pusNumIndices[usLists] - z - 1];
        }

        m_mapBranchGeometry[branches.m_nDiscreteLodLevel] = pBranchGeometry;
    }

    //m_pSpeedTree->DeleteBranchGeometry();
}


/*====================
  CTreeModel::LoadFrondGeometry
  ====================*/
void    CTreeModel::LoadFrondGeometry()
{
    for (unsigned short usLOD(0); usLOD < m_pSpeedTree->GetNumFrondLodLevels(); ++usLOD)
    {
        CSpeedTreeRT::SGeometry m_Geometry;
        m_pSpeedTree->GetGeometry(m_Geometry, SpeedTree_FrondGeometry, -1, usLOD, -1);
        CSpeedTreeRT::SGeometry::SIndexed &fronds = m_Geometry.m_sFronds;

        if (fronds.m_usVertexCount == 0)
            continue;

        // Allocate geometry chunk
        STreeGeometry *pFrondGeometry = K2_NEW(ctx_Models,  STreeGeometry);
        if (pFrondGeometry == NULL)
            EX_WARN(_T("CTreeModel::LoadFrondGeometry() - Failed to allocate geometry chunk"));

        // Store misc data
        pFrondGeometry->fAlphaTest = m_Geometry.m_fFrondAlphaTestValue;

        // Allocate memory for verts
        pFrondGeometry->usNumVerts = fronds.m_usVertexCount;
        pFrondGeometry->pfVerts = K2_NEW_ARRAY(ctx_Models, float, sizeof(SFrondVert)*pFrondGeometry->usNumVerts);
        if (pFrondGeometry->pfVerts == NULL)
            EX_WARN(_T("CTreeModel::LoadFrondGeometry() - Failed to allocate memory for verts"));

        // Store interlaced vertex data
        SFrondVert *pfVerts = (SFrondVert*)pFrondGeometry->pfVerts;
        for (unsigned short us(0); us < pFrondGeometry->usNumVerts; ++us)
        {
            pfVerts->v3Pos.Set(fronds.m_pCoords[us * 3 + X], fronds.m_pCoords[us * 3 + Y], fronds.m_pCoords[us * 3 + Z]);
            pfVerts->v3Nml.Set(fronds.m_pNormals[us * 3 + X], fronds.m_pNormals[us * 3 + Y], fronds.m_pNormals[us * 3 + Z]);
            pfVerts->v2Tex0.Set(fronds.m_pTexCoords0[us * 2 + X], fronds.m_pTexCoords0[us * 2 + Y]);
            ++pfVerts;
        }

        // Allocate memory for index lists
        pFrondGeometry->usNumLists = fronds.m_usNumStrips;
        pFrondGeometry->pusNumIndices = K2_NEW(ctx_Models,  unsigned)short[pFrondGeometry->usNumLists];
        if (pFrondGeometry->pusNumIndices == NULL)
            EX_WARN(_T("CTreeModel::LoadFrondGeometry() - Failed to allocate memory for index list lengths"));
        pFrondGeometry->ppwIndices = K2_NEW_ARRAY(ctx_Models, word*, pFrondGeometry->usNumLists);
        if (pFrondGeometry->ppwIndices == NULL)
            EX_WARN(_T("CTreeModel::LoadFrondGeometry() - Failed to allocate memory for index lists"));

        // Create and fill index lists
        for (unsigned short usLists(0); usLists < pFrondGeometry->usNumLists; ++usLists)
        {
            pFrondGeometry->pusNumIndices[usLists] = fronds.m_pStripLengths[usLists];
            pFrondGeometry->ppwIndices[usLists] = K2_NEW_ARRAY(ctx_Models, word, pFrondGeometry->pusNumIndices[usLists]);
            if (pFrondGeometry->ppwIndices == NULL)
                throw _TS("CTreeModel::LoadFrondGeometry() - Failed to allocate memory for index list");

            for (size_t z(0); z < pFrondGeometry->pusNumIndices[usLists]; ++z)
                pFrondGeometry->ppwIndices[usLists][z] = fronds.m_pStrips[usLists][pFrondGeometry->pusNumIndices[usLists] - z - 1];
        }

        m_mapFrondGeometry[fronds.m_nDiscreteLodLevel] = pFrondGeometry;
    }

    //m_pSpeedTree->DeleteFrondGeometry();
}


/*====================
  CTreeModel::LoadLeafGeometry
  ====================*/
void    CTreeModel::LoadLeafGeometry()
{
    for (unsigned short usLOD(0); usLOD < m_pSpeedTree->GetNumLeafLodLevels(); ++usLOD)
    {
        CSpeedTreeRT::SGeometry m_Geometry;
        m_pSpeedTree->GetGeometry(m_Geometry, SpeedTree_LeafGeometry, -1, -1, usLOD);
        CSpeedTreeRT::SGeometry::SLeaf &leaves = m_Geometry.m_sLeaves0;

        if (leaves.m_usLeafCount == 0)
            continue;

        // Allocate geometry chunk
        STreeGeometry *pLeafGeometry = K2_NEW(ctx_Models,  STreeGeometry);
        if (pLeafGeometry == NULL)
            EX_WARN(_T("CTreeModel::LoadLeafGeometry() - Failed to allocate geometry chunk"));

        // Allocate memory for verts
        pLeafGeometry->usNumVerts = leaves.m_usLeafCount * 4;
        pLeafGeometry->pfVerts = K2_NEW_ARRAY(ctx_Models, float, sizeof(SLeafVert)*pLeafGeometry->usNumVerts);
        if (pLeafGeometry->pfVerts == NULL)
            EX_WARN(_T("CTreeModel::LoadLeafGeometry() - Failed to allocate memory for verts"));

        // Store interlaced vertex data
        SLeafVert *pfVerts = (SLeafVert*)pLeafGeometry->pfVerts;
        for (unsigned short us(0); us < leaves.m_usLeafCount * 4; ++us, ++pfVerts)
        {
            int iLeaf(us / 4);
            int iVert(us % 4);
            pfVerts->v3Pos.Set(leaves.m_pCenterCoords[iLeaf * 3 + X], leaves.m_pCenterCoords[iLeaf * 3 + Y], leaves.m_pCenterCoords[iLeaf * 3 + Z]);
            pfVerts->v3Nml.Set(leaves.m_pNormals[iLeaf * 3 + X], leaves.m_pNormals[iLeaf * 3 + Y], leaves.m_pNormals[iLeaf * 3 + Z]);
            pfVerts->v2Tex0.Set(leaves.m_pLeafMapTexCoords[iLeaf][iVert * 2 + X], leaves.m_pLeafMapTexCoords[iLeaf][iVert * 2 + Y]);
            pfVerts->v2Wind.Set(leaves.m_pWindMatrixIndices[iLeaf], leaves.m_pWindWeights[us / 4]);
            pfVerts->v3Leaf.Set(float((leaves.m_pLeafClusterIndices[iLeaf] * 4) + iVert),
                m_pSpeedTree->GetLeafLodSizeAdjustments()[usLOD],
                leaves.m_pLeafClusterIndices[iLeaf]);
        }

        // Allocate memory for index lists
        pLeafGeometry->usNumLists = 1;
        pLeafGeometry->pusNumIndices = K2_NEW(ctx_Models,  unsigned)short[pLeafGeometry->usNumLists];
        if (pLeafGeometry->pusNumIndices == NULL)
            EX_WARN(_T("CTreeModel::LoadLeafGeometry() - Failed to allocate memory for index list lengths"));
        pLeafGeometry->ppwIndices = K2_NEW_ARRAY(ctx_Models,  word*, pLeafGeometry->usNumLists);
        if (pLeafGeometry->ppwIndices == NULL)
            EX_WARN(_T("CTreeModel::LoadLeafGeometry() - Failed to allocate memory for index lists"));

        // Create and fill index lists
        for (unsigned short usLists(0); usLists < pLeafGeometry->usNumLists; ++usLists)
        {
            pLeafGeometry->pusNumIndices[usLists] = leaves.m_usLeafCount * 6;
            pLeafGeometry->ppwIndices[usLists] = K2_NEW_ARRAY(ctx_Models, word, pLeafGeometry->pusNumIndices[usLists]);
            if (pLeafGeometry->ppwIndices == NULL)
                EX_WARN(_T("CTreeModel::LoadLeafGeometry() - Failed to allocate memory for index list"));

            word pzOffsets[6] = { 0, 3, 2, 2, 1, 0 };
            for (word w(0); w < pLeafGeometry->pusNumIndices[usLists]; ++w)
                pLeafGeometry->ppwIndices[usLists][w] = (w / 6) * 4 + pzOffsets[w % 6];
        }

        m_mapLeafGeometry[leaves.m_nDiscreteLodLevel] = pLeafGeometry;
    }
}


/*====================
  CTreeModel::FreeGeometry
  ====================*/
void    CTreeModel::FreeGeometry()
{
    for (TreeLODMap::iterator it(m_mapBranchGeometry.begin()); it != m_mapBranchGeometry.end(); ++it)
    {
        SAFE_DELETE_ARRAY(it->second->pfVerts);
        for (unsigned short us(0); us < it->second->usNumLists; ++us)
        {
            SAFE_DELETE_ARRAY(it->second->ppwIndices[us]);
        }
        SAFE_DELETE_ARRAY(it->second->ppwIndices);
        SAFE_DELETE_ARRAY(it->second->pusNumIndices);
        SAFE_DELETE(it->second);
    }
    m_mapBranchGeometry.clear();

    for (TreeLODMap::iterator it(m_mapFrondGeometry.begin()); it != m_mapFrondGeometry.end(); ++it)
    {
        SAFE_DELETE_ARRAY(it->second->pfVerts);
        for (unsigned short us(0); us < it->second->usNumLists; ++us)
        {
            SAFE_DELETE_ARRAY(it->second->ppwIndices[us]);
        }
        SAFE_DELETE_ARRAY(it->second->ppwIndices);
        SAFE_DELETE_ARRAY(it->second->pusNumIndices);
        SAFE_DELETE(it->second);
    }
    m_mapFrondGeometry.clear();

    for (TreeLODMap::iterator it(m_mapLeafGeometry.begin()); it != m_mapLeafGeometry.end(); ++it)
    {
        K2_DELETE_ARRAY(it->second->pfVerts);
        for (unsigned short us(0); us < it->second->usNumLists; ++us)
        {
            K2_DELETE_ARRAY(it->second->ppwIndices[us]);
        }
        K2_DELETE_ARRAY(it->second->ppwIndices);
        K2_DELETE_ARRAY(it->second->pusNumIndices);
        K2_DELETE(it->second);
    }
    m_mapLeafGeometry.clear();
}


/*====================
  CTreeModel::AddFrondMaterial
  ====================*/
void    CTreeModel::AddFrondMaterial(const tstring &sFileName)
{
    for (tsvector::iterator it(m_vsFrondMaterials.begin()); it != m_vsFrondMaterials.end(); ++it)
    {
        if (*it == sFileName)
            return;
    }

    m_vsFrondMaterials.push_back(sFileName);
}


/*====================
  CTreeModel::AddLeafMaterial
  ====================*/
void    CTreeModel::AddLeafMaterial(const tstring &sFileName)
{
    for (tsvector::iterator it(m_vsLeafMaterials.begin()); it != m_vsLeafMaterials.end(); ++it)
    {
        if (*it == sFileName)
            return;
    }

    m_vsLeafMaterials.push_back(sFileName);
}


/*====================
  CTreeModel::AddBillboardMaterial
  ====================*/
void    CTreeModel::AddBillboardMaterial(const tstring &sFileName)
{
    for (tsvector::iterator it(m_vsBillboardMaterials.begin()); it != m_vsBillboardMaterials.end(); ++it)
    {
        if (*it == sFileName)
            return;
    }

    m_vsBillboardMaterials.push_back(sFileName);
}


/*====================
  CTreeModel::ComputeLODLevel
  ====================*/
void    CTreeModel::ComputeLODLevel() const
{
    m_pSpeedTree->ComputeLodLevel();
}


/*====================
  CTreeModel::GetDiscreetBranchLOD
  ====================*/
SLODData    CTreeModel::GetDiscreetBranchLOD() const
{
    m_pSpeedTree->GetGeometry(m_Geometry, SpeedTree_BranchGeometry);

    SLODData lod;
    lod.m_bActive = true;
    lod.m_iLOD = static_cast<dword>(m_Geometry.m_sBranches.m_nDiscreteLodLevel);
    lod.m_dwAlphaTest = static_cast<dword>(m_Geometry.m_fBranchAlphaTestValue);

    return lod;
}


/*====================
  CTreeModel::GetDiscreetFrondLOD
  ====================*/
SLODData    CTreeModel::GetDiscreetFrondLOD() const
{
    m_pSpeedTree->GetGeometry(m_Geometry, SpeedTree_FrondGeometry);

    SLODData lod;
    lod.m_bActive = true;
    lod.m_iLOD = static_cast<dword>(m_Geometry.m_sFronds.m_nDiscreteLodLevel);
    lod.m_dwAlphaTest = static_cast<dword>(m_Geometry.m_fFrondAlphaTestValue);

    return lod;
}


/*====================
  CTreeModel::GetLeafLODData
  ====================*/
void    CTreeModel::GetLeafLODData(SLODData avLeafLODs[]) const
{
    // Get the discrete leaf data
    m_pSpeedTree->GetGeometry(m_Geometry, SpeedTree_LeafGeometry | SpeedTree_LeafLods);

    if (m_Geometry.m_sLeaves0.m_bIsActive)
        avLeafLODs[0] = SLODData(true, m_Geometry.m_sLeaves0.m_nDiscreteLodLevel, dword(m_Geometry.m_sLeaves0.m_fAlphaTestValue));
    if (m_Geometry.m_sLeaves1.m_bIsActive)
        avLeafLODs[1] = SLODData(true, m_Geometry.m_sLeaves1.m_nDiscreteLodLevel, dword(m_Geometry.m_sLeaves1.m_fAlphaTestValue));
}


/*====================
  CTreeModel::GetBillboardData
  ====================*/
void    CTreeModel::GetBillboardData(STreeBillboardData avBillboards[]) const
{
    // Get the discrete leaf data
    m_pSpeedTree->GetGeometry(m_Geometry, SpeedTree_BillboardGeometry);

    if (m_Geometry.m_sBillboard0.m_bIsActive)
        avBillboards[0] = STreeBillboardData(true, dword(m_Geometry.m_sBillboard0.m_fAlphaTestValue), m_Geometry.m_sBillboard0.m_pCoords, m_Geometry.m_sBillboard0.m_pTexCoords);
    if (m_Geometry.m_sBillboard1.m_bIsActive)
        avBillboards[1] = STreeBillboardData(true, dword(m_Geometry.m_sBillboard1.m_fAlphaTestValue), m_Geometry.m_sBillboard1.m_pCoords, m_Geometry.m_sBillboard1.m_pTexCoords);
}


/*====================
  CTreeModel::GetSeed
  ====================*/
uint    CTreeModel::GetSeed()
{
    return m_uiSeed;
}


/*====================
  CTreeModel::SetSeed
  ====================*/
void    CTreeModel::SetSeed(uint uiSeed)
{
    if (uiSeed == m_uiSeed)
        return;

    m_uiSeed = uiSeed;
    FreeGeometry();
    CSpeedTreeRT *pClone(m_pSpeedTree->Clone());
    ::K2_DELETE(m_pSpeedTree);
    m_pSpeedTree = pClone;
    m_pSpeedTree->Compute(NULL, uiSeed);
    LoadBranchGeometry();
    LoadFrondGeometry();
    LoadLeafGeometry();
}


/*====================
  CTreeModel::SetWindStrength
  ====================*/
void    CTreeModel::SetWindStrength(float fStrength)
{
    m_fWindFreqOffset = m_pSpeedTree->SetWindStrength(fStrength, m_fWindPrevStrength, m_fWindFreqOffset);
    m_fWindPrevStrength = fStrength;
}


/*====================
  CTreeModel::GetLeafBillboardTable
  ====================*/
const float*    CTreeModel::GetLeafBillboardTable(uint &uiSize) const
{
    // Update the base LOD (required for GPU wind)
    if (trees_useGPUWind)
        m_pSpeedTree->GetGeometry(m_Geometry, SpeedTree_LeafGeometry, -1, -1, 0);

    return m_pSpeedTree->GetLeafBillboardTable(uiSize);
}


/*====================
  CTreeModel::UpdateCamera
  ====================*/
void    CTreeModel::UpdateCamera(CVec3f v3Pos, CVec3f v3Dir)
{
    CSpeedTreeRT::SetCamera(v3Pos, v3Dir);
}


/*====================
  CTreeModel::SetTime
  ====================*/
void    CTreeModel::SetTime(float fSeconds)
{
    CSpeedTreeRT::SetTime(fSeconds);
}


/*====================
  CTreeModel::SetNumWindMatrices
  ====================*/
void    CTreeModel::SetNumWindMatrices(int iNumMatrices)
{
    CSpeedTreeRT::SetNumWindMatrices(iNumMatrices);
}


/*====================
  CTreeModel::GetNumMaterials
  ====================*/
uint    CTreeModel::GetNumMaterials() const
{
    return 3;
}
