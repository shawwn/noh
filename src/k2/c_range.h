// (C)2006 S2 Games
// c_range.h
//
//=============================================================================
#ifndef __C_RANGE_H__
#define __C_RANGE_H__

//=============================================================================
// CRange
//=============================================================================
template <class T>
class CRange
{
private:
	T	minvalue;
	T	maxvalue;

public:
	~CRange() {}
	CRange() {}
	CRange(T _Min, T _Max) : minvalue(_Min), maxvalue(_Max) {}

	operator T() const	{ return minvalue < maxvalue ? M_Randnum(minvalue, maxvalue) : maxvalue; }

	void	Set(T tVal)				{ minvalue = maxvalue = tVal; }
	void	Set(T min, T max)		{ minvalue = min; maxvalue = max; }

	T		Lerp(float fLerp) const	{ return LERP(fLerp, minvalue, maxvalue); }
	T		Min() const				{ return MIN(minvalue, maxvalue); }
	T		Max() const				{ return MAX(minvalue, maxvalue); }
};

typedef CRange<float>	CRangef;
typedef CRange<int>		CRangei;
//=============================================================================
#endif //__C_RANGE_H__
