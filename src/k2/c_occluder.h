// (C)2005 S2 Games
// c_occluder.h
//
//=============================================================================
#ifndef __C_OCCLUDER_H__
#define __C_OCCLUDER_H__

//=============================================================================
// Definitions
//=============================================================================
const int MAX_OCCLUDER_POINTS(16);
//=============================================================================

//=============================================================================
// COccluder
//=============================================================================
class COccluder
{
private:
    // TODO: Manage the verts in a better way
    uint    m_uiNumPoints;
    CVec3f  m_Points[MAX_OCCLUDER_POINTS];
    bool    m_bCull;

public:
    ~COccluder()    {}
    COccluder() :
    m_uiNumPoints(0),
    m_bCull(false)
    {
        MemManager.Set(m_Points, 0, sizeof(CVec3f) * MAX_OCCLUDER_POINTS);
    }

    bool    IsCulled() const            { return m_bCull; }
    void    SetCulling(bool bCull)      { m_bCull = bCull; }

    uint            GetNumPoints() const                        { return m_uiNumPoints; }
    const CVec3f&   GetPoint(uint uiIndex) const                { return m_Points[uiIndex]; }
    void            SetPoint(uint uiIndex, const CVec3f &v3)    { m_Points[uiIndex] = v3; }
    CVec3f*         GetPoints()                                 { return m_Points; }
    void            SetNumPoints(uint ui)                       { m_uiNumPoints = ui; }
    void            AddPoint(const CVec3f &v3Point)             { if (m_uiNumPoints >= MAX_OCCLUDER_POINTS) return; m_Points[m_uiNumPoints++] = v3Point; }

    CPlane          GetPlane() const
    {
        // Find a valid plane
        for (uint v1(0); v1 < m_uiNumPoints; ++v1)
        {
            for (uint v2(v1 + 1); v2 < m_uiNumPoints; ++v2)
            {
                for (uint v3(v2 + 1); v3 < m_uiNumPoints; ++v3)
                {
                    CPlane plane(m_Points[v1], m_Points[v2], m_Points[v3], true);
                    if (plane.IsValid())
                        return plane;
                }
            }
        }

        return CPlane(0.0f, 0.0f, 0.0f, 0.0f);
    }
};
//=============================================================================
#endif  //__C_OCCLUDER_H__
