// (C)2005 S2 Games
// d3d9g_scene.h
//
// Direct3D Scene
//=============================================================================
#ifndef __D3D9_SCENE_H__
#define __D3D9_SCENE_H__

extern CCvar<bool>		gfx_foliage;
extern CCvar<tstring>	gfx_foliageShader;
extern CCvar<bool>		gfx_clouds;
extern CCvar<tstring>	gfx_cloudTexture;

extern CCvar<bool>		gfx_depthFirst;
extern CCvar<int>		gfx_fogType;

extern CCvar<bool>		gfx_lighting;
extern CCvar<int>		gfx_textures;
extern CCvar<int>		gfx_wireframe;
extern CCvar<int>		gfx_points;
extern CCvar<int>		gfx_normals;

extern CCvar<int>		vid_maxDynamicLights;
extern CCvar<bool>		vid_dynamicLights;
extern CCvar<float>		vid_skyEpsilon;

class CSceneEntity;
enum EMaterialPhase;
extern  const CSceneEntity	*g_pCurrentEntity;

extern ResHandle		g_hCloudTexture;

extern int				g_iMaxDynamicLights;
extern int				g_iNumGuiQuads;

void	D3D_SetupScene();
void	D3D_SetupCamera(const CCamera &camera);
void	D3D_ObjectBounds(CBBoxf &bbObjects);
void	D3D_AddSceneEntities();

void	D3D_Add2dRect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags);
void	D3D_Add2dQuad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4,
					  const CVec2f& t1, const CVec2f& t2, const CVec2f& t3, const CVec2f& t4, ResHandle hTexture, int iFlags);
void	D3D_Add2dLine(const CVec2f& v1, const CVec2f& v2, const CVec4f &v4Color1, const CVec4f &v4Color2, int iFlags);

void	D3D_DrawQuads();
void	D3D_AddBox(const CBBoxf &bbBox, const CVec4f &v4Color, const D3DXMATRIXA16 &mWorld);
void	D3D_AddPoint(const CVec3f &v3Pos, const CVec4f &v4Color);
void	D3D_AddLine(const CVec3f &v3Start, const CVec3f &v3End, const CVec4f &v4Color);

void	D3D_AddTreeBillboardBatches(EMaterialPhase ePhase);

void	D3D_AddBillboard(const CVec3f &v3Pos, float width, float height, float angle, float s1, float t1, float s2, float t2, float frame, float param, ResHandle hMaterial, uint uiFlags, float fPitch, float fYaw, float fDepthBias, int iEffectLayer, float fDepth, const CAxis &aAxis, const CVec4f &v4Color);
void	D3D_AddEffectTriangle(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2, dword color0, dword color1, dword color2, const CVec4f &t0, const CVec4f &t1, const CVec4f &t2, ResHandle hMaterial);
void	D3D_AddTreeBillboard(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, dword color0, dword color1, dword color2, dword color3, const CVec2f &t0, const CVec2f &t1, const CVec2f &t2, const CVec2f &t3, ResHandle hMaterial, dword dwAlphaTest, const D3DXMATRIXA16 &mWorld);

extern bool		g_bActiveReflection;

#endif // __D3D9_SCENE_H__