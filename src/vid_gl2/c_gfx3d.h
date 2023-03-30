// (C)2008 S2 Games
// c_gfx3d.h
//
//=============================================================================
#ifndef __C_GFX3D_H__
#define __C_GFX3D_H__

//=============================================================================
// Definitions
//=============================================================================
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
extern	D3DXMATRIXA16			g_mCloudProj;
extern	D3DXMATRIXA16			g_mFowProj;
extern	uint					g_uiImageWidth;
extern	uint					g_uiImageHeight;
extern	float					g_fSceneScaleX;
extern	float					g_fSceneScaleY;

extern	bool					g_bInvertedProjection;

extern int					g_iMaxDynamicLights;
extern const CSceneEntity	*g_pCurrentEntity;

EXTERN_CVAR_BOOL(gfx_lighting);
EXTERN_CVAR_INT(gfx_textures);
EXTERN_CVAR_INT(gfx_wireframe);
EXTERN_CVAR_INT(gfx_points);
EXTERN_CVAR_INT(gfx_normals);	
EXTERN_CVAR_VEC3(gfx_fogColor);
EXTERN_CVAR_FLOAT(gfx_fogNear);
EXTERN_CVAR_FLOAT(gfx_fogFar);
EXTERN_CVAR_FLOAT(gfx_fogDensity);
EXTERN_CVAR_FLOAT(gfx_fogScale);
EXTERN_CVAR_INT(gfx_fogType);
EXTERN_CVAR_BOOL(gfx_clouds);
EXTERN_CVAR_STRING(gfx_cloudTexture);
EXTERN_CVAR_FLOAT(vid_skyEpsilon);
EXTERN_CVAR_BOOL(vid_dynamicLights);
EXTERN_CVAR_INT(vid_maxDynamicLights);

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
const int	MAX_EXTENDED_TRIANGLES = 8192;
const int	MAX_TREE_BILLBOARDS = 1024;

struct SEffectTriangle
{
	SEffectVertex	vert[3];
	ResHandle		hMaterial;
	int				iEffectLayer;
	float			fDepth;
};

struct SExtendedTriangle
{
	SExtendedVertex	vert[3];
	ResHandle		hMaterial;
};

struct STreeBillboards
{
	STreeBillboardVertex	vert[4];
	ResHandle		hMaterial;
	dword			dwAlphaTest;
};

struct SBox
{
	CBBoxf			bbBox;
	CVec4f			v4Color;
	D3DXMATRIXA16	mWorld;
};

struct SPoint
{
	CVec3f			v3Pos;
	CVec4f			v4Color;
};

struct SLine
{
	CVec3f			v3Start;
	CVec3f			v3End;
	CVec4f			v4Color;
};
//=============================================================================

//=============================================================================
// CGfx3D
//=============================================================================
class CGfx3D
{
	SINGLETON_DEF(CGfx3D)

protected:
	void	Setup3D();
	void	Exit3D();

	void	AddSky();
	void	SetMode(/*TODO*/);

	void	AddEntities();
	void	AddTerrain();
	void	AddFoliage();
	void	AddParticles();
	void	AddPolys();
	void	AddMisc();

	void	SetupScene(const CCamera &camera);

	void	AddBillboardBatches(EMaterialPhase ePhase);
	void	AddEffectTriangleBatches(EMaterialPhase ePhase);
	void	AddTreeBillboardBatches(EMaterialPhase ePhase);
	void	AddScenePolys(EMaterialPhase ePhase);
	void	AddBoxBatches();
	void	AddPointBatches();
	void	AddLineBatches();
	
	void	AddSceneEntity(IEmitter &cEmitter, int iEffectLayer, float fEffectDepth, bool bCull);
	void	AddParticleSystemSceneEntities(bool bCull);
	void	AddEmitter(IEmitter &cEmitter, int iEffectLayer, float fEffectDepth);
	void	AddParticleSystems();

	void	SetFowProj();
	void	SetCloudProj(float fTime);

	int		iNumBillboards;
	int		iNumBeams;
	int		iNumEffectTriangles;
	int		iNumTreeBillboards;
	int		iNumBoxes;
	int		iNumPoints;
	int		iNumLines;

	SEffectTriangle		EffectTriangles[MAX_EFFECT_TRIANGLES];
	STreeBillboards		TreeBillboards[MAX_TREE_BILLBOARDS];
	SBillboard			Billboards[MAX_BILLBOARDS];
	SBeam				Beams[MAX_BEAMS];
	SBox				Boxes[MAX_BOXES];
	SPoint				Points[MAX_POINTS];
	SLine				Lines[MAX_LINES];

public:
	~CGfx3D();

	void	Draw(CCamera &camera);
	void	Clear();
	void	Init();
	void	Shutdown();

	void	SetupCamera(const CCamera &cCamera);

	void	AddBox(const CBBoxf &bbBox, const CVec4f &v4Color, const D3DXMATRIXA16 &mWorld);
	void	AddPoint(const CVec3f &v3Pos, const CVec4f &v4Color);
	void	AddLine(const CVec3f &v3Start, const CVec3f &v3End, const CVec4f &v4Color);
	
	void	AddBillboard(const CVec3f &v3Pos, float width, float height, float angle, float s1, float t1, float s2, float t2, float frame, float param, ResHandle hMaterial, uint uiFlags, float fPitch, float fYaw, float fDepthBias, int iEffectLayer, float fDepth, const CVec4f &v4Color);
	void	AddBeam(const CVec3f &v3Start, const CVec3f &v3End, float fSize, float fTile, float fTaper, ResHandle hMaterial, const CVec4f &v4Color);
	void	AddEffectTriangle(const SEffectTriangle &sTri);
	void	AddTreeBillboard(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, dword color0, dword color1, dword color2, dword color3, const CVec2f &t0, const CVec2f &t1, const CVec2f &t2, const CVec2f &t3, ResHandle hMaterial, dword dwAlphaTest, const D3DXMATRIXA16 &mWorld);
	void	AddGroundSprite(const CVec3f &v3Pos, float fHeight, float fWidth, const CVec3f &v3Angles, const CVec4f &v4Color, float fFrame, float fParam, ResHandle hMaterial, int iEffectLayer, float fDepth);
	void	ForceEmpty();

	void	ObjectBounds(CBBoxf &bbObjects);
	void	AddWorld(EMaterialPhase ePhase);

	CCamera	Camera;

	GLuint	VBScenePoly;			// Vertex buffer for scene polys
	GLuint	VBEffectTriangle;		// Vertex buffer for effect triangles

	GLuint	VBBillboard;			// Vertex buffer for billboards

	GLuint	VBTreeBillboard;		// Vertex buffer for billboards
	GLuint	IBTreeBillboard;		// Index buffer for billboards

	GLuint	VBBox;					// Vertex buffer for drawing bounding boxes
	GLuint	IBBox;					// Index buffer for drawing bounding boxes

	GLuint	VBPoint;				// Vertex buffer for drawing points

	GLuint	VBLine;					// Vertex buffer for drawing lines
};
extern CGfx3D *Gfx3D;
//=============================================================================

#endif //__C_GFX3D_H__

