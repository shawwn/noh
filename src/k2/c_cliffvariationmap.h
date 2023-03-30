// (C)2005 S2 Games
// c_VertexCliffMap.h
//
//=============================================================================
#ifndef __C_VertexCliffMap_H__
#define __C_VertexCliffMap_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorld;
//=============================================================================

//=============================================================================
// CCliffVariationMap
//=============================================================================
class CCliffVariationMap : public IWorldComponent
{
private:
	uint				*m_pTileCliffDefinition;
	uint				*m_pTileCliffVariation;
	int					m_iCliffMapWidth;

	void	CalculateVertexNormal(int iX, int iY);

public:
	~CCliffVariationMap();
	CCliffVariationMap(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	void	Update(const CRecti &recArea);
	bool	Serialize(IBuffer *pBuffer);

	int		GetVariationMapWidth() { return m_iCliffMapWidth; }

	bool	GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
	bool	SetRegion(const CRecti &recArea, void *pSource, int iLayer);

	K2_API const uint	GetTileCliff(int x, int y);

	uint*	GetCliffDefinitionMap() 		{ return m_pTileCliffDefinition; }
	uint*	GetCliffVariationMap()			{ return m_pTileCliffVariation; }

	//K2_API byte	GetCliff(int x, int y);
};
//=============================================================================

#endif //__C_VertexCliffMap_H__
