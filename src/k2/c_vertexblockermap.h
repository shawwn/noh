// (C)2007 S2 Games
// c_vertexblockermap.h
//
//=============================================================================
#ifndef __C_VERTEXBLOCKERMAP_H__
#define __C_VERTEXBLOCKERMAP_H__

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
// CVertexBlockerMap
//=============================================================================
class CVertexBlockerMap : public IWorldComponent
{
private:
	byte*			m_pVertexBlockers;

public:
	~CVertexBlockerMap();
	CVertexBlockerMap(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	bool	Serialize(IBuffer *pBuffer);

	bool	GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
	bool	SetRegion(const CRecti &recArea, void *pSource, int iLayer);

	byte*	GetBlockerMap() const	{ return m_pVertexBlockers; }

	K2_API byte	GetBlockers(int x, int y);
	K2_API void	SetBlockers(int x, int y, byte yBlockers);

	K2_API bool	CalcBlocked(const CRecti &recArea) const;
};
//=============================================================================

#endif //__C_VERTEXCOLORMAP_H__
