// (C)2006 S2 Games
// c_statenpcabiliy.h
//
//=============================================================================
#ifndef __C_STATENPCABILITY_H__
#define __C_STATENPCABILITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
const uint STATE_DEBUFF     (BIT(0));
const uint STATE_DISPLAY    (BIT(1));
//=============================================================================

//=============================================================================
// CStateNpcAbility
//=============================================================================
class CStateNpcAbility : public IEntityState
{
private:
    static vector<SDataField>   *s_pvFields;

    DECLARE_ENT_ALLOCATOR(State, NpcAbility);

protected:
    byte        m_yNpcStateFlags;
    ResHandle   m_hIcon;
    ResHandle   m_hEffect;
    ResHandle   m_hModel;
    tstring     m_sAnimName;
    tstring     m_sSkin;
    bool        m_bStun;
    bool        m_bStack;
    
    ushort      m_unNpcAbilityEffectID;

public:
    virtual ~CStateNpcAbility() {}
    CStateNpcAbility();

    // Network
    GAME_SHARED_API virtual void    Baseline();
    GAME_SHARED_API virtual void    GetSnapshot(CEntitySnapshot &snapshot) const;
    GAME_SHARED_API virtual bool    ReadSnapshot(CEntitySnapshot &snapshot);

    static const vector<SDataField>&    GetTypeVector();

    void    Activated();
    void    Expired();
    
    void    SetNpcStateFlags(byte yFlags)               { m_yNpcStateFlags = yFlags; }
    void    SetIcon(ResHandle hIcon)                    { m_hIcon = hIcon; }
    void    SetEffect(ResHandle hEffect)                { m_hEffect = hEffect; }
    void    SetModel(ResHandle hModel)                  { m_hModel = hModel; }
    void    SetAnimName(const tstring &sAnimName)       { m_sAnimName = sAnimName; }
    void    SetSkin(const tstring &sSkin)               { m_sSkin = sSkin; }
    void    SetStun(bool bStun)                         { m_bStun = bStun; }
    void    SetStack(bool bStack)                       { m_bStack = bStack; }
    void    SetNpcAbilityEffectID(ushort unID)          { m_unNpcAbilityEffectID = unID; }

    virtual bool    IsMatch(ushort unType);

    // Overridden cvar accessors
    virtual tstring     GetIconPath() const;
    virtual tstring     GetEffectPath() const;
    virtual tstring     GetAnimName() const;
    virtual bool        GetIsDebuff() const;
    virtual bool        GetDisplayState() const;
    virtual tstring     GetSkin() const;
    virtual tstring     GetModelPath() const;
    virtual bool        GetStack() const;
};
//=============================================================================

#endif //__C_STATENPCABILITY_H__
