// (C)2005 S2 Games
// c_input.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_input.h"
#include "c_system.h"
#include "c_actionregistry.h"
#include "c_action.h"
#include "c_cmd.h"
#include "c_vid.h"
#include "c_uicmd.h"
#include "i_listwidget.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CInput);
CInput *g_pInput(CInput::GetInstance());

SIEvent g_NullInput = { INPUT_NONE, 0, { 0 } };

#define MAP_BUTTON(string, ebutton) \
    m_mapEButtonToString[ebutton] = _T(string); \
    m_mapStringToEButton[_T(string)] = ebutton;

#ifdef __APPLE__
#define MAP_BUTTON2(string, string2, ebutton) \
    m_mapEButtonToString[ebutton] = _T(string); \
    m_mapStringToEButton[_T(string)] = ebutton; \
    m_mapStringToEButton[_T(string2)] = ebutton;
#endif

#define MAP_AXIS(string, eaxis) \
    m_mapEAxisToString[eaxis] = _T(string); \
    m_mapStringToEAxis[_T(string)] = eaxis;

CVAR_FLOATF(    input_mouseSensitivity,     1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_mouseSensitivityX,    1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_mouseSensitivityY,    1.0f,           CVAR_SAVECONFIG);
CVAR_BOOLF(     input_mouseInvertY,         false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     input_mouseInvertX,         false,          CVAR_SAVECONFIG);

CVAR_INTF(      input_joyDeviceID,          -1,             CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joySensitivityX,      1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joySensitivityY,      1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joySensitivityZ,      1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joySensitivityR,      1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joySensitivityU,      1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joySensitivityV,      1.0f,           CVAR_SAVECONFIG);

CVAR_FLOATF(    input_joyDeadZoneX,         0.1f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyDeadZoneY,         0.1f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyDeadZoneZ,         0.1f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyDeadZoneR,         0.1f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyDeadZoneU,         0.1f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyDeadZoneV,         0.1f,           CVAR_SAVECONFIG);

CVAR_FLOATF(    input_joyGainX,             1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyGainY,             1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyGainZ,             1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyGainR,             1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyGainU,             1.0f,           CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyGainV,             1.0f,           CVAR_SAVECONFIG);

CVAR_BOOLF(     input_joyInvertX,           false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     input_joyInvertY,           false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     input_joyInvertZ,           false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     input_joyInvertR,           false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     input_joyInvertU,           false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     input_joyInvertV,           false,          CVAR_SAVECONFIG);

CVAR_BOOLF(     input_joyControlCursor,     false,          CVAR_SAVECONFIG);
CVAR_FLOATF(    input_joyCursorSpeed,       150.0f,         CVAR_SAVECONFIG);

CVAR_INTF(      input_joyCursorX,           AXIS_JOY_X,     CVAR_SAVECONFIG);
CVAR_INTF(      input_joyCursorY,           AXIS_JOY_Y,     CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CInput::CInput
  ====================*/
CInput::CInput() :
m_iFlags(0),
m_iCursorChange(1),
m_vButtonStates(NUM_BUTTONS),
m_vAxisStates(NUM_AXES)
{
    for (int iButton = 0; iButton < NUM_BUTTONS; ++iButton)
        m_vButtonStates[iButton] = false;

    for (int iAxis = 0; iAxis < NUM_AXES; ++iAxis)
        m_vAxisStates[iAxis] = 0.0f;

    for (int i(0); i < NUM_CURSORS; ++i)
    {
        m_Cursor[i].bConstrained = BOOL_NOT_SET;
        m_Cursor[i].bFrozen = BOOL_NOT_SET;
        m_Cursor[i].bHidden = BOOL_NOT_SET;
        m_Cursor[i].bRecenter = BOOL_NOT_SET;
        m_Cursor[i].hCursor = INVALID_RESOURCE;
    }

    MAP_BUTTON("INVALID", BUTTON_INVALID);
    MAP_BUTTON("???", BUTTON_UNSET);

    MAP_BUTTON("BACKSPACE", BUTTON_BACKSPACE);
    MAP_BUTTON("TAB", BUTTON_TAB);
#ifdef __APPLE__
    MAP_BUTTON2("RETURN", "ENTER", BUTTON_RETURN);
#else
    MAP_BUTTON("ENTER", BUTTON_ENTER);
#endif
    MAP_BUTTON("ESC", BUTTON_ESC);
    MAP_BUTTON("SPACE", BUTTON_SPACE);

    MAP_BUTTON("A", EButton('A'));
    MAP_BUTTON("B", EButton('B'));
    MAP_BUTTON("C", EButton('C'));
    MAP_BUTTON("D", EButton('D'));
    MAP_BUTTON("E", EButton('E'));
    MAP_BUTTON("F", EButton('F'));
    MAP_BUTTON("G", EButton('G'));
    MAP_BUTTON("H", EButton('H'));
    MAP_BUTTON("I", EButton('I'));
    MAP_BUTTON("J", EButton('J'));
    MAP_BUTTON("K", EButton('K'));
    MAP_BUTTON("L", EButton('L'));
    MAP_BUTTON("M", EButton('M'));
    MAP_BUTTON("N", EButton('N'));
    MAP_BUTTON("O", EButton('O'));
    MAP_BUTTON("P", EButton('P'));
    MAP_BUTTON("Q", EButton('Q'));
    MAP_BUTTON("R", EButton('R'));
    MAP_BUTTON("S", EButton('S'));
    MAP_BUTTON("T", EButton('T'));
    MAP_BUTTON("U", EButton('U'));
    MAP_BUTTON("V", EButton('V'));
    MAP_BUTTON("W", EButton('W'));
    MAP_BUTTON("X", EButton('X'));
    MAP_BUTTON("Y", EButton('Y'));
    MAP_BUTTON("Z", EButton('Z'));

    MAP_BUTTON("1", EButton('1'));
    MAP_BUTTON("2", EButton('2'));
    MAP_BUTTON("3", EButton('3'));
    MAP_BUTTON("4", EButton('4'));
    MAP_BUTTON("5", EButton('5'));
    MAP_BUTTON("6", EButton('6'));
    MAP_BUTTON("7", EButton('7'));
    MAP_BUTTON("8", EButton('8'));
    MAP_BUTTON("9", EButton('9'));
    MAP_BUTTON("0", EButton('0'));

    MAP_BUTTON("CAPS_LOCK", BUTTON_CAPS_LOCK);

    MAP_BUTTON("F1", BUTTON_F1);
    MAP_BUTTON("F2", BUTTON_F2);
    MAP_BUTTON("F3", BUTTON_F3);
    MAP_BUTTON("F4", BUTTON_F4);
    MAP_BUTTON("F5", BUTTON_F5);
    MAP_BUTTON("F6", BUTTON_F6);
    MAP_BUTTON("F7", BUTTON_F7);
    MAP_BUTTON("F8", BUTTON_F8);
    MAP_BUTTON("F9", BUTTON_F9);
    MAP_BUTTON("F10", BUTTON_F10);
    MAP_BUTTON("F11", BUTTON_F11);
    MAP_BUTTON("F12", BUTTON_F12);

    MAP_BUTTON("SHIFT", BUTTON_SHIFT);
    MAP_BUTTON("LSHIFT", BUTTON_LSHIFT);
    MAP_BUTTON("RSHIFT", BUTTON_RSHIFT);

    MAP_BUTTON("CTRL", BUTTON_CTRL);
    MAP_BUTTON("LCTRL", BUTTON_LCTRL);
    MAP_BUTTON("RCTRL", BUTTON_RCTRL);

#ifdef __APPLE__
    MAP_BUTTON2("OPT", "ALT", BUTTON_OPT);
    MAP_BUTTON2("LOPT", "LALT", BUTTON_LOPT);
    MAP_BUTTON2("ROPT", "RALT", BUTTON_ROPT);
    
    MAP_BUTTON2("CMD", "WIN", BUTTON_CMD);
    MAP_BUTTON2("LCMD", "LWIN", BUTTON_LCMD);
    MAP_BUTTON2("RCMD", "RWIN", BUTTON_RCMD);
#else
    MAP_BUTTON("ALT", BUTTON_ALT);
    MAP_BUTTON("LALT", BUTTON_LALT);
    MAP_BUTTON("RALT", BUTTON_RALT);

    MAP_BUTTON("WIN", BUTTON_WIN);
    MAP_BUTTON("LWIN", BUTTON_LWIN);
    MAP_BUTTON("RWIN", BUTTON_RWIN);
#endif

    MAP_BUTTON("MENU", BUTTON_MENU);

    MAP_BUTTON("UP", BUTTON_UP);
    MAP_BUTTON("LEFT", BUTTON_LEFT);
    MAP_BUTTON("DOWN", BUTTON_DOWN);
    MAP_BUTTON("RIGHT", BUTTON_RIGHT);

#ifdef __APPLE__
    MAP_BUTTON2("FN", "INS", BUTTON_FN)
#else
    MAP_BUTTON("INS", BUTTON_INS);
#endif
    MAP_BUTTON("DEL", BUTTON_DEL);
    MAP_BUTTON("HOME", BUTTON_HOME);
    MAP_BUTTON("END", BUTTON_END);
    MAP_BUTTON("PGUP", BUTTON_PGUP);
    MAP_BUTTON("PGDN", BUTTON_PGDN);

#ifdef __APPLE__
    MAP_BUTTON2("F13", "PRINTSCREEN", BUTTON_F13);
    MAP_BUTTON2("F14", "SCROLL_LOCK", BUTTON_F14);
    MAP_BUTTON2("F15", "PAUSE", BUTTON_F15);
#else
    MAP_BUTTON("PRINTSCREEN", BUTTON_PRINTSCREEN);
    MAP_BUTTON("SCROLL_LOCK", BUTTON_SCROLL_LOCK);
    MAP_BUTTON("PAUSE", BUTTON_PAUSE);
#endif

#ifdef __APPLE__
    MAP_BUTTON2("CLEAR", "NUM_LOCK", BUTTON_CLEAR);
#else
    MAP_BUTTON("NUM_LOCK", BUTTON_NUM_LOCK);
#endif
    MAP_BUTTON("DIVIDE", BUTTON_DIVIDE);
    MAP_BUTTON("MULTIPLY", BUTTON_MULTIPLY);
    MAP_BUTTON("ADD", BUTTON_ADD);
    MAP_BUTTON("SUBTRACT", BUTTON_SUBTRACT);
    MAP_BUTTON("DECIMAL", BUTTON_DECIMAL);
    MAP_BUTTON("NUM0", BUTTON_NUM0);
    MAP_BUTTON("NUM1", BUTTON_NUM1);
    MAP_BUTTON("NUM2", BUTTON_NUM2);
    MAP_BUTTON("NUM3", BUTTON_NUM3);
    MAP_BUTTON("NUM4", BUTTON_NUM4);
    MAP_BUTTON("NUM5", BUTTON_NUM5);
    MAP_BUTTON("NUM6", BUTTON_NUM6);
    MAP_BUTTON("NUM7", BUTTON_NUM7);
    MAP_BUTTON("NUM8", BUTTON_NUM8);
    MAP_BUTTON("NUM9", BUTTON_NUM9);
    MAP_BUTTON("NUM_ENTER", BUTTON_NUM_ENTER);

    MAP_BUTTON("MOUSEL", BUTTON_MOUSEL);
    MAP_BUTTON("MOUSER", BUTTON_MOUSER);
    MAP_BUTTON("MOUSEM", BUTTON_MOUSEM);
    MAP_BUTTON("WHEELUP", BUTTON_WHEELUP);
    MAP_BUTTON("WHEELDOWN", BUTTON_WHEELDOWN);
    MAP_BUTTON("WHEELLEFT", BUTTON_WHEELLEFT);
    MAP_BUTTON("WHEELRIGHT", BUTTON_WHEELRIGHT);
    MAP_BUTTON("MOUSEX1", BUTTON_MOUSEX1);
    MAP_BUTTON("MOUSEX2", BUTTON_MOUSEX2);

    // FIXME: These will have different names for different keyboard layouts
    MAP_BUTTON("SEMI-COLON", BUTTON_MISC1);
    MAP_BUTTON("SLASH", BUTTON_MISC2);
    MAP_BUTTON("~", BUTTON_MISC3);
    MAP_BUTTON("[", BUTTON_MISC4);
    MAP_BUTTON("BACK_SLASH", BUTTON_MISC5);
    MAP_BUTTON("]", BUTTON_MISC6);
    MAP_BUTTON("QUOTE", BUTTON_MISC7);

    MAP_BUTTON("=", BUTTON_PLUS);
    MAP_BUTTON("-", BUTTON_MINUS);
    MAP_BUTTON("COMMA", BUTTON_COMMA);
    MAP_BUTTON("PERIOD", BUTTON_PERIOD);

    MAP_BUTTON("JOY1", BUTTON_JOY1);
    MAP_BUTTON("JOY2", BUTTON_JOY2);
    MAP_BUTTON("JOY3", BUTTON_JOY3);
    MAP_BUTTON("JOY4", BUTTON_JOY4);
    MAP_BUTTON("JOY5", BUTTON_JOY5);
    MAP_BUTTON("JOY6", BUTTON_JOY6);
    MAP_BUTTON("JOY7", BUTTON_JOY7);
    MAP_BUTTON("JOY8", BUTTON_JOY8);
    MAP_BUTTON("JOY9", BUTTON_JOY9);
    MAP_BUTTON("JOY10", BUTTON_JOY10);
    MAP_BUTTON("JOY11", BUTTON_JOY11);
    MAP_BUTTON("JOY12", BUTTON_JOY12);
    MAP_BUTTON("JOY13", BUTTON_JOY13);
    MAP_BUTTON("JOY14", BUTTON_JOY14);
    MAP_BUTTON("JOY15", BUTTON_JOY15);
    MAP_BUTTON("JOY16", BUTTON_JOY16);
    MAP_BUTTON("JOY17", BUTTON_JOY17);
    MAP_BUTTON("JOY18", BUTTON_JOY18);
    MAP_BUTTON("JOY19", BUTTON_JOY19);
    MAP_BUTTON("JOY20", BUTTON_JOY20);
    MAP_BUTTON("JOY21", BUTTON_JOY21);
    MAP_BUTTON("JOY22", BUTTON_JOY22);
    MAP_BUTTON("JOY23", BUTTON_JOY23);
    MAP_BUTTON("JOY24", BUTTON_JOY24);
    MAP_BUTTON("JOY25", BUTTON_JOY25);
    MAP_BUTTON("JOY26", BUTTON_JOY26);
    MAP_BUTTON("JOY27", BUTTON_JOY27);
    MAP_BUTTON("JOY28", BUTTON_JOY28);
    MAP_BUTTON("JOY29", BUTTON_JOY29);
    MAP_BUTTON("JOY30", BUTTON_JOY30);
    MAP_BUTTON("JOY31", BUTTON_JOY31);
    MAP_BUTTON("JOY32", BUTTON_JOY32);

    MAP_BUTTON("POV_UP", BUTTON_JOY_POV_UP);
    MAP_BUTTON("POV_LEFT", BUTTON_JOY_POV_LEFT);
    MAP_BUTTON("POV_RIGHT", BUTTON_JOY_POV_RIGHT);
    MAP_BUTTON("POV_DOWN", BUTTON_JOY_POV_DOWN);

    MAP_BUTTON("JOY_X_POS", BUTTON_JOY_X_POS);
    MAP_BUTTON("JOY_X_NEG", BUTTON_JOY_X_NEG);
    MAP_BUTTON("JOY_Y_POS", BUTTON_JOY_Y_POS);
    MAP_BUTTON("JOY_Y_NEG", BUTTON_JOY_Y_NEG);
    MAP_BUTTON("JOY_Z_POS", BUTTON_JOY_Z_POS);
    MAP_BUTTON("JOY_Z_NEG", BUTTON_JOY_Z_NEG);
    MAP_BUTTON("JOY_R_POS", BUTTON_JOY_R_POS);
    MAP_BUTTON("JOY_R_NEG", BUTTON_JOY_R_NEG);
    MAP_BUTTON("JOY_U_POS", BUTTON_JOY_U_POS);
    MAP_BUTTON("JOY_U_NEG", BUTTON_JOY_U_NEG);
    MAP_BUTTON("JOY_V_POS", BUTTON_JOY_V_POS);
    MAP_BUTTON("JOY_V_NEG", BUTTON_JOY_V_NEG);
    
#ifdef __APPLE__
    MAP_BUTTON("EQUAL", BUTTON_EQUALS);
    MAP_BUTTON("F16", BUTTON_F16);
    MAP_BUTTON("F17", BUTTON_F17);
    MAP_BUTTON("F18", BUTTON_F18);
    MAP_BUTTON("F19", BUTTON_F19);
#endif

    MAP_AXIS("INVALID", AXIS_INVALID);
    MAP_AXIS("MOUSE_X", AXIS_MOUSE_X);
    MAP_AXIS("MOUSE_Y", AXIS_MOUSE_Y);
    MAP_AXIS("JOY_X", AXIS_JOY_X);
    MAP_AXIS("JOY_Y", AXIS_JOY_Y);
    MAP_AXIS("JOY_Z", AXIS_JOY_Z);
    MAP_AXIS("JOY_R", AXIS_JOY_R);
    MAP_AXIS("JOY_U", AXIS_JOY_U);
    MAP_AXIS("JOY_V", AXIS_JOY_V);

    m_mapJoyDeadZoneCvars[AXIS_JOY_X] = &input_joyDeadZoneX;
    m_mapJoyDeadZoneCvars[AXIS_JOY_Y] = &input_joyDeadZoneY;
    m_mapJoyDeadZoneCvars[AXIS_JOY_Z] = &input_joyDeadZoneZ;
    m_mapJoyDeadZoneCvars[AXIS_JOY_R] = &input_joyDeadZoneR;
    m_mapJoyDeadZoneCvars[AXIS_JOY_U] = &input_joyDeadZoneU;
    m_mapJoyDeadZoneCvars[AXIS_JOY_V] = &input_joyDeadZoneV;

    m_mapJoySensitivityCvars[AXIS_JOY_X] = &input_joySensitivityX;
    m_mapJoySensitivityCvars[AXIS_JOY_Y] = &input_joySensitivityY;
    m_mapJoySensitivityCvars[AXIS_JOY_Z] = &input_joySensitivityZ;
    m_mapJoySensitivityCvars[AXIS_JOY_R] = &input_joySensitivityR;
    m_mapJoySensitivityCvars[AXIS_JOY_U] = &input_joySensitivityU;
    m_mapJoySensitivityCvars[AXIS_JOY_V] = &input_joySensitivityV;

    m_mapJoyGainCvars[AXIS_JOY_X] = &input_joyGainX;
    m_mapJoyGainCvars[AXIS_JOY_Y] = &input_joyGainY;
    m_mapJoyGainCvars[AXIS_JOY_Z] = &input_joyGainZ;
    m_mapJoyGainCvars[AXIS_JOY_R] = &input_joyGainR;
    m_mapJoyGainCvars[AXIS_JOY_U] = &input_joyGainU;
    m_mapJoyGainCvars[AXIS_JOY_V] = &input_joyGainV;

    m_mapJoyInvertCvars[AXIS_JOY_X] = &input_joyInvertX;
    m_mapJoyInvertCvars[AXIS_JOY_Y] = &input_joyInvertY;
    m_mapJoyInvertCvars[AXIS_JOY_Z] = &input_joyInvertZ;
    m_mapJoyInvertCvars[AXIS_JOY_R] = &input_joyInvertR;
    m_mapJoyInvertCvars[AXIS_JOY_U] = &input_joyInvertU;
    m_mapJoyInvertCvars[AXIS_JOY_V] = &input_joyInvertV;
}


/*====================
  CInput::Init
  ====================*/
void    CInput::Init()
{
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, BUTTON_F8, BIND_MOD_NONE, _T("ToggleConsole"), _T(""), BIND_PRIORITY);
#if __APPLE__
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, BUTTON_MISC3, BIND_MOD_CTRL, _T("ToggleConsole"), _T(""), BIND_PRIORITY);
#endif
#ifdef __APPLE__
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, EButton('C'), BIND_MOD_CMD, _T("CopyInputLine"), _T(""));
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, EButton('V'), BIND_MOD_CMD, _T("PasteInputLine"), _T(""));
#else
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, EButton('C'), BIND_MOD_CTRL, _T("CopyInputLine"), _T(""));
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, EButton('V'), BIND_MOD_CTRL, _T("PasteInputLine"), _T(""));
#endif
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, BUTTON_F5, BIND_MOD_NONE, _T("Screenshot"), _T(""), BIND_PRIORITY);
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, BUTTON_F9, BIND_MOD_NONE, _T("Cmd"), _T("SceneStatsStart"), BIND_PRIORITY);
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, BUTTON_F7, BIND_MOD_NONE, _T("Cmd"), _T("NetStatsStart"), BIND_PRIORITY);
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, BUTTON_F11, BIND_MOD_NONE, _T("Cmd"), _T("ProfileStart"), BIND_PRIORITY);
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, BUTTON_F12, BIND_MOD_NONE, _T("Cmd"), _T("Toggle ui_draw"), BIND_PRIORITY);
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, BUTTON_PGUP, BIND_MOD_CTRL, _T("Cmd"), _T("NextClient"), BIND_PRIORITY);
    ActionRegistry.BindImpulse(BINDTABLE_CONSOLE, BUTTON_PGDN, BIND_MOD_CTRL, _T("Cmd"), _T("PrevClient"), BIND_PRIORITY);

#ifdef __APPLE__
    ActionRegistry.BindImpulse(BINDTABLE_UI, EButton('C'), BIND_MOD_CMD, _T("UICopyInputLine"), _T(""));
    ActionRegistry.BindImpulse(BINDTABLE_UI, EButton('V'), BIND_MOD_CMD, _T("UIPasteInputLine"), _T(""));
#else
    ActionRegistry.BindImpulse(BINDTABLE_UI, EButton('C'), BIND_MOD_CTRL, _T("UICopyInputLine"), _T(""));
    ActionRegistry.BindImpulse(BINDTABLE_UI, EButton('V'), BIND_MOD_CTRL, _T("UIPasteInputLine"), _T(""));
#endif
}
    

/*====================
  CInput::Frame
  ====================*/
void    CInput::Frame()
{
    // Show or hide the cursor
    if (IsCursorHidden())
        K2System.HideMouseCursor();
    else
        K2System.ShowMouseCursor();

    // No events if the window is not active
    if (!K2System.HasFocus())
        return;

    K2System.PollMouse();
    if (input_joyDeviceID != -1)
        K2System.PollJoysticks(input_joyDeviceID);

    // Constrain the cursor to the application window
    if (IsCursorFrozen() || IsCursorRecenter() || IsCursorConstrained())
        K2System.SetMouseClipping(GetCursorConstraint());
    else
        K2System.UnsetMouseClipping();

    if (IsCursorRecenter())
    {
        CRecti  recArea(K2System.GetWindowArea());
        m_vAxisStates[AXIS_MOUSE_X] = recArea.GetWidth() / 2.0f;
        m_vAxisStates[AXIS_MOUSE_Y] = recArea.GetHeight() / 2.0f;
        K2System.SetMousePos(INT_ROUND(m_vAxisStates[AXIS_MOUSE_X]), INT_ROUND(m_vAxisStates[AXIS_MOUSE_Y]));
    }
    else if (IsCursorFrozen())
    {
        K2System.SetMousePos(INT_ROUND(m_vAxisStates[AXIS_MOUSE_X]), INT_ROUND(m_vAxisStates[AXIS_MOUSE_Y]));
    }

    if (m_iCursorChange)
    {
        Vid.SetCursor(GetCursor());
        m_iCursorChange = 0;
    }
}


/*====================
  CInput::SetCursorPos
  ====================*/
void    CInput::SetCursorPos(float x, float y)
{
    if (m_vCursorPos.x == x && m_vCursorPos.y == y)
        return;

    AddEvent(CVec2f(x, y));
}


/*====================
  CInput::GetCursorPos
  ====================*/
CVec2f  CInput::GetCursorPos()
{
    return m_vCursorPos;
}


/*====================
  CInput::AddEvent
  ====================*/
void    CInput::AddEvent(const CVec2f &v2Pos)
{
    CVec2f v2Delta = v2Pos - m_vCursorPos;

    if (v2Delta.x == 0 && v2Delta.y == 0)
        return;

    m_vCursorPos = v2Pos;

    SIEvent event;
    event.eType = INPUT_CURSOR;
    event.cAbs.v2Cursor.x = v2Pos.x;
    event.cAbs.v2Cursor.y = v2Pos.y;
    event.cDelta.v2Cursor.x = v2Delta.x;
    event.cDelta.v2Cursor.y = v2Delta.y;
    event.iFlags = 0;
    if (IsCtrlDown())
        event.iFlags |= IEVENT_CTRL;
    if (IsAltDown())
        event.iFlags |= IEVENT_ALT;
    if (IsShiftDown())
        event.iFlags |= IEVENT_SHIFT;
#ifdef __APPLE__
    if (IsCommandDown())
        event.iFlags |= IEVENT_CMD;
#endif
    m_deqStream.push_back(event);
}

void    CInput::AddEvent(EAxis axis, float fValue, float fDelta)
{
    if (axis == AXIS_MOUSE_X || axis == AXIS_MOUSE_Y)
    {
        if (!(IsCursorRecenter() || IsCursorFrozen()))
            m_vAxisStates[axis] = fValue;

        SetCursorPos(axis == AXIS_MOUSE_X ? fValue : m_vCursorPos.x,
                    axis == AXIS_MOUSE_Y ? fValue : m_vCursorPos.y);
    }

    if (axis >= AXIS_JOY_X && axis <= AXIS_JOY_V)
    {
        if (m_mapJoyInvertCvars[axis]->GetBool())
            fDelta *= -1.0f;

#define AXIS_POS(value) ((value) > m_mapJoyDeadZoneCvars[axis]->GetFloat())
#define AXIS_NEG(value) ((value) < -m_mapJoyDeadZoneCvars[axis]->GetFloat())
        if (AXIS_POS(fDelta) && !AXIS_POS(m_vAxisStates[axis]))
            Input.AddEvent(EButton(BUTTON_JOY_X_POS + ((axis - AXIS_JOY_X) * 2)), true);
        else if (!AXIS_POS(fDelta) && AXIS_POS(m_vAxisStates[axis]))
            Input.AddEvent(EButton(BUTTON_JOY_X_POS + ((axis - AXIS_JOY_X) * 2)), false);
        if (AXIS_NEG(fDelta) && !AXIS_NEG(m_vAxisStates[axis]))
            Input.AddEvent(EButton(BUTTON_JOY_X_NEG + ((axis - AXIS_JOY_X) * 2)), true);
        else if (!AXIS_NEG(fDelta) && AXIS_NEG(m_vAxisStates[axis]))
            Input.AddEvent(EButton(BUTTON_JOY_X_NEG + ((axis - AXIS_JOY_X) * 2)), false);
#undef AXIS_POS
#undef AXIS_NEG

        m_vAxisStates[axis] = fDelta;

        if (fabs(fDelta) < m_mapJoyDeadZoneCvars[axis]->GetFloat())
            return;

        if (input_joyControlCursor)
        {
            SetCursorPos(m_vCursorPos.x + ((axis == input_joyCursorX) ? (fDelta * input_joyCursorSpeed * MsToSec(Host.GetFrameLength())) : 0.0f),
                        m_vCursorPos.y + ((axis == input_joyCursorY) ? (fDelta * input_joyCursorSpeed * MsToSec(Host.GetFrameLength())) : 0.0f));
            K2System.SetMousePos(m_vCursorPos.x, m_vCursorPos.y);
        }

        fDelta *= m_mapJoySensitivityCvars[axis]->GetFloat();
        fDelta = (pow(1.0f + fabs(fDelta), m_mapJoyGainCvars[axis]->GetFloat()) - 1.0f) * SIGN(fDelta);
    }

    SIEvent event;
    event.eType = INPUT_AXIS;
    event.uID.axis = axis;
    event.cDelta.fValue = fDelta;
    event.cAbs.fValue = fValue;
    event.iFlags = 0;
    if (IsCtrlDown())
        event.iFlags |= IEVENT_CTRL;
    if (IsAltDown())
        event.iFlags |= IEVENT_ALT;
    if (IsShiftDown())
        event.iFlags |= IEVENT_SHIFT;
#ifdef __APPLE__
    if (IsCommandDown())
        event.iFlags |= IEVENT_CMD;
#endif
    m_deqStream.push_back(event);
}

void    CInput::AddEvent(TCHAR c)
{
    SIEvent event;
    event.eType = INPUT_CHARACTER;
    event.uID.chr = c;
    event.iFlags = 0;

    if (IsCtrlDown())
        event.iFlags |= IEVENT_CTRL;
    if (IsAltDown())
        event.iFlags |= IEVENT_ALT;
    if (IsShiftDown())
        event.iFlags |= IEVENT_SHIFT;
#ifdef __APPLE__
    if (IsCommandDown())
        event.iFlags |= IEVENT_CMD;
#endif
    m_deqStream.push_back(event);
}

void    CInput::AddEvent(EButton eButton, bool bDown)
{
    SIEvent event;
    event.eType = INPUT_BUTTON;
    event.uID.btn = eButton;
    event.cDelta.fValue = (m_vButtonStates[eButton] == bDown) ? 0.0f : (bDown ? 1.0f : -1.0f);
    event.cAbs.fValue = bDown ? 1.0f : 0.0f;
    event.iFlags = 0;

    if (IsCtrlDown())
        event.iFlags |= IEVENT_CTRL;
    if (IsAltDown())
        event.iFlags |= IEVENT_ALT;
    if (IsShiftDown())
        event.iFlags |= IEVENT_SHIFT;
#ifdef __APPLE__
    if (IsCommandDown())
        event.iFlags |= IEVENT_CMD;
#endif

    m_deqStream.push_back(event);
    m_vButtonStates[eButton] = bDown;
}


void    CInput::AddEvent(EButton eButton, bool bDown, const CVec2f &v2Cursor)
{
    SIEvent event;
    event.eType = INPUT_BUTTON;
    event.uID.btn = eButton;
    event.cAbs.v2Cursor = v2Cursor;
    event.cDelta.fValue = (m_vButtonStates[eButton] == bDown) ? 0.0f : (bDown ? 1.0f : -1.0f);
    event.cAbs.fValue = bDown ? 1.0f : 0.0f;
    event.iFlags = 0;

    if (IsCtrlDown())
        event.iFlags |= IEVENT_CTRL;
    if (IsAltDown())
        event.iFlags |= IEVENT_ALT;
    if (IsShiftDown())
        event.iFlags |= IEVENT_SHIFT;
#ifdef __APPLE__
    if (IsCommandDown())
        event.iFlags |= IEVENT_CMD;
#endif

    m_deqStream.push_back(event);
    m_vButtonStates[eButton] = bDown;
}


/*====================
  CInput::SetButton

  For use with System_SetupKeystates
  ====================*/
void    CInput::SetButton(EButton eButton, bool bDown)
{
    if (m_vButtonStates[eButton] != bDown)
    {
        m_vButtonStates[eButton] = bDown;

        AddEvent(eButton, bDown, m_vCursorPos);
    }
}


/*====================
  CInput::Pop
  ====================*/
SIEvent CInput::Pop()
{
    if (m_deqStream.empty())
        return g_NullInput;

    SIEvent ret(m_deqStream.front());
    m_deqStream.pop_front();
    return ret;
}


/*====================
  CInput::Peek

  Note that this returns a reference, so if you do a pop
  after a peek, the peek reference becomes invalid
  ====================*/
const SIEvent&  CInput::Peek()
{
    if (m_deqStream.empty())
        return g_NullInput;
    else
        return m_deqStream.front();
}


/*====================
  CInput::Push
  ====================*/
void    CInput::Push(const SIEvent &ev)
{
    m_deqStream.push_back(ev);
}


/*====================
  CInput::ToString
  ====================*/
const tstring&  CInput::ToString(EButton button)
{
    EButtonToString::iterator it = m_mapEButtonToString.find(button);

    if (it == m_mapEButtonToString.end())
        return m_mapEButtonToString[BUTTON_INVALID];
    else
        return it->second;
}

const tstring& CInput::ToString(EAxis axis)
{
    EAxisToString::iterator it = m_mapEAxisToString.find(axis);

    if (it == m_mapEAxisToString.end())
        return m_mapEAxisToString[AXIS_INVALID];
    else
        return it->second;
}


/*====================
  CInput::MakeEButton
  ====================*/
EButton CInput::MakeEButton(const tstring &sButton)
{
    StringToEButton::iterator it = m_mapStringToEButton.find(sButton);

    if (it == m_mapStringToEButton.end())
        return m_mapStringToEButton[_T("INVALID")];
    else
        return it->second;
}


/*====================
  CInput::MakeEAxis
  ====================*/
EAxis   CInput::MakeEAxis(const tstring &sAxis)
{
    StringToEAxis::iterator it = m_mapStringToEAxis.find(sAxis);

    if (it == m_mapStringToEAxis.end())
        return m_mapStringToEAxis[_T("INVALID")];
    else
        return it->second;
}


/*====================
  CInput::GetButtonFromString
  ====================*/
EButton CInput::GetButtonFromString(const tstring &sBindString)
{
    size_t zOffset(sBindString.find_last_of(_T('+')));
    if (zOffset == tstring::npos)
        return MakeEButton(sBindString);

    return MakeEButton(sBindString.substr(zOffset + 1));
}


/*====================
  CInput::GetAxisFromString
  ====================*/
EAxis   CInput::GetAxisFromString(const tstring &sBindString)
{
    size_t zOffset(sBindString.find_last_of(_T('+')));
    if (zOffset == tstring::npos)
        return MakeEAxis(sBindString);

    return MakeEAxis(sBindString.substr(zOffset + 1));
}


/*====================
  CInput::GetBindModifierFromString
  ====================*/
int     CInput::GetBindModifierFromString(const tstring &sBindString)
{
    int iModifier(0);

    size_t zPrevOffset(0);
    size_t zOffset(0);
    for(;;)
    {
        zOffset = sBindString.find(_T('+'), zPrevOffset);
        if (zOffset == tstring::npos)
            break;

        tstring sModifier(LowerString(sBindString.substr(zPrevOffset, zOffset - zPrevOffset)));
        if (sModifier == _T("ctrl"))
            iModifier |= BIND_MOD_CTRL;
        if (sModifier == _T("alt"))
            iModifier |= BIND_MOD_ALT;
        if (sModifier == _T("shift"))
            iModifier |= BIND_MOD_SHIFT;
#ifdef __APPLE__
        if (sModifier == _T("cmd"))
            iModifier |= BIND_MOD_CMD;
        if (sModifier == _T("opt"))
            iModifier |= BIND_MOD_OPT;
#endif

        zPrevOffset = zOffset + 1;
    }

    return iModifier;
}


/*====================
  CInput::GetBindModifierFromFlags
  ====================*/
int     CInput::GetBindModifierFromFlags(int iFlags)
{
    int iModifier(0);
    if (iFlags & IEVENT_ALT)
        iModifier |= BIND_MOD_ALT;
    if (iFlags & IEVENT_CTRL)
        iModifier |= BIND_MOD_CTRL;
    if (iFlags & IEVENT_SHIFT)
        iModifier |= BIND_MOD_SHIFT;
#ifdef __APPLE__
    if (iFlags & IEVENT_CMD)
        iModifier |= BIND_MOD_CMD;
#endif

    return iModifier;
}


/*====================
  CInput::GetBindString
  ====================*/
tstring CInput::GetBindString(EButton eButton, int iModifier)
{
    tstring sBind(ToString(eButton));
    if (iModifier & BIND_MOD_SHIFT)
        sBind = _T("SHIFT+") + sBind;
#ifdef __APPLE__
    if (iModifier & BIND_MOD_OPT)
        sBind = _T("OPT+") + sBind; 
    if (iModifier & BIND_MOD_CMD)
        sBind = _T("CMD+") + sBind;
#else
    if (iModifier & BIND_MOD_ALT)
        sBind = _T("ALT+") + sBind;
#endif
    if (iModifier & BIND_MOD_CTRL)
        sBind = _T("CTRL+") + sBind;
    
    return sBind;
}

tstring CInput::GetBindString(EAxis eAxis, int iModifier)
{
    tstring sBind(ToString(eAxis));
    if (iModifier & BIND_MOD_SHIFT)
        sBind = _T("SHIFT+") + sBind;
#ifdef __APPLE__
    if (iModifier & BIND_MOD_OPT)
        sBind = _T("OPT+") + sBind; 
    if (iModifier & BIND_MOD_CMD)
        sBind = _T("CMD+") + sBind;
#else
    if (iModifier & BIND_MOD_ALT)
        sBind = _T("ALT+") + sBind;
#endif
    if (iModifier & BIND_MOD_CTRL)
        sBind = _T("CTRL+") + sBind;
    
    return sBind;
}


/*====================
  CInput::IsButtonDown
  ====================*/
bool    CInput::IsButtonDown(EButton button)
{
    return m_vButtonStates[button];
}


/*====================
  CInput::IsCtrlDown
  ====================*/
bool    CInput::IsCtrlDown()
{
    return m_vButtonStates[BUTTON_CTRL] ||
        m_vButtonStates[BUTTON_RCTRL] ||
        m_vButtonStates[BUTTON_LCTRL];
}


/*====================
  CInput::IsAltDown
  ====================*/
bool    CInput::IsAltDown()
{
    return m_vButtonStates[BUTTON_ALT] ||
        m_vButtonStates[BUTTON_RALT] ||
        m_vButtonStates[BUTTON_LALT];
}


/*====================
  CInput::IsShiftDown
  ====================*/
bool    CInput::IsShiftDown()
{
    return m_vButtonStates[BUTTON_SHIFT] ||
        m_vButtonStates[BUTTON_RSHIFT] ||
        m_vButtonStates[BUTTON_LSHIFT];
}

#ifdef __APPLE__
/*====================
 CInput::IsCommandDown
 ====================*/
bool    CInput::IsCommandDown()
{
    return m_vButtonStates[BUTTON_CMD] ||
    m_vButtonStates[BUTTON_RCMD] ||
    m_vButtonStates[BUTTON_LCMD];
}
#endif


/*====================
  CInput::GetAxisState
  ====================*/
float   CInput::GetAxisState(EAxis axis)
{
    return m_vAxisStates[axis];
}


/*====================
  CInput::FlushByTable
  ====================*/
void    CInput::FlushByTable(EBindTable eTable, int iFlags)
{
    // Step through the pending input and execute any appropriate binds
    deque<SIEvent>::iterator itInput(m_deqStream.begin());
    while (itInput != m_deqStream.end())
    {
        CBind *pBind(ActionRegistry.GetBind(eTable, itInput->uID.btn, GetBindModifierFromFlags(itInput->iFlags)));

        if (pBind != nullptr && pBind->HasAllFlags(iFlags))
            itInput = m_deqStream.erase(itInput);
        else
            ++itInput;
    }
}


/*====================
  CInput::ExecuteBinds
  ====================*/
void    CInput::ExecuteBinds(EBindTable eTable, int iFlags)
{
    // Step through the pending input and execute any appropriate binds
    deque<SIEvent>::iterator itInput = m_deqStream.begin();
    while (itInput != m_deqStream.end())
    {
        int     iModifier(GetBindModifierFromFlags(itInput->iFlags));
        CBind*  pBind(nullptr);
        bool    bUsed(false);

        switch (itInput->eType)
        {
        case INPUT_BUTTON:
            pBind = ActionRegistry.GetBind(eTable, itInput->uID.btn, iModifier);
            if (pBind == nullptr)
                pBind = ActionRegistry.GetBind(eTable, itInput->uID.btn, 0);

            if (pBind != nullptr && pBind->HasAllFlags(iFlags))
            {
                if ((pBind->GetFlags() & BIND_NOREPEAT) && (itInput->cDelta.fValue == 0.0f))
                    break;

                pBind->DoAction(itInput->cAbs.fValue, itInput->cDelta.fValue, itInput->cAbs.v2Cursor);
                bUsed = true;
            }
            break;

        case INPUT_AXIS:
            pBind = ActionRegistry.GetBind(eTable, itInput->uID.axis, iModifier);
            if (pBind == nullptr)
                pBind = ActionRegistry.GetBind(eTable, itInput->uID.axis, 0);

            if (pBind && pBind->HasAllFlags(iFlags))
            {
                // Sensitivity settings
                float fDelta(itInput->cDelta.fValue);
                switch (itInput->uID.axis)
                {
                case AXIS_MOUSE_X:
                    fDelta *= input_mouseSensitivityX * input_mouseSensitivity;
                    if (input_mouseInvertX)
                        fDelta *= -1.0f;
                    break;
                case AXIS_MOUSE_Y:
                    fDelta *= input_mouseSensitivityY * input_mouseSensitivity;
                    if (input_mouseInvertY)
                        fDelta *= -1.0f;
                    break;
                case AXIS_JOY_X:
                    fDelta *= input_joySensitivityX;
                    break;
                case AXIS_JOY_Y:
                    fDelta *= input_joySensitivityY;
                    break;
                case AXIS_JOY_Z:
                    fDelta *= input_joySensitivityZ;
                    break;
                case AXIS_JOY_R:
                    fDelta *= input_joySensitivityR;
                    break;
                case AXIS_JOY_U:
                    fDelta *= input_joySensitivityU;
                    break;
                case AXIS_JOY_V:
                    fDelta *= input_joySensitivityV;
                    break;
                case AXIS_INVALID:
                case NUM_AXES:
                    K2_UNREACHABLE();
                    break;
                }

                pBind->DoAction(itInput->cAbs.fValue, fDelta, itInput->cAbs.v2Cursor);
                bUsed = true;
            }
            break;
        case INPUT_NONE:
        case INPUT_CHARACTER:
        case INPUT_CURSOR:
            break;
        case NUM_INPUT_TYPES:
            K2_UNREACHABLE();
            break;
        }

        if (bUsed)
            itInput = m_deqStream.erase(itInput);
        else
            ++itInput;
    }
}


/*====================
  CInput::SetCursorHidden
  ====================*/
void    CInput::SetCursorHidden(ECursor eCursor, ECursorBool eValue)
{
    m_Cursor[eCursor].bHidden = eValue;
}


/*====================
  CInput::SetCursorFrozen
  ====================*/
void    CInput::SetCursorFrozen(ECursor eCursor, ECursorBool eValue)
{
    m_Cursor[eCursor].bFrozen = eValue;
}


/*====================
  CInput::SetCursorRecenter
  ====================*/
void    CInput::SetCursorRecenter(ECursor eCursor, ECursorBool eValue)
{
    m_Cursor[eCursor].bRecenter = eValue;
}


/*====================
  CInput::SetCursorConstrained
  ====================*/
void    CInput::SetCursorConstrained(ECursor eCursor, ECursorBool eValue)
{
    m_Cursor[eCursor].bConstrained = eValue;
}


/*====================
  CInput::SetCursorConstraint
  ====================*/
void    CInput::SetCursorConstraint(ECursor eCursor, CRectf recArea)
{
    m_Cursor[eCursor].recConstraints = recArea;
}


/*====================
  CInput::SetCursor
  ====================*/
void    CInput::SetCursor(ECursor eCursor, ResHandle hCursor)
{
    ResHandle hOldCursor(GetCursor());

    m_Cursor[eCursor].hCursor = hCursor;

    if (hCursor != hOldCursor)
        m_iCursorChange = 1;
}


/*====================
  CInput::GetCursorHidden
  ====================*/
ECursorBool CInput::GetCursorHidden(ECursor eCursor)
{
    return m_Cursor[eCursor].bHidden;
}


/*====================
  CInput::GetCursorFrozen
  ====================*/
ECursorBool CInput::GetCursorFrozen(ECursor eCursor)
{
    return m_Cursor[eCursor].bFrozen;
}


/*====================
  CInput::GetCursorRecenter
  ====================*/
ECursorBool CInput::GetCursorRecenter(ECursor eCursor)
{
    return m_Cursor[eCursor].bRecenter;
}


/*====================
  CInput::GetCursorConstrained
  ====================*/
ECursorBool CInput::GetCursorConstrained(ECursor eCursor)
{
    return m_Cursor[eCursor].bConstrained;
}


/*====================
  CInput::IsCursorHidden
  ====================*/
bool    CInput::IsCursorHidden()
{
    if (!K2System.HasFocus())
        return false;

    for (int i = 0; i < NUM_CURSORS; ++i)
    {
        if (m_Cursor[i].bHidden != BOOL_NOT_SET)
            return m_Cursor[i].bHidden == BOOL_TRUE;
    }

    return false;
}


/*====================
  CInput::IsCursorFrozen
  ====================*/
bool    CInput::IsCursorFrozen()
{
    if (!K2System.HasFocus())
        return false;

    for (int i = 0; i < NUM_CURSORS; ++i)
    {
        if (m_Cursor[i].bFrozen != BOOL_NOT_SET)
            return m_Cursor[i].bFrozen == BOOL_TRUE;
    }

    return false;
}


/*====================
  CInput::IsCursorRecenter
  ====================*/
bool    CInput::IsCursorRecenter()
{
    if (!K2System.HasFocus())
        return false;

    for (int i = 0; i < NUM_CURSORS; ++i)
    {
        if (m_Cursor[i].bRecenter != BOOL_NOT_SET)
            return m_Cursor[i].bRecenter == BOOL_TRUE;
    }

    return false;
}


/*====================
  CInput::IsCursorConstrained
  ====================*/
bool    CInput::IsCursorConstrained()
{
    for (int i = 0; i < NUM_CURSORS; ++i)
    {
        if (m_Cursor[i].bConstrained != BOOL_NOT_SET)
            return m_Cursor[i].bConstrained == BOOL_TRUE;
    }

    return false;
}


/*====================
  CInput::GetCursorConstraint
  ====================*/
CRectf  CInput::GetCursorConstraint()
{
    for (int i = 0; i < NUM_CURSORS; ++i)
    {
        if (m_Cursor[i].recConstraints.GetArea() > 0)
            return m_Cursor[i].recConstraints;
    }

    return CRecti(0, 0, Vid.GetScreenW(), Vid.GetScreenH());
}


/*====================
  CInput::GetCursor
  ====================*/
ResHandle   CInput::GetCursor()
{
    for (int i = 0; i < NUM_CURSORS; ++i)
    {
        if (m_Cursor[i].hCursor != INVALID_RESOURCE)
            return m_Cursor[i].hCursor;
    }

    return INVALID_RESOURCE;
}


/*--------------------
  ButtonList
  --------------------*/
CMD(ButtonList)
{
    int iNumFound(0);

    const EButtonToString &lButtons = CInput::GetInstance()->GetEButtonToStringMap();

    // Print actions
    for (EButtonToString::const_iterator it(lButtons.begin()); it != lButtons.end(); ++it)
    {
        if ((vArgList.size() == 0 || it->second.find(vArgList[0]) != tstring::npos) && it->first != BUTTON_INVALID)
        {
            Console << it->second << newl;
            ++iNumFound;
        }
    }

    Console << newl << iNumFound << _T(" matching buttons found") << newl;

    return true;
}


/*--------------------
  AxisList
  --------------------*/
CMD(AxisList)
{
    int iNumFound(0);

    const EAxisToString &lAxes = CInput::GetInstance()->GetEAxisToStringMap();

    // Print actions
    for (EAxisToString::const_iterator it(lAxes.begin()); it != lAxes.end(); ++it)
    {
        if ((vArgList.size() == 0 || it->second.find(vArgList[0]) != tstring::npos) && it->first != AXIS_INVALID)
        {
            Console << it->second << newl;
            ++iNumFound;
        }
    }

    Console << newl << iNumFound << _T(" matching buttons found") << newl;

    return true;
}


/*--------------------
  AddGameControllers
  --------------------*/
UI_VOID_CMD(AddGameControllers, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    imaps mapControllers;
    mapControllers[-1] = _T("Disabled");
    K2System.GetJoystickList(mapControllers);

    tstring sName(vArgList[0]->Evaluate());
    CXMLNode::PropertyMap mapParams;
    for (imaps_it it(mapControllers.begin()); it != mapControllers.end(); ++it)
    {
        mapParams[_T("label")] = it->second;

        pList->CreateNewListItemFromTemplate(sName, XtoA(it->first), mapParams);
    }
}


/*--------------------
  AddAxes
  --------------------*/
UI_VOID_CMD(AddAxes, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    tstring sName(vArgList[0]->Evaluate());
    CXMLNode::PropertyMap mapParams;
    for (int i(AXIS_JOY_X); i <= AXIS_JOY_V; ++i)
    {
        switch (i)
        {
        case AXIS_JOY_X: mapParams[_T("label")] = _T("X"); break;
        case AXIS_JOY_Y: mapParams[_T("label")] = _T("Y"); break;
        case AXIS_JOY_Z: mapParams[_T("label")] = _T("Z"); break;
        case AXIS_JOY_R: mapParams[_T("label")] = _T("R"); break;
        case AXIS_JOY_U: mapParams[_T("label")] = _T("U"); break;
        case AXIS_JOY_V: mapParams[_T("label")] = _T("V"); break;
        }

        pList->CreateNewListItemFromTemplate(sName, XtoA(i), mapParams);
    }
}


/*--------------------
  GetAxisName
  --------------------*/
UI_CMD(GetAxisName, 1)
{
    return Input.ToString(EAxis(AtoI(vArgList[0]->Evaluate())));
}


/*--------------------
  GetAxisValue
  --------------------*/
UI_CMD(GetAxisValue, 1)
{
    return XtoA(Input.MakeEAxis(vArgList[0]->Evaluate()));
}


/*--------------------
  GetCursorPosX
  --------------------*/
UI_CMD(GetCursorPosX, 0)
{
    return XtoA(Input.GetCursorPos().x);
}


/*--------------------
  GetCursorPosY
  --------------------*/
UI_CMD(GetCursorPosY, 0)
{
    return XtoA(Input.GetCursorPos().y);
}
