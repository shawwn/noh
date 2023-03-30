// (C)2006 S2 Games
// c_statepersistantreplenish.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statepersistantreplenish.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, PersistantReplenish);

vector<SDataField>* CStatePersistantReplenish::s_pvFields;
//=============================================================================


/*====================
  CStatePersistantReplenish::CEntityConfig::CEntityConfig
  ====================*/
CStatePersistantReplenish::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStatePersistantReplenish::CStatePersistantReplenish
  ====================*/
CStatePersistantReplenish::CStatePersistantReplenish() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),
m_unItemData(-1),
m_uiRegenMod(PERSISTANT_REGEN_NULL),
m_uiIncreaseMod(PERSISTANT_INCREASE_NULL),
m_uiReplenishMod(PERSISTANT_REPLENISH_NULL),
m_uiPersistantType(PERSISTANT_TYPE_NULL)
{
}


/*====================
  CStatePersistantReplenish::GetTypeVector
  ====================*/
const vector<SDataField>&   CStatePersistantReplenish::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IEntityState::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
        
        s_pvFields->push_back(SDataField(_T("m_unItemData"), FIELD_PUBLIC, TYPE_SHORT));
    }

    return *s_pvFields;
}


/*====================
  CStatePersistantReplenish::GetSnapshot
  ====================*/
void    CStatePersistantReplenish::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IEntityState::GetSnapshot(snapshot);

    snapshot.AddField(m_unItemData);
}


/*====================
  CStatePersistantReplenish::ReadSnapshot
  ====================*/
bool    CStatePersistantReplenish::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        if (!IEntityState::ReadSnapshot(snapshot))
            return false;

        snapshot.ReadNextField(m_unItemData);

        UpdateItemData();

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CStatePersistantReplenish::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CStatePersistantReplenish::Baseline
  ====================*/
void    CStatePersistantReplenish::Baseline()
{
    IEntityState::Baseline();

    m_unItemData = -1;
}


/*====================
  CStatePersistantReplenish::UpdateItemData
  ====================*/
void    CStatePersistantReplenish::UpdateItemData()
{
    if (m_unItemData == (ushort)-1)
    {
        m_uiPersistantType = 0;
        m_uiRegenMod = 0;
        m_uiIncreaseMod = 0;
        m_uiReplenishMod = 0;
        return;
    }

    m_uiPersistantType = (m_unItemData / 1000) % 10;
    m_uiRegenMod = (m_unItemData / 100) % 10;
    m_uiIncreaseMod = (m_unItemData / 10) % 10;
    m_uiReplenishMod = (m_unItemData % 10);

    FloatMod modValue;
    modValue.SetMult(GetMultiplier());

    switch (m_uiReplenishMod)
    {
    case PERSISTANT_REPLENISH_ARMOR:
        SetMod(MOD_ARMOR, modValue);
        break;
    case PERSISTANT_REPLENISH_SPEED:
        SetMod(MOD_SPEED, modValue);
        break;
    case PERSISTANT_REPLENISH_DAMAGE:
        SetMod(MOD_DAMAGE, modValue);
        break;
    }
}


/*====================
  CStatePersistantReplenish::SetItemData
  ====================*/
void    CStatePersistantReplenish::SetItemData(ushort unData)
{
    m_unItemData = unData;
    UpdateItemData();
}
