// (C)2007 S2 Games
// c_uiscripttoken.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uiscripttoken.h"
#include "c_uicmd.h"
#include "c_uitrigger.h"
//=============================================================================

/*====================
  CUIScriptToken::~CUIScriptToken
  ====================*/
CUIScriptToken::~CUIScriptToken()
{
    for (ScriptTokenVector_it it(m_vArgList.begin()); it != m_vArgList.end(); ++it)
        SAFE_DELETE(*it);
}


/*====================
  CUIScriptToken::Evaluate
  ====================*/
const tstring&  CUIScriptToken::Evaluate()
{
    switch (m_eType)
    {
    case TOKEN_CMD:
        if (m_pCmd == NULL)
            return TSNULL;
        SetValue(m_pCmd->Execute(m_pCaller, m_vArgList));
        return m_sLiteral;

    case TOKEN_CVAR:
        if (m_pCvar == NULL)
            return TSNULL;
        SetValue(m_pCvar->GetString());
        return m_sLiteral;

    case TOKEN_TRIGGER:
        if (m_pTrigger == NULL)
            return TSNULL;
        return m_pTrigger->GetLastParam();

    case TOKEN_LITERAL:
        return m_sLiteral;
    }
    return TSNULL;
}


/*====================
  CUIScriptToken::EvaluateInteger
  ====================*/
int     CUIScriptToken::EvaluateInteger()
{

    switch (m_eType)
    {
    case TOKEN_CMD:
        if (m_pCmd == NULL)
            return 0;
        SetValue(m_pCmd->Execute(m_pCaller, m_vArgList));
        return AtoI(m_sLiteral);
    case TOKEN_CVAR:
        if (m_pCvar == NULL)
            return 0;
        return m_pCvar->GetInteger();

    case TOKEN_TRIGGER:
        if (m_pTrigger == NULL)
            return 0;
        return AtoI(m_pTrigger->GetLastParam());

    case TOKEN_LITERAL:
        return AtoI(m_sLiteral);
    }
    return 0;
}


/*====================
  CUIScriptToken::Assign
  ====================*/
void    CUIScriptToken::Assign(const tstring &sValue)
{
    switch (m_eType)
    {
    case TOKEN_CMD:
        Console << _T("Can not assign a value to a command") << newl;
        break;

    case TOKEN_CVAR:
        if (m_pCvar == NULL)
            break;
        m_pCvar->Set(sValue);
        break;

    case TOKEN_TRIGGER:
        Console << _T("Can not assign a value to a trigger") << newl;
        break;

    case TOKEN_LITERAL:
        Console.Warn << _T("Assigned value to a string literal") << newl;
        m_sLiteral = sValue;
        break;
    }
}
