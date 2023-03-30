// (C)2008 S2 Games
// i_entityabilityattribute.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_entityabilityattribute.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint    IEntityAbilityAttribute::s_uiBaseType(ENTITY_BASE_TYPE_ABILITY_ATTRIBUTE);

DEFINE_ENTITY_DESC(IEntityAbilityAttribute, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game, TypeVector)();
    const TypeVector &vBase(IEntityAbility::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fStrength"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fAgility"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fIntelligence"), TYPE_FLOAT, 0, 0));
}
//=============================================================================

//=============================================================================
// Entity definition
//=============================================================================
START_ENTITY_DEFINITION_XML_PROCESSOR(IEntityAbilityAttribute, AbilityAttribute)
    CAbilityDefinition::ReadSettings(pDefinition, node, bMod);
END_ENTITY_DEFINITION_XML_PROCESSOR(AbilityAttribute, abilityattribute)
//=============================================================================

/*====================
  IEntityAbilityAttribute::IEntityAbilityAttribute
  ====================*/
IEntityAbilityAttribute::IEntityAbilityAttribute() :
m_fStrength(0.0f),
m_fAgility(0.0f),
m_fIntelligence(0.0f)
{
}


/*====================
  IEntityAbilityAttribute::Baseline
  ====================*/
void    IEntityAbilityAttribute::Baseline()
{
    IEntityAbility::Baseline();

    m_fStrength = 0.0f;
    m_fAgility = 0.0f;
    m_fIntelligence = 0.0f;
}


/*====================
  IEntityAbilityAttribute::GetSnapshot
  ====================*/
void    IEntityAbilityAttribute::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IEntityAbility::GetSnapshot(snapshot, uiFlags);

    snapshot.WriteField(m_fStrength);
    snapshot.WriteField(m_fAgility);
    snapshot.WriteField(m_fIntelligence);
}


/*====================
  IEntityAbilityAttribute::ReadSnapshot
  ====================*/
bool    IEntityAbilityAttribute::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        IEntityAbility::ReadSnapshot(snapshot, 1);

        snapshot.ReadField(m_fStrength);
        snapshot.ReadField(m_fAgility);
        snapshot.ReadField(m_fIntelligence);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IEntityAbilityAttribute::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IEntityAbilityAttribute::Spawn
  ====================*/
void    IEntityAbilityAttribute::Spawn()
{
    IEntityAbility::Spawn();

    m_fStrength = 0.0f;
    m_fAgility = 0.0f;
    m_fIntelligence = 0.0f;
}
