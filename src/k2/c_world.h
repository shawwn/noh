// (C)2005 S2 Games
// c_world.h
//
// world management
//=============================================================================
#ifndef __C_WORLD_H__
#define __C_WORLD_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_heightmap.h"
#include "c_tilenormalmap.h"
#include "c_vertexnormalmap.h"
#include "c_vertextangentmap.h"
#include "c_vertexcolormap.h"
#include "c_materiallist.h"
#include "c_texturelist.h"
#include "c_tilematerialmap.h"
#include "c_tilefoliagemap.h"
#include "c_tilesplitmap.h"
#include "c_vertexfoliagemap.h"
#include "c_worldentitylist.h"
#include "c_worldlightlist.h"
#include "c_worldentity.h"
#include "c_worldsoundlist.h"
#include "c_worldtree.h"
#include "c_worldoccluderlist.h"
#include "c_texelalphamap.h"
#include "c_texelocclusionmap.h"
#include "c_worldtriggerlist.h"
#include "c_navigationmap.h"
#include "c_navigationgraph.h"
#include "c_vertexblockermap.h"
#include "c_tilecliffmap.h"
#include "c_tilevisionmap.h"
#include "c_recyclepool.h"
#include "c_path.h"
#include "c_occlusionmap.h"
#include "c_vertexcameraheightmap.h"
#include "c_vertexcliffmap.h"
#include "c_cliffsetlist.h"
#include "c_cliffvariationmap.h"
#include "c_ramplist.h"
#include "c_tilerampmap.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
struct STraceInfo;
struct objectGrid_s;
struct linkedSurface_s;
struct pointInfo_s;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const tstring WORLD_PATH(_T("/maps/"));

// Technically, this can be as high as the number of bits in an int, but 
// any value even near that would be impractical
const int MAX_WORLD_SIZE(9);
const int DEFAULT_PATH_COUNT(32);

enum ESpaceType
{
    TILE_SPACE,
    GRID_SPACE,
    TEXEL_SPACE,
    NAV_TILE_SPACE,
    VIS_TILE_SPACE,

    NUM_SPACE_TYPES
};

typedef deque<IWorldComponent*>         WorldComponentList;
typedef WorldComponentList::iterator    WorldComponentList_it;

#define ADD_COMPONENT(id, name, host) \
if (m_yHostType & (host) && m_apWorldComponents[id] == nullptr) \
{ \
    m_p##name = K2_NEW(ctx_World,  C##name)(id); \
    if (m_p##name == nullptr) \
    { \
        Console.Err << _T("Failure allocating world component: ") _T(#name) << newl; \
        Free(); \
        m_bValid = false; \
        return; \
    } \
\
    m_apWorldComponents[id] = m_p##name; \
}

#define CLEAR_COMPONENT(id, name, host) \
if (m_yHostType & (host) && m_apWorldComponents[id] == nullptr) \
{ \
    m_p##name = nullptr; \
}

const byte WORLDHOST_NULL   (0);
const byte WORLDHOST_SERVER (BIT(0));
const byte WORLDHOST_CLIENT (BIT(1));
const byte WORLDHOST_BOTH   (WORLDHOST_SERVER | WORLDHOST_CLIENT);

typedef pair<int, char*>                FileDataEntry;
typedef map<tstring, FileDataEntry>     FileDataMap;
typedef FileDataMap::iterator           FileDataMap_it;
typedef pair<tstring, FileDataEntry>    FileDataPair;
//=============================================================================

//=============================================================================
// CWorld
//
// Manages global data for the entire world
//=============================================================================
class CWorld
{
private:
    bool        m_bValid;
    bool        m_bActive;
    byte        m_yHostType;
    CArchive*   m_pWorldArchive;
    bool        m_bPreloaded;

    // Core data
    tstring                     m_sName;
    tstring                     m_sPath;
    tstring                     m_sFancyName;
    tstring                     m_sLoadingImagePath;
    int                         m_iMajorVersion;
    int                         m_iMinorVersion;
    int                         m_iMicroVersion;
    float                       m_fScale;
    int                         m_iCliffSize;
    int                         m_iSize;
    int                         m_iVisibilitySize;
    int                         m_iWidth[NUM_SPACE_TYPES];
    int                         m_iHeight[NUM_SPACE_TYPES];
    float                       m_fTextureScale;
    int                         m_iTexelDensity;
    tsvector                    m_svMusic;
    float                       m_fGroundLevel;
    CVec4f                      m_v4MinimapPadding; // Top, right, bottom, left
    CRectf                      m_recGameBounds;
    CRectf                      m_recCameraBounds;
    bool                        m_bDev;
    bool                        m_bMusicShuffle;

    int                         m_iMinPlayersPerTeam;
    int                         m_iMaxPlayers;
    int                         m_iNavigationDensity;
    tstring                     m_sModifiers;

    uint                        m_uiComponentsLoaded;

    // Components
    IWorldComponent*            m_apWorldComponents[NUM_WORLD_COMPONENTS];
    WorldComponentList          m_lComponentsToLoad;

    CHeightMap*                 m_pHeightMap;
    CTileNormalMap*             m_pTileNormalMap;
    CVertexNormalMap*           m_pVertexNormalMap;
    CVertexTangentMap*          m_pVertexTangentMap;
    CVertexCliffMap*            m_pVertexCliffMap;
    CVertexColorMap*            m_pVertexColorMap;
    CMaterialList*              m_pMaterialList;
    CTextureList*               m_pTextureList;
    CTileMaterialMap*           m_pTileMaterialMap;
    CTileFoliageMap*            m_pTileFoliageMap;
    CTileSplitMap*              m_pTileSplitMap;
    CVertexFoliageMap*          m_pVertexFoliageMap;
    CWorldTree*                 m_pWorldTree;
    CWorldEntityList*           m_pWorldEntityList;
    CWorldLightList*            m_pWorldLightList;
    CWorldSoundList*            m_pWorldSoundList;
    CWorldOccluderList*         m_pWorldOccluderList;
    CTexelAlphaMap*             m_pTexelAlphaMap;
    CTexelOcclusionMap*         m_pTexelOcclusionMap;
    CWorldTriggerList*          m_pWorldTriggerList;
    CNavigationMap*             m_pNavigationMap;
    CNavigationGraph*           m_pNavigationGraph;
    CVertexBlockerMap*          m_pVertexBlockerMap;
    CTileCliffMap*              m_pTileCliffMap;
    CTileVisBlockerMap*         m_pTileVisBlockerMap;
    COcclusionMap*              m_pOcclusionMap;
    CVertexCameraHeightMap*     m_pVertexCameraHeightMap;
    CCliffVariationMap*         m_pCliffVariationMap;
    CCliffList*                 m_pCliffList;
    CTileRampMap*               m_pTileRampMap;

    // Per-terrain support
    ResHandle                   m_hTerrainTypeStringTable;
    map<uint,tstring>           m_mapTerrainType;
    void                        UpdateTerrainTypeMap();

    // Pooled memory for world-paths
    CRecyclePool<CPath>         *m_pWorldPaths;

    // Internal functions
    void                ValidateSizes(ESpaceType eSpace);
    IWorldComponent*    GetWorldComponent(EWorldComponent eComponent) const { return m_apWorldComponents[eComponent]; }

    // Disallow copying of CWorld
    CWorld();
    CWorld(CWorld&);
    CWorld& operator=(CWorld&);

    void    AllocateComponents();
    void    ClearComponentPointers();

public:
    K2_API ~CWorld();
    K2_API CWorld(byte yHost);

    K2_API bool     StartLoad(const tstring &sName, bool bPreload = false);
    K2_API bool     LoadNextComponent();
    K2_API float    GetLoadProgress();
    K2_API void     Free();
    K2_API void     Save(const tstring &sName);
    K2_API bool     New(const tstring &sName, int iSize, float fScale, int TexelDensity = 8, float fCustomTextureScale = 0.0f);
    K2_API void     Reset();

    bool                IsValid() const                             { return m_bValid; }
    bool                IsLoaded() const                            { return m_bActive; }
    bool                IsLoading() const                           { return (!IsLoaded() && !m_lComponentsToLoad.empty()); }
    
    bool                IsPreloaded(const tstring &sWorldName = TSNULL) const
    {
        return m_bActive && m_bPreloaded && (sWorldName.empty() || m_sName == sWorldName);
    }

    void                ClearPreloaded()                            { m_bPreloaded = false; }

    void                SetHostType(byte yHostType)                 { m_yHostType = yHostType; }
    byte                GetHostType() const                         { return m_yHostType; }

    void                SetName(const tstring &sName)               { m_sName = sName; }
    void                SetFancyName(const tstring &sFancyName)     { m_sFancyName = sFancyName; }
    void                SetLoadingImagePath(const tstring &sPath)   { m_sLoadingImagePath = sPath; }
    void                SetVersion(int iMajor, int iMinor, int iMicro)  { m_iMajorVersion = iMajor; m_iMinorVersion = iMinor; m_iMicroVersion = iMicro; }
    void                SetScale(float fScale)                      { m_fScale = fScale; }
    void                SetWidth(int iWidth, ESpaceType eSpace)     { m_iWidth[eSpace] = iWidth; ValidateSizes(eSpace); }
    void                SetHeight(int iHeight, ESpaceType eSpace)   { m_iHeight[eSpace] = iHeight; ValidateSizes(eSpace); }
    void                SetCliffSize(int iCliffSize)                { m_iCliffSize = iCliffSize; }
    void                SetSize(int iSize);
    void                SetTextureScale(float fScale)               { m_fTextureScale = fScale; }
    void                SetTexelDensity(int iTexelDensity)          { m_iTexelDensity = iTexelDensity; }
    void                SetMusicList(const tsvector &svMusic)       { m_svMusic = svMusic; }
    void                SetGroundLevel(float fGroundLevel)          { m_fGroundLevel = fGroundLevel; }
    void                SetMinimapPadding(CVec4f &v4Padding)        { m_v4MinimapPadding = v4Padding; }
    void                SetGameBounds(const CRectf &recGameBounds)  { m_recGameBounds = recGameBounds; }
    void                SetCameraBounds(const CRectf &recCameraBounds)  { m_recCameraBounds = recCameraBounds; }
    void                SetDev(bool bDev)                           { m_bDev = bDev; }
    void                SetMusicShuffle(bool bShuffle)              { m_bMusicShuffle = bShuffle; }
    void                SetVisibilitySize(int iSize)                { m_iVisibilitySize = iSize; }
    void                SetModifiers(const tstring &sModifiers)     { m_sModifiers = sModifiers; }

    void                SetMinPlayersPerTeam(int iMin)              { m_iMinPlayersPerTeam = CLAMP(iMin, 0, 8); }
    void                SetMaxPlayers(int iMax)                     { m_iMaxPlayers = CLAMP(iMax, 2, 32); }
    int                 GetMinPlayersPerTeam() const                { return m_iMinPlayersPerTeam; }
    int                 GetMaxPlayers() const                       { return m_iMaxPlayers; }

    const tstring&      GetName() const                             { return m_sName; }
    const tstring&      GetFancyName() const                        { return m_sFancyName; }
    const tstring&      GetLoadingImagePath() const                 { return m_sLoadingImagePath; }
    tstring             GetVersionString() const                    { return XtoA(m_iMajorVersion) + _T(".") + XtoA(m_iMinorVersion) + _T(".") + XtoA(m_iMicroVersion); }
    const tstring&      GetPath() const                             { return m_sPath; }
    float               GetScale() const                            { return m_fScale; }
    float               GetTextureScale() const                     { return m_fTextureScale; }
    float               GetNavigationScale() const                  { return m_fScale / (1<<m_iNavigationDensity); }
    uint                GetNavigationSize() const                   { return m_iSize + m_iNavigationDensity; }
    int                 GetTexelDensity() const                     { return m_iTexelDensity; }
    int                 GetTexelScale() const                       { return INT_FLOOR(m_fScale / m_iTexelDensity); }
    tsvector            GetMusicList() const                        { return m_svMusic; }
    float               GetGroundLevel() const                      { return m_fGroundLevel; }
    int                 GetSize() const                             { return m_iSize; }
    float               GetWorldWidth() const                       { return m_iWidth[TILE_SPACE] * m_fScale; }
    float               GetWorldHeight() const                      { return m_iHeight[TILE_SPACE] * m_fScale; }
    float               ScaleGridCoord(int iGrid) const             { return iGrid * m_fScale; }
    int                 GetTileFromCoord(float fCoord) const        { return INT_FLOOR(fCoord / m_fScale); }
    int                 GetVertFromCoord(float fCoord) const        { return INT_ROUND(fCoord / m_fScale); }
    int                 GetTexelFromCoord(float fCoord) const       { return INT_FLOOR(fCoord / (m_fScale / m_iTexelDensity)); }
    int                 GetGridWidth() const                        { return m_iWidth[GRID_SPACE]; }
    int                 GetGridHeight() const                       { return m_iHeight[GRID_SPACE]; }
    int                 GetGridArea() const                         { return m_iWidth[GRID_SPACE] * m_iHeight[GRID_SPACE]; }
    int                 GetGridIndex(int iX, int iY) const          { return iX + m_iWidth[GRID_SPACE] * iY; }
    int                 GetTileWidth() const                        { return m_iWidth[TILE_SPACE]; }
    int                 GetTileHeight() const                       { return m_iHeight[TILE_SPACE]; }
    int                 GetTileArea() const                         { return m_iWidth[TILE_SPACE] * m_iHeight[TILE_SPACE]; }
    int                 GetTileIndex(int iX, int iY) const          { return iX + m_iWidth[TILE_SPACE] * iY; }
    int                 GetCliffGridArea() const                    { return ((m_iWidth[GRID_SPACE] / m_iCliffSize) + 1) * ((m_iHeight[GRID_SPACE] / m_iCliffSize) + 1); }
    int                 GetCliffTileArea() const                    { return (m_iWidth[TILE_SPACE] / m_iCliffSize) * (m_iHeight[TILE_SPACE] / m_iCliffSize); }
    int                 GetCliffGridWidth() const                   { return m_iWidth[TILE_SPACE] / m_iCliffSize + 1; }
    int                 GetCliffTileWidth() const                   { return m_iWidth[TILE_SPACE] / m_iCliffSize; }
    int                 GetCliffGridHeight() const                  { return m_iHeight[GRID_SPACE] / m_iCliffSize + 1; }
    int                 GetCliffTileHeight() const                  { return m_iHeight[TILE_SPACE] / m_iCliffSize; }
    int                 GetCliffSize() const                        { return m_iCliffSize; }
    int                 GetCliffTileIndex(int iX, int iY) const     { return iX + (GetCliffTileWidth() * iY); }
    int                 GetTexelWidth() const                       { return m_iWidth[TEXEL_SPACE]; }
    int                 GetTexelHeight() const                      { return m_iHeight[TEXEL_SPACE]; }
    int                 GetTexelArea() const                        { return m_iWidth[TEXEL_SPACE] * m_iHeight[TEXEL_SPACE]; }
    int                 GetTexelIndex(int iX, int iY) const         { return iX + m_iWidth[TEXEL_SPACE] * iY; }
    int                 GetNavigationWidth() const                  { return m_iWidth[NAV_TILE_SPACE]; }
    int                 GetNavigationHeight() const                 { return m_iHeight[NAV_TILE_SPACE]; }
    CBBoxf              GetBounds() const                           { return m_pWorldTree->GetBounds(); }
    CBBoxf              GetTerrainBounds() const                    { return m_pWorldTree->GetTerrainBounds(); }
    const CVec4f&       GetMinimapPadding() const                   { return m_v4MinimapPadding; }
    const CRectf&       GetGameBounds() const                       { return m_recGameBounds; }
    const CRectf&       GetCameraBounds() const                     { return m_recCameraBounds; }
    bool                GetDev() const                              { return m_bDev; }
    bool                GetMusicShuffle() const                     { return m_bMusicShuffle; }
    const tstring&      GetModifiers() const                        { return m_sModifiers; }
    int                 GetVisibilitySize() const                   { return m_iVisibilitySize; }
    inline bool         IsInBounds(float fX, float fY) const;
    inline bool         IsInBounds(int iX, int iY, ESpaceType eSpace) const;
    inline bool         IsInGameBounds(float fX, float fY) const;

    K2_API bool     ClipRect(CRecti &recResult, ESpaceType eSpace) const;
    K2_API bool     GetRegion(EWorldComponent eComponent, CRecti &recArea, void *pArray, int iLayer = 0) const;
    K2_API bool     SetRegion(EWorldComponent eComponent, CRecti &recArea, void *pArray, int iLayer = 0);
    K2_API bool     UpdateComponent(EWorldComponent eComponent, const CRecti &recArea) const;

    //
    // Component interfaces
    //

    //Cliff 
    uint                AddCliffDef(ResHandle hCliffDef) const                                          { return m_pCliffList->AddCliff(hCliffDef); }
    ResHandle           GetCliffDefHandle(int iID) const                                                { return m_pCliffList->GetCliffHandle(iID); }
    uint                GetCliffDefID(ResHandle hCliffDef) const                                        { return m_pCliffList->GetCliffID(hCliffDef); }
    const byte          GetRampTileByTile(int iX, int iY) const                                         { return m_pTileRampMap->GetRampByTile(iX, iY); }
    const byte          GetRampTile(int iX, int iY) const                                               { return m_pTileRampMap->GetRamp(iX, iY); }
    void                SetRampTileByTile(int iX, int iY, byte yRampType) const                         { return m_pTileRampMap->SetRampByTile(iX, iY, yRampType); }
    void                SetRampTile(int iX, int iY, byte yRampType) const                               { return m_pTileRampMap->SetRamp(iX, iY, yRampType); }

    // Vertices, tiles, and texels
    float               GetGridPoint(int iX, int iY) const                          { return m_pHeightMap->GetGridPoint(iX, iY); }
    ETileSplitType      GetTileSplit(int iX, int iY) const                          { return m_pTileSplitMap->GetTileSplit(iX, iY); }
    const CPlane&       GetTilePlane(int iX, int iY, EGridTriangles eTri) const     { return m_pTileNormalMap->GetTilePlane(iX, iY, eTri); }
    const CVec3f&       GetTileNormal(int iX, int iY, EGridTriangles eTri) const    { return m_pTileNormalMap->GetTileNormal(iX, iY, eTri); }
    const CVec3f&       GetGridNormal(int iX, int iY) const                         { return m_pVertexNormalMap->GetVertexNormal(iX, iY); }
    const CVec3f&       GetGridTangent(int iX, int iY) const                        { return m_pVertexTangentMap->GetVertexTangent(iX, iY); }
    const CVec4b&       GetGridColor(int iX, int iY) const                          { return m_pVertexColorMap->GetVertexColor(iX, iY); }
    byte                GetTexelAlpha(int iX, int iY) const                         { return m_pTexelAlphaMap ? m_pTexelAlphaMap->GetTexelAlpha(iX, iY) : 255; }
    bool                HasTexelAlphaMap() const                                    { return m_pTexelAlphaMap != nullptr; }
    byte                GetTexelOcclusion(int iX, int iY) const                     { return m_pTexelOcclusionMap->GetTexelOcclusion(iX, iY); }
    float*              GetHeightMap() const                                        { return m_pHeightMap->GetHeightMap(); }
    byte*               GetSplitMap() const                                         { return m_pTileSplitMap->GetSplitMap(); }
    byte*               GetBlockerMap() const                                       { return m_pVertexBlockerMap->GetBlockerMap(); }
    byte                GetBlockers(int iX, int iY) const                           { return m_pVertexBlockerMap->GetBlockers(iX, iY); }
    int                 GetVertCliffMapWidth() const                                { return m_pVertexCliffMap->GetVertCliffMapWidth(); }
    int                 GetVertCliffMapHeight() const                               { return m_pVertexCliffMap->GetVertCliffMapHeight(); }
    int                 GetVertCliffMapArea() const                                 { return m_pVertexCliffMap->GetVertCliffMapHeight() * m_pVertexCliffMap->GetVertCliffMapWidth(); }
    int*                GetVertCliffMap() const                                     { return m_pVertexCliffMap->GetVertexCliffMap(); }
    int                 GetVertCliff(int iX, int iY) const                          { return m_pVertexCliffMap->GetVertexCliff(iX, iY); }
    int                 GetTileCliff(int iX, int iY) const                          { return m_pCliffVariationMap->GetTileCliff(iX, iY); }
    uint*               GetTileCliffVariationMap() const                            { return m_pCliffVariationMap->GetCliffVariationMap(); }
    uint*               GetTileCliffDefinitionMap() const                           { return m_pCliffVariationMap->GetCliffDefinitionMap(); }
    byte*               GetCliffMap() const                                         { return m_pTileCliffMap->GetCliffMap(); } //This is the texture removal used for cliffs.
    byte                GetCliff(int iX, int iY) const                              { return m_pTileCliffMap->GetCliff(iX, iY); }
    byte*               GetVisBlockerMap() const                                    { return m_pTileVisBlockerMap->GetVisBlockerMap(); }
    byte                GetVisBlocker(int iX, int iY) const                         { return m_pTileVisBlockerMap->GetVisBlocker(iX, iY); }
    byte*               GetTileRampMap() const                                      { return m_pTileRampMap->GetTileRampMap(); }
    K2_API bool         IsTileVisible(int iTileX, int iTileY, int iLayer) const;

    // Materials/Textures
    uint                AddMaterial(ResHandle hMaterial) const                      { return m_pMaterialList->AddMaterial(hMaterial); }
    ResHandle           GetMaterialHandle(int iID) const                            { return m_pMaterialList->GetMaterialHandle(iID); }
    uint                GetMaterialID(ResHandle hMaterial) const                    { return m_pMaterialList->GetMaterialID(hMaterial); }
    uint                AddTexture(ResHandle hTexture) const                        { return m_pTextureList->AddTexture(hTexture); }
    ResHandle           GetTextureHandle(int iID) const                             { return m_pTextureList->GetTextureHandle(iID); }
    uint                GetTextureID(ResHandle hTexture) const                      { return m_pTextureList->GetTextureID(hTexture); }
    ResHandle           GetTileMaterial(int iX, int iY, int iLayer) const           { return GetMaterialHandle(m_pTileMaterialMap->GetTileMaterialID(iX, iY, iLayer)); }
    ResHandle           GetTileDiffuseTexture(int iX, int iY, int iLayer) const     { return GetTextureHandle(m_pTileMaterialMap->GetTileDiffuseTextureID(iX, iY, iLayer)); }
    ResHandle           GetTileNormalmapTexture(int iX, int iY, int iLayer) const   { return GetTextureHandle(m_pTileMaterialMap->GetTileNormalmapTextureID(iX, iY, iLayer)); }
    void                SetTextureIDUsed(uint uiID) const                           { return m_pTextureList->SetTextureIDUsed(uiID); }

    // Foliage
    ResHandle           GetFoliageMaterial(int iX, int iY, int iLayer) const        { return GetMaterialHandle(m_pTileFoliageMap->GetTileMaterialID(iX, iY, iLayer)); }
    ResHandle           GetFoliageTexture(int iX, int iY, int iLayer) const         { return GetTextureHandle(m_pTileFoliageMap->GetTileTextureID(iX, iY, iLayer)); }
    float               GetFoliageDensity(int iX, int iY, int iLayer) const         { return m_pVertexFoliageMap->GetFoliageDensity(iX, iY, iLayer); }
    const CVec3f&       GetFoliageSize(int iX, int iY, int iLayer) const            { return m_pVertexFoliageMap->GetFoliageSize(iX, iY, iLayer); }
    const CVec3f&       GetFoliageVariance(int iX, int iY, int iLayer) const        { return m_pVertexFoliageMap->GetFoliageVariance(iX, iY, iLayer); }
    const CVec3f&       GetFoliageColor(int iX, int iY, int iLayer) const           { return m_pVertexFoliageMap->GetFoliageColor(iX, iY, iLayer); }
    byte                GetFoliageCrossQuads(int iX, int iY, int iLayer) const      { return m_pTileFoliageMap->GetNumCrossQuads(iX, iY, iLayer); }
    byte                GetFoliageFlags(int iX, int iY, int iLayer) const           { return m_pTileFoliageMap->GetFlags(iX, iY, iLayer); }
    
    // Collision tree
    CWorldTree&         GetWorldTree() const                                        { return *m_pWorldTree; }
    void                ClearWorldTree()                                            { m_pWorldTree->Release(); m_pWorldTree->Generate(this); }

    // Entities
    CWorldEntityList*       GetWorldEntityList() const                                      { return m_pWorldEntityList; }
    K2_API void             SetWorldEntityList(CWorldEntityList *pEntityList);
    void                    RestoreWorldEntityList() const                                  { return m_pWorldEntityList->Restore(*m_pWorldArchive); }
    inline CWorldEntity*    GetEntity(uint uiIndex, bool bThrow = NO_THROW) const           { return m_pWorldEntityList->GetEntity(uiIndex, bThrow); }
    inline CWorldEntity*    GetEntityByHandle(PoolHandle hHandle) const                     { return m_pWorldEntityList->GetEntityByHandle(hHandle); }
    inline PoolHandle       GetHandleByEntity(CWorldEntity *pEntity) const                  { return m_pWorldEntityList->GetHandleByEntity(pEntity); }
    uint                    AllocateNewEntity(uint uiIndex = INVALID_INDEX) const           { return m_pWorldEntityList->AllocateNewEntity(uiIndex); }
    void                    FreeEntityByHandle(PoolHandle hHandle) const                    { m_pWorldEntityList->FreeEntity(hHandle); }
    void                    DeleteEntity(uint uiIndex) const                                { m_pWorldEntityList->DeleteEntity(uiIndex); }
    bool                    EntityExists(uint uiIndex) const                                { return m_pWorldEntityList->Exists(uiIndex); }
    WorldEntList&           GetEntityList() const                                           { return m_pWorldEntityList->GetEntityList(); }
    void                LinkEntity(uint uiIndex, uint uiLinkFlags, uint uiSurfFlags) const  { m_pWorldTree->LinkEntity(GetEntity(uiIndex), uiLinkFlags, uiSurfFlags); }
    void                UnlinkEntity(uint uiIndex) const                                    { m_pWorldTree->UnlinkEntity(GetEntity(uiIndex)); }
    void                GetEntitiesInRegion(uivector &vResult, const CBBoxf &bbRegion, uint uiSurfFlags)                { m_pWorldTree->GetEntitiesInRegion(vResult, bbRegion, uiSurfFlags); }
    void                GetEntitiesInRadius(uivector &vResult, const CSphere &radius, uint uiSurfFlags)                 { m_pWorldTree->GetEntitiesInRadius(vResult, radius, uiSurfFlags); }
    void                GetEntitiesInRadius(uivector &vResult, const CVec2f &v2Center, float fRadius, uint uiSurfFlags) { m_pWorldTree->GetEntitiesInRadius(vResult, v2Center, fRadius, uiSurfFlags); }
    void                GetEntitiesInSurface(uivector &vResult, const CConvexPolyhedron &cSurface, uint uiSurfFlags)    { m_pWorldTree->GetEntitiesInSurface(vResult, cSurface, uiSurfFlags); }
    void                GetEntityHandlesInRegion(WorldEntVector &vResult, const CBBoxf &bbRegion, uint uiSurfFlags)     { m_pWorldTree->GetEntityHandlesInRegion(vResult, bbRegion, uiSurfFlags); }
    
    // Lights
    CWorldLight*        GetLight(uint uiIndex, bool bThrow = NO_THROW)              { return m_pWorldLightList->GetLight(uiIndex, bThrow); }
    uint                AllocateNewLight() const                                    { return m_pWorldLightList->AllocateNewLight(); }
    void                DeleteLight(uint uiIndex) const                             { return m_pWorldLightList->DeleteLight(uiIndex); }
    WorldLightsMap&     GetLightsMap() const                                        { return m_pWorldLightList->GetLightsMap(); }

    // Sounds
    CWorldSound*        GetSound(uint uiIndex, bool bThrow = NO_THROW)              { return m_pWorldSoundList->GetSound(uiIndex, bThrow); }
    uint                AllocateNewSound() const                                    { return m_pWorldSoundList->AllocateNewSound(); }
    void                DeleteSound(uint uiIndex) const                             { return m_pWorldSoundList->DeleteSound(uiIndex); }
    WorldSoundsMap&     GetSoundsMap() const                                        { return m_pWorldSoundList->GetSoundsMap(); }
    
    // Terrain Type for per terrain sounds/effects
    K2_API const tstring&   GetTerrainType(float fX, float fY);
    K2_API CStringTable*    GetTerrainTypesTable();

    // Occluders
    COccluder*          GetOccluder(uint uiIndex, bool bThrow = NO_THROW)           { return m_pWorldOccluderList->GetOccluder(uiIndex, bThrow); }
    uint                AllocateNewOccluder() const                                 { return m_pWorldOccluderList->AllocateNewOccluder(); }
    void                DeleteOccluder(uint uiIndex) const                          { return m_pWorldOccluderList->DeleteOccluder(uiIndex); }
    OccluderMap&        GetOccluderMap() const                                      { return m_pWorldOccluderList->GetOccluderMap(); }

    K2_API bool     TraceLine(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, int iIgnoreSurface = 0, uint uiIgnoreEntity = INVALID_INDEX);
    K2_API bool     TraceBox(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, int iIgnoreSurface = 0, uint uiIgnoreEntity = INVALID_INDEX);
    K2_API float    GetTerrainHeight(float fX, float fY) const;
    K2_API float    SampleGround(float fX, float fY);
    K2_API CVec3f   GetTerrainNormal(float fX, float fY) const;
    float           CalcMaxHeight(const CRecti &recArea) const                      { return m_pHeightMap->CalcMaxHeight(recArea); }
    float           CalcMinHeight(const CRecti &recArea) const                      { return m_pHeightMap->CalcMinHeight(recArea); }
    bool            CalcBlocked(const CRecti &recArea) const                        { return m_pVertexBlockerMap->CalcBlocked(recArea); }

    // Triggers
    tsmapts&        GetScriptMap()                                                      { return m_pWorldTriggerList->GetScriptMap(); }
    tstring     GetScript(const tstring &sName)                                     { return m_pWorldTriggerList->GetScript(sName); }
    void        AddScript(const tstring &sName, const tstring &sScript)             { m_pWorldTriggerList->RegisterNewScript(sName, sScript); }
    void        DeleteScript(const tstring &sName)                                  { m_pWorldTriggerList->DeleteScript(sName); }
    bool        HasScript(const tstring &sName)                                     { return m_pWorldTriggerList->HasScript(sName); }
    bool        IsScriptReserved(const tstring &sName)                              { return m_pWorldTriggerList->IsScriptReserved(sName); }

    // Imported resources
    K2_API bool     ImportFile(const tstring &sFilePath, const tstring &sPathInArchive);
    K2_API bool     DeleteImportedFile(const tstring &sPathInArchive);
    K2_API void     GetImportedFiles(tsvector &vList);

    // Ambient Occlusion Map
    void        CalculateOcclusionMap(int iSamples)                                 { m_pTexelOcclusionMap->Calculate(iSamples); }

    // Pathing
    CNavigationMap&         GetNavigationMap() const                                { return *m_pNavigationMap; }
    CNavigationGraph&       GetNavigationGraph() const                              { return *m_pNavigationGraph; }
    const CTileNormalMap&   GetTileNormalMap() const                                { return *m_pTileNormalMap; }
    K2_API CPath*           AccessPath(PoolHandle hPath) const                      { return m_pWorldPaths->GetReferenceByHandle(hPath); }
    K2_API PoolHandle       NewPath() const                                         { return m_pWorldPaths->New(CPath()); }
    K2_API PoolHandle       ClonePath(PoolHandle hPath) const;
    K2_API PoolHandle       FindPath(const CVec2f &v2Src, float fEntityWidth, uint uiNavigationFlags, const CVec2f &v2Goal, float fGoalRange, vector<PoolHandle> *pBlockers = nullptr) const;
    K2_API void             FreePath(PoolHandle hPath) const                        { m_pWorldPaths->Free(hPath); }
    K2_API PoolHandle       BlockPath(uint uiFlags, const CVec2f &v2Position, float fWidth, float fHeight);
    K2_API void             BlockPath(vector<PoolHandle> &vecBlockers, uint uiFlags, const CConvexPolyhedron &cSurf, float fStepHeight);
    K2_API void             ClearPath(PoolHandle hBlockerID);

    K2_API void             ClearPath(uint uiFlags, const CConvexPolyhedron &cSurf);
    K2_API void             AnalyzeTerrain();
    K2_API void             UpdateNavigation();

    // Occlusion
    bool                    GetOcclusion(const CRecti &recArea, byte *pDst, float fHeight)          { return m_pOcclusionMap->GetRegion(recArea, pDst, fHeight); }
    void                    OccludeRegion(const CVec3f &v3Pos, float fRadius, float fHeight) const  { m_pOcclusionMap->OccludeRegion(v3Pos, fRadius, fHeight); }
    void                    AddOccludeRegion(const CVec3f &v3Pos, float fRadius) const              { m_pOcclusionMap->AddOccludeRegion(v3Pos, fRadius); }
    void                    RemoveOccludeRegion(const CVec3f &v3Pos, float fRadius) const           { m_pOcclusionMap->RemoveOccludeRegion(v3Pos, fRadius); }

    // Camera
    float               GetCameraHeight(int iX, int iY) const                       { return m_pVertexCameraHeightMap->GetGridPoint(iX, iY); }
    K2_API float        GetCameraHeight(float fX, float fY) const;

    void                    WriteConfigFile(CArchive &cArchive);
};


/*====================
  CWorld::IsInBounds
  ====================*/
inline
bool    CWorld::IsInBounds(float fX, float fY) const
{
    if (fX >= 0.0f && fX < m_iWidth[TILE_SPACE] * m_fScale &&
        fY >= 0.0f && fY < m_iHeight[TILE_SPACE] * m_fScale)
        return true;
    else
        return false;
}

inline
bool    CWorld::IsInBounds(int iX, int iY, ESpaceType eSpace) const
{
    if (iX < 0 || iX >= m_iWidth[eSpace] ||
        iY < 0 || iY >= m_iHeight[eSpace])
        return false;

    return true;
}


/*====================
  CWorld::IsInGameBounds
  ====================*/
inline
bool    CWorld::IsInGameBounds(float fX, float fY) const
{
    if (fX >= m_recGameBounds.left && fX < m_recGameBounds.right &&
        fY >= m_recGameBounds.top && fY < m_recGameBounds.bottom)
        return true;
    else
        return false;
}

//=============================================================================c

#endif // __C_WORLD_H__
