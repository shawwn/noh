#ifndef __SCENEFACELIST_T__
#define __SCENEFACELIST_T__

typedef struct scenefacelist_s
{	
	SSceneFaceVert			*verts;
	int						nverts;
	ResHandle				hMaterial;
	bool					cull;
	int						flags;	//see POLY_* defines in savage_types.h
	struct scenefacelist_s	*next;
} scenefacelist_t;

#endif // __SCENEFACELIST_T__