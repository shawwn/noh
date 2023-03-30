// (C)2005 S2 Games
// c_worldcomponent.h
//
//=============================================================================
#ifndef __C_WORLDCOMPONENT_H__
#define __C_WORLDCOMPONENT_H__

//=============================================================================
// Declarations
//=============================================================================
class CArchive;
class IBuffer;
class CTerrainChunk;
class CWorld;
class CPath;

// This is the order in which these components are loaded
enum EWorldComponent
{
    WORLD_VERT_HEIGHT_MAP,
    WORLD_TILE_SPLIT_MAP,
    WORLD_TILE_NORMAL_MAP,
    WORLD_TILE_CLIFF_MAP,
    WORLD_TILE_VISBLOCKER_MAP,
    WORLD_VERT_NORMAL_MAP,
    WORLD_VERT_TANGENT_MAP,
    WORLD_VERT_COLOR_MAP,
    WORLD_VERT_BLOCKER_MAP,
    WORLD_MATERIAL_LIST,
    WORLD_TEXTURE_LIST,
    WORLD_TILE_MATERIAL_MAP,
    WORLD_TILE_FOLIAGE_MAP,
    WORLD_VERT_FOLIAGE_MAP,
    WORLD_TREE,
    WORLD_ENTITY_LIST,
    WORLD_LIGHT_LIST,
    WORLD_SOUND_LIST,
    WORLD_OCCLUDER_LIST,
    WORLD_TEXEL_ALPHA_MAP,
    WORLD_TEXEL_OCCLUSION_MAP,
    WORLD_TRIGGER_LIST,
    WORLD_NAVIGATION_MAP,
    WORLD_NAVIGATION_GRAPH,
    WORLD_OCCLUSION_MAP,
    WORLD_VERT_CAMERA_HEIGHT_MAP,
    WORLD_VERT_CLIFF_MAP,
    WORLD_CLIFFSET_LIST,
    WORLD_VARIATION_CLIFF_MAP,
    WORLD_RAMP_LIST,
    WORLD_TILE_RAMP_MAP,

    
    NUM_WORLD_COMPONENTS
};

const int NUM_TERRAIN_LAYERS(2);
const int NUM_FOLIAGE_LAYERS(2);
//=============================================================================

//=============================================================================
// IWorldComponent
// This is a base class for any data structure that is saved and loaded for
// a block of the world
//=============================================================================
class K2_API IWorldComponent
{
private:
    IWorldComponent();

protected:
    const CWorld*   m_pWorld;
    tstring         m_sName;
    bool            m_bChanged;     // Changed since the last save?
    EWorldComponent m_eComponent;

public:
    virtual ~IWorldComponent()                                                          {}
    IWorldComponent(EWorldComponent eComponent, tstring sName) : m_pWorld(NULL), m_sName(sName), m_bChanged(false), m_eComponent(eComponent)    {}

    bool            IsChanged() const       { return m_bChanged; }
    tstring         GetName() const         { return m_sName; }
    EWorldComponent GetComponentID() const  { return m_eComponent; }

    virtual bool    Load(CArchive &archive, const CWorld *pWorld) = 0;
    virtual bool    Save(CArchive &archive);
    virtual bool    Serialize(IBuffer *pBuffer)                     { return false; }
    virtual bool    Generate(const CWorld *pWorld) = 0;
    virtual void    Release() = 0;
    virtual void    Update(const CRecti &recArea)                       {}
    virtual void    Restore(CArchive &archive)                          {}

    virtual bool    GetRegion(const CRecti &recArea, void *pDest, int iLayer) const { return false; }
    virtual bool    SetRegion(const CRecti &recArea, void *pSource, int iLayer)     { return false; }
};
//=============================================================================

#endif //__C_WORLDCOMPONENT_H__
