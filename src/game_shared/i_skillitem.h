// (C)2006 S2 Games
// i_skillitem.h
//
//=============================================================================
#ifndef __I_SKILLITEM_H__
#define __I_SKILLITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_inventoryitem.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class ISkillToggle;
//=============================================================================

//=============================================================================
// ISkillItem
//=============================================================================
class ISkillItem : public IInventoryItem
{
protected:
    START_ENTITY_CONFIG(IInventoryItem)
        DECLARE_ENTITY_CVAR(tstring, AnimName)
        DECLARE_ENTITY_CVAR(bool, CanUseWithMelee)
        DECLARE_ENTITY_CVAR(bool, CanUseWithRanged)
        DECLARE_ENTITY_CVAR(uint, Duration)
        DECLARE_ENTITY_CVAR(uint, ActivationTime)
        DECLARE_ENTITY_CVAR(bool, Freeze)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

public:
    virtual ~ISkillItem()   {}
    ISkillItem(CEntityConfig *pConfig) :
    IInventoryItem(pConfig),
    m_pEntityConfig(pConfig)
    {}

    bool            IsSkill() const                     { return true; }
    virtual bool    IsMeleeSkill() const                { return false; }
    virtual bool    IsDeploySkill() const               { return false; }
    virtual bool    IsBuffSkill() const                 { return false; }
    virtual bool    IsToggleSkill() const               { return false; }

    virtual bool    ActivatePrimary(int iButtonStatus);
    virtual void    Impact()                            {}

    GAME_SHARED_API const ISkillToggle*     GetAsSkillToggle() const;
    GAME_SHARED_API ISkillToggle*           GetAsSkillToggle();
    

    // Settings
    ENTITY_CVAR_ACCESSOR(tstring, AnimName, _T(""))
    ENTITY_CVAR_ACCESSOR(bool, CanUseWithMelee, true)
    ENTITY_CVAR_ACCESSOR(bool, CanUseWithRanged, false)
    ENTITY_CVAR_ACCESSOR(uint, Duration, 2000)
    ENTITY_CVAR_ACCESSOR(uint, ActivationTime, 0)
    ENTITY_CVAR_ACCESSOR(bool, Freeze, true)

    TYPE_NAME("Skill")
};
//=============================================================================

#endif //__I_SKILLITEM_H__
