// (C)2005 S2 Games
// d3d9f_main.cpp
//
// Direct3D functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "../public/vid_driver_t.h"

#include "d3d9f_main.h"
#include "d3d9f_state.h"
#include "d3d9f_terrain.h"
#include "d3d9f_foliage.h"
#include "d3d9f_shader.h"
#include "d3d9f_material.h"
#include "d3d9f_model.h"
#include "d3d9f_scene.h"
#include "d3d9f_util.h"
#include "d3d9f_texture.h"
#include "c_treemodeldef.h"
#include "c_treescenemanager.h"
#include "c_shadowmap.h"
#include "c_fogofwar.h"
#include "c_postbuffer.h"
#include "c_scenebuffer.h"
#include "c_reflectionmap.h"
#include "c_shadercache.h"
#include "c_shaderregistry.h"
#include "c_texturecache.h"
#include "c_gamma.h"

#include "resource.h"

#include "../k2/c_vid.h"
#include "../k2/c_console.h"
#include "../k2/c_system.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_cursor.h"
#include "../k2/c_mesh.h"
#include "../k2/c_camera.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_texture.h"
#include "../k2/c_uimanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
WNDPROC					MainWndProc;
HINSTANCE				g_hInstance;

HWND					g_hWnd;
HDC						g_hDC;
IDirect3D9				*g_pD3D;
IDirect3DDevice9		*g_pd3dDevice;
D3DPRESENT_PARAMETERS	g_d3dPP;

IDirect3DVertexBuffer9	*g_pVBGuiQuad;		// Vertex buffer for drawing gui quads
IDirect3DIndexBuffer9	*g_pIBGuiQuad;		// Index buffer for drawing gui quads
DWORD					g_dwGuiQuadBase;

IDirect3DVertexBuffer9	*g_pVBFowQuad;		// Vertex buffer for drawing fog of war quads
IDirect3DIndexBuffer9	*g_pIBFowQuad;		// Index buffer for drawing fog of war quads
DWORD					g_dwFowQuadBase;

IDirect3DVertexBuffer9	*g_pVBScenePoly;	// Vertex buffer for scene polys

IDirect3DVertexBuffer9	*g_pVBEffectQuad;	// Vertex buffer for effect quads (billboards and beams)
IDirect3DIndexBuffer9	*g_pIBEffectQuad;	// Index buffer for effect quads (billboards and beams)
DWORD					g_dwEffectQuadBase;

IDirect3DVertexBuffer9	*g_pVBBox;			// Vertex buffer for drawing bounding boxes
IDirect3DIndexBuffer9	*g_pIBBox;			// Index buffer for drawing bounding boxes

IDirect3DVertexBuffer9	*g_pVBPoint;		// Vertex buffer for drawing points

IDirect3DVertexBuffer9	*g_pVBLine;			// Vertex buffer for drawing lines

IDirect3DVertexBuffer9	*g_pVBEffectTriangle;		// Vertex buffer for effect triangles
DWORD					g_dwEffectTriangleBase;

IDirect3DVertexBuffer9	*g_pVBExtendedTriangle;		// Vertex buffer for extended triangles

IDirect3DVertexBuffer9	*g_pVBTreeBillboard;		// Vertex buffer for tree billboards
IDirect3DIndexBuffer9	*g_pIBTreeBillboard;		// Vertex buffer for tree billboards

IDirect3DVertexBuffer9	*g_pVBSkybox;		// Vertex buffer for skybox
IDirect3DVertexBuffer9	*g_pVBPostBuffer;	// Vertex buffer for post processing

IDirect3DVertexBuffer9	*g_pVBStaticMeshes[MAX_MESHES];
IDirect3DVertexBuffer9	*g_pVBDynamicMeshes[MAX_MESHES];
IDirect3DVertexBuffer9	*g_pVBStaticMeshNormals[MAX_MESHES];
IDirect3DVertexBuffer9	*g_pVBDynamicMeshNormals[MAX_MESHES];
IDirect3DIndexBuffer9	*g_pIBMeshes[MAX_MESHES];

IDirect3DSurface9		*g_pBackBuffer;
IDirect3DSurface9		*g_pDepthBuffer;

IDirect3DQuery9			*g_pQueueLimitQuery;
bool					g_bQueueLimitQuery(false);

D3DVIEWPORT9			g_Viewport;
D3DXMATRIXA16			g_mIdentity;
D3DXMATRIXA16			g_mProj;
D3DXMATRIXA16			g_mView;
D3DXMATRIXA16			g_mViewRotate;
D3DXMATRIXA16			g_mViewOffset;
D3DXMATRIXA16			g_mViewProj;
D3DXMATRIXA16			g_mWorld;
D3DXMATRIXA16			g_mWorldRotate;
D3DXMATRIXA16			g_mWorldViewProj;
D3DXMATRIXA16			g_mLightViewProjTex;
D3DXMATRIXA16			g_mLightViewProjSplit[4];
D3DXMATRIXA16			g_mCloudProj;
D3DXMATRIXA16			g_mFowProj;
uint					g_uiImageWidth;
uint					g_uiImageHeight;
float					g_fSceneScaleX;
float					g_fSceneScaleY;

bool					g_bSplitLightProjection = false;

bool					g_bFullscreen = false;
SDeviceCaps				g_DeviceCaps;

CWorld					*g_pWorld;
const CCamera			*g_pCam;
CVec3f					g_vCamOrigin;
int						g_iScreenWidth;
int						g_iScreenHeight;
DWORD					g_dwDrawColor = 0xffffff;

bool					g_bD3DInitialized = false;
bool					g_bValidScene = false;
bool					g_bDeviceLost = false;
dword					g_dwNumSamplers(0);
dword					g_dwMaxSimultaneousTextures(0);
bool					g_bExclusive(false);
int						g_iMaxDynamicLights;

int						g_iCurrentVideoMode(-1);
SVidMode				g_CurrentVidMode;
SVidMode				g_VidModes[MAX_VID_MODES];
int						g_iNumVidModes;

SAAMode					g_CurrentAAMode;
SAAMode					g_AAModes[MAX_AA_MODES];
int						g_iNumAAModes;

ResHandle				g_hCursor(INVALID_RESOURCE);

const float				DEFAULT_OVERBRIGHT = 1.0f;

void	D3D_CheckDeviceFormats();
void	D3D_Setup2D();
void	D3D_DrawDebugPoly();
void	D3D_Release();
void	D3D_Clear();

CVAR_INT	(terrain_chunkSize,		64);
CVAR_INT	(foliage_chunkSize,		16);
CVAR_BOOLF	(d3d_exclusive,			true,	CVAR_SAVECONFIG);
CVAR_BOOLF	(d3d_pure,				true,	CVAR_SAVECONFIG);
CVAR_BOOLF	(d3d_hardwareTL,		true,	CVAR_SAVECONFIG);
CVAR_BOOLF	(d3d_hardwareRaster,	true,	CVAR_SAVECONFIG);
CVAR_INTF	(d3d_presentInterval,	0,		CVAR_SAVECONFIG);
CVAR_INTF	(d3d_flush,				1,		CVAR_SAVECONFIG);
CVAR_BOOLF	(vid_geometryPreload,	true,	CVAR_SAVECONFIG);
CVAR_BOOLF	(d3d_tripleBuffering,	false,	CVAR_SAVECONFIG);
CVAR_BOOLF	(d3d_software,			false,	CVAR_SAVECONFIG);

CVAR_STRINGF	(d3d_pixelShaderProfile,	"",		CVAR_READONLY);
CVAR_STRINGF	(d3d_vertexShaderProfile,	"",		CVAR_READONLY);

CVAR_BOOL	(win_changeDisplayMode,		false);

uint g_uiInitialTextureMem(0);
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

//=============================================================================
// Utility Functions
//=============================================================================

/*====================
  D3D_InitVBs

  Register non-managed VBs and IBs
  ====================*/
void	D3D_InitVBs()
{
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(1024 * sizeof(SEffectVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBScenePoly, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBScenePoly"));

	//
	// Billboard buffers
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(MAX_EFFECT_QUADS * 4 * sizeof(SEffectVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBEffectQuad, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBEffectQuad"));

	g_dwEffectQuadBase = 0;

	//
	// Box buffers
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(MAX_BOXES * 8 * sizeof(SLineVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBBox, NULL)))
		EX_FATAL(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBBox"));

	//
	// Point buffers
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(MAX_POINTS * sizeof(SLineVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBPoint, NULL)))
		EX_FATAL(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBPoint"));

	//
	// Line buffers
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(MAX_LINES * 2 * sizeof(SLineVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBLine, NULL)))
		EX_FATAL(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBLine"));

	//
	// Gui buffers
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(MAX_GUIQUADS * 4 * sizeof(SGuiVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBGuiQuad, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBGuiQuad"));

	g_dwGuiQuadBase = 0;

	//
	// Fog of war buffers
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(MAX_FOWQUADS * 4 * sizeof(SGuiVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBFowQuad, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBFowQuad"));

	g_dwFowQuadBase = 0;

	//
	// Skybox buffer
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(4 * sizeof(SSkyboxVertex),
		D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &g_pVBSkybox, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBSkybox"));

	SSkyboxVertex *pVertices;
	if (SUCCEEDED(g_pVBSkybox->Lock(0, 4 * sizeof(SSkyboxVertex), (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
	{
		if (pVertices != NULL)
		{
			pVertices[0].v = CVec4f(-1.0f, -1.0f, 1.0f, 1.0f);
			pVertices[1].v = CVec4f( 1.0f, -1.0f, 1.0f, 1.0f);
			pVertices[2].v = CVec4f( 1.0f,  1.0f, 1.0f, 1.0f);
			pVertices[3].v = CVec4f(-1.0f,  1.0f, 1.0f, 1.0f);
		}
		else
			Console << _T("Invalid skybox vertices pointer!") << newl;

		g_pVBSkybox->Unlock();
	}

	//
	// Effect triangle buffer
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(MAX_EFFECT_TRIANGLES * 3 * sizeof(SEffectVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBEffectTriangle, NULL)))
		EX_FATAL(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBEffectTriangle"));

	g_dwEffectTriangleBase = 0;

	//
	// Extended triangle buffer
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(MAX_EXTENDED_TRIANGLES * 3 * sizeof(SExtendedVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBExtendedTriangle, NULL)))
		EX_FATAL(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBExtendedTriangle"));

	//
	// Tree Billboard buffers
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(MAX_TREE_BILLBOARDS * 4 * sizeof(STreeBillboardVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBTreeBillboard, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBTreeBillboard"));

	//
	// Post processing buffer
	//

	if (FAILED(g_pd3dDevice->CreateVertexBuffer(4 * sizeof(SGuiVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBPostBuffer, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateVertexBuffer failed on g_pVBPostBuffer"));

	{
		SGuiVertex *pVertices;
		if (SUCCEEDED(g_pVBPostBuffer->Lock(0, 4 * sizeof(SGuiVertex), (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
		{
			if (pVertices != NULL)
			{
				pVertices[0].x = 0.0f;
				pVertices[0].y = 0.0f;
				pVertices[0].tu = 0.0f;
				pVertices[0].tv = 0.0f;
				pVertices[0].color = 0xffffffff;

				pVertices[1].x = 0.0f;
				pVertices[1].y = 1.0f;
				pVertices[1].tu = 0.0f;
				pVertices[1].tv = 1.0f;
				pVertices[1].color = 0xffffffff;

				pVertices[2].x = 1.0f;
				pVertices[2].y = 1.0f;
				pVertices[2].tu = 1.0f;
				pVertices[2].tv = 1.0f;
				pVertices[2].color = 0xffffffff;

				pVertices[3].x = 1.0f;
				pVertices[3].y = 0.0f;
				pVertices[3].tu = 1.0f;
				pVertices[3].tv = 0.0f;
				pVertices[3].color = 0xffffffff;
			}
			else
				Console << _T("Invalid post processing vertices pointer!") << newl;

			g_pVBPostBuffer->Unlock();
		}
	}
}


/*====================
  D3D_InitVBsManaged

  Register managed VBs and IBs
  ====================*/
void	D3D_InitVBsManaged()
{
	WORD *pIndices16;

	//
	// Effect quad buffers
	//

	if (FAILED(g_pd3dDevice->CreateIndexBuffer(MAX_EFFECT_QUADS * 6 * sizeof(WORD),
					D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pIBEffectQuad, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateIndexBuffer failed"));

	if (SUCCEEDED(g_pIBEffectQuad->Lock(0, MAX_EFFECT_QUADS * 6 * sizeof(WORD), (void**)&pIndices16, D3DLOCK_NOSYSLOCK)))
	{
		int i = 0;

		for (int n = 0; n < MAX_EFFECT_QUADS; ++n)
		{
			int n2(n << 2);

			pIndices16[i++] = n2;
			pIndices16[i++] = n2 + 1;
			pIndices16[i++] = n2 + 2;
			pIndices16[i++] = n2;
			pIndices16[i++] = n2 + 2;
			pIndices16[i++] = n2 + 3;
		}

		g_pIBEffectQuad->Unlock();
	}

	//
	// Box buffers
	//

	if (FAILED(g_pd3dDevice->CreateIndexBuffer(MAX_BOXES * 24 * sizeof(WORD),
					D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pIBBox, NULL)))
		EX_FATAL(_T("D3D_InitVBs(): CreateIndexBuffer failed"));

	if (SUCCEEDED(g_pIBBox->Lock(0, MAX_BOXES * 24 * sizeof(WORD), (void**)&pIndices16, D3DLOCK_NOSYSLOCK)))
	{
		int i = 0;
		for (int n = 0; n < MAX_BOXES; ++n)
		{
			// Top
			pIndices16[i++] = (n * 8) + 0;
			pIndices16[i++] = (n * 8) + 1;

			pIndices16[i++] = (n * 8) + 1;
			pIndices16[i++] = (n * 8) + 2;

			pIndices16[i++] = (n * 8) + 2;
			pIndices16[i++] = (n * 8) + 3;

			pIndices16[i++] = (n * 8) + 3;
			pIndices16[i++] = (n * 8) + 0;

			// Sides
			pIndices16[i++] = (n * 8) + 0;
			pIndices16[i++] = (n * 8) + 4;

			pIndices16[i++] = (n * 8) + 1;
			pIndices16[i++] = (n * 8) + 5;

			pIndices16[i++] = (n * 8) + 2;
			pIndices16[i++] = (n * 8) + 6;

			pIndices16[i++] = (n * 8) + 3;
			pIndices16[i++] = (n * 8) + 7;

			// Bottom
			pIndices16[i++] = (n * 8) + 4;
			pIndices16[i++] = (n * 8) + 5;

			pIndices16[i++] = (n * 8) + 5;
			pIndices16[i++] = (n * 8) + 6;

			pIndices16[i++] = (n * 8) + 6;
			pIndices16[i++] = (n * 8) + 7;

			pIndices16[i++] = (n * 8) + 7;
			pIndices16[i++] = (n * 8) + 4;
		}

		g_pIBBox->Unlock();
	}

	//
	// Gui buffers
	//

	if (FAILED(g_pd3dDevice->CreateIndexBuffer(MAX_GUIQUADS * 6 * sizeof(WORD),
					D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pIBGuiQuad, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateIndexBuffer failed"));

	if (SUCCEEDED(g_pIBGuiQuad->Lock(0, MAX_GUIQUADS * 6 * sizeof(WORD), (void**)&pIndices16, D3DLOCK_NOSYSLOCK)))
	{
		int i = 0;

		for (int n = 0; n < MAX_GUIQUADS; ++n)
		{
			pIndices16[i++] = (n<<2);
			pIndices16[i++] = (n<<2) + 1;
			pIndices16[i++] = (n<<2) + 2;
			pIndices16[i++] = (n<<2);
			pIndices16[i++] = (n<<2) + 2;
			pIndices16[i++] = (n<<2) + 3;
		}

		g_pIBGuiQuad->Unlock();
	}

	//
	// Fog of war buffers
	//

	if (FAILED(g_pd3dDevice->CreateIndexBuffer(MAX_FOWQUADS * 6 * sizeof(WORD),
					D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pIBFowQuad, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateIndexBuffer failed"));

	if (SUCCEEDED(g_pIBFowQuad->Lock(0, MAX_FOWQUADS * 6 * sizeof(WORD), (void**)&pIndices16, D3DLOCK_NOSYSLOCK)))
	{
		int i = 0;

		for (int n = 0; n < MAX_FOWQUADS; ++n)
		{
			pIndices16[i++] = (n<<2);
			pIndices16[i++] = (n<<2) + 1;
			pIndices16[i++] = (n<<2) + 2;
			pIndices16[i++] = (n<<2);
			pIndices16[i++] = (n<<2) + 2;
			pIndices16[i++] = (n<<2) + 3;
		}

		g_pIBFowQuad->Unlock();
	}

	//
	// Tree Billboard buffer
	//

	if (FAILED(g_pd3dDevice->CreateIndexBuffer(MAX_TREE_BILLBOARDS * 6 * sizeof(WORD),
					D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pIBTreeBillboard, NULL)))
		K2System.Error(_T("D3D_InitVBs(): CreateIndexBuffer failed"));

	if (SUCCEEDED(g_pIBTreeBillboard->Lock(0, MAX_TREE_BILLBOARDS * 6 * sizeof(WORD), (void**)&pIndices16, D3DLOCK_NOSYSLOCK)))
	{
		int i = 0;

		for (int n = 0; n < MAX_TREE_BILLBOARDS; ++n)
		{
			pIndices16[i++] = (n<<2);
			pIndices16[i++] = (n<<2) + 1;
			pIndices16[i++] = (n<<2) + 2;
			pIndices16[i++] = (n<<2);
			pIndices16[i++] = (n<<2) + 2;
			pIndices16[i++] = (n<<2) + 3;
		}

		g_pIBTreeBillboard->Unlock();
	}
}
//=============================================================================

//=============================================================================
// Vid Interface
//=============================================================================

/*====================
  D3D_Init
  ====================*/
int		D3D_Init()
{
	g_bD3DInitialized = false;

	// Grab the DX9 interface
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (g_pD3D == NULL)
		K2System.Error(_T("D3D_Init: Direct3DCreate9() failed"));

	// List devices
	uint uiNumAdapters(g_pD3D->GetAdapterCount());
	Console.Video << _T("Found ") << uiNumAdapters << _T(" video adapters") << newl;
	for (uint ui(0); ui < uiNumAdapters; ++ui)
	{
		D3DADAPTER_IDENTIFIER9 d3dAdapter;
		g_pD3D->GetAdapterIdentifier(ui, 0, &d3dAdapter);
		Console.Video
			<< _T("[") << ui << _T("] - ") << d3dAdapter.Description << newl
			<< _T("Name:      ") << d3dAdapter.DeviceName << newl
			<< _T("Driver:    ") << d3dAdapter.Driver << newl
			<< _T("Version:   ") << HIWORD(d3dAdapter.DriverVersion.HighPart) << _T(".")
							<< LOWORD(d3dAdapter.DriverVersion.HighPart) << _T(".")
							<< HIWORD(d3dAdapter.DriverVersion.LowPart) << _T(".")
							<< LOWORD(d3dAdapter.DriverVersion.LowPart) << newl
			<< _T("Vendor ID: ") << INT_HEX_STR(d3dAdapter.VendorId) << newl
			<< _T("Device ID: ") << INT_HEX_STR(d3dAdapter.DeviceId) << newl
			<< _T("SubSys ID: ") << INT_HEX_STR(d3dAdapter.SubSysId) << newl
			<< _T("Revision:  ") << INT_HEX_STR(d3dAdapter.Revision) << newl;
	}

	// EnumDisplaySettings
	int modenum = 0;
	BOOL bSuccess(FALSE);
	DEVMODE devmode;

	ZeroMemory(&devmode, sizeof(devmode));
	devmode.dmSize = sizeof(DEVMODE);

	// Fill in the default mode (Desktop)
	if (EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devmode))
	{
		g_VidModes[0].iWidth = devmode.dmPelsWidth;
		g_VidModes[0].iHeight = devmode.dmPelsHeight;
		g_VidModes[0].iBpp = devmode.dmBitsPerPel;
		g_VidModes[0].iRefreshRate = devmode.dmDisplayFrequency;
		g_VidModes[0].sName = _T("Desktop ") + ParenStr(XtoA(g_VidModes[0].iWidth) + _T("x") + XtoA(g_VidModes[0].iHeight) + _T("x") + XtoA(g_VidModes[0].iBpp) + _T("@") + XtoA(g_VidModes[0].iRefreshRate));
	}
	else
		K2System.Error(_T("EnumDisplaySettings failed on mode 0"));

	g_iNumVidModes = 1;  // Always keep mode 0 open

	do
	{
		bool bModeExists(false);
		bSuccess = EnumDisplaySettings(NULL, modenum, &devmode);
		if (!bSuccess)
		{
			if (modenum == 0)
				K2System.Error(_T("EnumDisplaySettings failed on mode 0"));
			break;
		}

		if (devmode.dmBitsPerPel >= 15 && devmode.dmPelsWidth >= 800 && devmode.dmPelsHeight >= 600 && g_iNumVidModes < MAX_VID_MODES)
		{
			// See if the mode is valid
			devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&devmode, CDS_TEST | CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
			{
				// Make sure the mode doesn't already exist
				for (int n = 1; n < g_iNumVidModes; ++n)
				{
					if (g_VidModes[n].iWidth == int(devmode.dmPelsWidth) &&
						g_VidModes[n].iHeight == int(devmode.dmPelsHeight) &&
						g_VidModes[n].iBpp == int(devmode.dmBitsPerPel) &&
						g_VidModes[n].iRefreshRate == int(devmode.dmDisplayFrequency))
						bModeExists = true;
				}
				if (!bModeExists)
				{
					// The mode is valid, so add it to g_VidModes
					g_VidModes[g_iNumVidModes].iWidth = int(devmode.dmPelsWidth);
					g_VidModes[g_iNumVidModes].iHeight = int(devmode.dmPelsHeight);
					g_VidModes[g_iNumVidModes].iBpp = int(devmode.dmBitsPerPel);
					g_VidModes[g_iNumVidModes].iRefreshRate = int(devmode.dmDisplayFrequency);
					g_VidModes[g_iNumVidModes].sName = XtoA(devmode.dmPelsWidth) + _T("x") + XtoA(devmode.dmPelsHeight) + _T("x") + XtoA(devmode.dmBitsPerPel) + _T(" @ ") + XtoA(devmode.dmDisplayFrequency) + _T(" Hz");

					Console.Video << _T("D3D mode ") << g_iNumVidModes << _T(": ")
								<< _T("Width: ") << devmode.dmPelsWidth
								<< _T(", Height: ") << devmode.dmPelsHeight
								<< _T(", Bpp: ") << devmode.dmBitsPerPel
								<< _T(", Refresh rate: ") << devmode.dmDisplayFrequency << newl;
					++g_iNumVidModes;
				}
			}
		}
		++modenum;
	}
	while (bSuccess);

	// Build anti-aliasing mode list
	g_AAModes[0].iSamples = 0;
	g_AAModes[0].iQuality = 0;
	g_AAModes[0].sName = _T("None");
	g_iNumAAModes = 1; // Always keep mode 0 open

	for (int i(2); i <= 16; ++i)
	{
		if (FAILED(g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_D24X8, FALSE, D3DMULTISAMPLE_TYPE(i), NULL)))
			continue;

		g_AAModes[g_iNumAAModes].iSamples = i;
		g_AAModes[g_iNumAAModes].iQuality = 0;
		g_AAModes[g_iNumAAModes].sName = XtoA(i) + _T("x");

		++g_iNumAAModes;
	}

	g_ShaderCache.Initialize();
	g_TextureCache.Initialize();
	g_Gamma.SaveGDIGammaRamp();

	return 1;
}


/*====================
  D3D_SetupPresentParams
  ====================*/
void	D3D_SetupPresentParams(D3DPRESENT_PARAMETERS &d3dpp)
{
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	switch (g_CurrentVidMode.iBpp)
	{
	case 32:
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		break;
	case 16:
		d3dpp.BackBufferFormat = D3DFMT_R5G6B5;
		break;
	case 15:
		d3dpp.BackBufferFormat = D3DFMT_R5G6B5;
		break;
	}

	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferCount = d3d_tripleBuffering ? 2 : 1;
	d3dpp.BackBufferWidth = g_CurrentVidMode.iWidth;
	d3dpp.BackBufferHeight = g_CurrentVidMode.iHeight;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

	switch (d3d_presentInterval)
	{
	case -1:
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		break;
	default:
	case 0:
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		break;
	case 1:
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		break;
	case 2:
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_TWO;
		break;
	case 3:
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_THREE;
		break;
	case 4:
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_FOUR;
		break;
	}

	// Validate fullscreen mode
	bool bValidMode(false);

	if (g_bFullscreen && d3d_exclusive)
	{
		UINT uiNumModes(g_pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, d3dpp.BackBufferFormat));
		for (UINT uiMode(0); uiMode < uiNumModes; ++uiMode)
		{
			D3DDISPLAYMODE displayMode;
			ZeroMemory(&displayMode, sizeof(displayMode));
			
			if (FAILED(g_pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, d3dpp.BackBufferFormat, uiMode, &displayMode)))
				continue;

			if (displayMode.Width == d3dpp.BackBufferWidth &&
				displayMode.Height == d3dpp.BackBufferHeight &&
				displayMode.Format == d3dpp.BackBufferFormat &&
				displayMode.RefreshRate == g_CurrentVidMode.iRefreshRate)
				bValidMode = true;
		}
	}
	else
	{
		bValidMode = true;
	}

	D3DMULTISAMPLE_TYPE eMultiSampleType(D3DMULTISAMPLE_NONE);

	if (g_CurrentAAMode.iSamples < 2)
		eMultiSampleType = D3DMULTISAMPLE_NONE;
	else if (g_CurrentAAMode.iSamples <= 16)
		eMultiSampleType = D3DMULTISAMPLE_TYPE(g_CurrentAAMode.iSamples);

	if (!FAILED(g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, FALSE, eMultiSampleType, NULL)) &&
		!FAILED(g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_D24X8, FALSE, eMultiSampleType, NULL)))
		d3dpp.MultiSampleType = eMultiSampleType;
	else
	{
		d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
		g_CurrentAAMode.iSamples = 0;
		g_CurrentAAMode.iQuality = 0;
		g_CurrentAAMode.sName = _T("Fallback");
	}

	if (g_bFullscreen && d3d_exclusive && bValidMode)
	{
		d3dpp.Windowed = FALSE;
		d3dpp.FullScreen_RefreshRateInHz = g_CurrentVidMode.iRefreshRate;

		g_bExclusive = true;
	}
	else
	{
		d3dpp.Windowed = TRUE;
		d3dpp.EnableAutoDepthStencil = TRUE;

		g_bExclusive = false;
	}
}


/*====================
  D3D_GetDeviceCaps

  Determine features supported by this video card in this video mode
  ====================*/
void	D3D_GetDeviceCaps()
{
	D3D_CheckDeviceFormats();

	D3DCAPS9 d3dCaps;
    g_pd3dDevice->GetDeviceCaps(&d3dCaps);

	g_DeviceCaps.bDepthBias = (d3dCaps.RasterCaps & D3DPRASTERCAPS_DEPTHBIAS) != 0;
	g_DeviceCaps.bSlopeScaledDepthBias = (d3dCaps.RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS) != 0;

	g_dwMaxSimultaneousTextures = d3dCaps.MaxSimultaneousTextures;

	g_iMaxDynamicLights = 0;
	g_DeviceCaps.bR32FShadowFilter = false;

	g_DeviceCaps.uiMaxAniso = d3dCaps.MaxAnisotropy;

	g_DeviceCaps.uiMaxTextureWidth = d3dCaps.MaxTextureWidth;
	g_DeviceCaps.uiMaxTextureHeight = d3dCaps.MaxTextureHeight;
	g_DeviceCaps.uiMaxVolumeExtent = d3dCaps.MaxVolumeExtent;
}


/*====================
  D3D_CheckDeviceFormats

  Determine features supported by this video card in this video mode
  ====================*/
void	D3D_CheckDeviceFormats()
{
	D3DCAPS9 d3dCaps;
    g_pd3dDevice->GetDeviceCaps(&d3dCaps);
	D3DDISPLAYMODE displaymode;
	g_pd3dDevice->GetDisplayMode(0, &displaymode);
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT1)))
	{
		Console.Video << _T("DXT1 supported") << newl;
		g_DeviceCaps.bDXT1 = true;
	}
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT2)))
		Console.Video << _T("DXT2 supported") << newl;
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT3)))
	{
		Console.Video << _T("DXT3 supported") << newl;
		g_DeviceCaps.bDXT3 = true;
	}
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT4)))
		Console.Video << _T("DXT4 supported") << newl;
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT5)))
	{
		Console.Video << _T("DXT5 supported") << newl;
		g_DeviceCaps.bDXT5 = true;
	}
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, D3DFMT_D24X8)))
	{
		Console.Video << _T("DepthStencil supported") << newl;
		g_DeviceCaps.bDepthStencil = true;
	}
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_R32F)))
	{
		Console.Video << _T("R32F supported") << newl;
		g_DeviceCaps.bR32F = true;
	}
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A8)))
	{
		Console.Video << _T("A8 supported") << newl;
		g_DeviceCaps.bA8 = true;
	}
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A8L8)))
	{
		Console.Video << _T("A8L8 supported") << newl;
		g_DeviceCaps.bA8L8 = true;
	}
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)))
	{
		Console.Video << _T("A8R8G8B8 render targets supported") << newl;
		g_DeviceCaps.bRenderTarget = true;
	}
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R32F)))
	{
		Console.Video << _T("R32F render targets supported") << newl;
		g_DeviceCaps.bR32FRenderTarget = true;
	}
	if (!FAILED(g_pD3D->CheckDeviceFormat(d3dCaps.AdapterOrdinal, d3dCaps.DeviceType, displaymode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, NV_D3DFMT_NULL)))
	{
		Console.Video << _T("NULL render targets supported") << newl;
		g_DeviceCaps.bNullRenderTarget = true;
	}
}


/*====================
  D3D_SetDefaultStates
  ====================*/
void	D3D_SetDefaultStates()
{
	D3D_SetRenderStateDefault(D3DRS_ZENABLE, FALSE);
	D3D_SetRenderStateDefault(D3DRS_CULLMODE, D3DCULL_NONE);
	D3D_SetRenderStateDefault(D3DRS_LIGHTING, FALSE);
	D3D_SetRenderStateDefault(D3DRS_ALPHABLENDENABLE, TRUE);
	D3D_SetRenderStateDefault(D3DRS_SRCBLEND, D3DBLEND_ONE);
	D3D_SetRenderStateDefault(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	D3D_SetRenderStateDefault(D3DRS_ALPHATESTENABLE, FALSE);
	D3D_SetRenderStateDefault(D3DRS_ALPHAREF, 90);
	D3D_SetRenderStateDefault(D3DRS_ZWRITEENABLE, TRUE);
	D3D_SetRenderStateDefault(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	D3D_SetRenderStateDefault(D3DRS_FOGENABLE, FALSE);
	D3D_SetRenderStateDefault(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	D3D_SetRenderStateDefault(D3DRS_FOGSTART, 0);
	D3D_SetRenderStateDefault(D3DRS_FOGEND, 0);
	D3D_SetRenderStateDefault(D3DRS_FOGCOLOR, 0);
	D3D_SetRenderStateDefault(D3DRS_AMBIENT, 0);
	D3D_SetRenderStateDefault(D3DRS_NORMALIZENORMALS, TRUE);
	D3D_SetRenderStateDefault(D3DRS_FILLMODE, D3DFILL_SOLID);
	D3D_SetRenderStateDefault(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	D3D_SetRenderStateDefault(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
	if (g_DeviceCaps.bDepthBias)
		D3D_SetRenderStateDefault(D3DRS_DEPTHBIAS, D3D_DWORD(0.0f));
	if (g_DeviceCaps.bSlopeScaledDepthBias)
		D3D_SetRenderStateDefault(D3DRS_SLOPESCALEDEPTHBIAS, D3D_DWORD(0.0f));
	D3D_SetRenderStateDefault(D3DRS_POINTSIZE, D3D_DWORD(4.0f));
	D3D_SetRenderStateDefault(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	D3D_SetRenderStateDefault(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	D3D_SetRenderStateDefault(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
	D3D_SetRenderStateDefault(D3DRS_STENCILENABLE, FALSE);
	D3D_SetRenderStateDefault(D3DRS_STENCILREF, 0);
	D3D_SetRenderStateDefault(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	D3D_SetRenderStateDefault(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
	D3D_SetRenderStateDefault(D3DRS_CLIPPLANEENABLE, 0);

	for (dword i(0); i < g_dwNumSamplers; ++i)
	{
		D3D_SetSamplerStateDefault(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		D3D_SetSamplerStateDefault(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		D3D_SetSamplerStateDefault(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		D3D_SetSamplerStateDefault(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		D3D_SetSamplerStateDefault(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		D3D_SetSamplerStateDefault(i, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);

		D3D_SetSamplerStateDefault(i, D3DSAMP_BORDERCOLOR, D3DCOLOR_ARGB(0, 0, 0, 0));

		D3D_SetSamplerStateDefault(i, D3DSAMP_MAXANISOTROPY, 1);

		D3D_SetTextureDefault(i, NULL);
	}

	// Limited by fixed function texture limit
	for (dword j(0); j < g_dwMaxSimultaneousTextures; ++j)
	{
		D3D_SetTextureStageStateDefault(j, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		D3D_SetTextureStageStateDefault(j, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		D3D_SetTextureStageStateDefault(j, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		D3D_SetTextureStageStateDefault(j, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
		D3D_SetTextureStageStateDefault(j, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	}

	D3D_SetIndicesDefault(NULL);
	D3D_SetStreamSourceDefault(0, NULL, 0, 0);
	D3D_SetVertexDeclarationDefault(NULL);

	D3DXMatrixIdentity(&g_mIdentity);
}


/*====================
  D3D_Reset
  ====================*/
void	D3D_Reset()
{
	// End the current scene if we have one
	D3D_EndFrame();

	D3D_Release();

	D3D_SetupPresentParams(g_d3dPP);

	if (g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICELOST)
		return;

	switch (g_pd3dDevice->Reset(&g_d3dPP))
	{
	case D3D_OK: // All is good
		break;
	default:
	case D3DERR_DEVICELOST: // Device is still lost, try again later
		g_bDeviceLost = true;
		return;
	case D3DERR_DRIVERINTERNALERROR: // Bad
		K2System.Error(_T("D3D_Reset: D3DERR_DRIVERINTERNALERROR"));
		break;
	case D3DERR_OUTOFVIDEOMEMORY: // Even badder
		K2System.Error(_T("D3D_Reset: D3DERR_OUTOFVIDEOMEMORY"));
		break;
	}

	if (g_bExclusive)
		Console.Video << "Using Direct3D exclusive mode" << newl;
	else
		Console.Video << "Using Direct3D non-exclusive mode" << newl;

	Console.Video << "Restarting Direct3D" << newl;

	D3D_GetDeviceCaps();

	if (FAILED(g_pd3dDevice->GetRenderTarget(0, &g_pBackBuffer)))
		K2System.Error(_T("D3D_Reset: GetRenderTarget failed"));

	if (FAILED(g_pd3dDevice->GetDepthStencilSurface(&g_pDepthBuffer)))
		K2System.Error(_T("D3D_Reset: GetDepthStencilSurface failed"));

	g_DeviceCaps.bQuery = SUCCEEDED(g_pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &g_pQueueLimitQuery));

	g_Shadowmap.Initialize();
	g_FogofWar.Initialize(g_FogofWar.GetWidth(), g_FogofWar.GetHeight(), g_FogofWar.GetSize());
	g_SceneBuffer.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
	g_PostBuffer.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
	g_ReflectionMap.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
	D3D_InitVBs();
	
	// TODO: Recreate dynamic mesh buffers

	D3D_SetDefaultStates();

	g_bDeviceLost = false;
	g_bValidScene = false;

	// Restore custom cursor
	D3D_SetCursor(g_hCursor);

	g_iNumGuiQuads = 0;

	D3D_BeginFrame();
	D3D_SetColor(BLACK);
	D3D_Clear();
	D3D_EndFrame();

	Console.Video << "Direct3D restart successful" << newl;
}


/*====================
  D3D_Create

  TODO: Better error messages on CreateDevice failures
  ====================*/
void	D3D_Create()
{
//#define PERFHUD
	uint uiAdapter(D3DADAPTER_DEFAULT);
	D3DDEVTYPE eDevType(d3d_software ? D3DDEVTYPE_REF : D3DDEVTYPE_HAL);
	bool bFoundPerfHud(false);

#ifdef PERFHUD
	uint uiNumAdapters(g_pD3D->GetAdapterCount());

	for (uint ui(0); ui < uiNumAdapters; ++ui)
	{
		D3DADAPTER_IDENTIFIER9 d3dAdapter;
		g_pD3D->GetAdapterIdentifier(ui, 0, &d3dAdapter);

		if (strstr(d3dAdapter.Description, "PerfHUD") != 0)
		{
			uiAdapter = ui;
			eDevType = D3DDEVTYPE_REF;
			bFoundPerfHud = true;
			break;
		}
	}
#endif

	D3DCAPS9 d3dCaps;
	g_pD3D->GetDeviceCaps(uiAdapter, eDevType, &d3dCaps);

	bool bPure((d3dCaps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0 && d3d_pure && !bFoundPerfHud);
	bool bHardwareTL((d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 && d3d_hardwareTL);
	bool bHardwareRaster((d3dCaps.DevCaps & D3DDEVCAPS_HWRASTERIZATION) != 0 && d3d_hardwareRaster);

	D3D_SetupPresentParams(g_d3dPP);

	if (g_bExclusive)
		Console.Video << "Using Direct3D exclusive mode" << newl;
	else
		Console.Video << "Using Direct3D non-exclusive mode" << newl;

	if (bPure && bHardwareTL)
	{
		Console.Video << "Creating pure hardware transform and lighting device" << newl;

		if (FAILED(g_pD3D->CreateDevice(uiAdapter, eDevType, g_hWnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE, &g_d3dPP, &g_pd3dDevice)))
			K2System.Error(_T("D3D_Create: CreateDevice() failed"));
	}
	else if (bHardwareTL)
	{
		Console.Video << "Creating hardware transform and lighting device" << newl;

		if (FAILED(g_pD3D->CreateDevice(uiAdapter, eDevType, g_hWnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dPP, &g_pd3dDevice)))
			K2System.Error(_T("D3D_Create: CreateDevice() failed"));
	}
	else if (bHardwareRaster)
	{
		Console.Video << "Creating software transform and lighting device" << newl;

		if (FAILED(g_pD3D->CreateDevice(uiAdapter, eDevType, g_hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_d3dPP, &g_pd3dDevice)))
			K2System.Error(_T("D3D_Create: CreateDevice() failed"));
	}
	else
	{
		Console.Video << "Creating software reference device" << newl;

		if (FAILED(g_pD3D->CreateDevice(uiAdapter, D3DDEVTYPE_REF, g_hWnd,
			0, &g_d3dPP, &g_pd3dDevice)))
			K2System.Error(_T("D3D_Create: CreateDevice() failed"));
	}

	D3D_GetDeviceCaps();

	g_uiInitialTextureMem = g_pd3dDevice->GetAvailableTextureMem();

	if (FAILED(g_pd3dDevice->GetRenderTarget(0, &g_pBackBuffer)))
		K2System.Error(_T("D3D_Create: GetRenderTarget failed"));

	if (FAILED(g_pd3dDevice->GetDepthStencilSurface(&g_pDepthBuffer)))
		K2System.Error(_T("D3D_Create: GetDepthStencilSurface failed"));
}


/*====================
  D3D_Start
  ====================*/
void	D3D_Start()
{
	static bool class_registered = false;
	//MSG msg;

	Console.Video << "Starting Direct3D" << newl;

	// Register window class
	if (!class_registered)
	{
		WNDCLASS wc;

		MemManager.Set(&wc, 0, sizeof(WNDCLASS));
		wc.lpfnWndProc = MainWndProc;
		wc.hInstance = g_hInstance;
		wc.lpszClassName = _T("K2_Direct3D");
		wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));

		if (!RegisterClass(&wc))
			K2System.Error(_T("D3D_Start: RegisterClass() failed"));

		class_registered = true;
	}

	DWORD dwWindowStyle(0);
	DWORD dwWindowStyleEx(WS_EX_ACCEPTFILES);

	if (!vid_fullscreen)
	{
		dwWindowStyle |= WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

		if (vid_windowResize)
			dwWindowStyle |= WS_SIZEBOX;
	}
	else
	{
		dwWindowStyle |= WS_SYSMENU;
	}

	// Create the window
	HWND hWnd = CreateWindowEx
	(
		dwWindowStyleEx,
		_T("K2_Direct3D"),
		K2System.GetGameName().c_str(),
		dwWindowStyle,
		0,
		0,
		g_CurrentVidMode.iWidth,
		g_CurrentVidMode.iHeight,
		NULL,
		NULL,
		g_hInstance,
		NULL
	);

	if (!hWnd)
		K2System.Error(_T("D3D_Start: CreateWindow() failed"));

	K2System.SetWindowHandle(hWnd);

	g_hWnd = hWnd;
	g_hDC = GetDC(g_hWnd);

	D3D_SetMode();

	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);

	//
	// Create D3D Device
	//

	D3D_Create();

	g_bD3DInitialized = true;

	// Initialize custom cursor
	D3D_SetCursor(g_hCursor);

	PatBlt(g_hDC, 0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, BLACKNESS);

	// Default rendering states
	D3D_SetDefaultStates();

	// Finish initializing D3D
	D3D_InitScene();
	D3D_InitVBs();
	D3D_InitVBsManaged();

	CTreeModelDef::Init();
	
	g_DeviceCaps.bQuery = SUCCEEDED(g_pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &g_pQueueLimitQuery)); 

	D3D_Setup2D();

	Console.Video << "Direct3D initialization successful" << newl;
}


/*====================
  D3D_SetMode
  ====================*/
int		D3D_SetMode()
{
	int iMode(-1);

	if (vid_resolution.GetSize() != 2)
		vid_resolution.Resize(2, 0);

	// Try to match a valid mode
	for (int i(0); i < g_iNumVidModes; ++i)
	{
		if (g_VidModes[i].iWidth == vid_resolution[0] &&
			g_VidModes[i].iHeight == vid_resolution[1] &&
			g_VidModes[i].iBpp == vid_bpp &&
			g_VidModes[i].iRefreshRate == vid_refreshRate)
		{
			g_CurrentVidMode = g_VidModes[i];
			iMode = i;
			break;
		}
	}

	if (iMode == -1)
	{
		if (vid_fullscreen)
		{
			g_CurrentVidMode = g_VidModes[0];
			iMode = 0;
		}
		else
		{
			g_CurrentVidMode.iWidth = vid_resolution[0];
			g_CurrentVidMode.iHeight = vid_resolution[1];
			g_CurrentVidMode.iBpp = vid_bpp;
			g_CurrentVidMode.iRefreshRate = vid_refreshRate;
		}
	}
	
	g_iCurrentVideoMode = iMode;

	if (vid_antialiasing.GetSize() != 2)
		vid_antialiasing.Resize(2, 0);

	g_CurrentAAMode.iSamples = vid_antialiasing[0];
	g_CurrentAAMode.iQuality = vid_antialiasing[1];
	g_CurrentAAMode.sName = _T("");

	//bool bWasExclusive(g_bExclusive);

	if (vid_fullscreen)
	{
		// Shrink our window so we fit on any resized screen
		if (g_hWnd != NULL)
		{
			//if (!SetWindowLong(g_hWnd, GWL_STYLE, 0))
			//	K2System.Error(_TS("SetWindowLong(GWL_STYLE) - ") + K2System.GetLastErrorString());
			//if (!SetWindowLong(g_hWnd, GWL_EXSTYLE, 0))
			//	K2System.Error(_TS("SetWindowLong(GWL_EXSTYLE) - ") + K2System.GetLastErrorString());
			if (!SetWindowPos(g_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOREDRAW))
				K2System.Error(_TS("SetWindowPos() - ") + K2System.GetLastErrorString());
		}

		if (!d3d_exclusive)
		{
			if (iMode != 0 || win_changeDisplayMode)
			{
				DEVMODE devmode;
				MemManager.Set(&devmode, 0, sizeof(devmode));
				devmode.dmPelsWidth = g_CurrentVidMode.iWidth;
				devmode.dmPelsHeight = g_CurrentVidMode.iHeight;
				devmode.dmBitsPerPel = g_CurrentVidMode.iBpp;
				devmode.dmDisplayFrequency = g_CurrentVidMode.iRefreshRate;
				devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

				devmode.dmSize = sizeof(devmode);

				bool bSuccess(false);

				bSuccess = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;

				if (!bSuccess && iMode != 0)
				{
					if (win_changeDisplayMode)
					{
						DEVMODE devmode0;
						MemManager.Set(&devmode0, 0, sizeof(devmode0));
						devmode0.dmPelsWidth = g_VidModes[0].iWidth;
						devmode0.dmPelsHeight = g_VidModes[0].iHeight;
						devmode0.dmBitsPerPel = g_VidModes[0].iBpp;
						devmode0.dmDisplayFrequency = g_VidModes[0].iRefreshRate;
						devmode0.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

						bSuccess = ChangeDisplaySettings(&devmode0, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;

						devmode = devmode0;
					}
					else
					{
						bSuccess = ChangeDisplaySettings(NULL, 0) == DISP_CHANGE_SUCCESSFUL;
					}

					g_iCurrentVideoMode = 0;
				}

				if (!bSuccess)
				{
					K2System.Error(_T("D3D_SetMode: Unable to set Mode 0"));
				}
			}
			else
			{
				if (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL)
					K2System.Error(_T("D3D_SetMode: Unable to set desktop video mode"));
			}
		}
		else
		{
			if (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL)
				K2System.Error(_T("D3D_SetMode: Unable to set desktop video mode"));
		}

		g_bFullscreen = true;
	}
	else
	{
		g_bFullscreen = false;

		if (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL)
			ChangeDisplaySettings(NULL, 0);
	}

	if (g_hWnd != NULL && !vid_fullscreen)
	{
		DWORD dwWindowStyle(WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU);

		if (vid_windowResize)
			dwWindowStyle |= WS_SIZEBOX;

		if (!SetWindowLong(g_hWnd, GWL_STYLE, dwWindowStyle))
			K2System.Error(_TS("SetWindowLong() - ") + K2System.GetLastErrorString());
		if (!SetWindowLong(g_hWnd, GWL_EXSTYLE, WS_EX_ACCEPTFILES))
			K2System.Error(_TS("SetWindowLong() - ") + K2System.GetLastErrorString());

		RECT winsize;
		winsize.left = 0;
		winsize.right = g_CurrentVidMode.iWidth;
		winsize.top = 0;
		winsize.bottom = g_CurrentVidMode.iHeight;
		AdjustWindowRect(&winsize, dwWindowStyle, false);
		if (!SetWindowPos(g_hWnd, HWND_NOTOPMOST, 0, 0, winsize.right - winsize.left, winsize.bottom - winsize.top, SWP_NOCOPYBITS | SWP_FRAMECHANGED))
			K2System.Error(_TS("SetWindowPos() - ") + K2System.GetLastErrorString());
		GetClientRect(g_hWnd, &winsize);

		// Resize render surface to match new window size (if it was clipped)
		g_CurrentVidMode.iWidth = MIN<int>(g_CurrentVidMode.iWidth, winsize.right - winsize.left);
		g_CurrentVidMode.iHeight = MIN<int>(g_CurrentVidMode.iHeight, winsize.bottom - winsize.top);
	}

	if (g_bD3DInitialized)
	{
		D3D_Reset();
	}

	if (g_hWnd != NULL)
	{
		if (g_bFullscreen)
		{
			if (!SetWindowLong(g_hWnd, GWL_STYLE, WS_SYSMENU))
				K2System.Error(_TS("SetWindowLong() - ") + K2System.GetLastErrorString());
			if (!SetWindowLong(g_hWnd, GWL_EXSTYLE, WS_EX_ACCEPTFILES))
				K2System.Error(_TS("SetWindowLong() - ") + K2System.GetLastErrorString());

			if (g_bExclusive)
			{
				if (!SetWindowPos(g_hWnd, HWND_TOP, 0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, SWP_NOCOPYBITS | SWP_FRAMECHANGED))
					K2System.Error(_TS("SetWindowPos() - ") + K2System.GetLastErrorString());
			}
			else
			{
				if (!SetWindowPos(g_hWnd, HWND_NOTOPMOST, 0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, SWP_NOCOPYBITS | SWP_FRAMECHANGED))
					K2System.Error(_TS("SetWindowPos() - ") + K2System.GetLastErrorString());
			}
		}

		ShowWindow(g_hWnd, SW_SHOW);
		
		PatBlt(g_hDC, 0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, BLACKNESS);
	}

	return g_iCurrentVideoMode;
}


/*====================
  D3D_GetMode
  ====================*/
bool	D3D_GetMode(int iMode, SVidMode *pVidMode)
{
	if (!pVidMode || iMode < 0 || iMode >= g_iNumVidModes)
		return false;

	pVidMode->iWidth = g_VidModes[iMode].iWidth;
	pVidMode->iHeight = g_VidModes[iMode].iHeight;
	pVidMode->iBpp = g_VidModes[iMode].iBpp;
	pVidMode->iRefreshRate = g_VidModes[iMode].iRefreshRate;
	pVidMode->sName = g_VidModes[iMode].sName;
	return true;
}


/*====================
  D3D_GetCurrentMode
  ====================*/
bool	D3D_GetCurrentMode(SVidMode *pVidMode)
{
	pVidMode->iWidth = g_CurrentVidMode.iWidth;
	pVidMode->iHeight = g_CurrentVidMode.iHeight;
	pVidMode->iBpp = g_CurrentVidMode.iBpp;
	pVidMode->iRefreshRate = g_CurrentVidMode.iRefreshRate;
	pVidMode->sName = g_CurrentVidMode.sName;
	return true;
}


/*====================
  D3D_IsFullScreen
  ====================*/
bool	D3D_IsFullScreen()
{
	return g_bFullscreen;
}


/*====================
  D3D_GetAAMode
  ====================*/
bool	D3D_GetAAMode(int iMode, SAAMode *pAAMode)
{
	if (!pAAMode || iMode < 0 || iMode >= g_iNumAAModes)
		return false;

	pAAMode->iSamples = g_AAModes[iMode].iSamples;
	pAAMode->iQuality = g_AAModes[iMode].iQuality;
	pAAMode->sName = g_AAModes[iMode].sName;
	return true;
}


/*====================
  D3D_GetCurrentAAMode
  ====================*/
bool	D3D_GetCurrentAAMode(SAAMode *pAAMode)
{
	pAAMode->iSamples = g_CurrentAAMode.iSamples;
	pAAMode->iQuality = g_CurrentAAMode.iQuality;
	pAAMode->sName = g_CurrentAAMode.sName;
	return true;
}


/*====================
  D3D_Release

  Release any resources that need to be released before a device Reset
  This is primarily any resource created with D3DPOOL_DEFAULT
  ====================*/
void	D3D_Release()
{
	SAFE_RELEASE(g_pVBGuiQuad);
	SAFE_RELEASE(g_pVBFowQuad);
	SAFE_RELEASE(g_pVBScenePoly);
	SAFE_RELEASE(g_pVBEffectQuad);
	SAFE_RELEASE(g_pVBBox);
	SAFE_RELEASE(g_pVBPoint);
	SAFE_RELEASE(g_pVBLine);
	SAFE_RELEASE(g_pVBSkybox);
	SAFE_RELEASE(g_pVBPostBuffer);
	SAFE_RELEASE(g_pVBEffectTriangle);
	SAFE_RELEASE(g_pVBExtendedTriangle);
	SAFE_RELEASE(g_pVBTreeBillboard);
	SAFE_RELEASE(g_pBackBuffer);
	SAFE_RELEASE(g_pDepthBuffer);
	SAFE_RELEASE(g_pQueueLimitQuery);

	for (int n = 0; n < MAX_MESHES; ++n)
	{
		SAFE_RELEASE(g_pVBDynamicMeshes[n]);
		SAFE_RELEASE(g_pVBDynamicMeshNormals[n]);
	}

	g_Shadowmap.Release();
	g_FogofWar.Release();
	g_SceneBuffer.Release();
	g_PostBuffer.Release();
	g_ReflectionMap.Release();
}


/*====================
  D3D_Destroy

  Destory all D3D resources
  ====================*/
void	D3D_Destroy()
{
	SAFE_RELEASE(g_pVBGuiQuad);
	SAFE_RELEASE(g_pIBGuiQuad);
	SAFE_RELEASE(g_pVBFowQuad);
	SAFE_RELEASE(g_pIBFowQuad);
	SAFE_RELEASE(g_pVBScenePoly);
	SAFE_RELEASE(g_pVBEffectQuad);
	SAFE_RELEASE(g_pIBEffectQuad);
	SAFE_RELEASE(g_pVBBox);
	SAFE_RELEASE(g_pIBBox);
	SAFE_RELEASE(g_pVBPoint);
	SAFE_RELEASE(g_pVBLine);
	SAFE_RELEASE(g_pVBSkybox);
	SAFE_RELEASE(g_pVBPostBuffer);
	SAFE_RELEASE(g_pVBEffectTriangle);
	SAFE_RELEASE(g_pVBExtendedTriangle);
	SAFE_RELEASE(g_pVBTreeBillboard);
	SAFE_RELEASE(g_pIBTreeBillboard);
	SAFE_RELEASE(g_pBackBuffer);
	SAFE_RELEASE(g_pDepthBuffer);
	SAFE_RELEASE(g_pQueueLimitQuery);

	for (int n = 0; n < MAX_MESHES; ++n)
	{
		SAFE_RELEASE(g_pIBMeshes[n]);
		SAFE_RELEASE(g_pVBDynamicMeshes[n]);
		SAFE_RELEASE(g_pVBStaticMeshes[n]);
		SAFE_RELEASE(g_pVBDynamicMeshNormals[n]);
		SAFE_RELEASE(g_pVBStaticMeshNormals[n]);
	}

	g_pTreeSceneManager->Destroy();

	D3D_DestroyShader();
	
	g_Shadowmap.Release();
	g_FogofWar.Release();
	g_SceneBuffer.Release();
	g_PostBuffer.Release();
	g_ReflectionMap.Release();

	SAFE_RELEASE(g_pd3dDevice);
}


/*====================
  D3D_Shutdown
  ====================*/
void	D3D_Shutdown()
{
	if (!g_bD3DInitialized)
		return;

	Console.Video << "Direct3D shutting down..." << newl;

	ShowWindow(g_hWnd, SW_HIDE);
	//UpdateWindow(g_hWnd);

	ChangeDisplaySettings(NULL, 0);
	DestroyWindow(g_hWnd);

	g_Gamma.RestoreGDIGammaRamp();

	D3D_CloseTextureArchive();

	D3D_Destroy();

	D3D_DestroyFoliage();
	D3D_DestroyTerrain();

	g_pTreeSceneManager->Shutdown();
	D3D_ShutdownShader();

	SAFE_RELEASE(g_pD3D);

	g_hWnd = NULL;

	UnregisterClass(_T("K2_Direct3D"), g_hInstance);

	g_bD3DInitialized = false;
	g_bValidScene = false;
}


/*====================
  D3D_BeginFrame
  ====================*/
void	D3D_BeginFrame()
{
	if (g_pd3dDevice == NULL || !g_bD3DInitialized)
		return;

	if (g_bValidScene) // Leave if we already have a valid scene
		return;

	if (g_bDeviceLost) // Try to restore a lost device
		D3D_Reset();

	if (g_bDeviceLost) // Leave if device is still lost
		return;
	
	if (g_bQueueLimitQuery && d3d_flush == 2)
	{
		D3D_WaitForQuery();
		g_bQueueLimitQuery = false;
	}

	g_Gamma.Update(vid_gamma);

	if (FAILED(g_pd3dDevice->BeginScene()))
	{
		Console.Warn << _T("D3D_BeginFrame: BeginScene failed") << newl;
		g_bValidScene = false;
	}

	g_bValidScene = true;
}


/*====================
  D3D_EndFrame
  ====================*/
void	D3D_EndFrame(void)
{
	if (!g_bValidScene || !g_bD3DInitialized)
		return;

	D3D_Setup2D();
	D3D_DrawQuads();

	g_pd3dDevice->EndScene();

	if (g_bQueueLimitQuery && d3d_flush == 1)
	{
		D3D_WaitForQuery();
		g_bQueueLimitQuery = false;
	}

	if (d3d_flush && g_DeviceCaps.bQuery)
	{
		g_pQueueLimitQuery->Issue(D3DISSUE_END);
		g_bQueueLimitQuery = true;
	}

	if (!g_pd3dDevice)
		return;

	// Present the back buffer
	switch (g_pd3dDevice->Present(NULL, NULL, NULL, NULL))
	{
	case D3D_OK: // Everything OK!
		break;
	case D3DERR_DEVICELOST: // Device is lost, try to restore
		g_bDeviceLost = true;
		break;
	case D3DERR_DRIVERINTERNALERROR: // Oh dear, bad stuff
		K2System.Error(_T("D3DERR_DRIVERINTERNALERROR"));
		break;
	case D3DERR_INVALIDCALL: // Oh dear, more bad stuff
		K2System.Error(_T("D3DERR_INVALIDCALL"));
		break;
	}

	g_bValidScene = false;

	if (g_bDeviceLost)
		D3D_Reset();

	g_lCustomMappings.clear();
}


/*====================
  D3D_SetColor
  ====================*/
void	D3D_SetColor(CVec4f v4Color)
{
	g_dwDrawColor = D3DCOLOR_ARGB(
		CLAMP(INT_ROUND(v4Color[A] * 255.0f), 0, 255),
		CLAMP(INT_ROUND(v4Color[R] * 255.0f), 0, 255),
		CLAMP(INT_ROUND(v4Color[G] * 255.0f), 0, 255),
		CLAMP(INT_ROUND(v4Color[B] * 255.0f), 0, 255)
	);
}


/*====================
  D3D_Notify
  ====================*/
void	D3D_Notify(EVidNotifyMessage eMsg, int iParam1, int iParam2, int iParam3, void *pData)
{
	if (!g_bD3DInitialized)
		return;

	switch(eMsg)
	{
	case VID_NOTIFY_NEW_WORLD:
		g_pWorld = static_cast<CWorld*>(pData);

		D3D_RebuildTerrain(terrain_chunkSize, static_cast<CWorld*>(pData));
		D3D_RebuildFoliage(foliage_chunkSize, static_cast<CWorld*>(pData));
		D3D_RebuildCliffs();
		break;

	case VID_NOTIFY_WORLD_DESTROYED:
		g_pWorld = NULL;

		D3D_DestroyTerrain();
		D3D_DestroyFoliage();
		break;

	case VID_NOTIFY_TERRAIN_COLOR_MODIFIED:
		if (iParam3)
			D3D_InvalidateTerrainLayer(TERRAIN_REBUILD_COLORS | TERRAIN_REBUILD_TEXCOORDS);
		else
			D3D_InvalidateTerrainVertex(iParam1, iParam2, TERRAIN_REBUILD_COLORS);
		break;

	case VID_NOTIFY_TERRAIN_NORMAL_MODIFIED:
		if (iParam3)
			D3D_InvalidateTerrainLayer(TERRAIN_REBUILD_NORMALS | TERRAIN_REBUILD_TEXCOORDS);
		else
			D3D_InvalidateTerrainVertex(iParam1, iParam2, TERRAIN_REBUILD_NORMALS);
		break;

	case VID_NOTIFY_TERRAIN_VERTEX_MODIFIED:
		if (iParam3)
			D3D_InvalidateTerrainLayer(TERRAIN_REBUILD_VERTICES);
		else
			D3D_InvalidateTerrainVertex(iParam1, iParam2, TERRAIN_REBUILD_VERTICES);
		break;

	case VID_NOTIFY_TERRAIN_SHADER_MODIFIED:
		if (iParam3)
			D3D_InvalidateTerrainLayer(TERRAIN_REBUILD_SHADERS);
		else
			D3D_InvalidateTerrainTile(iParam1, iParam2, TERRAIN_REBUILD_SHADERS);
		break;

	case VID_NOTIFY_TERRAIN_TEXCOORD_MODIFIED:
		if (iParam3)
			D3D_InvalidateTerrainLayer(TERRAIN_REBUILD_TEXCOORDS);
		else
			D3D_InvalidateTerrainVertex(iParam1, iParam2, TERRAIN_REBUILD_TEXCOORDS);
		break;

	case VID_NOTIFY_FOLIAGE_TEXTURE_MODIFIED:
		if (iParam3)
			D3D_InvalidateFoliageLayer(FOLIAGE_REBUILD_SHADERS);
		else
			D3D_InvalidateFoliageTile(iParam1, iParam2, FOLIAGE_REBUILD_SHADERS);
		break;

	case VID_NOTIFY_FOLIAGE_DENSITY_MODIFIED:
		if (iParam3)
			D3D_InvalidateFoliageLayer(FOLIAGE_REBUILD_VERTICES);
		else
			D3D_InvalidateFoliageVertex(iParam1, iParam2, FOLIAGE_REBUILD_VERTICES);
		break;

	case VID_NOTIFY_FOLIAGE_SIZE_MODIFIED:
		if (iParam3)
			D3D_InvalidateFoliageLayer(FOLIAGE_REBUILD_VERTICES);
		else
			D3D_InvalidateFoliageVertex(iParam1, iParam2, FOLIAGE_REBUILD_VERTICES);
		break;

	case VID_NOTIFY_TERRAIN_TEXEL_ALPHA_MODIFIED:
		if (iParam3)
			D3D_InvalidateTerrainLayer(TERRAIN_REBUILD_SHADERS);
		else
			D3D_InvalidateTerrainTexel(iParam1, iParam2, TERRAIN_REBUILD_ALPHAMAP);
		break;

	case VID_NOTIFY_UPDATE_SHADERS:
		D3D_FrameShader();
		break;

	case VID_NOTIFY_RELOAD_SHADER_CACHE:
		g_TextureCache.Reload();
		g_ShaderCache.Reload();
		g_ShaderRegistry.ReloadShaders();
		break;

	case VID_NOTIFY_REBUILD_CLIFFS:
		D3D_RebuildCliffs();
		break;

	case VID_NOTIFY_FOG_OF_WAR:
		if (g_FogofWar.GetWidth() != iParam1 || g_FogofWar.GetHeight() != iParam2 || g_FogofWar.GetSize() != iParam3)
		{
			g_FogofWar.Release();
			g_FogofWar.Initialize(iParam1, iParam2, iParam3);
		}
		break;
	}
}


/*====================
  D3D_GetFrameBuffer
  ====================*/
void	D3D_GetFrameBuffer(CBitmap &bmp)
{
	PROFILE("D3D_GetFrameBuffer");

	if (!g_bD3DInitialized)
		return;

	D3DDISPLAYMODE	displayMode;
	g_pd3dDevice->GetDisplayMode(0, &displayMode);

	IDirect3DSurface9	*pSurface;
	g_pd3dDevice->CreateOffscreenPlainSurface(displayMode.Width, displayMode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);

	g_pd3dDevice->GetFrontBufferData(0, pSurface);

	CRecti recWindowArea(K2System.GetWindowArea());

	RECT rect;
	rect.left = recWindowArea.left;
	rect.top = recWindowArea.top;
	rect.right = recWindowArea.right;
	rect.bottom = recWindowArea.bottom;

	D3DLOCKED_RECT d3dRect;
	pSurface->LockRect(&d3dRect, &rect, 0);

	bmp.Alloc(recWindowArea.GetWidth(), recWindowArea.GetHeight(), BITMAP_RGB);

	if (d3dRect.pBits)
	{
		byte *pDstData = static_cast<byte *>(bmp.GetBuffer());
		const byte *pSrcData = static_cast<const byte *>(d3dRect.pBits);
		int	iDeltaPitch = d3dRect.Pitch - recWindowArea.GetWidth() * 4;

		for (int y = 0; y < recWindowArea.GetHeight(); ++y)
		{
			for (int x = 0; x < recWindowArea.GetWidth(); ++x)
			{
				pDstData[0] = pSrcData[2];
				pDstData[1] = pSrcData[1];
				pDstData[2] = pSrcData[0];

				pDstData += 3;
				pSrcData += 4;
			}

			pSrcData += iDeltaPitch;
		}
	}

	pSurface->UnlockRect();

	SAFE_RELEASE(pSurface);
}


/*====================
  D3D_ProjectVertex
  ====================*/
CVec2f	D3D_ProjectVertex(const CCamera &cam, const CVec3f &vecVertex)
{
	PROFILE("D3D_ProjectVertex");

	if (!g_bD3DInitialized)
		return V2_ZERO;

	D3DXVECTOR3		out;
	D3DXVECTOR3		in;

	in[X] = vecVertex[X];
	in[Y] = vecVertex[Y];
	in[Z] = vecVertex[Z];

	D3DXVec3Project(&out, &in, &g_Viewport, &g_mProj, &g_mView, &g_mIdentity);

	return CVec2f(out[X], out[Y]);
}


/*====================
  D3D_GetHWnd
  ====================*/
void*	D3D_GetHWnd()
{
	return g_hWnd;
}


/*====================
  D3D_Clear
  ====================*/
void	D3D_Clear()
{
	if (!g_bD3DInitialized)
		return;

	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, g_dwDrawColor, 1.0f, 0);
}


/*====================
  D3D_GetCamera
  ====================*/
const CCamera*	D3D_GetCamera()
{
	return g_pCam;
}


/*====================
  D3D_ShowCursor
  ====================*/
void	D3D_ShowCursor(bool bShow)
{
	if (!g_bD3DInitialized)
		return;

	g_pd3dDevice->ShowCursor(bShow);
}


/*====================
  D3D_SetCursor
  ====================*/
void	D3D_SetCursor(ResHandle hCursor)
{
	g_hCursor = hCursor;

	if (!g_bD3DInitialized)
		return;

	if (g_hCursor == INVALID_RESOURCE)
	{
		g_pd3dDevice->ShowCursor(false);
		return;
	}

	CCursor *pCursor(g_ResourceManager.GetCursor(hCursor));
	if (pCursor == NULL)
	{
		g_pd3dDevice->ShowCursor(false);
		return;
	}

	CBitmap *pBitmap(pCursor->GetBitmapPointer());
	if (pBitmap == NULL)
	{
		g_pd3dDevice->ShowCursor(false);
		return;
	}

	IDirect3DSurface9 *pSurface(NULL);
	g_pd3dDevice->CreateOffscreenPlainSurface(pBitmap->GetWidth(), pBitmap->GetHeight(), D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
	
	D3DLOCKED_RECT d3dRect;
	pSurface->LockRect(&d3dRect, NULL, 0);

	if (d3dRect.pBits)
	{
		const byte *pSrcData(static_cast<const byte *>(pBitmap->GetBuffer()));
		byte *pDstData(static_cast<byte *>(d3dRect.pBits));
		int	iDeltaPitch(d3dRect.Pitch - pBitmap->GetWidth() * 4);

		// Invert
		pSrcData += pBitmap->GetWidth() * 4 * (pBitmap->GetHeight() - 1);
		iDeltaPitch -= pBitmap->GetWidth() * 4 * 2;

		for (int y(0); y < pBitmap->GetHeight(); ++y)
		{
			for (int x(0); x < pBitmap->GetWidth(); ++x)
			{
				pDstData[0] = pSrcData[2];
				pDstData[1] = pSrcData[1];
				pDstData[2] = pSrcData[0];
				pDstData[3] = pSrcData[3];

				pDstData += 4;
				pSrcData += 4;
			}

			pSrcData += iDeltaPitch;
		}
	}

	pSurface->UnlockRect();

	CVec2i v2Hotspot(pCursor->GetHotspot());

	g_pd3dDevice->SetCursorProperties(v2Hotspot.x, v2Hotspot.y, pSurface);
	g_pd3dDevice->ShowCursor(true);

	SAFE_RELEASE(pSurface);
}


/*====================
  D3D_TextureFilteringModeAvailable
  ====================*/
bool	D3D_TextureFilteringModeAvailable(ETextureFilteringModes eMode)
{
	if (g_auiMaxAniso[CLAMP<uint>(eMode, 0, NUM_TEXTUREFILTERING_MODES - 1)] > g_DeviceCaps.uiMaxAniso)
		return false;

	return true;
}


/*====================
  InitAPIs

  Sets up calls for core engine into vid code
  ====================*/
void	InitAPIs(SVidDriver *vid_api, WNDPROC _MainWndProc, HINSTANCE hInstance)
{
	g_hInstance = hInstance;
	MainWndProc = _MainWndProc;

	vid_api->sDriverName = _T("Direct3D 9.0c [HLSL]");

	vid_api->Init = D3D_Init;
	vid_api->Start = D3D_Start;
	vid_api->SetMode = D3D_SetMode;
	vid_api->GetMode = D3D_GetMode;
	vid_api->GetCurrentMode = D3D_GetCurrentMode;
	vid_api->IsFullScreen = D3D_IsFullScreen;
	vid_api->Shutdown = D3D_Shutdown;
	
	vid_api->TextureFilteringModeAvailable = D3D_TextureFilteringModeAvailable;

	vid_api->BeginFrame = D3D_BeginFrame;
	vid_api->EndFrame = D3D_EndFrame;
	vid_api->RenderScene = D3D_RenderScene;
	vid_api->Add2dRect = D3D_Add2dRect;
	vid_api->Add2dQuad = D3D_Add2dQuad;
	vid_api->Add2dLine = D3D_Add2dLine;
	vid_api->AddPoint = D3D_AddPoint;
	vid_api->AddLine = D3D_AddLine;
	vid_api->SetColor = D3D_SetColor;
	vid_api->Notify = D3D_Notify;
	vid_api->GetFrameBuffer = D3D_GetFrameBuffer;
	vid_api->ProjectVertex = D3D_ProjectVertex;
	vid_api->GetHWnd = D3D_GetHWnd;
	vid_api->Clear = D3D_Clear;

	vid_api->OpenTextureArchive = D3D_OpenTextureArchive;
	vid_api->CloseTextureArchive = D3D_CloseTextureArchive;
	vid_api->GetTextureList = D3D_GetTextureList;
	vid_api->TextureExists = D3D_TextureExists;
	vid_api->GetTextureColor = D3D_GetTextureColor;

	vid_api->RegisterTexture = D3D_RegisterTexture;
	vid_api->UnregisterTexture = D3D_UnregisterTexture;
	vid_api->RegisterVertexShader = D3D_RegisterVertexShader;
	vid_api->UnregisterVertexShader = D3D_UnregisterVertexShader;
	vid_api->RegisterPixelShader = D3D_RegisterPixelShader;
	vid_api->UnregisterPixelShader = D3D_UnregisterPixelShader;
	vid_api->RegisterShaderPair = D3D_RegisterShaderPair;
	vid_api->RegisterModel = D3D_RegisterModel;
	vid_api->UnregisterModel = D3D_UnregisterModel;

	vid_api->GetCamera = D3D_GetCamera;

	vid_api->RenderFogofWar = D3D_RenderFogofWar;
	vid_api->UpdateFogofWar = D3D_UpdateFogofWar;

	vid_api->ShowCursor = D3D_ShowCursor;
	vid_api->SetCursor = D3D_SetCursor;

	vid_api->GetAAMode = D3D_GetAAMode;
	vid_api->GetCurrentAAMode = D3D_GetCurrentAAMode;
}


/*--------------------
  cmdTextureMem
  --------------------*/
CMD(TextureMem)
{
	Console << _T("Available texture mem: ") << newl
		<< _T("Current: ") << GetByteString(g_pd3dDevice->GetAvailableTextureMem()) << newl
		<< _T("Initial: ") << GetByteString(g_uiInitialTextureMem) << newl
		<< _T("In use:  ") << GetByteString(g_uiInitialTextureMem - g_pd3dDevice->GetAvailableTextureMem()) << newl;
	return true;
}


/*--------------------
  cmdVidReset
  --------------------*/
CMD(VidReset)
{
	D3D_SetMode();

	return true;
}


/*--------------------
  ListVidModes
  --------------------*/
CMD(ListVidModes)
{
	for (int i(0); i < g_iNumVidModes; ++i)
		Console << XtoA(i, 2) << _T(": ") << g_VidModes[i].sName << newl;
	return true;
}


/*====================
  D3D_Alloc
  ====================*/
void*	D3D_Alloc(size_t z)
{
	static CHeap	s_heapVid("_vidd3d");
	return MemManager.Allocate(z, &s_heapVid);
}


#ifdef K2_DEBUG_MEM

/*====================
  operator new
  ====================*/
void*	operator new(size_t z)
{
	return D3D_Alloc(z);
}


/*====================
  operator new[]
  ====================*/
void*	operator new[](size_t z)
{
	return D3D_Alloc(z);
}

#endif