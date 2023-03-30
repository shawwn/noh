// (C)2005 S2 Games
// c_tilesplitmap.h
//
//=============================================================================
#ifndef __C_TILESPLITMAP_H__
#define __C_TILESPLITMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorld;
class CArchive;
//=============================================================================

//=============================================================================
// CTileSplitMap
//=============================================================================
class CTileSplitMap : public IWorldComponent
{
private:
	byte		*m_pTileSplitMap;

	void	CalculateTileSplit(int iX, int iY);

public:
	~CTileSplitMap();
	CTileSplitMap(EWorldComponent eComponent);

	bool	Save(CArchive &archive)		{ return true; }
	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	void	Update(const CRecti &recArea);

	K2_API ETileSplitType	GetTileSplit(int iX, int iY) const;

	byte*	GetSplitMap() const		{ return m_pTileSplitMap; }
};
//=============================================================================
#endif
