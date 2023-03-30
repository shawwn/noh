// (C)2005 S2 Games
// c_actionregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_actionregistry.h"
#include "c_action.h"
#include "stringutils.h"
#include "c_gamebind.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
SINGLETON_INIT(CActionRegistry)
CActionRegistry *g_pActionRegistry(CActionRegistry::GetInstance());
//=============================================================================

/*====================
  CActionRegistry::CActionRegistry
  ====================*/
CActionRegistry::CActionRegistry()
{
}


/*====================
  CActionRegistry::Register
  ====================*/
void    CActionRegistry::Register(IBaseInput *pAction)
{
    // Make sure there is no name collision
    ActionMap::iterator findit(m_mapActions.find(pAction->GetName()));
    if (findit != m_mapActions.end())
    {
        Console.Err << _T("An action named ") << QuoteStr(pAction->GetName())
                    << _T(" already exists.") << newl;
        //return;
    }

    m_mapActions[pAction->GetName()] = pAction;

    // update binds associated with this action
    for (int i(0); i < NUM_BINDTABLES; ++i)
    {       
        ButtonActionMap::iterator itButton(m_mapButtons[i].begin());
        while (itButton != m_mapButtons[i].end())
        {
            BindModMap::iterator itBind(itButton->second.begin());
            while (itBind != itButton->second.end())
            {
                if (itBind->second.GetActionName() == pAction->GetName() && itBind->second.GetActionType() == pAction->GetType())
                    itBind->second.SetAction(pAction);

                ++itBind;
            }

            ++itButton;
        }

        AxisActionMap::iterator itAxis(m_mapAxes[i].begin());
        while (itAxis != m_mapAxes[i].end())
        {
            BindModMap::iterator itBind(itAxis->second.begin());
            while (itBind != itAxis->second.end())
            {
                if (itBind->second.GetActionName() == pAction->GetName() && itBind->second.GetActionType() == pAction->GetType())
                    itBind->second.SetAction(pAction);

                ++itBind;
            }

            ++itAxis;
        }
    }
}


/*====================
  CActionRegistry::Unregister
  ====================*/
void    CActionRegistry::Unregister(IBaseInput *pAction)
{
    const tstring &sName(pAction->GetName());

    ActionMap::iterator findit(m_mapActions.find(sName));
    if (findit == m_mapActions.end() || findit->second != pAction)
    {
        Console.Dev << _T("Action ") << sName << _T(" not found.") << newl;
        return;
    }

    // disable binds associated with this action
    for (int i(0); i < NUM_BINDTABLES; ++i)
    {
        ButtonActionMap::iterator itButton(m_mapButtons[i].begin());
        while (itButton != m_mapButtons[i].end())
        {
            BindModMap::iterator itBind(itButton->second.begin());
            while (itBind != itButton->second.end())
            {
                if (itBind->second.GetActionName() == sName)
                    itBind->second.SetAction(NULL);

                ++itBind;
            }

            ++itButton;
        }

        AxisActionMap::iterator itAxis(m_mapAxes[i].begin());
        while (itAxis != m_mapAxes[i].end())
        {
            BindModMap::iterator itBind(itAxis->second.begin());
            while (itBind != itAxis->second.end())
            {
                if (itBind->second.GetActionName() == sName)
                    itBind->second.SetAction(NULL);

                ++itBind;
            }

            ++itAxis;
        }
    }

    //Console.Dev << _T("Action ") << sName << _T(" has been unregistered.") << newl;
    m_mapActions.erase(findit);
}


/*====================
  CActionRegistry::BindButton
  ====================*/
void    CActionRegistry::BindButton(EBindTable eTable, EButton eButton, int iModifier, const tstring &sAction, const tstring &sParam, int iFlags)
{
    // Remove this key from all game binds
    const ElementList &lGameBinds(ConsoleRegistry.GetGameBindList());
    tstring sKey(Input.GetBindString(eButton, iModifier));

    for (ElementList_cit cit(lGameBinds.begin()), citEnd(lGameBinds.end()); cit != citEnd; ++cit)
    {
        IGameBind *pGameBind(static_cast<IGameBind *>(cit->second));
        if (pGameBind->GetBindTable() != eTable)
            continue;

        if (pGameBind->GetKey1String() == sKey)
            pGameBind->SetKey1(TSNULL, false);
        if (pGameBind->GetKey2String() == sKey)
            pGameBind->SetKey2(TSNULL, false);
    }

    // Update the game bind table
    for (ElementList_cit cit(lGameBinds.begin()), citEnd(lGameBinds.end()); cit != citEnd; ++cit)
    {
        IGameBind *pGameBind(static_cast<IGameBind *>(cit->second));
        if (pGameBind->GetBindTable() == eTable &&
            pGameBind->GetAction() == sAction &&
            pGameBind->GetParam() == sParam)
        {
            if (pGameBind->GetKey1String() == Input.ToString(BUTTON_INVALID))
                pGameBind->SetKey1(sKey, false);
            else
                pGameBind->SetKey2(sKey, false);
        }
    }

    ActionMap::iterator itAction(m_mapActions.find(sAction));
    if (itAction == m_mapActions.end())
    {
        m_mapButtons[eTable][eButton][iModifier] = CBind(sAction, AT_BUTTON, sParam, iFlags);
        return;
    }

    if (itAction->second->GetType() != AT_BUTTON)
    {
        Console.Warn << SingleQuoteStr(sAction) << _T(" is not a button type action.") << newl;
        return;
    }

    if (itAction->second->GetFlags() & ACTION_CORE)
        iFlags |= BIND_CORE;
    if (itAction->second->GetFlags() & ACTION_CGAME)
        iFlags |= BIND_CGAME;
    if (itAction->second->GetFlags() & ACTION_SGAME)
        iFlags |= BIND_SGAME;

    if (!(iFlags & BIND_LOADCONFIG && 
        m_mapButtons[eTable].find(eButton) != m_mapButtons[eTable].end() &&
        m_mapButtons[eTable][eButton].find(iModifier) != m_mapButtons[eTable][eButton].end()))
    {
        m_mapButtons[eTable][eButton][iModifier] = CBind(itAction->second, sParam, iFlags);
    }
}


/*====================
  CActionRegistry::BindAxis
  ====================*/
void    CActionRegistry::BindAxis(EBindTable eTable, EAxis eAxis, int iModifier, const tstring &sAction, const tstring &sParam, int iFlags)
{
    // Remove this key from all game binds
    const ElementList &lGameBinds(ConsoleRegistry.GetGameBindList());
    tstring sKey(Input.GetBindString(eAxis, iModifier));

    for (ElementList_cit cit(lGameBinds.begin()), citEnd(lGameBinds.end()); cit != citEnd; ++cit)
    {
        IGameBind *pGameBind(static_cast<IGameBind *>(cit->second));
        if (pGameBind->GetBindTable() != eTable)
            continue;

        if (pGameBind->GetKey1String() == sKey)
            pGameBind->SetKey1(TSNULL, false);
        if (pGameBind->GetKey2String() == sKey)
            pGameBind->SetKey2(TSNULL, false);
    }

    // Update the game bind table
    for (ElementList_cit cit(lGameBinds.begin()), citEnd(lGameBinds.end()); cit != citEnd; ++cit)
    {
        IGameBind *pGameBind(static_cast<IGameBind *>(cit->second));
        if (pGameBind->GetBindTable() == eTable &&
            pGameBind->GetAction() == sAction &&
            pGameBind->GetParam() == sParam)
        {
            if (pGameBind->GetKey1String() == Input.ToString(BUTTON_INVALID))
                pGameBind->SetKey1(sKey, false);
            else
                pGameBind->SetKey2(sKey, false);
        }
    }

    ActionMap::iterator itAction(m_mapActions.find(sAction));
    if (itAction == m_mapActions.end())
    {
        m_mapAxes[eTable][eAxis][iModifier] = CBind(sAction, AT_AXIS, sParam, iFlags);
        return;
    }

    if (itAction->second->GetType() != AT_AXIS)
    {
        Console << SingleQuoteStr(sAction) << _T(" is not an axis.") << newl;
        return;
    }

    if (itAction->second->GetFlags() & ACTION_CORE)
        iFlags |= BIND_CORE;
    if (itAction->second->GetFlags() & ACTION_CGAME)
        iFlags |= BIND_CGAME;
    if (itAction->second->GetFlags() & ACTION_SGAME)
        iFlags |= BIND_SGAME;

    if (!(iFlags & BIND_LOADCONFIG && 
        m_mapAxes[eTable].find(eAxis) != m_mapAxes[eTable].end() &&
        m_mapAxes[eTable][eAxis].find(iModifier) != m_mapAxes[eTable][eAxis].end()))
    {
        m_mapAxes[eTable][eAxis][iModifier] = CBind(itAction->second, sParam, iFlags);
    }
}


/*====================
  CActionRegistry::BindImpulse
  ====================*/
void    CActionRegistry::BindImpulse(EBindTable eTable, EButton eButton, int iModifier, const tstring &sAction, const tstring &sParam, int iFlags)
{
    // Remove this key from all game binds
    const ElementList &lGameBinds(ConsoleRegistry.GetGameBindList());
    tstring sKey(Input.GetBindString(eButton, iModifier));

    for (ElementList_cit cit(lGameBinds.begin()), citEnd(lGameBinds.end()); cit != citEnd; ++cit)
    {
        IGameBind *pGameBind(static_cast<IGameBind *>(cit->second));
        if (pGameBind->GetBindTable() != eTable)
            continue;

        if (pGameBind->GetKey1String() == sKey)
            pGameBind->SetKey1(TSNULL, false);
        if (pGameBind->GetKey2String() == sKey)
            pGameBind->SetKey2(TSNULL, false);
    }

    // Update the game bind table
    for (ElementList_cit cit(lGameBinds.begin()), citEnd(lGameBinds.end()); cit != citEnd; ++cit)
    {
        IGameBind *pGameBind(static_cast<IGameBind *>(cit->second));
        if (pGameBind->GetBindTable() == eTable &&
            pGameBind->GetAction() == sAction &&
            pGameBind->GetParam() == sParam)
        {
            if (pGameBind->GetKey1String() == Input.ToString(BUTTON_INVALID))
                pGameBind->SetKey1(sKey, false);
            else
                pGameBind->SetKey2(sKey, false);
        }
    }

    ActionMap::iterator itAction(m_mapActions.find(sAction));
    if (itAction == m_mapActions.end())
    {
        m_mapButtons[eTable][eButton][iModifier] = CBind(sAction, AT_IMPULSE, sParam, iFlags);
        return;
    }

    if (itAction->second->GetType() != AT_IMPULSE)
    {
        Console << SingleQuoteStr(sAction) << _T(" is not an impulse.") << newl;
        return;
    }

    if (itAction->second->GetFlags() & ACTION_CORE)
        iFlags |= BIND_CORE;
    if (itAction->second->GetFlags() & ACTION_CGAME)
        iFlags |= BIND_CGAME;
    if (itAction->second->GetFlags() & ACTION_SGAME)
        iFlags |= BIND_SGAME;

    if (!(iFlags & BIND_LOADCONFIG && 
        m_mapButtons[eTable].find(eButton) != m_mapButtons[eTable].end() &&
        m_mapButtons[eTable][eButton].find(iModifier) != m_mapButtons[eTable][eButton].end()))
    {
        m_mapButtons[eTable][eButton][iModifier] = CBind(itAction->second, sParam, iFlags);
    }
}


/*====================
  CActionRegistry::UnbindButton
  ====================*/
void    CActionRegistry::UnbindButton(EBindTable eTable, EButton eButton, int iModifier)
{
    // Remove this key from all game binds
    const ElementList &lGameBinds(ConsoleRegistry.GetGameBindList());
    tstring sKey(Input.GetBindString(eButton, iModifier));

    for (ElementList_cit cit(lGameBinds.begin()), citEnd(lGameBinds.end()); cit != citEnd; ++cit)
    {
        IGameBind *pGameBind(static_cast<IGameBind *>(cit->second));
        if (pGameBind->GetBindTable() != eTable)
            continue;

        if (pGameBind->GetKey1String() == sKey)
            pGameBind->SetKey1(TSNULL, false);
        if (pGameBind->GetKey2String() == sKey)
            pGameBind->SetKey2(TSNULL, false);
    }

    m_mapButtons[eTable][eButton].erase(iModifier);
}


/*====================
  CActionRegistry::UnbindAxis
  ====================*/
void    CActionRegistry::UnbindAxis(EBindTable eTable, EAxis eAxis, int iModifier)
{
    // Remove this key from all game binds
    const ElementList &lGameBinds(ConsoleRegistry.GetGameBindList());
    tstring sKey(Input.GetBindString(eAxis, iModifier));

    for (ElementList_cit cit(lGameBinds.begin()), citEnd(lGameBinds.end()); cit != citEnd; ++cit)
    {
        IGameBind *pGameBind(static_cast<IGameBind *>(cit->second));
        if (pGameBind->GetBindTable() != eTable)
            continue;

        if (pGameBind->GetKey1String() == sKey)
            pGameBind->SetKey1(TSNULL, false);
        if (pGameBind->GetKey2String() == sKey)
            pGameBind->SetKey2(TSNULL, false);
    }

    m_mapAxes[eTable][eAxis].erase(iModifier);
}


/*====================
  CActionRegistry::UnbindImpulse
  ====================*/
void    CActionRegistry::UnbindImpulse(EBindTable eTable, EButton eButton, int iModifier)
{
    // Remove this key from all game binds
    const ElementList &lGameBinds(ConsoleRegistry.GetGameBindList());
    tstring sKey(Input.GetBindString(eButton, iModifier));

    for (ElementList_cit cit(lGameBinds.begin()), citEnd(lGameBinds.end()); cit != citEnd; ++cit)
    {
        IGameBind *pGameBind(static_cast<IGameBind *>(cit->second));
        if (pGameBind->GetBindTable() != eTable)
            continue;

        if (pGameBind->GetKey1String() == sKey)
            pGameBind->SetKey1(TSNULL, false);
        if (pGameBind->GetKey2String() == sKey)
            pGameBind->SetKey2(TSNULL, false);
    }

    m_mapButtons[eTable][eButton].erase(iModifier);
}


/*====================
  CActionRegistry::UnbindAll
  ====================*/
void    CActionRegistry::UnbindAll()
{
    for (int i(0); i < NUM_BINDTABLES; ++i)
    {
        m_mapButtons[i].clear();
        m_mapAxes[i].clear();
    }
}


/*====================
  CActionRegistry::UnbindTable
  ====================*/
void    CActionRegistry::UnbindTable(EBindTable eTable)
{
    m_mapButtons[eTable].clear();
    m_mapAxes[eTable].clear();
}


/*====================
  CActionRegistry::GetBind
  ====================*/
CBind*  CActionRegistry::GetBind(EBindTable eTable, EButton eButton, int iModifier)
{
    ButtonActionMap::iterator itButton(m_mapButtons[eTable].find(eButton));
    if (itButton == m_mapButtons[eTable].end())
        return NULL;

    BindModMap::iterator itBind(itButton->second.begin());
    CBind *pClosestBind(NULL);
    while (itBind != itButton->second.end())
    {
        if (itBind->first == iModifier)
            return &itBind->second;
        else if ((itBind->first & iModifier) == itBind->first)
            pClosestBind = &itBind->second;

        itBind++;
    }

    return pClosestBind;
}

CBind*  CActionRegistry::GetBind(EBindTable eTable, EAxis eAxis, int iModifier)
{
    AxisActionMap::iterator itAxis(m_mapAxes[eTable].find(eAxis));
    if (itAxis == m_mapAxes[eTable].end())
        return NULL;

    BindModMap::iterator itBind(itAxis->second.begin());
    CBind *pClosestBind(NULL);
    while (itBind != itAxis->second.end())
    {
        if (itBind->first == iModifier)
            return &itBind->second;
        else if ((itBind->first & iModifier) == itBind->first)
            pClosestBind = &itBind->second;

        itBind++;
    }

    return pClosestBind;
}
