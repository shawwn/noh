// (C)2005 S2 Games
// c_console.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "k2_api.h"

#include "c_console.h"
#include "c_system.h"
#include "c_consoleregistry.h"
#include "c_cmd.h"
#include "c_cvar.h"
#include "stringutils.h"
#include "evaluator.h"
#include "c_input.h"
#include "c_action.h"
#include "c_filemanager.h"
#include "c_script.h"
#include "c_scriptthread.h"
#include "c_draw2d.h"
#include "c_bind.h"
#include "c_actionregistry.h"
#include "c_function.h"
#include "c_alias.h"
#include "c_fontmap.h"
#include "c_uimanager.h"
#include "c_uitriggerregistry.h"
#include "c_uitrigger.h"
#include "c_uicmd.h"
#include "c_date.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
int                     CConsoleOutputBuffer::m_iRefCount;
bool                    CConsoleOutputBuffer::m_bReleased;
CConsoleOutputBuffer    *CConsoleOutputBuffer::m_pInstance;

int                     CConsoleNotifyBuffer::m_iRefCount;
bool                    CConsoleNotifyBuffer::m_bReleased;
CConsoleNotifyBuffer    *CConsoleNotifyBuffer::m_pInstance;

CVAR_STRINGF(con_font,          "system_medium",            CVAR_SAVECONFIG);
CVAR_STRINGF(con_prompt,        ">",                        CVAR_SAVECONFIG);
CVAR_VEC3F  (con_bgColor,       CVec3f(0.5f, 0.5f, 0.5f),   CVAR_SAVECONFIG);
CVAR_VEC3F  (con_color,         CVec3f(1.0f, 1.0f, 1.0f),   CVAR_SAVECONFIG);
CVAR_BOOLF  (con_wordWrap,      false,                      CVAR_SAVECONFIG);
CVAR_INTF   (con_tabWidth,      5,                          CVAR_SAVECONFIG);
CVAR_FLOATF (con_alpha,         1.0f,                       CVAR_SAVECONFIG);
CVAR_FLOATF (con_height,        1.0f,                       CVAR_SAVECONFIG);
CVAR_UINTF  (con_toggleTime,    200,                        CVAR_SAVECONFIG);

CVAR_BOOL   (con_showStd,       true);
CVAR_BOOL   (con_showClient,    true);
CVAR_BOOL   (con_showServer,    true);
CVAR_BOOL   (con_showDev,       true);
CVAR_BOOL   (con_showErr,       true);
CVAR_BOOL   (con_showWarn,      true);
CVAR_BOOL   (con_showNet,       true);
CVAR_BOOL   (con_showUI,        true);
CVAR_BOOL   (con_showMem,       true);
CVAR_BOOL   (con_showAI,        true);
CVAR_BOOL   (con_showRes,       true);
CVAR_BOOL   (con_showVid,       true);
CVAR_BOOL   (con_showGV,        true);
CVAR_BOOL   (con_showGVDebug,   true);

CVAR_FLOAT  (con_leftMargin,    5.0f);

CVAR_BOOL   (con_appendLog,     false);
CVAR_BOOL   (con_writeLog,      true);
CVAR_BOOL   (con_developer,     false);
CVAR_BOOL   (con_debugOutput,   true);

CVAR_BOOLF  (con_notify,        false,  CVAR_SAVECONFIG);
CVAR_INTF   (con_notifyLines,   8,      CVAR_SAVECONFIG);
CVAR_INTF   (con_notifyTime,    4000,   CVAR_SAVECONFIG);

SINGLETON_INIT(CConsole);
CConsole *g_pConsole(CConsole::GetInstance());

static int  s_iLastIfResult = -1;
//=============================================================================

/*====================
  Parse_EvalObjectLookup
  ====================*/
float   Parse_EvalObjectLookup(const tstring &sName)
{
    ICvar *pCvar = ICvar::Find(sName);

    if (pCvar)
        return pCvar->GetFloat();

    return 0.0f;
}


/*====================
  CConsoleOutputBuffer::CConsoleOutputBuffer
  ====================*/
CConsoleOutputBuffer::CConsoleOutputBuffer() :
m_zMaxOutputHistory(DEFAULT_CONSOLE_OUTPUT_HISTORY_SIZE),
m_bOutputLineInProgress(false)
{
}


/*====================
  CConsoleOutputBuffer::GetInstance
  ====================*/
CConsoleOutputBuffer*   CConsoleOutputBuffer::GetInstance()
{
    assert(!m_bReleased);

    if (m_pInstance == NULL)
    {
        m_pInstance = K2_NEW(ctx_Console,  CConsoleOutputBuffer);
        m_iRefCount = 0;
    }

    ++m_iRefCount;
    return m_pInstance;
}


/*====================
  CConsoleOutputBuffer::Release
  ====================*/
void    CConsoleOutputBuffer::Release()
{
    assert(!m_bReleased);
    assert(m_iRefCount > 0);

    --m_iRefCount;
    if (m_iRefCount == 0 && m_pInstance != NULL)
    {
        SAFE_DELETE(m_pInstance);
        m_bReleased = true;
    }
}


/*====================
  CConsoleOutputBuffer::SetMaxOutputHistory
  ====================*/
void    CConsoleOutputBuffer::SetMaxOutputHistory(size_t size)
{
    m_zMaxOutputHistory = size;
    if (m_zMaxOutputHistory < 0)
        m_zMaxOutputHistory = 0;
    m_deqOutputHistory.resize(m_zMaxOutputHistory);
}


/*====================
  CConsoleOutputBuffer::AddOutputHistory
  ====================*/
void    CConsoleOutputBuffer::AddOutputHistory(const tstring &sLine, CConsoleStream &Stream)
{
    tstring sOutputLine;
    CConsoleStream *pStream = &Stream;

    if (m_bOutputLineInProgress && !m_deqOutputHistory.empty())
    {
        sOutputLine = m_deqOutputHistory.front().sLine;
        pStream = m_deqOutputHistory.front().pStream;
        m_deqOutputHistory.pop_front();
        m_bOutputLineInProgress = false;
    }
    else
    {
        sOutputLine = Stream.GetPrefix();
    }
    
    sOutputLine += sLine;

    // standardize newline chars
    tstring::size_type pos(0);
    while (pos != tstring::npos)
    {
        pos = sOutputLine.find(_T('\r'));
        if (pos != tstring::npos)
            sOutputLine[pos] = _T('\n');
    }

    // break strings apart at newlines
    pos = 0;
    while (pos != tstring::npos)
    {
        pos = sOutputLine.find(_T('\n'));
        if (pos != tstring::npos)
        {
            if (K2System.GetConsoleWindowHandle() && CConsole::GetInstance()->IsStreamVisible(pStream))
                K2System.AddDedicatedConsoleText(sOutputLine.substr(0, pos) + _T("\r\n"));

            m_deqOutputHistory.push_front(SOutputLine(sOutputLine.substr(0, pos), pStream));
            sOutputLine = sOutputLine.substr(pos + 1);

            if (!sOutputLine.empty() && !Stream.GetPrefix().empty())
                sOutputLine = Stream.GetPrefix() + sOutputLine;

            if (m_deqOutputHistory.size() > m_zMaxOutputHistory)
                m_deqOutputHistory.pop_back();

            if (CConsole::GetInstance()->GetOutputHistoryPos() != 0)
                CConsole::GetInstance()->OutputHistoryScroll(1);
        }
    }

    if (!sOutputLine.empty())
    {
        m_deqOutputHistory.push_front(SOutputLine(sOutputLine, pStream));
        m_bOutputLineInProgress = true;
    }
}


/*====================
  CConsoleNotifyBuffer::CConsoleNotifyBuffer
  ====================*/
CConsoleNotifyBuffer::CConsoleNotifyBuffer() :
m_zMaxHistory(DEFAULT_CONSOLE_NOTIFY_HISTORY_SIZE),
m_bLineInProgress(false)
{
}


/*====================
  CConsoleNotifyBuffer::GetInstance
  ====================*/
CConsoleNotifyBuffer*   CConsoleNotifyBuffer::GetInstance()
{
    assert(!m_bReleased);

    if (m_pInstance == NULL)
    {
        m_pInstance = K2_NEW(ctx_Console,  CConsoleNotifyBuffer);
        m_iRefCount = 0;
    }

    ++m_iRefCount;
    return m_pInstance;
}


/*====================
  CConsoleNotifyBuffer::Release
  ====================*/
void    CConsoleNotifyBuffer::Release()
{
    assert(!m_bReleased);
    assert(m_iRefCount > 0);

    --m_iRefCount;
    if (m_iRefCount == 0 && m_pInstance != NULL)
    {
        SAFE_DELETE(m_pInstance);
        m_bReleased = true;
    }
}


/*====================
  CConsoleNotifyBuffer::SetMaxHistory
  ====================*/
void    CConsoleNotifyBuffer::SetMaxHistory(size_t size)
{
    m_zMaxHistory = size;
    if (m_zMaxHistory < 0)
        m_zMaxHistory = 0;
    m_deqHistory.clear();
}


/*====================
  CConsoleNotifyBuffer::AddHistory
  ====================*/
void    CConsoleNotifyBuffer::AddHistory(const tstring &sLine, CConsoleStream &Stream)
{
    tstring sOutputLine;
    CConsoleStream *pStream = &Stream;
    int iTime(Host.GetSystemTime());

    if (m_bLineInProgress && !m_deqHistory.empty())
    {
        sOutputLine = m_deqHistory.back().sLine;
        pStream = m_deqHistory.back().pStream;
        m_deqHistory.pop_back();
        m_bLineInProgress = false;
    }
    else
    {
        sOutputLine = Stream.GetPrefix();
    }

    sOutputLine += sLine;

    // standardize newline chars
    size_t pos(0);
    while(pos != tstring::npos)
    {
        pos = sOutputLine.find(_T('\r'));
        if (pos != tstring::npos)
            sOutputLine[pos] = _T('\n');
    }

    // break strings apart at newlines
    pos = 0;
    while (pos != tstring::npos)
    {
        pos = sOutputLine.find(_T('\n'));
        if (pos != tstring::npos)
        {
            m_deqHistory.push_back(SNotifyLine(iTime, sOutputLine.substr(0, pos), pStream));
            sOutputLine = sOutputLine.substr(pos + 1);

            if (!sOutputLine.empty() && !Stream.GetPrefix().empty())
                sOutputLine = Stream.GetPrefix() + sOutputLine;

            if (m_deqHistory.size() > m_zMaxHistory)
                m_deqHistory.pop_front();
        }
    }

    if (!sOutputLine.empty())
    {
        m_deqHistory.push_back(SNotifyLine(iTime, sOutputLine, pStream));
        m_bLineInProgress = true;
    }
}


/*====================
  CConsoleNotifyBuffer::Frame
  ====================*/
void    CConsoleNotifyBuffer::Frame()
{
    if (con_notifyLines.IsModified())
    {
        SetMaxHistory(con_notifyLines);
        con_notifyLines.SetModified(false);
    }

    int iKillTime = Host.GetSystemTime() - con_notifyTime;

    while (m_deqHistory.size() > 0 && m_deqHistory.front().iTime < iKillTime)
        m_deqHistory.pop_front();
}


/*====================
  CConsoleStream::CConsoleStream
  ====================*/
CConsoleStream::CConsoleStream(const CVec4f &v4Color, const tstring &sPrefix) :
m_bEnabled(true),
m_v4Color(v4Color),
m_sPrefix(sPrefix)
{
    m_pOutput = CConsoleOutputBuffer::GetInstance();
    m_pNotify = CConsoleNotifyBuffer::GetInstance();
}


/*====================
  CConsoleStream::AddOutputHistoryT
  ====================*/
void    CConsoleStream::AddOutputHistoryT(const tstring &sLine)
{
    if (!m_bEnabled)
        return;

    if (con_writeLog)
        CConsole::GetInstance()->WriteLog(sLine);

    CConsole::GetInstance()->Watch(sLine);

    m_pOutput->AddOutputHistory(sLine, *this);

    if (con_debugOutput)
        K2System.DebugOutput(sLine);

    if (con_notify && !CConsole::GetInstance()->IsActive())
        m_pNotify->AddHistory(sLine, *this);
}


/*====================
  CConsole::CConsole
  ====================*/
CConsole::CConsole() :
m_bActive(false),
m_pDefaultStream(&Std),
m_zMaxInputHistory(DEFAULT_CONSOLE_INPUT_HISTORY_SIZE),
m_iInputHistoryPos(-1),
m_iOutputHistoryPos(0),

m_zInputPos(0),
m_bOverwrite(false),

m_zInputPosInProgress(0),

m_fCursorBlinkRate(DEFAULT_CURSOR_BLINK_RATE),
m_fWidth(DEFAULT_CONSOLE_WIDTH),
m_fHeight(DEFAULT_CONSOLE_HEIGHT),
m_iRows(0),
m_iShiftPgRows(25),
m_iPgRows(3),

m_iScriptsRunning(0),
m_pActiveThread(NULL),

m_newl(_T("\n")),

m_iDedicatedConsolePos(0),
m_uiToggleTime(0),

m_bWatch(0),

// Streams
Std(        WHITE,                              _T("")),
Dev(        CVec4f(0.66f, 0.66f, 0.66f, 1.0f),  _T("Dev: ")),
Err(        CVec4f(0.66f, 0.00f, 0.00f, 1.0f),  _T("Error: ")),
Warn(       CVec4f(1.00f, 1.00f, 0.33f, 1.0f),  _T("Warning: ")),
Net(        CVec4f(0.33f, 0.33f, 1.00f, 1.0f),  _T("Net: ")),
Server(     CVec4f(0.00f, 0.66f, 0.00f, 1.0f),  _T("Sv: ")),
Client(     CVec4f(0.00f, 0.66f, 0.66f, 1.0f),  _T("Cl: ")),
UI(         CVec4f(1.00f, 0.33f, 0.33f, 1.0f),  _T("UI: ")),
Perf(       CVec4f(0.33f, 0.33f, 0.33f, 1.0f),  _T("Perf: ")),
Mem(        CVec4f(0.66f, 0.00f, 0.66f, 1.0f),  _T("Mem: ")),
AI(         CVec4f(1.00f, 0.33f, 1.00f, 1.0f),  _T("AI: ")),
Video(      CVec4f(0.00f, 0.00f, 0.66f, 1.0f),  _T("Vid: ")),
ServerGame( CVec4f(0.33f, 1.00f, 0.33f, 1.0f),  _T("SGame: ")),
ClientGame( CVec4f(0.33f, 1.00f, 1.00f, 1.0f),  _T("CGame: ")),
Res(        CVec4f(0.66f, 0.33f, 0.00f, 1.0f),  _T("Resource: ")),
GroupVoice( CVec4f(0.33f, 0.66f, 1.00f, 1.0f),  _T("GroupVoice: ")),
GroupVoiceDebug(    CVec4f(0.33f, 0.66f, 0.33f, 1.0f),  _T("GroupVoiceDebug: ")),
Script(     CVec4f(1.00f, 0.66f, 0.66f, 1.0f),  _T("Script: "))
{   
}


/*====================
  CConsole::OpenLog
  ====================*/
void    CConsole::OpenLog()
{
    m_hLogFile.Open(_T("~/console.log"), (con_appendLog ? FILE_APPEND : FILE_WRITE) | FILE_TEXT);
}


/*====================
  CConsole::CloseLog
  ====================*/
void    CConsole::CloseLog()
{
    if (m_hLogFile.IsOpen())
        m_hLogFile.Close();
}


/*====================
  CConsole::SetMaxInputHistory
 ====================*/
void    CConsole::SetMaxInputHistory(size_t size)
{
    m_zMaxInputHistory = size;
    if (m_zMaxInputHistory < 0)
        m_zMaxInputHistory = 0;
    m_deqInputHistory.resize(m_zMaxInputHistory);
}


/*====================
  CConsole::SetMaxOutputHistory
  ====================*/
void    CConsole::SetMaxOutputHistory(size_t size)
{
    Std.SetMaxOutputHistory(size);
}


/*====================
  CConsole::WriteConsoleHistory
  ====================*/
bool    CConsole::WriteConsoleHistory(CFileHandle &hFile)
{
    for (InputHistoryDeque_rit it = m_deqInputHistory.rbegin(); it != m_deqInputHistory.rend(); ++it)
    {
        hFile << _T("ConsoleHistoryAppend") << _T(" ") << QuoteStr(AddEscapeChars(*it)) << newl;
    }
    return true;
}


/*====================
  CConsole::AddInputHistory
 ====================*/
void    CConsole::AddInputHistory(const tstring &sLine)
{
    // don't duplicate the last line
    if (!m_deqInputHistory.empty())
    {
        if (sLine == m_deqInputHistory.front())
            return;
    }

    // don't add empty lines
    if (sLine.empty())
        return;

    m_deqInputHistory.push_front(sLine);

    if (m_deqInputHistory.size() > m_zMaxInputHistory)
        m_deqInputHistory.pop_back();
}


/*====================
  CConsole::AddCmdBuffer
  ====================*/
void    CConsole::AddCmdBuffer(const tstring &sLine)
{
    m_vecCmdBuffer.push_back(sLine);
    Std << con_prompt << m_sInputLine << newl;
}


/*====================
  CConsole::IsStreamVisible
 ====================*/
bool    CConsole::IsStreamVisible(CConsoleStream *pStream)
{
    if ((pStream == &Std && !con_showStd) ||
        (pStream == &Client && !con_showClient) ||
        (pStream == &Server && !con_showServer) ||
        (pStream == &Dev && !con_showDev) ||
        (pStream == &Err && !con_showErr) ||
        (pStream == &Warn && !con_showWarn) ||
        (pStream == &Net && !con_showNet) ||
        (pStream == &UI && !con_showUI) ||
        (pStream == &Mem && !con_showMem) ||
        (pStream == &Video && !con_showVid) ||
        (pStream == &Res && !con_showRes) ||
        (pStream == &AI && !con_showAI) ||
        (pStream == &GroupVoice && !con_showGV) ||
        (pStream == &GroupVoiceDebug && !con_showGVDebug))
        return false;
    else
        return true;
}


/*====================
  CConsole::OutputHistoryScroll
 ====================*/
void    CConsole::OutputHistoryScroll(int iAmount)
{
    int iBufferLength = int(Std.GetSize());

    int iMinLines(0);
    int iVisibleRows(0);

    while (iVisibleRows != m_iRows)
    {
        int iLines(iBufferLength - iMinLines - 1);
        if (iLines < 0)
            break;

        CConsoleStream *pStream = Std.GetLineStream(iLines);

        ++iMinLines;

        if (!IsStreamVisible(pStream))
            continue;

        ++iVisibleRows;
    }

    if (iBufferLength < iMinLines)
    {
        m_iOutputHistoryPos = 0;
        return;
    }

    if (iAmount > 0)
    {
        while (iAmount > 0 && (m_iOutputHistoryPos < iBufferLength - iMinLines))
        {
            ++m_iOutputHistoryPos;
            CConsoleStream *pStream = Std.GetLineStream(m_iOutputHistoryPos);

            if (!IsStreamVisible(pStream))
                continue;

            --iAmount;
        }

        while (m_iOutputHistoryPos < iBufferLength - iMinLines)
        {
            CConsoleStream *pStream = Std.GetLineStream(m_iOutputHistoryPos);

            if (IsStreamVisible(pStream))
                break;

            ++m_iOutputHistoryPos;
        }
    }
    else if (iAmount < 0)
    {
        while (iAmount < 0 && m_iOutputHistoryPos > 0)
        {
            --m_iOutputHistoryPos;
            CConsoleStream *pStream = Std.GetLineStream(m_iOutputHistoryPos);

            if (!IsStreamVisible(pStream))
                continue;

            ++iAmount;
        }

        while (m_iOutputHistoryPos > 0)
        {
            CConsoleStream *pStream = Std.GetLineStream(m_iOutputHistoryPos);

            if (IsStreamVisible(pStream))
                break;

            --m_iOutputHistoryPos;
        }
    }

}


/*====================
  CConsole::OutputHistoryGoto
 ====================*/
void    CConsole::OutputHistoryGoto(int iPos)
{
    int iBufferLength = int(Std.GetSize());

    m_iOutputHistoryPos = iPos;
    if (m_iOutputHistoryPos < 0)
        m_iOutputHistoryPos = 0;
    if (m_iOutputHistoryPos > iBufferLength - m_iRows)
        m_iOutputHistoryPos = iBufferLength - m_iRows;
    if (iBufferLength < m_iRows)
        m_iOutputHistoryPos = 0;
}


/*====================
  CConsole::DoCommand

  Execute a parsed console command
  ====================*/
void    CConsole::DoCommand(const tsvector &vArgList)
{
    PROFILE("CConsole::DoCommand");

    try
    {
        // Needs at least one element
        if (vArgList.empty())
            EX_WARN(_T("Empty arg list"));

        // Skip labels (labels must be at the start of a line)
        if (vArgList[0][0] == '@')
            return;

        tsvector_cit itBegin(vArgList.begin() + 1);
        tsvector_cit itEnd(vArgList.end());

        // Weed out comments
        for (tsvector_cit it(vArgList.begin()); it != vArgList.end(); ++it)
        {
            if (CompareNoCaseNum(*it, _T("//"), 2) == 0)
            {
                itEnd = it;
                break;
            }
        }

        // If it was a pure comment line, we're done
        if (vArgList.begin() == itEnd)
            return;

        CConsoleElement *pElement(GetElement(vArgList[0]));
        if (pElement == NULL)
            EX_MESSAGE(_T("Unknown identifier: ") + SingleQuoteStr(vArgList[0]));

        pElement->Execute(tsvector(itBegin, itEnd));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CConsole::DoCommand() - "), NO_THROW);
    }
}

/*====================
  CConsole::DoPrecache

  Attempts to precache any resources used by a command
  ====================*/
void    CConsole::DoPrecache(const tsvector &vArgList)
{
    PROFILE("CConsole::DoPrecache");

    try
    {
        // Needs at least one element
        if (vArgList.empty())
            EX_WARN(_T("Empty arg list"));

        // Skip labels (labels must be at the start of a line)
        if (vArgList[0][0] == '@')
            return;

        tsvector_cit itBegin(vArgList.begin() + 1);
        tsvector_cit itEnd(vArgList.end());

        // Weed out comments
        for (tsvector_cit it(vArgList.begin()); it != vArgList.end(); ++it)
        {
            if (CompareNoCaseNum(*it, _T("//"), 2) == 0)
            {
                itEnd = it;
                break;
            }
        }

        // If it was a pure comment line, we're done
        if (vArgList.begin() == itEnd)
            return;

        bool bFoundCmd(false);
        CConsoleElement *pElement(GetElement(vArgList[0]));

        // If the command is registered, attempt to precache it
        if (pElement != NULL)
            bFoundCmd = pElement->Precache(tsvector(itBegin, itEnd));

        if (!bFoundCmd)
        {
            // Attempt to find a registered precache command
            pElement = GetElement(vArgList[0] + _T("_Precache"));

            if (pElement != NULL && pElement->GetType() == ELEMENT_CMDPRECACHE)
                pElement->Execute(tsvector(itBegin, itEnd));
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CConsole::DoPrecache() - "), NO_THROW);
    }
}


/*====================
  Cmd_GetObjectValueString
  ====================*/
tstring     Cmd_GetObjectValueString(const tstring &sObjname)
{
    if (UITriggerRegistry.Exists(sObjname))
    {
        CUITrigger *pUITrigger(UITriggerRegistry.GetUITrigger(sObjname));

        if (pUITrigger)
            return pUITrigger->GetLastParam();
    }
    else if (CFunction::IsFunction(sObjname))
    {
        tstring sName;
        tsvector vArgList;

        if (CFunction::Parse(sObjname, sName, vArgList))
        {
            CConsoleElement *pElem = CConsoleRegistry::GetInstance()->GetElement(sName);

            if (pElem && pElem->GetType() == ELEMENT_FUNCTION)
                return pElem->Evaluate(vArgList);
        }
    }
    else
    {
        ICvar *pCvar;

        pCvar = ICvar::Find(sObjname);

        if (pCvar)
            return pCvar->GetString();
    }

    return _T("(UNDEFINED)");
}


/*====================
  Cmd_GetObjectValueString2
  ====================*/
tstring     Cmd_GetObjectValueString2(const tstring &sObjname)
{
    return TSNULL;
}


/*====================
  CConsole::Execute
  ====================*/
void    CConsole::Execute(const tstring &sCmdLine)
{
    PROFILE("CConsole::Execute");

    TCHAR cmdfull[CMD_MAX_LENGTH];

    TCHAR *c;
    TCHAR outcmd[CMD_MAX_LENGTH];
    TCHAR *out;
    bool grouped = false;
    bool escaped = false;
    TCHAR *cmd_argv[CMD_MAX_ARGS];
    int argcount = 1;

    if (sCmdLine.empty())
        return;

    _tcscpy(cmdfull, sCmdLine.c_str());

    //skip leading spaces
    c = SkipSpaces(cmdfull);

    if (!*c)            //no command
        return;

    if (_tcslen(c) > CMD_MAX_LENGTH)
    {
        Std << _T("Command too long: ") << c << newl;
        return;
    }

    out = outcmd;
    cmd_argv[0] = out;

    //set blah 5; do bleh blih bloh "erferf; afafaf; agaga"; blah
    while(*c && out < &outcmd[CMD_MAX_LENGTH])
    {
        if (!escaped)
        {
            switch (*c)
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

                        DoCommand(vArgList);
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
                                Std << sCmdLine << _T(": too many arguments!") << newl;
                                return;
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

                        size_t zLength(nextPound - c);
                        while (nextPound && *nextPound && zLength > 1 && *(nextPound - 1) == '|')
                        {
                            nextPound = _tcschr(nextPound + 1, *nextPound);
                        }

                        if (nextPound)
                        {
                            tstring sObjName(c + 1, nextPound - (c + 1));

                            size_t zFirstPipe(sObjName.find(_T("|#")));
                            while (zFirstPipe != tstring::npos)
                            {
                                size_t zNextPipe(sObjName.find(_T("|#"), zFirstPipe + 2));

                                if (zNextPipe != tstring::npos)
                                {
                                    tstring sNestedObjName(sObjName.substr(zFirstPipe + 2, zNextPipe - zFirstPipe - 2));
                                    tstring sNestedValueString(Cmd_GetObjectValueString(sNestedObjName));

                                    sObjName = sObjName.substr(0, zFirstPipe) + sNestedValueString + sObjName.substr(zNextPipe + 2);
                                }
                                else
                                    sObjName = sObjName.substr(0, zFirstPipe);

                                zFirstPipe = sObjName.find(_T("|#"));
                            }

                            tstring valuestring = Cmd_GetObjectValueString(sObjName);
                            _TCSNCPY_S(out, CMD_MAX_LENGTH - (out - outcmd), valuestring.c_str(), &outcmd[CMD_MAX_LENGTH] - out - 1);

                            c = nextPound;  //skip over the object name since we have parsed it
                            out += valuestring.length();
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
                            size_t exprlen(closeBracket - (c + 1));
                            MemManager.Copy(expr, c + 1, (exprlen > 1023 ? 1023 : exprlen) * sizeof(TCHAR));
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
                case '|':
                    *out = *c;
                    ++out;
                    if (*(c + 1))
                    {
                        ++c;
                        *out = *c;
                        ++out;
                    }
                    break;
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

    //any remaining commands

    if (*cmd_argv[0])
    {
        tsvector    vArgList;

        for (int i = 0; i < argcount; ++i)
            vArgList.push_back(cmd_argv[i]);

        DoCommand(vArgList);
    }
}

/*====================
  CConsole::PrecacheCommand
  ====================*/
void    CConsole::PrecacheCommand(const tstring &sCmdLine)
{
    PROFILE("CConsole::PrecacheCommand");

    TCHAR cmdfull[CMD_MAX_LENGTH];

    TCHAR *c;
    TCHAR outcmd[CMD_MAX_LENGTH];
    TCHAR *out;
    bool grouped = false;
    bool escaped = false;
    TCHAR *cmd_argv[CMD_MAX_ARGS];
    int argcount = 1;
    bool bTryPrecache(true);

    if (sCmdLine.empty())
        return;

    _tcscpy(cmdfull, sCmdLine.c_str());

    //skip leading spaces
    c = SkipSpaces(cmdfull);

    if (!*c)            //no command
        return;

    if (_tcslen(c) > CMD_MAX_LENGTH)
    {
        Std << _T("Command too long: ") << c << newl;
        return;
    }

    out = outcmd;
    cmd_argv[0] = out;

    //set blah 5; do bleh blih bloh "erferf; afafaf; agaga"; blah
    while(*c && out < &outcmd[CMD_MAX_LENGTH])
    {
        if (!escaped)
        {
            switch (*c)
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

                        if (bTryPrecache)
                        {
                            for (int i = 0; i < argcount; ++i)
                                vArgList.push_back(cmd_argv[i]);

                            DoPrecache(vArgList);
                        }
                        
                        bTryPrecache = true;

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
                                Std << sCmdLine << _T(": too many arguments!") << newl;
                                return;
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

                        size_t zLength(nextPound - c);
                        while (nextPound && *nextPound && zLength > 1 && *(nextPound - 1) == '|')
                        {
                            nextPound = _tcschr(nextPound + 1, *nextPound);
                        }

                        if (nextPound)
                        {
                            // We can't precache commands that include object names
                            bTryPrecache = false;
                            c = nextPound;
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
                            // We can't precache commands that include expressions
                            bTryPrecache = false;
                            c = closeBracket;
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
                case '|':
                    *out = *c;
                    ++out;
                    if (*(c + 1))
                    {
                        ++c;
                        *out = *c;
                        ++out;
                    }
                    break;
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

    //any remaining commands

    if (*cmd_argv[0] && bTryPrecache)
    {
        tsvector    vArgList;

        for (int i = 0; i < argcount; ++i)
            vArgList.push_back(cmd_argv[i]);

        DoPrecache(vArgList);
    }
}


/*====================
  CConsole::ExecuteScript
  ====================*/
bool    CConsole::ExecuteScript(const tstring &sData, bool bFile, tsmapts *mapParams)
{ 
    PROFILE("CConsole::ExecuteScript");

    if (bFile)
        Std << _T("Executing ") << QuoteStr(sData) << newl;

    CScriptThread *pNewThread(K2_NEW(ctx_Console,  CScriptThread));
    CScriptThread *pOldActiveThread(m_pActiveThread);

    m_pActiveThread = pNewThread;

    pNewThread->ExecuteScript(sData, bFile, mapParams);

    if (!pNewThread->IsFinished())
    {
        m_lScriptThreads.push_back(pNewThread);
        m_pActiveThread = pOldActiveThread;
        return true;
    }
    else
    {
        SAFE_DELETE(pNewThread);
        m_pActiveThread = pOldActiveThread;
        return true;
    }
}


/*====================
  CConsole::CallScript
  ====================*/
bool    CConsole::CallScript(const tstring &sFilename)
{
    if (m_pActiveThread)
    {
        m_pActiveThread->ExecuteScript(sFilename);
        return true;
    }

    return ExecuteScript(sFilename);
}


/*====================
  CConsole::Frame
  ====================*/
void    CConsole::Frame()
{
    PROFILE("CConsole::Frame");

    m_fHeight = con_height;

    // Execute any commands we may have put into the command buffer
    ExecCmdBuffer();

    m_pActiveThread = NULL;

    ThreadList::iterator it = m_lScriptThreads.begin();

    while (it != m_lScriptThreads.end())
    {
        m_pActiveThread = *it;

        m_pActiveThread->Frame();

        if (m_pActiveThread->IsFinished())
        {
            SAFE_DELETE(m_pActiveThread);
            it = m_lScriptThreads.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Update notify buffer
    Std.Frame();

    // Update dedicated console
    if (K2System.GetConsoleWindowHandle())
        K2System.UpdateDedicatedConsoleText();

    // Execute priority binds
    Input.ExecuteBinds(BINDTABLE_CONSOLE, BIND_PRIORITY);

    if (!m_bActive && !K2System.GetConsoleWindowHandle())
        return;

    // Execute non-priority binds
    Input.ExecuteBinds(BINDTABLE_CONSOLE, 0);

    static vector<SIEvent> vUnused;
    vUnused.clear();

    // Steal any other input the console wants
    while (!Input.IsEmpty())
    {
        SIEvent ev(Input.Pop());
        bool bUsed(true);

        switch (ev.eType)
        {
        case INPUT_AXIS:
            break;

        case INPUT_BUTTON:
            // Filter out key releases (but don't delete them)
            if (ev.cAbs.fValue == 0.0f)
            {
                bUsed = false;
                break;
            }

            switch(ev.uID.btn)
            {
            case BUTTON_WHEELUP:
                if (ev.iFlags & IEVENT_CTRL)
                    OutputHistoryScroll(10 * INT_ROUND(ev.cDelta.fValue));
                else
                    OutputHistoryScroll(INT_ROUND(ev.cDelta.fValue));
                break;

            case BUTTON_WHEELDOWN:
                if (ev.iFlags & IEVENT_CTRL)
                    OutputHistoryScroll(10 * -INT_ROUND(ev.cDelta.fValue));
                else
                    OutputHistoryScroll(-INT_ROUND(ev.cDelta.fValue));
                break;

            // Toggle overwriting
            case BUTTON_INS:
                m_bOverwrite = !m_bOverwrite;
                break;

            // Backspace does the same thing as delete, it just steps
            // back one space first
            case BUTTON_BACKSPACE:
                if (m_zInputPos == 0)
                    break;
                --m_zInputPos;
                // fall through to BUTTON_DEL

            case BUTTON_DEL:
                if (m_zInputPos < m_sInputLine.size())
                    m_sInputLine.erase(m_zInputPos, 1);
                break;

            // Submit the input line to the parser
            case BUTTON_ENTER:
                if (m_sInputLine == _T("!"))
                    m_sInputLine = GetLastCmd();

                Std << con_prompt << m_sInputLine << newl;
                Execute(m_sInputLine);
                AddInputHistory(m_sInputLine);
                m_iInputHistoryPos = -1;
                m_iOutputHistoryPos = 0;
                m_sInputLineInProgress.clear();
                // fall through to BUTTON_ESCAPE

            // Clear the line
            case BUTTON_ESC:
                m_zInputPos = 0;
                m_zInputPosInProgress = 0;
                m_sInputLine.clear();
                m_sInputLineInProgress.clear();
                break;

            // Cursor movement
            case BUTTON_LEFT:
                if (Input.IsCtrlDown())
                {
                    if (m_zInputPos == 0)
                        break;

                    bool bFoundWord(false);
                    while (m_zInputPos > 0)
                    {
                        bool bIsSeperator(IsTokenSeparator(m_sInputLine[m_zInputPos - 1]));
                        if (bIsSeperator)
                        {
                            if (bFoundWord)
                                break;
                        }
                        else
                        {
                            bFoundWord = true;
                        }
                        --m_zInputPos;
                    }
                }
                else
                {
                    if (m_zInputPos > 0)
                        --m_zInputPos;
                }
                break;

            case BUTTON_RIGHT:
                if (Input.IsCtrlDown())
                {
                    if (m_zInputPos == m_sInputLine.size())
                        break;

                    bool bFoundSpace(false);
                    while (m_zInputPos < m_sInputLine.size())
                    {
                        bool bIsSeperator(IsTokenSeparator(m_sInputLine[m_zInputPos]));
                        if (bIsSeperator)
                        {
                            bFoundSpace = true;
                        }
                        else
                        {
                            if (bFoundSpace)
                                break;
                        }
                        ++m_zInputPos;
                    }
                }
                else
                {
                    if (m_zInputPos < m_sInputLine.size())
                        ++m_zInputPos;
                }
                break;

            case BUTTON_HOME:
                if (ev.iFlags & IEVENT_CTRL)
                    OutputHistoryGoto(int(Std.GetSize()));
                else
                {
                    m_zInputPos = 0;
                }
                break;

            case BUTTON_END:
                if (ev.iFlags & IEVENT_CTRL)
                    OutputHistoryGoto(0);
                else
                {
                    m_zInputPos = m_sInputLine.length();
                }
                break;

            // Console history
            case BUTTON_PGUP:
                OutputHistoryScroll(m_iRows / 2);
                break;

            case BUTTON_PGDN:
                OutputHistoryScroll(-m_iRows / 2);
                break;

            // Command history
            case BUTTON_UP:
                if (ev.iFlags & IEVENT_CTRL)
                    OutputHistoryScroll(1);
                else
                {
                    if (m_iInputHistoryPos < int(m_deqInputHistory.size()) - 1)
                    {
                        if (m_iInputHistoryPos == -1)
                        {
                            m_sInputLineInProgress = m_sInputLine;
                            m_zInputPosInProgress = m_zInputPos;
                        }

                        ++m_iInputHistoryPos;
                        m_sInputLine = m_deqInputHistory[m_iInputHistoryPos];
                        m_zInputPos = m_sInputLine.size();
                    }
                }
                break;

            case BUTTON_DOWN:
                if (ev.iFlags & IEVENT_CTRL)
                    OutputHistoryScroll(-1);
                else
                {
                    if (m_iInputHistoryPos > 0)
                    {
                        --m_iInputHistoryPos;
                        m_sInputLine = m_deqInputHistory[m_iInputHistoryPos];
                        m_zInputPos = m_sInputLine.size();
                    }
                    else if (m_iInputHistoryPos == 0)
                    {
                        --m_iInputHistoryPos;
                        m_sInputLine = m_sInputLineInProgress;
                        m_zInputPos = m_zInputPosInProgress;
                    }
                }
                break;

            // Auto complete
            case BUTTON_TAB:
                {
                    if (m_zInputPos == 0)
                        break;

                    size_t zMatchStart(m_zInputPos);
                    while (zMatchStart > 0 && m_sInputLine[zMatchStart - 1] != _T(' ') && m_sInputLine[zMatchStart - 1] != _T(';'))
                        --zMatchStart;

                    bool bPerfectMatch(false);
                    tstring sTest(m_sInputLine.substr(zMatchStart));
                    if (sTest.empty())
                        break;

                    tstring sMatch(ConsoleRegistry.CompleteString(sTest, true, bPerfectMatch));
                    if (!sMatch.empty())
                    {
                        m_zInputPos = zMatchStart + sMatch.length();
                        m_sInputLine = m_sInputLine.substr(0, zMatchStart) + sMatch;

                        if (bPerfectMatch)
                        {
                            m_sInputLine += _T(' ');
                            ++m_zInputPos;
                        }
                    }
                }
                break;
            }
            break;

        case INPUT_CHARACTER:
            if (byte(ev.uID.chr) < 32)
                break;

            if (m_zInputPos != m_sInputLine.size())
                if (m_bOverwrite)
                    m_sInputLine.at(m_zInputPos) = ev.uID.chr;
                else
                    m_sInputLine.insert(m_zInputPos, 1, ev.uID.chr);
            else
                m_sInputLine.append(1, ev.uID.chr);

            ++m_zInputPos;
            break;

        default:
            break;
        }

        if (!bUsed)
            vUnused.push_back(ev);
    }

    for (vector<SIEvent>::iterator it(vUnused.begin()); it != vUnused.end(); ++it)
        Input.Push(*it);
}


/*====================
  CConsole::SetSize
  ====================*/
void    CConsole::SetSize(float fWidth, float fHeight)
{
    m_fWidth = fWidth;
    m_fHeight = fHeight;
}


/*====================
  CConsole::InsertAtCursor
  ====================*/
void    CConsole::InsertAtCursor(const tstring &sText)
{
    if (sText.empty())
        return;

    m_sInputLine = m_sInputLine.substr(0, m_zInputPos) + sText + m_sInputLine.substr(m_zInputPos);
    m_zInputPos += sText.size();
}


/*====================
  CConsole::Draw
  ====================*/
void    CConsole::Draw()
{
    PROFILE("CConsole::Draw");

    try
    {
        ResHandle hConsoleFont(g_ResourceManager.LookUpName(con_font, RES_FONTMAP));
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hConsoleFont));
        if (pFontMap == NULL)
            return;
        float fCharHeight(pFontMap->GetMaxHeight());

        if (!m_bActive && Host.GetSystemTime() >= m_uiToggleTime + con_toggleTime)
        {
            Input.SetCursorHidden(CURSOR_CONSOLE, BOOL_NOT_SET);
            Input.SetCursorConstrained(CURSOR_CONSOLE, BOOL_NOT_SET);
            Input.SetCursorFrozen(CURSOR_CONSOLE, BOOL_NOT_SET);
            Input.SetCursorRecenter(CURSOR_CONSOLE, BOOL_NOT_SET);

            int iNumNotifyLines(0);

            // Notify
            for (int i(0); i < int(Std.GetNotifySize()); ++i)
            {
                CConsoleStream *pStream = Std.GetNotifyLineStream(i);

                if (!Std.GetNotifyLine(i).empty())
                {
                    Draw2D.SetColor(BLACK);
                    Draw2D.String(con_leftMargin + 1.0f, fCharHeight * iNumNotifyLines + 1.0f, Std.GetNotifyLine(i), hConsoleFont);

                    Draw2D.SetColor(pStream->GetColor());
                    Draw2D.String(con_leftMargin, fCharHeight * iNumNotifyLines, Std.GetNotifyLine(i), hConsoleFont);
                }
                ++iNumNotifyLines;
            }
            return;
        }

        Input.SetCursorHidden(CURSOR_CONSOLE, BOOL_FALSE);
        Input.SetCursorConstrained(CURSOR_CONSOLE, BOOL_FALSE);
        Input.SetCursorFrozen(CURSOR_CONSOLE, BOOL_FALSE);
        Input.SetCursorRecenter(CURSOR_CONSOLE, BOOL_FALSE);

        float fOffset(con_toggleTime ? m_bActive ? MAX(1.0f - M_SmoothStepN(float(Host.GetSystemTime() - m_uiToggleTime) / con_toggleTime), 0.0f) : MIN(M_SmoothStepN(float(K2System.Milliseconds() - m_uiToggleTime) / con_toggleTime), 1.0f) : 0.0f);

        m_iRows = INT_FLOOR(Draw2D.GetScreenH() * m_fHeight / fCharHeight) - 1;

        // Background
        Draw2D.SetColor(CVec4f(con_bgColor[R], con_bgColor[G], con_bgColor[B], con_alpha));
        Draw2D.Rect(0.0f, -ROUND(fOffset * Draw2D.GetScreenH() * m_fHeight), Draw2D.GetScreenW() * m_fWidth, Draw2D.GetScreenH() * m_fHeight, g_ResourceManager.LookUpName(_T("console_background"), RES_TEXTURE));

        CVec4f conColor(con_color[R], con_color[G], con_color[B], 1.0f);

        // Output
        tsvector vsConsoleOutput;
        ColorVector vColors;
        int iNumLines(0);
        for (int i(0); i + m_iOutputHistoryPos < int(Std.GetSize()) && iNumLines < m_iRows; ++i)
        {
            CConsoleStream *pStream = Std.GetLineStream(i + m_iOutputHistoryPos);

            if (!IsStreamVisible(pStream))
                continue;

            vsConsoleOutput.push_back(Std.GetLine(i + m_iOutputHistoryPos));
            vColors.push_back(pStream->GetColor());

            ++iNumLines;
        }

        std::reverse(vsConsoleOutput.begin(), vsConsoleOutput.end());
        std::reverse(vColors.begin(), vColors.end());

        Draw2D.SetColor(Std.GetColor());
        Draw2D.String(con_leftMargin, 0.0f, Draw2D.GetScreenW() - con_leftMargin, fCharHeight * m_iRows - ROUND(fOffset * Draw2D.GetScreenH() * m_fHeight),
            vsConsoleOutput, vColors, hConsoleFont, con_wordWrap ? (DRAW_STRING_WRAP | DRAW_STRING_ANCHOR_BOTTOM) : DRAW_STRING_ANCHOR_BOTTOM);

        // Prompt
        Draw2D.String(con_leftMargin, fCharHeight * m_iRows - ROUND(fOffset * Draw2D.GetScreenH() * m_fHeight), con_prompt, hConsoleFont);

        // Input buffer
        Draw2D.String(pFontMap->GetStringWidth(con_prompt) + con_leftMargin, fCharHeight * m_iRows - ROUND(fOffset * Draw2D.GetScreenH() * m_fHeight), m_sInputLine, hConsoleFont);

        // Cursor
        if (int((Host.GetSystemTime() / 1000.0f) * m_fCursorBlinkRate) & 0x01)
        {
            StrColorMap vColors;
            StripColorCodes(m_sInputLine, vColors);
            if (!vColors.empty())
            {
                if (vColors.back().second.x < 0.0f)
                    Draw2D.SetColor(Std.GetColor());
                else
                    Draw2D.SetColor(vColors.back().second);
            }

            Draw2D.Rect(pFontMap->GetStringWidth(m_sInputLine.substr(0, m_zInputPos)) + con_leftMargin + pFontMap->GetStringWidth(con_prompt),
                fCharHeight * m_iRows + (m_bOverwrite ? 0 : (fCharHeight - 4.0f)) - ROUND(fOffset * Draw2D.GetScreenH() * m_fHeight),
                pFontMap->GetCharMapInfo(_T(' '))->m_fAdvance,
                m_bOverwrite ? fCharHeight : CONSOLE_CURSOR_HEIGHT);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CConsole::Draw() - "), NO_THROW);
    }
}


/*====================
  CConsole::ExecCmdBuffer
  Stuff to replace stuff in cmd.cpp
  ====================*/
void    CConsole::ExecCmdBuffer()
{
    for (CmdBufferVector_it it(m_vecCmdBuffer.begin()); it != m_vecCmdBuffer.end(); ++it)
        Execute(*it);

    m_vecCmdBuffer.clear();
}


/*====================
  CConsole::GotoScriptLabel
  ====================*/
void    CConsole::GotoScriptLabel(const tstring &sLabel)
{
    if (m_pActiveThread)
        m_pActiveThread->GotoScriptLabel(sLabel);
}


/*====================
  CConsole::PauseScript
  ====================*/
void    CConsole::PauseScript(dword dwMilliseconds)
{
    if (m_pActiveThread)
        m_pActiveThread->PauseScript(dwMilliseconds);
}


/*====================
  CConsole::Toggle
  ====================*/
void    CConsole::Toggle()
{
    if (!Host.HasClient())
        return;

    m_bActive = !m_bActive;

    if (Host.GetSystemTime() < m_uiToggleTime + con_toggleTime)
        m_uiToggleTime = 2 * Host.GetSystemTime() - con_toggleTime - m_uiToggleTime;
    else
        m_uiToggleTime = Host.GetSystemTime();
}


/*====================
  CConsole::FlushLogs
  ====================*/
void    CConsole::FlushLogs()
{
    if (m_hLogFile.IsOpen())
        m_hLogFile.Flush();
}


#if defined(linux) || defined(__APPLE__)
#include <ncurses.h>
CCursesConsole::CCursesConsole() :
m_iInputLineOffset(0),
m_bGeometryChanged(true)
{
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, true);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    char *term = getenv("TERM");
    if (term && string(term) == string("xterm"))
        m_bXTerm = true;
    else
        m_bXTerm = false;
    Console.Enable();
    Draw();
};

CCursesConsole::~CCursesConsole()
{
    Console.Disable();
    endwin();
};

void CCursesConsole::WrapLines(const tstring &sStr, tsvector &vsOut)
{
    if (sStr.length() <= m_iCols)
    {
        vsOut.push_back(sStr);
        return;
    }
    
    size_t zBreakPos(m_iCols-1);
    for (; zBreakPos > 0; --zBreakPos)
    {
        if (IsTokenSeparator(sStr[zBreakPos]))
            break;
    }
    
    if (zBreakPos == 0)
    {
        vsOut.push_back(sStr.substr(0, m_iCols));
        WrapLines(sStr.substr(m_iCols), vsOut);
    }
    else
    {
        vsOut.push_back(sStr.substr(0, zBreakPos));
        WrapLines(sStr.substr(zBreakPos + 1), vsOut);
    }
}

void CCursesConsole::Draw()
{
    if (m_bGeometryChanged)
    {
        getmaxyx(stdscr, m_iRows, m_iCols);
        Console.m_iRows = m_iRows;
        m_bGeometryChanged = false;
    }
    
    tsvector vsConsoleOutput;
    int iNumLines(0);
    for (int i(0); i + Console.m_iOutputHistoryPos < int(Console.Std.GetSize()) && iNumLines < m_iRows; ++i)
    {
        CConsoleStream *pStream = Console.Std.GetLineStream(i + Console.m_iOutputHistoryPos);
        
        if (!Console.IsStreamVisible(pStream))
            continue;
        
        vsConsoleOutput.push_back(Console.Std.GetLine(i + Console.m_iOutputHistoryPos));
        
        ++iNumLines;
    }
    
    // draw history
    int iRow(m_iRows-2); // save last line for input buffer
    for (tsvector_cit cit(vsConsoleOutput.begin()); cit != vsConsoleOutput.end() && iRow >= 0; ++cit)
    {
        tstring sClean(StripColorCodes(*cit));
        if (!con_wordWrap)
        {
            move(iRow, 0);
            hline(' ', m_iCols);
            printw("%s", TStringToString(sClean.substr(0, m_iCols)).c_str());
            --iRow;
        }
        else
        {
            tsvector vsLines;
            WrapLines(sClean, vsLines);
            while (!vsLines.empty() && iRow >= 0)
            {
                move(iRow, 0);
                hline(' ', m_iCols);
                printw("%s", TStringToString(vsLines.back()).c_str());
                vsLines.pop_back();
                --iRow;
            }
        }
    }
    
    // draw prompt & input buffer & set cursor
    int iVisibleBufferWidth(m_iCols - strlen(TStringToString(con_prompt).c_str()) - 1);
    move(m_iRows-1, 0);
    hline(' ', m_iCols);
    if (Console.m_sInputLine.length() > iVisibleBufferWidth)
    {
        if (Console.m_zInputPos < m_iInputLineOffset)
            m_iInputLineOffset = Console.m_zInputPos;
        else if (Console.m_zInputPos - m_iInputLineOffset > iVisibleBufferWidth)
            m_iInputLineOffset = Console.m_zInputPos - iVisibleBufferWidth;
    }
    else
    {
        m_iInputLineOffset = 0;
    }
    printw("%s%s", TStringToString(con_prompt).c_str(), TStringToString(Console.m_sInputLine.substr(m_iInputLineOffset, iVisibleBufferWidth)).c_str());
    
    move(m_iRows-1, strlen(TStringToString(con_prompt).c_str()) + Console.m_zInputPos - m_iInputLineOffset);
    if (Console.m_bOverwrite)
    {
        if (m_bXTerm)
            curs_set(1);
        else
            curs_set(2);
    }
    else
    {
        if (m_bXTerm)
        {
            curs_set(0);
            chgat(1, A_UNDERLINE, 1, NULL);
        }
        else
            curs_set(1);
    }
    
    refresh();
}
#endif // linux


/*--------------------
  cmdClear
  --------------------*/
CMD(Clear)
{
    CConsole::GetInstance()->Clear();
    return true;
}


/*--------------------
  cmdExec
  --------------------*/
CMD(Exec)
{
    if (vArgList.size() < 1)
    {
        (*CConsole::GetInstance()) << _T("syntax: exec <filename>") << newl;
        return false;
    }

    if (!CConsole::GetInstance()->ExecuteScript(vArgList[0]))
        CConsole::GetInstance()->Execute(vArgList[0]);

    return true;
}


/*--------------------
  cmdCall
  --------------------*/
CMD(Call)
{
    if (vArgList.size() < 1)
    {
        (*CConsole::GetInstance()) << _T("syntax: Call <filename>") << newl;
        return false;
    }

    if (!CConsole::GetInstance()->CallScript(vArgList[0]))
        CConsole::GetInstance()->Execute(vArgList[0]);

    return true;
}


/*--------------------
  cmdGoto
  --------------------*/
CMD(Goto)
{
    // must specify a label
    if (vArgList.empty())
        return false;

    CConsole::GetInstance()->GotoScriptLabel(vArgList[0]);

    return true;
}


/*--------------------
  cmdSleep
  --------------------*/
CMD(Sleep)
{
    // must specify a label
    if (vArgList.empty())
        return false;

    CConsole::GetInstance()->PauseScript(AtoI(vArgList[0]));

    return true;
}


/*--------------------
  cmdHitch
  --------------------*/
CMD(Hitch)
{
    if (vArgList.empty())
        return false;

    K2System.Sleep(AtoI(vArgList[0]));

    return true;
}


/*--------------------
  Break
  --------------------*/
CMD(Break)
{
#ifdef __GNUC__
#ifdef __ppc__
#else
    asm("int $0x03");
#endif
#else
    __asm int 0x03;
#endif
    return true;
}


/*--------------------
  cmdDo
  --------------------*/
CMD(Do)
{
    if (!vArgList.size())
    {
        (*CConsole::GetInstance()) << _T("syntax: do <variablename>") << newl;
        return false;
    }

    CConsole::GetInstance()->Execute(Cmd_GetObjectValueString(vArgList[0]));
    return true;
}


/*--------------------
  cmdIf

  Conditional execution
  --------------------*/
CMD(If)
{
    if (vArgList.size() < 2)
    {
        Console << _T("syntax: if <value OR variable> <command>") << newl;
        return false;
    }
    
    bool bValue(AtoB(vArgList[0]));

    if (bValue)
    {
        Console.Execute(ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));
        s_iLastIfResult = 1;
    }
    else
        s_iLastIfResult = 0;

    return true;
}


/*--------------------
  cmdElse

  Conditional execution, executes if the last 'if' command
  was not executed
  --------------------*/
CMD(Else)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: else <command>") << newl;
        return false;
    }

    if (s_iLastIfResult == -1)
    {
        Console << _T("error C2181: illegal else without matching if") << newl;
        return false;
    }

    if (s_iLastIfResult == 0)
        Console.Execute(ConcatinateArgs(vArgList));

    s_iLastIfResult = -1;

    return true;
}


/*--------------------
  cmdTime
  --------------------*/
CMD(Time)
{
    CDate date(true);   
    Console << date.GetTimeString() << _T(" ") << date.GetDateString(DATE_MONTH_FIRST) << newl; 
    return true;
}


/*--------------------
  cmdConsoleHistoryAppend
  --------------------*/
CMD(ConsoleHistoryAppend)
{
    if (!vArgList.empty())
        CConsole::GetInstance()->AddInputHistory(vArgList[0]);
    return true;
}


/*--------------------
  uicmdConsoleHistoryAppend
  --------------------*/
UI_VOID_CMD(ConsoleHistoryAppend, 1)
{
    if (!vArgList.empty())
        CConsole::GetInstance()->AddInputHistory(vArgList[0]->Evaluate());
}


/*--------------------
  cmdWriteConfigScript
  --------------------*/
CMD(WriteConfigScript)
{
    if (vArgList.empty())
    {
        Console << _T("syntax: WriteConfigScript <filename> [wildcard]") << newl;
        return false;
    }

    tstring sFilters(ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));
    Host.SaveConfig(vArgList[0], sFilters);
    return true;
}


/*--------------------
  cmdRepeat
  --------------------*/
CMD(Repeat)
{
    CConsole::GetInstance()->Execute(CConsole::GetInstance()->GetLastCmd());
    return true;
}


/*--------------------
  cmdFlushLogs
  --------------------*/
CMD(FlushLogs)
{
    CConsole::GetInstance()->FlushLogs();
    return true;
}


/*--------------------
  actToggleConsole
  --------------------*/
ACTION_IMPULSE(ToggleConsole)
{
    CConsole::GetInstance()->Toggle();
}


/*--------------------
  actCopyInputLine
  --------------------*/
ACTION_IMPULSE(CopyInputLine)
{
    K2System.CopyToClipboard(CConsole::GetInstance()->GetInputLine());
}


/*--------------------
  aPasteInputLine
  --------------------*/
ACTION_IMPULSE(PasteInputLine)
{
    if (K2System.IsClipboardString())
        CConsole::GetInstance()->InsertAtCursor(K2System.GetClipboardString());
}


/*--------------------
  CvarToggle
  --------------------*/
ACTION_BUTTON(CvarToggle)
{
    ICvar::SetString(sParam, XtoA(fValue));
}


/*--------------------
  Action: Cmd
  --------------------*/
ACTION_IMPULSE(Cmd)
{
    Console.Execute(sParam);
}


/*--------------------
  ToggleCvar
  --------------------*/
ACTION_IMPULSE(ToggleCvar)
{
    ICvar::Toggle(sParam);
}

