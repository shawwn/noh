// (C)2008 S2 Games
// c_texturecache.h
//
//=============================================================================
#ifndef __C_TEXTURECACHE_H__
#define __C_TEXTURECACHE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_buffer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const TCHAR* const TEXTURECACHE_FILENAME(_T("#/texturecache/texturecache.xml"));

class CTextureCacheNode
{
private:
    CFileHandle     m_hFile;
    CBufferDynamic  m_cBuffer;
    tstring         m_sDirectory;
    string          m_sDefines;

    void    Load();

public:
    ~CTextureCacheNode();
    CTextureCacheNode()     {}

    CTextureCacheNode(const tstring &sDirectory, const string &sDefines);

    const tstring&  GetDirectory() const        { return m_sDirectory; }
    const string&   GetDefines() const          { return m_sDefines; }

    void    CacheTexture(const tstring &sPath, uint uiTextureID, time_t tModTime);
    bool    LoadTexture(const tstring &sPath, time_t tModTime, tstring &sFilePath);
};
//=============================================================================

//=============================================================================
// CTextureCache
//=============================================================================
class CTextureCache
{
private:
    bool        m_bInitialized;
    tstring     m_sVersion;

    uint        m_uiActiveNode;

    vector<CTextureCacheNode *>     m_vTextureCacheNodes;

    void    WriteCacheDescriptor();

public:
    ~CTextureCache();
    CTextureCache();

    void    Initialize();
    void    Close();

    void            SetVersion(const tstring &sVersion)     { m_sVersion = sVersion; }
    const tstring&  GetVersion() const                      { return m_sVersion; }
    
    uint    RegisterNode(const tstring &sDirectory, const string &sDefines);
    void    ActivateNode(const string &sDefines);

    bool    LoadTexture(const tstring &sPath, const tstring &sReference, tstring &sFilePath);
    void    CacheTexture(const tstring &sPath, uint uiTextureID);
    void    CacheTexture(const tstring &sPath, uint uiTextureID, const tstring &sReference);

    void    Clear();
    void    Reload();
};

extern  CTextureCache       g_TextureCache;
//=============================================================================

#endif //__C_TEXTURECACHE_H__
