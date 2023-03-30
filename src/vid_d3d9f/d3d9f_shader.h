// (C)2005 S2 Games
// d3d9f_shader.h
//
// Direct3D Shaders
//=============================================================================
#ifndef __D3D9F_SHADER_H__
#define __D3D9F_SHADER_H__

//=============================================================================
// Headers
//=============================================================================
#include "d3d9f_main.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int	MAX_SHADERS(2048);

enum eVertexType
{
	VERTEX_GUI,
	VERTEX_FOLIAGE,
	VERTEX_EFFECT,
	VERTEX_TERRAIN,
	VERTEX_POSITION,
	VERTEX_LINE,
	VERTEX_SKYBOX,
	VERTEX_EXTENDED,
	VERTEX_TREE_BILLBOARD,
	NUM_VERTEX_TYPES
};

class CShaderVar;
class CShaderSampler;
class IMaterialParameter;
class CVertexShaderFileCallback;
class CPixelShaderFileCallback;

struct SVertexShaderConstant
{
	tstring			sName;
	D3DXHANDLE		hHandle;
	CShaderVar*		pShaderVar;
	uint			uiRegisterIndex;
	uint			uiSize;
};

struct SVertexShaderSlot
{
	bool						bActive;
	IDirect3DVertexShader9		*pShader;
	ID3DXConstantTable			*pConstantTable;
	CVertexShaderFileCallback	*pFileCallback;

	vector<SVertexShaderConstant>	vConstant;

	uint						uiNumRegisters;
	uint						uiNumConstants;

	uint						uiCRC32;
};

struct SPixelShaderConstant
{
	tstring				sName;
	D3DXHANDLE			hHandle;
	D3DXREGISTER_SET	eRegisterSet;
	uint				uiRegisterIndex;
	uint				uiSize;
	CShaderVar*			pShaderVar;
	CShaderSampler*		pShaderSamplers;
	uint				uiSubTexture;
};

struct SPixelShaderSlot
{
	bool						bActive;
	IDirect3DPixelShader9		*pShader;
	ID3DXConstantTable			*pConstantTable;
	CPixelShaderFileCallback	*pFileCallback;

	vector<SPixelShaderConstant>	vConstant;

	uint						uiNumRegisters;
	uint						uiNumConstants;

	uint						uiCRC32;
};
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CMaterial;
class CVertexShader;
class CPixelShader;

void	D3D_InitShader();
void	D3D_FrameShader();

int		D3D_RegisterVertexShader(CVertexShader *pVertexShader);
void	D3D_UnregisterVertexShader(CVertexShader *pVertexShader);

int		D3D_RegisterPixelShader(CPixelShader *pPixelShader);
void	D3D_UnregisterPixelShader(CPixelShader *pPixelShader);

void	D3D_RegisterShaderPair(CVertexShader *pVertexShader, CPixelShader *pPixelShader);

int		D3D_RegisterVertexDeclaration(dword fvf);
bool	D3D_LoadVertexShader(const tstring &sFilename, const tstring &sFunction, int &iIndex);
void	D3D_FreeVertexShader(int iShaderIndex);
bool	D3D_LoadPixelShader(const tstring &sFilename, const tstring &sFunction, int &iIndex);
void	D3D_FreePixelShader(int iShaderIndex);

void	D3D_DestroyShader();
void	D3D_ShutdownShader();

extern IDirect3DVertexDeclaration9	*g_pVertexDeclarations[NUM_VERTEX_TYPES + MAX_MESHES];
extern SVertexShaderSlot			g_aVertexShaderSlots[MAX_SHADERS];
extern SPixelShaderSlot				g_aPixelShaderSlots[MAX_SHADERS];

extern ResHandle					g_hNullMeshVS, g_hNullMeshPS;

extern int							g_iMaxVertexShader;
extern int							g_iMaxPixelShader;

extern uint g_auiMaxAniso[NUM_TEXTUREFILTERING_MODES];

EXTERN_CVAR_BOOL(vid_shadowFalloff);
EXTERN_CVAR_BOOL(vid_shaderPrecache);
EXTERN_CVAR_BOOL(vid_shaderRXGBNormalmap);
EXTERN_CVAR_BOOL(vid_terrainAlphamap);
EXTERN_CVAR_BOOL(vid_shaderGroundAmbient);
EXTERN_CVAR_INT(vid_shaderLightingQuality);
//=============================================================================

#endif // __D3D9F_SHADER_H__