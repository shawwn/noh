// (C)2007 S2 Games
// c_shadercache.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_shadercache.h"

#include "c_shaderpreprocessor.h"

#include "../k2/c_xmldoc.h"
#include "../k2/c_xmlnode.h"
#include "../k2/c_xmlprocroot.h"
#include "../k2/c_xmlmanager.h"

#include <sys/utime.h>
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CShaderCache        g_ShaderCache;
//=============================================================================

/*====================
  CShaderCacheNode::~CShaderCacheNode
  ====================*/
CShaderCacheNode::~CShaderCacheNode()
{
    if (m_uiFlags & NODE_REWRITE || m_uiFlags & NODE_APPEND)
    {
        CArchive &cArchive(g_ShaderCache.GetArchiveWrite());

        CBufferDynamic cBuffer;
        for (map<wstring, SShaderCacheFileEntry>::iterator it(m_mapEntries.begin()); it != m_mapEntries.end(); ++it)
        {
            cBuffer << it->first << wchar_t(0);
            cBuffer << it->second.uiSize;
            cBuffer << LONGLONG(it->second.tModTime);
            cBuffer << it->second.uiCRC32;
            cBuffer.Append(m_cBuffer.Get(it->second.uiPos), it->second.uiSize);
        }

        cArchive.WriteFile(m_sDirectory, cBuffer.Get(), cBuffer.GetLength());
    }
}


/*====================
  CShaderCacheNode::CShaderCacheNode
  ====================*/
CShaderCacheNode::CShaderCacheNode(const tstring &sDirectory, const string &sDefines) :
m_sDirectory(sDirectory),
m_sDefines(sDefines),
m_uiFlags(0)
{
}


/*====================
  CShaderCacheNode::Load
  ====================*/
void    CShaderCacheNode::Load()
{
    CFileHandle hFile(m_sDirectory, FILE_READ | FILE_BINARY, g_ShaderCache.GetArchiveRead());
    if (hFile.IsOpen())
    {
        uint uiFileSize(0);
        const char *pFileBuffer(hFile.GetBuffer(uiFileSize));
        m_cBuffer.Append(pFileBuffer, uiFileSize);

        while (m_cBuffer.GetReadPos() < m_cBuffer.GetLength() && !m_cBuffer.GetFaults())
        {           
            wstring sPath(m_cBuffer.ReadWString());
            
            uint uiSize(m_cBuffer.ReadInt());
            time_t tModTime(m_cBuffer.ReadInt64());
            uint uiCRC32(m_cBuffer.ReadInt());
            uint uiPos(m_cBuffer.GetReadPos());
            
            m_mapEntries[sPath] = SShaderCacheFileEntry(uiPos, uiSize, tModTime, uiCRC32);

            m_cBuffer.Advance(uiSize);
        }
    }
}


/*====================
  CShaderCacheNode::CacheShader
  ====================*/
void    CShaderCacheNode::CacheShader(const wstring &sPath, const byte *pBuffer, uint uiSize, time_t tModTime, uint uiCRC32)
{
    assert(m_uiFlags & NODE_LOADED);

    m_cBuffer << sPath << wchar_t(0);

    m_cBuffer << uiSize;
    m_cBuffer << LONGLONG(tModTime);
    m_cBuffer << uiCRC32;

    map<wstring, SShaderCacheFileEntry>::iterator itFind(m_mapEntries.find(sPath));
    if (itFind != m_mapEntries.end())
        m_uiFlags |= NODE_REWRITE;
    else
        m_uiFlags |= NODE_APPEND;

    m_mapEntries[sPath] = SShaderCacheFileEntry(m_cBuffer.GetLength(), uiSize, tModTime, uiCRC32);

    m_cBuffer.Append(pBuffer, uiSize);
}


/*====================
  CShaderCacheNode::LoadShader
  ====================*/
bool    CShaderCacheNode::LoadShader(const wstring &sPath, IBuffer &cBuffer, time_t tModTime, uint &uiCRC32)
{
    if (!(m_uiFlags & NODE_LOADED))
    {
        Load();
        m_uiFlags |= NODE_LOADED;
    }

    map<wstring, SShaderCacheFileEntry>::iterator itFind(m_mapEntries.find(sPath));
    if (itFind != m_mapEntries.end() && itFind->second.tModTime == tModTime)
    {
        uiCRC32 = itFind->second.uiCRC32;
        cBuffer.Append(m_cBuffer.Get(itFind->second.uiPos), itFind->second.uiSize);
        return true;
    }

    return false;
}


/*====================
  CShaderCache::~CShaderCache
  ====================*/
CShaderCache::~CShaderCache()
{
    Close();
}


/*====================
  CShaderCache::CShaderCache
  ====================*/
CShaderCache::CShaderCache() : 
m_sVersion(_T("5")),

m_uiActiveNode(-1),
m_bInitialized(false)
{   
}


/*====================
  CShaderCache::Initialize
  ====================*/
void    CShaderCache::Initialize()
{
    if (FileManager.Exists(SHADERCACHE_FILENAME, FILE_NOARCHIVES))
        XMLManager.Process(SHADERCACHE_FILENAME, _T("shadercache"), NULL, FILE_ALLOW_CUSTOM);
    else
        FileManager.DeleteTree(_T("#/shadercache"));

    m_cArchiveRead.Open(SHADERCACHE_ARCHIVE_FILENAME, ARCHIVE_READ);

    m_bInitialized = true;
}


/*====================
  CShaderCache::Close
  ====================*/
void    CShaderCache::Close()
{
    if (!m_bInitialized)
        return;

    m_cArchiveRead.Close();

    vector<CShaderCacheNode *>::iterator it;
    vector<CShaderCacheNode *>::iterator itEnd(m_vShaderCacheNodes.end());
    for (it = m_vShaderCacheNodes.begin(); it != itEnd; ++it)
    {
        if ((*it)->NeedsWrite())
            break;
    }

    if (it != itEnd)
    {
        m_cArchiveWrite.Open(SHADERCACHE_ARCHIVE_FILENAME, ARCHIVE_WRITE | ARCHIVE_APPEND);

        // Try again without append if we failed
        if (!m_cArchiveWrite.IsOpen())
            m_cArchiveWrite.Open(SHADERCACHE_ARCHIVE_FILENAME, ARCHIVE_WRITE | ARCHIVE_TRUNCATE);
    }

    for (it = m_vShaderCacheNodes.begin(); it != itEnd; ++it)
        K2_DELETE(*it);

    m_vShaderCacheNodes.clear();

    m_cArchiveWrite.Close();

    m_bInitialized = false;
}


/*====================
  CShaderCache::RegisterNode
  ====================*/
uint    CShaderCache::RegisterNode(const tstring &sDirectory, const string &sDefines)
{
    m_vShaderCacheNodes.push_back(K2_NEW(ctx_D3D9,   CShaderCacheNode)(sDirectory, sDefines));

    return uint(m_vShaderCacheNodes.size() - 1);
}


/*====================
  CShaderCache::ActivateNode
  ====================*/
void    CShaderCache::ActivateNode(const string &sDefines)
{
    for (vector<CShaderCacheNode *>::iterator it(m_vShaderCacheNodes.begin()); it != m_vShaderCacheNodes.end(); ++it)
    {
        if ((*it)->GetDefines() == sDefines)
        {
            m_uiActiveNode = it - m_vShaderCacheNodes.begin();
            return;
        }
    }

    m_uiActiveNode = -1;
}


/*====================
  CShaderCache::LoadShader
  ====================*/
bool    CShaderCache::LoadShader(const tstring &sPath, IBuffer &cBuffer, uint &uiCRC32)
{
    if (!m_bInitialized || m_uiActiveNode == -1)
        return false;

    _stat stat;
    if (FileManager.Stat(sPath, stat))
        return m_vShaderCacheNodes[m_uiActiveNode]->LoadShader(TStringToWString(sPath), cBuffer, stat.st_mtime, uiCRC32);
    else
        return false;
}


/*====================
  CShaderCache::CacheShader
  ====================*/
void    CShaderCache::CacheShader(const tstring &sPath, const byte *pBuffer, uint uiSize, uint uiCRC32)
{
    if (!m_bInitialized)
        return;

    // Create a new shader cache node if we don't have one set
    if (m_uiActiveNode == -1)
    {
        uint uiNextNode(0);
        for (;;)
        {
            vector<CShaderCacheNode *>::iterator it(m_vShaderCacheNodes.begin());

            for (; it != m_vShaderCacheNodes.end(); ++it)
            {
                if (AtoI((*it)->GetDirectory()) == uiNextNode)
                {
                    ++uiNextNode;
                    break;
                }
            }

            if (it == m_vShaderCacheNodes.end())
                break;
        }

        const tstring &sDirectory(XtoA(uiNextNode, FMT_PADZERO, 8));

        m_uiActiveNode = RegisterNode(sDirectory, g_ShaderPreprocessor.GetDefinitionString());

        m_vShaderCacheNodes[m_uiActiveNode]->SetLoaded(true);

        WriteCacheDescriptor();
    }

    _stat stat;
    if (FileManager.Stat(sPath, stat))
        m_vShaderCacheNodes[m_uiActiveNode]->CacheShader(TStringToWString(sPath), pBuffer, uiSize, stat.st_mtime, uiCRC32);
}


/*====================
  CShaderCache::WriteCacheDescriptor
  ====================*/
void    CShaderCache::WriteCacheDescriptor()
{
    CXMLDoc xmlMaterial(XML_ENCODE_UTF8);

    xmlMaterial.NewNode("shadercache");

        xmlMaterial.AddProperty("version", m_sVersion);

        for (vector<CShaderCacheNode*>::iterator it(m_vShaderCacheNodes.begin()); it != m_vShaderCacheNodes.end(); ++it)
        {
            xmlMaterial.NewNode("node");
                xmlMaterial.AddProperty("directory", (*it)->GetDirectory());
                xmlMaterial.AddProperty("defines", StringToTString((*it)->GetDefines()));
            xmlMaterial.EndNode();
        }

    xmlMaterial.EndNode();

    xmlMaterial.WriteFile(SHADERCACHE_FILENAME);

    Console.Video << _T("Wrote shadercache descriptor") << newl;
}


/*====================
  CShaderCache::Clear
  ====================*/
void    CShaderCache::Clear()
{
    Close();

    FileManager.DeleteTree(_T("#/shadercache"));

    m_cArchiveRead.Open(SHADERCACHE_ARCHIVE_FILENAME, ARCHIVE_READ);

    m_bInitialized = true;
}


/*====================
  CShaderCache::Reload
  ====================*/
void    CShaderCache::Reload()
{
    Close();
    Initialize();
}


/*====================
  ShaderCacheClear
  ====================*/
CMD(ShaderCacheClear)
{
    g_ShaderCache.Clear();
    return true;
}


namespace XMLShaderCache
{
    // <shadercache>
    DECLARE_XML_PROCESSOR(shadercache)
    BEGIN_XML_REGISTRATION(shadercache)
        REGISTER_XML_PROCESSOR(root)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(shadercache, void)
        const tstring &sVersion(node.GetProperty(_T("version")));
        if (sVersion != g_ShaderCache.GetVersion())
        {
            FileManager.DeleteTree(_T("#/shadercache"));
            return true;
        }
    END_XML_PROCESSOR(pVoid)


    // <node>
    DECLARE_XML_PROCESSOR(node)
    BEGIN_XML_REGISTRATION(node)
        REGISTER_XML_PROCESSOR(shadercache)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(node, void)
        g_ShaderCache.RegisterNode(node.GetProperty(_T("directory")), TStringToString(node.GetProperty(_T("defines"))));
    END_XML_PROCESSOR_NO_CHILDREN
}
