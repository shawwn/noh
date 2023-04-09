// (C)2008 S2 Games
// c_navgridZ.h
//
//=============================================================================
#ifndef __C_NAVGRIDZ_H__
#define __C_NAVGRIDZ_H__

//=============================================================================
// Declarations
//=============================================================================
enum EDivideType
{
    DIVIDE_HORIZONTAL,
    DIVIDE_VERTICAL,
};

const uint BSHIFT_BITS_PER_INT(5);
const uint GRIDZ_BITPOS_MASK(0x0000001f);

const uint GRIDZ_PASSIBLE(0xffffffff);
const uint INT_HIGHBIT(0x80000000);
const uint INT_LOWBIT(0x00000001);
const uint BITS_PER_INT(32);

class CSearchGateR;
class CBlockPool;
//=============================================================================

//=============================================================================
// CNavGridZ
//=============================================================================
class CNavGridZ
{
    friend class CNavGridUnits;

    static CBlockPool s_cMemPool;

    // 1s for passible, 0s for blocked
    // 00011100 (e.x. 3 leading 0s, 2 trailing 0s)
    uint    *m_pHorizontal;
    uint    *m_pVertical;
    uint    m_uiDownSize;
    uint    m_uiCnxnWidth, m_uiCnxnHeight;
    uint    m_uiIntsPerRow, m_uiIntsPerColumn;

    inline uint     IntOffsetFromIndex(uint uiIndex) const;
    inline uint     MinMaskFromIndex(uint uiIndex) const;
    inline uint     MaxMaskFromIndex(uint uiIndex) const;
    inline void     Reset();

public:
    CNavGridZ();
    CNavGridZ(uint uiWidth, uint uiHeight, uint uiDownSize);

    void            ClearBlocker(uint uiBeginSegmentX, uint uiEndSegmentX, uint uiBeginSegmentY, uint uiEndSegmentY);
    void            AddBlocker(uint uiBeginSegmentX, uint uiEndSegmentX, uint uiBeginSegmentY, uint uiEndSegmentY);

    uint            LinearGate(EDivideType eType, uint unGridPos, uint uiBeginSegment, uint uiEndSegment, uint uiEntityWidth, CSearchGateR &cGate) const;

    void            Init(uint uiPermanentGrid);
    void            Done();

    void            HalfSize(CNavGridZ &cLowRes) const;
    void            Merge(const CNavGridZ &cSample);
    void            AntiMerge(const CNavGridZ &cSample);
    void            CopyFrom(const CNavGridZ &cSample);

    static void     Alloc(uint uiWorldNavSize);
    static void     ReleaseAlloc();

    uint*           GetHorizontal() const       { return m_pHorizontal; }
    uint*           GetVertical() const         { return m_pVertical; }
    uint            GetIntsPerRow() const       { return m_uiIntsPerRow; }
    uint            GetIntsPerColumn() const    { return m_uiIntsPerColumn; }
    uint            GetDownSize() const         { return m_uiDownSize; }
    uint            GetWidth() const            { return m_uiCnxnWidth; }
    uint            GetHeight() const           { return m_uiCnxnHeight; }
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CNavGridZ::IntOffsetFromIndex
  ====================*/
inline
uint    CNavGridZ::IntOffsetFromIndex(uint uiIndex) const
{
    return (uiIndex >> BSHIFT_BITS_PER_INT);
}


/*====================
  CNavGridZ::MinMaskFromIndex
  ====================*/
inline
uint    CNavGridZ::MinMaskFromIndex(uint uiIndex) const
{
    uint uiShift(uiIndex & GRIDZ_BITPOS_MASK);

    return (GRIDZ_PASSIBLE >> uiShift);
}


/*====================
  CNavGridZ::MaxMaskFromIndex
  ====================*/
inline
uint    CNavGridZ::MaxMaskFromIndex(uint uiIndex) const
{
    uint uiShift(uiIndex & GRIDZ_BITPOS_MASK);

    return ~(GRIDZ_PASSIBLE >> uiShift);
}


/*====================
  CNavGridZ::Reset
  ====================*/
inline
void    CNavGridZ::Reset()
{
#if TKTK
    MemManager.Set(m_pHorizontal, GRIDZ_PASSIBLE, sizeof(uint) * m_uiIntsPerColumn * m_uiCnxnWidth);
    MemManager.Set(m_pVertical, GRIDZ_PASSIBLE, sizeof(uint) * m_uiIntsPerRow * m_uiCnxnHeight);
#else
    for (uint i = 0; i < m_uiIntsPerColumn * m_uiCnxnWidth; i++) {
        m_pHorizontal[i] = GRIDZ_PASSIBLE;
    }
    for (uint i = 0; i < m_uiIntsPerRow * m_uiCnxnHeight; i++) {
        m_pVertical[i] = GRIDZ_PASSIBLE;
    }
#endif
}
//=============================================================================

#endif // __C_NAVGRIDZ_H__
