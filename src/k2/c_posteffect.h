// (C)2008 S2 Games
// c_posteffect.h
//
//=============================================================================
#ifndef __C_POSTEFFECT_H__
#define __C_POSTEFFECT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// CPostFilter
//=============================================================================
class CPostFilter
{
private:
	ResHandle	m_hMaterial;

public:
	~CPostFilter() {}
	CPostFilter() : m_hMaterial(INVALID_RESOURCE) {}
	CPostFilter(ResHandle hMaterial) : m_hMaterial(hMaterial) {}

	void		SetMaterial(ResHandle hMaterial)	{ m_hMaterial = hMaterial; }
	ResHandle	GetMaterial() const					{ return m_hMaterial; }
};
//=============================================================================


//=============================================================================
// CPostEffect
//=============================================================================
class CPostEffect : public IResource
{
protected:
	vector<CPostFilter>		m_vFilters;

	CPostEffect();

public:
	~CPostEffect()	{}
	CPostEffect(const tstring &sPath) :
	IResource(sPath, TSNULL)
	{}

	K2_API	virtual uint			GetResType() const			{ return RES_POST_EFFECT; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{posteffect}")); return sTypeName; }

	const vector<CPostFilter>&	GetFilters() const
	{
		return m_vFilters;
	}
	
	void	AddFilter(const CPostFilter &cFilter)	{ m_vFilters.push_back(cFilter); }
	
	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free()									{}
};
//=============================================================================

#endif //__C_POSTEFFECT_H__
