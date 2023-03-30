// (C)2007 S2 Games
// c_searchnode.h
//
//=============================================================================
#ifndef __C_SEARCHNODE_H__
#define __C_SEARCHNODE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_searchgate.h"
#include "c_searchgateR.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Search Direction as bits
#define SDB_NORTH       BIT(0)
#define SDB_EAST        BIT(1)
#define SDB_SOUTH       BIT(2)
#define SDB_WEST        BIT(3)
#define SDB_COUNT       4

#define SN_NOTLISTED    BIT(13) //1
#define SN_NOTFROMSRC   BIT(14) //2
#define SN_NOTFROMDST   BIT(15) //3

enum ESearchDirection // Linked to CNavigationGraph::FindNeighbor - aOffsets, and CNavigationGraph::FindGate arrays
{
    SD_NORTH = 0,
    SD_EAST,
    SD_WEST,
    SD_SOUTH,
    SD_COUNT = 4,
    SD_INVALID = SD_COUNT,
    SD_MASK = SD_COUNT - 1,
};

// NodeData Masks
#define COST_MASK       0x00000fff
#define FLAGS_MASK      0x0000f000
#define HEURISTIC_MASK  0x0fff0000
#define DIR_MASK        0xf0000000
#define FLIPDIR_MASK    0x30000000

// Shift values, must remain synced with NodeData Masks
#define COST_SHIFT      0
#define FLAGS_SHIFT     12
#define HEURISTIC_SHIFT 16
#define DIR_SHIFT       28

typedef vector<class CSearchNode *> SearchNodeList;
//=============================================================================

//=============================================================================
// CSearchNode
// The Search NodE!
//=============================================================================
class CSearchNode
{
private:
    static byte m_syByteDirection[];
    uint m_uiNodeData; // [0.000.0.000] (dir flags, heuristic, node flags, cost)
    uint m_uiBias;

public:
    CSearchNode() : m_uiNodeData(0xffffffff) { }

    bool            operator>(const CSearchNode &cComp) const;

    inline void     Reset();
    inline bool     IsReset()                   { return m_uiNodeData == 0xffffffff; }
    inline void     SetCost(uint uiCost);
    inline uint     GetCost();
    
    inline void     SetHeuristic(uint uiEstimate);
    inline uint     GetHeuristic();

    inline void     SetBias(uint uiBias)        { m_uiBias = uiBias; }
    inline uint     GetBias() const             { return m_uiBias; }

    inline void     SetFlags(uint uiFlags)      { m_uiNodeData |= uiFlags; }
    inline void     ClearFlags(uint uiFlags)    { m_uiNodeData &= ~uiFlags; }
    inline uint     GetFlags() { return m_uiNodeData & FLAGS_MASK; }

    inline ESearchDirection ParentDirection() const;

    static inline int ParentDirectionReversed(int iDir)             { return iDir >= SD_INVALID ? SD_INVALID : ((~iDir) & SD_MASK); }
    static inline byte ParentDirectionByByte(ESearchDirection eDir) { return m_syByteDirection[eDir]; }
    static inline bool ParentDirectionValid(byte yDir)
    {
        static byte yInvalid1(SDB_NORTH | SDB_SOUTH), yInvalid2(SDB_EAST | SDB_WEST);

        if (((yDir & yInvalid1) == yInvalid1) || ((yDir & yInvalid2) == yInvalid2))
            return false;
        else
            return true;
    }

    inline void     SetDirection(int iDirection, bool bReverse = true);
    inline void     ReverseDirection();
};
//=============================================================================

//=============================================================================
// Inline Functions
//=============================================================================

/*====================
  CSearchNode::Reset
  ====================*/
inline
void    CSearchNode::Reset()
{
    m_uiNodeData = 0xffffffff;
}


/*====================
  CSearchNode::SetCost
  ====================*/
inline
void    CSearchNode::SetCost(uint uiCost)
{
    assert((uiCost & COST_MASK) == uiCost);

    m_uiNodeData = ((COST_MASK & uiCost) | (~COST_MASK & m_uiNodeData));
}


/*====================
  CSearchNode::GetCost
  ====================*/
inline
uint    CSearchNode::GetCost()
{
    return ((m_uiNodeData & COST_MASK) >> COST_SHIFT);
}


/*====================
  CSearchNode::SetHeuristic
  ====================*/
inline
void    CSearchNode::SetHeuristic(uint uiEstimate)
{
    assert(((uiEstimate << HEURISTIC_SHIFT) & HEURISTIC_MASK) >> HEURISTIC_SHIFT == uiEstimate);

    m_uiNodeData = ((m_uiNodeData & ~HEURISTIC_MASK) | ((uiEstimate << HEURISTIC_SHIFT) & HEURISTIC_MASK));
}


/*====================
  CSearchNode::GetHeuristic
  ====================*/
inline
uint    CSearchNode::GetHeuristic()
{
    return ((m_uiNodeData & HEURISTIC_MASK) >> HEURISTIC_SHIFT);
}


/*====================
  CSearchNode::ParentDirection
  ====================*/
inline
ESearchDirection    CSearchNode::ParentDirection() const
{
    uint uiDirection((DIR_MASK & m_uiNodeData) >> DIR_SHIFT);

    if (uiDirection < SD_COUNT)
        return (ESearchDirection)uiDirection;
    else
        return SD_INVALID;
}


/*====================
  CSearchNode::SetDirection
  ====================*/
inline
void    CSearchNode::SetDirection(int iDirection, bool bReverse)
{
    m_uiNodeData = (m_uiNodeData & ~DIR_MASK) | (iDirection << DIR_SHIFT);

    if (bReverse)
    {
        assert(iDirection < SD_COUNT);

        ReverseDirection();
    }
}


/*====================
  CSearchNode::ReverseDirection
  ====================*/
inline
void    CSearchNode::ReverseDirection()
{
    ESearchDirection eDir(ParentDirection());

    // Reverse direction flags, if not SD_INVALID
    if (eDir == SD_INVALID)
        return;

    m_uiNodeData = (m_uiNodeData & ~DIR_MASK) | ((~m_uiNodeData) & FLIPDIR_MASK);
}
//=============================================================================

#endif //__C_SEARCHNODE_H__
