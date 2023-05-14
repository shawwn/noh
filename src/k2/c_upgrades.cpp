// (C)2010 S2 Games
// c_upgrades.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_upgrades.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_xmlprocroot.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

/*====================
  CChatSymbol::CChatSymbol
  ====================*/
CChatSymbol::CChatSymbol(const tstring &sName, const tstring &sTexturePath) :
m_sName(sName),
//m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CTS("ChatSymbol_") + sName)),
m_uiDisplayNameIndex(INVALID_INDEX),
m_sTexturePath(sTexturePath)
{
}


/*====================
  CChatSymbol::CChatSymbol
  ====================*/
const tstring&  CChatSymbol::GetDisplayName() const
{
    //return Game.GetEntityString(m_uiDisplayNameIndex);
    return TSNULL;
}


/*====================
  CChatNameColor::CChatNameColor
  ====================*/
CChatNameColor::CChatNameColor(const tstring &sName, const tstring &sTexturePath, const tstring &sColor, const tstring &sIngameColor, uint uiSortIndex) :
m_sName(sName),
//m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CTS("ChatNameColor_") + sName)),
m_uiDisplayNameIndex(INVALID_INDEX),
m_sTexturePath(sTexturePath),
m_sColor(sColor),
m_sIngameColor(sIngameColor),
m_uiSortIndex(uiSortIndex)
{
}


/*====================
  CChatNameColor::CChatNameColor
  ====================*/
const tstring&  CChatNameColor::GetDisplayName() const
{
    //return Game.GetEntityString(m_uiDisplayNameIndex);
    return TSNULL;
}


/*====================
  CAccountIcon::CAccountIcon
  ====================*/
CAccountIcon::CAccountIcon(const tstring &sName, const tstring &sTexturePath) :
m_sName(sName),
//m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CTS("AccountIcon_") + sName)),
m_uiDisplayNameIndex(INVALID_INDEX),
m_sTexturePath(sTexturePath)
{
}


/*====================
  CAccountIcon::CAccountIcon
  ====================*/
const tstring&  CAccountIcon::GetDisplayName() const
{
    //return Game.GetEntityString(m_uiDisplayNameIndex);
    return TSNULL;
}


/*====================
  CAnnouncerVoice::CAnnouncerVoice
  ====================*/
CAnnouncerVoice::CAnnouncerVoice(const tstring &sName, const tstring &sVoiceSet, const tstring &sArcadeText) :
m_sName(sName),
//m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CTS("AnnouncerVoice_") + sName)),
m_uiDisplayNameIndex(INVALID_INDEX),
m_sVoiceSet(sVoiceSet),
m_sArcadeText(sArcadeText)
{
}


/*====================
  CAnnouncerVoice::CAnnouncerVoice
  ====================*/
const tstring&  CAnnouncerVoice::GetDisplayName() const
{
    //return Game.GetEntityString(m_uiDisplayNameIndex);
    return TSNULL;
}


/*====================
  CTaunt::CTaunt
  ====================*/
CTaunt::CTaunt(const tstring &sName, const tstring &sModifier) :
m_sName(sName),
//m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CTS("Taunt_") + sName)),
m_uiDisplayNameIndex(INVALID_INDEX),
m_sModifier(sModifier)
{
}


/*====================
  CTaunt::CTaunt
  ====================*/
const tstring&  CTaunt::GetDisplayName() const
{
    //return Game.GetEntityString(m_uiDisplayNameIndex);
    return TSNULL;
}



namespace Upgrades
{
// <upgrades>
DECLARE_XML_PROCESSOR(upgrades)
BEGIN_XML_REGISTRATION(upgrades)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(upgrades, CUpgrades)
    // ...
END_XML_PROCESSOR(pObject)


// <chatsymbol>
DECLARE_XML_PROCESSOR(chatsymbol)
BEGIN_XML_REGISTRATION(chatsymbol)
    REGISTER_XML_PROCESSOR(upgrades)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(chatsymbol, CUpgrades)
    pObject->RegisterChatSymbol(node);
END_XML_PROCESSOR_NO_CHILDREN

// <chatnamecolor>
DECLARE_XML_PROCESSOR(chatnamecolor)
BEGIN_XML_REGISTRATION(chatnamecolor)
    REGISTER_XML_PROCESSOR(upgrades)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(chatnamecolor, CUpgrades)
    pObject->RegisterChatNameColor(node);
END_XML_PROCESSOR_NO_CHILDREN

// <accounticon>
DECLARE_XML_PROCESSOR(accounticon)
BEGIN_XML_REGISTRATION(accounticon)
    REGISTER_XML_PROCESSOR(upgrades)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(accounticon, CUpgrades)
    pObject->RegisterAccountIcon(node);
END_XML_PROCESSOR_NO_CHILDREN

// <announcervoice>
DECLARE_XML_PROCESSOR(announcervoice)
BEGIN_XML_REGISTRATION(announcervoice)
    REGISTER_XML_PROCESSOR(upgrades)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(announcervoice, CUpgrades)
    pObject->RegisterAnnouncerVoice(node);
END_XML_PROCESSOR_NO_CHILDREN

// <taunt>
DECLARE_XML_PROCESSOR(taunt)
BEGIN_XML_REGISTRATION(taunt)
    REGISTER_XML_PROCESSOR(upgrades)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(taunt, CUpgrades)
    pObject->RegisterTaunt(node);
END_XML_PROCESSOR_NO_CHILDREN

}

/*====================
  CUpgrades::CUpgrades
  ====================*/
CUpgrades::CUpgrades()
{
}


/*====================
  CUpgrades::Clear
  ====================*/
void    CUpgrades::Clear()
{
    m_mapChatSymbols.clear();
    m_vChatSymbols.clear();

    m_mapChatNameColors.clear();
    m_vChatNameColors.clear();

    m_mapAccountIcons.clear();
    m_vAccountIcons.clear();

    m_mapAnnouncerVoices.clear();
    m_vAnnouncerVoices.clear();

    m_mapTaunts.clear();
    m_vTaunts.clear();
}


/*====================
  CUpgrades::PostLoad
  ====================*/
void    CUpgrades::PostLoad()
{
#if 0
    for (vector<CChatSymbol>::iterator it(m_vChatSymbols.begin()); it != m_vChatSymbols.end(); ++it)
        it->SetDisplayNameIndex(Game.GetEntityStringIndex(_CTS("ChatSymbol_") + it->GetName()));
#endif
}


/*====================
  CUpgrades::RegisterChatSymbol
  ====================*/
uint    CUpgrades::RegisterChatSymbol(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_CTS("name")));
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::iterator itFind(m_mapChatSymbols.find(sName));
    if (itFind != m_mapChatSymbols.end())
        return itFind->second;

    uint uiNewID(INT_SIZE(m_vChatSymbols.size()));
    m_vChatSymbols.push_back(CChatSymbol(sName, node.GetProperty(_CTS("texture"))));
    m_mapChatSymbols[sName] = uiNewID;
    return uiNewID;
}


/*====================
  CUpgrades::LookupChatSymbol
  ====================*/
uint    CUpgrades::LookupChatSymbol(const tstring &sName) const
{
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::const_iterator itFind(m_mapChatSymbols.find(sName));
    if (itFind == m_mapChatSymbols.end())
        return INVALID_INDEX;

    return itFind->second;
}


/*====================
  CUpgrades::RegisterChatNameColor
  ====================*/
uint    CUpgrades::RegisterChatNameColor(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_CTS("name")));
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::iterator itFind(m_mapChatNameColors.find(sName));
    if (itFind != m_mapChatNameColors.end())
        return itFind->second;

    uint uiNewID(INT_SIZE(m_vChatNameColors.size()));
    m_vChatNameColors.push_back(CChatNameColor(sName,
        node.GetProperty(_CTS("texture")),
        node.GetProperty(_CTS("color")),
        node.GetProperty(_CTS("ingamecolor")),
        node.GetPropertyInt(_CTS("sortindex"), 0)
    ));
    m_mapChatNameColors[sName] = uiNewID;
    return uiNewID;
}


/*====================
  CUpgrades::LookupChatNameColor
  ====================*/
uint    CUpgrades::LookupChatNameColor(const tstring &sName) const
{
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::const_iterator itFind(m_mapChatNameColors.find(sName));
    if (itFind == m_mapChatNameColors.end())
        return INVALID_INDEX;

    return itFind->second;
}


/*====================
  CUpgrades::RegisterAccountIcon
  ====================*/
uint    CUpgrades::RegisterAccountIcon(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_CTS("name")));
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::iterator itFind(m_mapAccountIcons.find(sName));
    if (itFind != m_mapAccountIcons.end())
        return itFind->second;

    uint uiNewID(INT_SIZE(m_vAccountIcons.size()));
    m_vAccountIcons.push_back(CAccountIcon(sName, node.GetProperty(_CTS("texture"))));
    m_mapAccountIcons[sName] = uiNewID;
    return uiNewID;
}


/*====================
  CUpgrades::LookupAccountIcon
  ====================*/
uint    CUpgrades::LookupAccountIcon(const tstring &sName) const
{
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::const_iterator itFind(m_mapAccountIcons.find(sName));
    if (itFind == m_mapAccountIcons.end())
        return INVALID_INDEX;

    return itFind->second;
}


/*====================
  CUpgrades::RegisterAnnouncerVoice
  ====================*/
uint    CUpgrades::RegisterAnnouncerVoice(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_CTS("name")));
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::iterator itFind(m_mapAnnouncerVoices.find(sName));
    if (itFind != m_mapAnnouncerVoices.end())
        return itFind->second;

    uint uiNewID(INT_SIZE(m_vAnnouncerVoices.size()));
    m_vAnnouncerVoices.push_back(CAnnouncerVoice(sName,
        node.GetProperty(_CTS("voiceset")),
        node.GetProperty(_CTS("arcadetext"))
    ));
    m_mapAnnouncerVoices[sName] = uiNewID;
    return uiNewID;
}


/*====================
  CUpgrades::LookupAnnouncerVoice
  ====================*/
uint    CUpgrades::LookupAnnouncerVoice(const tstring &sName) const
{
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::const_iterator itFind(m_mapAnnouncerVoices.find(sName));
    if (itFind == m_mapAnnouncerVoices.end())
        return INVALID_INDEX;

    return itFind->second;
}


/*====================
  CUpgrades::RegisterTaunt
  ====================*/
uint    CUpgrades::RegisterTaunt(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_CTS("name")));
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::iterator itFind(m_mapTaunts.find(sName));
    if (itFind != m_mapTaunts.end())
        return itFind->second;

    uint uiNewID(INT_SIZE(m_vTaunts.size()));
    m_vTaunts.push_back(CTaunt(sName,
        node.GetProperty(_CTS("modifier"))
    ));
    m_mapTaunts[sName] = uiNewID;
    return uiNewID;
}


/*====================
  CUpgrades::LookupTaunt
  ====================*/
uint    CUpgrades::LookupTaunt(const tstring &sName) const
{
    if (sName.empty())
        return INVALID_INDEX;
    
    map<tstring, uint>::const_iterator itFind(m_mapTaunts.find(sName));
    if (itFind == m_mapTaunts.end())
        return INVALID_INDEX;

    return itFind->second;
}


/*====================
  CUpgrades::WriteStringTable
  ====================*/
void    CUpgrades::WriteStringTable(CFileHandle &hFile, size_t zTabStop, size_t zColumnOffset)
{
    hFile << _CTS("// Upgrades") << newl;

    for (vector<CChatSymbol>::iterator it(m_vChatSymbols.begin()); it != m_vChatSymbols.end(); ++it)
        hFile << TabPad(_CTS("ChatSymbol_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    for (vector<CChatNameColor>::iterator it(m_vChatNameColors.begin()); it != m_vChatNameColors.end(); ++it)
        hFile << TabPad(_CTS("ChatNameColor_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    for (vector<CAccountIcon>::iterator it(m_vAccountIcons.begin()); it != m_vAccountIcons.end(); ++it)
        hFile << TabPad(_CTS("AccountIcon_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    for (vector<CAnnouncerVoice>::iterator it(m_vAnnouncerVoices.begin()); it != m_vAnnouncerVoices.end(); ++it)
        hFile << TabPad(_CTS("AnnouncerVoice_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    hFile << newl;
}
