// (C)2007 S2 Games
// c_graph.h
//
//=============================================================================
#ifndef __C_NAVIGATIONGRAPH_H__
#define __C_NAVIGATIONGRAPH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
#include "c_resultgate.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CSearchNode;

// Functor for operator>
template<class _Ty>
struct greaterDeref
{
    bool operator()(const _Ty& _Left, const _Ty& _Right) const
    {
        return (*_Left > *_Right);
    }
};

template <class T, class _Pred> class CPriorityQueue;
typedef CPriorityQueue<CSearchNode *, greaterDeref<CSearchNode *> > PrioritySearchQueue;
typedef vector<CSearchGate> SearchResult;

#define DOWNSIZE_LIMIT 1 // sync with definition in navigationmap (NUM_GRID_RESOLUTIONS)
//=============================================================================

//=============================================================================
// CNavigationGraph
//=============================================================================
class CNavigationGraph : public IWorldComponent
{
private:
    PrioritySearchQueue *m_pOpenNodesFromDst;
    PrioritySearchQueue *m_pOpenNodesFromSrc;
    CSearchNode         *m_pNodeBucket;
    CSearchGateR        *m_pGateBucket;
    uint                m_uiNodeOffsetX;
    uint                m_uiNodeOffsetY;
    uint                m_uiMaxNodeCount;
    uint                m_uiBucketWidth;
    uint                m_uiBucketHeight;
    uint                m_uiBucketMaskX;
    uint                m_uiBucketMaskY;
    uint                m_uiNavSize;
    uint                m_uiMaxQueueSize;

    // Graph vars for current search
    CPath               *m_pPathResult;
    CNavGridZ           *m_pCurrentGrid;
    CSearchNode         *m_pSrcNode;
    CSearchNode         *m_pDstNode;
    uint                m_uiDownSize;
    float               m_fGateScale;
    float               m_fInverseSearchScale;
    uint                m_uiHighestSmoothCost;
    float               m_fSrcX;
    float               m_fSrcY;
    float               m_fGoalX;
    float               m_fGoalY;
    uint                m_uiGoalRange;

    // common widths used in search tests
    int                 m_iEntityWidthSegments;
    float               m_fEntityRadius;

    // for smoothing the result
    uint                m_uiPathFound;
    SearchNodeList      m_vecNodesTraveled;
    PathResult          m_vResultGates;

    // for dest estimation
    uint                m_uiDistanceEst;
    uint                m_uiDirectionEst;
    CSearchNode         *m_pInitDst;
    // for src estimation
    uint                m_uiSrcDist;
    uint                m_uiSrcDirEst;
    CSearchNode         *m_pInitSrc;

    CSearchNode         *m_pBestDst;

    CVec2us             m_v2DirtyRegion;
    vector<CVec2us>     m_vDirtySpans;
    

    int             CoordToGrid(float fCoord)   { return ((int)floor(fCoord * m_fInverseSearchScale)); }
    inline float    GridToCoord(ushort unGrid)  { return (unGrid << m_uiDownSize) * m_fGateScale; }

    inline void     EstimateDestination();
    inline void     EstimateSource();
    void            SetDestinationArea(uint uiSqrDist);
    uint            FindGate(CSearchNode *pA, int iDirection, CSearchGateR &cGate);
    
    bool            LineOfSight(uint uiStartWaypt, uint uiDestWaypt, CResultGate &cResult);
    CResultGate     BuildResult(CSearchNode *pA);
    
    inline bool     AStarFromDst(uint uiSearchArea);
    inline bool     AStarFromDstNorth(CSearchNode *pCurrent, CSearchNode *pRelative, uint uiX, uint uiY, ESearchDirection eParentDirection);
    inline bool     AStarFromDstEast(CSearchNode *pCurrent, CSearchNode *pRelative, uint uiX, uint uiY, ESearchDirection eParentDirection);
    inline bool     AStarFromDstWest(CSearchNode *pCurrent, CSearchNode *pRelative, uint uiX, uint uiY, ESearchDirection eParentDirection);
    inline bool     AStarFromDstSouth(CSearchNode *pCurrent, CSearchNode *pRelative, uint uiX, uint uiY, ESearchDirection eParentDirection);
    
    inline bool     AStarFromSrc(uint uiSearchArea);
    inline bool     FloodFromSrc(uint uiSearchArea);
    bool            BidirectionalAStar();
    bool            AStar();
    bool            Flood();
    void            ConstructPath(CSearchNode *pContactNode, int iSrcDirection, int iDstDirection);
    void            ConstructTraveledPath(PathResult &vecPath);
    bool            Init(const CWorld* pWorld);
    void            ResetForNextSearch();
    void            IsReset();

    static inline void  TileDistance(int iDiffX, int iDiffY, uint &uiDiagonal, uint &uiLinear);
    static inline uint  TileDistance(int iDiffX, int iDiffY);

    inline void     TileDistance(CSearchNode *pSrc, CSearchNode *pDst, uint &uiDiagonal, uint &uiLinear);
    inline void     TileDistanceH(CSearchNode *pSrc, CSearchNode *pDst, uint &uiDiagonal, uint &uiLinear);
    inline void     TileDistanceV(CSearchNode *pSrc, CSearchNode *pDst, uint &uiDiagonal, uint &uiLinear);
    inline uint     TileDistance(CSearchNode *pSrc, CSearchNode *pDst);
    inline uint     SquaredDistance(CSearchNode *pSrc, CSearchNode *pDst);

    void            MarkDestinationArea(int x0, int y0, int radius);
    void            CloseDstNode(int x, int y);
    void            CloseDstHLine(int x0, int x1, int y);
    void            OpenDstNode(int x, int y);
    void            MarkDestinationArea(vector<PoolHandle> &vBlockers);

public:
    CNavigationGraph(EWorldComponent eComponent);

    virtual bool    Load(CArchive &archive, const CWorld *pWorld) { return Init(pWorld); };
    virtual bool    Generate(const CWorld *pWorld) { return Init(pWorld); }
    virtual void    Release();
    virtual bool    Save(CArchive &archive) { return true; }

    PoolHandle      FindPath(float fSrcX, float fSrcY, float fEntityWidth, uint uiNavigationFlags, float fGoalX, float fGoalY, float fGoalRange, vector<PoolHandle> *pBlockers = nullptr);

    inline CSearchNode* FindNeighbor(CSearchNode *pA, int iDirection);
    CSearchNode*    GetNode(uint uiX, uint uiY)                     { return &m_pNodeBucket[uiY * m_uiBucketWidth + uiX]; }

    inline uint     ValidateNode(CSearchNode *pA);

    inline uint     GetNodeX(CSearchNode *pA);
    inline uint     GetNodeY(CSearchNode *pA);

    inline CSearchGateR&    GetGate(CSearchNode *pA);
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CNavigationGraph::TileDistance
  ====================*/
inline
void    CNavigationGraph::TileDistance(int iDiffX, int iDiffY, uint &uiDiagonal, uint &uiLinear)
{
    if (iDiffX > iDiffY)
    {
        uiLinear = iDiffX - iDiffY;
        uiDiagonal = iDiffY << 1;
    }
    else
    {
        uiLinear = iDiffY - iDiffX;
        uiDiagonal = iDiffX << 1;
    }
}


/*====================
  CNavigationGraph::TileDistance
  ====================*/
inline
uint    CNavigationGraph::TileDistance(int iDiffX, int iDiffY)
{
    return iDiffX + iDiffY;
}


/*====================
  CNavigationGraph::GetNodeX
  ====================*/
inline
uint    CNavigationGraph::GetNodeX(CSearchNode *pA)
{
    return ((pA - m_pNodeBucket) & m_uiBucketMaskX);
}


/*====================
  CNavigationGraph::GetNodeY
  ====================*/
inline
uint    CNavigationGraph::GetNodeY(CSearchNode *pA)
{
    return (((pA - m_pNodeBucket) & m_uiBucketMaskY) >> m_uiNavSize);
}


/*====================
  CNavigationGraph::ValidateNode
  ====================*/
inline
uint    CNavigationGraph::ValidateNode(CSearchNode *pA)
{
    size_t uiOffset(pA > m_pNodeBucket ? pA - m_pNodeBucket : 0);

    if (((uiOffset & m_uiBucketMaskX) != 0) && ((uiOffset & m_uiBucketMaskY) != 0))
        return 1;

    return 0;
}


/*====================
  CNavigationGraph::GetGate
  ====================*/
inline
CSearchGateR&   CNavigationGraph::GetGate(CSearchNode *pA)
{
    size_t uiOffset(pA > m_pNodeBucket ? pA - m_pNodeBucket : 0);

    assert (uiOffset < m_uiMaxNodeCount);

    return *(m_pGateBucket + uiOffset);
}


/*====================
  CNavigationGraph::TileDistance
  ====================*/
inline
void    CNavigationGraph::TileDistance(CSearchNode *pSrc, CSearchNode *pDst, uint &uiDiagonal, uint &uiLinear)
{
    ptrdiff_t iSrcOffset(pSrc - m_pNodeBucket);
    ptrdiff_t iDstOffset(pDst - m_pNodeBucket);
    int iDiffX((iSrcOffset & m_uiBucketMaskX) - (iDstOffset & m_uiBucketMaskX));
    int iDiffY((iSrcOffset & m_uiBucketMaskY) - (iDstOffset & m_uiBucketMaskY));

    if (iDiffX < 0)
        iDiffX = -iDiffX;
    if (iDiffY < 0)
        iDiffY = -iDiffY;

    iDiffY >>= m_uiNavSize;

    TileDistance(iDiffX, iDiffY, uiDiagonal, uiLinear);
}


/*====================
  CNavigationGraph::TileDistanceH

  TileDistance from an East/West node
  ====================*/
inline
void    CNavigationGraph::TileDistanceH(CSearchNode *pSrc, CSearchNode *pDst, uint &uiDiagonal, uint &uiLinear)
{
    ptrdiff_t iSrcOffset(pSrc - m_pNodeBucket);
    ptrdiff_t iDstOffset(pDst - m_pNodeBucket);
    int iDiffX((iSrcOffset & m_uiBucketMaskX) - (iDstOffset & m_uiBucketMaskX));
    int iDiffY((iSrcOffset & m_uiBucketMaskY) - (iDstOffset & m_uiBucketMaskY));

    if (iDiffX < 0)
        iDiffX = -iDiffX;
    if (iDiffY < 0)
        iDiffY = -iDiffY;

    iDiffY >>= m_uiNavSize;

    TileDistance(iDiffX, iDiffY, uiDiagonal, uiLinear);

    if (iDiffY > iDiffX)
    {
        --uiLinear;
        ++uiDiagonal;
    }
}


/*====================
  CNavigationGraph::TileDistanceV

  TileDistance from an North/South node
  ====================*/
inline
void    CNavigationGraph::TileDistanceV(CSearchNode *pSrc, CSearchNode *pDst, uint &uiDiagonal, uint &uiLinear)
{
    int iSrcOffset(pSrc - m_pNodeBucket);
    int iDstOffset(pDst - m_pNodeBucket);
    int iDiffX((iSrcOffset & m_uiBucketMaskX) - (iDstOffset & m_uiBucketMaskX));
    int iDiffY((iSrcOffset & m_uiBucketMaskY) - (iDstOffset & m_uiBucketMaskY));

    if (iDiffX < 0)
        iDiffX = -iDiffX;
    if (iDiffY < 0)
        iDiffY = -iDiffY;

    iDiffY >>= m_uiNavSize;

    TileDistance(iDiffX, iDiffY, uiDiagonal, uiLinear);

    if (iDiffX > iDiffY)
    {
        --uiLinear;
        ++uiDiagonal;
    }
}


/*====================
  CNavigationGraph::TileDistance
  ====================*/
inline
uint    CNavigationGraph::TileDistance(CSearchNode *pSrc, CSearchNode *pDst)
{
    int iSrcOffset(pSrc - m_pNodeBucket);
    int iDstOffset(pDst - m_pNodeBucket);
    int iDiffX((iSrcOffset & m_uiBucketMaskX) - (iDstOffset & m_uiBucketMaskX));
    int iDiffY((iSrcOffset & m_uiBucketMaskY) - (iDstOffset & m_uiBucketMaskY));

    if (iDiffX < 0)
        iDiffX = -iDiffX;
    if (iDiffY < 0)
        iDiffY = -iDiffY;

    iDiffY >>= m_uiNavSize;

    return TileDistance(iDiffX, iDiffY);
}


/*====================
  CNavigationGraph::SquaredDistance
  ====================*/
inline
uint    CNavigationGraph::SquaredDistance(CSearchNode *pSrc, CSearchNode *pDst)
{
    int iSrcOffset(pSrc - m_pNodeBucket);
    int iDstOffset(pDst - m_pNodeBucket);
    int iDiffX((iSrcOffset & m_uiBucketMaskX) - (iDstOffset & m_uiBucketMaskX));
    int iDiffY((iSrcOffset & m_uiBucketMaskY) - (iDstOffset & m_uiBucketMaskY));
    
    iDiffY >>= m_uiNavSize;

    return iDiffX * iDiffX + iDiffY * iDiffY;
}


/*====================
  CNavigationGraph::FindNeighbor
  ====================*/
inline
CSearchNode*    CNavigationGraph::FindNeighbor(CSearchNode *pA, int iDirection)
{
    int aOffsets[SD_COUNT] =
    {
        (int)m_uiBucketWidth,
        1,
        -1,
        -(int)m_uiBucketWidth
    };

    return pA + aOffsets[iDirection];
}
//=============================================================================

#endif //__C_NAVIGATIONGRAPH_H__
