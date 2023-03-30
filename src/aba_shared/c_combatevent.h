// (C)2008 S2 Games
// c_combatevent.h
//
//=============================================================================
#ifndef __C_COMBATEVENT_H__
#define __C_COMBATEVENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_damageevent.h"
#include "i_combataction.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SCombatEventStateEntry
{
    tstring m_sName;
    uint    m_uiDuration;
    float   m_fChance;
    uint    m_uiLevel;

    SCombatEventStateEntry(const tstring &sName, uint uiLevel, uint uiDuration, float fChance) :
    m_sName(sName),
    m_uiLevel(uiLevel),
    m_uiDuration(uiDuration),
    m_fChance(fChance)
    {}
};

struct SCombatEventActionEntry
{
    tstring m_sName;
    float   m_fParam;
    float   m_fChance;

    SCombatEventActionEntry(const tstring &sName, float fParam, float fChance) :
    m_sName(sName),
    m_fParam(fParam),
    m_fChance(fChance)
    {}
};

typedef vector<SCombatEventActionEntry> CombatActionVector;
typedef CombatActionVector::iterator    CombatActionVector_it;

typedef vector<SCombatEventStateEntry>  CombatStateVector;
typedef CombatStateVector::iterator     CombatStateVector_it;

typedef pair<float, float>          FloatPair;
typedef vector<FloatPair>           FloatPairVector;
typedef FloatPairVector::iterator   FloatPairVector_it;
//=============================================================================

//=============================================================================
// CCombatEvent
//=============================================================================
class CCombatEvent
{
private:
    uint                m_uiInitiatorIndex;
    uint                m_uiInflictorIndex;

    uint                m_uiTargetIndex;
    CVec3f              m_v3TargetPosition;
    CVec3f              m_v3TargetDelta;

    uint                m_uiProxyUID;

    uint                m_uiEffectType;

    ESuperType          m_eSuperType;

    // Damage
    CDamageEvent        m_dmg;
    uint                m_uiDamageType;
    float               m_fBaseDamage;
    float               m_fAdditionalDamage;
    float               m_fDamageMultiplier;

    float               m_fBonusMultiplier;
    float               m_fBonusDamage;
    float               m_fTotalAdjustedDamage;
    
    // Attack modifiers
    float               m_fLifeSteal;
    
    FloatPairVector     m_vCriticals;

    float               m_fEvasion;
    float               m_fMissChance;
    float               m_fDeflection;

    bool                m_bNonLethal;
    bool                m_bTrueStrike;
    bool                m_bNoResponse;
    int                 m_iIssuedClientNumber;
    bool                m_bAttackAbility;

    vector<CCombatActionScript> m_aActionScripts[NUM_ACTION_SCRIPTS];

    byte                m_yOldRedirectionSequence;
    byte                m_yRedirectionSequence;

    bool                m_bNegated;
    bool                m_bSuccessful;
    bool                m_bRemoveBounceScripts;
    bool                m_bInvalid;

    // Activate
    float               m_fManaCost;
    uint                m_uiCooldownTime;

    void    CheckBounceScript(EEntityActionScript eScript);

public:
    ~CCombatEvent() {}
    CCombatEvent();

    void    SetInitiatorIndex(uint uiIndex)                 { m_uiInitiatorIndex = uiIndex; }
    void    SetInflictorIndex(uint uiIndex)                 { m_uiInflictorIndex = uiIndex; }

    void    SetTarget(uint uiIndex)                         { m_uiTargetIndex = uiIndex; }
    void    SetTarget(const CVec3f &v3Position)             { m_v3TargetPosition = v3Position; }
    uint    GetTarget() const                               { return m_uiTargetIndex; }

    void            SetDelta(const CVec3f &v3Delta)         { m_v3TargetDelta = v3Delta; }
    const CVec3f&   GetDelta() const                        { return m_v3TargetDelta; }

    void    SetProxyUID(uint uiUID)                         { m_uiProxyUID = uiUID; }
    uint    GetProxyUID() const                             { return m_uiProxyUID; }

    void    SetEffectType(uint uiEffectType)                { m_uiEffectType = uiEffectType; }
    uint    GetEffectType() const                           { return m_uiEffectType; }

    void        SetSuperType(ESuperType eType)              { m_eSuperType = eType; }
    ESuperType  GetSuperType() const                        { return m_eSuperType; }

    void    SetDamageType(uint uiType)                      { m_uiDamageType = uiType; }
    void    SetBaseDamage(float fBaseDamage)                { m_fBaseDamage = fBaseDamage; }
    void    SetAdditionalDamage(float fAdditionalDamage)    { m_fAdditionalDamage = fAdditionalDamage; }
    void    SetDamageMultiplier(float fDamageMultiplier)    { m_fDamageMultiplier = fDamageMultiplier; }
    
    void    AddBonusDamageMultiplier(float fMult)           { m_fBonusMultiplier += fMult; }
    void    AddBonusDamage(float fDamage)                   { m_fBonusDamage += fDamage; }
    
    uint    GetDamageType() const                           { return m_uiDamageType; }
    float   GetBaseDamage() const                           { return m_fBaseDamage; }
    float   GetAdditionalDamage() const                     { return m_fAdditionalDamage; }
    float   GetDamageMultiplier() const                     { return m_fDamageMultiplier; }
    
    void    SetBonusDamage(float fBonusDamage)              { m_fBonusDamage = fBonusDamage; }
    float   GetBonusDamage() const                          { return m_fBonusDamage; }

    void    SetBonusMultiplier(float fBonusMultiplier)      { m_fBonusMultiplier = fBonusMultiplier; }
    float   GetBonusMultiplier() const                      { return m_fBonusMultiplier; }

    float   GetTotalAdjustedDamage() const                  { return m_fTotalAdjustedDamage; }
    float   GetAppliedDamage() const                        { return m_dmg.GetAppliedDamage(); }

    void    SetTotalAdjustedDamage(float fValue)            { m_fTotalAdjustedDamage = fValue; }

    void    SetLifeSteal(float fAmount)                     { m_fLifeSteal = fAmount; }
    float   GetLifeSteal() const                            { return m_fLifeSteal; }

    void    AddCritical(float fChance, float fMult)         { if (fChance > 0.0f) m_vCriticals.push_back(FloatPair(fChance, fMult)); }
    void    ClearCriticals()                                { m_vCriticals.clear(); }

    void    SetEvasion(float fChance)                       { m_fEvasion = fChance; }
    float   GetEvasion() const                              { return m_fEvasion; }

    void    SetMissChance(float fChance)                    { m_fMissChance = fChance; }
    float   GetMissChance() const                           { return m_fMissChance; }

    void    AddDeflection(float fAmount)                    { m_fDeflection = MAX(fAmount, m_fDeflection); }
    void    SetDeflection(float fDeflection)                { m_fDeflection = fDeflection; }
    float   GetDeflection() const                           { return m_fDeflection; }

    void    SetNonLethal(bool bNonLethal)                   { m_bNonLethal = bNonLethal; }
    bool    GetNonLethal() const                            { return m_bNonLethal; }

    void    SetTrueStrike(bool bTrueStrike)                 { m_bTrueStrike = bTrueStrike; }
    bool    GetTrueStrike() const                           { return m_bTrueStrike; }

    void    SetNoResponse(bool bNoResponse)                 { m_bNoResponse = bNoResponse; }
    bool    GetNoResponse() const                           { return m_bNoResponse; }

    void    SetAttackAbility(bool bAttackAbility)           { m_bAttackAbility = bAttackAbility; }
    bool    GetAttackAbility() const                        { return m_bAttackAbility; }

    void    SetNegated(bool bNegated)                       { m_bNegated = bNegated; }
    bool    GetNegated() const                              { return m_bNegated; }

    void    SetIssuedClientNumber(int iClientNumber)        { m_iIssuedClientNumber = iClientNumber; }
    int     GetIssuedClientNumber() const                   { return m_iIssuedClientNumber; }

    void    SetInvalid(bool bInvalid)                       { m_bInvalid = bInvalid; }
    bool    GetInvalid() const                              { return m_bInvalid; }

    CCombatActionScript&    AddActionScript(EEntityActionScript eScript, const CCombatActionScript &cScript)
    {
        m_aActionScripts[eScript].push_back(cScript);
        return m_aActionScripts[eScript].back();
    }

    void    Process();
    bool    PreImpact();
    void    Impact();
    void    PostImpact();

    void    Redirect()              { ++m_yRedirectionSequence; }

    bool    GetSuccessful() const   { return m_bSuccessful; }

    void    ProcessInvalid();

    void    Reset();

    void    ClearBounceScripts()    { m_bRemoveBounceScripts = true; }
    void    CheckBounceScripts();

    void    AdjustDamageEvent(CDamageEvent &cDmg, IUnitEntity *pTarget);

    void        SetManaCost(float fManaCost)            { m_fManaCost = fManaCost; }
    float       GetManaCost() const                     { return m_fManaCost; }

    // Activate
    void        SetCooldownTime(uint uiCooldownTime)    { m_uiCooldownTime = uiCooldownTime; }
    uint        GetCooldownTime() const                 { return m_uiCooldownTime; }
};
//=============================================================================

#endif //__C_COMBATEVENT_H__
