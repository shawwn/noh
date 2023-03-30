// (C)2005 S2 Games
// c_host.h
//
//=============================================================================
#ifndef __C_HOST_H__
#define __C_HOST_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_upgrades.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHostServer;
class CHostClient;
class CHostInterface;
class IWidget;
class CInterface;
class CWidgetStyle;
class CSnapshot;
class CHTTPManager;

extern K2_API class CHost *g_pHost;

K2_API EXTERN_CVAR(float, host_timeScale);
EXTERN_CMD(WriteConfigScript);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifndef K2_EXPORTS
#define Host (*g_pHost)
#else
#define Host (*CHost::GetInstance())
#endif

const uint MAX_NAME_LENGTH(15);

typedef map<uint, CHostClient*>			HostClientMap;
typedef HostClientMap::iterator			HostClientMap_it;
typedef HostClientMap::const_iterator	HostClientMap_cit;
//=============================================================================

//=============================================================================
// CHost
//=============================================================================
class CHost
{
	SINGLETON_DEF(CHost)

private:
	// Core objects
	CHTTPManager*		m_pHTTPManager;

	// Frame timing
	uint				m_uiThisFrameEnd;
	float				m_fThisFrameEndSeconds;
	uint				m_uiThisFrameLength;
	uint				m_uiCurrentFrame;
	uint				m_uiSystemTime;		// Stored once per frame so we don't have to keep calling K2System.Milliseconds
	uint				m_uiLastSystemTime;
	uint				m_uiTickAccumulator;

	// FPS
	uint				m_uiLastFPSTime;
	uint				m_uiLastFPS;
	uint				m_uiFPSCount;

	// Replays
	bool				m_bReplay;

	// Version stamp
	tsvector			m_vsVersionStampText;
	bool				m_bShowVersionStamp;
	uint				m_uiVersionStampTime;

	// Connection data
	tstring				m_sServerAddr;
	CHostServer*		m_pServer;
	CHostInterface*		m_pInterfaceExt;
	uint				m_uiActiveClient;
	uint				m_uiNextClientID;
	HostClientMap		m_mapClients;
	CHostClient*		m_pCurrentClient;
	stack<uint>			m_stackClientStopRequests;

	uint				m_uiID;

	// Updates
	bool				m_bUpdateAvailable;

	bool				m_bResolutionChange;
	bool				m_bNoConfig;
	tstring				m_sRegisterName;
	word				m_wRegisterPort;
	bool				m_bSleeping;
	bool				m_bFirstFrame;
	bool				m_bShouldDisconnect;
	tstring				m_sShouldDisconnectReason;

#ifdef K2_GARENA
	tstring				m_sGarenaToken;
#endif

	// Upgrades
	CUpgrades			m_cUpgrades;

	// Internal functions
	void	UpdateTime();
	void	Frame(uint uiTickLength);
	void	ProcessCommandLine();

public:
	~CHost();

	// Core objects
	CHTTPManager*	GetHTTPManager() const			{ return m_pHTTPManager; }

	// Version stamp
	void			DrawVersionStamp();
	void			ShowVersionStamp();
	void			HideVersionStamp();
	void			ToggleVersionStamp()			{ if (m_bShowVersionStamp) HideVersionStamp(); else ShowVersionStamp(); }

	void			DrawInfoStrings();

	// Main interface
	K2_API void		Init(const tstring &sGame);
	K2_API void		Execute();
	K2_API void		Shutdown();
	
	void			PrintInitDebugInfo(const tstring &sMessage);

	uint			GetTime()						{ return m_uiThisFrameEnd; }
	float			GetTimeSeconds()				{ return m_fThisFrameEndSeconds; }
	uint			GetSystemTime()					{ return m_uiSystemTime; }
	uint			GetLastSystemTime()				{ return m_uiLastSystemTime; }
	uint			GetFrameLength()				{ return m_uiThisFrameLength; }
	uint			GetFrame()						{ return m_uiCurrentFrame; }

	// Accessors
	uint			GetID() const					{ return m_uiID; }
	bool			HasClient() const				{ return !m_mapClients.empty(); }
	bool			HasServer() const				{ return m_pServer != NULL; }

	bool			IsReplay() const				{ return m_bReplay; }

	void			SetActiveClient(uint uiIndex)	{ m_uiActiveClient = uiIndex; }
	void			NextClient();
	void			PrevClient();
	uint			GetActiveClientIndex() const	{ return m_uiActiveClient; }
	CHostClient*	GetActiveClient() const			{ HostClientMap_cit it(m_mapClients.find(m_uiActiveClient)); if (it == m_mapClients.end()) return NULL; return it->second; }
	CHostClient*	GetCurrentClient() const		{ return m_pCurrentClient; }

	// Misc commands
	bool					StartServer(const tstring &sName = TSNULL, const tstring &sGameSettings = TSNULL, bool bPractice = false, bool bLocal = false);
	void					StopServer(bool bFreeResources = true);
	
	bool					StartClient(const tstring &sModStack = TSNULL);
	void					StopClient(uint uiIndex = -1);
	void					RequestClientStop(uint uiIndex)	{ m_stackClientStopRequests.push(uiIndex); }
	void					ResetLocalClientTimeouts();

	// Client
	void					Connect(const tstring &sAddr, bool bSilent = false, bool bPractice = false, const tstring &sLoadingInterface = _T("loading"));
	void					Disconnect(const tstring &sReason = TSNULL);
	void					Reconnect();
	K2_API void				LoadAllClientResources();
	void					StartGame(const tstring &sType, const tstring &sName, const tstring &sOptions);
	void					PreloadWorld(const tstring &sWorldName);

	void					BanClient(int iClientNum, int iTime, const tstring &sReason);
	iset					GetAccountIDs();
	int						GetClientNumFromAccountID(int iAccountID);

	int						GetNumActiveClients();
	K2_API const tstring&	GetServerWorldName();

	bool					IsConnected();
	bool					IsInGame();
	tstring					GetConnectedAddress();

	K2_API void				FileDropNotify(const tsvector &vsFiles);

	bool					StartReplay(const tstring &sFilename);
	void					StopReplay();

	void					UpdateAvailable(const tstring &sVersion);
	void					UpdateComplete();

	K2_API	bool			IsIgnored(const tstring &sName);

	K2_API	CHostServer*	GetServer()							{ return m_pServer; }

	void					SaveConfig(const tstring &sFileName = TSNULL, const tstring &sFilter = TSNULL) const;

	void					SetResolutionChange(bool bResolutionChange)		{ m_bResolutionChange = bResolutionChange; }

	bool					GetNoConfig() const					{ return m_bNoConfig; }
	const tstring&			GetRegisterName() const				{ return m_sRegisterName; }
	word					GetRegisterPort() const				{ return m_wRegisterPort; }

	bool					IsSleeping() const					{ return m_bSleeping; }
	void					SetSleeping(bool bSleeping)			{ m_bSleeping = bSleeping; }

	void					SetNoConfig(bool bNoConfig)			{ m_bNoConfig = bNoConfig; }

	void					DisconnectNextFrame(const tstring &sReason = TSNULL)
	{
		m_bShouldDisconnect = true;
		m_sShouldDisconnectReason = sReason;
	}

#ifdef K2_GARENA
	const tstring&			GetGarenaToken() const				{ return m_sGarenaToken; }
#else
	const tstring&			GetGarenaToken() const				{ return TSNULL; }
#endif // K2_GARENA

	// Upgrades
	uint					LookupChatSymbol(const tstring &sName) const				{ return m_cUpgrades.LookupChatSymbol(sName); }
	uint					GetNumChatSymbols() const									{ return m_cUpgrades.GetNumChatSymbols(); }
	const CChatSymbol*		GetChatSymbol(uint uiChatSymbol) const						{ return m_cUpgrades.GetChatSymbol(uiChatSymbol); }
	const tstring&			GetChatSymbolDisplayName(uint uiChatSymbol) const			{ return m_cUpgrades.GetChatSymbolDisplayName(uiChatSymbol); }
	const tstring&			GetChatSymbolTexturePath(uint uiChatSymbol) const			{ return m_cUpgrades.GetChatSymbolTexturePath(uiChatSymbol); }
	
	uint					LookupChatNameColor(const tstring &sName) const				{ return m_cUpgrades.LookupChatNameColor(sName); }
	uint					GetNumChatNameColors() const								{ return m_cUpgrades.GetNumChatNameColors(); }
	const CChatNameColor*	GetChatNameColor(uint uiChatNameColor) const				{ return m_cUpgrades.GetChatNameColor(uiChatNameColor); }
	const tstring&			GetChatNameColorDisplayName(uint uiChatNameColor) const		{ return m_cUpgrades.GetChatNameColorDisplayName(uiChatNameColor); }
	const tstring&			GetChatNameColorTexturePath(uint uiChatNameColor) const		{ return m_cUpgrades.GetChatNameColorTexturePath(uiChatNameColor); }
	const tstring&			GetChatNameColorString(uint uiChatNameColor) const			{ return m_cUpgrades.GetChatNameColorString(uiChatNameColor); }
	const tstring&			GetChatNameColorIngameString(uint uiChatNameColor) const	{ return m_cUpgrades.GetChatNameColorIngameString(uiChatNameColor); }
	const uint				GetChatNameColorSortIndex(uint uiChatNameColor) const		{ return m_cUpgrades.GetChatNameColorSortIndex(uiChatNameColor); }

	uint					LookupAccountIcon(const tstring &sName) const				{ return m_cUpgrades.LookupAccountIcon(sName); }
	uint					GetNumAccountIcons() const									{ return m_cUpgrades.GetNumAccountIcons(); }
	const CAccountIcon*		GetAccountIcon(uint uiAccountIcon) const					{ return m_cUpgrades.GetAccountIcon(uiAccountIcon); }
	const tstring&			GetAccountIconDisplayName(uint uiAccountIcon) const			{ return m_cUpgrades.GetAccountIconDisplayName(uiAccountIcon); }
	const tstring&			GetAccountIconTexturePath(uint uiAccountIcon) const			{ return m_cUpgrades.GetAccountIconTexturePath(uiAccountIcon); }

	uint					LookupAnnouncerVoice(const tstring &sName) const			{ return m_cUpgrades.LookupAnnouncerVoice(sName); }
	uint					GetNumAnnouncerVoices() const								{ return m_cUpgrades.GetNumAnnouncerVoices(); }
	const CAnnouncerVoice*	GetAnnouncerVoice(uint uiAnnouncerVoice) const				{ return m_cUpgrades.GetAnnouncerVoice(uiAnnouncerVoice); }
	const tstring&			GetAnnouncerVoiceDisplayName(uint uiAnnouncerVoice) const	{ return m_cUpgrades.GetAnnouncerVoiceDisplayName(uiAnnouncerVoice); }
	const tstring&			GetAnnouncerVoiceSet(uint uiAnnouncerVoice) const			{ return m_cUpgrades.GetAnnouncerVoiceSet(uiAnnouncerVoice); }
	const tstring&			GetAnnouncerVoiceArcadeText(uint uiAnnouncerVoice) const	{ return m_cUpgrades.GetAnnouncerVoiceArcadeText(uiAnnouncerVoice); }

	uint					LookupTaunt(const tstring &sName) const						{ return m_cUpgrades.LookupTaunt(sName); }
	uint					GetNumTaunts() const										{ return m_cUpgrades.GetNumTaunts(); }
	const CTaunt*			GetTaunt(uint uiTaunt) const								{ return m_cUpgrades.GetTaunt(uiTaunt); }
	const tstring&			GetTauntDisplayName(uint uiTaunt) const						{ return m_cUpgrades.GetTauntDisplayName(uiTaunt); }
	const tstring&			GetTauntModifier(uint uiTaunt) const						{ return m_cUpgrades.GetTauntModifier(uiTaunt); }
};
//=============================================================================

#endif //__C_HOST_H__
