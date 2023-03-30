// (C)2006 S2 Games
// c_widgetstyle.h
//
//=============================================================================
#ifndef __C_WIDGETSTYLE_H__
#define __C_WIDGETSTYLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_xmlnode.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CInterface;
//=============================================================================

//=============================================================================
// CWidgetStyle
//=============================================================================
class CWidgetStyle
{
private:
	CInterface*				m_pInterface;
	CXMLNode::PropertyMap	m_mapProperties;

	CWidgetStyle();

public:
	K2_API ~CWidgetStyle()	{}
	K2_API CWidgetStyle(CInterface* pInterface);
	K2_API CWidgetStyle(CInterface* pInterface, const CXMLNode::PropertyMap& mapProperties);
	K2_API CWidgetStyle(CInterface* pInterface, const CXMLNode& node);

	K2_API const tstring&	GetProperty(const tstring &sName, const tstring &sDefault = TSNULL) const;
	K2_API int				GetPropertyInt(const tstring &sName, int iDefault = 0) const;
	K2_API float			GetPropertyFloat(const tstring &sName, float fDefault = 0.0f) const;
	K2_API bool				GetPropertyBool(const tstring &sName, bool bDefault = false) const;
	K2_API CVec3f			GetPropertyVec3(const tstring &sName, CVec3f v3Default = CVec3f(0.0f, 0.0f, 0.0f)) const;

	K2_API bool		HasProperty(const tstring &sName) const;

	K2_API void		RemoveProperty(const tstring &sName);

	K2_API void		SetProperty(const tstring &sName, const tstring &sValue);
	void			SetProperty(const tstring &sName, const TCHAR *szValue)		{ SetProperty(sName, tstring(szValue)); }
	void			SetProperty(const tstring &sName, float fValue)				{ SetProperty(sName, XtoA(fValue)); }
	void			SetProperty(const tstring &sName, int iValue)				{ SetProperty(sName, XtoA(iValue)); }
	void			SetProperty(const tstring &sName, bool bValue)				{ SetProperty(sName, XtoA(bValue)); }
	void			SetProperty(const tstring &sName, const tstring &sValue, const tstring &sSuffix)	{ if (!sValue.empty()) SetProperty(sName, sValue + sSuffix); }
	
	void			ApplySubstitutions(const CWidgetStyle &style);
};
//=============================================================================

#endif //__C_WIDGETSTYLE_H__
