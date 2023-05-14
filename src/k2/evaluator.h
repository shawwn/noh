// (C)2005 S2 Games
// evaluator.h
//
//=============================================================================
#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "c_pool.h"
#include "c_uiscripttoken.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IOperator;
class CEvaluatorToken;
class CExpressionEvaluator;
class IWidget;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef float   (*EvalLookupFn)(const tstring &sName);

enum EOperatorType
{
    OP_UNARY,
    OP_BINARY,
    OP_POST_UNARY,
    OP_NULL
};

const uint MAX_OPERATOR_LENGTH(2);

typedef map<uint, IOperator*>       OperatorMap;
typedef OperatorMap::iterator       OperatorMap_it;

K2_API float    Eval_Evaluate(const tstring &sExpression, bool &bError, EvalLookupFn pfnGetValue);

#define START_OPERATOR(name, symbol) \
class COperator##name : public IOperator \
{ \
public: \
    ~COperator##name()  {} \
    COperator##name() : \
    IOperator(symbol, _T("")) \
    { \
        m_iUnaryPrecedence = GetUnaryPrecedenceVirtual(); \
        m_iBinaryPrecedence = GetBinaryPrecedenceVirtual(); \
        m_iPostUnaryPrecedence = GetPostUnaryPrecedenceVirtual(); \
        m_bUnary = IsUnaryVirtual(); \
        m_bBinary = IsBinaryVirtual(); \
        m_bPostUnary = IsPostUnaryVirtual(); \
\
        CExpressionEvaluator::GetInstance()->RegisterOperator(this); \
    }

#define START_OPERATOR_PAIRED(name, symbol, companion) \
class COperator##name : public IOperator \
{ \
public: \
    ~COperator##name()  {} \
    COperator##name() : \
    IOperator(symbol, companion) \
    { \
        m_iUnaryPrecedence = GetUnaryPrecedenceVirtual(); \
        m_iBinaryPrecedence = GetBinaryPrecedenceVirtual(); \
        m_iPostUnaryPrecedence = GetPostUnaryPrecedenceVirtual(); \
        m_bUnary = IsUnaryVirtual(); \
        m_bBinary = IsBinaryVirtual(); \
        m_bPostUnary = IsPostUnaryVirtual(); \
\
        CExpressionEvaluator::GetInstance()->RegisterOperator(this); \
    }

#define UNARY_FUNCTION \
    int     GetUnaryPrecedenceVirtual() const   { return 3; } \
    bool    IsUnaryVirtual() const              { return true; } \
    void    ExecuteUnary(CEvaluatorToken *&a)

#define BINARY_FUNCTION(precedence) \
    int     GetBinaryPrecedenceVirtual() const  { return precedence; } \
    bool    IsBinaryVirtual() const         { return true; } \
    void    ExecuteBinary(CEvaluatorToken *&a, CEvaluatorToken *&b)

#define POST_UNARY_FUNCTION(precedence) \
    int     GetPostUnaryPrecedenceVirtual() const   { return precedence; } \
    bool    IsPostUnaryVirtual() const              { return true; } \
    void    ExecutePostUnary(CEvaluatorToken *&a)

#define END_OPERATOR(name) \
}; \
COperator##name g_Operator##name;

typedef vector<CEvaluatorToken> ParsedScript;

extern CExpressionEvaluator *g_pExpressionEvaluator;

#define ExpressionEvaluator (*g_pExpressionEvaluator)
//=============================================================================

//=============================================================================
// IOperator
//=============================================================================
class IOperator
{
protected:
    tstring     m_sSymbol;
    tstring     m_sCompanion;
    uint        m_uiSymbolCode;
    uint        m_uiCompanionCode;
    int         m_iUnaryPrecedence;
    int         m_iBinaryPrecedence;
    int         m_iPostUnaryPrecedence;
    bool        m_bUnary;
    bool        m_bBinary;
    bool        m_bPostUnary;

    IOperator();

public:
    virtual ~IOperator()    {}
    IOperator(const tstring &sSymbol, const tstring &sCompanion = TSNULL) :
    m_sSymbol(sSymbol),
    m_sCompanion(sCompanion),
    m_uiSymbolCode(GetOperatorCode(sSymbol)),
    m_uiCompanionCode(GetOperatorCode(sCompanion))
    {
    }

    virtual int             GetUnaryPrecedenceVirtual() const           { return INT_MAX; }
    virtual int             GetBinaryPrecedenceVirtual() const          { return INT_MAX; }
    virtual int             GetPostUnaryPrecedenceVirtual() const       { return INT_MAX; }
    virtual bool            IsUnaryVirtual() const                      { return false; }
    virtual bool            IsBinaryVirtual() const                     { return false; }
    virtual bool            IsPostUnaryVirtual() const                  { return false; }

    const tstring&          GetSymbol() const                           { return m_sSymbol; }
    const tstring&          GetCompanion() const                        { return m_sCompanion; }
    uint                    GetSymbolCode() const                       { return m_uiSymbolCode; }
    uint                    GetCompanionCode() const                    { return m_uiCompanionCode; }
    int                     GetUnaryPrecedence() const                  { return m_iUnaryPrecedence; }
    int                     GetBinaryPrecedence() const                 { return m_iBinaryPrecedence; }
    int                     GetPostUnaryPrecedence() const              { return m_iPostUnaryPrecedence; }
    bool                    IsUnary() const                             { return m_bUnary; }
    bool                    IsBinary() const                            { return m_bBinary; }
    bool                    IsPostUnary() const                         { return m_bPostUnary; }
    bool                    IsPaired() const                            { return m_uiCompanionCode != 0; }

    int             GetPrecedence(EOperatorType eType) const
    {
        switch (eType)
        {
        case OP_UNARY:
            return m_iUnaryPrecedence;
        case OP_BINARY:
            return m_iBinaryPrecedence;
        case OP_POST_UNARY:
            return m_iPostUnaryPrecedence;
        default:
            return INT_MAX;
        }
    }

    virtual void    ExecuteUnary(CEvaluatorToken *&a)                       {}
    virtual void    ExecutePostUnary(CEvaluatorToken *&a)                   {}
    virtual void    ExecuteBinary(CEvaluatorToken *&a, CEvaluatorToken *&b) {}

    static uint     GetOperatorCode(const tstring &sValue)
    {
        uint uiValue(0);

        for (tstring::const_iterator cit(sValue.begin()); cit != sValue.end(); ++cit)
            uiValue += ushort(*cit) << ((cit - sValue.begin()) << 3);

        return uiValue;
    }
};
//=============================================================================

//=============================================================================
// COpCode
//=============================================================================
class COpCode
{
private:
    EOperatorType   m_eType;
    IOperator*      m_pOperator;
    
public:
    ~COpCode()  {}
    COpCode(EOperatorType eType, IOperator *pOperator) :
    m_eType(eType), m_pOperator(pOperator)
    {}

    int     GetPrecedence() const               { return m_pOperator->GetPrecedence(m_eType); }
    bool    IsUnary() const                     { return m_eType == OP_UNARY; }
    bool    IsBinary() const                    { return m_eType == OP_BINARY; }
    bool    IsPostUnary() const                 { return m_eType == OP_POST_UNARY; }
    bool    IsPaired() const                    { return m_pOperator->IsPaired(); }

    EOperatorType   GetType() const             { return m_eType; }
    uint            GetSymbolCode() const       { return m_pOperator->GetSymbolCode(); }
    uint            GetCompanionCode() const    { return m_pOperator->GetCompanionCode(); }

    void    Invalidate()                        { m_eType = OP_NULL; }

    void    Execute(CEvaluatorToken *&a)
    {
        if (m_eType == OP_UNARY)
            m_pOperator->ExecuteUnary(a);
        else if (m_eType == OP_POST_UNARY)
            m_pOperator->ExecutePostUnary(a);
    }

    void    Execute(CEvaluatorToken *&a, CEvaluatorToken *&b)
    {
        m_pOperator->ExecuteBinary(a, b);
    }
};
//=============================================================================

//=============================================================================
// CEvaluatorToken
//=============================================================================
class CEvaluatorToken
{
private:
    bool                    m_bIsOperator;
    IOperator*              m_pOperator;
    CUIScriptToken*         m_pToken;

public:
    ~CEvaluatorToken();
    CEvaluatorToken() :
    m_bIsOperator(false),
    m_pOperator(nullptr),
    m_pToken(nullptr)
    {}

    CEvaluatorToken(IOperator *pOperator) :
    m_bIsOperator(pOperator != nullptr),
    m_pOperator(pOperator),
    m_pToken(nullptr)
    {}

    CEvaluatorToken(CUIScriptToken *pToken) :
    m_bIsOperator(false),
    m_pOperator(nullptr),
    m_pToken(pToken)
    {}

    CEvaluatorToken(const tstring &sLiteral);

    bool    IsOperator() const  { return m_bIsOperator; }

    // Operator Functions
    IOperator*  GetOperator() const { return m_pOperator; }

    // Symbol Functions
    void    AddArgumentChain(CEvaluatorToken *pToken);
    void    AddArgument(const tstring &sValue);
    void    LinkArgument(CEvaluatorToken *pToken);
    const tstring&  GetValue() const;
    void    SetValue(const tstring &sValue);
    void    AssignValue(const tstring &sValue);
    void    AssignToken(CUIScriptToken *pToken);
};
//=============================================================================

//=============================================================================
// CExpressionEvaluator
//=============================================================================
class CExpressionEvaluator
{
    SINGLETON_DEF(CExpressionEvaluator)

private:
    struct Environment
    {
        CPool<CEvaluatorToken>  m_EvaluatorTokenPool;
        CPool<CUIScriptToken>   m_ScriptTokenPool;

        vector<COpCode>             m_stackOperators;
        vector<CEvaluatorToken*>    m_stackSymbols;
        bool                        m_bLastTokenWasValue;
        bool                        m_bError;
        tstring                     m_sError;
        IWidget*                    m_pActiveWidget;
        tstring                     m_sExpression;
        tsvector                    m_vParams;

        Environment() :
        m_EvaluatorTokenPool(1, uint(-1)),
        m_ScriptTokenPool(1, uint(-1)),
        m_bLastTokenWasValue(false),
        m_bError(false),
        m_pActiveWidget(nullptr)
        {}

        ~Environment()
        {
            while (!m_stackSymbols.empty())
            {
                SAFE_DELETE(m_stackSymbols.back());
                m_stackSymbols.pop_back();
            }
        }

        void    Clear()
        {
            while(!m_stackOperators.empty())
                m_stackOperators.pop_back();
            while (!m_stackSymbols.empty())
            {
                SAFE_DELETE(m_stackSymbols.back());
                m_stackSymbols.pop_back();
            }
            m_bLastTokenWasValue = false;
            m_bError = false;
            m_sError = TSNULL;
            m_EvaluatorTokenPool.Reset();
            m_ScriptTokenPool.Reset();
        }

        void            SetError(const tstring &sError) { m_bError = true; m_sError = sError; }
        bool            HasError() const                { return m_bError; }
        const tstring&  GetErrorMessage() const         { return m_sError; }
    };

    OperatorMap             m_mapOperators;

    vector<Environment>     m_vEnvironments;
    int                     m_iCurrentEnvironmentIndex;

    void            PushEnvironment();
    void            PopEnvironment();

    void            PushSymbol(const tstring &sValue);
    void            PushLiteral(const tstring &sValue);
    void            PushOperator(IOperator *pOperator);
    void            PushToken(CEvaluatorToken *pToken);

    void            EvaluateStack();

    Environment&    GetCurrentEnvironment()         { return m_vEnvironments[m_iCurrentEnvironmentIndex]; }
    void            SetError(const tstring &sError) { m_vEnvironments[m_iCurrentEnvironmentIndex].m_bError = true; m_vEnvironments[m_iCurrentEnvironmentIndex].m_sError = sError; }
    bool            HasError() const                { return m_vEnvironments[m_iCurrentEnvironmentIndex].m_bError; }
    const tstring&  GetErrorMessage() const         { return m_vEnvironments[m_iCurrentEnvironmentIndex].m_sError; }


public:
    ~CExpressionEvaluator() {}

    CEvaluatorToken*    GetToken(const tstring &sSymbol, CEvaluatorToken *pResultToken = nullptr);

    void    RegisterOperator(IOperator *pOperator);

    tstring Evaluate(const tstring &sExpression, IWidget *pActiveWidget, const tsvector &vParams);
    bool    Parse(const tstring &sExpression, EvalLookupFn pfnGetValue, ParsedScript &vTokens);

    CEvaluatorToken*    AllocEvaluatorToken()   { return m_vEnvironments[m_iCurrentEnvironmentIndex].m_EvaluatorTokenPool.Allocate(); }
    CUIScriptToken*     AllocScriptToken()      { return m_vEnvironments[m_iCurrentEnvironmentIndex].m_ScriptTokenPool.Allocate(); }

    const tsvector&     GetCurrentParams() const    { return m_vEnvironments[m_iCurrentEnvironmentIndex].m_vParams; }
};
//=============================================================================

#endif //__EVALUATOR_H__
