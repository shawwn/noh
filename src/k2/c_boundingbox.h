// (C)2005 S2 Games
// c_boundingbox.h
//=============================================================================
#ifndef __C_BOUNDINGBOX_H__
#define __C_BOUNDINGBOX_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec3.h"
#include "c_axis.h"
#include "c_rect.h"
//=============================================================================

//=============================================================================
// CBoundingBox
//=============================================================================
template <class T>
class CBoundingBox
{
private:
    CVec3<T>    m_v3Min, m_v3Max;

public:
    ~CBoundingBox() {}

    // Constructors
    CBoundingBox() :
    m_v3Min(FAR_AWAY, FAR_AWAY, FAR_AWAY),
    m_v3Max(-FAR_AWAY, -FAR_AWAY, -FAR_AWAY)
    {}

    CBoundingBox(const vector< CVec3<T> > &vPoints) :
    m_v3Min(FAR_AWAY, FAR_AWAY, FAR_AWAY),
    m_v3Max(-FAR_AWAY, -FAR_AWAY, -FAR_AWAY)
    {
        if (vPoints.empty())
        {
            m_v3Min.Clear();
            m_v3Max.Clear();
            return;
        }

        for (typename vector< CVec3<T> >::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
            AddPoint(*it);
    }

    CBoundingBox(const CVec3<T> &v3Min, const CVec3<T> &v3Max) :
    m_v3Min(v3Min),
    m_v3Max(v3Max)
    {
    }

    CBoundingBox(float fMin, float fMax) :
    m_v3Min(CVec3f(fMin, fMin, fMin)),
    m_v3Max(CVec3f(fMax, fMax, fMax))
    {
    }

    CBoundingBox(float fMin, float fMax, const CVec3f &v3Pos) :
    m_v3Min(CVec3f(fMin, fMin, fMin) + v3Pos),
    m_v3Max(CVec3f(fMax, fMax, fMax) + v3Pos)
    {
    }

    // Accessors
    const CVec3<T>& GetMin() const  { return m_v3Min; }
    const CVec3<T>& GetMax() const  { return m_v3Max; }

    CVec3<T>&   GetMinRef()     { return m_v3Min; }
    CVec3<T>&   GetMaxRef()     { return m_v3Max; }

    T   GetDim(EVectorComponent e) const    { return m_v3Max[e] - m_v3Min[e]; }
    T   GetMin(EVectorComponent e) const    { return m_v3Min[e]; }
    T   GetMax(EVectorComponent e) const    { return m_v3Max[e]; }

    // Utlities
    void    Clear()
    {
        m_v3Min.Set(FAR_AWAY, FAR_AWAY, FAR_AWAY);
        m_v3Max.Set(-FAR_AWAY, -FAR_AWAY, -FAR_AWAY);
    }

    void    Zero()
    {
        m_v3Min.Clear();
        m_v3Max.Clear();
    }

    bool    IsZero() const
    {
        return (
            (m_v3Min.x == 0.0f) &&
            (m_v3Min.y == 0.0f) &&
            (m_v3Min.z == 0.0f) &&
            (m_v3Max.x == 0.0f) &&
            (m_v3Max.y == 0.0f) &&
            (m_v3Max.z == 0.0f));
    }

    void    Set(T xMin, T yMin, T zMin, T xMax, T yMax, T zMax)
    {
        m_v3Min.Set(xMin, yMin, zMin);
        m_v3Max.Set(xMax, yMax, zMax);
    }

    void    Set(const CVec3<T> v3Min, const CVec3<T> v3Max)
    {
        m_v3Min = v3Min;
        m_v3Max = v3Max;
    }

    void    SetCylinder(float fRadius, float fHeight)
    {
        m_v3Min.Set(-fRadius, -fRadius, 0.0f);
        m_v3Max.Set(fRadius, fRadius, fHeight);
    }

    void    SetSphere(float fRadius)
    {
        m_v3Min.Set(-fRadius, -fRadius, -fRadius);
        m_v3Max.Set(fRadius, fRadius, fRadius);
    }

    void    AddPoint(const CVec3<T> &v3Point)
    {
        m_v3Min.x = MIN(m_v3Min.x, v3Point.x);
        m_v3Min.y = MIN(m_v3Min.y, v3Point.y);
        m_v3Min.z = MIN(m_v3Min.z, v3Point.z);

        m_v3Max.x = MAX(m_v3Max.x, v3Point.x);
        m_v3Max.y = MAX(m_v3Max.y, v3Point.y);
        m_v3Max.z = MAX(m_v3Max.z, v3Point.z);
    }

    void    AddPoints(const vector< CVec3<T> > &vPoints)
    {
        for (typename vector< CVec3<T> >::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
            AddPoint(*it);
    }

    void    AddBox(const CBoundingBox<T> &bb)
    {
        m_v3Min.x = MIN(m_v3Min.x, bb.m_v3Min.x);
        m_v3Min.y = MIN(m_v3Min.y, bb.m_v3Min.y);
        m_v3Min.z = MIN(m_v3Min.z, bb.m_v3Min.z);

        m_v3Max.x = MAX(m_v3Max.x, bb.m_v3Max.x);
        m_v3Max.y = MAX(m_v3Max.y, bb.m_v3Max.y);
        m_v3Max.z = MAX(m_v3Max.z, bb.m_v3Max.z);
    }

    void    Scale(float fScale)
    {
        m_v3Min *= fScale;
        m_v3Max *= fScale;
    }

    void    ScaleXY(float fScale)
    {
        m_v3Min[X] *= fScale;
        m_v3Min[Y] *= fScale;
        m_v3Max[X] *= fScale;
        m_v3Max[Y] *= fScale;
    }

    void    Expand(T amt)
    {
        m_v3Min.x -= amt;
        m_v3Min.y -= amt;
        m_v3Min.z -= amt;
        m_v3Max.x += amt;
        m_v3Max.y += amt;
        m_v3Max.z += amt;
    }

    void    ExpandXY(T x, T y)
    {
        m_v3Min.x -= x;
        m_v3Min.y -= y;
        m_v3Max.x += x;
        m_v3Max.y += y;
    }

    void    ExpandXY(T amt)
    {
        m_v3Min.x -= amt;
        m_v3Min.y -= amt;
        m_v3Max.x += amt;
        m_v3Max.y += amt;
    }

    void    StretchZ(float fScale)
    {
        if (fScale > 0.0f)
            m_v3Max[Z] += (m_v3Max[Z] - m_v3Min[Z]) * fScale;
        else
            m_v3Min[Z] += (m_v3Max[Z] - m_v3Min[Z]) * fScale;
    }

    vector< CVec3<T> >  GetCorners()
    {
        vector< CVec3<T> >  vCorners;
        for (int i(0); i < 8; ++i)
        {
            vCorners.push_back(CVec3<T>(
                (i & BIT(0)) ? m_v3Min.x : m_v3Max.x,
                (i & BIT(1)) ? m_v3Min.y : m_v3Max.y,
                (i & BIT(2)) ? m_v3Min.z : m_v3Max.z));
        }
        return vCorners;
    }

    void    Transform(const CVec3<T> &v3Pos, const CAxis &axis, float fScale)
    {
        CVec3<T> v3Min(m_v3Min);
        CVec3<T> v3Max(m_v3Max);
        Clear();

        for (uint i(0); i < 8; ++i)
        {
            CVec3<T> v3Point
            (
                (i & BIT(0)) ? v3Min.x : v3Max.x,
                (i & BIT(1)) ? v3Min.y : v3Max.y,
                (i & BIT(2)) ? v3Min.z : v3Max.z
            );

            AddPoint(TransformPoint(v3Point, axis, v3Pos, fScale));
        }
    }

    void    TransformXY(const CVec3<T> &v3Point, const CAxis &axis)
    {
        T minz(m_v3Min[Z]);
        T maxz(m_v3Max[Z]);
        Transform(v3Point, axis, 1.0f);
        m_v3Min[Z] = minz;
        m_v3Max[Z] = maxz;
    }

    void    Offset(const CVec3<T> &v3Point)
    {
        m_v3Min += v3Point;
        m_v3Max += v3Point;
    }

    bool    Contains(const CVec3<T> &v3Point) const
    {
        if (v3Point.x < m_v3Min.x || v3Point.y < m_v3Min.y || v3Point.z < m_v3Min.z ||
            v3Point.x > m_v3Max.x || v3Point.y > m_v3Max.y || v3Point.z > m_v3Max.z)
            return false;

        return true;
    }

    CVec3<T>    GetMid() const
    {
        return (m_v3Min + m_v3Max) / 2.0f;
    }

    CVec3<T>    Clamp(const CVec3<T> &v3Point) const
    {
        return CVec3f(CLAMP(v3Point.x, m_v3Min.x, m_v3Max.x), CLAMP(v3Point.y, m_v3Min.y, m_v3Max.y), CLAMP(v3Point.z, m_v3Min.z, m_v3Max.z));
    }

    CBoundingBox<T>&    operator+=(const CVec3<T> &v3Point)     { AddPoint(v3Point); return *this; }
    CBoundingBox<T>&    operator+=(const CBoundingBox<T> &bb)   { AddBox(bb); return *this; }
    CBoundingBox<T>&    operator*=(float fScale)                { Scale(fScale); return *this; }
    
    CBoundingBox<T>     operator+(const CVec3<T> &v3Offset) const   { return CBoundingBox(m_v3Min + v3Offset, m_v3Max + v3Offset); }

    CBoundingBox<T>     operator*(const CVec3<T> &b) const  { return CBoundingBox(m_v3Min * b, m_v3Max * b); }
    CBoundingBox<T>     operator/(const CVec3<T> &b) const  { return CBoundingBox(m_v3Min / b, m_v3Max / b); }
    CBoundingBox<T>     operator*(T s) const                { return CBoundingBox(m_v3Min * s, m_v3Max * s); }
    CBoundingBox<T>     operator/(T s) const                { return CBoundingBox(m_v3Min / s, m_v3Max / s); }

    CRect<T>            GetXYRect() const                   { return CRect<T>(m_v3Min.x, m_v3Min.y, m_v3Max.x, m_v3Max.y); }
};
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef CBoundingBox<int>   CBBoxi;
typedef CBoundingBox<float> CBBoxf;

// Export an instance of CBBoxf
// See http://support.microsoft.com/default.aspx?scid=kb;EN-US;168958
#ifdef _WIN32
#pragma warning (disable : 4231)
K2_EXTERN template class K2_API CBoundingBox<float>;
#pragma warning (default : 4231)
#endif

/*====================
  IntersectBounds
  ====================*/
template <class T>
inline bool IntersectBounds(const CBoundingBox<T> &bbA, const CBoundingBox<T> &bbB)
{
    const CVec3<T> &v3MinA(bbA.GetMin());
    const CVec3<T> &v3MaxA(bbA.GetMax());
    const CVec3<T> &v3MinB(bbB.GetMin());
    const CVec3<T> &v3MaxB(bbB.GetMax());

    if (v3MinA.x > v3MaxB.x || v3MaxA.x < v3MinB.x ||
        v3MinA.y > v3MaxB.y || v3MaxA.y < v3MinB.y ||
        v3MinA.z > v3MaxB.z || v3MaxA.z < v3MinB.z)
        return false;

    return true;
}


/*====================
  IntersectBounds
  ====================*/
template <class T>
inline bool IntersectBounds(const CBoundingBox<T> &bbA, const CRect<T> &recB)
{
    const CVec3<T> &v3MinA(bbA.GetMin());
    const CVec3<T> &v3MaxA(bbA.GetMax());

    if (v3MinA.x > recB.right || v3MaxA.x < recB.left ||
        v3MinA.y > recB.bottom || v3MaxA.y < recB.top)
        return false;

    return true;
}

//=============================================================================

#endif // __C_BOUNDINGBOX_H__
