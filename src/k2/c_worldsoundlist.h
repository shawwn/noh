// (C)2007 S2 Games
// c_worldsoundlist.h
//
//=============================================================================
#ifndef __C_WORLDSOUNDLIST_H__
#define __C_WORLDSOUNDLIST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldSound;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<uint, CWorldSound*>		WorldSoundsMap;
typedef pair<uint, CWorldSound*>	WorldSoundsEntry;
typedef WorldSoundsMap::iterator	WorldSoundsMap_it;
//=============================================================================

//=============================================================================
// CWorldSoundList
//=============================================================================
class CWorldSoundList : public IWorldComponent
{
private:
	WorldSoundsMap	m_mapSounds;

public:
	~CWorldSoundList();
	CWorldSoundList(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	bool	Serialize(IBuffer *pBuffer);

	K2_API uint				AllocateNewSound(uint uiIndex = INVALID_INDEX);
	K2_API CWorldSound*		GetSound(uint uiIndex, bool bThrow = NO_THROW);
	K2_API WorldSoundsMap&	GetSoundsMap()									{ return m_mapSounds; }
	K2_API void				DeleteSound(uint uiIndex);
};
//=============================================================================

#endif //__C_WORLDSOUNDLIST_H__
