// (C)2005 S2 Games
// evaluator.cpp
//
// expression evaluator
// original code by Shawn Hargreaves
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "evaluator.h"
#include "c_uicmd.h"
#include "c_uiscripttoken.h"
#include "c_uicmdregistry.h"
#include "c_uiscript.h"
#include "i_widget.h"
#include "c_uitriggerregistry.h"
#include "c_uimanager.h"
#include "c_interface.h"
#include "c_table.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CExpressionEvaluator);
CExpressionEvaluator *g_pExpressionEvaluator(CExpressionEvaluator::GetInstance());

const float E   =   2.71828182845904523536f;
const float pi  =   3.14159265358979323846f;

// internal state information
int evaluate_error;
TCHAR operator_stack[256];
float value_stack[256];
int stack_depth;
float current_val;
int current_valid;

// operator tokens
const TCHAR OP_OPEN_PAREN =     '(';
const TCHAR OP_CLOSE_PAREN =    ')';
const TCHAR OP_PLUS =           '+';
const TCHAR OP_MINUS =          '-';
const TCHAR OP_MUL =            '*';
const TCHAR OP_DIV =            '/';
const TCHAR OP_POWER =          '^';
const TCHAR OP_NEGATE =         '~';
const TCHAR OP_SQRT =           'q';
const TCHAR OP_SIN =            's';
const TCHAR OP_COS =            'c';
const TCHAR OP_TAN =            't';
const TCHAR OP_ASIN =           'S';
const TCHAR OP_ACOS =           'C';
const TCHAR OP_ATAN =           'T';
const TCHAR OP_LOG =            'l';
const TCHAR OP_LN =             'L';
const TCHAR OP_CEIL =           'e';
const TCHAR OP_FLOOR =          'f';
const TCHAR OP_ROUND =          'r';
const TCHAR OP_SATURATE =       'A';
const TCHAR OP_ABS =            'a';
const TCHAR OP_MOD =            '%';
const TCHAR OP_EQUALS =         '=';
const TCHAR OP_NOT_EQUALS =     '#';
const TCHAR OP_LESS =           '<';
const TCHAR OP_GREATER =        '>';
const TCHAR OP_LESS_EQUALS =    '{';
const TCHAR OP_GREATER_EQUALS = '}';
const TCHAR OP_OR =             '|';
const TCHAR OP_AND =            '&';
const TCHAR OP_NOT =            '!';
const TCHAR OP_TERMINATOR =     ' ';

START_OPERATOR(Plus, _T("+"))
BINARY_FUNCTION(6)  { a->SetValue(XtoA(AtoF(a->GetValue()) + AtoF(b->GetValue()))); }
END_OPERATOR(Plus)

START_OPERATOR(Minus, _T("-"))
UNARY_FUNCTION      { a->SetValue(XtoA(-AtoF(a->GetValue()))); }
BINARY_FUNCTION(6)  { a->SetValue(XtoA(AtoF(a->GetValue()) - AtoF(b->GetValue()))); }
END_OPERATOR(Minus)

START_OPERATOR(Star, _T("*"))
UNARY_FUNCTION
{
    tstring sName(a->GetValue());
    ExpressionEvaluator.GetToken(sName, a);
}
BINARY_FUNCTION(5)  { a->SetValue(XtoA(AtoF(a->GetValue()) * AtoF(b->GetValue()))); }
END_OPERATOR(Star)

START_OPERATOR(Divide, _T("/"))
BINARY_FUNCTION(5)
{
    float fB(AtoF(b->GetValue()));

    if (fB != 0.0f)
        a->SetValue(XtoA(AtoF(a->GetValue()) / fB)); 
    else
        a->SetValue(_T("0"));
}
END_OPERATOR(Divide)

START_OPERATOR(Modulus, _T("%"))
BINARY_FUNCTION(5)  { a->SetValue(XtoA(fmod(AtoF(a->GetValue()), AtoF(b->GetValue())))); }
END_OPERATOR(Modulus)

START_OPERATOR(Paste, _T("#"))
BINARY_FUNCTION(7)  { a->SetValue(a->GetValue() + b->GetValue()); }
END_OPERATOR(Paste)

START_OPERATOR(Increment, _T("++"))
UNARY_FUNCTION          { a->AssignValue(XtoA(AtoF(a->GetValue()) + 1.0f)); }
POST_UNARY_FUNCTION(3)  { tstring sValue(a->GetValue()); a->AssignValue(XtoA(AtoF(sValue) + 1.0f)); a->SetValue(sValue); }
END_OPERATOR(Increment)

START_OPERATOR(Decrement, _T("--"))
UNARY_FUNCTION          { a->AssignValue(XtoA(AtoF(a->GetValue()) - 1.0f)); }
POST_UNARY_FUNCTION(3)  { tstring sValue(a->GetValue()); a->AssignValue(XtoA(AtoF(sValue) - 1.0f)); a->SetValue(sValue); }
END_OPERATOR(Decrement)

int Factorial(int i)
{
    if (i <= 1)
        return 1;

    return i * Factorial(i - 1);
}

START_OPERATOR(Bang, _T("!"))
UNARY_FUNCTION
{
    if (AtoB(a->GetValue()) == false)
        a->SetValue(_T("1"));
    else
        a->SetValue(_T("0"));
}
POST_UNARY_FUNCTION(3)  { a->SetValue(XtoA(Factorial(AtoI(a->GetValue())))); }
END_OPERATOR(Bang)

START_OPERATOR(LogicalAnd, _T("&&"))
BINARY_FUNCTION(13)
{
    if (AtoB(a->GetValue()) && AtoB(b->GetValue()))
        a->SetValue(_T("1"));
    else
        a->SetValue(_T("0"));
}
END_OPERATOR(LogicalAnd)

START_OPERATOR(LogicalOr, _T("||"))
BINARY_FUNCTION(14)
{
    if (AtoB(a->GetValue()) || AtoB(b->GetValue()))
        a->SetValue(_T("1"));
    else
        a->SetValue(_T("0"));
}
END_OPERATOR(LogicalOr)

START_OPERATOR(Equal, _T("=="))
BINARY_FUNCTION(9)
{
    if (AtoF(a->GetValue()) == AtoF(b->GetValue()))
        a->SetValue(_T("1"));
    else
        a->SetValue(_T("0"));
}
END_OPERATOR(Equal)

START_OPERATOR(NotEqual, _T("!="))
BINARY_FUNCTION(9)
{
    if (AtoF(a->GetValue()) != AtoF(b->GetValue()))
        a->SetValue(_T("1"));
    else
        a->SetValue(_T("0"));
}
END_OPERATOR(NotEqual)

START_OPERATOR(LessThan, _T("<"))
BINARY_FUNCTION(8)
{
    if (AtoF(a->GetValue()) < AtoF(b->GetValue()))
        a->SetValue(_T("1"));
    else
        a->SetValue(_T("0"));
}
END_OPERATOR(LessThan)

START_OPERATOR(GreaterThan, _T(">"))
BINARY_FUNCTION(8)
{
    if (AtoF(a->GetValue()) > AtoF(b->GetValue()))
        a->SetValue(_T("1"));
    else
        a->SetValue(_T("0"));
}
END_OPERATOR(GreaterThan)

START_OPERATOR(LessThanEqual, _T("<="))
BINARY_FUNCTION(8)
{
    if (AtoF(a->GetValue()) <= AtoF(b->GetValue()))
        a->SetValue(_T("1"));
    else
        a->SetValue(_T("0"));
}
END_OPERATOR(LessThanEqual)

START_OPERATOR(GreaterThanEqual, _T(">="))
BINARY_FUNCTION(8)
{
    if (AtoF(a->GetValue()) >= AtoF(b->GetValue()))
        a->SetValue(_T("1"));
    else
        a->SetValue(_T("0"));
}
END_OPERATOR(GreaterThanEqual)

START_OPERATOR_PAIRED(OpenParen, _T("("), _T(")"))
UNARY_FUNCTION      {}
BINARY_FUNCTION(2)  { a->AddArgumentChain(b);  }
END_OPERATOR(OpenParen)

START_OPERATOR(EmptyParen, _T("()"))
POST_UNARY_FUNCTION(2)  {}
END_OPERATOR(EmptyParen)

START_OPERATOR(CloseParen, _T(")"))
POST_UNARY_FUNCTION(20) {}
END_OPERATOR(CloseParen)

START_OPERATOR(Comma, _T(","))
BINARY_FUNCTION(17)     { a->LinkArgument(b); }
END_OPERATOR(Comma)

START_OPERATOR(Assign, _T("="))
BINARY_FUNCTION(16)     { a->AssignValue(b->GetValue()); }
END_OPERATOR(Assign)

START_OPERATOR(Null, _T(""))
END_OPERATOR(Null)

START_OPERATOR(ConditionalMultiStatementLeft, _T("?"))
BINARY_FUNCTION(15)     { if (AtoB(a->GetValue())) b->GetValue(); }
END_OPERATOR(ConditionalMultiStatementLeft)

START_OPERATOR(ConditionalMultiStatementRight, _T(":"))
BINARY_FUNCTION(15)     { if (!AtoB(a->GetValue())) b->GetValue(); }
END_OPERATOR(ConditionalMultiStatementRight)
//=============================================================================


/*====================
  CExpressionEvaluator::CExpressionEvaluator
  ====================*/
CExpressionEvaluator::CExpressionEvaluator() :
m_vEnvironments(16),
m_iCurrentEnvironmentIndex(-1)
{
}


/*====================
  CExpressionEvaluator::RegisterOperator
  ====================*/
void    CExpressionEvaluator::RegisterOperator(IOperator *pOperator)
{
    assert(pOperator != NULL);
    assert(!(pOperator->IsPostUnary() && pOperator->IsBinary()));
    assert(m_mapOperators.find(pOperator->GetSymbolCode()) == m_mapOperators.end());
    m_mapOperators[pOperator->GetSymbolCode()] = pOperator;
}


/*====================
  Eval_Precedence

  Returns the precedence of the specified operator.
 ====================*/
int     Eval_Precedence(TCHAR op)
{
    switch (op)
    {
    case OP_OPEN_PAREN:
    case OP_CLOSE_PAREN:
        return 1;

    case OP_TERMINATOR:
        return 2;

    case OP_OR:
    case OP_AND:
        return 3;

    case OP_EQUALS:
    case OP_NOT_EQUALS:
    case OP_LESS:
    case OP_GREATER:
    case OP_LESS_EQUALS:
    case OP_GREATER_EQUALS:
        return 4;

    case OP_PLUS:
    case OP_MINUS:
        return 5;

    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
        return 6;

    case OP_POWER:
        return 7;

    case OP_NEGATE:
    case OP_NOT:
    case OP_SQRT:
    case OP_SIN:
    case OP_COS:
    case OP_TAN:
    case OP_ASIN:
    case OP_ACOS:
    case OP_ATAN:
    case OP_LOG:
    case OP_LN:
    case OP_CEIL:
    case OP_FLOOR:
    case OP_ROUND:
    case OP_SATURATE:
    case OP_ABS:
        return 8;

    default:
        return -1;
    }
}


/*====================
  Eval_IsUnary
 ====================*/
int     Eval_IsUnary(TCHAR op)
{
    if ((op == OP_NEGATE) || (op == OP_SQRT) || (op == OP_SIN)   ||
        (op == OP_COS)    || (op == OP_TAN)  || (op == OP_ASIN)  ||
        (op == OP_ACOS)   || (op == OP_ATAN) || (op == OP_LOG)   ||
        (op == OP_LN)     || (op == OP_CEIL) || (op == OP_FLOOR) ||
        (op == OP_ROUND)  || (op == OP_ABS)  || (op == OP_NOT) ||
        (op == OP_SATURATE))
        return true;

    return false;
}


/*====================
  Eval_AddValue
 ====================*/
void    Eval_AddValue(float val)
{
    if (current_valid)
    {
        evaluate_error = true;
        return;
    }

    current_val = val;
    current_valid = true;
}


/*====================
  Eval_AddOperator
 ====================*/
void    Eval_AddOperator(TCHAR op)
{
    // bodge for unary negation
    if ((op == OP_MINUS) && (!current_valid))
        op = OP_NEGATE;

    // check validity
    if ((op == OP_PLUS)        || (op == OP_MINUS)          ||
        (op == OP_MUL)         || (op == OP_DIV)            ||
        (op == OP_POWER)       || (op == OP_MOD)            ||
        (op == OP_EQUALS)      || (op == OP_NOT_EQUALS)     ||
        (op == OP_LESS)        || (op == OP_GREATER)        ||
        (op == OP_LESS_EQUALS) || (op == OP_GREATER_EQUALS) ||
        (op == OP_OR)          || (op == OP_AND))
    {
        if (!current_valid)
        {
            evaluate_error = true;
            return;
            }
        }

    // evaluate
    if (op != OP_OPEN_PAREN)
    {
        while ((stack_depth > 0) &&
            ((Eval_Precedence(op) <= Eval_Precedence(operator_stack[stack_depth - 1]) && (!Eval_IsUnary(operator_stack[stack_depth - 1]))) ||
            (Eval_Precedence(op) < Eval_Precedence(operator_stack[stack_depth - 1]) && (Eval_IsUnary(operator_stack[stack_depth - 1])))))
        {

            --stack_depth;

            switch (operator_stack[stack_depth])
            {
            case OP_PLUS:
                current_val = value_stack[stack_depth] + current_val;
                break;

            case OP_MINUS:
                current_val = value_stack[stack_depth] - current_val;
                break;

            case OP_MUL:
                current_val = value_stack[stack_depth] * current_val;
                break;

            case OP_DIV:
                if (current_val != 0)
                    current_val = value_stack[stack_depth] / current_val;
                else
                    current_val = 0;
                break;

            case OP_POWER:
                current_val = pow(value_stack[stack_depth], current_val);
                break;

            case OP_NEGATE:
                current_val = -current_val;
                break;

            case OP_SQRT:
                if (current_val >= 0)
                    current_val = sqrt(current_val);
                else
                    current_val = 0;
                break;

            case OP_SIN:
                current_val = sin(DEG2RAD(current_val));
                break;

            case OP_COS:
                current_val = cos(DEG2RAD(current_val));
                break;

            case OP_TAN:
                current_val = tan(DEG2RAD(current_val));
                break;

            case OP_ASIN:
                if ((current_val >= -1) && (current_val <= 1))
                    current_val = RAD2DEG(asin(current_val));
                else
                    current_val = 0;
                break;

            case OP_ACOS:
                if ((current_val >= -1) && (current_val <= 1))
                    current_val = RAD2DEG(acos(current_val));
                else
                    current_val = 0;
                break;

            case OP_ATAN:
                current_val = RAD2DEG(atan(current_val));
                break;

            case OP_LOG:
                if (current_val > 0)
                    current_val = log10(current_val);
                else
                    current_val = 0;
                break;

            case OP_LN:
                if (current_val > 0)
                    current_val = log(current_val);
                else
                    current_val = 0;
                break;

            case OP_CEIL:
                current_val = ceil(current_val);
                break;

            case OP_FLOOR:
                current_val = floor(current_val);
                break;

            case OP_ROUND:
                current_val = floor(current_val + 0.5f);
                break;

            case OP_SATURATE:
                current_val = CLAMP(current_val, 0.0f, 1.0f);
                break;

            case OP_ABS:
                current_val = fabs(current_val);
                break;

            case OP_MOD:
                if (current_val >= 1)
                    current_val = fmod(value_stack[stack_depth], current_val);
                else
                    current_val = 0;
                break;

            case OP_EQUALS:
                current_val = (value_stack[stack_depth] == current_val);
                break;

            case OP_NOT_EQUALS:
                current_val = (value_stack[stack_depth] != current_val);
                break;

            case OP_LESS:
                current_val = (value_stack[stack_depth] < current_val);
                break;

            case OP_GREATER:
                current_val = (value_stack[stack_depth] > current_val);
                break;

            case OP_LESS_EQUALS:
                current_val = (value_stack[stack_depth] <= current_val);
                break;

            case OP_GREATER_EQUALS:
                current_val = (value_stack[stack_depth] >= current_val);
                break;

            case OP_OR:
                current_val = ((int)value_stack[stack_depth] || (int)current_val);
                break;

            case OP_AND:
                current_val = ((int)value_stack[stack_depth] && (int)current_val);
                break;

            case OP_NOT:
                current_val = !(int)current_val;
                break;

            case OP_OPEN_PAREN:
                if (op == OP_CLOSE_PAREN)
                    return;
                break;
            }
        }
    }

    // push onto the stack
    if (op != OP_CLOSE_PAREN)
    {
        operator_stack[stack_depth] = op;
        value_stack[stack_depth] = current_val;
        ++stack_depth;
        current_val = 0;
        current_valid = false;
    }
    else
    {
        if (stack_depth <= 0)
        evaluate_error = true;
    }
}


/*====================
  Eval_Evaluate
 ====================*/
float   Eval_Evaluate(const tstring &sExpression, bool &bError, EvalLookupFn pfnGetValue)
{
#if 0
    // FIXME: use the string in the function
    TCHAR _equation[1024];
    _TCSNCPY_S(_equation, 1024, sExpression.c_str(), 1023);
    _equation[1023] = 0;
    TCHAR *equation = _equation;
#else
    const TCHAR *equation(sExpression.c_str());
#endif

    TCHAR buf[256];
    float val;
    int i;

    stack_depth = 0;
    current_val = 0;
    current_valid = false;
    evaluate_error = false;

    while ((*equation) && (!evaluate_error))
    {
        // skip whitespace
        while (isspace(*equation))
            ++equation;

        switch (*equation)
        {

        case '+':
            // addition
            Eval_AddOperator(OP_PLUS);
            ++equation;
            break;

        case '-':
            // subtraction
            Eval_AddOperator(OP_MINUS);
            ++equation;
            break;

        case '*':
            // multiplication
            Eval_AddOperator(OP_MUL);
            ++equation;
            break;

        case '/':
            // division
            Eval_AddOperator(OP_DIV);
            ++equation;
            break;

        case '^':
            // rasing to a power (_not_ XOR!)
            Eval_AddOperator(OP_POWER);
            ++equation;
            break;

        case '%':
            // modulus
            Eval_AddOperator(OP_MOD);
            ++equation;
            break;

        case '|':
            // logical or
            Eval_AddOperator(OP_OR);
            ++equation;
            break;

        case '&':
            // logical and
            Eval_AddOperator(OP_AND);
            ++equation;
            break;

        case '=':
            // equality test (requires dual ==)
            if (equation[1] == '=')
            {
                Eval_AddOperator(OP_EQUALS);
                equation += 2;
            }
            else
            {
                evaluate_error = true;
            }
            break;

        case '!':
            // could be inequality test or logical not
            if (equation[1] == '=')
            {
                Eval_AddOperator(OP_NOT_EQUALS);
                equation += 2;
            }
            else
            {
                Eval_AddOperator(OP_NOT);
                ++equation;
            }
            break;

        case '<':
            // could be less than or less / equal test
            if (equation[1] == '=')
            {
                Eval_AddOperator(OP_LESS_EQUALS);
                equation += 2;
            }
            else
            {
                Eval_AddOperator(OP_LESS);
                ++equation;
            }
            break;

        case '>':
            // could be greater than or greater / equal test
            if (equation[1] == '=')
            {
                Eval_AddOperator(OP_GREATER_EQUALS);
                equation += 2;
            }
            else
            {
                Eval_AddOperator(OP_GREATER);
                ++equation;
            }
            break;

        case '(':
            // open bracket
            Eval_AddOperator(OP_OPEN_PAREN);
            ++equation;
            break;

        case ')':
            // close bracket
            Eval_AddOperator(OP_CLOSE_PAREN);
            ++equation;
            break;

        case '0':
            // special case for hex constants (0x prefix)
            if (equation[1] == 'x')
            {
                equation += 2;

                for (i = 0; isxdigit(equation[i]); ++i)
                    buf[i] = equation[i];

                buf[i] = 0;
                equation += i;

                val = float(_tcstol(buf, NULL, 16));
                Eval_AddValue(val);
                break;
            }
            // else fall through

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            // floating point constant
            for (i = 0; isdigit(equation[i]) || (equation[i] == '.'); ++i)
                buf[i] = equation[i];

            buf[i] = 0;
            equation += i;

            val = AtoF(buf);
            Eval_AddValue(val);
            break;

        default:
            // this is a string, could be a variable or function
            for (i = 0; (isalpha(equation[i])) || (equation[i] == '_') || isdigit(equation[i]) || (equation[i] == ':'); ++i)
                buf[i] = equation[i];

            buf[i] = 0;
            equation += i;

            if (_tcscmp(buf, _T("pi")) == 0)
            {
                // pi (built in constant)
                Eval_AddValue(pi);
            }
            else if (_tcscmp(buf, _T("e")) == 0)
            {
                // e (built in constant)
                Eval_AddValue(E);
            }
            else if (_tcscmp(buf, _T("sqrt")) == 0)
            {
                // square root function
                Eval_AddOperator(OP_SQRT);
            }
            else if (_tcscmp(buf, _T("sin")) == 0)
            {
                // sin function
                Eval_AddOperator(OP_SIN);
            }
            else if (_tcscmp(buf, _T("cos")) == 0)
            {
                // cos function
                Eval_AddOperator(OP_COS);
            }
            else if (_tcscmp(buf, _T("tan")) == 0)
            {
                // tan function
                Eval_AddOperator(OP_TAN);
            }
            else if (_tcscmp(buf, _T("asin")) == 0)
            {
                // inverse sin function
                Eval_AddOperator(OP_ASIN);
            }
            else if (_tcscmp(buf, _T("acos")) == 0)
            {
                // inverse cos function
                Eval_AddOperator(OP_ACOS);
            }
            else if (_tcscmp(buf, _T("atan")) == 0)
            {
                // inverse tan function
                Eval_AddOperator(OP_ATAN);
            }
            else if (_tcscmp(buf, _T("log")) == 0)
            {
                // base 10 logarithm function
                Eval_AddOperator(OP_LOG);
            }
            else if (_tcscmp(buf, _T("ln")) == 0)
            {
                //natural logarithm function
                Eval_AddOperator(OP_LN);
            }
            else if (_tcscmp(buf, _T("ceil")) == 0)
            {
                //round upwards function
                Eval_AddOperator(OP_CEIL);
            }
            else if (_tcscmp(buf, _T("floor")) == 0)
            {
                //round downwards function
                Eval_AddOperator(OP_FLOOR);
            }
            else if (_tcscmp(buf, _T("round")) == 0)
            {
                // round to nearest integer function
                Eval_AddOperator(OP_ROUND);
            }
            else if (_tcscmp(buf, _T("saturate")) == 0)
            {
                // round to nearest integer function
                Eval_AddOperator(OP_SATURATE);
            }
            else if (_tcscmp(buf, _T("abs")) == 0)
            {
                // absolute value function
                Eval_AddOperator(OP_ABS);
            }
            else if (_tcscmp(buf, _T("rand")) == 0)
            {
                // random number between 0 and 1
                Eval_AddValue((rand() & RAND_MAX) / float(RAND_MAX));
            }
            else
            {
                // user - supplied callback for looking up variables
                if ((buf[0]) && (pfnGetValue))
                {
                    Eval_AddValue(pfnGetValue(buf));
                }
                else
                {
                    bError = true;
                    return 0;
                }
            }
            break;
        }
    }

    if ((evaluate_error) || (!current_valid))
    {
        bError = true;
        return 0;
    }

    // force a stack flush
    Eval_AddOperator(OP_TERMINATOR);

    if (stack_depth != 1)
    {
        bError = true;
        return 0;
    }

    bError = false;

    return value_stack[0];
}


/*====================
  CEvaluatorToken::~CEvaluatorToken
  ====================*/
CEvaluatorToken::~CEvaluatorToken()
{
    SAFE_DELETE(m_pToken);
}


/*====================
  CEvaluatorToken::CEvaluatorToken
  ====================*/
CEvaluatorToken::CEvaluatorToken(const tstring &sLiteral) :
m_bIsOperator(false),
m_pOperator(NULL),
m_pToken(K2_NEW(ctx_Script,  CUIScriptToken)(NULL, sLiteral))
{}


/*====================
  CEvaluatorToken::AddArgumentChain
  ====================*/
void    CEvaluatorToken::AddArgumentChain(CEvaluatorToken *pToken)
{
    if (m_pToken == NULL)
        return;

    CUIScriptToken *pArgToken(pToken->m_pToken);
    pToken->m_pToken = NULL;
    while (pArgToken != NULL)
    {
        m_pToken->AddArgument(pArgToken);
        pArgToken = pArgToken->GetNextLink();
    }
}


/*====================
  CEvaluatorToken::AddArgumentChain
  ====================*/
void    CEvaluatorToken::AddArgument(const tstring &sValue)
{
    if (m_pToken == NULL)
        return;

    m_pToken->AddArgument(sValue);
}


/*====================
  CEvaluatorToken::LinkArgument
  ====================*/
void    CEvaluatorToken::LinkArgument(CEvaluatorToken *pToken)
{
    if (m_pToken == NULL)
        return;

    m_pToken->LinkArgument(pToken->m_pToken);
    pToken->m_pToken = NULL;
}


/*====================
  CEvaluatorToken::GetValue
  ====================*/
const tstring&  CEvaluatorToken::GetValue() const
{
    if (m_pToken == NULL)
        return TSNULL;

    if (m_bIsOperator)
    {
        Console.Err << _T("Operator referenced as symbol") << newl;
        return TSNULL;
    }

    return m_pToken->Evaluate();
}


/*====================
  CEvaluatorToken::SetValue
  ====================*/
void    CEvaluatorToken::SetValue(const tstring &sValue)
{
    if (m_pToken == NULL)
        return;

    if (m_bIsOperator)
    {
        Console.Err << _T("Operator referenced as symbol") << newl;
        return;
    }

    m_pToken->SetValue(sValue);
}


/*====================
  CEvaluatorToken::AssignValue
  ====================*/
void    CEvaluatorToken::AssignValue(const tstring &sValue)
{
    if (m_pToken == NULL)
        return;

    if (m_bIsOperator)
    {
        Console.Err << _T("Operator referenced as symbol") << newl;
        return;
    }

    m_pToken->Assign(sValue);
}


/*====================
  CEvaluatorToken::AssignToken
  ====================*/
void    CEvaluatorToken::AssignToken(CUIScriptToken *pToken)
{
    if (pToken == NULL)
        return;

    SAFE_DELETE(m_pToken);
    m_pToken = pToken;
}


/*====================
  CExpressionEvaluator::PushEnvironment
  ====================*/
void    CExpressionEvaluator::PushEnvironment()
{
    //PROFILE("CExpressionEvaluator::PushEnvironment");

    ++m_iCurrentEnvironmentIndex;
    assert(m_iCurrentEnvironmentIndex >= 0);
    if (int(m_vEnvironments.size()) <= m_iCurrentEnvironmentIndex)
    {
#if 0
        m_vEnvironments.resize(m_iCurrentEnvironmentIndex + 1);
#else
        K2System.Error(_T("UIScript environment stack overflow"));
#endif
    }
    else
        m_vEnvironments[m_iCurrentEnvironmentIndex].Clear();
}


/*====================
  CExpressionEvaluator::PopEnvironment
  ====================*/
void    CExpressionEvaluator::PopEnvironment()
{
    //PROFILE("CExpressionEvaluator::PopEnvironment");
    --m_iCurrentEnvironmentIndex;
}


/*====================
  CExpressionEvaluator::GetToken
  ====================*/
CEvaluatorToken*    CExpressionEvaluator::GetToken(const tstring &sSymbol, CEvaluatorToken *pResultToken)
{
    //PROFILE("CExpressionEvaluator::GetToken");

    CUIScriptToken *pNewScriptToken(NULL);
    IWidget *pActiveWidget(GetCurrentEnvironment().m_pActiveWidget);

    // Number
    if (!sSymbol.empty() && sSymbol[0] <= _T('9') && sSymbol[0] >= _T('0'))
    {
        pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, sSymbol);
    }
    // Parameter
    else if (sSymbol.length() >= 5 &&
        sSymbol[0] == _T('p') &&
        sSymbol[1] == _T('a') &&
        sSymbol[2] == _T('r') &&
        sSymbol[3] == _T('a') &&
        sSymbol[4] == _T('m'))
    {
        if (pActiveWidget == NULL)
        {
            SetError(QuoteStr(GetCurrentEnvironment().m_sExpression) + newl + _T("'param' is not valid because there is no active widget"));
            return NULL;
        }

        const tsvector &vEventParams(GetCurrentEnvironment().m_vParams);

        size_t zLength(sSymbol.length());
        uint uiParam;
        if(zLength == 6)
            uiParam = sSymbol[5] - _T('0');
        else if (zLength == 7)
            uiParam = (sSymbol[5] - _T('0')) * 10 + sSymbol[6] - _T('0');
        else if (zLength == 8)
            uiParam = (sSymbol[5] - _T('0')) * 100 + (sSymbol[6] - _T('0')) * 10 + sSymbol[7] - _T('0');
        else
            uiParam = 0;

        if (vEventParams.size() <= uiParam)
        {
            SetError(QuoteStr(GetCurrentEnvironment().m_sExpression) + newl + SingleQuoteStr(sSymbol) + _T(" is not valid for the active widget"));
            return NULL;
        }

        pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, vEventParams[uiParam]);
    }
    // Boolean
    else if (TStringCompare(sSymbol, _T("false")) == 0)
    {
        pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, _CTS("0"));
    }
    else if (sSymbol.length() == 4)
    {
        if (TStringCompare(sSymbol, _T("true")) == 0)
        {
            pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, _CTS("1"));
        }
        // Active widget value
        else if (TStringCompare(sSymbol, _T("this")) == 0)
        {
            pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, pActiveWidget->GetValue());
        }
        // Active widget name
        else if (TStringCompare(sSymbol, _T("name")) == 0)
        {
            pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, pActiveWidget->GetName());
        }
        else if (TStringCompare(sSymbol, _T("data")) == 0)
        {
            if (pActiveWidget == NULL || pActiveWidget->GetType() != WIDGET_TABLE)
            {
                SetError(_T("Cannot get 'data' for widget ") + SingleQuoteStr(pActiveWidget->GetName()) + _T(" because it is not a table"));
                return NULL;
            }

            pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, static_cast<CTable*>(pActiveWidget)->GetEventData());
        }
    }
    // Table information
    else if (TStringCompare(sSymbol, _T("dataid")) == 0)
    {
        if (pActiveWidget == NULL || pActiveWidget->GetType() != WIDGET_TABLE)
        {
            SetError(_T("Cannot get 'dataid' for widget ") + SingleQuoteStr(pActiveWidget->GetName()) + _T(" because it is not a table"));
            return NULL;
        }

        pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, static_cast<CTable*>(pActiveWidget)->GetEventDataID());
    }
    else if (sSymbol.length() == 3)
    {
        if (TStringCompare(sSymbol, _T("row")) == 0)
        {
            if (pActiveWidget == NULL || pActiveWidget->GetType() != WIDGET_TABLE)
            {
                SetError(_T("Cannot get 'row' for widget ") + SingleQuoteStr(pActiveWidget->GetName()) + _T(" because it is not a table"));
                return NULL;
            }

            pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, XtoA(static_cast<CTable*>(pActiveWidget)->GetEventRow()));
        }
        else if (TStringCompare(sSymbol, _T("col")) == 0)
        {
            if (pActiveWidget == NULL || pActiveWidget->GetType() != WIDGET_TABLE)
            {
                SetError(_T("Cannot get 'col' for widget ") + SingleQuoteStr(pActiveWidget->GetName()) + _T(" because it is not a table"));
                return NULL;
            }

            pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, XtoA(static_cast<CTable*>(pActiveWidget)->GetEventCol()));
        }
        else if (TStringCompare(sSymbol, _T("and")) == 0)
        {
            PushOperator(&g_OperatorLogicalAnd);
            return NULL;
        }
    }
    // Alternative operators
    else if (sSymbol.length() == 2)
    {
        if (TStringCompare(sSymbol, _T("gt")) == 0)
        {
            PushOperator(&g_OperatorGreaterThan);
            return NULL;
        }
        else if (TStringCompare(sSymbol, _T("lt")) == 0)
        {
            PushOperator(&g_OperatorLessThan);
            return NULL;
        }
        else if (TStringCompare(sSymbol, _T("ge")) == 0)
        {
            PushOperator(&g_OperatorGreaterThanEqual);
            return NULL;
        }
        else if (TStringCompare(sSymbol, _T("le")) == 0)
        {
            PushOperator(&g_OperatorLessThanEqual);
            return NULL;
        }
        else if (TStringCompare(sSymbol, _T("or")) == 0)
        {
            PushOperator(&g_OperatorLogicalOr);
            return NULL;
        }
    }

    // Command
    if (pNewScriptToken == NULL)
    {
        CUICmd *pUICmd(pUICmdRegistry->GetUICmd(sSymbol));
        if (pUICmd != NULL)
            pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, pUICmd);
    }

    // Widget name
    if (pNewScriptToken == NULL)
    {
        CInterface *pInterface(UIManager.GetActiveInterface());
        if (pInterface != NULL)
        {
            IWidget *pWidget(pInterface->GetWidget(sSymbol));
            if (pWidget != NULL)
                pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, pWidget->GetValue());
        }
    }

    // Cvar
    if (pNewScriptToken == NULL)
    {
        ICvar *pCvar(ConsoleRegistry.GetCvar(sSymbol));
        if (pCvar != NULL)
            pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, pCvar);
    }

    // Trigger
    if (pNewScriptToken == NULL)
    {
        CUITrigger *pUITrigger(CUITriggerRegistry::GetInstance()->GetUITrigger(sSymbol));
        if (pUITrigger != NULL)
            pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, pUITrigger);
    }

    if (pNewScriptToken == NULL)
    {
        Console.Warn << QuoteStr(GetCurrentEnvironment().m_sExpression) << newl << _T("Could not find a value for symbol: ") << sSymbol << newl;
        pNewScriptToken = K2_NEW(ctx_Script,  CUIScriptToken)(pActiveWidget, TSNULL);
    }

    if (pResultToken != NULL)
    {
        pResultToken->AssignToken(pNewScriptToken);
        return pResultToken;
    }

    return K2_NEW(ctx_Script,  CEvaluatorToken)(pNewScriptToken);
}


/*====================
  CExpressionEvaluator::PushSymbol
  ====================*/
void    CExpressionEvaluator::PushSymbol(const tstring &sValue)
{
    //PROFILE("CExpressionEvaluator::PushSymbol");

    CEvaluatorToken *pNewToken(GetToken(sValue));
    PushToken(pNewToken);
}


/*====================
  CExpressionEvaluator::PushLiteral
  ====================*/
void    CExpressionEvaluator::PushLiteral(const tstring &sValue)
{
    //PROFILE("CExpressionEvaluator::PushLiteral");

    CUIScriptToken *pNewToken(K2_NEW(ctx_Script,  CUIScriptToken)(GetCurrentEnvironment().m_pActiveWidget, sValue));
    PushToken(K2_NEW(ctx_Script,  CEvaluatorToken)(pNewToken));
}


/*====================
  CExpressionEvaluator::PushOperator
  ====================*/
void    CExpressionEvaluator::PushOperator(IOperator *pOperator)
{
    //PROFILE("CExpressionEvaluator::PushOperator");

    if (pOperator == NULL)
    {
        SetError(_T("Invalid operator"));
        return;
    }

    Environment &environment(GetCurrentEnvironment());

    EOperatorType eUsage(OP_NULL);
    if (!pOperator->IsUnary() && (pOperator->IsBinary() || pOperator->IsPostUnary()))
    {
        
        if (pOperator->IsBinary() && !pOperator->IsPostUnary())
            eUsage = OP_BINARY;
        else if (!pOperator->IsBinary() && pOperator->IsPostUnary())
            eUsage = OP_POST_UNARY;

        if (!environment.m_bLastTokenWasValue)
        {
            SetError(QuoteStr(environment.m_sExpression) + newl + _T("Missing symbol before operator: ") + pOperator->GetSymbol());
            return;
        }
    }
    else if (pOperator->IsUnary() && (!pOperator->IsBinary() && !pOperator->IsPostUnary()))
    {
        eUsage = OP_UNARY;
        if (environment.m_bLastTokenWasValue)
        {
            SetError(QuoteStr(environment.m_sExpression) + newl + _T("Missing operator before symbol: ") + XtoA(environment.m_stackSymbols.back()->GetValue()));
            return;
        }
    }
    else if (pOperator->IsUnary() && (pOperator->IsBinary() || pOperator->IsPostUnary()))
    {
        if (environment.m_bLastTokenWasValue)
        {
            if (pOperator->IsBinary() && !pOperator->IsPostUnary())
                eUsage = OP_BINARY;
            else if (!pOperator->IsBinary() && pOperator->IsPostUnary())
                eUsage = OP_POST_UNARY;
        }
        else
        {
            eUsage = OP_UNARY;
        }
    }

    // Push onto the stack
    //if (eUsage != OP_NULL)
    {
        environment.m_stackOperators.push_back(COpCode(eUsage, pOperator));
        if (eUsage != OP_POST_UNARY)
            environment.m_bLastTokenWasValue = false;
    }

    EvaluateStack();
}


/*====================
  CExpressionEvaluator::PushToken
  ====================*/
void    CExpressionEvaluator::PushToken(CEvaluatorToken *pToken)
{
    //PROFILE("CExpressionEvaluator::PushToken");

    if (pToken == NULL)
        return;

    Environment &environment(GetCurrentEnvironment());
    if (pToken->IsOperator())
    {
        PushOperator(pToken->GetOperator());
        SAFE_DELETE(pToken);
    }
    else
    {
        if (environment.m_bLastTokenWasValue)
        {
            SetError(QuoteStr(environment.m_sExpression) + newl + _T("Missing operator before value: ") + pToken->GetValue());
            return;
        }

        environment.m_stackSymbols.push_back(pToken);
        environment.m_bLastTokenWasValue = true;
    }
}


/*====================
  CExpressionEvaluator::EvaluateStack
  ====================*/
void    CExpressionEvaluator::EvaluateStack()
{
    //PROFILE("CExpressionEvaluator::EvaluateStack");

    Environment &environment(GetCurrentEnvironment());

    vector<COpCode> &stackOps(environment.m_stackOperators);
    if (stackOps.empty())
        return;

    COpCode opCurrent(stackOps.back());
    stackOps.pop_back();

    while (!stackOps.empty())
    {
        COpCode opPrev(stackOps.back());

        if (opPrev.IsUnary() && opCurrent.GetPrecedence() <= opPrev.GetPrecedence())
            break;
        if (opPrev.IsBinary() && opCurrent.GetPrecedence() < opPrev.GetPrecedence())
            break;
        if (opPrev.IsPostUnary() && opCurrent.GetPrecedence() < opPrev.GetPrecedence())
            break;
        if (opPrev.IsPaired() && opCurrent.GetSymbolCode() != opPrev.GetCompanionCode())
            break;

        stackOps.pop_back();

        CEvaluatorToken* pA(environment.m_stackSymbols.back());
        environment.m_stackSymbols.pop_back();

        if (opPrev.IsUnary() || opPrev.IsPostUnary())
        {
            opPrev.Execute(pA);
            environment.m_stackSymbols.push_back(pA);
        }
        else if (opPrev.IsBinary())
        {
            if (environment.m_stackSymbols.empty())
            {
                SetError(_T("Value stack underflow"));
                break;
            }

            CEvaluatorToken* pB(environment.m_stackSymbols.back());
            environment.m_stackSymbols.pop_back();
            opPrev.Execute(pB, pA);
            SAFE_DELETE(pA);
            environment.m_stackSymbols.push_back(pB);
        }

        if (opPrev.IsPaired() && opCurrent.GetSymbolCode() == opPrev.GetCompanionCode())
        {
            opCurrent.Invalidate();
            break;
        }
    }

    if (opCurrent.GetType() != OP_NULL)
        stackOps.push_back(opCurrent);
}


/*====================
  CExpressionEvaluator::Evaluate
  ====================*/
tstring CExpressionEvaluator::Evaluate(const tstring &sExpression, IWidget *pActiveWidget, const tsvector &vParams)
{
    PROFILE("CExpressionEvaluator::Evaluate");

    if (sExpression.empty())
        return _T("");

    try
    {
        PushEnvironment();
        Environment &environment(GetCurrentEnvironment());
        environment.m_pActiveWidget = pActiveWidget;
        environment.m_sExpression = sExpression;
        environment.m_vParams = vParams;

        for (size_t z(0); z < sExpression.length(); )
        {
            { PROFILE("Whitespace");
            z = SkipSpaces(sExpression, z);
            if (z == tstring::npos)
                break;
            } // PROFILE("Whitespace")

            { PROFILE("Block comments");
            // Block comment
            if (sExpression[z] == _T('#') && (z + 1) < sExpression.length() && sExpression[z + 1] == _T('#')) {
                z += 2;
                while (z < sExpression.length()) {
                    if (sExpression[z] == '#') {
                        if ((z + 1) < sExpression.length() && sExpression[z + 1] == '#') {
                            z += 2;
                            break;
                        }
                    }
                    z++;
                }
                continue;
            }
            } // PROFILE("Block comments")

            { PROFILE("String literals");
            // String literals
            if (sExpression[z] == _T('\''))
            {
                ++z;
                size_t zStart(z);
                bool bEscape(false);
                tstring sLiteral;
                while (z < sExpression.length())
                {
                    if (!bEscape && sExpression[z] == _T('\\'))
                    {
                        sLiteral += sExpression.substr(zStart, z - zStart);
                        zStart = ++z;
                        bEscape = true;
                        continue;
                    }

                    if (!bEscape && sExpression[z] == _T('\''))
                    {
                        ++z;
                        break;
                    }

                    bEscape = false;
                    ++z;
                }

                sLiteral += sExpression.substr(zStart, z - zStart - 1);
                PushLiteral(sLiteral);
                continue;
            }
            } // PROFILE("String literals")

            { PROFILE("Command seperation");
            // Command seperation
            if (sExpression[z] == _T(';'))
            {
                PushOperator(&g_OperatorNull);
                if (!environment.m_stackSymbols.empty())
                    environment.m_stackSymbols.back()->GetValue();
                while (!environment.m_stackOperators.empty())
                    environment.m_stackOperators.pop_back();
                while (!environment.m_stackSymbols.empty())
                {
                    SAFE_DELETE(environment.m_stackSymbols.back());
                    environment.m_stackSymbols.pop_back();
                }
                environment.m_bLastTokenWasValue = false;
                ++z;
                continue;
            }
            } // PROFILE("Command seperation")
            
            { PROFILE("Operators");
            // Operators
            bool bFoundOperator(false);
            for (int i(MAX_OPERATOR_LENGTH); i >= 1; --i)
            {
                OperatorMap_it itOperator(m_mapOperators.find(IOperator::GetOperatorCode(sExpression.substr(z, i))));
                if (itOperator != m_mapOperators.end())
                {
                    PushOperator(itOperator->second);
                    if (environment.HasError())
                        EX_ERROR(environment.GetErrorMessage());
                    z += i;
                    bFoundOperator = true;
                    break;
                }
            }
            if (bFoundOperator)
                continue;
            } // PROFILE("Operators")
            

            { PROFILE("Symbols");
            // Symbols
            size_t zSymbolStart(z);
            while (
                (sExpression[z] >= _T('a') && sExpression[z] <= _T('z')) ||
                (sExpression[z] >= _T('A') && sExpression[z] <= _T('Z')) ||
                (sExpression[z] >= _T('0') && sExpression[z] <= _T('9')) ||
                sExpression[z] == _T('.') ||
                sExpression[z] == _T('_')
                )
                ++z;
            if (zSymbolStart < z)
            {
                PushSymbol(sExpression.substr(zSymbolStart, z - zSymbolStart));
                if (HasError())
                    EX_ERROR(GetErrorMessage());
                continue;
            }
            } // PROFILE("Symbols")

            // Junk
            EX_ERROR(_T("Syntax error: ") + sExpression.substr(0, z) + sRed + sExpression.substr(z, 1) + sNoColor + sExpression.substr(z + 1));
        }

        tstring sReturn;
        if (m_vEnvironments[m_iCurrentEnvironmentIndex].m_stackSymbols.empty())
            sReturn = TSNULL;
        else
            sReturn = m_vEnvironments[m_iCurrentEnvironmentIndex].m_stackSymbols.back()->GetValue();
        PopEnvironment();
        return sReturn;
    }
    catch (CException &ex)
    {
        PopEnvironment();
        ex.Process(_T("CExpressionEvaluator()::Evaluate() - "), NO_THROW);
        return TSNULL;
    }
}


/*====================
  CExpressionEvaluator::Parse
  ====================*/
bool    CExpressionEvaluator::Parse(const tstring &sExpression, EvalLookupFn pfnGetValue, ParsedScript &vTokens)
{
    return true;
}


CMD(eval)
{
    tstring sExpression(ConcatinateArgs(vArgList));
    Console << ExpressionEvaluator.Evaluate(sExpression, NULL, VSNULL) << newl;
    return true;
}


CMD(ProfileEval)
{
    CInterface *pInterface(UIManager.GetActiveInterface());
    IWidget *pWidget(pInterface->GetWidget(_T("__WIDGET_0002")));
    tsvector vParams;
    vParams.push_back(_T("123.456"));
    pWidget->SetEventParam(vParams);
    uint uiCount(vArgList.size() > 0 ? AtoI(vArgList[0]) : 1000);

    PROFILE_EX("Evaluator", PROFILE_EVALUATOR);

    LONGLONG llStartTime(K2System.GetTicks());
    for (uint ui(0); ui < uiCount; ++ui)
        UIScript.Evaluate(pWidget, _T("SetText(Round(param) # 'abc');"));
    LONGLONG llEndTime(K2System.GetTicks());

    Console << _T("Evaluation took ") << XtoA(float(llEndTime - llStartTime) / K2System.GetFrequency() * 1000.0f, 0, 0, 3) << _T(" ms") << newl;
    return true;
}
