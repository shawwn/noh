// (C)2005 S2 Games
// c_xmlproc_resourcelist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_resourcelist.h"
#include "c_xmlprocroot.h"
#include "c_texture.h"
#include "c_sample.h"
#include "c_cursor.h"
#include "c_uimanager.h"
#include "c_resourcemanager.h"
//=============================================================================

namespace XMLResourceList
{
	// <resourcelist>
	BEGIN_XML_REGISTRATION(resourcelist)
		REGISTER_XML_PROCESSOR(root)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(resourcelist, void)
	END_XML_PROCESSOR(NULL)

	// <texture>
	DECLARE_XML_PROCESSOR(texture)
	BEGIN_XML_REGISTRATION(texture)
		REGISTER_XML_PROCESSOR(resourcelist)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(texture, void)
		const tstring &sPath(node.GetProperty(_T("file")));
		const tstring &sName(node.GetProperty(_T("name")));

		CTexture *pTexture(K2_NEW(ctx_Resources,  CTexture)(sPath, TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_COMPRESS, TEXFMT_A8R8G8B8));
		if (pTexture != NULL)
			pTexture->SetName(sName);
		g_ResourceManager.Register(pTexture, RES_TEXTURE);
	END_XML_PROCESSOR_NO_CHILDREN

	// <sample>
	DECLARE_XML_PROCESSOR(sample)
	BEGIN_XML_REGISTRATION(sample)
		REGISTER_XML_PROCESSOR(resourcelist)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(sample, void)
		const tstring &sPath(node.GetProperty(_T("file")));
		const tstring &sName(node.GetProperty(_T("name")));

		uint uiFlags(SND_2D);
		CSample *pSample(K2_NEW(ctx_Resources,  CSample)(sPath, uiFlags));
		if (pSample != NULL)
			pSample->SetName(sName);
		g_ResourceManager.Register(pSample, RES_SAMPLE);
	END_XML_PROCESSOR_NO_CHILDREN

	// <stringtable>
	DECLARE_XML_PROCESSOR(stringtable)
	BEGIN_XML_REGISTRATION(stringtable)
		REGISTER_XML_PROCESSOR(resourcelist)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(stringtable, void)
		const tstring &sPath(node.GetProperty(_T("file")));

		g_ResourceManager.Register(sPath, RES_STRINGTABLE);
	END_XML_PROCESSOR_NO_CHILDREN

	// <interface>
	DECLARE_XML_PROCESSOR(interface)
	BEGIN_XML_REGISTRATION(interface)
		REGISTER_XML_PROCESSOR(resourcelist)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(interface, void)
		const tstring &sPath(node.GetProperty(_T("file")));
		UIManager.LoadInterface(sPath);
	END_XML_PROCESSOR_NO_CHILDREN

	// <cursor>
	DECLARE_XML_PROCESSOR(cursor)
	BEGIN_XML_REGISTRATION(cursor)
		REGISTER_XML_PROCESSOR(resourcelist)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(cursor, void)
		const tstring &sPath(node.GetProperty(_T("file")));
		const tstring &sName(node.GetProperty(_T("name")));

		CCursor *pCursor(K2_NEW(ctx_Resources,  CCursor)(sPath));
		if (pCursor != NULL)
			pCursor->SetName(sName);
		g_ResourceManager.Register(pCursor, RES_K2CURSOR);
	END_XML_PROCESSOR_NO_CHILDREN
}