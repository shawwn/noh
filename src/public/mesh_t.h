// (C)2005 S2 Games
// mesh_t.h
//
//=============================================================================
#ifndef __MESH_T__
#define __MESH_T__

//=============================================================================
// Headers
//=============================================================================
#include "../shared/shared_types.h"
//=============================================================================

typedef int singleLink_t;

const int MAX_RENDER_DATA = 8;

typedef enum
{
	RD_VERTS,
	RD_TVERTS,
	RD_COLORS,
	RD_NORMALS,
}
renderData_enum;

//a mesh is like a "sub object" of a model
//meshes may use any of the 3 following "modes":
//
//MESH_SKINNED_BLENDED
//	each vertex on the mesh is linked to one or more bones
//  if any one vertex in the mesh is linked to more than one bone, this is the mode that gets set
//MESH_SKINNED_NONBLENDED
//  one bone link per vertex
//  this mode gets set for all other meshes, including meshes that did not have physique applied
//  (a bone will be generated for this mesh that all vertices are linked to)
//  even meshes which use keyframe data will have their geometry set (though it may not get used)

typedef enum
{
	MESH_SKINNED_BLENDED = 1,
	MESH_SKINNED_NONBLENDED	
}
meshModes_enum;

typedef struct mesh_s
{
	class CModel	*model;		//model this mesh belongs to

	tstring			sName;

	char			defaultShader[SKIN_SHADERNAME_LENGTH];

	int				mode;		//see MESH_* defines above

	int				flags;

	vec3_t			bmin;		//bounding box (in MESH coordinates)
	vec3_t			bmax;		//bounding box (in MESH coordinates)
	
	int				numFaces;
	uivec3_t		*faceList;

	int				num_verts;	//number of vertices
	
	vec2_t			*tverts[MAX_UV_CHANNELS];	//texture coords
	bvec4_t			*colors[MAX_VERTEX_COLOR_CHANNELS];	//vertex colors
	vec3_t			*normals;	//vertex normals
	vec3_t			*tangents[MAX_UV_CHANNELS];	//vertex tangents
	vec3_t			*verts;		//vertex coords (always in MODEL space)
	vec3_t			*boneSpaceVerts;	//vertex coords (in BONE space, to optimize calculations)

	/*  
		mode == MESH_SKINNED_BLENDED

		The verts array will store coordinates in MODEL space
		(the world coords of the 3dsmax scene).  Verts are
		then blended via:

		for (v = 0; v < mesh->num_verts; ++v)
		{
			blendedVert_t *blend = &mesh->blended_verts[v];
			M_ClearVec3(outVerts[v]);
			for (link = 0; link < mesh->blended_verts[v].num_links; ++link)
			{
				vec3_t point;
				CBone *bone = &model->bones[blend->links[link]];

				M_TransformPoint(mesh->verts[v], bone->invBase, point);		//get the point into the initial bone space
				M_TransformPoint(point, bone->current_transform, point);	//transform the point into the current bone space
				M_ScaleVec3(point, blend->weights[link], point);
				M_AddVec3(outVerts[v], point, outVerts[v]);
			}
		}

	*/
	byte					*linkPool;		//blendedLinks sets pointers here, rather than allocating its own memory
	struct blendedLink_s	*blendedLinks;	//vertBlend only allocated if mode == MESH_SKINNED_BLENDED

	/* 
		mode == MESH_SKINNED_NONBLENDED  (or LOD fallback for MESH_SKINNED_BLENDED)

		The verts array will store coordinates in BONE space, 
		so the calculation is simple:

		for (v = 0; v < mesh->num_verts; ++v)
		{
			M_TransformPoint(mesh->verts[v], bones[mesh->singleLinks[v]]->current_transform, outVerts[v]);
		}

		We could do this calculation in hardware if we split up the mesh according to link
	*/
	singleLink_t	*singleLinks;	//allocated for nonblended meshes.  can also be used as an LOD fallback for blended meshes.

	int				bonelink;		//if > -1, ALL vertices of this mesh are linked to this bone only, in which case both
									//singleLinks and blendedLinks will be NULL.

	int				hasRenderData;	
	void			*renderData[MAX_RENDER_DATA];	//this may be used by the renderer to cache static geometry
	int				renderDataRefCount;				//refcount is used to make sure we don't reference old data

	int				sbuffer;						// index to static vertex buffer
	int				dbuffer;						// index to dynamic vertex buffer
	int				ibuffer;						// index to index buffer
	int				vertexStride;					// vertex stride
	dword			vertexFVF;						// vertex fvf
	int				vertexDecl;						// vertex declaration
}
mesh_t;

#endif // __MESH_T__ 
