// (C)2005 S2 Games
// d3d9_scene.cpp
//
// Direct3D scene functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9_main.h"
#include "d3d9_scene.h"
#include "d3d9_state.h"
#include "d3d9_terrain.h"
#include "d3d9_model.h"
#include "d3d9_material.h"
#include "d3d9_shader.h"
#include "d3d9_texture.h"
#include "d3d9_util.h"
#include "d3d9_foliage.h"
#include "c_treemodeldef.h"
#include "c_treescenemanager.h"
#include "c_renderlist.h"
#include "c_shadowmap.h"
#include "c_fogofwar.h"
#include "c_scenebuffer.h"
#include "c_postbuffer.h"
#include "c_reflectionmap.h"
#include "c_velocitymap.h"

#include "../k2/c_boundingcone.h"
#include "../k2/c_boundingbox.h"
#include "../k2/c_convexhull.h"
#include "../k2/i_model.h"
#include "../k2/c_model.h"
#include "../k2/c_vid.h"
#include "../k2/c_camera.h"
#include "../k2/c_frustum.h"
#include "../k2/c_orthofrustum.h"
#include "../k2/c_material.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_world.h"
#include "../k2/c_particlesystem.h"
#include "../k2/i_emitter.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_treemodel.h"
#include "../k2/c_convexpolygon.h"
#include "../k2/c_convexpolygon2.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
enum EGuiQuadType
{
	GUI_RECT,
	GUI_QUAD,
	GUI_LINE
};

struct SGuiQuad
{
	EGuiQuadType	eType;
	
	float		x[4], y[4], w, h;
	float		s[4], t[4];
	dword		color;
    ResHandle	hTexture;
	int			iFlags;
};

SGuiQuad	g_GuiQuads[MAX_GUIQUADS];
int			g_iNumGuiQuads;

SBillboard	g_Billboards[MAX_BILLBOARDS];
int			g_iNumBillboards;

SBeam		g_Beams[MAX_BEAMS];
int			g_iNumBeams;

struct SBox
{
	CBBoxf			bbBox;
	CVec4f			v4Color;
	D3DXMATRIXA16	mWorld;
} g_Boxes[MAX_BOXES];
int		g_iNumBoxes;

struct SPoint
{
	CVec3f			v3Pos;
	CVec4f			v4Color;
} g_Points[MAX_POINTS];
int		g_iNumPoints;

struct SLine
{
	CVec3f			v3Start;
	CVec3f			v3End;
	CVec4f			v4Color;
} g_Lines[MAX_LINES];
int		g_iNumLines;

struct SEffectTriangle
{
	SEffectVertex	vert[3];
	ResHandle		hMaterial;
	int				iEffectLayer;
	float			fDepth;
} g_EffectTriangles[MAX_EFFECT_TRIANGLES];
int		g_iNumEffectTriangles;

struct SExtendedTriangle
{
	SExtendedVertex	vert[3];
	ResHandle		hMaterial;
} g_ExtendedTriangles[MAX_EXTENDED_TRIANGLES];
int		g_iNumExtendedTriangles;

struct STreeBillboards
{
	STreeBillboardVertex	vert[4];
	ResHandle		hMaterial;
	dword			dwAlphaTest;
} g_TreeBillboards[MAX_TREE_BILLBOARDS];
int		g_iNumTreeBillboards;

bool			g_bInvertedProjection = false;
const CSceneEntity	*g_pCurrentEntity;

ResHandle	g_hCloudTexture(INVALID_RESOURCE);

void	D3D_DrawBoxes(EMaterialPhase ePhase);

CONST_STRING(IMAGE, _T("image"));
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOL		(gfx_sky,						true);

CVAR_BOOLF		(gfx_foliage,					true,						CVAR_SAVECONFIG);

CVAR_BOOLF		(gfx_clouds,					false,						CVAR_WORLDCONFIG);
CVAR_STRINGF	(gfx_cloudTexture,				"/world/sky/cloud_shadows/cloud_shadows1.tga", CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_cloudScale,				200.0f,						CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_cloudSpeedX,				100.0f,						CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_cloudSpeedY,				50.0f,						CVAR_WORLDCONFIG);

CVAR_BOOLF		(vid_drawParticleSystems,		true,						CONEL_DEV);

CVAR_BOOLF		(gfx_depthFirst,				false,						CVAR_SAVECONFIG);
CVAR_INTF		(gfx_fogType,					0,							CVAR_WORLDCONFIG);

CVAR_BOOL		(gfx_infiniteFrustum,			false);

CVAR_BOOLF		(gfx_lighting,					true,						CONEL_DEV);
CVAR_INTF		(gfx_textures,					0,							CONEL_DEV);
CVAR_INTF		(gfx_wireframe,					0,							CONEL_DEV);
CVAR_INTF		(gfx_points,					0,							CONEL_DEV);
CVAR_INTF		(gfx_normals,					0,							CONEL_DEV);

CVAR_BOOLF		(vid_dynamicLights,				true,						CVAR_SAVECONFIG);
CVAR_INTR		(vid_maxDynamicLights,			4,							CVAR_SAVECONFIG,	0, 4);

CVAR_FLOAT		(vid_skyEpsilon,				0.001f);

CVAR_BOOL		(d3d_crossFrameCycleBuffer,		true);
//=============================================================================

/*====================
  D3D_Add2dRect
  ====================*/
void	D3D_Add2dRect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags)
{
	if (!g_bValidScene || g_iNumGuiQuads >= MAX_GUIQUADS)
		return;

	if (hTexture == INVALID_RESOURCE)
	{
		Console.Warn << _T("D3D_Add2dRect() - Invalid texture handle") << newl;
		return;
	}

	g_GuiQuads[g_iNumGuiQuads].eType = GUI_RECT;
	g_GuiQuads[g_iNumGuiQuads].x[0] = x;
	g_GuiQuads[g_iNumGuiQuads].y[0] = y;
	g_GuiQuads[g_iNumGuiQuads].w = w;
	g_GuiQuads[g_iNumGuiQuads].h = h;

	g_GuiQuads[g_iNumGuiQuads].s[0] = s1;
	g_GuiQuads[g_iNumGuiQuads].t[0] = t1;
	g_GuiQuads[g_iNumGuiQuads].s[1] = s2;
	g_GuiQuads[g_iNumGuiQuads].t[1] = t2;

	g_GuiQuads[g_iNumGuiQuads].color = g_dwDrawColor;
	g_GuiQuads[g_iNumGuiQuads].hTexture = hTexture;
	g_GuiQuads[g_iNumGuiQuads].iFlags = iFlags;

	++g_iNumGuiQuads;
}


/*====================
  D3D_Add2dQuad
  ====================*/
void	D3D_Add2dQuad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4,
					  const CVec2f& t1, const CVec2f& t2, const CVec2f& t3, const CVec2f& t4, ResHandle hTexture, int iFlags)
{
	if (!g_bValidScene || g_iNumGuiQuads >= MAX_GUIQUADS)
		return;

	if (hTexture == INVALID_RESOURCE)
	{
		Console.Warn << _T("D3D_Add2dQuad() - Invalid texture handle") << newl;
		return;
	}

	g_GuiQuads[g_iNumGuiQuads].eType = GUI_QUAD;
	g_GuiQuads[g_iNumGuiQuads].x[0] = v1.x;
	g_GuiQuads[g_iNumGuiQuads].y[0] = v1.y;
	g_GuiQuads[g_iNumGuiQuads].x[1] = v2.x;
	g_GuiQuads[g_iNumGuiQuads].y[1] = v2.y;
	g_GuiQuads[g_iNumGuiQuads].x[2] = v3.x;
	g_GuiQuads[g_iNumGuiQuads].y[2] = v3.y;
	g_GuiQuads[g_iNumGuiQuads].x[3] = v4.x;
	g_GuiQuads[g_iNumGuiQuads].y[3] = v4.y;
	g_GuiQuads[g_iNumGuiQuads].w = 0.0f;
	g_GuiQuads[g_iNumGuiQuads].h = 0.0f;

	g_GuiQuads[g_iNumGuiQuads].s[0] = t1.x;
	g_GuiQuads[g_iNumGuiQuads].t[0] = t1.y;
	g_GuiQuads[g_iNumGuiQuads].s[1] = t2.x;
	g_GuiQuads[g_iNumGuiQuads].t[1] = t2.y;
	g_GuiQuads[g_iNumGuiQuads].s[2] = t3.x;
	g_GuiQuads[g_iNumGuiQuads].t[2] = t3.y;
	g_GuiQuads[g_iNumGuiQuads].s[3] = t4.x;
	g_GuiQuads[g_iNumGuiQuads].t[3] = t4.y;

	g_GuiQuads[g_iNumGuiQuads].color = g_dwDrawColor;
	g_GuiQuads[g_iNumGuiQuads].hTexture = hTexture;
	g_GuiQuads[g_iNumGuiQuads].iFlags = iFlags;

	++g_iNumGuiQuads;
}


/*====================
  D3D_Add2dLine
  ====================*/
void	D3D_Add2dLine(const CVec2f& v1, const CVec2f& v2, const CVec4f &v4Color1, const CVec4f &v4Color2, int iFlags)
{
	if (!g_bValidScene || g_iNumGuiQuads >= MAX_GUIQUADS)
		return;

	g_GuiQuads[g_iNumGuiQuads].eType = GUI_LINE;
	g_GuiQuads[g_iNumGuiQuads].x[0] = v1.x;
	g_GuiQuads[g_iNumGuiQuads].y[0] = v1.y;
	g_GuiQuads[g_iNumGuiQuads].x[1] = v2.x;
	g_GuiQuads[g_iNumGuiQuads].y[1] = v2.y;
	g_GuiQuads[g_iNumGuiQuads].s[0] = v4Color1[0];
	g_GuiQuads[g_iNumGuiQuads].s[1] = v4Color1[1];
	g_GuiQuads[g_iNumGuiQuads].s[2] = v4Color1[2];
	g_GuiQuads[g_iNumGuiQuads].s[3] = v4Color1[3];
	g_GuiQuads[g_iNumGuiQuads].t[0] = v4Color2[0];
	g_GuiQuads[g_iNumGuiQuads].t[1] = v4Color2[1];
	g_GuiQuads[g_iNumGuiQuads].t[2] = v4Color2[2];
	g_GuiQuads[g_iNumGuiQuads].t[3] = v4Color2[3];
	g_GuiQuads[g_iNumGuiQuads].color = g_dwDrawColor;
	g_GuiQuads[g_iNumGuiQuads].iFlags = iFlags;

	++g_iNumGuiQuads;
}


/*====================
  D3D_SetupCamera
  ====================*/
void	D3D_SetupCamera(const CCamera &camera)
{
	PROFILE("D3D_SetupCamera");

	g_pCam = &camera;

	if (camera.HasFlags(CAM_ORTHO))
	{
		// Projection matrix
		D3DXMATRIXA16 mProj;

		D3DXMatrixOrthoOffCenterRH(&mProj,
			-camera.GetOrthoWidth() / 2.0f, camera.GetOrthoWidth() / 2.0f,
			-camera.GetOrthoHeight() / 2.0f, camera.GetOrthoHeight() / 2.0f,
			camera.GetZNear(), camera.GetZFar());
		g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);
		g_mProj = mProj;
	}
	else if (camera.HasFlags(CAM_INVERSEPROJECTION))
	{
		// Projection matrix
		float yScale = 1.0f / tan((DEG2RAD(camera.GetFovY()) / 2.0f));
		float xScale = yScale / camera.GetAspect();
		float zn = camera.GetZNear();

		// Normal projection matrix solved when zn = -zf

		D3DXMATRIXA16 mProj(xScale, 0.0f,    0.0f,     0.0f,
		                    0.0f,   yScale,  0.0f,     0.0f,
		                    0.0f,   0.0f,   -0.5f,    -1.0f,
		                    0.0f,   0.0f,   -0.5f*zn,  0.0f);

		g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);
		g_mProj = mProj;
	}
	else
	{
		// Projection matrix
		float yScale = 1.0f / tan((DEG2RAD(camera.GetFovY()) / 2.0f));
		float xScale = yScale / camera.GetAspect();
		float zn = camera.GetZNear();
		float zf = camera.GetZFar();

		if (gfx_infiniteFrustum || camera.HasFlags(CAM_INFINITE))
		{
			D3DXMATRIXA16 mProj(xScale, 0.0f,    0.0f,  0.0f,
			                    0.0f,   yScale,  0.0f,  0.0f,
			                    0.0f,   0.0f,   -1.0f, -1.0f,
			                    0.0f,   0.0f,   -zn,    0.0f);

			g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);
			g_mProj = mProj;
		}
		else if (camera.HasFlags(CAM_CUBEPROJECTION))
		{
			D3DXMATRIXA16 mProj(xScale, 0.0f,    0.0f,                0.0f,
			                    0.0f,   yScale,  0.0f,                0.0f,
			                    0.0f,   0.0f,   -(zf+zn)/(zf-zn),    -1.0f,
			                    0.0f,   0.0f,   -2.0f*zf*zn/(zf-zn),  0.0f);

			g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);
			g_mProj = mProj;
		}
		else
		{
			D3DXMATRIXA16 mProj(xScale, 0.0f,   0.0f,           0.0f,
			                    0.0f,   yScale, 0.0f,           0.0f,
			                    0.0f,   0.0f,   zf/(zn-zf),    -1.0f,
			                    0.0f,   0.0f,   zn*zf/(zn-zf),  0.0f);

			g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);
			g_mProj = mProj;
		}
	}

	// View matrix
	D3DXMATRIXA16 mView;
	D3DXMATRIXA16 mViewAxis;
	D3DXMATRIXA16 mViewTranslation;

	D3DXMATRIXA16 mWorldTranslation;

	D3DXMatrixIdentity(&mView);
	D3DXMatrixTranslation(&mViewTranslation, -camera.GetOrigin(X), -camera.GetOrigin(Y), -camera.GetOrigin(Z));
	D3DXMatrixTranslation(&mWorldTranslation, camera.GetOrigin(X), camera.GetOrigin(Y), camera.GetOrigin(Z));

	mViewAxis[0] = camera.GetViewAxis(RIGHT).x;
	mViewAxis[1] = camera.GetViewAxis(UP).x;
	mViewAxis[2] = -camera.GetViewAxis(FORWARD).x;
	mViewAxis[3] = 0.0f;

	mViewAxis[4] = camera.GetViewAxis(RIGHT).y;
	mViewAxis[5] = camera.GetViewAxis(UP).y;
	mViewAxis[6] = -camera.GetViewAxis(FORWARD).y;
	mViewAxis[7] = 0.0f;

	mViewAxis[8] = camera.GetViewAxis(RIGHT).z;
	mViewAxis[9] = camera.GetViewAxis(UP).z;
	mViewAxis[10] = -camera.GetViewAxis(FORWARD).z;
	mViewAxis[11] = 0.0f;

	mViewAxis[12] = 0.0f;
	mViewAxis[13] = 0.0f;
	mViewAxis[14] = 0.0f;
	mViewAxis[15] = 1.0f;

	D3DXMatrixMultiply(&mView, &mViewAxis, &mView);
	D3DXMatrixMultiply(&mView, &mViewTranslation, &mView);

	g_pd3dDevice->SetTransform(D3DTS_VIEW, &mView);
	g_mView = mView;
	g_mViewRotate = mViewAxis;
	g_mViewOffset = mViewTranslation;
	g_mWorldOffset = mWorldTranslation;
	g_mViewProj = g_mView * g_mProj;

	// World matrix
	g_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &g_mIdentity);
	g_mWorld = g_mIdentity;

	g_Viewport.X = INT_ROUND(camera.GetX());
	g_Viewport.Y = INT_ROUND(camera.GetY());
	g_Viewport.Width  = INT_ROUND(camera.GetWidth());
	g_Viewport.Height = INT_ROUND(camera.GetHeight());
	
	if (camera.GetFlags() & CAM_DEPTH_COMPRESS)
	{
		g_Viewport.MinZ = 0.0f;
		g_Viewport.MaxZ = 0.1f;
	}
	else
	{
		g_Viewport.MinZ = 0.0f;
		g_Viewport.MaxZ = 1.0f;
	}

	g_pd3dDevice->SetViewport(&g_Viewport);
}


/*====================
  D3D_SetupScene
  ====================*/
void	D3D_SetupScene()
{
	PROFILE("D3D_SetupScene");

	g_iScreenWidth = g_CurrentVidMode.iWidth;
	g_iScreenHeight = g_CurrentVidMode.iHeight;

	// Setup the default 3d rendering state
	D3D_SetRenderState(D3DRS_ZENABLE, TRUE);
	D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	D3D_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	D3D_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	D3D_SetRenderState(D3DRS_ALPHAREF, vid_alphaTestRef);
	D3D_SetRenderState(D3DRS_POINTSIZE, D3D_DWORD(4.0f));
	D3D_SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

	// Determine the number of dynamic lights
	switch (g_DeviceCaps.dwMaxPixelShaderVersion)
	{
	case D3DPS_VERSION(3, 0):
		g_iMaxDynamicLights = (vid_dynamicLights && vid_shaderLightingQuality == 0) ? MIN(MAX_POINT_LIGHTS, int(vid_maxDynamicLights)) : 0;
		break;
	case D3DPS_VERSION(2, 0):
		g_iMaxDynamicLights = 0;
		break;
	case D3DPS_VERSION(1, 4):
		g_iMaxDynamicLights = 0;
		break;
	case D3DPS_VERSION(1, 1):
		g_iMaxDynamicLights = 0;
		break;
	default:
		g_iMaxDynamicLights = 0;
		break;
	}

	g_bLighting = true;
	g_iNumActiveBones = 0;
	g_iNumActivePointLights = 0;
	g_pCurrentEntity = NULL;
	g_iTexcoords = 1;
	g_bTexkill = false;
}


/*====================
  D3D_Setup2D
  ====================*/
void	D3D_Setup2D()
{
	g_iScreenWidth = g_CurrentVidMode.iWidth;
	g_iScreenHeight = g_CurrentVidMode.iHeight;

	// Projection matrix
	D3DXMATRIXA16 mProj;
	D3DXMatrixOrthoOffCenterRH(&mProj, 0.5f, g_iScreenWidth + 0.5f, g_iScreenHeight + 0.5f, 0.5f, 0.0f, 1.0f);	
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);
	g_bInvertedProjection = false;

	// View matrix
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &g_mIdentity);
	g_mView = g_mIdentity;

	// World matrix
	g_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &g_mIdentity);
	g_mWorld = g_mIdentity;
	g_mWorldViewProj = mProj;

	D3D_SetRenderState(D3DRS_ZENABLE, FALSE);
	D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	D3D_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	D3D_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	D3D_SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

	g_Viewport.X = 0;
	g_Viewport.Y = 0;
	g_Viewport.Width  = g_iScreenWidth;
	g_Viewport.Height = g_iScreenHeight;
	g_Viewport.MinZ = 0.0f;
	g_Viewport.MaxZ = 1.0f;

	g_pd3dDevice->SetViewport(&g_Viewport);

	g_bLighting = false;
	g_iNumActiveBones = 0;
	g_bShadows = false;
	g_bFogofWar = false;
	g_bFog = false;
	g_iNumActivePointLights = 0;
	g_iTexcoords = 1;
	g_bTexkill = false;
}


/*====================
  D3D_CleanupScene
  ====================*/
void	D3D_CleanupScene()
{
	g_RenderList.Clear();

	g_iNumBillboards = 0;
	g_iNumBeams = 0;
	g_iNumEffectTriangles = 0;
	g_iNumExtendedTriangles = 0;
	g_iNumBoxes = 0;
	g_iNumPoints = 0;
	g_iNumLines = 0;

	//
	// Unbind
	//

	for (uint ui(0); ui < g_dwNumSamplers; ++ui)
		D3D_SetTexture(ui, NULL); 

	D3D_SetIndices(NULL);
	D3D_SetStreamSource(0, NULL, 0, 0);

	D3D_Setup2D();
}


/*====================
  D3D_InitScene
  ====================*/
void	D3D_InitScene(void)
{
	D3D_InitTexture();
	D3D_InitShader();

	g_Shadowmap.Initialize();
	g_SceneBuffer.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
	g_PostBuffer.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
	g_ReflectionMap.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
	g_VelocityMap.Initialize();
	
	D3D_InitModel();
}


/*====================
  D3D_AddEffectQuadBatches
  ====================*/
void	D3D_AddEffectQuadBatches(EMaterialPhase ePhase)
{
	PROFILE("D3D_AddBillboardBatches");

	int iNumEffectQuads(g_iNumBillboards + g_iNumBeams);

	if (iNumEffectQuads == 0)
		return;

	// Clamp number of billboards and beams to make room if we exceeded max
	if (iNumEffectQuads > MAX_EFFECT_QUADS)
	{
		g_iNumBillboards -= (iNumEffectQuads - MAX_EFFECT_QUADS);
		iNumEffectQuads = MAX_EFFECT_QUADS;
	}

	//
	// Build billboard vertex buffer
	//

	START_PROFILE("Build vertex buffer");

	SEffectVertex* pVertices;

	if (!d3d_crossFrameCycleBuffer || g_dwEffectQuadBase + iNumEffectQuads >= MAX_EFFECT_QUADS)
		g_dwEffectQuadBase = 0;

	if (FAILED(g_pVBEffectQuad->Lock(g_dwEffectQuadBase * 4 * sizeof(SEffectVertex), iNumEffectQuads * 4 * sizeof(SEffectVertex), (void**)&pVertices, g_dwEffectQuadBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
		return;

	CVec3f v3CamUp(g_pCam->GetViewAxis(UP));
	CVec3f v3CamRight(g_pCam->GetViewAxis(RIGHT));

	// Add billboards
	for (int n(0); n < g_iNumBillboards; ++n)
	{
		CVec3f v3Up(g_Billboards[n].uiFlags & BBOARD_LOCK_UP ? g_Billboards[n].aAxis.Up() : v3CamUp);
		CVec3f v3Right(g_Billboards[n].uiFlags & BBOARD_LOCK_RIGHT ? g_Billboards[n].aAxis.Right() : v3CamRight);

		if (g_Billboards[n].fYaw)
		{
			// Rotate v3Right around v3Up
			CVec3f p0(v3Right);
			CVec3f p1(CrossProduct(v3Up, p0));

			float fSin, fCos;
			M_SinCos(DEG2RAD(g_Billboards[n].fYaw), fSin, fCos);

			v3Right = p0 * fCos + p1 * fSin;

			if (g_Billboards[n].fPitch)
			{
				v3Right = M_RotatePointAroundAxis(v3Right, g_Billboards[n].uiFlags & BBOARD_LOCK_RIGHT ? g_Billboards[n].aAxis.Right() : v3CamRight, g_Billboards[n].fPitch);
				v3Up = M_RotatePointAroundAxis(v3Up, g_Billboards[n].uiFlags & BBOARD_LOCK_RIGHT ? g_Billboards[n].aAxis.Right() : v3CamRight, g_Billboards[n].fPitch);
			}
		}
		else if (g_Billboards[n].fPitch)
		{
			// Rotate v3Up around v3Right
			CVec3f p0(v3Up);
			CVec3f p1(CrossProduct(v3Right, p0));

			float fSin, fCos;
			M_SinCos(DEG2RAD(g_Billboards[n].fPitch), fSin, fCos);

			v3Up = p0 * fCos + p1 * fSin;
		}
		
		if (g_Billboards[n].angle)
		{
			// Rotate v3Right and v3Up around forward vector
			CVec3f v3R(v3Right);
			CVec3f v3U(v3Up);
			
			float fSin, fCos;
			M_SinCos(DEG2RAD(g_Billboards[n].angle), fSin, fCos);

			v3Right = v3R * fCos + v3U * fSin;
			v3Up = v3U * fCos - v3R * fSin;
		}
	
		CVec3f	v3Points[4];

		if (g_Billboards[n].fDepthBias)
		{
			CVec3f v3CamDirection(Normalize(g_Billboards[n].v3Pos - g_pCam->GetOrigin()));

			g_Billboards[n].v3Pos.ScaleAdd(v3CamDirection, g_Billboards[n].fDepthBias);
		}

		if (g_Billboards[n].uiFlags & BBOARD_FLARE)
		{
			v3Right *= g_Billboards[n].width * 0.5f;
			v3Up *= g_Billboards[n].height;

			v3Points[0] = g_Billboards[n].v3Pos; v3Points[0] += v3Right;
			v3Points[1] = g_Billboards[n].v3Pos; v3Points[1] += v3Right; v3Points[1] += v3Up; 
			v3Points[2] = g_Billboards[n].v3Pos; v3Points[2] -= v3Right; v3Points[2] += v3Up;
			v3Points[3] = g_Billboards[n].v3Pos; v3Points[3] -= v3Right;
		}
		else if (g_Billboards[n].uiFlags & BBOARD_OFFCENTER)
		{
			CVec2f v2Center(g_Billboards[n].v2Center);
			CVec3f av3Offsets[4] =
			{
				v3Right * (g_Billboards[n].width * (1.0f - v2Center.x)),
				v3Up * (g_Billboards[n].height * (1.0f - v2Center.y)),
				v3Right * (g_Billboards[n].width * -v2Center.x),
				v3Up * (g_Billboards[n].height * -v2Center.y)
			};

			v3Points[0] = g_Billboards[n].v3Pos; v3Points[0] += av3Offsets[0]; v3Points[0] += av3Offsets[3];
			v3Points[1] = g_Billboards[n].v3Pos; v3Points[1] += av3Offsets[0]; v3Points[1] += av3Offsets[1];
			v3Points[2] = g_Billboards[n].v3Pos; v3Points[2] += av3Offsets[2]; v3Points[2] += av3Offsets[1];
			v3Points[3] = g_Billboards[n].v3Pos; v3Points[3] += av3Offsets[2]; v3Points[3] += av3Offsets[3];
		}
		else
		{
			v3Right *= g_Billboards[n].width * 0.5f;
			v3Up *= g_Billboards[n].height * 0.5f;

			v3Points[0] = g_Billboards[n].v3Pos; v3Points[0] += v3Right; v3Points[0] -= v3Up;
			v3Points[1] = g_Billboards[n].v3Pos; v3Points[1] += v3Right; v3Points[1] += v3Up; 
			v3Points[2] = g_Billboards[n].v3Pos; v3Points[2] -= v3Right; v3Points[2] += v3Up;
			v3Points[3] = g_Billboards[n].v3Pos; v3Points[3] -= v3Right; v3Points[3] -= v3Up;
		}

		dword dwColor(g_Billboards[n].color.GetAsDWord());

		pVertices->v = v3Points[0];
		pVertices->color = dwColor;
		pVertices->t[0] = g_Billboards[n].s2;
		pVertices->t[1] = g_Billboards[n].t1;
		pVertices->t[2] = g_Billboards[n].frame;
		pVertices->t[3] = g_Billboards[n].param;
		++pVertices;

		pVertices->v = v3Points[1];
		pVertices->color = dwColor;
		pVertices->t[0] = g_Billboards[n].s2;
		pVertices->t[1] = g_Billboards[n].t2;
		pVertices->t[2] = g_Billboards[n].frame;
		pVertices->t[3] = g_Billboards[n].param;
		++pVertices;

		pVertices->v = v3Points[2];
		pVertices->color = dwColor;
		pVertices->t[0] = g_Billboards[n].s1;
		pVertices->t[1] = g_Billboards[n].t2;
		pVertices->t[2] = g_Billboards[n].frame;
		pVertices->t[3] = g_Billboards[n].param;
		++pVertices;

		pVertices->v = v3Points[3];
		pVertices->color = dwColor;
		pVertices->t[0] = g_Billboards[n].s1;
		pVertices->t[1] = g_Billboards[n].t1;
		pVertices->t[2] = g_Billboards[n].frame;
		pVertices->t[3] = g_Billboards[n].param;
		++pVertices;
	}

	// Add beams
	for (int n = 0; n < g_iNumBeams; ++n)
	{
		float fTex0(0.0f);
		float fTex1(g_Beams[n].fTile);

		CVec2f v2Dir(Normalize(g_pCam->WorldToView(g_Beams[n].v3Start) - g_pCam->WorldToView(g_Beams[n].v3End)));

		CVec3f v3Dir(v3CamRight * v2Dir.x + v3CamUp * v2Dir.y);
		
		CVec3f v3Width(CrossProduct(v3Dir, g_pCam->GetViewAxis(FORWARD)));

		dword dwStartColor(g_Beams[n].v4StartColor.GetAsDWord());
		dword dwEndColor(g_Beams[n].v4EndColor.GetAsDWord());

		pVertices->v = g_Beams[n].v3Start + v3Width * (g_Beams[n].fStartSize / 2.0f);
		pVertices->color = dwStartColor;
		pVertices->t = CVec4f(fTex0, 0.0f, g_Beams[n].fStartFrame, g_Beams[n].fStartParam);
		++pVertices;

		pVertices->v = g_Beams[n].v3End + v3Width * (g_Beams[n].fEndSize / 2.0f);
		pVertices->color = dwEndColor;
		pVertices->t = CVec4f(fTex1, 0.0f, g_Beams[n].fEndFrame, g_Beams[n].fEndParam);
		++pVertices;

		pVertices->v = g_Beams[n].v3End - v3Width * (g_Beams[n].fEndSize / 2.0f);
		pVertices->color = dwEndColor;
		pVertices->t = CVec4f(fTex1, 1.0f, g_Beams[n].fEndFrame, g_Beams[n].fEndParam);
		++pVertices;

		pVertices->v = g_Beams[n].v3Start - v3Width * (g_Beams[n].fStartSize / 2.0f);
		pVertices->color = dwStartColor;
		pVertices->t = CVec4f(fTex0, 1.0f, g_Beams[n].fStartFrame, g_Beams[n].fStartParam);
		++pVertices;
	}

	g_pVBEffectQuad->Unlock();
	
	END_PROFILE

	//
	// Add billboard batches to the renderlist
	//

	for (int n(0); n < g_iNumBillboards; ++n)
	{
		ResHandle hMaterial(g_Billboards[n].hMaterial);

		CMaterial *pMaterial(&D3D_GetMaterial(hMaterial));

		if (pMaterial == NULL)
			continue;

		if (!pMaterial->HasPhase(ePhase))
			continue;

		int iMaxEffectLayer(g_Billboards[n].iEffectLayer);

		int f;
		for (f = n + 1; f < g_iNumBillboards && g_Billboards[f].hMaterial == hMaterial && g_Billboards[f].iEffectLayer >= iMaxEffectLayer; ++f)
			iMaxEffectLayer = g_Billboards[f].iEffectLayer;

		g_RenderList.Add(K2_NEW(ctx_D3D9,   CEffectQuadRenderer)(hMaterial, g_dwEffectQuadBase + n, g_dwEffectQuadBase + f, (iMaxEffectLayer << 20) + n, -FAR_AWAY));

		n += (f - n) - 1;
	}

	int iBufferOffset(g_iNumBillboards);

	//
	// Add beam batches to the renderlist
	//

	for (int n = 0; n < g_iNumBeams; ++n)
	{
		ResHandle hMaterial = g_Beams[n].hMaterial;

		if (!D3D_GetMaterial(hMaterial).HasPhase(ePhase))
			continue;

		int iMaxEffectLayer(g_Beams[n].iEffectLayer);

		int f;
		for (f = n + 1; f < g_iNumBeams && g_Beams[f].hMaterial == hMaterial && g_Beams[f].iEffectLayer >= iMaxEffectLayer; ++f)
			iMaxEffectLayer = g_Beams[f].iEffectLayer;

		g_RenderList.Add(K2_NEW(ctx_D3D9,   CEffectQuadRenderer)(hMaterial, g_dwEffectQuadBase + iBufferOffset + n, g_dwEffectQuadBase + iBufferOffset + f, (iMaxEffectLayer << 20) + n, -FAR_AWAY));

		n += (f - n) - 1;
	}

	g_dwEffectQuadBase += iNumEffectQuads;
}


/*====================
  D3D_AddBillboard
  ====================*/
void	D3D_AddBillboard
(
	const CVec3f &v3Pos,
	float width,
	float height,
	float angle,
	float s1,
	float t1,
	float s2,
	float t2,
	float frame,
	float param,
	ResHandle hMaterial,
	uint uiFlags,
	float fPitch,
	float fYaw,
	float fDepthBias,
	int iEffectLayer,
	float fDepth,
	const CAxis &aAxis,
	const CVec4f &v4Color
)
{
	if (!g_bValidScene || g_iNumBillboards >= MAX_BILLBOARDS)
		return;

	int N(g_iNumBillboards);

	g_Billboards[N].v3Pos = v3Pos;
	g_Billboards[N].width = width;
	g_Billboards[N].height = height;
	g_Billboards[N].angle = angle;
	g_Billboards[N].s1 = s1;
	g_Billboards[N].t1 = t1;
	g_Billboards[N].s2 = s2;
	g_Billboards[N].t2 = t2;
	g_Billboards[N].frame = frame;
	g_Billboards[N].param = param;
	g_Billboards[N].color = v4Color;
	g_Billboards[N].hMaterial = hMaterial;
	g_Billboards[N].uiFlags = uiFlags;
	g_Billboards[N].fPitch = fPitch;
	g_Billboards[N].fYaw = fYaw;
	g_Billboards[N].fDepthBias = fDepthBias;
	g_Billboards[N].iEffectLayer = iEffectLayer;
	g_Billboards[N].fDepth = fDepth;
	g_Billboards[N].aAxis = aAxis;

	if (!g_Billboards[N].s1 && !g_Billboards[N].t1 && !g_Billboards[N].s2 && !g_Billboards[N].t2)
	{
		g_Billboards[N].s2 = 1.0f;
		g_Billboards[N].t2 = 1.0f;
	}

	++g_iNumBillboards;
}


/*====================
  D3D_AddBeam
  ====================*/
void	D3D_AddBeam(const CVec3f &v3Start, const CVec3f &v3End, float fSize, float fTile, float fTaper, ResHandle hMaterial, int iEffectLayer)
{
	if (!g_bValidScene || g_iNumBeams >= MAX_BEAMS)
		return;

	int N(g_iNumBeams);

	g_Beams[N].v3Start = v3Start;
	g_Beams[N].v3End = v3End;
	g_Beams[N].fStartSize = g_Beams[N].fEndSize = fSize;
	g_Beams[N].fTile = fTile;
	g_Beams[N].fTaper = fTaper;
	g_Beams[N].v4StartColor = g_Beams[N].v4EndColor = CVec4f(g_dwDrawColor);
	g_Beams[N].fStartFrame = g_Beams[N].fEndFrame = 0.0f;
	g_Beams[N].fStartParam = g_Beams[N].fEndParam = 0.0f;
	g_Beams[N].hMaterial = hMaterial;
	g_Beams[N].iEffectLayer = iEffectLayer;
	++g_iNumBeams;
}


/*====================
  D3D_AddBoxBatches
  ====================*/
void	D3D_AddBoxBatches()
{
	PROFILE("D3D_AddBoxBatches");

	if (g_iNumBoxes == 0)
		return;

	SLineVertex* pVertices;

	if (FAILED(g_pVBBox->Lock(0, g_iNumBoxes * 8 * sizeof(SLineVertex), (void**)&pVertices, D3DLOCK_DISCARD)))
		return;

	vector<CVec3f> vPoints(8);

	for (int n = 0; n < g_iNumBoxes; ++n)
	{
		vPoints[0].x = g_Boxes[n].bbBox.GetMin()[0];
		vPoints[0].y = g_Boxes[n].bbBox.GetMin()[1];
		vPoints[0].z = g_Boxes[n].bbBox.GetMax()[2];

		vPoints[1].x = g_Boxes[n].bbBox.GetMin()[0];
		vPoints[1].y = g_Boxes[n].bbBox.GetMax()[1];
		vPoints[1].z = g_Boxes[n].bbBox.GetMax()[2];

		vPoints[2].x = g_Boxes[n].bbBox.GetMax()[0];
		vPoints[2].y = g_Boxes[n].bbBox.GetMax()[1];
		vPoints[2].z = g_Boxes[n].bbBox.GetMax()[2];

		vPoints[3].x = g_Boxes[n].bbBox.GetMax()[0];
		vPoints[3].y = g_Boxes[n].bbBox.GetMin()[1];
		vPoints[3].z = g_Boxes[n].bbBox.GetMax()[2];

		vPoints[4].x = g_Boxes[n].bbBox.GetMin()[0];
		vPoints[4].y = g_Boxes[n].bbBox.GetMin()[1];
		vPoints[4].z = g_Boxes[n].bbBox.GetMin()[2];

		vPoints[5].x = g_Boxes[n].bbBox.GetMin()[0];
		vPoints[5].y = g_Boxes[n].bbBox.GetMax()[1];
		vPoints[5].z = g_Boxes[n].bbBox.GetMin()[2];

		vPoints[6].x = g_Boxes[n].bbBox.GetMax()[0];
		vPoints[6].y = g_Boxes[n].bbBox.GetMax()[1];
		vPoints[6].z = g_Boxes[n].bbBox.GetMin()[2];

		vPoints[7].x = g_Boxes[n].bbBox.GetMax()[0];
		vPoints[7].y = g_Boxes[n].bbBox.GetMin()[1];
		vPoints[7].z = g_Boxes[n].bbBox.GetMin()[2];

		D3D_TransformPoints(vPoints, &g_Boxes[n].mWorld);

		for (int iVert = 0; iVert < 8; ++iVert)
		{
			pVertices->v = vPoints[iVert];
			pVertices->color = D3D_Color(g_Boxes[n].v4Color);
			++pVertices;
		}
	}

	g_pVBBox->Unlock();

	g_RenderList.Add(K2_NEW(ctx_D3D9,   CBoxRenderer)(g_iNumBoxes));
}


/*====================
  D3D_AddBox
  ====================*/
void	D3D_AddBox(const CBBoxf &bbBox, const CVec4f &v4Color, const D3DXMATRIXA16 &mWorld)
{
	if (!g_bValidScene || g_iNumBoxes >= MAX_BOXES)
		return;

	int N(g_iNumBoxes);

	g_Boxes[N].bbBox = bbBox;
	g_Boxes[N].v4Color = v4Color;
	g_Boxes[N].mWorld = mWorld;
	++g_iNumBoxes;
}


/*====================
  D3D_AddPointBatches
  ====================*/
void	D3D_AddPointBatches()
{
	PROFILE("D3D_AddPointBatches");

	if (g_iNumPoints == 0)
		return;

	SLineVertex* pVertices;

	if (FAILED(g_pVBPoint->Lock(0, g_iNumPoints * sizeof(SLineVertex), (void**)&pVertices, D3DLOCK_DISCARD)))
		return;

	for (int n = 0; n < g_iNumPoints; ++n)
	{
		pVertices[n].v = g_Points[n].v3Pos;
		pVertices[n].color = D3D_Color(g_Points[n].v4Color);

	}

	g_pVBPoint->Unlock();

	g_RenderList.Add(K2_NEW(ctx_D3D9,   CPointRenderer)(g_iNumPoints));
}


/*====================
  D3D_AddPoint
  ====================*/
void	D3D_AddPoint(const CVec3f &v3Pos, const CVec4f &v4Color)
{
	if (!g_bValidScene || g_iNumPoints >= MAX_POINTS)
		return;

	int N(g_iNumPoints);

	g_Points[N].v3Pos = v3Pos;
	g_Points[N].v4Color = v4Color;
	++g_iNumPoints;
}


/*====================
  D3D_AddLineBatches
  ====================*/
void	D3D_AddLineBatches()
{
	PROFILE("D3D_AddLineBatches");

	if (g_iNumLines == 0)
		return;

	SLineVertex* pVertices;

	if (FAILED(g_pVBLine->Lock(0, g_iNumLines * sizeof(SLineVertex), (void**)&pVertices, D3DLOCK_DISCARD)))
		return;

	for (int n = 0; n < g_iNumLines; ++n)
	{
		pVertices->v = g_Lines[n].v3Start;
		pVertices->color = D3D_Color(g_Lines[n].v4Color);
		++pVertices;

		pVertices->v = g_Lines[n].v3End;
		pVertices->color = D3D_Color(g_Lines[n].v4Color);
		++pVertices;
	}

	g_pVBLine->Unlock();
	
	g_RenderList.Add(K2_NEW(ctx_D3D9,   CLineRenderer)(g_iNumLines));
}


/*====================
  D3D_AddLine
  ====================*/
void	D3D_AddLine(const CVec3f &v3Start, const CVec3f &v3End, const CVec4f &v4Color)
{
	if (!g_bValidScene || g_iNumLines >= MAX_LINES)
		return;

	int N(g_iNumLines);

	g_Lines[N].v3Start = v3Start;
	g_Lines[N].v3End = v3End;
	g_Lines[N].v4Color = v4Color;
	++g_iNumLines;
}


/*====================
  D3D_AddEffectTriangleBatches
  ====================*/
void	D3D_AddEffectTriangleBatches(EMaterialPhase ePhase)
{
	if (g_iNumEffectTriangles == 0)
		return;

	//
	// Build effect triangle vertex buffer
	//

	START_PROFILE("Build vertex buffer");

	SEffectVertex* pVertices;

	if (!d3d_crossFrameCycleBuffer || g_dwEffectTriangleBase + g_iNumEffectTriangles >= MAX_EFFECT_TRIANGLES)
		g_dwEffectTriangleBase = 0;

	if (FAILED(g_pVBEffectTriangle->Lock(g_dwEffectTriangleBase * 3 * sizeof(SEffectVertex), g_iNumEffectTriangles * 3 * sizeof(SEffectVertex), (void**)&pVertices, g_dwEffectTriangleBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
		return;

	int q = 0;
	for (int n(0); n < g_iNumEffectTriangles; ++n)
	{
		MemManager.Copy(&pVertices[q], &g_EffectTriangles[n].vert[0], 3 * sizeof(SEffectVertex));
		q += 3;
	}

	g_pVBEffectTriangle->Unlock();

	END_PROFILE

	//
	// Add effect triangle batches to renderlist
	//

	for (int n(0); n < g_iNumEffectTriangles; ++n)
	{
		ResHandle hMaterial(g_EffectTriangles[n].hMaterial);

		if (!D3D_GetMaterial(hMaterial).HasPhase(ePhase))
			continue;

		int iMinEffectLayer(g_EffectTriangles[n].iEffectLayer);

		int f;
		for (f = n + 1; f < g_iNumEffectTriangles && g_EffectTriangles[f].hMaterial == hMaterial; ++f)
			iMinEffectLayer = MIN(iMinEffectLayer, g_EffectTriangles[f].iEffectLayer);

		g_RenderList.Add(K2_NEW(ctx_D3D9,   CEffectTriangleRenderer)(hMaterial, g_dwEffectTriangleBase + n, g_dwEffectTriangleBase + f, (iMinEffectLayer << 20) + n, -FAR_AWAY));

		n += (f - n) - 1;
	}

	g_dwEffectTriangleBase += g_iNumEffectTriangles;
}


/*====================
  D3D_AddEffectTriangle
  ====================*/
void	D3D_AddEffectTriangle
(
	const CVec3f &v0,
	const CVec3f &v1,
	const CVec3f &v2,
	dword color0,
	dword color1,
	dword color2,
	const CVec4f &t0,
	const CVec4f &t1,
	const CVec4f &t2,
	ResHandle hMaterial
)
{
	if (!g_bValidScene || g_iNumEffectTriangles >= MAX_EFFECT_TRIANGLES)
		return;

	int N(g_iNumEffectTriangles);

	g_EffectTriangles[N].vert[0].v = v0;
	g_EffectTriangles[N].vert[0].color = color0;
	g_EffectTriangles[N].vert[0].t = t0;

	g_EffectTriangles[N].vert[1].v = v1;
	g_EffectTriangles[N].vert[1].color = color1;
	g_EffectTriangles[N].vert[1].t = t1;

	g_EffectTriangles[N].vert[2].v = v2;
	g_EffectTriangles[N].vert[2].color = color2;
	g_EffectTriangles[N].vert[2].t = t2;

	g_EffectTriangles[N].hMaterial = hMaterial;
	g_EffectTriangles[N].iEffectLayer = 0;
	g_EffectTriangles[N].fDepth = 0.0f;

	++g_iNumEffectTriangles;
}


/*====================
  D3D_AddEffectTriangle
  ====================*/
void	D3D_AddEffectTriangle(const SEffectTriangle &sTri)
{
	if (!g_bValidScene || g_iNumEffectTriangles >= MAX_EFFECT_TRIANGLES)
		return;

	MemManager.Copy(&g_EffectTriangles[g_iNumEffectTriangles], &sTri, sizeof(SEffectTriangle));
	++g_iNumEffectTriangles;
}


/*====================
  D3D_AddExtendedTriangleBatches
  ====================*/
void	D3D_AddExtendedTriangleBatches(EMaterialPhase ePhase)
{
	PROFILE("D3D_AddExtendedTriangleBatches");

	if (g_iNumExtendedTriangles == 0)
		return;

	//
	// Build extended triangle vertex buffer
	//

	SExtendedVertex* pVertices;

	if (FAILED(g_pVBExtendedTriangle->Lock(0, g_iNumExtendedTriangles * 3 * sizeof(SExtendedVertex), (void**)&pVertices, D3DLOCK_DISCARD)))
		return;

	int q = 0;
	for (int n(0); n < g_iNumExtendedTriangles; ++n)
	{
		MemManager.Copy(&pVertices[q], &g_ExtendedTriangles[n].vert[0], 3 * sizeof(SExtendedVertex));
		q += 3;
	}

	g_pVBExtendedTriangle->Unlock();

	//
	// Add extended triangle batches to renderlist
	//

	for (int n(0); n < g_iNumExtendedTriangles; ++n)
	{
		ResHandle hMaterial(g_ExtendedTriangles[n].hMaterial);

		if (!D3D_GetMaterial(hMaterial).HasPhase(ePhase))
			continue;

		int f;
		for (f = n + 1; f < g_iNumExtendedTriangles && g_ExtendedTriangles[f].hMaterial == hMaterial; ++f);

		g_RenderList.Add(K2_NEW(ctx_D3D9,   CExtendedTriangleRenderer)(hMaterial, n, f, 0, -FAR_AWAY));

		n += (f - n) - 1;
	}
}


/*====================
  D3D_AddExtendedTriangle
  ====================*/
void	D3D_AddExtendedTriangle
(
	const CVec3f &v0,
	const CVec3f &v1,
	const CVec3f &v2,
	const CVec3f &n0,
	const CVec3f &n1,
	const CVec3f &n2,
	dword color0,
	dword color1,
	dword color2,
	const CVec4f &t0,
	const CVec4f &t1,
	const CVec4f &t2,
	const CVec3f &tan0,
	const CVec3f &tan1,
	const CVec3f &tan2,
	ResHandle hMaterial
)
{
	if (!g_bValidScene || g_iNumExtendedTriangles >= MAX_EXTENDED_TRIANGLES)
		return;

	int N(g_iNumExtendedTriangles);

	g_ExtendedTriangles[N].vert[0].v = v0;
	g_ExtendedTriangles[N].vert[0].n = n0;
	g_ExtendedTriangles[N].vert[0].color = color0;
	g_ExtendedTriangles[N].vert[0].t = t0;
	g_ExtendedTriangles[N].vert[0].tan = tan0;

	g_ExtendedTriangles[N].vert[1].v = v1;
	g_ExtendedTriangles[N].vert[1].n = n1;
	g_ExtendedTriangles[N].vert[1].color = color1;
	g_ExtendedTriangles[N].vert[1].t = t1;
	g_ExtendedTriangles[N].vert[1].tan = tan1;

	g_ExtendedTriangles[N].vert[2].v = v2;
	g_ExtendedTriangles[N].vert[2].n = n2;
	g_ExtendedTriangles[N].vert[2].color = color2;
	g_ExtendedTriangles[N].vert[2].t = t2;
	g_ExtendedTriangles[N].vert[2].tan = tan2;

	g_ExtendedTriangles[N].hMaterial = hMaterial;

	++g_iNumExtendedTriangles;
}


/*====================
  D3D_AddTreeBillboardBatches
  ====================*/
void	D3D_AddTreeBillboardBatches(EMaterialPhase ePhase)
{
	if (g_iNumTreeBillboards == 0)
		return;

	//
	// Build extended triangle vertex buffer
	//

	STreeBillboardVertex* pVertices;

	if (FAILED(g_pVBTreeBillboard->Lock(0, g_iNumTreeBillboards * 4 * sizeof(STreeBillboardVertex), (void**)&pVertices, D3DLOCK_DISCARD)))
		return;

	int q = 0;
	for (int n(0); n < g_iNumTreeBillboards; ++n)
	{
		MemManager.Copy(&pVertices[q], &g_TreeBillboards[n].vert[0], 4 * sizeof(STreeBillboardVertex));
		q += 4;
	}

	g_pVBTreeBillboard->Unlock();

	//
	// Add tree billboard batches to renderlist
	//

	for (int n(0); n < g_iNumTreeBillboards; ++n)
	{
		ResHandle hMaterial(g_TreeBillboards[n].hMaterial);
		dword dwAlphaTest(g_TreeBillboards[n].dwAlphaTest);

		if (!D3D_GetMaterial(hMaterial).HasPhase(ePhase))
			continue;

		int f;
		for (f = n + 1; f < g_iNumTreeBillboards && g_TreeBillboards[f].hMaterial == hMaterial && g_TreeBillboards[f].dwAlphaTest == dwAlphaTest; ++f);

		g_RenderList.Add(K2_NEW(ctx_D3D9,   CTreeBillboardRenderer)(hMaterial, dwAlphaTest, n, f, 0, 0.0f));

		n += (f - n) - 1;
	}

	g_iNumTreeBillboards = 0;
}


/*====================
  D3D_AddTreeBillboard
  ====================*/
void	D3D_AddTreeBillboard
(
	const CVec3f &v0,
	const CVec3f &v1,
	const CVec3f &v2,
	const CVec3f &v3,
	dword color0,
	dword color1,
	dword color2,
	dword color3,
	const CVec2f &t0,
	const CVec2f &t1,
	const CVec2f &t2,
	const CVec2f &t3,
	ResHandle hMaterial,
	dword dwAlphaTest,
	const D3DXMATRIXA16 &mWorld
)
{
	if (!g_bValidScene || g_iNumTreeBillboards >= MAX_TREE_BILLBOARDS)
		return;

	int N(g_iNumTreeBillboards);

	g_TreeBillboards[N].vert[0].v = D3D_TransformPoint(v0, mWorld);
	g_TreeBillboards[N].vert[0].color = color0;
	g_TreeBillboards[N].vert[0].t = t0;

	g_TreeBillboards[N].vert[1].v = D3D_TransformPoint(v1, mWorld);
	g_TreeBillboards[N].vert[1].color = color1;
	g_TreeBillboards[N].vert[1].t = t1;

	g_TreeBillboards[N].vert[2].v = D3D_TransformPoint(v2, mWorld);
	g_TreeBillboards[N].vert[2].color = color2;
	g_TreeBillboards[N].vert[2].t = t2;

	g_TreeBillboards[N].vert[3].v = D3D_TransformPoint(v3, mWorld);
	g_TreeBillboards[N].vert[3].color = color3;
	g_TreeBillboards[N].vert[3].t = t3;

	g_TreeBillboards[N].hMaterial = hMaterial;
	g_TreeBillboards[N].dwAlphaTest = dwAlphaTest;

	++g_iNumTreeBillboards;
}


/*====================
  D3D_AddScenePolys
  ====================*/
void	D3D_AddScenePolys(EMaterialPhase ePhase)
{
	const SceneFaceList &SceneFaces(SceneManager.GetFaceList());
	for (SceneFaceList::const_iterator it(SceneFaces.begin()); it != SceneFaces.end(); ++it)
	{
		g_RenderList.Add(K2_NEW(ctx_D3D9,   CScenePolyRenderer)(it->hMaterial, it->verts, it->zNumVerts, it->flags));
	}
}

/*====================
  D3D_LerpEffectVertex
  ====================*/
void	D3D_LerpEffectVertex(float fLerp0, float fLerp1, const SEffectVertex values[], SEffectVertex &sOut)
{
	sOut.v = LERP(fLerp1, LERP(fLerp0, values[0].v, values[1].v), LERP(fLerp0, values[2].v, values[3].v));
	sOut.t = LERP(fLerp1, LERP(fLerp0, values[0].t, values[1].t), LERP(fLerp0, values[2].t, values[3].t));
	sOut.color = LERP(fLerp1, LERP(fLerp0, CVec4f(values[0].color), CVec4f(values[1].color)), LERP(fLerp0, CVec4f(values[2].color), CVec4f(values[3].color))).GetAsDWord();
}


/*====================
  D3D_AddGroundSprite
  ====================*/
void	D3D_AddGroundSprite(const CVec3f &v3Pos, float fHeight, float fWidth, const CVec3f &v3Angles, const CVec4f &v4Color, float fFrame, float fParam, ResHandle hMaterial, int iEffectLayer, float fDepth)
{
	PROFILE("D3D_AddGroundSprite");

	if (!g_bValidScene || !g_pWorld)
		return;

	dword dwColor(v4Color.GetAsDWord());

	CAxis aAxis(v3Angles);
	CVec2f av2Corners[4] = 
	{
		CVec2f(v3Pos.xy() + aAxis[RIGHT].xy() * -fWidth + aAxis[FORWARD].xy() * -fHeight),
		CVec2f(v3Pos.xy() + aAxis[RIGHT].xy() * -fWidth + aAxis[FORWARD].xy() * fHeight),
		CVec2f(v3Pos.xy() + aAxis[RIGHT].xy() * fWidth + aAxis[FORWARD].xy() * fHeight),
		CVec2f(v3Pos.xy() + aAxis[RIGHT].xy() * fWidth + aAxis[FORWARD].xy() * -fHeight)
	};

	CRectf recArea(FAR_AWAY, FAR_AWAY, -FAR_AWAY, -FAR_AWAY);
	for (int i(0); i < 4; ++i)
		recArea.AddPoint(av2Corners[i]);

	CVec2f vX(av2Corners[3] - av2Corners[0]);
	CVec2f vY(av2Corners[1] - av2Corners[0]);

	float dX(DotProduct(vX, vX));
	float dY(DotProduct(vY, vY));

	float fTileSize(g_pWorld->GetScale());
	const CVec2f &v2Min(recArea.lt());
	const CVec2f &v2Max(recArea.rb());

	int iMinX(CLAMP(INT_FLOOR(v2Min.x / fTileSize), 0, g_pWorld->GetTileWidth() - 1));
	int iMinY(CLAMP(INT_FLOOR(v2Min.y / fTileSize), 0, g_pWorld->GetTileWidth() - 1));
	int iMaxX(CLAMP(INT_CEIL(v2Max.x / fTileSize), 0, g_pWorld->GetTileHeight() - 1));
	int iMaxY(CLAMP(INT_CEIL(v2Max.y / fTileSize), 0, g_pWorld->GetTileHeight() - 1));

	float fScale(g_pWorld->GetScale());
	byte *pSplitMap(g_pWorld->GetSplitMap());
	int iTileWidth(g_pWorld->GetTileWidth());

	for (int iY(iMinY); iY <= iMaxY; ++iY)
	{
		for (int iX(iMinX); iX <= iMaxX; ++iX)
		{
			// Planes
			if (pSplitMap[iY * iTileWidth + iX] == SPLIT_NEG)
			{
				for (int iTri(0); iTri < 2; ++iTri)
				{
					CConvexPolygon2 workingPoly;

					// Planes
					if (iTri == TRIANGLE_LEFT)
					{
						CVec2f v0(iX * fScale - av2Corners[0].x, iY * fScale - av2Corners[0].y);
						CVec2f v1((iX + 1) * fScale - av2Corners[0].x, v0.y);
						CVec2f v2(v0.x, (iY + 1) * fScale - av2Corners[0].y);

						workingPoly.AddVertex(CVec2f(DotProduct(v0, vX) / dX, DotProduct(v0, vY) / dY));
						workingPoly.AddVertex(CVec2f(DotProduct(v1, vX) / dX, DotProduct(v1, vY) / dY));
						workingPoly.AddVertex(CVec2f(DotProduct(v2, vX) / dX, DotProduct(v2, vY) / dY));

						workingPoly.ClipNegX(0.0f);
						workingPoly.ClipNegY(0.0f);
						workingPoly.ClipPosX(1.0f);
						workingPoly.ClipPosY(1.0f);
					}
					else
					{
						CVec2f v0((iX + 1) * fScale - av2Corners[0].x, (iY + 1) * fScale - av2Corners[0].y);
						CVec2f v1(iX * fScale - av2Corners[0].x, v0.y);
						CVec2f v2(v0.x, iY * fScale - av2Corners[0].y);

						workingPoly.AddVertex(CVec2f(DotProduct(v0, vX) / dX, DotProduct(v0, vY) / dY));
						workingPoly.AddVertex(CVec2f(DotProduct(v1, vX) / dX, DotProduct(v1, vY) / dY));
						workingPoly.AddVertex(CVec2f(DotProduct(v2, vX) / dX, DotProduct(v2, vY) / dY));

						workingPoly.ClipNegX(0.0f);
						workingPoly.ClipNegY(0.0f);
						workingPoly.ClipPosX(1.0f);
						workingPoly.ClipPosY(1.0f);
					}

					if (workingPoly.GetNumVerts() > 2)
					{
						const CPlane &cPlane(g_pWorld->GetTilePlane(iX, iY, EGridTriangles(iTri)));

						SEffectTriangle sTri;

						sTri.hMaterial = hMaterial;
						sTri.iEffectLayer = iEffectLayer;
						sTri.fDepth = fDepth;

						const CVec2f &p0(workingPoly.GetVertex(0));
						CVec2f v0(av2Corners[0] + vX * p0.x + vY * p0.y);

						sTri.vert[0].v = cPlane.Project(v0);
						sTri.vert[0].t = CVec4f(p0.x, p0.y, fFrame, fParam);
						sTri.vert[0].color = dwColor;

						sTri.vert[1].t.z = fFrame;
						sTri.vert[1].t.w = fParam;
						sTri.vert[1].color = dwColor;

						sTri.vert[2].t.z = fFrame;
						sTri.vert[2].t.w = fParam;
						sTri.vert[2].color = dwColor;

						for (uint t(0); t < workingPoly.GetNumVerts() - 2; ++t)
						{
							const CVec2f &p1(workingPoly.GetVertex(t + 1));
							CVec2f v1(av2Corners[0] + vX * p1.x + vY * p1.y);

							sTri.vert[1].v = cPlane.Project(v1);
							sTri.vert[1].t.x = p1.x;
							sTri.vert[1].t.y = p1.y;
														
							const CVec2f &p2(workingPoly.GetVertex(t + 2));
							CVec2f v2(av2Corners[0] + vX * p2.x + vY * p2.y);

							sTri.vert[2].v = cPlane.Project(v2);
							sTri.vert[2].t.x = p2.x;
							sTri.vert[2].t.y = p2.y;
						
							D3D_AddEffectTriangle(sTri);
						}
					}
				}
			}
			else
			{
				for (int iTri(0); iTri < 2; ++iTri)
				{
					CConvexPolygon2 workingPoly;

					// Planes
					if (iTri == TRIANGLE_LEFT)
					{
						CVec2f v0(iX * fScale - av2Corners[0].x, (iY + 1) * fScale - av2Corners[0].y);
						CVec2f v1(v0.x, iY * fScale - av2Corners[0].y);
						CVec2f v2((iX + 1) * fScale - av2Corners[0].x, v0.y);

						workingPoly.AddVertex(CVec2f(DotProduct(v0, vX) / dX, DotProduct(v0, vY) / dY));
						workingPoly.AddVertex(CVec2f(DotProduct(v1, vX) / dX, DotProduct(v1, vY) / dY));
						workingPoly.AddVertex(CVec2f(DotProduct(v2, vX) / dX, DotProduct(v2, vY) / dY));

						workingPoly.ClipNegX(0.0f);
						workingPoly.ClipNegY(0.0f);
						workingPoly.ClipPosX(1.0f);
						workingPoly.ClipPosY(1.0f);
					}
					else
					{
						CVec2f v0((iX + 1) * fScale - av2Corners[0].x, iY * fScale - av2Corners[0].y);
						CVec2f v1(v0.x, (iY + 1) * fScale - av2Corners[0].y);
						CVec2f v2(iX * fScale - av2Corners[0].x, v0.y);

						workingPoly.AddVertex(CVec2f(DotProduct(v0, vX) / dX, DotProduct(v0, vY) / dY));
						workingPoly.AddVertex(CVec2f(DotProduct(v1, vX) / dX, DotProduct(v1, vY) / dY));
						workingPoly.AddVertex(CVec2f(DotProduct(v2, vX) / dX, DotProduct(v2, vY) / dY));

						workingPoly.ClipNegX(0.0f);
						workingPoly.ClipNegY(0.0f);
						workingPoly.ClipPosX(1.0f);
						workingPoly.ClipPosY(1.0f);
					}

					if (workingPoly.GetNumVerts() > 2)
					{
						const CPlane &cPlane(g_pWorld->GetTilePlane(iX, iY, EGridTriangles(iTri)));

						SEffectTriangle sTri;

						sTri.hMaterial = hMaterial;
						sTri.iEffectLayer = iEffectLayer;
						sTri.fDepth = fDepth;

						const CVec2f &p0(workingPoly.GetVertex(0));
						CVec2f v0(av2Corners[0] + vX * p0.x + vY * p0.y);

						sTri.vert[0].v = cPlane.Project(v0);
						sTri.vert[0].t = CVec4f(p0.x, p0.y, fFrame, fParam);
						sTri.vert[0].color = dwColor;

						sTri.vert[1].t.z = fFrame;
						sTri.vert[1].t.w = fParam;
						sTri.vert[1].color = dwColor;

						sTri.vert[2].t.z = fFrame;
						sTri.vert[2].t.w = fParam;
						sTri.vert[2].color = dwColor;

						for (uint t(0); t < workingPoly.GetNumVerts() - 2; ++t)
						{
							const CVec2f &p1(workingPoly.GetVertex(t + 1));
							CVec2f v1(av2Corners[0] + vX * p1.x + vY * p1.y);

							sTri.vert[1].v = cPlane.Project(v1);
							sTri.vert[1].t.x = p1.x;
							sTri.vert[1].t.y = p1.y;
														
							const CVec2f &p2(workingPoly.GetVertex(t + 2));
							CVec2f v2(av2Corners[0] + vX * p2.x + vY * p2.y);

							sTri.vert[2].v = cPlane.Project(v2);
							sTri.vert[2].t.x = p2.x;
							sTri.vert[2].t.y = p2.y;
						
							D3D_AddEffectTriangle(sTri);
						}
					}
				}
			}
		}
	}
}


/*==========================
  D3D_AddSky
  ==========================*/
void	D3D_AddSky()
{
	PROFILE("D3D_AddSky");

	if (g_pCam->HasFlags(CAM_NO_SKY) || !gfx_sky)
		return;

	float fOffset(FAR_AWAY / 1000.0f);

	SceneEntityList &skyEntities(SceneManager.GetSkyEntityList());
	for (SceneEntityList::iterator it(skyEntities.begin()); it != skyEntities.end(); ++it)
	{
		SSceneEntityEntry &cEntry(**it);
		CSceneEntity &cEntity(cEntry.cEntity);
		cEntity.effectdepth = FAR_AWAY * 2.0f - fOffset;

		switch (cEntity.objtype)
		{
		case OBJTYPE_MODEL:
			D3D_AddModelToLists(cEntity, cEntry);
			break;

		case OBJTYPE_BILLBOARD:
			if (!D3D_GetMaterial(cEntity.hRes).HasPhase(PHASE_COLOR))
				continue;

			D3D_AddBillboard
			(
				cEntity.GetPosition(),
				cEntity.width * cEntity.scale,
				cEntity.height * cEntity.scale,
				cEntity.angle[ROLL],
				cEntity.s1,
				cEntity.t1,
				cEntity.s2,
				cEntity.t2,
				cEntity.frame,
				cEntity.param,
				cEntity.hRes,
				cEntity.hSkin,
				cEntity.angle[PITCH],
				cEntity.angle[YAW],
				0.0f,
				0,
				cEntity.effectdepth,
				AXIS_IDENTITY,
				cEntity.color
			);
			break;
		}

		fOffset += (FAR_AWAY / 1000.0f);
	}

	if (SceneManager.GetDrawSkybox())
		g_RenderList.Add(K2_NEW(ctx_D3D9,   CSkyRenderer)());
}


/*====================
  D3D_AddSceneEntities

  draw all models
  ====================*/
void	D3D_AddSceneEntities()
{
	PROFILE("D3D_AddSceneEntities");

	g_pTreeSceneManager->ResetLeaves();

	SceneEntityList &lSceneEntities(SceneManager.GetEntityList());
	SceneEntityList::iterator itEnd(lSceneEntities.end());
	for (SceneEntityList::iterator it(lSceneEntities.begin()); it != itEnd; ++it)
	{
		SSceneEntityEntry &cEntry(**it);
		
		if (cEntry.bCull && cEntry.bCullShadow)
			continue;
		
		CSceneEntity &cEntity(cEntry.cEntity);

		switch (cEntity.objtype)
		{
		case OBJTYPE_MODEL:
			D3D_AddModelToLists(cEntity, cEntry);
			break;

		case OBJTYPE_BILLBOARD:
			if (!g_ResourceManager.GetMaterial(cEntity.hRes))
				continue;

			D3D_AddBillboard
			(
				cEntity.GetPosition(),
				cEntity.width * cEntity.scale,
				cEntity.height * cEntity.scale,
				cEntity.angle[ROLL],
				cEntity.s1,
				cEntity.t1,
				cEntity.s2,
				cEntity.t2,
				cEntity.frame,
				cEntity.param,
				cEntity.hRes,
				cEntity.hSkin,
				cEntity.angle[PITCH],
				cEntity.angle[YAW],
				0.0f,
				0,
				0.0f,
				cEntity.axis,
				cEntity.color
			);
			break;

		case OBJTYPE_BEAM:
			if (!g_ResourceManager.GetMaterial(cEntity.hRes))
				continue;

			D3D_SetColor(cEntity.color);
			D3D_AddBeam
			(
				cEntity.GetPosition(),
				cEntity.angle,
				cEntity.scale,
				cEntity.height,
				cEntity.s1,
				cEntity.hRes,
				cEntity.effectlayer
			);
			break;

		case OBJTYPE_GROUNDSPRITE:
			if (!g_ResourceManager.GetMaterial(cEntity.hRes))
				continue;

			D3D_AddGroundSprite
			(
				cEntity.GetPosition(),
				cEntity.height * cEntity.scale,
				cEntity.width * cEntity.scale,
				cEntity.angle,
				cEntity.color,
				cEntity.frame,
				cEntity.param,
				cEntity.hRes,
				cEntity.effectlayer,
				cEntity.effectdepth
			);
			break;

		default:
			break;
		}
	}
}


/*====================
  D3D_ObjectBounds

  adds all visible models to the global scene bounding box
  ====================*/
void	D3D_ObjectBounds(CBBoxf &bbObjects)
{
	PROFILE("D3D_ObjectBounds");

	const SceneEntityList &lSceneEntities(SceneManager.GetEntityList());
	SceneEntityList::const_iterator itEnd(lSceneEntities.end());
	for (SceneEntityList::const_iterator it(lSceneEntities.begin()); it != itEnd; ++it)
	{
		SSceneEntityEntry &cEntry(**it);
		CSceneEntity &cEntity(cEntry.cEntity);

		if (cEntry.bCull)
			continue;

		switch (cEntity.objtype)
		{
		case OBJTYPE_MODEL:
			{
				if (cEntity.flags & SCENEENT_USE_BOUNDS)
				{
					bbObjects += cEntity.bounds;
				}
				else
				{
					IModel *pModel(g_ResourceManager.GetModel(cEntity.hRes)->GetModelFile());
					if (pModel == NULL)
						continue;

					CBBoxf bbModel(pModel->GetBounds());

					if (bbModel.GetDim(X) < 0.0f || bbModel.GetDim(Y) < 0.0f || bbModel.GetDim(Z) < 0.0f)
						continue;

					bbModel.Transform(cEntity.GetPosition(), cEntity.axis, cEntity.scale);
					bbObjects += bbModel;
				}
			}
			break;
		default:
			break;
		}
	}
}


/*====================
  D3D_AddSceneEntity
  ====================*/
void	D3D_AddSceneEntity(IEmitter &cEmitter, int iEffectLayer, float fEffectDepth, bool bCull)
{
	// Imbedded Emitters
	uint uiNumEmitters(cEmitter.GetNumEmitters());
	for (uint uiEmitter(0); uiEmitter < uiNumEmitters; ++uiEmitter)
	{
		IEmitter *pEmitter(cEmitter.GetEmitter(uiEmitter));
		while (pEmitter != NULL)
		{
			D3D_AddSceneEntity(*pEmitter, iEffectLayer, fEffectDepth, bCull);
			pEmitter = pEmitter->GetNextEmitter();
		}
	}

	//
	// Add Lights
	//

	uint uiNumLights(cEmitter.GetNumLights());

	for (uint uiLight(0); uiLight < uiNumLights; ++uiLight)
	{
		CSceneLight scLight;

		if (cEmitter.GetLight(uiLight, scLight))
			SceneManager.AddLight(scLight);
	}

	//
	// Add Entities
	//

	uint uiNumEntities(cEmitter.GetNumEntities());

	for (uint uiEntity(0); uiEntity < uiNumEntities; ++uiEntity)
	{
		CSceneEntity scEntity;
		scEntity.Clear();

		if (cEmitter.GetEntity(uiEntity, scEntity))
		{
			scEntity.effectlayer = iEffectLayer;
			scEntity.effectdepth = fEffectDepth;

			SceneManager.AddEntity(scEntity, bCull);
		}
	}
}


/*====================
  D3D_AddParticleSystemSceneEntities
  ====================*/
void	D3D_AddParticleSystemSceneEntities(bool bCull)
{
	PROFILE("D3D_AddParticleSystemSceneEntities");

	if (!vid_drawParticleSystems || !g_bValidScene || g_pCam == NULL)
		return;

	SceneParticleSystemList &lParticleSystems(SceneManager.GetParticleSystemList());
	for (SceneParticleSystemList::iterator it(lParticleSystems.begin()), itEnd(lParticleSystems.end()); it != itEnd; ++it)
	{
		SSceneParticleSystemEntry &cEntry(**it);
		const CParticleSystem *pParticleSystem(cEntry.pParticleSystem);

		if (cEntry.bCull)
			continue;

		int iEffectLayer(0);
		//float fEffectDepth(DotProduct(g_pCam->GetViewAxis(FORWARD), pParticleSystem->GetSourcePosition()));

		const EmitterList &lEmitters(pParticleSystem->GetEmitterList());
		for (EmitterList::const_iterator it(lEmitters.begin()), itEnd(lEmitters.end()); it != itEnd; ++it)
		{
			D3D_AddSceneEntity(**it, iEffectLayer, -FAR_AWAY, bCull);
			++iEffectLayer;
		}
	}
}


/*====================
  D3D_AddEmitter
  ====================*/
void	D3D_AddEmitter(IEmitter &cEmitter, int iEffectLayer, float fEffectDepth)
{
	// Imbedded Emitters
	uint uiNumEmitters(cEmitter.GetNumEmitters());
	for (uint uiEmitter(0); uiEmitter < uiNumEmitters; ++uiEmitter)
	{
		IEmitter *pEmitter(cEmitter.GetEmitter(uiEmitter));
		while (pEmitter != NULL)
		{
			D3D_AddEmitter(*pEmitter, iEffectLayer, fEffectDepth);
			pEmitter = pEmitter->GetNextEmitter();
		}
	}

	//
	// Add Billboards
	//

	uint uiNumBillboards(cEmitter.GetNumBillboards());

	for (uint uiBillboard(0); uiBillboard < uiNumBillboards; ++uiBillboard)
	{
		if (g_iNumBillboards >= MAX_BILLBOARDS)
			continue;

		if (cEmitter.GetBillboard(uiBillboard, g_Billboards[g_iNumBillboards]))
		{
			g_Billboards[g_iNumBillboards].iEffectLayer = iEffectLayer;
			g_Billboards[g_iNumBillboards].fDepth = fEffectDepth;
			++g_iNumBillboards;
		}
	}

	//
	// Add Beams
	//

	uint uiNumBeams(cEmitter.GetNumBeams());

	for (uint uiBeam(0); uiBeam < uiNumBeams; ++uiBeam)
	{
		if (g_iNumBeams >= MAX_BEAMS)
			continue;

		if (cEmitter.GetBeam(uiBeam, g_Beams[g_iNumBeams]))
		{
			g_Beams[g_iNumBeams].iEffectLayer = iEffectLayer;
			++g_iNumBeams;
		}
	}

	//
	// Add Triangles
	//

	uint uiNumTriangles(cEmitter.GetNumTriangles());

	for (uint uiTriangle(0); uiTriangle < uiNumTriangles; ++uiTriangle)
	{
		if (g_iNumEffectTriangles >= MAX_EFFECT_TRIANGLES)
			continue;

		if (cEmitter.GetTriangle(uiTriangle, reinterpret_cast<STriangle &>(g_EffectTriangles[g_iNumEffectTriangles])))
		{
			g_EffectTriangles[g_iNumEffectTriangles].iEffectLayer = iEffectLayer;
			g_EffectTriangles[g_iNumEffectTriangles].fDepth = fEffectDepth;
			++g_iNumEffectTriangles;
		}
	}
}


/*====================
  D3D_AddParticleSystems
  ====================*/
void	D3D_AddParticleSystems()
{
	PROFILE("D3D_AddParticleSystems");

	if (!vid_drawParticleSystems || !g_bValidScene)
		return;

	SceneParticleSystemList &lParticleSystems(SceneManager.GetParticleSystemList());
	for (SceneParticleSystemList::iterator it(lParticleSystems.begin()), itEnd(lParticleSystems.end()); it != itEnd; ++it)
	{
		SSceneParticleSystemEntry &cEntry(**it);
		const CParticleSystem *pParticleSystem(cEntry.pParticleSystem);

		if (cEntry.bCull)
			continue;

		int iEffectLayer(0);
		float fEffectDepth(DotProduct(g_pCam->GetViewAxis(FORWARD), pParticleSystem->GetSourcePosition()));

		const EmitterList &lEmitters(pParticleSystem->GetEmitterList());
		for (EmitterList::const_iterator it(lEmitters.begin()), itEnd(lEmitters.end()); it != itEnd; ++it)
		{
			D3D_AddEmitter(**it, iEffectLayer, fEffectDepth);
			++iEffectLayer;
		}
	}
}


/*====================
  D3D_SetCloudProj
  ====================*/
void	D3D_SetCloudProj(float fTime)
{
	CVec3f v3SunLightDir(-SceneManager.GetSunPos());

	CAxis aAxis;
	aAxis.SetFromForwardVec(v3SunLightDir);

	D3DXMATRIXA16 mViewTranslation;
	D3DXMatrixTranslation(&mViewTranslation, gfx_cloudSpeedX * fTime, gfx_cloudSpeedY * fTime, 0.0f);

	D3DXMATRIXA16 mViewAxis;
	mViewAxis[0] = aAxis[RIGHT].x;
	mViewAxis[1] = aAxis[UP].x;
	mViewAxis[2] = -aAxis[FORWARD].x;
	mViewAxis[3] = 0.0f;

	mViewAxis[4] = aAxis[RIGHT].y;
	mViewAxis[5] = aAxis[UP].y;
	mViewAxis[6] = -aAxis[FORWARD].y;
	mViewAxis[7] = 0.0f;

	mViewAxis[8] = aAxis[RIGHT].z;
	mViewAxis[9] = aAxis[UP].z;
	mViewAxis[10] = -aAxis[FORWARD].z;
	mViewAxis[11] = 0.0f;

	mViewAxis[12] = 0.0f;
	mViewAxis[13] = 0.0f;
	mViewAxis[14] = 0.0f;
	mViewAxis[15] = 1.0f;

	float fScale(gfx_cloudScale * 100.0f);

	D3DXMATRIXA16 mProj;

	D3DXMatrixOrthoOffCenterRH(&mProj, -fScale / 2.0f, fScale / 2.0f, -fScale / 2.0f, fScale / 2.0f, 0.0f, 1.0f);

	g_mCloudProj = mViewTranslation * mViewAxis * mProj;
}


/*====================
  D3D_SetFowProj
  ====================*/
void	D3D_SetFowProj()
{
	if (terrain.pWorld)
		D3DXMatrixScaling(&g_mFowProj, 1.0f / terrain.pWorld->GetWorldWidth(), 1.0f / terrain.pWorld->GetWorldHeight(), 1.0f);
	else
		D3DXMatrixScaling(&g_mFowProj, 1.0f, 1.0f, 1.0f);
}


/*====================
  D3D_RenderScene
  ====================*/
void	D3D_RenderScene(CCamera &camera)
{
	PROFILE("D3D_RenderScene");

	D3D_FrameShader();

	if (g_iNumGuiQuads > 0)
	{
		D3D_Setup2D();
		D3D_DrawQuads();
	}

	if (!g_bValidScene)
	{
		D3D_Setup2D();
		return;
	}

	D3D_SetupScene();

	g_vCamOrigin = camera.GetOrigin();
	g_bShadows = g_bCamShadows = g_Shadowmap.GetActive() && !(camera.GetFlags() & CAM_NO_SHADOWS);
	g_bFogofWar = !!(camera.GetFlags() & CAM_FOG_OF_WAR);
	g_bFog = g_bCamFog = !(camera.GetFlags() & CAM_NO_FOG);

	D3D_AddParticleSystemSceneEntities((camera.GetFlags() & CAM_NO_CULL) == 0);

	D3D_SetCloudProj(camera.GetTime());
	D3D_SetFowProj();
	CTreeModel::SetTime(camera.GetTime());

	D3D_FlagVisibleTerrainChunks();

	//
	// Shadow phase setup
	//

	if (g_Shadowmap.GetActive() && !(camera.GetFlags() & CAM_NO_SHADOWS))
	{
		// This must be done before D3D_AddSceneEntities so that
		// entities are properly culled out of the shadow map

		g_Shadowmap.Setup(camera);
	}

	g_pCam = &camera;

	// Populate render list
	D3D_AddSceneEntities();

	if (!camera.HasFlags(CAM_NO_TERRAIN))
		D3D_AddTerrainChunks();
	
	D3D_AddTreeBillboardBatches(PHASE_COLOR);

	//
	// Shadow Phase
	//

	if (g_Shadowmap.GetActive() && !(camera.GetFlags() & CAM_NO_SHADOWS))
	{
		g_Shadowmap.Render();
	}

	g_bInvertedProjection = false;

	// Initialize the scene: setup the camera, set some global vars
	D3D_SetupCamera(camera);

	// Build up effects pools for batching
	D3D_AddParticleSystems();
	D3D_AddFoliageChunks();
	D3D_AddScenePolys(PHASE_COLOR);

	D3D_AddSky();

	{
		PROFILE("Add Misc Batches");

		D3D_AddEffectQuadBatches(PHASE_COLOR);
		D3D_AddEffectTriangleBatches(PHASE_COLOR);
		D3D_AddExtendedTriangleBatches(PHASE_COLOR);
		D3D_AddBoxBatches();
		D3D_AddPointBatches();
		D3D_AddLineBatches();
	}

	//
	// Reflection Phase
	//

	if (g_ReflectionMap.GetActive() && !(camera.GetFlags() & CAM_NO_REFLECTIONS))
	{
		g_ReflectionMap.Render(camera);
	}

	//
	// Velocity Phase
	//

	if (g_VelocityMap.GetActive())
	{
		g_VelocityMap.Render(camera);
	}

	if (!(camera.GetFlags() & CAM_NO_DEPTH_CLEAR))
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

	//
	// Depth Phase
	//

	if (gfx_depthFirst)
	{
		PROFILE("Depth Phase");

		// Disable color writes
		D3D_SetRenderState(D3DRS_COLORWRITEENABLE, 0);

		g_RenderList.Setup(PHASE_DEPTH);
		g_RenderList.Sort();
		g_RenderList.Render(PHASE_DEPTH);

		// Re-enable color writes
		D3D_SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
	}


	//
	// Color Phase
	//

	{
		PROFILE("Color Phase");

		// Initialize the scene: setup the camera, set some global vars
		D3D_SetupCamera(camera);

		g_RenderList.Setup(PHASE_COLOR);
		g_RenderList.Sort();
		g_RenderList.Render(PHASE_COLOR);

		if (!camera.HasFlags(CAM_NO_POST))
			g_PostBuffer.Render();

		D3D_CleanupScene();
	}
}


/*====================
  D3D_RenderFogofWar
  ====================*/
void	D3D_RenderFogofWar(float fClear, bool bTexture, float fLerp)
{
	g_FogofWar.Render(fClear, bTexture, fLerp);
}


/*====================
  D3D_UpdateFogofWar
  ====================*/
void	D3D_UpdateFogofWar(const CBitmap &cBmp)
{
	g_FogofWar.Update(cBmp);
}


#if 0
/*====================
  D3D_DrawQuads

  Draw all queued gui quads and clear the queue
  ====================*/
void	D3D_DrawQuads()
{
	if (g_iNumGuiQuads == 0 || !g_bValidScene || !g_pVBGuiQuad)
		return;

	PROFILE("D3D_DrawQuads");

	SGuiVertex* pVertices;
	int i;
	static DWORD dwStaticBase(0);
		
	if (dwStaticBase + g_iNumGuiQuads >= MAX_GUIQUADS)
		dwStaticBase = 0;

	DWORD dwBase(dwStaticBase);

	D3D_SetStreamSource(0, g_pVBGuiQuad, 0, sizeof(SGuiVertex));
	D3D_SetIndices(g_pIBGuiQuad);

	D3D_SelectMaterial(g_MaterialGUI, PHASE_COLOR, VERTEX_GUI, SceneManager.GetShaderTime(), false);

	ResHandle hWhiteTexture(g_ResourceManager.GetWhiteTexture());
	CTexture *pWhiteTexture(g_ResourceManager.GetTexture(hWhiteTexture));
	int iWhiteTextureFlags(pWhiteTexture != NULL ? pWhiteTexture->GetTextureFlags() : 0);

	CMaterial *pMaterial(NULL);
	int iPrevFlags(0);

	for (i = 0; i < g_iNumGuiQuads; ++i)
	{
		SGuiQuad &quad = g_GuiQuads[i];

		if (quad.eType == GUI_LINE)
		{
			int f;
			int iFlags(quad.iFlags);

			for (f = i + 1; f < g_iNumGuiQuads && g_GuiQuads[f].eType == GUI_LINE && g_GuiQuads[f].iFlags == iFlags; ++f);

			int iBatchSize(f - i);

			if (FAILED(g_pVBGuiQuad->Lock(dwBase * 4 * sizeof(SGuiVertex), iBatchSize * 4 * sizeof(SGuiVertex), (void**)&pVertices, dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
				return;

			for (int v = 0; v < iBatchSize; ++v)
			{
				SGuiQuad &quad = g_GuiQuads[i + v];

				pVertices->x = quad.x[0];
				pVertices->y = quad.y[0];
				pVertices->z = 0.0f;
				pVertices->color = D3DCOLOR_ARGB(
					CLAMP(INT_ROUND(quad.s[A] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.s[R] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.s[G] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.s[B] * 255.0f), 0, 255)
				);
				pVertices->tu = 0.0f;
				pVertices->tv = 0.0f;
				++pVertices;

				pVertices->x = quad.x[1];
				pVertices->y = quad.y[1];
				pVertices->z = 0.0f;
				pVertices->color = D3DCOLOR_ARGB(
					CLAMP(INT_ROUND(quad.t[A] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.t[R] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.t[G] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.t[B] * 255.0f), 0, 255)
				);
				pVertices->tu = 0.0f;
				pVertices->tv = 0.0f;
				++pVertices;

				pVertices->x = quad.x[1];
				pVertices->y = quad.y[1];
				pVertices->z = 0.0f;
				pVertices->color = D3DCOLOR_ARGB(
					CLAMP(INT_ROUND(quad.t[A] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.t[R] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.t[G] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.t[B] * 255.0f), 0, 255)
				);
				pVertices->tu = 0.0f;
				pVertices->tv = 0.0f;
				++pVertices;

				pVertices->x = quad.x[1];
				pVertices->y = quad.y[1];
				pVertices->z = 0.0f;
				pVertices->color = D3DCOLOR_ARGB(
					CLAMP(INT_ROUND(quad.t[A] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.t[R] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.t[G] * 255.0f), 0, 255),
					CLAMP(INT_ROUND(quad.t[B] * 255.0f), 0, 255)
				);
				pVertices->tu = 0.0f;
				pVertices->tv = 0.0f;
				++pVertices;
			}

			g_pVBGuiQuad->Unlock();

			CMaterial &material((iFlags & GUI_GRAYSCALE) ? g_MaterialGUIGrayScale : g_MaterialGUI);
		
			if (pMaterial != &material)
			{
				CMaterialPhase &phase(material.GetPhase(PHASE_COLOR));
				CMaterialSampler &sampler(phase.GetSampler(0));

				sampler.SetTexture(hWhiteTexture);

				phase.SetCullMode(CULL_BACK);

				if (iFlags & GUI_ADDITIVE)
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE);
				}
				else if (iFlags & GUI_OVERLAY)
				{
					phase.SetSrcBlend(BLEND_DEST_COLOR);
					phase.SetDstBlend(BLEND_SRC_COLOR);
				}
				else if (iFlags & GUI_FOG)
				{
					phase.SetSrcBlend(BLEND_ZERO);
					phase.SetDstBlend(BLEND_SRC_ALPHA);
				}
				else
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE_MINUS_SRC_ALPHA);
				}

				if (iFlags & GUI_TILE_U)
					sampler.AddFlags(SAM_REPEAT_U);
				else
					sampler.ClearFlags(SAM_REPEAT_U);

				if (iFlags & GUI_TILE_V)
					sampler.AddFlags(SAM_REPEAT_V);
				else
					sampler.ClearFlags(SAM_REPEAT_V);

				D3D_SelectPixelShader(material, PHASE_COLOR, SceneManager.GetShaderTime());
				pMaterial = &material;
				iPrevFlags = iFlags;
			}
			else
			{
				int iStageIndex(D3D_GetSamplerStageIndex(IMAGE));

				D3D_UpdateShaderTexture(iStageIndex, hWhiteTexture);

				if (iFlags & GUI_ADDITIVE)
				{
					D3D_SetSrcBlendMode(BLEND_SRC_ALPHA);
					D3D_SetDstBlendMode(BLEND_ONE);
				}
				else if (iFlags & GUI_OVERLAY)
				{
					D3D_SetSrcBlendMode(BLEND_DEST_COLOR);
					D3D_SetDstBlendMode(BLEND_SRC_COLOR);
				}
				else if (iFlags & GUI_FOG)
				{
					D3D_SetSrcBlendMode(BLEND_ZERO);
					D3D_SetDstBlendMode(BLEND_SRC_ALPHA);
				}
				else
				{
					D3D_SetSrcBlendMode(BLEND_SRC_ALPHA);
					D3D_SetDstBlendMode(BLEND_ONE_MINUS_SRC_ALPHA);
				}

				if (iFlags & GUI_TILE_U)
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				else
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

				if (iFlags & GUI_TILE_V)
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
				else
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

				// Setup texture mipmaps
				if (iWhiteTextureFlags & TEX_NO_MIPMAPS)
				{
					D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
				}
				else
				{
					if (vid_textureFiltering >= TEXTUREFILTERING_ANISOTROPIC2)
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					else if (vid_textureFiltering == TEXTUREFILTERING_TRILINEAR)
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					else
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
				}
			}

			D3D_DrawIndexedPrimitive(D3DPT_LINELIST, 0, (dwStaticBase + i) * 4, iBatchSize << 2, (dwStaticBase + i) * 6, iBatchSize * 3);

			dwBase += iBatchSize;

			i += iBatchSize - 1;
		}
		else
		{
			ResHandle hTexture(quad.hTexture);
			int iFlags(quad.iFlags);
			int f;

			for (f = i + 1; f < g_iNumGuiQuads && g_GuiQuads[f].eType != GUI_LINE && g_GuiQuads[f].hTexture == hTexture && g_GuiQuads[f].iFlags == iFlags; ++f);

			int iBatchSize(f - i);

			if (FAILED(g_pVBGuiQuad->Lock(dwBase * 4 * sizeof(SGuiVertex), iBatchSize * 4 * sizeof(SGuiVertex), (void**)&pVertices, dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
				return;

			for (int v = 0; v < iBatchSize; ++v)
			{
				SGuiQuad &quad = g_GuiQuads[i + v];

				if (quad.eType == GUI_RECT)
				{
					pVertices->x = quad.x[0];
					pVertices->y = quad.y[0];
					pVertices->z = 0.0f;
					pVertices->color = quad.color;
					pVertices->tu = quad.s[0];
					pVertices->tv = quad.t[1];
					++pVertices;

					pVertices->x = quad.x[0];
					pVertices->y = quad.y[0] + quad.h;
					pVertices->z = 0.0f;
					pVertices->color = quad.color;
					pVertices->tu = quad.s[0];
					pVertices->tv = quad.t[0];
					++pVertices;

					pVertices->x = quad.x[0] + quad.w;
					pVertices->y = quad.y[0] + quad.h;
					pVertices->z = 0.0f;
					pVertices->color = quad.color;
					pVertices->tu = quad.s[1];
					pVertices->tv = quad.t[0];
					++pVertices;

					pVertices->x = quad.x[0] + quad.w;
					pVertices->y = quad.y[0];
					pVertices->z = 0.0f;
					pVertices->color = quad.color;
					pVertices->tu = quad.s[1];
					pVertices->tv = quad.t[1];
					++pVertices;
				}
				else
				{
					pVertices->x = quad.x[0];
					pVertices->y = quad.y[0];
					pVertices->z = 0.0f;
					pVertices->color = quad.color;
					pVertices->tu = quad.s[0];
					pVertices->tv = quad.t[0];
					++pVertices;

					pVertices->x = quad.x[1];
					pVertices->y = quad.y[1];
					pVertices->z = 0.0f;
					pVertices->color = quad.color;
					pVertices->tu = quad.s[1];
					pVertices->tv = quad.t[1];
					++pVertices;

					pVertices->x = quad.x[2];
					pVertices->y = quad.y[2];
					pVertices->z = 0.0f;
					pVertices->color = quad.color;
					pVertices->tu = quad.s[2];
					pVertices->tv = quad.t[2];
					++pVertices;

					pVertices->x = quad.x[3];
					pVertices->y = quad.y[3];
					pVertices->z = 0.0f;
					pVertices->color = quad.color;
					pVertices->tu = quad.s[3];
					pVertices->tv = quad.t[3];
					++pVertices;
				}
			}

			g_pVBGuiQuad->Unlock();

			CMaterial &material((iFlags & GUI_GRAYSCALE) ? g_MaterialGUIGrayScale : (iFlags & GUI_BLUR) ? g_MaterialGUIBlur : g_MaterialGUI);

			if (pMaterial != &material)
			{
				CMaterialPhase &phase(material.GetPhase(PHASE_COLOR));
				CMaterialSampler &sampler(phase.GetSampler(0));

				sampler.SetTexture(hTexture);

				phase.SetCullMode(CULL_BACK);

				if (iFlags & GUI_ADDITIVE)
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE);
				}
				else if (iFlags & GUI_OVERLAY)
				{
					phase.SetSrcBlend(BLEND_DEST_COLOR);
					phase.SetDstBlend(BLEND_SRC_COLOR);
				}
				else if (iFlags & GUI_FOG)
				{
					phase.SetSrcBlend(BLEND_ZERO);
					phase.SetDstBlend(BLEND_SRC_ALPHA);
				}
				else
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE_MINUS_SRC_ALPHA);
				}

				if (iFlags & GUI_TILE_U)
					sampler.AddFlags(SAM_REPEAT_U);
				else
					sampler.ClearFlags(SAM_REPEAT_U);

				if (iFlags & GUI_TILE_V)
					sampler.AddFlags(SAM_REPEAT_V);
				else
					sampler.ClearFlags(SAM_REPEAT_V);

				D3D_SelectPixelShader(material, PHASE_COLOR, SceneManager.GetShaderTime());
				pMaterial = &material;
				iPrevFlags = iFlags;
			}
			else
			{
				int iStageIndex(D3D_GetSamplerStageIndex(IMAGE));

				D3D_UpdateShaderTexture(iStageIndex, hTexture);

				CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));
	
				int iTextureFlags(pTexture != NULL ? pTexture->GetTextureFlags() : 0);

				if (iFlags & GUI_ADDITIVE)
				{
					D3D_SetSrcBlendMode(BLEND_SRC_ALPHA);
					D3D_SetDstBlendMode(BLEND_ONE);
				}
				else if (iFlags & GUI_OVERLAY)
				{
					D3D_SetSrcBlendMode(BLEND_DEST_COLOR);
					D3D_SetDstBlendMode(BLEND_SRC_COLOR);
				}
				else if (iFlags & GUI_FOG)
				{
					D3D_SetSrcBlendMode(BLEND_ZERO);
					D3D_SetDstBlendMode(BLEND_SRC_ALPHA);
				}
				else
				{
					D3D_SetSrcBlendMode(BLEND_SRC_ALPHA);
					D3D_SetDstBlendMode(BLEND_ONE_MINUS_SRC_ALPHA);
				}

				if (iFlags & GUI_TILE_U)
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				else
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

				if (iFlags & GUI_TILE_V)
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
				else
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

				// Setup texture mipmaps
				if (iTextureFlags & TEX_NO_MIPMAPS)
				{
					D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
				}
				else
				{
					if (vid_textureFiltering >= TEXTUREFILTERING_ANISOTROPIC2)
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					else if (vid_textureFiltering == TEXTUREFILTERING_TRILINEAR)
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					else
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
				}
			}

			D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (dwStaticBase + i) * 4, iBatchSize << 2, (dwStaticBase + i) * 6, iBatchSize << 1);

			dwBase += iBatchSize;

			i += iBatchSize - 1;
		}
	}

	g_iNumGuiQuads = 0;
	dwStaticBase += dwBase;
}
#else
/*====================
  D3D_DrawQuads

  Draw all queued gui quads and clear the queue
  ====================*/
void	D3D_DrawQuads()
{
	if (g_iNumGuiQuads == 0 || !g_bValidScene || !g_pVBGuiQuad)
		return;

	PROFILE("D3D_DrawQuads");

	SGuiVertex* pVertices;
	int i;

	if (!d3d_crossFrameCycleBuffer || g_dwGuiQuadBase + g_iNumGuiQuads >= MAX_GUIQUADS)
		g_dwGuiQuadBase = 0;

	if (FAILED(g_pVBGuiQuad->Lock(g_dwGuiQuadBase * 4 * sizeof(SGuiVertex), g_iNumGuiQuads * 4 * sizeof(SGuiVertex), (void**)&pVertices, g_dwGuiQuadBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
		return;

	for (i = 0; i < g_iNumGuiQuads; ++i)
	{
		SGuiQuad &quad = g_GuiQuads[i];

		switch (quad.eType)
		{
		case GUI_RECT:
			pVertices->x = quad.x[0];
			pVertices->y = quad.y[0];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[0];
			pVertices->tv = quad.t[1];
			++pVertices;

			pVertices->x = quad.x[0];
			pVertices->y = quad.y[0] + quad.h;
			pVertices->color = quad.color;
			pVertices->tu = quad.s[0];
			pVertices->tv = quad.t[0];
			++pVertices;

			pVertices->x = quad.x[0] + quad.w;
			pVertices->y = quad.y[0] + quad.h;
			pVertices->color = quad.color;
			pVertices->tu = quad.s[1];
			pVertices->tv = quad.t[0];
			++pVertices;

			pVertices->x = quad.x[0] + quad.w;
			pVertices->y = quad.y[0];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[1];
			pVertices->tv = quad.t[1];
			++pVertices;
			break;
		case GUI_QUAD:
			pVertices->x = quad.x[0];
			pVertices->y = quad.y[0];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[0];
			pVertices->tv = quad.t[0];
			++pVertices;

			pVertices->x = quad.x[1];
			pVertices->y = quad.y[1];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[1];
			pVertices->tv = quad.t[1];
			++pVertices;

			pVertices->x = quad.x[2];
			pVertices->y = quad.y[2];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[2];
			pVertices->tv = quad.t[2];
			++pVertices;

			pVertices->x = quad.x[3];
			pVertices->y = quad.y[3];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[3];
			pVertices->tv = quad.t[3];
			++pVertices;
			break;
		case GUI_LINE:
			pVertices->x = quad.x[0];
			pVertices->y = quad.y[0];
			pVertices->color = D3DCOLOR_ARGB(
				CLAMP(INT_ROUND(quad.s[A] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.s[R] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.s[G] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.s[B] * 255.0f), 0, 255)
			);
			pVertices->tu = 0.0f;
			pVertices->tv = 0.0f;
			++pVertices;

			pVertices->x = quad.x[1];
			pVertices->y = quad.y[1];
			pVertices->color = D3DCOLOR_ARGB(
				CLAMP(INT_ROUND(quad.t[A] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.t[R] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.t[G] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.t[B] * 255.0f), 0, 255)
			);
			pVertices->tu = 0.0f;
			pVertices->tv = 0.0f;
			++pVertices;

			pVertices->x = quad.x[1];
			pVertices->y = quad.y[1];
			pVertices->color = D3DCOLOR_ARGB(
				CLAMP(INT_ROUND(quad.t[A] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.t[R] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.t[G] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.t[B] * 255.0f), 0, 255)
			);
			pVertices->tu = 0.0f;
			pVertices->tv = 0.0f;
			++pVertices;

			pVertices->x = quad.x[1];
			pVertices->y = quad.y[1];
			pVertices->color = D3DCOLOR_ARGB(
				CLAMP(INT_ROUND(quad.t[A] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.t[R] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.t[G] * 255.0f), 0, 255),
				CLAMP(INT_ROUND(quad.t[B] * 255.0f), 0, 255)
			);
			pVertices->tu = 0.0f;
			pVertices->tv = 0.0f;
			++pVertices;
			break;
		}
	}

	g_pVBGuiQuad->Unlock();

	D3D_SetStreamSource(0, g_pVBGuiQuad, 0, sizeof(SGuiVertex));
	D3D_SetIndices(g_pIBGuiQuad);

	D3D_SelectMaterial(g_MaterialGUI, PHASE_COLOR, VERTEX_GUI, SceneManager.GetShaderTime(), false);

	ResHandle hWhiteTexture(g_ResourceManager.GetWhiteTexture());
	CTexture *pWhiteTexture(g_ResourceManager.GetTexture(hWhiteTexture));
	int iWhiteTextureFlags(pWhiteTexture != NULL ? pWhiteTexture->GetTextureFlags() : 0);

	CMaterial *pMaterial(NULL);
	int iStageIndex(0);

	for (i = 0; i < g_iNumGuiQuads; ++i)
	{
		SGuiQuad &quad = g_GuiQuads[i];

		if (quad.eType == GUI_LINE)
		{
			int f;
			int iFlags(quad.iFlags);

			for (f = i + 1; f < g_iNumGuiQuads && g_GuiQuads[f].eType == GUI_LINE && g_GuiQuads[f].iFlags == iFlags; ++f);

			CMaterial &material((iFlags & GUI_GRAYSCALE) ? g_MaterialGUIGrayScale : g_MaterialGUI);
		
			if (pMaterial != &material)
			{
				CMaterialPhase &phase(material.GetPhase(PHASE_COLOR));
				CMaterialSampler &sampler(phase.GetSampler(0));

				sampler.SetTexture(hWhiteTexture);

				phase.SetCullMode(CULL_BACK);

				if (iFlags & GUI_ADDITIVE)
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE);
				}
				else if (iFlags & GUI_OVERLAY)
				{
					phase.SetSrcBlend(BLEND_DEST_COLOR);
					phase.SetDstBlend(BLEND_SRC_COLOR);
				}
				else if (iFlags & GUI_FOG)
				{
					phase.SetSrcBlend(BLEND_ZERO);
					phase.SetDstBlend(BLEND_SRC_ALPHA);
				}
				else
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE_MINUS_SRC_ALPHA);
				}

				if (iFlags & GUI_TILE_U)
					sampler.AddFlags(SAM_REPEAT_U);
				else
					sampler.ClearFlags(SAM_REPEAT_U);

				if (iFlags & GUI_TILE_V)
					sampler.AddFlags(SAM_REPEAT_V);
				else
					sampler.ClearFlags(SAM_REPEAT_V);

				D3D_SelectPixelShader(material, PHASE_COLOR, SceneManager.GetShaderTime());
				pMaterial = &material;
				iStageIndex = D3D_GetSamplerStageIndex(IMAGE);
			}
			else
			{
				D3D_UpdateShaderTexture(iStageIndex, hWhiteTexture);

				if (iFlags & GUI_ADDITIVE)
				{
					D3D_SetSrcBlendMode(BLEND_SRC_ALPHA);
					D3D_SetDstBlendMode(BLEND_ONE);
				}
				else if (iFlags & GUI_OVERLAY)
				{
					D3D_SetSrcBlendMode(BLEND_DEST_COLOR);
					D3D_SetDstBlendMode(BLEND_SRC_COLOR);
				}
				else if (iFlags & GUI_FOG)
				{
					D3D_SetSrcBlendMode(BLEND_ZERO);
					D3D_SetDstBlendMode(BLEND_SRC_ALPHA);
				}
				else
				{
					D3D_SetSrcBlendMode(BLEND_SRC_ALPHA);
					D3D_SetDstBlendMode(BLEND_ONE_MINUS_SRC_ALPHA);
				}

				if (iFlags & GUI_TILE_U)
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				else
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

				if (iFlags & GUI_TILE_V)
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
				else
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

				// Setup texture mipmaps
				if (iWhiteTextureFlags & TEX_NO_MIPMAPS)
				{
					D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
				}
				else
				{
					if (vid_textureFiltering >= TEXTUREFILTERING_ANISOTROPIC2)
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					else if (vid_textureFiltering == TEXTUREFILTERING_TRILINEAR)
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					else
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
				}
			}

			D3D_DrawIndexedPrimitive(D3DPT_LINELIST, 0, (g_dwGuiQuadBase + i) * 4, (f - i) << 2, (g_dwGuiQuadBase + i) * 6, (f - i) * 3);

			i += (f - i) - 1;
		}
		else
		{
			ResHandle hTexture(quad.hTexture);
			int iFlags(quad.iFlags);
			int f;
			
			for (f = i + 1; f < g_iNumGuiQuads && g_GuiQuads[f].eType != GUI_LINE && g_GuiQuads[f].hTexture == hTexture && g_GuiQuads[f].iFlags == iFlags; ++f);

			CMaterial &material((iFlags & GUI_GRAYSCALE) ? g_MaterialGUIGrayScale : (iFlags & GUI_BLUR) ? g_MaterialGUIBlur : g_MaterialGUI);

			if (pMaterial != &material)
			{
				CMaterialPhase &phase(material.GetPhase(PHASE_COLOR));
				CMaterialSampler &sampler(phase.GetSampler(0));

				sampler.SetTexture(hTexture);

				phase.SetCullMode(CULL_BACK);

				if (iFlags & GUI_ADDITIVE)
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE);
				}
				else if (iFlags & GUI_OVERLAY)
				{
					phase.SetSrcBlend(BLEND_DEST_COLOR);
					phase.SetDstBlend(BLEND_SRC_COLOR);
				}
				else if (iFlags & GUI_FOG)
				{
					phase.SetSrcBlend(BLEND_ZERO);
					phase.SetDstBlend(BLEND_SRC_ALPHA);
				}
				else
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE_MINUS_SRC_ALPHA);
				}

				if (iFlags & GUI_TILE_U)
					sampler.AddFlags(SAM_REPEAT_U);
				else
					sampler.ClearFlags(SAM_REPEAT_U);

				if (iFlags & GUI_TILE_V)
					sampler.AddFlags(SAM_REPEAT_V);
				else
					sampler.ClearFlags(SAM_REPEAT_V);

				D3D_SelectPixelShader(material, PHASE_COLOR, SceneManager.GetShaderTime());
				pMaterial = &material;
				iStageIndex = D3D_GetSamplerStageIndex(IMAGE);
			}
			else
			{
				D3D_UpdateShaderTexture(iStageIndex, hTexture);

				CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));
	
				int iTextureFlags(pTexture != NULL ? pTexture->GetTextureFlags() : 0);

				if (iFlags & GUI_ADDITIVE)
				{
					D3D_SetSrcBlendMode(BLEND_SRC_ALPHA);
					D3D_SetDstBlendMode(BLEND_ONE);
				}
				else if (iFlags & GUI_OVERLAY)
				{
					D3D_SetSrcBlendMode(BLEND_DEST_COLOR);
					D3D_SetDstBlendMode(BLEND_SRC_COLOR);
				}
				else if (iFlags & GUI_FOG)
				{
					D3D_SetSrcBlendMode(BLEND_ZERO);
					D3D_SetDstBlendMode(BLEND_SRC_ALPHA);
				}
				else
				{
					D3D_SetSrcBlendMode(BLEND_SRC_ALPHA);
					D3D_SetDstBlendMode(BLEND_ONE_MINUS_SRC_ALPHA);
				}

				if (iFlags & GUI_TILE_U)
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				else
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

				if (iFlags & GUI_TILE_V)
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
				else
					D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

				// Setup texture mipmaps
				if (iTextureFlags & TEX_NO_MIPMAPS)
				{
					D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
				}
				else
				{
					if (vid_textureFiltering >= TEXTUREFILTERING_ANISOTROPIC2)
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					else if (vid_textureFiltering == TEXTUREFILTERING_TRILINEAR)
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					else
						D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
				}
			}

			D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (g_dwGuiQuadBase + i) * 4, (f - i) << 2, (g_dwGuiQuadBase + i) * 6, (f - i) << 1);

			i += (f - i) - 1;
		}
	}

	g_dwGuiQuadBase += g_iNumGuiQuads;
	g_iNumGuiQuads = 0;
}
#endif