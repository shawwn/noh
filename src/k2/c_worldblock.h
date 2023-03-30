// (C)2005 S2 Games
// c_worldblock.h
//
//=============================================================================
#ifndef __C_WORLDBLOCK_H__
#define __C_WORLDBLOCK_H__

//=============================================================================
// Headers
//=============================================================================
#include "shared_api.h"

#include "c_occluder.h"
#include "c_tilenormalmap.h"
#include "c_worldtree.h"
#include "c_world.h"
#include "c_plane.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IBlockComponent;
class CHeightmap;
class CColorMap;
class CNormalMap;
class CAxis;
class COccluder;
class CVec3;
class CTileNormalMap;
class CConvexPolyhedron;
class CEntList;

struct STraceInfo;
struct linkedSurface_s;
struct objectGrid_s;
struct SSceneFaceVert;
struct pointInfo_s;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, IBlockComponent*>  ComponentMap;
//=============================================================================

//=============================================================================
// CWorldBlock
//
// Manages all the data for a portion of the world
// These are swapped in and out of memory as they are needed by the game
//=============================================================================
class CWorldBlock
{
    friend class CWorld;

private:
    bool                m_bActive;

    tstring             m_sName;
    tstring             m_sFilename;
    int                 m_iSize, m_iGridSize;
    float               m_fScale;
    float               m_fWorldSize;

public:
    CWorldBlock();
    ~CWorldBlock();

    void                Load(const tstring &sName, int iLocalBlock, const CWorld &world);
    void                Generate(const tstring &sName, int iLocalBlock, const CWorld &world);
    void                PostProcess();
    void                Free();
    void                New();
    SHARED_API bool     Save();
    
    IBlockComponent*    GetComponent(const tstring &sName);

    const tstring&      GetName()           { return m_sName; }
    const tstring&      GetFilename()       { return m_sFilename; }


    CTileNormalMap&     GetTileNormalMap()  { return m_TileNormalMap; }
    CWorldTree&         GetWorldTree()      { return m_WorldTree; }
    CHeightmap*         GetHeightmap()      { return m_pHeightmap; }
    CColorMap*          GetColormap()       { return m_pColormap; }
    CNormalMap*         GetNormalmap()      { return m_pNormalmap; }
    CEntList*           GetEntList()        { return m_pEntList; }

    int                 GetBlockSize()      { return m_iSize; }
    float               GetWorldSize()      { return m_fWorldSize; }

    float               GetGridHeight(int ix, int iy);
    const CPlane&       GetTileNormal(int ix, int iy, EGridTriangles tri) const { return m_TileNormalMap.Get(ix, iy, tri); }

    bool                TraceLine(STraceInfo &result, const CVec3f &start, const CVec3f &end, int iIgnoreSurface, CEntity *pIgnoreEntity);

    void                FitPolyToTerrain(SSceneFaceVert *pVerts, int iNumVerts, unsigned int shader, int iFlags, 
                                            void (*clipCallback)(int iNumVerts, SSceneFaceVert *pVerts, unsigned int shader, int iFlags))
    {
        return m_WorldTree.FitPolyToTerrain(pVerts, iNumVerts, shader, iFlags, clipCallback);
    }

    SHARED_API void     LinkEntity(CEntity *pEntity)        { m_WorldTree.LinkEntity(pEntity); }
    SHARED_API void     UnlinkEntity(CEntity *pEntity)      { m_WorldTree.UnlinkEntity(pEntity); }
    SHARED_API bool     IsLinked(CEntity *pEntity)          { return m_WorldTree.IsLinked(pEntity); }

    SHARED_API void     SampleGround(float fX, float fY, SPointInfo &pResult) const;
    SHARED_API float    SampleGround(float fX, float fY, bool bIgnoreStages, CEntity *pIgnoreEntity) const;

    size_t              GetNumOccluders()                   { return static_cast<int>(m_vOccluders.size()); }
    COccluder&          GetOccluder(int index)              { return m_vOccluders[index]; }
    void                ClearOccluders()                    { m_vOccluders.clear(); }
    void                AddOccluder(const COccluder &occ)   { m_vOccluders.push_back(occ); }
    void                RemoveOccluder(int index)           { m_vOccluders.erase(m_vOccluders.begin() + (index - 1)); }
};


//=============================================================================
#endif
