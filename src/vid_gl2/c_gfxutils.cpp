// (C)2008 S2 Games
// c_gfxutils.cpp
//
// Textures
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfxutils.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CGfxUtils)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGfxUtils *GfxUtils(CGfxUtils::GetInstance());
//=============================================================================

/*====================
  CGfxUtils::~CGfxUtils
  ====================*/
CGfxUtils::~CGfxUtils()
{
}


/*====================
  CGfxUtils::CGfxUtils
  ====================*/
CGfxUtils::CGfxUtils()
{
}


/*====================
  CGfxUtils::GetCurrentColor
  ====================*/
float   CGfxUtils::GetCurrentColor(int channel)
{
    if(channel < 0 || channel > 3)
        return 0;
    return currentColor[channel];
}


/*====================
  CGfxUtils::GetCurrentColor
  ====================*/
uint    CGfxUtils::GetCurrentColor()
{
    return D3DCOLOR_ARGB
    (
        CLAMP(INT_FLOOR(currentColor[3] * 255.0f), 0, 255),
        CLAMP(INT_FLOOR(currentColor[2] * 255.0f), 0, 255),
        CLAMP(INT_FLOOR(currentColor[1] * 255.0f), 0, 255),
        CLAMP(INT_FLOOR(currentColor[0] * 255.0f), 0, 255)
    );
}


/*====================
  CGfxUtils::SetCurrentColor
  ====================*/
void    CGfxUtils::SetCurrentColor(CVec4f v4Color)
{
    currentColor[R] = v4Color[R];
    currentColor[G] = v4Color[G];
    currentColor[B] = v4Color[B];
    currentColor[A] = v4Color[A];
}


/*====================
  CGfxUtils::AxisToMatrix
  ====================*/
void    CGfxUtils::AxisToMatrix(const CAxis &axis, D3DXMATRIXA16 *tm)
{
    (*tm)[0] = axis[RIGHT][X];
    (*tm)[1] = axis[RIGHT][Y];
    (*tm)[2] = axis[RIGHT][Z];
    (*tm)[3] = 0.0f;

    (*tm)[4] = axis[FORWARD][X];
    (*tm)[5] = axis[FORWARD][Y];
    (*tm)[6] = axis[FORWARD][Z];
    (*tm)[7] = 0.0f;

    (*tm)[8] = axis[UP][X];
    (*tm)[9] = axis[UP][Y];
    (*tm)[10] = axis[UP][Z];
    (*tm)[11] = 0.0f;

    (*tm)[12] = 0.0f;
    (*tm)[13] = 0.0f;
    (*tm)[14] = 0.0f;
    (*tm)[15] = 1.0f;
}


/*====================
  CGfxUtils::TransformToMatrix
  ====================*/
void    CGfxUtils::TransformToMatrix(const matrix43_t *transform, D3DXMATRIXA16 *tm)
{
    (*tm)[0] = transform->axis[0][0];
    (*tm)[1] = transform->axis[0][1];
    (*tm)[2] = transform->axis[0][2];
    (*tm)[3] = 0.0f;
    (*tm)[4] = transform->axis[1][0];
    (*tm)[5] = transform->axis[1][1];
    (*tm)[6] = transform->axis[1][2];
    (*tm)[7] = 0.0f;
    (*tm)[8] = transform->axis[2][0];
    (*tm)[9] = transform->axis[2][1];
    (*tm)[10] = transform->axis[2][2];
    (*tm)[11] = 0.0f;
    (*tm)[12] = transform->pos[0];
    (*tm)[13] = transform->pos[1];
    (*tm)[14] = transform->pos[2];
    (*tm)[15] = 1.0f;
}


/*====================
  CGfxUtils::TransformPoint
  ====================*/
CVec3f  CGfxUtils::TransformPoint(const CVec3f &vPoint, const D3DXMATRIXA16 &mTransform)
{
    D3DXVECTOR4 v(vPoint.x, vPoint.y, vPoint.z, 1.0f);
    D3DXVec4Transform(&v, &v, &mTransform);
    return CVec3f(v.x/v.w, v.y/v.w, v.z/v.w);
}


/*====================
  CGfxUtils::TransformNormal
  ====================*/
CVec3f  CGfxUtils::TransformNormal(const CVec3f &vNormal, const D3DXMATRIXA16 &mTransform)
{
    D3DXVECTOR3 v(vNormal.x, vNormal.y, vNormal.z);

    D3DXVec3TransformNormal(&v, &v, &mTransform);

    return CVec3f(v.x, v.y, v.z);
}


/*====================
  CGfxUtils::GetMaterial
  ====================*/
CMaterial&  CGfxUtils::GetMaterial(ResHandle hMaterial)
{
    return *g_ResourceManager.GetMaterial(hMaterial);
}


/*====================
  GL_CheckFrameBufferStatus
  ====================*/
bool    GL_CheckFrameBufferStatus(const tstring &sName)
{
    GLenum status(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
    switch (status)
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        return true;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        Console.Warn << sName << _T(" - Unsupported framebuffer format") << newl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        Console.Warn << sName << _T(" - Framebuffer incomplete attachment") << newl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        Console.Warn << sName << _T(" - Framebuffer incomplete, missing attachment") << newl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        Console.Warn << sName << _T(" - Framebuffer incomplete, attached images must have same dimensions") << newl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        Console.Warn << sName << _T(" - Framebuffer incomplete, attached images must have same format") << newl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        Console.Warn << sName << _T(" - Framebuffer incomplete, missing draw buffer") << newl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        Console.Warn << sName << _T(" - Framebuffer incomplete, missing read buffer") << newl;
        return false;
    default:
        assert(false);
        return false;
    }
}


/*====================
  CGfxUtils::TransformPoints
  ====================*/
void    CGfxUtils::TransformPoints(vector<CVec3f> &vPoints, D3DXMATRIXA16 *mTransform)
{
    for (vector<CVec3f>::iterator it = vPoints.begin(); it != vPoints.end(); ++it)
    {
        D3DXVECTOR4 v(it->x, it->y, it->z, 1.0f);

        D3DXVec4Transform(&v, &v, mTransform);

        it->x = v.x/v.w;
        it->y = v.y/v.w;
        it->z = v.z/v.w;
    }
}
