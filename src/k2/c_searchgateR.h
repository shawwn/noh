// (C)2007 S2 Games
// c_searchgateR.h
//
//=============================================================================
#ifndef __C_SEARCHGATER_H__
#define __C_SEARCHGATER_H__
// the span of a node which is travelable by the entity

class CSearchGateR
{
private:
	union
	{
		short m_unMin;
		short m_unPositiveExtent;
	};
	union
	{
		short m_unMax;
		short m_unNegativeExtent;
	};
	
public:
	CSearchGateR() : m_unMin(0), m_unMax(0) { }
	CSearchGateR(int iMin, int iMax) : m_unMin(iMin), m_unMax(iMax) { }

	void SwapExtentDirection() { SWAP(m_unNegativeExtent, m_unPositiveExtent); }

	int Valid() const { return m_unMin != INVALID_INDEX; }
	int Length() const { return m_unMax + m_unMin; }
	int Min() const { return m_unMin; }
	int Max() const { return m_unMax; }
	int NegativeExtent() const { return m_unNegativeExtent; }
	int PositiveExtent() const { return m_unPositiveExtent; }

	void SetMin(uint uiMin) { m_unMin = uiMin; }
	void SetMax(uint uiMax) { m_unMax = uiMax; }
};

#endif //__C_SEARCHGATER_H__
