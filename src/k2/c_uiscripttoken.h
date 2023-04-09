// (C)2007 S2 Games
// c_uiscripttoken.h
//
//=============================================================================
#ifndef __C_UISCRIPTTOKEN_H__
#define __C_UISCRIPTTOKEN_H__

//=============================================================================
// Declarations
//=============================================================================
class CUICmd;
class CUITrigger;
class IWidget;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<class CUIScriptToken*>       ScriptTokenVector;
typedef ScriptTokenVector::iterator         ScriptTokenVector_it;
typedef ScriptTokenVector::const_iterator   ScriptTokenVector_cit;
//=============================================================================

//=============================================================================
// CUIScriptToken
//=============================================================================
class CUIScriptToken
{
public:
    enum ETokenType
    {
        TOKEN_NULL,
        TOKEN_CMD,
        TOKEN_CVAR,
        TOKEN_TRIGGER,
        TOKEN_LITERAL
    };

private:
    IWidget*    m_pCaller;
    ETokenType  m_eType;

    CUICmd*     m_pCmd;
    ICvar*      m_pCvar;
    CUITrigger* m_pTrigger;
    IWidget*    m_pWidget;
    tstring     m_sLiteral;
    
    CUIScriptToken*     m_pLink;
    ScriptTokenVector   m_vArgList;

public:
    ~CUIScriptToken();
    CUIScriptToken() :
    m_pCaller(NULL),
    m_eType(TOKEN_NULL),
    m_pCmd(NULL),
    m_pCvar(NULL),
    m_pTrigger(NULL),
    m_pWidget(NULL),
    m_pLink(NULL)
    {}

    CUIScriptToken(IWidget *pCaller, ETokenType eType) :
    m_pCaller(pCaller),
    m_eType(eType),
    m_pCmd(NULL),
    m_pCvar(NULL),
    m_pTrigger(NULL),
    m_pWidget(NULL),
    m_pLink(NULL)
    {}

    CUIScriptToken(IWidget *pCaller, CUICmd *pCmd) :
    m_pCaller(pCaller),
    m_eType(TOKEN_CMD),
    m_pCmd(pCmd),
    m_pCvar(NULL),
    m_pTrigger(NULL),
    m_pWidget(NULL),
    m_pLink(NULL)
    {}

    CUIScriptToken(IWidget *pCaller, ICvar *pCvar) :
    m_pCaller(pCaller),
    m_eType(TOKEN_CVAR),
    m_pCmd(NULL),
    m_pCvar(pCvar),
    m_pTrigger(NULL),
    m_pWidget(NULL),
    m_pLink(NULL)
    {}

    CUIScriptToken(IWidget *pCaller, CUITrigger *pTrigger) :
    m_pCaller(pCaller),
    m_eType(TOKEN_TRIGGER),
    m_pCmd(NULL),
    m_pCvar(NULL),
    m_pTrigger(pTrigger),
    m_pWidget(NULL),
    m_pLink(NULL)
    {}

    CUIScriptToken(IWidget *pCaller, const tstring &sLiteral) :
    m_pCaller(pCaller),
    m_eType(TOKEN_LITERAL),
    m_pCmd(NULL),
    m_pCvar(NULL),
    m_pTrigger(NULL),
    m_pWidget(NULL),
    m_sLiteral(sLiteral),
    m_pLink(NULL)
    {}

    ETokenType  GetType() const { return m_eType; }

    ICvar*          GetCvar() const     { return m_pCvar; }
    IWidget*        GetWidget() const   { return m_pWidget; }

    K2_API const tstring&   Evaluate();
    K2_API int      EvaluateInteger();
    void            Assign(const tstring &sValue);

    void            SetValue(const tstring &sValue) { m_eType = TOKEN_LITERAL; m_sLiteral = sValue; }

    void            LinkArgument(CUIScriptToken *pToken)    { CUIScriptToken *pLink(this); while (pLink->m_pLink != NULL) pLink = pLink->m_pLink; pLink->m_pLink = pToken; }
    CUIScriptToken* GetNextLink()                           { return m_pLink; }
    void            AddArgument(CUIScriptToken *pToken)     { m_vArgList.push_back(pToken); }
    void            AddArgument(const tstring &sValue)      { m_vArgList.push_back(K2_NEW(ctx_Console,  CUIScriptToken)(m_pCaller, sValue)); }

    CUIScriptToken* GetArg(uint uiIndex) const              { if (uiIndex < m_vArgList.size()) return m_vArgList[uiIndex]; return NULL; }
};
//=============================================================================

#include "evaluator.h" // need this to make sure CUIScriptToken::new/delete is defined everywhere it is inlined

#endif //__C_UISCRIPTTOKEN_H__
