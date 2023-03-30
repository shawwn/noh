// (C)2007 S2 Games
// c_entitygameinfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitygameinfo.h"

#include "../k2/c_snapshot.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>* CEntityGameInfo::s_pvFields;

DEFINE_ENT_ALLOCATOR(Entity, GameInfo)
//=============================================================================

/*====================
  CEntityGameInfo::CEntityGameInfo
  ====================*/
CEntityGameInfo::CEntityGameInfo() :
IGameEntity(NULL),

m_uiGamePhase(GAME_PHASE_INVALID),
m_uiPhaseStartTime(INVALID_TIME),
m_uiPhaseDuration(INVALID_TIME),
m_iGameMatchID(-1),
m_bSuddenDeathActive(false)
{
}


/*====================
  CEntityGameInfo::GetTypeVector
  ====================*/
const vector<SDataField>&   CEntityGameInfo::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IGameEntity::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
        s_pvFields->push_back(SDataField(_T("m_uiGamePhase"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiPhaseStartTime"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiPhaseDuration"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_iGameMatchID"), FIELD_PUBLIC, TYPE_INT));
    }

    return *s_pvFields;
}


/*====================
  CEntityGameInfo::GetSnapshot
  ====================*/
void    CEntityGameInfo::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IGameEntity::GetSnapshot(snapshot);

    snapshot.AddField(m_uiGamePhase | (m_bSuddenDeathActive ? BIT(31) : 0));
    snapshot.AddField(m_uiPhaseStartTime);
    snapshot.AddField(m_uiPhaseDuration);
    snapshot.AddField(m_iGameMatchID);
}


/*====================
  CEntityGameInfo::ReadSnapshot
  ====================*/
bool    CEntityGameInfo::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        IGameEntity::ReadSnapshot(snapshot);

        snapshot.ReadNextField(m_uiGamePhase);
        m_bSuddenDeathActive = ((m_uiGamePhase & BIT(31)) != 0);
        snapshot.ReadNextField(m_uiPhaseStartTime);
        snapshot.ReadNextField(m_uiPhaseDuration);
        snapshot.ReadNextField(m_iGameMatchID);
        
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityGameInfo::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CEntityGameInfo::Baseline
  ====================*/
void    CEntityGameInfo::Baseline()
{
    IGameEntity::Baseline();

    m_uiGamePhase = GAME_PHASE_INVALID;
    m_uiPhaseStartTime = INVALID_TIME;
    m_uiPhaseDuration = INVALID_TIME;
    m_iGameMatchID = -1;
    m_bSuddenDeathActive = false;
}
