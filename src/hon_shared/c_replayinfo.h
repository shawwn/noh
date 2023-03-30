// (C)2010 S2 Games
// c_replayinfo.h
//
//=============================================================================
#ifndef __C_REPLAYINFO_H__
#define __C_REPLAYINFO_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_xmlnode.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// CReplayInfo
//=============================================================================
class CReplayInfo
{
private:
    typedef vector<CXMLNode>    PlayerInfoVec;

    CXMLNode            m_cGameInfoProperties;
    PlayerInfoVec       m_vPlayerInfo;
    uint                m_uiNumPlayers;

public:
    ~CReplayInfo() {}
    CReplayInfo() {}

    void                Reset();
    void                SetReplayGameInfo(const CXMLNode& cNode);
    void                SetReplayPlayerInfo(const CXMLNode& cNode);

    const CXMLNode&     GetGameInfo() const                         { return m_cGameInfoProperties; }
    const CXMLNode&     GetPlayerInfo(uint uiPlayerIdx) const
    {
        if (uiPlayerIdx >= (uint)m_vPlayerInfo.size())
            return g_NullXMLNode;

        return m_vPlayerInfo[uiPlayerIdx];
    }

    bool                HasPlayer(uint uiPlayerIdx) const
    {
        if (uiPlayerIdx >= 10)
            return false;

        return (m_vPlayerInfo[uiPlayerIdx].GetName() == TSNULL);
    }
};
//=============================================================================

#endif //__C_REPLAYINFO_H__
