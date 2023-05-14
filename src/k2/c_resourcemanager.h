// (C)2005 S2 Games
// c_resourcemanager.h
//
//=============================================================================
#ifndef __C_RESOURCEMANAGER_H__
#define __C_RESOURCEMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_resourcemanager_constants.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IResourceWatcher;
class IFileChangeCallback;

extern K2_API class CResourceManager &g_ResourceManager;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<CConvexPolyhedron>           SurfVector;
//=============================================================================

//=============================================================================
// CResourceManager
//=============================================================================
class CResourceManager
{
    SINGLETON_DEF(CResourceManager)

private:
    typedef TStringSet                                  MapResourcesSet;
    typedef hash_set<IFileChangeCallback*>              FileChangeCallbackSet;
    typedef hash_map<tstring, FileChangeCallbackSet>    FileChangeCallbackMap;
    typedef hash_map<tstring, ResHandle>                PathHandleMap;

    ResourceLibVector           m_vResourceLibs;
    PathHandleMap               m_mapActivePaths;
    FileChangeCallbackMap       m_mapFileChangeCallbacks;
    MapResourcesSet             m_setMapSpecificResources;

    // Special resources
    ResHandle                   m_hWhiteTexture;
    ResHandle                   m_hBlackTexture;
    ResHandle                   m_hCheckerTexture;
    ResHandle                   m_hInvisTexture;
    ResHandle                   m_hFlatTexture;
    ResHandle                   m_hTeapotModel;

    // Null resources
    CMaterial*                  m_pNullMaterial;

    struct SResLoadProfile
    {
        uint            uiType;
        ULONGLONG       llTime;
        ULONGLONG       llMemory;

        SResLoadProfile() : uiType(RES_UNKNOWN), llTime(0), llMemory(0) {}
        SResLoadProfile(uint _uiType, ULONGLONG _llTime, ULONGLONG _llMemory) : uiType(_uiType), llTime(_llTime), llMemory(_llMemory) {}
    };

    vector<SResLoadProfile>     m_vResLoadProfile;

public:
    ~CResourceManager();

    void                        RegisterLibrary(uint uiType, IResourceLibrary *pLib);
    void                        UnregisterLibrary(uint uiType);
    K2_API IResourceLibrary*    GetLib(uint uiType);

    K2_API ResHandle            Register(const tstring &sPath, uint uiType, uint uiIgnoreFlags = 0);
    K2_API ResHandle            Register(IResource *pResource, uint uiType, uint uiIgnoreFlags = 0);
    
    K2_API void                 Unregister(ResHandle hResource, EUnregisterResource eUnregisterOp);
    K2_API bool                 Reload(ResHandle hResource);
    K2_API bool                 Reload(const tstring &sPath);
    K2_API void                 ReloadByFlag(int iFlag);
    K2_API ResHandle            LookUpPath(const tstring &sPath);
    K2_API ResHandle            LookUpName(const tstring &sName, EResourceType eType);
    K2_API ResHandle            LookUpPrecached(const tstring &sPath);
    K2_API IResource*           LookUpHandle(ResHandle hResource, uint uiType = RES_UNKNOWN);
    K2_API IResource*           Get(ResHandle hResource);
    K2_API IResource*           Get(ResHandle hResource, EResourceType eType);
    K2_API void                 RegisterFileChangeCallback(IFileChangeCallback *pCallback);
    K2_API void                 UnregisterFileChangeCallback(IFileChangeCallback *pCallback);

    K2_API void                 RemoveResourceWatcher(IResourceWatcher* pWatcher, ResHandle hUnregisterFrom);
    K2_API void                 AddResourceWatcher(IResourceWatcher* pWatcher, ResHandle hRegisterWith);

    K2_API void                 RegisterMapResource(const tstring &sPath)       { m_setMapSpecificResources.insert(sPath); }
    K2_API bool                 IsMapResource(const tstring &sPath)         { return (m_setMapSpecificResources.find(sPath) != m_setMapSpecificResources.end()); }
    K2_API void                 ClearMapResources();

    // Get* Shortcuts
    K2_API CFontMap*            GetFontMap(ResHandle hFontMap);
    K2_API CFontFace*           GetFontFace(ResHandle hFontFace);
    K2_API CMaterial*           GetMaterial(ResHandle hMaterial);
    K2_API CTexture*            GetTexture(ResHandle hTexture);
    K2_API CVertexShader*       GetVertexShader(ResHandle hVertexShader);
    K2_API CPixelShader*        GetPixelShader(ResHandle hPixelShader);
    K2_API CModel*              GetModel(ResHandle hModel);
    K2_API CClip*               GetClip(ResHandle hClip);
    K2_API CSample*             GetSample(ResHandle hSample);
    K2_API CStringTable*        GetStringTable(ResHandle hStringTable);
    K2_API CEffect*             GetEffect(ResHandle hEffect);
    K2_API CInterfaceResource*  GetInterface(ResHandle hInterface);
    K2_API CPostEffect*         GetPostEffect(ResHandle hPostEffect);
    K2_API CBitmapResource*     GetBitmapResource(ResHandle hBitmapResource);
    K2_API CCursor*             GetCursor(ResHandle hCursor);
    K2_API CResourceReference*  GetReference(ResHandle hReference);

    template <class T>
    T*  Get(ResHandle h)    { return static_cast<T*>(Get(h)); }

    // Utility Functions
    K2_API const tstring&       GetPath(ResHandle hResource);
    K2_API SkinHandle           GetSkin(ResHandle hModel, const tstring &sName);
    K2_API void                 PrecacheSkin(ResHandle hModel, SkinHandle hSkin);
    K2_API int                  GetAnim(ResHandle hModel, const tstring &sName);
    K2_API const tstring&       GetAnimName(ResHandle hModel, uint uiIndex);
    K2_API int                  GetAnimLength(ResHandle hModel, uint uiIndex);
    K2_API const CBBoxf&        GetModelBounds(ResHandle hModel);
    K2_API CBBoxf               GetModelSurfaceBounds(ResHandle hModel);
    K2_API const SurfVector&    GetModelSurfaces(ResHandle hModel);
    K2_API const tstring&       GetString(ResHandle hStringTable, const tstring &sKey);
    K2_API void                 UpdateReference(ResHandle hReference, ResHandle hResource);
    K2_API ResHandle            GetReferencedResource(ResHandle hReference);
    K2_API bool                 IsStringTableExtension(const tstring &sPath);
    K2_API void                 ReloadStringTableExtension(const tstring &sPath);
    
    // Special resource accessors
    K2_API bool                 RegisterStandardResources();
    ResHandle                   GetWhiteTexture() const         { return m_hWhiteTexture; }
    ResHandle                   GetBlackTexture() const         { return m_hBlackTexture; }
    ResHandle                   GetCheckerTexture() const       { return m_hCheckerTexture; }
    ResHandle                   GetInvisTexture() const         { return m_hInvisTexture; }
    ResHandle                   GetFlatTexture() const          { return m_hFlatTexture; }
    ResHandle                   GetTeapotModel() const          { return m_hTeapotModel; }

    K2_API void                 PrintResource(CConsoleStream& cStream, ResHandle hResource, bool bCompact = false, const TCHAR* pOverrideColor = nullptr);
    K2_API void                 PrintResource(CConsoleStream& cStream, IResource* pResource, bool bCompact = false, const TCHAR* pOverrideColor = nullptr);

    void                        ListResources();
    void                        ListResourceUsage() const;

    // retrieve a list of resources matching the wildcards.
    K2_API uint                 FindResources(vector<ResPtrVec>& vResults, const tstring &sWildcard);

    // execute a command on each resource that matches 'sWildcard'.
    K2_API void                 ExecCommand(const tstring &sWildcard = _T("*"),
                                            const tstring &sCmd = _T("list"),
                                            const tsvector& vArgList = tsvector());

    void                        StartResLoadProfile(uint uiType, ULONGLONG llTime, ULONGLONG llMemory)
    {
        m_vResLoadProfile.push_back(SResLoadProfile(uiType, llTime, llMemory));
    }

    K2_API void                 EndResLoadProfile(uint uiType, ULONGLONG llTime, ULONGLONG llMemory);
};
//=============================================================================

#endif //__C_RESOURCEMANAGER_H__
