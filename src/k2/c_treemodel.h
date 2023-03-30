// (C)2005 S2 Games
// c_treemodel.h
//
//=============================================================================
#ifndef __C_TREEMODEL_H__
#define __C_TREEMODEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_model.h"
#include "SpeedTreeRT.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SBranchVert
{
    CVec3f  v3Pos;
    CVec3f  v3Nml;
    CVec2f  v2Tex0;
    CVec3f  v3Tan;
};

struct SFrondVert
{
    CVec3f  v3Pos;
    CVec3f  v3Nml;
    CVec2f  v2Tex0;
};

struct SLeafVert
{
    CVec3f  v3Pos;
    CVec3f  v3Nml;
    CVec2f  v2Tex0;
    CVec2f  v2Wind;     // matrix index, strength
    CVec3f  v3Leaf;     // placement, scale, rock index
};

struct STreeGeometry
{
    unsigned short  usNumVerts;
    float           *pfVerts;

    unsigned short  usNumLists;
    unsigned short  *pusNumIndices;
    word            **ppwIndices;

    float           fAlphaTest;
};

struct SLODData
{
    SLODData() {}
    SLODData(bool bActive, int iLOD, dword dwAlphaTest) :
    m_bActive(bActive),
    m_iLOD(iLOD),
    m_dwAlphaTest(dwAlphaTest)
    {
    }

    bool    m_bActive;
    int     m_iLOD;
    dword   m_dwAlphaTest;
};

struct STreeBillboardData
{
    STreeBillboardData() {}
    STreeBillboardData(bool bActive, dword dwAlphaTest, const float *pCoords, const float *pTexCoords) :
    m_bActive(bActive),
    m_dwAlphaTest(dwAlphaTest),
    m_pCoords(pCoords),
    m_pTexCoords(pTexCoords)
    {
    }

    bool    m_bActive;
    dword   m_dwAlphaTest;
    const float *m_pCoords;
    const float *m_pTexCoords;
};

typedef map<unsigned short, STreeGeometry*> TreeLODMap;
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSpeedTreeRT;
//=============================================================================

//=============================================================================
// CTreeModel
//=============================================================================
class CTreeModel : public IModel
{
private:
    // SpeedTree interface
    CSpeedTreeRT*   m_pSpeedTree;

    uint            m_uiSeed;

    // Index received from vid module
    uint            m_uiVidDefIndex;

    // Materials
    tstring         m_sBranchMaterial;
    tsvector            m_vsFrondMaterials;
    tsvector            m_vsLeafMaterials;
    tsvector            m_vsBillboardMaterials;

    // Geometry
    TreeLODMap      m_mapBranchGeometry;
    TreeLODMap      m_mapFrondGeometry;
    TreeLODMap      m_mapLeafGeometry;

    void            LoadBranchGeometry();
    void            LoadFrondGeometry();
    void            LoadLeafGeometry();
    void            FreeGeometry();

    // Wind
    float           m_fWindPrevStrength;
    float           m_fWindFreqOffset;

    vector<CBBoxf>  m_bbCollisionBounds;
        
    mutable CSpeedTreeRT::SGeometry m_Geometry;

public:
    K2_API ~CTreeModel();
    K2_API CTreeModel();

    bool                Load(const tstring &sFileName, uint uiIgnoreFlags);
    void                ProcessProperties(const class CXMLNode &node)   {}
    void                PostLoad();

    void    SetVidDefIndex(uint ui)     { m_uiVidDefIndex = ui; }
    uint    GetVidDefIndex() const      { return m_uiVidDefIndex; }

    // Materials
    void    SetBranchMaterial(const tstring &sFileName)     { m_sBranchMaterial = sFileName; }
    void    AddFrondMaterial(const tstring &sFileName);
    void    AddLeafMaterial(const tstring &sFileName);
    void    AddBillboardMaterial(const tstring &sFileName);

    const tstring&      GetBranchMaterial() const       { return m_sBranchMaterial; }
    const tsvector&     GetFrondMaterials() const       { return m_vsFrondMaterials; }
    const tsvector&     GetLeafMaterials() const        { return m_vsLeafMaterials; }
    const tsvector&     GetBillboardMaterials() const   { return m_vsBillboardMaterials; }

    // Geometry
    const TreeLODMap&   GetBranchLODMap()       { return m_mapBranchGeometry; }
    const TreeLODMap&   GetFrondLODMap()        { return m_mapFrondGeometry; }
    const TreeLODMap&   GetLeafLODMap()         { return m_mapLeafGeometry; }

    uint            GetSeed();
    virtual void    SetSeed(uint uiSeed);

    K2_API void SetWindStrength(float fStrength);

    K2_API void             ComputeLODLevel() const;
    K2_API SLODData         GetDiscreetBranchLOD() const;
    K2_API SLODData         GetDiscreetFrondLOD() const;
    K2_API void             GetLeafLODData(SLODData avLeafLODs[]) const;
    K2_API void             GetBillboardData(STreeBillboardData avBillboards[]) const;

    K2_API const float*     GetLeafBillboardTable(uint &uiSize) const;

    // SpeedTree interface
    K2_API static void  UpdateCamera(CVec3f v3Pos, CVec3f v3Dir);
    K2_API static void  SetTime(float fSeconds);
    K2_API static void  SetNumWindMatrices(int iNumMatrices);

    K2_API uint         GetNumMaterials() const;

    const vector<CBBoxf>&   GetCollisionBounds() const { return m_bbCollisionBounds; }
};
//=============================================================================

#endif //__C_TREEMODEL_H__
