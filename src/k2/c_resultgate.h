// (C)2007 S2 Games
// c_gate.h
//
//=============================================================================
#ifndef __C_RESULTGATE_H__
#define __C_RESULTGATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_searchnode.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<class CResultGate>   PathResult;
typedef PathResult::iterator        PathResult_it;
typedef PathResult::const_iterator  PathResult_cit;
//=============================================================================

//=============================================================================
// CResultGate
//=============================================================================
class K2_API CResultGate
{
private:
    float m_fPositiveRadius;
    float m_fNegativeRadius;
    float m_fPositiveSq;
    float m_fNegativeSq;
    CVec2f m_v2Path;
    CVec4f m_v4Color;
    ESearchDirection m_eDirection;

public:
    CResultGate() : m_fPositiveRadius(0.0f), m_fNegativeRadius(0.0f), m_v2Path(V2_ZERO) { }
    CResultGate(CVec2f v2Path, float fNegativeRadius, float fPositiveRadius, ESearchDirection eDir, CVec4f v4Color) : m_fPositiveRadius(fPositiveRadius), m_fNegativeRadius(fNegativeRadius), m_fPositiveSq(m_fPositiveRadius * m_fPositiveRadius), m_fNegativeSq(m_fNegativeRadius * m_fNegativeRadius), m_v2Path(v2Path), m_v4Color(v4Color), m_eDirection(eDir) { }

    void    SetPath(CVec2f v2Path) { m_v2Path = v2Path; }

    CVec2f  GetPath()               { return m_v2Path; }
    CVec4f  GetColor()              { return m_v4Color; }
    float   GetRadiusNegative()     { return m_fNegativeRadius; }
    float   GetRadiusPositive()     { return m_fPositiveRadius; }
    float   GetSqPositive()         { return m_fPositiveSq; }
    float   GetSqNegative()         { return m_fNegativeSq; }
    ESearchDirection    Direction() { return m_eDirection; }

    void    ZeroRadius()            { m_fNegativeRadius = m_fPositiveRadius = m_fPositiveSq = m_fNegativeSq = 0.0f; }
};
//=============================================================================

#endif //__C_RESULTGATE_H__
