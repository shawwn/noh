// (C)2005 S2 Games
// c_xmlnode.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlnode.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
K2_API CXMLNode		g_NullXMLNode;
//=============================================================================

/*====================
  CXMLNode::CXMLNode
  ====================*/
CXMLNode::CXMLNode()
{
}


CXMLNode::CXMLNode(const tstring &sPropertys)
{
	tstring sStrTmp(TSNULL);
	tstring sPropHolder(TSNULL);
	tstring sValueHolder(TSNULL);
	bool	bHasProp(false);
	uint	uiPropStart(-1);
	uint	uiValueStart(-1);

	for (uint uiStrPos(0); uiStrPos < sPropertys.length(); ++uiStrPos)
	{
		sStrTmp = sPropertys.substr(uiStrPos, 1);
		if (IsTokenSeparator(int(sStrTmp[0])))
			continue;
		
		if (!bHasProp)
		{
			if (sStrTmp != _T("=") && !IsTokenSeparator(int(sStrTmp[0])) && uiPropStart == -1)
			{
				uiPropStart = uiStrPos;
			}
			else if ((sStrTmp == _T("=") || IsTokenSeparator(int(sStrTmp[0]))) && uiPropStart != -1)
			{
				sPropHolder = sPropertys.substr(uiPropStart, uiStrPos - uiPropStart);
				bHasProp = true;
			}
		}
		else
		{
			if ((sStrTmp == _T("\"") || sStrTmp == _T("'")) && uiValueStart == -1)
			{
				uiValueStart = uiStrPos + 1;
			}
			else if ((sStrTmp == _T("\"") || sStrTmp == _T("'")) && uiValueStart != -1)
			{
				sValueHolder = sPropertys.substr(uiValueStart, uiStrPos - uiValueStart);
				SetProperty(sPropHolder, sValueHolder);
				sPropHolder.clear();
				sValueHolder.clear();
				uiPropStart = -1;
				uiValueStart = -1;
				bHasProp = false;
			}
		}
	}
}


/*====================
  CXMLNode::~CXMLNode
  ====================*/
CXMLNode::~CXMLNode()
{
}


/*====================
  CXMLNode::~CXMLNode
  ====================*/
void CXMLNode::Clear()
{
	m_sName.clear();
	m_mapProperties.clear();
	m_lChildren.clear();
}


/*====================
  CXMLNode::GetProperty
  ====================*/
const tstring&	CXMLNode::GetProperty(const tstring &sName, const tstring &sDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return sDefaultValue;
	else
		return findit->second;
}


/*====================
  CXMLNode::GetPropertyInt
  ====================*/
int		CXMLNode::GetPropertyInt(const tstring &sName, int iDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return iDefaultValue;
	else
		return AtoI(findit->second);
}


/*====================
  CXMLNode::GetPropertyFloat
  ====================*/
float	CXMLNode::GetPropertyFloat(const tstring &sName, float fDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return fDefaultValue;
	else
		return AtoF(findit->second);
}


/*====================
  CXMLNode::GetPropertyBool
  ====================*/
bool	CXMLNode::GetPropertyBool(const tstring &sName, bool bDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return bDefaultValue;
	else
		return AtoB(findit->second);
}


/*====================
  CXMLNode::GetPropertyCvar
  ====================*/
ICvar*	CXMLNode::GetPropertyCvar(const tstring &sName, ICvar *pDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return pDefaultValue;
	else
	{
		ICvar	*pCvar = NULL;
		if (!ConsoleRegistry.Exists(findit->second))
			pCvar = NULL;
		else
		{
			CConsoleElement *pElement = Console.GetElement(findit->second);

			if (pElement->GetType() == ELEMENT_CVAR)
				pCvar = static_cast<ICvar *>(pElement);
			else
				pCvar = NULL;
		}

		return pCvar;
	}
}


/*====================
  CXMLNode::GetPropertyV2
  ====================*/
CVec2f	CXMLNode::GetPropertyV2(const tstring &sName, const CVec2f &v2Default) const
{
	PropertyMap::const_iterator itFind(m_mapProperties.find(sName));
	if (itFind == m_mapProperties.end())
		return v2Default;
	else
		return AtoV2(itFind->second);
}


/*====================
  CXMLNode::GetPropertyV3
  ====================*/
CVec3f	CXMLNode::GetPropertyV3(const tstring &sName, const CVec3f &v3Default) const
{
	PropertyMap::const_iterator itFind(m_mapProperties.find(sName));
	if (itFind == m_mapProperties.end())
		return v3Default;
	else
		return AtoV3(itFind->second);
}


/*====================
  CXMLNode::GetPropertyV4
  ====================*/
CVec4f	CXMLNode::GetPropertyV4(const tstring &sName, const CVec4f &v4Default) const
{
	PropertyMap::const_iterator itFind(m_mapProperties.find(sName));
	if (itFind == m_mapProperties.end())
		return v4Default;
	else
		return AtoV4(itFind->second);
}


/*====================
  CXMLNode::SetProperty
  ====================*/
void	CXMLNode::SetProperty(const tstring &sName, const tstring &sValue)
{
	m_mapProperties[sName] = sValue;
}


/*====================
  CXMLNode::IsText
  ====================*/
bool	CXMLNode::IsText()
{
	PropertyMap::iterator find = m_mapProperties.find(_T("type"));

	if (find != m_mapProperties.end())
		if (find->second == _T("text"))
			return true;

	return false;
}


/*====================
  CXMLNode::ApplySubstitutions
  ====================*/
void	CXMLNode::ApplySubstitutions(CXMLNode &cParams)
{
	PROFILE("CXMLNode::ApplySubstitutions");

	for (PropertyMap::iterator it(m_mapProperties.begin()); it != m_mapProperties.end(); ++it)
	{
		size_t zOffset(0);
		while (zOffset != tstring::npos)
		{
			size_t zStart(it->second.find(_T('{'), zOffset));
			if (zStart == tstring::npos)
				break;
			size_t zEnd(it->second.find(_T('}'), zStart));
			if (zEnd == tstring::npos)
				break;

			// Default parameter
			size_t zMid(it->second.find(_T('='), zStart));
			if (zMid < zEnd)
			{
				const tstring &sVar(it->second.substr(zStart + 1, zMid - zStart - 1));
				if (cParams.HasProperty(sVar))
				{
					const tstring &sValue(cParams.GetProperty(sVar));
					zOffset = zStart + sValue.length();
					it->second.replace(zStart, zEnd - zStart + 1, sValue);
				}
				else
				{
					const tstring &sValue(it->second.substr(zMid + 1, zEnd - zMid - 1));
					zOffset = zStart + sValue.length();
					it->second.replace(zStart, zEnd - zStart + 1, sValue);
				}
				continue;
			}

			const tstring &sVar(it->second.substr(zStart + 1, zEnd - zStart - 1));
			const tstring &sValue(cParams.GetProperty(sVar));
			zOffset = zStart + sValue.length();
			it->second.replace(zStart, zEnd - zStart + 1, sValue);
		}
	}

	for (List_it it(m_lChildren.begin()), itEnd(m_lChildren.end()); it != itEnd; ++it)
		it->ApplySubstitutions(cParams);
}


/*====================
  CXMLNodeWrite::CXMLNodeWrite
  ====================*/
CXMLNodeWrite::CXMLNodeWrite()
{
}


/*====================
  CXMLNodeWrite::~CXMLNodeWrite
  ====================*/
CXMLNodeWrite::~CXMLNodeWrite()
{
}


/*====================
  CXMLNodeWrite::SetProperty
  ====================*/
void	CXMLNodeWrite::SetProperty(const tstring &sName, const tstring &sValue)
{
	PropertyMap::iterator findit(m_mapProperties.find(sName));

	if (findit == m_mapProperties.end())
	{
		m_lProperties.push_back(TStringPair(sName, sValue));
	}
	else
	{
		for (PropertyList::iterator it(m_lProperties.begin()); it != m_lProperties.end(); ++it)
		{
			if (it->first == sName)
			{
				it->second = sValue;
				break;
			}
		}
	}

	m_mapProperties[sName] = sValue;
}


/*====================
  CXMLNodeWrite::RemoveProperty
  ====================*/
void	CXMLNodeWrite::RemoveProperty(const tstring &sName)
{
	PropertyMap::iterator itFind(m_mapProperties.find(sName));

	if (itFind == m_mapProperties.end())
		return;

	for (PropertyList::iterator it(m_lProperties.begin()); it != m_lProperties.end(); ++it)
	{
		if (it->first == sName)
		{
			m_lProperties.erase(it);
			break;
		}
	}

	m_mapProperties.erase(itFind);
}


/*====================
  CXMLNodeWrite::IsText
  ====================*/
bool	CXMLNodeWrite::IsText()
{
	PropertyMap::iterator find = m_mapProperties.find(_T("type"));

	if (find != m_mapProperties.end())
		if (find->second == _T("text"))
			return true;

	return false;
}
