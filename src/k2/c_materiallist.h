// (C)2005 S2 Games
// c_materiallist.h
//
//=============================================================================
#ifndef __C_MATERIALLIST_H__
#define __C_MATERIALLIST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SMaterialListEntry
{
	uint		uiID;
	ResHandle	hMaterial;
};
//=============================================================================

//=============================================================================
// CMaterialList
//=============================================================================
class CMaterialList : public IWorldComponent
{
private:
	typedef map<uint, SMaterialListEntry>	MaterialIDMap;
	typedef map<ResHandle, SMaterialListEntry>		MaterialHandleMap;

	MaterialIDMap		m_mapMaterials;
	MaterialHandleMap	m_mapResHandles;

public:
	~CMaterialList();
	CMaterialList(EWorldComponent eComponent);

	bool		Load(CArchive &archive, const CWorld *pWorld);
	bool		Generate(const CWorld *pWorld);
	bool		Serialize(IBuffer *pBuffer);
	void		Release();

	K2_API uint	AddMaterial(ResHandle hResource);
	K2_API ResHandle		GetMaterialHandle(uint uiID);
	K2_API uint	GetMaterialID(ResHandle hMaterial);
	K2_API void			AddMaterial(uint uiID, const tstring &sMaterial);
};
//=============================================================================
#endif //__C_MATERIALLIST_H__
