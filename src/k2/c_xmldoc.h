// (C)2005 S2 Games
// c_xmldoc.h
//
//=============================================================================
#ifndef __C_XMLDOC_H__
#define __C_XMLDOC_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "c_buffer.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CXMLNode;
class CXMLNodeWrite;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EXMLEncoding
{
	XML_ENCODE_UTF8,
	XML_ENCODE_UTF16
};
//=============================================================================

//=============================================================================
// CXMLDoc
// Provides an interface for constructing an XML document and then retrieving
// it as a buffer
//=============================================================================
class CXMLDoc
{
private:
	void*			m_pDoc;
	void*			m_pRoot;

	EXMLEncoding	m_eEncoding;

	stack<void*>	m_nodeStack;

	CBufferDynamic	m_buffer;

	void		ConvertNodes(void *node, CXMLNodeWrite &c_node);

	static int	WriteBuffer(void *context, const char *buffer, int len);
	void		WriteNode(CFileHandle &hFile, const CXMLNodeWrite &node, int iIndent);
	void		WriteNodeLong(CFileHandle &hFile, const CXMLNodeWrite &node, int iIndent);

public:
	K2_API ~CXMLDoc();
	K2_API CXMLDoc(EXMLEncoding eEncoding = XML_ENCODE_UTF8);

	K2_API void	Clear();

	K2_API bool	NewNode(const string &sName, const string &sContent = "");
	bool		NewNode(const wstring &sName, const wstring &sContent = L"")	{ return NewNode(WStringToUTF8(sName), WStringToUTF8(sContent)); }
	bool		NewNode(const string &sName, const wstring &sContent)			{ return NewNode(sName, WStringToUTF8(sContent)); }
	bool		NewNode(const wstring &sName, const string &sContent)			{ return NewNode(WStringToUTF8(sName), sContent); }
	K2_API bool	NewTextNode(const string &sName, const string &sContent = "");
	bool		NewTextNode(const wstring &sName, const wstring &sContent = L"")	{ return NewTextNode(WStringToUTF8(sName), WStringToUTF8(sContent)); }
	bool		NewTextNode(const string &sName, const wstring &sContent)			{ return NewTextNode(sName, WStringToUTF8(sContent)); }
	bool		NewTextNode(const wstring &sName, const string &sContent)			{ return NewTextNode(WStringToUTF8(sName), sContent); }
	K2_API bool	EndNode();

	K2_API bool	DeleteNode();

	K2_API IBuffer*	GetBuffer();

	K2_API void	WriteFile(const tstring &sPath, bool bLong = false);

	K2_API bool				TraverseChildren();
	K2_API bool				TraverseChildrenReverse();

	K2_API bool				TraverseNextChild();
	K2_API bool				TraversePrevChild();

	K2_API string			GetProperty(const string &sName);
	K2_API wstring			GetProperty(const wstring &sName)							{ return UTF8ToWString(GetProperty(WStringToUTF8(sName))); }

	K2_API string			GetNodeName();

	K2_API bool				AddProperty(const string &sName, const string &sValue);
	bool					AddProperty(const string &sName, const wstring &sValue)		{ return AddProperty(sName, WStringToUTF8(sValue)); }
	bool					AddProperty(const wstring &sName, const wstring &sValue)	{ return AddProperty(WStringToUTF8(sName), WStringToUTF8(sValue)); }
	template<class T> bool	AddProperty(const string &sName, T value)					{ return AddProperty(sName, XtoS(value)); }
	bool					AddProperty(const string &sName, const char *szValue)		{ return AddProperty(sName, string(szValue)); }

	K2_API bool	ReadBuffer(const char *pBuffer, int iSize);
};
//=============================================================================
#endif //__C_XMLDOC_H__
