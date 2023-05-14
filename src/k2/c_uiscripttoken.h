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
    m_pCaller(nullptr),
    m_eType(TOKEN_NULL),
    m_pCmd(nullptr),
    m_pCvar(nullptr),
    m_pTrigger(nullptr),
    m_pWidget(nullptr),
    m_pLink(nullptr)
    {}

    CUIScriptToken(IWidget *pCaller, ETokenType eType) :
    m_pCaller(pCaller),
    m_eType(eType),
    m_pCmd(nullptr),
    m_pCvar(nullptr),
    m_pTrigger(nullptr),
    m_pWidget(nullptr),
    m_pLink(nullptr)
    {}

    CUIScriptToken(IWidget *pCaller, CUICmd *pCmd) :
    m_pCaller(pCaller),
    m_eType(TOKEN_CMD),
    m_pCmd(pCmd),
    m_pCvar(nullptr),
    m_pTrigger(nullptr),
    m_pWidget(nullptr),
    m_pLink(nullptr)
    {}

    CUIScriptToken(IWidget *pCaller, ICvar *pCvar) :
    m_pCaller(pCaller),
    m_eType(TOKEN_CVAR),
    m_pCmd(nullptr),
    m_pCvar(pCvar),
    m_pTrigger(nullptr),
    m_pWidget(nullptr),
    m_pLink(nullptr)
    {}

    CUIScriptToken(IWidget *pCaller, CUITrigger *pTrigger) :
    m_pCaller(pCaller),
    m_eType(TOKEN_TRIGGER),
    m_pCmd(nullptr),
    m_pCvar(nullptr),
    m_pTrigger(pTrigger),
    m_pWidget(nullptr),
    m_pLink(nullptr)
    {}

    CUIScriptToken(IWidget *pCaller, const tstring &sLiteral) :
    m_pCaller(pCaller),
    m_eType(TOKEN_LITERAL),
    m_pCmd(nullptr),
    m_pCvar(nullptr),
    m_pTrigger(nullptr),
    m_pWidget(nullptr),
    m_sLiteral(sLiteral),
    m_pLink(nullptr)
    {}

    ETokenType  GetType() const { return m_eType; }

    ICvar*          GetCvar() const     { return m_pCvar; }
    IWidget*        GetWidget() const   { return m_pWidget; }

    K2_API const tstring&   Evaluate();
    K2_API int      EvaluateInteger();
    void            Assign(const tstring &sValue);

    void            SetValue(const tstring &sValue) { m_eType = TOKEN_LITERAL; m_sLiteral = sValue; }

    void            LinkArgument(CUIScriptToken *pToken)    { CUIScriptToken *pLink(this); while (pLink->m_pLink != nullptr) pLink = pLink->m_pLink; pLink->m_pLink = pToken; }
    CUIScriptToken* GetNextLink()                           { return m_pLink; }
    void            AddArgument(CUIScriptToken *pToken)     { m_vArgList.push_back(pToken); }
    void            AddArgument(const tstring &sValue)      { m_vArgList.push_back(K2_NEW(ctx_Console,  CUIScriptToken)(m_pCaller, sValue)); }

    CUIScriptToken* GetArg(uint uiIndex) const              { if (uiIndex < m_vArgList.size()) return m_vArgList[uiIndex]; return nullptr; }
};
//=============================================================================

#include "evaluator.h" // need this to make sure CUIScriptToken::new/delete is defined everywhere it is inlined

#endif //__C_UISCRIPTTOKEN_H__
