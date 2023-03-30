// (C)2005 S2 Games
// c_bitmap.h
//
//=============================================================================
#ifndef __C_BITMAP_H__
#define __C_BITMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EBMPType
{
    BITMAP_NULL,
    BITMAP_ALPHA = 1,
    BITMAP_LUMINANCE_ALPHA = 2,
    BITMAP_RGB = 3,
    BITMAP_RGBA = 4,

    BITMAP_RGBA16F = 8,
    BITMAP_RGBA32F = 16,

    BITMAP_DXT1,
    BITMAP_DXT2,
    BITMAP_DXT3,
    BITMAP_DXT4,
    BITMAP_DXT5,

    BITMAP_GRAYSCALE
};

const int BMP_CHANNEL_NULL(-1);
const int BMP_CHANNEL_WHITE(-2);
const int BMP_CHANNEL_BLACK(-3);

// DDS support
const dword DDSD_CAPS           (0x000000001);
const dword DDSD_HEIGHT         (0x000000002);
const dword DDSD_WIDTH          (0x000000004);
const dword DDSD_PITCH          (0x000000008);
const dword DDSD_PIXELFORMAT    (0x000001000);
const dword DDSD_MIPMAPCOUNT    (0x000020000);
const dword DDSD_LINEARSIZE     (0x000080000);
const dword DDSD_DEPTH          (0x000800000);

const dword DDPF_ALPHAPIXELS    (0x000000001);
const dword DDPF_FOURCC         (0x000000004);
const dword DDPF_RGB            (0x000000040);

const dword DDSCAPS_COMPLEX     (0x000000008);
const dword DDSCAPS_TEXTURE     (0x000001000);
const dword DDSCAPS_MIPMAP      (0x004000000);

const dword DDSCAPS2_CUBEMAP            (0x000000200);
const dword DDSCAPS2_CUBEMAP_POSITIVEX  (0x000000400);
const dword DDSCAPS2_CUBEMAP_NEGATIVEX  (0x000000800);
const dword DDSCAPS2_CUBEMAP_POSITIVEY  (0x000001000);
const dword DDSCAPS2_CUBEMAP_NEGATIVEY  (0x000002000);
const dword DDSCAPS2_CUBEMAP_POSITIVEZ  (0x000004000);
const dword DDSCAPS2_CUBEMAP_NEGATIVEZ  (0x000008000);
const dword DDSCAPS2_VOLUME             (0x002000000);

const dword DDS_REQUIRED_FIELDS(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);

const dword DDS_MAGIC_NUMBER('D' | 'D' << 8 | 'S' << 16 | ' ' << 24);
const dword DXT1_NUMBER('D' | 'X' << 8 | 'T' << 16 | '1' << 24);
const dword DXT2_NUMBER('D' | 'X' << 8 | 'T' << 16 | '2' << 24);
const dword DXT3_NUMBER('D' | 'X' << 8 | 'T' << 16 | '3' << 24);
const dword DXT4_NUMBER('D' | 'X' << 8 | 'T' << 16 | '4' << 24);
const dword DXT5_NUMBER('D' | 'X' << 8 | 'T' << 16 | '5' << 24);

struct SDDSPixelFormat
{
    dword   dwSize;
    dword   dwFlags;
    dword   dwFourCC;
    dword   dwBitCount;
    dword   dwRBitMask;
    dword   dwGBitMask;
    dword   dwBBitMask;
    dword   dwABitmask;
};

struct SDDSCaps
{
    dword   dwCaps1;
    dword   dwCaps2;
    dword   dwReserved;
};

struct SDDSHeader
{
    dword           dwMagic;
    dword           dwSize;
    dword           dwFlags;
    dword           dwHeight;
    dword           dwWidth;
    dword           dwPitch;
    dword           dwDepth;
    dword           dwNumMaps;
    dword           dwReserved[11];
    SDDSPixelFormat PixelFormat;
    SDDSCaps        Caps;
    dword           dwReserved2;
};

struct SBitmapFormatDescriptor
{
    int     iPixelBytes;
    int     aStart[4];
    int     aBits[4];
    int     aIntDefault[4];
    float   aFloatDefault[4];
    bool    bFloat;
};
//=============================================================================

//=============================================================================
// CBitmap
//=============================================================================
class K2_API CBitmap
{
private:
    byte    *m_pData;
    int     m_iSize;

    int     m_iWidth;
    int     m_iHeight;
    int     m_iBMPType;
    
    int     m_iRedChannel;
    int     m_iGreenChannel;
    int     m_iBlueChannel;
    int     m_iAlphaChannel;

    uint    m_uiFileFlags;

    void    GetColor4f(float x, float y, float &r, float &g, float &b, float &a) const;
    void    ScaleUp(CBitmap &out, int iWidth, int iHeight) const;
    void    ScaleDown(CBitmap &out, int iWidth, int iHeight) const;
    void    ScaleDown(CBitmap &out, int iWidth, int iHeight, float fGamma) const;
    void    GetChannelInfo(int iChannel, int &iStart, int &iDefault, float &fDefault, int &iBits) const;

public:
    CBitmap();
    CBitmap(const CBitmap &c);
    CBitmap(const tstring &sFileName, bool bMonoAsAlpha = false);
    CBitmap(CFileHandle &hFile);
    CBitmap(int iWidth, int iHeight, int iBMPType); // alloc an empty bitmap
    CBitmap(int iWidth, int iHeight, int iBMPType, int iRedChannel, int iGreenChannel, int iBlueChannel, int iAlphaChannel);

    ~CBitmap();

    int     GetBMPType() const          { return m_iBMPType; }
    int     GetWidth() const            { return m_iWidth; }
    int     GetHeight() const           { return m_iHeight; }
    const byte* GetBuffer() const       { return m_pData; }
    byte*   GetBuffer()                 { return m_pData; }
    int     GetSize() const             { return m_iSize; }
    int     GetRedChannel() const       { return m_iRedChannel; }
    int     GetGreenChannel() const     { return m_iGreenChannel; }
    int     GetBlueChannel() const      { return m_iBlueChannel; }
    int     GetAlphaChannel() const     { return m_iAlphaChannel; }

    byte&   operator[](int i)           { return m_pData[i]; }

    bool    Load(const tstring &sFilename, bool bMonoAsAlpha = false);
    bool    LoadPNG(const tstring &Filename, bool bMonoAsAlpha = false);
    bool    LoadTGA(const tstring &sFilename, bool bMonoAsAlpha = false);
    bool    LoadJPEG(const tstring &sFilename, bool bMonoAsAlpha = false);
    bool    LoadGIF(const tstring &sFilename);
    bool    LoadDDS(const tstring &sFileName);
    bool    LoadAuto(const tstring &sFileName, bool bMonoAsAlpha = false);

    bool    Load(CFileHandle &hFile);
    bool    LoadJPEG(CFileHandle &hFile);

    void    GetColor(int x, int y, bvec4_t color);
    CVec4b  GetColor(int x, int y);
    CVec4b  GetAverageColor();
    void    DesaturateToAlpha(int width, int height);
    void    Free();
    bool    WritePNG(const tstring &sFilename);
    bool    WriteJPEG(const tstring &sFilename, int quality); // quality goes from 1 - 100
    bool    WriteTGA(const tstring &sFileName);
    void    Flip();
    void    Scale(CBitmap &out, int width, int height) const;
    void    Scale(CBitmap &out, int width, int height, float fGamma) const;
    void    Copy(CBitmap &out) const;

    void    SetRedChannel(int iType)    { m_iRedChannel = iType; }
    void    SetGreenChannel(int iType)  { m_iGreenChannel = iType; }
    void    SetBlueChannel(int iType)   { m_iBlueChannel = iType; }
    void    SetAlphaChannel(int iType)  { m_iAlphaChannel = iType; }
    void    SetPixel4b(int x, int y, byte r, byte g, byte b, byte a);
    void    SetPixel4f(int x, int y, float r, float g, float b, float a);
    void    Clear4b(byte r, byte g, byte b, byte a);
    void    Alloc(int iWidth, int iHeight, int iBMPType);
    void    Alloc(int iWidth, int iHeight, int iBMPType, int iRedChannel, int iGreenChannel, int iBlueChannel, int iAlphaChannel);
    void    AllocCompressed(int iSize, int iWidth, int iHeight, int iBMPType);

    void    Lerp(float fLerp, const CBitmap &a, const CBitmap &b);

    void    GetFormatDescriptor(SBitmapFormatDescriptor &cDescriptor) const;

    void    SetFileFlags(uint uiFlags)  { m_uiFileFlags = uiFlags; }
    uint    GetFileFlags() const        { return m_uiFileFlags; }

    static void     GenerateThumbnails(const tstring &sDirName, int iSize);
    static void     GenerateThumbnail(const tstring &sFilename, int iSize);
};
//=============================================================================

K2_API void     Bitmap_Init();

#endif //__C_BITMAP_H__
