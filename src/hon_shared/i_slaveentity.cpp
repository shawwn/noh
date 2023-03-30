// (C)2008 S2 Games
// i_slaveentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_slaveentity.h"

#include "i_unitentity.h"
#include "c_player.h"

#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENTITY_DESC(ISlaveEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    const TypeVector &vBase(IGameEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiOwnerIndex"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLevel"), TYPE_INT, 5, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yCharges"), TYPE_CHAR, 7, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yAttackModPriority"), TYPE_CHAR, 1, 0));
}
//=============================================================================

/*====================
  ISlaveEntity::ISlaveEntity
  ====================*/
ISlaveEntity::ISlaveEntity() :
IGameEntity(NULL),

m_uiOwnerIndex(INVALID_INDEX),
m_ySlot(INVALID_SLOT),
m_uiLevel(0),
m_yCharges(0),
m_fAccumulator(0.0f),
m_yAttackModPriority(0),
m_uiFadeStartTime(INVALID_TIME),
m_uiProxyUID(INVALID_INDEX),
m_uiSpawnerUID(INVALID_INDEX)
{
}


/*====================
  ISlaveEntity::Baseline
  ====================*/
void    ISlaveEntity::Baseline()
{
    IGameEntity::Baseline();

    m_uiOwnerIndex = INVALID_INDEX;
    m_uiLevel = 0;
    m_yCharges = 0;
    m_yAttackModPriority = 0;
}


/*====================
  ISlaveEntity::GetSnapshot
  ====================*/
void    ISlaveEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IGameEntity::GetSnapshot(snapshot, uiFlags);

    snapshot.WriteGameIndex(m_uiOwnerIndex);
    snapshot.WriteField(m_uiLevel);
    snapshot.WriteField(m_yCharges);
    snapshot.WriteField(m_yAttackModPriority);
}


/*====================
  ISlaveEntity::ReadSnapshot
  ====================*/
bool    ISlaveEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        IGameEntity::ReadSnapshot(snapshot, 1);

        snapshot.ReadGameIndex(m_uiOwnerIndex);
        snapshot.ReadField(m_uiLevel);
        snapshot.ReadField(m_yCharges);
        snapshot.ReadField(m_yAttackModPriority);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("ISlaveEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  ISlaveEntity::GetPrivateClient
  ====================*/
int     ISlaveEntity::GetPrivateClient()
{
    IUnitEntity *pUnit(GetOwner());
    if (pUnit == NULL)
        return -1;

    return pUnit->GetOwnerClientNumber();
}


/*====================
  ISlaveEntity::Spawn
  ====================*/
void    ISlaveEntity::Spawn()
{
    if (Game.IsServer())
    {
        // Spawn action
        ISlaveDefinition *pDefinition(GetDefinition<ISlaveDefinition>());
        if (pDefinition != NULL)
            pDefinition->ExecuteActionScript(ACTION_SCRIPT_SPAWN, this, GetOwner(), this, GetOwner(), GetOwner()->GetPosition(), GetProxy(0), GetLevel());
    }
}


/*====================
  ISlaveEntity::ClientPrecache
  ====================*/
void    ISlaveEntity::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IGameEntity::ClientPrecache(pConfig, eScheme, sModifier);
}


/*====================
  ISlaveEntity::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void    ISlaveEntity::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IGameEntity::ServerPrecache(pConfig, eScheme, sModifier);
}


/*====================
  ISlaveEntity::UpdateModifiers
  ====================*/
void    ISlaveEntity::UpdateModifiers(const uivector &vModifiers)
{
    m_vModifierKeys = vModifiers;

    uint uiModifierBits(GetModifierBits(vModifiers));
    if (m_uiActiveModifierKey != INVALID_INDEX)
        uiModifierBits |= GetModifierBit(m_uiActiveModifierKey);

    // Activate conditional modifiers
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return;

    // Grab base definition
    IEntityDefinition *pDefinition(GetBaseDefinition<IEntityDefinition>());
    if (pDefinition == NULL)
        return;

    const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
    for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
    {
        if (cit->second->GetExclusive())
            continue;

        const tstring &sCondition(cit->second->GetCondition());
        if (sCondition.empty())
            continue;

        tsvector vsTypes(TokenizeString(sCondition, _T(' ')));

        tsvector_cit itType(vsTypes.begin()), itTypeEnd(vsTypes.end());
        for (; itType != itTypeEnd; ++itType)
        {
            if (!itType->empty() && (*itType)[0] == _T('!'))
            {
                if (pOwner->IsTargetType(itType->substr(1), pOwner))
                    break;
            }
            else
            {
                if (!pOwner->IsTargetType(*itType, pOwner))
                    break;
            }
        }
        if (itType == itTypeEnd)
            uiModifierBits |= cit->first;
    }

    SetModifierBits(uiModifierBits);
}


/*====================
  ISlaveEntity::GetEffectDescription
  ====================*/
const tstring&  ISlaveEntity::GetEffectDescription(EEntityActionScript eAction)
{
    if (m_pDefinition == NULL)
        return TSNULL;

    return static_cast<ISlaveDefinition *>(m_pDefinition)->GetEffectDescription(eAction);
}


/*====================
  ISlaveEntity::GetEffectDescriptionIndex
  ====================*/
uint    ISlaveEntity::GetEffectDescriptionIndex(EEntityActionScript eAction)
{
    if (m_pDefinition == NULL)
        return INVALID_INDEX;

    return static_cast<ISlaveDefinition *>(m_pDefinition)->GetEffectDescriptionIndex(eAction);
}


/*====================
  ISlaveEntity::AddTimedCharges
  ====================*/
void    ISlaveEntity::AddTimedCharges(int iCharges, uint uiExpireTime)
{
    for (; iCharges > 0; --iCharges)
    {
        ++m_yCharges;
        m_vTimedCharges.push_back(uiExpireTime);
    }

    int iMaxCharges(GetMaxCharges());

    // Expire the old timers if we're over the limit
    if (iMaxCharges != -1)
    {
        while (m_yCharges > iMaxCharges)
        {
            uivector_it it(m_vTimedCharges.begin());
            uivector_it itOldest(m_vTimedCharges.begin());

            for (uivector_it it(m_vTimedCharges.begin()), itEnd(m_vTimedCharges.end()); it != itEnd; ++it)
            {
                if (*it < *itOldest)
                    itOldest = it;
            }

            if (itOldest != m_vTimedCharges.end())
                m_vTimedCharges.erase(itOldest);

            --m_yCharges;
        }
    }
}


/*====================
  ISlaveEntity::ServerFrameAction
  ====================*/
bool    ISlaveEntity::ServerFrameAction()
{
    if (m_pDefinition != NULL)
        static_cast<ISlaveDefinition *>(m_pDefinition)->ExecuteActionScript(ACTION_SCRIPT_FRAME, this, GetOwner(), this, GetOwner(), GetOwner()->GetPosition(), GetProxy(0), GetLevel());

    return IGameEntity::ServerFrameAction();
}


/*====================
  ISlaveEntity::ServerFrameCleanup
  ====================*/
bool    ISlaveEntity::ServerFrameCleanup()
{
    uivector_it it(m_vTimedCharges.begin());
    while (it != m_vTimedCharges.end())
    {
        if (*it <= Game.GetGameTime())
        {
            it = m_vTimedCharges.erase(it);
            RemoveCharge();
        }
        else
        {
            ++it;
        }
    }

    return IGameEntity::ServerFrameCleanup();
}


/*====================
  ISlaveEntity::GetStealthFade
  ====================*/
float   ISlaveEntity::GetStealthFade()
{
    if (!IsActive() || GetStealthType() == 0)
        return 0.0f;
    else if (m_uiFadeStartTime == INVALID_TIME)
        return 1.0f;
    else if (Game.GetGameTime() < m_uiFadeStartTime)
        return 0.0f;
    else if (GetFadeTime() == 0)
        return 1.0f;
    else
        return CLAMP((Game.GetGameTime() - m_uiFadeStartTime) / float(GetFadeTime()), 0.0f, 1.0f);
}

