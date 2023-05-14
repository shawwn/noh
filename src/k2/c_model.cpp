// (C)2005 S2 Games
// c_model.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_model.h"
#include "i_resourcelibrary.h"
#include "c_modelallocatorregistry.h"
#include "c_xmlmanager.h"
#include "i_model.h"
#include "c_vid.h"
#include "c_resourcemanager.h"
#include "c_filechangecallback.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*  AllocModel(const tstring &sPath);
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_STRINGF(model_quality,     "high",     CVAR_SAVECONFIG);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibModel(RES_MODEL, _T("Models"), CModel::ResTypeName(), true, AllocModel);
//=============================================================================

//=============================================================================
// CModelFileCallback
//=============================================================================
class CModelFileCallback : public IFileChangeCallback
{
private:
    tstring     m_sModel;

public:
    ~CModelFileCallback() {}
    CModelFileCallback(const tstring &sPath, const tstring &sModel) :
    IFileChangeCallback(sPath),
    m_sModel(sModel)
    {
    }

    void    Execute()
    {
        g_ResourceManager.Reload(m_sModel);
    }
};
//=============================================================================

/*====================
  AllocModel
  ====================*/
IResource*  AllocModel(const tstring &sPath)
{
    return K2_NEW(ctx_Models,  CModel)(sPath);
}


/*====================
  CModel::CModel
  ====================*/
CModel::CModel(const tstring &sPath) :
IResource(sPath, TSNULL),
m_pModelFile(nullptr)
{
}


/*====================
  CModel::Load
  ====================*/
int     CModel::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CModel::Load");

    try
    {
        if (!m_sPath.empty())
            Console.Res << _T("Loading Model ") << SingleQuoteStr(m_sPath);
        else if (!m_sName.empty())
            Console.Res << _T("Loading Model ") << SingleQuoteStr(m_sName);
        else
            Console.Res << _T("Loading Unknown Model");
        
#if 0
        if ((uiIgnoreFlags & RES_MODEL_IGNORE_VID))
            Console.Res << _T(" [IGNORE_VID]");
        if ((uiIgnoreFlags & RES_MODEL_IGNORE_EVENTS))
            Console.Res << _T(" [IGNORE_EVENTS]");
        if ((uiIgnoreFlags & RES_MODEL_IGNORE_GEOM))
            Console.Res << _T(" [IGNORE_GEOM]");
        if ((uiIgnoreFlags & RES_MODEL_IGNORE_POSE))
            Console.Res << _T(" [IGNORE_POSE]");
#endif

        Console.Res << newl;

        m_uiIgnoreFlags = uiIgnoreFlags;

        // Process the model XML file
        if (!XMLManager.ReadBuffer(pData, uiSize, _T("model"), this))
            EX_WARN(_T("Couldn't process XML"));

        // Register the model with the vid driver
        if (!(uiIgnoreFlags & RES_MODEL_IGNORE_VID))
            Vid.RegisterModel(this);

        if (m_pModelFile)
            m_pModelFile->PostLoad();

        m_pCallback = K2_NEW(ctx_Models,  CModelFileCallback)(m_sModelFilename, m_sPath);
        g_ResourceManager.RegisterFileChangeCallback(m_pCallback);

        if (!m_sPath.empty())
            Console.Res << "Finished loading model " << SingleQuoteStr(m_sPath) << newl;
        else if (!m_sName.empty())
            Console.Res << "Finished loading model " << SingleQuoteStr(m_sName) << newl;
        else
            Console.Res << "Finished loading model" << newl;
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CModel::Load(") + m_sPath + _TS(") - "), THROW);
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CModel::Free
  ====================*/
void    CModel::Free()
{
    if (!(m_uiIgnoreFlags & RES_MODEL_IGNORE_VID))
        Vid.UnregisterModel(this);
    
    SAFE_DELETE(m_pModelFile);

    if (m_pCallback != nullptr)
        g_ResourceManager.UnregisterFileChangeCallback(m_pCallback);
    SAFE_DELETE(m_pCallback);
}


/*====================
  CModel::LoadNull
  ====================*/
bool    CModel::LoadNull()
{
    try
    {
        CXMLNode nodeNULL;
        Allocate(_T(""), _T("/core/null/null.model"), _T("K2"), nodeNULL);

        // Register the model with the vid driver
        Vid.RegisterModel(this);

        if (m_pModelFile)
            m_pModelFile->PostLoad();

        m_pCallback = K2_NEW(ctx_Models,  CModelFileCallback)(m_sModelFilename, m_sPath);
        g_ResourceManager.RegisterFileChangeCallback(m_pCallback);
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CModel::Load(") + m_sName + _TS(") - "), NO_THROW);
        EX_FATAL(_T("Model nullptr resource failure"));
    }

    return true;
}


/*====================
  CModel::Allocate
  ====================*/
bool    CModel::Allocate(const tstring &sName, const tstring &sFilename, const tstring &sType, const CXMLNode &node)
{
    SAFE_DELETE(m_pModelFile);

    tstring sFilename2(sFilename);

    if (node.HasProperty(model_quality))
        sFilename2 = node.GetProperty(model_quality);

    m_pModelFile = CModelAllocatorRegistry::GetInstance()->Allocate(sType);
    m_sModelFilename = FileManager.SanitizePath(sFilename2);

    m_pModelFile->SetName(sName);
    m_pModelFile->SetModel(this);
    m_pModelFile->ProcessProperties(node);
    
    if (!m_pModelFile->Load(sFilename2, m_uiIgnoreFlags))
    {
        Console.Err << _T("Model loading failed, replacing with null model") << newl;

        K2_DELETE(m_pModelFile);

        m_pModelFile = CModelAllocatorRegistry::GetInstance()->Allocate(_T("K2"));
        m_pModelFile->SetName(sName);

        if (!m_pModelFile->Load(_T("/core/null/null.model"), m_uiIgnoreFlags))
            EX_FATAL(_T("ModelFile nullptr resource failure"));
    }

    return true;
}


/*====================
  CModel::GetSkinHandle
  ====================*/
SkinHandle  CModel::GetSkinHandle(const tstring &sName) const
{
    if (m_pModelFile)
        return m_pModelFile->GetSkinHandle(sName);
    else
        return 0;
}


/*====================
  CModel::GetBounds
  ====================*/
static CBBoxf   s_bbDummy;
const CBBoxf&   CModel::GetBounds() const
{
    if (m_pModelFile)
        return m_pModelFile->GetBounds();
    else
        return s_bbDummy;
}


/*--------------------
  precacheStartEffect
  --------------------*/
CMD_PRECACHE(StartEffect)
{
    if (vArgList.size() < 1)
        return false;

    g_ResourceManager.Register(vArgList[0], RES_EFFECT);
    return true;
}
