// (C)2007 S2 Games
// c_timermaner.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_timermanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CTimerManager)

CTimerManager *g_pTimerManager(CTimerManager::GetInstance());
//=============================================================================

/*====================
  CTimerManager::CTimerManager
  ====================*/
CTimerManager::CTimerManager()
{
}


/*--------------------
  PrintTimes
  --------------------*/
CMD(PrintTimes)
{
    TimerManager.PrintTimes();
    return true;
}


/*--------------------
  StartTimer
  --------------------*/
CMD(StartTimer)
{
    if (vArgList.empty())
        return false;

    CTimer *pTimer(TimerManager.AddTimer(vArgList[0]));
    if (pTimer != nullptr)
        pTimer->Start();
    return true;
}


/*--------------------
  StopTimer
  --------------------*/
CMD(StopTimer)
{
    if (vArgList.empty())
        return false;

    TimerManager.StopTimer(vArgList[0]);
    return true;
}


/*--------------------
  ResetTimer
  --------------------*/
CMD(ResetTimer)
{
    if (vArgList.empty())
        return false;

    TimerManager.ResetTimer(vArgList[0]);
    return true;
}
