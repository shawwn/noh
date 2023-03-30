// (C)2008 S2 Games
// c_stateblock.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_stateblock.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CStateBlock::CStateBlock
  ====================*/
CStateBlock::CStateBlock() :
m_iModifiedCount(0)
{
}

CStateBlock::CStateBlock(const IBuffer &buffer) :
m_iModifiedCount(0)
{
	Set(buffer);
}


/*====================
  CStateBlock::GetDifference
  ====================*/
void	CStateBlock::GetDifference(CStateBlock &ss)
{
	PROFILE("CStateBlock::GetDifference");

	ss.Set(m_cBuffer);
}


/*====================
  CStateBlock::Set
  ====================*/
void	CStateBlock::Set(const IBuffer &buffer)
{
	m_cBuffer.Write(buffer.Get(), buffer.GetLength());
}
