// (C)2005 S2 Games
// i_tool.h
//
//=============================================================================
#ifndef __I_TOOL_H__
#define __I_TOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_toolbox.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EToolState
{
    STATE_HOVERING = 0,
    STATE_SELECT,
    STATE_TRANSLATE_XY,
    STATE_TRANSLATE_Z,
    STATE_SCALE,
    STATE_SCALE_UNIFORM,
    STATE_ROTATE_YAW,
    STATE_ROTATE_PITCH,
    STATE_ROTATE_ROLL,
    STATE_CREATE,
};

enum EToolCenterMode
{
    CENTER_AVERAGE,
    CENTER_INDIVIDUAL,
    CENTER_HOVER
};

#define BRUSH_INDEX(x, y)   (recArea.left + (x)) + ((recArea.top + (y)) * iBrushSize)

#define GET_TOOL(name, capname) static_cast<C##name##Tool*>(ToolBox.GetTool(TOOL_##capname, true))
//=============================================================================

//=============================================================================
// ITool
// Parent class for a level editor tool
//=============================================================================
class ITool
{
protected:
    tstring m_sName;
    EToolID m_eID;
    int     m_iX, m_iY;
    bool    m_bModifier1;
    bool    m_bModifier2;
    bool    m_bModifier3;

public:
    ITool(EToolID eID, const tstring &sName) :
    m_sName(sName),
    m_eID(eID),
    m_iX(0),
    m_iY(0),
    m_bModifier1(false),
    m_bModifier2(false),
    m_bModifier3(false)
    {
    }

    virtual ~ITool() {};

    const tstring&  GetName() const { return m_sName; }
    EToolID         GetID() const   { return m_eID; }

    // Each tool has four bindable actions (buttons)
    virtual void PrimaryUp() = 0;
    virtual void PrimaryDown() = 0;
    virtual void SecondaryUp() = 0;
    virtual void SecondaryDown() = 0;
    virtual void TertiaryUp() = 0;
    virtual void TertiaryDown() = 0;
    virtual void QuaternaryUp() = 0;
    virtual void QuaternaryDown() = 0;

    virtual void Cancel() = 0;
    virtual void Delete() = 0;

    virtual void Enter() {};
    virtual void Leave() {};

    virtual void Frame(float fFrameTime) = 0;

    virtual void Render() {}    // tool specific 3d rendering
    virtual void Draw() {}      // tool specific 2d drawing

    void    SetModifier1(bool bValue)   { m_bModifier1 = bValue; }
    void    SetModifier2(bool bValue)   { m_bModifier2 = bValue; }
    void    SetModifier3(bool bValue)   { m_bModifier3 = bValue; }
};
//=============================================================================
#endif //__I_TOOL_H__
