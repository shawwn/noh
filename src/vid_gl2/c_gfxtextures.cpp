// (C)2008 S2 Games
// c_gfxtextures.cpp
//
// Textures
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfxtextures.h"

#include "c_gfxshaders.h"
#include "c_procedural.h"
#include "c_proceduralregistry.h"
#include "c_texturecache.h"
#include "c_texturearchive.h"

#include "../k2/i_resourcelibrary.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CGfxTextures)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGfxTextures *GfxTextures(CGfxTextures::GetInstance());

vector<CTextureArchive*>    g_vTextureArchives;
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_INTF   (vid_textureDownsize,       0,          CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_textureCompression,    true,       CVAR_SAVECONFIG);
CVAR_INTF   (vid_textureMaxSize,        4096,       CVAR_SAVECONFIG);
CVAR_BOOL   (vid_textureCache,          true);
CVAR_FLOAT  (vid_textureGammaCorrect,   2.2f);
//=============================================================================

/*====================
  CGfxTextures::~CGfxTextures
  ====================*/
CGfxTextures::~CGfxTextures()
{
    mapTextures.clear();
}


/*====================
  CGfxTextures::CGfxTextures
  ====================*/
CGfxTextures::CGfxTextures() : 
m_iWhite(0),
m_iBlack(0)
{
    mapTextures.clear();
}


/*====================
  CGfxTextures::SetDefaultTexParameters2D
  ====================*/
void    CGfxTextures::SetDefaultTexParameters2D(int iTextureFlags)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (!(iTextureFlags & TEX_NO_MIPMAPS))
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureMinFilterMipmap);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureMinFilter);
    }

    if (GLEW_EXT_texture_filter_anisotropic)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);

    PRINT_GLERROR_BREAK();
}


/*====================
  CGfxTextures::SetDefaultTexParametersCube
  ====================*/
void    CGfxTextures::SetDefaultTexParametersCube(int iTextureFlags)
{
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (~iTextureFlags & TEX_NO_MIPMAPS)
    {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, g_textureMinFilterMipmap);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, g_textureMinFilter);
    }

    if (GLEW_EXT_texture_filter_anisotropic)
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);

    PRINT_GLERROR_BREAK();
}


/*====================
  CGfxTextures::SetDefaultTexParametersVolume
  ====================*/
void    CGfxTextures::SetDefaultTexParametersVolume(int iTextureFlags)
{
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    if (!(iTextureFlags & TEX_NO_MIPMAPS))
    {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, g_textureMinFilterMipmap);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, g_textureMinFilter);
    }

    if (GLEW_EXT_texture_filter_anisotropic)
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);

    PRINT_GLERROR_BREAK();
}


/*====================
  CGfxTextures::UploadBitmap2D
  ====================*/
void    CGfxTextures::UploadBitmap2D(const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat, GLuint texLevel, bool genMipmaps)
{
    const CBitmap *pImage(nullptr);
    CBitmap bmpResized;

    if (ResizeBitmap(bitmap, bmpResized, iTextureFlags))
        pImage = &bmpResized;
    else
        pImage = &bitmap;

    GLint internalFormat;
    GLenum format;
    GLenum type;
    CBitmap bmpSwizzled;

    if (eFormat == TEXFMT_NORMALMAP_RXGB)
    {
        bmpSwizzled.Alloc(pImage->GetWidth(), pImage->GetHeight(), BITMAP_RGBA);

        uint uiNumPixels(pImage->GetWidth() * pImage->GetHeight());
        const byte *pSrc(pImage->GetBuffer());
        byte *pDst(bmpSwizzled.GetBuffer());

        uint uiSrcSize(pImage->GetBMPType());
        uint uiDstSize(bmpSwizzled.GetBMPType());

        for (uint ui(0); ui < uiNumPixels; ++ui, pSrc += uiSrcSize, pDst += uiDstSize)
        {
            pDst[R] = 255;
            pDst[G] = pSrc[G];
            pDst[B] = pSrc[B];
            pDst[A] = pSrc[R];
        }

        internalFormat = GL_RGBA8;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        pImage = &bmpSwizzled;
    }
    else if (eFormat == TEXFMT_NORMALMAP_S)
    {
        bmpSwizzled.Alloc(pImage->GetWidth(), pImage->GetHeight(), BITMAP_RGBA);

        uint uiNumPixels(pImage->GetWidth() * pImage->GetHeight());
        const byte *pSrc(pImage->GetBuffer());
        byte *pDst(bmpSwizzled.GetBuffer());

        uint uiSrcSize(pImage->GetBMPType());
        uint uiDstSize(bmpSwizzled.GetBMPType());

        for (uint ui(0); ui < uiNumPixels; ++ui, pSrc += uiSrcSize, pDst += uiDstSize)
        {
            pDst[R] = 255;
            pDst[G] = pSrc[A];
            pDst[B] = 255;
            pDst[A] = 255;
        }

        internalFormat = GL_RGBA8;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        pImage = &bmpSwizzled;
    }
    else
    {
        GetBitmapInfo(*pImage, eFormat, internalFormat, format, type);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    PRINT_GLERROR_BREAK();

    SetDefaultTexParameters2D(iTextureFlags);

    if (genMipmaps && !(iTextureFlags & TEX_NO_MIPMAPS))
        gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, pImage->GetWidth(), pImage->GetHeight(), format, type, pImage->GetBuffer());
    else
        glTexImage2D(GL_TEXTURE_2D, texLevel, internalFormat, pImage->GetWidth(), pImage->GetHeight(), 0, format, type, pImage->GetBuffer());

    PRINT_GLERROR_BREAK();
}


/*====================
  CGfxTextures::UploadBitmapCube
  ====================*/
void    CGfxTextures::UploadBitmapCube(const CBitmap bitmap[], int iTextureFlags, ETextureFormat eFormat, GLuint texLevel)
{
    const CBitmap *pImage[6];
    CBitmap bmpResized[6];

    for (int i(0); i < 6; ++i)
    {
        if (ResizeBitmap(bitmap[i], bmpResized[i], iTextureFlags))
            pImage[i] = &bmpResized[i];
        else
            pImage[i] = &bitmap[i];
    }

    GLint internalFormat;
    GLenum format;
    GLenum type;
    GetBitmapInfo(*pImage[0], eFormat, internalFormat, format, type);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    PRINT_GLERROR_BREAK();

    SetDefaultTexParametersCube(iTextureFlags);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, ~iTextureFlags & TEX_NO_MIPMAPS ? GL_TRUE : GL_FALSE);

    for (int i(0); i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texLevel, internalFormat, pImage[i]->GetWidth(), pImage[i]->GetHeight(), 0, format, type, pImage[i]->GetBuffer());

    PRINT_GLERROR_BREAK();
}


/*====================
  CGfxTextures::UploadBitmapVolume
  ====================*/
void    CGfxTextures::UploadBitmapVolume(const CBitmap bitmap[], int iDepth, int iTextureFlags, ETextureFormat eFormat, GLuint texLevel)
{
    const CBitmap **pImage = K2_NEW_ARRAY(ctx_GL2, const CBitmap *, iDepth);
    CBitmap *bmpResized = K2_NEW_ARRAY(ctx_GL2, CBitmap, iDepth);

    for (int i(0); i < iDepth; ++i)
    {
        if (ResizeBitmap(bitmap[i], bmpResized[i], iTextureFlags))
            pImage[i] = &bmpResized[i];
        else
            pImage[i] = &bitmap[i];
    }

    GLint internalFormat;
    GLenum format;
    GLenum type;
    GetBitmapInfo(*pImage[0], eFormat, internalFormat, format, type);

    PRINT_GLERROR_BREAK();

    SetDefaultTexParametersVolume(iTextureFlags);

    byte *pBuffer = K2_NEW_ARRAY(ctx_GL2, byte , pImage[0]->GetSize() * iDepth);
    for (int i(0); i < iDepth; ++i)
        MemManager.Copy(pBuffer + pImage[0]->GetSize() * i, pImage[i]->GetBuffer(), pImage[i]->GetSize());

#if 0
    glTexParameteri(GL_TEXTURE_3D, GL_GENERATE_MIPMAP, !(iTextureFlags & TEX_NO_MIPMAPS) ? GL_TRUE : GL_FALSE);

    glTexImage3D(GL_TEXTURE_3D, texLevel, internalFormat, pImage[0]->GetWidth(), pImage[0]->GetHeight(), iDepth, 0, format, type, pBuffer);
#else
    glTexImage3D(GL_TEXTURE_3D, texLevel, internalFormat, pImage[0]->GetWidth(), pImage[0]->GetHeight(), iDepth, 0, format, type, pBuffer);

    if (!(iTextureFlags & TEX_NO_MIPMAPS))
        glGenerateMipmapEXT(GL_TEXTURE_3D);
#endif

    PRINT_GLERROR_BREAK();

    K2_DELETE_ARRAY(pImage);
    K2_DELETE_ARRAY(bmpResized);
    K2_DELETE_ARRAY(pBuffer);
}


/*====================
  CGfxTextures::RegisterTexture
  ====================*/
int     CGfxTextures::RegisterTexture(CTexture *pTexture)
{
    GLuint uiTextureID(0);

    // See if this texture is already in memory
    TextureMap::iterator findit = mapTextures.find(pTexture->GetPath());
    if (findit != mapTextures.end())
    {
        // Yep, so just use the instance that's already in memory
        pTexture->SetIndex(findit->second.uiTextureID);
        pTexture->SetIndex2(findit->second.uiTextureID2);
        return 1;
    }

    // Nope, then load it from disk and put it into vid memory
    switch (pTexture->GetType())
    {
    default:
    case TEXTURE_2D:
        if (pTexture->HasBitmap())
        {
            uiTextureID = Register2DTexture(*pTexture->GetBitmap(), pTexture->GetTextureFlags(), pTexture->GetFormat(), pTexture->GetPath());
        }
        else
        {
            if (pTexture->GetFormat() == TEXFMT_NORMALMAP && vid_shaderRXGBNormalmap)
            {
                if ((uiTextureID = RegisterArchivedTexture(pTexture, _T("_rxgb"))) != 0)
                {
                }
                else if (vid_textureCache && (uiTextureID = RegisterCachedTexture(pTexture, pTexture->GetPath(), _T("_rxgb"))) != 0)
                {
                }
                else
                {
                    CBitmap bitmap;
                    if (!bitmap.Load(pTexture->GetPath()))
                        EX_WARN(_T("not found or bad texture"));

                    uiTextureID = Register2DTexture(bitmap, pTexture->GetTextureFlags(), TEXFMT_NORMALMAP_RXGB, pTexture->GetPath());

                    if (vid_textureCache)
                        g_TextureCache.CacheTexture(Filename_AppendSuffix(pTexture->GetPath(), _T("_rxgb")), uiTextureID, pTexture->GetPath());
                }

                uint uiTextureID2(0);

                if ((uiTextureID2 = RegisterArchivedTexture(pTexture, _T("_s"))) != 0)
                {
                }
                else if (vid_textureCache && (uiTextureID2 = RegisterCachedTexture(pTexture, pTexture->GetPath(), _T("_s"))) != 0)
                {
                }
                else
                {
                    CBitmap bitmap;
                    if (bitmap.Load(pTexture->GetPath()) && bitmap.GetBMPType() == BITMAP_RGBA)
                    {
                        uiTextureID2 = Register2DTexture(bitmap, pTexture->GetTextureFlags(), TEXFMT_NORMALMAP_S, pTexture->GetPath());

                        if (vid_textureCache)
                            g_TextureCache.CacheTexture(Filename_AppendSuffix(pTexture->GetPath(), _T("_s")), uiTextureID2, pTexture->GetPath());
                    }
                    else
                    {
                        uiTextureID2 = m_iBlack;
                    }
                }

                pTexture->SetIndex2(uiTextureID2);
            }
            else
            {
                if ((uiTextureID = RegisterArchivedTexture(pTexture)) != 0)
                {
                    // TODO: Profiling information
                }
                else if (vid_textureCache && (uiTextureID = RegisterCachedTexture(pTexture)) != 0)
                {
                    // TODO: Profiling information
                }
                else
                {
                    CBitmap bitmap;
                    if (!bitmap.Load(pTexture->GetPath()))
                        EX_WARN(_T("not found or bad texture"));

                    uiTextureID = Register2DTexture(bitmap, pTexture->GetTextureFlags(), pTexture->GetFormat(), pTexture->GetPath());

                    if (vid_textureCache)
                        g_TextureCache.CacheTexture(pTexture->GetPath(), uiTextureID);
                }
            }
        }
        break;

    case TEXTURE_CUBE:
        {
            if ((uiTextureID = RegisterArchivedTexture(pTexture)) != 0)
            {
                // TODO: Profiling information
            }
            else if (vid_textureCache && (uiTextureID = RegisterCachedTexture(pTexture, Filename_AppendSuffix(pTexture->GetPath(), _T("_posx")), TSNULL)) != 0)
            {
                // TODO: Profiling information
            }
            else
            {
                CBitmap bitmaps[6];

                if (!bitmaps[0].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_posx"))) ||
                    !bitmaps[1].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_negx"))) ||
                    !bitmaps[2].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_posy"))) ||
                    !bitmaps[3].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_negy"))) ||
                    !bitmaps[4].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_posz"))) ||
                    !bitmaps[5].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_negz"))))
                {
                    EX_WARN(_T("not found or bad cubemap"));
                }

                uiTextureID = RegisterCubeTexture(bitmaps, pTexture->GetTextureFlags(), pTexture->GetFormat(), pTexture->GetPath());

                if (vid_textureCache)
                    g_TextureCache.CacheTexture(pTexture->GetPath(), uiTextureID, Filename_AppendSuffix(pTexture->GetPath(), _T("_posx")));
            }
        }
        break;

    case TEXTURE_VOLUME:
        {
            if ((uiTextureID = RegisterArchivedTexture(pTexture)) != 0)
            {
                // TODO: Profiling information
            }
            else if (vid_textureCache && (uiTextureID = RegisterCachedTexture(pTexture, Filename_AppendSuffix(pTexture->GetPath(), XtoA(0, FMT_PADZERO, 4)), TSNULL)) != 0)
            {
                // TODO: Profiling information
            }
            else
            {
                int iDepth(0);

                for (int iZ(0); iZ < vid_textureMaxSize; ++iZ)
                {
                    if (!FileManager.Exists(Filename_AppendSuffix(pTexture->GetPath(), XtoA(iZ, FMT_PADZERO, 4))))
                        break;

                    ++iDepth;
                }

                iDepth = M_FloorPow2(iDepth + 1);

                CBitmap *bitmaps(K2_NEW_ARRAY(ctx_GL2, CBitmap, iDepth));

                // Load bitmaps
                for (int iZ(0); iZ < iDepth; ++iZ)
                {
                    if (!bitmaps[iZ].Load(Filename_AppendSuffix(pTexture->GetPath(), XtoA(iZ, FMT_PADZERO, 4))))
                    {
                        K2_DELETE_ARRAY(bitmaps);
                        EX_WARN(_T("not found or bad volume texture"));
                    }
                }

                uiTextureID = RegisterVolumeTexture(bitmaps, iDepth, pTexture->GetTextureFlags(), pTexture->GetFormat(), pTexture->GetPath());
                K2_DELETE_ARRAY(bitmaps);

                if (vid_textureCache)
                    g_TextureCache.CacheTexture(pTexture->GetPath(), uiTextureID, Filename_AppendSuffix(pTexture->GetPath(), XtoA(0, FMT_PADZERO, 4)));
            }
        }
        break;
    }

    pTexture->SetIndex(uiTextureID);

    mapTextures[pTexture->GetPath()] = STextureMapEntry(pTexture->GetIndex(), pTexture->GetIndex2(), pTexture->GetHandle());
    return 0;
}


/*====================
  CGfxTextures::UnregisterTexture
  ====================*/
void    CGfxTextures::UnregisterTexture(const tstring &sName)
{
    TextureMap::iterator itFind(mapTextures.find(sName));
    if (itFind != mapTextures.end())
    {
        GLuint uiTextureID(itFind->second.uiTextureID);
        GL_SAFE_DELETE(glDeleteTextures, uiTextureID);

        GLuint uiTextureID2(itFind->second.uiTextureID2);
        if (uiTextureID2 != m_iWhite && uiTextureID2 != m_iBlack)
            GL_SAFE_DELETE(glDeleteTextures, uiTextureID2);

        mapTextures.erase(itFind);
    }
}


/*====================
  CGfxTextures::Update2DTexture
  ====================*/
void    CGfxTextures::Update2DTexture(const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat, uint texID)
{
    glBindTexture(GL_TEXTURE_2D, texID);
    UploadBitmap2D(bitmap, iTextureFlags, eFormat, 0, true);
}


/*====================
  CGfxTextures::Register2DTexture
  ====================*/
uint    CGfxTextures::Register2DTexture(const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat, const tstring &sName)
{
    GLuint uiTextureID(RegisterTextureID());

    glPushAttrib(GL_ENABLE_BIT);

    glActiveTextureARB(GL_TEXTURE0_ARB);

    glBindTexture(GL_TEXTURE_2D, uiTextureID);

    PRINT_GLERROR_BREAK();

    UploadBitmap2D(bitmap, iTextureFlags, eFormat, 0, true);

    glPopAttrib();

    return uiTextureID;
}


/*====================
  CGfxTextures::RegisterCubeTexture
  ====================*/
uint    CGfxTextures::RegisterCubeTexture(const CBitmap bitmaps[], int iTextureFlags, ETextureFormat eFormat, const tstring &sName)
{
    GLuint uiTextureID(RegisterTextureID());

    glPushAttrib(GL_ENABLE_BIT);

    glActiveTextureARB(GL_TEXTURE0_ARB);

    glBindTexture(GL_TEXTURE_CUBE_MAP, uiTextureID);

    PRINT_GLERROR_BREAK();

    UploadBitmapCube(bitmaps, iTextureFlags, eFormat, 0);

    glPopAttrib();

    return uiTextureID;
}


/*====================
  CGfxTextures::RegisterVolumeTexture
  ====================*/
uint    CGfxTextures::RegisterVolumeTexture(const CBitmap bitmaps[], int iDepth, int iTextureFlags, ETextureFormat eFormat, const tstring &sName)
{
    GLuint uiTextureID(RegisterTextureID());

    glPushAttrib(GL_ENABLE_BIT);

    glActiveTextureARB(GL_TEXTURE0_ARB);

    glBindTexture(GL_TEXTURE_3D, uiTextureID);

    PRINT_GLERROR_BREAK();

    UploadBitmapVolume(bitmaps, iDepth, iTextureFlags, eFormat, 0);

    glPopAttrib();

    return uiTextureID;
}


/*====================
  CGfxTextures::ArchivedTextureExists
  ====================*/
bool    CGfxTextures::ArchivedTextureExists(const tstring &sFilename, uint uiTextureFlags)
{
    try
    {
        if (uiTextureFlags & TEX_FULL_QUALITY)
            TextureDefine("DOWNSIZE", "0");
        else
            TextureDefine("DOWNSIZE", TStringToString(XtoA(vid_textureDownsize)));

        if (uiTextureFlags & TEX_NO_COMPRESS)
            TextureDefine("COMPRESSION", "0");
        else
            TextureDefine("COMPRESSION", vid_textureCompression ? "1" : "0");

        if (uiTextureFlags & TEX_NO_MIPMAPS)
            TextureDefine("MIPMAPS", "0");
        else
            TextureDefine("MIPMAPS", "1");

        tstring sCleanFilename(FileManager.SanitizePath(sFilename));

        for (vector<CTextureArchive*>::iterator it(g_vTextureArchives.begin()); it != g_vTextureArchives.end(); ++it)
        {
            (*it)->ActivateNode(GetTextureDefinitionString());

            if ((*it)->TextureExists(sCleanFilename))
                return true;
        }

        return false;
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CGfxTextures::ArchivedTextureExists(") + sFilename + _TS(") - "), NO_THROW);
        return false;
    }
}


/*====================
  CGfxTextures::RegisterArchivedTexture
  ====================*/
uint    CGfxTextures::RegisterArchivedTexture(CTexture *pTexture, const tstring &sSuffix)
{
    try
    {
        int iTextureFlags(pTexture->GetTextureFlags());

        if (pTexture->GetType() == TEXTURE_VOLUME)
            iTextureFlags |= TEX_NO_COMPRESS;

        if (iTextureFlags & TEX_FULL_QUALITY)
            TextureDefine("DOWNSIZE", "0");
        else
            TextureDefine("DOWNSIZE", TStringToString(XtoA(vid_textureDownsize)));

        if (iTextureFlags & TEX_NO_COMPRESS)
            TextureDefine("COMPRESSION", "0");
        else
            TextureDefine("COMPRESSION", /*vid_textureCompression ? "1" : "0"*/"1");

        if (iTextureFlags & TEX_NO_MIPMAPS)
            TextureDefine("MIPMAPS", "0");
        else
            TextureDefine("MIPMAPS", "1");

        const tstring &sPath(!sSuffix.empty() ? Filename_AppendSuffix(pTexture->GetPath(), sSuffix) : pTexture->GetPath());

        CFileHandle hTexture;
        for (vector<CTextureArchive*>::iterator it(g_vTextureArchives.begin()); it != g_vTextureArchives.end(); ++it)
        {
            (*it)->ActivateNode(GetTextureDefinitionString());

            (*it)->LoadTexture(sPath, hTexture);
            if (hTexture.IsOpen())
                break;
        }

        if (!hTexture.IsOpen())
            return 0;

        uint uiSize(0);
        const char *pBuffer(hTexture.GetBuffer(uiSize));
        uint uiTextureID(0);

        if (pTexture->GetType() == TEXTURE_2D)
        {
            int iFlags(SOIL_FLAG_DDS_LOAD_DIRECT);
            if ((!g_DeviceCaps.bTextureCompression || !vid_textureCompression) && !(iTextureFlags & TEX_NO_COMPRESS))
                iFlags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_DECOMPRESS;
            uiTextureID = SOIL_load_OGL_texture_from_memory
            (
                (byte *)pBuffer,
                (int)uiSize,
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                iFlags
            );

            if (uiTextureID == 0)
                EX_WARN(_T("TEXTURE_2D load failed"));

            glBindTexture(GL_TEXTURE_2D, uiTextureID);
            SetDefaultTexParameters2D(iTextureFlags);
        }
        else if (pTexture->GetType() == TEXTURE_CUBE)
        {
            int iFlags(SOIL_FLAG_DDS_LOAD_DIRECT);
            if ((!g_DeviceCaps.bTextureCompression || !vid_textureCompression) && !(iTextureFlags & TEX_NO_COMPRESS))
                iFlags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_DECOMPRESS;
            uiTextureID = SOIL_load_OGL_single_cubemap_from_memory
            (
                (byte *)pBuffer,
                (int)uiSize,
                SOIL_DDS_CUBEMAP_FACE_ORDER,
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                iFlags
            );

            if (uiTextureID == 0)
                EX_WARN(_T("TEXTURE_CUBE load failed"));

            glBindTexture(GL_TEXTURE_CUBE_MAP, uiTextureID);
            SetDefaultTexParametersCube(iTextureFlags);
        }
        else if (pTexture->GetType() == TEXTURE_VOLUME)
        {
            int iFlags(SOIL_FLAG_DDS_LOAD_DIRECT);
            if ((!g_DeviceCaps.bTextureCompression || !vid_textureCompression) && !(iTextureFlags & TEX_NO_COMPRESS))
                iFlags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_DECOMPRESS;
            uiTextureID = SOIL_load_OGL_single_texture3D_from_memory
            (
                (byte *)pBuffer,
                (int)uiSize,
                0,
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                iFlags
            );

            if (uiTextureID == 0)
                EX_WARN(_T("TEXTURE_VOLUME load failed"));

            glBindTexture(GL_TEXTURE_3D, uiTextureID);
            SetDefaultTexParametersVolume(iTextureFlags);
        }

        if (uiTextureID == 0)
            EX_WARN(_T("Texture load failed"));
        
        hTexture.Close();

        return uiTextureID;
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CGfxTextures::RegisterArchivedTexture(") + pTexture->GetPath() + _TS(") - "), NO_THROW);
        return 0;
    }
}


/*====================
  GL_GetTextureList
  ====================*/
void    GL_GetTextureList(const tstring &sPath, const tstring &sSearch, tsvector &vResult)
{
#if 0
    for (vector<CArchive*>::iterator it(g_vTextureArchives.begin()); it != g_vTextureArchives.end(); ++it)
    {
        tsvector vFiles;
        (*it)->GetFileList(vFiles);
        for (tsvector_it itFiles(vFiles.begin()); itFiles != vFiles.end(); ++itFiles)
        {
            if (itFiles->find(sSearch) != tstring::npos)
                vResult.push_back(*itFiles);
        }
    }

    for (tsvector_it it(g_vTexturePaths.begin()); it != g_vTexturePaths.end(); ++it)
        FileManager.GetFileList(*it, sSearch, true, vResult);
#endif
    for (vector<CTextureArchive*>::iterator it(g_vTextureArchives.begin()); it != g_vTextureArchives.end(); ++it)
    {
        tsvector vFiles;
        (*it)->GetTextureList(vFiles);
        for (tsvector_it itFiles(vFiles.begin()); itFiles != vFiles.end(); ++itFiles)
        {
            if ((sSearch.empty() || itFiles->find(sSearch) != tstring::npos && (sPath.empty() || itFiles->find(sPath) !=  tstring::npos)))
                vResult.push_back((*itFiles).substr(8, (*itFiles).length() - 11) + _T("tga"));
        }
    }
}


/*====================
  CGfxTextures::RegisterCachedTexture
  ====================*/
uint    CGfxTextures::RegisterCachedTexture(CTexture *pTexture, const tstring &sReference, const tstring &sSuffix)
{
    try
    {
        int iTextureFlags(pTexture->GetTextureFlags());

        if (pTexture->GetType() == TEXTURE_VOLUME)
            iTextureFlags |= TEX_NO_COMPRESS;

        if (iTextureFlags & TEX_FULL_QUALITY)
            TextureDefine("DOWNSIZE", "0");
        else
            TextureDefine("DOWNSIZE", XtoS(vid_textureDownsize));

        if (iTextureFlags & TEX_NO_COMPRESS)
            TextureDefine("COMPRESSION", "0");
        else
            TextureDefine("COMPRESSION", vid_textureCompression ? "1" : "0");

        if (iTextureFlags & TEX_NO_MIPMAPS)
            TextureDefine("MIPMAPS", "0");
        else
            TextureDefine("MIPMAPS", "1");

        g_TextureCache.ActivateNode(GetTextureDefinitionString());

        tstring sFilePath;

        if (!g_TextureCache.LoadTexture(Filename_AppendSuffix(pTexture->GetPath(), sSuffix), sReference, sFilePath))
            return 0;

        string sSystemPath(TStringToString(FileManager.GetSystemPath(sFilePath)));

        uint uiTextureID(0);

        if (pTexture->GetType() == TEXTURE_2D)
        {
            int iFlags(SOIL_FLAG_DDS_LOAD_DIRECT);
            if ((!g_DeviceCaps.bTextureCompression || !vid_textureCompression) && !(iTextureFlags & TEX_NO_COMPRESS))
                iFlags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_DECOMPRESS;
            uiTextureID = SOIL_load_OGL_texture
            (
                sSystemPath.c_str(),
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                iFlags
            );

            if (uiTextureID == 0)
                EX_WARN(_T("TEXTURE_2D load failed"));

            glBindTexture(GL_TEXTURE_2D, uiTextureID);
            SetDefaultTexParameters2D(iTextureFlags);
        }
        else if (pTexture->GetType() == TEXTURE_CUBE)
        {
            int iFlags(SOIL_FLAG_DDS_LOAD_DIRECT);
            if ((!g_DeviceCaps.bTextureCompression || !vid_textureCompression) && !(iTextureFlags & TEX_NO_COMPRESS))
                iFlags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_DECOMPRESS;
            uiTextureID =  SOIL_load_OGL_single_cubemap
            (
                sSystemPath.c_str(),
                SOIL_DDS_CUBEMAP_FACE_ORDER,
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                iFlags
            );

            if (uiTextureID == 0)
                EX_WARN(_T("TEXTURE_CUBE load failed"));

            glBindTexture(GL_TEXTURE_CUBE_MAP, uiTextureID);
            SetDefaultTexParametersCube(iTextureFlags);
        }
        else if (pTexture->GetType() == TEXTURE_VOLUME)
        {
#if 0
            int iFlags(SOIL_FLAG_DDS_LOAD_DIRECT);
            if ((!g_DeviceCaps.bTextureCompression || !vid_textureCompression) && !(iTextureFlags & TEX_NO_COMPRESS))
                iFlags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_DECOMPRESS;
            uiTextureID = SOIL_load_OGL_single_texture3D
            (
                sSystemPath.c_str(),
                0,
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                iFlags
            );

            if (uiTextureID == 0)
                EX_WARN(_T("TEXTURE_VOLUME load failed"));

            glBindTexture(GL_TEXTURE_3D, uiTextureID);
            SetDefaultTexParametersVolume(iTextureFlags);
#else
            EX_WARN(_T("TEXTURE_VOLUME not supported"));
#endif
        }

        if (uiTextureID == 0)
            EX_WARN(_T("Texture load failed"));
        
        if (GLEW_EXT_texture_filter_anisotropic)
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);

        return uiTextureID;
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CGfxTextures::RegisterCachedTexture(") + pTexture->GetPath() + _TS(") - "), NO_THROW);
        return 0;
    }
}


/*====================
  CGfxTextures::RegisterCachedTexture
  ====================*/
uint    CGfxTextures::RegisterCachedTexture(CTexture *pTexture)
{
    return RegisterCachedTexture(pTexture, pTexture->GetPath(), TSNULL);
}


/*====================
  CGfxTextures::ResizeBitmap
  ====================*/
bool    CGfxTextures::ResizeBitmap(const CBitmap &bitmap, CBitmap &bmpResized, int iTextureFlags)
{
    // Resize, if necessary
    int iNewWidth(bitmap.GetWidth());
    int iNewHeight(bitmap.GetHeight());

    bool bPerf(false); // Generate a performance warning

    int iDownsize = vid_textureDownsize;
    if (iDownsize > 0 && !(iTextureFlags & TEX_FULL_QUALITY))
    {
        iNewWidth = MAX(M_FloorPow2(iNewWidth) >> iDownsize, 1);
        iNewHeight = MAX(M_FloorPow2(iNewHeight) >> iDownsize, 1);
    }
    if ((iNewWidth > vid_textureMaxSize || iNewHeight > vid_textureMaxSize) && !(iTextureFlags & TEX_FULL_QUALITY))
    {
        iNewWidth = M_FloorPow2(MIN(iNewWidth, int(vid_textureMaxSize)));
        iNewHeight = M_FloorPow2(MIN(iNewHeight, int(vid_textureMaxSize)));
    }
    if (!M_IsPow2(iNewWidth) || !M_IsPow2(iNewHeight)) // Resize texture if its not a power of 2
    {
        iNewWidth = M_FloorPow2(iNewWidth);
        iNewHeight = M_FloorPow2(iNewHeight);

        bPerf = true;
    }

    if (bitmap.GetWidth() != iNewWidth || bitmap.GetHeight() != iNewHeight)
    {
        if (bPerf)
        {
            Console.Perf << "Resizing a bitmap from "
                << bitmap.GetWidth() << "x" << bitmap.GetHeight() << " to "
                << iNewWidth << "x" << iNewHeight << newl;
        }
        else
        {
            Console.Dev << "Resizing a bitmap from "
                << bitmap.GetWidth() << "x" << bitmap.GetHeight() << " to "
                << iNewWidth << "x" << iNewHeight << newl;
        }
        bitmap.Scale(bmpResized, iNewWidth, iNewHeight);

        return 1;
    }
    return 0;
}


/*====================
  CGfxTextures::GetBitmapInfo
  ====================*/
void    CGfxTextures::GetBitmapInfo(const CBitmap &bitmap, const ETextureFormat &eFormat, GLint &internalFormat, GLenum &format, GLenum &type)
{
    switch (bitmap.GetBMPType())
    {
    case BITMAP_ALPHA:
        format = GL_ALPHA; type = GL_UNSIGNED_BYTE; break;
    case BITMAP_LUMINANCE_ALPHA:
        format = GL_LUMINANCE_ALPHA; type = GL_UNSIGNED_BYTE; break;
    case BITMAP_RGB:
        format = GL_RGB; type = GL_UNSIGNED_BYTE; break;
    case BITMAP_RGBA:
        format = GL_RGBA; type = GL_UNSIGNED_BYTE; break;
    case BITMAP_RGBA16F:
        format = GL_RGBA; type = GL_HALF_FLOAT_ARB; break;
    case BITMAP_RGBA32F:
        format = GL_RGBA; type = GL_FLOAT; break;
    default:
        format = GL_RGBA; type = GL_UNSIGNED_BYTE; break;
    }

    switch (eFormat)
    {
    default:
    case TEXFMT_A8R8G8B8:
    case TEXFMT_NORMALMAP:
    case TEXFMT_NORMALMAP_RXGB:
    case TEXFMT_NORMALMAP_S:
        internalFormat = GL_RGBA8; break;
    case TEXFMT_A1R5G5B5:
        internalFormat = GL_RGB5_A1; break;
    case TEXFMT_A4R4G4B4:
        internalFormat = GL_RGBA4; break;
    case TEXFMT_A8:
        internalFormat = GL_ALPHA8; break;
    case TEXFMT_A8L8:
        internalFormat = GL_LUMINANCE8_ALPHA8; break;
    case TEXFMT_R16G16:
        internalFormat = GL_LUMINANCE16_ALPHA16; break;
    case TEXFMT_U8V8:
        internalFormat = GL_LUMINANCE8_ALPHA8; break; // TODO: Is this the right format?
    case TEXFMT_U16V16:
        internalFormat = GL_LUMINANCE16_ALPHA16; break; // TODO: Is this the right format?
    case TEXFMT_R16F:
        internalFormat = GL_LUMINANCE_FLOAT16_ATI; break;
    case TEXFMT_R16G16F:
        internalFormat = GL_LUMINANCE_ALPHA_FLOAT16_ATI; break;
    case TEXFMT_A16R16G16B16F:
        internalFormat = GL_RGBA_FLOAT16_ATI; break;
    case TEXFMT_R32F:
        internalFormat = GL_LUMINANCE_FLOAT32_ATI; break;
    case TEXFMT_R32G32F:
        internalFormat = GL_LUMINANCE_ALPHA_FLOAT32_ATI; break;
    case TEXFMT_A32R32G32B32F:
        internalFormat = GL_RGBA32F_ARB; break;
    }
}


/*====================
  CGfxTextures::RegisterRenderTargetTexture
  ====================*/
uint    CGfxTextures::RegisterRenderTargetTexture(const tstring &sName, int iSizeX, int iSizeY, GLint iInternalFormat, bool bMipmap)
{
    // See if this texture is already in memory
    TextureMap::iterator findit(mapTextures.find(sName));

    if (findit != mapTextures.end())
    {
        // Yep, so just return the instance that's already in memory
        return findit->second.uiTextureID;
    }

    GLuint uiTextureID(RegisterTextureID());

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, uiTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    PRINT_GLERROR_BREAK();

    if (bMipmap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureMinFilterMipmap);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureMinFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, iInternalFormat, iSizeX, iSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    PRINT_GLERROR_BREAK();

    mapTextures[sName] = STextureMapEntry(uiTextureID, 0, INVALID_RESOURCE);

    return uiTextureID;
}


/*====================
  CGfxTextures::RegisterDynamicTexture
  ====================*/
uint    CGfxTextures::RegisterDynamicTexture(const tstring &sName, int iSizeX, int iSizeY, GLint iInternalFormat, bool bMipmap)
{
    // See if this texture is already in memory
    TextureMap::iterator findit(mapTextures.find(sName));

    if (findit != mapTextures.end())
    {
        // Yep, so just return the instance that's already in memory
        return findit->second.uiTextureID;
    }

    GLuint uiTextureID(RegisterTextureID());

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, uiTextureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    PRINT_GLERROR_BREAK();

    if (bMipmap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureMinFilterMipmap);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureMinFilter);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, iInternalFormat, iSizeX, iSizeY, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);

    PRINT_GLERROR_BREAK();

    mapTextures[sName] = STextureMapEntry(uiTextureID, 0, INVALID_RESOURCE);

    return uiTextureID;
}


/*====================
  CGfxTextures::RegisterProcedural
  ====================*/
GLuint  CGfxTextures::RegisterProcedural(const CProcedural &cProcedural, GLuint &uiTextureID2)
{
    GLuint uiTextureID = RegisterTextureID();
    glBindTexture(GL_TEXTURE_2D, uiTextureID);

    try
    {
        ETextureFormat eFormat(cProcedural.GetFormat());

        if (cProcedural.IsMipmaps())
        {
            int iNumLevels(M_Log2(MAX(cProcedural.GetWidth(), cProcedural.GetHeight())) + 1);
            CBitmap *bmp = K2_NEW_ARRAY(ctx_GL2, CBitmap, iNumLevels);

            if (eFormat == TEXFMT_R16F ||
                eFormat == TEXFMT_R16G16F ||
                eFormat == TEXFMT_A16R16G16B16F ||
                eFormat == TEXFMT_R32F ||
                eFormat == TEXFMT_R32G32F ||
                eFormat == TEXFMT_A32R32G32B32F)
            {
                for (int i(0); i < iNumLevels; ++i)
                {
                    int iWidth(cProcedural.GetWidth() >> i);
                    int iHeight(cProcedural.GetHeight() >> i);

                    bmp[i].Alloc(iWidth, iHeight, BITMAP_RGBA32F);

                    for (int y = 0; y < iHeight; ++y)
                    {
                        for (int x = 0; x < iWidth; ++x)
                        {
                            CVec4f vColor(cProcedural.Get((x + 0.5f) / iWidth, (y + 0.5f) / iHeight, i));

                            bmp[i].SetPixel4f(x, y, vColor.x, vColor.y, vColor.z, vColor.w);
                        }
                    }
                    UploadBitmap2D(bmp[i], cProcedural.GetFlags(), eFormat, i, false);
                }
            }
            else
            {
                for (int i(0); i < iNumLevels; ++i)
                {
                    int iWidth(cProcedural.GetWidth() >> i);
                    int iHeight(cProcedural.GetHeight() >> i);

                    bmp[i].Alloc(iWidth, iHeight, BITMAP_RGBA);

                    for (int y = 0; y < iHeight; ++y)
                    {
                        for (int x = 0; x < iWidth; ++x)
                        {
                            CVec4f vColor(cProcedural.Get((x + 0.5f) / iWidth, (y + 0.5f) / iHeight, i));

                            bmp[i].SetPixel4b(x, y,
                                INT_ROUND(CLAMP(vColor.x, 0.0f, 1.0f) * 255.0f),
                                INT_ROUND(CLAMP(vColor.y, 0.0f, 1.0f) * 255.0f),
                                INT_ROUND(CLAMP(vColor.z, 0.0f, 1.0f) * 255.0f),
                                INT_ROUND(CLAMP(vColor.w, 0.0f, 1.0f) * 255.0f));
                        }
                    }
                    UploadBitmap2D(bmp[i], cProcedural.GetFlags(), eFormat, i, false);
                }
            }
            
            K2_DELETE_ARRAY(bmp);
        }
        else
        {
            CBitmap bmp;

            if (eFormat == TEXFMT_R16F ||
                eFormat == TEXFMT_R16G16F ||
                eFormat == TEXFMT_A16R16G16B16F ||
                eFormat == TEXFMT_R32F ||
                eFormat == TEXFMT_R32G32F ||
                eFormat == TEXFMT_A32R32G32B32F)
            {
                bmp.Alloc(cProcedural.GetWidth(), cProcedural.GetHeight(), BITMAP_RGBA32F);

                for (int y = 0; y < cProcedural.GetHeight(); ++y)
                {
                    for (int x = 0; x < cProcedural.GetWidth(); ++x)
                    {
                        CVec4f vColor(cProcedural.Get((x + 0.5f) / cProcedural.GetWidth(), (y + 0.5f) / cProcedural.GetHeight()));

                        bmp.SetPixel4f(x, y, vColor.x, vColor.y, vColor.z, vColor.w);
                    }
                }
            }
            else
            {
                bmp.Alloc(cProcedural.GetWidth(), cProcedural.GetHeight(), BITMAP_RGBA);

                for (int y = 0; y < cProcedural.GetHeight(); ++y)
                {
                    for (int x = 0; x < cProcedural.GetWidth(); ++x)
                    {
                        CVec4f vColor(cProcedural.Get((x + 0.5f) / cProcedural.GetWidth(), (y + 0.5f) / cProcedural.GetHeight()));

                        bmp.SetPixel4b(x, y,
                            INT_ROUND(CLAMP(vColor.x, 0.0f, 1.0f) * 255.0f),
                            INT_ROUND(CLAMP(vColor.y, 0.0f, 1.0f) * 255.0f),
                            INT_ROUND(CLAMP(vColor.z, 0.0f, 1.0f) * 255.0f),
                            INT_ROUND(CLAMP(vColor.w, 0.0f, 1.0f) * 255.0f));
                    }
                }
            }

            if (eFormat == TEXFMT_NORMALMAP && vid_shaderRXGBNormalmap)
            {
                uiTextureID2 = Register2DTexture(bmp, cProcedural.GetFlags(), TEXFMT_NORMALMAP_S, cProcedural.GetName());
                return Register2DTexture(bmp, cProcedural.GetFlags(), TEXFMT_NORMALMAP_RXGB, cProcedural.GetName());
            }
            else
            {
                uiTextureID2 = -1;
                return Register2DTexture(bmp, cProcedural.GetFlags(), eFormat, cProcedural.GetName());
            }
        }
    }
    catch (CException &ex)
    {
        glDeleteTextures(1, &uiTextureID);
        ex.Process(_TS("CGfxTextures::RegisterProcedural(") + cProcedural.GetName() + _TS(") - "), NO_THROW);
        return 0;
    }

    return uiTextureID;
}


/*====================
  CGfxTextures::TextureDefine
  ====================*/
void    CGfxTextures::TextureDefine(const string &sName, const string &sDefinition)
{
    m_mapTextureDefinitions[sName] = sDefinition;
}


/*====================
  CGfxTextures::TextureUndefine
  ====================*/
void    CGfxTextures::TextureUndefine(const string &sName)
{
    m_mapTextureDefinitions.erase(sName);
}


/*====================
  CGfxTextures::GetTextureDefinitionString
  ====================*/
string  CGfxTextures::GetTextureDefinitionString()
{
    string s;

    for (DefinitionMap::iterator it(m_mapTextureDefinitions.begin()); it != m_mapTextureDefinitions.end(); ++it)
    {
        if (it->first.empty())
            continue;

        if (it->second.empty())
            s += it->first + ";";
        else
            s += it->first + "='" + it->second + "';";
    }

    return s;
}


/*====================
  CGfxTextures::Init
  ====================*/
void    CGfxTextures::Init()
{
    //
    // Procdurals
    //

    GL_OpenTextureArchive(true);

    CProceduralRegistry::GetInstance()->RegisterProcedurals();
    m_iWhite = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("white"));
    m_iBlack = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("black"));
}


/*====================
  CGfxTextures::Shutdown
  ====================*/
void    CGfxTextures::Shutdown()
{
    TextureMap::iterator itEnd(mapTextures.end());
    for (TextureMap::iterator it(mapTextures.begin()); it != itEnd; ++it)
    {
        GL_SAFE_DELETE(glDeleteTextures, it->second.uiTextureID);
        GL_SAFE_DELETE(glDeleteTextures, it->second.uiTextureID2);
    }

    mapTextures.clear();

    PRINT_GLERROR_BREAK();
}


/*====================
  GL_CloseTextureArchive
  ====================*/
void    GL_CloseTextureArchive()
{
    for (vector<CTextureArchive*>::iterator it(g_vTextureArchives.begin()); it != g_vTextureArchives.end(); ++it)
        SAFE_DELETE(*it);
    g_vTextureArchives.clear();
}


/*====================
  GL_OpenTextureArchive
  ====================*/
void    GL_OpenTextureArchive(bool bNoReload)
{
    bool bReload(!g_vTextureArchives.empty() && !bNoReload);

    GL_CloseTextureArchive();

    int iModDepth(1);
    tstring sMod(FileManager.GetTopModPath());
    while (!sMod.empty())
    {
        g_vTextureArchives.push_back(K2_NEW(ctx_GL2,  CTextureArchive)(sMod));

        sMod = FileManager.GetModPath(iModDepth);
        ++iModDepth;
    }

    if (bReload)
        g_ResourceManager.GetLib(RES_TEXTURE)->ReloadAll();
}


/*====================
  ReloadTextures
  ====================*/
CMD(ReloadTextures)
{
    GL_SetupTextureFilterSettings();
    GL_OpenTextureArchive(true);
    g_ResourceManager.GetLib(RES_TEXTURE)->ReloadAll();
    return true;
}


/*====================
  GL_TextureExists
  ====================*/
bool    GL_TextureExists(const tstring &sFilename, uint uiTextureFlags)
{
    if (GfxTextures->ArchivedTextureExists(sFilename, uiTextureFlags))
        return true;
    else if (FileManager.Exists(sFilename))
        return true;

    // try each fallback extension.
    for (auto sFallbackExt : {_T("png"), _T("dds"), _T("tga")})
    {
        if (CompareNoCase(Filename_GetExtension(sFilename), sFallbackExt) == 0)
            continue;
        tstring sNewFilename(Filename_StripExtension(sFilename) + _T(".") + sFallbackExt);
        if (GfxTextures->ArchivedTextureExists(sNewFilename, uiTextureFlags))
            return true;
        else if (FileManager.Exists(sNewFilename))
            return true;
    }
    return false;
}
