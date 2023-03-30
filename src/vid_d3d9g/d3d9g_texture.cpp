// (C)2005 S2 Games
// d3d9g_texture.cpp
//
// Direct3D texture functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9g_main.h"
#include "d3d9g_texture.h"
#include "d3d9g_util.h"
#include "d3d9g_scene.h"
#include "d3d9g_shader.h"
#include "c_procedural.h"
#include "c_proceduralregistry.h"
#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "c_texturecache.h"
#include "c_texturearchive.h"

#include "../k2/c_resourcemanager.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_camera.h"
#include "../k2/c_texture.h"
#include "../k2/i_resourcelibrary.h"
#include "../k2/c_function.h"
#include "../k2/c_uimanager.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_eventmanager.h"

#include <DXerr.h>
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
TextureMap					g_mapTextures;

int							g_iWhite;
int							g_iBlack;
int							g_iInvis;

IDirect3DBaseTexture9*		g_pTextures[MAX_TEXTURES];
IDirect3DCubeTexture9*		g_pTexturesCube[MAX_TEXTURES];
IDirect3DVolumeTexture9*	g_pTexturesVolume[MAX_TEXTURES];
IDirect3DTexture9*			g_pTextures2D[MAX_TEXTURES];

CVAR_INTF	(vid_textureDownsize,		0,			CVAR_SAVECONFIG);
CVAR_BOOLF	(vid_textureCompression,	true,		CVAR_SAVECONFIG);
CVAR_BOOLF	(vid_textureAutogenMipmaps,	false,		CVAR_SAVECONFIG);
CVAR_INTF	(vid_textureMaxSize,		4096,		CVAR_SAVECONFIG);
CVAR_BOOLF	(vid_texturePreload,		true,		CVAR_SAVECONFIG);
CVAR_BOOL	(vid_textureProfile,		false);
CVAR_BOOL	(vid_textureCache,			true);
CVAR_FLOAT	(vid_textureGammaCorrect,	2.2f);

LONGLONG g_llStartTime(0);
LONGLONG g_llSectionStart(0);
LONGLONG g_llTotalTime(0);
LONGLONG g_llIndexLookup(0);
LONGLONG g_llFileInfo(0);
LONGLONG g_llFileRead(0);
LONGLONG g_llRegister(0);
LONGLONG g_llResize(0);
LONGLONG g_llMipMaps(0);
LONGLONG g_llCompress(0);
LONGLONG g_llPreLoad(0);

LONGLONG g_llTotalTextures(0);
LONGLONG g_llHighTextureTime(0);

LONGLONG	g_llTotalTextureLoadTime(0);
uint		g_uiLoadRequests(0);
uint		g_uiCacheMisses(0);
uint		g_uiMissingTextures(0);
tsvector		g_vCacheMisses;

vector<CTextureArchive*>	g_vTextureArchives;

ResHandle	g_hSkyReference(INVALID_RESOURCE);
ResHandle	g_hSkinReference(INVALID_RESOURCE);

static DefinitionMap	s_mapTextureDefinitions;
//=============================================================================

/*--------------------
  ReloadTextures
  --------------------*/
CMD(ReloadTextures)
{
	D3D_OpenTextureArchive(true);
	g_ResourceManager.GetLib(RES_TEXTURE)->ReloadAll();
	return true;
}


/*====================
  D3D_OpenTextureArchive
  ====================*/
void	D3D_OpenTextureArchive(bool bNoReload)
{
	bool bReload(!g_vTextureArchives.empty() && !bNoReload);

	D3D_CloseTextureArchive();

	int iModDepth(1);
	tstring sMod(FileManager.GetTopModPath());
	while (!sMod.empty())
	{
		g_vTextureArchives.push_back(new CTextureArchive(sMod));

		sMod = FileManager.GetModPath(iModDepth);
		++iModDepth;
	}

	if (bReload)
		g_ResourceManager.GetLib(RES_TEXTURE)->ReloadAll();
}


/*====================
  D3D_CloseTextureArchive
  ====================*/
void	D3D_CloseTextureArchive()
{
	for (vector<CTextureArchive*>::iterator it(g_vTextureArchives.begin()); it != g_vTextureArchives.end(); ++it)
		SAFE_DELETE(*it);
	g_vTextureArchives.clear();
}


/*====================
  D3D_GetTextureList
  ====================*/
void	D3D_GetTextureList(const tstring &sSearch, tsvector &vResult)
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
#endif
}


/*====================
  D3D_CompressTexture
  ====================*/
IDirect3DTexture9*	D3D_CompressTexture(IDirect3DTexture9 *pOriginalTexture, D3DFORMAT eFmt, bool bMipmaps)
{
	IDirect3DSurface9 *pDestSurface(NULL);
	IDirect3DSurface9 *pSrcSurface(NULL);
	IDirect3DTexture9 *pCompressedTexture(NULL);

	try
	{
		D3DSURFACE_DESC desc;
		pOriginalTexture->GetLevelDesc(0, &desc);

		if ((desc.Width % 4 != 0) || (desc.Height % 4 != 0))
			EX_DEBUG(_T("Image dimension is not a multiple of 4"));

		// Determine how many levels are required
		uint uiNumLevels;
		if (bMipmaps)
		{
			float fDim(float(MAX(desc.Width, desc.Height)));
			uiNumLevels = uint(log(fDim) / log(2.0f)) + 1;
		}
		else
		{
			uiNumLevels = 1;
		}

		if (desc.Format != D3DFMT_R8G8B8 &&
			desc.Format != D3DFMT_X8R8G8B8 &&
			desc.Format != D3DFMT_A8R8G8B8)
			return pOriginalTexture;

		// Determine compression to use
		if (desc.Format == D3DFMT_R8G8B8 || desc.Format == D3DFMT_X8R8G8B8)
		{
			if (!g_DeviceCaps.bDXT1)
				EX_WARN(_T("Texture compression not supported, turn off vid_textureCompression"));
			eFmt = D3DFMT_DXT1;
		}
		else if (!g_DeviceCaps.bDXT5)
		{
			if (!g_DeviceCaps.bDXT3)
				EX_WARN(_T("Texture compression not supported, turn off vid_textureCompression"));
			eFmt = D3DFMT_DXT3;
		}

		// Create the new texture
		if (FAILED(g_pd3dDevice->CreateTexture(desc.Width, desc.Height, uiNumLevels, 0, eFmt,
			D3DPOOL_MANAGED, &pCompressedTexture, NULL)))
			EX_WARN(_T("CreateTexture failed"));

		if (pOriginalTexture->GetLevelCount() == uiNumLevels)
		{
			// Compress the mipmaps
			for (uint uiLevel(0); uiLevel < uiNumLevels; ++uiLevel)
			{
				// Copy the surface of the original texture
				if (FAILED(pOriginalTexture->GetSurfaceLevel(uiLevel, &pSrcSurface)))
					EX_WARN(_T("Couldn't retrieve source surface"));

				if (FAILED(pCompressedTexture->GetSurfaceLevel(uiLevel, &pDestSurface)))
					EX_WARN(_T("Couldn't retrieve destination surface"));

				if (FAILED(D3DXLoadSurfaceFromSurface(pDestSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_FILTER_NONE, 0)))
					EX_WARN(_T("D3DXLoadSurfaceFromSurface failed"));

				SAFE_RELEASE(pDestSurface)
				SAFE_RELEASE(pSrcSurface);
			}
		}
		else
		{
			// Copy the surface of the original texture
			if (FAILED(pOriginalTexture->GetSurfaceLevel(0, &pSrcSurface)))
				EX_WARN(_T("Couldn't retrieve source surface"));

			// Generate the mipmaps
			for (uint uiLevel(0); uiLevel < uiNumLevels; ++uiLevel)
			{
				if (FAILED(pCompressedTexture->GetSurfaceLevel(uiLevel, &pDestSurface)))
					EX_WARN(_T("Couldn't retrieve destination surface"));

				if (FAILED(D3DXLoadSurfaceFromSurface(pDestSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_DEFAULT, 0)))
					EX_WARN(_T("D3DXLoadSurfaceFromSurface failed"));

				SAFE_RELEASE(pDestSurface)
			}

			SAFE_RELEASE(pSrcSurface);
		}

		// Free the original texture and return the new compressed version
		SAFE_RELEASE(pOriginalTexture);
		return pCompressedTexture;
	}
	catch (CException &ex)
	{
		SAFE_RELEASE(pDestSurface);
		SAFE_RELEASE(pSrcSurface);
		SAFE_RELEASE(pCompressedTexture);
		ex.Process(_T("D3D_CompressTexture() - "), NO_THROW);
		return pOriginalTexture;
	}
}


/*====================
  D3D_CompressCubeTexture
  ====================*/
IDirect3DCubeTexture9*	D3D_CompressCubeTexture(IDirect3DCubeTexture9 *pOriginalTexture, D3DFORMAT eFmt, bool bMipmaps)
{
	IDirect3DSurface9 *pDestSurface(NULL);
	IDirect3DSurface9 *pSrcSurface(NULL);
	IDirect3DCubeTexture9 *pCompressedTexture(NULL);

	try
	{
		D3DSURFACE_DESC desc;
		pOriginalTexture->GetLevelDesc(0, &desc);

		if (desc.Width % 4 != 0)
			EX_DEBUG(_T("Image dimension is not a multiple of 4"));

		// Determine how many levels are required
		uint uiNumLevels;
		if (bMipmaps)
		{
			float fDim(float(desc.Width));
			uiNumLevels = uint(log(fDim) / log(2.0f)) + 1;
		}
		else
		{
			uiNumLevels = 1;
		}

		if (desc.Format != D3DFMT_R8G8B8 &&
			desc.Format != D3DFMT_X8R8G8B8 &&
			desc.Format != D3DFMT_A8R8G8B8)
			return pOriginalTexture;

		// Determine compression to use
		if (desc.Format == D3DFMT_R8G8B8 || desc.Format == D3DFMT_X8R8G8B8)
		{
			if (!g_DeviceCaps.bDXT1)
				EX_WARN(_T("Texture compression not supported, turn off vid_textureCompression"));
			eFmt = D3DFMT_DXT1;
		}
		else if (!g_DeviceCaps.bDXT5)
		{
			if (!g_DeviceCaps.bDXT3)
				EX_WARN(_T("Texture compression not supported, turn off vid_textureCompression"));
			eFmt = D3DFMT_DXT3;
		}

		// Create the new texture
		if (FAILED(g_pd3dDevice->CreateCubeTexture(desc.Width, uiNumLevels, 0, eFmt,
			D3DPOOL_MANAGED, &pCompressedTexture, NULL)))
			EX_WARN(_T("CreateCubeTexture failed"));

		if (pOriginalTexture->GetLevelCount() == uiNumLevels)
		{
			for (int iFace(0); iFace < 6; ++iFace)
			{
				// Compress the mipmaps
				for (uint uiLevel(0); uiLevel < uiNumLevels; ++uiLevel)
				{
					// Copy the surface of the original texture
					if (FAILED(pOriginalTexture->GetCubeMapSurface(D3DCUBEMAP_FACES(iFace), uiLevel, &pSrcSurface)))
						EX_WARN(_T("Couldn't retrieve source surface"));

					if (FAILED(pCompressedTexture->GetCubeMapSurface(D3DCUBEMAP_FACES(iFace), uiLevel, &pDestSurface)))
						EX_WARN(_T("Couldn't retrieve destination surface"));

					if (FAILED(D3DXLoadSurfaceFromSurface(pDestSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_FILTER_NONE, 0)))
						EX_WARN(_T("D3DXLoadSurfaceFromSurface failed"));

					SAFE_RELEASE(pDestSurface)
					SAFE_RELEASE(pSrcSurface);
				}
			}
		}
		else
		{
			for (int iFace(0); iFace < 6; ++iFace)
			{
				// Copy the surface of the original texture
				if (FAILED(pOriginalTexture->GetCubeMapSurface(D3DCUBEMAP_FACES(iFace), 0, &pSrcSurface)))
					EX_WARN(_T("Couldn't retrieve source surface"));

				// Generate the mipmaps
				for (uint uiLevel(0); uiLevel < uiNumLevels; ++uiLevel)
				{
					if (FAILED(pCompressedTexture->GetCubeMapSurface(D3DCUBEMAP_FACES(iFace), uiLevel, &pDestSurface)))
						EX_WARN(_T("Couldn't retrieve destination surface"));

					if (FAILED(D3DXLoadSurfaceFromSurface(pDestSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_DEFAULT, 0)))
						EX_WARN(_T("D3DXLoadSurfaceFromSurface failed"));

					SAFE_RELEASE(pDestSurface)
				}

				SAFE_RELEASE(pSrcSurface);
			}
		}

		// Free the original texture and return the new compressed version
		SAFE_RELEASE(pOriginalTexture);
		return pCompressedTexture;
	}
	catch (CException &ex)
	{
		SAFE_RELEASE(pDestSurface);
		SAFE_RELEASE(pSrcSurface);
		SAFE_RELEASE(pCompressedTexture);
		ex.Process(_T("D3D_CompressCubeTexture() - "), NO_THROW);
		return pOriginalTexture;
	}
}


/*====================
  D3D_CompressVolumeTexture
  ====================*/
IDirect3DVolumeTexture9*	D3D_CompressVolumeTexture(IDirect3DVolumeTexture9 *pOriginalTexture, D3DFORMAT eFmt, bool bMipmaps)
{
	IDirect3DVolume9 *pDestVolume(NULL);
	IDirect3DVolume9 *pSrcVolume(NULL);
	IDirect3DVolumeTexture9 *pCompressedTexture(NULL);

	try
	{
		D3DVOLUME_DESC desc;
		pOriginalTexture->GetLevelDesc(0, &desc);

		if (desc.Width % 4 != 0)
			EX_DEBUG(_T("Image dimension is not a multiple of 4"));

		// Determine how many levels are required
		uint uiNumLevels;
		if (bMipmaps)
		{
			float fDim(float(MAX(MAX(desc.Width, desc.Height), desc.Depth)));
			uiNumLevels = uint(log(fDim) / log(2.0f)) + 1;
		}
		else
		{
			uiNumLevels = 1;
		}

		if (desc.Format != D3DFMT_R8G8B8 &&
			desc.Format != D3DFMT_X8R8G8B8 &&
			desc.Format != D3DFMT_A8R8G8B8)
			return pOriginalTexture;

		// Determine compression to use
		if (desc.Format == D3DFMT_R8G8B8 || desc.Format == D3DFMT_X8R8G8B8)
		{
			if (!g_DeviceCaps.bDXT1)
				EX_WARN(_T("Texture compression not supported, turn off vid_textureCompression"));
			eFmt = D3DFMT_DXT1;
		}
		else if (!g_DeviceCaps.bDXT5)
		{
			if (!g_DeviceCaps.bDXT3)
				EX_WARN(_T("Texture compression not supported, turn off vid_textureCompression"));
			eFmt = D3DFMT_DXT3;
		}

		// Create the new texture
		if (FAILED(g_pd3dDevice->CreateVolumeTexture(desc.Width, desc.Height, desc.Depth, uiNumLevels, 0, eFmt,
			D3DPOOL_MANAGED, &pCompressedTexture, NULL)))
			EX_WARN(_T("CreateVolumeTexture failed"));

		if (pOriginalTexture->GetLevelCount() == uiNumLevels)
		{
			// Compress the mipmaps
			for (uint uiLevel(0); uiLevel < uiNumLevels; ++uiLevel)
			{
				// Copy the surface of the original texture
				if (FAILED(pOriginalTexture->GetVolumeLevel(uiLevel, &pSrcVolume)))
					EX_WARN(_T("Couldn't retrieve source surface"));

				if (FAILED(pCompressedTexture->GetVolumeLevel(uiLevel, &pDestVolume)))
					EX_WARN(_T("Couldn't retrieve destination surface"));

				if (FAILED(D3DXLoadVolumeFromVolume(pDestVolume, NULL, NULL, pSrcVolume, NULL, NULL, D3DX_FILTER_NONE, 0)))
					EX_WARN(_T("D3DXLoadVolumeFromVolume failed"));

				SAFE_RELEASE(pDestVolume)
				SAFE_RELEASE(pSrcVolume);
			}
		}
		else
		{
			// Copy the surface of the original texture
			if (FAILED(pOriginalTexture->GetVolumeLevel(0, &pSrcVolume)))
				EX_WARN(_T("Couldn't retrieve source surface"));

			// Generate the mipmaps
			for (uint uiLevel(0); uiLevel < uiNumLevels; ++uiLevel)
			{
				if (FAILED(pCompressedTexture->GetVolumeLevel(uiLevel, &pDestVolume)))
					EX_WARN(_T("Couldn't retrieve destination surface"));

				if (FAILED(D3DXLoadVolumeFromVolume(pDestVolume, NULL, NULL, pSrcVolume, NULL, NULL, D3DX_DEFAULT, 0)))
					EX_WARN(_T("D3DXLoadVolumeFromVolume failed"));

				SAFE_RELEASE(pDestVolume)
			}

			SAFE_RELEASE(pSrcVolume);
		}

		// Free the original texture and return the new compressed version
		SAFE_RELEASE(pOriginalTexture);
		return pCompressedTexture;
	}
	catch (CException &ex)
	{
		SAFE_RELEASE(pDestVolume);
		SAFE_RELEASE(pSrcVolume);
		SAFE_RELEASE(pCompressedTexture);
		ex.Process(_T("D3D_CompressCubeTexture() - "), NO_THROW);
		return pOriginalTexture;
	}
}


/*====================
  BitVectorSetInteger
  ====================*/
inline
void	BitVectorSetInteger(dword *pDst, int iDstStart, uint uiValue, int iBits)
{
    int iStartDword(iDstStart / 32);
	int iStartBit(iDstStart % 32);

	// Clamp uiValue
	if (iBits != 32)
		uiValue &= ((1 << iBits) - 1);

	// Only dword
	if (iBits <= 32 - iStartBit)
	{
		if (iBits == 32)
			pDst[iStartDword] &= ~(-1 << iStartBit);
		else
			pDst[iStartDword] &= ~(((1 << iBits) - 1) << iStartBit);

		pDst[iStartDword] |= uiValue << iStartBit;
		return;
	}

	// First dword
	pDst[iStartDword] &= ~((1 << iStartBit) - 1); // Wipe the last iStartBits
	pDst[iStartDword] |= uiValue << iStartBit;

	// Second dword
	pDst[iStartDword + 1] &= ((1 << (32 - iStartBit)) - 1); // Wipe the first 32 - iStartBits
	pDst[iStartDword + 1] |= uiValue >> iStartBit;
}


/*====================
  BitVectorGetInteger
  ====================*/
inline
uint	BitVectorGetInteger(const dword *pSrc, int iSrcStart, int iBits)
{
    int iStartDword(iSrcStart / 32);
	int iStartBit(iSrcStart % 32);

	uint uiValue(0);

	// Only dword
	if (iBits <= 32 - iStartBit)
	{
		if (iBits == 32)
			uiValue = (pSrc[iStartDword] >> iStartBit) & (-1);
		else
			uiValue = (pSrc[iStartDword] >> iStartBit) & ((1 << iBits) - 1);
		
		return uiValue;
	}

	// First dword
	uiValue |= pSrc[iStartDword] >> iStartBit;

	// Second dword
	uiValue |= pSrc[iStartDword + 1] << iStartBit;

	// Clamp uiValue
	uiValue &= ((1 << iBits) - 1);

	return uiValue;
}

/*
struct STextureFormatConverter
{
	D3DFORMAT	d3dFormat;
	int			iDstSize;
	int			aDstStart[4];
	int			aDstBits[4];
	bool		bDstFloat;
} g_TextureFormatConverter[NUM_TEXTURE_FORMATS] =
{
	{ }
};
*/


/*====================
  D3D_CopyBitmapToBuffer

  Converts the pixel format stored in CBitmap into the desired texture format
  ====================*/
void	D3D_CopyBitmapToBuffer(const CBitmap &bitmap, ETextureFormat eFormat, byte *pDstData, int iPitch)
{
	if (eFormat == TEXFMT_NORMALMAP)
		eFormat = TEXFMT_A8R8G8B8;

	// Channel names are R(0), G(1), B(2), A(3)

	D3DFORMAT d3dFormat;
	int iDstSize;
	int aDstStart[4] = {-1, -1, -1, -1};
	int aDstBits[4] = {0, 0, 0, 0};
	bool bDstFloat(false);

	switch (eFormat)
	{
	default:
	case TEXFMT_A8R8G8B8:
	case TEXFMT_NORMALMAP:
		d3dFormat = D3DFMT_A8R8G8B8;
		iDstSize = 4;
		bDstFloat = false;
		aDstStart[R] = 16;
		aDstStart[G] = 8;
		aDstStart[B] = 0;
		aDstStart[A] = 24;
		aDstBits[R] = 8;
		aDstBits[G] = 8;
		aDstBits[B] = 8;
		aDstBits[A] = 8;
		break;
	case TEXFMT_A1R5G5B5:
		d3dFormat = D3DFMT_A1R5G5B5;
		iDstSize = 4;
		bDstFloat = false;
		aDstStart[R] = 0;
		aDstStart[G] = 5;
		aDstStart[B] = 10;
		aDstStart[A] = 15;
		aDstBits[R] = 5;
		aDstBits[G] = 5;
		aDstBits[B] = 5;
		aDstBits[A] = 1;
		break;
	case TEXFMT_A4R4G4B4:
		d3dFormat = D3DFMT_A4R4G4B4;
		iDstSize = 2;
		bDstFloat = false;
		aDstStart[R] = 0;
		aDstStart[G] = 4;
		aDstStart[B] = 8;
		aDstStart[A] = 12;
		aDstBits[R] = 4;
		aDstBits[G] = 4;
		aDstBits[B] = 4;
		aDstBits[A] = 4;
		break;
	case TEXFMT_A8:
		d3dFormat = D3DFMT_A8;
		iDstSize = 1;
		bDstFloat = false;
		aDstStart[A] = 0;
		aDstBits[A] = 8;
		break;
	case TEXFMT_A8L8:
		d3dFormat = D3DFMT_A8L8;
		iDstSize = 2;
		bDstFloat = false;
		aDstStart[B] = 0;
		aDstStart[A] = 8;
		aDstBits[B] = 8;
		aDstBits[A] = 8;
		break;
	case TEXFMT_R16G16:
		d3dFormat = D3DFMT_G16R16;
		iDstSize = 4;
		bDstFloat = false;
		aDstStart[R] = 0;
		aDstStart[G] = 16;
		aDstBits[R] = 16;
		aDstBits[G] = 16;
		break;
	case TEXFMT_U8V8:
		d3dFormat = D3DFMT_V8U8;
		iDstSize = 2;
		bDstFloat = false;
		aDstStart[R] = 0;
		aDstStart[G] = 8;
		aDstBits[R] = 8;
		aDstBits[G] = 8;
		break;
	case TEXFMT_U16V16:
		d3dFormat = D3DFMT_V16U16;
		iDstSize = 4;
		bDstFloat = false;
		aDstStart[R] = 0;
		aDstStart[G] = 16;
		aDstBits[R] = 16;
		aDstBits[G] = 16;
		break;
	case TEXFMT_R16F:
		d3dFormat = D3DFMT_R16F;
		iDstSize = 2;
		bDstFloat = true;
		aDstStart[R] = 0;
		aDstBits[R] = 16;
		break;
	case TEXFMT_R16G16F:
		d3dFormat = D3DFMT_G16R16F;
		iDstSize = 4;
		bDstFloat = true;
		aDstStart[R] = 0;
		aDstStart[G] = 16;
		aDstBits[R] = 16;
		aDstBits[G] = 16;
		break;
	case TEXFMT_A16R16G16B16F:
		d3dFormat = D3DFMT_A16B16G16R16F;
		iDstSize = 8;
		bDstFloat = true;
		aDstStart[R] = 0;
		aDstStart[G] = 16;
		aDstStart[B] = 32;
		aDstStart[A] = 48;
		aDstBits[R] = 16;
		aDstBits[G] = 16;
		aDstBits[B] = 16;
		aDstBits[A] = 16;
		break;
	case TEXFMT_R32F:
		d3dFormat = D3DFMT_R32F;
		iDstSize = 4;
		bDstFloat = true;
		aDstStart[R] = 0;
		aDstBits[R] = 32;
		break;
	case TEXFMT_R32G32F:
		d3dFormat = D3DFMT_G32R32F;
		iDstSize = 8;
		bDstFloat = true;
		aDstStart[R] = 0;
		aDstStart[G] = 32;
		aDstBits[R] = 32;
		aDstBits[R] = 32;
		break;
	case TEXFMT_A32R32G32B32F:
		d3dFormat = D3DFMT_A32B32G32R32F;
		iDstSize = 16;
		bDstFloat = true;
		aDstStart[R] = 0;
		aDstStart[G] = 32;
		aDstStart[B] = 64;
		aDstStart[A] = 96;
		aDstBits[R] = 32;
		aDstBits[G] = 32;
		aDstBits[B] = 32;
		aDstBits[A] = 32;
		break;
	case TEXFMT_NORMALMAP_RXGB:
		d3dFormat = D3DFMT_A8R8G8B8;
		iDstSize = 4;
		bDstFloat = false;

		aDstStart[R] = 24;
		aDstStart[G] = 8;
		aDstStart[B] = 0;
		aDstBits[R] = 8;
		aDstBits[G] = 8;
		aDstBits[B] = 8;

		if (bitmap.GetBMPType() == BITMAP_RGBA && iPitch == bitmap.GetWidth() * iDstSize)
		{
		}
		else if (bitmap.GetBMPType() == BITMAP_RGB && iPitch == bitmap.GetWidth() * iDstSize)
		{
		}
		else
		{
			memset(pDstData, 255, bitmap.GetWidth() * bitmap.GetHeight() * sizeof(iDstSize));
		}

		break;
	case TEXFMT_NORMALMAP_S:
		d3dFormat = D3DFMT_A8R8G8B8;
		iDstSize = 4;
		bDstFloat = false;

		aDstStart[A] = 8;
		aDstBits[A] = 8;

		if (bitmap.GetBMPType() == BITMAP_RGBA && iPitch == bitmap.GetWidth() * iDstSize)
		{
		}
		else
		{
			memset(pDstData, 255, bitmap.GetWidth() * bitmap.GetHeight() * sizeof(iDstSize));
		}

		break;
	}

	SBitmapFormatDescriptor cSrc;
	bitmap.GetFormatDescriptor(cSrc);

	const byte *pSrcData(static_cast<const byte *>(bitmap.GetBuffer()));

	int iExtra(iPitch - bitmap.GetWidth() * iDstSize);
	//int iExtra(0);

	// First some special optimized cases for common integer->integer textures
	if (bitmap.GetBMPType() == BITMAP_RGBA && eFormat == TEXFMT_A8R8G8B8 && iExtra == 0)
	{
		int iNumPixels(bitmap.GetWidth() * bitmap.GetHeight());
		for (int d(0); d < iNumPixels; ++d, pDstData += 4, pSrcData += 4)
		{
			pDstData[0] = pSrcData[2];
			pDstData[1] = pSrcData[1];
			pDstData[2] = pSrcData[0];
			pDstData[3] = pSrcData[3];
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_RGB && eFormat == TEXFMT_A8R8G8B8 && iExtra == 0)
	{
		int iNumPixels(bitmap.GetWidth() * bitmap.GetHeight());
		for (int d(0); d < iNumPixels; ++d, pDstData += 4, pSrcData += 3)
		{
			pDstData[0] = pSrcData[2];
			pDstData[1] = pSrcData[1];
			pDstData[2] = pSrcData[0];
			pDstData[3] = 255;
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_RGBA && eFormat == TEXFMT_NORMALMAP_RXGB && iExtra == 0)
	{
		int iNumPixels(bitmap.GetWidth() * bitmap.GetHeight());
		for (int d(0); d < iNumPixels; ++d, pDstData += 4, pSrcData += 4)
		{
			pDstData[0] = pSrcData[2];
			pDstData[1] = pSrcData[1];
			pDstData[2] = 255;
			pDstData[3] = pSrcData[0];
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_RGB && eFormat == TEXFMT_NORMALMAP_RXGB && iExtra == 0)
	{
		int iNumPixels(bitmap.GetWidth() * bitmap.GetHeight());
		for (int d(0); d < iNumPixels; ++d, pDstData += 4, pSrcData += 3)
		{
			pDstData[0] = pSrcData[2];
			pDstData[1] = pSrcData[1];
			pDstData[2] = 255;
			pDstData[3] = pSrcData[0];
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_RGBA && eFormat == TEXFMT_NORMALMAP_S && iExtra == 0)
	{
		int iNumPixels(bitmap.GetWidth() * bitmap.GetHeight());
		for (int d(0); d < iNumPixels; ++d, pDstData += 4, pSrcData += 4)
		{
			pDstData[0] = 255;
			pDstData[1] = pSrcData[3];
			pDstData[2] = 255;
			pDstData[3] = 255;
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_ALPHA && eFormat == TEXFMT_A8 && iExtra == 0)
	{
		MemManager.Copy(pDstData, pSrcData, bitmap.GetWidth() * bitmap.GetHeight());
	}
	else if (bitmap.GetBMPType() == BITMAP_ALPHA && eFormat == TEXFMT_A8L8 && iExtra == 0)
	{
		int iNumPixels(bitmap.GetWidth() * bitmap.GetHeight());
		for (int d(0); d < iNumPixels; ++d, pDstData += 2, pSrcData += 1)
		{
			pDstData[0] = 255;
			pDstData[1] = pSrcData[0];
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_ALPHA &&
			bitmap.GetRedChannel() == BMP_CHANNEL_WHITE &&
			bitmap.GetGreenChannel() == BMP_CHANNEL_WHITE &&
			bitmap.GetBlueChannel() == BMP_CHANNEL_WHITE &&
			bitmap.GetAlphaChannel() == 0 &&
			eFormat == TEXFMT_A8R8G8B8 &&
			iExtra == 0)
	{
		int iNumPixels(bitmap.GetWidth() * bitmap.GetHeight());
		for (int d(0); d < iNumPixels; ++d, pDstData += 4, pSrcData += 1)
		{
			pDstData[0] = 255;
			pDstData[1] = 255;
			pDstData[2] = 255;
			pDstData[3] = pSrcData[0];
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_ALPHA &&
			bitmap.GetRedChannel() == 0 &&
			bitmap.GetGreenChannel() == 0 &&
			bitmap.GetBlueChannel() == 0 &&
			bitmap.GetAlphaChannel() == BMP_CHANNEL_WHITE &&
			eFormat == TEXFMT_A8R8G8B8 &&
			iExtra == 0)
	{
		int iNumPixels(bitmap.GetWidth() * bitmap.GetHeight());
		for (int d(0); d < iNumPixels; ++d, pDstData += 4, pSrcData += 1)
		{
			pDstData[0] = pSrcData[0];
			pDstData[1] = pSrcData[0];
			pDstData[2] = pSrcData[0];
			pDstData[3] = 255;
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_RGBA && eFormat == TEXFMT_A8R8G8B8)
	{
		for (int y(0); y < bitmap.GetHeight(); ++y, pDstData += iExtra)
		{
			for (int x(0); x < bitmap.GetWidth(); ++x, pDstData += 4, pSrcData += 4)
			{
				pDstData[0] = pSrcData[2];
				pDstData[1] = pSrcData[1];
				pDstData[2] = pSrcData[0];
				pDstData[3] = pSrcData[3];
			}
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_RGB && eFormat == TEXFMT_A8R8G8B8)
	{
		for (int y(0); y < bitmap.GetHeight(); ++y, pDstData += iExtra)
		{
			for (int x(0); x < bitmap.GetWidth(); ++x, pDstData += 4, pSrcData += 3)
			{
				pDstData[0] = pSrcData[2];
				pDstData[1] = pSrcData[1];
				pDstData[2] = pSrcData[0];
				pDstData[3] = 255;
			}
		}
	}
	else if (bitmap.GetBMPType() == BITMAP_ALPHA && eFormat == TEXFMT_A8)
	{
		for (int y(0); y < bitmap.GetHeight(); ++y, pDstData += bitmap.GetWidth() * iDstSize + iExtra)
			MemManager.Copy(pDstData, pSrcData, bitmap.GetWidth());
	}
	else if (bDstFloat && cSrc.bFloat)
	{
		for (int y(0); y < bitmap.GetHeight(); ++y, pDstData += iExtra)
		{
			for (int x(0); x < bitmap.GetWidth(); ++x, pDstData += iDstSize, pSrcData += cSrc.iPixelBytes)
			{
				for (int iChannel(0); iChannel < 4; ++iChannel)
				{
					if (aDstStart[iChannel] != -1)
					{
						if (cSrc.aStart[iChannel] != -1)
						{
							uint uiValue(BitVectorGetInteger(reinterpret_cast<const dword *>(pSrcData), cSrc.aStart[iChannel], cSrc.aBits[iChannel]));

							if (cSrc.aBits[iChannel] == aDstBits[iChannel])
								BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, aDstBits[iChannel]);
							else
							{
								if (cSrc.aBits[iChannel] == 32 && aDstBits[iChannel] == 16)
								{
									float fValue(*reinterpret_cast<float *>(&uiValue));
									D3DXFLOAT16 f16Value(fValue);

									uiValue = (*reinterpret_cast<uint *>(&f16Value)) & 0xffff;

									BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, 16);
								}
								else if (cSrc.aBits[iChannel] == 16 && aDstBits[iChannel] == 32)
								{
									D3DXFLOAT16 f16Value(*reinterpret_cast<D3DXFLOAT16 *>(&uiValue));
									float fValue(f16Value);

									uiValue = *reinterpret_cast<uint *>(&fValue);

									BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, 32);
								}
							}
						}
						else
						{
							if (aDstBits[iChannel] == 32)
							{
								uint uiValue(*reinterpret_cast<uint *>(&cSrc.aFloatDefault[iChannel]));
								BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, 32);
							}
							else if (aDstBits[iChannel] == 16)
							{
								uint uiValue((*reinterpret_cast<uint *>(&D3DXFLOAT16(cSrc.aFloatDefault[iChannel]))) & 0xffff);
								BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, 16);
							}
						}
					}
				}
			}
		}
	}
	else if (bDstFloat) // Integer to float texture
	{
		for (int y(0); y < bitmap.GetHeight(); ++y, pDstData += iExtra)
		{
			for (int x(0); x < bitmap.GetWidth(); ++x, pDstData += iDstSize, pSrcData += cSrc.iPixelBytes)
			{
				for (int iChannel(0); iChannel < 4; ++iChannel)
				{
					if (aDstStart[iChannel] != -1)
					{
						if (cSrc.aStart[iChannel] != -1)
						{
							uint uiValue(BitVectorGetInteger(reinterpret_cast<const dword *>(pSrcData), cSrc.aStart[iChannel], cSrc.aBits[iChannel]));

							if (aDstBits[iChannel] == 32)
							{
								float fValue(uiValue / float((1 << cSrc.aBits[iChannel]) - 1)); // remap to [0-1]

								uiValue = *reinterpret_cast<uint *>(&fValue);

								BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, 32);
							}
							else if (aDstBits[iChannel] == 16)
							{
								float fValue(uiValue / float((1 << cSrc.aBits[iChannel]) - 1)); // remap to [0-1]

								D3DXFLOAT16 f16Value(fValue);

								uiValue = (*reinterpret_cast<uint *>(&f16Value)) & 0xffff;

								BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, 16);
							}
						}
						else
						{
							if (aDstBits[iChannel] == 32)
							{
								uint uiValue(*reinterpret_cast<uint *>(&cSrc.aFloatDefault[iChannel]));
								BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, 32);
							}
							else if (aDstBits[iChannel] == 16)
							{
								uint uiValue((*reinterpret_cast<uint *>(&D3DXFLOAT16(cSrc.aFloatDefault[iChannel]))) & 0xffff);
								BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, 16);
							}
						}
					}
				}
			}
		}
	}
	else if (cSrc.bFloat) // Float to integer texture
	{
		for (int y(0); y < bitmap.GetHeight(); ++y, pDstData += iExtra)
		{
			for (int x(0); x < bitmap.GetWidth(); ++x, pDstData += iDstSize, pSrcData += cSrc.iPixelBytes)
			{
				for (int iChannel(0); iChannel < 4; ++iChannel)
				{
					if (aDstStart[iChannel] != -1)
					{
						if (cSrc.aStart[iChannel] != -1)
						{
							uint uiValue(BitVectorGetInteger(reinterpret_cast<const dword *>(pSrcData), cSrc.aStart[iChannel], cSrc.aBits[iChannel]));

							if (cSrc.aBits[iChannel] == 32)
							{
								float fValue(*reinterpret_cast<float *>(&uiValue));
								uiValue = *reinterpret_cast<uint *>(&fValue);
							}
							else if (cSrc.aBits[iChannel] == 16)
							{
								D3DXFLOAT16 f16Value(*reinterpret_cast<D3DXFLOAT16 *>(&uiValue));
								uiValue = (*reinterpret_cast<uint *>(&f16Value)) & 0xffff;
							}

							BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], uiValue, aDstBits[iChannel]);
						}
						else
						{
							BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], cSrc.aIntDefault[iChannel], aDstBits[iChannel]);
						}
					}
				}
			}
		}
	}
	else
	{
		for (int y(0); y < bitmap.GetHeight(); ++y, pDstData += iExtra)
		{
			for (int x(0); x < bitmap.GetWidth(); ++x, pDstData += iDstSize, pSrcData += cSrc.iPixelBytes)
			{
				for (int iChannel(0); iChannel < 4; ++iChannel)
				{
					if (aDstStart[iChannel] != -1)
					{
						if (cSrc.aStart[iChannel] != -1)
							BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], BitVectorGetInteger(reinterpret_cast<const dword *>(pSrcData), cSrc.aStart[iChannel], cSrc.aBits[iChannel]), aDstBits[iChannel]);
						else if (aDstBits[iChannel] != 0)
							BitVectorSetInteger(reinterpret_cast<dword *>(pDstData), aDstStart[iChannel], cSrc.aIntDefault[iChannel], aDstBits[iChannel]);
					}
				}
			}
		}
	}
}


/*====================
  D3D_Register2DTexture
  ====================*/
int		D3D_Register2DTexture(const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat, const tstring &sName)
{
	try
	{
		// Find empty texture slot
		g_llSectionStart = K2System.GetTicks();
		int iIndex(MAX_TEXTURES);
		for (int i(0); i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] != NULL)
				continue;

			iIndex = i;
			break;
		}
		if (iIndex == MAX_TEXTURES)
			EX_WARN(_T("No free texture slots"));
		g_llIndexLookup = K2System.GetTicks() - g_llSectionStart;

		g_llSectionStart = K2System.GetTicks();
		g_llFileInfo = K2System.GetTicks() - g_llSectionStart;

		// Resize, if necessary
		g_llSectionStart = K2System.GetTicks();
		const CBitmap *pImage(NULL);
		CBitmap bmpResized;

		int iNewWidth(bitmap.GetWidth());
		int iNewHeight(bitmap.GetHeight());

		bool bPerf(false); // Generate a performance warning

		if (vid_textureDownsize > 0 && !(iTextureFlags & TEX_FULL_QUALITY))
		{
			iNewWidth = MAX(M_FloorPow2(iNewWidth) >> vid_textureDownsize, 2);
			iNewHeight = MAX(M_FloorPow2(iNewHeight)  >> vid_textureDownsize, 2);
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
				Console.Perf << "Resizing " << sName << " from "
					<< bitmap.GetWidth() << "x" << bitmap.GetHeight() << " to "
					<< iNewWidth << "x" << iNewHeight << newl;
			}
			else
			{
				Console.Dev << "Resizing " << sName << " from "
					<< bitmap.GetWidth() << "x" << bitmap.GetHeight() << " to "
					<< iNewWidth << "x" << iNewHeight << newl;
			}

			bitmap.Scale(bmpResized, iNewWidth, iNewHeight);
			pImage = &bmpResized;
		}
		else
		{
			pImage = &bitmap;
		}
		g_llResize = K2System.GetTicks() - g_llSectionStart;

		IDirect3DTexture9 *pTexture(NULL);
		D3DFORMAT d3dFormat;
		switch (eFormat)
		{
		default:
		case TEXFMT_A8R8G8B8:
		case TEXFMT_NORMALMAP:
		case TEXFMT_NORMALMAP_RXGB:
		case TEXFMT_NORMALMAP_S:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case TEXFMT_A1R5G5B5:
			d3dFormat = D3DFMT_A1R5G5B5;
			break;
		case TEXFMT_A4R4G4B4:
			d3dFormat = D3DFMT_A4R4G4B4;
			break;
		case TEXFMT_A8:
			d3dFormat = D3DFMT_A8;
			break;
		case TEXFMT_A8L8:
			d3dFormat = D3DFMT_A8L8;
			break;
		case TEXFMT_R16G16:
			d3dFormat = D3DFMT_G16R16;
			break;
		case TEXFMT_U8V8:
			d3dFormat = D3DFMT_V8U8;
			break;
		case TEXFMT_U16V16:
			d3dFormat = D3DFMT_V16U16;
			break;
		case TEXFMT_R16F:
			d3dFormat = D3DFMT_R16F;
			break;
		case TEXFMT_R16G16F:
			d3dFormat = D3DFMT_G16R16F;
			break;
		case TEXFMT_A16R16G16B16F:
			d3dFormat = D3DFMT_A16B16G16R16F;
			break;
		case TEXFMT_R32F:
			d3dFormat = D3DFMT_R32F;
			break;
		case TEXFMT_R32G32F:
			d3dFormat = D3DFMT_G32R32F;
			break;
		case TEXFMT_A32R32G32B32F:
			d3dFormat = D3DFMT_A32B32G32R32F;
			break;
		}

		g_llSectionStart = K2System.GetTicks();
		if (vid_textureAutogenMipmaps || (iTextureFlags & TEX_NO_MIPMAPS))
		{
			// Only fill the top texture layer

			// Create the texture
			if (FAILED(g_pd3dDevice->CreateTexture(pImage->GetWidth(), pImage->GetHeight(),
				(iTextureFlags & TEX_NO_MIPMAPS) ? 1 : 0,
				(iTextureFlags & TEX_NO_MIPMAPS) ? 0 : D3DUSAGE_AUTOGENMIPMAP,
				d3dFormat, D3DPOOL_MANAGED, &pTexture, NULL)))
				EX_WARN(_T("CreateTexture failed"));

			// Fill the surface
			D3DLOCKED_RECT d3drect;
			if (FAILED(pTexture->LockRect(0, &d3drect, NULL, 0)))
				EX_WARN(_T("LockRect failed"));

			if (d3drect.pBits)
				D3D_CopyBitmapToBuffer(*pImage, eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

			if (FAILED(pTexture->UnlockRect(0)))
				EX_WARN(_T("UnlockRect failed"));
		}
		else
		{
			// Generate mipmaps and fill all texture layers

			CBitmap bmpCopy;
			CBitmap bmpWorking(*pImage);

			// Create the texture
			if (FAILED(g_pd3dDevice->CreateTexture(bmpWorking.GetWidth(), bmpWorking.GetHeight(), 0, 0,
				d3dFormat, D3DPOOL_MANAGED, &pTexture, NULL)))
				EX_WARN(_T("CreateTexture failed"));

			for (uint uiLevel(0); uiLevel < pTexture->GetLevelCount(); ++uiLevel)
			{
				// Fill the surface
				D3DLOCKED_RECT d3drect;
				if (FAILED(pTexture->LockRect(uiLevel, &d3drect, NULL, 0)))
					EX_WARN(_T("LockRect failed"));

				if (d3drect.pBits)
					D3D_CopyBitmapToBuffer(bmpWorking, eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

				bmpWorking.Copy(bmpCopy);

				if (vid_textureGammaCorrect != 1.0f && eFormat != TEXFMT_NORMALMAP && eFormat != TEXFMT_NORMALMAP_RXGB)
					bmpCopy.Scale(bmpWorking, MAX(bmpCopy.GetWidth() >> 1, 1), MAX(bmpCopy.GetHeight() >> 1, 1), vid_textureGammaCorrect);
				else
					bmpCopy.Scale(bmpWorking, MAX(bmpCopy.GetWidth() >> 1, 1), MAX(bmpCopy.GetHeight() >> 1, 1));

				if (FAILED(pTexture->UnlockRect(uiLevel)))
					EX_WARN(_T("UnlockRect failed"));
			}
		}
		g_llMipMaps = K2System.GetTicks() - g_llSectionStart;

		D3DFORMAT eCompressedFormat((bitmap.GetBMPType() == BITMAP_RGB || eFormat == TEXFMT_NORMALMAP_S) && eFormat != TEXFMT_NORMALMAP_RXGB ? D3DFMT_DXT1 : D3DFMT_DXT5);

		// Create the compressed version
		g_llSectionStart = K2System.GetTicks();
		if (vid_textureCompression && !(iTextureFlags & TEX_NO_COMPRESS) && pImage->GetWidth() >= 4 && pImage->GetHeight() >= 4)
			pTexture = D3D_CompressTexture(pTexture, eCompressedFormat, !(iTextureFlags & TEX_NO_MIPMAPS));
		g_llCompress = K2System.GetTicks() - g_llSectionStart;

		if (vid_texturePreload)
		{
			g_llSectionStart = K2System.GetTicks();
			pTexture->PreLoad();
			g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
		}
		else
			g_llPreLoad = 0;

		g_pTextures[iIndex] = g_pTextures2D[iIndex] = pTexture;
		return iIndex;
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_Register2DTexture(") + sName + _TS(") - "), NO_THROW);
		return -1;
	}
}


/*====================
  D3D_ArchivedTextureExists
  ====================*/
bool	D3D_ArchivedTextureExists(const tstring &sFilename, uint uiTextureFlags)
{
	try
	{
		if (uiTextureFlags & TEX_FULL_QUALITY)
			D3D_TextureDefine("DOWNSIZE", "0");
		else
			D3D_TextureDefine("DOWNSIZE", TStringToString(XtoA(vid_textureDownsize)));

		if (uiTextureFlags & TEX_NO_COMPRESS)
			D3D_TextureDefine("COMPRESSION", "0");
		else
			D3D_TextureDefine("COMPRESSION", vid_textureCompression ? "1" : "0");

		if (uiTextureFlags & TEX_NO_MIPMAPS)
			D3D_TextureDefine("MIPMAPS", "0");
		else
			D3D_TextureDefine("MIPMAPS", "1");

		tstring sCleanFilename(FileManager.SanitizePath(sFilename));

		for (vector<CTextureArchive*>::iterator it(g_vTextureArchives.begin()); it != g_vTextureArchives.end(); ++it)
		{
			(*it)->ActivateNode(D3D_GetTextureDefinitionString());

			if ((*it)->TextureExists(sCleanFilename))
				return true;
		}

		return false;
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_ArchivedTextureExists(") + sFilename + _TS(") - "), NO_THROW);
		return false;
	}
}


/*====================
  D3D_RegisterArchivedTexture
  ====================*/
int		D3D_RegisterArchivedTexture(CTexture *pTexture, const tstring &sSuffix = TSNULL)
{
	try
	{
		// Find empty texture slot
		g_llSectionStart = K2System.GetTicks();
		int iIndex(MAX_TEXTURES);
		for (int i(0); i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] != NULL)
				continue;

			iIndex = i;
			break;
		}
		if (iIndex == MAX_TEXTURES)
			EX_WARN(_T("No free texture slots"));
		g_llIndexLookup = K2System.GetTicks() - g_llSectionStart;

		// Load the file
		g_llSectionStart = K2System.GetTicks();

		int iTextureFlags(pTexture->GetTextureFlags());

		if (pTexture->GetType() == TEXTURE_VOLUME)
			iTextureFlags |= TEX_NO_COMPRESS;

		if (iTextureFlags & TEX_FULL_QUALITY)
			D3D_TextureDefine("DOWNSIZE", "0");
		else
			D3D_TextureDefine("DOWNSIZE", TStringToString(XtoA(vid_textureDownsize)));

		if (iTextureFlags & TEX_NO_COMPRESS)
			D3D_TextureDefine("COMPRESSION", "0");
		else
			D3D_TextureDefine("COMPRESSION", vid_textureCompression ? "1" : "0");

		if (iTextureFlags & TEX_NO_MIPMAPS)
			D3D_TextureDefine("MIPMAPS", "0");
		else
			D3D_TextureDefine("MIPMAPS", "1");

		const tstring &sPath(!sSuffix.empty() ? Filename_AppendSuffix(pTexture->GetPath(), sSuffix) : pTexture->GetPath());

		CFileHandle hTexture;
		for (vector<CTextureArchive*>::iterator it(g_vTextureArchives.begin()); it != g_vTextureArchives.end(); ++it)
		{
			(*it)->ActivateNode(D3D_GetTextureDefinitionString());

			(*it)->LoadTexture(sPath, hTexture);
			if (hTexture.IsOpen())
				break;
		}

		if (!hTexture.IsOpen())
			return -1;

		uint uiSize(0);
		LPCVOID pData(hTexture.GetBuffer(uiSize));
		g_llFileRead = K2System.GetTicks() - g_llSectionStart;

		g_llSectionStart = K2System.GetTicks();
		if (pTexture->GetType() == TEXTURE_CUBE)
		{
			IDirect3DCubeTexture9*	pD3DCubeTexture(NULL);
			if (D3DXCreateCubeTextureFromFileInMemoryEx(
				g_pd3dDevice,
				pData, uiSize,
				D3DX_DEFAULT,							// Size
				D3DX_FROM_FILE,							// Mip Levels
				0,										// Usage
				D3DFMT_UNKNOWN,							// Format
				D3DPOOL_MANAGED,						// Memory Pool
				D3DX_DEFAULT, D3DX_DEFAULT,				// Filter/MipFilter
				0, NULL, NULL, &pD3DCubeTexture) != D3D_OK)
				EX_ERROR(_T("D3DXCreateCubeTextureFromFileInMemoryEx failed: ") + K2System.GetLastErrorString());

			g_llRegister = K2System.GetTicks() - g_llSectionStart;

			if (vid_texturePreload)
			{
				g_llSectionStart = K2System.GetTicks();
				pD3DCubeTexture->PreLoad();
				g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
			}
			else
				g_llPreLoad = 0;

			g_pTextures[iIndex] = g_pTexturesCube[iIndex] = pD3DCubeTexture;
		}
		else if (pTexture->GetType() == TEXTURE_VOLUME)
		{
			IDirect3DVolumeTexture9*	pD3DVolumeTexture(NULL);
			if (D3DXCreateVolumeTextureFromFileInMemoryEx(
				g_pd3dDevice,
				pData, uiSize,
				D3DX_DEFAULT, D3DX_DEFAULT,				// Width/Height
				D3DX_DEFAULT,							// Depth
				D3DX_FROM_FILE,							// Mip Levels
				0,										// Usage
				D3DFMT_UNKNOWN,							// Format
				D3DPOOL_MANAGED,						// Memory Pool
				D3DX_DEFAULT, D3DX_DEFAULT,				// Filter/MipFilter
				0, NULL, NULL, &pD3DVolumeTexture) != D3D_OK)
				EX_ERROR(_T("D3DXCreateVolumeTextureFromFileInMemoryEx failed: ") + K2System.GetLastErrorString());

			g_llRegister = K2System.GetTicks() - g_llSectionStart;

			if (vid_texturePreload)
			{
				g_llSectionStart = K2System.GetTicks();
				pD3DVolumeTexture->PreLoad();
				g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
			}
			else
				g_llPreLoad = 0;

			g_pTextures[iIndex] = g_pTexturesVolume[iIndex] = pD3DVolumeTexture;
		}
		else
		{
			IDirect3DTexture9*	pD3DTexture(NULL);
			if (D3DXCreateTextureFromFileInMemoryEx(
				g_pd3dDevice,
				pData, uiSize,
				D3DX_DEFAULT, D3DX_DEFAULT,				// Width/Height
				D3DX_FROM_FILE,							// Mip Levels
				0,										// Usage
				D3DFMT_UNKNOWN,							// Format
				D3DPOOL_MANAGED,						// Memory Pool
				D3DX_DEFAULT, D3DX_DEFAULT,				// Filter/MipFilter
				0, NULL, NULL, &pD3DTexture) != D3D_OK)
				EX_ERROR(_T("D3DXCreateTextureFromFileInMemoryEx failed: ") + K2System.GetLastErrorString());

			g_llRegister = K2System.GetTicks() - g_llSectionStart;

			if (vid_texturePreload)
			{
				g_llSectionStart = K2System.GetTicks();
				pD3DTexture->PreLoad();
				g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
			}
			else
				g_llPreLoad = 0;

			g_pTextures[iIndex] = g_pTextures2D[iIndex] = pD3DTexture;
		}

		hTexture.Close();

		return iIndex;
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_RegisterArchivedTexture(") + pTexture->GetPath() + _TS(") - "), NO_THROW);
		return -1;
	}
}


/*====================
  D3D_RegisterCachedTexture
  ====================*/
int		D3D_RegisterCachedTexture(CTexture *pTexture, const tstring &sReference, const tstring &sSuffix)
{
	try
	{
		// Find empty texture slot
		g_llSectionStart = K2System.GetTicks();
		int iIndex(MAX_TEXTURES);
		for (int i(0); i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] != NULL)
				continue;

			iIndex = i;
			break;
		}
		if (iIndex == MAX_TEXTURES)
			EX_WARN(_T("No free texture slots"));
		g_llIndexLookup = K2System.GetTicks() - g_llSectionStart;

		// Load the file
		g_llSectionStart = K2System.GetTicks();

		int iTextureFlags(pTexture->GetTextureFlags());

		if (pTexture->GetType() == TEXTURE_VOLUME)
			iTextureFlags |= TEX_NO_COMPRESS;

		if (iTextureFlags & TEX_FULL_QUALITY)
			D3D_TextureDefine("DOWNSIZE", "0");
		else
			D3D_TextureDefine("DOWNSIZE", TStringToString(XtoA(vid_textureDownsize)));

		if (iTextureFlags & TEX_NO_COMPRESS)
			D3D_TextureDefine("COMPRESSION", "0");
		else
			D3D_TextureDefine("COMPRESSION", vid_textureCompression ? "1" : "0");

		if (iTextureFlags & TEX_NO_MIPMAPS)
			D3D_TextureDefine("MIPMAPS", "0");
		else
			D3D_TextureDefine("MIPMAPS", "1");

		g_TextureCache.ActivateNode(D3D_GetTextureDefinitionString());

		tstring sFilePath;

		if (!g_TextureCache.LoadTexture(Filename_AppendSuffix(pTexture->GetPath(), sSuffix), sReference, sFilePath))
			return -1;

		tstring sSystemPath(FileManager.GetSystemPath(sFilePath));

		g_llFileRead = K2System.GetTicks() - g_llSectionStart;

		g_llSectionStart = K2System.GetTicks();
		if (pTexture->GetType() == TEXTURE_CUBE)
		{
			IDirect3DCubeTexture9*	pD3DCubeTexture(NULL);
			if (D3DXCreateCubeTextureFromFileEx(
				g_pd3dDevice,
				sSystemPath.c_str(),
				D3DX_DEFAULT,							// Size
				D3DX_FROM_FILE,							// Mip Levels
				0,										// Usage
				D3DFMT_UNKNOWN,							// Format
				D3DPOOL_MANAGED,						// Memory Pool
				D3DX_DEFAULT, D3DX_DEFAULT,				// Filter/MipFilter
				0, NULL, NULL, &pD3DCubeTexture) != D3D_OK)
				EX_ERROR(_T("D3DXCreateCubeTextureFromFileEx failed: ") + K2System.GetLastErrorString());

			g_llRegister = K2System.GetTicks() - g_llSectionStart;

			if (vid_texturePreload)
			{
				g_llSectionStart = K2System.GetTicks();
				pD3DCubeTexture->PreLoad();
				g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
			}
			else
				g_llPreLoad = 0;

			g_pTextures[iIndex] = g_pTexturesCube[iIndex] = pD3DCubeTexture;
		}
		else if (pTexture->GetType() == TEXTURE_VOLUME)
		{
			IDirect3DVolumeTexture9*	pD3DVolumeTexture(NULL);
			if (D3DXCreateVolumeTextureFromFileEx(
				g_pd3dDevice,
				sSystemPath.c_str(),
				D3DX_DEFAULT, D3DX_DEFAULT,				// Width/Height
				D3DX_DEFAULT,							// Depth
				D3DX_FROM_FILE,							// Mip Levels
				0,										// Usage
				D3DFMT_UNKNOWN,							// Format
				D3DPOOL_MANAGED,						// Memory Pool
				D3DX_DEFAULT, D3DX_DEFAULT,				// Filter/MipFilter
				0, NULL, NULL, &pD3DVolumeTexture) != D3D_OK)
				EX_ERROR(_T("D3DXCreateVolumeTextureFromFileEx failed: ") + K2System.GetLastErrorString());

			g_llRegister = K2System.GetTicks() - g_llSectionStart;

			if (vid_texturePreload)
			{
				g_llSectionStart = K2System.GetTicks();
				pD3DVolumeTexture->PreLoad();
				g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
			}
			else
				g_llPreLoad = 0;

			g_pTextures[iIndex] = g_pTexturesVolume[iIndex] = pD3DVolumeTexture;
		}
		else
		{
			IDirect3DTexture9*	pD3DTexture(NULL);
			if (D3DXCreateTextureFromFileEx(
				g_pd3dDevice,
				sSystemPath.c_str(),
				D3DX_DEFAULT, D3DX_DEFAULT,				// Width/Height
				D3DX_FROM_FILE,							// Mip Levels
				0,										// Usage
				D3DFMT_UNKNOWN,							// Format
				D3DPOOL_MANAGED,						// Memory Pool
				D3DX_DEFAULT, D3DX_DEFAULT,				// Filter/MipFilter
				0, NULL, NULL, &pD3DTexture) != D3D_OK)
				EX_ERROR(_T("D3DXCreateTextureFromFileEx failed: ") + K2System.GetLastErrorString());

			g_llRegister = K2System.GetTicks() - g_llSectionStart;

			if (vid_texturePreload)
			{
				g_llSectionStart = K2System.GetTicks();
				pD3DTexture->PreLoad();
				g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
			}
			else
				g_llPreLoad = 0;

			g_pTextures[iIndex] = g_pTextures2D[iIndex] = pD3DTexture;
		}

		return iIndex;
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_RegisterCachedTexture(") + pTexture->GetPath() + _TS(") - "), NO_THROW);
		return -1;
	}
}


/*====================
  D3D_RegisterCachedTexture
  ====================*/
int		D3D_RegisterCachedTexture(CTexture *pTexture)
{
	return D3D_RegisterCachedTexture(pTexture, pTexture->GetPath(), TSNULL);
}


static CTextureArchive *s_pTextureArchive(NULL);
static sset s_setIgnore;

/*--------------------
  WriteTextureArchive
  --------------------*/
CMD(WriteTextureArchive)
{
	if (g_vTextureArchives.empty())
		return false;

	s_setIgnore.clear();

	CFile *pFile(NULL);

	pFile = FileManager.GetFile(_T(":/texture_ignore.txt"), FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_TEXT);

	if (pFile != NULL)
	{
		tstring sFilename;

		while (!pFile->IsEOF())
		{
			sFilename = pFile->ReadLine();

			StripNewline(sFilename);
			sFilename = LowerString(sFilename);

			if (!sFilename.empty())
				s_setIgnore.insert(sFilename);
		}
	}

	s_pTextureArchive = g_vTextureArchives.front();
	s_pTextureArchive->Clear();

	// Client stuff
	Host.LoadAllClientResources();

	int iOldDownsize(vid_textureDownsize);
	bool bOldTextureCache(vid_textureCache);
	
	vid_textureCache = false;

	tstring sOldModelQuality(ICvar::GetString(_T("model_quality")));

	// Load all LODs
	if (sOldModelQuality != _T("high"))
	{
		ICvar::SetString(_T("model_quality"), _T("high"));
		g_ResourceManager.GetLib(RES_MODEL)->ReloadAll();
	}
	if (sOldModelQuality != _T("med"))
	{
		ICvar::SetString(_T("model_quality"), _T("med"));
		g_ResourceManager.GetLib(RES_MODEL)->ReloadAll();
	}
	if (sOldModelQuality != _T("low"))
	{
		ICvar::SetString(_T("model_quality"), _T("low"));
		g_ResourceManager.GetLib(RES_MODEL)->ReloadAll();
	}

	ICvar::SetString(_T("model_quality"), sOldModelQuality);

	for (int i(0); i <= 2; ++i)
	{
		//if (vid_textureDownsize != i)
		{
			vid_textureDownsize = i;
			g_ResourceManager.GetLib(RES_TEXTURE)->ReloadAll();
		}

		class CWriteTexturesFunctions : public CLoadJob<TextureMap>::IFunctions
		{
		public:


			~CWriteTexturesFunctions() {}
			float	Frame(TextureMap::iterator &it, float f) const
			{
				SetTitle(_T("Writing texture archive ") + ParenStr(_T("downsize ") + XtoA(vid_textureDownsize)));
				SetProgress(f);

				return 0.0f;
			}

			float	PostFrame(TextureMap::iterator &it, float f) const
			{
				if (it->first.empty() ||
					it->first[0] == _T('$') ||
					it->first[0] == _T('*'))
				{
					++it;
					return 1.0f;
				}

				bool bIgnored(false);
				tstring sLower(LowerString(it->first));

				for (sset::iterator itIgnore(s_setIgnore.begin()), itIgnoreEnd(s_setIgnore.end()); itIgnore != itIgnoreEnd; ++itIgnore)
				{
					if (EqualsWildcard(*itIgnore, sLower))
					{
						bIgnored = true;
						break;
					}
				}

				if (bIgnored)
				{
					++it;
					return 1.0f;
				}

				CTexture *pTexture(g_ResourceManager.GetTexture(it->second.hTexture));
				if (pTexture == NULL)
				{
					++it;
					return 1.0f;
				}

				int iTextureFlags(pTexture->GetTextureFlags());

				if (pTexture->GetType() == TEXTURE_VOLUME)
					iTextureFlags |= TEX_NO_COMPRESS;

				if (iTextureFlags & TEX_FULL_QUALITY)
					D3D_TextureDefine("DOWNSIZE", "0");
				else
					D3D_TextureDefine("DOWNSIZE", TStringToString(XtoA(vid_textureDownsize)));

				if (iTextureFlags & TEX_NO_COMPRESS)
					D3D_TextureDefine("COMPRESSION", "0");
				else
					D3D_TextureDefine("COMPRESSION", vid_textureCompression ? "1" : "0");

				if (iTextureFlags & TEX_NO_MIPMAPS)
					D3D_TextureDefine("MIPMAPS", "0");
				else
					D3D_TextureDefine("MIPMAPS", "1");

				s_pTextureArchive->ActivateNode(D3D_GetTextureDefinitionString());

				if (pTexture->GetFormat() == TEXFMT_NORMALMAP && vid_shaderRXGBNormalmap)
				{
					s_pTextureArchive->WriteTexture(Filename_AppendSuffix(it->first, _T("_rxgb")), g_pTextures[pTexture->GetIndex()], false);

					if (pTexture->GetIndex2() != -1)
						s_pTextureArchive->WriteTexture(Filename_AppendSuffix(it->first, _T("_s")), g_pTextures[pTexture->GetIndex2()], false);
				}
				else
				{
					s_pTextureArchive->WriteTexture(it->first, g_pTextures[pTexture->GetIndex()], false);
				}
				++it;

				return 1.0f;
			}
		};
		CWriteTexturesFunctions fnWriteTextures;
		CLoadJob<TextureMap> jobWriteTextures(g_mapTextures, &fnWriteTextures, LOADING_DISPLAY_INTERFACE);
		jobWriteTextures.Execute(g_mapTextures.size());
	}

	vid_textureCache = bOldTextureCache;

	if (vid_textureDownsize != iOldDownsize)
	{
		vid_textureDownsize = iOldDownsize;
		g_ResourceManager.GetLib(RES_TEXTURE)->ReloadAll();
	}

	// Reload texture archives
	D3D_OpenTextureArchive(true);

	return true;
}


/*====================
  D3D_Register2DTextureMipmaps

  Register a texture with custom/pregenerated mipmaps
  ====================*/
int		D3D_Register2DTextureMipmaps(const CBitmap bitmap[], int iNumLevels, int iTextureFlags, ETextureFormat eFormat, const tstring &sName)
{
	try
	{
		// Find empty texture slot
		int iIndex(MAX_TEXTURES);
		for (int i(0); i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] != NULL)
				continue;

			iIndex = i;
			break;
		}
		if (iIndex == MAX_TEXTURES)
			EX_WARN(_T("No free texture slots"));

		// Resize, if necessary
		const CBitmap **pImage = new const CBitmap *[iNumLevels];

#if 0
		int iNewWidth(bitmap[0].GetWidth());
		int iNewHeight(bitmap[0].GetHeight());

		bool bPerf(false); // Generate a performance warning

		if (vid_textureDownsize > 0 && !(iTextureFlags & TEX_FULL_QUALITY))
		{
			iNewWidth = MAX(M_FloorPow2(iNewWidth) >> vid_textureDownsize, 2);
			iNewHeight = MAX(M_FloorPow2(iNewHeight) >> vid_textureDownsize, 2);
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

		if (bitmap[0].GetWidth() != iNewWidth || bitmap[0].GetHeight() != iNewHeight)
		{
			if (bPerf)
			{
				Console.Perf << "Resizing " << sName << " from "
					<< bitmap[0].GetWidth() << "x" << bitmap[0].GetHeight() << " to "
					<< iNewWidth << "x" << iNewHeight << newl;
			}
			else
			{
				Console.Dev << "Resizing " << sName << " from "
					<< bitmap[0].GetWidth() << "x" << bitmap[0].GetHeight() << " to "
					<< iNewWidth << "x" << iNewHeight << newl;
			}

			bitmap[0].Scale(bmpResized, iNewWidth, iNewHeight);
			pImage[0] = &bmpResized;
		}
		else
		{
			pImage[0] = &bitmap;
		}
#endif

		for (int i(0); i < iNumLevels; ++i)
			pImage[i] = &bitmap[i];

		IDirect3DTexture9	*pTexture(NULL);

		D3DFORMAT d3dFormat;

		switch (eFormat)
		{
		default:
		case TEXFMT_A8R8G8B8:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case TEXFMT_A1R5G5B5:
			d3dFormat = D3DFMT_A1R5G5B5;
			break;
		case TEXFMT_A4R4G4B4:
			d3dFormat = D3DFMT_A4R4G4B4;
			break;
		case TEXFMT_A8:
			d3dFormat = D3DFMT_A8;
			break;
		case TEXFMT_A8L8:
			d3dFormat = D3DFMT_A8L8;
			break;
		case TEXFMT_R16G16:
			d3dFormat = D3DFMT_G16R16;
			break;
		case TEXFMT_U8V8:
			d3dFormat = D3DFMT_V8U8;
			break;
		case TEXFMT_U16V16:
			d3dFormat = D3DFMT_V16U16;
			break;
		case TEXFMT_R16F:
			d3dFormat = D3DFMT_R16F;
			break;
		case TEXFMT_R16G16F:
			d3dFormat = D3DFMT_G16R16F;
			break;
		case TEXFMT_A16R16G16B16F:
			d3dFormat = D3DFMT_A16B16G16R16F;
			break;
		case TEXFMT_R32F:
			d3dFormat = D3DFMT_R32F;
			break;
		case TEXFMT_R32G32F:
			d3dFormat = D3DFMT_G32R32F;
			break;
		case TEXFMT_A32R32G32B32F:
			d3dFormat = D3DFMT_A32B32G32R32F;
			break;
		}

		if (iTextureFlags & TEX_NO_MIPMAPS)
		{
			// Only fill the top texture layer

			// Create the texture
			if (FAILED(g_pd3dDevice->CreateTexture(pImage[0]->GetWidth(), pImage[0]->GetHeight(),
				1, 0, d3dFormat, D3DPOOL_MANAGED, &pTexture, NULL)))
				EX_WARN(_T("CreateTexture failed"));

			// Fill the surface
			D3DLOCKED_RECT d3drect;
			if (FAILED(pTexture->LockRect(0, &d3drect, NULL, 0)))
				EX_WARN(_T("LockRect failed"));

			if (d3drect.pBits)
				D3D_CopyBitmapToBuffer(*pImage[0], eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

			if (FAILED(pTexture->UnlockRect(0)))
				EX_WARN(_T("UnlockRect failed"));
		}
		else
		{
			// Fill all texture layers

			// Create the texture
			if (FAILED(g_pd3dDevice->CreateTexture(pImage[0]->GetWidth(), pImage[0]->GetHeight(), 0, 0,
				d3dFormat, D3DPOOL_MANAGED, &pTexture, NULL)))
				EX_WARN(_T("CreateTexture failed"));

			if (pTexture->GetLevelCount() != iNumLevels)
				EX_WARN(_T("pTexture->GetLevelCount() != iNumLevels"));

			for (uint uiLevel(0); uiLevel < pTexture->GetLevelCount(); ++uiLevel)
			{
				// Fill the surface
				D3DLOCKED_RECT d3drect;
				if (FAILED(pTexture->LockRect(uiLevel, &d3drect, NULL, 0)))
					EX_WARN(_T("LockRect failed"));

				if (d3drect.pBits)
					D3D_CopyBitmapToBuffer(*pImage[uiLevel], eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

				if (FAILED(pTexture->UnlockRect(uiLevel)))
					EX_WARN(_T("UnlockRect failed"));
			}
		}

		// Create the compressed version
#if 0
		if (vid_textureCompression && !(iTextureFlags & TEX_NO_COMPRESS) && pImage[0]->GetWidth() >= 4 && pImage[0]->GetHeight() >= 4)
			pTexture = D3D_CompressTexture(pImage[0]->GetBMPType(), pTexture);
#endif

		if (vid_texturePreload)
		{
			g_llSectionStart = K2System.GetTicks();
			pTexture->PreLoad();
			g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
		}
		else
			g_llPreLoad = 0;

		g_pTextures[iIndex] = g_pTextures2D[iIndex] = pTexture;
		return iIndex;
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_Register2DTextureMipmaps(") + sName + _TS(") - "), NO_THROW);
		return -1;
	}
}


/*====================
  D3D_Register2DProcedural
  ====================*/
int		D3D_Register2DProcedural(const CProcedural &cProcedural, int &iTexture2)
{
	try
	{
		ETextureFormat eFormat(cProcedural.GetFormat());

		if (cProcedural.IsMipmaps())
		{
			int iNumLevels(M_Log2(MAX(cProcedural.GetWidth(), cProcedural.GetHeight())) + 1);

			CBitmap *bmp = new CBitmap[iNumLevels];

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
				}
			}

			return D3D_Register2DTextureMipmaps(bmp, iNumLevels, cProcedural.GetFlags(), eFormat, cProcedural.GetName());
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
				iTexture2 = D3D_Register2DTexture(bmp, cProcedural.GetFlags(), TEXFMT_NORMALMAP_S, cProcedural.GetName());
				return D3D_Register2DTexture(bmp, cProcedural.GetFlags(), TEXFMT_NORMALMAP_RXGB, cProcedural.GetName());
			}
			else
			{
				iTexture2 = -1;
				return D3D_Register2DTexture(bmp, cProcedural.GetFlags(), eFormat, cProcedural.GetName());
			}
		}
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_Register2DProcedural(") + cProcedural.GetName() + _TS(") - "), NO_THROW);
		iTexture2 = -1;
		return -1;
	}
}


/*====================
  D3D_RegisterCubeTexture
  ====================*/
int		D3D_RegisterCubeTexture(const CBitmap bitmaps[], int iTextureFlags, ETextureFormat eFormat, const tstring &sName)
{
	try
	{
		// Find empty texture slot
		int iIndex(MAX_TEXTURES);
		for (int i(0); i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] != NULL)
				continue;

			iIndex = i;
			break;
		}
		if (iIndex == MAX_TEXTURES)
			EX_WARN(_T("No free texture slots"));

		// Resize, if necessary
		const CBitmap *pImage[6];
		CBitmap bmpResized[6];
		int iEdgeLength(bitmaps[0].GetWidth());

		int iNewEdgeLength(iEdgeLength);

		bool bPerf(false); // Generate a performance warning

		if (vid_textureDownsize > 0 && !(iTextureFlags & TEX_FULL_QUALITY))
		{
			iNewEdgeLength = MAX(M_FloorPow2(iNewEdgeLength) >> vid_textureDownsize, 2);
		}
		if (iNewEdgeLength > vid_textureMaxSize && !(iTextureFlags & TEX_FULL_QUALITY))
		{
			iNewEdgeLength = M_FloorPow2(MIN(iNewEdgeLength, int(vid_textureMaxSize)));
		}
		if (!M_IsPow2(iNewEdgeLength)) // Resize texture if its not a power of 2
		{
			iNewEdgeLength = M_FloorPow2(iNewEdgeLength);

			bPerf = true;
		}

		for (int iFace(0); iFace < 6; ++iFace)
		{
			if (bitmaps[iFace].GetWidth() != iNewEdgeLength || bitmaps[iFace].GetHeight() != iNewEdgeLength)
			{
				if (bPerf || bitmaps[iFace].GetWidth() != iEdgeLength || bitmaps[iFace].GetHeight() != iEdgeLength)
				{
					Console.Perf << "Resizing " << sName << " from "
						<< bitmaps[iFace].GetWidth() << "x" << bitmaps[iFace].GetHeight() << " to "
						<< iNewEdgeLength << "x" << iNewEdgeLength << newl;
				}
				else
				{
					Console.Dev << "Resizing " << sName << " from "
						<< bitmaps[iFace].GetWidth() << "x" << bitmaps[iFace].GetHeight() << " to "
						<< iNewEdgeLength << "x" << iNewEdgeLength << newl;
				}

				bitmaps[iFace].Scale(bmpResized[iFace], iNewEdgeLength, iNewEdgeLength);
				pImage[iFace] = &bmpResized[iFace];
			}
			else
			{
				pImage[iFace] = &bitmaps[iFace];
			}
		}

		IDirect3DCubeTexture9	*pTexture(NULL);

		D3DFORMAT d3dFormat;

		switch (eFormat)
		{
		default:
		case TEXFMT_A8R8G8B8:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case TEXFMT_A1R5G5B5:
			d3dFormat = D3DFMT_A1R5G5B5;
			break;
		case TEXFMT_A4R4G4B4:
			d3dFormat = D3DFMT_A4R4G4B4;
			break;
		case TEXFMT_A8:
			d3dFormat = D3DFMT_A8;
			break;
		case TEXFMT_A8L8:
			d3dFormat = D3DFMT_A8L8;
			break;
		case TEXFMT_R16G16:
			d3dFormat = D3DFMT_G16R16;
			break;
		case TEXFMT_U8V8:
			d3dFormat = D3DFMT_V8U8;
			break;
		case TEXFMT_U16V16:
			d3dFormat = D3DFMT_V16U16;
			break;
		case TEXFMT_R16F:
			d3dFormat = D3DFMT_R16F;
			break;
		case TEXFMT_R16G16F:
			d3dFormat = D3DFMT_G16R16F;
			break;
		case TEXFMT_A16R16G16B16F:
			d3dFormat = D3DFMT_A16B16G16R16F;
			break;
		case TEXFMT_R32F:
			d3dFormat = D3DFMT_R32F;
			break;
		case TEXFMT_R32G32F:
			d3dFormat = D3DFMT_G32R32F;
			break;
		case TEXFMT_A32R32G32B32F:
			d3dFormat = D3DFMT_A32B32G32R32F;
			break;
		}

		if (vid_textureAutogenMipmaps || (iTextureFlags & TEX_NO_MIPMAPS))
		{
			// Only fill the top texture layer

			// Create the texture
			if (FAILED(g_pd3dDevice->CreateCubeTexture(iNewEdgeLength,
				(iTextureFlags & TEX_NO_MIPMAPS) ? 1 : 0,
				(iTextureFlags & TEX_NO_MIPMAPS) ? 0 : D3DUSAGE_AUTOGENMIPMAP,
				d3dFormat, D3DPOOL_MANAGED, &pTexture, NULL)))
				EX_WARN(_T("CreateCubeTexture failed"));

			for (int iFace(0); iFace < 6; ++iFace)
			{
				// Fill the surface
				D3DLOCKED_RECT d3drect;
				if (FAILED(pTexture->LockRect(D3DCUBEMAP_FACES(iFace), 0, &d3drect, NULL, 0)))
					EX_WARN(_T("LockRect failed"));

				if (d3drect.pBits)
					D3D_CopyBitmapToBuffer(*(pImage[iFace]), eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

				if (FAILED(pTexture->UnlockRect(D3DCUBEMAP_FACES(iFace), 0)))
					EX_WARN(_T("UnlockRect failed"));
			}
		}
		else
		{
			// Generate mipmaps and fill all texture layers

			// Create the texture
			if (FAILED(g_pd3dDevice->CreateCubeTexture(iNewEdgeLength, 0, 0,
				d3dFormat, D3DPOOL_MANAGED, &pTexture, NULL)))
				EX_WARN(_T("CreateCubeTexture failed"));

			for (int iFace(0); iFace < 6; ++iFace)
			{
				CBitmap bmpCopy;
				CBitmap bmpWorking(*(pImage[iFace]));

				for (uint uiLevel(0); uiLevel < pTexture->GetLevelCount(); ++uiLevel)
				{
					// Fill the surface
					D3DLOCKED_RECT d3drect;
					if (FAILED(pTexture->LockRect(D3DCUBEMAP_FACES(iFace), uiLevel, &d3drect, NULL, 0)))
						EX_WARN(_T("LockRect failed"));

					if (d3drect.pBits)
						D3D_CopyBitmapToBuffer(bmpWorking, eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

					bmpWorking.Copy(bmpCopy);

					if (vid_textureGammaCorrect != 1.0f)
						bmpCopy.Scale(bmpWorking, MAX(bmpCopy.GetWidth() >> 1, 1), MAX(bmpCopy.GetHeight() >> 1, 1), vid_textureGammaCorrect);
					else
						bmpCopy.Scale(bmpWorking, MAX(bmpCopy.GetWidth() >> 1, 1), MAX(bmpCopy.GetHeight() >> 1, 1));

					if (FAILED(pTexture->UnlockRect(D3DCUBEMAP_FACES(iFace), uiLevel)))
						EX_WARN(_T("UnlockRect failed"));
				}
			}
		}

		D3DFORMAT eCompressedFormat(bitmaps[0].GetBMPType() == BITMAP_RGB ? D3DFMT_DXT1 : D3DFMT_DXT5);

		// Create the compressed version
		g_llSectionStart = K2System.GetTicks();
		if (vid_textureCompression && !(iTextureFlags & TEX_NO_COMPRESS) && pImage[0]->GetWidth() >= 4)
			pTexture = D3D_CompressCubeTexture(pTexture, eCompressedFormat, !(iTextureFlags & TEX_NO_MIPMAPS));
		g_llCompress = K2System.GetTicks() - g_llSectionStart;

		if (vid_texturePreload)
		{
			g_llSectionStart = K2System.GetTicks();
			pTexture->PreLoad();
			g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
		}
		else
			g_llPreLoad = 0;

		g_pTextures[iIndex] = g_pTexturesCube[iIndex] = pTexture;
		return iIndex;
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_RegisterCubeTexture(") + sName + _TS(") - "), NO_THROW);
		return -1;
	}
}


/*====================
  D3D_RegisterVolumeTexture
  ====================*/
int		D3D_RegisterVolumeTexture(const CBitmap bitmaps[], int iDepth, int iTextureFlags, ETextureFormat eFormat, const tstring &sName)
{
	const CBitmap **pImage = NULL;
	CBitmap *bmpResized(NULL);

	try
	{
		// Find empty texture slot
		int iIndex(MAX_TEXTURES);
		for (int i(0); i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] != NULL)
				continue;

			iIndex = i;
			break;
		}
		if (iIndex == MAX_TEXTURES)
			EX_WARN(_T("No free texture slots"));

		iTextureFlags |= TEX_NO_COMPRESS;

		// Resize, if necessary
		pImage = new const CBitmap *[iDepth];
		bmpResized = new CBitmap[iDepth];

		int iWidth(bitmaps[0].GetWidth());
		int iHeight(bitmaps[0].GetHeight());

		int iNewWidth(iWidth);
		int iNewHeight(iHeight);

		bool bPerf(false); // Generate a performance warning

		if (vid_textureDownsize > 0 && !(iTextureFlags & TEX_FULL_QUALITY))
		{
			iNewWidth = MAX(M_FloorPow2(iNewWidth) >> vid_textureDownsize, 2);
			iNewHeight = MAX(M_FloorPow2(iNewHeight) >> vid_textureDownsize, 2);
		}
		if ((iNewWidth > vid_textureMaxSize || iNewHeight > vid_textureMaxSize) && !(iTextureFlags & TEX_FULL_QUALITY))
		{
			iNewWidth = M_FloorPow2(MIN(iNewWidth, int(vid_textureMaxSize)));
			iNewHeight = M_FloorPow2(MIN(iNewHeight, int(vid_textureMaxSize)));
		}

		if (iNewWidth > int(g_DeviceCaps.uiMaxVolumeExtent))
			iNewWidth = M_FloorPow2(g_DeviceCaps.uiMaxVolumeExtent);
		if (iNewHeight > int(g_DeviceCaps.uiMaxVolumeExtent))
			iNewHeight = M_FloorPow2(g_DeviceCaps.uiMaxVolumeExtent);
		if (iDepth > int(g_DeviceCaps.uiMaxVolumeExtent))
			iDepth = M_FloorPow2(g_DeviceCaps.uiMaxVolumeExtent);

		iNewWidth = MIN(iNewWidth, 256);
		iNewHeight = MIN(iNewHeight, 256);
		iDepth = MIN(iDepth, 256);

		if (!M_IsPow2(iNewWidth) || !M_IsPow2(iNewHeight)) // Resize texture if its not a power of 2
		{
			iNewWidth = M_FloorPow2(iNewWidth);
			iNewHeight = M_FloorPow2(iNewHeight);

			bPerf = true;
		}

		for (int iLayer(0); iLayer < iDepth; ++iLayer)
		{
			if (bitmaps[iLayer].GetWidth() != iNewWidth || bitmaps[iLayer].GetHeight() != iNewHeight)
			{
				if (bPerf || bitmaps[iLayer].GetWidth() != iWidth || bitmaps[iLayer].GetHeight() != iHeight)
				{
					Console.Perf << "Resizing " << sName << " from "
						<< bitmaps[iLayer].GetWidth() << "x" << bitmaps[iLayer].GetHeight() << " to "
						<< iNewWidth << "x" << iNewHeight << newl;
				}
				else
				{
					Console.Dev << "Resizing " << sName << " from "
						<< bitmaps[iLayer].GetWidth() << "x" << bitmaps[iLayer].GetHeight() << " to "
						<< iNewWidth << "x" << iNewHeight << newl;
				}

				bitmaps[iLayer].Scale(bmpResized[iLayer], iNewWidth, iNewHeight);
				pImage[iLayer] = &bmpResized[iLayer];
			}
			else
			{
				pImage[iLayer] = &bitmaps[iLayer];
			}
		}

		IDirect3DVolumeTexture9	*pTexture(NULL);

		D3DFORMAT d3dFormat;

		switch (eFormat)
		{
		default:
		case TEXFMT_A8R8G8B8:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case TEXFMT_A1R5G5B5:
			d3dFormat = D3DFMT_A1R5G5B5;
			break;
		case TEXFMT_A4R4G4B4:
			d3dFormat = D3DFMT_A4R4G4B4;
			break;
		case TEXFMT_A8:
			d3dFormat = D3DFMT_A8;
			break;
		case TEXFMT_A8L8:
			d3dFormat = D3DFMT_A8L8;
			break;
		case TEXFMT_R16G16:
			d3dFormat = D3DFMT_G16R16;
			break;
		case TEXFMT_U8V8:
			d3dFormat = D3DFMT_V8U8;
			break;
		case TEXFMT_U16V16:
			d3dFormat = D3DFMT_V16U16;
			break;
		case TEXFMT_R16F:
			d3dFormat = D3DFMT_R16F;
			break;
		case TEXFMT_R16G16F:
			d3dFormat = D3DFMT_G16R16F;
			break;
		case TEXFMT_A16R16G16B16F:
			d3dFormat = D3DFMT_A16B16G16R16F;
			break;
		case TEXFMT_R32F:
			d3dFormat = D3DFMT_R32F;
			break;
		case TEXFMT_R32G32F:
			d3dFormat = D3DFMT_G32R32F;
			break;
		case TEXFMT_A32R32G32B32F:
			d3dFormat = D3DFMT_A32B32G32R32F;
			break;
		}

		if (/*vid_textureAutogenMipmaps || */(iTextureFlags & TEX_NO_MIPMAPS))
		{
			// Only fill the top texture layer

			// Create the texture
			if (FAILED(g_pd3dDevice->CreateVolumeTexture(iNewWidth, iNewHeight, iDepth,
				(iTextureFlags & TEX_NO_MIPMAPS) ? 1 : 0,
				(iTextureFlags & TEX_NO_MIPMAPS) ? 0 : D3DUSAGE_AUTOGENMIPMAP,
				d3dFormat, D3DPOOL_MANAGED, &pTexture, NULL)))
				EX_WARN(_T("CreateVolumeTexture failed"));

			D3DLOCKED_BOX d3dbox;
			pTexture->LockBox(0, &d3dbox, NULL, 0);

			if (d3dbox.pBits)
			{
				byte *pDstData(static_cast<byte *>(d3dbox.pBits));

				for (int iZ(0); iZ < iDepth; ++iZ, pDstData += d3dbox.SlicePitch)
					D3D_CopyBitmapToBuffer(*pImage[iZ], eFormat, pDstData, d3dbox.RowPitch);
			}

			pTexture->UnlockBox(0);
		}
		else
		{
			// Generate mipmaps and fill all texture layers

			// Create the texture
			if (FAILED(g_pd3dDevice->CreateVolumeTexture(iNewWidth, iNewHeight, iDepth,
				0, 0, d3dFormat, D3DPOOL_MANAGED, &pTexture, NULL)))
				EX_WARN(_T("CreateVolumeTexture failed"));

			int iWorkingWidth(iNewWidth);
			int iWorkingHeight(iNewHeight);
			int iWorkingDepth(iDepth);

			CBitmap *pWorkingBitmaps(new CBitmap[iWorkingDepth]);
			for (int i(0); i < iWorkingDepth; ++i)
				pImage[i]->Copy(pWorkingBitmaps[i]);

			for (uint uiLevel(0); uiLevel < pTexture->GetLevelCount(); ++uiLevel)
			{
				D3DLOCKED_BOX d3dbox;
				pTexture->LockBox(uiLevel, &d3dbox, NULL, 0);

				if (d3dbox.pBits)
				{
					byte *pDstData(static_cast<byte *>(d3dbox.pBits));

					for (int iZ(0); iZ < iWorkingDepth; ++iZ, pDstData += d3dbox.SlicePitch)
						D3D_CopyBitmapToBuffer(pWorkingBitmaps[iZ], eFormat, pDstData, d3dbox.RowPitch);
				}

				pTexture->UnlockBox(uiLevel);

				// Resize working volume texture
				CBitmap *pCopyBitmaps(new CBitmap[iWorkingDepth]);

				iWorkingWidth = MAX(iWorkingWidth >> 1, 1);
				iWorkingHeight = MAX(iWorkingHeight >> 1, 1);

				for (int i(0); i < iWorkingDepth; ++i)
				{
					if (vid_textureGammaCorrect != 1.0f)
						pWorkingBitmaps[i].Scale(pCopyBitmaps[i], iWorkingWidth, iWorkingHeight, vid_textureGammaCorrect);
					else
						pWorkingBitmaps[i].Scale(pCopyBitmaps[i], iWorkingWidth, iWorkingHeight);
				}

				int iNewWorkingDepth = MAX(iWorkingDepth >> 1, 1);

				if (iNewWorkingDepth != iWorkingDepth)
				{
					if (pWorkingBitmaps)
						delete[] pWorkingBitmaps;

					pWorkingBitmaps = new CBitmap[iNewWorkingDepth];

					for (int i(0); i < iNewWorkingDepth; ++i)
						pWorkingBitmaps[i].Lerp(0.5f, pCopyBitmaps[i * 2], pCopyBitmaps[i * 2 + 1]);

					iWorkingDepth = iNewWorkingDepth;
				}
				else
				{
					for (int i(0); i < iWorkingDepth; ++i)
						pCopyBitmaps[i].Copy(pWorkingBitmaps[i]);
				}

				delete[] pCopyBitmaps;
			}
		}

		D3DFORMAT eCompressedFormat(bitmaps[0].GetBMPType() == BITMAP_RGB ? D3DFMT_DXT1 : D3DFMT_DXT5);

		// Create the compressed version
		g_llSectionStart = K2System.GetTicks();
		if (vid_textureCompression && !(iTextureFlags & TEX_NO_COMPRESS) && pImage[0]->GetWidth() >= 4 && pImage[0]->GetHeight() >= 4)
			pTexture = D3D_CompressVolumeTexture(pTexture, eCompressedFormat, !(iTextureFlags & TEX_NO_MIPMAPS));
		g_llCompress = K2System.GetTicks() - g_llSectionStart;

		delete[] pImage;
		delete[] bmpResized;

		if (vid_texturePreload)
		{
			g_llSectionStart = K2System.GetTicks();
			pTexture->PreLoad();
			g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
		}
		else
			g_llPreLoad = 0;

		g_pTextures[iIndex] = g_pTexturesVolume[iIndex] = pTexture;
		return iIndex;
	}
	catch (CException &ex)
	{
		if (pImage) delete[] pImage;
		if (bmpResized) delete[] bmpResized;

		ex.Process(_TS("D3D_RegisterVolumeMap(") + sName + _TS(") - "), NO_THROW);
		return -1;
	}
}


/*====================
  D3D_RegisterRenderTargetTexture
  ====================*/
int		D3D_RegisterRenderTargetTexture(const tstring &sName, int iSizeX, int iSizeY, D3DFORMAT eFormat, bool bMipmap)
{
	// See if this texture is already in memory
	TextureMap::iterator findit = g_mapTextures.find(sName);

	if (findit != g_mapTextures.end())
	{
		// Yep, so just return the instance that's already in memory
		return findit->second.iIndex;
	}
	else
	{
		int i;

		// Find empty texture slot
		for (i = 0; i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] == NULL)
				break;
		}

		if (i == MAX_TEXTURES)
			return -1;

		DWORD dwUsage(D3DUSAGE_RENDERTARGET);
		if (bMipmap)
			dwUsage |= D3DUSAGE_AUTOGENMIPMAP;

		if (FAILED(g_pd3dDevice->CreateTexture(iSizeX, iSizeY, 1, dwUsage,
			eFormat, D3DPOOL_DEFAULT, &g_pTextures2D[i], NULL)))
		{
			Console.Dev << _T("D3D_RegisterRenderTargetTexture: CreateTexture failed on ") << sName << _T(" ") << iSizeX << _T("x") << iSizeY << newl;
			return -1;
		}

		g_pTextures[i] = g_pTextures2D[i]; // Store base pointer in the shared texture array

		g_mapTextures[sName] = STextureMapEntry(i, -1, INVALID_RESOURCE);

		return i;
	}
}


/*====================
  D3D_RegisterBlankTexture
  ====================*/
int		D3D_RegisterBlankTexture(const tstring &sName, int iSizeX, int iSizeY, D3DFORMAT eFormat, bool bMipmap)
{
	// See if this texture is already in memory
	TextureMap::iterator findit = g_mapTextures.find(sName);

	if (findit != g_mapTextures.end())
	{
		// Yep, so just return the instance that's already in memory
		return findit->second.iIndex;
	}
	else
	{
		int i;

		// Find empty texture slot
		for (i = 0; i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] == NULL)
				break;
		}

		if (i == MAX_TEXTURES)
			return -1;

		DWORD dwUsage(0);
		if (bMipmap)
			dwUsage |= D3DUSAGE_AUTOGENMIPMAP;

		if (FAILED(g_pd3dDevice->CreateTexture(iSizeX, iSizeY, 1, dwUsage,
			eFormat, D3DPOOL_DEFAULT, &g_pTextures2D[i], NULL)))
		{
			Console.Dev << _T("D3D_RegisterRenderTargetTexture: CreateTexture failed on ") << sName << _T(" ") << iSizeX << _T("x") << iSizeY << newl;
			return -1;
		}

		g_pTextures[i] = g_pTextures2D[i]; // Store base pointer in the shared texture array

		g_mapTextures[sName] = STextureMapEntry(i, -1, INVALID_RESOURCE);

		return i;
	}
}


/*====================
  D3D_RegisterDepthTexture
  ====================*/
int		D3D_RegisterDepthTexture(const tstring &sName, int iSizeX, int iSizeY, D3DFORMAT eFormat)
{
	// See if this texture is already in memory
	TextureMap::iterator findit = g_mapTextures.find(sName);

	if (findit != g_mapTextures.end())
	{
		// Yep, so just return the instance that's already in memory
		return findit->second.iIndex;
	}
	else
	{
		int i;

		// Find empty texture slot
		for (i = 0; i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] == NULL)
				break;
		}

		if (i == MAX_TEXTURES)
			return -1;

		if (FAILED(g_pd3dDevice->CreateTexture(iSizeX, iSizeY, 1, D3DUSAGE_DEPTHSTENCIL,
			eFormat, D3DPOOL_DEFAULT, &g_pTextures2D[i], NULL)))
		{
			Console.Dev << _T("D3D_RegisterDepthTexture: CreateTexture failed") << newl;
			return -1;
		}

		g_pTextures[i] = g_pTextures2D[i]; // Store base pointer in the shared texture array

		g_mapTextures[sName] = STextureMapEntry(i, -1, INVALID_RESOURCE);

		return i;
	}
}


/*====================
  D3D_RegisterDynamicTexture
  ====================*/
int		D3D_RegisterDynamicTexture(const tstring &sName, const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat)
{
	// See if this texture is already in memory
	TextureMap::iterator findit = g_mapTextures.find(sName);

	if (findit != g_mapTextures.end())
	{
		// Yep, so just return the instance that's already in memory
		return findit->second.iIndex;
	}

	try
	{
		// Find empty texture slot
		int iIndex(MAX_TEXTURES);
		for (int i(0); i < MAX_TEXTURES; ++i)
		{
			if (g_pTextures[i] != NULL)
				continue;

			iIndex = i;
			break;
		}
		if (iIndex == MAX_TEXTURES)
			EX_WARN(_T("No free texture slots"));

		g_llIndexLookup = 0;
		g_llSectionStart = K2System.GetTicks();
		g_llFileInfo = K2System.GetTicks() - g_llSectionStart;

		// Resize, if necessary
		g_llSectionStart = K2System.GetTicks();
		const CBitmap *pImage(NULL);
		CBitmap bmpResized;

		int iNewWidth(bitmap.GetWidth());
		int iNewHeight(bitmap.GetHeight());

		bool bPerf(false); // Generate a performance warning

		if (vid_textureDownsize > 0 && !(iTextureFlags & TEX_FULL_QUALITY))
		{
			iNewWidth = MAX(M_FloorPow2(iNewWidth) >> vid_textureDownsize, 2);
			iNewHeight = MAX(M_FloorPow2(iNewHeight)  >> vid_textureDownsize, 2);
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
				Console.Perf << "Resizing " << sName << " from "
					<< bitmap.GetWidth() << "x" << bitmap.GetHeight() << " to "
					<< iNewWidth << "x" << iNewHeight << newl;
			}
			else
			{
				Console.Dev << "Resizing " << sName << " from "
					<< bitmap.GetWidth() << "x" << bitmap.GetHeight() << " to "
					<< iNewWidth << "x" << iNewHeight << newl;
			}

			bitmap.Scale(bmpResized, iNewWidth, iNewHeight);
			pImage = &bmpResized;
		}
		else
		{
			pImage = &bitmap;
		}
		g_llResize = K2System.GetTicks() - g_llSectionStart;

		IDirect3DTexture9	*pTexture(NULL);
		D3DFORMAT d3dFormat;
		switch (eFormat)
		{
		default:
		case TEXFMT_A8R8G8B8:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case TEXFMT_A1R5G5B5:
			d3dFormat = D3DFMT_A1R5G5B5;
			break;
		case TEXFMT_A4R4G4B4:
			d3dFormat = D3DFMT_A4R4G4B4;
			break;
		case TEXFMT_A8:
			d3dFormat = D3DFMT_A8;
			break;
		case TEXFMT_A8L8:
			d3dFormat = D3DFMT_A8L8;
			break;
		case TEXFMT_R16G16:
			d3dFormat = D3DFMT_G16R16;
			break;
		case TEXFMT_U8V8:
			d3dFormat = D3DFMT_V8U8;
			break;
		case TEXFMT_U16V16:
			d3dFormat = D3DFMT_V16U16;
			break;
		case TEXFMT_R16F:
			d3dFormat = D3DFMT_R16F;
			break;
		case TEXFMT_R16G16F:
			d3dFormat = D3DFMT_G16R16F;
			break;
		case TEXFMT_A16R16G16B16F:
			d3dFormat = D3DFMT_A16B16G16R16F;
			break;
		case TEXFMT_R32F:
			d3dFormat = D3DFMT_R32F;
			break;
		case TEXFMT_R32G32F:
			d3dFormat = D3DFMT_G32R32F;
			break;
		case TEXFMT_A32R32G32B32F:
			d3dFormat = D3DFMT_A32B32G32R32F;
			break;
		}

		g_llSectionStart = K2System.GetTicks();
		
		// Only fill the top texture layer

		// Create the texture
		if (FAILED(g_pd3dDevice->CreateTexture(pImage->GetWidth(), pImage->GetHeight(),
			(iTextureFlags & TEX_NO_MIPMAPS) ? 1 : 0,
			((iTextureFlags & TEX_NO_MIPMAPS) ? 0 : D3DUSAGE_AUTOGENMIPMAP) | D3DUSAGE_DYNAMIC,
			d3dFormat, D3DPOOL_DEFAULT, &pTexture, NULL)))
			throw CException(_T("CreateTexture failed"), E_WARNING);

		// Fill the surface
		D3DLOCKED_RECT d3drect;
		if (FAILED(pTexture->LockRect(0, &d3drect, NULL, D3DLOCK_DISCARD)))
			throw CException(_T("LockRect failed"), E_WARNING);

		if (d3drect.pBits)
			D3D_CopyBitmapToBuffer(*pImage, eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

		if (FAILED(pTexture->UnlockRect(0)))
			throw CException(_T("UnlockRect failed"), E_WARNING);

		g_llMipMaps = K2System.GetTicks() - g_llSectionStart;
		g_llPreLoad = 0;

		g_mapTextures[sName] = STextureMapEntry(iIndex, -1, INVALID_RESOURCE);

		g_pTextures[iIndex] = g_pTextures2D[iIndex] = pTexture;
		return iIndex;
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_RegisterDynamicTexture(") + sName + _TS(") - "), NO_THROW);
		return -1;
	}
}


/*====================
  D3D_RegisterTexture
  ====================*/
int		D3D_RegisterTexture(CTexture *pTexture)
{
	int	iTexture(-1);

	// See if this texture is already in memory
	TextureMap::iterator itFind(g_mapTextures.find(pTexture->GetPath()));

	if (itFind != g_mapTextures.end())
	{
		// Yep, so just use the instance that's already in memory
		pTexture->SetIndex(itFind->second.iIndex);
		pTexture->SetIndex2(itFind->second.iIndex2);
		return 1;
	}

	// Nope, then load it from disk and put it into vid memory
	switch (pTexture->GetType())
	{
	default:
	case TEXTURE_2D:
		if (pTexture->HasBitmap())
		{
			iTexture = D3D_Register2DTexture(*pTexture->GetBitmap(), pTexture->GetTextureFlags(), pTexture->GetFormat(), pTexture->GetPath());
		}
		else
		{
			float fTimeScale(1000.0f / K2System.GetFrequency());

			if (vid_textureProfile) Console << _T("Start RegisterTexture: ") << pTexture->GetPath() << newl;
			g_llStartTime = g_llSectionStart = K2System.GetTicks();

			if (pTexture->GetFormat() == TEXFMT_NORMALMAP && vid_shaderRXGBNormalmap)
			{
				if ((iTexture = D3D_RegisterArchivedTexture(pTexture, _T("_rxgb"))) != -1)
				{
				}
				else if (vid_textureCache && (iTexture = D3D_RegisterCachedTexture(pTexture, pTexture->GetPath(), _T("_rxgb"))) != -1)
				{
				}
				else
				{
					CBitmap bitmap;
					if (!bitmap.Load(pTexture->GetPath()))
					{
						++g_uiMissingTextures;
						EX_WARN(_T("not found or bad texture"));
					}

					iTexture = D3D_Register2DTexture(bitmap, pTexture->GetTextureFlags(), TEXFMT_NORMALMAP_RXGB, pTexture->GetPath());

					if (vid_textureCache)
						g_TextureCache.CacheTexture(Filename_AppendSuffix(pTexture->GetPath(), _T("_rxgb")), g_pTextures[iTexture], pTexture->GetPath());
				}

				int iTexture2(-1);

				if ((iTexture2 = D3D_RegisterArchivedTexture(pTexture, _T("_s"))) != -1)
				{
				}
				else if (vid_textureCache && (iTexture2 = D3D_RegisterCachedTexture(pTexture, pTexture->GetPath(), _T("_s"))) != -1)
				{
				}
				else
				{
					CBitmap bitmap;
					if (bitmap.Load(pTexture->GetPath()) && bitmap.GetBMPType() == BITMAP_RGBA)
					{
						iTexture2 = D3D_Register2DTexture(bitmap, pTexture->GetTextureFlags(), TEXFMT_NORMALMAP_S, pTexture->GetPath());

						if (vid_textureCache)
							g_TextureCache.CacheTexture(Filename_AppendSuffix(pTexture->GetPath(), _T("_s")), g_pTextures[iTexture2], pTexture->GetPath());
					}
					else
					{
						iTexture2 = g_iBlack;
					}
				}

				pTexture->SetIndex2(iTexture2);
			}
			else
			{
				if ((iTexture = D3D_RegisterArchivedTexture(pTexture)) != -1)
				{
					// Check the texture archive
					g_llTotalTime = K2System.GetTicks() - g_llStartTime;
					g_llTotalTextureLoadTime += g_llTotalTime;
					g_llTotalTextures++;

					if (g_llTotalTime > g_llHighTextureTime)
						g_llHighTextureTime = g_llTotalTime;

					if (vid_textureProfile)
					{
						Console << _T("Texture archive hit!") << newl;
						Console << _T("  Find index: ") << g_llIndexLookup * fTimeScale << newl
								<< _T("  File read: ") << g_llFileRead * fTimeScale << newl
								<< _T("  Register: ") << g_llRegister * fTimeScale << newl
								<< _T("  Preload: ") << g_llPreLoad * fTimeScale << newl
								<< _T("End RegsiterTexture: ") << pTexture->GetPath() << _T(" in ") << g_llTotalTime * fTimeScale << _T("ms") << newl;
					}
				}
				else if (vid_textureCache && (iTexture = D3D_RegisterCachedTexture(pTexture)) != -1)
				{
					// Check the texture cache
					g_llTotalTime = K2System.GetTicks() - g_llStartTime;
					g_llTotalTextureLoadTime += g_llTotalTime;
					g_llTotalTextures++;

					if (g_llTotalTime > g_llHighTextureTime)
						g_llHighTextureTime = g_llTotalTime;

					if (vid_textureProfile)
					{
						Console << _T("Texture cache hit!") << newl;
						Console << _T("  Find index: ") << g_llIndexLookup * fTimeScale << newl
								<< _T("  File read: ") << g_llFileRead * fTimeScale << newl
								<< _T("  Register: ") << g_llRegister * fTimeScale << newl
								<< _T("  Preload: ") << g_llPreLoad * fTimeScale << newl
								<< _T("End RegsiterTexture: ") << pTexture->GetPath() << _T(" in ") << g_llTotalTime * fTimeScale << _T("ms") << newl;
					}
				}
				else
				{
					CBitmap bitmap;
					if (!bitmap.Load(pTexture->GetPath()))
					{
						++g_uiMissingTextures;
						EX_WARN(_T("not found or bad texture"));
					}
					++g_uiCacheMisses;
					g_vCacheMisses.push_back(pTexture->GetPath());
					g_llFileRead = K2System.GetTicks() - g_llSectionStart;
					iTexture = D3D_Register2DTexture(bitmap, pTexture->GetTextureFlags(), pTexture->GetFormat(), pTexture->GetPath());
					g_llTotalTime = K2System.GetTicks() - g_llStartTime;
					g_llTotalTextureLoadTime += g_llTotalTime;
					g_llTotalTextures++;

					if (g_llTotalTime > g_llHighTextureTime)
						g_llHighTextureTime = g_llTotalTime;

					if (vid_textureCache)
						g_TextureCache.CacheTexture(pTexture->GetPath(), g_pTextures[iTexture]);

					if (vid_textureProfile)
					{
						Console << _T("  File read: ") << g_llFileRead * fTimeScale << newl
								<< _T("  Find info: ") << g_llFileInfo * fTimeScale << newl
								<< _T("  Find index: ") << g_llIndexLookup * fTimeScale << newl
								<< _T("  Resize: ") << g_llResize * fTimeScale << newl
								<< _T("  Mip Maps: ") << g_llMipMaps * fTimeScale << newl
								<< _T("  Compress: ") << g_llCompress * fTimeScale << newl
								<< _T("  Preload: ") << g_llPreLoad * fTimeScale << newl
								<< _T("End RegsiterTexture: ") << pTexture->GetPath() << _T(" in ") << g_llTotalTime * fTimeScale << _T("ms") << newl;
					}
				}
			}
		}
		break;

	case TEXTURE_CUBE:
		{
			// Check the cache
			if ((iTexture = D3D_RegisterArchivedTexture(pTexture)) != -1)
			{
				// TODO: Profiling information
			}
			else if (vid_textureCache && (iTexture = D3D_RegisterCachedTexture(pTexture, Filename_AppendSuffix(pTexture->GetPath(), _T("_posx")), TSNULL)) != -1)
			{
				// TODO: Profiling information
			}
			else
			{
				++g_uiCacheMisses;
				g_vCacheMisses.push_back(pTexture->GetPath());

				CBitmap bitmaps[6];

				if (!bitmaps[0].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_posx"))) ||
					!bitmaps[1].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_negx"))) ||
					!bitmaps[2].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_posy"))) ||
					!bitmaps[3].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_negy"))) ||
					!bitmaps[4].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_posz"))) ||
					!bitmaps[5].Load(Filename_AppendSuffix(pTexture->GetPath(), _T("_negz"))))
				{
					++g_uiMissingTextures;
					EX_WARN(_T("not found or bad cubemap"));
				}

				iTexture = D3D_RegisterCubeTexture(bitmaps, pTexture->GetTextureFlags(), pTexture->GetFormat(), pTexture->GetPath());

				if (vid_textureCache)
					g_TextureCache.CacheTexture(pTexture->GetPath(), g_pTextures[iTexture], Filename_AppendSuffix(pTexture->GetPath(), _T("_posx")));
			}
		}
		break;

	case TEXTURE_VOLUME:
		{
			if ((iTexture = D3D_RegisterArchivedTexture(pTexture)) != -1)
			{
				// TODO: Profiling information
			}
			else if (vid_textureCache && (iTexture = D3D_RegisterCachedTexture(pTexture, Filename_AppendSuffix(pTexture->GetPath(), XtoA(0, FMT_PADZERO, 4)), TSNULL)) != -1)
			{
				// TODO: Profiling information
			}
			else
			{
				++g_uiCacheMisses;
				g_vCacheMisses.push_back(pTexture->GetPath());

				int iDepth(0);

				for (int iZ(0); iZ < vid_textureMaxSize; ++iZ)
				{
					if (!FileManager.Exists(Filename_AppendSuffix(pTexture->GetPath(), XtoA(iZ, FMT_PADZERO, 4))))
						break;

					++iDepth;
				}

				iDepth = M_FloorPow2(iDepth + 1);

				CBitmap *bitmaps(new CBitmap[iDepth]);

				// Load bitmaps
				for (int iZ(0); iZ < iDepth; ++iZ)
				{
					if (!bitmaps[iZ].Load(Filename_AppendSuffix(pTexture->GetPath(), XtoA(iZ, FMT_PADZERO, 4))))
					{
						++g_uiMissingTextures;
						delete[] bitmaps;
						EX_WARN(_T("not found or bad volume texture"));
					}
				}

				iTexture = D3D_RegisterVolumeTexture(bitmaps, iDepth, pTexture->GetTextureFlags(), pTexture->GetFormat(), pTexture->GetPath());
				delete[] bitmaps;

				if (vid_textureCache)
					g_TextureCache.CacheTexture(pTexture->GetPath(), g_pTextures[iTexture], Filename_AppendSuffix(pTexture->GetPath(), XtoA(0, FMT_PADZERO, 4)));
			}
		}
		break;
	}

	pTexture->SetIndex(iTexture);
	
	g_mapTextures[pTexture->GetPath()] = STextureMapEntry(pTexture->GetIndex(), pTexture->GetIndex2(), pTexture->GetHandle());

	return 0;
}


/*====================
  D3D_UnregisterTexture
  ====================*/
void	D3D_UnregisterTexture(CTexture *pTexture)
{
	if (g_mapTextures.empty())
		return;

	// See if this texture is in memory
	TextureMap::iterator itFind(g_mapTextures.find(pTexture->GetPath()));

	if (itFind != g_mapTextures.end())
	{
		int iIndex(pTexture->GetIndex());

		if (iIndex != -1)
		{
			switch (pTexture->GetType())
			{
			case TEXTURE_2D:
				SAFE_RELEASE(g_pTextures2D[iIndex]);
				break;
			case TEXTURE_CUBE:
				SAFE_RELEASE(g_pTexturesCube[iIndex]);
				break;
			case TEXTURE_VOLUME:
				SAFE_RELEASE(g_pTexturesVolume[iIndex]);
				break;
			}

			g_pTextures[iIndex] = NULL;
		}

		int iIndex2(pTexture->GetIndex2());

		if (iIndex2 != -1 && iIndex2 != g_iWhite && iIndex2 != g_iBlack)
		{
			switch (pTexture->GetType())
			{
			case TEXTURE_2D:
				SAFE_RELEASE(g_pTextures2D[iIndex2]);
				break;
			case TEXTURE_CUBE:
				SAFE_RELEASE(g_pTexturesCube[iIndex2]);
				break;
			case TEXTURE_VOLUME:
				SAFE_RELEASE(g_pTexturesVolume[iIndex2]);
				break;
			}

			g_pTextures[iIndex2] = NULL;
		}

		g_mapTextures.erase(itFind);
	}

	pTexture->SetIndex(-1);
	pTexture->SetIndex2(-1);
}



/*====================
  D3D_Unregister2DTexture
  ====================*/
void	D3D_Unregister2DTexture(const tstring &sName)
{
	// See if this render target is in memory
	TextureMap::iterator itFind(g_mapTextures.find(sName));

	if (itFind != g_mapTextures.end())
	{
		int iIndex(itFind->second.iIndex);

		SAFE_RELEASE(g_pTextures2D[iIndex]);
		g_pTextures[iIndex] = NULL;

		g_mapTextures.erase(itFind);
	}
}


/*====================
  D3D_Update2DTexture
  ====================*/
void	D3D_Update2DTexture(int iIndex, const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat)
{
	try
	{
		// Resize, if necessary
		const CBitmap *pImage(NULL);
		CBitmap bmpResized;

		IDirect3DTexture9	*pTexture(g_pTextures2D[iIndex]);

		D3DSURFACE_DESC d3dDesc;
		pTexture->GetLevelDesc(0, &d3dDesc);

		int iNewWidth(d3dDesc.Width);
		int iNewHeight(d3dDesc.Height);

		if (bitmap.GetWidth() != iNewWidth || bitmap.GetHeight() != iNewHeight)
		{
			Console.Perf << "Resizing " << XtoA(iIndex) << " from "
				<< bitmap.GetWidth() << "x" << bitmap.GetHeight() << " to "
				<< iNewWidth << "x" << iNewHeight << newl;

			bitmap.Scale(bmpResized, iNewWidth, iNewHeight);
			pImage = &bmpResized;
		}
		else
		{
			pImage = &bitmap;
		}

		D3DFORMAT d3dFormat;

		switch (eFormat)
		{
		default:
		case TEXFMT_A8R8G8B8:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case TEXFMT_A1R5G5B5:
			d3dFormat = D3DFMT_A1R5G5B5;
			break;
		case TEXFMT_A4R4G4B4:
			d3dFormat = D3DFMT_A4R4G4B4;
			break;
		case TEXFMT_A8:
			d3dFormat = D3DFMT_A8;
			break;
		case TEXFMT_A8L8:
			d3dFormat = D3DFMT_A8L8;
			break;
		case TEXFMT_R16G16:
			d3dFormat = D3DFMT_G16R16;
			break;
		case TEXFMT_U8V8:
			d3dFormat = D3DFMT_V8U8;
			break;
		case TEXFMT_U16V16:
			d3dFormat = D3DFMT_V16U16;
			break;
		case TEXFMT_R16F:
			d3dFormat = D3DFMT_R16F;
			break;
		case TEXFMT_R16G16F:
			d3dFormat = D3DFMT_G16R16F;
			break;
		case TEXFMT_A16R16G16B16F:
			d3dFormat = D3DFMT_A16B16G16R16F;
			break;
		case TEXFMT_R32F:
			d3dFormat = D3DFMT_R32F;
			break;
		case TEXFMT_R32G32F:
			d3dFormat = D3DFMT_G32R32F;
			break;
		case TEXFMT_A32R32G32B32F:
			d3dFormat = D3DFMT_A32B32G32R32F;
			break;
		}

		if (vid_textureAutogenMipmaps || (iTextureFlags & TEX_NO_MIPMAPS))
		{
			// Only fill the top texture layer

			// Fill the surface
			D3DLOCKED_RECT d3drect;
			if (FAILED(pTexture->LockRect(0, &d3drect, NULL, 0)))
				EX_WARN(_T("LockRect failed"));

			if (d3drect.pBits)
				D3D_CopyBitmapToBuffer(*pImage, eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

			if (FAILED(pTexture->UnlockRect(0)))
				EX_WARN(_T("UnlockRect failed"));
		}
		else
		{
			// Generate mipmaps and fill all texture layers

			CBitmap bmpCopy;
			CBitmap bmpWorking(*pImage);

			for (uint uiLevel(0); uiLevel < pTexture->GetLevelCount(); ++uiLevel)
			{
				// Fill the surface
				D3DLOCKED_RECT d3drect;
				if (FAILED(pTexture->LockRect(uiLevel, &d3drect, NULL, 0)))
					EX_WARN(_T("LockRect failed"));

				if (d3drect.pBits)
					D3D_CopyBitmapToBuffer(bmpWorking, eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

				bmpWorking.Copy(bmpCopy);
				bmpCopy.Scale(bmpWorking, MAX(bmpCopy.GetWidth() >> 1, 1), MAX(bmpCopy.GetHeight() >> 1, 1));

				if (FAILED(pTexture->UnlockRect(uiLevel)))
					EX_WARN(_T("UnlockRect failed"));
			}
		}

		D3DFORMAT eCompressedFormat(bitmap.GetBMPType() == BITMAP_RGB ? D3DFMT_DXT1 : D3DFMT_DXT5);

		// Create the compressed version
		if (vid_textureCompression && !(iTextureFlags & TEX_NO_COMPRESS) && pImage->GetWidth() >= 4 && pImage->GetHeight() >= 4)
			pTexture = D3D_CompressTexture(pTexture, eCompressedFormat, !(iTextureFlags & TEX_NO_MIPMAPS));

		if (vid_texturePreload)
		{
			g_llSectionStart = K2System.GetTicks();
			pTexture->PreLoad();
			g_llPreLoad = K2System.GetTicks() - g_llSectionStart;
		}
		else
			g_llPreLoad = 0;
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_Update2DTexture(") + XtoA(iIndex) + _TS(") - "), NO_THROW);
	}
}


/*====================
  D3D_UpdateDynamicTexture
  ====================*/
void	D3D_UpdateDynamicTexture(int iIndex, const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat)
{
	if (g_bDeviceLost)
		return;

	try
	{
		// Resize, if necessary
		const CBitmap *pImage(NULL);
		CBitmap bmpResized;

		IDirect3DTexture9	*pTexture(g_pTextures2D[iIndex]);

		D3DSURFACE_DESC d3dDesc;
		pTexture->GetLevelDesc(0, &d3dDesc);

		int iNewWidth(d3dDesc.Width);
		int iNewHeight(d3dDesc.Height);

		if (bitmap.GetWidth() != iNewWidth || bitmap.GetHeight() != iNewHeight)
		{
			Console.Perf << "Resizing " << XtoA(iIndex) << " from "
				<< bitmap.GetWidth() << "x" << bitmap.GetHeight() << " to "
				<< iNewWidth << "x" << iNewHeight << newl;

			bitmap.Scale(bmpResized, iNewWidth, iNewHeight);
			pImage = &bmpResized;
		}
		else
		{
			pImage = &bitmap;
		}

		D3DFORMAT d3dFormat;

		switch (eFormat)
		{
		default:
		case TEXFMT_A8R8G8B8:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case TEXFMT_A1R5G5B5:
			d3dFormat = D3DFMT_A1R5G5B5;
			break;
		case TEXFMT_A4R4G4B4:
			d3dFormat = D3DFMT_A4R4G4B4;
			break;
		case TEXFMT_A8:
			d3dFormat = D3DFMT_A8;
			break;
		case TEXFMT_A8L8:
			d3dFormat = D3DFMT_A8L8;
			break;
		case TEXFMT_R16G16:
			d3dFormat = D3DFMT_G16R16;
			break;
		case TEXFMT_U8V8:
			d3dFormat = D3DFMT_V8U8;
			break;
		case TEXFMT_U16V16:
			d3dFormat = D3DFMT_V16U16;
			break;
		case TEXFMT_R16F:
			d3dFormat = D3DFMT_R16F;
			break;
		case TEXFMT_R16G16F:
			d3dFormat = D3DFMT_G16R16F;
			break;
		case TEXFMT_A16R16G16B16F:
			d3dFormat = D3DFMT_A16B16G16R16F;
			break;
		case TEXFMT_R32F:
			d3dFormat = D3DFMT_R32F;
			break;
		case TEXFMT_R32G32F:
			d3dFormat = D3DFMT_G32R32F;
			break;
		case TEXFMT_A32R32G32B32F:
			d3dFormat = D3DFMT_A32B32G32R32F;
			break;
		}

		// Only fill the top texture layer

		// Fill the surface
		D3DLOCKED_RECT d3drect;
		if (FAILED(pTexture->LockRect(0, &d3drect, NULL, D3DLOCK_DISCARD)))
			throw CException(_T("LockRect failed"), E_WARNING);

		if (d3drect.pBits)
			D3D_CopyBitmapToBuffer(*pImage, eFormat, static_cast<byte *>(d3drect.pBits), d3drect.Pitch);

		if (FAILED(pTexture->UnlockRect(0)))
			throw CException(_T("UnlockRect failed"), E_WARNING);

		g_llPreLoad = 0;
	}
	catch (CException &ex)
	{
		ex.Process(_TS("D3D_UpdateDynamicTexture(") + XtoA(iIndex) + _TS(") - "), NO_THROW);
	}
}


/*====================
  D3D_GetTextureColor
  ====================*/
CVec4f	D3D_GetTextureColor(CTexture *pResource)
{
	if (pResource->GetIndex() == -1)
		return WHITE;

	IDirect3DTexture9	*pTexture(g_pTextures2D[pResource->GetIndex()]);
	
	IDirect3DSurface9	*pDestSurface(NULL);
	IDirect3DSurface9	*pSrcSurface(NULL);

	try
	{
		uint uiNumLevels(pTexture->GetLevelCount());
		uint uiReadLevel(uiNumLevels - 1);

		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(uiReadLevel, &desc);

		if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pDestSurface, NULL)))
			EX_WARN(_T("CreateOffscreenPlainSurface failed"));

		// Copy the surface of the original texture
		if (FAILED(pTexture->GetSurfaceLevel(uiReadLevel, &pSrcSurface)))
			EX_WARN(_T("Couldn't retrieve source surface"));

		if (FAILED(D3DXLoadSurfaceFromSurface(pDestSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_DEFAULT, 0)))
			EX_WARN(_T("D3DXLoadSurfaceFromSurface failed"));

		D3DLOCKED_RECT d3drect;
		if (FAILED(pDestSurface->LockRect(&d3drect, NULL, 0)))
			EX_WARN(_T("LockRect failed"));

		CVec4f v4Color;

		CVec4ll v4Tmp(0, 0, 0, 0);

		byte *pData(static_cast<byte *>(d3drect.pBits));
		int iExtra(d3drect.Pitch - desc.Width * sizeof(DWORD));

		for (uint y(0); y < desc.Height; ++y, pData += iExtra)
		{
			for (uint x(0); x < desc.Width; ++x, pData += 4)
			{
				v4Tmp[0] += pData[2];
				v4Tmp[1] += pData[1];
				v4Tmp[2] += pData[0];
				v4Tmp[3] += pData[3];
			}
		}

		for (int iComponent = 0; iComponent < 4; ++iComponent)
			v4Color[iComponent] = v4Tmp[iComponent] / (desc.Width * desc.Width) / 255.0f;

		return v4Color;
	}
	catch (CException &ex)
	{
		SAFE_RELEASE(pDestSurface);
		SAFE_RELEASE(pSrcSurface);
		ex.Process(_T("D3D_GetTextureColor() - "), NO_THROW);
		return WHITE;
	}

	SAFE_RELEASE(pDestSurface)
	SAFE_RELEASE(pSrcSurface);

	return WHITE;
}


/*====================
  D3D_TextureDefine
  ====================*/
void	D3D_TextureDefine(const string &sName, const string &sDefinition)
{
	s_mapTextureDefinitions[sName] = sDefinition;
}


/*====================
  D3D_TextureUndefine
  ====================*/
void	D3D_TextureUndefine(const string &sName)
{
	s_mapTextureDefinitions.erase(sName);
}


/*====================
  D3D_GetTextureDefinitionString
  ====================*/
string	D3D_GetTextureDefinitionString()
{
	string s;

	for (DefinitionMap::iterator it(s_mapTextureDefinitions.begin()); it != s_mapTextureDefinitions.end(); ++it)
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
  D3D_TextureExists
  ====================*/
bool	D3D_TextureExists(const tstring &sFilename, uint uiTextureFlags)
{
	if (D3D_ArchivedTextureExists(sFilename, uiTextureFlags))
		return true;
	else
		return FileManager.Exists(sFilename);
}


/*====================
  D3D_InitTexture
  ====================*/
void	D3D_InitTexture()
{
	PROFILE("D3D_InitTexture");

	//
	// Procedurals
	//

	D3D_OpenTextureArchive();

	CProceduralRegistry::GetInstance()->RegisterProcedurals();
	g_iWhite = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("white"));
	g_iBlack = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("black"));
	g_iInvis = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("invis"));

	g_hSkinReference = g_ResourceManager.Register(_T("!skin"), RES_REFERENCE);
	g_hSkyReference = g_ResourceManager.Register(_T("!sky"), RES_REFERENCE);
}


CMD(TextureLoadTime)
{
	float fTimeScale(1000.0f / K2System.GetFrequency());
	Console << _T("Total Time: ") << g_llTotalTextureLoadTime * fTimeScale << newl;
	Console << _T("Average Time: ") << (g_llTotalTextureLoadTime * fTimeScale) / g_llTotalTextures << newl;
	Console << _T("High Time: ") << g_llHighTextureTime * fTimeScale << newl;
	Console << _T("Total Textures: ") << g_llTotalTextures << newl;
	Console << _T("Missing Textures: ") << g_uiMissingTextures << newl;
	Console << _T("Cache Misses: ") << g_uiCacheMisses << newl;
	for (tsvector_it it(g_vCacheMisses.begin()); it != g_vCacheMisses.end(); it++)
		Console << *it << newl;

	return true;
}
