// (C)2005 S2 Games
// c_uitextureregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uitextureregistry.h"
#include "c_resourcemanager.h"

#include "../k2/c_bitmap.h"
#include "../k2/c_filemanager.h"
#include "../k2/c_texture.h"
#include "../k2/c_vid.h"
//=============================================================================

#undef pUITextureRegistry

//=============================================================================
// Globals
//=============================================================================
CUITextureRegistry* CUITextureRegistry::s_pInstance;
bool                CUITextureRegistry::s_bReleased(false);

CUITextureRegistry  *pUITextureRegistry(CUITextureRegistry::GetInstance());

CVAR_UINT(ui_textureMaxConcurrentDownloads, 4);
//=============================================================================

/*====================
  CUITextureRegistry::GetInstance()
  ====================*/
CUITextureRegistry *CUITextureRegistry::GetInstance()
{
    if (!s_pInstance)
        s_pInstance = K2_NEW(ctx_Singleton,  CUITextureRegistry);

    return s_pInstance;
}


/*====================
  CUITextureRegistry::Exists
  ====================*/
bool    CUITextureRegistry::Exists(const tstring &sFilename)
{
    if (sFilename.empty())
        return true;

    ImageMap::const_iterator find(m_mapTextures.find(LowerString(FileManager.SanitizePath(sFilename))));

    if (find == m_mapTextures.end())
        return false;

    return true;
}


/*====================
  CUITextureRegistry::Register
  ====================*/
bool    CUITextureRegistry::Register(const tstring &sFilename, uint uiTextureFlags, ResHandle &hTexture)
{
    PROFILE("CUITextureRegistry::Register");

    if (sFilename.empty())
    {
        hTexture = g_ResourceManager.GetWhiteTexture();
        return true;
    }

    tstring sFullFilename(LowerString(FileManager.SanitizePath(sFilename)));

    // Look up the entry
    ImageMap::iterator itFind(m_mapTextures.find(sFullFilename));
    if (itFind == m_mapTextures.end())
    {
        // If the entry doesn't exist, create one
        itFind = m_mapTextures.insert(make_pair(sFullFilename, SWidgetImage())).first;
        itFind->second.hTexture = INVALID_RESOURCE;
    }

    // Sanity check: we must have created an entry.
    assert(itFind != m_mapTextures.end());
    if (itFind == m_mapTextures.end())
    {
        hTexture = g_ResourceManager.GetWhiteTexture();
        Console.Err << _T("CUITextureRegistry::Register(") << sFilename << _T(") - Programmer error") << newl;
        return false;
    }

    // Always register the texture, to add it to the current resource scope.
    SWidgetImage& widget(itFind->second);
    widget.hTexture = g_ResourceManager.Register(K2_NEW(ctx_Widgets,  CTexture)(sFilename, TEXTURE_2D, TEX_FULL_QUALITY | uiTextureFlags, TEXFMT_A8R8G8B8), RES_TEXTURE);

    if (widget.hTexture == INVALID_RESOURCE)
        Console.Err << _T("Loading texture ") << sFilename << _T(" failed") << newl;

    int iWidth(0), iHeight(0);
#if 0
    // TODO: get the width and height of the texture from the renderer.
    CTexture* pTexture(g_ResourceManager.GetTexture(pWidget.hTexture));
    if (pTexture != nullptr)
    {
        Vid.GetTextureInfo(pTexture->GetIndex(), nullptr, &iWidth, &iHeight, nullptr);
    }
#endif
    widget.iWidth = iWidth;
    widget.iHeight = iHeight;

    hTexture = widget.hTexture;

    // Use a white texture if the resource failed to load.
    CTexture* pTexture(g_ResourceManager.GetTexture(widget.hTexture));
    if (pTexture == nullptr || pTexture->HasFlags(RES_LOAD_FAILED))
        hTexture = g_ResourceManager.GetWhiteTexture();

    return true;
}


/*====================
  CUITextureRegistry::TextureExists
  ====================*/
bool    CUITextureRegistry::TextureExists(const tstring &sFilename, uint uiTextureFlags)
{
    return Vid.TextureExists(sFilename, TEX_FULL_QUALITY | uiTextureFlags);
}


/*====================
  CUITextureRegistry::GetResHandle
  ====================*/
ResHandle   CUITextureRegistry::GetResHandle(const tstring &sFilename)
{
    ImageMap::const_iterator find(m_mapTextures.find(FileManager.SanitizePath(sFilename)));

    if (find == m_mapTextures.end())
        return g_ResourceManager.GetWhiteTexture();
    else
        return find->second.hTexture;
}


/*====================
  CUITextureRegistry::GetTexDimensions
  ====================*/
void    CUITextureRegistry::GetTexDimensions(const tstring &sFilename, int &iWidth, int &iHeight)
{
    ImageMap::const_iterator findit(m_mapTextures.find(sFilename));

    if (findit == m_mapTextures.end())
        return;

    iWidth = findit->second.iWidth;
    iHeight = findit->second.iHeight;
}


/*====================
  CUITextureRegistry::Frame
  ====================*/
void    CUITextureRegistry::Frame()
{
    // Update downloads
    for (DownloadMap::iterator it(m_mapDownloads.begin());
        it != m_mapDownloads.end() && m_uiNumDownloading > 0;
        ++it)
    {
        SDownloadedImage &cDownload(it->second);

        if (!cDownload.bDownloading)
            continue;

        m_fileHTTP.SetFileTarget(it->second.hFile.GetFile());
        m_fileHTTP.Open(it->first, FILE_HTTP_WRITETOFILE);

        if (m_fileHTTP.IsOpen())
        {
            if (!m_fileHTTP.ErrorEncountered())
                Console.Net << _T("[UI] Finished download on ") << it->first << newl;
            else
                Console.Net << _T("[UI] Error downloading ") << it->first << newl;
            
            it->second.bDownloading = false;
            it->second.bFinished = true;
            it->second.hFile.Close();

            --m_uiNumDownloading;
        }
    }

    // Find new downloads to start
    while (!m_deqDownloads.empty() && m_uiNumDownloading < ui_textureMaxConcurrentDownloads)
    {
        const tstring &sURL(m_deqDownloads.front());

        SDownloadedImage &cDownload(m_mapDownloads[sURL]);

        cDownload.sURL = sURL;
        cDownload.bFinished = false;
        cDownload.bDownloading = false;
        cDownload.hTexture = INVALID_RESOURCE;
        cDownload.uiRefCount = 0;
        cDownload.sPath = _T("~/webcache/") + XtoA(m_uiDownloadSequence, FMT_PADZERO | FMT_NOPREFIX, 8, 16);

        ++m_uiDownloadSequence;

        cDownload.hFile.Open(cDownload.sPath, FILE_WRITE | FILE_BINARY);

        if (!cDownload.hFile.IsOpen())
        {
            m_deqDownloads.pop_front();
            continue;
        }

        m_fileHTTP.SetFileTarget(cDownload.hFile.GetFile());
        m_fileHTTP.Open(sURL, FILE_HTTP_WRITETOFILE);

        cDownload.bDownloading = true;

        ++m_uiNumDownloading;
        m_deqDownloads.pop_front();
    }
}


/*====================
  CUITextureRegistry::StartDownload
  ====================*/
void    CUITextureRegistry::StartDownload(const tstring &sURL)
{
    DownloadMap::iterator itFind(m_mapDownloads.find(sURL));
    if (itFind != m_mapDownloads.end())
        return;

    for (DownloadDeque::iterator it(m_deqDownloads.begin()); it != m_deqDownloads.end(); ++it)
    {
        if (*it == sURL)
            return;
    }

    m_deqDownloads.push_back(sURL);
}


/*====================
  CUITextureRegistry::IsDownloaded
  ====================*/
bool    CUITextureRegistry::IsDownloaded(const tstring &sURL)
{
    DownloadMap::iterator itFind(m_mapDownloads.find(sURL));
    if (itFind == m_mapDownloads.end())
        return false;
    
    if (itFind->second.bFinished)
        return true;

    return false;
}


/*====================
  CUITextureRegistry::GetDownloadedTexture
  ====================*/
ResHandle   CUITextureRegistry::GetDownloadedTexture(const tstring &sURL)
{
    DownloadMap::iterator itFind(m_mapDownloads.find(sURL));
    if (itFind == m_mapDownloads.end() || !itFind->second.bFinished)
        return g_ResourceManager.GetWhiteTexture();

    if (itFind->second.hTexture == INVALID_RESOURCE)
    {
        CBitmap cBitmap;
        cBitmap.LoadAuto(itFind->second.sPath);

        itFind->second.hTexture = g_ResourceManager.Register(K2_NEW(ctx_Resources,  CTexture)(_T("*") + sURL, &cBitmap, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

        if (itFind->second.hTexture == INVALID_RESOURCE)
            itFind->second.hTexture = g_ResourceManager.GetCheckerTexture();
    }

    ++itFind->second.uiRefCount;

    return itFind->second.hTexture;
}


/*====================
  CUITextureRegistry::ReleaseDownloadedTexture
  ====================*/
void    CUITextureRegistry::ReleaseDownloadedTexture(const tstring &sURL)
{
    DownloadMap::iterator itFind(m_mapDownloads.find(sURL));
    if (itFind == m_mapDownloads.end() || !itFind->second.bFinished)
    {
        Console.Warn << _T("Released non-existent texture") << newl;
        return;
    }

    if (itFind->second.uiRefCount > 1)
    {
        --itFind->second.uiRefCount;
        return;
    }

    Console.Dev << _T("Releasing ") << sURL << newl;

    g_ResourceManager.Unregister(itFind->second.hTexture, UNREG_DELETE_HANDLE);

    itFind->second.hTexture = INVALID_RESOURCE;
    itFind->second.uiRefCount = 0;
}


/*====================
  CUITextureRegistry::PrintDownloads
  ====================*/
void    CUITextureRegistry::PrintDownloads()
{
    for (DownloadMap::iterator it(m_mapDownloads.begin()); it != m_mapDownloads.end(); ++it)
    {
        Console << it->second.sURL << _T(" ") << it->second.uiRefCount << _T(" ") << newl;
    }
}


CMD(PrintTextureDownloads)
{
    UITextureRegistry.PrintDownloads();
    return true;
}