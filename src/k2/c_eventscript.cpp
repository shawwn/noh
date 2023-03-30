// (C)2006 S2 Games
// c_eventscript.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_eventscript.h"
#include "c_eventcmd.h"
#include "c_eventcmdregistry.h"

#include "c_console.h"
#include "parser.h"
#include "evaluator.h"
#include "c_cmd.h"
#include "c_function.h"

#undef pEventScript
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CEventScript*   CEventScript::s_pInstance;
bool        CEventScript::s_bReleased(false);

CEventScript *pEventScript = CEventScript::GetInstance();
//=============================================================================

/*====================
  CEventScript::CEventScript
  ====================*/
CEventScript::CEventScript()
{
}


/*====================
  CEventScript::~CEventScript
  ====================*/
CEventScript::~CEventScript()
{
}

/*====================
  CEventScript::GetInstance
  ====================*/
CEventScript*   CEventScript::GetInstance()
{
    assert(!s_bReleased);

    if (!s_pInstance)
        s_pInstance = K2_NEW(ctx_Models,  CEventScript);

    return s_pInstance;
}

/*====================
  CEventScript::Release
  ====================*/
void    CEventScript::Release()
{
    assert(!s_bReleased);

    if (s_pInstance)
        K2_DELETE(s_pInstance);

    s_bReleased = true;
}

float   Parse_EvalObjectLookup(const tstring &sName);

/*====================
  CEventScript::DoCommand

  Execute a parsed eventscript command
  ====================*/
bool    CEventScript::DoCommand(const tsvector &vArgList, int iTimeNudge)
{
    PROFILE("CEventScript::DoCommand");

    // Skip labels (labels must be at the start of a line)
    if (vArgList[0][0] == '@')
        return false;

    tsvector::const_iterator itBegin = vArgList.begin() + 1;
    tsvector::const_iterator itEnd = vArgList.end();

    // Weed out comments
    for (tsvector::const_iterator it = vArgList.begin(); it != vArgList.end(); ++it)
    {
        if (CompareNoCaseNum(*it, _T("//"), 2) == 0)
        {
            itEnd = it;
            break;
        }
    }

    // if it was a pure comment line, we're done
    if (vArgList.begin() == itEnd)
        return false;

    CEventCmd *pEventCmd = pEventCmdRegistry->GetEventCmd(vArgList.front());
    if (pEventCmd)
    {
        return pEventCmd->Execute(tsvector(itBegin, itEnd), iTimeNudge);
    }

    Console.Std << _T("Unknown identifier: '") << vArgList.front() << _T("'") << newl;
    return false;
}


/*====================
  CEventScript::Execute
  ====================*/
bool    CEventScript::Execute(const tstring &sScript, int iTimeNudge)
{
    PROFILE("CEventScript::Execute");

    TCHAR cmdfull[CMD_MAX_LENGTH];

    TCHAR *c;
    TCHAR outcmd[CMD_MAX_LENGTH];
    TCHAR *out;
    bool grouped = false;
    bool escaped = false;
    TCHAR *cmd_argv[CMD_MAX_ARGS];
    int argcount = 1;

    if (sScript.empty())
        return false;

    _tcscpy(cmdfull, sScript.c_str());

    //skip leading spaces
    c = SkipSpaces(cmdfull);

    if (!*c)            //no command
        return false;

    if (_tcslen(c) > CMD_MAX_LENGTH)
    {
        Console.Std << _T("Command too long: ") << c << newl;
        return false;
    }

    out = outcmd;
    cmd_argv[0] = out;

    //set blah 5; do bleh blih bloh "erferf; afafaf; agaga"; blah
    while(*c && out < &outcmd[CMD_MAX_LENGTH])
    {
        if (!escaped)
        {
            switch(*c)
            {

                case '"':               //argument grouper
                    grouped = !grouped;
                    break;
                case '\\':              //escape character
                    escaped = true;
                    break;
                case ';':               //command separator
                    if (!grouped)
                    {
                        //perform command
                        *out = 0;
                        tsvector    vArgList;

                        for (int i = 0; i < argcount; ++i)
                            vArgList.push_back(cmd_argv[i]);

                        DoCommand(vArgList, iTimeNudge);
                        argcount = 1;
                        out = outcmd;   //clear output for next command
                        cmd_argv[0] = out;
                        c = SkipSpaces(c + 1) - 1;  //- 1 because of the c++ below
                    }
                    else
                    {
                        *out = *c;
                        ++out;
                    }
                    break;
                case ' ':               //argument separator
                case '\t':
                case '\n':
                    if (!grouped)
                    {
                        *out = 0;
                        ++out;
                        c = SkipSpaces(c);
                        if (*c)
                        {
                            //start a new argument
                            cmd_argv[argcount] = out;
                            ++argcount;
                            if (argcount >= CMD_MAX_ARGS)
                            {
                                Console.Std << sScript << _T(": too many arguments!") << newl;
                                return false;
                            }
                        }
                        c--;    //to compensate for the c++ below
                    }
                    else
                    {
                        *out = *c;
                        ++out;
                    }
                    break;
                case '#':               //value insertion command
                case '$':               //value insertion command, evaluate immediately (ignore quotes)
                    if (!grouped || *c == '$')
                    {
                        TCHAR *nextPound = _tcschr(c + 1, *c);

                        if (nextPound)
                        {
                            TCHAR objname[256];
                            size_t namelen = (nextPound - (c + 1));
                            tstring valuestring;

                            MemManager.Copy(objname, c + 1, (namelen > 255 ? 255 : namelen) * sizeof(TCHAR));
                            objname[namelen] = 0;   //null terminate the string

                            valuestring = Cmd_GetObjectValueString(objname);
                            _TCSNCPY_S(out, CMD_MAX_LENGTH - (out - outcmd), valuestring.c_str(), &outcmd[CMD_MAX_LENGTH] - out - 1);

                            c = nextPound;  //skip over the object name since we have parsed it
                            out += _tcslen(valuestring.c_str());
                        }
                        else
                        {
                            *out = *c;
                            ++out;
                        }
                    }
                    else
                    {
                        *out = *c;
                        ++out;
                    }
                    break;
                case '[':               //expression
                case '{':               //expression, evaluate immediately (ignore quotes)
                {
                    if (!grouped || *c == '{')
                    {
                        TCHAR *closeBracket = _tcschr(c + 1, *c == '[' ? ']' : '}');

                        if (closeBracket)
                        {
                            static TCHAR expr[1024];
                            size_t exprlen = (closeBracket - (c + 1));
                            MemManager.Copy(expr, c + 1, exprlen > 1023 ? 1023 : exprlen);
                            expr[exprlen] = 0;      //null terminate the string

                            bool bError(false);
                            float fValue(Eval_Evaluate(expr, bError, Parse_EvalObjectLookup));

                            tstring sValue;
                            if (bError)
                            {
                                sValue = _T("(EXPR_ERROR)");
                            }
                            else
                            {
                                if (fValue - floor(fValue) < EPSILON)
                                    sValue = XtoA(fValue, 0, 0, 0);
                                else
                                    sValue = XtoA(fValue);
                            }
                            _TCSNCPY_S(out, CMD_MAX_LENGTH - (out - outcmd), sValue.c_str(), &outcmd[CMD_MAX_LENGTH] - out);

                            c = closeBracket;
                            out += MIN(sValue.length(), size_t(&outcmd[CMD_MAX_LENGTH] - out));
                        }
                        else
                        {
                            *out = *c;
                            ++out;
                        }
                    }
                    else
                    {
                        *out = *c;
                        ++out;
                    }
                    break;
                }
                default:
                    *out = *c;
                    ++out;
                    break;
            }
        }
        else
        {
            //we've ignored the previous \ character
            *out = *c;
            ++out;
            escaped = false;
        }

        ++c;
    }

    *out = 0;

    // any remaining commands

    if (*cmd_argv[0])
    {
        tsvector    vArgList;

        for (int i = 0; i < argcount; ++i)
            vArgList.push_back(cmd_argv[i]);

        return DoCommand(vArgList, iTimeNudge);
    }

    return false;
}



/*--------------------
  eventCmd
  --------------------*/
EVENT_CMD(Cmd)
{
    Console.Execute(ConcatinateArgs(vArgList));
    return false;
}


/*--------------------
  eventEcho
  --------------------*/
EVENT_CMD(Echo)
{
    Console << ParenStr(iTimeNudge) << " ";

    for (size_t z(0); z < vArgList.size(); ++z)
        Console << vArgList[z] << " ";

    Console << newl;

    return true;
}
