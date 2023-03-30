// (C)2006 S2 Games
// c_meleeattackevent.h
//
//=============================================================================
#ifndef __C_MELEEATTACKEVENT_H__
#define __C_MELEEATTACKEVENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_range.h"
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IMeleeItem;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EMeleeMetric
{
    MELEE_HEIGHT,
    MELEE_RANGE,
    MELEE_ANGLE,

    NUM_MELEE_METRICS
};
//=============================================================================

//=============================================================================
// CMeleeAttackEvent
//=============================================================================
class CMeleeAttackEvent
{
private:
    bool        m_bActive;

    IMeleeItem* m_pWeapon;

    tstring     m_sAnimName;
    uint        m_uiAttackTime;
    uint        m_uiImmobileTime;
    CRangef     m_rangeDamage;
    svector     m_vStates;
    uivector    m_vStateDurations;

    bool        m_bImpacted;
    uint        m_uiImpactTime;
    uint        m_uiImpactEndTime;
    uint        m_uiStartTime;

    float       m_fDamage;
    float       m_fRearAttackMultiplier;
    int         m_iDamageFlags;
    float       m_fStaminaCost;
    bool        m_bStaminaRequired;
    CVec3f      m_v3Push;
    CVec3f      m_v3Lunge;
    float       m_fHealthLeach;

    float       m_fMin[NUM_MELEE_METRICS];
    float       m_fMax[NUM_MELEE_METRICS];
    float       m_fStep[NUM_MELEE_METRICS];

    float       m_fPivotHeight;
    float       m_fPivotFactor;

public:
    ~CMeleeAttackEvent()    {}
    GAME_SHARED_API CMeleeAttackEvent();

    GAME_SHARED_API void    Clear();

    void                    SetActive()                                             { m_bActive = true; }
    void                    SetInactive()                                           { m_bActive = false; }
    bool                    IsActive() const                                        { return m_bActive; }

    IMeleeItem*             GetWeaponPointer() const                                { return m_pWeapon; }
    tstring                 GetAnimName() const                                     { return m_sAnimName; }
    uint                    GetLength() const                                       { return m_uiAttackTime; }
    float                   GetDamage()                                             { return m_fDamage; }
    float                   GetRearAttackMultiplier() const                         { return m_fRearAttackMultiplier; }
    int                     GetDamageFlags() const                                  { return m_iDamageFlags; }
    uint                    GetImpactTime() const                                   { return m_uiImpactTime; }
    uint                    GetImpactEndTime() const                                { return m_uiImpactEndTime; }
    uint                    GetImmobileTime() const                                 { return m_uiImmobileTime; }
    const svector&          GetStateVector() const                                  { return m_vStates; }
    const uivector&         GetStateDurationVector() const                          { return m_vStateDurations; }
    float                   GetStaminaCost() const                                  { return m_fStaminaCost; }
    CVec3f                  GetPush() const                                         { return m_v3Push; }
    CVec3f                  GetLunge() const                                        { return m_v3Lunge; }
    float                   GetHealthLeach() const                                  { return m_fHealthLeach; }
    bool                    GetStaminaRequired() const                              { return m_bStaminaRequired; }
    uint                    GetStartTime() const                                    { return m_uiStartTime; }

    void                    SetWeaponPointer(IMeleeItem *pWeapon)                   { m_pWeapon = pWeapon; }
    void                    SetAnim(const tstring &sName, uint uiLength)            { m_sAnimName = sName; m_uiAttackTime = uiLength; }
    void                    SetDamage(float fDamage)                                { m_rangeDamage.Set(fDamage, fDamage); m_fDamage = fDamage; }
    void                    SetDamage(float fMin, float fMax)                       { m_rangeDamage.Set(fMin, fMax); m_fDamage = m_rangeDamage; }
    void                    SetRearAttackMultiplier(float fMult)                    { m_fRearAttackMultiplier = fMult; }
    void                    SetImpactTime(uint uiImpactTime)                        { m_uiImpactTime = uiImpactTime; }
    void                    SetImpactEndTime(uint uiImpactEndTime)                  { m_uiImpactEndTime = uiImpactEndTime; }
    void                    SetImmobileTime(uint uiImmobileTime)                    { m_uiImmobileTime = uiImmobileTime; }
    void                    ClearDamageFlags()                                      { m_iDamageFlags = 0; }
    void                    SetDamageFlags(int iFlags)                              { m_iDamageFlags = iFlags; }
    void                    AddDamageFlag(int iFlag)                                { m_iDamageFlags |= iFlag; }
    void                    RemoveDamageFlag(int iFlag)                             { m_iDamageFlags &= ~iFlag; }
    void                    AddState(const tstring &sName, uint uiDuration)         { m_vStates.push_back(sName); m_vStateDurations.push_back(uiDuration); }
    void                    SetStaminaCost(float fCost)                             { m_fStaminaCost = fCost; }
    void                    SetPush(const CVec3f &v3Push)                           { m_v3Push = v3Push; }
    void                    SetLunge(const CVec3f &v3Lunge)                         { m_v3Lunge = v3Lunge; }
    void                    SetHealthLeach(float fLeach)                            { m_fHealthLeach = fLeach; }
    void                    SetStaminaRequired(bool bValue)                         { m_bStaminaRequired = bValue; }
    void                    SetStartTime(uint uiValue)                              { m_uiStartTime = uiValue; }

    GAME_SHARED_API void    SetMetric(EMeleeMetric eMetric, float fMin, float fMax, float fStep);
    void                    SetPivot(float fHeight, float fFactor)                  { m_fPivotHeight = fHeight; m_fPivotFactor = fFactor; }
    float                   GetMin(EMeleeMetric eMetric) const                      { return m_fMin[eMetric]; }
    float                   GetMax(EMeleeMetric eMetric) const                      { return m_fMax[eMetric]; }
    float                   GetStep(EMeleeMetric eMetric) const                     { return m_fStep[eMetric]; }

    void                    Push(const CVec3f &v3Angles, ICombatEntity *pTarget);
    bool                    TryImpact(uint uiTime);
    
    GAME_SHARED_API CVec3f  GetCenter(const CVec3f &v3Center, const CAxis &axis, float fHeight) const;
    GAME_SHARED_API CVec3f  GetDir(const CAxis &axis, float fAngle) const;
};
//=============================================================================

#endif // __C_MELEEATTACKEVENT_H__
