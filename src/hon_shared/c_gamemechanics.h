// (C)2008 S2 Games
// c_gamemechanics.h
//
//=============================================================================
#ifndef __C_GAMEMECHANICS_H__
#define __C_GAMEMECHANICS_H__

//=============================================================================
// Declarations
//=============================================================================
class CGameMechanics;
class CXMLNode;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint INVALID_ATTACK_TYPE(-1);
const uint INVALID_COMBAT_TYPE(-1);
const uint INVALID_TARGET_SCHEME(-1);
const uint INVALID_ARMOR_TYPE(-1);
const byte INVALID_POPUP(-1);
const byte INVALID_PING(-1);

enum EGlobalCondition
{
    GLOBAL_CONDITION_DAY,
    GLOBAL_CONDITION_NIGHT,

    INVALID_GLOBAL_CONDITION
};

enum ETargetTrait
{
    TARGET_TRAIT_ALL,
    TARGET_TRAIT_NONE,

    TARGET_TRAIT_SELF,
    TARGET_TRAIT_OTHER,

    TARGET_TRAIT_ALLY,
    TARGET_TRAIT_ENEMY,
    TARGET_TRAIT_NEUTRAL,
    TARGET_TRAIT_PASSIVE,
    TARGET_TRAIT_FRIENDLY,

    TARGET_TRAIT_ALIVE,
    TARGET_TRAIT_CORPSE,
    TARGET_TRAIT_DEAD,

    TARGET_TRAIT_UNIT,
    TARGET_TRAIT_HERO,
    TARGET_TRAIT_CREEP,
    TARGET_TRAIT_BUILDING,
    TARGET_TRAIT_PET,
    TARGET_TRAIT_GADGET,
    TARGET_TRAIT_POWERUP,
    TARGET_TRAIT_TREE,
    TARGET_TRAIT_CHEST,

    TARGET_TRAIT_ILLUSION,

    TARGET_TRAIT_MINE,
    TARGET_TRAIT_PLAYER_CONTROLLED,
    TARGET_TRAIT_OWNER,
    TARGET_TRAIT_OWNED,

    TARGET_TRAIT_DENIABLE,
    TARGET_TRAIT_SMACKABLE,
    TARGET_TRAIT_NOHELP,
    TARGET_TRAIT_VISIBLE,
    TARGET_TRAIT_FULL,

    TARGET_TRAIT_PERKS,

    TARGET_TRAIT_IMMOBILIZED,
    TARGET_TRAIT_RESTRAINED,
    TARGET_TRAIT_DISARMED,
    TARGET_TRAIT_SILENCED,
    TARGET_TRAIT_PERPLEXED,
    TARGET_TRAIT_STUNNED,

    TARGET_TRAIT_STEALTH,

    TARGET_TRAIT_MOVING,
    TARGET_TRAIT_IDLE,
    TARGET_TRAIT_ATTACKING,
    TARGET_TRAIT_CASTING,

    TARGET_TRAIT_MANAPOOL,

    TARGET_TRAIT_DELETED,

    INVALID_TARGET_TYPE
};

enum EPopup
{
    POPUP_GOLD,
    POPUP_EXPERIENCE,
    POPUP_CRITICAL,
    POPUP_DEFLECTION,
    POPUP_DENY,
    POPUP_MISS,
    POPUP_TOOFAR,
    POPUP_CREEP_KILL,

    NUM_RESERVED_POPUPS
};

enum EPing
{
    PING_ALERT,
    PING_BUILDING_ATTACK,
    PING_KILL_HERO,
    PING_ALLY_BUILDING_KILL,
    PING_ENEMY_BUILDING_KILL,
    PING_ATTACK_BUILDING,
    PING_ATTACK_HERO,
    PING_PROTECT_BUILDING,

    NUM_RESERVED_PINGS
};

enum EStateStackType
{
    STATE_STACK_NONE = 0,
    STATE_STACK_NOSELF,
    STATE_STACK_FULL,

    INVALID_STATE_STACK_TYPE
};

EGlobalCondition    GetGlobalConditionFromString(const tstring &sName);
ETargetTrait        GetTargetTraitFromString(const tstring &sName);
EStateStackType     GetStateStackTypeFromString(const tstring &sName);
//=============================================================================

//=============================================================================
// CEffectType
//=============================================================================
class CEffectType
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;
    uint    m_uiEffectBit;
    bool    m_bAssist;

    CEffectType();

public:
    ~CEffectType()  {}
    GAME_SHARED_API CEffectType(const tstring &sName, uint uiEffectBit, bool bAssist);

    void                            SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&                  GetName() const                     { return m_sName; }
    GAME_SHARED_API const tstring&  GetDisplayName() const;
    uint                            GetEffectBit() const                { return m_uiEffectBit; }
    bool                            GetAssist() const                   { return m_bAssist; }
};
//=============================================================================

//=============================================================================
// CImmunityType
//=============================================================================
class CImmunityType
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;
    uint    m_uiImmunityBits;

    CImmunityType();

public:
    ~CImmunityType()    {}
    GAME_SHARED_API CImmunityType(const tstring &sName, uint uiImmunityBits);

    void                            SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&                  GetName() const                     { return m_sName; }
    GAME_SHARED_API const tstring&  GetDisplayName() const;
    uint                            GetImmunityBits() const             { return m_uiImmunityBits; }
};
//=============================================================================

//=============================================================================
// CAttackType
//=============================================================================
class CAttackType
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;

    float   m_fDeniedExpMultiplier;
    float   m_fUphillMissChance;

    CAttackType();

public:
    ~CAttackType()  {}
    GAME_SHARED_API CAttackType(const tstring &sName, float fDeniedExpMultiplier, float fUphillMissChance);

    void                            SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&                  GetName() const                     { return m_sName; }
    GAME_SHARED_API const tstring&  GetDisplayName() const;
    float                           GetDeniedExpMultiplier() const      { return m_fDeniedExpMultiplier; }
    float                           GetUphillMissChance() const         { return m_fUphillMissChance; }
};
//=============================================================================

//=============================================================================
// CTargetScheme
//=============================================================================
class CTargetScheme
{
public:
    enum ETestType
    {
        TARGET_SCHEME_TEST_GLOBAL,
        TARGET_SCHEME_TEST_NOT_GLOBAL,
        TARGET_SCHEME_TEST_TRAIT,
        TARGET_SCHEME_TEST_NOT_TRAIT,
        TARGET_SCHEME_TEST_ATTACK,
        TARGET_SCHEME_TEST_NOT_ATTACK,
        TARGET_SCHEME_TEST_ENTITY,
        TARGET_SCHEME_TEST_NOT_ENTITY,
        TARGET_SCHEME_TEST_UNIT_TYPE,
        TARGET_SCHEME_TEST_NOT_UNIT_TYPE,
        TARGET_SCHEME_TEST_ATTRIBUTE,
        TARGET_SCHEME_TEST_NOT_ATTRIBUTE,
        TARGET_SCHEME_TEST_STRING,
        TARGET_SCHEME_TEST_NOT_STRING
    };

    struct STestRecord
    {
        ETestType           m_eTest;
        tstring             m_sString;

        union
        {
            ETargetTrait        m_eTrait;
            EGlobalCondition    m_eGlobal;
            uint                m_uiValue;
            EAttribute          m_eAttribute;
        };

        STestRecord(ETestType eTest, ETargetTrait eTrait) : m_eTest(eTest) { m_eTrait = eTrait; }
        STestRecord(ETestType eTest, EGlobalCondition eGlobal) : m_eTest(eTest) { m_eGlobal = eGlobal; }
        STestRecord(ETestType eTest, uint uiValue) : m_eTest(eTest) { m_uiValue = uiValue; }
        STestRecord(ETestType eTest, const tstring &sString) : m_eTest(eTest) { m_sString = sString; }
        STestRecord(ETestType eTest, EAttribute eAttribute) : m_eTest(eTest) { m_eAttribute = eAttribute; }
    };

    typedef vector<STestRecord>                 TestRecordVector;
    typedef TestRecordVector::iterator          TestRecordVector_it;
    typedef TestRecordVector::const_iterator    TestRecordVector_cit;

private:
    tstring             m_sName;
    uint                m_uiDisplayNameIndex;

    vector<STestRecord> m_vAllow;
    vector<STestRecord> m_vRestrict;

    vector<STestRecord> m_vAllow2;
    vector<STestRecord> m_vRestrict2;

    CTargetScheme();

public:
    ~CTargetScheme()    {}
    GAME_SHARED_API CTargetScheme(const tstring &sName, const tstring &sAllow, const tstring &sRestrict, const tstring &sAllow2, const tstring &sRestrict2, const CGameMechanics *pMechanics);

    void                            SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&                  GetName() const                     { return m_sName; }
    GAME_SHARED_API const tstring&  GetDisplayName() const;
    const vector<STestRecord>&      GetAllowList() const                { return m_vAllow; }
    const vector<STestRecord>&      GetRestrictList() const             { return m_vRestrict; }
    const vector<STestRecord>&      GetAllow2List() const               { return m_vAllow2; }
    const vector<STestRecord>&      GetRestrict2List() const            { return m_vRestrict2; }
};
//=============================================================================

//=============================================================================
// CStealthType
//=============================================================================
class CStealthType
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;
    uint    m_uiStealthBit;

    CStealthType();

public:
    ~CStealthType() {}
    GAME_SHARED_API CStealthType(const tstring &sName, uint uiStealthBit);

    void                            SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&                  GetName() const                     { return m_sName; }
    GAME_SHARED_API const tstring&  GetDisplayName() const;
    uint                            GetStealthBit() const               { return m_uiStealthBit; }
};
//=============================================================================

//=============================================================================
// CRevealType
//=============================================================================
class CRevealType
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;
    uint    m_uiRevealBits;

    CRevealType();

public:
    ~CRevealType()  {}
    CRevealType(const tstring &sName, uint uiRevealBits);

    void                            SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&                  GetName() const                     { return m_sName; }
    GAME_SHARED_API const tstring&  GetDisplayName() const;
    uint                            GetRevealBits() const               { return m_uiRevealBits; }
};
//=============================================================================

//=============================================================================
// CArmorType
//=============================================================================
class CArmorType
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;
    uint    m_uiEffectType;
    float   m_fFactor;

    CArmorType();

public:
    ~CArmorType()   {}
    GAME_SHARED_API CArmorType(const tstring &sName, uint uiEffectType, float fFactor);

    void                            SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&                  GetName() const                     { return m_sName; }
    GAME_SHARED_API const tstring&  GetDisplayName() const;

    bool    IsEffective(uint uiEffectType) const                        { return (m_uiEffectType & uiEffectType) != 0; }
    float   GetDamageAdjustment(float fArmor) const
    {
        if (fArmor >= 0.0f)
            return (fArmor * m_fFactor) / (1.0f + (fArmor * m_fFactor));
        return pow(1.0f - m_fFactor, -fArmor) - 1.0f;
    }
};
//=============================================================================

//=============================================================================
// CPopup
//=============================================================================
class CPopup
{
private:
    tstring m_sName;
    byte    m_yType;

    uint    m_uiMessageIndex;
    bool    m_bShowValue;
    bool    m_bUsePlayerColor;
    bool    m_bSelfOnly;
    bool    m_bTeamOnly;
    bool    m_bSpectatorOnly;
    CVec4f  m_v4Color;
    float   m_fStartX;
    float   m_fStartY;
    float   m_fSpeedX;
    float   m_fSpeedY;
    uint    m_uiDuration;
    uint    m_uiFadeTime;

    CPopup();

public:
    ~CPopup()   {}
    CPopup(const tstring &sName, byte yType, const CXMLNode &node);

    void                            SetMessageIndex(uint uiIndex)       { m_uiMessageIndex = uiIndex; }

    const tstring&                  GetName() const                     { return m_sName; }
    byte                            GetType() const                     { return m_yType; }
    GAME_SHARED_API tstring         GetMessage(ushort unValue = 0) const;
    GAME_SHARED_API const tstring&  GetRawMessage() const;
    bool                            GetShowValue() const                { return m_bShowValue; }
    bool                            GetUsePlayerColor() const           { return m_bUsePlayerColor; }
    bool                            GetSelfOnly() const                 { return m_bSelfOnly; }
    bool                            GetTeamOnly() const                 { return m_bTeamOnly; }
    bool                            GetSpectatorOnly() const            { return m_bSpectatorOnly; }
    const CVec4f&                   GetColor() const                    { return m_v4Color; }
    float                           GetStartX() const                   { return m_fStartX; }
    float                           GetStartY() const                   { return m_fStartY; }
    float                           GetSpeedX() const                   { return m_fSpeedX; }
    float                           GetSpeedY() const                   { return m_fSpeedY; }
    uint                            GetDuration() const                 { return m_uiDuration; }
    uint                            GetFadeTime() const                 { return m_uiFadeTime; }
};
//=============================================================================

//=============================================================================
// CPing
//=============================================================================
class CPing
{
private:
    tstring m_sName;
    byte    m_yType;

    bool    m_bUsePlayerColor;
    CVec4f  m_v4Color;
    tstring m_sEffectPath;
    bool    m_bSelfOnly;
    bool    m_bTargetOnly;
    bool    m_bTeamOnly;

    CPing();

public:
    ~CPing()    {}
    CPing(const tstring &sName, byte yType, const CXMLNode &node);

    const tstring&                  GetName() const                     { return m_sName; }
    byte                            GetType() const                     { return m_yType; }
    bool                            GetUsePlayerColor() const           { return m_bUsePlayerColor; }
    const CVec4f&                   GetColor() const                    { return m_v4Color; }
    const tstring&                  GetEffectPath() const               { return m_sEffectPath; }
    bool                            GetSelfOnly() const                 { return m_bSelfOnly; }
    bool                            GetTargetOnly() const               { return m_bTargetOnly; }
    bool                            GetTeamOnly() const                 { return m_bTeamOnly; }
};
//=============================================================================

//=============================================================================
// CCombatTable
//=============================================================================
class CCombatTable
{
private:
    tstring m_sName;
    uint    m_uiDisplayNameIndex;

    fvector m_vAttackMultiplier;
    fvector m_vSpellMultiplier;
    ivector m_vAggroPriority;
    ivector m_vAttackPriority;
    ivector m_vProximityPriority;
    ivector m_vTargetPriority;

    const CGameMechanics*   m_pGameMechanics;

    CCombatTable();

public:
    ~CCombatTable() {}
    GAME_SHARED_API CCombatTable(const tstring &sName, uint uiNumCombatTypes, const CGameMechanics *pMechanics);

    void                            SetDisplayNameIndex(uint uiIndex)   { m_uiDisplayNameIndex = uiIndex; }

    const tstring&                  GetName() const                     { return m_sName; }
    GAME_SHARED_API const tstring&  GetDisplayName() const;

    const CGameMechanics*   GetGameMechanics() const                    { return m_pGameMechanics; }

    void    SetAttackMultiplier(uint uiTargetType, float fMult)         { if (uiTargetType >= m_vAttackMultiplier.size()) return; m_vAttackMultiplier[uiTargetType] = fMult; }
    void    SetSpellMultiplier(uint uiTargetType, float fMult)          { if (uiTargetType >= m_vSpellMultiplier.size()) return; m_vSpellMultiplier[uiTargetType] = fMult; }
    void    SetAggroPriority(uint uiTargetType, int iPriority)          { if (uiTargetType >= m_vAggroPriority.size()) return; m_vAggroPriority[uiTargetType] = iPriority; }
    void    SetAttackPriority(uint uiTargetType, int iPriority)         { if (uiTargetType >= m_vAttackPriority.size()) return; m_vAttackPriority[uiTargetType] = iPriority; }
    void    SetProximityPriority(uint uiTargetType, int iPriority)      { if (uiTargetType >= m_vProximityPriority.size()) return; m_vProximityPriority[uiTargetType] = iPriority; }
    void    SetTargetPriority(uint uiTargetType, int iPriority)         { if (uiTargetType >= m_vTargetPriority.size()) return; m_vTargetPriority[uiTargetType] = iPriority; }

    float   GetAttackMultiplier(uint uiTargetType)                      { return (uiTargetType >= m_vAttackMultiplier.size()) ? 1.0f : m_vAttackMultiplier[uiTargetType]; }
    float   GetSpellMultiplier(uint uiTargetType)                       { return (uiTargetType >= m_vSpellMultiplier.size()) ? 1.0f : m_vSpellMultiplier[uiTargetType]; }
    int     GetAggroPriority(uint uiTargetType)                         { return (uiTargetType >= m_vAggroPriority.size()) ? 0.0f : m_vAggroPriority[uiTargetType]; }
    int     GetAttackPriority(uint uiTargetType)                        { return (uiTargetType >= m_vAttackPriority.size()) ? 0.0f : m_vAttackPriority[uiTargetType]; }
    int     GetProximityPriority(uint uiTargetType)                     { return (uiTargetType >= m_vProximityPriority.size()) ? 0.0f : m_vProximityPriority[uiTargetType]; }
    int     GetTargetPriority(uint uiTargetType)                        { return (uiTargetType >= m_vTargetPriority.size()) ? 0.0f : m_vTargetPriority[uiTargetType]; }
};
//=============================================================================

//=============================================================================
// CGameMechanics
//=============================================================================
class CGameMechanics
{
private:
    map<tstring, uint>      m_mapAttackTypes;
    vector<CAttackType>     m_vAttackTypes;

    map<tstring, uint>      m_mapCombatTypes;
    vector<CCombatTable>    m_vCombatTables;

    uint                    m_uiEffectTypeBitMarker;
    map<tstring, uint>      m_mapEffectTypes;
    vector<CEffectType>     m_vEffectTypes;
    map<tstring, uint>      m_mapImmunityTypes;
    vector<CImmunityType>   m_vImmunityTypes;

    uint                    m_uiStealthBitMarker;
    map<tstring, uint>      m_mapStealthTypes;
    vector<CStealthType>    m_vStealthTypes;
    map<tstring, uint>      m_mapRevealTypes;
    vector<CRevealType>     m_vRevealTypes;

    map<tstring, uint>      m_mapArmorTypes;
    vector<CArmorType>      m_vArmorTypes;

    map<tstring, uint>      m_mapTargetSchemes;
    vector<CTargetScheme>   m_vTargetSchemes;

    map<tstring, byte>      m_mapPopups;
    vector<CPopup>          m_vPopups;
    const CPopup*           m_apPopups[NUM_RESERVED_POPUPS];

    map<tstring, byte>      m_mapPings;
    vector<CPing>           m_vPings;
    const CPing*            m_apPings[NUM_RESERVED_PINGS];

    uint                    m_uiDebuffEffectType;
    uint                    m_uiBuffEffectType;
    uint                    m_uiDisableEffectType;

public:
    ~CGameMechanics()   {}
    CGameMechanics();

    void                    Clear();
    GAME_SHARED_API void    PostLoad();

    uint                RegisterAttackType(const CXMLNode &node);
    uint                LookupAttackType(const tstring &sName) const;
    const CAttackType*  GetAttackType(uint uiIndex) const                   { return uiIndex < m_vAttackTypes.size() ? &m_vAttackTypes[uiIndex] : NULL; }
    const tstring&      GetAttackTypeDisplayName(uint uiIndex) const        { return uiIndex < m_vAttackTypes.size() ? m_vAttackTypes[uiIndex].GetDisplayName() : TSNULL; }

    uint                    RegisterCombatType(const tstring &sName, uint uiNumCombatTypes);
    uint                    LookupCombatType(const tstring &sName) const    { map<tstring, uint>::const_iterator it(m_mapCombatTypes.find(sName)); return (it == m_mapCombatTypes.end()) ? INVALID_COMBAT_TYPE : it->second; }
    CCombatTable*           GetCombatTable(uint uiIndex)                    { return (uiIndex >= m_vCombatTables.size()) ? NULL : &m_vCombatTables[uiIndex]; }
    CCombatTable*           GetCombatTable(const tstring &sName)            { return GetCombatTable(LookupCombatType(sName)); }
    float                   GetAttackMultiplier(uint uiCombatTypeA, uint uiCombatTypeB);
    float                   GetSpellMultiplier(uint uiCombatTypeA, uint uiCombatTypeB);
    int                     GetAggroPriority(uint uiCombatTypeA, uint uiCombatTypeB);
    int                     GetTargetPriority(uint uiCombatTypeA, uint uiCombatTypeB);
    int                     GetAttackPriority(uint uiCombatTypeA, uint uiCombatTypeB);
    int                     GetProximityPriority(uint uiCombatTypeA, uint uiCombatTypeB);

    uint                    RegisterEffectType(const tstring &sName, bool bAssist);
    uint                    LookupEffectType(const tstring &sName) const;
    uint                    RegisterImmunityType(const tstring &sName, const tstring &sImmunityList);
    uint                    LookupImmunityType(const tstring &sName) const;
    GAME_SHARED_API tstring GetEffectTypeString(uint uiEffectType) const;
    GAME_SHARED_API bool    IsDebuff(uint uiEffectType) const;
    GAME_SHARED_API bool    IsBuff(uint uiEffectType) const;
    GAME_SHARED_API bool    IsDisable(uint uiEffectType) const;
    GAME_SHARED_API bool    IsAssist(uint uiEffectType) const;

    uint                    RegisterArmorType(const tstring &sName, const tstring &sEffects, float fFactor);
    uint                    LookupArmorType(const tstring &sName) const;
    bool                    IsArmorEffective(uint uiArmorType, uint uiEffectType) const;
    GAME_SHARED_API float   GetArmorDamageAdjustment(uint uiArmorType, float fArmor) const;
    const CArmorType*       GetArmorType(uint uiArmorType) const            { return uiArmorType < m_vArmorTypes.size() ? &m_vArmorTypes[uiArmorType] : NULL; }

    uint    RegisterStealthType(const tstring &sName);
    uint    LookupStealthType(const tstring &sName) const;
    uint    RegisterRevealType(const tstring &sName, const tstring &sRevealList);
    uint    LookupRevealType(const tstring &sName) const;

    uint                    RegisterTargetScheme(const tstring &sName, const tstring &sAllow, const tstring &sRestrict, const tstring &sAllow2, const tstring &sRestrict2);
    GAME_SHARED_API uint    LookupTargetScheme(const tstring &sName) const;
    const tstring&          GetTargetSchemeDisplayName(uint uiIndex) const  { return uiIndex < m_vTargetSchemes.size() ? m_vTargetSchemes[uiIndex].GetDisplayName() : TSNULL; }
    const CTargetScheme*    GetTargetScheme(uint uiIndex) const             { return uiIndex < m_vTargetSchemes.size() ? &m_vTargetSchemes[uiIndex] : NULL; }

    byte                    RegisterPopup(const CXMLNode &node);
    GAME_SHARED_API byte    LookupPopup(const tstring &sName) const;
    const CPopup*           GetPopup(byte yIndex) const                     { return yIndex < m_vPopups.size() ? &m_vPopups[yIndex] : NULL; }
    const CPopup*           GetPopup(EPopup eType) const                    { return m_apPopups[eType]; }

    byte                    RegisterPing(const CXMLNode &node);
    GAME_SHARED_API byte    LookupPing(const tstring &sName) const;
    const CPing*            GetPing(byte yIndex) const                      { return yIndex < m_vPings.size() ? &m_vPings[yIndex] : NULL; }
    const CPing*            GetPing(EPing eType) const                      { return m_apPings[eType]; }

    GAME_SHARED_API void    WriteStringTable(CFileHandle &hFile, size_t zTabStop, size_t zColumnOffset);
};
//=============================================================================

#endif //__C_GAMEMECHANICS_H__

