// (C)2009 S2 Games
// c_glslpreprocessor.cpp
//
// Replacement GLSL preprocessor
// Handles the following preprocessor directives:
//   define
//   undef
//   if
//   ifdef
//   ifndef
//   else
//   elif
//   endif
// The remaining ones are left for the GLSL driver preprocessor
//
// Bugs/unimplemented:
//   defined preprocessor operator not implemented
//   cannot define macros with arguments
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_glslpreprocessor.h"

CVAR_BOOL(vid_debugGLSLShaderPreprocessor, false);
//=============================================================================

inline bool IsWhiteSpace(char c)
{
    if (c == ' ' || c == '\t')
        return true;
    return false;
}

inline bool IsAlpha(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return true;
    return false;
}

inline bool IsDigit(char c)
{
    if (c >= '0' && c <= '9')
        return true;
    return false;
}

string CGLSLPreprocessor::GetIdentifier(const string &sLine, string::size_type &zPos)
{
    if (zPos >= sLine.length())
        return "";
    
    if (!IsAlpha(sLine[zPos]) && sLine[zPos] != '_')
        EX_ERROR(_T("Invalid identifier"));
    
    string sIdentifier; 
    while (zPos < sLine.length() && (IsAlpha(sLine[zPos]) || IsDigit(sLine[zPos]) || sLine[zPos] == '_'))
        sIdentifier += sLine[zPos++];
    
    return sIdentifier;
}

string CGLSLPreprocessor::DoSubstitutions(const string &sLine, string::size_type zPos, bool bMissingIsError)
{
    string sReturn;
    
    while (zPos < sLine.length())
    {
        if (IsAlpha(sLine[zPos]) || sLine[zPos] == '_')
        {
            string sVar(GetIdentifier(sLine, zPos));
            map<string,string>::iterator findIt(m_mapDefines.find(sVar));
            if (findIt != m_mapDefines.end())
            {
                sReturn += findIt->second;
            }
            else if (bMissingIsError)
            {
                EX_ERROR(StringToTString(sVar) + _TS(" is not defined"));
            }
            else
            {
                sReturn += sVar;
            }
        }
        else
        {
            sReturn += sLine[zPos++];
        }
    }
    
    return sReturn;
}

string GetNextToken(const string& sExpression, string::size_type &zPos)
{
    if ((zPos = sExpression.find_first_not_of(" ", zPos)) == string::npos)
        return "";  
    
    string sToken;
    if (IsDigit(sExpression[zPos]))
    {
        while (zPos < sExpression.length() && IsDigit(sExpression[zPos]))
            sToken += sExpression[zPos++];
    }
    else
    {
        sToken = sExpression[zPos++];
        if (zPos < sExpression.length())
        {
            switch (sExpression[zPos-1])
            {
                case '<':
                    if (sExpression[zPos] == '<')
                    {
                        zPos++;
                        return "["; // <<
                    }
                    else if (sExpression[zPos] == '=')
                    {
                        zPos++;
                        return "{"; // <=
                    }
                    break;
                case '>':
                    if (sExpression[zPos] == '>')
                    {
                        zPos++;
                        return "]"; // >>
                    }
                    else if (sExpression[zPos] == '=')
                    {
                        zPos++;
                        return "}"; // >=
                    }
                    break;
                case '=':
                    if (sExpression[zPos] == '=') // = is used for ==
                        zPos++;
                    break;
                case '!':
                    if (sExpression[zPos] == '=')
                    {
                        zPos++;
                        return "#"; // !=
                    }
                    break;
                case '|':
                    if (sExpression[zPos] == '|')
                    {
                        zPos++;
                        return "o"; // ||
                    }
                    break;
                case '&':
                    if (sExpression[zPos] == '&')
                    {
                        zPos++;
                        return "a"; // &&
                    }
                    break;
            }
        }
    }
    return sToken;
}

bool OperatorLeftAssociative(char cOp)
{
    switch (cOp)
    {
        case ',':
        case '.':
        case '~':
        case '!':
            return false;
        default:
            return true;
    }
}

int GetOperatorPrecedence(char cOp)
{
    switch (cOp)
    {
        case ',':
        case '.':
        case '~':
        case '!':
            return 11;
        case '*':
        case '/':
        case '%':
            return 10;
        case '+':
        case '-':
            return 9;
        case '[':
        case ']':
            return 8;
        case '<':
        case '>':
        case '{':
        case '}':
            return 7;
        case '=':
        case '#':
            return 6;
        case '&':
            return 5;
        case '^':
            return 4;
        case '|':
            return 3;
        case 'a':
            return 2;
        case 'o':
            return 1;
        case '(':
            return -1;
        default:
            return 0;
    }
}

void CGLSLPreprocessor::PerformOperation(char cOp)
{
    if (m_viValue.size() < 1)
        EX_ERROR(_T("Insufficient operands\n"));
    
    switch (cOp)
    {
    // unary operations
#define OP(x1,x2) case x1: m_viValue.back() = x2 m_viValue.back(); return;
        OP(',',+);
        OP('.',-);
        OP('~',~);
        OP('!',!);
#undef OP
    }
    
    if (m_viValue.size() < 2)
        EX_ERROR(_T("Insufficient operands\n"));
    
    int iTemp(m_viValue.back());
    m_viValue.pop_back();
    
    switch (cOp)
    {
    // arithmetic & bitwise operations
#define OP(x1,x2) case x1: m_viValue.back() x2##= iTemp; break;
        OP('*',*);
        OP('/',/);
        OP('%',%);
        OP('+',+);
        OP('-',-);
        OP(']',>>);
        OP('[',<<);
        OP('&',&);
        OP('^',^);
        OP('|',|);
#undef OP
    // logical operations
#define OP(x1,x2) case x1: m_viValue.back() = (m_viValue.back() x2 iTemp ? 1 : 0); break;
        OP('<',<);
        OP('{',<=);
        OP('>',>);
        OP('}',>=);
        OP('=',==);
        OP('#',!=);
        OP('a',&&);
        OP('o',||);
#undef OP
    }
}

void CGLSLPreprocessor::AddOp(char cOp)
{
    if (OperatorLeftAssociative(cOp))
    {
        while (!m_vcOp.empty() && (GetOperatorPrecedence(cOp) <= GetOperatorPrecedence(m_vcOp.back())))
        {
            PerformOperation(m_vcOp.back());
            m_vcOp.pop_back();
        }
    }
    else
    {
        while (!m_vcOp.empty() && (GetOperatorPrecedence(cOp) < GetOperatorPrecedence(m_vcOp.back())))
        {
            PerformOperation(m_vcOp.back());
            m_vcOp.pop_back();
        }
    }
    
    m_vcOp.push_back(cOp);
}

bool CGLSLPreprocessor::Evaluate(const string &sExpression)
{
    m_vcOp.clear();
    m_viValue.clear();
    bool bOperator(true);
    
    string::size_type zPos(0);
    string sToken;
    while (!(sToken = GetNextToken(sExpression, zPos)).empty())
    {
        switch (sToken[0])
        {
            case '(':
            {
                m_vcOp.push_back(sToken[0]);
            }
            break;
            
            case ')':
            {
                while (!m_vcOp.empty() && m_vcOp.back() != '(')
                {
                    PerformOperation(m_vcOp.back());
                    m_vcOp.pop_back();
                }
                if (m_vcOp.empty())
                    EX_ERROR(_T("Mismatched parenthesis in expression"));
                m_vcOp.pop_back();
            }
            break;
            
            case '~':
            case '!':
            // defined() operator
            {
                if (!bOperator)
                    EX_ERROR(_T("Invalid use of unary operator"));
                
                AddOp(sToken[0]);
            }
            break;
            
            case '+':
            case '-':
            {
                char cOp(sToken[0]);
                if (bOperator)
                    cOp++; // transform '+' to ',' and '-' to '.', used to represent the unary versions
                else
                    bOperator = true;
                
                AddOp(cOp);
            }
            break;
            
            case '*':
            case '/':
            case '%':
            case '[':
            case ']':
            case '<':
            case '>':
            case '{':
            case '}':
            case '=':
            case '#':
            case '&':
            case '^':
            case '|':
            case 'a':
            case 'o':
            {
                if (bOperator)
                    EX_ERROR(_T("Insufficient operands"));
                
                bOperator = true;
                
                AddOp(sToken[0]);
            }
            break;
            
            default:
            {
                if (!bOperator)
                    EX_ERROR(_T("Missing operator"));
                bOperator = false;
                
                if (IsDigit(sToken[0]))
                {
                    m_viValue.push_back(strtol(sToken.c_str(), nullptr, 0));
                }
            }
        }
    }
    
    while (!m_vcOp.empty())
    {
        if (m_vcOp.back() == '(')
            EX_ERROR(_T("Mismatched parenthesis in expression"));
        PerformOperation(m_vcOp.back());
        m_vcOp.pop_back();
    }
    
    if (m_viValue.size() > 1)
        Console.Warn << _T("CGLSLPreprocessor::Evaluate() - ") << XtoA(m_iLineNum) << _T(": excess values left on stack") << newl;
    if (m_viValue.empty())
        EX_ERROR(_T("Empty expression"));
    
    return m_viValue.back() != 0;
}

void CGLSLPreprocessor::ProcessDirective(const string &sLine)
{
    string::size_type zPos(sLine.find_first_not_of(" ", 1));
    
    // empty directive
    if (zPos == string::npos)
        return;
    
    string sDirective(GetIdentifier(sLine, zPos));
    
    if (sDirective.compare("define") == 0)
    {
        if ((zPos = sLine.find_first_not_of(" ", zPos)) == string::npos)
            EX_ERROR(_T("Error in #define expression"));
        
        string sName(GetIdentifier(sLine, zPos));
        
        if (sName.compare(0, 2, "__") == 0 || sName.compare(0, 3, "GL_") == 0)
        {
            Console.Warn << _T("CGLSLPreprocessor::ProcessDirective - ") << XtoA(m_iLineNum) << _T(": Ignoring #define with reserved macro name: ") << sName << newl;
            return;
        }
        
        string sValue;
        if ((zPos = sLine.find_first_not_of(" ", zPos)) != string::npos)
        {
            sValue = sLine.substr(zPos);
        }
        
        map<string,string>::iterator findIt(m_mapDefines.find(sName));
        if (findIt != m_mapDefines.end())
            EX_ERROR(_TS("Macro ") + StringToTString(sName) + _TS(" is already defined")); 
        
        m_mapDefines[sName] = sValue;
    }
    else if (sDirective.compare("undef") == 0)
    {
        if ((zPos = sLine.find_first_not_of(" ", zPos)) == string::npos)
            EX_ERROR(_T("Error in #undef expression"));
        
        string sName(GetIdentifier(sLine, zPos));
        
        if (sName.compare(0, 2, "__") == 0 || sName.compare(0, 3, "GL_") == 0)
        {
            Console.Warn << _T("CGLSLPreprocessor::ProcessDirective - ") << XtoA(m_iLineNum) << _T(": Ignoring #undef with reserved macro name: ") << sName << newl;
            return;
        }
        
        map<string,string>::iterator findIt(m_mapDefines.find(sName));
        if (findIt == m_mapDefines.end())
        {
            Console.Warn << _T("CGLSLPreprocessor::ProcessDirective - ") << XtoA(m_iLineNum) << _T(": macro '") << sName << _T(" is not defined") << newl;
            return;
        }
        
        m_mapDefines.erase(findIt);
    }
    else if (sDirective.compare("if") == 0)
    {
        if ((zPos = sLine.find_first_not_of(" ", zPos)) == string::npos)
            EX_ERROR(_T("Error in #if expression"));
        
        string sExpression(DoSubstitutions(sLine, zPos, true));
        
        m_cBT.If(Evaluate(sExpression));
    }
    else if (sDirective.compare("ifdef") == 0)
    {
        if ((zPos = sLine.find_first_not_of(" ", zPos)) == string::npos)
            EX_ERROR(_T("Error in #ifdef expression"));
        
        string sName(GetIdentifier(sLine, zPos));
        
        map<string,string>::iterator findIt(m_mapDefines.find(sName));
        m_cBT.If(findIt != m_mapDefines.end());
    }
    else if (sDirective.compare("ifndef") == 0)
    {
        if ((zPos = sLine.find_first_not_of(" ", zPos)) == string::npos)
            EX_ERROR(_T("Error in #ifndef expression"));
        
        string sName(GetIdentifier(sLine, zPos));
        
        map<string,string>::iterator findIt(m_mapDefines.find(sName));
        m_cBT.If(findIt == m_mapDefines.end());
    }
    else if (sDirective.compare("else") == 0)
    {
        m_cBT.Else();
    }
    else if (sDirective.compare("elif") == 0)
    {
        if ((zPos = sLine.find_first_not_of(" ", zPos)) == string::npos)
            EX_ERROR(_T("Error in #elif expression"));
        
        string sExpression(DoSubstitutions(sLine, zPos, true));
        
        m_cBT.Elif(Evaluate(sExpression));
    }
    else if (sDirective.compare("endif") == 0)
    {
        m_cBT.Endif();
    }
    else if (sDirective.compare("error") == 0
        || sDirective.compare("pragma") == 0
        || sDirective.compare("extension") == 0
        || sDirective.compare("version") == 0
        || sDirective.compare("line") == 0)
    {
        ProcessLine(sLine);
    }
    else
    {
        Console.Warn << _T("CGLSLPreprocessor::ProcessDirective() - ") << XtoA(m_iLineNum) << _T(": Unknown GLSL preprocessor directive: ") << sDirective << newl;
        ProcessLine(sLine);
    }
}

void CGLSLPreprocessor::ProcessLine(const string &sLine)
{
    if (!m_cBT.Active())
        return;
    
    m_sProcessedSource += DoSubstitutions(sLine) + '\n';
}

void CGLSLPreprocessor::AddSource(const char *source)
{
    try
    {
        const char *s = source;
        string sLine;
        m_iLineNum = 1;
        while (*s)
        {
            // comment handling
            if (*s == '/')
            {
                if (s[1] && s[1] == '/')
                {
                    while (*++s && *s != '\n' && *s != '\r');
                }
                else if (s[1] && s[1] == '*')
                {
                    while (s[1] && !(s[0] == '*' && s[1] == '/'))
                    {
                        if (*s == '\n' || *s == '\r')
                        {
                            if (!sLine.empty())
                            {
                                if (IsWhiteSpace(*sLine.rbegin()))
                                    sLine.erase(sLine.length()-1);
                                if (!sLine.empty())
                                {
                                    if (sLine[0] == '#')
                                        ProcessDirective(sLine);
                                    else
                                        ProcessLine(sLine);
                                }
                            }
                            sLine.clear();
                            ++m_iLineNum;
                        }
                        s++;
                    }
                    s+=2;
                }
            }
            
            if (IsWhiteSpace(*s))
            {
                if (!sLine.empty() && !IsWhiteSpace(*sLine.rbegin()))
                    sLine += ' ';
            }
            else if (*s == '\n' || *s == '\r')
            {
                if (!sLine.empty())
                {
                    if (IsWhiteSpace(*sLine.rbegin()))
                        sLine.erase(sLine.length()-1);
                    if (!sLine.empty())
                    {
                        if (sLine[0] == '#')
                            ProcessDirective(sLine);
                        else
                            ProcessLine(sLine);
                    }
                }
                sLine.clear();
                ++m_iLineNum;
            }
            else
                sLine += *s;
            
            s++;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CGLSLPreprocessor::AddSource() - ") + XtoA(m_iLineNum) + _T(": "), NO_THROW);
    }
}

char *CGLSLPreprocessor::AllocSourceArray()
{
    if (vid_debugGLSLShaderPreprocessor)
    {
        Console.Dev << _T("Processed source:\n");
        int iLine(1);
        size_t  zLast = 0;
        for (size_t zIndex(0); zIndex < m_sProcessedSource.length(); ++zIndex)
        {
            if (m_sProcessedSource[zIndex] == '\n')
            {
                Console.Dev << XtoA(iLine++, FMT_NONE, 3) << _T(": ") << m_sProcessedSource.substr(zLast, zIndex - zLast) << newl;
                zLast = zIndex + 1;
            }
        }
        
    }
    
    char *szSource = K2_NEW_ARRAY(ctx_GL2, char, m_sProcessedSource.size()+1);
    STRCPY_S(szSource, m_sProcessedSource.size() + 1, m_sProcessedSource.c_str());
    return szSource;
}
