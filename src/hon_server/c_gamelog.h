// (C)2008 S2 Games
// c_gamelog.h
//
//=============================================================================
#ifndef __C_GAMELOG_H__
#define __C_GAMELOG_H__

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
class IUnitEntity;
//=============================================================================

//=============================================================================
// CGameLog
//=============================================================================
class CGameLog
{
private:
    CFileHandle m_hLogFile;

public:
    ~CGameLog() {}
    CGameLog()  {}

    void    Open(uint uiMatchID = -1);
    void    Close()                     { m_hLogFile.Close(); }

    const tstring&  GetPath() const     { return m_hLogFile.GetPath(); }

    void    WriteInfo(EGameLogEvent eEvent, const tstring &sTagA = TSNULL, const tstring &sValueA = TSNULL, const tstring &sTagB = TSNULL, const tstring &sValueB = TSNULL);
    void    WriteStatus(EGameLogEvent eEvent, const tstring &sTag = TSNULL, const tstring &sValue = TSNULL);
    void    WritePlayer(EGameLogEvent eEvent, CPlayer *pPlayer, const tstring &sParamA = TSNULL, const tstring &sParamB = TSNULL);

    void    WriteKill(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, ivector *pAssists);
    void    WriteAssist(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, CPlayer *pPlayer);
    void    WriteDamage(IUnitEntity *pTarget, int iPlayer, ushort unAttackerType, ushort unInflictorType, float fDamage);
    void    WriteItem(EGameLogEvent eEvent, IEntityItem *pItem, IUnitEntity *pTarget = nullptr);
    void    WriteDeny(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, float fExperience, ushort unGold);
    void    WriteExperience(EGameLogEvent eEvent, IUnitEntity *pTarget, IUnitEntity *pSource, float fExperience);
    void    WriteGold(EGameLogEvent eEvent, CPlayer *pPlayer, IUnitEntity *pSource, ushort unGold);
    void    WriteAbility(EGameLogEvent eEvent, IEntityAbility *pAbility, IUnitEntity *pTarget = nullptr);
    void    WriteHero(EGameLogEvent eEvent, IHeroEntity *pHero, const tstring &sParamA = TSNULL);
    void    WriteAward(EGameLogEvent eEvent, IUnitEntity *pAttacker, IUnitEntity *pTarget = nullptr, ushort unGold = 0);
};
//=============================================================================

#endif //__C_GAMELOG_H__
