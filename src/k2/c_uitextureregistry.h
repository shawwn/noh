// (C)2005 S2 Games
// c_uitextureregistry.h
//
//=============================================================================
#ifndef __C_UITEXTUREREGISTRY_H__
#define __C_UITEXTUREREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_filehttp.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
struct SWidgetImage
{
    int         iWidth;
    int         iHeight;
    ResHandle   hTexture;
};

typedef hash_map<tstring, SWidgetImage> ImageMap;

struct SDownloadedImage
{
    tstring     sURL;
    CFileHandle hFile;
    bool        bDownloading;
    bool        bFinished;
    tstring     sPath;
    ResHandle   hTexture;
    uint        uiRefCount;
};

typedef map<tstring, SDownloadedImage>  DownloadMap;
typedef deque<tstring>                  DownloadDeque;
//=============================================================================

//=============================================================================
// CUITextureRegistry
//=============================================================================
class CUITextureRegistry
{
    static CUITextureRegistry*          s_pInstance;
    static bool                         s_bReleased;

    ImageMap            m_mapTextures;

    CFileHTTP           m_fileHTTP;
    DownloadMap         m_mapDownloads;
    DownloadDeque       m_deqDownloads;
    uint                m_uiDownloadSequence;
    uint                m_uiNumDownloading;

    CUITextureRegistry() :
    m_uiDownloadSequence(0),
    m_uiNumDownloading(0)
    {
    }

    CUITextureRegistry (CUITextureRegistry&);
    CUITextureRegistry operator= (CUITextureRegistry&);
    ~CUITextureRegistry () {}

public:
    static  CUITextureRegistry *GetInstance();
    static  void        Release();

    bool                Exists(const tstring &sName);
    K2_API bool         Register(const tstring &sFilename, uint uiTextureFlags, ResHandle &hTexture);
    void                GetTexDimensions(const tstring &sFilename, int &iWidth, int &iHeight);
    ResHandle           GetResHandle(const tstring &sFilename);
    K2_API bool         TextureExists(const tstring &sFilename, uint uiTextureFlags);

    K2_API void         Frame();

    K2_API void         StartDownload(const tstring &sURL);
    K2_API bool         IsDownloaded(const tstring &sURL);
    K2_API ResHandle    GetDownloadedTexture(const tstring &sURL);
    K2_API void         ReleaseDownloadedTexture(const tstring &sURL);

    K2_API void         PrintDownloads();
};
extern K2_API CUITextureRegistry *pUITextureRegistry;
#define UITextureRegistry (*pUITextureRegistry)
#ifdef UI_EXPORTS
#define pUITextureRegistry CUITextureRegistry::GetInstance()
#endif
//=============================================================================

#endif // __C_UITEXTUREREGISTRY_H__
