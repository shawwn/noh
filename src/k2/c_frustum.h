// (C)2005 S2 Games
// c_frustum.h
//=============================================================================
#ifndef __C_FRUSTUM_H__
#define __C_FRUSTUM_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "c_vec.h"
#include "c_plane.h"
#include "c_boundingcone.h"
#include "c_axis.h"
#include "c_boundingbox.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSphere;
class CCamera;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int NUM_FRUSTUM_PLANES(6);
//=============================================================================

//=============================================================================
// CFrustum
//=============================================================================
class CFrustum
{
private:
    CVec3f  m_vOrigin;
    float   m_fFovX;
    float   m_fFovY;
    float   m_fNear;
    float   m_fFar;
    CAxis   m_Axis;
    CBBoxf  m_bbBounds;

    CPlane  m_Planes[NUM_FRUSTUM_PLANES];

    void    CalcPlanes();

    float   Tighten(const vector<CVec3f> &vPoints);

public:
    K2_API  CFrustum();
    K2_API  CFrustum(const CVec3f &vOrigin, const CAxis &aAxis, float fFovX, float fFovY, float fNear, float fFar);
    K2_API  CFrustum(const CPlane &Planes);
    K2_API  CFrustum(const vector<CVec3f> &vPoints, const CVec3f &vOrigin, const CVec3f &vDirection);
    K2_API  CFrustum(const vector<CVec3f> &vPoints, const CVec3f &vOrigin, const CAxis &aAxis);
    K2_API  CFrustum(const vector<CVec3f> &vPoints, const CVec3f &vOrigin);
    K2_API  CFrustum(const CBoundingCone &BoundingCone);

    //K2_API    void    Update(const CVec3f &vOrigin, const CAxis &aAxis, float fFovX, float fFovY, float fNear, float fFar);
    K2_API  void    Update(const CCamera &camera);

    const CVec3f&   GetOrigin()     { return m_vOrigin; }
    const CAxis&    GetAxis()       { return m_Axis; }
    float           GetFovX()       { return m_fFovX; }
    float           GetFovY()       { return m_fFovY; }
    float           GetNear()       { return m_fNear; }
    float           GetFar()        { return m_fFar; }

    const CAxis&    GetAxis() const                 { return m_Axis; }
    const CVec3f&   GetAxis(EAxisComponent e) const { return m_Axis[e]; }

    CBBoxf&         GetBounds()     { return m_bbBounds; }
    const CPlane&   GetPlane(uint ui)   { return m_Planes[ui]; }

    K2_API  bool    Touches(const CBBoxf &bbBox) const;
    K2_API  bool    Touches(const CVec3f &vPoint) const;
    K2_API  bool    Touches(const CSphere &sSphere) const;
    K2_API  bool    Touches(const CBBoxf &bbBox, const CAxis &aAxis, const CVec3f &v3Pos) const;
};
//=============================================================================
#endif // __C_FRUSTUM_H__
