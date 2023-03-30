// (C)2005 S2 Games
// c_Clifflist.h
//
//=============================================================================
#ifndef __C_cliffLIST_H__
#define __C_cliffLIST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CCliffList
//=============================================================================
class CCliffList : public IWorldComponent
{
private:
	typedef map<uint, ResHandle>	CliffIDMap;
	typedef map<ResHandle, uint>	CliffHandleMap;

	CliffIDMap		m_mapCliffs;
	CliffHandleMap	m_mapResHandles;

public:
	~CCliffList();
	CCliffList(EWorldComponent eComponent);

	bool		Load(CArchive &archive, const CWorld *pWorld);
	bool		Generate(const CWorld *pWorld);
	bool		Serialize(IBuffer *pBuffer);
	void		Release();
	
	K2_API uint			AddCliff(ResHandle hCliff);
	K2_API ResHandle	GetCliffHandle(uint uiID);
	K2_API uint			GetCliffID(ResHandle hCliff);
	K2_API void			AddCliff(uint uiID, const tstring &sCliff);
	K2_API void			GetCliffIDList(vector<uint> &vuiID);
};
//=============================================================================
#endif //__C_CliffLIST_H__
