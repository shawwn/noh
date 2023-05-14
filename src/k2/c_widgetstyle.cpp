// (C)2006 S2 Games
// c_widgetstyle.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_widgetstyle.h"
#include "c_interface.h"
//=============================================================================

/*====================
  CWidgetStyle::CWidgetStyle
  ====================*/
CWidgetStyle::CWidgetStyle(CInterface* pInterface) :
m_pInterface(pInterface)
{
}

CWidgetStyle::CWidgetStyle(CInterface* pInterface, const CXMLNode::PropertyMap& mapProperties) :
m_pInterface(pInterface),
m_mapProperties(mapProperties)
{
}

CWidgetStyle::CWidgetStyle(CInterface* pInterface, const CXMLNode& node) :
m_pInterface(pInterface),
m_mapProperties(node.GetPropertyMap())
{
    if (!node.HasProperty(_T("content")))
        m_mapProperties[_T("content")] = node.GetContents();

    if (m_pInterface != nullptr && HasProperty(_T("style")))
    {
        CWidgetStyle* pStyle(m_pInterface->GetStyle(GetProperty(_T("style"))));
        RemoveProperty(_T("style"));
        if (pStyle != nullptr)
        {
            for (CXMLNode::PropertyMap_it it(pStyle->m_mapProperties.begin()); it != pStyle->m_mapProperties.end(); ++it)
            {
                if (!HasProperty(it->first))
                    m_mapProperties[it->first] = it->second;
            }
        }
    }
}


/*====================
  CWidgetStyle::GetProperty
  ====================*/
const tstring&  CWidgetStyle::GetProperty(const tstring &sName, const tstring &sDefault) const
{
    CXMLNode::PropertyMap_cit itFind(m_mapProperties.find(sName));
    if (itFind == m_mapProperties.end())
        return sDefault;

    return itFind->second;
}


/*====================
  CWidgetStyle::GetPropertyInt
  ====================*/
int     CWidgetStyle::GetPropertyInt(const tstring &sName, int iDefault) const
{
    CXMLNode::PropertyMap_cit itFind(m_mapProperties.find(sName));
    if (itFind == m_mapProperties.end())
        return iDefault;

    return AtoI(itFind->second);
}


/*====================
  CWidgetStyle::GetPropertyFloat
  ====================*/
float   CWidgetStyle::GetPropertyFloat(const tstring &sName, float fDefault) const
{
    CXMLNode::PropertyMap_cit itFind(m_mapProperties.find(sName));
    if (itFind == m_mapProperties.end())
        return fDefault;

    return AtoF(itFind->second);
}


/*====================
  CWidgetStyle::GetPropertyBool
  ====================*/
bool    CWidgetStyle::GetPropertyBool(const tstring &sName, bool bDefault) const
{
    CXMLNode::PropertyMap_cit itFind(m_mapProperties.find(sName));
    if (itFind == m_mapProperties.end())
        return bDefault;

    return AtoB(itFind->second);
}


/*====================
  CWidgetStyle::GetPropertyVec3
  ====================*/
CVec3f  CWidgetStyle::GetPropertyVec3(const tstring &sName, CVec3f v3Default) const
{
    CXMLNode::PropertyMap_cit itFind(m_mapProperties.find(sName));
    if (itFind == m_mapProperties.end())
        return v3Default;

    return AtoV3(itFind->second);
}


/*====================
  CWidgetStyle::HasProperty
  ====================*/
bool    CWidgetStyle::HasProperty(const tstring &sName) const
{
    CXMLNode::PropertyMap_cit itFind(m_mapProperties.find(sName));
    if (itFind == m_mapProperties.end())
        return false;

    return true;
}


/*====================
  CWidgetStyle::RemoveProperty
  ====================*/
void    CWidgetStyle::RemoveProperty(const tstring &sName)
{
    m_mapProperties.erase(sName);
}


/*====================
  CWidgetStyle::SetProperty
  ====================*/
void    CWidgetStyle::SetProperty(const tstring &sName, const tstring &sValue)
{
    if (sName.empty())
        return;

    m_mapProperties[sName] = sValue;
}


/*====================
  CWidgetStyle::ApplySubstitutions
  ====================*/
void    CWidgetStyle::ApplySubstitutions(const CWidgetStyle &style)
{
    PROFILE("CWidgetStyle::ApplySubstitutions");

    for (CXMLNode::PropertyMap_it it(m_mapProperties.begin()); it != m_mapProperties.end(); ++it)
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
                if (style.HasProperty(sVar))
                {
                    const tstring &sValue(style.GetProperty(sVar));
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
            const tstring &sValue(style.GetProperty(sVar));
            zOffset = zStart + sValue.length();
            it->second.replace(zStart, zEnd - zStart + 1, sValue);
        }
    }
}
