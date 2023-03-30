// (C)2008 S2 Games
// c_path.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_path.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

/*====================
  CPath::CopyFrom
  ====================*/
void    CPath::CopyFrom(const CPath& path)
{
    m_vecSmoothPath = path.m_vecSmoothPath;
    m_vecSimplePath = path.m_vecSimplePath;
    m_unCompletedWaypt = path.m_unCompletedWaypt;
    m_unLastProjectedWaypt = path.m_unLastProjectedWaypt;
    m_unNumWaypts = path.m_unNumWaypts;
    m_uiWorldEntity = path.m_uiWorldEntity;
    m_bFinished = path.m_bFinished;
}

/*====================
  CPath::SealPath
  ====================*/
bool    CPath::SealPath(float fSrcX, float fSrcY, float fDstX, float fDstY)
{
    if (m_vecSmoothPath.size() < 2)
        return false;

    m_vecSmoothPath.front().SetPath(CVec2f(fSrcX, fSrcY));
    m_vecSmoothPath.back().SetPath(CVec2f(fDstX, fDstY));

    if (m_vecSimplePath.size() >= 2)
    {
        m_vecSimplePath.front().SetPath(CVec2f(fSrcX, fSrcY));
        m_vecSimplePath.back().SetPath(CVec2f(fDstX, fDstY));
    }

    m_unNumWaypts = (ushort)m_vecSmoothPath.size();
    return true;
}

bool    CPath::SealPath(float fSrcX, float fSrcY)
{
    if (m_vecSmoothPath.size() < 2)
        return false;

    m_vecSmoothPath.front().SetPath(CVec2f(fSrcX, fSrcY));

    if (m_vecSimplePath.size())
    {
        m_vecSimplePath.front().SetPath(CVec2f(fSrcX, fSrcY));
        //m_vecSimplePath.pop_back();
    }

    m_unNumWaypts = (ushort)m_vecSmoothPath.size();
    return true;
}


/*====================
  CPath::AddPoint
  ====================*/
void    CPath::AddPoint(float fX, float fY)
{
    if (m_unNumWaypts)
    {
        m_vecSmoothPath.back().SetPath(CVec2f(fX, fY));
    }
    else
    {
        m_vecSmoothPath.push_back(CResultGate(CVec2f(fX, fY), 0.0f, 0.0f, SD_INVALID, V4_ZERO));
        ++m_unNumWaypts;
    }
}


/*====================
  CPath::ClosestSegment2d
  ====================*/
ushort  CPath::ClosestSegment2d(const CVec2f &v2Position, float &fDistanceSq, CVec2f &v2OnPath)
{
    ushort unSegment(m_unCompletedWaypt);
    CVec2f v2Pt(v2Position);

    fDistanceSq = FAR_AWAY;

    for (int iWaypts(0); (iWaypts < WAYPT_SEARCH_LIMIT) && ((iWaypts + m_unCompletedWaypt + 1) < m_unNumWaypts); ++iWaypts)
    {
        v2OnPath = M_ClosestPointToSegment2d(m_vecSmoothPath[m_unCompletedWaypt + iWaypts].GetPath(), m_vecSmoothPath[m_unCompletedWaypt + iWaypts + 1].GetPath(), v2Position);
        float fTestDistanceSq(DistanceSq(v2Position, v2OnPath));

        if (fTestDistanceSq < fDistanceSq)
        {
            unSegment = m_unCompletedWaypt + iWaypts;
            fDistanceSq = fTestDistanceSq;
            v2Pt = v2OnPath;
        }
    }

    v2OnPath = v2Pt;
    return unSegment;
}


/*====================
  CPath::TravelPath2d
  ====================*/
CVec2f  CPath::TravelPath2d(const CVec2f &v2Start, float fDistance, float fEndDistance)
{
    int iWaypt(0);
    float fWayptDistance;

    float fLength(GetLength(v2Start));

    float fSearchDistance(MIN(fDistance, fLength - fEndDistance));
    
    if (m_vecSmoothPath.empty() || fSearchDistance <= 0.001f)
        return v2Start;

    CVec2f v2Src(v2Start), v2Dst(m_vecSmoothPath[m_unCompletedWaypt + 1].GetPath());

    while (fSearchDistance > (fWayptDistance = Distance(v2Src, v2Dst)))
    {
        fSearchDistance -= fWayptDistance;
        ++iWaypt;

        // If we traveled the full distance of the last line segment, return the destination
        if ((m_unCompletedWaypt + iWaypt + 1) >= (m_unNumWaypts))
        {
            m_bFinished = true;
            return m_vecSmoothPath[m_unNumWaypts - 1].GetPath();
        }

        v2Src = m_vecSmoothPath[m_unCompletedWaypt + iWaypt].GetPath();
        v2Dst = m_vecSmoothPath[m_unCompletedWaypt + iWaypt + 1].GetPath();
    }

    CVec2f v2Direction(v2Dst - v2Src);
    v2Direction.Normalize();
    v2Direction *= fSearchDistance;

    return v2Src + v2Direction;
}


/*====================
  CPath::CalcNearGoal
  ====================*/
CVec2f  CPath::CalcNearGoal(const CVec2f &v2UnitPos, float fDistance, float fEndDistance)
{
    float fUnitDistanceFromPathSq;
    CVec2f v2NearGoal;

    if (m_unNumWaypts > 1)
    {
        ushort unClosestSegment(ClosestSegment2d(v2UnitPos, fUnitDistanceFromPathSq, v2NearGoal));

        if (unClosestSegment != m_unCompletedWaypt)
            m_unCompletedWaypt = unClosestSegment;

        return TravelPath2d(v2NearGoal, fDistance, fEndDistance);
    }
    else if (m_unNumWaypts == 1)
    {
        return m_vecSmoothPath[0].GetPath();
    }
    else
    {
        return v2UnitPos;
    }
}


/*====================
  CPath::AtGoal
  ====================*/
bool    CPath::AtGoal(CVec2f &v2Position, float fCollisionSize)
{
    if (m_unNumWaypts)
    {
        if (M_GetDistanceSqVec2(v2Position, GetGoal()) < SQR(fCollisionSize))
            return true;
        else
            return false;
    }

    return true;
}


/*====================
  CPath::GetLength
  ====================*/
float   CPath::GetLength()
{
    if (m_unNumWaypts < 2)
        return 0.0f;

    float fDist(0.0f);
    PathResult_it itPrev(m_vecSmoothPath.begin());
    PathResult_it itNext(itPrev + 1);
    
    do
    {
        fDist += Distance(itPrev->GetPath(), itNext->GetPath());
        ++itPrev;
        ++itNext;
    }
    while (itNext != m_vecSmoothPath.end());

    return fDist;
}

float   CPath::GetLength(const CVec2f &v2Position)
{
    if (m_unNumWaypts == 0)
        return 0.0f;

    if (m_unNumWaypts == 1)
        return Distance(v2Position, m_vecSmoothPath[0].GetPath());

    CVec2f v2Waypoint;
    float fDist(0.0f);
    ushort unWaypoint(ClosestSegment2d(v2Position, fDist, v2Waypoint) + 1);

    fDist = Distance(v2Position, v2Waypoint);
    
    while (unWaypoint < m_unNumWaypts)
    {
        fDist += Distance(v2Waypoint, m_vecSmoothPath[unWaypoint].GetPath());
        v2Waypoint = m_vecSmoothPath[unWaypoint].GetPath();
        ++unWaypoint;
    }

    return fDist;
}


/*====================
  CPath::DistanceFromPath
  ====================*/
float   CPath::DistanceFromPath(const CVec2f &v2Position, float &fGateDistSq, bool &bPositive)
{
    float fDistanceFromPath;
    CVec2f v2PtOnPath;
    ushort unSegment;

    unSegment = ClosestSegment2d(v2Position, fDistanceFromPath, v2PtOnPath);
    m_unLastProjectedWaypt = unSegment;

    // which side of the plane is v2Position on?
    CVec2f v2PathDir(m_vecSmoothPath[unSegment + 1].GetPath() - m_vecSmoothPath[unSegment].GetPath());
    CVec2f v2PositionDir(v2Position - m_vecSmoothPath[unSegment].GetPath());

    //if (m_vecSmoothPath[unSegment].Direction() == SD_NORTH)
    //  Console << _T("Check") << newl;

    //v2PathDir.Normalize();
    //v2PositionDir.Normalize();

    float fPathAngle(M_GetYawFromForwardVec2(v2PathDir));
    float fPositionAngle(M_GetYawFromForwardVec2(v2PositionDir));
    float fAngleDiff(fPositionAngle - fPathAngle);

    if (fAngleDiff > 180.f)
        fAngleDiff -= 360.f;
    else if (fAngleDiff < -180.f)
        fAngleDiff += 360.f;

    //if (fAngleDiff >= 0)
    if (fAngleDiff < 0.0f)
    {
        fGateDistSq = MIN(m_vecSmoothPath[unSegment].GetSqNegative(), m_vecSmoothPath[unSegment + 1].GetSqNegative());
        bPositive = false;
    }
    else
    {
        fGateDistSq = MIN(m_vecSmoothPath[unSegment].GetSqPositive(), m_vecSmoothPath[unSegment + 1].GetSqPositive());
        bPositive = true;
    }

    return fDistanceFromPath;
}

#ifdef NEEDED
bool CPath::ValidCourseForPath(const CVec2f &v2Target, bool &bPositive)
{
    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldEntity));

    if (pWorldEnt)
    {
        float fRadiusSq;
        float fDist(DistanceFromPath(v2Target, fRadiusSq, bPositive));

        return fDist < fRadiusSq;
    }
    else
    {
        return false;
    }
}

void CPath::DebugRender()
{

    for (int i(0); i+1<m_unNumWaypts; ++i)
    {
        CVec2f v2Src(m_vecSmoothPath[i].Path());
        CVec2f v2End(m_vecSmoothPath[i+1].Path());

//#ifdef SHOW_GATES
        CVec2f v2PosGate, v2NegGate;
        CVec2f v2Dir(v2End - v2Src);

        v2Dir.Normalize();
        v2Dir.Rotate(90.f);

        v2PosGate = v2Dir * m_vecSmoothPath[i].GetRadiusPositive();
        v2PosGate += v2Src;

        v2NegGate = v2Dir * -m_vecSmoothPath[i].GetRadiusNegative();
        v2NegGate += v2Src;

        DebugRenderer.AddLine(
            CVec3f(v2PosGate.x, v2PosGate.y, Game.GetTerrainHeight(v2PosGate.x, v2PosGate.y)),
            CVec3f(v2NegGate.x, v2NegGate.y, Game.GetTerrainHeight(v2NegGate.x, v2NegGate.y)), 
            m_vecSmoothPath[i].Color());

//#endif

        {
            CVec4f v4PathLineColor;

            if (i == m_unCompletedWaypt)
                v4PathLineColor = CVec4f(1.0f, 1.0f, 1.0f, 1.0f);
            else if (i == m_unLastProjectedWaypt)
                v4PathLineColor = CVec4f(1.0f, 0.0f, 0.0f, 1.0f);
            else
                v4PathLineColor = CVec4f(0.0f, 1.0f, 0.0f, 1.0f);

            DebugRenderer.AddLine(
                CVec3f(v2Src.x, v2Src.y, Game.GetTerrainHeight(v2Src.x, v2Src.y)),
                CVec3f(v2End.x, v2End.y, Game.GetTerrainHeight(v2End.x, v2End.y)),
                v4PathLineColor);
        }
        
    }
}

#endif
