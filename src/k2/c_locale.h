// (C)2005 S2 Games
// c_locale.h
//
//=============================================================================
#ifndef __C_LOCALE_H__
#define __C_LOCALE_H__

//=============================================================================
// Headers
//=============================================================================
#include "shared_api.h"

#include "c_worldblockhandle.h"
#include "c_entiterator.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldBlock;
//=============================================================================

//=============================================================================
// CLocale
// CWorld allocates one of these for each client and references CWorldBlocks
// relative to the client it belongs to
//=============================================================================
class SHARED_API CLocale
{
private:
	CBlockHandle	m_pBlockHandles[NUM_LOCAL_BLOCKS];

public:
	CLocale();
	~CLocale();

	const CBlockHandle&	operator[](int i) const	{ return m_pBlockHandles[i]; }
	CBlockHandle&		Get(int i)				{ return m_pBlockHandles[i]; }

	void			Shift(int iDir);
	void			UpdateNeighborLinks();
	static int		GetNeighboringBlockNum(int fromBlock, int iDir);
	int				GetBlockNum(const CWorldBlock *pBlock);

	void			TranslateCoords(int fromBlock, int toBlock, const CVec3f &in, CVec3f &out);

	CEntIterator	GetEntIterator() const		{ return CEntIterator(*this); }
};

/*====================
  CLocale::GetNeighborBlockNum

  Returns neighboring block number if it exists.
  ====================*/
inline
int CLocale::GetNeighboringBlockNum(int fromBlock, int iDir)
{
	if (ABS((fromBlock % 3) + (iDir % 3) - 2) > 1 ||
		ABS((fromBlock / 3) + (iDir / 3) - 2) > 1)
		return -1;

	return fromBlock + (iDir - CENTER);
}
//=============================================================================
#endif //__C_LOCALE_H__
