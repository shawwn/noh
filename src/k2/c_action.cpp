// (C)2005 S2 Games
// c_action.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_action.h"
#include "c_actionregistry.h"
#include "c_cmd.h"
#include "c_system.h"
#include "c_xmldoc.h"
#include "c_uicmd.h"
//=============================================================================

/*====================
  EBindTableFromString
  ====================*/
EBindTable  EBindTableFromString(const tstring &sBindTable)
{
    if (CompareNoCase(sBindTable, _T("console")) == 0)
        return BINDTABLE_CONSOLE;
    else if (CompareNoCase(sBindTable, _T("ui")) == 0)
        return BINDTABLE_UI;
    else if (CompareNoCase(sBindTable, _T("game")) == 0)
        return BINDTABLE_GAME;
    else if (CompareNoCase(sBindTable, _T("player")) == 0)
        return BINDTABLE_GAME_PLAYER;
    else if (CompareNoCase(sBindTable, _T("commander")) == 0)
        return BINDTABLE_GAME_COMMANDER;
    else if (CompareNoCase(sBindTable, _T("voicecommand")) == 0)
        return BINDTABLE_GAME_VOICECOMMAND;
    else if (CompareNoCase(sBindTable, _T("voicecommand_sub")) == 0)
        return BINDTABLE_GAME_VOICECOMMAND_SUB;
    else if (CompareNoCase(sBindTable, _T("shop")) == 0)
        return BINDTABLE_GAME_SHOP;
    else
        return EBindTable(-1);
}


/*====================
  EBindTableToString
  ====================*/
tstring EBindTableToString(EBindTable eBindTable)
{
    switch (eBindTable)
    {
    case BINDTABLE_CONSOLE:
        return _T("console");
    case BINDTABLE_UI:
        return _T("ui");
    case BINDTABLE_GAME:
        return _T("game");
    case BINDTABLE_GAME_PLAYER:
        return _T("player");
    case BINDTABLE_GAME_COMMANDER:
        return _T("commander");
    case BINDTABLE_GAME_VOICECOMMAND:
        return _T("voicecommand");
    case BINDTABLE_GAME_VOICECOMMAND_SUB:
        return _T("voicecommand_sub");
    case BINDTABLE_GAME_SHOP:
        return _T("shop");
    default:
        return _T("");
    }
}


/*====================
  CAction::CAction
  ====================*/
CAction::CAction(const tstring &sName, EActionType eType, ActionFn_t pfnAction, int iFlags) :
IBaseInput(sName, eType, iFlags),
m_pfnAction(pfnAction)
{
    if (m_pfnAction == NULL)
        K2System.Error(_T("Tried to register an action with a NULL function."));
}


/*====================
  CAction::Do
  ====================*/
void    CAction::Do(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)
{
    switch (m_eType)
    {
    case AT_BUTTON:
        if (!(m_iFlags & ACTION_NOREPEAT && fDelta == 0.0f))
            m_pfnAction(fValue ? 1.0f : 0.0f, fDelta, v2Cursor, sParam);
        break;
    case AT_AXIS:
        m_pfnAction(fValue, fDelta, v2Cursor, sParam);
        break;
    case AT_IMPULSE:
        if (fValue && !(m_iFlags & ACTION_NOREPEAT && fDelta == 0.0f))
            m_pfnAction(1.0f, fDelta, v2Cursor, sParam);
        break;
    case AT_INVALID:
        K2_UNREACHABLE();
        break;
    }
}


/*====================
  CAction::operator()
  ====================*/
void    CAction::operator()(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)
{
    Do(fValue, fDelta, v2Cursor, sParam);
}


/*====================
  CAction::WriteConfigFile
  ====================*/
bool    CAction::WriteConfigFile(CFileHandle &hFile, const tsvector &wildcards, int iFlags)
{
    for (int n(0); n < NUM_BINDTABLES; ++n)
    {
        const ButtonActionMap &lButtons(ActionRegistry.GetButtonActionMap(EBindTable(n)));

        // Save buttons
        for (ButtonActionMap::const_iterator itButton(lButtons.begin()); itButton != lButtons.end(); ++itButton)
        {
            for (BindModMap::const_iterator itBind(itButton->second.begin()); itBind != itButton->second.end(); ++itBind)
            {
                if (itBind->second.GetActionType() != AT_BUTTON ||
                    !itBind->second.HasAllFlags(iFlags))
                    continue;

                hFile << _T("BindButton ")
                    << EBindTableToString(EBindTable(n)) << SPACE
                    << Input.GetBindString(itButton->first, itBind->first) << SPACE
                    << itBind->second.GetActionName() << SPACE
                    << QuoteStr(AddEscapeChars(itBind->second.GetParam())) << newl;
            }
        }

        const AxisActionMap &lAxis(ActionRegistry.GetAxisActionMap(EBindTable(n)));

        // Save axes
        for (AxisActionMap::const_iterator itAxis(lAxis.begin()); itAxis != lAxis.end(); ++itAxis)
        {
            for (BindModMap::const_iterator itBind(itAxis->second.begin()); itBind != itAxis->second.end(); ++itBind)
            {
                if (itBind->second.HasAllFlags(iFlags))
                    hFile << _T("BindAxis ")
                    << EBindTableToString(EBindTable(n)) << SPACE
                    << Input.GetBindString(itAxis->first, itBind->first) << SPACE
                    << itBind->second.GetActionName() << SPACE
                    << QuoteStr(AddEscapeChars(itBind->second.GetParam())) << newl;
            }
        }

        // Save impulses
        for (ButtonActionMap::const_iterator itButton(lButtons.begin()); itButton != lButtons.end(); ++itButton)
        {
            for (BindModMap::const_iterator itBind(itButton->second.begin()); itBind != itButton->second.end(); ++itBind)
            {
                if (itBind->second.GetActionType() != AT_IMPULSE ||
                    !itBind->second.HasAllFlags(iFlags))
                    continue;

                if (itBind->second.HasFlags(BIND_NOREPEAT|BIND_PRIORITY))
                {
                    hFile << _T("BindImpulseEx ")
                        << EBindTableToString(EBindTable(n)) << SPACE
                        << Input.GetBindString(itButton->first, itBind->first) << SPACE
                        << itBind->second.GetActionName() << SPACE
                        << (itBind->second.HasFlags(BIND_NOREPEAT) ? _T("n") : _T(""))
                        << (itBind->second.HasFlags(BIND_PRIORITY) ? _T("p") : _T(""))
                        << SPACE
                        << QuoteStr(AddEscapeChars(itBind->second.GetParam())) << newl;
                }
                else
                {
                    hFile << _T("BindImpulse ")
                        << EBindTableToString(EBindTable(n)) << SPACE
                        << Input.GetBindString(itButton->first, itBind->first) << SPACE
                        << itBind->second.GetActionName() << SPACE
                        << QuoteStr(AddEscapeChars(itBind->second.GetParam())) << newl;
                }
            }
        }
    }

    return true;
}


/*--------------------
  BindButton
  --------------------*/
CMD(BindButton)
{
    if (vArgList.size() < 3)
    {
        Console << _T("syntax: BindButton <console|ui|game|player|commander> <button> <action> [param]") << newl;
        return false;
    }

    EBindTable eBindTable(EBindTableFromString(vArgList[0]));
    if (eBindTable == EBindTable(-1))
    {
        Console << SingleQuoteStr(vArgList[0]) << "is not a valid bind table" << newl;
        return false;
    }

    EButton eButton(Input.GetButtonFromString(vArgList[1]));
    int iModifier(Input.GetBindModifierFromString(vArgList[1]));
/*  KYLE: Commented out to allow BUTTON_INVALID specify a cleared bind
    if (eButton)
    {*/
        ActionRegistry.BindButton(eBindTable, eButton, iModifier, vArgList[2], vArgList.size() > 3 ? vArgList[3] : _T(""), BIND_SAVECONFIG);
        return true;
//  }

/*  Console << SingleQuoteStr(vArgList[1]) << "is not a valid button name" << newl;
    return false;*/
}


/*--------------------
  BindAxis
  --------------------*/
CMD(BindAxis)
{
    if (vArgList.size() < 3)
    {
        Console << _T("syntax: BindAxis <console|ui|game|player|commander> <axis> <action> [param]") << newl;
        return false;
    }

    EBindTable eBindTable(EBindTableFromString(vArgList[0]));
    if (eBindTable == EBindTable(-1))
    {
        Console << SingleQuoteStr(vArgList[0]) << "is not a valid bind table" << newl;
        return false;
    }

    EAxis eAxis(Input.MakeEAxis(vArgList[1]));
    int iModifier(Input.GetBindModifierFromString(vArgList[1]));
    if (eAxis == AXIS_INVALID)
    {
        Console << SingleQuoteStr(vArgList[1]) << "is not a valid axis name" << newl;
        return false;
    }

    ActionRegistry.BindAxis(eBindTable, eAxis, iModifier, vArgList[2], vArgList.size() > 3 ? vArgList[3] : _T(""), BIND_SAVECONFIG);
    return true;
}


/*--------------------
  BindImpulse
  --------------------*/
CMD(BindImpulse)
{
    if (vArgList.size() < 3)
    {
        Console << _T("syntax: BindImpulse <console|ui|game|player|commander> <button> <action> [param]") << newl;
        return false;
    }

    EBindTable eBindTable(EBindTableFromString(vArgList[0]));
    if (eBindTable == EBindTable(-1))
    {
        Console << SingleQuoteStr(vArgList[0]) << _T(" is not a valid bind table") << newl;
        return false;
    }

    EButton eButton(Input.GetButtonFromString(vArgList[1]));
    int iModifier(Input.GetBindModifierFromString(vArgList[1]));
/*  KYLE: Commented out to allow BUTTON_INVALID specify a cleared bind
    if (eButton == BUTTON_INVALID)
    {
        Console << SingleQuoteStr(vArgList[1]) << _T(" is not a valid button name") << newl;
        return false;
    }*/

    ActionRegistry.BindImpulse(eBindTable, eButton, iModifier, vArgList[2], vArgList.size() > 3 ? vArgList[3] : _T(""), BIND_SAVECONFIG);
    return true;
}


/*--------------------
  BindImpulseEx
  --------------------*/
CMD(BindImpulseEx)
{
    if (vArgList.size() < 4)
    {
        Console << _T("syntax: BindImpulseEx <console|ui|game|player|commander> <button> <action> <flags> [param]") << newl
                << _T("  Flags:") << newl
                << _T("  n - No repeat") << newl
                << _T("  d - No save") << newl
                << _T("  p - Priority") << newl;
        return false;
    }

    EBindTable eBindTable(EBindTableFromString(vArgList[0]));
    if (eBindTable == EBindTable(-1))
    {
        Console << SingleQuoteStr(vArgList[0]) << _T(" is not a valid bind table") << newl;
        return false;
    }

    int iFlags(0);
    if (vArgList[3].find(_T('n')) != tstring::npos)
        iFlags |= BIND_NOREPEAT;
    if (vArgList[3].find(_T('d')) == tstring::npos)
        iFlags |= BIND_SAVECONFIG;
    if (vArgList[3].find(_T('p')) != tstring::npos)
        iFlags |= BIND_PRIORITY;

    EButton eButton(Input.GetButtonFromString(vArgList[1]));
    int iModifier(Input.GetBindModifierFromString(vArgList[1]));
    if (eButton == BUTTON_INVALID)
    {
        Console << SingleQuoteStr(vArgList[1]) << _T(" is not a valid button name") << newl;
        return false;
    }

    ActionRegistry.BindImpulse(eBindTable, eButton, iModifier, vArgList[2], vArgList.size() > 4 ? vArgList[4] : _T(""), iFlags);
    return true;
}


/*--------------------
  Bind
  --------------------*/
CMD(Bind)
{
    if (vArgList.size() < 2)
    {
        Console << _T("syntax: Bind <button> <cmd>") << newl;
        return false;
    }

    tstring sCmd(ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));

    EButton eButton(Input.GetButtonFromString(vArgList[0]));
    int iModifier(Input.GetBindModifierFromString(vArgList[0]));
    if (eButton == BUTTON_INVALID)
    {
        Console << SingleQuoteStr(vArgList[0]) << _T(" is not a valid button name") << newl;
        return false;
    }

    CActionRegistry::GetInstance()->BindImpulse(BINDTABLE_GAME, eButton, iModifier, _T("Cmd"), sCmd, BIND_SAVECONFIG);
    return true;
}


/*--------------------
  Unbind
  --------------------*/
CMD(Unbind)
{
    if (vArgList.size() < 2)
    {
        Console << _T("syntax: Unbind <console|ui|game|player|commander> <button/axis name>") << newl;
        return false;
    }

    EBindTable eBindTable(EBindTableFromString(vArgList[0]));
    if (eBindTable == EBindTable(-1))
    {
        Console << SingleQuoteStr(vArgList[0]) << _T(" is not a valid bind table") << newl;
        return false;
    }

    int iModifier(Input.GetBindModifierFromString(vArgList[1]));

    EButton eButton(Input.GetButtonFromString(vArgList[1]));
    if (eButton != BUTTON_INVALID)
    {
        ActionRegistry.UnbindButton(eBindTable, eButton, iModifier);
        ActionRegistry.UnbindImpulse(eBindTable, eButton, iModifier);
        return true;
    }

    EAxis eAxis(Input.GetAxisFromString(vArgList[1]));
    if (eAxis != AXIS_INVALID)
    {
        ActionRegistry.UnbindAxis(eBindTable, eAxis, iModifier);
        return true;
    }

    Console << SingleQuoteStr(vArgList[1]) << _T(" is not a valid button or axis name") << newl;
    return false;
}


/*--------------------
  UnbindAll
  --------------------*/
CMD(UnbindAll)
{
    Console << _T("Unbinding all previously bound buttons and axes...") << newl;
    ActionRegistry.UnbindAll();
    return true;
}


/*--------------------
  UnbindTable
  --------------------*/
CMD(UnbindTable)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: UnbindTable <console|ui|game|player|commander>") << newl;
        return false;
    }

    EBindTable eBindTable(EBindTableFromString(vArgList[0]));
    if (eBindTable == EBindTable(-1))
    {
        Console << SingleQuoteStr(vArgList[0]) << " is not a valid bind table" << newl;
        return false;
    }

    ActionRegistry.UnbindTable(eBindTable);
    return true;
}



/*--------------------
  BindList

  prints a list of all currently bound buttons and axes
  --------------------*/
CMD(BindList)
{
    int iNumFound(0);

    for (int i(0); i < NUM_BINDTABLES; ++i)
    {
        Console << _T("Listing binds for: ") << EBindTableToString(EBindTable(i)) << newl;
        if (vArgList.size() > 0)
            Console << _T("Containing pattern: ") << vArgList[0] << newl;

#define PRINT_BINDS(type, color) \
        const type##ActionMap &l##type(ActionRegistry.Get##type##ActionMap(EBindTable(i))); \
        for (type##ActionMap::const_iterator it(l##type.begin()); it != l##type.end(); ++it) \
        { \
            for (BindModMap::const_iterator itBind(it->second.begin()); itBind != it->second.end(); ++itBind) \
            { \
                const tstring &sName(itBind->second.GetActionName()); \
                if (vArgList.size() < 1 || sName.find(vArgList[0]) != tstring::npos) \
                { \
                    tstring sAction(itBind->second.GetAction() ? sName : _T("^v") + sName); \
                    tstring sParam(itBind->second.GetParam()); \
                    Console << color << Input.GetBindString(it->first, itBind->first) << sNoColor << _T(" -> ") \
                            << sAction << SPACE << QuoteStr(sParam) << newl; \
                    ++iNumFound; \
                } \
            }\
        }

        PRINT_BINDS(Button, sGreen)
        PRINT_BINDS(Axis, sYellow)

#undef PRINT_BINDS
    }

    Console << newl << iNumFound << _T(" matching binds found") << newl << newl;

    Console << _T("Legend:") << newl
            << sGreen << _T("Green    ") << sNoColor << _T("Button") << newl
            << sYellow << _T("Yellow   ") << sNoColor << _T("Axis") << newl;

    return true;
}


/*--------------------
  ActionList
  --------------------*/
CMD(ActionList)
{
    int iNumFound(0);

    const ActionMap &lActions = CActionRegistry::GetInstance()->GetActionMap();

    // Print actions
    for (ActionMap::const_iterator it(lActions.begin()); it != lActions.end(); ++it)
    {
        if (vArgList.size() == 0 || it->second->GetName().find(vArgList[0]) != tstring::npos)
        {
            Console << it->second->GetName() << newl;
            ++iNumFound;
        }
    }

    Console << newl << iNumFound << _T(" matching actions found") << newl;
    return true;
}


/*--------------------
  Action
  --------------------*/
CMD(Action)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: action <action> [value] [param]") << newl;
        return false;
    }

    IBaseInput *pAction(CActionRegistry::GetInstance()->GetAction(vArgList[0]));

    if (pAction == NULL)
        return false;

    pAction->Do(vArgList.size() > 1 ? AtoF(vArgList[1]) : 1.0f, 1.0f, Input.GetCursorPos(), vArgList.size() > 2 ? vArgList[2] : _T(""));
    return true;
}


/*--------------------
  uiGetKeybindButton
  --------------------*/
UI_CMD(GetKeybindButton, 3)
{
    EBindTable eBindTable(EBindTableFromString(vArgList[0]->Evaluate()));
    const tstring &sAction(vArgList[1]->Evaluate());
    const tstring &sParam(vArgList[2]->Evaluate());
    int iSkip(vArgList.size() > 3 ? AtoI(vArgList[3]->Evaluate()) : 0);

    if (eBindTable == EBindTable(-1))
        return _T("<INVALID TABLE>");

    const ButtonActionMap &lButton(ActionRegistry.GetButtonActionMap(eBindTable)); 
    for (ButtonActionMap::const_iterator it(lButton.begin()); it != lButton.end(); ++it)
    {
        if (it->first == BUTTON_INVALID)
            continue;

        for (BindModMap::const_iterator itBind(it->second.begin()); itBind != it->second.end(); ++itBind)
        {
            if (itBind->second.GetActionName() == sAction && itBind->second.GetParam() == sParam)
            {
                if (iSkip == 0)
                    return Input.GetBindString(it->first, itBind->first);
                else
                    --iSkip;
            }
        }
    }

    return _T("None");
}


/*--------------------
  GetJoystickAxisBind
  --------------------*/
UI_CMD(GetJoystickAxisBind, 3)
{
    EBindTable eBindTable(EBindTableFromString(vArgList[0]->Evaluate()));
    const tstring &sAction(vArgList[1]->Evaluate());
    const tstring &sParam(vArgList[2]->Evaluate());
    int iSkip(vArgList.size() > 3 ? AtoI(vArgList[3]->Evaluate()) : 0);

    if (eBindTable == EBindTable(-1))
        return _T("<INVALID TABLE>");

    const AxisActionMap &mapAxis(ActionRegistry.GetAxisActionMap(eBindTable)); 
    for (AxisActionMap::const_iterator cit(mapAxis.begin()); cit != mapAxis.end(); ++cit)
    {
        if (cit->first < AXIS_JOY_X || cit->first > AXIS_JOY_V)
            continue;

        for (BindModMap::const_iterator itBind(cit->second.begin()); itBind != cit->second.end(); ++itBind)
        {
            if (itBind->second.GetActionName() == sAction && itBind->second.GetParam() == sParam)
            {
                if (iSkip == 0)
                    return Input.GetBindString(cit->first, itBind->first);
                else
                    --iSkip;
            }
        }
    }

    return _T("None");
}


/*--------------------
  ClearJoystickAxisBinds
  --------------------*/
UI_VOID_CMD(ClearJoystickAxisBinds, 3)
{
    EBindTable eBindTable(EBindTableFromString(vArgList[0]->Evaluate()));
    const tstring &sAction(vArgList[1]->Evaluate());
    const tstring &sParam(vArgList[2]->Evaluate());
    int iSkip(vArgList.size() > 3 ? AtoI(vArgList[3]->Evaluate()) : 0);

    if (eBindTable == EBindTable(-1))
        return;

    const AxisActionMap &mapAxis(ActionRegistry.GetAxisActionMap(eBindTable)); 
    for (AxisActionMap::const_iterator cit(mapAxis.begin()); cit != mapAxis.end(); ++cit)
    {
        if (cit->first < AXIS_JOY_X || cit->first > AXIS_JOY_V)
            continue;

        for (BindModMap::const_iterator itBind(cit->second.begin()); itBind != cit->second.end(); ++itBind)
        {
            if (itBind->second.GetActionName() == sAction && itBind->second.GetParam() == sParam)
            {
                if (iSkip == 0)
                    return ActionRegistry.UnbindAxis(eBindTable, cit->first, 0);
                else
                    --iSkip;
            }
        }
    }
}


/*--------------------
  Action
  --------------------*/
UI_VOID_CMD(Action, 1)
{
    IBaseInput *pAction(CActionRegistry::GetInstance()->GetAction(vArgList[0]->Evaluate()));
    if (!pAction)
        return;

    pAction->Do(vArgList.size() > 1 ? AtoF(vArgList[1]->Evaluate()) : 1.0f, 1.0f, Input.GetCursorPos(), vArgList.size() > 2 ? vArgList[2]->Evaluate() : _T(""));
}

