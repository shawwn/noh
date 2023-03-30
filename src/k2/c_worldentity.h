// (C)2005 S2 Games
// c_worldentity.h
//
//=============================================================================
#ifndef __C_WORLDENTITY_H__
#define __C_WORLDENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_convexpolyhedron.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CWorldTreeNode;

typedef map<tstring, tstring> WEPropertyMap;

const uint WE_NOT_SOLID(BIT(0));
//=============================================================================

//=============================================================================
// CWorldEntity
//=============================================================================
class CWorldEntity
{
private:
    uint        m_uiIndex;

    uint        m_uiSurfFlags;

    PoolOffset      m_hNextBounds;
    PoolOffset      m_hNextSurface;
    PoolOffset      m_hNextModel;
    PoolOffset      m_hNextRender;

    CBBoxf      m_bbWorldBounds;
    CBBoxf      m_bbModelBounds;

    uint        m_uiGameIndex;
    uint        m_uiSeed;
    tstring     m_sName;
    tstring     m_sType;
    int         m_iTeam;

    CVec3f      m_v3Position;
    CVec3f      m_v3Angles;
    CAxis       m_aAxis;
    float       m_fScale;
    float       m_fScale2; // Scales bounds also, unlike m_fScale

    tstring     m_sInitialState;

    CBBoxf      m_bbBounds;
    CBBoxf      m_bbSurfaceBounds;
    CBBoxf      m_bbRenderBounds;

    float       m_fOcclusionRadius; 

    vector<CConvexPolyhedron>   m_vWorldSurfs;

    tstring     m_sModelPath;
    tstring     m_sSkinName;
    ResHandle   m_hModel;
    ResHandle   m_hSkin;
    ResHandle   m_hTreeDefinition;
    uint        m_uiFlags;

    WEPropertyMap   m_mapProperties;

    CWorldTreeNode  *m_pBoundsNode;
    CWorldTreeNode  *m_pSurfaceNode;
    CWorldTreeNode  *m_pModelNode;
    CWorldTreeNode  *m_pRenderNode;

public:
    ~CWorldEntity() {}
    CWorldEntity() :
    m_uiIndex(INVALID_INDEX),
    m_uiGameIndex(INVALID_INDEX),
    m_uiSeed(0),
    m_sName(_T("")),
    m_sType(_T("Prop_Scenery")),
    m_hTreeDefinition(NULL),
    m_iTeam(0),
    m_uiSurfFlags(0),

    m_v3Position(V_ZERO),
    m_v3Angles(V_ZERO),
    m_fScale(1.0f),
    m_fScale2(1.0f),
    m_sInitialState(TSNULL),
    m_bbBounds(V3_ZERO, V3_ZERO),
    m_sModelPath(TSNULL),
    m_hModel(INVALID_RESOURCE),
    m_sSkinName(TSNULL),
    m_hSkin(0),
    m_fOcclusionRadius(0.0f),
    m_uiFlags(0),
    
    m_pBoundsNode(NULL),
    m_pSurfaceNode(NULL),
    m_pModelNode(NULL),
    m_pRenderNode(NULL),
    m_hNextBounds(INVALID_POOL_OFFSET),
    m_hNextSurface(INVALID_POOL_OFFSET),
    m_hNextModel(INVALID_POOL_OFFSET),
    m_hNextRender(INVALID_POOL_OFFSET)
    {
    }

    void    Clone(const CWorldEntity &entity)
    {
        // Clone entity settings
        m_uiSeed = entity.m_uiSeed;
        m_sName = entity.m_sName;
        m_sType = entity.m_sType;
        m_iTeam = entity.m_iTeam;

        m_v3Position = entity.m_v3Position;
        m_v3Angles = entity.m_v3Angles;
        m_fScale = entity.m_fScale;

        m_bbBounds = entity.m_bbBounds;

        m_hModel = entity.m_hModel;
        m_sModelPath = entity.m_sModelPath;
        m_hSkin = entity.m_hSkin;
        m_sSkinName = entity.m_sSkinName;

        m_hTreeDefinition = entity.m_hTreeDefinition;

        m_uiFlags = entity.m_uiFlags;

        m_mapProperties = entity.m_mapProperties;
    }

    uint            GetIndex() const                                { return m_uiIndex; }
    void            SetIndex(uint uiIndex)                          { m_uiIndex = uiIndex; }

    uint            GetGameIndex() const                            { return m_uiGameIndex; }
    void            SetGameIndex(uint uiIndex)                      { m_uiGameIndex = uiIndex; }

    void            IncrementSeed()                                 { ++m_uiSeed; }
    void            DecrementSeed()                                 { --m_uiSeed; }
    uint            GetSeed() const                                 { return m_uiSeed; }
    void            SetSeed(uint uiSeed)                            { m_uiSeed = uiSeed; }

    tstring         GetName() const                                 { return m_sName; }
    void            SetName(const tstring &sName)                   { m_sName = sName; }

    tstring         GetType() const                                 { return m_sType; }
    void            SetType(const tstring &sType)                   { m_sType = sType; }

    int             GetTeam() const                                 { return m_iTeam; }
    void            SetTeam(int iTeam)                              { m_iTeam = iTeam; }

    const CVec3f&   GetPosition() const                             { return m_v3Position; }
    void            SetPosition(const CVec3f &v3Position)           { m_v3Position = v3Position; }
    void            SetPosition(float fX, float fY, float fZ)       { m_v3Position.Set(fX, fY, fZ); }

    const CVec3f&   GetAngles() const                               { return m_v3Angles; }
    void            SetAngles(const CVec3f &v3Angles)               { m_v3Angles = v3Angles; }
    void            AdjustAngle(EEulerComponent e, float fAngle)    { m_v3Angles[e] += fAngle; }

    const CAxis&    GetAxis() const                                 { return m_aAxis; }
    void            SetAxis(const CAxis &aAxis)                     { m_aAxis = aAxis; }

    float           GetScale() const                                { return m_fScale; }
    void            SetScale(float fScale)                          { m_fScale = fScale; }
    void            AdjustScale(float fScale)                       { m_fScale *= fScale; }

    tstring         GetInitialState() const                         { return m_sInitialState; }

    float           GetScale2() const                               { return m_fScale2; }
    void            SetScale2(float fScale)                         { m_fScale2 = fScale; }

    const CBBoxf&   GetBounds() const                               { return m_bbBounds; }
    void            SetBounds(const CBBoxf &bbBounds)               { m_bbBounds = bbBounds; }

    ResHandle       GetModelHandle() const                          { return m_hModel; }
    void            SetModelHandle(ResHandle hModel)                { m_hModel = hModel; }

    const tstring&  GetModelPath() const                            { return m_sModelPath; }
    void            SetModelPath(const tstring &sModelPath)         { m_sModelPath = sModelPath; }

    ResHandle       GetSkin() const                                 { return m_hSkin; }
    void            SetSkin(ResHandle hSkin)                        { m_hSkin = hSkin; }

    const tstring&  GetSkinName() const                             { return m_sSkinName; }
    void            SetSkinName(const tstring &sSkinName)           { m_sSkinName = sSkinName; }

    ResHandle       GetTreeDefinition() const                       { return m_hTreeDefinition; }
    void            SetTreeDefinition(const ResHandle hTreeDef)     { m_hTreeDefinition = hTreeDef; }

    float           GetOcclusionRadius() const                      { return m_fOcclusionRadius; }
    void            SetOcclusionRadius(float fOcclusionRadius)      { m_fOcclusionRadius = fOcclusionRadius; }

    bool            HasProperty(const tstring &sName) const                             { return m_mapProperties.find(sName) != m_mapProperties.end(); }
    void            SetProperty(const tstring &sName, const tstring &sValue)            { m_mapProperties[sName] = sValue; }
    int             GetPropertyInt(const tstring &sName, int iDefault = 0) const        { if (HasProperty(sName)) return AtoI(GetProperty(sName)); return iDefault; }
    float           GetPropertyFloat(const tstring &sName, float fDefault = 0.0f) const { if (HasProperty(sName)) return AtoF(GetProperty(sName)); return fDefault; }
    bool            GetPropertyBool(const tstring &sName, bool bDefault = false) const  { if (HasProperty(sName)) return AtoB(GetProperty(sName)); return bDefault; }
    tstring         GetProperty(const tstring &sName, const tstring &sDefault = TSNULL) const
    {
        WEPropertyMap::const_iterator findit(m_mapProperties.find(sName));

        if (findit != m_mapProperties.end())
            return findit->second;
        else
            return sDefault;
    }

    const WEPropertyMap&    GetPropertyMap() const  { return m_mapProperties; }

    void            SetWorldBounds(const CBBoxf &bb)    { m_bbWorldBounds = bb; }
    const CBBoxf&   GetWorldBounds() const              { return m_bbWorldBounds; }

    void            SetSurfaceBounds(const CBBoxf &bb)  { m_bbSurfaceBounds = bb; }
    const CBBoxf&   GetSurfaceBounds() const            { return m_bbSurfaceBounds; }

    void            SetModelBounds(const CBBoxf &bb)    { m_bbModelBounds = bb; }
    const CBBoxf&   GetModelBounds() const              { return m_bbModelBounds; }

    void            SetRenderBounds(const CBBoxf &bb)   { m_bbRenderBounds = bb; }
    const CBBoxf&   GetRenderBounds() const             { return m_bbRenderBounds; }

    vector<CConvexPolyhedron>&  GetWorldSurfsRef()      { return m_vWorldSurfs; }

    void SetSurfFlags(uint uiSurfFlags) { m_uiSurfFlags = uiSurfFlags; } // Careful: Any flags set on the entity apply to all tests, contrary to prior behavior
    uint GetSurfFlags() { return m_uiSurfFlags; }

    void SetNextBounds(PoolOffset hOffset) { m_hNextBounds = hOffset; }
    PoolOffset GetOffsetBounds() { return m_hNextBounds; }
    CWorldEntity *GetNextBounds() { if (m_hNextBounds == INVALID_POOL_OFFSET) return NULL; else return this + m_hNextBounds; }

    void SetNextSurface(PoolOffset hOffset) { m_hNextSurface = hOffset; }
    PoolOffset GetOffsetSurface() { return m_hNextSurface; }
    CWorldEntity *GetNextSurface() { if (m_hNextSurface == INVALID_POOL_OFFSET) return NULL; return this + m_hNextSurface; }

    void SetNextModel(PoolOffset hOffset) { m_hNextModel = hOffset; }
    PoolOffset GetOffsetModel() { return m_hNextModel; }
    CWorldEntity *GetNextModel() { if (m_hNextModel == INVALID_POOL_OFFSET) return NULL; return this + m_hNextModel; }

    void SetNextRender(PoolOffset hOffset) { m_hNextRender = hOffset; }
    PoolOffset GetOffsetRender() { return m_hNextRender; }
    CWorldEntity *GetNextRender() { if (m_hNextRender == INVALID_POOL_OFFSET) return NULL; return this + m_hNextRender; }

    void SetBoundsNode(CWorldTreeNode *pNode) { m_pBoundsNode = pNode; }
    CWorldTreeNode *GetBoundsNode() { return m_pBoundsNode; }

    void SetSurfaceNode(CWorldTreeNode *pNode) { m_pSurfaceNode = pNode; }
    CWorldTreeNode *GetSurfaceNode() { return m_pSurfaceNode; }

    void SetModelNode(CWorldTreeNode *pNode) { m_pModelNode = pNode; }
    CWorldTreeNode *GetModelNode() { return m_pModelNode; }

    void SetRenderNode(CWorldTreeNode *pNode) { m_pRenderNode = pNode; }
    CWorldTreeNode *GetRenderNode() { return m_pRenderNode; }

    bool HasFlags(uint uiFlags) const { return (m_uiFlags & uiFlags) != 0; }
    bool HasAllFlags(uint uiFlags) const { return (m_uiFlags & uiFlags) == uiFlags; }
    void AddFlags(uint uiFlags) { m_uiFlags |= uiFlags; }
    void RemoveFlags(uint uiFlags) { m_uiFlags &= ~uiFlags; }
    void ClearFlags() { m_uiFlags = 0; }
};
//=============================================================================

#endif //__C_WORLDENTITY_H__
