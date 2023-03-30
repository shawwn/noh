// (C)2006 S2 Games
// c_statenpcability.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statenpcability.h"
#include "c_npcability.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR(State, NpcAbility);

vector<SDataField>* CStateNpcAbility::s_pvFields;

extern const CNpcAbilityEffect  *g_pNpcAbilityEffect;
//=============================================================================


/*====================
  CStateNpcAbility::CStateNpcAbility
  ====================*/
CStateNpcAbility::CStateNpcAbility() :
IEntityState(NULL),

m_yNpcStateFlags(0),
m_hIcon(INVALID_RESOURCE),
m_hEffect(INVALID_RESOURCE),
m_hModel(INVALID_RESOURCE),
m_bStun(false),
m_bStack(false),

m_unNpcAbilityEffectID(-1)
{
}


/*====================
  CStateNpcAbility::GetTypeVector
  ====================*/
const vector<SDataField>&   CStateNpcAbility::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IEntityState::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
        
        s_pvFields->push_back(SDataField(_T("m_yNpcStateFlags"), FIELD_PUBLIC, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_hIcon"), FIELD_PUBLIC, TYPE_RESHANDLE));
    }

    return *s_pvFields;
}


/*====================
  CStateNpcAbility::GetSnapshot
  ====================*/
void    CStateNpcAbility::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IEntityState::GetSnapshot(snapshot);

    snapshot.AddField(m_yNpcStateFlags);
    snapshot.AddResHandle(m_hIcon);
}


/*====================
  CStateNpcAbility::ReadSnapshot
  ====================*/
bool    CStateNpcAbility::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        if (!IEntityState::ReadSnapshot(snapshot))
            return false;

        snapshot.ReadNextField(m_yNpcStateFlags);
        snapshot.ReadNextResHandle(m_hIcon);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CStateNpcAbility::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CStateNpcAbility::Baseline
  ====================*/
void    CStateNpcAbility::Baseline()
{
    IEntityState::Baseline();

    m_yNpcStateFlags = (STATE_DEBUFF | STATE_DISPLAY);
    m_hIcon = INVALID_RESOURCE;
}


/*====================
  CStateNpcAbility::Activated
  ====================*/
void    CStateNpcAbility::Activated()
{
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner == NULL)
        return;

    if (g_pNpcAbilityEffect)
        g_pNpcAbilityEffect->Activated(this);
    
    IEntityState::Activated();

    if (m_bStun)
    {
        if (pOwner->GetAsCombatEnt())
            pOwner->GetAsCombatEnt()->SetAction(PLAYER_ACTION_STUNNED | PLAYER_ACTION_IMMOBILE, GetExpireTime());
    }
}


/*====================
  CStateNpcAbility::Expired
  ====================*/
void    CStateNpcAbility::Expired()
{
    IEntityState::Expired();
}


/*====================
  CStateNpcAbility::GetIconPath
  ====================*/
tstring     CStateNpcAbility::GetIconPath() const
{
    if (m_hIcon != INVALID_RESOURCE)
        return g_ResourceManager.GetPath(m_hIcon);
    else
        return _T("");
}


/*====================
  CStateNpcAbility::GetEffectPath
  ====================*/
tstring     CStateNpcAbility::GetEffectPath() const
{
    if (m_hEffect != INVALID_RESOURCE)
        return g_ResourceManager.GetPath(m_hEffect);
    else
        return _T("");
}


/*====================
  CStateNpcAbility::GetAnimName
  ====================*/
tstring     CStateNpcAbility::GetAnimName() const
{
    return m_sAnimName;
}


/*====================
  CStateNpcAbility::GetIsDebuff
  ====================*/
bool        CStateNpcAbility::GetIsDebuff() const
{
    return (m_yNpcStateFlags & STATE_DEBUFF) != 0;
}


/*====================
  CStateNpcAbility::GetDisplayState
  ====================*/
bool        CStateNpcAbility::GetDisplayState() const
{
    return (m_yNpcStateFlags & STATE_DISPLAY) != 0;
}


/*====================
  CStateNpcAbility::GetSkin
  ====================*/
tstring     CStateNpcAbility::GetSkin() const
{
    return m_sSkin;
}


/*====================
  CStateNpcAbility::GetModelPath
  ====================*/
tstring     CStateNpcAbility::GetModelPath() const
{
    if (m_hModel != INVALID_RESOURCE)
        return g_ResourceManager.GetPath(m_hModel);
    else
        return _T("");
}


/*====================
  CStateNpcAbility::GetStack
  ====================*/
bool        CStateNpcAbility::GetStack() const
{
    return m_bStack;
}


/*====================
  CStateNpcAbility::IsMatch
  ====================*/
bool    CStateNpcAbility::IsMatch(ushort unType)
{
    return m_unType == unType && m_unNpcAbilityEffectID == g_pNpcAbilityEffect->GetID();
}
