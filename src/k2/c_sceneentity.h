// (C)2006 S2 Games
// c_sceneentity.h
//
//=============================================================================
#ifndef __C_SCENEENTITY_H__
#define __C_SCENEENTITY_H__

//=============================================================================
// Declarations
//=============================================================================
class CSkeleton;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint		SCENEENT_NO_SHADOW				(BIT(0));
const uint		SCENEENT_SOLID_COLOR			(BIT(1));
const uint		SCENEENT_SHOW_BOUNDS			(BIT(2));
const uint		SCENEENT_MESH_BOUNDS			(BIT(3));
const uint		SCENEENT_NEVER_CULL				(BIT(4));
const uint		SCENEENT_NO_ZWRITE				(BIT(5));
const uint		SCENEENT_NO_ZTEST				(BIT(6));
const uint		SCENEENT_USE_AXIS				(BIT(7));
const uint		SCENEENT_USE_BOUNDS				(BIT(8));
const uint		SCENEENT_WIREFRAME				(BIT(9));
const uint		SCENEENT_POINTS					(BIT(10));
const uint		SCENEENT_BILLBOARD_ALL_AXES		(BIT(11));
const uint		SCENEENT_SHOW_WIRE				(BIT(12));
const uint		SCENEENT_RTS_SILHOUETTE			(BIT(13));		// draw the silhouette if occluded by something.  uses the sceneobject color to determine the color of the silhouette
const uint		SCENEENT_SINGLE_MATERIAL		(BIT(14));		// ignore the skin and use a single shader for meshes
const uint		SCENEENT_ALWAYS_BLEND			(BIT(15));
const uint		SCENEENT_NO_LIGHTING			(BIT(16));
const uint		SCENEENT_NO_ADD_TEX				(BIT(17));
const uint		SCENEENT_NO_ALPHATEST			(BIT(18));
const uint		SCENEENT_FOG_OF_WAR				(BIT(19));		// Project fog of war texture on to this entity
const uint		SCENEENT_NO_FOG					(BIT(20));
const uint		SCENEENT_TERRAIN_TEXTURES		(BIT(21));

const uint		SCENEENT_SKYBOX		(SCENEENT_SOLID_COLOR | SCENEENT_ALWAYS_BLEND | SCENEENT_NO_ALPHATEST);
//=============================================================================

//=============================================================================
// CSceneEntity
//=============================================================================
class K2_API CSceneEntity
{
private:
	CVec3f		m_v3Position;

public:
	~CSceneEntity();
	CSceneEntity();

	void		Clear();

	const CVec3f&	GetPosition() const						{ return m_v3Position; }
	void		SetPosition(const CVec3f &v3)				{ m_v3Position = v3; }
	void		SetPosition(float fX, float fY, float fZ)	{ m_v3Position = CVec3f(fX, fY, fZ); }
	void		Translate(const CVec3f &v3)					{ m_v3Position += v3; }
	void		Translate(float fX, float fY, float fZ)		{ m_v3Position += CVec3f(fX, fY, fZ); }

	////
	CVec3f		angle;
	CAxis		axis;			// used only if SCENEENT_USE_AXIS is set, otherwise 'angle' is used
	float		scale;

	ResHandle	hRes;			// model for models, material for polys
	uint		hSkin;			// skin index for models, material override if SCENEENT_SINGLE_MATERIAL

	float		s1;				// used only if it's a billboard
	float		t1;
	float		s2;
	float		t2;

	CVec4f		color;			// used if SCENEENT_SOLID_COLOR is set

	CSkeleton	*skeleton;		// information about bone positions, for deforming characters

	float		width, height;	// for billboards

	int			objtype;

	uint		flags;			// see SCENEENT_* defines above

	mutable int		*custom_mapping;	// used internally

	int			effectlayer;
	float		effectdepth;
	float		frame;
	float		param;
	
	CBBoxf		bounds;

	CVec4f		teamcolor;
};
//=============================================================================

#endif //__C_SCENEENTITY_H__
