// (C)2008 S2 Games
// c_searchnode.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_searchnode.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
byte CSearchNode::m_syByteDirection[] = { SDB_NORTH, SDB_EAST, SDB_WEST, SDB_SOUTH, SDB_NORTH | SDB_EAST | SDB_SOUTH | SDB_WEST };
//=============================================================================

/*====================
  CSearchNode::operator>
  ====================*/
bool	CSearchNode::operator>(const CSearchNode &cComp) const
{
	if ((m_uiNodeData & HEURISTIC_MASK) == (cComp.m_uiNodeData & HEURISTIC_MASK))
		return m_uiBias > cComp.m_uiBias;
	else
		return (m_uiNodeData & HEURISTIC_MASK) > (cComp.m_uiNodeData & HEURISTIC_MASK);
}
