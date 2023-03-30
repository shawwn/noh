// (C)2009 S2 Games
// c_bitmapresource.h
//
//=============================================================================
#ifndef __C_BITMAPRESOURCE_H__
#define __C_BITMAPRESOURCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/i_resource.h"
#include "../k2/c_bitmap.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBitmap;
class CWidgetStyle;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CBitmapResource
//=============================================================================
class CBitmapResource : public IResource
{
private:
	CBitmap	m_cBitmap;

public:
	K2_API ~CBitmapResource()	{}
	K2_API CBitmapResource(const tstring &sPath);

	K2_API	virtual uint			GetResType() const			{ return RES_BITMAP; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{bitmap}")); return sTypeName; }

	CBitmap&	GetBitmap()	{ return m_cBitmap; }

	int			Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void		Free();
};
//=============================================================================

#endif //__C_BITMAPRESOURCE_H__
