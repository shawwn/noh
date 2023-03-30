// (C)2007 S2 Games
// c_path.h
//
//=============================================================================
#ifndef __C_PATH_H__
#define __C_PATH_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_resultgate.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define WAYPT_SEARCH_LIMIT 10 // can now be based on path resolution & projection length

#define DETAIL_SIZE 50
#define SIMPLIFIED_ESTIMATE 32
//=============================================================================

//=============================================================================
// CPath
//=============================================================================
class K2_API CPath
{
private:
    PathResult  m_vecSmoothPath;
    PathResult  m_vecSimplePath;
    ushort      m_unCompletedWaypt;
    ushort      m_unLastProjectedWaypt;
    ushort      m_unNumWaypts;
    uint        m_uiWorldEntity;
    bool        m_bFinished;

    ushort  ClosestSegment2d(const CVec2f &v2Position, float &fDistanceSq, CVec2f &v2OnPath);
    CVec2f  TravelPath2d(const CVec2f &v2Start, float fDistance, float fEndDistance);
    float   DistanceFromPath(const CVec2f &v2Position, float &fGateDistSq, bool &bPositive);

public:
    CPath() : m_unCompletedWaypt(0), m_unLastProjectedWaypt(0), m_unNumWaypts(0), m_uiWorldEntity(0), m_bFinished(false)  { }

    void        CopyFrom(const CPath& path);

    // Path must be sealed after it is built
    bool        SealPath(float fSrcX, float fSrcY, float fDstX, float fDstY);
    bool        SealPath(float fSrcX, float fSrcY);
    void        AddPoint(float fX, float fY);

    // Advance the completed waypoint and return the next goal on the path
    CVec2f      CalcNearGoal(const CVec2f &v2UnitPos, float fDistance, float fEndDistance);
    bool        AtGoal(CVec2f &v2Position, float fCollisionSize);
    bool        Exists()            { return m_unNumWaypts > 1; }
    
    CVec2f      GetStart()          { return m_vecSmoothPath[0].GetPath(); }
    CVec2f      GetGoal()           { return m_vecSmoothPath[m_unNumWaypts-1].GetPath(); }
    
    float       GetLength();
    float       GetLength(const CVec2f &v2Position);

    bool        IsFinished()        { return m_bFinished; }
    bool        IsValid()           { return !m_vecSmoothPath.empty(); }

    void        SetWorldEntity(uint uiWorldIndex)   { m_uiWorldEntity = uiWorldIndex; }

    // Return if the specified point is close enough to the path to be valid
    //bool      ValidCourseForPath(const CVec2f &v2Target, bool &bPositive);
    void        ReserveResult()     { m_vecSmoothPath.reserve(DETAIL_SIZE); m_vecSimplePath.reserve(SIMPLIFIED_ESTIMATE); }
    PathResult& GetSmoothResult()   { return m_vecSmoothPath; }
    PathResult& GetSimpleResult()   { return m_vecSimplePath; }

    ushort      GetWaypointCount()  { return m_unNumWaypts; }
    ushort      GetLastWaypoint()   { return m_unCompletedWaypt; }
    ushort      GetLastProjected()  { return m_unLastProjectedWaypt; }
    //void DebugRender();
};
//=============================================================================

#endif //__C_PATH_H__
