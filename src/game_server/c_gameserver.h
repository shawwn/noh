// (C)2005 S2 Games
// c_gameserver.h
//
//=============================================================================
#ifndef __C_GAMESERVER_H__
#define __C_GAMESERVER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_triggermanager.h"
#include "../game_shared/i_game.h"
#include "../game_shared/c_teaminfo.h"
#include "../k2/c_dbmanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CGameServer;
class IGameEntity;
class CServerEntityDirectory;
class CHostServer;
class CWorld;
class CPacket;
class CSnapshot;
class CStatsTracker;
class CEntityGameInfo;
class CClientConnection;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define GameServer (*CGameServer::GetInstance())

typedef map<int, SPersistantItemVault>	PersistantVaultMap;
typedef pair<int, SPersistantItemVault>	PersistantVaultPair;
typedef PersistantVaultMap::iterator	PersistantVaultMap_it;

#define SERVER_CMD(name) \
	static bool	svcmd##name##Fn(const svector &vArgList);\
CMD(name)\
{\
	bool bReturn(false);\
\
	IGame *pGame(Game.GetCurrentGamePointer());\
	Game.SetCurrentGamePointer(CGameServer::GetInstance());\
\
	bReturn = svcmd##name##Fn(vArgList);\
\
	Game.SetCurrentGamePointer(pGame);\
\
	return bReturn;\
}\
bool	svcmd##name##Fn(const svector &vArgList)



#define SERVER_FCN(name) \
	static tstring	svfn##name##Fn(const svector &vArgList);\
FUNCTION(name)\
{\
	tstring sReturn(_T(""));\
\
	IGame *pGame(Game.GetCurrentGamePointer());\
	Game.SetCurrentGamePointer(CGameServer::GetInstance());\
\
	sReturn = svfn##name##Fn(vArgList);\
\
	Game.SetCurrentGamePointer(pGame);\
\
	return sReturn;\
}\
tstring	svfn##name##Fn(const svector &vArgList)
//=============================================================================

//=============================================================================
// CGameServer
//=============================================================================
class CGameServer : public IGame
{
	SINGLETON_DEF(CGameServer)

private:
	CServerEntityDirectory*	m_pServerEntityDirectory;
	CHostServer*			m_pHostServer;

	CDBManager*				m_pDBManager;

	CEntityGameInfo*		m_pGameInfo;

	uint					m_uiLastUpdateCheck;

	smaps					m_mapCensor;

	PersistantVaultMap		m_mapPersistantVaults;

	vector<ushort>			m_vItemDrops;

	uint					m_uiLastStatusNotifyTime;

	uiset					m_setCommanderReviewSubmitted;
	uiset					m_setKarmaReviewSubmitted;

	uint					m_uiLastGameLength;

	// Message handlers
	bool	ProcessUnitRequest();
	bool	ProcessTeamRequest();

	void	PrecacheEntities();
	void	PrecacheEntity(const tstring &sName);

	void		AnalyzeTerrain()				{ GetWorldPointer()->AnalyzeTerrain(); }
	void		UpdateNavigation()				{ GetWorldPointer()->UpdateNavigation(); }

public:
	~CGameServer();

	bool			IsServer()								{ return true; }

	IGameEntity*	AllocateEntity(const tstring &sName, uint uiMinIndex = INVALID_INDEX);
	IGameEntity*	AllocateEntity(ushort unType, uint uiMinIndex = INVALID_INDEX);
	void			DeleteEntity(IGameEntity *pEntity);
	void			DeleteEntity(uint uiIndex);

	// API functions
	void			SetGamePointer()						{ IGame::SetCurrentGamePointer(this); }
	bool			Initialize(CHostServer *pHostServer);
	void			Frame();
	bool			LoadWorld();
	bool			StartReplay(const tstring &sFilename);
	void			StopReplay();
	void			AddClient(CClientConnection *pClientConnection);
	void			RemoveClient(int iClientNum);
	bool			ProcessClientSnapshot(int iClientNum, CClientSnapshot &snapshot);
	bool			ProcessGameData(int iClientNum, CPacket &pkt);
	void			GetSnapshot(CSnapshot &snapshot);
	void			ReauthClient(CClientConnection *pClientConnection);
	void			Shutdown();
	void			UnloadWorld();

	bool			LoadNextMap();
	void			SetRace(int iTeam, const tstring &sRaceName);
	
	int				GetClientNumFromAccountID(int iAccountID);
	int				GetClientNumFromName(const tstring &sName);
	IPlayerEntity*	GetPlayerFromClientNum(int iClientNum);

	// Client requests
	void			ChangeTeam(int iClientID, int iTeam);
	IPlayerEntity*	ChangeUnit(int iClientNum, ushort unNewUnitID, int iFlags = 0);
	void			CancelSacrifice(int iClientNum);
	void			SacrificeUnit(int iClientNum, ushort unNewUnitID);
	int				PurchasePersistantItem(int iClientNum, int iVaultNum);
	bool			PurchaseItem(int iClientNum, const tstring &sClass, bool bCheckPurchaseRules = true);
	void			SellItem(int iSlot, IPlayerEntity *pPlayer, int iNumSold);
	
	void			StartBuilding(int iClientNum, CPacket &pkt);
	void			PlaceBuilding(int iClientNum, CPacket &pkt);
	void			StopBuilding(int iClientNum);

	int				GetClientCount(int iTeam = -1);
	int				GetConnectedClientCount(int iTeam = -1);

	void			StartGame();
	void			EndGame(int iLosingTeam);
	void			Restart();
	void			StartWarmup();

	void			SetupFrame();
	void			ActiveFrame();
	void			EndedFrame();
	void			DatabaseFrame();

	void			SendStats();

	void			BroadcastGameData(const IBuffer &buffer, bool bReliable, int iExcludeClient = -1);

	void			SendPersistantDataToClient(int iClientNum);

	void			SendMessage(const tstring &sMsg, int iClientNum);
	void			SendGameData(int iClient, const IBuffer &buffer, bool bReliable);

	void			StateStringChanged(uint uiID, const CStateString &ss);

	CStateString&	GetStateString(uint uiID);

	virtual uint	GetServerFrame();
	virtual uint	GetServerTime();
	virtual uint	GetPrevServerTime();
	virtual uint	GetServerFrameLength();

	int				AddTeam(const tstring &sName, CTeamDefinition &teamdef);
	
	// Resources
	ResHandle		RegisterModel(const tstring &sPath);
	ResHandle		RegisterEffect(const tstring &sPath);

	void			SendVoiceData(int iClientNum, uint uiLength, char *pData);
	void			StoppedTalking(int iClientNum);
	void			StartedTalking(int iClientNum);
	iset			GetVoiceTargets(int iClientNum);
	void			RemoveVoiceClient(int iClientNum);
	void			RemoveAllVoiceClients();

	int				GivePersistantItem(int iClientNum, ushort unItemData);
	int				GiveItem(int iClientNum, const tstring &sItem);

	// Trigger-related
	void			RegisterEntityScript(uint uiEntity, const tstring &sName, const tstring &sScript)		{ TriggerManager.RegisterEntityScript(uiEntity, sName, sScript); }
	void			RegisterGlobalScript(const tstring &sName, const tstring &sScript)						{ TriggerManager.RegisterGlobalScript(sName, sScript); }
	bool			TriggerEntityScript(uint uiEntity, const tstring &sName)								{ return TriggerManager.TriggerEntityScript(uiEntity, sName); }
	bool			TriggerGlobalScript(const tstring &sName)												{ return TriggerManager.TriggerGlobalScript(sName); }

	bool			TriggerScript(const tstring &sName)														{ return TriggerManager.TriggerGlobalScript(sName); }

	void			CopyEntityScripts(uint uiIndex, uint uiTargetIndex)										{ TriggerManager.CopyEntityScripts(uiIndex, uiTargetIndex); }
	void			ClearEntityScripts(uint uiIndex)														{ TriggerManager.ClearEntityScripts(uiIndex); }
	void			ClearAllEntityScripts()																	{ TriggerManager.ClearAllEntityScripts(); }
	void			ClearGlobalScripts()																	{ TriggerManager.ClearGlobalScripts(); }

	void			RegisterTriggerParam(const tstring &sName, const tstring &sValue)						{ TriggerManager.RegisterTriggerParam(sName, sValue); }

	tstring			CensorChat(const tstring &sMessage);
	void			InitCensor();

	virtual ushort	GetRandomItem(IVisualEntity *pEntity = NULL);
	virtual ushort	GetRandomPersistantItem();

	ushort			GetPersistantItemType(int iClientNum, int iVaultNum);
	uint			GetPersistantItemID(int iClientNum, int iVaultNum);

	void			MatchStatEvent(int iClientNumber, EPlayerMatchStat eStat, float fValue, int iTargetClientID = -1, ushort unInflictorType = INVALID_ENT_TYPE, ushort unTargetType = INVALID_ENT_TYPE, uint uiTime = INVALID_TIME);
	void			MatchStatEvent(int iClientNumber, EPlayerMatchStat eStat, int iValue, int iTargetClientID = -1, ushort unInflictorType = INVALID_ENT_TYPE, ushort unTargetType = INVALID_ENT_TYPE, uint uiTime = INVALID_TIME);

	tstring			GetServerStatus();

	void			Kick(int iClientNum, const tstring sReason);
	void			Ban(int iClientNum, int iLength, const tstring sReason);

	bool			ResetWorld();

	uint			GetLastGameLength() const				{ return m_uiLastGameLength; }

	// Alan's pathing
	virtual PoolHandle	FindPath(uint uiWorldEntityIndex, const CVec2f &goal) const						{ return GetWorldPointer()->FindPath( uiWorldEntityIndex, goal ); }
	virtual PoolHandle	FindPath(const CVec2f &v2Src, float fEntityWidth, uint uiNavigationFlags, const CVec2f &v2Goal) const	{ return GetWorldPointer()->FindPath(v2Src, fEntityWidth, uiNavigationFlags, v2Goal); }
	virtual CPath*		AccessPath(PoolHandle hPath ) const												{ return GetWorldPointer()->AccessPath(hPath); }
	virtual void		FreePath(PoolHandle hPath ) const													{ return GetWorldPointer()->FreePath(hPath); }
	virtual void		BlockPath(uint uiFlags, const CVec2f &v2Position, float fWidth, float fHeight)	{ GetWorldPointer()->BlockPath(uiFlags, v2Position, fWidth, fHeight); }
	virtual void		BlockPath(uint uiFlags, const CConvexPolyhedron &cSurf, float fStepHeight)							{ GetWorldPointer()->BlockPath(uiFlags, cSurf, fStepHeight); }
	virtual void		ClearPath(uint uiFlags, const CVec2f &v2Position, float fWidth, float fHeight)	{ GetWorldPointer()->ClearPath(uiFlags, v2Position, fWidth, fHeight); }
	virtual void		ClearPath(uint uiFlags, const CConvexPolyhedron &cSurf)							{ GetWorldPointer()->ClearPath(uiFlags, cSurf); }
};
//=============================================================================

#endif //__C_GAMESERVER_H__
