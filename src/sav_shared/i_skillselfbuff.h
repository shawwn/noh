// (C)2006 S2 Games
// i_skillselfbuff.h
//
//=============================================================================
#ifndef __I_SKILLSELFBUFF_H__
#define __I_SKILLSELFBUFF_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// ISkillSelfBuff
//=============================================================================
class ISkillSelfBuff : public ISkillItem
{
protected:
    START_ENTITY_CONFIG(ISkillItem)
        DECLARE_ENTITY_CVAR(tstring, SelfState)
        DECLARE_ENTITY_CVAR(uint, SelfStateDuration)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

public:
    virtual ~ISkillSelfBuff()   {}
    ISkillSelfBuff(CEntityConfig *pConfig) :
    ISkillItem(pConfig),
    m_pEntityConfig(pConfig)
    {}

    bool            IsBuffSkill() const { return true; }

    virtual void    Activate()  { ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }
    virtual void    Impact();

    static void     ClientPrecache(CEntityConfig *pConfig);
    static void     ServerPrecache(CEntityConfig *pConfig);

    // Settings
    ENTITY_CVAR_ACCESSOR(tstring, SelfState, _T(""))
    ENTITY_CVAR_ACCESSOR(uint, SelfStateDuration, 0)

    TYPE_NAME("Self buff")
};
//=============================================================================

#endif //__I_SKILLSELFBUFF_H__
