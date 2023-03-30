// (C)2008 S2 Games
// c_navgridunits.h
//
//=============================================================================
#ifndef __C_NAVGRIDUNITS_H__
#define __C_NAVGRIDUNITS_H__

//=============================================================================
// Declarations
//=============================================================================
#define BSHIFT_UNITS_PER_INT 3
#define UNIT_MASK 0x7
#define GRIDUNITS_PASSIBLE 0x11111111

class CNavGridZ;
//=============================================================================

//=============================================================================
// CNavGridUnits
//=============================================================================
class CNavGridUnits
{
private:
    // Inverted from CNavGridZ -> 0 means passible, any greater value is a refcount of the blockers on the grid (impassible)
    uint *m_pHorizontal;
    uint *m_pVertical;
    uint m_uiCnxnWidth, m_uiCnxnHeight;
    uint m_uiIntsPerRow, m_uiIntsPerColumn;
    CNavGridZ *m_pLinkedTo;

    inline uint IntOffsetFromIndex(uint uiIndex) const;
    inline uint MinMaskFromIndex(uint uiIndex) const;
    inline uint MaxMaskFromIndex(uint uiIndex) const;
    inline byte DwordToByte(uint uiRefCount);

    void UpdateLinked(uint uiMinSegmentX, uint uiMaxSegmentX, uint uiMinSegmentY, uint uiMaxSegmentY);

public:
    ~CNavGridUnits();
    CNavGridUnits(uint uiWidth, uint uiHeight);

    void Reset();
    void ClearBlocker(uint uiMinSegmentX, uint uiMaxSegmentX, uint uiMinSegmentY, uint uiMaxSegmentY);
    void AddBlocker(uint uiMinSegmentX, uint uiMaxSegmentX, uint uiMinSegmentY, uint uiMaxSegmentY);

    void Standardize(CNavGridZ *pStandardGrid);
    void LinkTo(CNavGridZ *pStandardGrid);
};
//=============================================================================

#endif //__CNAVGRIDUNITS_H__
