// (C)2006 S2 Games
// c_sceneentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_sceneentity.h"
//=============================================================================

/*====================
  CSceneEntity::~CSceneEntity
  ====================*/
CSceneEntity::~CSceneEntity()
{
}


/*====================
  CSceneEntity::CSceneEntity
  ====================*/
CSceneEntity::CSceneEntity() :
m_v3Position(V_ZERO),
angle(V_ZERO),
axis(),
scale(1.0f),

hRes(INVALID_RESOURCE),
hSkin(0),

s1(0.0f),
t1(0.0f),
s2(1.0f),
t2(1.0f),

color(0.0f, 0.0f, 0.0f, 0.0f),

skeleton(NULL),

width(0.0f),
height(0.0f),

objtype(),

flags(0),

custom_mapping(NULL),

effectlayer(0),
effectdepth(0.0f),

frame(0.0f),
param(0.0f)
{
}


/*====================
  CSceneEntity::Clear
  ====================*/
void	CSceneEntity::Clear()
{
	m_v3Position.Clear();

	angle.Clear();
	axis.Clear();
	scale = 1.0f;

	hRes = INVALID_RESOURCE;
	hSkin = 0;

	s1 = 0.0f;
	t1 = 0.0f;
	s2 = 1.0f;
	t2 = 1.0f;

	color = CVec4f(1.0f, 1.0f, 1.0f, 1.0f);

	skeleton = NULL;

	width = 0.0f;
	height = 0.0f;

	objtype = OBJTYPE_MODEL;

	flags = 0;

	custom_mapping = NULL;

	effectlayer = 0;
	effectdepth = 0.0f;

	frame = 0.0f;
	param = 0.0f;

	teamcolor = CVec4f(1.0f, 1.0f, 1.0f, 1.0f);
}
