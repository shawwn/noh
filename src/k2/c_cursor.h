// (C)2009 S2 Games
// c_cursor.h
//
//=============================================================================
#ifndef __C_CURSOR_H__
#define __C_CURSOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBitmap;
//=============================================================================

//=============================================================================
// CCursor
//=============================================================================
class CCursor : public IResource
{
protected:
	ResHandle	m_hBitmap;
	CVec2i		m_v2Hotspot;

	CCursor();

public:
	~CCursor()	{}
	CCursor(const tstring &sPath) :
	IResource(sPath, TSNULL)
	{}

	K2_API	virtual uint			GetResType() const			{ return RES_K2CURSOR; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{cursor}")); return sTypeName; }

	ResHandle		GetBitmap() const					{ return m_hBitmap; }
	void			SetBitmap(ResHandle hBitmap)		{ m_hBitmap = hBitmap; }

	const CVec2i&	GetHotspot() const					{ return m_v2Hotspot; }
	void			SetHotspot(const CVec2i &v2Hotspot)	{ m_v2Hotspot = v2Hotspot; }
	
	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free()								{}

	K2_API CBitmap*	GetBitmapPointer();
};
//=============================================================================

#endif //__C_CURSOR_H__
