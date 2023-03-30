// (C)2008 S2 Games
// c_gfxutils.h
//
//=============================================================================
#ifndef __C_GFXUTILS_H__
#define __C_GFXUTILS_H__

//=============================================================================
// Definitions
//=============================================================================
class CMaterial;
typedef pair<tstring, tstring> StringPair;

bool    GL_CheckFrameBufferStatus(const tstring &sName);

inline uint     GL_Color(const CVec4f &v4Color)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    return uint
    (
        CLAMP(INT_FLOOR(v4Color[0] * 255.0f), 0, 255) + 
        (CLAMP(INT_FLOOR(v4Color[1] * 255.0f), 0, 255) << 8) + 
        (CLAMP(INT_FLOOR(v4Color[2] * 255.0f), 0, 255) << 16) +
        (CLAMP(INT_FLOOR(v4Color[3] * 255.0f), 0, 255) << 24)
    );
#else
    return uint
    (
        CLAMP(INT_FLOOR(v4Color[3] * 255.0f), 0, 255) + 
        (CLAMP(INT_FLOOR(v4Color[2] * 255.0f), 0, 255) << 8) + 
        (CLAMP(INT_FLOOR(v4Color[1] * 255.0f), 0, 255) << 16) +
        (CLAMP(INT_FLOOR(v4Color[0] * 255.0f), 0, 255) << 24)
    );
#endif
}
//=============================================================================

//=============================================================================
// CGfxUtils
//=============================================================================
class CGfxUtils
{
    SINGLETON_DEF(CGfxUtils)
public:
    int         ParseDefinitions(const tstring &sString, vector<StringPair> &v);
    CMaterial&  GetMaterial(ResHandle hMaterial);
        
    void        TransformPoints(vector<CVec3f> &vPoints, D3DXMATRIXA16 *mTransform);
    CVec3f      TransformPoint(const CVec3f &vPoint, const D3DXMATRIXA16 &mTransform);
    CVec3f      TransformNormal(const CVec3f &vNormal, const D3DXMATRIXA16 &mTransform);

    void        TransformToMatrix(const matrix43_t *transform, D3DXMATRIXA16 *tm);
    void        AxisToMatrix(const CAxis &axis, D3DXMATRIXA16 *tm);

    uint        GetCurrentColor();
    float       GetCurrentColor(int channel);
    void        SetCurrentColor(CVec4f v4Color);

    // TODO
    //uint      D3D_DWORD(float fValue);

    void        BGRAtoRGBA(dword &c);

    ~CGfxUtils();
protected:
    float       currentColor[4];
};
extern CGfxUtils *GfxUtils;
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CGfxUtils::BGRAtoRGBA
  ====================*/
inline
void    CGfxUtils::BGRAtoRGBA(dword &c)
{
    dword r, b;
    r = (c&(255<<16))>>16;
    b = c&(255);
    c &= ~(255|(255<<16));
    c |= r|(b<<16);
}
//=============================================================================

#endif //__C_GFXUTILS_H__
