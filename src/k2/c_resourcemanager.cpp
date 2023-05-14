// (C)2005 S2 Games
// c_resourcemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_resourcemanager.h"
#include "i_resourcelibrary.h"
#include "i_resource.h"
#include "c_texture.h"
#include "c_material.h"
#include "c_statestring.h"
#include "c_resourcereference.h"
#include "c_anim.h"
#include "c_k2model.h"
#include "c_xmlmanager.h"
#include "c_uimanager.h"
#include "c_filechangecallback.h"
#include "c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CResourceManager    &g_ResourceManager(*CResourceManager::GetInstance());

SINGLETON_INIT(CResourceManager)

EXTERN_CVAR_STRING(host_language);
//=============================================================================

/*====================
  CResourceManager::CResourceManager
  ====================*/
CResourceManager::CResourceManager() :
m_hWhiteTexture(INVALID_RESOURCE),
m_hBlackTexture(INVALID_RESOURCE),
m_hCheckerTexture(INVALID_RESOURCE),
m_hInvisTexture(INVALID_RESOURCE),
m_hFlatTexture(INVALID_RESOURCE),
m_hTeapotModel(INVALID_RESOURCE)
{
    CResourceInfo::GetInstance()->ActivateContextPushTop(_T("global"));
}


/*====================
  CResourceManager::~CResourceManager
  ====================*/
CResourceManager::~CResourceManager()
{
    g_ResourceInfo.DeactivateContextPopTop(_T("global"));
}


/*====================
  CResourceManager::RegisterStandardResources
  ====================*/
bool    CResourceManager::RegisterStandardResources()
{
    PROFILE("CResourceManager::RegisterStandardResources");

    bool bFault(false);

    m_pNullMaterial = K2_NEW(ctx_Resources,  CMaterial)(TSNULL, _T("nullptr Material"));
    if (m_pNullMaterial == nullptr || !m_pNullMaterial->LoadNull())
    {
        bFault = true;
        if (m_pNullMaterial != nullptr)
            K2_DELETE(m_pNullMaterial);
        m_pNullMaterial = nullptr;
        Console.Warn << _T("CResourceManager::RegisterResources() - Failed to generate a nullptr material") << newl;
    }

    m_hWhiteTexture = Register(_T("$white"), RES_TEXTURE);
    m_hBlackTexture = Register(_T("$black"), RES_TEXTURE);
    m_hCheckerTexture = Register(_T("$checker"), RES_TEXTURE);
    m_hInvisTexture = Register(_T("$invis"), RES_TEXTURE);
    m_hFlatTexture = Register(_T("$flat"), RES_TEXTURE);
    m_hTeapotModel = Register(_T("/core/null/null.mdf"), RES_MODEL);

#if 0
    const tsvector& vModPathStack(FileManager.GetModStack());
    for (tsvector::const_reverse_iterator itMod(vModPathStack.rbegin()); itMod != vModPathStack.rend(); ++itMod)
    {
        CFileHandle hResourceList(_T("/core_") + host_language + _T(".resources"), FILE_READ, *itMod);
        if (!hResourceList.IsOpen())
            continue;

        XMLManager.Process(hResourceList, _T("resourcelist"));
    }
#else
    XMLManager.Process(_T("/core_") + host_language + _T(".resources"), _T("resourcelist"));
#endif

    return bFault;
}


/*====================
  CResourceManager::RegisterLibrary
  ====================*/
void    CResourceManager::RegisterLibrary(uint uiType, IResourceLibrary *pLib)
{
    if (m_vResourceLibs.size() <= uiType)
        m_vResourceLibs.resize(uiType + 1, nullptr);

    if (m_vResourceLibs[uiType] != nullptr)
    {
        Console.Err << _T("CResourceManager::RegisterLibrary() - Duplicate library type") << newl;
        return;
    }

    m_vResourceLibs[uiType] = pLib;
}


/*====================
  CResourceManager::UnregisterLibrary
  ====================*/
void    CResourceManager::UnregisterLibrary(uint uiType)
{
    if (m_vResourceLibs.size() <= uiType)
        return;

    m_vResourceLibs[uiType] = nullptr;
}


/*====================
  CResourceManager::GetLib
  ====================*/
IResourceLibrary*   CResourceManager::GetLib(uint uiType)
{
    if (uiType >= m_vResourceLibs.size() || m_vResourceLibs[uiType] == nullptr)
    {
        Console.Err << _T("No library registered for resource type: ") << uiType << newl;
        return nullptr;
    }

    return m_vResourceLibs[uiType];
}


/*====================
  CResoureManager::Register
  ====================*/
ResHandle   CResourceManager::Register(IResource *pResource, uint uiType, uint uiIgnoreFlags)
{
    PROFILE("CResourceManager::Register");

    assert(!(pResource->GetPath().empty() && pResource->GetName().empty()));

    try
    {
        ResHandle hResult;
        tstring sPath(pResource->GetPath());

        if (!K2System.IsDedicatedServer() && !K2System.IsServerManager())
            uiIgnoreFlags = 0;

        if (!sPath.empty() && sPath[0] == _T('!'))
        {
            IResourceLibrary *pLib(GetLib(RES_REFERENCE));

            if (uiType == RES_REFERENCE)
            {
                ResHandle hOld(pLib->LookUpPath(sPath));
                
                if (hOld != INVALID_RESOURCE)
                {
                    static_cast<CResourceReference *>(Get(hOld))->SetReference(static_cast<CResourceReference *>(pResource)->GetReference());

                    K2_DELETE(pResource);
                    return hOld;
                }

                hResult = pLib->Register(pResource, uiIgnoreFlags);
            }
            else
            {
                hResult = pLib->Register(sPath, uiIgnoreFlags);
            }
        }
        else
        {       
            IResourceLibrary *pLib(GetLib(uiType));
            hResult = pLib->Register(pResource, uiIgnoreFlags);
        }
        
        if (hResult != INVALID_RESOURCE && !Get(hResult)->GetPath().empty())
            m_mapActivePaths[Get(hResult)->GetLocalizedPath()] = hResult;

        return hResult;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::Register() - "), NO_THROW);
        return INVALID_RESOURCE;
    }
}

ResHandle   CResourceManager::Register(const tstring &sPath, uint uiType, uint uiIgnoreFlags)
{
    PROFILE("CResourceManager::Register");

    try
    {
        if (sPath.empty())
            return INVALID_RESOURCE;

        if (!K2System.IsDedicatedServer() && !K2System.IsServerManager())
            uiIgnoreFlags = 0;

        ResHandle hResult;
        if (!sPath.empty() && sPath[0] == _T('!')) // Check for reference
        {           
            IResourceLibrary *pLib(GetLib(RES_REFERENCE));
            hResult = pLib->Register(sPath, uiIgnoreFlags);
        }
        else
        {       
            IResourceLibrary *pLib(GetLib(uiType));
            hResult = pLib->Register(sPath, uiIgnoreFlags);
        }

        IResource *pRes(Get(hResult));

        if (pRes != nullptr && !pRes->GetPath().empty())
        {
            m_mapActivePaths[pRes->GetLocalizedPath()] = hResult;

            if (IsMapResource(pRes->GetPath()))
                pRes->AddFlags(RES_MAP_SPECIFIC);
            else
                pRes->ClearFlags(RES_MAP_SPECIFIC);
        }

        return hResult;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::Register() - "), NO_THROW);
        return INVALID_RESOURCE;
    }
}


/*====================
  CResourceManager::Unregister
  ====================*/
void    CResourceManager::Unregister(ResHandle hResource, EUnregisterResource eUnregisterOp)
{
    if (hResource == INVALID_RESOURCE)
        return;

    try
    {
        IResourceLibrary *pLib(GetLib(Res_GetType(hResource)));

        IResource *pResource(pLib->Get(hResource));
        if (pResource == nullptr)
            EX_ERROR(_T("Couldn't retrieve resource"));

        if (eUnregisterOp == UNREG_RESERVE_HANDLE)
            Console.Res << _T("Unregistering ");
        else if (eUnregisterOp == UNREG_DELETE_HANDLE)
            Console.Res << _T("Deleting ");
        PrintResource(Console.DefaultStream(), hResource);
        Console << newl;

        if (!pResource->GetPath().empty())
        {
            PathHandleMap::iterator findit(m_mapActivePaths.find(pResource->GetPath()));
            if (findit != m_mapActivePaths.end())
                m_mapActivePaths.erase(findit);
        }

        pLib->Unregister(hResource, eUnregisterOp);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::Unregister() - "), NO_THROW);
    }
}


/*====================
  CResourceManager::Reload
  ====================*/
bool    CResourceManager::Reload(ResHandle hResource)
{
    if (hResource == INVALID_RESOURCE)
        return false;

    try
    {
        IResourceLibrary *pLib(GetLib(Res_GetType(hResource)));
        return pLib->Reload(hResource, 0xffffffff);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::Reload() - "), NO_THROW);
        return false;
    }
}


/*====================
  CResourceManager::Reload
  ====================*/
bool    CResourceManager::Reload(const tstring &sDirtyPath)
{
    tstring sPath(FileManager.SanitizePath(sDirtyPath));

    FileChangeCallbackSet setFileChangeCallbacks;
    FileChangeCallbackMap::iterator itFind(m_mapFileChangeCallbacks.find(sPath));
    if (itFind != m_mapFileChangeCallbacks.end())
        setFileChangeCallbacks = itFind->second; // Copy because set can change during reloading

    for (FileChangeCallbackSet::iterator it(setFileChangeCallbacks.begin()); it != setFileChangeCallbacks.end(); ++it)
    {
        IFileChangeCallback* pFileChangeCallback(*it);
        if (pFileChangeCallback->GetPath() == sPath)
        {
            if (pFileChangeCallback->TryExecute())
            {
                pFileChangeCallback->Execute();
                pFileChangeCallback->AfterExecute();
            }
        }
    }

    UIManager.ProcessDeferedReloads();

    return Reload(LookUpPath(sPath));
}


/*====================
  CResourceManager::ReloadByFlag
  ====================*/
void    CResourceManager::ReloadByFlag(int iFlag)
{
    try
    {
        for (ResourceLibVector::iterator it(m_vResourceLibs.begin()); it != m_vResourceLibs.end(); ++it)
        {
            if (*it == nullptr)
                continue;

            (*it)->ReloadByFlag(iFlag);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::ReloadByFlag() - "), NO_THROW);
    }
}


/*====================
  CResourceManager::LookUpPath
  ====================*/
ResHandle   CResourceManager::LookUpPath(const tstring &sDirtyPath)
{
    tstring sPath(FileManager.SanitizePath(sDirtyPath));
    try
    {
        PathHandleMap::iterator findit(m_mapActivePaths.find(sPath));
        if (findit == m_mapActivePaths.end())
        {
            //Console.Warn << _T("No resource found with path: ") << sPath << newl;
            return INVALID_RESOURCE;
        }

        return findit->second;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::LookUpPath() - "), NO_THROW);
        return INVALID_RESOURCE;
    }
}


/*====================
  CResourceManager::LookUpName
  ====================*/
ResHandle   CResourceManager::LookUpName(const tstring &sName, EResourceType eType)
{
    try
    {
        IResourceLibrary *pLib(GetLib(eType));
        if (pLib == nullptr)
            EX_ERROR(_T("Library not found"));

        return pLib->LookUpName(sName);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::LookUpName() - "), NO_THROW);
        return INVALID_RESOURCE;
    }
}


/*====================
  CResourceManager::LookUpPrecached
  ====================*/
ResHandle   CResourceManager::LookUpPrecached(const tstring &sDirtyPath)
{
    ResHandle hRes(LookUpPath(sDirtyPath));
    if (hRes == INVALID_RESOURCE)
    {
        tstring sPath(FileManager.SanitizePath(sDirtyPath));
        Console.Warn << _T("Resource '^y") << sPath << _T("^*' was not precached.  Loading...") << newl;
    }
    return hRes;
}


/*====================
  CResourceManager::LookUpHandle
  ====================*/
IResource*  CResourceManager::LookUpHandle(ResHandle hResource, uint uiType)
{
    try
    {
        if (hResource == INVALID_RESOURCE)
            return nullptr;

        uint uiResType(Res_GetType(hResource));

        if (uiType == RES_UNKNOWN)
            uiType = uiResType;
        else if (uiResType != uiType)
            EX_ERROR(_T("Handle is of wrong type"));

        IResourceLibrary *pLib(GetLib(uiResType));
        if (pLib == nullptr)
            return nullptr;

        IResource* pResource(pLib->LookUpHandle(hResource));
        return pResource;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::LookUpHandle(") + XtoA(hResource, 0, 0, 16) + _T(", ") + XtoA(uiType) + _T(") - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  CResourceManager::Get
  ====================*/
IResource*  CResourceManager::Get(ResHandle hResource)
{
    try
    {
        if (hResource == INVALID_RESOURCE)
            return nullptr;

        IResourceLibrary *pLib(GetLib(Res_GetType(hResource)));
        if (pLib == nullptr)
            return nullptr;

        IResource* pResource(pLib->Get(hResource));
        return pResource;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::Get() - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  CResourceManager::Get

  Resource lookup with reference support
  ====================*/
IResource*  CResourceManager::Get(ResHandle hResource, EResourceType eType)
{
    try
    {
        if (Res_GetType(hResource) == RES_REFERENCE)
            hResource = GetReferencedResource(hResource);

        if (hResource == INVALID_RESOURCE || Res_GetType(hResource) != eType)
            return nullptr;

        IResourceLibrary *pLib(GetLib(eType));

        IResource* pResource(pLib->Get(hResource));
        return pResource;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CResourceManager::Get() - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  CResourceManager::RegisterFileChangeCallback
  ====================*/
void    CResourceManager::RegisterFileChangeCallback(IFileChangeCallback *pCallback)
{
    if (pCallback == nullptr)
        return;

    const tstring &sPath(pCallback->GetPath());

    FileChangeCallbackMap::iterator itFind(m_mapFileChangeCallbacks.find(sPath));
    if (itFind == m_mapFileChangeCallbacks.end())
        itFind = m_mapFileChangeCallbacks.insert(std::make_pair(sPath, FileChangeCallbackSet())).first;

    FileChangeCallbackSet& setCallbacks(itFind->second);
    setCallbacks.insert(pCallback);
}


/*====================
  CResourceManager::UnregisterFileChangeCallback
  ====================*/
void    CResourceManager::UnregisterFileChangeCallback(IFileChangeCallback *pCallback)
{
    if (pCallback == nullptr)
        return;

    const tstring &sPath(pCallback->GetPath());

    FileChangeCallbackMap::iterator itFind(m_mapFileChangeCallbacks.find(sPath));
    assert(itFind != m_mapFileChangeCallbacks.end());
    if (itFind == m_mapFileChangeCallbacks.end())
        return;

    FileChangeCallbackSet& setCallbacks(itFind->second);
    setCallbacks.erase(pCallback);

    if (setCallbacks.empty())
        m_mapFileChangeCallbacks.erase(itFind);
}


/*====================
  CResourceManager::RemoveResourceWatcher
  ====================*/
void        CResourceManager::RemoveResourceWatcher(IResourceWatcher* pWatcher, ResHandle hUnregisterFrom)
{
    if (hUnregisterFrom == INVALID_RESOURCE)
        return;

    IResourceLibrary *pLib(GetLib(Res_GetType(hUnregisterFrom)));
    assert(pLib != nullptr);
    if (pLib == nullptr)
        return;

    pLib->RemoveResourceWatcher(pWatcher, hUnregisterFrom);
}


/*====================
  CResourceManager::AddResourceWatcher
  ====================*/
void        CResourceManager::AddResourceWatcher(IResourceWatcher* pWatcher, ResHandle hRegisterWith)
{
    if (hRegisterWith == INVALID_RESOURCE)
        return;

    IResourceLibrary *pLib(GetLib(Res_GetType(hRegisterWith)));
    assert(pLib != nullptr);
    if (pLib == nullptr)
        return;

    pLib->AddResourceWatcher(pWatcher, hRegisterWith);
}


//=============================================================================
// Get* Shortcuts
//=============================================================================
#include "c_fontmap.h"
#include "c_fontface.h"
#include "c_material.h"
#include "c_texture.h"
#include "c_vertexshader.h"
#include "c_pixelshader.h"
#include "c_model.h"
#include "c_clip.h"
#include "c_sample.h"
#include "c_stringtable.h"
#include "c_effect.h"
#include "c_interfaceresource.h"
#include "c_posteffect.h"
#include "c_bitmapresource.h"
#include "c_cursor.h"
#include "c_resourcereference.h"

/*====================
  CResourceManager::GetFontMap
  ====================*/
CFontMap*   CResourceManager::GetFontMap(ResHandle hFontMap)
{
    return static_cast<CFontMap *>(Get(hFontMap, RES_FONTMAP));
}


/*====================
  CResourceManager::GetFontFace
  ====================*/
CFontFace*  CResourceManager::GetFontFace(ResHandle hFontFace)
{
    return static_cast<CFontFace *>(Get(hFontFace, RES_FONTFACE));
}


/*====================
  CResourceManager::GetMaterial
  ====================*/
CMaterial*  CResourceManager::GetMaterial(ResHandle hMaterial)
{
    return static_cast<CMaterial *>(Get(hMaterial, RES_MATERIAL));
}


/*====================
  CResourceManager::GetTexture
  ====================*/
CTexture*   CResourceManager::GetTexture(ResHandle hTexture)
{
    return static_cast<CTexture *>(Get(hTexture, RES_TEXTURE));
}


/*====================
  CResourceManager::ClearMapResources
  ====================*/
void        CResourceManager::ClearMapResources()
{
    m_setMapSpecificResources.clear();
    ReloadByFlag(RES_MAP_SPECIFIC);
}


/*====================
  CResourceManager::GetVertexShader
  ====================*/
CVertexShader*  CResourceManager::GetVertexShader(ResHandle hVertexShader)
{
    return static_cast<CVertexShader *>(Get(hVertexShader, RES_VERTEX_SHADER));
}


/*====================
  CResourceManager::GetPixelShader
  ====================*/
CPixelShader*   CResourceManager::GetPixelShader(ResHandle hPixelShader)
{
    return static_cast<CPixelShader *>(Get(hPixelShader, RES_PIXEL_SHADER));
}


/*====================
  CResourceManager::GetModel
  ====================*/
CModel* CResourceManager::GetModel(ResHandle hModel)
{
    return static_cast<CModel *>(Get(hModel, RES_MODEL));
}


/*====================
  CResourceManager::GetClip
  ====================*/
CClip*  CResourceManager::GetClip(ResHandle hClip)
{
    return static_cast<CClip *>(Get(hClip, RES_CLIP));
}


/*====================
  CResourceManager::GetSample
  ====================*/
CSample*    CResourceManager::GetSample(ResHandle hSample)
{
    return static_cast<CSample *>(Get(hSample, RES_SAMPLE));
}


/*====================
  CResourceManager::GetStringTable
  ====================*/
CStringTable*   CResourceManager::GetStringTable(ResHandle hStringTable)
{
    return static_cast<CStringTable *>(Get(hStringTable, RES_STRINGTABLE));
}


/*====================
  CResourceManager::GetEffect
  ====================*/
CEffect*    CResourceManager::GetEffect(ResHandle hEffect)
{
    return static_cast<CEffect *>(Get(hEffect, RES_EFFECT));
}


/*====================
  CResourceManager::GetInterface
  ====================*/
CInterfaceResource* CResourceManager::GetInterface(ResHandle hInterface)
{
    return static_cast<CInterfaceResource *>(Get(hInterface, RES_INTERFACE));
}


/*====================
  CResourceManager::GetPostEffect
  ====================*/
CPostEffect*    CResourceManager::GetPostEffect(ResHandle hPostEffect)
{
    return static_cast<CPostEffect *>(Get(hPostEffect, RES_POST_EFFECT));
}


/*====================
  CResourceManager::GetBitmapResource
  ====================*/
CBitmapResource*    CResourceManager::GetBitmapResource(ResHandle hBitmapResource)
{
    return static_cast<CBitmapResource *>(Get(hBitmapResource, RES_BITMAP));
}


/*====================
  CResourceManager::GetCursor
  ====================*/
CCursor*    CResourceManager::GetCursor(ResHandle hCursor)
{
    return static_cast<CCursor *>(Get(hCursor, RES_K2CURSOR));
}

/*====================
  CResourceManager::GetReference
  ====================*/
CResourceReference* CResourceManager::GetReference(ResHandle hReference)
{
    if (Res_GetType(hReference) != RES_REFERENCE)
        return nullptr;

    return static_cast<CResourceReference*>(Get(hReference));
}


/*====================
  CResourceManager::GetSkin
  ====================*/
SkinHandle      CResourceManager::GetSkin(ResHandle hModel, const tstring &sName)
{
    CModel *pModel(GetModel(hModel));

    if (pModel)
        return pModel->GetSkinHandle(sName);
    else
        return 0;
}


/*====================
  CResourceManager::GetAnim
  ====================*/
int     CResourceManager::GetAnim(ResHandle hModel, const tstring &sName)
{
    CModel *pModel(GetModel(hModel));

    if (!pModel || !pModel->GetModelFile() || pModel->GetModelFile()->GetType() != MODEL_K2)
        return -1;

    CK2Model *pK2Model = static_cast<CK2Model *>(pModel->GetModelFile());

    return pK2Model->GetAnimIndex(sName);
}


/*====================
  CResourceManager::PrecacheSkin
  ====================*/
void    CResourceManager::PrecacheSkin(ResHandle hModel, SkinHandle hSkin)
{
    CModel *pModel(GetModel(hModel));

    if (!pModel || !pModel->GetModelFile() || pModel->GetModelFile()->GetType() != MODEL_K2)
        return;

    CK2Model *pK2Model(static_cast<CK2Model *>(pModel->GetModelFile()));

    if (hSkin == INVALID_RESOURCE)
        pK2Model->LoadAllSkinMaterials();
    else
        pK2Model->LoadSkinMaterials(hSkin);
} 


/*====================
  CResourceManager::GetAnimName
  ====================*/
const tstring&  CResourceManager::GetAnimName(ResHandle hModel, uint uiIndex)
{
    CModel *pModel = GetModel(hModel);

    if (!pModel || !pModel->GetModelFile() || pModel->GetModelFile()->GetType() != MODEL_K2)
        return TSNULL;

    CK2Model *pK2Model = static_cast<CK2Model *>(pModel->GetModelFile());

    CAnim *pAnim(pK2Model->GetAnim(uiIndex));
    if (pAnim)
        return pAnim->GetName();
    else
        return TSNULL;
}


/*====================
  CResourceManager::GetAnimLength
  ====================*/
int     CResourceManager::GetAnimLength(ResHandle hModel, uint uiIndex)
{
    CModel *pModel = GetModel(hModel);

    if (!pModel || !pModel->GetModelFile() || pModel->GetModelFile()->GetType() != MODEL_K2)
        return 0;

    CK2Model *pK2Model = static_cast<CK2Model *>(pModel->GetModelFile());

    CAnim *pAnim(pK2Model->GetAnim(uiIndex));
    if (pAnim)
        return pAnim->GetLength();
    else
        return 0;
}


/*====================
  CResourceManager::GetModelBounds
  ====================*/
static CBBoxf   s_bbDummy;
const CBBoxf&   CResourceManager::GetModelBounds(ResHandle hModel)
{
    CModel *pModel = GetModel(hModel);

    if (pModel)
        return pModel->GetBounds();
    else
        return s_bbDummy;
}


/*====================
  CResourceManager::GetModelSurfaceBounds
  ====================*/
CBBoxf  CResourceManager::GetModelSurfaceBounds(ResHandle hModel)
{
    CModel *pModelRes = GetModel(hModel);

    if (!pModelRes || !pModelRes->GetModelFile())
        return CBBoxf(0.0f, 0.0f);

    IModel *pModel(pModelRes->GetModelFile());

    CBBoxf bbBounds;

    SurfVector &surfs(pModel->GetSurfs());
    if (surfs.size() == 0)
    {
        //Console.Warn << _T("GetModelSurfaceBounds() - Model has no surfs, using model bounds: ") << pModel->GetName() << newl;
        return pModel->GetBounds();
    }
    for (SurfVector::iterator itSurf(surfs.begin()); itSurf != surfs.end(); ++itSurf)
        bbBounds.AddBox(itSurf->GetBounds());

    return bbBounds;
}


/*====================
  CResourceManager::GetModelSurfaces
  ====================*/
const SurfVector&   CResourceManager::GetModelSurfaces(ResHandle hModel)
{
    CModel *pModelRes = GetModel(hModel);

    if (!pModelRes || !pModelRes->GetModelFile())
    {
        if (hModel == m_hTeapotModel)
            EX_ERROR(_T("nullptr model not correctly registered!"));

        return GetModelSurfaces(m_hTeapotModel);
    }

    IModel *pModel(pModelRes->GetModelFile());

    return pModel->GetSurfs();
}


/*====================
  CResourceManager::GetString
  ====================*/
const tstring&  CResourceManager::GetString(ResHandle hStringTable, const tstring &sKey)
{
    CStringTable *pTable(GetStringTable(hStringTable));

    if (pTable)
        return pTable->Get(sKey);
    else
        return TSNULL;
}


/*====================
  CResourceManager::GetPath
  ====================*/
const tstring&  CResourceManager::GetPath(ResHandle hResource)
{
    IResource *pResource(Get(hResource));

    if (!pResource)
        return TSNULL;
    else
        return pResource->GetPath();
}


/*====================
  CResourceManager::UpdateReference
  ====================*/
void    CResourceManager::UpdateReference(ResHandle hReference, ResHandle hResource)
{
    CResourceReference *pReference(GetReference(hReference));
    
    if (pReference)
        pReference->SetReference(hResource);
}


/*====================
  CResourceManager::GetReferencedResource
  ====================*/
ResHandle   CResourceManager::GetReferencedResource(ResHandle hReference)
{
    CResourceReference *pReference(GetReference(hReference));
    
    if (pReference)
        return pReference->GetReference();
    else
        return INVALID_RESOURCE;
}


/*====================
  CResourceManager::IsStringTableExtension
  ====================*/
bool    CResourceManager::IsStringTableExtension(const tstring &sPath)
{
    IResourceLibrary *pLib(GetLib(RES_STRINGTABLE));
    if (pLib == nullptr)
        return false;

    tstring sStrippedPath(Filename_StripExtension(sPath));
    const ResNameMap &mapPaths(pLib->GetResourcePathMap());

    for (ResNameMap::const_iterator it(mapPaths.begin()), itEnd(mapPaths.end()); it != itEnd; ++it)
    {
        tstring sStrippedPathRes(Filename_StripExtension(it->first));

        if (sStrippedPathRes.compare(0, sStrippedPathRes.length(), sStrippedPath, 0, sStrippedPathRes.length()) == 0)
            return true;
    }

    return false;
}


/*====================
  CResourceManager::ReloadStringTableExtension
  ====================*/
void    CResourceManager::ReloadStringTableExtension(const tstring &sPath)
{
    IResourceLibrary *pLib(GetLib(RES_STRINGTABLE));
    if (pLib == nullptr)
        return;

    tstring sStrippedPath(Filename_StripExtension(sPath));
    const ResNameMap &mapPaths(pLib->GetResourcePathMap());

    for (ResNameMap::const_iterator it(mapPaths.begin()), itEnd(mapPaths.end()); it != itEnd; ++it)
    {
        tstring sStrippedPathRes(Filename_StripExtension(it->first));

        if (sStrippedPathRes.compare(0, sStrippedPathRes.length(), sStrippedPath, 0, sStrippedPathRes.length()) == 0)
        {
            Reload(it->second);
            break;
        }
    }
}


/*====================
  CResourceManager::PrintResource
  ====================*/
void    CResourceManager::PrintResource(CConsoleStream& cStream, ResHandle hResource, bool bCompact, const TCHAR* pOverrideColor)
{
    const TCHAR* col(pOverrideColor);

    if (hResource == INVALID_RESOURCE)
    {
        cStream << (col ? col : _T("^r")) << _T("{invalid resource}");
        return;
    }

    IResource* pRes(LookUpHandle(hResource, Res_GetType(hResource)));
    if (pRes != nullptr)
    {
        PrintResource(cStream, pRes, bCompact, pOverrideColor);
        return;
    }

    // the resource has been unregistered or the handle is invalid; print as much info as possible.
    cStream << (col ? col : _T("^r"));

    cStream << _T("#") << XtoA(Res_GetIndex(hResource), FMT_PADZERO, 4);

    uint uiResType(Res_GetType(hResource));
    IResourceLibrary* pLib(GetLib(uiResType));
    assert(pLib != nullptr);
    if (pLib == nullptr)
        cStream << XtoA(_TS("[UNKNOWN] {type ") + XtoA(uiResType) + _T("}"), FMT_ALIGNLEFT, 16);
    else
        cStream << XtoA(_TS("[DELETED] ") + pLib->GetTypeName(), FMT_ALIGNLEFT, 16);
}


/*====================
  CResourceManager::PrintResource
  ====================*/
void    CResourceManager::PrintResource(CConsoleStream& cStream, IResource* pResource, bool bCompact, const TCHAR* pOverrideColor)
{
    const TCHAR* col(pOverrideColor);

    if (pResource == nullptr)
    {
        cStream << (col ? col : _T("^r")) << _T("{nullptr resource}");
        return;
    }

    ResHandle hRes(pResource->GetHandle());
    assert(hRes != INVALID_RESOURCE);

    uint uiType(Res_GetType(hRes));
    uint uiIdx(Res_GetIndex(hRes));

    if (uiType == RES_REFERENCE)
        bCompact = true;

    cStream << (col ? col : _T("^c")) << _T("#") << XtoA(uiIdx, FMT_PADZERO, 4);
    cStream << (col ? col : _T("^o")) << _T(" ") << XtoA(pResource->GetResTypeName(), FMT_ALIGNLEFT, bCompact ? 0 : 16);
    cStream << (col ? col : _T("^w")) << _T(" ") << XtoA(pResource->GetName(), FMT_ALIGNLEFT, bCompact ? 0 : 20);
    cStream << (col ? col : _T("^y")) << _T(" ") << XtoA(pResource->GetPath(), FMT_ALIGNLEFT, (uiType == RES_REFERENCE) ? 20 : 0);

    if (pResource->HasFlags(RES_EXTERNAL))
        cStream << (col ? col : _T("^g")) << _T(" [EXTERN]");

    if (pResource->HasFlags(RES_LOAD_FAILED))
        cStream << (col ? col : _T("^r")) << _T(" [FAILED]");

    // for resource references, print the target resource.
    if (uiType == RES_REFERENCE)
    {
        cStream << (col ? col : _T("^m")) << _T("  ->  ");

        ResHandle hReferenced(GetReferencedResource(hRes));

        if (hReferenced == INVALID_RESOURCE)
            cStream << (col ? col : _T("^v")) << _T("{none}");
        else
            PrintResource(cStream, hReferenced, true, pOverrideColor);
    }
}


/*====================
  CResourceManager::ListResources
  ====================*/
void    CResourceManager::ListResources()
{
    ExecCommand(_T("list"), _T("*"));
}


/*====================
  CResourceManager::ListResourceUsage
  ====================*/
void    CResourceManager::ListResourceUsage() const
{
    const LONGLONG FREQUENCY = K2System.GetFrequency();
    ULONGLONG llTotalTime(0);
    ULONGLONG llTotalMemory(0);

    Console << _T("Resource Usage") << newl
            << _T("---------") << newl;

    Console << _T("Library            Load Time  Memory") << newl;

    for (ResourceLibVector::const_iterator it(m_vResourceLibs.begin()); it != m_vResourceLibs.end(); ++it)
    {
        if (*it == nullptr)
            continue;

        Console << XtoA(tstring((*it)->GetName()).substr(0, 18), FMT_ALIGNLEFT, 19)
                << XtoA(XtoA(float((*it)->GetLoadTime()) / FREQUENCY) + _T(" s"), FMT_ALIGNLEFT, 11)
                << GetByteString((*it)->GetLoadMemory()) << newl;

        llTotalTime += (*it)->GetLoadTime();
        llTotalMemory += (*it)->GetLoadMemory();
    }

    Console << XtoA(tstring(_T("Total")).substr(0, 18), FMT_ALIGNLEFT, 19)
                << XtoA(XtoA(float(llTotalTime) / FREQUENCY) + _T(" s"), FMT_ALIGNLEFT, 11)
                << GetByteString(llTotalMemory) << newl;
}


/*====================
  CResourceManager::EndResLoadProfile
  ====================*/
void    CResourceManager::EndResLoadProfile(uint uiType, ULONGLONG llTime, ULONGLONG llMemory)
{
    if (m_vResLoadProfile.empty())
        return;

    const SResLoadProfile &cResLoadProfile(m_vResLoadProfile.back());
    IResourceLibrary *pLib(GetLib(uiType));

    if (cResLoadProfile.uiType != uiType || !pLib)
    {
        m_vResLoadProfile.pop_back();
        return;
    }

    ULONGLONG llDeltaTime(llTime > cResLoadProfile.llTime ? llTime - cResLoadProfile.llTime : 0);
    ULONGLONG llDeltaMemory(llMemory > cResLoadProfile.llMemory ? llMemory - cResLoadProfile.llMemory : 0);

    pLib->AddLoadTime(llDeltaTime);
    pLib->AddLoadMemory(llDeltaMemory);
    pLib->AddLoadCount(1);

    m_vResLoadProfile.pop_back();

    for (vector<SResLoadProfile>::iterator it(m_vResLoadProfile.begin()); it != m_vResLoadProfile.end(); ++it)
    {
        it->llTime += llDeltaTime;
        it->llMemory += llDeltaMemory;
    }
}


/*====================
  CResourceManager::FindResources
  ====================*/
uint    CResourceManager::FindResources(vector<ResPtrVec>& vResults, const tstring &sWildcard)
{
    uint uiTotal(0);

    // create an empty vector per resource lib.
    vResults.clear();

    for (ResourceLibVector::const_iterator it(m_vResourceLibs.begin()); it != m_vResourceLibs.end(); ++it)
    {
        vResults.push_back(ResPtrVec());
        ResPtrVec &vRes(vResults.back());

        IResourceLibrary* pLib(*it);
        if (pLib == nullptr)
            continue;

        uiTotal += pLib->FindResources(vRes, sWildcard);
    }

    return uiTotal;
}


/*====================
  CResourceManager::ExecCommand
  ====================*/
void    CResourceManager::ExecCommand(const tstring &sDirtyWildcard, const tstring &sDirtyCmd, const tsvector& vArgList)
{
    tstring sCmd(LowerString(sDirtyCmd));
    tstring sWildcard(LowerString(sDirtyWildcard));

    // find all resources matching the wildcard.
    vector<ResPtrVec> vMatches;
    FindResources(vMatches, sWildcard);

    // convert the resources into handles...
    vector<ResourceList> vHandleMatches;
    for (size_t i(0); i < vMatches.size(); ++i)
    {
        vHandleMatches.push_back(ResourceList());
        ResourceList& vResHandles(vHandleMatches.back());

        ResPtrVec &vRes(vMatches[i]);
        if (vRes.empty())
            continue;

        for (ResPtrVec::iterator it(vRes.begin()), itEnd(vRes.end());
            it != itEnd;
            ++it)
        {
            IResource* pResource(*it);

            vResHandles.push_back(pResource->GetHandle());
        }
    }

    // List resources
    if (sCmd == _T("list"))
    {
        Console << _T("Resources") << newl
                << _T("---------") << newl;

        for (size_t i(0); i < vMatches.size(); ++i)
        {
            ResPtrVec &vRes(vMatches[i]);
            if (vRes.empty())
                continue;

            IResourceLibrary* pLib(m_vResourceLibs[i]);
            Console << _T("Library: ") << pLib->GetName() << newl;

            for (ResPtrVec::iterator it(vRes.begin()), itEnd(vRes.end());
                it != itEnd;
                ++it)
            {
                IResource* pResource(*it);
                PrintResource(Console.DefaultStream(), pResource);
                Console << newl;
            }

            Console << newl;
        }

        return;
    }

    // Reload resources
    if (sCmd == _T("reload"))
    {
        for (size_t i(0); i < vHandleMatches.size(); ++i)
        {
            ResourceList& vRes(vHandleMatches[i]);
            if (vRes.empty())
                continue;

            IResourceLibrary* pLib(m_vResourceLibs[i]);
            Console << _T("Library: ") << pLib->GetName() << newl;

            for (ResourceList::iterator it(vRes.begin()), itEnd(vRes.end());
                it != itEnd;
                ++it)
            {
                ResHandle hResource(*it);
                Reload(hResource);
            }

            Console << newl;
        }
        return;
    }

    // Unregister resources
    if (sCmd == _T("unregister"))
    {
        for (size_t i(0); i < vMatches.size(); ++i)
        {
            ResourceList& vRes(vHandleMatches[i]);
            if (vRes.empty())
                continue;

            IResourceLibrary* pLib(m_vResourceLibs[i]);
            Console << _T("Library: ") << pLib->GetName() << newl;

            for (ResourceList::iterator it(vRes.begin()), itEnd(vRes.end());
                it != itEnd;
                ++it)
            {
                ResHandle hResource(*it);
                Unregister(hResource, UNREG_RESERVE_HANDLE);
            }

            Console << newl;
        }

        return;
    }

    Console << _T("ResourceCmd <wildcard> <command>") << newl;
    Console << _T("    where <command> = list, reload, or unregister") << newl;
}
