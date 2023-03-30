// (C)2006 S2 Games
// c_temporalpropertyrange.h
//
//=============================================================================
#ifndef __C_TEMPORALPROPERTYRANGE_H__
#define __C_TEMPORALPROPERTYRANGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_range.h"
#include "c_temporalproperty.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CTemporalPropertyRange
//=============================================================================
template <class T>
class CTemporalPropertyRange
{
private:
    CRange<T>   start, end, mid, speed;
    CRangef     m_rfMidPos;
    bool        m_bDynamic;

public:
    ~CTemporalPropertyRange() {}

    CTemporalPropertyRange() {}

    CTemporalPropertyRange(T _MinStart, T _MaxStart, T _MinEnd, T _MaxEnd, T _MinMid, T _MaxMid, float fMinMidPos, float fMaxMidPos, bool bDynamic,
            T _MinSpeed, T _MaxSpeed) :
        start(_MinStart, _MaxStart), end(_MinEnd, _MaxEnd), mid(_MinMid, _MaxMid), m_rfMidPos(fMinMidPos, fMaxMidPos), m_bDynamic(bDynamic),
        speed(_MinSpeed, _MaxSpeed)
        {}

    operator CTemporalProperty<T>() const
    {
        if (m_bDynamic)
        {
            return CTemporalProperty<T>(start, end, mid, m_rfMidPos, speed);
        }
        else
        {
            T _value(start);
            return CTemporalProperty<T>(_value, _value, _value, 0.0f, speed);
        }
    }

    T   Min() const
    {
        if (m_bDynamic)
        {
            return CTemporalProperty<T>(start.Min(), end.Min(), mid.Min(), m_rfMidPos.Min(), speed).Min();
        }
        else
        {
            T _value(start.Min());
            return CTemporalProperty<T>(_value, _value, _value, 0.0f, speed).Min();
        }
    }

    T   Max() const
    {
        if (m_bDynamic)
        {
            return CTemporalProperty<T>(start.Max(), end.Max(), mid.Max(), m_rfMidPos.Max(), speed).Max();
        }
        else
        {
            T _value(start.Max());
            return CTemporalProperty<T>(_value, _value, _value, 0.0f, speed).Max();
        }
    }
};

typedef CTemporalPropertyRange<float>   CTemporalPropertyRangef;
typedef CTemporalPropertyRange<int>     CTemporalPropertyRangei;
//=============================================================================
#endif //__C_TEMPORALPROPERTYRANGE_H__
