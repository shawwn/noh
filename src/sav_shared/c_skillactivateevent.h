// (C)2006 S2 Games
// c_skillactivateevent.h
//
//=============================================================================
#ifndef __C_SKILLACTIVATEEVENT_H__
#define __C_SKILLACTIVATEEVENT_H__

//=============================================================================
// Declarations
//=============================================================================
class ISkillItem;
//=============================================================================

//=============================================================================
// CSkillActivateEvent
//=============================================================================
class CSkillActivateEvent
{
private:
    ICombatEntity*  m_pOwner;
    int             m_iSlot;

    bool            m_bActive;
    uint            m_uiActivateTime;
    bool            m_bActivated;

public:
    ~CSkillActivateEvent()  {}
    CSkillActivateEvent();

    void    Clear();

    void    SetActive()                             { m_bActive = true; }
    void    SetInactive()                           { m_bActive = false; }
    bool    IsActive() const                        { return m_bActive; }

    void    SetOwner(ICombatEntity *pOwner)         { m_pOwner = pOwner; }
    void    SetSlot(int iSlot)                      { m_iSlot = iSlot; }
    void    SetActivateTime(uint uiActivateTime)    { m_uiActivateTime = uiActivateTime; }

    bool    TryImpact();
};
//=============================================================================

#endif //__C_SKILLACTIVATEEVENT_H__
