// (C)2009 S2 Games
// c_convexpolygon2.h
//=============================================================================
#ifndef __C_CONVEXPOLYGON2_H__
#define __C_CONVEXPOLYGON2_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_plane2.h"
#include "intersection.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
const int   MAX_CONVEXPOLYGON2_VERTS(8);
//=============================================================================

//=============================================================================
// CConvexPolygon2
//=============================================================================
class CConvexPolygon2
{
private:
    CVec2f          m_aVerts[MAX_CONVEXPOLYGON2_VERTS];
    uint            m_uiNumVerts;

public:
    ~CConvexPolygon2() {}
    CConvexPolygon2() : m_uiNumVerts(0) {}
    inline CConvexPolygon2(const CConvexPolygon2 &c);

    uint            GetNumVerts() const     { return m_uiNumVerts; }
    const CVec2f&   GetVertex(uint n) const { return m_aVerts[n]; }

    inline void     AddVertex(const CVec2f &v2Point);
    inline void     Clip(const CPlane2 &plPlane);

    inline void     ClipPosX(float fX);
    inline void     ClipPosY(float fY);
    inline void     ClipNegX(float fX);
    inline void     ClipNegY(float fY);
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CConvexPolygon2::CConvexPolygon2
  ====================*/
CConvexPolygon2::CConvexPolygon2(const CConvexPolygon2 &c) : 
m_uiNumVerts(c.m_uiNumVerts)
{
    MemManager.Copy(m_aVerts, c.m_aVerts, sizeof(CVec2f) * m_uiNumVerts);
}


/*====================
  CConvexPolygon2::AddVertex
  ====================*/
void    CConvexPolygon2::AddVertex(const CVec2f &v2Point)
{
    if (m_uiNumVerts >= MAX_CONVEXPOLYGON2_VERTS)
        return;

    m_aVerts[m_uiNumVerts++] = v2Point;
}


/*====================
  CConvexPolygon2::Clip

  Clip everything on the positive side of the plane
  ====================*/
void    CConvexPolygon2::Clip(const CPlane2 &plPlane)
{
    if (m_uiNumVerts < 2)
        return;

    CVec2f aNewVerts[MAX_CONVEXPOLYGON2_VERTS];
    uint uiNumNewVerts(0);

    bool bPositive(plPlane.Distance(m_aVerts[0]) > 0.0f);

    for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
    {
        if (bPositive)
        {
            float d2(plPlane.Distance(m_aVerts[uiVert]));

            if (d2 > 0.0f)
                continue;
            else 
            {
                bPositive = false;

                // Crossing from negative into positive
                float d1(plPlane.Distance(m_aVerts[uiVert - 1]));
                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiVert - 1], m_aVerts[uiVert]);
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];
            }
        }
        else
        {
            float d2(plPlane.Distance(m_aVerts[uiVert]));

            if (d2 <= 0.0f)
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];
            else
            {
                bPositive = true;

                // Crossing from positive into negative
                float d1(plPlane.Distance(m_aVerts[uiVert - 1]));
                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiVert - 1], m_aVerts[uiVert]);
            }
        }
    }

    if (bPositive)
    {
        float d2(plPlane.Distance(m_aVerts[0]));

        if (d2 <= 0.0f)
        {
            // Crossing from negative into positive
            float d1(plPlane.Distance(m_aVerts[m_uiNumVerts - 1]));
            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[m_uiNumVerts - 1], m_aVerts[0]);
            aNewVerts[uiNumNewVerts++] = m_aVerts[0];
        }
    }
    else
    {
        float d2(plPlane.Distance(m_aVerts[0]));

        if (d2 <= 0.0f)
            aNewVerts[uiNumNewVerts++] = m_aVerts[0];
        else
        {
            // Crossing from positive into negative
            float d1(plPlane.Distance(m_aVerts[m_uiNumVerts - 1]));
            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[m_uiNumVerts - 1], m_aVerts[0]);
        }
    }

    if (uiNumNewVerts > 0)
        MemManager.Copy(m_aVerts, aNewVerts, sizeof(CVec2f) * uiNumNewVerts);
    m_uiNumVerts = uiNumNewVerts;
}


/*====================
  CConvexPolygon2::ClipPosX

  Clip everything on the positive side of x
  ====================*/
void    CConvexPolygon2::ClipPosX(float fX)
{
    if (m_uiNumVerts < 2)
        return;

    bool bClip(m_aVerts[0].x > fX);

    if (bClip)
    {
        uint uiEnter(INVALID_INDEX), uiLeave(INVALID_INDEX);

        for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
        {
            if (bClip)
            {
                bClip = m_aVerts[uiVert].x > fX;

                if (bClip)
                    continue;
                else 
                {
                    // Crossing from clipping to not clipping
                    uiEnter = uiVert - 1;
                }
            }
            else
            {
                bClip = m_aVerts[uiVert].x > fX;

                if (bClip)
                {
                    // Crossing from not clipping to clipping
                    uiLeave = uiVert - 1;
                    break;
                }
            }
        }

        if (uiEnter == INVALID_INDEX)
        {
            m_uiNumVerts = 0;
            return;
        }
        else
        {
            if (!bClip)
            {
                // Crossing from not clipping to clipping
                uiLeave = m_uiNumVerts - 1;
            }

            CVec2f aNewVerts[MAX_CONVEXPOLYGON2_VERTS];
            uint uiNumNewVerts(0);

            // Crossing from clipping to not clipping
            float d1(m_aVerts[uiEnter].x - fX);
            float d2(m_aVerts[uiEnter + 1].x - fX);

            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[uiEnter + 1]);
            aNewVerts[uiNumNewVerts++] = m_aVerts[uiEnter + 1];

            for (uint uiVert(uiEnter + 2); uiVert <= uiLeave; ++uiVert)
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];

            if (uiLeave == m_uiNumVerts - 1)
            {
                // Crossing from not clipping to clipping
                float d1(m_aVerts[m_uiNumVerts - 1].x - fX);
                float d2(m_aVerts[0].x - fX);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[m_uiNumVerts - 1], m_aVerts[0]);
            }
            else
            {
                // Crossing from not clipping to clipping
                float d1(m_aVerts[uiLeave].x - fX);
                float d2(m_aVerts[uiLeave + 1].x - fX);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiLeave], m_aVerts[uiLeave + 1]);
            }

            MemManager.Copy(m_aVerts, aNewVerts, sizeof(CVec2f) * uiNumNewVerts);
            m_uiNumVerts = uiNumNewVerts;
            return;
        }
    }
    else
    {
        uint uiLeave(uint(-1)), uiEnter(uint(-1));

        for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
        {
            if (bClip)
            {
                bClip = m_aVerts[uiVert].x > fX;

                if (bClip)
                    continue;
                else 
                {
                    // Crossing from clipping to not clipping
                    uiEnter = uiVert - 1;
                    break;
                }
            }
            else
            {
                bClip = m_aVerts[uiVert].x > fX;

                if (bClip)
                {
                    // Crossing from not clipping to clipping
                    uiLeave = uiVert - 1;
                }
            }
        }

        if (uiLeave == INVALID_INDEX)
        {
            return;
        }
        else
        {
            if (bClip)
            {
                // Crossing from clipping to not clipping
                uiEnter = m_uiNumVerts - 1;
            }

            CVec2f aNewVerts[MAX_CONVEXPOLYGON2_VERTS];
            uint uiNumNewVerts(0);

            for (uint uiVert(0); uiVert <= uiLeave; ++uiVert)
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];

            // Crossing from not clipping to clipping
            float d1(m_aVerts[uiLeave].x - fX);
            float d2(m_aVerts[uiLeave + 1].x - fX);

            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiLeave], m_aVerts[uiLeave + 1]);

            if (uiEnter == m_uiNumVerts - 1)
            {
                // Crossing from clipping to not clipping
                float d1(m_aVerts[uiEnter].x - fX);
                float d2(m_aVerts[0].x - fX);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[0]);
            }
            else
            {
                // Crossing from clipping to not clipping
                float d1(m_aVerts[uiEnter].x - fX);
                float d2(m_aVerts[uiEnter + 1].x - fX);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[uiEnter + 1]);

                for (uint uiVert(uiEnter + 1); uiVert < m_uiNumVerts; ++uiVert)
                    aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];
            }

            MemManager.Copy(m_aVerts, aNewVerts, sizeof(CVec2f) * uiNumNewVerts);
            m_uiNumVerts = uiNumNewVerts;
            return;
        }
    }
}


/*====================
  CConvexPolygon2::ClipPosY

  Clip everything on the positive side of Y
  ====================*/
void    CConvexPolygon2::ClipPosY(float fY)
{
    if (m_uiNumVerts < 2)
        return;

    bool bClip(m_aVerts[0].y > fY);

    if (bClip)
    {
        uint uiEnter(INVALID_INDEX), uiLeave(INVALID_INDEX);

        for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
        {
            if (bClip)
            {
                bClip = m_aVerts[uiVert].y > fY;

                if (bClip)
                    continue;
                else 
                {
                    // Crossing from clipping to not clipping
                    uiEnter = uiVert - 1;
                }
            }
            else
            {
                bClip = m_aVerts[uiVert].y > fY;

                if (bClip)
                {
                    // Crossing from not clipping to clipping
                    uiLeave = uiVert - 1;
                    break;
                }
            }
        }

        if (uiEnter == INVALID_INDEX)
        {
            m_uiNumVerts = 0;
            return;
        }
        else
        {
            if (!bClip)
            {
                // Crossing from not clipping to clipping
                uiLeave = m_uiNumVerts - 1;
            }

            CVec2f aNewVerts[MAX_CONVEXPOLYGON2_VERTS];
            uint uiNumNewVerts(0);

            // Crossing from clipping to not clipping
            float d1(m_aVerts[uiEnter].y - fY);
            float d2(m_aVerts[uiEnter + 1].y - fY);

            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[uiEnter + 1]);
            aNewVerts[uiNumNewVerts++] = m_aVerts[uiEnter + 1];

            for (uint uiVert(uiEnter + 2); uiVert <= uiLeave; ++uiVert)
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];

            if (uiLeave == m_uiNumVerts - 1)
            {
                // Crossing from not clipping to clipping
                float d1(m_aVerts[m_uiNumVerts - 1].y - fY);
                float d2(m_aVerts[0].y - fY);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[m_uiNumVerts - 1], m_aVerts[0]);
            }
            else
            {
                // Crossing from not clipping to clipping
                float d1(m_aVerts[uiLeave].y - fY);
                float d2(m_aVerts[uiLeave + 1].y - fY);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiLeave], m_aVerts[uiLeave + 1]);
            }

            MemManager.Copy(m_aVerts, aNewVerts, sizeof(CVec2f) * uiNumNewVerts);
            m_uiNumVerts = uiNumNewVerts;
            return;
        }
    }
    else
    {
        uint uiLeave(uint(-1)), uiEnter(uint(-1));

        for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
        {
            if (bClip)
            {
                bClip = m_aVerts[uiVert].y > fY;

                if (bClip)
                    continue;
                else 
                {
                    // Crossing from clipping to not clipping
                    uiEnter = uiVert - 1;
                    break;
                }
            }
            else
            {
                bClip = m_aVerts[uiVert].y > fY;

                if (bClip)
                {
                    // Crossing from not clipping to clipping
                    uiLeave = uiVert - 1;
                }
            }
        }

        if (uiLeave == INVALID_INDEX)
        {
            return;
        }
        else
        {
            if (bClip)
            {
                // Crossing from clipping to not clipping
                uiEnter = m_uiNumVerts - 1;
            }

            CVec2f aNewVerts[MAX_CONVEXPOLYGON2_VERTS];
            uint uiNumNewVerts(0);

            for (uint uiVert(0); uiVert <= uiLeave; ++uiVert)
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];

            // Crossing from not clipping to clipping
            float d1(m_aVerts[uiLeave].y - fY);
            float d2(m_aVerts[uiLeave + 1].y - fY);

            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiLeave], m_aVerts[uiLeave + 1]);

            if (uiEnter == m_uiNumVerts - 1)
            {
                // Crossing from clipping to not clipping
                float d1(m_aVerts[uiEnter].y - fY);
                float d2(m_aVerts[0].y - fY);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[0]);
            }
            else
            {
                // Crossing from clipping to not clipping
                float d1(m_aVerts[uiEnter].y - fY);
                float d2(m_aVerts[uiEnter + 1].y - fY);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[uiEnter + 1]);

                for (uint uiVert(uiEnter + 1); uiVert < m_uiNumVerts; ++uiVert)
                    aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];
            }

            MemManager.Copy(m_aVerts, aNewVerts, sizeof(CVec2f) * uiNumNewVerts);
            m_uiNumVerts = uiNumNewVerts;
            return;
        }
    }
}


/*====================
  CConvexPolygon2::ClipNegX

  Clip everything on the negative side of x
  ====================*/
void    CConvexPolygon2::ClipNegX(float fX)
{
    if (m_uiNumVerts < 2)
        return;

    bool bClip(m_aVerts[0].x < fX);

    if (bClip)
    {
        uint uiEnter(INVALID_INDEX), uiLeave(INVALID_INDEX);

        for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
        {
            if (bClip)
            {
                bClip = m_aVerts[uiVert].x < fX;

                if (bClip)
                    continue;
                else 
                {
                    // Crossing from clipping to not clipping
                    uiEnter = uiVert - 1;
                }
            }
            else
            {
                bClip = m_aVerts[uiVert].x < fX;

                if (bClip)
                {
                    // Crossing from not clipping to clipping
                    uiLeave = uiVert - 1;
                    break;
                }
            }
        }

        if (uiEnter == INVALID_INDEX)
        {
            m_uiNumVerts = 0;
            return;
        }
        else
        {
            if (!bClip)
            {
                // Crossing from not clipping to clipping
                uiLeave = m_uiNumVerts - 1;
            }

            CVec2f aNewVerts[MAX_CONVEXPOLYGON2_VERTS];
            uint uiNumNewVerts(0);

            // Crossing from clipping to not clipping
            float d1(fX - m_aVerts[uiEnter].x);
            float d2(fX - m_aVerts[uiEnter + 1].x);

            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[uiEnter + 1]);
            aNewVerts[uiNumNewVerts++] = m_aVerts[uiEnter + 1];

            for (uint uiVert(uiEnter + 2); uiVert <= uiLeave; ++uiVert)
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];

            if (uiLeave == m_uiNumVerts - 1)
            {
                // Crossing from not clipping to clipping
                float d1(fX - m_aVerts[m_uiNumVerts - 1].x);
                float d2(fX - m_aVerts[0].x);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[m_uiNumVerts - 1], m_aVerts[0]);
            }
            else
            {
                // Crossing from not clipping to clipping
                float d1(fX - m_aVerts[uiLeave].x);
                float d2(fX - m_aVerts[uiLeave + 1].x);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiLeave], m_aVerts[uiLeave + 1]);
            }

            MemManager.Copy(m_aVerts, aNewVerts, sizeof(CVec2f) * uiNumNewVerts);
            m_uiNumVerts = uiNumNewVerts;
            return;
        }
    }
    else
    {
        uint uiLeave(uint(-1)), uiEnter(uint(-1));

        for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
        {
            if (bClip)
            {
                bClip = m_aVerts[uiVert].x < fX;

                if (bClip)
                    continue;
                else 
                {
                    // Crossing from clipping to not clipping
                    uiEnter = uiVert - 1;
                    break;
                }
            }
            else
            {
                bClip = m_aVerts[uiVert].x < fX;

                if (bClip)
                {
                    // Crossing from not clipping to clipping
                    uiLeave = uiVert - 1;
                }
            }
        }

        if (uiLeave == INVALID_INDEX)
        {
            return;
        }
        else
        {
            if (bClip)
            {
                // Crossing from clipping to not clipping
                uiEnter = m_uiNumVerts - 1;
            }

            CVec2f aNewVerts[MAX_CONVEXPOLYGON2_VERTS];
            uint uiNumNewVerts(0);

            for (uint uiVert(0); uiVert <= uiLeave; ++uiVert)
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];

            // Crossing from not clipping to clipping
            float d1(fX - m_aVerts[uiLeave].x);
            float d2(fX - m_aVerts[uiLeave + 1].x);

            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiLeave], m_aVerts[uiLeave + 1]);

            if (uiEnter == m_uiNumVerts - 1)
            {
                // Crossing from clipping to not clipping
                float d1(fX - m_aVerts[uiEnter].x);
                float d2(fX - m_aVerts[0].x);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[0]);
            }
            else
            {
                // Crossing from clipping to not clipping
                float d1(fX - m_aVerts[uiEnter].x);
                float d2(fX - m_aVerts[uiEnter + 1].x);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[uiEnter + 1]);

                for (uint uiVert(uiEnter + 1); uiVert < m_uiNumVerts; ++uiVert)
                    aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];
            }

            MemManager.Copy(m_aVerts, aNewVerts, sizeof(CVec2f) * uiNumNewVerts);
            m_uiNumVerts = uiNumNewVerts;
            return;
        }
    }
}


/*====================
  CConvexPolygon2::ClipNegY

  Clip everything on the negative side of y
  ====================*/
void    CConvexPolygon2::ClipNegY(float fY)
{
    if (m_uiNumVerts < 2)
        return;

    bool bClip(m_aVerts[0].y < fY);

    if (bClip)
    {
        uint uiEnter(INVALID_INDEX), uiLeave(INVALID_INDEX);

        for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
        {
            if (bClip)
            {
                bClip = m_aVerts[uiVert].y < fY;

                if (bClip)
                    continue;
                else 
                {
                    // Crossing from clipping to not clipping
                    uiEnter = uiVert - 1;
                }
            }
            else
            {
                bClip = m_aVerts[uiVert].y < fY;

                if (bClip)
                {
                    // Crossing from not clipping to clipping
                    uiLeave = uiVert - 1;
                    break;
                }
            }
        }

        if (uiEnter == INVALID_INDEX)
        {
            m_uiNumVerts = 0;
            return;
        }
        else
        {
            if (!bClip)
            {
                // Crossing from not clipping to clipping
                uiLeave = m_uiNumVerts - 1;
            }

            CVec2f aNewVerts[MAX_CONVEXPOLYGON2_VERTS];
            uint uiNumNewVerts(0);

            // Crossing from clipping to not clipping
            float d1(fY - m_aVerts[uiEnter].y);
            float d2(fY - m_aVerts[uiEnter + 1].y);

            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[uiEnter + 1]);
            aNewVerts[uiNumNewVerts++] = m_aVerts[uiEnter + 1];

            for (uint uiVert(uiEnter + 2); uiVert <= uiLeave; ++uiVert)
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];

            if (uiLeave == m_uiNumVerts - 1)
            {
                // Crossing from not clipping to clipping
                float d1(fY - m_aVerts[m_uiNumVerts - 1].y);
                float d2(fY - m_aVerts[0].y);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[m_uiNumVerts - 1], m_aVerts[0]);
            }
            else
            {
                // Crossing from not clipping to clipping
                float d1(fY - m_aVerts[uiLeave].y);
                float d2(fY - m_aVerts[uiLeave + 1].y);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiLeave], m_aVerts[uiLeave + 1]);
            }

            MemManager.Copy(m_aVerts, aNewVerts, sizeof(CVec2f) * uiNumNewVerts);
            m_uiNumVerts = uiNumNewVerts;
            return;
        }
    }
    else
    {
        uint uiLeave(uint(-1)), uiEnter(uint(-1));

        for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
        {
            if (bClip)
            {
                bClip = m_aVerts[uiVert].y < fY;

                if (bClip)
                    continue;
                else 
                {
                    // Crossing from clipping to not clipping
                    uiEnter = uiVert - 1;
                    break;
                }
            }
            else
            {
                bClip = m_aVerts[uiVert].y < fY;

                if (bClip)
                {
                    // Crossing from not clipping to clipping
                    uiLeave = uiVert - 1;
                }
            }
        }

        if (uiLeave == INVALID_INDEX)
        {
            return;
        }
        else
        {
            if (bClip)
            {
                // Crossing from clipping to not clipping
                uiEnter = m_uiNumVerts - 1;
            }

            CVec2f aNewVerts[MAX_CONVEXPOLYGON2_VERTS];
            uint uiNumNewVerts(0);

            for (uint uiVert(0); uiVert <= uiLeave; ++uiVert)
                aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];

            // Crossing from not clipping to clipping
            float d1(fY - m_aVerts[uiLeave].y);
            float d2(fY - m_aVerts[uiLeave + 1].y);

            float fFraction(d1 / (d1 - d2));

            aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiLeave], m_aVerts[uiLeave + 1]);

            if (uiEnter == m_uiNumVerts - 1)
            {
                // Crossing from clipping to not clipping
                float d1(fY - m_aVerts[uiEnter].y);
                float d2(fY - m_aVerts[0].y);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[0]);
            }
            else
            {
                // Crossing from clipping to not clipping
                float d1(fY - m_aVerts[uiEnter].y);
                float d2(fY - m_aVerts[uiEnter + 1].y);

                float fFraction(d1 / (d1 - d2));

                aNewVerts[uiNumNewVerts++] = LERP(fFraction, m_aVerts[uiEnter], m_aVerts[uiEnter + 1]);

                for (uint uiVert(uiEnter + 1); uiVert < m_uiNumVerts; ++uiVert)
                    aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];
            }

            MemManager.Copy(m_aVerts, aNewVerts, sizeof(CVec2f) * uiNumNewVerts);
            m_uiNumVerts = uiNumNewVerts;
            return;
        }
    }
}
//=============================================================================

#endif //__C_CONVEXPOLYGON2_H__
