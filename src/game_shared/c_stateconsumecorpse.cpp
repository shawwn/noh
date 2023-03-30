// (C)2006 S2 Games
// c_stateconsumecorpse.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateconsumecorpse.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, ConsumeCorpse);

vector<SDataField>* CStateConsumeCorpse::s_pvFields;
//=============================================================================

/*====================
  CStateConsumeCorpse::CEntityConfig::CEntityConfig
  ====================*/
CStateConsumeCorpse::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(MaxCorpses, 3),
INIT_ENTITY_CVAR(CorpseDecayTime, 15000),
INIT_ENTITY_CVAR(CorpseMultScale, 0.15f),
INIT_ENTITY_CVAR(CorpseMultDamage, 0.15f),
INIT_ENTITY_CVAR(CorpseAddDamage, 0.0f),
INIT_ENTITY_CVAR(CorpseMultHealth, 0.15f),
INIT_ENTITY_CVAR(CorpseAddHealth, 0.0f),
INIT_ENTITY_CVAR(CorpseMultHealthRegen, 0.15f),
INIT_ENTITY_CVAR(CorpseAddHealthRegen, 0.0f)
{
}

/*====================
  CStateConsumeCorpse::StateFrame
  ====================*/
void    CStateConsumeCorpse::StateFrame()
{
    // Lerp between old and new scale
    m_modScale.Set(0.0f, LERP(Game.GetFrameLength() / 1000.0f, m_modScale.GetMult(), 1.0f + (GetCorpseMultScale() * m_fCorpsesEaten)), 0.0f);

    // Use current values for damage, health and health regen
    m_modDamage.Set(GetCorpseAddDamage() * m_fCorpsesEaten, 1.0f + (GetCorpseMultDamage() * m_fCorpsesEaten), 0.0f);
    m_modHealth.Set(GetCorpseAddHealth() * m_fCorpsesEaten, 1.0f + (GetCorpseMultHealth() * m_fCorpsesEaten), 0.0f);
    m_modHealthRegen.Set(GetCorpseAddHealthRegen() * m_fCorpsesEaten, 1.0f + (GetCorpseMultHealthRegen() * m_fCorpsesEaten), 0.0f);

    if (GetCorpseDecayTime() > 0 && m_fCorpsesEaten > 0.0f)
    {
        m_fCorpsesEaten -= (Game.GetFrameLength() / float(GetCorpseDecayTime()));
        m_fCorpsesEaten = MAX(m_fCorpsesEaten, 0.0f);
    }
}

/*====================
  CStateConsumeCorpse::GetTypeVector
  ====================*/
const vector<SDataField>&   CStateConsumeCorpse::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IEntityState::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

        s_pvFields->push_back(SDataField(_T("m_fCorpsesEaten"), FIELD_PUBLIC, TYPE_FLOAT, 17));
    }

    return *s_pvFields;
}


/*====================
  CStateConsumeCorpse::GetSnapshot
  ====================*/
void    CStateConsumeCorpse::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IEntityState::GetSnapshot(snapshot);

    snapshot.AddField(m_fCorpsesEaten);
}


/*====================
  CStateConsumeCorpse::ReadSnapshot
  ====================*/
bool    CStateConsumeCorpse::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        IEntityState::ReadSnapshot(snapshot);

        snapshot.ReadNextField(m_fCorpsesEaten);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CStateConsumeCorpse::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CStateConsumeCorpse::Baseline
  ====================*/
void    CStateConsumeCorpse::Baseline()
{
    IEntityState::Baseline();

    m_fCorpsesEaten = 0.0f;
}
