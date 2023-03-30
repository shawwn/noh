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
typedef map<uint, string>			MapStateString;
typedef map<uint, CBufferDynamic>	MapStateBlock;

struct SReplayKeyFrame
{
	uint			uiFrame;
	CBufferBit		cBuffer;
	size_t			zPos;
	uint			uiStateString;
	uint			uiStateBlock;
};

struct SReplayEntityDesc
{
	tstring				sName;
	uint				uiVersion;
	TypeVector			cFieldTypes;
	uint				uiSize;
	CEntitySnapshot		cBaseline;
	SEntityDesc			cCompatDesc;
};

typedef map<ushort, SReplayEntityDesc>	EntDescIDMap;
typedef map<uint, SReplayEntityDesc>	BaseDynamicEntDescMap;
typedef map<ushort, uint>				DynamicEntDescMap;
//=============================================================================

//=============================================================================
// CReplayManager
//=============================================================================
class CReplayManager
{
SINGLETON_DEF(CReplayManager)

private:
	tstring			m_sFilename;
	CArchive		m_cAchive;
	CFileHandle		m_hReplayData;
	CFileHandle		m_hKeyFrames;
	bool			m_bPlaying;
	bool			m_bRecording;
	uint			m_uiNextUniqueID;
	byte			m_yGameVersion[4];
	bool			m_bFirstFrame;

	map<uint, SReplayKeyFrame>	m_mapKeyFrames;
	uint			m_uiNumFrames;

	vector<svector>			m_vStateStringStore;
	vector<MapStateBlock>	m_vStateBlockStore;
	
	bool	m_bFrameOpen;
	int		m_iCurrentFrame;

	string				m_sWorldName;
	CSnapshot			m_cCurrentSnapshot;
	PoolHandle			m_hLastSnapshot;
	MapClientGameData	m_mapGameData;
	MapClientGameData	m_mapGameDataReliable;
	MapStateString		m_mapStateStrings;
	MapStateBlock		m_mapStateBlocks;

	size_t				m_zStartPos;
	uint				m_uiBeginTime;
	uint				m_uiEndTime;
	uint				m_uiReplayVersion;

	int					m_iSpeed;

	uint				m_uiNumBitEntityFields;

	svector				m_vStateStrings;

	void				GenerateKeyFrames();
	void				TestKeyFrames();
	void				ReadSnapshot(int iFrame, CBufferBit &cBuffer);

	EntDescIDMap			m_mapEntDescs;
	BaseDynamicEntDescMap	m_mapBaseDynamicEntDescs;
	DynamicEntDescMap		m_mapDynamicEntDescs;
	
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
	~CReplayManager();

	GAME_SHARED_API void	StartRecording(const tstring &sFilename);
	GAME_SHARED_API void	StopRecording();

	GAME_SHARED_API bool	StartPlayback(const tstring &sFilename, bool bGenerateKeyFrames = true);
	GAME_SHARED_API void	StopPlayback();

	GAME_SHARED_API bool	IsRecording();
	GAME_SHARED_API bool	IsPlaying();

	GAME_SHARED_API void	StartFrame(int iFrame);
	GAME_SHARED_API void	EndFrame(PoolHandle hSnapshot);

	GAME_SHARED_API void	WriteGameData(uint iClient, const IBuffer &buffer, bool bReliable);
	GAME_SHARED_API void	WriteStateString(uint uiID, const CStateString &ss);
	GAME_SHARED_API void	WriteStateBlock(uint uiID, const IBuffer &buffer);

	GAME_SHARED_API tstring	GetWorldName();
	GAME_SHARED_API void	GetSnapshot(CSnapshot &snapshot);
	GAME_SHARED_API void	GetGameData(uint iClient, IBuffer &buffer);
	GAME_SHARED_API void	GetGameDataReliable(uint iClient, IBuffer &buffer);
	GAME_SHARED_API MapStateString&	GetStateStrings()		{ return m_mapStateStrings; }
	GAME_SHARED_API MapStateBlock&	GetStateBlocks()		{ return m_mapStateBlocks; }
	GAME_SHARED_API tstring	GetReplayFilename()				{ return m_sFilename; }

	GAME_SHARED_API void	SetPlaybackFrame(int iFrame);

	uint	GetBeginTime() const							{ return m_uiBeginTime; }
	uint	GetEndTime() const								{ return m_uiEndTime; }

	uint	GetFrame() const								{ return uint(m_iCurrentFrame); }
	uint	GetEndFrame() const								{ return m_uiNumFrames; }

	int						GetPlaybackSpeed() const						{ return m_iSpeed; }
	GAME_SHARED_API float	GetSpeedScale() const;
	void					SetPlaybackSpeed(int iSpeed);

	GAME_SHARED_API void	SetPaused(bool bPaused);
	GAME_SHARED_API bool	IsPaused() const;

	GAME_SHARED_API void	Profile(const tstring &sFilename, int iClient);
	GAME_SHARED_API void	Parse(const tstring &sFilename);
	GAME_SHARED_API void	Encode(const tstring &sInFilename, const tstring &sOutFilename);
	GAME_SHARED_API void	Encode2(const tstring &sInFilename, const tstring &sOutFilename);

	uint	GetReplayVersion() const						{ return m_uiReplayVersion; }

	GAME_SHARED_API void	UpdateNetIndexes();

	GAME_SHARED_API const	SReplayEntityDesc*	GetTypeDesc(ushort unType) const;
	GAME_SHARED_API const	SEntityDesc*		GetCompatTypeDesc(ushort unType) const;
};

extern GAME_SHARED_API CReplayManager *pReplayManager;
#define ReplayManager (*pReplayManager)
#ifdef GAME_SHARED_EXPORTS
#define pReplayManager CReplayManager::GetInstance()
#endif
//=============================================================================

#endif //__C_REPLAYMANAGER__
