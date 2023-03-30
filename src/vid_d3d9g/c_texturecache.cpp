// (C)2008 S2 Games
// c_texturecache.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_texturecache.h"

#include "d3d9g_texture.h"

#include "../k2/c_xmldoc.h"
#include "../k2/c_xmlnode.h"
#include "../k2/c_xmlprocroot.h"
#include "../k2/c_xmlmanager.h"

#include <sys/utime.h>
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CTextureCache       g_TextureCache;
//=============================================================================

/*====================
  CTextureCacheNode::~CTextureCacheNode
  ====================*/
CTextureCacheNode::~CTextureCacheNode()
{
}


/*====================
  CTextureCacheNode::CTextureCacheNode
  ====================*/
CTextureCacheNode::CTextureCacheNode(const tstring &sDirectory, const string &sDefines) :
m_sDirectory(sDirectory),
m_sDefines(sDefines)
{
}


/*====================
  CTextureCacheNode::Load
  ====================*/
void    CTextureCacheNode::Load()
{
}


/*====================
  CTextureCacheNode::CacheTexture
  ====================*/
void    CTextureCacheNode::CacheTexture(const tstring &sPath, IDirect3DBaseTexture9 *pTexture, time_t tModTime)
{   
    tstring sTexturePath(Filename_StripExtension(sPath) + _T(".dds"));
    tstring sFilePath(_T("#/texturecache/") + m_sDirectory + sTexturePath);

    FileManager.MakeDir(Filename_GetPath(sFilePath));
    
    tstring sSystemPath(FileManager.GetSystemPath(sFilePath, TSNULL, true));
    D3DXSaveTextureToFile(sSystemPath.c_str(), D3DXIFF_DDS, pTexture, NULL);

    _utimbuf cTime;
    cTime.actime = tModTime;
    cTime.modtime = tModTime;

    _tutime(sSystemPath.c_str(), &cTime);
}


/*====================
  CTextureCacheNode::LoadTexture
  ====================*/
bool    CTextureCacheNode::LoadTexture(const tstring &sPath, time_t tModTime, tstring &sFilePath)
{
    tstring sTexturePath(Filename_StripExtension(sPath) + _T(".dds"));

    sFilePath = _T("#/texturecache/") + m_sDirectory + sTexturePath;

    _stat stat;
    if (FileManager.Stat(sFilePath, stat))
    {
        if (stat.st_mtime == tModTime)
            return true;
    }

    return false;
}


/*====================
  CTextureCache::~CTextureCache
  ====================*/
CTextureCache::~CTextureCache()
{
    Close();
}


/*====================
  CTextureCache::CTextureCache
  ====================*/
CTextureCache::CTextureCache() : 
m_sVersion(_T("4")),

m_uiActiveNode(-1),
m_bInitialized(false)
{   
}


/*====================
  CTextureCache::Initialize
  ====================*/
void    CTextureCache::Initialize()
{
    if (FileManager.Exists(TEXTURECACHE_FILENAME, FILE_NOARCHIVES))
        XMLManager.Process(TEXTURECACHE_FILENAME, _T("texturecache"), NULL);
    else
        FileManager.DeleteTree(_T("#/texturecache"));

    m_bInitialized = true;
}


/*====================
  CTextureCache::Close
  ====================*/
void    CTextureCache::Close()
{
    if (!m_bInitialized)
        return;

    for (vector<CTextureCacheNode *>::iterator it(m_vTextureCacheNodes.begin()); it != m_vTextureCacheNodes.end(); ++it)
        delete *it;

    m_vTextureCacheNodes.clear();
}


/*====================
  CTextureCache::RegisterNode
  ====================*/
uint    CTextureCache::RegisterNode(const tstring &sDirectory, const string &sDefines)
{
    m_vTextureCacheNodes.push_back(new CTextureCacheNode(sDirectory, sDefines));

    return uint(m_vTextureCacheNodes.size() - 1);
}


/*====================
  CTextureCache::ActivateNode
  ====================*/
void    CTextureCache::ActivateNode(const string &sDefines)
{
    for (vector<CTextureCacheNode *>::iterator it(m_vTextureCacheNodes.begin()); it != m_vTextureCacheNodes.end(); ++it)
    {
        if ((*it)->GetDefines() == sDefines)
        {
            m_uiActiveNode = it - m_vTextureCacheNodes.begin();
            return;
        }
    }

    m_uiActiveNode = -1;
}


/*====================
  CTextureCache::LoadTexture
  ====================*/
bool    CTextureCache::LoadTexture(const tstring &sPath, const tstring &sReference, tstring &sFilePath)
{
    if (!m_bInitialized || m_uiActiveNode == -1)
        return false;

    _stat stat;
    if (FileManager.Stat(sReference, stat))
        return m_vTextureCacheNodes[m_uiActiveNode]->LoadTexture(sPath, stat.st_mtime, sFilePath);
    else
        return false;
}


/*====================
  CTextureCache::CacheTexture
  ====================*/
void    CTextureCache::CacheTexture(const tstring &sPath, IDirect3DBaseTexture9 *pTexture, const tstring &sReference)
{
    if (!m_bInitialized)
        return;

    // Create a new texture cache node if we don't have one set
    if (m_uiActiveNode == -1)
    {
        uint uiNextNode(0);
        for (;;)
        {
            vector<CTextureCacheNode *>::iterator it(m_vTextureCacheNodes.begin());

            for (; it != m_vTextureCacheNodes.end(); ++it)
            {
                if (AtoI((*it)->GetDirectory()) == uiNextNode)
                {
                    ++uiNextNode;
                    break;
                }
            }

            if (it == m_vTextureCacheNodes.end())
                break;
        }

        const tstring &sDirectory(XtoA(uiNextNode, FMT_PADZERO, 8));

        m_uiActiveNode = RegisterNode(sDirectory, D3D_GetTextureDefinitionString());

        WriteCacheDescriptor();
    }

    if (!sReference.empty() && sReference[0] == _T('$'))
    {
        m_vTextureCacheNodes[m_uiActiveNode]->CacheTexture(_T("/") + sReference.substr(1), pTexture, 0);
    }
    else
    {
        _stat stat;
        if (FileManager.Stat(sReference, stat))
            m_vTextureCacheNodes[m_uiActiveNode]->CacheTexture(sPath, pTexture, stat.st_mtime);
    }
}


/*====================
  CTextureCache::CacheTexture
  ====================*/
void    CTextureCache::CacheTexture(const tstring &sPath, IDirect3DBaseTexture9 *pTexture)
{
    CacheTexture(sPath, pTexture, sPath);
}


/*====================
  CTextureCache::WriteCacheDescriptor
  ====================*/
void    CTextureCache::WriteCacheDescriptor()
{
    CXMLDoc xmlDescriptor(XML_ENCODE_UTF8);

    xmlDescriptor.NewNode("texturecache");

        xmlDescriptor.AddProperty("version", m_sVersion);
        xmlDescriptor.AddProperty("gamma", XtoA(vid_textureGammaCorrect));

        for (vector<CTextureCacheNode*>::iterator it(m_vTextureCacheNodes.begin()); it != m_vTextureCacheNodes.end(); ++it)
        {
            xmlDescriptor.NewNode("node");
                xmlDescriptor.AddProperty("directory", (*it)->GetDirectory());
                xmlDescriptor.AddProperty("defines", StringToTString((*it)->GetDefines()));
            xmlDescriptor.EndNode();
        }

    xmlDescriptor.EndNode();

    xmlDescriptor.WriteFile(TEXTURECACHE_FILENAME);

    Console.Video << _T("Wrote texturecache descriptor") << newl;
}


/*====================
  CTextureCache::Clear
  ====================*/
void    CTextureCache::Clear()
{
    Close();

    FileManager.DeleteTree(_T("#/texturecache"));

    m_bInitialized = true;
}


/*====================
  CTextureCache::Reload
  ====================*/
void    CTextureCache::Reload()
{
    Close();
    Initialize();
}


/*====================
  TextureCacheClear
  ====================*/
CMD(TextureCacheClear)
{
    g_TextureCache.Clear();
    return true;
}


namespace XMLTextureCache
{
    // <texturecache>
    DECLARE_XML_PROCESSOR(texturecache)
    BEGIN_XML_REGISTRATION(texturecache)
        REGISTER_XML_PROCESSOR(root)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(texturecache, void)
        const tstring &sVersion(node.GetProperty(_T("version")));
        float fGamma(node.GetPropertyFloat(_T("gamma")));
        if (sVersion != g_TextureCache.GetVersion() || fGamma != vid_textureGammaCorrect)
        {
            FileManager.DeleteTree(_T("#/texturecache"));
            return true;
        }
    END_XML_PROCESSOR(pVoid)


    // <node>
    DECLARE_XML_PROCESSOR(node)
    BEGIN_XML_REGISTRATION(node)
        REGISTER_XML_PROCESSOR(texturecache)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(node, void)
        g_TextureCache.RegisterNode(node.GetProperty(_T("directory")), TStringToString(node.GetProperty(_T("defines"))));
    END_XML_PROCESSOR_NO_CHILDREN
};
