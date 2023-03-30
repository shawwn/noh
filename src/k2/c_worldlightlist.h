// (C)2006 S2 Games
// c_worldlightlist.h
//
//=============================================================================
#ifndef __C_WORLDLIGHTLIST_H__
#define __C_WORLDLIGHTLIST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldLight;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<uint, CWorldLight*>		WorldLightsMap;
typedef pair<uint, CWorldLight*>	WorldLightsEntry;
typedef WorldLightsMap::iterator	WorldLightsMap_it;
//=============================================================================

//=============================================================================
// CWorldLightList
//=============================================================================
class CWorldLightList : public IWorldComponent
{
private:
	WorldLightsMap	m_mapLights;

public:
	~CWorldLightList();
	CWorldLightList(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	bool	Serialize(IBuffer *pBuffer);

	K2_API uint				AllocateNewLight(uint uiIndex = INVALID_INDEX);
	K2_API CWorldLight*		GetLight(uint uiIndex, bool bThrow = NO_THROW);
	K2_API WorldLightsMap&	GetLightsMap()									{ return m_mapLights; }
	K2_API void				DeleteLight(uint uiIndex);
};
//=============================================================================

#endif //__C_WORLDLIGHTLIST_H__
