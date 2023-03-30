// (C)2006 S2 Games
// i_game.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_game.h"
#include "c_teaminfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
IGame*  IGame::s_pGame(NULL);
//=============================================================================


/*====================
  IGame::SetTeam
  ====================*/
void    IGame::SetTeam(uint ui, CEntityTeamInfo *pTeam)
{
    while (m_vTeams.size() <= ui)
    {
        m_vTeams.resize(m_vTeams.size() + 1);
        m_vTeams[m_vTeams.size() - 1] = NULL;
    }

    m_vTeams[ui] = pTeam;
}


/*====================
  IGame::ClearTeams
  ====================*/
void    IGame::ClearTeams()
{
    m_vTeams.clear();
}


/*====================
  IGame::GetTeamFromIndex
  ====================*/
CEntityTeamInfo*    IGame::GetTeamFromIndex(uint uiIndex)
{
    for (vector<CEntityTeamInfo*>::iterator it(m_vTeams.begin()); it != m_vTeams.end(); ++it)
    {
        if ((*it)->GetIndex() == uiIndex)
            return *it;
    }

    return NULL;
}


/*====================
  IGame::TraceLine
  ====================*/
bool    IGame::TraceLine(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, int iIgnoreSurface, uint uiIgnoreEntity)
{
    bool bHit = m_pWorld->TraceLine(result, v3Start, v3End, iIgnoreSurface, uiIgnoreEntity);
    return bHit;
}

/*====================
  IGame::TraceBox
  ====================*/
bool    IGame::TraceBox(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, int iIgnoreSurface, uint uiIgnoreEntity)
{
    bool bHit = m_pWorld->TraceBox(result, v3Start, v3End, bbBounds, iIgnoreSurface, uiIgnoreEntity);
    return bHit;
}
