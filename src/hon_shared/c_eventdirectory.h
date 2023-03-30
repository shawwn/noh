// (C)2006 S2 Games
// c_eventdirectory.h
//
//=============================================================================
#ifndef __C_EVENTDIRECTORY_H__
#define __C_EVENTDIRECTORY_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_gameevent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSnapshot;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int MAX_ACTIVE_GAME_EVENTS(256);
//=============================================================================

//=============================================================================
// CEventDirectory
//=============================================================================
class CEventDirectory
{
private:
	uint				m_uiNextEvent;
	deque<CGameEvent>	m_deqEvents;
	CBufferDynamic		m_buffer;

public:
	GAME_SHARED_API ~CEventDirectory();
	GAME_SHARED_API CEventDirectory();

	GAME_SHARED_API void	Clear();
	GAME_SHARED_API uint	AddEvent(const CGameEvent &ev);		
	GAME_SHARED_API void	DeleteEvent(uint uiEvent);
	GAME_SHARED_API void	GetSnapshot(CSnapshot &snapshot);
	GAME_SHARED_API void	Frame();
	GAME_SHARED_API void	SynchNewEvents();
	GAME_SHARED_API void	DeleteRelatedEvents(uint uiIndex);
};
//=============================================================================

#endif //__C_EVENTDIRECTORY_H__
