// (C)2005 S2 Games
// c_heightmap.h
//
//=============================================================================
#ifndef __C_HEIGHTMAP_H__
#define __C_HEIGHTMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// CHeightMap
//=============================================================================
class CHeightMap : public IWorldComponent
{
private:
	float	*m_pHeightMap;

public:
	~CHeightMap();
	CHeightMap(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	bool	Serialize(IBuffer *pBuffer);
	void	Release();

	bool	GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
	bool	SetRegion(const CRecti &recArea, void *pSource, int iLayer);

	K2_API float	GetGridPoint(int iX, int iY);
	K2_API float	GetGridPoint(int i);

	K2_API float	CalcMaxHeight(const CRecti &recArea) const;
	K2_API float	CalcMinHeight(const CRecti &recArea) const;

	float*	GetHeightMap() const		{ return m_pHeightMap; }
};
//=============================================================================
#endif //__C_HEIGHTMAP_H__
