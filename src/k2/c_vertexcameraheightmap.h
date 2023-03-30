// (C)2008 S2 Games
// c_vertexcameraheightmap.h
//
//=============================================================================
#ifndef __C_VERTEXCAMERAHEIGHTMAP_H__
#define __C_VERTEXCAMERAHEIGHTMAP_H__

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
// CVertexCameraHeightMap
//=============================================================================
class CVertexCameraHeightMap : public IWorldComponent
{
private:
	float	*m_pHeightMap;

public:
	~CVertexCameraHeightMap();
	CVertexCameraHeightMap(EWorldComponent eComponent);

	bool	Save(CArchive &archive)		{ return true; }
	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	void	Update(const CRecti &recArea);

	bool	GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
	bool	SetRegion(const CRecti &recArea, void *pSource, int iLayer);

	K2_API float	GetGridPoint(int iX, int iY);
	K2_API float	GetGridPoint(int i);

	float*	GetHeightMap() const		{ return m_pHeightMap; }
};
//=============================================================================

#endif //__C_VERTEXCAMERAHEIGHTMAP_H__
