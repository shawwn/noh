// (C)2005 S2 Games
// c_console.h
//
//=============================================================================
#ifndef __C_CONSOLE_H__
#define __C_CONSOLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "xtoa.h"
#include "c_consoleelement.h"
#include "c_consoleregistry.h"
#include "c_filehandle.h"
#include "c_vec.h"
#include "stringutils.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CConsoleStream;
class CScriptThread;

typedef list<CScriptThread*> ThreadList;

extern K2_API class CConsole *g_pConsole;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int   DEFAULT_CONSOLE_INPUT_HISTORY_SIZE(64);

const int   DEFAULT_CONSOLE_OUTPUT_HISTORY_SIZE(10000);
const int   DEFAULT_CONSOLE_NOTIFY_HISTORY_SIZE(10);
const float CONSOLE_CURSOR_HEIGHT(2.0f);                        // in pixels

const float DEFAULT_CONSOLE_WIDTH(1.0f);
const float DEFAULT_CONSOLE_HEIGHT(1.0f);
const float DEFAULT_CONSOLE_FONT_WIDTH(10.0f);
const float DEFAULT_CONSOLE_FONT_HEIGHT(16.0f);
const float DEFAULT_CURSOR_BLINK_RATE(8.0f);

#ifndef K2_EXPORTS
#define Console (*g_pConsole)
#else
#define Console (*CConsole::GetInstance())
#endif

#define newl ((*g_pConsole).GetNewl())

struct SOutputLine
{
    tstring         sLine;
    CConsoleStream* pStream;

    SOutputLine() :
    pStream(nullptr)
    {
    }

    SOutputLine(const tstring &_sLine, CConsoleStream *_pStream) :
    sLine(_sLine),
    pStream(_pStream)
    {
    }
};


struct SNotifyLine
{
    int             iTime;
    tstring         sLine;
    CConsoleStream* pStream;

    SNotifyLine()
    {
    }

    SNotifyLine(int _iTime, const tstring &_sLine, CConsoleStream *_pStream) :
    iTime(_iTime),
    sLine(_sLine),
    pStream(_pStream)
    {
    }
};

tstring     Cmd_GetObjectValueString(const tstring &sObjname);
tstring     Cmd_GetObjectValueString2(const tstring &sObjname);
//=============================================================================


//=============================================================================
// CConsoleOutputBuffer
//=============================================================================
class CConsoleOutputBuffer
{
private:
    typedef deque<SOutputLine>      OutputHistoryDeque;

    static bool                     m_bReleased;
    static int                      m_iRefCount;
    static CConsoleOutputBuffer*    m_pInstance;

    size_t                          m_zMaxOutputHistory;
    OutputHistoryDeque              m_deqOutputHistory;
    bool                            m_bOutputLineInProgress;

    CConsoleOutputBuffer();
    CConsoleOutputBuffer(CConsoleOutputBuffer&);
    CConsoleOutputBuffer& operator=(CConsoleOutputBuffer&);

public:
    static CConsoleOutputBuffer*    GetInstance();
    static void                     Release();

    void            AddOutputHistory(const tstring &sLine, CConsoleStream &Stream);
    void            SetMaxOutputHistory(size_t size);

    void            Clear()                 { m_deqOutputHistory.clear(); }
    size_t          GetSize()               { return m_deqOutputHistory.size(); }
    const tstring&  GetLine(size_t n)       { return m_deqOutputHistory[n].sLine; }
    CConsoleStream* GetLineStream(size_t n) { return m_deqOutputHistory[n].pStream; }
};
//=============================================================================

//=============================================================================
// CConsoleNotifyBuffer
//=============================================================================
class CConsoleNotifyBuffer
{
private:
    typedef deque<SNotifyLine>      NotifyHistoryDeque;

    static bool                     m_bReleased;
    static int                      m_iRefCount;
    static CConsoleNotifyBuffer*    m_pInstance;

    size_t                          m_zMaxHistory;
    NotifyHistoryDeque              m_deqHistory;
    bool                            m_bLineInProgress;

    CConsoleNotifyBuffer();
    CConsoleNotifyBuffer(CConsoleNotifyBuffer&);
    CConsoleNotifyBuffer& operator=(CConsoleNotifyBuffer&);

public:
    static CConsoleNotifyBuffer*    GetInstance();
    static void                     Release();

    void            Frame();

    void            AddHistory(const tstring &sLine, CConsoleStream &Stream);
    void            SetMaxHistory(size_t size);

    void            Clear()             { m_deqHistory.clear(); }
    size_t          GetSize()           { return m_deqHistory.size(); }
    const tstring&  GetLine(size_t n)   { return m_deqHistory[n].sLine; }
    CConsoleStream* GetLineStream(size_t n) { return m_deqHistory[n].pStream; }
};
//=============================================================================

//=============================================================================
// CConsoleStream
//=============================================================================
class CConsoleStream
{
private:
    CConsoleOutputBuffer*   m_pOutput;
    CConsoleNotifyBuffer*   m_pNotify;

    K2_API void     AddOutputHistoryT(const tstring &sLine);
    void            AddOutputHistory(const string &sLine)       { AddOutputHistoryT(StringToTString(sLine)); }
    void            AddOutputHistory(const wstring &sLine)      { AddOutputHistoryT(WStringToTString(sLine)); }

    bool            m_bEnabled;
    CVec4f          m_v4Color;
    tstring         m_sPrefix;

    CConsoleStream();
    CConsoleStream(CConsoleStream&);
    CConsoleStream& operator=(CConsoleStream&);

public:
    CConsoleStream(const CVec4f &v4Color, const tstring &sPrefix);
    ~CConsoleStream()                                   { m_pOutput->Release(); }

    void            Frame()                             { m_pNotify->Frame(); }
    bool            IsEnabled()                         { return m_bEnabled; } // UTTAR
    void            Enable()                            { m_bEnabled = true; }
    void            Disable()                           { m_bEnabled = false; }
    void            Toggle()                            { m_bEnabled = !m_bEnabled; }
    void            Clear()                             { m_pOutput->Clear(); }

    void            SetMaxOutputHistory(size_t size)    { m_pOutput->SetMaxOutputHistory(size); }

    const CVec4f&   GetColor()                          { return m_v4Color; }
    const tstring&  GetPrefix()                         { return m_sPrefix; }
    size_t          GetSize()                           { return m_pOutput->GetSize(); }
    const tstring&  GetLine(size_t n)                   { return m_pOutput->GetLine(n); }
    CConsoleStream* GetLineStream(size_t n)             { return m_pOutput->GetLineStream(n); }

    size_t          GetNotifySize()                     { return m_pNotify->GetSize(); }
    const tstring&  GetNotifyLine(size_t n)             { return m_pNotify->GetLine(n); }
    CConsoleStream* GetNotifyLineStream(size_t n)       { return m_pNotify->GetLineStream(n); }

    CConsoleStream& operator<<(const char *sz)          { AddOutputHistory(sz); return *this; }
    CConsoleStream& operator<<(const wchar_t *sz)       { AddOutputHistory(sz); return *this; }
    CConsoleStream& operator<<(const string &s)         { AddOutputHistory(s); return *this; }
    CConsoleStream& operator<<(const wstring &s)        { AddOutputHistory(s); return *this; }
    CConsoleStream& operator<<(const void *p)           { *this << XtoA(p); return *this; }
    CConsoleStream& operator<<(char c);
    CConsoleStream& operator<<(wchar_t c);
    CConsoleStream& operator<<(int i)                   { *this << XtoA(i); return *this; }
    CConsoleStream& operator<<(long l)                  { *this << XtoA(l); return *this; }
    CConsoleStream& operator<<(unsigned int ui)         { *this << XtoA(ui); return *this; }
    CConsoleStream& operator<<(unsigned long ul)        { *this << XtoA(ul); return *this; }
    CConsoleStream& operator<<(float f)                 { *this << XtoA(f); return *this; }
    CConsoleStream& operator<<(double d)                { *this << XtoA(d); return *this; }
    CConsoleStream& operator<<(LONGLONG ll)             { *this << XtoA(ll); return *this; }
    CConsoleStream& operator<<(ULONGLONG ull)           { *this << XtoA(ull); return *this; }
    CConsoleStream& operator<<(const CVec2f &v2)        { *this << XtoA(v2); return *this; }
    CConsoleStream& operator<<(const CVec3f &v3)        { *this << XtoA(v3); return *this; }
    CConsoleStream& operator<<(const CVec4f &v4)        { *this << XtoA(v4); return *this; }
};

/*====================
  CConsoleStream::operator<<
  ====================*/
inline
CConsoleStream& CConsoleStream::operator<<(char c)
{
    char sz[2] = { 0, 0 };
    sz[0] = c;

    *this << sz;
    return *this;
}

inline
CConsoleStream& CConsoleStream::operator<<(wchar_t c)
{
    wchar_t sz[2] = { 0, 0 };
    sz[0] = c;

    *this << sz;
    return *this;
}
//=============================================================================

//=============================================================================
// CConsole
//=============================================================================
class CConsole
{
SINGLETON_DEF(CConsole)

    friend class CConsoleOutputBuffer;
#if defined(linux) || defined(__APPLE__)
    friend class CCursesConsole;
#endif

private:
    typedef deque<tstring>                          InputHistoryDeque;
    typedef InputHistoryDeque::iterator             InputHistoryDeque_it;
    typedef InputHistoryDeque::const_iterator       InputHistoryDeque_cit;
    typedef InputHistoryDeque::reverse_iterator     InputHistoryDeque_rit;

    typedef vector<tstring>                         CmdBufferVector;
    typedef CmdBufferVector::iterator               CmdBufferVector_it;
    typedef CmdBufferVector::const_iterator         CmdBufferVector_cit;
    typedef CmdBufferVector::reverse_iterator       CmdBufferVector_rit;

    bool                m_bActive;

    CConsoleStream*     m_pDefaultStream;

    size_t              m_zMaxInputHistory;
    InputHistoryDeque   m_deqInputHistory;
    int                 m_iInputHistoryPos;

    CmdBufferVector     m_vecCmdBuffer;

    int                 m_iOutputHistoryPos;

    tstring             m_sInputLine;
    size_t              m_zInputPos;
    bool                m_bOverwrite;

    tstring             m_sInputLineInProgress;
    size_t              m_zInputPosInProgress;

    float               m_fCursorBlinkRate;
    float               m_fWidth, m_fHeight;
    int                 m_iRows, m_iShiftPgRows, m_iPgRows;

    int                 m_iScriptsRunning;
    CScriptThread*      m_pActiveThread;
    ThreadList          m_lScriptThreads;

    CFileHandle         m_hLogFile;

    tstring             m_newl;

    int                 m_iDedicatedConsolePos;
    uint                m_uiToggleTime;

    bool                m_bWatch;
    tstring             m_sWatch;

    bool    IsStreamVisible(CConsoleStream *pStream);

public:
    ~CConsole() {}

    CConsoleStream  Std, Dev, Err, Warn, Net, Server, Client, UI, Perf, Mem, AI, Video, ServerGame, ClientGame, Res, GroupVoice, GroupVoiceDebug, Script;
    
    void                    SetDefaultStream(CConsoleStream &stream)    { m_pDefaultStream = &stream; }
    CConsoleStream&         DefaultStream()                         { assert(m_pDefaultStream != nullptr); return *m_pDefaultStream; }

    const tstring&          GetNewl() const                         { return m_newl; }

    void                    Enable()                                { m_bActive = true; }
    void                    Disable()                               { m_bActive = false; }
    void                    Toggle();
    bool                    IsActive() const                        { return m_bActive; }
    void                    Clear()                                 { m_iOutputHistoryPos = 0; Std.Clear(); }

    K2_API void             OpenLog();
    K2_API void             CloseLog();
    
    K2_API void             WriteLog(const tstring &s);

    CConsoleElement*        GetElement(const tstring &sName)        { return ConsoleRegistry.GetElement(sName); }

    K2_API void             SetMaxInputHistory(size_t size);
    K2_API void             SetMaxOutputHistory(size_t size);
    K2_API bool             WriteConsoleHistory(CFileHandle &hFile);
    K2_API void             AddInputHistory(const tstring &sLine);

    K2_API void             AddCmdBuffer(const tstring &sLine);
    void                    ClearCmdBuffer()                        { m_vecCmdBuffer.clear(); }

    K2_API void             ExecCmdBuffer();

    size_t                  GetInputHistorySize() const             { return m_deqInputHistory.size(); }

    K2_API void             Frame();

    K2_API void             DoCommand(const tsvector &vArgList);
    K2_API void             Execute(const tstring &sCmdLine);
    K2_API bool             ExecuteScript(const tstring &sData, bool bFile = true, tsmapts *mapParams = nullptr);
    bool                    CallScript(const tstring &sFilename);
    void                    GotoScriptLabel(const tstring &sLabel);
    void                    PauseScript(dword dwMilliseconds);

    K2_API void             PrecacheCommand(const tstring &sCmdLine);
    K2_API void             DoPrecache(const tsvector &vArgList);

    CConsoleStream&         operator<<(void *p)                     { return (*m_pDefaultStream) << p; }
    CConsoleStream&         operator<<(const TCHAR *sz)             { return (*m_pDefaultStream) << sz; }
    CConsoleStream&         operator<<(const tstring &s)            { return (*m_pDefaultStream) << s; }
    CConsoleStream&         operator<<(char c)                      { return (*m_pDefaultStream) << c; }
    CConsoleStream&         operator<<(wchar_t c)                   { return (*m_pDefaultStream) << c; }
    CConsoleStream&         operator<<(int i)                       { return (*m_pDefaultStream) << i; }
    CConsoleStream&         operator<<(unsigned int ui)             { return (*m_pDefaultStream) << ui; }
    CConsoleStream&         operator<<(long l)                      { return (*m_pDefaultStream) << l; }
    CConsoleStream&         operator<<(unsigned long ul)            { return (*m_pDefaultStream) << ul; }
    CConsoleStream&         operator<<(float f)                     { return (*m_pDefaultStream) << f; }
    CConsoleStream&         operator<<(double d)                    { return (*m_pDefaultStream) << d; }
    CConsoleStream&         operator<<(LONGLONG ll)                 { return (*m_pDefaultStream) << ll; }
    CConsoleStream&         operator<<(ULONGLONG ull)               { return (*m_pDefaultStream) << ull; }
    CConsoleStream&         operator<<(const CVec2f &v2)            { return (*m_pDefaultStream) << v2; }
    CConsoleStream&         operator<<(const CVec3f &v3)            { return (*m_pDefaultStream) << v3; }
    CConsoleStream&         operator<<(const CVec4f &v4)            { return (*m_pDefaultStream) << v4; }

    K2_API void             SetSize(float fWidth, float fHeight);
    void                    SetCursorBlinkRate(float fRate)         { m_fCursorBlinkRate = fRate; }
    void                    SetInputLine(const tstring &sText)      { m_sInputLine = sText; m_zInputPos = sText.length(); }
    const tstring&          GetInputLine()                          { return m_sInputLine; }
    void                    InsertAtCursor(const tstring &sText);
    uint                    GetInputPos() const                     { return uint(m_zInputPos); }

    int                     GetOutputHistoryPos()                   { return m_iOutputHistoryPos; }
    void                    OutputHistoryScroll(int iAmount);
    void                    OutputHistoryGoto(int iPos);

    const tstring&          GetLastCmd()                            { return m_deqInputHistory.empty() ? TSNULL : m_deqInputHistory[0]; }

    K2_API void             Draw();

    void                    StartWatch()        { m_bWatch = true; m_sWatch.clear(); }
    void                    EndWatch()          { m_bWatch = false; m_sWatch.clear(); }
    bool                    IsWatching()        { return m_bWatch; }
    const tstring&          GetWatchBuffer()    { return m_sWatch; }

    void                    Watch(const tstring &s)
    {
        if (m_bWatch)
            m_sWatch += s;
    }

    K2_API void             FlushLogs();
};
//=============================================================================

#if defined(linux) || defined(__APPLE__)
class CCursesConsole
{
private:
    int m_iRows, m_iCols;
    int m_iInputLineOffset;
    bool m_bGeometryChanged;
    bool m_bXTerm;
    void WrapLines(const tstring &sStr, tsvector &vsOut);
public:
    CCursesConsole();
    ~CCursesConsole();
    
    void SetGeometryChanged(bool bGeometryChanged) { m_bGeometryChanged = bGeometryChanged; }
    void Draw();
};
#endif

#endif // __C_CONSOLE_H__
