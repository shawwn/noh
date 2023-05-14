// (C)2005 S2 Games
// c_bitmap.cpp
//
// general bitmap loading functions
//
// TODO:
// - setup exception handling for the loading functions
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include <libpng16/png.h>
#include <libpng16/pngstruct.h>
#include <libpng16/pnginfo.h>

extern "C"
{
#include <jpeglib.h>
#include <jerror.h>
}

#include <gif_lib.h>

#include "c_bitmap.h"

#include "c_filemanager.h"
#include "c_filehandle.h"
#include "c_cmd.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct TargaHeader
{
    byte    id_length, colormap_type, image_type;
    ushort  colormap_index, colormap_length;
    byte    colormap_size;
    ushort  x_origin, y_origin, width, height;
    byte    pixel_size, attributes;
};

struct SJPEGDestManager
{
    struct jpeg_destination_mgr m_pJPEGDestMgr;
    CFileHandle*                m_pFileHandle;
    int                         m_iBufferSize;
    char*                       m_pBuffer;
};
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
// Exceptions are not propagated through c librarys under gnu/linux.  Need to 
//   use longjump/setjmp then throw the exception
#ifdef __GNUC__
jmp_buf g_JPEGJmpBuf;   // jmp buf for the jpeg error handler (png has one in the read/write struct)
tstring g_sErrorMsg;    // string to store error
#endif
//=============================================================================

/*====================
  CBitmap::CBitmap
  ====================*/
CBitmap::CBitmap() :
m_pData(nullptr),
m_iWidth(0),
m_iHeight(0),
m_iSize(0),
m_iBMPType(BITMAP_NULL),
m_iRedChannel(BMP_CHANNEL_NULL),
m_iGreenChannel(BMP_CHANNEL_NULL),
m_iBlueChannel(BMP_CHANNEL_NULL),
m_iAlphaChannel(BMP_CHANNEL_NULL),
m_uiFileFlags(0)
{
}


/*====================
  CBitmap::CBitmap
  ====================*/
CBitmap::CBitmap(int iWidth, int iHeight, int iBMPType) :
m_pData(nullptr),
m_iSize(0),
m_iBMPType(BITMAP_NULL),
m_iRedChannel(BMP_CHANNEL_NULL),
m_iGreenChannel(BMP_CHANNEL_NULL),
m_iBlueChannel(BMP_CHANNEL_NULL),
m_iAlphaChannel(BMP_CHANNEL_NULL),
m_uiFileFlags(0)
{
    Alloc(iWidth, iHeight, iBMPType);
}


/*====================
  CBitmap::CBitmap
  ====================*/
CBitmap::CBitmap(int iWidth, int iHeight, int iBMPType, int iRedChannel, int iGreenChannel, int iBlueChannel, int iAlphaChannel) :
m_pData(nullptr),
m_iSize(0),
m_iBMPType(BITMAP_NULL),
m_iRedChannel(BMP_CHANNEL_NULL),
m_iGreenChannel(BMP_CHANNEL_NULL),
m_iBlueChannel(BMP_CHANNEL_NULL),
m_iAlphaChannel(BMP_CHANNEL_NULL),
m_uiFileFlags(0)
{
    Alloc(iWidth, iHeight, iBMPType, iRedChannel, iGreenChannel, iBlueChannel, iAlphaChannel);
}


/*====================
  CBitmap::CBitmap
  ====================*/
CBitmap::CBitmap(const tstring &sFilename, bool bMonoAsAlpha) :
m_pData(nullptr),
m_iSize(0),
m_iBMPType(BITMAP_NULL),
m_iRedChannel(BMP_CHANNEL_NULL),
m_iGreenChannel(BMP_CHANNEL_NULL),
m_iBlueChannel(BMP_CHANNEL_NULL),
m_iAlphaChannel(BMP_CHANNEL_NULL),
m_uiFileFlags(0)
{
    Load(sFilename, bMonoAsAlpha);
}


/*====================
  CBitmap::CBitmap
  ====================*/
CBitmap::CBitmap(CFileHandle &hFile) :
m_pData(nullptr),
m_iSize(0),
m_iBMPType(BITMAP_NULL),
m_iRedChannel(BMP_CHANNEL_NULL),
m_iGreenChannel(BMP_CHANNEL_NULL),
m_iBlueChannel(BMP_CHANNEL_NULL),
m_iAlphaChannel(BMP_CHANNEL_NULL),
m_uiFileFlags(0)
{
    Load(hFile);
}


/*====================
  CBitmap::CBitmap
  ====================*/
CBitmap::CBitmap(const CBitmap &c) :
m_pData(nullptr),
m_iSize(0),
m_iBMPType(BITMAP_NULL)
{
    m_iWidth = c.m_iWidth;
    m_iHeight = c.m_iHeight;
    m_iSize = c.m_iSize;
    m_pData = K2_NEW_ARRAY(ctx_Bitmap, byte, m_iSize);
    m_iBMPType = c.m_iBMPType;

    MemManager.Copy(m_pData, c.m_pData, m_iSize);

    m_iRedChannel = c.m_iRedChannel;
    m_iGreenChannel = c.m_iGreenChannel;
    m_iBlueChannel = c.m_iBlueChannel;
    m_iAlphaChannel = c.m_iAlphaChannel;
    m_uiFileFlags = c.m_uiFileFlags;
}


/*====================
  CBitmap::~CBitmap
  ====================*/
CBitmap::~CBitmap()
{
    Free();
}


/*====================
  CBitmap::Load
  ====================*/
bool    CBitmap::Load(const tstring &sFilename, bool bMonoAsAlpha)
{
    PROFILE("CBitmap::Load");

    if (sFilename.empty())
        return false;

    tstring sExt(Filename_GetExtension(sFilename));

    if (CompareNoCase(sExt, _T("dds")) == 0)
        return LoadDDS(sFilename);

    if (CompareNoCase(sExt, _T("tga")) == 0)
        return LoadTGA(sFilename, bMonoAsAlpha);

    if (CompareNoCase(sExt, _T("png")) == 0)
        return LoadPNG(sFilename, bMonoAsAlpha);

    if (CompareNoCase(sExt, _T("jpg")) == 0)
        return LoadJPEG(sFilename, bMonoAsAlpha);

    if (CompareNoCase(sExt, _T("gif")) == 0)
        return LoadGIF(sFilename);

    if (CompareNoCase(sExt, _T("thumb")) == 0)
        return LoadPNG(sFilename, bMonoAsAlpha);

    return LoadAuto(sFilename, bMonoAsAlpha);
}


/*====================
  CBitmap::LoadAuto
  ====================*/
bool    CBitmap::LoadAuto(const tstring &sFilename, bool bMonoAsAlpha)
{
    try
    {
        // Read the file
        CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY | m_uiFileFlags);
        if (!hFile.IsOpen())
            EX_ERROR(_T("Failed opening file"));

        if (hFile.GetLength() < 1)
            EX_ERROR(_T("File too small"));

        byte y(hFile.ReadByte());

        hFile.Close();

        if (y == 'D')
            return LoadDDS(sFilename);
        
        if (y == 0x89)
            return LoadPNG(sFilename, bMonoAsAlpha);

        if (y == 0xff)
            return LoadJPEG(sFilename, bMonoAsAlpha);

        if (y == 'G')
            return LoadGIF(sFilename);

        return LoadTGA(sFilename, bMonoAsAlpha);
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CBitmap::LoadAuto(") + sFilename + _TS(") - "), NO_THROW);
        return false;
    }
}


/*====================
  CBitmap::Load
  ====================*/
bool    CBitmap::Load(CFileHandle &hFile)
{
    PROFILE("CBitmap::Load");

    if (hFile.GetPath().empty())
        return false;

    tstring sExt(Filename_GetExtension(hFile.GetPath()));

#if 0
    if (CompareNoCase(sExt, _T("dds")) == 0)
        return LoadDDS(hFile);

    if (CompareNoCase(sExt, _T("tga")) == 0)
        return LoadTGA(hFile);

    if (CompareNoCase(sExt, _T("png")) == 0)
        return LoadPNG(hFile);

    if (CompareNoCase(sExt, _T("jpg")) == 0)
        return LoadJPEG(hFile);

    if (CompareNoCase(sExt, _T("thumb")) == 0)
        return LoadPNG(hFile);
#endif

    return false;
}


/*====================
  CBitmap::LoadDDS
  ====================*/
bool    CBitmap::LoadDDS(const tstring &sFilename)
{
    try
    {
        // Read the file
        CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY | m_uiFileFlags);
        if (!hFile.IsOpen())
            EX_ERROR(_T("Failed opening file"));

        uint uiSize(0);
        const char *pBuffer(hFile.GetBuffer(uiSize));
        if (pBuffer == nullptr || uiSize == 0)
            EX_ERROR(_T("Failed retrieving buffer"));

        SDDSHeader *pHeader((SDDSHeader*)pBuffer);
        
        ToLittle(pHeader->dwMagic);
        ToLittle(pHeader->dwSize);
        ToLittle(pHeader->dwFlags);
        ToLittle(pHeader->PixelFormat.dwSize);
        ToLittle(pHeader->dwWidth);
        ToLittle(pHeader->dwHeight);
        ToLittle(pHeader->PixelFormat.dwFlags);
        ToLittle(pHeader->dwPitch);
        ToLittle(pHeader->PixelFormat.dwFourCC);

        // Validate header
        if (pHeader->dwMagic != DDS_MAGIC_NUMBER)
            EX_ERROR(_T("Not a valid DDS file"));

        if (pHeader->dwSize != 124)
            EX_ERROR(_T("Invalid size for header"));

        if ((pHeader->dwFlags & DDS_REQUIRED_FIELDS) != DDS_REQUIRED_FIELDS)
            EX_ERROR(_T("Missing required fields in header"));

        if (pHeader->PixelFormat.dwSize != 32)
            EX_ERROR(_T("Invalid size for PixelFormat"));

        // Dimensions
        m_iWidth = pHeader->dwWidth;
        m_iHeight = pHeader->dwHeight;

        // Pixel format
        if (pHeader->PixelFormat.dwFlags & DDPF_FOURCC)
        {
            //if (!(pHeader->dwFlags & DDSD_LINEARSIZE))
            //  EX_ERROR(_T("No size specified for a compressed image"));

            m_iSize = pHeader->dwPitch;

            switch (pHeader->PixelFormat.dwFourCC)
            {
            case DXT1_NUMBER:
                m_iBMPType = BITMAP_DXT1;
                break;

            case DXT2_NUMBER:
                m_iBMPType = BITMAP_DXT2;
                break;

            case DXT3_NUMBER:
                m_iBMPType = BITMAP_DXT3;
                break;

            case DXT4_NUMBER:
                m_iBMPType = BITMAP_DXT4;
                break;

            case DXT5_NUMBER:
                m_iBMPType = BITMAP_DXT5;
                break;

            default:
                EX_ERROR(_T("Invalid compression format"));
            }
        }
        else if (pHeader->PixelFormat.dwFlags & DDPF_RGB)
        {
            if (pHeader->PixelFormat.dwFlags & DDPF_ALPHAPIXELS)
                m_iBMPType = BITMAP_RGBA;
            else
                m_iBMPType = BITMAP_RGB;
        }
        else
        {
            EX_ERROR(_T("No pixel format specified"));
        }

        m_iRedChannel = BMP_CHANNEL_NULL;
        m_iGreenChannel = BMP_CHANNEL_NULL;
        m_iBlueChannel = BMP_CHANNEL_NULL;
        m_iAlphaChannel = BMP_CHANNEL_NULL;

        m_pData = K2_NEW_ARRAY(ctx_Bitmap, byte, uiSize);
        if (m_pData == nullptr)
            EX_ERROR(_T("Failed to allocate buffer"));

        MemManager.Copy(m_pData, pBuffer, uiSize);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CBitmap::LoadDDS(") + sFilename + _TS(") - "), NO_THROW);
        return false;
    }
}


/*====================
  PNG_Read
  ====================*/
void    PNG_Read(png_structp png_ptr, png_bytep data, png_size_t length)
{
    ((CFileHandle*)png_ptr->io_ptr)->Read((char*)data, int(length));
}


/*====================
  PNG_Write
  ====================*/
void    PNG_Write(png_structp pPNGStruct, png_bytep data, png_size_t length)
{
    ((CFileHandle*)pPNGStruct->io_ptr)->Write((char*)data, length);
}


/*====================
  PNG_Flush
  ====================*/
void    PNG_Flush(png_structp pPNGStruct)
{
    //((CFileHandle*)pPNGStruct->io_ptr)->Flush();
}


/*====================
  PNG_Malloc
  ====================*/
png_voidp   PNG_Malloc(png_structp pPNGStruct, png_size_t zSize)
{
    return K2_NEW_ARRAY(ctx_Bitmap, byte, zSize);
}


/*====================
  PNG_Free
  ====================*/
void    PNG_Free(png_structp pPNGStruct, png_voidp pMem)
{
    K2_DELETE_ARRAY(static_cast<byte*>(pMem));
}


/*====================
  PNG_Error
  ====================*/
void    PNG_Error(png_structp pPNGStruct, png_const_charp szMsg)
{
    TCHAR   szTemp[1024];
    MemManager.Set(szTemp, 0, 1024);
    SingleToTCHAR(szTemp, 1024, szMsg, 1023);

#if 1
#ifdef __GNUC__
    g_sErrorMsg = szTemp;
    longjmp(png_jmpbuf(pPNGStruct), 1);
#else
    EX_ERROR(tstring(szTemp));
#endif
#else
    Console.Warn << szMsg << newl;
#endif
}


/*====================
  PNG_Warn
  ====================*/
void    PNG_Warn(png_structp pPNGStruct, png_const_charp szMsg)
{
    Console.Warn << szMsg << newl;
}


/*====================
  CBitmap::LoadPNG
  ====================*/
bool    CBitmap::LoadPNG(const tstring &sFilename, bool bMonoAsAlpha)
{
    if (m_pData != nullptr)
        Free();

    if (sFilename.empty())
        return false;

    png_structp pPNGRead(nullptr);
    png_infop pPNGInfo(nullptr);
    try
    {
        CFileHandle file(sFilename, FILE_READ | FILE_BINARY | m_uiFileFlags);
        if (!file.IsOpen())
            EX_ERROR(_T("Could not open file"));

        // Create and initialize the png_struct
        pPNGRead = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, nullptr, PNG_Error, PNG_Warn, nullptr, PNG_Malloc, PNG_Free);
        if (pPNGRead == nullptr)
            EX_ERROR(_T("png_create_read_struct() failed"));

        // Set custom functions
        png_set_mem_fn(pPNGRead, nullptr, PNG_Malloc, PNG_Free);
        png_set_error_fn(pPNGRead, nullptr, PNG_Error, PNG_Warn);
        png_set_read_fn(pPNGRead, (png_voidp)&file, PNG_Read);

        // Allocate / initialize the memory for image information
        pPNGInfo = png_create_info_struct(pPNGRead);
        if (pPNGInfo == nullptr)
            EX_ERROR(_T("png_create_info_struct() failed"));

#ifdef __GNUC__
        if (setjmp(png_jmpbuf(pPNGRead)))
        {
            EX_ERROR(g_sErrorMsg);
        }
#endif

        uint png_transforms(PNG_TRANSFORM_EXPAND);
        bool needToUpdate(false);

        png_read_png(pPNGRead, pPNGInfo, png_transforms, nullptr);

        /* Expand paletted or RGB images with transparency to full alpha channels
        * so the data will be available as RGBA quartets.
        */
        if (png_get_valid(pPNGRead, pPNGInfo, PNG_INFO_tRNS))
        {
            png_set_tRNS_to_alpha(pPNGRead);
            needToUpdate = true;
        }

        if (pPNGInfo->color_type == PNG_COLOR_TYPE_PALETTE)
        {
            png_set_palette_to_rgb(pPNGRead);
            png_read_png(pPNGRead, pPNGInfo, png_transforms, nullptr);
            needToUpdate = true;
        }

        if (pPNGInfo->color_type == PNG_COLOR_TYPE_GRAY)
        {
            if (pPNGInfo->bit_depth < 8)
            {
                png_set_expand_gray_1_2_4_to_8(pPNGRead);

                png_read_png(pPNGRead, pPNGInfo, png_transforms, nullptr);
                needToUpdate = true;
            }
        }

        if (needToUpdate)
            png_read_update_info(pPNGRead, pPNGInfo);

        if (pPNGInfo->color_type == PNG_COLOR_TYPE_PALETTE)
            EX_ERROR(_T("Paletted PNG files are not supported"));

        m_iWidth = pPNGInfo->width;
        m_iHeight = pPNGInfo->height;

        switch (pPNGInfo->channels)
        {
        case 1: m_iBMPType = BITMAP_ALPHA; break;
        case 2: m_iBMPType = BITMAP_LUMINANCE_ALPHA; break;
        case 3: m_iBMPType = BITMAP_RGB; break;
        case 4: m_iBMPType = BITMAP_RGBA; break;
        default:
            EX_ERROR(_T("Unsupported number of channels ") + ParenStr(XtoA(pPNGInfo->channels)));
        }

        if (pPNGInfo->bit_depth != 8)
            EX_ERROR(_T("Unsupported bit depth ") + ParenStr(XtoA(pPNGInfo->bit_depth)));

        m_pData = K2_NEW_ARRAY(ctx_Bitmap, byte, pPNGInfo->channels*pPNGInfo->width*pPNGInfo->height);

        for (uint index(0); index < pPNGRead->num_rows; ++index)
        {
            MemManager.Copy(&m_pData[index * pPNGInfo->width * pPNGInfo->pixel_depth / pPNGInfo->bit_depth],
                    pPNGInfo->row_pointers[pPNGRead->num_rows - index - 1],
                    pPNGInfo->pixel_depth / pPNGInfo->bit_depth * pPNGInfo->width);
        }

        m_iSize = m_iBMPType * m_iWidth * m_iHeight;

        if (m_iBMPType == BITMAP_RGBA)
        {
            m_iRedChannel = 0;
            m_iGreenChannel = 1;
            m_iBlueChannel = 2;
            m_iAlphaChannel = 3;
        }
        else if (m_iBMPType == BITMAP_RGB)
        {
            m_iRedChannel = 0;
            m_iGreenChannel = 1;
            m_iBlueChannel = 2;
            m_iAlphaChannel = BMP_CHANNEL_WHITE;
        }
        else if (m_iBMPType == BITMAP_LUMINANCE_ALPHA)
        {
            m_iRedChannel = BMP_CHANNEL_BLACK;
            m_iGreenChannel = BMP_CHANNEL_BLACK;
            m_iBlueChannel = 0;
            m_iAlphaChannel = 1;
        }
        else if (m_iBMPType == BITMAP_ALPHA)
        {
            if (bMonoAsAlpha)
            {
                // Set mono channel as alpha
                m_iRedChannel = BMP_CHANNEL_WHITE;
                m_iGreenChannel = BMP_CHANNEL_WHITE;
                m_iBlueChannel = BMP_CHANNEL_WHITE;
                m_iAlphaChannel = 0;
            }
            else
            {
                // Duplicate mono channel into RGB
                m_iRedChannel = 0;
                m_iGreenChannel = 0;
                m_iBlueChannel = 0;
                m_iAlphaChannel = BMP_CHANNEL_WHITE;
            }
        }

        Console.Res << sFilename << _T(" - ") << m_iBMPType * m_iWidth * m_iHeight <<
            _T(" bytes (") << ((m_iBMPType == BITMAP_RGBA) ? _T("RGBA") : _T("RGB")) << _T(")") << newl;

        // Clean up after the read, and free any memory allocated
        png_destroy_read_struct(&pPNGRead, &pPNGInfo, nullptr);
    }
    catch (CException &ex)
    {
        if (pPNGRead != nullptr)
            png_destroy_read_struct(&pPNGRead, &pPNGInfo, nullptr);

        ex.Process(_T("CBitmap::LoadPNG() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CBitmap::LoadTGA

  0 no image data is present,
  1 uncompressed, color-mapped image,
  2 uncompressed, true-color image,
  3 uncompressed, black-and-white image,
  9 run-length encoded, color-mapped Image,
  10 run-length encoded, true-color image and,
  11 run-length encoded, black-and-white Image
  ====================*/
bool    CBitmap::LoadTGA(const tstring &sFilename, bool bMonoAsAlpha)
{
    if (m_pData)
        Free();

    if (sFilename.empty())
        return false;

    m_pData = nullptr;

    CFileHandle fBitmap(sFilename, FILE_READ | FILE_BINARY | m_uiFileFlags);
    if (!fBitmap.IsOpen())
        return false;

    TargaHeader     targa_header;
    byte            *targa_rgba;

    targa_header.id_length = fBitmap.ReadByte();
    targa_header.colormap_type = fBitmap.ReadByte();
    targa_header.image_type = fBitmap.ReadByte();

    targa_header.colormap_index = fBitmap.ReadInt16();
    targa_header.colormap_length = fBitmap.ReadInt16();
    targa_header.colormap_size = fBitmap.ReadByte();
    targa_header.x_origin = fBitmap.ReadInt16();
    targa_header.y_origin = fBitmap.ReadInt16();
    targa_header.width = fBitmap.ReadInt16();
    targa_header.height = fBitmap.ReadInt16();
    targa_header.pixel_size = fBitmap.ReadByte();
    targa_header.attributes = fBitmap.ReadByte();

    if (targa_header.image_type != 2 &&
        targa_header.image_type != 3 &&
        targa_header.image_type != 10)
    {
        Console.Err << _T("CBitmap::LoadTGA() - Only type 2, 3 and 10 images supported") << newl;
        return false;
    }

    if (targa_header.colormap_type != 0 ||
        (targa_header.pixel_size != 32 && targa_header.pixel_size != 24 && targa_header.pixel_size != 8))
    {
        Console.Err << _T("CBitmap::LoadTGA() - Only 8/24/32 bit images supported (no colormaps)") << newl;
        return false;
    }

    bool bInverted((targa_header.attributes & BIT(5)) != 0);

    int columns = targa_header.width;
    int rows = targa_header.height;
    int numPixels = columns * rows;
    byte    buf[4];

    int iOutputSize;

    if (targa_header.pixel_size == 8)
        iOutputSize = 1;
    else if (targa_header.pixel_size == 24)
        iOutputSize = 3;
    else if (targa_header.pixel_size == 32)
        iOutputSize = 4;
    else
    {
        Console.Err << _T("CBitmap::LoadTGA() - Unknown pixel size") << newl;
        return false;
    }

    targa_rgba = K2_NEW_ARRAY(ctx_Bitmap, byte, numPixels*iOutputSize);

    if (targa_header.id_length != 0)
    {
        if (targa_header.id_length < numPixels*(targa_header.pixel_size >> 3))
        {
            fBitmap.Read((char*)targa_rgba, targa_header.id_length);
        }
        else
        {
            Console.Err << _T("The size of the comment is larger than the size of the image data!") << newl;
            K2_DELETE_ARRAY(targa_rgba);
            return false;
        }
    }

    byte    *pBuffer;
    int     row, column;

    // Uncompressed, RGB images
    if (targa_header.image_type == 2 && targa_header.pixel_size == 24)
    {
        for (row = 0; row < rows; row++)
        {
            if (!bInverted)
                pBuffer = targa_rgba + row * columns * 3;
            else
                pBuffer = targa_rgba + (rows - row - 1) * columns * 3;

            for (column = 0; column < columns; ++column)
            {
                fBitmap.Read((char*)buf, 3);

                *pBuffer++ = buf[2];
                *pBuffer++ = buf[1];
                *pBuffer++ = buf[0];
            }
        }
    }
    // Uncompressed, RGBA images
    else if (targa_header.image_type == 2 && targa_header.pixel_size == 32)
    {
        for (row = 0; row < rows; row++)
        {
            if (!bInverted)
                pBuffer = targa_rgba + row * columns * 4;
            else
                pBuffer = targa_rgba + (rows - row - 1) * columns * 4;

            for (column = 0; column < columns; ++column)
            {
                fBitmap.Read((char*)buf, 4);

                *pBuffer++ = buf[2];
                *pBuffer++ = buf[1];
                *pBuffer++ = buf[0];
                *pBuffer++ = buf[3];
            }
        }
    }
    // Uncompressed, gray-scale images
    else if (targa_header.image_type == 3 && targa_header.pixel_size == 8)
    {
        for (row = 0; row < rows; row++)
        {
            if (!bInverted)
                pBuffer = targa_rgba + row * columns;
            else
                pBuffer = targa_rgba + (rows - row - 1) * columns;

            for (column = 0; column < columns; ++column)
            {
                fBitmap.Read((char*)buf, 1);
                *pBuffer++ = buf[0];
            }
        }
    }
    // Runlength encoded RGB images
    else if (targa_header.image_type == 10)
    {
        byte    packetHeader, packetSize, j;

        for (row = 0; row < rows; ++row)
        {
            if (!bInverted)
                pBuffer = targa_rgba + row * columns * (targa_header.pixel_size >> 3);
            else
                pBuffer = targa_rgba + (rows - row - 1) * columns * (targa_header.pixel_size >> 3);

            for (column = 0; column < columns; )
            {
                packetHeader = fBitmap.ReadByte();
                packetSize = 1 + (packetHeader & 0x7f);

                // run-length packet
                if (packetHeader & 0x80)
                {
                    byte    red, green, blue, alphabyte;

                    switch (targa_header.pixel_size)
                    {
                    case 24:
                        fBitmap.Read((char*)buf, 3);

                        blue = buf[0];
                        green = buf[1];
                        red = buf[2];
                        alphabyte = 255;
                        break;

                    case 32:
                    default:
                        fBitmap.Read((char*)buf, 4);

                        blue = buf[0];
                        green = buf[1];
                        red = buf[2];
                        alphabyte = buf[3];
                        break;
                    }

                    for (j = 0; j < packetSize; ++j)
                    {
                        *pBuffer++ = red;
                        *pBuffer++ = green;
                        *pBuffer++ = blue;

                        if (targa_header.pixel_size == 32)
                            *pBuffer++ = alphabyte;
                        ++column;

                        // run spans across rows
                        if (column == columns)
                        {
                            column = 0;
                            if (row < rows - 1)
                                ++row;
                            else
                                goto breakOut;
                            pBuffer = targa_rgba + row * columns * (targa_header.pixel_size >> 3);
                        }
                    }
                }
                // non run-length packet
                else
                {
                    for (j = 0; j < packetSize; ++j)
                    {
                        switch (targa_header.pixel_size)
                        {
                        case 24:
                            fBitmap.Read((char*)buf, 3);

                            *pBuffer++ = buf[2];
                            *pBuffer++ = buf[1];
                            *pBuffer++ = buf[0];
                            break;
                        case 32:
                            fBitmap.Read((char*)buf, 4);

                            *pBuffer++ = buf[2];
                            *pBuffer++ = buf[1];
                            *pBuffer++ = buf[0];
                            *pBuffer++ = buf[3];
                            break;
                        }
                        ++column;

                        // pixel packet run spans across rows
                        if (column == columns)
                        {
                            column = 0;
                            if (row < rows - 1)
                                ++row;
                            else
                                goto breakOut;
                            pBuffer = targa_rgba + row * columns * (targa_header.pixel_size >> 3);
                        }
                    }
                }
            }
            breakOut:;
        }
    }
    else
    {
        Console.Err << _T("CBitmap::LoadTGA() - Unsupported image type") << newl;
        return false;
    }


    m_iWidth = columns;
    m_iHeight = rows;

    int iPixelSize(targa_header.pixel_size);
    if (iPixelSize == 32)
        m_iBMPType = BITMAP_RGBA;
    else if (iPixelSize == 24)
        m_iBMPType = BITMAP_RGB;
    else if (iPixelSize == 8)
        m_iBMPType = BITMAP_ALPHA;
    else
        m_iBMPType = BITMAP_NULL;

    m_pData = targa_rgba;
    m_iSize = m_iWidth * m_iHeight * m_iBMPType;

    if (m_iBMPType == BITMAP_RGBA)
    {
        m_iRedChannel = 0;
        m_iGreenChannel = 1;
        m_iBlueChannel = 2;
        m_iAlphaChannel = 3;
    }
    else if (m_iBMPType == BITMAP_RGB)
    {
        m_iRedChannel = 0;
        m_iGreenChannel = 1;
        m_iBlueChannel = 2;
        m_iAlphaChannel = BMP_CHANNEL_WHITE;
    }
    else if (m_iBMPType == BITMAP_LUMINANCE_ALPHA)
    {
        m_iRedChannel = BMP_CHANNEL_BLACK;
        m_iGreenChannel = BMP_CHANNEL_BLACK;
        m_iBlueChannel = 0;
        m_iAlphaChannel = 1;
    }
    else if (m_iBMPType == BITMAP_ALPHA)
    {
        if (bMonoAsAlpha)
        {
            // Set mono channel as alpha
            m_iRedChannel = BMP_CHANNEL_WHITE;
            m_iGreenChannel = BMP_CHANNEL_WHITE;
            m_iBlueChannel = BMP_CHANNEL_WHITE;
            m_iAlphaChannel = 0;
        }
        else
        {
            // Duplicate mono channel into RGB
            m_iRedChannel = 0;
            m_iGreenChannel = 0;
            m_iBlueChannel = 0;
            m_iAlphaChannel = BMP_CHANNEL_WHITE;
        }
    }

    Console.Res << sFilename << _T(" - ") << m_iBMPType * m_iWidth * m_iHeight <<
        _T(" bytes (") << ((m_iBMPType == BITMAP_RGBA) ? _T("RGBA") : _T("RGB")) << _T(")") << newl;

    return true;
}

/*====================
  CBitmap::GenerateThumbnail
  ====================*/
void    CBitmap::GenerateThumbnail(const tstring &sFilename, int iSize)
{
    tstring sThumbnailFilename = _T("~/") + Filename_GetPath(sFilename) + _T("/thumbnails/") +
        Filename_StripPath(sFilename) + _T(".thumb");

    if (FileManager.Exists(sThumbnailFilename))
        return;  //don't recreate the thumbnail  (FIXME: add in a date check to see if the file was modified?)

    CBitmap bmp;

    if (!bmp.Load(sFilename))
    {
        Console.Dev << _T("Failure trying to load ") << SingleQuoteStr(sFilename) << _T(" for thumbnail generation") << newl;
        return;
    }

    CBitmap thumbnail;

    bmp.Scale(thumbnail, iSize, iSize);

    thumbnail.WritePNG(sThumbnailFilename);
}


/*====================
  CBitmap::GenerateThumbnails

  look through all images in a directory and generate thumbnails to be used by the thumbnailgrid
  ====================*/
void    CBitmap::GenerateThumbnails(const tstring &sDirname, int iSize)
{
    tsvector vFileList;
    vFileList.clear();
    FileManager.GetFileList(sDirname, _T("*.*"), false, vFileList, true);

    for (tsvector::iterator it = vFileList.begin(); it != vFileList.end(); ++it)
        CBitmap::GenerateThumbnail(*it, iSize);
}



/*--------------------
  cmdCreateThumbs
  --------------------*/
CMD(CreateThumbs)
{
    if (vArgList.size() < 1)
        return false;

    CBitmap::GenerateThumbnails(vArgList[0], 32);
    return true;
}


/*====================
  CBitmap::GetColor
  ====================*/
void    CBitmap::GetColor(int x, int y, bvec4_t color)
{
    try
    {
        if (m_pData == nullptr)
            EX_WARN(_T("No data in bitmap"));

        if (m_iBMPType > BITMAP_RGBA)
            EX_WARN(_T("Invalid action for compressed bitmap"));

        color[0] = m_pData[(m_iWidth * y + x) * m_iBMPType];
        if (m_iBMPType > 1)
            color[1] = m_pData[(m_iWidth * y + x) * m_iBMPType + 1];
        if (m_iBMPType > 2)
            color[2] = m_pData[(m_iWidth * y + x) * m_iBMPType + 2];
        if (m_iBMPType > 3)
            color[3] = m_pData[(m_iWidth * y + x) * m_iBMPType + 3];
    }
    catch (CException &ex)
    {
        ex.Process(_T("CBitmap::GetColor() - "));
    }
}


/*====================
  CBitmap::GetColor
  ====================*/
CVec4b  CBitmap::GetColor(int x, int y)
{
    try
    {
        if (m_pData == nullptr)
            EX_WARN(_T("No data in bitmap"));

        if (m_iBMPType > BITMAP_RGBA)
            EX_WARN(_T("Invalid action for compressed bitmap"));

        CVec4b color;

        color[0] = m_pData[(m_iWidth * y + x) * m_iBMPType];
        if (m_iBMPType > 1)
            color[1] = m_pData[(m_iWidth * y + x) * m_iBMPType + 1];
        if (m_iBMPType > 2)
            color[2] = m_pData[(m_iWidth * y + x) * m_iBMPType + 2];
        if (m_iBMPType > 3)
            color[3] = m_pData[(m_iWidth * y + x) * m_iBMPType + 3];

        return color;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CBitmap::GetColor() - "));
        return CVec4b(0, 0, 0, 0);
    }
}


/*====================
  CBitmap::GetAverageColor
  ====================*/
CVec4b  CBitmap::GetAverageColor()
{
    try
    {
        if (m_pData == nullptr)
            EX_WARN(_T("No data in bitmap"));

        if (m_iBMPType > BITMAP_RGBA)
            EX_WARN(_T("Invalid action for compressed bitmap"));

        CVec4b v4Color;

        CVec4ll v4Tmp(0, 0, 0, 0);
        for (int n(0); n < m_iWidth * m_iHeight * m_iBMPType; n += m_iBMPType)
        {
            for (int iComponent = 0; iComponent < m_iBMPType; ++iComponent)
                v4Tmp[iComponent] += m_pData[n + iComponent];
        }

        for (int iComponent = 0; iComponent < m_iBMPType; ++iComponent)
            v4Color[iComponent] = byte(v4Tmp[iComponent] / (m_iWidth * m_iHeight));

        return v4Color;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CBitmap::GetAverageColor() - "));
        return CVec4b(255, 255, 255, 255);
    }
}


/*====================
  CBitmap::Alloc
  ====================*/
void    CBitmap::Alloc(int iWidth, int iHeight, int iBMPType)
{
    if (m_pData != nullptr)
        Free();

    bool bGrayscale(false);

    if (iBMPType == BITMAP_GRAYSCALE)
    {
        iBMPType = BITMAP_ALPHA;
        bGrayscale = true;
    }

    if (iBMPType > 4 && iBMPType != BITMAP_RGBA16F && iBMPType != BITMAP_RGBA32F)
    {
        m_iWidth = 0;
        m_iHeight = 0;
        m_iBMPType = BITMAP_NULL;
        m_iSize = 0;

        m_iRedChannel = BMP_CHANNEL_NULL;
        m_iGreenChannel = BMP_CHANNEL_NULL;
        m_iBlueChannel = BMP_CHANNEL_NULL;
        m_iAlphaChannel = BMP_CHANNEL_NULL;
        return;
    }

    m_iSize = iWidth * iHeight * iBMPType;
    m_pData = K2_NEW_ARRAY(ctx_Bitmap, byte, m_iSize);
    m_iBMPType = iBMPType;
    m_iWidth = iWidth;
    m_iHeight = iHeight;

    if (m_iBMPType == BITMAP_RGBA)
    {
        m_iRedChannel = 0;
        m_iGreenChannel = 1;
        m_iBlueChannel = 2;
        m_iAlphaChannel = 3;
    }
    else if (m_iBMPType == BITMAP_RGB)
    {
        m_iRedChannel = 0;
        m_iGreenChannel = 1;
        m_iBlueChannel = 2;
        m_iAlphaChannel = BMP_CHANNEL_WHITE;
    }
    else if (m_iBMPType == BITMAP_LUMINANCE_ALPHA)
    {
        m_iRedChannel = BMP_CHANNEL_BLACK;
        m_iGreenChannel = BMP_CHANNEL_BLACK;
        m_iBlueChannel = 0;
        m_iAlphaChannel = 1;
    }
    else if (m_iBMPType == BITMAP_ALPHA)
    {
        if (bGrayscale)
        {
            m_iRedChannel = 0;
            m_iGreenChannel = 0;
            m_iBlueChannel = 0;
            m_iAlphaChannel = BMP_CHANNEL_WHITE;
        }
        else
        {
            m_iRedChannel = BMP_CHANNEL_WHITE;
            m_iGreenChannel = BMP_CHANNEL_WHITE;
            m_iBlueChannel = BMP_CHANNEL_WHITE;
            m_iAlphaChannel = 0;
        }
    }
    else
    {
        m_iRedChannel = BMP_CHANNEL_NULL;
        m_iGreenChannel = BMP_CHANNEL_NULL;
        m_iBlueChannel = BMP_CHANNEL_NULL;
        m_iAlphaChannel = BMP_CHANNEL_NULL;
    }
}


/*====================
  CBitmap::Alloc
  ====================*/
void    CBitmap::Alloc(int iWidth, int iHeight, int iBMPType, int iRedChannel, int iGreenChannel, int iBlueChannel, int iAlphaChannel)
{
    if (m_pData != nullptr)
        Free();

    m_iSize = iWidth * iHeight * iBMPType;
    m_pData = K2_NEW_ARRAY(ctx_Bitmap, byte, m_iSize);
    m_iBMPType = iBMPType;
    m_iWidth = iWidth;
    m_iHeight = iHeight;

    m_iRedChannel = iRedChannel;
    m_iGreenChannel = iGreenChannel;
    m_iBlueChannel = iBlueChannel;
    m_iAlphaChannel = iAlphaChannel;
}


/*====================
  CBitmap::AllocCompressed
  ====================*/
void    CBitmap::AllocCompressed(int iSize, int iWidth, int iHeight, int iBMPType)
{
    if (m_pData != nullptr)
        Free();

    m_iSize = iSize;
    m_pData = K2_NEW_ARRAY(ctx_Bitmap, byte, m_iSize);
    m_iBMPType = iBMPType;
    m_iWidth = iWidth;
    m_iHeight = iHeight;

    m_iRedChannel = BMP_CHANNEL_NULL;
    m_iGreenChannel = BMP_CHANNEL_NULL;
    m_iBlueChannel = BMP_CHANNEL_NULL;
    m_iAlphaChannel = BMP_CHANNEL_NULL;
}


/*====================
  CBitmap::Free
  ====================*/
void    CBitmap::Free()
{
    SAFE_DELETE_ARRAY(m_pData);
}


/*====================
  CBitmap::WritePNG

  write a png file
  ====================*/
bool    CBitmap::WritePNG(const tstring &sFilename)
{
    png_structp pPNGWrite(nullptr);
    png_infop   pPNGInfo(nullptr);
    try
    {
        // Open the file
        CFileHandle hPNGFile(sFilename, FILE_WRITE | FILE_BINARY);
        if (!hPNGFile.IsOpen())
            EX_ERROR(_T("Could not open file."));

        // Create and initialize the png_struct
        pPNGWrite = png_create_write_struct_2(PNG_LIBPNG_VER_STRING, nullptr, PNG_Error, PNG_Warn, nullptr, PNG_Malloc, PNG_Free);
        if (pPNGWrite == nullptr)
            EX_ERROR(_T("png_create_write_struct failed."));
        
#ifdef __GNUC__
        if (setjmp(png_jmpbuf(pPNGWrite)))
        {
            EX_ERROR(g_sErrorMsg);
        }
#endif

        // Set custom functions
        png_set_mem_fn(pPNGWrite, nullptr, PNG_Malloc, PNG_Free);
        png_set_error_fn(pPNGWrite, nullptr, PNG_Error, PNG_Warn);
        png_set_write_fn(pPNGWrite, (png_voidp)&hPNGFile, PNG_Write, PNG_Flush);
        
        // Allocate / initialize the image information data.
        pPNGInfo = png_create_info_struct(pPNGWrite);
        if (pPNGInfo == nullptr)
            EX_ERROR(_T("png_create_info_struct failed."));

        int iColorType, iChannels;
        switch(m_iBMPType)
        {
        case BITMAP_ALPHA:
            iColorType = PNG_COLOR_TYPE_GRAY;
            iChannels = 1;
            break;

        case BITMAP_LUMINANCE_ALPHA:
            iColorType = PNG_COLOR_TYPE_GA;
            iChannels = 2;
            break;

        case BITMAP_RGB:
            iColorType = PNG_COLOR_TYPE_RGB;
            iChannels = 3;
            break;

        case BITMAP_RGBA:
            iColorType = PNG_COLOR_TYPE_RGBA;
            iChannels = 4;
            break;

        case BITMAP_DXT1:
        case BITMAP_DXT2:
        case BITMAP_DXT3:
        case BITMAP_DXT4:
        case BITMAP_DXT5:
            EX_ERROR(_T("Can't write from a compressed bitmap format"));

        default:
            EX_ERROR(_TS("Unknown BMPType ") + ParenStr(XtoA(m_iBMPType)));
        }

        // HACK: The bit depth parameter would have always been 8 before...
        png_set_IHDR(pPNGWrite, pPNGInfo, m_iWidth, m_iHeight, 8 /*m_iMode / iChannels*/, iColorType,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        pPNGInfo->valid = PNG_INFO_IDAT;

        pPNGInfo->row_pointers = (png_bytepp)png_malloc(pPNGWrite, sizeof(png_bytep) * (m_iHeight + 1));
        pPNGInfo->row_pointers[m_iHeight] = 0;
        for (int k = 0; k < m_iHeight; ++k)
            pPNGInfo->row_pointers[k] = m_pData + k * m_iWidth * iChannels;

        png_write_png(pPNGWrite, pPNGInfo, PNG_TRANSFORM_IDENTITY, nullptr);

        // Clean up after the write, and free any memory allocated
        png_destroy_write_struct(&pPNGWrite, &pPNGInfo);
    }
    catch(CException &ex)
    {
        if (pPNGWrite != nullptr)
            png_destroy_write_struct(&pPNGWrite, &pPNGInfo);

        ex.Process(_TS("CBitmap::WritePNG(") + sFilename + _TS(") - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CBitmap::WriteTGA

  write a tga file
  ====================*/
bool    CBitmap::WriteTGA(const tstring &sFilename)
{
    try
    {
        // Open the file
        CFileHandle hOutFile(sFilename, FILE_WRITE | FILE_BINARY);
        if (!hOutFile.IsOpen())
            EX_ERROR(_T("Could not open file"));

        if (m_iBMPType != BITMAP_RGB && m_iBMPType != BITMAP_RGBA)
            EX_ERROR(_T("Unsupported type"));

        hOutFile.WriteByte(byte(0)); // id_length
        hOutFile.WriteByte(byte(0)); // colormap_type
        hOutFile.WriteByte(byte(2)); // image_type (2 uncompressed true-color)

        hOutFile.WriteInt16(ushort(0)); // colormap_index
        hOutFile.WriteInt16(ushort(0)); // colormap_length
        hOutFile.WriteByte(byte(0)); // colormap_size

        hOutFile.WriteInt16(ushort(0)); // x_origin
        hOutFile.WriteInt16(ushort(0)); // y_origin
        hOutFile.WriteInt16(LittleShort(ushort(m_iWidth))); // width
        hOutFile.WriteInt16(LittleShort(ushort(m_iHeight))); // height

        if (m_iBMPType == BITMAP_RGB)
        {
            hOutFile.WriteByte(byte(24)); // pixel_size
            hOutFile.WriteByte(byte(0)); // attributes

            byte *p(&m_pData[0]);
            for (uint uiPixel(m_iWidth * m_iHeight); uiPixel != 0; --uiPixel, p += 3)
            {
                hOutFile.WriteByte(p[2]);
                hOutFile.WriteByte(p[1]);
                hOutFile.WriteByte(p[0]);
            }
        }
        else if (m_iBMPType == BITMAP_RGBA)
        {
            hOutFile.WriteByte(byte(32)); // pixel_size
            hOutFile.WriteByte(byte(8)); // attributes

            byte *p(&m_pData[0]);
            for (uint uiPixel(m_iWidth * m_iHeight); uiPixel != 0; --uiPixel, p += 4)
            {
                hOutFile.WriteByte(p[2]);
                hOutFile.WriteByte(p[1]);
                hOutFile.WriteByte(p[0]);
                hOutFile.WriteByte(p[3]);
            }
        }
    }
    catch(CException &ex)
    {
        ex.Process(_TS("CBitmap::WriteTGA(") + sFilename + _TS(") - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CBitmap::Flip
  ====================*/
void    CBitmap::Flip()
{
    byte *pBuffer(nullptr);

    try
    {
        if (m_iBMPType > BITMAP_RGBA)
            EX_ERROR(_T("Cannot flip a compressed bitmap"));

        int iScanlineSize(m_iBMPType * m_iWidth);
        pBuffer = K2_NEW_ARRAY(ctx_Bitmap, byte, iScanlineSize);
        if (pBuffer == nullptr)
            EX_ERROR(_T("Failed to allocate buffer"));

        int half(m_iHeight >> 1);
        for (int i(0); i < half; ++i)
        {
            MemManager.Move(pBuffer, &m_pData[i * iScanlineSize], iScanlineSize);
            MemManager.Move(&m_pData[i * iScanlineSize], &m_pData[(m_iHeight - i-1) * iScanlineSize], iScanlineSize);
            MemManager.Move(&m_pData[(m_iHeight - i-1) * iScanlineSize], pBuffer, iScanlineSize);
        }

        K2_DELETE_ARRAY(pBuffer);
    }
    catch (CException &ex)
    {
        if (pBuffer != nullptr)
            K2_DELETE_ARRAY(pBuffer);
        ex.Process(_T("CBitmap::Flip() - "));
    }
}


/*====================
  CBitmap::GetColor4f
  ====================*/
void    CBitmap::GetColor4f(float x, float y, float &r, float &g, float &b, float &a) const
{
    if (m_iBMPType > BITMAP_RGBA)
    {
        Console.Warn << _T("CBitmap::GetColor4f() - Invalid operation for compressed bitmap") << newl;
        return;
    }

    float tmp_r, tmp_g, tmp_b, tmp_a;
    float xdec, ydec;

    if (x + 1 >= m_iWidth || y + 1 >= m_iHeight)
    {
        r = m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x)) * m_iBMPType];
        g = m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x)) * m_iBMPType + 1];
        b = m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x)) * m_iBMPType + 2];
        if (m_iBMPType == BITMAP_RGBA)
            a = m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x))*m_iBMPType + 3];
        return;
    }

    xdec = 1 - (x - static_cast<int>(x));
    ydec = 1 - (y - static_cast<int>(y));

    tmp_r  = m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x))*m_iBMPType] * ((xdec + ydec) / 2 );
    tmp_r += m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x) + 1)*m_iBMPType] * (((1 - xdec) + ydec) / 2);
    tmp_r += m_pData[(static_cast<int>(y + 1) * m_iWidth + static_cast<int>(x))*m_iBMPType] * ((xdec + (1 - ydec)) / 2 );
    tmp_r += m_pData[(static_cast<int>(y + 1) * m_iWidth + static_cast<int>(x) + 1)*m_iBMPType] * (((1 - xdec) + (1 - ydec)) / 2 );

    tmp_g  = m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x))*m_iBMPType + 1] * ((xdec + ydec) / 2 );
    tmp_g += m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x) + 1)*m_iBMPType + 1] * (((1 - xdec) + ydec) / 2 );
    tmp_g += m_pData[(static_cast<int>(y + 1) * m_iWidth + static_cast<int>(x))*m_iBMPType + 1] * ((xdec + (1 - ydec)) / 2 );
    tmp_g += m_pData[(static_cast<int>(y + 1) * m_iWidth + static_cast<int>(x) + 1)*m_iBMPType + 1] * (((1 - xdec) + (1 - ydec)) / 2 );

    tmp_b  = m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x))*m_iBMPType + 2] * ((xdec + ydec) / 2 );
    tmp_b += m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x) + 1)*m_iBMPType + 2] * (((1 - xdec) + ydec) / 2 );
    tmp_b += m_pData[(static_cast<int>(y + 1) * m_iWidth + static_cast<int>(x))*m_iBMPType + 2] * ((xdec + (1 - ydec)) / 2 );
    tmp_b += m_pData[(static_cast<int>(y + 1) * m_iWidth + static_cast<int>(x) + 1)*m_iBMPType + 2] * (((1 - xdec) + (1 - ydec)) / 2 );

    if (m_iBMPType == BITMAP_RGBA)
    {
        tmp_a  = m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x))*m_iBMPType + 3] * ((xdec + ydec) / 2 );
        tmp_a += m_pData[(static_cast<int>(y) * m_iWidth + static_cast<int>(x) + 1)*m_iBMPType + 3] * (((1 - xdec) + ydec) / 2 );
        tmp_a += m_pData[(static_cast<int>(y + 1) * m_iWidth + static_cast<int>(x))*m_iBMPType + 3] * ((xdec + (1 - ydec)) / 2 );
        tmp_a += m_pData[(static_cast<int>(y + 1) * m_iWidth + static_cast<int>(x) + 1)*m_iBMPType + 3] * (((1 - xdec) + (1 - ydec)) / 2 );

        a = tmp_a / 2.0f;
    }

    r = tmp_r / 2.0f;
    g = tmp_g / 2.0f;
    b = tmp_b / 2.0f;
}


/*====================
  CBitmap::ScaleUp
  ====================*/
void    CBitmap::ScaleUp(CBitmap &out, int iWidth, int iHeight) const
{
    if (m_iBMPType > BITMAP_RGBA32F)
    {
        Console.Warn << _T("CBitmap::ScaleUp() - Invalid operation for compressed bitmap") << newl;
        return;
    }

    float x, y, xstep, ystep;
    float avg_r, avg_g, avg_b, avg_a;
    int offset;

    ystep = static_cast<float>(iHeight)/m_iHeight;
    xstep = static_cast<float>(iWidth)/m_iWidth;

    out.Alloc(iWidth, iHeight, m_iBMPType);

    for (int j(0); j < iHeight; ++j)
    {
        y = j / ystep;

        for (int i(0); i < iWidth; ++i)
        {
            x = i / xstep;

            offset = (j * iWidth + i) * m_iBMPType;

            GetColor4f(x, y, avg_r, avg_g, avg_b, avg_a);

            out.m_pData[offset] = static_cast<byte>(avg_r);
            if (m_iBMPType >= 2)
                out.m_pData[offset + 1] = static_cast<byte>(avg_g);
            if (m_iBMPType >= 3)
                out.m_pData[offset + 2] = static_cast<byte>(avg_b);
            if (m_iBMPType >= 4)
                out.m_pData[offset + 3] = static_cast<byte>(avg_a);
        }
    }
}


/*====================
  CBitmap::ScaleDown
  ====================*/
void    CBitmap::ScaleDown(CBitmap &out, int iWidth, int iHeight) const
{
    if (m_iBMPType > BITMAP_RGBA32F)
    {
        Console.Warn << _T("CBitmap::ScaleDown() - Invalid operation for compressed bitmap") << newl;
        return;
    }

    float xstep, ystep;
    float avg_r, avg_g, avg_b, avg_a;
    int tmp_x, tmp_y, i, j, offset, x, y;

    ystep = static_cast<float>(m_iHeight)/iHeight;
    xstep = static_cast<float>(m_iWidth)/iWidth;

    out.Alloc(iWidth, iHeight, m_iBMPType);

    if (m_iBMPType == BITMAP_RGBA32F)
    {
        float *pFloatData(reinterpret_cast<float *>(m_pData));

        for (j = 0; j < iHeight; ++j)
        {
            for (i = 0; i < iWidth; ++i)
            {
                x = INT_ROUND(i * xstep);
                y = INT_ROUND(j * ystep);

                avg_r = avg_g = avg_b = avg_a = 0;
                tmp_x = 0; //in case ystep <= 0, this won't crash
                for (tmp_y = 0;  tmp_y < ystep; ++tmp_y)
                {
                    for (tmp_x = 0;  tmp_x < xstep; ++tmp_x)
                    {
                        offset = (((y + tmp_y) * m_iWidth) + x + tmp_x) * 4;
                        avg_r += pFloatData[offset];
                        avg_g += pFloatData[offset + 1];
                        avg_b += pFloatData[offset + 2];
                        avg_a += pFloatData[offset + 3];
                    }
                }

                avg_r /= tmp_x * tmp_y;
                avg_g /= tmp_x * tmp_y;
                avg_b /= tmp_x * tmp_y;
                avg_a /= tmp_x * tmp_y;

                offset = (j * iWidth + i) * 4;

                float *pOutFloatData(reinterpret_cast<float *>(out.m_pData));

                pOutFloatData[offset] = avg_r;
                pOutFloatData[offset + 1] = avg_g;
                pOutFloatData[offset + 2] = avg_b;
                pOutFloatData[offset + 3] = avg_a;
            }
        }
    }
    else
    {
        for (j = 0; j < iHeight; ++j)
        {
            for (i = 0; i < iWidth; ++i)
            {
                x = INT_FLOOR(i * xstep);
                y = INT_FLOOR(j * ystep);

                avg_r = avg_g = avg_b = avg_a = 0;
                tmp_x = 0; //in case ystep <= 0, this won't crash
                for (tmp_y = 0;  tmp_y < ystep; ++tmp_y)
                {
                    for (tmp_x = 0;  tmp_x < xstep; ++tmp_x)
                    {
                        offset = (((y + tmp_y) * m_iWidth) + x + tmp_x) * m_iBMPType;

                        avg_r += m_pData[offset];

                        if (m_iBMPType >= 2)
                            avg_g += m_pData[offset + 1];
                        if (m_iBMPType >= 3)
                            avg_b += m_pData[offset + 2];
                        if (m_iBMPType >= 4)
                            avg_a += m_pData[offset + 3];
                    }
                }

                avg_r /= tmp_x * tmp_y;
                avg_g /= tmp_x * tmp_y;
                avg_b /= tmp_x * tmp_y;
                avg_a /= tmp_x * tmp_y;

                offset = (j * iWidth + i) * m_iBMPType;

                out.m_pData[offset] = BYTE_ROUND(avg_r);
                if (m_iBMPType >= 2)
                    out.m_pData[offset + 1] = BYTE_ROUND(avg_g);
                if (m_iBMPType >= 3)
                    out.m_pData[offset + 2] = BYTE_ROUND(avg_b);
                if (m_iBMPType >= 4)
                    out.m_pData[offset + 3] = BYTE_ROUND(avg_a);
            }
        }
    }
}


/*====================
  CBitmap::Scale
  ====================*/
void    CBitmap::Scale(CBitmap &out, int iWidth, int iHeight) const
{
    if (!m_pData)
        return;

    if (iHeight >= m_iHeight && iWidth >= m_iWidth)
    {
        ScaleUp(out, iWidth, iHeight);
    }
    else if (iHeight <= m_iHeight && iWidth <= m_iWidth)
    {
        ScaleDown(out, iWidth, iHeight);
    }
    else if (iHeight <= m_iHeight && iWidth > m_iWidth)
    {
        CBitmap tmp_bmp;

        ScaleUp(tmp_bmp, iWidth, m_iHeight);
        tmp_bmp.ScaleDown(out, m_iWidth, iHeight);
    }
    else if (iHeight > m_iHeight && iWidth <= m_iWidth)
    {
        CBitmap tmp_bmp;

        ScaleUp(tmp_bmp, m_iWidth, iHeight);
        tmp_bmp.ScaleDown(out, iWidth, m_iHeight);
    }

    out.m_iRedChannel = m_iRedChannel;
    out.m_iGreenChannel = m_iGreenChannel;
    out.m_iBlueChannel = m_iBlueChannel;
    out.m_iAlphaChannel = m_iAlphaChannel;
}


/*====================
  CBitmap::ScaleDown
  ====================*/
void    CBitmap::ScaleDown(CBitmap &out, int iWidth, int iHeight, float fGamma) const
{
    if (m_iBMPType > BITMAP_RGBA32F)
    {
        Console.Warn << _T("CBitmap::GammaScaleDown() - Invalid operation for compressed bitmap") << newl;
        return;
    }

    float xstep, ystep;
    float avg_r, avg_g, avg_b, avg_a;
    int tmp_x, tmp_y, i, j, offset, x, y;

    ystep = static_cast<float>(m_iHeight) / iHeight;
    xstep = static_cast<float>(m_iWidth) / iWidth;

    out.Alloc(iWidth, iHeight, m_iBMPType);

    if (m_iBMPType == BITMAP_RGBA32F)
    {
        float *pFloatData(reinterpret_cast<float *>(m_pData));

        for (j = 0; j < iHeight; ++j)
        {
            for (i = 0; i < iWidth; ++i)
            {
                x = INT_ROUND(i * xstep);
                y = INT_ROUND(j * ystep);

                avg_r = avg_g = avg_b = avg_a = 0;
                tmp_x = 0; //in case ystep <= 0, this won't crash
                for (tmp_y = 0;  tmp_y < ystep; ++tmp_y)
                {
                    for (tmp_x = 0;  tmp_x < xstep; ++tmp_x)
                    {
                        offset = (((y + tmp_y) * m_iWidth) + x + tmp_x) * 4;
                        avg_r += pFloatData[offset];
                        avg_g += pFloatData[offset + 1];
                        avg_b += pFloatData[offset + 2];
                        avg_a += pFloatData[offset + 3];
                    }
                }

                avg_r /= tmp_x * tmp_y;
                avg_g /= tmp_x * tmp_y;
                avg_b /= tmp_x * tmp_y;
                avg_a /= tmp_x * tmp_y;

                offset = (j * iWidth + i) * 4;

                float *pOutFloatData(reinterpret_cast<float *>(out.m_pData));

                pOutFloatData[offset] = avg_r;
                pOutFloatData[offset + 1] = avg_g;
                pOutFloatData[offset + 2] = avg_b;
                pOutFloatData[offset + 3] = avg_a;
            }
        }
    }
    else
    {
        float fInvGamma(1.0f / fGamma);

        for (j = 0; j < iHeight; ++j)
        {
            for (i = 0; i < iWidth; ++i)
            {
                x = INT_FLOOR(i * xstep);
                y = INT_FLOOR(j * ystep);

                avg_r = avg_g = avg_b = avg_a = 0;
                tmp_x = 0; //in case ystep <= 0, this won't crash
                for (tmp_y = 0;  tmp_y < ystep; ++tmp_y)
                {
                    for (tmp_x = 0;  tmp_x < xstep; ++tmp_x)
                    {
                        offset = (((y + tmp_y) * m_iWidth) + x + tmp_x) * m_iBMPType;

                        avg_r += pow(float(m_pData[offset]), fGamma);

                        if (m_iBMPType >= 2)
                            avg_g += pow(float(m_pData[offset + 1]), fGamma);
                        if (m_iBMPType >= 3)
                            avg_b += pow(float(m_pData[offset + 2]), fGamma);
                        if (m_iBMPType >= 4)
                            avg_a += float(m_pData[offset + 3]);
                    }
                }

                avg_r /= tmp_x * tmp_y;
                avg_g /= tmp_x * tmp_y;
                avg_b /= tmp_x * tmp_y;
                avg_a /= tmp_x * tmp_y;

                avg_r = pow(avg_r, fInvGamma);
                avg_g = pow(avg_g, fInvGamma);
                avg_b = pow(avg_b, fInvGamma);

                offset = (j * iWidth + i) * m_iBMPType;

                out.m_pData[offset] = BYTE_ROUND(avg_r);
                if (m_iBMPType >= 2)
                    out.m_pData[offset + 1] = BYTE_ROUND(avg_g);
                if (m_iBMPType >= 3)
                    out.m_pData[offset + 2] = BYTE_ROUND(avg_b);
                if (m_iBMPType >= 4)
                    out.m_pData[offset + 3] = BYTE_ROUND(avg_a);
            }
        }
    }
}


/*====================
  CBitmap::Scale
  ====================*/
void    CBitmap::Scale(CBitmap &out, int iWidth, int iHeight, float fGamma) const
{
    if (!m_pData)
        return;

    if (iHeight >= m_iHeight && iWidth >= m_iWidth)
    {
        ScaleUp(out, iWidth, iHeight);
    }
    else if (iHeight <= m_iHeight && iWidth <= m_iWidth)
    {
        ScaleDown(out, iWidth, iHeight, fGamma);
    }
    else if (iHeight <= m_iHeight && iWidth > m_iWidth)
    {
        CBitmap tmp_bmp;

        ScaleUp(tmp_bmp, iWidth, m_iHeight);
        tmp_bmp.ScaleDown(out, m_iWidth, iHeight);
    }
    else if (iHeight > m_iHeight && iWidth <= m_iWidth)
    {
        CBitmap tmp_bmp;

        ScaleUp(tmp_bmp, m_iWidth, iHeight);
        tmp_bmp.ScaleDown(out, iWidth, m_iHeight);
    }

    out.m_iRedChannel = m_iRedChannel;
    out.m_iGreenChannel = m_iGreenChannel;
    out.m_iBlueChannel = m_iBlueChannel;
    out.m_iAlphaChannel = m_iAlphaChannel;
}


/*====================
  CBitmap::Copy
  ====================*/
void    CBitmap::Copy(CBitmap &out) const
{
    if (!m_pData)
        return;

    if (out.m_iSize != m_iSize)
    {
        out.Free();
        
        out.m_iSize = m_iSize;
        out.m_pData = K2_NEW_ARRAY(ctx_Bitmap, byte, m_iSize);
    }

    out.m_iWidth = m_iWidth;
    out.m_iHeight = m_iHeight;
    out.m_iBMPType = m_iBMPType;

    MemManager.Copy(out.m_pData, m_pData, m_iSize);

    out.m_iRedChannel = m_iRedChannel;
    out.m_iGreenChannel = m_iGreenChannel;
    out.m_iBlueChannel = m_iBlueChannel;
    out.m_iAlphaChannel = m_iAlphaChannel;
}


/*====================
  CBitmap::SetPixel4b
  ====================*/
void    CBitmap::SetPixel4b(int x, int y, byte r, byte g, byte b, byte a)
{
    if (m_iBMPType > BITMAP_RGBA)
    {
        Console.Warn << _T("CBitmap::SetPixel4b() - Invalid operation for compressed bitmap") << newl;
        return;
    }

    if (!m_pData || x >= m_iWidth || y >= m_iHeight || x < 0 || y < 0)
        return;

    int index = (y * m_iWidth + x) * m_iBMPType;

    m_pData[index] = r;
    m_pData[index + 1] = g;
    m_pData[index + 2] = b;
    if (m_iBMPType == BITMAP_RGBA)
        m_pData[index + 3] = a;
}


/*====================
  CBitmap::SetPixel4f
  ====================*/
void    CBitmap::SetPixel4f(int x, int y, float r, float g, float b, float a)
{
    if (m_iBMPType != BITMAP_RGBA32F)
    {
        Console.Warn << _T("CBitmap::SetPixel4f() - Operation only valid for RGBA32F bitmaps") << newl;
        return;
    }

    if (!m_pData || x >= m_iWidth || y >= m_iHeight || x < 0 || y < 0)
        return;

    int index = (y * m_iWidth + x) * 4;

    float *pFloatData(reinterpret_cast<float *>(m_pData));

    pFloatData[index] = r;
    pFloatData[index + 1] = g;
    pFloatData[index + 2] = b;
    pFloatData[index + 3] = a;
}


/*====================
  CBitmap::Clear4b
  ====================*/
void    CBitmap::Clear4b(byte r, byte g, byte b, byte a)
{
    if (m_iBMPType > BITMAP_RGBA)
    {
        Console.Warn << _T("CBitmap::Clear4b() - Invalid operation for compressed bitmap") << newl;
        return;
    }

    if (!m_pData)
        return;

    if (m_iBMPType == BITMAP_RGBA)
    {
        for (int i = 0; i < m_iSize; i += m_iBMPType)
        {
            m_pData[i] = r;
            m_pData[i + 1] = g;
            m_pData[i + 2] = b;
            m_pData[i + 3] = a;
        }
    }
    else
    {
        for (int i = 0; i < m_iSize; i += m_iBMPType)
        {
            m_pData[i] = r;
            m_pData[i + 1] = g;
            m_pData[i + 2] = b;
        }
    }
}


/*====================
  CBitmap::DesaturateToAlpha
  ====================*/
void    CBitmap::DesaturateToAlpha(int iWidth, int iHeight)
{
    if (m_iBMPType > BITMAP_RGBA)
    {
        Console.Warn << _T("CBitmap::DesaturateToAlpha() - Invalid operation for compressed bitmap") << newl;
        return;
    }

    for (int i(0); i < iWidth * iHeight; i+=m_iBMPType)
    {
        int iTotal = m_pData[i] + m_pData[i + 1] + m_pData[i + 2];
        m_pData[i] = 0;
        m_pData[i + 1] = 0;
        m_pData[i + 2] = 0;
        m_pData[i + 3] = 255 - static_cast<byte>(iTotal / 3);
    }
}


/*====================
  CBitmap::Lerp
  ====================*/
void    CBitmap::Lerp(float fLerp, const CBitmap &a, const CBitmap &b)
{
    Free();
    Alloc(a.m_iWidth, a.m_iHeight, a.m_iBMPType);

    for (int i(0); i < m_iWidth * m_iHeight * m_iBMPType; ++i)
        m_pData[i] = LERP(fLerp, a.m_pData[i], b.m_pData[i]);
}


//=============================================================================

// code borrowed from libjpeg sample
// Expanded data source object for CFileHandle input

struct SHandleSourceMgr
{
    jpeg_source_mgr pub;            // public fields

    CFileHandle     *pFile;         // source stream
    JOCTET          *pBuffer;       // start of buffer
    bool            bStartOfFile;   // have we gotten any data yet?
};

#define INPUT_BUF_SIZE  4096    // choose an efficiently fread'able size


/*====================
  JPEG_InitSource

  Initialize source --- called by jpeg_read_header
  before any data is actually read.
  ====================*/
void    JPEG_InitSource(j_decompress_ptr cinfo)
{
    SHandleSourceMgr *pSrc = (SHandleSourceMgr *) cinfo->src;

    // We reset the empty - input - file flag for each image,
    // but we don't clear the input buffer.
    // This is correct behavior for reading a series of images from one source.

    pSrc->bStartOfFile = true;
}


/*====================
  _JPEG_FillInputBuffer
  
  Fill the input buffer --- called whenever buffer is emptied.
  
  In typical applications, this should read fresh data into the buffer
  (ignoring the current state of next_input_byte & bytes_in_buffer),
  reset the pointer & count to the start of the buffer, and return TRUE
  indicating that the buffer has been reloaded.  It is not necessary to
  fill the buffer entirely, only to obtain at least one more byte.

  There is no such thing as an EOF return.  If the end of the file has been
  reached, the routine has a choice of ERREXIT() or inserting fake data into
  the buffer.  In most cases, generating a warning message and inserting a
  fake EOI marker is the best course of action --- this will allow the
  decompressor to output however much of the image is there.  However,
  the resulting error message is misleading if the real problem is an empty
  input file, so we handle that case specially.
  ====================*/
boolean JPEG_FillInputBuffer(j_decompress_ptr cinfo)
{
    SHandleSourceMgr *pSrc = (SHandleSourceMgr *) cinfo->src;
    size_t nbytes = pSrc->pFile->Read((char *)pSrc->pBuffer, INPUT_BUF_SIZE);

    if (nbytes <= 0)
    {
        if (pSrc->bStartOfFile) // Treat empty input file as fatal error
            ERREXIT(cinfo, JERR_INPUT_EMPTY);

        WARNMS(cinfo, JWRN_JPEG_EOF);

        // Insert a fake EOI marker
        pSrc->pBuffer[0] = (JOCTET) 0xFF;
        pSrc->pBuffer[1] = (JOCTET) JPEG_EOI;
        nbytes = 2;
    }

    pSrc->pub.next_input_byte = pSrc->pBuffer;
    pSrc->pub.bytes_in_buffer = nbytes;
    pSrc->bStartOfFile = false;

    return TRUE;
}


/*====================
  JPEG_SkipInputData

  Skip data --- used to skip over a potentially large amount of
  uninteresting data (such as an APPn marker).

  Writers of suspendable - input applications must note that skip_input_data
  is not granted the right to give a suspension return.  If the skip extends
  beyond the data currently in the buffer, the buffer can be marked empty so
  that the next read will cause a fill_input_buffer call that can suspend.
  Arranging for additional bytes to be discarded before reloading the input
  buffer is the application writer's problem.
  ====================*/
void    JPEG_SkipInputData(j_decompress_ptr cinfo, long iNumBytes)
{
    SHandleSourceMgr *pSrc = (SHandleSourceMgr *) cinfo->src;

    // Just a dumb implementation for now.  Could use fseek() except
    // it doesn't work on pipes.  Not clear that being smart is worth
    // any trouble anyway --- large skips are infrequent.

    if (iNumBytes > 0)
    {
        while (iNumBytes > (long)pSrc->pub.bytes_in_buffer)
        {
            iNumBytes -= (long)pSrc->pub.bytes_in_buffer;
            (void)JPEG_FillInputBuffer(cinfo);

            // note we assume that fill_input_buffer will never return FALSE,
            // so suspension need not be handled.

        }

        pSrc->pub.next_input_byte += (size_t)iNumBytes;
        pSrc->pub.bytes_in_buffer -= (size_t)iNumBytes;
    }
}


/*====================
  JPEG_TermSource

  Terminate source --- called by jpeg_finish_decompress
  after all data has been read.
  ====================*/
void    JPEG_TermSource(j_decompress_ptr cinfo)
{
    // no work necessary here
}


/*====================
  JPEG_SetSource
  
  Prepare for input from a CFileHandle.
  The caller must have already opened the handle, and is responsible
  for closing it after finishing decompression.
  ====================*/
void    JPEG_SetSource(j_decompress_ptr cinfo, CFileHandle *pFile)
{
    SHandleSourceMgr *pSrc;

    // The source object and input buffer are made permanent so that a series
    // of JPEG images can be read from the same file by calling jpeg_stdio_src
    // only before the first one.  (If we discarded the buffer at the end of
    // one image, we'd likely lose the start of the next one.)
    // This makes it unsafe to use this manager and a different source
    // manager serially with the same JPEG object.  Caveat programmer.

    if (cinfo->src == nullptr) // first time for this JPEG object?
    {
        cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(SHandleSourceMgr));
        pSrc = (SHandleSourceMgr *)cinfo->src;
        pSrc->pBuffer = (JOCTET *)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
    }

    pSrc = (SHandleSourceMgr *)cinfo->src;
    pSrc->pub.init_source = JPEG_InitSource;
    pSrc->pub.fill_input_buffer = JPEG_FillInputBuffer;
    pSrc->pub.skip_input_data = JPEG_SkipInputData;
    pSrc->pub.resync_to_restart = jpeg_resync_to_restart;   // use default method
    pSrc->pub.term_source = JPEG_TermSource;
    pSrc->pub.bytes_in_buffer = 0;                          // forces fill_input_buffer on first read
    pSrc->pub.next_input_byte = nullptr;                       // until buffer loaded
    pSrc->pFile = pFile;
}


/*====================
  JPEG_OutputMessage

  Override for the default message output function, so that it prints to the console
  ====================*/
void    JPEG_OutputMessage(j_common_ptr cinfo)
{
    char    buffer[JMSG_LENGTH_MAX];
    TCHAR   tbuffer[JMSG_LENGTH_MAX];

    (*cinfo->err->format_message)(cinfo, buffer);
    SingleToTCHAR(tbuffer, JMSG_LENGTH_MAX, buffer, JMSG_LENGTH_MAX);
    Console.Err << _T("JPEG: ") << tbuffer << newl;
}


/*====================
  JPEG_ErrorExit

  Override the default handling for a fatal error, which would normally exit the program
  Instead, throw an exception that the LoadJPEG will catch
  ====================*/
void    JPEG_ErrorExit(j_common_ptr cinfo)
{
    char    buffer[JMSG_LENGTH_MAX];
    TCHAR   szbuffer[JMSG_LENGTH_MAX];

    (*cinfo->err->format_message)(cinfo, buffer);
    SingleToTCHAR(szbuffer, JMSG_LENGTH_MAX, buffer, JMSG_LENGTH_MAX);
#ifdef __GNUC__
    g_sErrorMsg = szbuffer;
    longjmp(g_JPEGJmpBuf, 1);
#else
    EX_ERROR(szbuffer);
#endif
}


/*====================
  JPEG_InitDestination
  ====================*/
void    JPEG_InitDestination(j_compress_ptr pInfo)
{
    SJPEGDestManager *pDest(reinterpret_cast<SJPEGDestManager*>(pInfo->dest));

    pInfo->dest->free_in_buffer = pDest->m_iBufferSize;
    pInfo->dest->next_output_byte = (JOCTET*)pDest->m_pBuffer;
}


/*====================
  JPEG_EmptyOutputBuffer
  ====================*/
boolean JPEG_EmptyOutputBuffer(j_compress_ptr pInfo)
{
    SJPEGDestManager *pDest(reinterpret_cast<SJPEGDestManager*>(pInfo->dest));
    
    pDest->m_pFileHandle->Write(pDest->m_pBuffer, pDest->m_iBufferSize);
    pInfo->dest->next_output_byte = (JOCTET*)pDest->m_pBuffer;
    pInfo->dest->free_in_buffer = pDest->m_iBufferSize;
    return TRUE;
}


/*====================
  JPEG_TermDestination
  ====================*/
void    JPEG_TermDestination(j_compress_ptr pInfo)
{
    SJPEGDestManager *pDest(reinterpret_cast<SJPEGDestManager*>(pInfo->dest));
    
    pDest->m_pFileHandle->Write(pDest->m_pBuffer, pDest->m_iBufferSize - pInfo->dest->free_in_buffer);
    SAFE_DELETE_ARRAY(pDest->m_pBuffer);
    SAFE_DELETE(pDest);
}


/*====================
  JPEG_SetupDest
  ====================*/
void    JPEG_SetupDest(jpeg_compress_struct *pInfo, CFileHandle *pFile, int iBufferSize = 4096)
{
    SJPEGDestManager *pDestManager(K2_NEW(ctx_Singleton,  SJPEGDestManager));
    pDestManager->m_iBufferSize = iBufferSize;
    pDestManager->m_pBuffer = K2_NEW_ARRAY(ctx_Bitmap, char, iBufferSize);
    pDestManager->m_pFileHandle = pFile;
    pInfo->dest = (jpeg_destination_mgr*)pDestManager;
    pInfo->dest->init_destination = JPEG_InitDestination;
    pInfo->dest->empty_output_buffer = JPEG_EmptyOutputBuffer;
    pInfo->dest->term_destination = JPEG_TermDestination;
}


/*====================
  CBitmap::WriteJPEG
  ====================*/
bool    CBitmap::WriteJPEG(const tstring &sFilename, int quality)
{
    try
    {
        struct jpeg_compress_struct cinfo;

#ifdef __GNUC__
        if (setjmp(g_JPEGJmpBuf))
        {
            EX_ERROR(g_sErrorMsg);
        }
#endif

        // TODO: Use custom error processing
        struct jpeg_error_mgr jerr;
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        // Use custom output functions
        CFileHandle hOutFile(sFilename, FILE_WRITE | FILE_BINARY);
        JPEG_SetupDest(&cinfo, &hOutFile);

        // TODO: Use custom memory allocation

        // First we supply a description of the input image.
        // Four fields of the cinfo struct must be filled in:
        cinfo.image_width = m_iWidth;
        cinfo.image_height = m_iHeight;
        cinfo.input_components = 3;         // # of color components per pixel
        cinfo.in_color_space = JCS_RGB;     // colorspace of input image

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality * 255 / 100, TRUE);
        jpeg_start_compress(&cinfo, TRUE);

        int row_stride(m_iWidth * 3);   // JSAMPLEs per row in the image buffer
        JSAMPROW row_pointer[1];        // pointer to JSAMPLE row[s]
        while (cinfo.next_scanline < cinfo.image_height)
        {
            row_pointer[0] = &m_pData[cinfo.next_scanline * row_stride];
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CBitmap::WriteJPEG() - "), NO_THROW);
        return false;
    }
}


/*====================
  CBitmap::LoadJPEG
  ====================*/
bool    CBitmap::LoadJPEG(const tstring &sFilename, bool bMonoAsAlpha)
{
    if (m_pData != nullptr)
        Free();

    jpeg_decompress_struct  cinfo;

    try
    {
        // Initialize JPEG decompression object
        MemManager.Set(&cinfo, 0, sizeof(cinfo));

        CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY | m_uiFileFlags);
        if (!hFile.IsOpen())
            EX_ERROR(_T("Failed to open file"));

#ifdef __GNUC__
        if (setjmp(g_JPEGJmpBuf))
        {
            EX_ERROR(g_sErrorMsg);
        }
#endif

        // Override the default error handling
        jpeg_error_mgr jerr;
        cinfo.err = jpeg_std_error(&jerr);
        jerr.error_exit = JPEG_ErrorExit;
        jerr.output_message = JPEG_OutputMessage;

        // Start the decompression
        jpeg_create_decompress(&cinfo);
        JPEG_SetSource(&cinfo, &hFile);
        jpeg_read_header(&cinfo, TRUE);

        jpeg_start_decompress(&cinfo);

        // Fill in the CBitmap
        m_iWidth = cinfo.output_width;
        m_iHeight = cinfo.output_height;

        if (cinfo.out_color_space == JCS_RGB)
            m_iBMPType = BITMAP_RGB;
        else if (cinfo.out_color_space == JCS_GRAYSCALE)
            m_iBMPType = BITMAP_ALPHA;

        m_iSize = m_iWidth * m_iHeight * m_iBMPType;

        m_pData = K2_NEW_ARRAY(ctx_Bitmap, byte, m_iSize);

        if (m_iBMPType == BITMAP_RGB)
        {
            m_iRedChannel = 0;
            m_iGreenChannel = 1;
            m_iBlueChannel = 2;
            m_iAlphaChannel = BMP_CHANNEL_WHITE;
        }
        else if (m_iBMPType == BITMAP_ALPHA)
        {
            if (bMonoAsAlpha)
            {
                // Set mono channel as alpha
                m_iRedChannel = BMP_CHANNEL_WHITE;
                m_iGreenChannel = BMP_CHANNEL_WHITE;
                m_iBlueChannel = BMP_CHANNEL_WHITE;
                m_iAlphaChannel = 0;
            }
            else
            {
                // Duplicate mono channel into RGB
                m_iRedChannel = 0;
                m_iGreenChannel = 0;
                m_iBlueChannel = 0;
                m_iAlphaChannel = BMP_CHANNEL_WHITE;
            }
        }

        int row_stride = cinfo.output_width * cinfo.output_components;

        // Make a one - row - high sample array that will go away when done with image
        unsigned char **buffer = K2_NEW_ARRAY(ctx_Bitmap, byte*, cinfo.rec_outbuf_height);
        for (int i = 0; i < cinfo.rec_outbuf_height; ++i)
            buffer[i] = K2_NEW_ARRAY(ctx_Bitmap, byte, row_stride*sizeof(JSAMPLE));

        // Retrieve the decompressed data
        while (cinfo.output_scanline < cinfo.output_height)
        {
            jpeg_read_scanlines(&cinfo, buffer, cinfo.rec_outbuf_height);
            for (int i = 0; i < cinfo.rec_outbuf_height; ++i)
                MemManager.Copy(&m_pData[((cinfo.output_height - cinfo.output_scanline) * row_stride) + ((cinfo.rec_outbuf_height - i - 1) * row_stride)], buffer[i], row_stride);
        }

        // Free the buffers
        for (int i = 0; i < cinfo.rec_outbuf_height; ++i)
            K2_DELETE_ARRAY(buffer[i]);
        K2_DELETE_ARRAY(buffer);

        // Finish decompression
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        if (jerr.num_warnings > 0)
            Console.Warn << jerr.num_warnings << _T(" warnings occurred during decompression of file: ") << sFilename << newl;

        Console.Res << sFilename << _T(" - ") << m_iBMPType * m_iWidth * m_iHeight <<
            _T(" bytes (") << ((m_iBMPType == BITMAP_RGBA) ? _T("RGBA") : _T("RGB")) << _T(")") << newl;

        return true;
    }
    catch (CException &ex)
    {
        jpeg_destroy_decompress(&cinfo);
        ex.Process(_TS("CBitmap::LoadJPEG(") + sFilename + _T(") - "), NO_THROW);
        return false;
    }
}


/*====================
  GIF_Read
  ====================*/
int     GIF_Read(GifFileType *GifFile, GifByteType *pBuffer, int iSize)
{
    CFileHandle &hFile(*static_cast<CFileHandle *>(GifFile->UserData));

    return hFile.Read((char *)pBuffer, iSize);
}


/*====================
  CBitmap::LoadGIF
  ====================*/
bool    CBitmap::LoadGIF(const tstring &sFilename)
{
    if (m_pData != nullptr)
        Free();

    GifFileType *GifFile(nullptr);

    try
    {
        CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY | m_uiFileFlags);
        if (!hFile.IsOpen())
            EX_ERROR(_T("Failed to open file"));

        int iError = 0;
        GifFileType *GifFile(DGifOpen(&hFile, GIF_Read, &iError));
        if (GifFile == nullptr)
            EX_ERROR(_T("DGifOpen failed"));

        if (DGifSlurp(GifFile) == GIF_ERROR)
            EX_ERROR(_T("DGifSlurp failed"));

        if (GifFile->ImageCount < 1)
            EX_ERROR(_T("No image data"));

        ColorMapObject *pColormap(GifFile->SColorMap);

        if (pColormap == nullptr || pColormap->BitsPerPixel != 8)
            EX_ERROR(_T("No colormap"));

        int iTransparentIndex(-1);

        if (GifFile->SavedImages[0].ExtensionBlockCount > 0 && 
            GifFile->SavedImages[0].ExtensionBlocks[0].Function == GRAPHICS_EXT_FUNC_CODE && 
            GifFile->SavedImages[0].ExtensionBlocks[0].ByteCount >= 4)
        {
            byte yTransparency(GifFile->SavedImages[0].ExtensionBlocks[0].Bytes[0]);
            // 1, 2 Delay for animation
            byte yIndex(GifFile->SavedImages[0].ExtensionBlocks[0].Bytes[3]);

            if (yTransparency)
                iTransparentIndex = yIndex;
        }

        m_iWidth = GifFile->Image.Width;
        m_iHeight = GifFile->Image.Height;

        if (iTransparentIndex != -1)
        {
            m_iBMPType = BITMAP_RGBA;

            m_iRedChannel = 0;
            m_iGreenChannel = 1;
            m_iBlueChannel = 2;
            m_iAlphaChannel = 3;
        }
        else
        {
            m_iBMPType = BITMAP_RGB;

            m_iRedChannel = 0;
            m_iGreenChannel = 1;
            m_iBlueChannel = 2;
            m_iAlphaChannel = BMP_CHANNEL_WHITE;
        }

        m_iSize = m_iWidth * m_iHeight * m_iBMPType;

        m_pData = K2_NEW_ARRAY(ctx_Bitmap, byte, m_iSize);

        byte *pSrc(GifFile->SavedImages[0].RasterBits);

        if (iTransparentIndex != -1)
        {
            for (int iY(GifFile->SavedImages[0].ImageDesc.Height); iY > 0; --iY)
            {
                byte *pDest(&m_pData[(iY - 1) * m_iWidth * m_iBMPType]);

                for (int iX(GifFile->SavedImages[0].ImageDesc.Width); iX > 0; --iX)
                {
                    int iColorIndex(*pSrc++);

                    GifColorType &cColor(pColormap->Colors[iColorIndex]);

                    *pDest++ = cColor.Red;
                    *pDest++ = cColor.Green;
                    *pDest++ = cColor.Blue;
                    *pDest++ = iColorIndex == iTransparentIndex ? 0 : 255;
                }
            }
        }
        else
        {
            for (int iY(GifFile->SavedImages[0].ImageDesc.Height); iY > 0; --iY)
            {
                byte *pDest(&m_pData[(iY - 1) * m_iWidth * m_iBMPType]);

                for (int iX(GifFile->SavedImages[0].ImageDesc.Width); iX > 0; --iX)
                {
                    GifColorType &cColor(pColormap->Colors[*pSrc++]);

                    *pDest++ = cColor.Red;
                    *pDest++ = cColor.Green;
                    *pDest++ = cColor.Blue;
                }
            }
        }

        Console.Res << sFilename << _T(" - ") << m_iBMPType * m_iWidth * m_iHeight <<
            _T(" bytes (") << ((m_iBMPType == BITMAP_RGBA) ? _T("RGBA") : _T("RGB")) << _T(")") << newl;

        return true;
    }
    catch (CException &ex)
    {
        if (GifFile != nullptr) {
            int iError = 0;
            DGifCloseFile(GifFile, &iError);
        }

        ex.Process(_TS("CBitmap::LoadGIF(") + sFilename + _T(") - "), NO_THROW);
        return false;
    }
}


/*====================
  CBitmap::GetChannelInfo
  ====================*/
void    CBitmap::GetChannelInfo(int iChannel, int &iStart, int &iDefault, float &fDefault, int &iBits) const
{
    if (iChannel == BMP_CHANNEL_BLACK)
    {
        iStart = -1;
        iDefault = 0;
        fDefault = 0.0f;
        iBits = 0;
    }
    else if (iChannel == BMP_CHANNEL_WHITE)
    {
        iStart = -1;
        iDefault = 255;
        fDefault = 1.0f;
        iBits = 0;
    }
    else
    {
        iStart = iChannel << 3;
        iDefault = 255;
        fDefault = 1.0f;
        iBits = 8;
    }
}


/*====================
  CBitmap::GetFormatDescriptor
  ====================*/
void    CBitmap::GetFormatDescriptor(SBitmapFormatDescriptor &cDescriptor) const
{
    assert(m_iRedChannel != BMP_CHANNEL_NULL &&
        m_iGreenChannel != BMP_CHANNEL_NULL &&
        m_iBlueChannel != BMP_CHANNEL_NULL &&
        m_iAlphaChannel != BMP_CHANNEL_NULL);

    switch (m_iBMPType)
    {
    case BITMAP_RGBA:
    case BITMAP_RGB:
    case BITMAP_LUMINANCE_ALPHA:
    case BITMAP_ALPHA:
        {
            cDescriptor.iPixelBytes = m_iBMPType;
            cDescriptor.bFloat = false;

            GetChannelInfo(m_iRedChannel, cDescriptor.aStart[R], cDescriptor.aIntDefault[R], cDescriptor.aFloatDefault[R], cDescriptor.aBits[R]);
            GetChannelInfo(m_iGreenChannel, cDescriptor.aStart[G], cDescriptor.aIntDefault[G], cDescriptor.aFloatDefault[G], cDescriptor.aBits[G]);
            GetChannelInfo(m_iBlueChannel, cDescriptor.aStart[B], cDescriptor.aIntDefault[B], cDescriptor.aFloatDefault[B], cDescriptor.aBits[B]);
            GetChannelInfo(m_iAlphaChannel, cDescriptor.aStart[A], cDescriptor.aIntDefault[A], cDescriptor.aFloatDefault[A], cDescriptor.aBits[A]);
        }
        break;
    case BITMAP_RGBA32F:
        cDescriptor.iPixelBytes = 16;
        cDescriptor.bFloat = true;
        cDescriptor.aStart[R] = 0;
        cDescriptor.aStart[G] = 32;
        cDescriptor.aStart[B] = 64;
        cDescriptor.aStart[A] = 96;
        cDescriptor.aIntDefault[R] = 255;
        cDescriptor.aIntDefault[G] = 255;
        cDescriptor.aIntDefault[B] = 255;
        cDescriptor.aIntDefault[A] = 255;
        cDescriptor.aFloatDefault[R] = 1.0f;
        cDescriptor.aFloatDefault[G] = 1.0f;
        cDescriptor.aFloatDefault[B] = 1.0f;
        cDescriptor.aFloatDefault[A] = 1.0f;
        cDescriptor.aBits[R] = 32;
        cDescriptor.aBits[G] = 32;
        cDescriptor.aBits[B] = 32;
        cDescriptor.aBits[A] = 32;
        break;
    default:
        assert(0);
    }
}


/*====================
  Bitmap_Init
  ====================*/
void    Bitmap_Init()
{
    Console.Dev << _T("Using png library version ") << png_access_version_number() << newl;

    if (png_access_version_number() != PNG_LIBPNG_VER) {
#if TKTK // Make PNG library version mismatch a warning instead of a fatal error
        K2System.Error(_T("PNG header and library versions do not match"));
#else
        Console.Dev << _T("PNG header and library versions do not match") << newl;
#endif
    }
}
