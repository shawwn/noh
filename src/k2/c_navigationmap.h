// (C)2008 S2 Games
// c_navigationmap.h
//
//=============================================================================
#ifndef __C_NAVIGATIONMAP_H__
#define __C_NAVIGATIONMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
#include "c_navgridZ.h"
#include "c_recyclepool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSearchGateR;
class CNavGridUnits;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum eGridResolutions
{
	EGRIDRES_FULL = 0,
	EGRIDRES_HALF,
	NUM_GRID_RESOLUTIONS,
};

enum eGridTypes
{
	ENAVGRID_SUMMARY = 0,
	ENAVGRID_ANTI,
	ENAVGRID_BUILDING,
	ENAVGRID_TREE,
	ENAVGRID_CLIFF,
	ENAVGRID_UNIT, // Merged each frame, not included in the summary
	ENAVGRID_COUNT,
	ENAVGRID_MAX = (ENAVGRID_COUNT + 1) * (NUM_GRID_RESOLUTIONS + 1),
};

struct SBlocker
{
	int iBeginX;
	int iEndX;
	int iBeginY;
	int iEndY;
	uint uiFlags;
};

typedef map<uint, PoolHandle> CustomGridMap;
typedef pair<uint, PoolHandle> CustomGridPair;
//=============================================================================

//=============================================================================
// CNavigationMap
//=============================================================================
class CNavigationMap : public IWorldComponent
{
	CRecyclePool<SBlocker>	m_cBlockers;
	CRecyclePool<CNavGridZ>	m_cGrids;
	PoolHandle				m_ahGrids[NUM_GRID_RESOLUTIONS][ENAVGRID_COUNT];
	
	CNavGridUnits			*m_pBuildingGrid;
	CNavGridUnits			*m_pTreeGrid;
	CNavGridUnits			*m_pAntiGrid;
	CNavGridUnits			*m_pUnitGrid;

	CustomGridMap			m_mapCustom;
	CustomGridMap			m_perFrameCustom;

	uint					m_uiGridFlags[ENAVGRID_COUNT];

	uint					m_uiMapChanges;
	uint					m_uiReady;
	float					m_fInverseTileScale;
	float					m_fNavigationScale;

	CNavGridZ*			GetGrid(uint uiFlags, uint uiResolution);
	inline void			UpdateGrid(eGridTypes eType);
	inline void			BuildSummary();
	inline uint			FlagsByResolution(uint uiFlags, uint uiDownRes);
	bool				Init(const CWorld* pWorld);
public:
	~CNavigationMap()	{ Release(); }
	CNavigationMap(EWorldComponent eComponent);

	virtual bool		Load(CArchive &archive, const CWorld *pWorld) { return Init(pWorld); };
	virtual bool		Generate(const CWorld *pWorld) { return Init(pWorld); }
	virtual void		Release();
	virtual bool		Save(CArchive &archive) { return true; }

	// Not all blockers have a world entity (cliffs, perhaps some spells)
	K2_API void			AnalyzeTerrain();
	K2_API PoolHandle	AddBlocker(uint uiFlags, float fPosX, float fPosY, float fWidth, float fHeight);
	K2_API void			AddBlocker2(uint uiFlags, float fPosX, float fPosY, float fWidth, float fHeight);
	K2_API void			AddBlocker(vector<PoolHandle> &vecBlockers, uint uiFlags, const CConvexPolyhedron &cSurf, float fStepHeight);
	K2_API void			ClearBlocker(PoolHandle hBlockerID);
	inline uint			LinearGate(EDivideType eType, uint unGridPos, uint uiMinSegment, uint uiMaxSegment) const;
	inline void			LinearExtent(EDivideType eType, uint uiGridPos, CSearchGateR &cGate) const;
	K2_API CNavGridZ*	PrepForSearch(uint uiFlags, uint uiResolution);
	SBlocker*			GetBlocker(PoolHandle hBlocker)		{ return m_cBlockers.GetReferenceByHandle(hBlocker); }

	void				UpdateNavigation();

	const CWorld&		GetWorld() const		{ return *m_pWorld; }

	bool				IsReady() const			{ return m_uiReady > 0; }
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CNavigationMap::UpdateGrid
  ====================*/
inline
void	CNavigationMap::UpdateGrid(eGridTypes eType)
{
	PROFILE("CNavigationMap::UpdateGrid");

	CNavGridZ *apGrids[NUM_GRID_RESOLUTIONS] =
	{
		m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][eType]),
		m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_HALF][eType])
	};

	apGrids[EGRIDRES_FULL]->HalfSize(*apGrids[EGRIDRES_HALF]);
}


/*====================
  CNavigationMap::BuildSummary
  ====================*/
inline
void	CNavigationMap::BuildSummary()
{
	PROFILE("CNavigationMap::BuildSummary");

	CNavGridZ *apGrids[4] =
	{
		m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_SUMMARY]),
		m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_BUILDING]),
		m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_TREE]),
		m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_CLIFF]),
	};

	apGrids[0]->CopyFrom(*apGrids[1]);
	apGrids[0]->Merge(*apGrids[2]);
	apGrids[0]->Merge(*apGrids[3]);
}
//=============================================================================

#endif //__C_NAVIGATIONMAP_H__
