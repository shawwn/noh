// (C)2005 S2 Games
// i_xmlproc_tag.h
//
//=============================================================================
#ifndef __I_XMLPROC_TAG_H__
#define __I_XMLPROC_TAG_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_tag.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class ITag;
//=============================================================================

//=============================================================================
// IXMLProc_Tag
//=============================================================================
class IXMLProc_Tag : public IXMLProcessor
{
protected:
	ITag *m_pParent;
	tstring m_sInterfaceName;

public:
	IXMLProc_Tag(const tstring &sElement) : IXMLProcessor(sElement) {}
	virtual ~IXMLProc_Tag() {}

	void	SetParameters(ITag *pTag, const tstring &sInterfaceName)
	{
		m_pParent = pTag;
		m_sInterfaceName = sInterfaceName;
	}
};
//=============================================================================
#endif //__I_XMLPROC_TAG_H__
