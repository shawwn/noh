// (C)2006 S2 Games
// c_eventdirectory.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "c_eventdirectory.h"

#include "../k2/c_snapshot.h"
//=============================================================================

/*====================
  CEventDirectory::~CEventDirectory
  ====================*/
CEventDirectory::~CEventDirectory()
{
}


/*====================
  CEventDirectory::CEventDirectory
  ====================*/
CEventDirectory::CEventDirectory() :
m_uiNextEvent(0),
m_buffer(40)
{
}


/*====================
  CEventDirectory::GetSnapshot
  ====================*/
void    CEventDirectory::GetSnapshot(CSnapshot &snapshot)
{
    for (deque<CGameEvent>::iterator it(m_deqEvents.begin()); it != m_deqEvents.end(); ++it)
    {
        it->GetBuffer(m_buffer);
        snapshot.AddEventSnapshot(m_buffer);
    }
}


/*====================
  CEventDirectory::Frame
  ====================*/
void    CEventDirectory::Frame()
{
    PROFILE("CEventDirectory::Frame");

    deque<CGameEvent>::iterator it(m_deqEvents.begin());

    while (it != m_deqEvents.end())
    {
        if (!it->Frame())
        {
            it->Clear();
            it = m_deqEvents.erase(it);
            continue;
        }
        it->AddToScene();
        ++it;
    }
}


/*====================
  CEventDirectory::SynchNewEvents
  ====================*/
void    CEventDirectory::SynchNewEvents()
{
    PROFILE("CEventDirectory::SynchNewEvents");

    deque<CGameEvent>::iterator it(m_deqEvents.begin());
    while (it != m_deqEvents.end())
    {
        if (it->IsNew())
        {
            it->SynchWithEntity();
            it->MarkAsOld();
        }
        ++it;
    }
}


/*====================
  CEventDirectory::AddEvent
  ====================*/
uint    CEventDirectory::AddEvent(const CGameEvent &ev)     
{
    PROFILE("CEventDirectory::AddEvent");

    m_deqEvents.push_back(ev);

    uint uiEvent(m_uiNextEvent);
    m_deqEvents.back().SetIndex(uiEvent);
    if (m_deqEvents.size() > MAX_ACTIVE_GAME_EVENTS)
    {
        m_deqEvents.front().Clear();
        m_deqEvents.pop_front();
    }

    ++m_uiNextEvent;

    return uiEvent;
}


/*====================
  CEventDirectory::DeleteEvent
  ====================*/
void    CEventDirectory::DeleteEvent(uint uiEvent)      
{
    for (deque<CGameEvent>::iterator it(m_deqEvents.begin()); it != m_deqEvents.end(); ++it)
    {
        if (it->GetIndex() == uiEvent)
        {
            it->Clear();
            m_deqEvents.erase(it);
            return;
        }
    }
}


/*====================
  CEventDirectory::Clear
  ====================*/
void    CEventDirectory::Clear()
{
    for (deque<CGameEvent>::iterator it(m_deqEvents.begin()); it != m_deqEvents.end(); ++it)
        it->Clear();

    m_deqEvents.clear();
}


/*====================
  CEventDirectory::DeleteRelatedEvents
  ====================*/
void    CEventDirectory::DeleteRelatedEvents(uint uiIndex)
{
    deque<CGameEvent>::iterator it(m_deqEvents.begin());
    while (it != m_deqEvents.end())
    {
        if (!it->IsNew() &&
            (it->GetSourceEntityIndex() == uiIndex || it->GetTargetEntityIndex() == uiIndex))
        {
            it->Clear();
            it = m_deqEvents.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
