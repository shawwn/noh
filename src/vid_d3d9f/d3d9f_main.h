// (C)2005 S2 Games
// d3d9f_main.h
//
// Direct3D
//=============================================================================
#ifndef __D3D9F_MAIN_H__
#define __D3D9F_MAIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_vid.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBitmap;
class CWorld;

void	D3D_PrintWarnings();
int		D3D_Init();
void	D3D_Start();
int		D3D_SetMode();
bool	D3D_GetMode(int mode, struct SVidMode *vidmode);
bool	D3D_IsFullScreen(void);
void	D3D_Shutdown(void);

void	D3D_BeginFrame(void);
void	D3D_EndFrame(void);
void	D3D_InitScene(void);
void	D3D_RenderScene(class CCamera &camera);
void	D3D_RenderFogofWar(float fClear, bool bTexture, float fLerp);
void	D3D_UpdateFogofWar(const CBitmap &cBmp);
void	D3D_SetColor(CVec4f color);
void	D3D_Notify(int message, const class CWorld &world, int param1, int param2, int param3);
void	D3D_GetFrameBuffer(CBitmap &bmp);
CVec2f	D3D_ProjectVertex(const CCamera &cam, const CVec3f &vecVertex);
void	D3D_SetCursor(ResHandle hCursor);

void*		D3D_GetHWnd();

const int	MAX_TEXTURES = 4096;
const int	MAX_GUIQUADS = 16384;
const int	MAX_FOWQUADS = 2;
const int	MAX_EFFECT_QUADS = 16384;
const int	MAX_BILLBOARDS = 16384; // 132072
const int	MAX_BEAMS = 1024;
const int	MAX_BOXES = 2048;
const int	MAX_POINTS = 8192;
const int	MAX_LINES = 8192;
const int	MAX_MESHES = 4096;
const int	FOLIAGE_BATCH_SIZE = 4192;
const int	MAX_MIPMAP_LEVELS = 11;
const int	MAX_VERTEX_SHADERS = 256;
const int	MAX_VERTEX_DECLARATIONS = 256;
const int	MAX_POINT_LIGHTS = 4;
const int	MAX_EFFECT_TRIANGLES = 32768;
const int	MAX_EXTENDED_TRIANGLES = 1024;
const int	MAX_TREE_BILLBOARDS = 1024;

extern	HDC						g_hDC;

extern	IDirect3DDevice9		*g_pd3dDevice;

extern	IDirect3DVertexBuffer9	*g_pVBGuiQuad;		// Vertex buffer for drawing gui quads
extern	IDirect3DIndexBuffer9	*g_pIBGuiQuad;		// Index buffer for drawing gui quads
extern	DWORD					g_dwGuiQuadBase;

extern	IDirect3DVertexBuffer9	*g_pVBFowQuad;		// Vertex buffer for drawing fog of war quads
extern	IDirect3DIndexBuffer9	*g_pIBFowQuad;		// Index buffer for drawing fog of war quads
extern	DWORD					g_dwFowQuadBase;

extern	IDirect3DVertexBuffer9	*g_pVBScenePoly;	// Vertex buffer for scene polys

extern	IDirect3DVertexBuffer9	*g_pVBEffectQuad;	// Vertex buffer for effect quads (billboards and beams)
extern	IDirect3DIndexBuffer9	*g_pIBEffectQuad;	// Index buffer for effect quads (billboards and beams)
extern	DWORD					g_dwEffectQuadBase;

extern	IDirect3DVertexBuffer9	*g_pVBBox;			// Vertex buffer for drawing bounding boxes
extern	IDirect3DIndexBuffer9	*g_pIBBox;			// Index buffer for drawing bounding boxes

extern	IDirect3DVertexBuffer9	*g_pVBPoint;		// Vertex buffer for drawing points

extern	IDirect3DVertexBuffer9	*g_pVBLine;			// Vertex buffer for drawing lines

extern	IDirect3DVertexBuffer9	*g_pVBEffectTriangle;		// Vertex buffer for effect triangles
extern	DWORD					g_dwEffectTriangleBase;

extern	IDirect3DVertexBuffer9	*g_pVBExtendedTriangle;		// Vertex buffer for extended triangles

extern	IDirect3DVertexBuffer9	*g_pVBTreeBillboard;	// Vertex buffer for billboards
extern	IDirect3DIndexBuffer9	*g_pIBTreeBillboard;	// Index buffer for billboards

extern	IDirect3DVertexBuffer9	*g_pVBSkybox;		// Vertex buffer for skybox
extern	IDirect3DVertexBuffer9	*g_pVBPostBuffer;	// Vertex buffer for post processing

extern	IDirect3DVertexBuffer9	*g_pVBStaticMeshes[MAX_MESHES];
extern	IDirect3DVertexBuffer9	*g_pVBDynamicMeshes[MAX_MESHES];
extern	IDirect3DVertexBuffer9	*g_pVBStaticMeshNormals[MAX_MESHES];
extern	IDirect3DVertexBuffer9	*g_pVBDynamicMeshNormals[MAX_MESHES];
extern	IDirect3DIndexBuffer9	*g_pIBMeshes[MAX_MESHES];

extern	IDirect3DSurface9		*g_pDSShadow;

extern	IDirect3DSurface9		*g_pBackBuffer;
extern	IDirect3DSurface9		*g_pDepthBuffer;

extern	IDirect3DQuery9			*g_pQueueLimitQuery;

extern	D3DVIEWPORT9			g_Viewport;
extern	D3DXMATRIXA16			g_mIdentity;
extern	D3DXMATRIXA16			g_mProj;
extern	D3DXMATRIXA16			g_mView;
extern	D3DXMATRIXA16			g_mViewRotate;
extern	D3DXMATRIXA16			g_mViewOffset;
extern	D3DXMATRIXA16			g_mViewProj;
extern	D3DXMATRIXA16			g_mWorld;
extern	D3DXMATRIXA16			g_mWorldRotate;
extern	D3DXMATRIXA16			g_mWorldViewProj;
extern	D3DXMATRIXA16			g_mLightViewProjTex;
extern	D3DXMATRIXA16			g_mLightViewProjSplit[4];
extern	D3DXMATRIXA16			g_mCloudProj;
extern	D3DXMATRIXA16			g_mFowProj;
extern	uint					g_uiImageWidth;
extern	uint					g_uiImageHeight;
extern	float					g_fSceneScaleX;
extern	float					g_fSceneScaleY;

extern	bool					g_bSplitLightProjection;

extern	bool					g_bFullscreen;
extern	bool					g_bExclusive;

extern	CWorld					*g_pWorld;
extern	const CCamera			*g_pCam;
extern	CVec3f					g_vCamOrigin;
extern	int						g_iScreenWidth;
extern	int						g_iScreenHeight;
extern	dword					g_dwDrawColor;
extern	bool					g_bValidScene;
extern	bool					g_bDeviceLost;

extern	bool					g_bInvertedProjection;

extern SVidMode					g_CurrentVidMode;

EXTERN_CVAR_INT(d3d_flush);
EXTERN_CVAR_BOOL(vid_geometryPreload);
EXTERN_CVAR_INT(terrain_chunkSize);

struct SDeviceCaps
{
	// Texture formats
	bool	bDXT1;
	bool	bDXT3;
	bool	bDXT5;
	bool	bDepthStencil;
	bool	bR32F;
	bool	bA8;
	bool	bA8L8;

	// Render targets
	bool	bRenderTarget;
	bool	bR32FRenderTarget;

	// Render states
	bool	bDepthBias;
	bool	bSlopeScaledDepthBias;

	bool	bQuery;

	bool	bR32FShadowFilter;

	uint	uiMaxAniso;

	bool	bNullRenderTarget;

	uint	uiMaxTextureWidth;
	uint	uiMaxTextureHeight;
	uint	uiMaxVolumeExtent;
};
extern SDeviceCaps g_DeviceCaps;

extern	dword					g_dwNumSamplers;

#define D3DFVF_NORMAL4B			D3DFVF_XYZRHW

const D3DFORMAT NV_D3DFMT_NULL(D3DFORMAT(MAKEFOURCC('N','U','L','L')));
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SGuiVertex
{
	float x, y, z;
	dword color;
	float tu, tv;
};

struct SFoliageVertex
{
	CVec3f	v;
	CVec4b	n;
	CVec4b	data;
};

struct SEffectVertex
{
	CVec3f	v;
	dword	color;
	CVec4f	t;
};

struct SExtendedVertex
{
	CVec3f	v;
	CVec3f	n;
	dword	color;
	CVec4f	t;
	CVec3f	tan;
};

struct SPositionVertex
{
	CVec3f	v;
};

struct SLineVertex
{
	CVec3f	v;
	dword	color;
};

struct SSkyboxVertex
{
	CVec4f	v;
};

struct STreeBillboardVertex
{
	CVec3f	v;
	dword	color;
	CVec2f	t;
};

#define SAFE_RELEASE(p)	{ if(p) { (p)->Release(); (p)=NULL; } }
//=============================================================================

#endif // __D3D9F_MAIN_H__
