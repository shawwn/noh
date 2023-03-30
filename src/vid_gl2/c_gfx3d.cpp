// (C)2008 S2 Games
// c_gfx3d.cpp
//
// 3D Rendering
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfx3d.h"

#include "c_gfxmaterials.h"
#include "c_gfxutils.h"
#include "c_gfxterrain.h"
#include "c_gfxshaders.h"
#include "c_shadowmap.h"
#include "c_treescenemanager.h"
#include "c_renderlist.h"
#include "c_postbuffer.h"

#include "../k2/c_convexpolygon2.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
SINGLETON_INIT(CGfx3D)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGfx3D *Gfx3D(CGfx3D::GetInstance());

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
D3DXMATRIXA16			g_mCloudProj;
D3DXMATRIXA16			g_mFowProj;
uint					g_uiImageWidth;
uint					g_uiImageHeight;
float					g_fSceneScaleX;
float					g_fSceneScaleY;

bool					g_bInvertedProjection;

int					g_iMaxDynamicLights;
const CSceneEntity	*g_pCurrentEntity;

extern bool				g_bValidScene;
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOL		(gfx_infiniteFrustum,			false);
CVAR_BOOL		(gfx_sky,						true);

CVAR_BOOLF		(gfx_foliage,			true,							CVAR_SAVECONFIG);

CVAR_BOOLF		(gfx_lighting,			true,							CONEL_DEV);
CVAR_INTF		(gfx_textures,			0,								CONEL_DEV);
CVAR_INTF		(gfx_wireframe,			0,								CONEL_DEV);
CVAR_INTF		(gfx_points,			0,								CONEL_DEV);
CVAR_INTF		(gfx_normals,			0,								CONEL_DEV);

CVAR_VEC3F		(gfx_fogColor,			CVec3f(0.75f, 0.75f, 0.75f),	CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_fogNear,			4500.0f,						CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_fogFar,			9000.0f,						CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_fogDensity,		0.0005f,						CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_fogScale,			1.0f,							CVAR_WORLDCONFIG);
CVAR_INTF		(gfx_fogType,			1,								CVAR_WORLDCONFIG);

CVAR_BOOLF		(gfx_clouds,			false,							CVAR_WORLDCONFIG);
CVAR_STRINGF	(gfx_cloudTexture,		"/world/sky/cloud_shadows/cloud_shadows1.tga", CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_cloudScale,		200.0f,							CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_cloudSpeedX,		100.0f,							CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_cloudSpeedY,		50.0f,							CVAR_WORLDCONFIG);

CVAR_BOOLF		(vid_drawParticleSystems,	true,						CONEL_DEV);

CVAR_BOOLF		(vid_dynamicLights,			true,						CVAR_SAVECONFIG);
CVAR_INTF		(vid_maxDynamicLights,		4,							CVAR_SAVECONFIG);

CVAR_FLOAT		(vid_skyEpsilon,				0.001f);
//=============================================================================

/*====================
  CGfx3D::~CGfx3D
  ====================*/
CGfx3D::~CGfx3D()
{
}


/*====================
  CGfx3D::CGfx3D
  ====================*/
CGfx3D::CGfx3D() :
VBScenePoly(0),
VBBillboard(0),
VBEffectTriangle(0),
VBTreeBillboard(0),
IBTreeBillboard(0),
VBBox(0),
IBBox(0),
VBPoint(0),
VBLine(0),

iNumBillboards(0),
iNumBeams(0),
iNumEffectTriangles(0),
iNumTreeBillboards(0),
iNumBoxes(0),
iNumPoints(0),
iNumLines(0)
{
}


/*====================
  CGfx3D::Draw
  ====================*/
void	CGfx3D::Draw(CCamera &newCamera)
{
	Camera = newCamera;
	g_pCam = &Camera;

	g_vCamOrigin = newCamera.GetOrigin();
	g_bShadows = g_bCamShadows = g_Shadowmap.GetActive() && !(newCamera.GetFlags() & CAM_NO_SHADOWS);
	g_bFogofWar = g_bCamFogofWar = !!(newCamera.GetFlags() & CAM_FOG_OF_WAR);
	g_bFog = g_bCamFog = !(newCamera.GetFlags() & CAM_NO_FOG);

	Setup3D();

	SetCloudProj(newCamera.GetTime());
	SetFowProj();
	CTreeModel::SetTime(newCamera.GetTime());

	AddParticleSystemSceneEntities((newCamera.GetFlags() & CAM_NO_CULL) == 0);

	PRINT_GLERROR_BREAK();

	//
	// Shadow Phase
	//

	if (g_bShadows)
	{
		PROFILE("Shadow Phase");
		g_Shadowmap.Render(newCamera);
	}

	PRINT_GLERROR_BREAK();

	SetupCamera(newCamera);

	g_bInvertedProjection = false;
	
	//
	// Color Phase
	//
	AddParticleSystems();
	AddWorld(PHASE_COLOR);
	AddSky();

	AddBillboardBatches(PHASE_COLOR);
	AddEffectTriangleBatches(PHASE_COLOR);
	AddTreeBillboardBatches(PHASE_COLOR);
	GL_AddFoliageChunks();
	AddScenePolys(PHASE_COLOR);
	AddBoxBatches();
	AddPointBatches();
	AddLineBatches();

	g_RenderList.Setup(PHASE_COLOR);
	g_RenderList.Sort();
	g_RenderList.Render(PHASE_COLOR);
	g_RenderList.Clear();

	PRINT_GLERROR_BREAK();

	Exit3D();

	if (!newCamera.HasFlags(CAM_NO_POST))
		g_PostBuffer.Render();
}


/*====================
  CGfx3D::Clear
  ====================*/
void	CGfx3D::Clear()
{
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glEnable(GL_BLEND);

	glClearColor(GfxUtils->GetCurrentColor(0), GfxUtils->GetCurrentColor(1), GfxUtils->GetCurrentColor(2), GfxUtils->GetCurrentColor(3));
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


/*====================
  CGfx3D::Init
  ====================*/
void	CGfx3D::Init()
{
	ushort *pIndices16;

	//
	// Scenepoly buffer
	//

	glGenBuffersARB(1, &VBScenePoly);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBScenePoly);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 1024 * sizeof(SEffectVertex), NULL, GL_STREAM_DRAW_ARB);

	//
	// Billboard buffers
	//

	glGenBuffersARB(1, &VBBillboard);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBBillboard);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_BILLBOARDS * 4 * sizeof(SEffectVertex), NULL, GL_STREAM_DRAW_ARB);

	//
	// Effect triangle buffer
	//

	glGenBuffersARB(1, &VBEffectTriangle);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBEffectTriangle);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_EFFECT_TRIANGLES * 3 * sizeof(SEffectVertex), NULL, GL_STREAM_DRAW_ARB);

	//
	// Tree Billboard buffers
	//

	glGenBuffersARB(1, &VBTreeBillboard);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBTreeBillboard);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_TREE_BILLBOARDS * 4 * sizeof(STreeBillboardVertex), NULL, GL_STREAM_DRAW_ARB);

	glGenBuffersARB(1, &IBTreeBillboard);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, IBTreeBillboard);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, MAX_TREE_BILLBOARDS * 6 * sizeof(ushort), NULL, GL_STATIC_DRAW_ARB);

	pIndices16 = (ushort*)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_ARB);
	for (int n = 0, i = 0; n < MAX_TREE_BILLBOARDS; ++n)
	{
		pIndices16[i++] = (n<<2);
		pIndices16[i++] = (n<<2) + 1;
		pIndices16[i++] = (n<<2) + 2;
		pIndices16[i++] = (n<<2);
		pIndices16[i++] = (n<<2) + 2;
		pIndices16[i++] = (n<<2) + 3;
	}
	glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER);

	//
	// Box buffers
	//

	glGenBuffersARB(1, &VBBox);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBBox);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_BOXES * 8 * sizeof(SLineVertex), NULL, GL_STREAM_DRAW_ARB);

	glGenBuffersARB(1, &IBBox);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, IBBox);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, MAX_BOXES * 24 * sizeof(GLushort), NULL, GL_STATIC_DRAW_ARB);

	pIndices16 = (GLushort *)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_ARB);
	for (int n = 0, i = 0; n < MAX_BOXES; ++n)
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
	glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER);

	//
	// Point buffer
	//

	glGenBuffersARB(1, &VBPoint);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBPoint);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_POINTS * sizeof(SLineVertex), NULL, GL_STREAM_DRAW_ARB);

	//
	// Line buffer
	//

	glGenBuffersARB(1, &VBLine);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBLine);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_LINES * 2 * sizeof(SLineVertex), NULL, GL_STREAM_DRAW_ARB);
}


/*====================
  CGfx3D::AddBoxBatches
  ====================*/
void	CGfx3D::AddBoxBatches()
{
	if (iNumBoxes == 0)
		return;

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBBox);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, iNumBoxes * 8 * sizeof(SLineVertex), NULL, GL_STREAM_DRAW_ARB);
	SLineVertex *pVertices = (SLineVertex *)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);

	vector<CVec3f> vPoints(8);

	for (int n = 0; n < iNumBoxes; ++n)
	{
		vPoints[0].x = Boxes[n].bbBox.GetMin()[0];
		vPoints[0].y = Boxes[n].bbBox.GetMin()[1];
		vPoints[0].z = Boxes[n].bbBox.GetMax()[2];

		vPoints[1].x = Boxes[n].bbBox.GetMin()[0];
		vPoints[1].y = Boxes[n].bbBox.GetMax()[1];
		vPoints[1].z = Boxes[n].bbBox.GetMax()[2];

		vPoints[2].x = Boxes[n].bbBox.GetMax()[0];
		vPoints[2].y = Boxes[n].bbBox.GetMax()[1];
		vPoints[2].z = Boxes[n].bbBox.GetMax()[2];

		vPoints[3].x = Boxes[n].bbBox.GetMax()[0];
		vPoints[3].y = Boxes[n].bbBox.GetMin()[1];
		vPoints[3].z = Boxes[n].bbBox.GetMax()[2];

		vPoints[4].x = Boxes[n].bbBox.GetMin()[0];
		vPoints[4].y = Boxes[n].bbBox.GetMin()[1];
		vPoints[4].z = Boxes[n].bbBox.GetMin()[2];

		vPoints[5].x = Boxes[n].bbBox.GetMin()[0];
		vPoints[5].y = Boxes[n].bbBox.GetMax()[1];
		vPoints[5].z = Boxes[n].bbBox.GetMin()[2];

		vPoints[6].x = Boxes[n].bbBox.GetMax()[0];
		vPoints[6].y = Boxes[n].bbBox.GetMax()[1];
		vPoints[6].z = Boxes[n].bbBox.GetMin()[2];

		vPoints[7].x = Boxes[n].bbBox.GetMax()[0];
		vPoints[7].y = Boxes[n].bbBox.GetMin()[1];
		vPoints[7].z = Boxes[n].bbBox.GetMin()[2];

		GfxUtils->TransformPoints(vPoints, &Boxes[n].mWorld);

		for (int iVert = 0; iVert < 8; ++iVert)
		{
			pVertices->v = vPoints[iVert];
			pVertices->color = GL_Color(Boxes[n].v4Color);
			++pVertices;
		}
	}
	
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	g_RenderList.Add(K2_NEW(ctx_GL2,    CBoxRenderer)(iNumBoxes));
}


/*====================
  CGfx3D::AddBox
  ====================*/
void	CGfx3D::AddBox(const CBBoxf &bbBox, const CVec4f &v4Color, const D3DXMATRIXA16 &mWorld)
{
	if (!g_bValidScene || iNumBoxes >= MAX_BOXES)
		return;

	int N(iNumBoxes);

	Boxes[N].bbBox = bbBox;
	Boxes[N].v4Color = v4Color;
	Boxes[N].mWorld = mWorld;
	++iNumBoxes;
}


/*====================
  CGfx3D::AddPointBatches
  ====================*/
void	CGfx3D::AddPointBatches()
{
	if (iNumPoints == 0)
		return;

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBPoint);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_POINTS * sizeof(SLineVertex), NULL, GL_STREAM_DRAW_ARB);
	SLineVertex *pVertices = (SLineVertex *)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);

	for (int n = 0; n < iNumPoints; ++n)
	{
		pVertices[n].v = Points[n].v3Pos;
		pVertices[n].color = GL_Color(Points[n].v4Color);
	}

	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	g_RenderList.Add(K2_NEW(ctx_GL2,    CPointRenderer)(iNumPoints));
}


/*====================
  CGfx3D::AddPoint
  ====================*/
void	CGfx3D::AddPoint(const CVec3f &v3Pos, const CVec4f &v4Color)
{
	if (!g_bValidScene || iNumPoints >= MAX_POINTS)
		return;

	int N(iNumPoints);

	Points[N].v3Pos = v3Pos;
	Points[N].v4Color = v4Color;
	++iNumPoints;
}


/*====================
  CGfx3D::AddLineBatches
  ====================*/
void	CGfx3D::AddLineBatches()
{
	if (iNumLines == 0)
		return;

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBLine);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_LINES * 2 * sizeof(SLineVertex), NULL, GL_STREAM_DRAW_ARB);
	SLineVertex *pVertices = (SLineVertex *)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);

	for (int n = 0; n < iNumLines; ++n)
	{
		pVertices->v = Lines[n].v3Start;
		pVertices->color = GL_Color(Lines[n].v4Color);
		++pVertices;

		pVertices->v = Lines[n].v3End;
		pVertices->color = GL_Color(Lines[n].v4Color);
		++pVertices;
	}

	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	
	g_RenderList.Add(K2_NEW(ctx_GL2,    CLineRenderer)(iNumLines));
}


/*====================
  CGfx3D::AddLine
  ====================*/
void	CGfx3D::AddLine(const CVec3f &v3Start, const CVec3f &v3End, const CVec4f &v4Color)
{
	if (!g_bValidScene || iNumLines >= MAX_LINES)
		return;

	int N(iNumLines);

	Lines[N].v3Start = v3Start;
	Lines[N].v3End = v3End;
	Lines[N].v4Color = v4Color;
	++iNumLines;
}


/*====================
  CGfx3D::Setup3D
  ====================*/
void	CGfx3D::Setup3D()
{
	glClearDepth(1.0f);

	CVec4f v4SunPos(SceneManager.GetSunPos(), 0.0f);

	g_iMaxDynamicLights = (vid_dynamicLights && vid_shaderLightingQuality == 0) ? MIN(MAX_POINT_LIGHTS, int(vid_maxDynamicLights)) : 0;
	g_bLighting = true;
	g_iNumActiveBones = 0;
	g_iNumActivePointLights = 0;
	g_pCurrentEntity = NULL;
	g_bTexkill = false;

	g_iScreenWidth = g_CurrentVidMode.iWidth;
	g_iScreenHeight = g_CurrentVidMode.iHeight;
}


/*====================
  CGfx3D::SetupCamera
  ====================*/
void	CGfx3D::SetupCamera(const CCamera &cCamera)
{
	PROFILE("CGfx3D::SetupCamera");

	D3DXMatrixIdentity(&g_mIdentity);
	glMatrixMode(GL_PROJECTION);
	if (cCamera.HasFlags(CAM_ORTHO))
	{
		PROFILE("Ortho");

		glLoadIdentity();
		glOrtho(-cCamera.GetOrthoWidth() / 2.0f, cCamera.GetOrthoWidth() / 2.0f,
			-cCamera.GetOrthoHeight() / 2.0f, cCamera.GetOrthoHeight() / 2.0f,
			cCamera.GetZNear(), cCamera.GetZFar());

		glGetFloatv(GL_PROJECTION_MATRIX, (float*)g_mProj.m);
	}
	else if (cCamera.HasFlags(CAM_INVERSEPROJECTION))
	{
		PROFILE("Inverse");

		// Projection matrix
		float yScale = 1.0f / tan((DEG2RAD(cCamera.GetFovY()) / 2.0f));
		float xScale = yScale / cCamera.GetAspect();
		float zn = cCamera.GetZNear();

		// Normal projection matrix solved when zn = -zf

		D3DXMATRIX mProj(xScale, 0.0f,    0.0f,     0.0f,
		                 0.0f,   yScale,  0.0f,     0.0f,
		                 0.0f,   0.0f,   -0.5f,    -1.0f,
		                 0.0f,   0.0f,   -0.5f*zn,  0.0f);

		glLoadMatrixf((float*)mProj.m);
		g_mProj = mProj;
	}
	else
	{
		PROFILE("Proj");

		// Projection matrix
		if (gfx_infiniteFrustum || cCamera.HasFlags(CAM_INFINITE))
		{
			float yScale = 1.0f / tan((DEG2RAD(cCamera.GetFovY()) / 2.0f));
			float xScale = yScale / cCamera.GetAspect();
			float zn = cCamera.GetZNear();
			//float zf = cCamera.GetZFar();

			D3DXMATRIX mProj(xScale, 0.0f,    0.0f,  0.0f,
			                 0.0f,   yScale,  0.0f,  0.0f,
			                 0.0f,   0.0f,   -1.0f, -1.0f,
			                 0.0f,   0.0f,   -zn,    0.0f);

			glLoadMatrixf((float*)mProj.m);
			g_mProj = mProj;
		}
		else
		{
			float fNear(cCamera.GetZNear());
			float fFar(cCamera.GetZFar());

			float fMaxY(fNear * tan((DEG2RAD(cCamera.GetFovY()) / 2.0f)));
			float fMinY(-fMaxY);

			float fMaxX(fNear * tan((DEG2RAD(cCamera.GetFovX()) / 2.0f)));
			float fMinX(-fMaxX);

			float fWidth(fMaxX - fMinX);
			float fHeight(fMaxY - fMinY);
			float fDepth(fFar - fNear);

			D3DXMATRIXA16 mProj;

			mProj[0] = 2.0f * fNear / fWidth;
			mProj[1] = 0.0f;
			mProj[2] = 0.0f;
			mProj[3] = 0.0f;
			
			mProj[4] = 0.0f;
			mProj[5] = 2.0f * fNear / fHeight;
			mProj[6] = 0.0f;
			mProj[7] = 0.0f;
			
			mProj[8] = (fMaxX + fMinX) / fWidth;
			mProj[9] = (fMaxY + fMinY) / fHeight;
			mProj[10] = -(fFar + fNear) / fDepth;
			mProj[11] = -1.0f;
			
			mProj[12] = 0.0f;
			mProj[13] = 0.0f;
			mProj[14] = -2.0f * fFar * fNear / fDepth;
			mProj[15] = 0.0f;

			glLoadMatrixf((float*)mProj.m);
			g_mProj = mProj;
		}
	}

	// View matrix
	glMatrixMode(GL_MODELVIEW);

	// View matrix
	D3DXMATRIXA16 mView;
	D3DXMATRIXA16 mViewAxis;
	D3DXMATRIXA16 mViewTranslation;

	D3DXMatrixIdentity(&mView);
	D3DXMatrixTranslation(&mViewTranslation, -cCamera.GetOrigin(X), -cCamera.GetOrigin(Y), -cCamera.GetOrigin(Z));

	mViewAxis[0] = cCamera.GetViewAxis(RIGHT).x;
	mViewAxis[1] = cCamera.GetViewAxis(UP).x;
	mViewAxis[2] = -cCamera.GetViewAxis(FORWARD).x;
	mViewAxis[3] = 0.0f;

	mViewAxis[4] = cCamera.GetViewAxis(RIGHT).y;
	mViewAxis[5] = cCamera.GetViewAxis(UP).y;
	mViewAxis[6] = -cCamera.GetViewAxis(FORWARD).y;
	mViewAxis[7] = 0.0f;

	mViewAxis[8] = cCamera.GetViewAxis(RIGHT).z;
	mViewAxis[9] = cCamera.GetViewAxis(UP).z;
	mViewAxis[10] = -cCamera.GetViewAxis(FORWARD).z;
	mViewAxis[11] = 0.0f;

	mViewAxis[12] = 0.0f;
	mViewAxis[13] = 0.0f;
	mViewAxis[14] = 0.0f;
	mViewAxis[15] = 1.0f;

	D3DXMatrixMultiply(&mView, &mViewAxis, &mView);
	D3DXMatrixMultiply(&mView, &mViewTranslation, &mView);
	
	glLoadMatrixf((float*)mView);

	g_mView = mView;
	g_mViewRotate = mViewAxis;
	g_mViewOffset = mViewTranslation;
	
	g_mWorld = g_mIdentity;
	g_mViewProj = g_mView * g_mProj;
	
	int iCamX = INT_ROUND(cCamera.GetX());
	int iCamY = INT_ROUND(cCamera.GetY());
	int iCamWidth = INT_ROUND(cCamera.GetWidth());
	int iCamHeight = INT_ROUND(cCamera.GetHeight());

	glViewport(iCamX, g_iScreenHeight - (iCamY + iCamHeight), iCamWidth, iCamHeight);

	if (cCamera.GetFlags() & CAM_DEPTH_COMPRESS)
		glDepthRange(0.0f, 0.1f);
	else
		glDepthRange(0.0f, 1.0f);
}


/*====================
  CGfx3D::Exit3D
  ====================*/
void	CGfx3D::Exit3D()
{
	iNumBillboards = 0;
	iNumBeams = 0;
	iNumEffectTriangles = 0;
	iNumTreeBillboards = 0;
	iNumBoxes = 0;
	iNumPoints = 0;
	iNumLines = 0;
}


/*====================
  CGfx3D::AddSky
  ====================*/
void	CGfx3D::AddSky()
{
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
			GfxModels->AddModelToLists(cEntity, PHASE_COLOR);
			break;

		case OBJTYPE_BILLBOARD:
			if (!GfxUtils->GetMaterial(cEntity.hRes).HasPhase(PHASE_COLOR))
				continue;

			AddBillboard
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
				cEntity.color
			);
			break;
		}

		fOffset += (FAR_AWAY / 1000.0f);
	}

	if (SceneManager.GetDrawSkybox())
		g_RenderList.Add(K2_NEW(ctx_GL2,    CSkyRenderer)());
}


/*====================
  CGfx3D::AddWorld
  ====================*/
void	CGfx3D::AddWorld(EMaterialPhase ePhase)
{
	PROFILE("CGfx3D::AddWorld");

	if (!g_pCam->HasFlags(CAM_NO_TERRAIN))
		GfxTerrain->AddTerrainChunks(ePhase);
	
	g_pTreeSceneManager->ResetLeaves();
	SceneEntityList &lSceneEntities(SceneManager.GetEntityList());
	for (SceneEntityList::iterator it(lSceneEntities.begin()); it != lSceneEntities.end(); ++it)
	{
		SSceneEntityEntry &cEntry(**it);
		CSceneEntity &cEntity(cEntry.cEntity);

		if ((ePhase == PHASE_COLOR || ePhase == PHASE_DEPTH) && cEntry.bCull)
			continue;

		if (ePhase == PHASE_SHADOW && cEntry.bCullShadow)
			continue;

		switch (cEntity.objtype)
		{
			case OBJTYPE_MODEL:
				GfxModels->AddModelToLists(cEntity, ePhase);
				break;

			case OBJTYPE_BILLBOARD:
				if (!g_ResourceManager.GetMaterial(cEntity.hRes) || !GfxUtils->GetMaterial(cEntity.hRes).HasPhase(ePhase))
					continue;

				AddBillboard
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
					cEntity.color
				);
				break;

			case OBJTYPE_BEAM:
				if (!g_ResourceManager.GetMaterial(cEntity.hRes) || !GfxUtils->GetMaterial(cEntity.hRes).HasPhase(ePhase))
					continue;

				AddBeam
				(
					cEntity.GetPosition(),
					cEntity.angle,
					cEntity.scale,
					cEntity.height,
					cEntity.s1,
					cEntity.hRes,
					cEntity.color
				);
				break;

			case OBJTYPE_GROUNDSPRITE:
				if (!g_ResourceManager.GetMaterial(cEntity.hRes) || !GfxUtils->GetMaterial(cEntity.hRes).HasPhase(ePhase))
					continue;

				AddGroundSprite
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
  CGfx3D::AddBillboardBatches
  ====================*/
void	CGfx3D::AddBillboardBatches(EMaterialPhase ePhase)
{
	int iNumEffectQuads(iNumBillboards + iNumBeams);

	if (iNumEffectQuads == 0)
		return;

	// Clamp number of billboards and beams to make room if we exceeded max
	if (iNumEffectQuads > MAX_BILLBOARDS)
	{
		iNumBillboards -= (iNumEffectQuads - MAX_BILLBOARDS);
		iNumEffectQuads = MAX_BILLBOARDS;
	}

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBBillboard);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_BILLBOARDS * 4 * sizeof(SEffectVertex), NULL, GL_STREAM_DRAW_ARB);
	SEffectVertex* pVertices((SEffectVertex*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB));

	if (pVertices == NULL)
		return;

	CVec3f v3CamUp(g_pCam->GetViewAxis(UP));
	CVec3f v3CamRight(g_pCam->GetViewAxis(RIGHT));

	for (int n(0); n < iNumBillboards; ++n)
	{
		CVec3f v3Up(Billboards[n].uiFlags & BBOARD_LOCK_UP ? Billboards[n].aAxis.Up() : v3CamUp);
		CVec3f v3Right(Billboards[n].uiFlags & BBOARD_LOCK_RIGHT ? Billboards[n].aAxis.Right() : v3CamRight);

		if (Billboards[n].fYaw)
		{
			// Rotate v3Right around v3Up
			CVec3f p0(v3Right);
			CVec3f p1(CrossProduct(v3Up, p0));

			float fSin, fCos;
			M_SinCos(DEG2RAD(Billboards[n].fYaw), fSin, fCos);

			v3Right = p0 * fCos + p1 * fSin;

			if (Billboards[n].fPitch)
			{
				v3Right = M_RotatePointAroundAxis(v3Right, Billboards[n].uiFlags & BBOARD_LOCK_RIGHT ? Billboards[n].aAxis.Right() : v3CamRight, Billboards[n].fPitch);
				v3Up = M_RotatePointAroundAxis(v3Up, Billboards[n].uiFlags & BBOARD_LOCK_RIGHT ? Billboards[n].aAxis.Right() : v3CamRight, Billboards[n].fPitch);
			}
		}
		else if (Billboards[n].fPitch)
		{
			// Rotate v3Up around v3Right
			CVec3f p0(v3Up);
			CVec3f p1(CrossProduct(v3Right, p0));

			float fSin, fCos;
			M_SinCos(DEG2RAD(Billboards[n].fPitch), fSin, fCos);

			v3Up = p0 * fCos + p1 * fSin;
		}
		
		if (Billboards[n].angle)
		{
			// Rotate v3Right and v3Up around forward vector
			CVec3f v3R(v3Right);
			CVec3f v3U(v3Up);
			
			float fSin, fCos;
			M_SinCos(DEG2RAD(Billboards[n].angle), fSin, fCos);

			v3Right = v3R * fCos + v3U * fSin;
			v3Up = v3U * fCos - v3R * fSin;
		}
	
		CVec3f	v3Points[4];

		if (Billboards[n].fDepthBias)
		{
			CVec3f v3CamDirection(Normalize(Billboards[n].v3Pos - g_pCam->GetOrigin()));

			Billboards[n].v3Pos.ScaleAdd(v3CamDirection, Billboards[n].fDepthBias);
		}

		if (Billboards[n].uiFlags & BBOARD_FLARE)
		{
			v3Right *= Billboards[n].width * 0.5f;
			v3Up *= Billboards[n].height;

			v3Points[0] = Billboards[n].v3Pos; v3Points[0] += v3Right;
			v3Points[1] = Billboards[n].v3Pos; v3Points[1] += v3Right; v3Points[1] += v3Up; 
			v3Points[2] = Billboards[n].v3Pos; v3Points[2] -= v3Right; v3Points[2] += v3Up;
			v3Points[3] = Billboards[n].v3Pos; v3Points[3] -= v3Right;
		}
		else if (Billboards[n].uiFlags & BBOARD_OFFCENTER)
		{
			CVec2f v2Center(Billboards[n].v2Center);
			CVec3f av3Offsets[4] =
			{
				v3Right * (Billboards[n].width * (1.0f - v2Center.x)),
				v3Up * (Billboards[n].height * (1.0f - v2Center.y)),
				v3Right * (Billboards[n].width * -v2Center.x),
				v3Up * (Billboards[n].height * -v2Center.y)
			};

			v3Points[0] = Billboards[n].v3Pos; v3Points[0] += av3Offsets[0]; v3Points[0] += av3Offsets[3];
			v3Points[1] = Billboards[n].v3Pos; v3Points[1] += av3Offsets[0]; v3Points[1] += av3Offsets[1];
			v3Points[2] = Billboards[n].v3Pos; v3Points[2] += av3Offsets[2]; v3Points[2] += av3Offsets[1];
			v3Points[3] = Billboards[n].v3Pos; v3Points[3] += av3Offsets[2]; v3Points[3] += av3Offsets[3];
		}
		else
		{
			v3Right *= Billboards[n].width * 0.5f;
			v3Up *= Billboards[n].height * 0.5f;

			v3Points[0] = Billboards[n].v3Pos; v3Points[0] += v3Right; v3Points[0] -= v3Up;
			v3Points[1] = Billboards[n].v3Pos; v3Points[1] += v3Right; v3Points[1] += v3Up; 
			v3Points[2] = Billboards[n].v3Pos; v3Points[2] -= v3Right; v3Points[2] += v3Up;
			v3Points[3] = Billboards[n].v3Pos; v3Points[3] -= v3Right; v3Points[3] -= v3Up;
		}

		dword dwColor(Billboards[n].color.GetAsDWordGL());

		pVertices->v = v3Points[0];
		pVertices->color = dwColor;
		pVertices->t[0] = Billboards[n].s2;
		pVertices->t[1] = Billboards[n].t1;
		pVertices->t[2] = Billboards[n].frame;
		pVertices->t[3] = Billboards[n].param;
		++pVertices;

		pVertices->v = v3Points[1];
		pVertices->color = dwColor;
		pVertices->t[0] = Billboards[n].s2;
		pVertices->t[1] = Billboards[n].t2;
		pVertices->t[2] = Billboards[n].frame;
		pVertices->t[3] = Billboards[n].param;
		++pVertices;

		pVertices->v = v3Points[2];
		pVertices->color = dwColor;
		pVertices->t[0] = Billboards[n].s1;
		pVertices->t[1] = Billboards[n].t2;
		pVertices->t[2] = Billboards[n].frame;
		pVertices->t[3] = Billboards[n].param;
		++pVertices;

		pVertices->v = v3Points[3];
		pVertices->color = dwColor;
		pVertices->t[0] = Billboards[n].s1;
		pVertices->t[1] = Billboards[n].t1;
		pVertices->t[2] = Billboards[n].frame;
		pVertices->t[3] = Billboards[n].param;
		++pVertices;
	}

	for (int n = 0; n < iNumBeams; ++n)
	{
		float fTex0(0.0f);
		float fTex1(Beams[n].fTile);

		CVec2f v2Dir(Normalize(g_pCam->WorldToView(Beams[n].v3Start) - g_pCam->WorldToView(Beams[n].v3End)));

		CVec3f v3Dir(g_pCam->GetViewAxis(RIGHT) * v2Dir.x + g_pCam->GetViewAxis(UP) * v2Dir.y);
		
		CVec3f v3Width(CrossProduct(v3Dir, g_pCam->GetViewAxis(FORWARD)));

		dword dwStartColor(Beams[n].v4StartColor.GetAsDWordGL());
		dword dwEndColor(Beams[n].v4EndColor.GetAsDWordGL());

		pVertices->v = Beams[n].v3Start + v3Width * (Beams[n].fStartSize / 2.0f);
		pVertices->color = dwStartColor;
		pVertices->t = CVec4f(fTex0, 0.0f, Beams[n].fStartFrame, Beams[n].fStartParam);
		++pVertices;

		pVertices->v = Beams[n].v3End + v3Width * (Beams[n].fEndSize / 2.0f);
		pVertices->color = dwEndColor;
		pVertices->t = CVec4f(fTex1, 0.0f, Beams[n].fEndFrame, Beams[n].fEndParam);
		++pVertices;

		pVertices->v = Beams[n].v3End - v3Width * (Beams[n].fEndSize / 2.0f);
		pVertices->color = dwEndColor;
		pVertices->t = CVec4f(fTex1, 1.0f, Beams[n].fEndFrame, Beams[n].fEndParam);
		++pVertices;

		pVertices->v = Beams[n].v3Start - v3Width * (Beams[n].fStartSize / 2.0f);
		pVertices->color = dwStartColor;
		pVertices->t = CVec4f(fTex0, 1.0f, Beams[n].fStartFrame, Beams[n].fStartParam);
		++pVertices;
	}

	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	//
	// Add billboard batches to the renderlist
	//

	for (int n(0); n < iNumBillboards; ++n)
	{
		ResHandle hMaterial(Billboards[n].hMaterial);

		if (!GfxUtils->GetMaterial(hMaterial).HasPhase(ePhase))
			continue;

		int iMaxEffectLayer(Billboards[n].iEffectLayer);

		int f;
		for (f = n + 1; f < iNumBillboards && Billboards[f].hMaterial == hMaterial && Billboards[f].iEffectLayer >= iMaxEffectLayer; ++f)
			iMaxEffectLayer = Billboards[f].iEffectLayer;

		g_RenderList.Add(K2_NEW(ctx_GL2,    CBillboardRenderer)(hMaterial, n, f, (iMaxEffectLayer << 20) + n, -FAR_AWAY));

		n += (f - n) - 1;
	}

	int iBufferOffset(iNumBillboards);

	//
	// Add beam batches to the renderlist
	//

	for (int n = 0; n < iNumBeams; ++n)
	{
		ResHandle hMaterial(Beams[n].hMaterial);

		if (!GfxUtils->GetMaterial(hMaterial).HasPhase(ePhase))
			continue;

		int iMaxEffectLayer(Beams[n].iEffectLayer);

		int f;
		for (f = n + 1; f < iNumBeams && Beams[f].hMaterial == hMaterial && Beams[f].iEffectLayer >= iMaxEffectLayer; ++f)
			iMaxEffectLayer = Beams[f].iEffectLayer;


		g_RenderList.Add(K2_NEW(ctx_GL2,    CBillboardRenderer)(hMaterial, iBufferOffset + n, iBufferOffset + f, (iMaxEffectLayer << 20) + n, -FAR_AWAY));

		n += (f - n) - 1;
	}
}


/*====================
  CGfx3D::AddBillboard
  ====================*/
void	CGfx3D::AddBillboard(const CVec3f &v3Pos, float width, float height, float angle, float s1, float t1, float s2, float t2, float frame, float param, ResHandle hMaterial, uint uiFlags, float fPitch, float fYaw, float fDepthBias, int iEffectLayer, float fDepth, const CVec4f &v4Color)
{
	if (!g_bValidScene || iNumBillboards >= MAX_BILLBOARDS)
		return;

	int N(iNumBillboards);

	Billboards[N].v3Pos = v3Pos;
	Billboards[N].width = width;
	Billboards[N].height = height;
	Billboards[N].angle = angle;
	Billboards[N].s1 = s1;
	Billboards[N].t1 = t1;
	Billboards[N].s2 = s2;
	Billboards[N].t2 = t2;
	Billboards[N].frame = frame;
	Billboards[N].param = param;
	Billboards[N].color = v4Color;
	Billboards[N].hMaterial = hMaterial;
	Billboards[N].uiFlags = uiFlags;
	Billboards[N].fPitch = fPitch;
	Billboards[N].fYaw = fYaw;
	Billboards[N].fDepthBias = fDepthBias;
	Billboards[N].iEffectLayer = iEffectLayer;
	Billboards[N].fDepth = fDepth;

	++iNumBillboards;
}


/*====================
  CGfx3D::AddBeam
  ====================*/
void	CGfx3D::AddBeam(const CVec3f &v3Start, const CVec3f &v3End, float fSize, float fTile, float fTaper, ResHandle hMaterial, const CVec4f &v4Color)
{
	if (!g_bValidScene || iNumBeams >= MAX_BEAMS)
		return;

	int N(iNumBeams);

	Beams[N].v3Start = v3Start;
	Beams[N].v3End = v3End;
	Beams[N].fStartSize = Beams[N].fEndSize = fSize;
	Beams[N].fTile = fTile;
	Beams[N].fTaper = fTaper;
	Beams[N].v4StartColor = Beams[N].v4EndColor = v4Color;
	Beams[N].fStartFrame = Beams[N].fEndFrame = 0.0f;
	Beams[N].fStartParam = Beams[N].fEndParam = 0.0f;
	Beams[N].hMaterial = hMaterial;
	++iNumBeams;
}


/*====================
  CGfx3D::AddEffectTriangleBatches
  ====================*/
void	CGfx3D::AddEffectTriangleBatches(EMaterialPhase ePhase)
{
	if (iNumEffectTriangles == 0)
		return;

	//
	// Build effect triangle vertex buffer
	//

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBEffectTriangle);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_EFFECT_TRIANGLES * 3 * sizeof(SEffectVertex), NULL, GL_STREAM_DRAW_ARB);
	SEffectVertex* pVertices = (SEffectVertex*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);

	int q = 0;
	for (int n(0); n < iNumEffectTriangles; ++n)
	{
		MemManager.Copy(&pVertices[q], &EffectTriangles[n].vert[0], 3 * sizeof(SEffectVertex));
		q += 3;
	}

	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	//
	// Add effect triangle batches to renderlist
	//

	for (int n(0); n < iNumEffectTriangles; ++n)
	{
		ResHandle hMaterial(EffectTriangles[n].hMaterial);
		int iEffectLayer(EffectTriangles[n].iEffectLayer);
		float fDepth(EffectTriangles[n].fDepth);

		if (!GfxUtils->GetMaterial(hMaterial).HasPhase(ePhase))
			continue;

		int f;
		for (f = n + 1; f < iNumEffectTriangles && EffectTriangles[f].hMaterial == hMaterial; ++f);

		g_RenderList.Add(K2_NEW(ctx_GL2,    CEffectTriangleRenderer)(hMaterial, n, f, iEffectLayer, fDepth));

		n += (f - n) - 1;
	}
}


/*====================
  CGfx3D::AddEffectTriangle
  ====================*/
void	CGfx3D::AddEffectTriangle(const SEffectTriangle &sTri)
{
	if (!g_bValidScene || iNumEffectTriangles >= MAX_EFFECT_TRIANGLES)
		return;

	EffectTriangles[iNumEffectTriangles] = sTri;
	++iNumEffectTriangles;
}


/*====================
  CGfx3D::AddTreeBillboardBatches
  ====================*/
void	CGfx3D::AddTreeBillboardBatches(EMaterialPhase ePhase)
{
	if (iNumTreeBillboards == 0)
		return;

	//
	// Build extended triangle vertex buffer
	//

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBTreeBillboard);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_TREE_BILLBOARDS * 4 * sizeof(STreeBillboardVertex), NULL, GL_STREAM_DRAW_ARB);
	STreeBillboardVertex* pVertices = (STreeBillboardVertex*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);

	int q = 0;
	for (int n(0); n < iNumTreeBillboards; ++n)
	{
		MemManager.Copy(&pVertices[q], &TreeBillboards[n].vert[0], 4 * sizeof(STreeBillboardVertex));
		q += 4;
	}

	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	//
	// Add tree billboard batches to renderlist
	//

	for (int n(0); n < iNumTreeBillboards; ++n)
	{
		ResHandle hMaterial(TreeBillboards[n].hMaterial);
		dword dwAlphaTest(TreeBillboards[n].dwAlphaTest);

		if (!GfxUtils->GetMaterial(hMaterial).HasPhase(ePhase))
			continue;

		int f;
		for (f = n + 1; f < iNumTreeBillboards && TreeBillboards[f].hMaterial == hMaterial && TreeBillboards[f].dwAlphaTest == dwAlphaTest; ++f);

		g_RenderList.Add(K2_NEW(ctx_GL2,    CTreeBillboardRenderer)(hMaterial, dwAlphaTest, n, f, 0, 0.0f));

		n += (f - n) - 1;
	}

	iNumTreeBillboards = 0;
}


/*====================
  CGfx3D::AddTreeBillboard
  ====================*/
void	CGfx3D::AddTreeBillboard
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
	if (!g_bValidScene || iNumTreeBillboards >= MAX_TREE_BILLBOARDS)
		return;

	int N(iNumTreeBillboards);

	TreeBillboards[N].vert[0].v = GfxUtils->TransformPoint(v0, mWorld);
	TreeBillboards[N].vert[0].color = color0;
	TreeBillboards[N].vert[0].t = t0;

	TreeBillboards[N].vert[1].v = GfxUtils->TransformPoint(v1, mWorld);
	TreeBillboards[N].vert[1].color = color1;
	TreeBillboards[N].vert[1].t = t1;

	TreeBillboards[N].vert[2].v = GfxUtils->TransformPoint(v2, mWorld);
	TreeBillboards[N].vert[2].color = color2;
	TreeBillboards[N].vert[2].t = t2;

	TreeBillboards[N].vert[3].v = GfxUtils->TransformPoint(v3, mWorld);
	TreeBillboards[N].vert[3].color = color3;
	TreeBillboards[N].vert[3].t = t3;

	TreeBillboards[N].hMaterial = hMaterial;
	TreeBillboards[N].dwAlphaTest = dwAlphaTest;

	++iNumTreeBillboards;
}


/*====================
  CGfx3D::AddScenePolys
  ====================*/
void	CGfx3D::AddScenePolys(EMaterialPhase ePhase)
{
	const SceneFaceList &SceneFaces(SceneManager.GetFaceList());
	for (SceneFaceList::const_iterator it(SceneFaces.begin()); it != SceneFaces.end(); ++it)
	{
		g_RenderList.Add(K2_NEW(ctx_GL2,    CScenePolyRenderer)(it->hMaterial, it->verts, it->zNumVerts, it->flags));
	}
}


/*====================
  CGfx3D::AddGroundSprite
  ====================*/
void	CGfx3D::AddGroundSprite(const CVec3f &v3Pos, float fHeight, float fWidth, const CVec3f &v3Angles, const CVec4f &v4Color, float fFrame, float fParam, ResHandle hMaterial, int iEffectLayer, float fDepth)
{
	PROFILE("CGfx3D::AddGroundSprite");

	if (!g_bValidScene || !GfxTerrain->pWorld)
		return;

	dword dwColor(v4Color.GetAsDWordGL());

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

	float fTileSize(GfxTerrain->pWorld->GetScale());
	const CVec2f &v2Min(recArea.lt());
	const CVec2f &v2Max(recArea.rb());

	int iMinX(CLAMP(INT_FLOOR(v2Min.x / fTileSize), 0, GfxTerrain->pWorld->GetTileWidth() - 1));
	int iMinY(CLAMP(INT_FLOOR(v2Min.y / fTileSize), 0, GfxTerrain->pWorld->GetTileWidth() - 1));
	int iMaxX(CLAMP(INT_CEIL(v2Max.x / fTileSize), 0, GfxTerrain->pWorld->GetTileHeight() - 1));
	int iMaxY(CLAMP(INT_CEIL(v2Max.y / fTileSize), 0, GfxTerrain->pWorld->GetTileHeight() - 1));

	float fScale(GfxTerrain->pWorld->GetScale());
	byte *pSplitMap(GfxTerrain->pWorld->GetSplitMap());
	int iTileWidth(GfxTerrain->pWorld->GetTileWidth());

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
						const CPlane &cPlane(GfxTerrain->pWorld->GetTilePlane(iX, iY, EGridTriangles(iTri)));

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
						
							AddEffectTriangle(sTri);
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
						const CPlane &cPlane(GfxTerrain->pWorld->GetTilePlane(iX, iY, EGridTriangles(iTri)));

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
						
							AddEffectTriangle(sTri);
						}
					}
				}
			}
		}
	}
}


/*====================
  CGfx3D::AddSceneEntity
  ====================*/
void	CGfx3D::AddSceneEntity(IEmitter &cEmitter, int iEffectLayer, float fEffectDepth, bool bCull)
{
	// Imbedded Emitters
	uint uiNumEmitters(cEmitter.GetNumEmitters());
	for (uint uiEmitter(0); uiEmitter < uiNumEmitters; ++uiEmitter)
	{
		IEmitter *pEmitter(cEmitter.GetEmitter(uiEmitter));
		while (pEmitter != NULL)
		{
			AddSceneEntity(*pEmitter, iEffectLayer, fEffectDepth, bCull);
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

			SceneManager.AddEntity(scEntity);
		}
	}
}


/*====================
  CGfx3D::AddParticleSystemSceneEntities
  ====================*/
void	CGfx3D::AddParticleSystemSceneEntities(bool bCull)
{
	PROFILE("CGfx3D::AddParticleSystemSceneEntities");

	if (!vid_drawParticleSystems || !g_bValidScene || g_pCam == NULL)
		return;

	SceneParticleSystemList &lParticleSystems(SceneManager.GetParticleSystemList());
	for (SceneParticleSystemList::iterator it(lParticleSystems.begin()); it != lParticleSystems.end(); ++it)
	{
		SSceneParticleSystemEntry &cEntry(**it);
		const CParticleSystem *pParticleSystem(cEntry.pParticleSystem);

		if (cEntry.bCull)
			continue;

		int iEffectLayer(0);
		float fEffectDepth(DotProduct(g_pCam->GetViewAxis(FORWARD), pParticleSystem->GetSourcePosition()));

		const EmitterList &lEmitters(pParticleSystem->GetEmitterList());
		for (EmitterList::const_iterator it(lEmitters.begin()); it != lEmitters.end(); ++it)
		{
			// Imbedded Emitters
			uint uiNumEmitters((*it)->GetNumEmitters());
			for (uint uiEmitter(0); uiEmitter < uiNumEmitters; ++uiEmitter)
			{
				IEmitter *pEmitter((*it)->GetEmitter(uiEmitter));

				if (pEmitter)
					AddSceneEntity(*pEmitter, iEffectLayer, fEffectDepth, bCull);
			}

			AddSceneEntity(**it, iEffectLayer, fEffectDepth, bCull);
			++iEffectLayer;
		}
	}
}


/*====================
  CGfx3D::AddEmitter
  ====================*/
void	CGfx3D::AddEmitter(IEmitter &cEmitter, int iEffectLayer, float fEffectDepth)
{
	// Imbedded Emitters
	uint uiNumEmitters(cEmitter.GetNumEmitters());
	for (uint uiEmitter(0); uiEmitter < uiNumEmitters; ++uiEmitter)
	{
		IEmitter *pEmitter(cEmitter.GetEmitter(uiEmitter));
		while (pEmitter != NULL)
		{
			AddEmitter(*pEmitter, iEffectLayer, fEffectDepth);
			pEmitter = pEmitter->GetNextEmitter();
		}
	}

	//
	// Add Billboards
	//

	uint uiNumBillboards(cEmitter.GetNumBillboards());

	for (uint uiBillboard(0); uiBillboard < uiNumBillboards; ++uiBillboard)
	{
		if (iNumBillboards >= MAX_BILLBOARDS)
			continue;

		if (cEmitter.GetBillboard(uiBillboard, Billboards[iNumBillboards]))
		{
			Billboards[iNumBillboards].iEffectLayer = iEffectLayer;
			Billboards[iNumBillboards].fDepth = fEffectDepth;
			++iNumBillboards;
		}
	}

	//
	// Add Beams
	//

	uint uiNumBeams(cEmitter.GetNumBeams());

	for (uint uiBeam(0); uiBeam < uiNumBeams; ++uiBeam)
	{
		if (iNumBeams >= MAX_BEAMS)
			continue;

		if (cEmitter.GetBeam(uiBeam, Beams[iNumBeams]))
		{
			++iNumBeams;
		}
	}

	//
	// Add Triangles
	//

	uint uiNumTriangles(cEmitter.GetNumTriangles());

	for (uint uiTriangle(0); uiTriangle < uiNumTriangles; ++uiTriangle)
	{
		if (iNumEffectTriangles >= MAX_EFFECT_TRIANGLES)
			continue;

		if (cEmitter.GetTriangle(uiTriangle, reinterpret_cast<STriangle &>(EffectTriangles[iNumEffectTriangles])))
		{
			GfxUtils->BGRAtoRGBA(EffectTriangles[iNumEffectTriangles].vert[0].color);
			GfxUtils->BGRAtoRGBA(EffectTriangles[iNumEffectTriangles].vert[1].color);
			GfxUtils->BGRAtoRGBA(EffectTriangles[iNumEffectTriangles].vert[2].color);

			EffectTriangles[iNumEffectTriangles].iEffectLayer = iEffectLayer;
			EffectTriangles[iNumEffectTriangles].fDepth = fEffectDepth;
			++iNumEffectTriangles;
		}
	}
}


/*====================
  CGfx3D::AddParticleSystems
  ====================*/
void	CGfx3D::AddParticleSystems()
{
	PROFILE("CGfx3D::AddParticleSystems");

	if (!vid_drawParticleSystems || !g_bValidScene)
		return;

	SceneParticleSystemList &lParticleSystems(SceneManager.GetParticleSystemList());
	for (SceneParticleSystemList::iterator it(lParticleSystems.begin()); it != lParticleSystems.end(); ++it)
	{
		SSceneParticleSystemEntry &cEntry(**it);
		const CParticleSystem *pParticleSystem(cEntry.pParticleSystem);

		if (cEntry.bCull)
			continue;

		int iEffectLayer(0);
		float fEffectDepth(DotProduct(g_pCam->GetViewAxis(FORWARD), pParticleSystem->GetSourcePosition()));

		const EmitterList &lEmitters(pParticleSystem->GetEmitterList());
		for (EmitterList::const_iterator it(lEmitters.begin()); it != lEmitters.end(); ++it)
		{
			// Imbedded Emitters
			uint uiNumEmitters((*it)->GetNumEmitters());
			for (uint uiEmitter(0); uiEmitter < uiNumEmitters; ++uiEmitter)
			{
				IEmitter *pEmitter((*it)->GetEmitter(uiEmitter));

				if (pEmitter)
					AddEmitter(*pEmitter, iEffectLayer, fEffectDepth);
			}
			
			AddEmitter(**it, iEffectLayer, fEffectDepth);
			++iEffectLayer;
		}
	}
}


/*====================
  CGfx3D::ObjectBounds

  adds all visible models to the global scene bounding box
  ====================*/
void	CGfx3D::ObjectBounds(CBBoxf &bbObjects)
{
	PROFILE("CGfx3D::ObjectBounds");

	const SceneEntityList &lSceneEntities(SceneManager.GetEntityList());
	for (SceneEntityList::const_iterator it(lSceneEntities.begin()); it != lSceneEntities.end(); ++it)
	{
		SSceneEntityEntry &cEntry(**it);
		CSceneEntity &cEntity(cEntry.cEntity);

		if (cEntry.bCull)
			continue;

		switch (cEntity.objtype)
		{
			case OBJTYPE_MODEL:
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
				break;
			default:
				break;
		}
	}
}


/*====================
  CGfx3D::SetFowProj
  ====================*/
void	CGfx3D::SetFowProj()
{
	if (GfxTerrain->pWorld)
		D3DXMatrixScaling(&g_mFowProj, 1.0f / GfxTerrain->pWorld->GetWorldWidth(), 1.0f / GfxTerrain->pWorld->GetWorldHeight(), 1.0f);
	else
		D3DXMatrixScaling(&g_mFowProj, 1.0f, 1.0f, 1.0f);
}


/*====================
  CGfx3D::SetCloudProj
  ====================*/
void	CGfx3D::SetCloudProj(float fTime)
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
  CGfx3D::Shutdown
  ====================*/
void	CGfx3D::Shutdown()
{
	GL_SAFE_DELETE(glDeleteBuffersARB, VBScenePoly);
	GL_SAFE_DELETE(glDeleteBuffersARB, VBBillboard);
	GL_SAFE_DELETE(glDeleteBuffersARB, VBEffectTriangle);
	GL_SAFE_DELETE(glDeleteBuffersARB, VBTreeBillboard);
	GL_SAFE_DELETE(glDeleteBuffersARB, IBTreeBillboard);
	GL_SAFE_DELETE(glDeleteBuffersARB, VBBox);
	GL_SAFE_DELETE(glDeleteBuffersARB, IBBox);
	GL_SAFE_DELETE(glDeleteBuffersARB, VBPoint);
	GL_SAFE_DELETE(glDeleteBuffersARB, VBLine);

	PRINT_GLERROR_BREAK();
}
