// (C)2008 S2 Games
// c_navgridunits.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"
#include "c_navigationmap.h"
#include "c_navgridUnits.h"
#include "c_world.h"
#include "c_bytemanager.h"
#include "c_navgridZ.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define BSHIFT_GRIDZ_TO_GRIDUNITS (BSHIFT_BITS_PER_INT - BSHIFT_UNITS_PER_INT)
//=============================================================================

/*====================
  CNavGridUnits::IntOffsetFromIndex
  ====================*/
inline uint CNavGridUnits::IntOffsetFromIndex(uint uiIndex) const
{
	return (uiIndex >> BSHIFT_UNITS_PER_INT);
}


/*====================
  CNavGridUnits::MinMaskFromIndex
  ====================*/
inline uint CNavGridUnits::MinMaskFromIndex(uint uiIndex) const
{
	return (GRIDUNITS_PASSIBLE>>((uiIndex & UNIT_MASK)<<2));
}


/*====================
  CNavGridUnits::MaxMaskFromIndex
  ====================*/
inline uint CNavGridUnits::MaxMaskFromIndex(uint uiIndex) const
{
	return ~(GRIDUNITS_PASSIBLE >> ((uiIndex & UNIT_MASK) << 2)) & GRIDUNITS_PASSIBLE;
}


/*====================
  CNavGridUnits::DwordToByte
  ====================*/
inline byte CNavGridUnits::DwordToByte(uint uiRefCount)
{
	uint uiResult(0);

	// Blur the bits to result in 8 4-bit sets which are either 1 or 0
	uiRefCount |= ((0xaaaaaaaa & uiRefCount) >> 1);
	uiRefCount |= ((0x55555555 & uiRefCount) << 1);
	uiRefCount |= ((0xcccccccc & uiRefCount) >> 2);
	uiRefCount |= ((0x33333333 & uiRefCount) << 2);

	uiRefCount &= 0x11111111;
	uint uiBit(0);
	while (uiRefCount > 0)
	{
		uiResult |= (uiRefCount & 0xF) << uiBit;
		uiRefCount >>= 4;
		++uiBit;
	}

	uiResult = ~uiResult;

	return (byte)(uiResult & 0xFF);
}


/*====================
  CNavGridUnits::CNavGridUnits
  ====================*/
CNavGridUnits::CNavGridUnits(uint uiWidth, uint uiHeight) :
m_uiCnxnWidth(uiWidth),
m_uiCnxnHeight(uiHeight),
m_pLinkedTo(NULL)
{
	m_uiIntsPerRow = uiWidth >> BSHIFT_UNITS_PER_INT;
	m_uiIntsPerColumn = uiHeight >> BSHIFT_UNITS_PER_INT;

	m_pHorizontal = K2_NEW_ARRAY(ctx_Nav, uint, m_uiIntsPerRow * m_uiCnxnWidth);
	m_pVertical = K2_NEW_ARRAY(ctx_Nav, uint, m_uiIntsPerColumn * m_uiCnxnHeight);

	Reset();
}


/*====================
  CNavGridUnits::~CNavGridUnits
  ====================*/
CNavGridUnits::~CNavGridUnits()
{
	SAFE_DELETE_ARRAY(m_pHorizontal);
	SAFE_DELETE_ARRAY(m_pVertical);
}


/*====================
  CNavGridUnits::Reset

  Unit grids are a refcount, 0 means passible
  ====================*/
void	CNavGridUnits::Reset()
{
	memset(m_pHorizontal, 0, sizeof(uint) * m_uiIntsPerRow * m_uiCnxnWidth);
	memset(m_pVertical, 0, sizeof(uint) * m_uiIntsPerColumn * m_uiCnxnHeight);
}


/*====================
  CNavGridUnits::UpdateLinked
  ====================*/
void	CNavGridUnits::UpdateLinked(uint uiBeginSegmentX, uint uiEndSegmentX, uint uiBeginSegmentY, uint uiEndSegmentY)
{
	if (m_pLinkedTo == NULL)
		return;

	uint *pDst[2] = { m_pLinkedTo->m_pVertical, m_pLinkedTo->m_pHorizontal };
	uint *pSrc[2] = { m_pVertical, m_pHorizontal };
	uint uiDstSpan[2] = { m_pLinkedTo->m_uiIntsPerColumn, m_pLinkedTo->m_uiIntsPerRow };
	uint uiSrcSpan[2] = { m_uiIntsPerColumn, m_uiIntsPerRow };
	uint uiBegin[2] = { uiBeginSegmentY, uiBeginSegmentX };
	uint uiEnd[2] = { uiEndSegmentY, uiEndSegmentX };

	for (int i(0); i < 2; ++i)
	{
		uint uiRangeStart(uiBegin[TOGGLE(i)]), uiRangeEnd(uiEnd[TOGGLE(i)]);
		uint uiSegmentStart(m_pLinkedTo->IntOffsetFromIndex(uiBegin[i])), uiSegmentEnd(m_pLinkedTo->IntOffsetFromIndex(uiEnd[i]));
		uint uiSegments(uiSegmentEnd - uiSegmentStart + 1);
		
		if (uiRangeStart > 0)
			--uiRangeStart;

		uint uiRange(uiRangeEnd - uiRangeStart + 1);

		uint *pDst2(pDst[i] + (uiRangeStart * uiDstSpan[i]));
		uint *pSrc2(pSrc[i] + (uiRangeStart * uiSrcSpan[i]));
		
		pDst2 += uiSegmentStart;
		pSrc2 += (uiSegmentStart << BSHIFT_GRIDZ_TO_GRIDUNITS);

		while (--uiRange)
		{
			uint *pDstSegments(pDst2), *pSrcSegments(pSrc2);
			uint *pDstStop(pDst2 + uiSegments);

			while (pDstSegments < pDstStop)
			{
				uint uiShift(32);

				*pDstSegments = 0;
				do
				{
					uiShift -= 8;
					*pDstSegments |= (DwordToByte(*pSrcSegments++)) << uiShift;
				} while (uiShift);
				++pDstSegments;
			}
			
			pDst2 += uiDstSpan[i];
			pSrc2 += uiSrcSpan[i];
		}
	}
}


/*====================
  CNavGridUnits::ClearBlocker

  Refcount subtracted in the segment range of [begin, end)
  ====================*/
void	CNavGridUnits::ClearBlocker(uint uiBeginSegmentX, uint uiEndSegmentX, uint uiBeginSegmentY, uint uiEndSegmentY)
{
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
				*pWorking2 -= uiMask;
		}
		else
		{
			for (uint uiCurrent(uiRangeStart); uiCurrent < uiRangeEnd; ++uiCurrent, pWorking2 += m_uiIntsPerRow)
			{
				pWorking2[uiOffsetMin] -= uiMaskMin;
				
				for (uint uiOffsetMid(uiOffsetMin + 1); uiOffsetMid < uiOffsetMax; ++uiOffsetMid)
					pWorking2[uiOffsetMid] -= GRIDUNITS_PASSIBLE;

				pWorking2[uiOffsetMax] -= uiMaskMax;
			}
		}
	}

	UpdateLinked(uiBeginSegmentX, uiEndSegmentX, uiBeginSegmentY, uiEndSegmentY);
}


/*====================
  CNavGridUnits::AddBlocker

  Refcount added in the segment range of [begin, end)
  ====================*/
void	CNavGridUnits::AddBlocker(uint uiBeginSegmentX, uint uiEndSegmentX, uint uiBeginSegmentY, uint uiEndSegmentY)
{
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
				*pWorking2 += uiMask;
		}
		else
		{
			for (uint uiCurrent(uiRangeStart); uiCurrent < uiRangeEnd; ++uiCurrent, pWorking2 += m_uiIntsPerRow)
			{
				pWorking2[uiOffsetMin] += uiMaskMin;
				
				for (uint uiOffsetMid(uiOffsetMin + 1); uiOffsetMid < uiOffsetMax; ++uiOffsetMid)
					pWorking2[uiOffsetMid] += GRIDUNITS_PASSIBLE;

				pWorking2[uiOffsetMax] += uiMaskMax;
			}
		}
	}

	UpdateLinked(uiBeginSegmentX, uiEndSegmentX, uiBeginSegmentY, uiEndSegmentY);
}


/*====================
  CNavGridUnits::LinkTo
  ====================*/
void CNavGridUnits::LinkTo(CNavGridZ *pStandardGrid)
{
	m_pLinkedTo = pStandardGrid;
}