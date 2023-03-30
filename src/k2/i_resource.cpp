// (C)2005 S2 Games
// i_resource.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_resource.h"
#include "i_resourcewatcher2.h"

//=============================================================================

/*====================
  IResource::~IResource
  ====================*/
IResource::~IResource()
{
	SAFE_DELETE_ARRAY(m_pData);
}


/*====================
  IResource::IResource
  ====================*/
IResource::IResource(const tstring &sPath, const tstring &sName) :
m_sPath(FileManager.SanitizePath(sPath)),
m_sName(sName),
m_iFlags(0),
m_pData(NULL),
m_uiSize(0),
m_hHandle(INVALID_RESOURCE),
m_uiNetIndex(INVALID_INDEX),
m_uiIgnoreFlags(0)
{
}


/*====================
  IResource::IResource
  ====================*/
IResource::IResource(const IResource &c) :
m_sPath(c.m_sPath),
m_sName(c.m_sName),
m_iFlags(c.m_iFlags),
m_uiSize(c.m_uiSize),
m_hHandle(c.m_hHandle),
m_uiNetIndex(c.m_uiNetIndex),
m_uiIgnoreFlags(c.m_uiIgnoreFlags)
{
	if (c.m_pData != NULL)
	{
		m_pData = K2_NEW_ARRAY(ctx_Resources, char, m_uiSize);
		MemManager.Copy((char *)m_pData, c.m_pData, m_uiSize);
	}
	else
		m_pData = NULL;
}


/*====================
  IResource::IsVirtualResource
  ====================*/
bool	IResource::IsVirtualResource() const
{
	const tstring &sPath(GetPath());

	if (sPath.empty())
		return false;

	if (sPath[0] == _T('$') || sPath[0] == _T('!') || sPath[0] == _T('*') || sPath.find(_CWS("%"), 0) != tstring::npos) 
		return true;

	return false;
}


/*====================
  IResource::MatchesWildcard
  ====================*/
bool	IResource::MatchesWildcard(const tstring &sWild,
								   bool bMatchAgainstPath,
								   bool bMatchAgainstName,
								   bool bMatchAgainstType) const
{
	if (bMatchAgainstPath)
	{
		if (EqualsWildcards(sWild, m_sPath))
			return true;
	}

	if (bMatchAgainstName)
	{
		if (EqualsWildcards(sWild, m_sName))
			return true;
	}

	if (bMatchAgainstType)
	{
		if (EqualsWildcards(sWild, GetResTypeName()))
			return true;
	}

	return false;
}

