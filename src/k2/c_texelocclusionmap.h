// (C)2007 S2 Games
// c_texelocclusionmap.h
//
//=============================================================================
#ifndef __C_TEXELOCCLUSIONMAP_H__
#define __C_TEXELOCCLUSIONMAP_H__

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
// CTexelOcclusionMap
//=============================================================================
class CTexelOcclusionMap : public IWorldComponent
{
private:
	bool		m_bActive;
	byte		*m_pTexelOcclusion;

public:
	~CTexelOcclusionMap();
	CTexelOcclusionMap(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	bool	Serialize(IBuffer *pBuffer);

	bool	GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
	bool	SetRegion(const CRecti &recArea, void *pSource, int iLayer);

	K2_API byte	GetTexelOcclusion(int iX, int iY);
	K2_API void	SetOcclusion(int iX, int iY, byte yAlpha);

	K2_API void	Calculate(int iSamples);
};
//=============================================================================
#endif //__C_TEXELALPHAMAP_H__
