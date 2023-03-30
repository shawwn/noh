// (C)2008 S2 Games
// c_lane.h
//
//=============================================================================
#ifndef __C_LANE_H__
#define __C_LANE_H__

//=============================================================================
// CLane
//=============================================================================
class CLane
{
private:
    struct SWaypoint
    {
        CVec2f  v2Pos;
        CVec2f  v2Dir;

        SWaypoint() : v2Pos(V2_ZERO), v2Dir(V2_ZERO) {}
        SWaypoint(const CVec2f &_v2Pos, const CVec2f &_v2Dir) : v2Pos(_v2Pos), v2Dir(_v2Dir) {}
    };

    vector<SWaypoint>   m_vWaypoints;

public:
    ~CLane() {}
    CLane() {}

    void            AddWaypoint(const CVec2f &v2Waypoint);
    CVec2f          GetNextWaypoint(const CVec2f &v2Pos, const CVec2f &v2CurrentWaypoint);
};
//=============================================================================

#endif //__C_LANE_H__