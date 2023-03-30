// (C)2005 S2 Games
// c_texturelist.h
//
//=============================================================================
#ifndef __C_TEXTURELIST_H__
#define __C_TEXTURELIST_H__

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
// CTextureList
//=============================================================================
class CTextureList : public IWorldComponent
{
private:
	typedef map<uint, ResHandle>	TextureIDMap;
	typedef map<ResHandle, uint>	TextureHandleMap;
	typedef set<uint>				TextureUsageSet;

	TextureIDMap		m_mapTextures;
	TextureHandleMap	m_mapResHandles;
	TextureUsageSet		m_setUsed;

public:
	~CTextureList();
	CTextureList(EWorldComponent eComponent);

	bool		Load(CArchive &archive, const CWorld *pWorld);
	bool		Generate(const CWorld *pWorld);
	bool		Serialize(IBuffer *pBuffer);
	void		Release();
	
	void		ClearTextureIDUsage();

	K2_API uint		AddTexture(ResHandle hTexture);
	K2_API ResHandle	GetTextureHandle(uint uiID);
	K2_API uint		GetTextureID(ResHandle hTexture);
	K2_API void		AddTexture(uint uiID, const tstring &sTexture);
	K2_API void		GetTextureIDList(vector<uint> &vuiID);
	K2_API void		SetTextureIDUsed(uint uiID);
};
//=============================================================================
#endif //__C_TEXTURELIST_H__
