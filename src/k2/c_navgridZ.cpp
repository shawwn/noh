// (C)2009 S2 Games
// c_navgridZ.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_navigationmap.h"
#include "c_navigationgraph.h"
#include "c_navgridZ.h"
#include "c_world.h"
#include "c_blockpool.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CBlockPool CNavGridZ::s_cMemPool;
//=============================================================================

/*====================
  CNavGridZ::Alloc
  ====================*/
void    CNavGridZ::Alloc(uint uiWorldNavSize)
{
    // uiElements: add one grid for grow room, shift by num grid resolutions, double for horiz/vert allocation
    //((ENAVGRID_COUNT + 1) << (NUM_GRID_RESOLUTIONS + 1))
    s_cMemPool.Init(((ENAVGRID_COUNT + 1) << (NUM_GRID_RESOLUTIONS + 2)), (uiWorldNavSize << 1) - DOWNSIZE_LIMIT);
}


/*====================
  CNavGridZ::ReleaseAlloc
  ====================*/
void    CNavGridZ::ReleaseAlloc()
{
    s_cMemPool.ReleaseAll();
}


/*====================
  CNavGridZ::CNavGridZ
  ====================*/
CNavGridZ::CNavGridZ() :
m_uiCnxnWidth(0),
m_uiCnxnHeight(0),
m_uiDownSize(0),
m_pHorizontal(NULL),
m_pVertical(NULL),
m_uiIntsPerColumn(0),
m_uiIntsPerRow(0)
{
}

CNavGridZ::CNavGridZ(uint uiWidth, uint uiHeight, uint uiDownSize)
: m_uiCnxnWidth(uiWidth), m_uiCnxnHeight(uiHeight), m_uiDownSize(uiDownSize), m_pHorizontal(NULL), m_pVertical(NULL)
{
    m_uiIntsPerRow = m_uiCnxnWidth >> BSHIFT_BITS_PER_INT << uiDownSize;
    m_uiIntsPerColumn = m_uiCnxnHeight >> BSHIFT_BITS_PER_INT << uiDownSize;
}


/*====================
  CNavGridZ::Init
  ====================*/
void    CNavGridZ::Init(uint uiPermanentGrid)
{
    if (uiPermanentGrid)
    {
        m_pHorizontal = s_cMemPool.NewLongTerm<uint *>((1 << DOWNSIZE_LIMIT) >> m_uiDownSize);
        m_pVertical = s_cMemPool.NewLongTerm<uint *>((1 << DOWNSIZE_LIMIT) >> m_uiDownSize);
        Reset();
    }
    else
    {
        m_pHorizontal = s_cMemPool.NewShortTerm<uint *>((1 << DOWNSIZE_LIMIT) >> m_uiDownSize);
        m_pVertical = s_cMemPool.NewShortTerm<uint *>((1 << DOWNSIZE_LIMIT) >> m_uiDownSize);
    }

    if (m_pHorizontal == NULL || m_pVertical == NULL)
        K2System.Error(_T("CNavGridZ::Init: Insufficient pool"));
}


/*====================
  CNavGridZ::Done
  ====================*/
void    CNavGridZ::Done()
{
    if (m_pHorizontal != NULL)
    {
        s_cMemPool.Free(m_pHorizontal, ((1 << DOWNSIZE_LIMIT) >> m_uiDownSize));
        m_pHorizontal = NULL;
    }

    if (m_pVertical != NULL)
    {
        s_cMemPool.Free(m_pVertical, ((1 << DOWNSIZE_LIMIT) >> m_uiDownSize));
        m_pVertical = NULL;
    }
}

#if 1
/*====================
  CNavGridZ::LinearGate
  ====================*/
uint    CNavGridZ::LinearGate(EDivideType eType, uint unGridPos, uint uiBeginSegment, uint uiEndSegment, uint uiEntityWidth, CSearchGateR &cGate) const
{
    uint *pWorking(eType == DIVIDE_HORIZONTAL ? &m_pHorizontal[unGridPos * m_uiIntsPerColumn] : &m_pVertical[unGridPos * m_uiIntsPerRow]);
    uint uiLimit(eType == DIVIDE_HORIZONTAL ? m_uiIntsPerRow : m_uiIntsPerColumn);
    uint uiSearchOffset(IntOffsetFromIndex(uiBeginSegment));
    uint uiSegmentMask(MinMaskFromIndex(uiBeginSegment));
    if (uiEndSegment & GRIDZ_BITPOS_MASK)
        uiSegmentMask &= MaxMaskFromIndex(uiEndSegment);

    if (pWorking[uiSearchOffset] & uiSegmentMask)
    {
        uint uiLeftExtent(uiSearchOffset), uiRightExtent(uiSearchOffset);
        uint uiGateSrc(0);
        uint uiGateLength(0);

        while ((uiLeftExtent != 0) && pWorking[uiLeftExtent] == GRIDZ_PASSIBLE)
            --uiLeftExtent;

        while (((uiRightExtent + 1) != uiLimit) && pWorking[uiRightExtent] == GRIDZ_PASSIBLE)
            ++uiRightExtent;

        if (uiLeftExtent != uiRightExtent)
        {
            uint uiRightOnes(LZC(~pWorking[uiRightExtent])), uiLeftOnes(TZC(~pWorking[uiLeftExtent]));

            uiGateLength = (((uiRightExtent - uiLeftExtent) - 1) << BSHIFT_BITS_PER_INT) + uiLeftOnes + uiRightOnes;
            uiGateSrc = (uiLeftExtent << BSHIFT_BITS_PER_INT) + (WORDBITS - uiLeftOnes);
        }
        else
        {
            uint uiSegmentBlockers(~pWorking[uiRightExtent]);
            uint uiSegmentPassible(~uiSegmentBlockers);

            // Node is passible
            if (uiSegmentPassible & uiSegmentMask)
            {
                uint uiSolution;

                do
                {
                    uiSolution = (uiSegmentPassible | (uiSegmentPassible - 1));
                    uiSolution = (uiSolution + 1) ^ uiSegmentPassible;
                    uiSolution &= uiSegmentPassible;
                    uiSegmentPassible &= ~uiSolution;
                } while ((uiSolution & uiSegmentMask) == 0);

                uiGateLength = POPCOUNT(uiSolution);
                uiGateSrc = (uiRightExtent << BSHIFT_BITS_PER_INT) + (WORDBITS - TZC(uiSolution) - uiGateLength);

                if ((uiSolution & INT_LOWBIT) && (uiRightExtent + 1) != uiLimit)
                {
                    while ((uiRightExtent + 1) != uiLimit && pWorking[++uiRightExtent] == GRIDZ_PASSIBLE)
                        uiGateLength += BITS_PER_INT;

                    uiGateLength += LZC(~pWorking[uiRightExtent]);
                }
                else if ((uiSolution & INT_HIGHBIT) && uiLeftExtent > 0)
                {
                    while (--uiLeftExtent != 0 && pWorking[uiLeftExtent] == GRIDZ_PASSIBLE)
                    {
                        uiGateSrc -= BITS_PER_INT;
                        uiGateLength += BITS_PER_INT;
                    }

                    uiGateSrc -= TZC(~pWorking[uiLeftExtent]);
                    uiGateLength += TZC(~pWorking[uiLeftExtent]);
                }
            }
        }

        if (uiGateLength &&
            uiGateLength >= uiEntityWidth &&
            (uiGateLength == uiEntityWidth || uiGateSrc < uiEndSegment - (uiEntityWidth >> m_uiDownSize >> 1)) &&
            (uiGateLength == uiEntityWidth || uiEndSegment <= uiGateSrc + uiGateLength - (uiEntityWidth >> m_uiDownSize >> 1)))
        {
            cGate.SetMin(uiGateSrc + uiGateLength - uiEndSegment);
            cGate.SetMax(uiEndSegment - uiGateSrc);

            return 1;
        }
    }

    return 0;
}
#else
/*====================
  CNavGridZ::LinearGate
  ====================*/
uint    CNavGridZ::LinearGate(EDivideType eType, uint unGridPos, uint uiBeginSegment, uint uiEndSegment, uint uiEntityWidth, CSearchGateR &cGate) const
{
    uint *pWorking(eType == DIVIDE_HORIZONTAL ? &m_pHorizontal[unGridPos * m_uiIntsPerColumn] : &m_pVertical[unGridPos * m_uiIntsPerRow]);
    uint uiSearchOffset(IntOffsetFromIndex(uiBeginSegment));
    uint uiSegmentMask(MinMaskFromIndex(uiBeginSegment));
    if (uiEndSegment & GRIDZ_BITPOS_MASK)
        uiSegmentMask &= MaxMaskFromIndex(uiEndSegment);

    // Min/Max Segment must lie within the same int
    assert(((uiEndSegment & GRIDZ_BITPOS_MASK) == 0) || (IntOffsetFromIndex(uiBeginSegment) == IntOffsetFromIndex(uiEndSegment)));

    if (pWorking[uiSearchOffset] & uiSegmentMask)
    {
        uint uiGateSrc(0);
        uint uiGateLength(256);

        cGate.SetMin(uiGateSrc + uiGateLength - uiEndSegment);
        cGate.SetMax(uiEndSegment - uiGateSrc);

        return 1;
    }

    return 0;
}
#endif

/*====================
  CNavGridZ::LinearGate

  Prereq: the passed NavGrid must have been reset prior to the call
  ====================*/
void    CNavGridZ::HalfSize(CNavGridZ &cLowRes) const
{
    //uint uiStart, uiStop;
    uint uiIntsPerColumn(cLowRes.m_uiIntsPerColumn);
    uint uiIntsPerRow(cLowRes.m_uiIntsPerRow);
    uint *pSrc;
    uint *pDst;

    if (cLowRes.m_uiDownSize != m_uiDownSize + 1)
        return;

    //uiStart = K2System.Microseconds();
    // Horizontal
    pSrc = &m_pHorizontal[uiIntsPerRow];
    pDst = cLowRes.m_pHorizontal;
    for (uint y(1); y < m_uiCnxnHeight - 1; y += 2)
    {
        // Write current row
        for (uint b(0); b < uiIntsPerRow; ++b)
        {
            *pDst = *pSrc;

            ++pSrc;
            ++pDst;
        }

        // Merge with previous row's blockers
        pSrc -= uiIntsPerRow * 2;
        pDst -= uiIntsPerRow;
        
        for (uint b(0); b < uiIntsPerRow; ++b)
        {
            *pDst &= *pSrc;

            ++pSrc;
            ++pDst;
        }

        pSrc += uiIntsPerRow;

        pDst -= uiIntsPerRow;

        // Merge with next row's blockers
        if (m_uiDownSize == 0)
        {
            for (uint b(0); b < uiIntsPerRow; ++b)
            {
                *pDst &= *pSrc;

                // Block shared gates
                uint uiBlocked(~(*pDst));
                uint uiEvenBlocked(uiBlocked & 0xAAAAAAAA);
                uint uiOddBlocked(uiBlocked & 0x55555555);

                *pDst &= ~(uiEvenBlocked >> 1);
                *pDst &= ~(uiOddBlocked << 1);

                ++pSrc;
                ++pDst;
            }
        }
        else
        {
            for (uint b(0); b < uiIntsPerRow; ++b)
            {
                *pDst &= *pSrc;

                // Block shared gates
                uint uiBlocked(~(*pDst));
                uint uiBlocked0(uiBlocked & 0x11111111);
                uint uiBlocked2(uiBlocked & 0x44444444);

                *pDst &= ~(uiBlocked0 << 2);
                *pDst &= ~(uiBlocked0 << 3);

                *pDst &= ~(uiBlocked2 >> 1);
                *pDst &= ~(uiBlocked2 >> 2);
                
                ++pSrc;
                ++pDst;
            }
        }
    }
    
    // Vertical
    pSrc = &m_pVertical[uiIntsPerColumn];
    pDst = cLowRes.m_pVertical;
    for (uint x(1); x < m_uiCnxnWidth; x += 2)
    {
        // Write current column
        for (uint b(0); b < uiIntsPerColumn; ++b)
        {
            *pDst = *pSrc;

            ++pSrc;
            ++pDst;
        }

        // Merge with previous column's blockers
        pSrc -= uiIntsPerColumn * 2;
        pDst -= uiIntsPerRow;
        
        for (uint b(0); b < uiIntsPerColumn; ++b)
        {
            *pDst &= *pSrc;

            ++pSrc;
            ++pDst;
        }

        pSrc += uiIntsPerColumn;

        pDst -= uiIntsPerColumn;

        // Merge with next column's blockers
        if (m_uiDownSize == 0)
        {
            for (uint b(0); b < uiIntsPerColumn; ++b)
            {
                *pDst &= *pSrc;

                // Block shared gates
                uint uiBlocked(~(*pDst));
                uint uiEvenBlocked(uiBlocked & 0xAAAAAAAA);
                uint uiOddBlocked(uiBlocked & 0x55555555);

                *pDst &= ~(uiEvenBlocked >> 1);
                *pDst &= ~(uiOddBlocked << 1);

                ++pSrc;
                ++pDst;
            }
        }
        else
        {
            for (uint b(0); b < uiIntsPerColumn; ++b)
            {
                *pDst &= *pSrc;

                // Block shared gates
                uint uiBlocked(~(*pDst));
                uint uiBlocked0(uiBlocked & 0x11111111);
                uint uiBlocked2(uiBlocked & 0x44444444);

                *pDst &= ~(uiBlocked0 << 2);
                *pDst &= ~(uiBlocked0 << 3);

                *pDst &= ~(uiBlocked2 >> 1);
                *pDst &= ~(uiBlocked2 >> 2);
                
                ++pSrc;
                ++pDst;
            }
        }
    }
    
    //uiStop = K2System.Microseconds();
    //Console << _T("HalfSizing required ") << uiStop - uiStart << _T("us") << newl;
}


/*====================
  CNavGridZ::Merge
  ====================*/
void    CNavGridZ::Merge(const CNavGridZ &cSample)
{
    assert(m_uiDownSize == cSample.m_uiDownSize);

    uint *pDst, *pLimit, *pMerge;

    pDst = m_pHorizontal;
    pLimit = &m_pHorizontal[m_uiIntsPerColumn * m_uiCnxnWidth];
    pMerge = cSample.m_pHorizontal;
    for (; pDst<pLimit; ++pMerge, ++pDst)
        *pDst &= *pMerge;

    pDst = m_pVertical;
    pLimit = &m_pVertical[m_uiIntsPerRow * m_uiCnxnHeight];
    pMerge = cSample.m_pVertical;
    for (; pDst<pLimit; ++pDst, ++pMerge)
        *pDst &= *pMerge;
}


/*====================
  CNavGridZ::AntiMerge
  ====================*/
void    CNavGridZ::AntiMerge(const CNavGridZ &cSample)
{
    assert(m_uiDownSize == cSample.m_uiDownSize);

    uint *pDst, *pLimit, *pMerge;

    pDst = m_pHorizontal;
    pLimit = &m_pHorizontal[m_uiIntsPerColumn * m_uiCnxnWidth];
    pMerge = cSample.m_pHorizontal;
    for (; pDst<pLimit; ++pMerge, ++pDst)
        *pDst |= ~*pMerge;

    pDst = m_pVertical;
    pLimit = &m_pVertical[m_uiIntsPerRow * m_uiCnxnHeight];
    pMerge = cSample.m_pVertical;
    for (; pDst<pLimit; ++pDst, ++pMerge)
        *pDst |= ~*pMerge;
}


/*====================
  CNavGridZ::CopyFrom
  ====================*/
void    CNavGridZ::CopyFrom(const CNavGridZ &cSample)
{
    MEMCPY_S(m_pHorizontal, sizeof(uint) * m_uiIntsPerRow * m_uiCnxnWidth, cSample.m_pHorizontal, sizeof(uint) * cSample.m_uiIntsPerRow * cSample.m_uiCnxnWidth);
    MEMCPY_S(m_pVertical, sizeof(uint) * m_uiIntsPerColumn * m_uiCnxnHeight, cSample.m_pVertical, sizeof(uint) * cSample.m_uiIntsPerColumn * cSample.m_uiCnxnHeight);
}


/*====================
  CNavGridZ::AddBlocker
  ====================*/
void    CNavGridZ::AddBlocker(uint uiBeginSegmentX, uint uiEndSegmentX, uint uiBeginSegmentY, uint uiEndSegmentY)
{
    assert(m_uiDownSize == 0);
    assert(uiBeginSegmentX < m_uiCnxnWidth);
    assert(uiBeginSegmentY < m_uiCnxnHeight);
    assert(uiEndSegmentX > 0);
    assert(uiEndSegmentY > 0);
    assert(uiEndSegmentX <= m_uiCnxnWidth);
    assert(uiEndSegmentY <= m_uiCnxnHeight);
    assert(uiBeginSegmentX < uiEndSegmentX);
    assert(uiBeginSegmentY < uiEndSegmentY);

    uint *pWorking[2] = { m_pVertical, m_pHorizontal };
    uint uiBeginFrom[2] = { (uiBeginSegmentX ? uiBeginSegmentX - 1 : uiBeginSegmentX) * m_uiIntsPerRow, (uiBeginSegmentY ? uiBeginSegmentY - 1 : uiBeginSegmentY) * m_uiIntsPerColumn };
    uint uiBegin[2] = { uiBeginSegmentY, uiBeginSegmentX };
    uint uiEnd[2] = { uiEndSegmentY, uiEndSegmentX };

    for (int i(0); i < 2; ++i)
    {
        uint *pWorking2(pWorking[i] + uiBeginFrom[i]);
        uint uiRangeStart, uiRangeEnd;
        uint uiOffsetMin, uiOffsetMax;
        uint uiMaskMin, uiMaskMax;

        uiOffsetMin = IntOffsetFromIndex(uiBegin[i]);
        uiOffsetMax = IntOffsetFromIndex(uiEnd[i]);
        uiMaskMin = MinMaskFromIndex(uiBegin[i]);
        uiMaskMax = MaxMaskFromIndex(uiEnd[i]);
        uiRangeStart = uiBegin[TOGGLE(i)];
        uiRangeEnd = uiEnd[TOGGLE(i)];

        if (uiRangeStart > 0)
            --uiRangeStart;

        if (uiOffsetMin == uiOffsetMax)
        {
            pWorking2 += uiOffsetMin;
            uint uiMask(uiMaskMin & uiMaskMax);

            for (uint uiCurrent(uiRangeStart); uiCurrent < uiRangeEnd; ++uiCurrent, pWorking2 += m_uiIntsPerRow)
                *pWorking2 &= ~uiMask;
        }
        else
        {
            for (uint uiCurrent(uiRangeStart); uiCurrent < uiRangeEnd; ++uiCurrent, pWorking2 += m_uiIntsPerRow)
            {
                pWorking2[uiOffsetMin] &= ~uiMaskMin;
                
                for (uint uiOffsetMid(uiOffsetMin + 1); uiOffsetMid < uiOffsetMax; ++uiOffsetMid)
                    pWorking2[uiOffsetMid] &= ~GRIDZ_PASSIBLE;

                pWorking2[uiOffsetMax] &= ~uiMaskMax;
            }
        }
    }
}


/*====================
  CNavGridZ::ClearBlocker
  ====================*/
void    CNavGridZ::ClearBlocker(uint uiBeginSegmentX, uint uiEndSegmentX, uint uiBeginSegmentY, uint uiEndSegmentY)
{
    assert(m_uiDownSize == 0);
    assert(uiBeginSegmentX < m_uiCnxnWidth);
    assert(uiBeginSegmentY < m_uiCnxnHeight);
    assert(uiEndSegmentX > 0);
    assert(uiEndSegmentY > 0);
    assert(uiEndSegmentX <= m_uiCnxnWidth);
    assert(uiEndSegmentY <= m_uiCnxnHeight);
    assert(uiBeginSegmentX < uiEndSegmentX);
    assert(uiBeginSegmentY < uiEndSegmentY);

    uint *pWorking[2] = { m_pVertical, m_pHorizontal };
    uint uiBeginFrom[2] = { (uiBeginSegmentX ? uiBeginSegmentX - 1 : uiBeginSegmentX) * m_uiIntsPerRow, (uiBeginSegmentY ? uiBeginSegmentY - 1 : uiBeginSegmentY) * m_uiIntsPerColumn };
    uint uiBegin[2] = { uiBeginSegmentY, uiBeginSegmentX };
    uint uiEnd[2] = { uiEndSegmentY, uiEndSegmentX };

    for (int i(0); i < 2; ++i)
    {
        uint *pWorking2(pWorking[i] + uiBeginFrom[i]);
        uint uiRangeStart, uiRangeEnd;
        uint uiOffsetMin, uiOffsetMax;
        uint uiMaskMin, uiMaskMax;

        uiOffsetMin = IntOffsetFromIndex(uiBegin[i]);
        uiOffsetMax = IntOffsetFromIndex(uiEnd[i]);
        uiMaskMin = MinMaskFromIndex(uiBegin[i]);
        uiMaskMax = MaxMaskFromIndex(uiEnd[i]);
        uiRangeStart = uiBegin[TOGGLE(i)];
        uiRangeEnd = uiEnd[TOGGLE(i)];

        if (uiRangeStart > 0)
            --uiRangeStart;

        if (uiOffsetMin == uiOffsetMax)
        {
            pWorking2 += uiOffsetMin;
            uint uiMask(uiMaskMin & uiMaskMax);

            for (uint uiCurrent(uiRangeStart); uiCurrent < uiRangeEnd; ++uiCurrent, pWorking2 += m_uiIntsPerRow)
                *pWorking2 |= uiMask;
        }
        else
        {
            for (uint uiCurrent(uiRangeStart); uiCurrent < uiRangeEnd; ++uiCurrent, pWorking2 += m_uiIntsPerRow)
            {
                pWorking2[uiOffsetMin] |= uiMaskMin;
                
                for (uint uiOffsetMid(uiOffsetMin + 1); uiOffsetMid < uiOffsetMax; ++uiOffsetMid)
                    pWorking2[uiOffsetMid] |= GRIDZ_PASSIBLE;

                pWorking2[uiOffsetMax] |= uiMaskMax;
            }
        }
    }
}
