// (C)2005 S2 Games
// c_scenemanager.h
//
//=============================================================================
#ifndef __C_SCENEMANAGER_H__
#define __C_SCENEMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_occluder.h"
#include "c_frustum.h"
#include "c_sceneentity.h"
#include "c_scenelight.h"
#include "c_camera.h"
#include "c_pool.h"
#include "c_convexpolyhedron.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSceneLight;
class CParticleSystem;
class CSceneEntityModifier;
class COrthoFrustum;

const uint  POLY_LIGHTING           BIT(0);
const uint  POLY_DOUBLESIDED        BIT(1);
const uint  POLY_NO_DEPTH_TEST      BIT(2);
const uint  POLY_NO_DEPTH_WRITE     BIT(3);
const uint  POLY_WIREFRAME          BIT(4);
const uint  POLY_POINT              BIT(5);
const uint  POLY_LINESTRIP          BIT(6);
const uint  POLY_LINELIST           BIT(7);
const uint  POLY_TRILIST            BIT(8);
const uint  POLY_POINTLIST          BIT(9);

K2_API EXTERN_CVAR_FLOAT(scene_farClip);
K2_API EXTERN_CVAR_FLOAT(scene_worldFarClip);
K2_API EXTERN_CVAR_FLOAT(scene_brightMin);
K2_API EXTERN_CVAR_FLOAT(scene_brightMax);
K2_API EXTERN_CVAR_FLOAT(scene_brightScale);

extern K2_API class CSceneManager *g_pSceneManager;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SOccluderVolume
{
    size_t      zNumPlanes;
    CPlane      planes[MAX_OCCLUDER_POINTS + 1];
    float       influence;
    COccluder   *pOccluder;
};

struct SSceneEntityEntry
{
    static CPool<SSceneEntityEntry>     s_Pool;
    void*   operator new(size_t z, const char *szContext = nullptr, const char *szType = nullptr, const char *szFile = nullptr, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }

    SSceneEntityEntry();
    SSceneEntityEntry(const CSceneEntity &cEnt) : cEntity(cEnt), bCull(false), bCullShadow(false) {}

    bool            bCull;          // If true, we do not draw this
    bool            bCullShadow;    // If true, we do not draw this into the shadowmap
    
    CSceneEntity    cEntity;
};

struct SSceneParticleSystemEntry
{
    static CPool<SSceneParticleSystemEntry>     s_Pool;
    void*   operator new(size_t z, const char *szContext = nullptr, const char *szType = nullptr, const char *szFile = nullptr, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }

    SSceneParticleSystemEntry();
    SSceneParticleSystemEntry(const CParticleSystem *pSystem) : pParticleSystem(pSystem), bCull(false), bCullShadow(false) {}

    bool            bCull;          // If true, we do not draw this
    bool            bCullShadow;    // If true, we do not draw this into the shadowmap

    const CParticleSystem   *pParticleSystem;
};

struct SSceneLightEntry
{
    static CPool<SSceneLightEntry>      s_Pool;
    void*   operator new(size_t z, const char *szContext = nullptr, const char *szType = nullptr, const char *szFile = nullptr, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }

    SSceneLightEntry();
    SSceneLightEntry(const CSceneLight &cSceneLight) : cLight(cSceneLight), bCull(false), bCullShadow(false) {}

    bool            bCull;          // If true, we do not draw this
    bool            bCullShadow;    // If true, we do not draw this into the shadowmap

    CSceneLight     cLight;
};

struct SSceneModifierEntry
{
    static CPool<SSceneModifierEntry>   s_Pool;
    void*   operator new(size_t z, const char *szContext = nullptr, const char *szType = nullptr, const char *szFile = nullptr, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }

    SSceneModifierEntry();
    SSceneModifierEntry(const CSceneEntityModifier *pSystem) : pModifier(pSystem), pNext(nullptr) {}

    const CSceneEntityModifier  *pModifier;
    SSceneModifierEntry         *pNext;
};

struct SSceneFaceVert
{
    CVec3f      vtx;
    CVec2f      tex;
    CVec4b      col;
};

struct SSceneFaceEntry
{
    SSceneFaceVert          *verts;
    uint                    zNumVerts;
    ResHandle               hMaterial;
    bool                    cull;
    int                     flags;  //see POLY_* defines in savage_types.h
};

typedef list<COccluder>         OccluderList;
typedef list<SOccluderVolume>   OccluderVolumeList;

typedef vector<SSceneEntityEntry *>         SceneEntityList;
typedef vector<SSceneParticleSystemEntry *> SceneParticleSystemList;
typedef vector<SSceneLightEntry *>          SceneLightList;
typedef map<uint, SSceneModifierEntry *>    SceneEntityModifierMap;

typedef list<SSceneFaceEntry>           SceneFaceList;

#ifdef K2_EXPORTS
#define SceneManager (*CSceneManager::GetInstance())
#else
#define SceneManager (*g_pSceneManager)
#endif
//=============================================================================

//=============================================================================
// CSceneManager
//=============================================================================
class CSceneManager
{
    SINGLETON_DEF(CSceneManager)

private:
    float           m_fTime;

    CCamera         m_Camera;

    // Frustum
    CFrustum        m_Frustum;
    CPlane          m_FrustumPlanes[6];

    CConvexPolyhedron   m_cSceneHull;

    // Stats
    int             m_iNumRenderCalls;
    int             m_iNumOutsideFrustum;
    int             m_iNumOccluded;
    int             m_iNumEntities;
    int             m_iNumCulledObjects;
    int             m_iNumRenderedObjects;
    uint            m_uiNumTriangles;
    float           m_fMTrisPerSecond;

    uint            m_uiLastSceneTime;

    // Render Lists
    SceneEntityList     m_lEntities;
    SceneEntityList     m_lSkyEntities;
    SceneParticleSystemList m_lParticleSystems;
    SceneLightList      m_lLights;
    SceneFaceList       m_lFaces;

    // Occluders
    OccluderList        m_lOccluders;
    OccluderVolumeList  m_lOccluderVolumes;

    // Modifiers
    SceneEntityModifierMap  m_mapModifiers;

    // Environment
    CVec3f              m_v3SunPos;
    float               m_fSkyAngle;
    float               m_fSkyTopAngle;
    float               m_fAltSkyAngle;
    float               m_fAltSkyTopAngle;

    ResHandle           m_hSkyModel;
    ResHandle           m_hSkyTopModel;
    ResHandle           m_hSunMaterial;

    bool            m_bShadowFalloff;

    // Internal functions
    void    DrawOccluders();
    void    BuildVolume(const CVec3f &v3Point, const COccluder &occluder, CPlane planes[]);
    void    BuildOccluderList(const CCamera &camera);

    void    CullEntities();
    void    CullEntitiesShadow();
    void    CullLights();

    void    DrawMoon(const CCamera &cam);
    void    DrawLightning(const CCamera &cam);

public:
    ~CSceneManager();

    float               GetShaderTime()                 { return m_fTime; }
    CPlane*             GetFrustumPlanes()              { return m_FrustumPlanes; }

    OccluderList&       GetOccluderList()               { return m_lOccluders; }
    SceneEntityList&    GetEntityList()                 { return m_lEntities; }
    SceneEntityList&    GetSkyEntityList()              { return m_lSkyEntities; }
    SceneFaceList&      GetFaceList()                   { return m_lFaces; }
    SceneLightList&     GetLightList()                  { return m_lLights; }
    SceneParticleSystemList&    GetParticleSystemList()         { return m_lParticleSystems; }

    K2_API SSceneModifierEntry* GetModifiers(uint uiEntityIndex);

    const CVec3f&       GetSunPos()                     { UpdateSunPosition(); return m_v3SunPos; }
    const CVec3f&       GetCameraPos()                  { return m_Camera.GetOrigin(); }
    const CVec3f&       GetCameraDir()                  { return m_Camera.GetViewAxis().Forward(); }

    K2_API CVec3f   GetTerrainSunColor();
    K2_API CVec3f   GetTerrainAmbientColor();
    K2_API CVec3f   GetEntitySunColor();
    K2_API CVec3f   GetEntityAmbientColor();

    K2_API float    GetFoliageDrawDistance();
    K2_API float    GetEntityDrawDistance();

    K2_API CVec3f   GetSceneBgColor();

    float           GetSceneDrawDistance()              { return m_Camera.GetZFar(); }

    K2_API bool     GetDrawSkybox();
    K2_API tstring  GetSkyboxMaterial();
    K2_API CVec4f   GetSkyColor();

    K2_API bool     AABBIsVisible(const CBBoxf &bbBox);
    K2_API bool     PointIsVisible(const CVec3f &v3pos);
    K2_API void     SetFrustum(const CCamera &camera);

    K2_API SSceneEntityEntry&   AddEntity(const CSceneEntity &cSceneEntity, bool bCull = true);
    K2_API void     AddSkyEntity(const CSceneEntity &cSceneEntity);
    K2_API void     AddPoly(uint zNumVerts, SSceneFaceVert *verts, ResHandle hMaterial, int flags);
    K2_API void     AddOccluder(const COccluder &occluder);
    K2_API void     AddLight(const CSceneLight &cLight);
    K2_API void     AddParticleSystem(const CParticleSystem *pParticleSystem, bool bCull);
    K2_API void     AddModifier(const CSceneEntityModifier *pModifier, uint uiSourceEntityIndex, uint uiTargetEntityIndex);

    K2_API void     CullEntitiesShadow(const CConvexPolyhedron &cScene);

    K2_API void     DrawSky(const CCamera &cam, float fFrameTime);
    K2_API  void    DrawSun(const CCamera &cam);
    K2_API void     UpdateSunPosition();

    K2_API void     PrepCamera(const CCamera &camera);
    K2_API void     SceneCull();
    K2_API void     Render();
    K2_API void     ClearBackground();
    K2_API void     Clear();
    K2_API void     DrawStats();

    K2_API bool     AABBInFrustum(const CBBoxf &bbBox, bool bIgnoreNearFar);
    K2_API bool     OBBInFrustum(const CBBoxf &bb, const CVec3f &v3Pos, const CAxis &axis, bool bIgnoreNearFar);
    K2_API bool     OBBIsVisible(const CBBoxf &bb, const CVec3f &v3Pos, const CAxis &axis);
    K2_API bool     BoxInSelectionRect(const CBBoxf &bb);
    K2_API bool     PointInFrustum(const CVec3f &v3Point, bool ignoreNearFar);

    CBBoxf&         GetFrustumBounds()  { return m_Frustum.GetBounds(); }

    K2_API void     SetTimeOfDay(float fTimeOfDay);
};
//=============================================================================
#endif //__C_SCENEMANAGER_H__
