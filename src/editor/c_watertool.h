// (C)2010 S2 Games
// c_watertool.h
//
//=============================================================================
#ifndef __C_WATERTOOL_H__
#define __C_WATERTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_toolbox.h"
#include "i_tool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBrush;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EWaterMode
{
    WATER_PAINT = 0,
    WATER_ALPHA,
    WATER_FILL
};

    // Ramp Flags per tile.
//const uint RAMP_FLAG_SET                  (BIT(0));
//=============================================================================

//=============================================================================
// CWaterTool
//=============================================================================
class CWaterTool : public ITool
{
private:
    bool        m_bValidPosition;
    int         m_iWaterHeight;
    bool        m_bWorking;
    bool        m_bInverse;
    bool        m_bPrimaryDown;
    bool        m_bSecondaryDown;
    bool        m_bFirstLoop;
    uint*       m_uiWaterDrewMap;
    uint        m_uiMapSize;
    CVec3f      m_v3EndPos;
    ResHandle   m_hLineMaterial;
    ResHandle   m_hWaterMaterial;
    ResHandle   m_hFont;
    tstring     m_sLastWaterDef;
    ResHandle   m_WaterDefinitionHandle;
    int         m_iOldX, m_iOldY, m_iXOffset, m_iYOffset, m_iXWaterCenter, m_iYWaterCenter, m_iXPaint, m_iYPaint;

    static void WaterModify(byte *pRegion, const CRecti &recArea, const CBrush &brush, bool bAdd);

public:
    CWaterTool();
    ~CWaterTool()               { if (m_uiWaterDrewMap) K2_DELETE_ARRAY(m_uiWaterDrewMap);}


    void    CalcToolProperties();
    void    WaterDraw();
    void    ClampWaterRectToGrid(CRecti *rArea);
    void    ClampRectToGrid(CRecti *rArea);

    void    PrimaryUp();
    void    PrimaryDown();
    void    SecondaryUp();
    void    SecondaryDown();
    void    TertiaryUp();
    void    TertiaryDown();
    void    QuaternaryUp();
    void    QuaternaryDown();

    void    Cancel();
    void    Delete();
    void    Enter();

    void    Frame(float fFrameTime);

    void    Draw();
    void    Render();
};
//=============================================================================

#endif // __C_WATERTOOL_H__
