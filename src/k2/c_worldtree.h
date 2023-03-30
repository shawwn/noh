// (C)2005 S2 Games
// c_worldtree.h
//
//=============================================================================
#ifndef __C_WORLDTREE_H__
#define __C_WORLDTREE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
#include "c_convexpolyhedron.h"
#include "c_sphere.h"
#include "c_recyclepool.h"
#include "c_worldentitylist.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
enum EGridTriangles;

class CK2Model;
class CTreeModel;
class CWorldEntity;
class CWorldTreeNode;
class CConvexPolyhedron;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int MAX_TREE_LEVELS(17);

struct SWorkingTraceVars
{
	CBBoxf	bbBounds;
	CBBoxf	bbBoundsWorld;
	CVec3f	v3Start;
	CVec3f	v3End;
	CVec3f	v3Dir;
	CVec3f	v3Delta;
	uint	uiIgnoreSurface;
	uint	uiTestSurface;
	uint	uiIgnoreEntity;
	float	fFraction;
	CPlane	plPlane;
	float	fLength;
	bool	bStartedInSurface;
	bool	bEmbedded;
	bool	bStartEqualsEnd;
	float	fEpsilonFrac;
	bool	bIsPoint;
	int		iHitType;
	uint	uiHitEntity;
	uint	uiSurfaceIndex;
	uint	uiSurfFlags;
};

struct SNodeStack
{
	uint uiPos;
	uint uiSize;

	CWorldTreeNode**	pNodes;
};

struct SNodeStack2Node
{
	CWorldTreeNode*		pWorldTreeNode;
	float				fEnter;
	float				fExit;
};

struct SNodeStack2
{
	uint uiPos;
	uint uiSize;

	SNodeStack2Node*	pNodes;
};

enum EPartitionType
{
	NO_SPLIT = 0,
	X_SPLIT,
	Y_SPLIT,
	Z_SPLIT
};

enum ELinkFlags
{
	LINK_BOUNDS = BIT(0),
	LINK_SURFACE = BIT(1),
	LINK_MODEL = BIT(2),
	LINK_RENDER = BIT(3),
	LINK_OCCLUDE = BIT(4)
	// TODO: LINK_SKELETON = BIT(4)
};

enum EHitType
{
	HIT_TERRAIN = 1,
	HIT_PROP,
	HIT_ENTITY
};
//=============================================================================

//=============================================================================
// CWorldTreeNode
//=============================================================================
class CWorldTreeNode
{
private:
	CBBoxf		m_bbBounds; // bounding box, world space

	CVec2s		m_v2GridMin;
	CVec2s		m_v2GridMax;

	float		m_fTerrainHeightMin;
	float		m_fTerrainHeightMax;

	EPartitionType	m_eSplitType;
	float			m_fSplitPos;

	CWorldTreeNode	*m_pChildLeft; // Right child is always left + 1
	CWorldTreeNode	*m_pParent;

	PoolHandle	m_hLinkedBoundsStatic;
	PoolHandle	m_hLinkedSurfacesStatic;
	PoolHandle	m_hLinkedModelsStatic;
	
	PoolHandle	m_hLinkedBoundsDynamic;
	PoolHandle	m_hLinkedSurfacesDynamic;
	PoolHandle	m_hLinkedModelsDynamic;

	PoolHandle	m_hLinkedRenders;

	byte		m_yBlockers;

public:
	~CWorldTreeNode();
	CWorldTreeNode();

	const CBBoxf&	GetBounds() const				{ return m_bbBounds; }
	const CVec2s&	GetGridMin() const				{ return m_v2GridMin; }
	const CVec2s&	GetGridMax() const				{ return m_v2GridMax; }
	float			GetTerrainHeightMin() const		{ return m_fTerrainHeightMin; }
	float			GetTerrainHeightMax() const		{ return m_fTerrainHeightMax; }
	EPartitionType	GetSplitType() const			{ return m_eSplitType; }
	float			GetSplitPos() const				{ return m_fSplitPos; }
	CWorldTreeNode*	GetChildLeft() const			{ return m_pChildLeft; }
	CWorldTreeNode*	GetChildRight() const			{ return m_pChildLeft + 1; }
	CWorldTreeNode*	GetParent() const				{ return m_pParent; }
	byte			GetBlockers() const				{ return m_yBlockers; }

	void	SetBounds(const CBBoxf &bbBounds)				{ m_bbBounds = bbBounds; }
	void	SetGridMin(const CVec2s &v2GridMin)				{ m_v2GridMin = v2GridMin; }
	void	SetGridMax(const CVec2s &v2GridMax)				{ m_v2GridMax = v2GridMax; }
	void	SetTerrainHeightMin(float fTerrainHeightMin)	{ m_fTerrainHeightMin = fTerrainHeightMin; }
	void	SetTerrainHeightMax(float fTerrainHeightMax)	{ m_fTerrainHeightMax = fTerrainHeightMax; }
	void	SetSplitType(EPartitionType eSplitType)			{ m_eSplitType = eSplitType; }
	void	SetSplitPos(float fSplitPos)					{ m_fSplitPos = fSplitPos; }
	void	SetChildLeft(CWorldTreeNode *pChild)			{ m_pChildLeft = pChild; }
	void	SetParent(CWorldTreeNode *pParent)				{ m_pParent = pParent; }
	void	SetBlockers(byte yBlockers)						{ m_yBlockers = yBlockers; }

	PoolOffset	LinkBoundsStatic(PoolHandle hWorldEntHandle);
	PoolOffset	LinkSurfaceStatic(PoolHandle hWorldEntHandle);
	PoolOffset	LinkModelStatic(PoolHandle hWorldEntHandle);

	PoolOffset	LinkBoundsDynamic(PoolHandle hWorldEntHandle);
	PoolOffset	LinkSurfaceDynamic(PoolHandle hWorldEntHandle);
	PoolOffset	LinkModelDynamic(PoolHandle hWorldEntHandle);
	
	PoolOffset	LinkRender(PoolHandle hWorldEntHandle);

	void	UnlinkBoundsStatic(CWorldEntity *pPrev, CWorldEntity *pRemoved);
	void	UnlinkSurfaceStatic(CWorldEntity *pPrev, CWorldEntity *pRemoved);
	void	UnlinkModelStatic(CWorldEntity *pPrev, CWorldEntity *pRemoved);

	void	UnlinkBoundsDynamic(CWorldEntity *pPrev, CWorldEntity *pRemoved);
	void	UnlinkSurfaceDynamic(CWorldEntity *pPrev, CWorldEntity *pRemoved);
	void	UnlinkModelDynamic(CWorldEntity *pPrev, CWorldEntity *pRemoved);

	void	UnlinkRender(CWorldEntity *pPrev, CWorldEntity *pRemoved);

	void	UpdateBounds(const CWorld &cWorld, bool bRecurse);

	PoolHandle	GetHeadOfLinkedBoundsStatic()		{ return m_hLinkedBoundsStatic; }
	PoolHandle	GetHeadOfLinkedSurfacesStatic()		{ return m_hLinkedSurfacesStatic; }
	PoolHandle	GetHeadOfLinkedModelsStatic()		{ return m_hLinkedModelsStatic; }

	PoolHandle	GetHeadOfLinkedBoundsDynamic()		{ return m_hLinkedBoundsDynamic; }
	PoolHandle	GetHeadOfLinkedSurfacesDynamic()	{ return m_hLinkedSurfacesDynamic; }
	PoolHandle	GetHeadOfLinkedModelsDynamic()		{ return m_hLinkedModelsDynamic; }

	PoolHandle	GetHeadOfLinkedRenders()			{ return m_hLinkedRenders; }
};
//=============================================================================

//=============================================================================
// CWorldTree
//=============================================================================
class CWorldTree : public IWorldComponent
{
private:
	bool			m_bInitialized;

	CWorldTreeNode				*m_pNodeBuffer;
	CWorldTreeNode				*m_pEntityLinkLimit;
	vector<CWorldTreeNode *>	m_vWorldTree;
	vector<int>					m_vTreeLevelSize;

	float			*m_pHeightMap;
	byte			*m_pSplitMap;
	byte			*m_pBlockerMap;

	SNodeStack	m_Stack;
	SNodeStack2 m_Stack2;

	// debugging variables
	uint	m_uiSurfacesTested;
	int		nodesChecked;
	int		boundsChecked;
	int		surfacesChecked;
	int		modelsChecked;
	int		triIntersections;
	int		maxSurfsInNode;
	int		nodesPassed;

	SWorkingTraceVars tv;

protected:
	inline void		ClearStack();
	inline void		PushNode(CWorldTreeNode *node);
	inline CWorldTreeNode*	PopNode();

	inline void		ClearStack2();
	inline void		PushNode2(CWorldTreeNode *pWorldTreeNode, float fEnter, float fExit);
	inline SNodeStack2Node*	PopNode2();

	bool	IntersectLineWithWorld();
	bool	IntersectLineWithSurface(CWorldEntity *pEnt);
	bool	IntersectLineWithModel(CWorldEntity *pEnt);
	bool	IntersectLineWithK2Model(CK2Model *pK2Model, const CAxis &axis, const CVec3f &v3Pos, float fScale);
	bool	IntersectLineWithTreeModel(CTreeModel *pTreeModel, const CAxis &axis, const CVec3f &v3Pos, float fScale);

	bool	IntersectMovingBoundsWithWorld();
	bool	IntersectMovingBoundsWithSurface(CWorldEntity *pEnt);

	void	ResetWorkingTraceVars(struct STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, int iIgnoreSurface, uint uiIgnoreEntity);
	void	PrintTraceStats();

	void	LinkBounds(CWorldEntity *pEnt);
	void	LinkSurface(CWorldEntity *pEnt);
	void	LinkModel(CWorldEntity *pEnt);
	void	LinkRender(CWorldEntity *pEnt);

	void	UnlinkBounds(CWorldEntity *pEnt);
	void	UnlinkSurface(CWorldEntity *pEnt);
	void	UnlinkModel(CWorldEntity *pEnt);
	void	UnlinkRender(CWorldEntity *pEnt);

	bool	TestTerrain(CWorldTreeNode *pNode, const CVec3f &v3Start, const CVec3f &v3End);

public:
	~CWorldTree();
	CWorldTree(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld)	{ return Generate(pWorld); }
	bool	Generate(const CWorld *pWorld);
	bool	Save(CArchive &archive)							{ return true; }
	void	Release();

	void	BuildTree(const CWorld *pWorld, int iLevels);
	void	InitTerrainHeight();
	void	Update(const CRecti &rect);

	bool	TraceLine(struct STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, int iIgnoreSurface, uint uiIgnoreEntity);
	bool	TraceBox(struct STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, int iIgnoreSurface, uint uiIgnoreEntity);
	K2_API void	GetEntitiesInRegion(uivector &vResult, const CBBoxf &bbRegion, uint uiIgnoreSurface);
	K2_API void	GetEntitiesInRadius(uivector &vResult, const CSphere &radius, uint uiIgnoreSurface);
	K2_API void	GetEntitiesInRadius(uivector &vResult, const CVec2f &v2Center, float fRadius, uint uiIgnoreSurface);
	K2_API void	GetEntitiesInSurface(uivector &vResult, const CConvexPolyhedron &cSurface, uint uiIgnoreSurface);
	K2_API void	GetEntityHandlesInRegion(WorldEntVector &vResult, const CBBoxf &bbRegion, uint uiIgnoreSurface);

	CWorldTreeNode*		FindBestFitNode(const CBBoxf &bbBounds);

	K2_API void	LinkEntity(CWorldEntity *pEnt, uint uiLinkFlags, uint uiSurfFlags);
	K2_API void	UnlinkEntity(CWorldEntity *pEnt);

	CWorldTreeNode*		GetWorldTree(uint uiLevel)		{ return (uiLevel < m_vWorldTree.size()) ? m_vWorldTree[uiLevel] : m_vWorldTree[0]; }

	CBBoxf		GetBounds() const
	{
		if (m_vWorldTree.size() > 0 && m_vWorldTree[0]) return m_vWorldTree[0][0].GetBounds(); else return CBBoxf(0.0f, 0.0f);
	}

	CBBoxf		GetTerrainBounds() const
	{
		if (m_vWorldTree.size() > 0 && m_vWorldTree[0])
		{
			const CBBoxf &bbWorld(m_vWorldTree[0][0].GetBounds());
			
			return CBBoxf
			(
				CVec3f(bbWorld.GetMin().x, bbWorld.GetMin().y, m_vWorldTree[0][0].GetTerrainHeightMin()),
				CVec3f(bbWorld.GetMax().x, bbWorld.GetMax().y, m_vWorldTree[0][0].GetTerrainHeightMax())
			);
		}
		else
			return CBBoxf(0.0f, 0.0f);
	}

	bool	TestBoundsVisibilty(const CVec3f &v3ViewPos, const CBBoxf &bbBoundsWorld);
};
//=============================================================================

#endif //__C_WORLDTREE_H__
