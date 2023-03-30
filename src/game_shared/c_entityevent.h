// (C)2006 S2 Games
// c_entityevent.h
//
//=============================================================================
#ifndef __C_ENTITYEVENT_H__
#define __C_ENTITYEVENT_H__

//=============================================================================
// Definitions
//=============================================================================
enum EEntityEvent
{
	ENTITY_EVENT_NULL,

	ENTITY_EVENT_CHECK,
	NUM_ENTITY_EVENTS
};
//=============================================================================

//=============================================================================
// CEntityEvent
//=============================================================================
class CEntityEvent
{
private:
	EEntityEvent	m_eEvent;
	bool			m_bCompleted;

public:
	~CEntityEvent()														{}
	CEntityEvent() : m_eEvent(ENTITY_EVENT_NULL), m_bCompleted(false)	{}

	
	void			Reset(EEntityEvent eEvent)	{ m_eEvent = eEvent; m_bCompleted = false;}
	EEntityEvent	GetEvent() const			{ return m_eEvent; }
	byte			GetByte() const				{ return m_eEvent; }
	bool			IsCompleted() const			{ return m_bCompleted; }
	void			MarkCompleted()				{ m_bCompleted = true; }
};
//=============================================================================

#endif //__C_ENTITYEVENT_H__
