// (C)2007 S2 Games
// c_replaymanager.h
//
//=============================================================================
#ifndef __C_REPLAYMANAGER__
#define __C_REPLAYMANAGER__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_snapshot.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBufferDynamic;

typedef map<uint, CBufferDynamic>	MapClientGameData;
typedef map<uint, tstring>			MapStateString;

struct SReplayKeyFrame
{
	uint			uiFrame;
	CBufferDynamic	bufSnapshot;
	size_t			zPos;
};
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint REPLAY_VERSION(0x00000011);
//=============================================================================

//=============================================================================
// CReplayManager
//=============================================================================
class CReplayManager
{
SINGLETON_DEF(CReplayManager)

private:
	tstring			m_sFilename;
	CFileHandle		m_hReplayData;
	CFileHandle		m_hKeyFrames;
	bool			m_bPlaying;
	bool			m_bRecording;

	map<uint, SReplayKeyFrame>	m_mapKeyFrames;
	uint			m_uiNumFrames;
	
	bool	m_bFrameOpen;
	int		m_iCurrentFrame;

	tstring				m_sWorldName;
	CSnapshot			m_cLastSnapshot;
	CSnapshot			m_cCurrentSnapshot;
	MapClientGameData	m_mapGameData;
	MapClientGameData	m_mapGameDataReliable;
	MapStateString		m_mapStateStrings;

	size_t				m_zStartPos;
	uint				m_uiBeginTime;
	uint				m_uiEndTime;

	int					m_iSpeed;

	void				GenerateKeyFrames();
	void				ReadSnapshot(int iFrame, IBuffer &cBuffer);
	
	// moved these to the class from CReplayManager::Profile since they need to have external linkage to be used as a template param
	struct SProfileEntitySnapshot
	{
		uint			uiCount;
		uint			uiSnapshotBytes;
		uint			uiHeaderBytes;
		uint			uiTransmitFlagBytes;
		uint			uiFieldBytes;
		vector<uint>	vFieldChanges;

		SProfileEntitySnapshot() : uiCount(0), uiSnapshotBytes(0), uiHeaderBytes(0), uiTransmitFlagBytes(0), uiFieldBytes(0) {}
	};
	
	struct SProfileField
	{
		uint			uiCount;
		uint			uiBytes;

		SProfileField() : uiCount(0), uiBytes(0) {}
	};

public:
	~CReplayManager()	{}

	GAME_SHARED_API void	StartRecording(const tstring &sFilename);
	GAME_SHARED_API void	StopRecording();

	GAME_SHARED_API bool	StartPlayback(const tstring &sFilename);
	GAME_SHARED_API void	StopPlayback();

	GAME_SHARED_API bool	IsRecording();
	GAME_SHARED_API bool	IsPlaying();

	GAME_SHARED_API void	StartFrame(int iFrame);
	GAME_SHARED_API void	EndFrame();

	GAME_SHARED_API void	WriteGameData(uint iClient, const IBuffer &buffer, bool bReliable);
	GAME_SHARED_API void	WriteSnapshot(const CSnapshot &snapshot);
	GAME_SHARED_API void	WriteStateString(uint uiID, const CStateString &ss);

	GAME_SHARED_API tstring	GetWorldName();
	GAME_SHARED_API void	GetSnapshot(CSnapshot &snapshot);
	GAME_SHARED_API void	GetGameData(uint iClient, IBuffer &buffer);
	GAME_SHARED_API void	GetGameDataReliable(uint iClient, IBuffer &buffer);
	GAME_SHARED_API MapStateString&	GetStateStrings()		{ return m_mapStateStrings; }
	GAME_SHARED_API tstring	GetReplayFilename()				{ return m_sFilename; }

	GAME_SHARED_API void	SetPlaybackFrame(int iFrame);

	uint	GetBeginTime() const							{ return m_uiBeginTime; }
	uint	GetEndTime() const								{ return m_uiEndTime; }

	uint	GetFrame() const								{ return uint(m_iCurrentFrame); }
	uint	GetEndFrame() const								{ return m_uiNumFrames; }

	int		GetPlaybackSpeed() const						{ return m_iSpeed; }
	void	SetPlaybackSpeed(int iSpeed);

	void	Profile(const tstring &sFilename, int iClient);
	void	Parse(const tstring &sFilename);
};

extern GAME_SHARED_API CReplayManager *pReplayManager;
#define ReplayManager (*pReplayManager)
#ifdef GAME_SHARED_EXPORTS
#define pReplayManager CReplayManager::GetInstance()
#endif
//=============================================================================
#endif // __C_REPLAYMANAGER__
