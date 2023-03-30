// (C)2005 S2 Games
// d3d9f_texture.h
//
// Direct3D Shaders
//=============================================================================
#ifndef __D3D9F_TEXTURE_H__
#define __D3D9F_TEXTURE_H__

//=============================================================================
// Headers
//=============================================================================
#include "d3d9f_main.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CMaterial;
class CProcedural;
class CTexture;

enum ETextureFormat;
enum ETextureType;

void    D3D_InitTexture();

int     D3D_Register2DTexture(const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat, const tstring &sName);
int     D3D_RegisterTexture(CTexture *pTexture);
void    D3D_UnregisterTexture(CTexture *pTexture);
void    D3D_Unregister2DTexture(const tstring &sName);

int     D3D_Register2DProcedural(const CProcedural &cProcedural, int &iTexture2);
int     D3D_RegisterRenderTargetTexture(const tstring &sName, int iSizeX, int iSizeY, D3DFORMAT eFormat, bool bMipmap);
int     D3D_RegisterDepthTexture(const tstring &sName, int iSizeX, int iSizeY, D3DFORMAT eFormat);
int     D3D_RegisterDynamicTexture(const tstring &sName, const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat);
int     D3D_RegisterBlankTexture(const tstring &sName, int iSizeX, int iSizeY, D3DFORMAT eFormat, bool bMipmap);

void    D3D_Update2DTexture(int iIndex, const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat);
void    D3D_UpdateDynamicTexture(int iIndex, const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat);

CVec4f  D3D_GetTextureColor(CTexture *pResource);

bool    D3D_TextureExists(const tstring &sFilename, uint uiTextureFlags);

void    D3D_InitShadowmap();
void    D3D_ReleaseShadowmap();
void    D3D_DestroyShadowmap();

void    D3D_OpenTextureArchive(bool bNoReload = false);
void    D3D_CloseTextureArchive();

void    D3D_GetTextureList(const tstring &sSearch, tsvector &vResult);

void    D3D_TextureDefine(const string &sName, const string &sDefinition = "");
void    D3D_TextureUndefine(const string &sName);
string  D3D_GetTextureDefinitionString();

extern int                  g_iWhite;
extern int                  g_iBlack;
extern int                  g_iInvis;

struct STextureMapEntry
{
    int             iIndex;
    int             iIndex2;
    ResHandle       hTexture;

    STextureMapEntry() {}
    STextureMapEntry(int _iIndex, int _iIndex2, ResHandle _hTexture) : iIndex(_iIndex), iIndex2(_iIndex2), hTexture(_hTexture) {}
};

typedef map<tstring, STextureMapEntry>  TextureMap;
extern  TextureMap                      g_mapTextures;

extern  IDirect3DBaseTexture9   *g_pTextures[MAX_TEXTURES];
extern  IDirect3DCubeTexture9   *g_pTexturesCube[MAX_TEXTURES];
extern  IDirect3DTexture9       *g_pTextures2D[MAX_TEXTURES];
extern  IDirect3DVolumeTexture9 *g_pTexturesVolume[MAX_TEXTURES];

extern ResHandle    g_hSkyReference;
extern ResHandle    g_hSkinReference;

EXTERN_CVAR_FLOAT(vid_textureGammaCorrect);
//=============================================================================

#endif // __D3D9F_SHADER_H__
