// (C)2007 S2 Games
// c_timermanager.h
//
//=============================================================================
#ifndef __C_TIMERMANAGER_H__
#define __C_TIMERMANAGER_H__

//=============================================================================
// Definitions
//=============================================================================
extern K2_API class CTimerManager *g_pTimerManager;
#define TimerManager (*g_pTimerManager)
//=============================================================================

//=============================================================================
// CTimer
//=============================================================================
class CTimer
{
private:
    tstring     m_sName;
    ULONGLONG   m_ullStartTime;
    ULONGLONG   m_ullTotalTime;

public:
    ~CTimer()   {}
    CTimer() :
    m_ullStartTime(0),
    m_ullTotalTime(0)
    {}

    CTimer(const tstring &sName) :
    m_sName(sName),
    m_ullStartTime(0),
    m_ullTotalTime(0)
    {}

    void    Start() { if (m_ullStartTime == 0) m_ullStartTime = K2System.GetTicks(); }
    void    Stop()  { if (m_ullStartTime != 0) m_ullTotalTime += K2System.GetTicks() - m_ullStartTime; m_ullStartTime = 0;}
    void    Reset() { m_ullStartTime = 0; m_ullTotalTime = 0; }

    ULONGLONG   GetTicks() const    { return m_ullTotalTime + ((m_ullStartTime == 0) ? 0 : K2System.GetTicks() - m_ullStartTime); }
    float       GetMs() const       { return GetTicks() * (1000.0f / K2System.GetFrequency()); }
};
//=============================================================================

//=============================================================================
// CTimerManager
//=============================================================================
class CTimerManager
{
private:
    SINGLETON_DEF(CTimerManager)

    typedef map<tstring, CTimer*>       TimerMap;
    typedef TimerMap::iterator          TimerMap_it;
    typedef TimerMap::const_iterator    TimerMap_cit;

    TimerMap    m_mapTimers;

public:
    ~CTimerManager()
    {
        for (TimerMap_it it(m_mapTimers.begin()); it != m_mapTimers.end(); ++it)
            SAFE_DELETE(it->second);
    }

    CTimer* AddTimer(const tstring &sName, bool bStart = true)
    {
        TimerMap_it it(m_mapTimers.find(sName));
        if (it != m_mapTimers.end())
            return it->second;

        CTimer *pNewTimer(K2_NEW(ctx_Default,  CTimer));
        if (pNewTimer == NULL)
            return NULL;

        m_mapTimers[sName] = pNewTimer;
        if (bStart)
            pNewTimer->Start();
        return pNewTimer;
    }

    void        DeleteTimer(const tstring &sName)
    {
        TimerMap_it it(m_mapTimers.find(sName));
        if (it == m_mapTimers.end())
            return;

        SAFE_DELETE(it->second);
        m_mapTimers.erase(it);
    }

    void        StartTimer(const tstring &sName)        { TimerMap_it it(m_mapTimers.find(sName)); if (it != m_mapTimers.end()) it->second->Start(); }
    void        StopTimer(const tstring &sName)         { TimerMap_it it(m_mapTimers.find(sName)); if (it != m_mapTimers.end()) it->second->Stop(); }
    void        ResetTimer(const tstring &sName)        { TimerMap_it it(m_mapTimers.find(sName)); if (it != m_mapTimers.end()) it->second->Reset(); }
    ULONGLONG   GetTicks(const tstring &sName) const    { TimerMap_cit it(m_mapTimers.find(sName)); if (it == m_mapTimers.end()) return 0; else return it->second->GetTicks(); }
    float       GetMs(const tstring &sName) const       { TimerMap_cit it(m_mapTimers.find(sName)); if (it == m_mapTimers.end()) return 0.0f; else return it->second->GetMs();}

    void        PrintTimes() const
    {
        for (TimerMap_cit it(m_mapTimers.begin()); it != m_mapTimers.end(); ++it)
            Console << it->first << _T(": ") << it->second->GetMs() << newl;
    }
};
//=============================================================================

#endif //__C_TIMERMANAGER_H__
