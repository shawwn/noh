// (C)2007 S2 Games
// c_scenestats.h
//
//=============================================================================
#ifndef __C_SCENESTATS_H__
#define __C_SCENESTATS_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "c_material.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ESceneStatMode
{
	SS_TOTALS,
	SS_PHASE,
	SS_BATCHTYPE,
	SS_COMBINED,
	NUM_SCENESTAT_MODES
};

enum ESceneStatBatchType
{
	SSBATCH_DEBUG,
	SSBATCH_EFFECT,
	SSBATCH_FOLIAGE,
	SSBATCH_STATICMESH,
	SSBATCH_DYNAMICMESH,
	SSBATCH_SCENEPOLY,
	SSBATCH_TERRAIN,
	SSBATCH_TREEBILLBOARD,
	SSBATCH_TREEBRANCH,
	SSBATCH_TREEFROND,
	SSBATCH_TREELEAF,
	NUM_SSBATCH_TYPES
};
//=============================================================================

//=============================================================================
// CSceneStats
//=============================================================================
class CSceneStats
{
private:
	bool			m_bActive;
	bool			m_bDraw;
	bool			m_bRecord;

	ESceneStatMode	m_eMode;

	int				m_iVerts;
	int				m_iTris;
	int				m_iBatches;

	int				m_aiPhaseVerts[2];
	int				m_aiPhaseTris[2];
	int				m_aiPhaseBatches[2];

	int				m_aiBatchTypeVerts[NUM_SSBATCH_TYPES];
	int				m_aiBatchTypeTris[NUM_SSBATCH_TYPES];
	int				m_aiBatchTypeBatches[NUM_SSBATCH_TYPES];

	int				m_aiCombinedVerts[2][NUM_SSBATCH_TYPES];
	int				m_aiCombinedTris[2][NUM_SSBATCH_TYPES];
	int				m_aiCombinedBatches[2][NUM_SSBATCH_TYPES];

public:
	CSceneStats();

	~CSceneStats();

	K2_API void	RecordBatch(int iVerts, int iTris, EMaterialPhase ePhase, ESceneStatBatchType eBatchType);

	bool	IsActive()					{ return m_bActive; }
	void	SetActive(bool bActive)		{ m_bActive = bActive; }
	void	SetDraw(bool bDraw)			{ m_bDraw = bDraw; }
	bool	GetRecord()				{ return m_bRecord; }
	void	SetRecord(bool bRecord)	{ m_bRecord = bRecord; }

	K2_API void	ResetFrame();

	K2_API void	Frame();
	K2_API void	Draw();
};

extern K2_API CSceneStats SceneStats;
//=============================================================================

#endif // __C_SCENESTATS_H__
