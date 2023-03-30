// (C)2005 S2 Games
// c_mesh.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_mesh.h"
#include "../public/blendedlink_t.h"
//=============================================================================

/*====================
  CMesh::CMesh
  ====================*/
CMesh::CMesh() :
m_pModel(NULL),
m_fCumArea(0.0f),
m_vFaceAreas(0),

mode(0),
flags(0),
surfflags(0),
renderflags(0),
numFaces(0),
faceList(NULL),
num_verts(0),
num_uv_channels(0),
num_color_channels(0),
verts(NULL),
normals(NULL),
linkPool(NULL),
blendedLinks(NULL),
singleLinks(NULL),
bonelink(0),
sbuffer(-1),
dbuffer(-1),
ibuffer(-1),
vertexStride(0),
vertexFVF(0),
vertexDecl(0)
{
	M_ClearVec3(bmin);
	M_ClearVec3(bmax);

	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		tverts[n] = NULL;
		tangents[n] = NULL;
		signs[n] = NULL;
	}

	for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
		colors[n] = NULL;
}


/*====================
  CMesh::~CMesh
  ====================*/
CMesh::~CMesh()
{
}


/*====================
  CMesh::PostLoad

  Throw away data we don't need during runtime
  ====================*/
void	CMesh::PostLoad()
{
	if (normals)
		renderflags |= MESH_NORMALS;

	// TODO: Detect the renderer and decide to keep it or not based on that
	//SAFE_DELETE_ARRAY(normals);
	//SAFE_DELETE_ARRAY(linkPool);
	//SAFE_DELETE_ARRAY(blendedLinks);
	//SAFE_DELETE_ARRAY(singleLinks);

	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (tverts[n] != NULL)
			++num_uv_channels;

		//SAFE_DELETE_ARRAY(tverts[n]);
		//SAFE_DELETE_ARRAY(tangents[n]);
	}

	for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		if (colors[n] != NULL)
			++num_color_channels;

		//SAFE_DELETE_ARRAY(colors[n]);
	}
}


/*====================
  CMesh::Free
  ====================*/
void	CMesh::Free()
{
	SAFE_DELETE_ARRAY(faceList);
	SAFE_DELETE_ARRAY(normals);
	SAFE_DELETE_ARRAY(verts);
	SAFE_DELETE_ARRAY(linkPool);
	SAFE_DELETE_ARRAY(blendedLinks);
	SAFE_DELETE_ARRAY(singleLinks);

	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		SAFE_DELETE_ARRAY(tverts[n]);
		SAFE_DELETE_ARRAY(tangents[n]);
		SAFE_DELETE_ARRAY(signs[n]);
	}

	for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		SAFE_DELETE_ARRAY(colors[n]);
	}
}
