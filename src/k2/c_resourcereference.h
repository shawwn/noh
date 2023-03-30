// (C)2006 S2 Games
// c_resourcereference.h
//
//=============================================================================
#ifndef __C_RESOURCE_REFERENCE_H__
#define __C_RESOURCE_REFERENCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CResourceReference
//=============================================================================
class CResourceReference : public IResource
{
private:
	ResHandle		m_hReference;

public:
	K2_API ~CResourceReference();
	K2_API CResourceReference(const tstring &sPath);
	K2_API CResourceReference(const tstring &sName, ResHandle hReference);

	K2_API	virtual uint			GetResType() const			{ return RES_REFERENCE; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{reference}")); return sTypeName; }

	void		SetReference(ResHandle hReference);
	ResHandle	GetReference()						{ return m_hReference; }

	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free();
};
//=============================================================================
#endif //__C_RESOURCE_REFERENCE_H__
