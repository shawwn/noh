// (C)2006 S2 Games
// c_temporalrange.h
//
//=============================================================================
#ifndef __C_TEMPORALRANGE_H__
#define __C_TEMPORALRANGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_range.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CTemporalRange
//=============================================================================
template <class T>
class CTemporalRange
{
private:
    CRange<T>   start, end;
    bool        m_bDynamic;

public:
    ~CTemporalRange() {}

    CTemporalRange() {}

    CTemporalRange(T _Min, T _Max) :
        start(_Min, _Max), end(_Min, _Max), m_bDynamic(false) {}

    CTemporalRange(T _MinStart, T _MaxStart, T _MinEnd, T _MaxEnd, bool bDynamic = true) :
        start(_MinStart, _MaxStart), end(_MinEnd, _MaxEnd), m_bDynamic(bDynamic) {}

    bool    GetValues(T &outStart, T &outEnd) const
    {
        if (m_bDynamic)
        {
            outStart = start;
            outEnd = end;
            return true;
        }
        else
        {
            outStart = outEnd = start;
            return false;
        }
    }

    operator CRange<T>() const
    {
        if (m_bDynamic)
        {
            return CRange<T>(start, end);
        }
        else
        {
            T value(start);
            return CRange<T>(value, value);
        }
    }
};

typedef CTemporalRange<float>   CTemporalRangef;
typedef CTemporalRange<int>     CTemporalRangei;
//=============================================================================
#endif //__C_TEMPORALRANGE_H__
