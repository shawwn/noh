// (C)2005 S2 Games
// c_lightmaptool.h
//
//=============================================================================
#ifndef __C_LIGHTMAPTOOL_H__
#define __C_LIGHTMAPTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_vec4.h"

#include "c_toolbox.h"
#include "i_tool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBrush;
//=============================================================================

//=============================================================================
// CLightmapTool
// Lightmap Tool
//=============================================================================
class CLightmapTool : public ITool
{
private:
    bool    m_bValidPosition;
    CVec4f  m_vecColor;
    bool    m_bWorking;
    bool    m_bInverse;

    static void LerpColor(CVec4b *pRegion, const CRecti &recArea, const CBrush &brush, const CVec4f &color, float fScale);

public:
    CLightmapTool();
    virtual ~CLightmapTool()                {}

    void    CalcToolProperties();
    void    PaintVertex(float fFrameTime);

    void    PrimaryUp();
    void    PrimaryDown();
    void    SecondaryUp();
    void    SecondaryDown();
    void    TertiaryUp()        {}
    void    TertiaryDown()      {}
    void    QuaternaryUp()      {}
    void    QuaternaryDown()    {}

    void    Cancel()            {}
    void    Delete()            {}

    void    Frame(float fFrameTime);
};
//=============================================================================
#endif //__C_LIGHTMAPTOOL_H__
