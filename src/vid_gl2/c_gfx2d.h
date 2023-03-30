// (C)2008 S2 Games
// c_gfx2d.h
//
//=============================================================================
#ifndef __C_GFX2D_H__
#define __C_GFX2D_H__

//=============================================================================
// Definitions
//=============================================================================
const int   MAX_GUIQUADS = 16384;

enum EGuiQuadType
{
    GUI_RECT,
    GUI_QUAD,
    GUI_LINE
};

struct SGuiQuad
{
    EGuiQuadType    eType;
    float           x[4], y[4];
    float           s[4], t[4];
    ResHandle       hTexture;
    int             iFlags;
    float           color[4];
};
//=============================================================================

//=============================================================================
// CGfx2D
//=============================================================================
class CGfx2D
{
    SINGLETON_DEF(CGfx2D)

protected:
    int         iNumGuiElements;
    SGuiQuad    GuiElements[MAX_GUIQUADS];

    GLuint      VBGuiQuad;      // Vertex buffer for gui quads

    void    Setup2D();
    void    Exit2D();

public:
    ~CGfx2D();

    void    Init();
    void    Shutdown();

    void    AddRect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags);
    void    AddLine(const CVec2f& v1, const CVec2f& v2, const CVec4f &v4Color1, const CVec4f &v4Color2, int iFlags);
    void    AddQuad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4, const CVec2f& t1, const CVec2f& t2, const CVec2f& t3, const CVec2f& t4, ResHandle hTexture, int iFlags);

    void    Draw();
    void    ForceEmpty();
};
extern CGfx2D *Gfx2D;
//=============================================================================

#endif //__C_GFX2D_H__

