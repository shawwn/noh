// (C)2008 S2 Games
// c_tilevisionmap.h
//
//=============================================================================
#ifndef __C_TILEVISIONMAP_H__
#define __C_TILEVISIONMAP_H__

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
// CTileVisBlockerMap
//=============================================================================
class CTileVisBlockerMap : public IWorldComponent
{
private:
	byte*			m_pTileVisBlockers;

public:
	~CTileVisBlockerMap();
	CTileVisBlockerMap(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	bool	Serialize(IBuffer *pBuffer);

	bool	GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
	bool	SetRegion(const CRecti &recArea, void *pSource, int iLayer);

	byte*	GetVisBlockerMap() const		{ return m_pTileVisBlockers; }

	K2_API byte	GetVisBlocker(int x, int y);
	K2_API void	SetVisBlocker(int x, int y, byte yVisBlocker);
};
//=============================================================================

#endif //__C_TILEVISIONMAP_H__
