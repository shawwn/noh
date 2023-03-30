// (C)2007 S2 Games
// i_skilltoggle.h
//
//=============================================================================
#ifndef __I_SKILLTOGGLE_H__
#define __I_SKILLTOGGLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// ISkillToggle
//=============================================================================
class ISkillToggle : public ISkillItem
{
protected:
    START_ENTITY_CONFIG(ISkillItem)
        DECLARE_ENTITY_CVAR(tstring, EndAnimName)
        DECLARE_ENTITY_CVAR(uint, FinishTime)
        DECLARE_ENTITY_CVAR(tstring, SelfState)
        DECLARE_ENTITY_CVAR(tstring, RadiusState)
        DECLARE_ENTITY_CVAR(bool, TargetStatusLiving)
        DECLARE_ENTITY_CVAR(bool, TargetStatusDead)
        DECLARE_ENTITY_CVAR(bool, TargetStatusCorpse)
        DECLARE_ENTITY_CVAR(bool, TargetTeamAlly)
        DECLARE_ENTITY_CVAR(bool, TargetTeamEnemy)
        DECLARE_ENTITY_CVAR(bool, TargetTypePlayer)
        DECLARE_ENTITY_CVAR(bool, TargetTypeVehicle)
        DECLARE_ENTITY_CVAR(bool, TargetTypeBuilding)
        DECLARE_ENTITY_CVAR(bool, TargetTypeGadget)
        DECLARE_ENTITY_CVAR(bool, TargetTypeHellbourne)
        DECLARE_ENTITY_CVAR(bool, TargetTypePet)
        DECLARE_ENTITY_CVAR(bool, TargetTypeNPC)
        DECLARE_ENTITY_CVAR(bool, TargetTypeSiege)
        DECLARE_ENTITY_CVAR(float, Radius)
        DECLARE_ENTITY_CVAR(float, ManaCostPerSecond)
        DECLARE_ENTITY_CVAR(uint, MaxTime)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    bool    m_bActive;
    int     m_iSelfStateSlot;
    uint    m_uiActivationTime;

public:
    virtual ~ISkillToggle() {}
    ISkillToggle(CEntityConfig *pConfig) :
    ISkillItem(pConfig),
    m_pEntityConfig(pConfig),
    m_bActive(false),
    m_iSelfStateSlot(-1)
    {}

    bool            IsToggleSkill() const   { return true; }

    virtual void    ActiveFrame();

    virtual bool    IsValidTarget(IGameEntity *pEnt, bool bImpact);

    virtual void    Activate()  { ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }
    virtual void    Impact();
    virtual void    Deactivate();
    virtual bool    ActivatePrimary(int iButtonStatus);

    static void     ClientPrecache(CEntityConfig *pConfig);
    static void     ServerPrecache(CEntityConfig *pConfig);

    // Settings
    ENTITY_CVAR_ACCESSOR(tstring, EndAnimName, _T(""))
    ENTITY_CVAR_ACCESSOR(uint, FinishTime, 0)
    ENTITY_CVAR_ACCESSOR(tstring, SelfState, _T(""))
    ENTITY_CVAR_ACCESSOR(tstring, RadiusState, _T(""))
    ENTITY_CVAR_ACCESSOR(float, Radius, 0.0f)
    ENTITY_CVAR_ACCESSOR(float, ManaCostPerSecond, 0.0f)

    TYPE_NAME("Toggle ability")
};
//=============================================================================

#endif //__I_SKILLTOGGLE_H__
