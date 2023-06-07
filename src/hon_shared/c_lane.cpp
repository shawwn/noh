// (C)2008 S2 Games
// c_lane.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "c_lane.h"
//=============================================================================

/*====================
  CLane::AddWaypoint
  ====================*/
void    CLane::AddWaypoint(const CVec2f &v2Waypoint)
{
    if (!m_vWaypoints.empty())
        m_vWaypoints.back().v2Dir = Normalize(v2Waypoint - m_vWaypoints.back().v2Pos);

    m_vWaypoints.push_back(SWaypoint(v2Waypoint, V2_ZERO));
}


/*====================
  CLane::GetNextWaypoint
  ====================*/
CVec2f  CLane::GetNextWaypoint(const CVec2f &v2Pos, const CVec2f &v2CurrentWaypoint)
{
    if (m_vWaypoints.empty())
        return V2_ZERO;

    CVec2f v2Waypoint(m_vWaypoints.back().v2Pos);
    float fBestDistSq(FAR_AWAY);

    vector<SWaypoint>::iterator it(m_vWaypoints.begin()), itEnd(m_vWaypoints.end());

    // Search for the old waypoint and use that as the start of the search if found
    for (; it != itEnd; ++it)
    {
        if (it->v2Pos == v2CurrentWaypoint)
            break;
    }
    if (it == itEnd)
        it = m_vWaypoints.begin();
    
    for (; it != itEnd; ++it)
    {
        if (DotProduct(v2Pos - it->v2Pos, it->v2Dir) >= 0.0f)
            continue; // Ignore waypoints behind us

        float fDistSq(DistanceSq(v2Pos, it->v2Pos));

        if (fDistSq < SQR(100.0f))
            continue; // Ignore waypoints that are too close

        if (fDistSq < fBestDistSq)
        {
            fBestDistSq = fDistSq;
            v2Waypoint = it->v2Pos;
        }
    }

    return v2Waypoint;
}