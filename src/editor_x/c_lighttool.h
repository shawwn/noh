// (C)2005 S2 Games
// c_lighttool.h
//
//=============================================================================
#ifndef __C_LIGHTTOOL_H__
#define __C_LIGHTTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_toolbox.h"
#include "i_tool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldLight;

const int NUM_CIRCLE_SEGMENTS = 64;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ELightEditMode
{
    LIGHT_CREATE = 0,
    LIGHT_SELECT,
    LIGHT_TRANSLATE_XY,
    LIGHT_TRANSLATE_Z,
    LIGHT_SCALE
};
//=============================================================================

//=============================================================================
// CLightTraceResult
//=============================================================================
class CLightTraceResult
{
public:
    uint            uiIndex;
    float           fFraction;
    CVec3f          v3EndPos;
    CWorldLight*    pLight;

    CLightTraceResult() :
    uiIndex(INVALID_INDEX),
    fFraction(FAR_AWAY),
    v3EndPos(0.0f, 0.0f, 0.0f),
    pLight(NULL)
    {}

    void    Clear()
    {
        uiIndex = INVALID_INDEX;
        fFraction = FAR_AWAY;
        v3EndPos.Clear();
        pLight = NULL;
    }
};
//=============================================================================

//=============================================================================
// CLightTool
//=============================================================================
class CLightTool : public ITool
{
private:
    bool            m_bCloning;
    CLightTraceResult       m_Trace;
    uiset           m_setSelection;
    uiset           m_setOldSelection;
    uiset           m_setHoverSelection;
    CVec2f          m_vStartCursorPos;
    CVec2f          m_vOldCursorPos;
    int             m_iState;
    CVec3f          m_vTranslate;
    CVec3f          m_vTrueTranslate;
    float           m_fScale;
    float           m_fTrueScale;
    CVec3f          m_vRotation;
    CVec3f          m_vTrueRotation;
    bool            m_bSnapCursor;
    CVec3f          m_v3EndPos;
    ResHandle       m_hLineMaterial;

    uint            m_uiHoverIndex;



    float           m_fCosTable[NUM_CIRCLE_SEGMENTS + 1];
    float           m_fSinTable[NUM_CIRCLE_SEGMENTS + 1];

    CVec3f          GetLightPosition(uint zIndex);

    bool            CursorLightTrace(CLightTraceResult &result);
    CVec3f          SelectionCenter();
    void            SnapCursor(const CVec3f &vOrigin);
    void            CloneSelection();

    void            Hovering();
    void            TranslateXY();
    void            TranslateZ();
    void            Scale();

    void            Create();

    void            StartSelect();
    void            StartTranslateXY();
    void            StartTransform(int iState);

    void            ApplySelect();
    void            ApplyTransform();

    void            DrawLightRings(uint zIndex);

    void            UpdateHoverSelection();

public:
    CLightTool();
    virtual ~CLightTool()                   {}

    void        CalcToolProperties();
    uint        CreateLight(const CVec3f &vPos = V_ZERO);

    void        PrimaryUp();
    void        PrimaryDown();
    void        SecondaryUp()       {}
    void        SecondaryDown();
    void        TertiaryUp()        {}
    void        TertiaryDown()      {}
    void        QuaternaryUp()      {}
    void        QuaternaryDown()    {}

    void        Cancel();
    void        Delete();

    void        Frame(float fFrameTime);

    bool        IsSelectionActive();
    CRectf      GetSelectionRect();
    int         IsSelected(uint zIndex);
    int         IsHoverSelected(uint zIndex);

    void        Split();

    void        Draw();
    void        Render();

    // Current transformation
    CVec3f      GetCurrentLightPosition(uint uiLight);
    float       GetStartFalloff(uint uiLight);
    float       GetEndFalloff(uint uiLight);

    void        SetSelectionColor(EColorComponent eColor, float fLevel);
    void        SetSelectionStartFalloff(float fStartFalloff);
    void        SetSelectionEndFalloff(float fEndFalloff);

    const uiset&        GetSelectionSet()           { return m_setSelection; }
};
//=============================================================================
#endif //__C_LIGHTTOOL_H__
