//(C)2006 S2 Games
// c_vertexfoliagemap.h
//
//=============================================================================
#ifndef __C_VERTEXFOLIAGEMAP_H__
#define __C_VERTEXFOLIAGEMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SFoliageVertexEntry
{
	float	fDensity;
	CVec3f	v3Size;
	CVec3f	v3Variance;
	CVec3f	v3Color;
};
//=============================================================================

//=============================================================================
// CVertexFoliageMap
//=============================================================================
class CVertexFoliageMap : public IWorldComponent
{
private:
	SFoliageVertexEntry		*m_pFoliageVertices[NUM_FOLIAGE_LAYERS];

public:
	CVertexFoliageMap(EWorldComponent eComponent);
	~CVertexFoliageMap();

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	bool	Serialize(IBuffer *pBuffer);
	void	Release();

	bool	GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
	bool	SetRegion(const CRecti &recArea, void *pSource, int iLayer);

	K2_API float	GetFoliageDensity(int iX, int iY, int iLayer);
	K2_API const CVec3f&	GetFoliageSize(int iX, int iY, int iLayer);
	K2_API const CVec3f&	GetFoliageVariance(int iX, int iY, int iLayer);
	K2_API const CVec3f&	GetFoliageColor(int iX, int iY, int iLayer);
};
//=============================================================================

#endif //__C_VERTEXFOLIAGEMAP_H__
