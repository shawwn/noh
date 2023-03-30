// (C)2008 S2 Games
// c_gfxterrain.h
//
//=============================================================================
#ifndef __C_GFXTERRAIN_H__
#define __C_GFXTERRAIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EMaterialPhase;
class CConvexPolyhedron;

#define	TERRAIN_REBUILD_VERTICES	BIT(0)
#define	TERRAIN_REBUILD_COLORS		BIT(1)
#define	TERRAIN_REBUILD_TEXCOORDS	BIT(2)
#define	TERRAIN_REBUILD_SHADERS		BIT(3)
#define TERRAIN_REBUILD_NORMALS		BIT(4)
#define TERRAIN_REBUILD_ALPHAMAP	BIT(5)

#define	VERTS_PER_CHUNK	((GfxTerrain->iChunkSize + 1) * (GfxTerrain->iChunkSize + 1))
#define	TILES_PER_CHUNK	((GfxTerrain->iChunkSize) * (GfxTerrain->iChunkSize))
#define	MAX_ELEMS_PER_CHUNK (GfxTerrain->iChunkSize * GfxTerrain->iChunkSize * 6)

const int MAX_CHUNKS_X = 32;
const int MAX_CHUNKS_Y = 32;

EXTERN_CVAR_BOOL(vid_terrainSinglePass);

struct STerrainArray
{
	ResHandle		hMaterial;
	ResHandle		ahDiffuse[NUM_TERRAIN_LAYERS];
	ResHandle		ahNormalmap[NUM_TERRAIN_LAYERS];
	
	uint	*pElemList;
	uint	uiNumElems;

	uint	uiNumFaces;
	
	uint	uiStartIndex;
};

struct STerrainCliffArray
{
	ResHandle		hMaterial;
	ResHandle		hDiffuse;
	ResHandle		hNormalmap;

	uint	uiNumVerts;
	uint	uiNumFaces;
	
	uint	uiStartVert;
	uint	uiStartIndex;

	vector<pair<CWorldEntity *, uint> >	vCliffs;
};

struct STerrainChunk
{
	int				iNumArrays;
	STerrainArray	*pArrays[256];

	int				iNumCliffArrays;
	STerrainCliffArray	*pCliffArrays[256];

	int				iValidityFlags;
	bool			bVisible;
	bool			bVisibleShadow;

	GLuint			uiVB;
	GLuint			uiIB;

	uint			uiAlphaMap;
	CBBoxf			bbBounds;

	GLuint			uiVBCliff;
	GLuint			uiIBCliff;

	int				iNumCliffVerts;
	int				iNumCliffFaces;

	uint			uiNumFaces;
};
//=============================================================================

//=============================================================================
// CGfxTerrain
//=============================================================================
class CGfxTerrain
{
	SINGLETON_DEF(CGfxTerrain)
private:
	int				m_iCliffVertexStride;
	AttributeMap	m_mapCliffAttributes;

	int					TerrainTextureCompare(ResHandle ah0[], ResHandle ah1[]);
	STerrainArray*		AllocTerrainArray();
	STerrainCliffArray*	AllocTerrainCliffArray();

	void			RebuildCliffs();

public:
	~CGfxTerrain();

	void	Init();
	void	Destroy();
	void	Rebuild(int chunkSize, const class CWorld *pWorld);
	void	Shutdown();

	void	InvalidateTerrainVertex(int iTileX, int iTileY, int iFlags);
	void	InvalidateTerrainTile(int iTileX, int iTileY, int iFlags);
	void	InvalidateTerrainTexel(int iTexelX, int iTexelY, int iFlags);
	void	InvalidateTerrainLayer(int flags);

	void	AddTerrainChunks(EMaterialPhase ePhase);
	void	FlagVisibleTerrainChunks();
	void	FlagVisibleTerrainChunksShadow(const CConvexPolyhedron &cScene);
	void	TerrainBounds(CBBoxf &bbTerrain);

	const	CWorld		*pWorld;

	void	AllocateTerrainChunk(int iX, int iY);
	void	DestroyTerrainChunk(int iX, int iY);
	void	RebuildTerrainChunk(int iX, int iY);

	int				GetCliffStride() const		{ return m_iCliffVertexStride; }
	AttributeMap&	GetCliffAttributes()		{ return m_mapCliffAttributes; }

	int					iChunkSize;
	int					iNumChunksX;
	int					iNumChunksY;
	STerrainChunk		chunks[MAX_CHUNKS_Y][MAX_CHUNKS_X];

	ResHandle			hTerrainMaterial;
	ResHandle			hTerrainSingleMaterial;
	ResHandle			hTerrainDiffuseReference;
	ResHandle			hTerrainNormalmapReference;
	ResHandle			hTerrainDiffuse2Reference;
	ResHandle			hTerrainNormalmap2Reference;
	ResHandle			hTerrainCheckerDiffuse;
	ResHandle			hTerrainCheckerNormalmap;
	GLuint				uiTerrainAlphaMap;

	AttributeMap		mapAttributes;
};
extern CGfxTerrain *GfxTerrain;
//=============================================================================

#endif //__C_GFXTERRAIN_H__
