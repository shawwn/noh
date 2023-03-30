// (C)2008 S2 Games
// c_navigationgraph.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_world.h"
#include "c_navigationgraph.h"
#include "c_navigationmap.h"
#include "c_priorityqueue.h"
#include "c_searchnode.h"
#include "c_resultgate.h"
#include "c_worldentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define SEGMENT_WAYPOINT_RESERVE (g_PathTimeoutNodes >> 5)
#define TRAVELED_NODES_RESERVE ((m_uiBucketWidth + m_uiBucketHeight) * 2)

CVAR_INT(g_PathDetail, 0);
CVAR_INT(g_PathBidirectionalChanges, 1);
CVAR_UINT(g_PathBidirectionalSearchArea, 200);
CVAR_BOOL(g_PathAllowEstSrc, true);
CVAR_BOOL(g_PathAllowEstDst, true);
CVAR_UINT(g_PathTimeoutNodes, 16384);
CVAR_UINT(g_PathEstimationLimit, 32);
CVAR_UINT(g_PathMaxDownsize, DOWNSIZE_LIMIT);
CVAR_BOOL(g_PathDelayReset, false);

const uint VALID_PATH(1);
const uint ABORT_PATHING(2);

const uint LINEAR_WEIGHT(7);
const uint DIAGONAL_WEIGHT(5);

const uint H_WEIGHT_NUMERATOR(1);
const uint H_WEIGHT_DENOMINATOR(1);
//=============================================================================

/*====================
  CNavigationGraph::CNavigationGraph
  ====================*/
CNavigationGraph::CNavigationGraph(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("NavigationGraph")),
m_pOpenNodesFromDst(NULL),
m_pOpenNodesFromSrc(NULL),
m_pNodeBucket(NULL),
m_pGateBucket(NULL),
m_uiHighestSmoothCost(0),
m_uiMaxQueueSize(0),
m_uiGoalRange(0)
{
}


/*====================
  CNavigationGraph::EstimateDestination
  ====================*/
void	CNavigationGraph::EstimateDestination()
{
	do
	{
		if (m_uiDistanceEst > g_PathEstimationLimit)
		{
			m_uiPathFound = ABORT_PATHING;
			return;
		}

		m_pDstNode = m_pInitDst;

		switch (m_uiDirectionEst)
		{
		case SD_NORTH:
			{
				for (uint uiDistance(0); GetNodeY(m_pDstNode) < m_uiBucketHeight - 1 && uiDistance < m_uiDistanceEst; ++uiDistance)
					m_pDstNode += m_uiBucketWidth;
			}
			break;
		case SD_EAST:
			{
				for (uint uiDistance(0); GetNodeX(m_pDstNode) < m_uiBucketWidth - 1 && uiDistance < m_uiDistanceEst; ++uiDistance)
					m_pDstNode += 1;
			}
			break;
		case SD_WEST:
			{
				for (uint uiDistance(0); GetNodeX(m_pDstNode) > 0 && uiDistance < m_uiDistanceEst; ++uiDistance)
					m_pDstNode -= 1;
			}
			break;
		case SD_SOUTH:
			{
				for (uint uiDistance(0); GetNodeY(m_pDstNode) > 0 && uiDistance < m_uiDistanceEst; ++uiDistance)
					m_pDstNode -= m_uiBucketWidth;
			}
			break;
		}

		++m_uiDirectionEst;

		if (m_uiDirectionEst >= SD_COUNT)
		{
			m_uiDirectionEst = 0;
			++m_uiDistanceEst;
		}

	} while ((ValidateNode(m_pDstNode) == 0) || (~m_pDstNode->GetFlags() & SN_NOTFROMDST));

	if (~m_pDstNode->GetFlags() & SN_NOTFROMSRC)
	{
		m_pDstNode->ClearFlags(SN_NOTFROMDST);
		ConstructPath(m_pDstNode, m_pDstNode->ParentDirection(), SD_INVALID);
	}
	else
	{
		m_pOpenNodesFromDst->Push(m_pDstNode);
		m_pDstNode->ClearFlags(SN_NOTFROMDST);
		m_pDstNode->SetCost(0);

		uint uiNewX(GetNodeX(m_pDstNode));
		uint uiNewY(GetNodeY(m_pDstNode));

		if (uiNewY < m_v2DirtyRegion.x)
		{
			m_v2DirtyRegion.x = ushort(uiNewY);
			m_vDirtySpans[uiNewY] = CVec2us(uiNewX, uiNewX);
		}

		if (uiNewY > m_v2DirtyRegion.y)
		{
			m_v2DirtyRegion.y = ushort(uiNewY);
			m_vDirtySpans[uiNewY] = CVec2us(uiNewX, uiNewX);
		}
		
		CVec2us &v2Span(m_vDirtySpans[uiNewY]);
		if (uiNewX < v2Span.x)
			v2Span.x = ushort(uiNewX);
		if (uiNewX > v2Span.y)
			v2Span.y = ushort(uiNewX);
	}
}


/*====================
  CNavigationGraph::EstimateSource
  ====================*/
void	CNavigationGraph::EstimateSource()
{
	do
	{
		if (m_uiSrcDist > g_PathEstimationLimit)
		{
			m_uiPathFound = ABORT_PATHING;
			return;
		}

		m_pSrcNode = m_pInitSrc;

		switch (m_uiSrcDirEst)
		{
		case SD_NORTH:
			{
				for (uint uiDistance(0); GetNodeY(m_pSrcNode) < m_uiBucketHeight - 1 && uiDistance < m_uiSrcDist; ++uiDistance)
					m_pSrcNode += m_uiBucketWidth;
			}
			break;
		case SD_EAST:
			{
				for (uint uiDistance(0); GetNodeX(m_pSrcNode) < m_uiBucketWidth - 1 && uiDistance < m_uiSrcDist; ++uiDistance)
					m_pSrcNode += 1;
			}
			break;
		case SD_WEST:
			{
				for (uint uiDistance(0); GetNodeX(m_pSrcNode) > 0 && uiDistance < m_uiSrcDist; ++uiDistance)
					m_pSrcNode -= 1;
			}
			break;
		case SD_SOUTH:
			{
				for (uint uiDistance(0); GetNodeY(m_pSrcNode) > 0 && uiDistance < m_uiSrcDist; ++uiDistance)
					m_pSrcNode -= m_uiBucketWidth;
			}
			break;
		}

		++m_uiSrcDirEst;

		if (m_uiSrcDirEst >= SD_COUNT)
		{
			m_uiSrcDirEst = 0;
			++m_uiSrcDist;
		}

	} while ((ValidateNode(m_pSrcNode) == 0) || (~m_pSrcNode->GetFlags() & SN_NOTFROMSRC));

	if (~m_pSrcNode->GetFlags() & SN_NOTFROMDST)
	{
		m_pSrcNode->ClearFlags(SN_NOTFROMSRC);
		ConstructPath(m_pSrcNode, SD_INVALID, m_pSrcNode->ParentDirection());
	}
	else
	{
		m_pOpenNodesFromSrc->Push(m_pSrcNode);
		m_pSrcNode->ClearFlags(SN_NOTFROMSRC);
		m_pSrcNode->SetCost(0);

		uint uiNewX(GetNodeX(m_pSrcNode));
		uint uiNewY(GetNodeY(m_pSrcNode));

		if (uiNewY < m_v2DirtyRegion.x)
		{
			m_v2DirtyRegion.x = ushort(uiNewY);
			m_vDirtySpans[uiNewY] = CVec2us(uiNewX, uiNewX);
		}

		if (uiNewY > m_v2DirtyRegion.y)
		{
			m_v2DirtyRegion.y = ushort(uiNewY);
			m_vDirtySpans[uiNewY] = CVec2us(uiNewX, uiNewX);
		}
		
		CVec2us &v2Span(m_vDirtySpans[uiNewY]);
		if (uiNewX < v2Span.x)
			v2Span.x = ushort(uiNewX);
		if (uiNewX > v2Span.y)
			v2Span.y = ushort(uiNewX);
	}
}


/*====================
  CNavigationGraph::CloseDstNode
  ====================*/
void	CNavigationGraph::CloseDstNode(int x, int y)
{
	if (m_uiPathFound != 0)
		return;

	x = CLAMP<int>(x, 0, m_uiBucketWidth - 1);
	y = CLAMP<int>(y, 0, m_uiBucketHeight - 1);

	CSearchNode *pNode(&m_pNodeBucket[y * m_uiBucketWidth + x]);

	if (~pNode->GetFlags() & SN_NOTFROMDST)
		return;

	if (~pNode->GetFlags() & SN_NOTFROMSRC)
	{
		ConstructPath(pNode, SD_INVALID, CSearchNode::ParentDirectionReversed(pNode->ParentDirection()));
		m_pOpenNodesFromDst->Reset();
		return;
	}

	pNode->ClearFlags(SN_NOTFROMDST);
	pNode->SetFlags(SN_NOTLISTED);

	uint uiDiagonal, uiLinear;
	TileDistance(m_pSrcNode, pNode, uiDiagonal, uiLinear);
	pNode->SetCost(0);
	pNode->SetHeuristic((uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
	pNode->SetBias(SquaredDistance(m_pSrcNode, pNode));

	if (y < m_v2DirtyRegion.x)
	{
		m_v2DirtyRegion.x = ushort(y);
		m_vDirtySpans[y] = CVec2us(x, x);
	}

	if (y > m_v2DirtyRegion.y)
	{
		m_v2DirtyRegion.y = ushort(y);
		m_vDirtySpans[y] = CVec2us(x, x);
	}
	
	CVec2us &v2Span(m_vDirtySpans[y]);
	if (x < v2Span.x)
		v2Span.x = ushort(x);
	if (x > v2Span.y)
		v2Span.y = ushort(x);
}


/*====================
  CRasterBuffer::DrawHLine
  ====================*/
void	CNavigationGraph::CloseDstHLine(int x0, int x1, int y)
{
	for (int x(x0); x <= x1; ++x)
		CloseDstNode(x, y);
}


/*====================
  CNavigationGraph::OpenDstNode
  ====================*/
void	CNavigationGraph::OpenDstNode(int x, int y)
{
	if (m_uiPathFound != 0)
		return;

	x = CLAMP<int>(x, 0, m_uiBucketWidth - 1);
	y = CLAMP<int>(y, 0, m_uiBucketHeight - 1);

	CSearchNode *pNode(&m_pNodeBucket[y * m_uiBucketWidth + x]);

	if (~pNode->GetFlags() & SN_NOTFROMSRC)
	{
		ConstructPath(pNode, pNode->ParentDirection(), SD_INVALID);
		m_pOpenNodesFromDst->Reset();
		return;
	}

	pNode->ClearFlags(SN_NOTFROMDST);
	pNode->ClearFlags(SN_NOTLISTED);

	uint uiDiagonal, uiLinear;
	TileDistance(m_pSrcNode, pNode, uiDiagonal, uiLinear);
	pNode->SetCost(0);
	pNode->SetHeuristic((uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
	pNode->SetBias(SquaredDistance(m_pSrcNode, pNode));

#if 0
	int iSrcOffset(m_pDstNode - m_pNodeBucket);
	int iDstOffset(pNode - m_pNodeBucket);
	int iDiffX((iSrcOffset & m_uiBucketMaskX) - (iDstOffset & m_uiBucketMaskX));
	int iDiffY((iSrcOffset & m_uiBucketMaskY) - (iDstOffset & m_uiBucketMaskY));

	iDiffY >>= m_uiNavSize;

	if (ABS(iDiffY) > ABS(iDiffX))
	{
		if (iDiffY < 0)
			pNode->SetDirection(SD_NORTH);
		else
			pNode->SetDirection(SD_SOUTH);
	}
	else
	{
		if (iDiffX < 0)
			pNode->SetDirection(SD_EAST);
		else
			pNode->SetDirection(SD_WEST);
	}
#endif

	CSearchGateR &cGate(GetGate(pNode));
	cGate.SetMin(0);
	cGate.SetMax(0);

	m_pOpenNodesFromDst->Push(pNode);

	if (y < m_v2DirtyRegion.x)
	{
		m_v2DirtyRegion.x = ushort(y);
		m_vDirtySpans[y] = CVec2us(x, x);
	}

	if (y > m_v2DirtyRegion.y)
	{
		m_v2DirtyRegion.y = ushort(y);
		m_vDirtySpans[y] = CVec2us(x, x);
	}
	
	CVec2us &v2Span(m_vDirtySpans[y]);
	if (x < v2Span.x)
		v2Span.x = ushort(x);
	if (x > v2Span.y)
		v2Span.y = ushort(x);
}


/*====================
  CNavigationGraph::MarkDestinationArea

  based on http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
  ====================*/
void	CNavigationGraph::MarkDestinationArea(int x0, int y0, int radius)
{
	//
	// Mark closed nodes (filled circle)
	//

	if (radius > 0)
	{
		int f(1 - radius);
		int ddF_x(0);
		int ddF_y(-2 * radius);
		int x(0);
		int y(radius);

		CloseDstNode(x0, y0 + radius);
		CloseDstNode(x0, y0 - radius);
		CloseDstHLine(x0 - radius, x0 + radius, y0);

		while (x < y) 
		{
			if (f >= 0) 
			{
				y--;
				ddF_y += 2;
				f += ddF_y;
			}

			x++;
			ddF_x += 2;
			f += ddF_x + 1;

			CloseDstHLine(x0 - x, x0 + x, y0 + y);
			CloseDstHLine(x0 - x, x0 + x, y0 - y);
			CloseDstHLine(x0 - y, x0 + y, y0 + x);
			CloseDstHLine(x0 - y, x0 + y, y0 - x);
		}
	}


	//
	// Mark open nodes (circle outline)
	//

	{
		int f(1 - radius);
		int ddF_x(0);
		int ddF_y(-2 * radius);
		int x(0);
		int y(radius);

		OpenDstNode(x0, y0 + radius);
		OpenDstNode(x0, y0 - radius);
		OpenDstNode(x0 + radius, y0);
		OpenDstNode(x0 - radius, y0);

		while (x < y) 
		{
			if (f >= 0) 
			{
				y--;
				ddF_y += 2;
				f += ddF_y;
			}

			x++;
			ddF_x += 2;
			f += ddF_x + 1;
			
			OpenDstNode(x0 + x, y0 + y);
			OpenDstNode(x0 - x, y0 + y);
			OpenDstNode(x0 + x, y0 - y);
			OpenDstNode(x0 - x, y0 - y);
			OpenDstNode(x0 + y, y0 + x);
			OpenDstNode(x0 - y, y0 + x);
			OpenDstNode(x0 + y, y0 - x);
			OpenDstNode(x0 - y, y0 - x);
		}
	}
}


/*====================
  CNavigationGraph::MarkDestinationArea

  Stroke destination area defined by the blockers
  ====================*/
void	CNavigationGraph::MarkDestinationArea(vector<PoolHandle> &vBlockers)
{
	if (vBlockers.empty())
		return;

	CNavigationMap &cNavMap(m_pWorld->GetNavigationMap());
	
	SBlocker *pBlocker(cNavMap.GetBlocker(vBlockers[0]));
	if (pBlocker == NULL)
		return;

	// Close each blocker
	// Sides of each blocker
	for (vector<PoolHandle>::iterator it(vBlockers.begin()), itEnd(vBlockers.end()); it != itEnd; ++it)
	{
		SBlocker *pBlocker(cNavMap.GetBlocker(*it));
		if (pBlocker == NULL)
			continue;

		for (int y(pBlocker->iBeginY >> m_uiDownSize); y <= pBlocker->iEndY >> m_uiDownSize; ++y)
		{
			for (int x(pBlocker->iBeginX >> m_uiDownSize); x <= pBlocker->iEndX >> m_uiDownSize; ++x)
			{
				CloseDstNode(x, y);
			}
		}
	}

	// Top (-y side of first blocker)
	if (pBlocker->iBeginY >> m_uiDownSize > 0)
	{
		for (int i(pBlocker->iBeginX >> m_uiDownSize); i <= pBlocker->iEndX >> m_uiDownSize; ++i)
			OpenDstNode(i, (pBlocker->iBeginY >> m_uiDownSize) - 1);
	}

	// Sides of each blocker
	for (vector<PoolHandle>::iterator it(vBlockers.begin()), itEnd(vBlockers.end()); it != itEnd; ++it)
	{
		SBlocker *pBlocker(cNavMap.GetBlocker(*it));
		if (pBlocker == NULL)
			continue;

		for (int i(pBlocker->iBeginY >> m_uiDownSize); i <= pBlocker->iEndY >> m_uiDownSize; ++i)
		{
			if (pBlocker->iBeginX >> m_uiDownSize > 0)
				OpenDstNode((pBlocker->iBeginX >> m_uiDownSize) - 1, i);
			if (pBlocker->iEndX >> m_uiDownSize < int(m_uiBucketWidth - 1))
				OpenDstNode((pBlocker->iEndX >> m_uiDownSize) + 1, i);
		}
	}

	// Bottom (+y side of first blocker)
	if (pBlocker->iEndY >> m_uiDownSize < int(m_uiBucketHeight - 1))
	{
		for (int i(pBlocker->iBeginX >> m_uiDownSize); i <= pBlocker->iEndX >> m_uiDownSize; ++i)
			OpenDstNode(i, (pBlocker->iEndY >> m_uiDownSize) + 1);
	}
}


/*====================
  CNavigationGraph::FindGate
  ====================*/
uint	CNavigationGraph::FindGate(CSearchNode *pA, int iDirection, CSearchGateR &cGate)
{
	uint uiX(GetNodeX(pA)), uiY(GetNodeY(pA));
	uint uiaGridPos[SD_COUNT] = { uiY, uiX, uiX - 1, uiY - 1 };
	uint uiaSegment[SD_COUNT] = { uiX, uiY, uiY, uiX };
	EDivideType eaType[SD_COUNT] = { DIVIDE_HORIZONTAL, DIVIDE_VERTICAL, DIVIDE_VERTICAL, DIVIDE_HORIZONTAL };
	uint uiBeginSegment(uiaSegment[iDirection] << m_uiDownSize);
	uint uiEndSegment(uiBeginSegment + (1 << m_uiDownSize));

	return m_pCurrentGrid->LinearGate(eaType[iDirection], uiaGridPos[iDirection], uiBeginSegment, uiEndSegment, m_iEntityWidthSegments, cGate);
}


/*====================
  CNavigationGraph::LineOfSight
  ====================*/
bool	CNavigationGraph::LineOfSight(uint uiStartWaypt, uint uiDestWaypt, CResultGate &cResult)
{
	CResultGate &cSrc(m_vResultGates[uiStartWaypt]);
	CResultGate &cDst(m_vResultGates[uiDestWaypt]);
	CVec2f v2ToDst(cDst.GetPath() - cSrc.GetPath());
	CVec2f v2Slope(v2ToDst.x != 0.0f ? v2ToDst.y / v2ToDst.x : 0.0f, v2ToDst.y != 0.0f ? v2ToDst.x / v2ToDst.y : 0.0f);

	float fMult[SD_COUNT] =
	{
		1.0f,
		-1.0f,
		1.0f,
		-1.0f
	};

	for (uint uiCurrentWaypoint(uiStartWaypt + 1); uiCurrentWaypoint < uiDestWaypt; ++uiCurrentWaypoint)
	{
		CResultGate &cGate(m_vResultGates[uiCurrentWaypoint]);

		CVec2f v2ToCurrent(cGate.GetPath() - cSrc.GetPath());
		CVec2f v2LineXY(v2ToCurrent.y * v2Slope.y, v2ToCurrent.x * v2Slope.x);

		float fEquiv[SD_COUNT] =
		{
			v2ToCurrent.x - v2LineXY.x,
			v2ToCurrent.y - v2LineXY.y,
			v2ToCurrent.y - v2LineXY.y,
			v2ToCurrent.x - v2LineXY.x
		};

		float fDistance(fEquiv[cGate.Direction()] * fMult[cGate.Direction()]);

		if (fDistance > 0.0f)
		{
			if (cGate.GetRadiusPositive() < fDistance)
				return false;
		}
		else if (fDistance < 0.0f)
		{
			if (cGate.GetRadiusNegative() < -fDistance)
				return false;
		}
	}

	return true;
}


/*====================
  CNavigationGraph::BuildResult
  ====================*/
CResultGate		CNavigationGraph::BuildResult(CSearchNode *pA)
{
	float fX(0.0f), fY(0.0f);
	float fGateMin, fGateMax;
	float fCenter(m_fGateScale * pow(2.0f, float(m_uiDownSize)) * 0.5f);
	CResultGate cRet;
	CSearchGateR cGate(GetGate(pA));
	uint uiX(GetNodeX(pA)), uiY(GetNodeY(pA));

	switch (pA->ParentDirection())
	{
	case SD_NORTH:
		fX = GridToCoord(uiX) + fCenter;
		fY = GridToCoord(uiY + 1);

		fGateMin = cGate.Min() * m_fGateScale - fCenter - m_fEntityRadius;
		fGateMax = cGate.Max() * m_fGateScale + fCenter - m_fEntityRadius;

		if (fGateMin < 0.0f)
		{
			fX -= fGateMin;
			fGateMax += fGateMin;
			fGateMin = 0.0f;
		}
		else if (fGateMax < 0.0f)
		{
			fX += fGateMax;
			fGateMin += fGateMax;
			fGateMax = 0.0f;
		}

		cRet = CResultGate(CVec2f(fX, fY), fGateMax, fGateMin, pA->ParentDirection(), CVec4f(0.0f, 0.0f, 1.0f, 1.0f));
		break;
	case SD_SOUTH:
		fX = GridToCoord(uiX) + fCenter;
		fY = GridToCoord(uiY);

		fGateMin = cGate.Min() * m_fGateScale + fCenter - m_fEntityRadius;
		fGateMax = cGate.Max() * m_fGateScale - fCenter - m_fEntityRadius;

		if (fGateMin < 0.0f)
		{
			fX += fGateMin;
			fGateMax += fGateMin;
			fGateMin = 0.0f;
		}
		else if (fGateMax < 0.0f)
		{
			fX -= fGateMax;
			fGateMin += fGateMax;
			fGateMax = 0.0f;
		}

		cRet = CResultGate(CVec2f(fX, fY), fGateMax, fGateMin, pA->ParentDirection(), CVec4f(0.0f, 0.0f, 1.0f, 1.0f));
		break;
	case SD_EAST:
		fX = GridToCoord(uiX + 1);
		fY = GridToCoord(uiY) + fCenter;

		fGateMin = cGate.Min() * m_fGateScale + fCenter - m_fEntityRadius;
		fGateMax = cGate.Max() * m_fGateScale - fCenter - m_fEntityRadius;

		if (fGateMin < 0.0f)
		{
			fY += fGateMin;
			fGateMax += fGateMin;
			fGateMin = 0.0f;
		}
		else if (fGateMax < 0.0f)
		{
			fY -= fGateMax;
			fGateMin += fGateMax;
			fGateMax = 0.0f;
		}

		cRet = CResultGate(CVec2f(fX, fY), fGateMax, fGateMin, pA->ParentDirection(), CVec4f(0.0f, 0.0f, 1.0f, 1.0f));
		break;
	case SD_WEST:
		fX = GridToCoord(uiX);
		fY = GridToCoord(uiY) + fCenter;

		fGateMin = cGate.Min() * m_fGateScale - fCenter - m_fEntityRadius;
		fGateMax = cGate.Max() * m_fGateScale + fCenter - m_fEntityRadius;

		if (fGateMin < 0.0f)
		{
			fY -= fGateMin;
			fGateMax += fGateMin;
			fGateMin = 0.0f;
		}
		else if (fGateMax < 0.0f)
		{
			fY += fGateMax;
			fGateMin += fGateMax;
			fGateMax = 0.0f;
		}

		cRet = CResultGate(CVec2f(fX, fY), fGateMax, fGateMin, pA->ParentDirection(), CVec4f(0.0f, 0.0f, 1.0f, 1.0f));
		break;
	case SD_INVALID:
		fX = GridToCoord(uiX) + fCenter;
		fY = GridToCoord(uiY) + fCenter;

		fGateMin = 0.0f;
		fGateMax = 0.0f;
		break;
	default:
		//eh-ror
		//assert(0);
		break;
	};

	return cRet;
}


/*====================
  CNavigationGraph::Release
  ====================*/
void	CNavigationGraph::Release()
{
	SAFE_DELETE(m_pOpenNodesFromDst);
	SAFE_DELETE(m_pOpenNodesFromSrc);
	SAFE_DELETE_ARRAY(m_pNodeBucket);
	SAFE_DELETE_ARRAY(m_pGateBucket);
}


/*====================
  CNavigationGraph::Init
  ====================*/
bool	CNavigationGraph::Init(const CWorld* pWorld)
{
	Release();

	m_pWorld = pWorld;

	m_uiBucketWidth = pWorld->GetNavigationWidth();
	m_uiBucketHeight = pWorld->GetNavigationHeight();
	m_uiMaxNodeCount = m_uiBucketWidth * m_uiBucketHeight;

#if 0
	m_uiMaxQueueSize = MAX(m_uiMaxNodeCount >> 5, 256u);
#else
	m_uiMaxQueueSize = m_uiMaxNodeCount;
#endif

	if (!m_pOpenNodesFromSrc)
	{
		m_pOpenNodesFromSrc = K2_NEW(ctx_Nav, PrioritySearchQueue)();
		m_pOpenNodesFromSrc->Init(g_PathTimeoutNodes);
	}

	if (!m_pOpenNodesFromDst)
	{
		m_pOpenNodesFromDst = K2_NEW(ctx_Nav, PrioritySearchQueue)();
		m_pOpenNodesFromDst->Init(g_PathTimeoutNodes);
	}

	if (!m_pNodeBucket)
	{
		m_pNodeBucket = K2_NEW_ARRAY(ctx_Nav, CSearchNode, m_uiMaxNodeCount);

		//uint uiStart(K2System.Microseconds());
		memset(m_pNodeBucket, 0xFFFFFFFF, sizeof(CSearchNode) * m_uiMaxNodeCount);
		//uint uiStop(K2System.Microseconds());

		m_uiNavSize = pWorld->GetNavigationSize();
		m_uiBucketMaskX = (1 << m_uiNavSize) - 1;
		m_uiBucketMaskY = (m_uiBucketMaskX << m_uiNavSize);

		// reserves based on path timeout node-count
		m_vecNodesTraveled.reserve(TRAVELED_NODES_RESERVE);
		m_vResultGates.reserve(SEGMENT_WAYPOINT_RESERVE);
	}

	if (!m_pGateBucket)
	{
		m_pGateBucket = K2_NEW_ARRAY(ctx_Nav, CSearchGateR, m_uiMaxNodeCount);
	}

	m_vDirtySpans.resize(m_uiBucketWidth);

	CVec2us unClear(ushort(m_uiBucketWidth), 0);
	for (uint uiY(0); uiY < m_uiBucketWidth; ++uiY)
		m_vDirtySpans[uiY] = unClear;
	
	m_v2DirtyRegion = CVec2us(ushort(m_uiBucketHeight), 0);

	return true;
}


/*====================
  CNavigationGraph::ResetForNextSearch
  ====================*/
inline
void	CNavigationGraph::ResetForNextSearch()
{
	PROFILE("CNavigationGraph::ResetForNextSearch");

	CVec2us unClear(ushort(m_uiBucketWidth), 0);

	for (uint uiY(m_v2DirtyRegion.x); uiY <= m_v2DirtyRegion.y; ++uiY)
	{
		CVec2us &v2Span(m_vDirtySpans[uiY]);
		if (v2Span == unClear)
			continue;

		assert(v2Span.y >= v2Span.x);

		memset(&m_pNodeBucket[uiY * m_uiBucketWidth + v2Span.x], 0xFFFFFFFF, (v2Span.y - v2Span.x + 1) * sizeof(CSearchNode));

		v2Span = unClear;
	}

	// Reset the heap
	m_pOpenNodesFromDst->Reset();
	m_pOpenNodesFromSrc->Reset();

	// Reset the smoothing structures
	m_vecNodesTraveled.clear();
	m_vResultGates.clear();

	m_v2DirtyRegion = CVec2us(ushort(m_uiBucketHeight), 0);
}


/*====================
  CNavigationGraph::IsReset()
  ====================*/
void	CNavigationGraph::IsReset()
{
	CSearchNode *pNode(m_pNodeBucket);
	for (uint y(0); y < m_uiBucketHeight; ++y)
	{
		for (uint x(0); x<m_uiBucketWidth; ++x)
		{
			if (!pNode->IsReset())
				assert(0);
		}
	}
}


/*====================
  CNavigationGraph::AStarFromDstNorth
  ====================*/
inline
bool	CNavigationGraph::AStarFromDstNorth(CSearchNode *pCurrent, CSearchNode *pRelative, uint uiX, uint uiY, ESearchDirection eParentDirection)
{
	uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID ? 0 : (eParentDirection == SD_NORTH || eParentDirection == SD_SOUTH) ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));

	// check cost to see if we found a better route
	if (~pRelative->GetFlags() & SN_NOTFROMSRC || uiCost < pRelative->GetCost())
	{
		CSearchGateR cGate;

		uint uiBeginSegment(uiX << m_uiDownSize);
		uint uiEndSegment(uiBeginSegment + (1 << m_uiDownSize));

		if (m_pCurrentGrid->LinearGate(DIVIDE_HORIZONTAL, uiY, uiBeginSegment, uiEndSegment, m_iEntityWidthSegments, cGate))
		{
			GetGate(pRelative) = cGate;

			pRelative->ClearFlags(SN_NOTFROMDST);

			// Check if it is a valid result path
			if (~pRelative->GetFlags() & SN_NOTFROMSRC)
			{
				ConstructPath(pRelative, pRelative->ParentDirection(), CSearchNode::ParentDirectionReversed(SD_NORTH));
				return true;
			}

			uint uiDiagonal, uiLinear;
			TileDistanceV(pRelative, m_pInitSrc, uiDiagonal, uiLinear);

			// Set costs and relative information
			pRelative->SetDirection(SD_NORTH);
			pRelative->SetCost(uiCost);

			// Set Heuristic
			uint uiHeuristic(uiCost + (uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
			pRelative->SetHeuristic(uiHeuristic);
			pRelative->SetBias(SquaredDistance(pRelative, m_pInitSrc));

			if (pRelative->GetFlags() & SN_NOTLISTED)
			{
				pRelative->ClearFlags(SN_NOTLISTED);
				m_pOpenNodesFromDst->Push(pRelative);

				uint uiNewY(uiY + 1);

				if (uiNewY > m_v2DirtyRegion.y)
				{
					m_v2DirtyRegion.y = ushort(uiNewY);
					m_vDirtySpans[uiNewY] = CVec2us(uiX, uiX);
				}
				else
				{
					CVec2us &v2Span(m_vDirtySpans[uiNewY]);
					if (uiX < v2Span.x)
						v2Span.x = ushort(uiX);
					if (uiX > v2Span.y)
						v2Span.y = ushort(uiX);
				}
			}
			else
			{
				m_pOpenNodesFromDst->Update(pRelative);
			}
		}
	}

	return false;
}


/*====================
  CNavigationGraph::AStarFromDstEast
  ====================*/
inline
bool	CNavigationGraph::AStarFromDstEast(CSearchNode *pCurrent, CSearchNode *pRelative, uint uiX, uint uiY, ESearchDirection eParentDirection)
{
	uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID ? 0 : (eParentDirection == SD_EAST || eParentDirection == SD_WEST) ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));

	// check cost to see if we found a better route
	if (~pRelative->GetFlags() & SN_NOTFROMSRC || uiCost < pRelative->GetCost())
	{
		CSearchGateR cGate;

		uint uiBeginSegment(uiY << m_uiDownSize);
		uint uiEndSegment(uiBeginSegment + (1 << m_uiDownSize));

		if (m_pCurrentGrid->LinearGate(DIVIDE_VERTICAL, uiX, uiBeginSegment, uiEndSegment, m_iEntityWidthSegments, cGate))
		{
			GetGate(pRelative) = cGate;

			pRelative->ClearFlags(SN_NOTFROMDST);

			// Check if it is a valid result path
			if (~pRelative->GetFlags() & SN_NOTFROMSRC)
			{
				ConstructPath(pRelative, pRelative->ParentDirection(), CSearchNode::ParentDirectionReversed(SD_EAST));
				return true;
			}

			uint uiDiagonal, uiLinear;
			TileDistanceH(pRelative, m_pInitSrc, uiDiagonal, uiLinear);

			// Set costs and relative information
			pRelative->SetDirection(SD_EAST);
			pRelative->SetCost(uiCost);

			// Set Heuristic
			uint uiHeuristic(uiCost + (uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
			pRelative->SetHeuristic(uiHeuristic);
			pRelative->SetBias(SquaredDistance(pRelative, m_pInitSrc));

			if (pRelative->GetFlags() & SN_NOTLISTED)
			{
				pRelative->ClearFlags(SN_NOTLISTED);
				m_pOpenNodesFromDst->Push(pRelative);

				uint uiNewX(uiX + 1);

				CVec2us &v2Span(m_vDirtySpans[uiY]);
				if (uiNewX > v2Span.y)
					v2Span.y = ushort(uiNewX);
			}
			else
			{
				m_pOpenNodesFromDst->Update(pRelative);
			}
		}
	}

	return false;
}


/*====================
  CNavigationGraph::AStarFromDstWest
  ====================*/
inline
bool	CNavigationGraph::AStarFromDstWest(CSearchNode *pCurrent, CSearchNode *pRelative, uint uiX, uint uiY, ESearchDirection eParentDirection)
{
	uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID ? 0 : (eParentDirection == SD_EAST || eParentDirection == SD_WEST) ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));

	// check cost to see if we found a better route
	if (~pRelative->GetFlags() & SN_NOTFROMSRC || uiCost < pRelative->GetCost())
	{
		CSearchGateR cGate;

		uint uiBeginSegment(uiY << m_uiDownSize);
		uint uiEndSegment(uiBeginSegment + (1 << m_uiDownSize));

		if (m_pCurrentGrid->LinearGate(DIVIDE_VERTICAL, uiX - 1, uiBeginSegment, uiEndSegment, m_iEntityWidthSegments, cGate))
		{
			GetGate(pRelative) = cGate;

			pRelative->ClearFlags(SN_NOTFROMDST);

			// Check if it is a valid result path
			if (~pRelative->GetFlags() & SN_NOTFROMSRC)
			{
				ConstructPath(pRelative, pRelative->ParentDirection(), CSearchNode::ParentDirectionReversed(SD_WEST));
				return true;
			}

			uint uiDiagonal, uiLinear;
			TileDistanceH(pRelative, m_pInitSrc, uiDiagonal, uiLinear);

			// Set costs and relative information
			pRelative->SetDirection(SD_WEST);
			pRelative->SetCost(uiCost);

			// Set Heuristic
			uint uiHeuristic(uiCost + (uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
			pRelative->SetHeuristic(uiHeuristic);
			pRelative->SetBias(SquaredDistance(pRelative, m_pInitSrc));

			if (pRelative->GetFlags() & SN_NOTLISTED)
			{
				pRelative->ClearFlags(SN_NOTLISTED);
				m_pOpenNodesFromDst->Push(pRelative);

				uint uiNewX(uiX - 1);

				CVec2us &v2Span(m_vDirtySpans[uiY]);
				if (uiNewX < v2Span.x)
					v2Span.x = ushort(uiNewX);
			}
			else
			{
				m_pOpenNodesFromDst->Update(pRelative);
			}

			
		}
	}

	return false;
}


/*====================
  CNavigationGraph::AStarFromDstSouth
  ====================*/
inline
bool	CNavigationGraph::AStarFromDstSouth(CSearchNode *pCurrent, CSearchNode *pRelative, uint uiX, uint uiY, ESearchDirection eParentDirection)
{
	uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID ? 0 : (eParentDirection == SD_NORTH || eParentDirection == SD_SOUTH) ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));

	// check cost to see if we found a better route
	if (~pRelative->GetFlags() & SN_NOTFROMSRC || uiCost < pRelative->GetCost())
	{
		CSearchGateR cGate;

		uint uiBeginSegment(uiX << m_uiDownSize);
		uint uiEndSegment(uiBeginSegment + (1 << m_uiDownSize));

		if (m_pCurrentGrid->LinearGate(DIVIDE_HORIZONTAL, uiY - 1, uiBeginSegment, uiEndSegment, m_iEntityWidthSegments, cGate))
		{
			GetGate(pRelative) = cGate;

			pRelative->ClearFlags(SN_NOTFROMDST);

			// Check if it is a valid result path
			if (~pRelative->GetFlags() & SN_NOTFROMSRC)
			{
				ConstructPath(pRelative, pRelative->ParentDirection(), CSearchNode::ParentDirectionReversed(SD_SOUTH));
				return true;
			}

			uint uiDiagonal, uiLinear;
			TileDistanceV(pRelative, m_pInitSrc, uiDiagonal, uiLinear);

			// Set costs and relative information
			pRelative->SetDirection(SD_SOUTH);
			pRelative->SetCost(uiCost);

			// Set Heuristic
			uint uiHeuristic(uiCost + (uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
			pRelative->SetHeuristic(uiHeuristic);
			pRelative->SetBias(SquaredDistance(pRelative, m_pInitSrc));

			if (pRelative->GetFlags() & SN_NOTLISTED)
			{
				pRelative->ClearFlags(SN_NOTLISTED);
				m_pOpenNodesFromDst->Push(pRelative);

				uint uiNewY(uiY - 1);

				if (uiNewY < m_v2DirtyRegion.x)
				{
					m_v2DirtyRegion.x = ushort(uiNewY);
					m_vDirtySpans[uiNewY] = CVec2us(uiX, uiX);
				}
				else
				{
					CVec2us &v2Span(m_vDirtySpans[uiNewY]);
					if (uiX < v2Span.x)
						v2Span.x = ushort(uiX);
					if (uiX > v2Span.y)
						v2Span.y = ushort(uiX);
				}
			}
			else
			{
				m_pOpenNodesFromDst->Update(pRelative);
			}
		}
	}

	return false;
}


/*====================
  CNavigationGraph::AStarFromDst
  ====================*/
inline
bool	CNavigationGraph::AStarFromDst(uint uiSearchArea)
{
	while (!m_pOpenNodesFromDst->Empty())
	{
		// check for search-pause
		if (uiSearchArea-- == 0)
		{
			m_uiPathFound = ABORT_PATHING;
			return true;
		}

		// Top Node
		CSearchNode *pCurrent(m_pOpenNodesFromDst->Pop());

		// Flag for reset
		pCurrent->SetFlags(SN_NOTLISTED);

		uint uiX(GetNodeX(pCurrent)), uiY(GetNodeY(pCurrent));
		ESearchDirection eParentDirection(pCurrent->ParentDirection());
		
		//
		// Add neighbors
		//
		
		// SD_NORTH
		if (uiY < m_uiBucketHeight - 1) 
		{
			CSearchNode *pRelative(pCurrent + m_uiBucketWidth); // y + 1

			if (AStarFromDstNorth(pCurrent, pRelative, uiX, uiY, eParentDirection))
				return true;

			//
			// Add diagonals
			//

			if (~pRelative->GetFlags() & SN_NOTLISTED)
			{
				int iDiffX(GetNodeX(m_pInitSrc) - uiX);
				int iDiffY(GetNodeY(m_pInitSrc) - uiY);

				if (iDiffX > 0 && (iDiffX > ABS(iDiffY) || iDiffX == iDiffY))
				{
					uint uiX2(GetNodeX(pRelative)), uiY2(GetNodeY(pRelative));
					ESearchDirection eParentDirection2(pRelative->ParentDirection());

					// SD_NORTH->SD_EAST Diagonal
					if (uiX2 < m_uiBucketWidth - 1)
					{
						CSearchNode *pRelative2(pRelative + 1); // x + 1

						if (AStarFromDstEast(pRelative, pRelative2, uiX2, uiY2, eParentDirection2))
							return true;
					}
				}

				if (iDiffX < 0 && (-iDiffX > ABS(iDiffY) || -iDiffX == iDiffY))
				{
					uint uiX2(GetNodeX(pRelative)), uiY2(GetNodeY(pRelative));
					ESearchDirection eParentDirection2(pRelative->ParentDirection());

					// SD_NORTH->SD_WEST Diagonal
					if (uiX2 > 0)
					{
						CSearchNode *pRelative2(pRelative - 1); // x - 1

						if (AStarFromDstWest(pRelative, pRelative2, uiX2, uiY2, eParentDirection2))
							return true;
					}
				}
			}
		}

		// SD_EAST
		if (uiX < m_uiBucketWidth - 1)
		{
			CSearchNode *pRelative(pCurrent + 1); // x + 1

			if (AStarFromDstEast(pCurrent, pRelative, uiX, uiY, eParentDirection))
				return true;

			//
			// Add diagonals
			//

			if (~pRelative->GetFlags() & SN_NOTLISTED)
			{
				int iDiffX(ABS<int>(GetNodeX(m_pInitSrc) - uiX));
				int iDiffY(GetNodeY(m_pInitSrc) - uiY);

				if (iDiffY > 0 && iDiffY > iDiffX)
				{
					uint uiX2(GetNodeX(pRelative)), uiY2(GetNodeY(pRelative));
					ESearchDirection eParentDirection2(pRelative->ParentDirection());

					// SD_EAST->SD_NORTH Diagonal
					if (uiY2 < m_uiBucketHeight - 1) 
					{
						CSearchNode *pRelative2(pRelative + m_uiBucketWidth); // y + 1

						if (AStarFromDstNorth(pRelative, pRelative2, uiX2, uiY2, eParentDirection2))
							return true;
					}
				}

				if (iDiffY < 0 && -iDiffY > iDiffX)
				{
					uint uiX2(GetNodeX(pRelative)), uiY2(GetNodeY(pRelative));
					ESearchDirection eParentDirection2(pRelative->ParentDirection());

					// SD_EAST->SD_SOUTH Diagonal
					if (uiY2 > 0) 
					{
						CSearchNode *pRelative2(pRelative - m_uiBucketWidth); // y - 1

						if (AStarFromDstSouth(pRelative, pRelative2, uiX2, uiY2, eParentDirection2))
							return true;
					}
				}
			}
		}

		// SD_WEST
		if (uiX > 0)
		{
			CSearchNode *pRelative(pCurrent - 1); // x - 1

			if (AStarFromDstWest(pCurrent, pRelative, uiX, uiY, eParentDirection))
				return true;

			//
			// Add diagonals
			//

			if (~pRelative->GetFlags() & SN_NOTLISTED)
			{
				int iDiffX(ABS<int>(GetNodeX(m_pInitSrc) - uiX));
				int iDiffY(GetNodeY(m_pInitSrc) - uiY);

				if (iDiffY > 0 && iDiffY > iDiffX)
				{
					uint uiX2(GetNodeX(pRelative)), uiY2(GetNodeY(pRelative));
					ESearchDirection eParentDirection2(pRelative->ParentDirection());

					// SD_WEST->SD_NORTH Diagonal
					if (uiY2 < m_uiBucketHeight - 1) 
					{
						CSearchNode *pRelative2(pRelative + m_uiBucketWidth); // y + 1

						if (AStarFromDstNorth(pRelative, pRelative2, uiX2, uiY2, eParentDirection2))
							return true;
					}
				}

				if (iDiffY < 0 && -iDiffY > iDiffX)
				{
					uint uiX2(GetNodeX(pRelative)), uiY2(GetNodeY(pRelative));
					ESearchDirection eParentDirection2(pRelative->ParentDirection());

					// SD_WEST->SD_SOUTH Diagonal
					if (uiY2 > 0) 
					{
						CSearchNode *pRelative2(pRelative - m_uiBucketWidth); // y - 1

						if (AStarFromDstSouth(pRelative, pRelative2, uiX2, uiY2, eParentDirection2))
							return true;
					}
				}
			}
		}

		// SD_SOUTH
		if (uiY > 0)
		{
			CSearchNode *pRelative(pCurrent - m_uiBucketWidth);

			if (AStarFromDstSouth(pCurrent, pRelative, uiX, uiY, eParentDirection))
				return true;

			//
			// Add diagonals
			//

			if (~pRelative->GetFlags() & SN_NOTLISTED)
			{
				int iDiffX(GetNodeX(m_pInitSrc) - uiX);
				int iDiffY(GetNodeY(m_pInitSrc) - uiY);

				if (iDiffX > 0 && (iDiffX > ABS(iDiffY) || -iDiffX == iDiffY))
				{
					uint uiX2(GetNodeX(pRelative)), uiY2(GetNodeY(pRelative));
					ESearchDirection eParentDirection2(pRelative->ParentDirection());

					// SD_SOUTH->SD_EAST Diagonal
					if (uiX2 < m_uiBucketWidth - 1)
					{
						CSearchNode *pRelative2(pRelative + 1); // x + 1

						if (AStarFromDstEast(pRelative, pRelative2, uiX2, uiY2, eParentDirection2))
							return true;
					}
				}

				if (iDiffX < 0 && (-iDiffX > ABS(iDiffY) || iDiffX == iDiffY))
				{
					uint uiX2(GetNodeX(pRelative)), uiY2(GetNodeY(pRelative));
					ESearchDirection eParentDirection2(pRelative->ParentDirection());

					// SD_SOUTH->SD_WEST Diagonal
					if (uiX2 > 0)
					{
						CSearchNode *pRelative2(pRelative - 1); // x - 1

						if (AStarFromDstWest(pRelative, pRelative2, uiX2, uiY2, eParentDirection2))
							return true;
					}
				}
			}
		}
	}

	return false;
}


/*====================
  CNavigationGraph::FloodFromSrc
  ====================*/
inline
bool	CNavigationGraph::FloodFromSrc(uint uiSearchArea)
{
	while (!m_pOpenNodesFromSrc->Empty())
	{
		// Check for search-pause
		if (uiSearchArea-- == 0)
		{
			m_uiPathFound = ABORT_PATHING;
			return true;
		}

		// Top Node
		CSearchNode *pCurrent(m_pOpenNodesFromSrc->Pop());

		// Flag for reset
		pCurrent->SetFlags(SN_NOTLISTED);

		uint uiX(GetNodeX(pCurrent)), uiY(GetNodeY(pCurrent));
		ESearchDirection eParentDirection(pCurrent->ParentDirection());

		//
		// Add neighbors
		//

		// SD_NORTH
		if (uiY < m_uiBucketHeight - 1)
		{
			CSearchNode *pRelative(pCurrent + m_uiBucketWidth); // y + 1
			uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID || eParentDirection == SD_NORTH || eParentDirection == SD_SOUTH ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));
		
			// check cost to see if we found a better route
			if (~pRelative->GetFlags() & SN_NOTFROMDST || uiCost < pRelative->GetCost())
			{
				CSearchGateR cGate;

				// No valid way into the node from this direction
				if (FindGate(pRelative, CSearchNode::ParentDirectionReversed(SD_NORTH), cGate))
				{
					// Check if it is a valid result path
					//!!Gate is set by the other side of the search for the bridging node!!
					pRelative->ClearFlags(SN_NOTFROMSRC);
					if (~pRelative->GetFlags() & SN_NOTFROMDST)
					{
						// if this is the destination, we need the gate information
						if (pRelative->ParentDirection() == SD_INVALID)
							GetGate(pRelative) = cGate;

						ConstructPath(pRelative, CSearchNode::ParentDirectionReversed(SD_NORTH), pRelative->ParentDirection());
						return true;
					}

					// Set costs and relative information
					// !! Heuristic is simply the cost in the case of the Src-check - we want to expand in all directions to confirm area passability
					GetGate(pRelative) = cGate;
					pRelative->SetDirection(SD_NORTH);
					pRelative->SetCost(uiCost);
					pRelative->SetHeuristic(uiCost);
					pRelative->SetBias(0);

					if (pRelative->GetFlags() & SN_NOTLISTED)
					{
						pRelative->ClearFlags(SN_NOTLISTED);
						m_pOpenNodesFromSrc->Push(pRelative);

						uint uiNewY(uiY + 1);

						if (uiNewY > m_v2DirtyRegion.y)
						{
							m_v2DirtyRegion.y = ushort(uiNewY);
							m_vDirtySpans[uiNewY] = CVec2us(uiX, uiX);
						}
						else
						{
							CVec2us &v2Span(m_vDirtySpans[uiNewY]);
							if (uiX < v2Span.x)
								v2Span.x = ushort(uiX);
							if (uiX > v2Span.y)
								v2Span.y = ushort(uiX);
						}
					}
					else
					{
						m_pOpenNodesFromSrc->Update(pRelative);
					}
				}
			}
		}

		// SD_EAST
		if (uiX < m_uiBucketWidth - 1)
		{
			CSearchNode *pRelative(pCurrent + 1); // x + 1
			uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID || eParentDirection == SD_EAST || eParentDirection == SD_WEST ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));
		
			// check cost to see if we found a better route
			if (~pRelative->GetFlags() & SN_NOTFROMDST || uiCost < pRelative->GetCost())
			{
				CSearchGateR cGate;

				// No valid way into the node from this direction
				if (FindGate(pRelative, CSearchNode::ParentDirectionReversed(SD_EAST), cGate))
				{
					// Check if it is a valid result path
					//!!Gate is set by the other side of the search for the bridging node!!
					pRelative->ClearFlags(SN_NOTFROMSRC);
					if (~pRelative->GetFlags() & SN_NOTFROMDST)
					{
						// if this is the destination, we need the gate information
						if (pRelative->ParentDirection() == SD_INVALID)
							GetGate(pRelative) = cGate;

						ConstructPath(pRelative, CSearchNode::ParentDirectionReversed(SD_EAST), pRelative->ParentDirection());
						return true;
					}

					// Set costs and relative information
					// !! Heuristic is simply the cost in the case of the Src-check - we want to expand in all directions to confirm area passability
					GetGate(pRelative) = cGate;
					pRelative->SetDirection(SD_EAST);
					pRelative->SetCost(uiCost);
					pRelative->SetHeuristic(uiCost);
					pRelative->SetBias(0);

					if (pRelative->GetFlags() & SN_NOTLISTED)
					{
						pRelative->ClearFlags(SN_NOTLISTED);
						m_pOpenNodesFromSrc->Push(pRelative);

						uint uiNewX(uiX + 1);

						CVec2us &v2Span(m_vDirtySpans[uiY]);
						if (uiNewX > v2Span.y)
							v2Span.y = ushort(uiNewX);
					}
					else
					{
						m_pOpenNodesFromSrc->Update(pRelative);
					}
				}
			}
		}

		// SD_WEST
		if (uiX > 0)
		{
			CSearchNode *pRelative(pCurrent - 1); // x - 1
			uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID || eParentDirection == SD_EAST || eParentDirection == SD_WEST ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));
		
			// check cost to see if we found a better route
			if (~pRelative->GetFlags() & SN_NOTFROMDST || uiCost < pRelative->GetCost())
			{
				CSearchGateR cGate;

				// No valid way into the node from this direction
				if (FindGate(pRelative, CSearchNode::ParentDirectionReversed(SD_WEST), cGate))
				{
					// Check if it is a valid result path
					//!!Gate is set by the other side of the search for the bridging node!!
					pRelative->ClearFlags(SN_NOTFROMSRC);
					if (~pRelative->GetFlags() & SN_NOTFROMDST)
					{
						// if this is the destination, we need the gate information
						if (pRelative->ParentDirection() == SD_INVALID)
							GetGate(pRelative) = cGate;

						ConstructPath(pRelative, CSearchNode::ParentDirectionReversed(SD_WEST), pRelative->ParentDirection());
						return true;
					}

					// Set costs and relative information
					// !! Heuristic is simply the cost in the case of the Src-check - we want to expand in all directions to confirm area passability
					GetGate(pRelative) = cGate;
					pRelative->SetDirection(SD_WEST);
					pRelative->SetCost(uiCost);
					pRelative->SetHeuristic(uiCost);
					pRelative->SetBias(0);

					if (pRelative->GetFlags() & SN_NOTLISTED)
					{
						pRelative->ClearFlags(SN_NOTLISTED);
						m_pOpenNodesFromSrc->Push(pRelative);

						uint uiNewX(uiX - 1);

						CVec2us &v2Span(m_vDirtySpans[uiY]);
						if (uiNewX < v2Span.x)
							v2Span.x = ushort(uiNewX);
					}
					else
					{
						m_pOpenNodesFromSrc->Update(pRelative);
					}
				}
			}
		}

		// SD_SOUTH
		if (uiY > 0)
		{
			CSearchNode *pRelative(pCurrent - m_uiBucketWidth);
			uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID || eParentDirection == SD_NORTH || eParentDirection == SD_SOUTH ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));
		
			// check cost to see if we found a better route
			if (~pRelative->GetFlags() & SN_NOTFROMDST || uiCost < pRelative->GetCost())
			{
				CSearchGateR cGate;

				// No valid way into the node from this direction
				if (FindGate(pRelative, CSearchNode::ParentDirectionReversed(SD_SOUTH), cGate))
				{
					// Check if it is a valid result path
					//!!Gate is set by the other side of the search for the bridging node!!
					pRelative->ClearFlags(SN_NOTFROMSRC);
					if (~pRelative->GetFlags() & SN_NOTFROMDST)
					{
						// if this is the destination, we need the gate information
						if (pRelative->ParentDirection() == SD_INVALID)
							GetGate(pRelative) = cGate;

						ConstructPath(pRelative, CSearchNode::ParentDirectionReversed(SD_SOUTH), pRelative->ParentDirection());
						return true;
					}

					// Set costs and relative information
					// !! Heuristic is simply the cost in the case of the Src-check - we want to expand in all directions to confirm area passability
					GetGate(pRelative) = cGate;
					pRelative->SetDirection(SD_SOUTH);
					pRelative->SetCost(uiCost);
					pRelative->SetHeuristic(uiCost);
					pRelative->SetBias(0);

					if (pRelative->GetFlags() & SN_NOTLISTED)
					{
						pRelative->ClearFlags(SN_NOTLISTED);
						m_pOpenNodesFromSrc->Push(pRelative);

						uint uiNewY(uiY - 1);

						if (uiNewY < m_v2DirtyRegion.x)
						{
							m_v2DirtyRegion.x = ushort(uiNewY);
							m_vDirtySpans[uiNewY] = CVec2us(uiX, uiX);
						}
						else
						{
							CVec2us &v2Span(m_vDirtySpans[uiNewY]);
							if (uiX < v2Span.x)
								v2Span.x = ushort(uiX);
							if (uiX > v2Span.y)
								v2Span.y = ushort(uiX);
						}
					}
					else
					{
						m_pOpenNodesFromSrc->Update(pRelative);
					}
				}
			}
		}
	}

	return false;
}



/*====================
  CNavigationGraph::AStarFromSrc
  ====================*/
inline
bool	CNavigationGraph::AStarFromSrc(uint uiSearchArea)
{
	uint uiDiagonal, uiLinear;

	while (!m_pOpenNodesFromSrc->Empty())
	{
		// Check for search-pause
		if (uiSearchArea-- == 0)
		{
			m_uiPathFound = ABORT_PATHING;
			return true;
		}

		// Top Node
		CSearchNode *pCurrent(m_pOpenNodesFromSrc->Pop());

		// Flag for reset
		pCurrent->SetFlags(SN_NOTLISTED);

		uint uiX(GetNodeX(pCurrent)), uiY(GetNodeY(pCurrent));
		ESearchDirection eParentDirection(pCurrent->ParentDirection());

		//
		// Add neighbors
		//

		// SD_NORTH
		if (uiY < m_uiBucketHeight - 1)
		{
			CSearchNode *pRelative(pCurrent + m_uiBucketWidth); // y + 1
			uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID || eParentDirection == SD_NORTH || eParentDirection == SD_SOUTH ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));
		
			// check cost to see if we found a better route
			if (~pRelative->GetFlags() & SN_NOTFROMDST || uiCost < pRelative->GetCost())
			{
				CSearchGateR cGate;

				// No valid way into the node from this direction
				if (FindGate(pRelative, CSearchNode::ParentDirectionReversed(SD_NORTH), cGate))
				{
					GetGate(pRelative) = cGate;

					pRelative->ClearFlags(SN_NOTFROMSRC);

					// Check if it is a valid result path
					if (~pRelative->GetFlags() & SN_NOTFROMDST)
					{
						ConstructPath(pRelative, CSearchNode::ParentDirectionReversed(SD_NORTH), pRelative->ParentDirection());
						return true;
					}

					TileDistanceV(pRelative, m_pInitDst, uiDiagonal, uiLinear);

					// Set costs and relative information
					pRelative->SetDirection(SD_NORTH);
					pRelative->SetCost(uiCost);

					// Set Heuristic
					uint uiHeuristic(uiCost + (uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
					pRelative->SetHeuristic(uiHeuristic);
					pRelative->SetBias(SquaredDistance(pRelative, m_pInitDst));

					if (pRelative->GetFlags() & SN_NOTLISTED)
					{
						pRelative->ClearFlags(SN_NOTLISTED);
						m_pOpenNodesFromSrc->Push(pRelative);

						uint uiNewY(uiY + 1);

						if (uiNewY > m_v2DirtyRegion.y)
						{
							m_v2DirtyRegion.y = ushort(uiNewY);
							m_vDirtySpans[uiNewY] = CVec2us(uiX, uiX);
						}
						else
						{
							CVec2us &v2Span(m_vDirtySpans[uiNewY]);
							if (uiX < v2Span.x)
								v2Span.x = ushort(uiX);
							if (uiX > v2Span.y)
								v2Span.y = ushort(uiX);
						}
					}
					else
					{
						m_pOpenNodesFromSrc->Update(pRelative);
					}
				}
			}
		}

		// SD_EAST
		if (uiX < m_uiBucketWidth - 1)
		{
			CSearchNode *pRelative(pCurrent + 1); // x + 1
			uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID || eParentDirection == SD_EAST || eParentDirection == SD_WEST ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));
		
			// check cost to see if we found a better route
			if (~pRelative->GetFlags() & SN_NOTFROMDST || uiCost < pRelative->GetCost())
			{
				CSearchGateR cGate;

				// No valid way into the node from this direction
				if (FindGate(pRelative, CSearchNode::ParentDirectionReversed(SD_EAST), cGate))
				{
					GetGate(pRelative) = cGate;

					pRelative->ClearFlags(SN_NOTFROMSRC);

					// Check if it is a valid result path
					if (~pRelative->GetFlags() & SN_NOTFROMDST)
					{
						ConstructPath(pRelative, CSearchNode::ParentDirectionReversed(SD_EAST), pRelative->ParentDirection());
						return true;
					}

					TileDistanceH(pRelative, m_pInitDst, uiDiagonal, uiLinear);

					// Set costs and relative information
					pRelative->SetDirection(SD_EAST);
					pRelative->SetCost(uiCost);

					// Set Heuristic
					uint uiHeuristic(uiCost + (uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
					pRelative->SetHeuristic(uiHeuristic);
					pRelative->SetBias(SquaredDistance(pRelative, m_pInitDst));

					if (pRelative->GetFlags() & SN_NOTLISTED)
					{
						pRelative->ClearFlags(SN_NOTLISTED);
						m_pOpenNodesFromSrc->Push(pRelative);

						uint uiNewX(uiX + 1);

						CVec2us &v2Span(m_vDirtySpans[uiY]);
						if (uiNewX > v2Span.y)
							v2Span.y = ushort(uiNewX);
					}
					else
					{
						m_pOpenNodesFromSrc->Update(pRelative);
					}
				}
			}
		}

		// SD_WEST
		if (uiX > 0)
		{
			CSearchNode *pRelative(pCurrent - 1); // x - 1
			uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID || eParentDirection == SD_EAST || eParentDirection == SD_WEST ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));
		
			// check cost to see if we found a better route
			if (~pRelative->GetFlags() & SN_NOTFROMDST || uiCost < pRelative->GetCost())
			{
				CSearchGateR cGate;

				// No valid way into the node from this direction
				if (FindGate(pRelative, CSearchNode::ParentDirectionReversed(SD_WEST), cGate))
				{
					GetGate(pRelative) = cGate;

					pRelative->ClearFlags(SN_NOTFROMSRC);

					// Check if it is a valid result path
					if (~pRelative->GetFlags() & SN_NOTFROMDST)
					{
						ConstructPath(pRelative, CSearchNode::ParentDirectionReversed(SD_WEST), pRelative->ParentDirection());
						return true;
					}

					TileDistanceH(pRelative, m_pInitDst, uiDiagonal, uiLinear);

					// Set costs and relative information
					pRelative->SetDirection(SD_WEST);
					pRelative->SetCost(uiCost);

					// Set Heuristic
					uint uiHeuristic(uiCost + (uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
					pRelative->SetHeuristic(uiHeuristic);
					pRelative->SetBias(SquaredDistance(pRelative, m_pInitDst));

					if (pRelative->GetFlags() & SN_NOTLISTED)
					{
						pRelative->ClearFlags(SN_NOTLISTED);
						m_pOpenNodesFromSrc->Push(pRelative);

						uint uiNewX(uiX - 1);

						CVec2us &v2Span(m_vDirtySpans[uiY]);
						if (uiNewX < v2Span.x)
							v2Span.x = ushort(uiNewX);
					}
					else
					{
						m_pOpenNodesFromSrc->Update(pRelative);
					}
				}
			}
		}

		// SD_SOUTH
		if (uiY > 0)
		{
			CSearchNode *pRelative(pCurrent - m_uiBucketWidth);
			uint uiCost(pCurrent->GetCost() + (eParentDirection == SD_INVALID || eParentDirection == SD_NORTH || eParentDirection == SD_SOUTH ? LINEAR_WEIGHT : DIAGONAL_WEIGHT));
		
			// check cost to see if we found a better route
			if (~pRelative->GetFlags() & SN_NOTFROMDST || uiCost < pRelative->GetCost())
			{
				CSearchGateR cGate;

				// No valid way into the node from this direction
				if (FindGate(pRelative, CSearchNode::ParentDirectionReversed(SD_SOUTH), cGate))
				{
					GetGate(pRelative) = cGate;

					pRelative->ClearFlags(SN_NOTFROMSRC);

					// Check if it is a valid result path
					if (~pRelative->GetFlags() & SN_NOTFROMDST)
					{
						ConstructPath(pRelative, CSearchNode::ParentDirectionReversed(SD_SOUTH), pRelative->ParentDirection());
						return true;
					}

					TileDistanceV(pRelative, m_pInitDst, uiDiagonal, uiLinear);

					// Set costs and relative information
					pRelative->SetDirection(SD_SOUTH);
					pRelative->SetCost(uiCost);

					// Set Heuristic
					uint uiHeuristic(uiCost + (uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
					pRelative->SetHeuristic(uiHeuristic);
					pRelative->SetBias(SquaredDistance(pRelative, m_pInitDst));

					if (pRelative->GetFlags() & SN_NOTLISTED)
					{
						pRelative->ClearFlags(SN_NOTLISTED);
						m_pOpenNodesFromSrc->Push(pRelative);

						uint uiNewY(uiY - 1);

						if (uiNewY < m_v2DirtyRegion.x)
						{
							m_v2DirtyRegion.x = ushort(uiNewY);
							m_vDirtySpans[uiNewY] = CVec2us(uiX, uiX);
						}
						else
						{
							CVec2us &v2Span(m_vDirtySpans[uiNewY]);
							if (uiX < v2Span.x)
								v2Span.x = ushort(uiX);
							if (uiX > v2Span.y)
								v2Span.y = ushort(uiX);
						}
					}
					else
					{
						m_pOpenNodesFromSrc->Update(pRelative);
					}
				}
			}
		}
	}

	return false;
}


/*====================
  CNavigationGraph::BidirectionalAStar
  ====================*/
bool	CNavigationGraph::BidirectionalAStar()
{
	PROFILE("CNavigationGraph::BidirectionalAStar");

	int iChanges(g_PathBidirectionalChanges);
	uint uiNodesSearched(0);

	// Reset the search count, and remember to cleanup the source node (it is never placed in the queue)
	m_uiPathFound = 0;
	m_uiDistanceEst = 1;
	m_uiDirectionEst = 0;
	m_pInitSrc = m_pSrcNode;
	m_pInitDst = m_pDstNode;
	m_uiSrcDist = 1;
	m_uiSrcDirEst = 0;

	while (iChanges)
	{
		uiNodesSearched += g_PathBidirectionalSearchArea;
		if (!AStarFromDst(g_PathBidirectionalSearchArea))
		{
			if (iChanges != g_PathBidirectionalChanges || !g_PathAllowEstDst)
				return false;

			EstimateDestination();

			if (m_uiPathFound)
				return true;

			continue;
		}

		if (m_uiPathFound == VALID_PATH)
			return true;
		m_uiPathFound = 0;

		uiNodesSearched += g_PathBidirectionalSearchArea;
		if (!FloodFromSrc(g_PathBidirectionalSearchArea))
		{
			if (iChanges != g_PathBidirectionalChanges || !g_PathAllowEstSrc)
				return false;

			do
			{
				EstimateSource();

				if (m_uiPathFound)
					return true;

			} while (!FloodFromSrc(g_PathBidirectionalSearchArea));
		}

		if (m_uiPathFound == VALID_PATH)
			return true;

		m_uiPathFound = 0;
		--iChanges;
	}

	return AStarFromDst(g_PathTimeoutNodes - uiNodesSearched);
}


/*====================
  CNavigationGraph::AStar
  ====================*/
bool	CNavigationGraph::AStar()
{
	PROFILE("CNavigationGraph::AStar");

	// Reset the search count, and remember to cleanup the source node (it is never placed in the queue)
	m_uiPathFound = 0;
	m_uiDistanceEst = 1;
	m_uiDirectionEst = 0;
	m_pInitSrc = m_pSrcNode;
	m_pInitDst = m_pDstNode;
	m_pBestDst = m_pInitSrc;
	m_uiSrcDist = 1;
	m_uiSrcDirEst = 0;

	AStarFromSrc(g_PathTimeoutNodes);

	return m_uiPathFound == VALID_PATH;
}


/*====================
  CNavigationGraph::Flood
  ====================*/
bool	CNavigationGraph::Flood()
{
	PROFILE("CNavigationGraph::Flood");

	// Reset the search count, and remember to cleanup the source node (it is never placed in the queue)
	m_uiPathFound = 0;
	m_uiDistanceEst = 1;
	m_uiDirectionEst = 0;
	m_pInitSrc = m_pSrcNode;
	m_pInitDst = m_pDstNode;
	m_pBestDst = m_pInitSrc;
	m_uiSrcDist = 1;
	m_uiSrcDirEst = 0;

	FloodFromSrc(g_PathTimeoutNodes);

	return m_uiPathFound == VALID_PATH;
}


/*====================
  CNavigationGraph::ConstructTraveledPath
  ====================*/
void	CNavigationGraph::ConstructTraveledPath(PathResult &vecPath)
{
	PROFILE("CNavigationGraph::ConstructTraveledPath");

	uint uiPtStart(0), uiPtTestTo(1);

	if (m_vResultGates.size() == 0)
		return;

	uint uiLimit(uint(m_vResultGates.size() - 1));

	vecPath.push_back(m_vResultGates[0]);

	while (true)
	{
		CResultGate cResult;
		bool bLineOfSight(false);

		uiPtTestTo = uiLimit;

		while (uiPtTestTo > uiPtStart + 1)
		{
			if (bLineOfSight = LineOfSight(uiPtStart, uiPtTestTo, cResult))
				break;

			--uiPtTestTo;
		}

		vecPath.push_back(m_vResultGates[uiPtTestTo]);

		if (uiPtTestTo == uiLimit)
			break;

		uiPtStart = uiPtTestTo;
	}
}


/*====================
  CNavigationGraph::ConstructPath
  ====================*/
void	CNavigationGraph::ConstructPath(CSearchNode *pContactNode, int iSrcDirection, int iDstDirection)
{
	PROFILE("CNavigationGraph::ConstructPath");

	CSearchNode *pCurrent(NULL);

	assert (m_uiPathFound == 0);
	m_uiPathFound = 1;

	if (iSrcDirection != SD_INVALID)
	{
		pContactNode->SetDirection(iSrcDirection, false);
		pCurrent = pContactNode;
		while (1)
		{
			if (m_vecNodesTraveled.size() == TRAVELED_NODES_RESERVE)
			{
				Console << _T("Long path") << newl;
				break;
			}

			m_vecNodesTraveled.push_back(pCurrent);

			if (pCurrent->ParentDirection() == SD_INVALID)
			{
				m_pSrcNode = pCurrent;
				break;
			}

			pCurrent = FindNeighbor(pCurrent, pCurrent->ParentDirection());
		}

		// reverse vector entries and swap node directions
		{
			uint uiBegin(0), uiEnd((uint)m_vecNodesTraveled.size() - 1);

			while (uiBegin < uiEnd)
			{
				SWAP(m_vecNodesTraveled[uiBegin], m_vecNodesTraveled[uiEnd]);

				// Reverses the dir
				m_vecNodesTraveled[uiBegin]->ReverseDirection();
				m_vecNodesTraveled[uiEnd]->ReverseDirection();

				++uiBegin;
				--uiEnd;
			}

			if (uiBegin == uiEnd)
				m_vecNodesTraveled[uiBegin]->ReverseDirection();
		}

		{
			SearchNodeList::const_iterator cit(m_vecNodesTraveled.begin()), citNext, citEnd(m_vecNodesTraveled.end());
			citNext = cit+1;
			for (; citNext != citEnd; ++cit, ++citNext)
			{
				CSearchGateR &cCurrent(GetGate(*cit));

				(*cit)->SetDirection((*citNext)->ParentDirection(), false);
				cCurrent = GetGate(*citNext);

				if ((*cit)->ParentDirection() == SD_WEST || (*cit)->ParentDirection() == SD_NORTH)
					cCurrent.SwapExtentDirection();
			}
			(*cit)->SetDirection(SD_INVALID, false);
		}
	}

	if (iDstDirection != SD_INVALID)
	{
		if (m_vecNodesTraveled.size())
			m_vecNodesTraveled.pop_back();

		pContactNode->SetDirection(iDstDirection, false);

		pCurrent = pContactNode;
		while (1)
		{
			if (m_vecNodesTraveled.size() == TRAVELED_NODES_RESERVE)
			{
				Console << _T("Long path") << newl;
				break;
			}

			m_vecNodesTraveled.push_back(pCurrent);

			if (pCurrent->ParentDirection() == SD_WEST || pCurrent->ParentDirection() == SD_NORTH)
			{
				GetGate(pCurrent).SwapExtentDirection();
			}
			else if (pCurrent->ParentDirection() == SD_INVALID)
			{
				m_pDstNode = pCurrent;
				break;
			}

			pCurrent = FindNeighbor(pCurrent, pCurrent->ParentDirection());
		}
	}

	// Begin Result assembly
	if (g_PathDetail)
	{
		PathResult &vecPath(m_pPathResult->GetSimpleResult());
		SearchNodeList::const_iterator cit(m_vecNodesTraveled.begin()), citEnd(m_vecNodesTraveled.end());

		vecPath.push_back(CResultGate(CVec2f(m_fSrcX, m_fSrcY), 0.0f, 0.0f, SD_INVALID, CVec4f(0.0f, 0.0f, 1.0f, 1.0f)));

		for (; cit != citEnd; ++cit)
			vecPath.push_back(BuildResult(*cit));

		if (m_pDstNode == m_pInitDst)
			vecPath.back() = CResultGate(CVec2f(m_fGoalX, m_fGoalY), 0.0f, 0.0f, SD_INVALID, CVec4f(0.0f, 0.0f, 1.0f, 1.0f));
		else
		{
			float fCenter(m_fGateScale * pow(2.0f, float(m_uiDownSize)) * 0.5f);
			uint uiX(GetNodeX(m_pDstNode)), uiY(GetNodeY(m_pDstNode));

			vecPath.back() = CResultGate(CVec2f(GridToCoord(uiX) + fCenter, GridToCoord(uiY) + fCenter), 0.0f, 0.0f, SD_INVALID, CVec4f(0.0f, 0.0f, 1.0f, 1.0f));
		}
	}

	{
		SearchNodeList::const_iterator cit(m_vecNodesTraveled.begin()), citEnd(m_vecNodesTraveled.end());

		m_vResultGates.push_back(CResultGate(CVec2f(m_fSrcX, m_fSrcY), 0.0f, 0.0f, SD_INVALID, CVec4f(0.0f, 0.0f, 1.0f, 1.0f)));

		for (; cit != citEnd; ++cit)
			m_vResultGates.push_back(BuildResult(*cit));

		if (m_pDstNode == m_pInitDst)
			m_vResultGates.back() = CResultGate(CVec2f(m_fGoalX, m_fGoalY), 0.0f, 0.0f, SD_INVALID, CVec4f(0.0f, 0.0f, 1.0f, 1.0f));
		else
		{
			float fCenter(m_fGateScale * pow(2.0f, float(m_uiDownSize)) * 0.5f);
			uint uiX(GetNodeX(m_pDstNode)), uiY(GetNodeY(m_pDstNode));

			m_vResultGates.back() = CResultGate(CVec2f(GridToCoord(uiX) + fCenter, GridToCoord(uiY) + fCenter), 0.0f, 0.0f, SD_INVALID, CVec4f(0.0f, 0.0f, 1.0f, 1.0f));
		}
	}

	ConstructTraveledPath(m_pPathResult->GetSmoothResult());
}


/*====================
  CNavigationGraph::FindPath
  ====================*/
PoolHandle	CNavigationGraph::FindPath(float fSrcX, float fSrcY, float fEntityRadius, uint uiNavigationFlags, float fGoalX, float fGoalY, float fGoalRange, vector<PoolHandle> *pBlockers)
{
	PROFILE("CNavigationGraph::FindPath");

	PoolHandle hRet(INVALID_POOL_HANDLE);
	uint iSrcX, iSrcY, iDstX, iDstY;
	CNavigationMap &cNavMap(m_pWorld->GetNavigationMap());
	float fNavigationScale(m_pWorld->GetNavigationScale());

	m_fSrcX = CLAMP<float>(fSrcX, 0.0f, m_pWorld->GetWorldWidth() - 0.001f);
	m_fGoalX = CLAMP<float>(fGoalX, 0.0f, m_pWorld->GetWorldWidth() - 0.001f);
	m_fSrcY = CLAMP<float>(fSrcY, 0.0f, m_pWorld->GetWorldHeight() - 0.001f);
	m_fGoalY = CLAMP<float>(fGoalY, 0.0f, m_pWorld->GetWorldHeight() - 0.001f);

	if (m_pNodeBucket == NULL)
		return INVALID_POOL_HANDLE;

	if (g_PathDelayReset)
		ResetForNextSearch();

	int iDistanceX, iDistanceY;
	int iGoalDistanceX(64), iGoalDistanceY(64);

	hRet = m_pWorld->NewPath();

	if (hRet == INVALID_POOL_HANDLE)
		return hRet;

	m_pPathResult = m_pWorld->AccessPath(hRet);
	m_pPathResult->ReserveResult();

	iSrcX = INT_FLOOR(m_fSrcX / fNavigationScale);
	iSrcY = INT_FLOOR(m_fSrcY / fNavigationScale);
	iDstX = INT_FLOOR(m_fGoalX / fNavigationScale);
	iDstY = INT_FLOOR(m_fGoalY / fNavigationScale);

	m_fGateScale = fNavigationScale;
	m_fEntityRadius = fEntityRadius;
	m_iEntityWidthSegments = INT_CEIL(fEntityRadius / fNavigationScale * 2.0f); // Entity width in gate segments rounded up

	m_uiDownSize = 0;

	iDistanceX = iSrcX > iDstX ? iSrcX - iDstX : iDstX - iSrcX;
	iDistanceY = iSrcY > iDstY ? iSrcY - iDstY : iDstY - iSrcY;

	while (m_uiDownSize < g_PathMaxDownsize && (iDistanceX > iGoalDistanceX || iDistanceY > iGoalDistanceY))
	{
		++m_uiDownSize;
		iSrcY >>= 1;
		iSrcX >>= 1;
		iDstY >>= 1;
		iDstX >>= 1;

		iDistanceX = iSrcX > iDstX ? iSrcX - iDstX : iDstX - iSrcX;
		iDistanceY = iSrcY > iDstY ? iSrcY - iDstY : iDstY - iSrcY;
	};

	m_fInverseSearchScale = 1.0f / (fNavigationScale * (1 << m_uiDownSize));

	assert(iSrcY < m_uiBucketHeight && iDstY < m_uiBucketHeight);
	assert(iSrcX < m_uiBucketWidth && iDstX < m_uiBucketWidth);

	// Seek the src/dst nodes
	m_pSrcNode = &m_pNodeBucket[iSrcY * m_uiBucketWidth + iSrcX];
	m_pDstNode = &m_pNodeBucket[iDstY * m_uiBucketWidth + iDstX];

	m_uiGoalRange = MAX(INT_FLOOR(fGoalRange / fNavigationScale / (1 << m_uiDownSize)) - 1, 0);
	m_uiPathFound = 0;

	// Set per-search vars
	m_pCurrentGrid = cNavMap.PrepForSearch(uiNavigationFlags, m_uiDownSize);

	if (m_pSrcNode == m_pDstNode || m_pCurrentGrid == NULL)
	{
		m_pPathResult->GetSmoothResult().push_back(CResultGate());
		m_pPathResult->GetSmoothResult().push_back(CResultGate());
		m_pPathResult->SealPath(m_fSrcX, m_fSrcY, m_fGoalX, m_fGoalY);
	}
	else
	{
		m_v2DirtyRegion.x = MIN(iSrcY, iDstY);
		m_v2DirtyRegion.y = MAX(iSrcY, iDstY);

		if (iSrcY == iDstY)
		{
			m_vDirtySpans[iSrcY].x = MIN(iSrcX, iDstX);
			m_vDirtySpans[iSrcY].y = MAX(iSrcX, iDstX);
		}
		else
		{
			m_vDirtySpans[iSrcY] = CVec2us(iSrcX, iSrcX);
			m_vDirtySpans[iDstY] = CVec2us(iDstX, iDstX);
		}

		m_pSrcNode->ClearFlags(SN_NOTFROMSRC);
		m_pSrcNode->ClearFlags(SN_NOTLISTED);
		m_pSrcNode->SetCost(0);
		m_pSrcNode->SetHeuristic(TileDistance(m_pSrcNode, m_pDstNode));
		m_pSrcNode->SetBias(SquaredDistance(m_pSrcNode, m_pDstNode));
		m_pOpenNodesFromSrc->Push(m_pSrcNode);

		if (m_uiGoalRange > 0)
		{
			MarkDestinationArea(iDstX, iDstY, m_uiGoalRange);
		}
		else if (pBlockers != NULL && !pBlockers->empty())
		{
			MarkDestinationArea(*pBlockers);
		}
		else
		{
			m_pDstNode->ClearFlags(SN_NOTFROMDST);
			m_pDstNode->ClearFlags(SN_NOTLISTED);
			m_pDstNode->SetCost(0);

			uint uiDiagonal, uiLinear;
			TileDistance(m_pSrcNode, m_pDstNode, uiDiagonal, uiLinear);

			m_pDstNode->SetHeuristic((uiDiagonal * DIAGONAL_WEIGHT + uiLinear * LINEAR_WEIGHT) * H_WEIGHT_NUMERATOR / H_WEIGHT_DENOMINATOR);
			m_pDstNode->SetBias(SquaredDistance(m_pSrcNode, m_pDstNode));
			m_pOpenNodesFromDst->Push(m_pDstNode);
		}

		if (m_uiPathFound == 0)
		{
			// assertions to confirm the start/end nodes have been reset
			assert(m_pSrcNode->ParentDirection() == SD_INVALID);
			assert(m_pDstNode->ParentDirection() == SD_INVALID);

#if 1
			BidirectionalAStar();
#elif 1
			Flood();
#else
			AStar();
#endif
		}

		if (m_uiPathFound == ABORT_PATHING)
		{
			//Console << _T("Search aborted after ") << (K2System.Microseconds() - m_uiSearchStart) << _T("us") << newl;
			m_pPathResult = NULL;
			m_pWorld->FreePath(hRet);
			hRet = INVALID_POOL_HANDLE;
		}
		else
		{
			m_pPathResult->SealPath(m_fSrcX, m_fSrcY);
		}
	}

	if (!g_PathDelayReset)
		ResetForNextSearch();

#if 0
	if (m_pPathResult != NULL)
		Console << _T("Goal Range(") << fGoalRange << _T(") Path Range(") << Distance(m_pPathResult->GetGoal(), CVec2f(m_fGoalX, m_fGoalY)) << _T(")") << newl;
#endif

	return hRet;
}
