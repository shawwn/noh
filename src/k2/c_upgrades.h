// (C)2010 S2 Games
// c_upgrades.h
//
//=============================================================================
#ifndef __C_UPGRADES_H__
#define __C_UPGRADES_H__

//=============================================================================
// Declarations
//=============================================================================
class CUpgrades;
class CXMLNode;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EUpgradeType
{
    UPGRADE_VALUE_INVALID = 0,

    UPGRADE_CHAT_SYMBOL,
    UPGRADE_CHAT_NAME_COLOR,
    UPGRADE_ACCOUNT_ICON,
    UPGRADE_ANNOUNCER_VOICE,
    UPGRADE_TAUNT
};

//template<> inline EUpgradeType    GetDefaultEmptyValue<EUpgradeType>()    { return UPGRADE_VALUE_INVALID; }

inline EUpgradeType GetUpgradeTypeFromString(const tstring &sUpgradeType)
{
    if (TStringCompare(sUpgradeType, _T("cs")) == 0)
        return UPGRADE_CHAT_SYMBOL;
    else if (TStringCompare(sUpgradeType, _T("cc")) == 0)
        return UPGRADE_CHAT_NAME_COLOR;
    else if (TStringCompare(sUpgradeType, _T("ai")) == 0)
        return UPGRADE_ACCOUNT_ICON;
    else if (TStringCompare(sUpgradeType, _T("av")) == 0)
        return UPGRADE_ANNOUNCER_VOICE;
    else if (TStringCompare(sUpgradeType, _T("t")) == 0)
        return UPGRADE_TAUNT;
    else
        return UPGRADE_VALUE_INVALID;
}

inline EUpgradeType&    AtoX(const tstring &s, EUpgradeType &e) { return e = GetUpgradeTypeFromString(s); }
//=============================================================================

//=============================================================================
// CChatSymbol
//=============================================================================
class CChatSymbol
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;

    tstring m_sTexturePath;

    CChatSymbol();

public:
    ~CChatSymbol()  {}
    K2_API CChatSymbol(const tstring &sName, const tstring &sTexturePath);

    void                    SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&          GetName() const                     { return m_sName; }
    K2_API const tstring&   GetDisplayName() const;
    const tstring&          GetTexturePath() const              { return m_sTexturePath; }
};
//=============================================================================

//=============================================================================
// CChatNameColor
//=============================================================================
class CChatNameColor
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;

    tstring m_sTexturePath;
    tstring m_sColor;
    tstring m_sIngameColor;
    uint    m_uiSortIndex;

    CChatNameColor();

public:
    ~CChatNameColor()   {}
    K2_API CChatNameColor(const tstring &sName, const tstring &sTexturePath, const tstring &sColor, const tstring &sIngameColor, uint uiSortIndex);

    void                    SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&          GetName() const                     { return m_sName; }
    K2_API const tstring&   GetDisplayName() const;
    const tstring&          GetTexturePath() const              { return m_sTexturePath; }
    const tstring&          GetColor() const                    { return m_sColor; }
    const tstring&          GetIngameColor() const              { return m_sIngameColor; }
    uint                    GetSortIndex() const                { return m_uiSortIndex; }
};
//=============================================================================

//=============================================================================
// CAccountIcon
//=============================================================================
class CAccountIcon
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;

    tstring m_sTexturePath;

    CAccountIcon();

public:
    ~CAccountIcon() {}
    K2_API CAccountIcon(const tstring &sName, const tstring &sTexturePath);

    void                    SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&          GetName() const                     { return m_sName; }
    K2_API const tstring&   GetDisplayName() const;
    const tstring&          GetTexturePath() const              { return m_sTexturePath; }
};
//=============================================================================

//=============================================================================
// CAnnouncerVoice
//=============================================================================
class CAnnouncerVoice
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;

    tstring m_sVoiceSet;
    tstring m_sArcadeText;

    CAnnouncerVoice();

public:
    ~CAnnouncerVoice()  {}
    K2_API CAnnouncerVoice(const tstring &sName, const tstring &sVoiceSet, const tstring &sArcadeText);

    void                    SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&          GetName() const                     { return m_sName; }
    K2_API const tstring&   GetDisplayName() const;
    const tstring&          GetVoiceSet() const                 { return m_sVoiceSet; }
    const tstring&          GetArcadeText() const               { return m_sArcadeText; }
};
//=============================================================================

//=============================================================================
// CTaunt
//=============================================================================
class CTaunt
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;

    tstring m_sModifier;

    CTaunt();

public:
    ~CTaunt()   {}
    K2_API CTaunt(const tstring &sName, const tstring &sModifier);

    void                    SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&          GetName() const                     { return m_sName; }
    K2_API const tstring&   GetDisplayName() const;
    const tstring&          GetModifier() const                 { return m_sModifier; }
};
//=============================================================================

//=============================================================================
// CUpgrades
//=============================================================================
class CUpgrades
{
private:
    map<tstring, uint>      m_mapChatSymbols;
    vector<CChatSymbol>     m_vChatSymbols;

    map<tstring, uint>      m_mapChatNameColors;
    vector<CChatNameColor>  m_vChatNameColors;

    map<tstring, uint>      m_mapAccountIcons;
    vector<CAccountIcon>    m_vAccountIcons;

    map<tstring, uint>      m_mapAnnouncerVoices;
    vector<CAnnouncerVoice> m_vAnnouncerVoices;

    map<tstring, uint>      m_mapTaunts;
    vector<CTaunt>  m_vTaunts;

public:
    ~CUpgrades()    {}
    CUpgrades();

    void                Clear();
    K2_API void         PostLoad();

    uint                RegisterChatSymbol(const CXMLNode &node);
    K2_API uint         LookupChatSymbol(const tstring &sName) const;
    uint                GetNumChatSymbols() const                           { return uint(m_vChatSymbols.size()); }
    const CChatSymbol*  GetChatSymbol(uint uiIndex) const                   { return uiIndex < m_vChatSymbols.size() ? &m_vChatSymbols[uiIndex] : nullptr; }
    const tstring&      GetChatSymbolDisplayName(uint uiIndex) const        { return uiIndex < m_vChatSymbols.size() ? m_vChatSymbols[uiIndex].GetDisplayName() : TSNULL; }
    const tstring&      GetChatSymbolTexturePath(uint uiIndex) const        { return uiIndex < m_vChatSymbols.size() ? m_vChatSymbols[uiIndex].GetTexturePath() : TSNULL; }

    uint                RegisterChatNameColor(const CXMLNode &node);
    K2_API uint         LookupChatNameColor(const tstring &sName) const;
    uint                GetNumChatNameColors() const                        { return uint(m_vChatNameColors.size()); }
    const CChatNameColor*   GetChatNameColor(uint uiIndex) const            { return uiIndex < m_vChatNameColors.size() ? &m_vChatNameColors[uiIndex] : nullptr; }
    const tstring&      GetChatNameColorDisplayName(uint uiIndex) const     { return uiIndex < m_vChatNameColors.size() ? m_vChatNameColors[uiIndex].GetDisplayName() : TSNULL; }
    const tstring&      GetChatNameColorTexturePath(uint uiIndex) const     { return uiIndex < m_vChatNameColors.size() ? m_vChatNameColors[uiIndex].GetTexturePath() : TSNULL; }
    const tstring&      GetChatNameColorString(uint uiIndex) const          { return uiIndex < m_vChatNameColors.size() ? m_vChatNameColors[uiIndex].GetColor() : TSNULL; }
    const tstring&      GetChatNameColorIngameString(uint uiIndex) const    { return uiIndex < m_vChatNameColors.size() ? m_vChatNameColors[uiIndex].GetIngameColor() : TSNULL; }
    uint                GetChatNameColorSortIndex(uint uiIndex) const       { return uiIndex < m_vChatNameColors.size() ? m_vChatNameColors[uiIndex].GetSortIndex() : 0; }

    uint                RegisterAccountIcon(const CXMLNode &node);
    K2_API uint         LookupAccountIcon(const tstring &sName) const;
    uint                GetNumAccountIcons() const                          { return uint(m_vAccountIcons.size()); }
    const CAccountIcon* GetAccountIcon(uint uiIndex) const                  { return uiIndex < m_vAccountIcons.size() ? &m_vAccountIcons[uiIndex] : nullptr; }
    const tstring&      GetAccountIconDisplayName(uint uiIndex) const       { return uiIndex < m_vAccountIcons.size() ? m_vAccountIcons[uiIndex].GetDisplayName() : TSNULL; }
    const tstring&      GetAccountIconTexturePath(uint uiIndex) const       { return uiIndex < m_vAccountIcons.size() ? m_vAccountIcons[uiIndex].GetTexturePath() : TSNULL; }

    uint                RegisterAnnouncerVoice(const CXMLNode &node);
    K2_API uint         LookupAnnouncerVoice(const tstring &sName) const;
    uint                GetNumAnnouncerVoices() const                       { return uint(m_vAnnouncerVoices.size()); }
    const CAnnouncerVoice*  GetAnnouncerVoice(uint uiIndex) const           { return uiIndex < m_vAnnouncerVoices.size() ? &m_vAnnouncerVoices[uiIndex] : nullptr; }
    const tstring&      GetAnnouncerVoiceDisplayName(uint uiIndex) const    { return uiIndex < m_vAnnouncerVoices.size() ? m_vAnnouncerVoices[uiIndex].GetDisplayName() : TSNULL; }
    const tstring&      GetAnnouncerVoiceSet(uint uiIndex) const            { return uiIndex < m_vAnnouncerVoices.size() ? m_vAnnouncerVoices[uiIndex].GetVoiceSet() : TSNULL; }
    const tstring&      GetAnnouncerVoiceArcadeText(uint uiIndex) const     { return uiIndex < m_vAnnouncerVoices.size() ? m_vAnnouncerVoices[uiIndex].GetArcadeText() : TSNULL; }

    uint                RegisterTaunt(const CXMLNode &node);
    K2_API uint         LookupTaunt(const tstring &sName) const;
    uint                GetNumTaunts() const                                { return uint(m_vTaunts.size()); }
    const CTaunt*       GetTaunt(uint uiIndex) const                        { return uiIndex < m_vTaunts.size() ? &m_vTaunts[uiIndex] : nullptr; }
    const tstring&      GetTauntDisplayName(uint uiIndex) const             { return uiIndex < m_vTaunts.size() ? m_vTaunts[uiIndex].GetDisplayName() : TSNULL; }
    const tstring&      GetTauntModifier(uint uiIndex) const                { return uiIndex < m_vTaunts.size() ? m_vTaunts[uiIndex].GetModifier() : TSNULL; }

    K2_API void WriteStringTable(CFileHandle &hFile, size_t zTabStop, size_t zColumnOffset);
};
//=============================================================================

#endif //__C_UPGRADES_H__

